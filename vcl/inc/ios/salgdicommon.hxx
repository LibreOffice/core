/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _VCL_IOS_SALGDICOMMON_H
#define _VCL_IOS_SALGDICOMMON_H

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

#endif /* _VCL_IOS_SALGDICOMMON_H */
