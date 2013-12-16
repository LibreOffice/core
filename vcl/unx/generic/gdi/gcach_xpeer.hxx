/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_GCACH_XPEER_HXX
#define _SV_GCACH_XPEER_HXX

#include <tools/prex.h>
#include <X11/extensions/Xrender.h>
#include <tools/postx.h>

#include <glyphcache.hxx>

class SalDisplay;
struct MultiScreenGlyph;
typedef Glyph XRenderGlyph;

class X11GlyphPeer
: public GlyphCachePeer
{
public:
                        X11GlyphPeer();
    virtual            ~X11GlyphPeer();

    Pixmap              GetPixmap( ServerFont&, sal_GlyphId, int nScreen );
    const RawBitmap*    GetRawBitmap( ServerFont&, sal_GlyphId );
    bool                ForcedAntialiasing( const ServerFont&, int nScreen ) const;

    GlyphSet            GetGlyphSet( ServerFont&, int nScreen );
    XRenderGlyph        GetXRGlyph( ServerFont&, sal_GlyphId );

protected:
    void                InitAntialiasing();

    virtual void        RemovingFont( ServerFont& );
    virtual void        RemovingGlyph( ServerFont&, GlyphData&, sal_GlyphId );

    MultiScreenGlyph*   PrepareForMultiscreen( ExtGlyphData& ) const;
    void                SetRenderGlyph( GlyphData&, Glyph ) const;
    void                SetRawBitmap( GlyphData&, const RawBitmap* ) const;
    void                SetPixmap( GlyphData&, Pixmap, int nScreen ) const;
    XRenderGlyph        GetRenderGlyph( const GlyphData& ) const;
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
