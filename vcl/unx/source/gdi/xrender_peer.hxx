/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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

#include <prex.h>
#include <X11/extensions/Xrender.h>
#include <postx.h>

#include <osl/module.h>

class XRenderPeer
{
public:
    static XRenderPeer& GetInstance();
    int                 GetVersion() const;

    sal_uInt32          InitRenderText( int nMaxDepth );

protected:
                        XRenderPeer();
                        ~XRenderPeer();
    void                InitRenderLib();

    Display*            mpDisplay;
    XRenderPictFormat*  mpGlyphFormat;
    int                 mnRenderVersion;
    oslModule           mpRenderLib;

public:
    XRenderPictFormat* FindVisualFormat( Visual* ) const;
    XRenderPictFormat* FindPictureFormat( unsigned long nMask,
        const XRenderPictFormat& ) const;
    Picture     CreatePicture( Drawable, XRenderPictFormat*,
                    unsigned long, const XRenderPictureAttributes& ) const;
    void        SetPictureClipRegion( Picture, XLIB_Region ) const;
    void        CompositePicture( int,Picture,Picture,Picture,
                    int,int,int,int,int,int,unsigned int,unsigned int) const;
    void        FreePicture( Picture ) const;

    GlyphSet    CreateGlyphSet() const;
    void        FreeGlyphSet( GlyphSet ) const;
    void        AddGlyph( GlyphSet, Glyph nGlyphId, const XGlyphInfo&,
                    const char* pBuffer, int nBufSize ) const;
    void        FreeGlyph( GlyphSet, Glyph nGlyphId ) const;
    void        CompositeString32( Picture aSrc, Picture aDst, GlyphSet,
                    int nDstX, int nDstY, const unsigned* pText, int nTextLen ) const;
    void        FillRectangle( int, Picture, const XRenderColor*,
                               int, int, unsigned int, unsigned int);

private:
    XRenderPictFormat* (*mpXRenderFindFormat)(Display*,unsigned long,
        const XRenderPictFormat*,int);
    XRenderPictFormat* (*mpXRenderFindVisualFormat)(Display*,Visual*);
    Bool        (*mpXRenderQueryExtension)(Display*,int*,int*);
    void        (*mpXRenderQueryVersion)(Display*,int*,int*);

    Picture     (*mpXRenderCreatePicture)(Display*,Drawable,XRenderPictFormat*,
                    unsigned long,const XRenderPictureAttributes*);
    void        (*mpXRenderSetPictureClipRegion)(Display*,Picture,XLIB_Region);
    void        (*mpXRenderFreePicture)(Display*,Picture);
    void        (*mpXRenderComposite)(Display*,int,Picture,Picture,Picture,
                    int,int,int,int,int,int,unsigned,unsigned);

    GlyphSet    (*mpXRenderCreateGlyphSet)(Display*,XRenderPictFormat*);
    void        (*mpXRenderFreeGlyphSet)(Display*,GlyphSet);
    void        (*mpXRenderAddGlyphs)(Display*,GlyphSet,Glyph*,
                    const XGlyphInfo*,int,const char*,int);
    void        (*mpXRenderFreeGlyphs)(Display*,GlyphSet,Glyph*,int);
    void        (*mpXRenderCompositeString32)(Display*,int,Picture,Picture,
                    XRenderPictFormat*,GlyphSet,int,int,int,int,const unsigned*,int);
    void        (*mpXRenderFillRectangle)(Display*,int,Picture,
                    const XRenderColor*,int,int,unsigned int,unsigned int);
};

//=====================================================================

class ScopedPic
{
public:
               ScopedPic( XRenderPeer& rPeer, Picture& rPic );
               ~ScopedPic();
    Picture&   Get();

private:
    XRenderPeer& mrRenderPeer;
    Picture      maPicture;

private: // prevent copy and assignmet
           ScopedPic( const ScopedPic& );
    void   operator=( const ScopedPic& );
};

//=====================================================================

inline int XRenderPeer::GetVersion() const
{
    return mnRenderVersion;
}

inline XRenderPictFormat* XRenderPeer::FindVisualFormat( Visual* pVisual ) const
{
#ifdef XRENDER_LINK
    return XRenderFindVisualFormat ( mpDisplay, pVisual );
#else
    return (*mpXRenderFindVisualFormat)( mpDisplay, pVisual );
#endif
}

inline XRenderPictFormat* XRenderPeer::FindPictureFormat( unsigned long nFormatMask,
    const XRenderPictFormat& rFormatAttr ) const
{
#ifdef XRENDER_LINK
    return XRenderFindFormat( mpDisplay, nFormatMask, &rFormatAttr, 0 );
#else
    return (*mpXRenderFindFormat)( mpDisplay, nFormatMask, &rFormatAttr, 0 );
#endif
}

inline Picture XRenderPeer::CreatePicture( Drawable aDrawable,
    XRenderPictFormat* pVisFormat, unsigned long nValueMask,
    const XRenderPictureAttributes& rRenderAttr ) const
{
#ifdef XRENDER_LINK
    return XRenderCreatePicture( mpDisplay, aDrawable, pVisFormat,
                                 nValueMask, &rRenderAttr );
#else
    return (*mpXRenderCreatePicture)( mpDisplay, aDrawable, pVisFormat,
        nValueMask, &rRenderAttr );
#endif
}

inline void XRenderPeer::SetPictureClipRegion( Picture aPicture,
    XLIB_Region aXlibRegion ) const
{
#ifdef XRENDER_LINK
    XRenderSetPictureClipRegion( mpDisplay, aPicture, aXlibRegion );
#else
    (*mpXRenderSetPictureClipRegion)( mpDisplay, aPicture, aXlibRegion );
#endif
}

inline void XRenderPeer::CompositePicture( int nXRenderOp,
    Picture aSrcPic, Picture aMaskPic, Picture aDstPic,
    int nSrcX, int nSrcY, int nMaskX, int nMaskY, int nDstX, int nDstY,
    unsigned nWidth, unsigned nHeight ) const
{
#ifdef XRENDER_LINK
    XRenderComposite( mpDisplay, nXRenderOp, aSrcPic, aMaskPic, aDstPic,
                      nSrcX, nSrcY, nMaskX, nMaskY, nDstX, nDstY, nWidth, nHeight );
#else
    (*mpXRenderComposite)( mpDisplay, nXRenderOp, aSrcPic, aMaskPic, aDstPic,
        nSrcX, nSrcY, nMaskX, nMaskY, nDstX, nDstY, nWidth, nHeight );
#endif
}

inline void XRenderPeer::FreePicture( Picture aPicture ) const
{
#ifdef XRENDER_LINK
    XRenderFreePicture( mpDisplay, aPicture );
#else
    (*mpXRenderFreePicture)( mpDisplay, aPicture );
#endif
}

inline GlyphSet XRenderPeer::CreateGlyphSet() const
{
#ifdef XRENDER_LINK
    return XRenderCreateGlyphSet( mpDisplay, mpGlyphFormat );
#else
    return (*mpXRenderCreateGlyphSet)( mpDisplay, mpGlyphFormat );
#endif
}

inline void XRenderPeer::FreeGlyphSet( GlyphSet aGS ) const
{
#ifdef XRENDER_LINK
    XRenderFreeGlyphSet( mpDisplay, aGS );
#else
    (*mpXRenderFreeGlyphSet)( mpDisplay, aGS );
#endif
}

inline void XRenderPeer::AddGlyph( GlyphSet aGS, Glyph nGlyphId,
    const XGlyphInfo& rGI, const char* pBuffer, int nBufSize ) const
{
#ifdef XRENDER_LINK
    XRenderAddGlyphs( mpDisplay, aGS, &nGlyphId, &rGI, 1,
                      const_cast<char*>(pBuffer), nBufSize );
#else
    (*mpXRenderAddGlyphs)( mpDisplay, aGS, &nGlyphId, &rGI, 1,
        const_cast<char*>(pBuffer), nBufSize );
#endif
}

inline void XRenderPeer::FreeGlyph( GlyphSet aGS, Glyph nGlyphId ) const
{
    (void)aGS; (void)nGlyphId;

    // XRenderFreeGlyphs not implemented yet for version<=0.2
    // #108209# disabled because of crash potential,
    // the glyph leak is not too bad because they will
    // be cleaned up when the glyphset is released
#if 0 // TODO: reenable when it works without problems
    if( mnRenderVersion >= 0x05 )
    {
#ifdef XRENDER_LINK
        XRenderFreeGlyphs( mpDisplay, aGS, &nGlyphId, 1 );
#else
        (*mpXRenderFreeGlyphs)( mpDisplay, aGS, &nGlyphId, 1 );
#endif
    }
#endif
}

inline void XRenderPeer::CompositeString32( Picture aSrc, Picture aDst,
    GlyphSet aGlyphSet, int nDstX, int nDstY,
    const unsigned* pText, int nTextLen ) const
{
#ifdef XRENDER_LINK
    XRenderCompositeString32( mpDisplay, PictOpOver, aSrc, aDst, NULL,
                              aGlyphSet, 0, 0, nDstX, nDstY, pText, nTextLen );
#else
    (*mpXRenderCompositeString32)( mpDisplay, PictOpOver, aSrc, aDst, NULL,
        aGlyphSet, 0, 0, nDstX, nDstY, pText, nTextLen );
#endif
}

inline void XRenderPeer::FillRectangle( int a, Picture b, const XRenderColor* c,
                                        int d, int e, unsigned int f, unsigned int g)
{
#ifdef XRENDER_LINK
    XRenderFillRectangle( mpDisplay, a, b, c, d, e, f, g );
#else
    (*mpXRenderFillRectangle)( mpDisplay, a, b, c, d, e, f, g );
#endif
}

//=====================================================================

inline ScopedPic::ScopedPic( XRenderPeer& rPeer, Picture& rPic )
:   mrRenderPeer( rPeer)
,   maPicture( rPic )
{}

inline ScopedPic::~ScopedPic()
{
    if( maPicture )
        mrRenderPeer.FreePicture( maPicture );
}

inline Picture& ScopedPic::Get()
{
    return maPicture;
}

//=====================================================================

#endif // _SV_XRENDER_PEER_HXX
