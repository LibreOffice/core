/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gcach_xpeer.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:05:15 $
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

class X11GlyphPeer
: public GlyphCachePeer
{
public:
                        X11GlyphPeer();

    void                SetDisplay( Display*, Visual* );

    Pixmap              GetPixmap( ServerFont&, int nGlyphIndex );
    const RawBitmap*    GetRawBitmap( ServerFont&, int nGlyphIndex );
    bool                ForcedAntialiasing( const ServerFont& ) const;

    GlyphSet            GetGlyphSet( ServerFont& );
    Glyph               GetGlyphId( ServerFont&, int nGlyphIndex );

protected:
    virtual void        RemovingFont( ServerFont& );
    virtual void        RemovingGlyph( ServerFont&, GlyphData&, int nGlyphIndex );

private:
    enum { EMPTY_KIND=0, PIXMAP_KIND, XRENDER_KIND, AAFORCED_KIND };

    Display*            mpDisplay;
    RawBitmap           maRawBitmap;
    bool                mbForcedAA;

    bool                mbUsingXRender;
    XRenderPictFormat*  mpGlyphFormat;

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
#endif
};

#endif // _SV_GCACH_XPEER_HXX
