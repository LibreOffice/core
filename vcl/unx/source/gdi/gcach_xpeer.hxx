/*************************************************************************
 *
 *  $RCSfile: gcach_xpeer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hdu $ $Date: 2001-04-05 07:38:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_GCACH_XPEER_HXX
#define _SV_GCACH_XPEER_HXX

#include <glyphcache.hxx>

#ifdef USE_XRENDER
    #define Region XLIB_Region
    #include <X11/extensions/Xrender.h>
    #undef Region
#endif

class X11GlyphPeer
: public GlyphCachePeer
{
public:
                        X11GlyphPeer();

    void                SetDisplay( Display*, Visual* );

    Pixmap              GetPixmap( ServerFont&, int nGlyphIndex );
    const RawBitmap*    GetRawBitmap( ServerFont&, int nGlyphIndex );
    bool                ForcedAntialiasing( const ServerFont& ) const;

#ifdef USE_XRENDER
    GlyphSet            GetGlyphSet( ServerFont& );
    Glyph               GetGlyphId( ServerFont&, int nGlyphIndex );
#endif // USE_XRENDER

protected:
    virtual void        RemovingFont( ServerFont& );
    virtual void        RemovingGlyph( ServerFont&, GlyphData&, int nGlyphIndex );

private:
    enum { EMPTY_KIND=0, PIXMAP_KIND, XRENDER_KIND, AAFORCED_KIND };

    Display*            mpDisplay;
    RawBitmap           maRawBitmap;
    bool                mbForcedAA;

#ifdef USE_XRENDER
    bool                mbUsingXRender;
    XRenderPictFormat*  mpGlyphFormat;

public:
    XRenderPictFormat*  (*pXRenderFindFormat)(Display*,unsigned long,XRenderPictFormat*,int);
    XRenderPictFormat*  (*pXRenderFindVisualFormat)(Display*,Visual*);
    Bool        (*pXRenderQueryExtension)(Display*,int*,int*);
    void        (*pXRenderQueryVersion)(Display*,int*,int*);
    GlyphSet    (*pXRenderCreateGlyphSet)(Display*,XRenderPictFormat*);
    void        (*pXRenderFreeGlyphSet)(Display*,GlyphSet);
    void        (*pXRenderAddGlyphs)(Display*,GlyphSet,Glyph*,XGlyphInfo*,int,char*,int);
    void        (*pXRenderCompositeString16)(Display*,int,Picture,Picture,XRenderPictFormat*,GlyphSet,int,int,int,int,unsigned short*,int);
    Picture     (*pXRenderCreatePicture)(Display*,Drawable,XRenderPictFormat*,unsigned long,XRenderPictureAttributes*);
    void        (*pXRenderSetPictureClipRegion)(Display*,Picture,XLIB_Region);
    void        (*pXRenderFreePicture)(Display*,Picture);
#endif // USE_XRENDER
};

#endif // _SV_GCACH_XPEER_HXX
