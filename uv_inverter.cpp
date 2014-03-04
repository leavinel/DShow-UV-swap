/**
 * @file
 * DirectShow filter - UV invert
 * 
 * @author Leav Wu (leavinel@gmail.com)
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <windows.h>
#include <dshow.h>

#include <initguid.h>
#include "uv_inverter.h"


UVInverter::UVInverter (void):
    CTransformFilter (NAME("UV inverter"), NULL, CLSID_UVInverter),
    flog (NULL)
{
#ifdef DEBUG
    flog = fopen ("z:\\log.txt", "w");
    assert (flog);
#endif
}

UVInverter::~UVInverter (void)
{
#ifdef DEBUG
    fclose (flog);
#endif
}

/**
 * Write a string to log file
 */
void UVInverter::log (const char *s)
{
#ifdef DEBUG
    fputs (s, flog);
#endif
}

/**
 * fprintf() to log file
 */
void UVInverter::logf (const char *s_fmt, ...)
{
#ifdef DEBUG
    va_list ap;

    va_start (ap, s_fmt);
    vfprintf (flog, s_fmt, ap);
    va_end (ap);
#endif
}

/**
 * Newline log file
 */
void UVInverter::lognl (void)
{
#ifdef DEBUG
    fputc ('\n', flog);
#endif
}

/**
 * Write GUID to log file
 */
void UVInverter::log_guid (const GUID *guid)
{
#ifdef DEBUG
    int i;

    logf ("%08lx-%04x-%04x-", guid->Data1, guid->Data2, guid->Data3);

    for (i = 0; i < 8; i++)
        logf ("%02x", guid->Data4[i]);
#endif
}

HRESULT UVInverter::CheckInputType (const CMediaType *ptype)
{
    /* Check if it's a video stream */
    if (*ptype->FormatType() != FORMAT_VideoInfo
     || *ptype->Type() != MEDIATYPE_Video)
        return VFW_E_TYPE_NOT_ACCEPTED;

    /* Check the color space */
    const GUID *subtype = ptype->Subtype();

    if (*subtype != MEDIASUBTYPE_UYVY)
        return VFW_E_TYPE_NOT_ACCEPTED;

    return S_OK;
}

HRESULT UVInverter::GetMediaType (int pos, CMediaType *ptype)
{
    if (pos < 0)
        return E_INVALIDARG;

    if (pos > 1)
        return VFW_S_NO_MORE_ITEMS;

    m_pInput->ConnectionMediaType (ptype);
    return S_OK;
}

HRESULT UVInverter::CheckTransform (const CMediaType *pin, const CMediaType *pout)
{
    if (*pout->FormatType() != FORMAT_VideoInfo
     || *pout->Type() != MEDIATYPE_Video)
        return VFW_E_TYPE_NOT_ACCEPTED;

    return S_OK;
}

HRESULT UVInverter::DecideBufferSize (IMemAllocator *palloc, ALLOCATOR_PROPERTIES *pprop)
{
    ALLOCATOR_PROPERTIES req, actual;
    IMemAllocator *palloc_in;
    HRESULT hr;

    ASSERT (m_pInput->IsConnected());
    hr = m_pInput->GetAllocator (&palloc_in);

    if (FAILED(hr))
        return hr;

    palloc_in->GetProperties (&req);

    pprop->cBuffers = req.cBuffers;
    pprop->cbBuffer = req.cbBuffer;
    pprop->cbAlign = req.cbAlign;

    if (pprop->cBuffers <= 0)
        pprop->cBuffers = 1;

    if (pprop->cbBuffer <= 0)
        pprop->cbBuffer = 1;

    hr = palloc->SetProperties (pprop, &actual);

    if (FAILED(hr))
        return hr;

    if (  (req.cBuffers > actual.cBuffers)
       || (req.cbBuffer > actual.cbBuffer)
       || (req.cbAlign  > actual.cbAlign))
        return E_FAIL;

    return NOERROR;
}

HRESULT UVInverter::Transform (IMediaSample *pin, IMediaSample *pout)
{
    HRESULT hr;
    DWORD *src, *dst, pix, tmp;
    AM_MEDIA_TYPE *pmt;
    const VIDEOINFOHEADER *phdr;
    int x, y;

    hr = pin->GetPointer ((BYTE**)&src);

    if (FAILED (hr))
        return hr;

    hr = pout->GetPointer ((BYTE**)&dst);

    if (FAILED (hr))
        return hr;

    pmt = &m_pInput->CurrentMediaType();
    phdr = (VIDEOINFOHEADER*)pmt->pbFormat;

    for (y = 0; y < phdr->bmiHeader.biHeight; y++)
    {
        for (x = 0; x < phdr->bmiHeader.biWidth; x+=2) // 32-bit contains 2 pixels
        {
            /*
             *  3   2   1   0
             * Y1  V1  Y0  U0
             */
            pix = *(src++);
            tmp = pix & 0xFF00FF;
            *(dst++) = (pix & 0xFF00FF00) | (tmp >> 16) | (tmp << 16);
        }
    }

    return NOERROR;
}
