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

#ifndef _SV_GCACH_XPEER_HXX
#define _SV_GCACH_XPEER_HXX

#include <tools/prex.h>
#include <X11/extensions/Xrender.h>
#include <tools/postx.h>

#include <glyphcache.hxx>

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
