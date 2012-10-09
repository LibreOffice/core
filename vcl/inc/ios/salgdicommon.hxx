/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
