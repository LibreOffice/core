/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef _VCL_AQUA_SALGDICOMMON_H
#define _VCL_AQUA_SALGDICOMMON_H

// abstracting quartz color instead of having to use an CGFloat[] array
class RGBAColor
{
public:
    RGBAColor( SalColor );
    RGBAColor( float fRed, float fGreen, float fBlue, float fAlpha ); //NOTUSEDYET
    const float* AsArray() const { return &m_fRed; }
    bool IsVisible() const { return m_fAlpha > 0; }
    void SetAlpha( float fAlpha ) { m_fAlpha = fAlpha; }
private:
    float m_fRed, m_fGreen, m_fBlue, m_fAlpha;
};

inline RGBAColor::RGBAColor( SalColor nSalColor )
:   m_fRed( SALCOLOR_RED(nSalColor) * (1.0/255))
,   m_fGreen( SALCOLOR_GREEN(nSalColor) * (1.0/255))
,   m_fBlue( SALCOLOR_BLUE(nSalColor) * (1.0/255))
,   m_fAlpha( 1.0 )  // opaque
{}

inline RGBAColor::RGBAColor( float fRed, float fGreen, float fBlue, float fAlpha )
:   m_fRed( fRed )
,   m_fGreen( fGreen )
,   m_fBlue( fBlue )
,   m_fAlpha( fAlpha )
{}

class XorEmulation
{
public:
    XorEmulation();
    ~XorEmulation();

    void SetTarget( int nWidth, int nHeight, int nBitmapDepth, CGContextRef, CGLayerRef );
    bool UpdateTarget();
    void Enable() { m_bIsEnabled = true; }
    void Disable() { m_bIsEnabled = false; }
    bool IsEnabled() const { return m_bIsEnabled; }
    CGContextRef GetTargetContext() const { return m_xTargetContext; }
    CGContextRef GetMaskContext() const { return (m_bIsEnabled ? m_xMaskContext : NULL); }

private:
    CGLayerRef m_xTargetLayer;
    CGContextRef m_xTargetContext;
    CGContextRef m_xMaskContext;
    CGContextRef m_xTempContext;
    sal_uLong* m_pMaskBuffer;
    sal_uLong* m_pTempBuffer;
    int m_nBufferLongs;
    bool m_bIsEnabled;
};

#endif /* _VCL_AQUA_SALGDICOMMON_H */
