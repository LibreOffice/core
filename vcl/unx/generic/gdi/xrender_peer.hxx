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

#ifndef _SV_XRENDER_PEER_HXX
#define _SV_XRENDER_PEER_HXX

#include <tools/prex.h>
struct _XTrap; // on some older systems this is not declared within Xrender.h
#include <X11/extensions/Xrender.h>
#include <tools/postx.h>

#include <vcl/salgtype.hxx>
#include <osl/module.h>

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
    void        SetPictureClipRegion( Picture, XLIB_Region ) const;
    void        CompositePicture( int nOp, Picture aSrc, Picture aMask, Picture aDst,
                    int nXSrc, int nYSrc, int nXMask, int nYMask,
                    int nXDst, int nYDst, unsigned nWidth, unsigned nHeight ) const;
    void        FreePicture( Picture ) const;

    GlyphSet    CreateGlyphSet() const;
    void        FreeGlyphSet( GlyphSet ) const;
    void        AddGlyph( GlyphSet, Glyph nGlyphId, const XGlyphInfo&,
                    const char* pBuffer, int nBufSize ) const;
    void        FreeGlyph( GlyphSet, Glyph nGlyphId ) const;
    void        CompositeString32( Picture aSrc, Picture aDst, GlyphSet,
                    int nDstX, int nDstY, const unsigned* pText, int nTextLen ) const;
    void        FillRectangle( int nOp, Picture aDst, const XRenderColor*,
                               int nX, int nY, unsigned nW, unsigned nH ) const;
    void        CompositeTrapezoids( int nOp, Picture aSrc, Picture aDst,
                    const XRenderPictFormat*, int nXSrc, int nYSrc,
                    const XTrapezoid*, int nCount ) const;
    void        AddTraps( Picture aDst, int nXOfs, int nYOfs,
                    const _XTrap*, int nCount ) const;
};

//=====================================================================

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
    XLIB_Region aXlibRegion ) const
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

inline GlyphSet XRenderPeer::CreateGlyphSet() const
{
    return XRenderCreateGlyphSet( mpDisplay, mpStandardFormatA8 );
}

inline void XRenderPeer::FreeGlyphSet( GlyphSet aGS ) const
{
    XRenderFreeGlyphSet( mpDisplay, aGS );
}

inline void XRenderPeer::AddGlyph( GlyphSet aGS, Glyph nGlyphId,
    const XGlyphInfo& rGI, const char* pBuffer, int nBufSize ) const
{
    XRenderAddGlyphs( mpDisplay, aGS, &nGlyphId, &rGI, 1,
                      const_cast<char*>(pBuffer), nBufSize );
}

inline void XRenderPeer::FreeGlyph( GlyphSet aGS, Glyph nGlyphId ) const
{
    (void)aGS; (void)nGlyphId;

    // XRenderFreeGlyphs not implemented yet for version<=0.2
    // #108209# disabled because of crash potential,
    // the glyph leak is not too bad because they will
    // be cleaned up when the glyphset is released
}

inline void XRenderPeer::CompositeString32( Picture aSrc, Picture aDst,
    GlyphSet aGlyphSet, int nDstX, int nDstY,
    const unsigned* pText, int nTextLen ) const
{
    XRenderCompositeString32( mpDisplay, PictOpOver, aSrc, aDst, NULL,
                              aGlyphSet, 0, 0, nDstX, nDstY, pText, nTextLen );
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

inline void XRenderPeer::AddTraps( Picture aDst, int nXOfs, int nYOfs,
    const _XTrap* pTraps, int nCount ) const
{
    XRenderAddTraps( mpDisplay, aDst, nXOfs, nYOfs, pTraps, nCount );
}

//=====================================================================

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

//=====================================================================

#endif // _SV_XRENDER_PEER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
