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

#ifndef INCLUDED_VCL_UNX_GENERIC_GDI_XRENDER_PEER_HXX
#define INCLUDED_VCL_UNX_GENERIC_GDI_XRENDER_PEER_HXX

#include <prex.h>
struct _XTrap; // on some older systems this is not declared within Xrender.h
#include <X11/extensions/Xrender.h>
#include <postx.h>

#include <vcl/salgtype.hxx>
#include <osl/module.h>

typedef Glyph XRenderGlyph;

class XRenderPeer
{
public:
    static XRenderPeer& GetInstance();

private:
                        XRenderPeer();
    void                InitRenderLib();

    Display*            mpDisplay;
    XRenderPictFormat*  mpStandardFormatA8;

public:
    XRenderPictFormat* GetStandardFormatA8() const;
    XRenderPictFormat* FindStandardFormat(int nFormat) const;

    // the methods below are thin wrappers for the XRENDER API
    XRenderPictFormat* FindVisualFormat( Visual* ) const;
    XRenderPictFormat* FindPictureFormat( unsigned long nMask,
        const XRenderPictFormat& ) const;
    Picture     CreatePicture( Drawable, const XRenderPictFormat*,
                    unsigned long nDrawable, const XRenderPictureAttributes* ) const;
    void        ChangePicture( Picture, unsigned long nValueMask,
                    const XRenderPictureAttributes* ) const;
    void        SetPictureClipRegion( Picture, Region ) const;
    void        CompositePicture( int nOp, Picture aSrc, Picture aMask, Picture aDst,
                    int nXSrc, int nYSrc, int nXMask, int nYMask,
                    int nXDst, int nYDst, unsigned nWidth, unsigned nHeight ) const;
    void        FreePicture( Picture ) const;

    void        FillRectangle( int nOp, Picture aDst, const XRenderColor*,
                               int nX, int nY, unsigned nW, unsigned nH ) const;
    void        CompositeTrapezoids( int nOp, Picture aSrc, Picture aDst,
                    const XRenderPictFormat*, int nXSrc, int nYSrc,
                    const XTrapezoid*, int nCount ) const;
};

inline XRenderPictFormat* XRenderPeer::GetStandardFormatA8() const
{
    return mpStandardFormatA8;
}

inline XRenderPictFormat* XRenderPeer::FindStandardFormat(int nFormat) const
{
    return XRenderFindStandardFormat(mpDisplay, nFormat);
}

inline XRenderPictFormat* XRenderPeer::FindVisualFormat( Visual* pVisual ) const
{
    return XRenderFindVisualFormat ( mpDisplay, pVisual );
}

inline XRenderPictFormat* XRenderPeer::FindPictureFormat( unsigned long nFormatMask,
    const XRenderPictFormat& rFormatAttr ) const
{
    return XRenderFindFormat( mpDisplay, nFormatMask, &rFormatAttr, 0 );
}

inline Picture XRenderPeer::CreatePicture( Drawable aDrawable,
    const XRenderPictFormat* pVisFormat, unsigned long nValueMask,
    const XRenderPictureAttributes* pRenderAttr ) const
{
    return XRenderCreatePicture( mpDisplay, aDrawable, pVisFormat,
                                 nValueMask, pRenderAttr );
}

inline void XRenderPeer::ChangePicture( Picture aPicture,
    unsigned long nValueMask, const XRenderPictureAttributes* pRenderAttr ) const
{
    XRenderChangePicture( mpDisplay, aPicture, nValueMask, pRenderAttr );
}

inline void XRenderPeer::SetPictureClipRegion( Picture aPicture,
    Region aXlibRegion ) const
{
    XRenderSetPictureClipRegion( mpDisplay, aPicture, aXlibRegion );
}

inline void XRenderPeer::CompositePicture( int nXRenderOp,
    Picture aSrcPic, Picture aMaskPic, Picture aDstPic,
    int nSrcX, int nSrcY, int nMaskX, int nMaskY, int nDstX, int nDstY,
    unsigned nWidth, unsigned nHeight ) const
{
    XRenderComposite( mpDisplay, nXRenderOp, aSrcPic, aMaskPic, aDstPic,
                      nSrcX, nSrcY, nMaskX, nMaskY, nDstX, nDstY, nWidth, nHeight );
}

inline void XRenderPeer::FreePicture( Picture aPicture ) const
{
    XRenderFreePicture( mpDisplay, aPicture );
}

inline void XRenderPeer::FillRectangle( int a, Picture b, const XRenderColor* c,
    int d, int e, unsigned int f, unsigned int g) const
{
    XRenderFillRectangle( mpDisplay, a, b, c, d, e, f, g );
}

inline void XRenderPeer::CompositeTrapezoids( int nOp,
    Picture aSrc, Picture aDst, const XRenderPictFormat* pXRPF,
    int nXSrc, int nYSrc, const XTrapezoid* pXT, int nCount ) const
{
    XRenderCompositeTrapezoids( mpDisplay, nOp, aSrc, aDst, pXRPF,
        nXSrc, nYSrc, pXT, nCount );
}

inline XRenderColor GetXRenderColor( const SalColor& rSalColor, double fTransparency = 0.0 )
{
    XRenderColor aRetVal;
    // convert the SalColor
    aRetVal.red   = SALCOLOR_RED(   rSalColor ); aRetVal.red   |= (aRetVal.red   << 8);
    aRetVal.green = SALCOLOR_GREEN( rSalColor ); aRetVal.green |= (aRetVal.green << 8);
    aRetVal.blue  = SALCOLOR_BLUE(  rSalColor ); aRetVal.blue  |= (aRetVal.blue  << 8);

    // handle transparency
    aRetVal.alpha = 0xFFFF; // default to opaque
    if( fTransparency != 0 )
    {
        const double fAlpha = 1.0 - fTransparency;
        aRetVal.alpha = static_cast<sal_uInt16>(fAlpha * 0xFFFF + 0.5);
        // xrender wants pre-multiplied colors
        aRetVal.red   = static_cast<sal_uInt16>(fAlpha * aRetVal.red + 0.5);
        aRetVal.green = static_cast<sal_uInt16>(fAlpha * aRetVal.green + 0.5);
        aRetVal.blue  = static_cast<sal_uInt16>(fAlpha * aRetVal.blue + 0.5);
    }

    return aRetVal;
}

#endif // INCLUDED_VCL_UNX_GENERIC_GDI_XRENDER_PEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
