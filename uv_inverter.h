/**
 * @file
 * DirectShow filter - UV invert
 * 
 * @author Leav Wu (leavinel@gmail.com)
 */

#ifndef UV_INVERT_H_
#define UV_INVERT_H_

#include <stdio.h>
#include <streams.h>

DEFINE_GUID (CLSID_UVInverter,
    0x8a8772f2, 0x6458, 0x40be, 0xa2, 0xff, 0x0d, 0x2a, 0x25, 0xd9, 0x59, 0xa2);

class UVInverter: public CTransformFilter
{
public:
    UVInverter (void);
    ~UVInverter (void);
    HRESULT CheckInputType (const CMediaType *ptype);
    HRESULT CheckTransform (const CMediaType *pin, const CMediaType *pout);
    HRESULT DecideBufferSize (IMemAllocator *palloc, ALLOCATOR_PROPERTIES *pprop);
    HRESULT GetMediaType (int pos, CMediaType *ptype);
    HRESULT Transform (IMediaSample *pin, IMediaSample *pout);

private:
    /** Log file */
    FILE *flog;
    void log (const char *s);
    void logf (const char *s_fmt, ...);
    void lognl (void);
    void log_guid (const GUID *guid);
};


#endif /* UV_INVERT_H_ */
