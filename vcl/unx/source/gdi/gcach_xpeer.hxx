/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gcach_xpeer.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:05:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_GCACH_XPEER_HXX
#define _SV_GCACH_XPEER_HXX

#include <glyphcache.hxx>

#include <prex.h>
#include <X11/extensions/Xrender.h>
#include <postx.h>

class SalDisplay;
struct MultiScreenGlyph;

class X11GlyphPeer
: public GlyphCachePeer
{
public:
                        X11GlyphPeer();
    virtual            ~X11GlyphPeer();

    void                SetDisplay( const SalDisplay& );

    Pixmap              GetPixmap( ServerFont&, int nGlyphIndex, int nScreen );
    const RawBitmap*    GetRawBitmap( ServerFont&, int nGlyphIndex );
    bool                ForcedAntialiasing( const ServerFont&, int nScreen ) const;

    GlyphSet            GetGlyphSet( ServerFont&, int nScreen );
    Glyph               GetGlyphId( ServerFont&, int nGlyphIndex );

protected:
    virtual void        RemovingFont( ServerFont& );
    virtual void        RemovingGlyph( ServerFont&, GlyphData&, int nGlyphIndex );

    MultiScreenGlyph*   PrepareForMultiscreen( ExtGlyphData& ) const;
    void                SetRenderGlyph( GlyphData&, Glyph ) const;
    void                SetRawBitmap( GlyphData&, const RawBitmap* ) const;
    void                SetPixmap( GlyphData&, Pixmap, int nScreen ) const;
    Glyph               GetRenderGlyph( const GlyphData& ) const;
    const RawBitmap*    GetRawBitmap( const GlyphData& ) const;
    Pixmap              GetPixmap( const GlyphData&, int nScreen ) const;

private:
    Display*            mpDisplay;

    // thirty-two screens should be enough for everyone...
    static const int MAX_GCACH_SCREENS = 32;
    int                 mnMaxScreens;
    int                 mnDefaultScreen;
    int                 mnExtByteCount;
    sal_uInt32          mnForcedAA;
    sal_uInt32          mnUsingXRender;

    int                 mnRenderVersion;
    XRenderPictFormat*  mpGlyphFormat;
    RawBitmap           maRawBitmap;

#ifndef XRENDER_LINK
public:
    XRenderPictFormat*  (*pXRenderFindFormat)(Display*,unsigned long,XRenderPictFormat*,int);
    XRenderPictFormat*  (*pXRenderFindVisualFormat)(Display*,Visual*);
    Bool        (*pXRenderQueryExtension)(Display*,int*,int*);
    void        (*pXRenderQueryVersion)(Display*,int*,int*);
    GlyphSet    (*pXRenderCreateGlyphSet)(Display*,XRenderPictFormat*);
    void        (*pXRenderFreeGlyphSet)(Display*,GlyphSet);
    void        (*pXRenderAddGlyphs)(Display*,GlyphSet,Glyph*,XGlyphInfo*,int,char*,int);
    void        (*pXRenderFreeGlyphs)(Display*,GlyphSet,Glyph*,int);
    void        (*pXRenderCompositeString32)(Display*,int,Picture,Picture,XRenderPictFormat*,GlyphSet,int,int,int,int,unsigned*,int);
    Picture     (*pXRenderCreatePicture)(Display*,Drawable,XRenderPictFormat*,unsigned long,XRenderPictureAttributes*);
    void        (*pXRenderSetPictureClipRegion)(Display*,Picture,XLIB_Region);
    void        (*pXRenderFreePicture)(Display*,Picture);
    void        (*pXRenderFillRectangle)(Display*,int,Picture,_Xconst XRenderColor*,int,int,unsigned int,unsigned int);
#endif
};

#endif // _SV_GCACH_XPEER_HXX
