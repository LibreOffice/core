/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gcach_xpeer.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:24:59 $
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

#ifndef _VCL_DLLAPI_H
#include "dllapi.h"
#endif


class SalDisplay;
struct MultiScreenGlyph;

class X11GlyphPeer
: public GlyphCachePeer
{
public:
                        X11GlyphPeer();
    virtual            ~X11GlyphPeer();

    Pixmap              GetPixmap( ServerFont&, int nGlyphIndex, int nScreen );
    const RawBitmap*    GetRawBitmap( ServerFont&, int nGlyphIndex );
    bool                ForcedAntialiasing( const ServerFont&, int nScreen ) const;

    GlyphSet            GetGlyphSet( ServerFont&, int nScreen );
    Glyph               GetGlyphId( ServerFont&, int nGlyphIndex );

protected:
    void                InitAntialiasing();

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
    RawBitmap           maRawBitmap;
    sal_uInt32          mnForcedAA;
    sal_uInt32          mnUsingXRender;
};

class X11GlyphCache : public GlyphCache
{
public:
    X11GlyphPeer&      GetPeer() { return reinterpret_cast<X11GlyphPeer&>( mrPeer ); }
static X11GlyphCache&  GetInstance();
static void            KillInstance();

private:
                       X11GlyphCache( X11GlyphPeer& );
};

#endif // _SV_GCACH_XPEER_HXX
