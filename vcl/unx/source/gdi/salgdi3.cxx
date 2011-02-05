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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gcach_xpeer.hxx"
#include "xrender_peer.hxx"
#include "salunx.h"
#include "saldata.hxx"
#include "saldisp.hxx"
#include "salgdi.h"
#include "pspgraphics.h"
#include "salvd.h"
#include "xfont.hxx"
#include <vcl/sysdata.hxx>
#include "xlfd_attr.hxx"
#include "xlfd_smpl.hxx"
#include "xlfd_extd.hxx"
#include "salcvt.hxx"

#include "vcl/printergfx.hxx"
#include "vcl/fontmanager.hxx"
#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"
#include "vcl/svapp.hxx"
#include "vcl/impfont.hxx"
#include "vcl/salframe.hxx"
#include "vcl/outdev.h"

#include "sal/alloca.h"
#include "sal/types.h"

#include "rtl/tencinfo.h"

#include "osl/file.hxx"

#include "tools/string.hxx"
#include "tools/debug.hxx"
#include "tools/stream.hxx"

#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "i18npool/mslangid.hxx"

#include <boost/unordered_set.hpp>

#ifdef ENABLE_GRAPHITE
#include <vcl/graphite_layout.hxx>
#include <vcl/graphite_serverfont.hxx>
#endif

struct cairo_surface_t;
struct cairo_t;
struct cairo_font_face_t;
typedef void* FT_Face;
struct cairo_matrix_t {
    double xx; double yx;
    double xy; double yy;
    double x0; double y0;
};
struct cairo_glyph_t
{
    unsigned long index;
    double x;
    double y;
};
struct BOX
{
    short x1, x2, y1, y2;
};
struct _XRegion
{
    long size;
    long numRects;
    BOX *rects;
    BOX extents;
};
using namespace rtl;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// ----------------------------------------------------------------------------
//
// manage X11 fonts and self rastered fonts
//
// ----------------------------------------------------------------------------

#ifndef _USE_PRINT_EXTENSION_

class FontLookup
{
    public:

        struct hash;
        struct equal;
        typedef ::boost::unordered_set< FontLookup,
                                 FontLookup::hash,
                                 FontLookup::equal > fl_hashset;

    private:

        rtl::OString        maName;
        FontWeight          mnWeight;
        FontItalic          mnItalic;
        sal_Bool            mbDisplay;

    public:

                            FontLookup ( ::std::list< psp::fontID >::iterator& it,
                                          const psp::PrintFontManager& rMgr );
                            FontLookup (const Xlfd& rFont);
                            FontLookup (const FontLookup &rRef) :
                                    maName   (rRef.maName),
                                    mnWeight (rRef.mnWeight),
                                    mnItalic (rRef.mnItalic),
                                    mbDisplay(rRef.mbDisplay)
                            {}
                            ~FontLookup ()
                            {}

        static void         BuildSet (fl_hashset& rSet);
        static bool         InSet (const fl_hashset& rSet, const Xlfd& rXfld);
        bool                InSet (const fl_hashset& rSet) const;

        bool                operator== (const FontLookup &rRef) const
                            {
                                return     (abs(mnWeight - rRef.mnWeight) < 2)
                                        && (mnItalic == rRef.mnItalic)
                                        && (maName   == rRef.maName)
                                        && (mbDisplay== rRef.mbDisplay);
                            }
        FontLookup&         operator= (const FontLookup &rRef)
                            {
                                mnWeight = rRef.mnWeight;
                                mnItalic = rRef.mnItalic;
                                maName   = rRef.maName;
                                mbDisplay= rRef.mbDisplay;

                                return *this;
                            }
        size_t              Hash() const
                            {
                                return maName.hashCode ();
                            }

        struct equal
        {
            bool operator()(const FontLookup &r1, const FontLookup &r2) const
            {
                return r1 == r2;
            }
        };
        struct hash
        {
            size_t operator()(const FontLookup &rArg) const
            {
                return rArg.Hash();
            }
        };
};

FontLookup::FontLookup ( ::std::list< psp::fontID >::iterator& it,
                         const psp::PrintFontManager& rMgr )
{
    psp::FastPrintFontInfo aInfo;
    if (rMgr.getFontFastInfo (*it, aInfo))
    {
        mnItalic = PspGraphics::ToFontItalic (aInfo.m_eItalic);
        mnWeight = PspGraphics::ToFontWeight (aInfo.m_eWeight);
        mbDisplay=    aInfo.m_eType == psp::fonttype::Builtin
                   || aInfo.m_eType == psp::fonttype::Unknown ? False : True;
        maName   = rtl::OUStringToOString
            ( aInfo.m_aFamilyName,
              RTL_TEXTENCODING_ISO_8859_1).toAsciiLowerCase();

        sal_Int32       n_length = maName.getLength();
        const sal_Char* p_from   = maName.getStr();
        sal_Char*       p_to     = (sal_Char*)alloca (n_length + 1);

        sal_Int32 i, j;
        for (i = 0, j = 0; i < n_length; i++)
        {
            if ( p_from[i] != ' ' )
                p_to[j++] = p_from[i];
        }
        maName = rtl::OString (p_to, j);
        if (mnItalic == ITALIC_OBLIQUE)
            mnItalic = ITALIC_NORMAL;
    }
    else
    {
        mnItalic = ITALIC_DONTKNOW;
        mnWeight = WEIGHT_DONTKNOW;
        mbDisplay= False;
    }
}

FontLookup::FontLookup (const Xlfd& rFont)
{
    AttributeProvider* pFactory = rFont.mpFactory;
    Attribute*         pAttr;

    pAttr    = pFactory->RetrieveSlant  (rFont.mnSlant);
    mnItalic = (FontItalic)pAttr->GetValue();
    pAttr    = pFactory->RetrieveWeight (rFont.mnWeight);
    mnWeight = (FontWeight)pAttr->GetValue();
    pAttr    = pFactory->RetrieveFamily (rFont.mnFamily);
    maName   = pAttr->GetKey();

    if (mnItalic == ITALIC_OBLIQUE)
        mnItalic = ITALIC_NORMAL;

    mbDisplay = True;
}

void
FontLookup::BuildSet (FontLookup::fl_hashset &rSet)
{
    ::std::list< psp::fontID > aIdList;

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aIdList, NULL, false );

    ::std::list< psp::fontID >::iterator it;
    for (it = aIdList.begin(); it != aIdList.end(); ++it)
    {
        FontLookup aItem (it, rMgr);
        rSet.insert (aItem);
    }
}

bool
FontLookup::InSet (const FontLookup::fl_hashset& rSet) const
{
      fl_hashset::const_iterator it = rSet.find(*this);
      return it == rSet.end() ? false : true;
}

bool
FontLookup::InSet (const FontLookup::fl_hashset& rSet, const Xlfd& rXlfd)
{
    FontLookup aNeedle (rXlfd);
    return aNeedle.InSet (rSet);
}

#endif

// ----------------------------------------------------------------------------
//
// SalDisplay
//
// ----------------------------------------------------------------------------

XlfdStorage*
SalDisplay::GetXlfdList() const
{
    if ( mpFontList != NULL )
    {
        return mpFontList;
    }
    else
    {
        const_cast<SalDisplay*>(this)->mpFactory  = new AttributeProvider;
        const_cast<SalDisplay*>(this)->mpFontList = new XlfdStorage;
        const_cast<SalDisplay*>(this)->mpFallbackFactory = new VirtualXlfd;

        int i, nFontCount;
        const int nMaxCount  = 64 * 1024 - 1;
        Display *pDisplay = GetDisplay();
        char **ppFontList = XListFonts(pDisplay, "-*", nMaxCount, &nFontCount);

        //
        // create a list of simple Xlfd font information
        //

        Xlfd  *pXlfdList = (Xlfd*)malloc( nFontCount * sizeof(Xlfd) );
        int    nXlfdCount = 0;

        for ( i = 0; i < nFontCount; i++ )
        {
            if ( pXlfdList[ nXlfdCount ].FromString(ppFontList[i], mpFactory) )
                ++nXlfdCount;
        }

        XFreeFontNames( ppFontList );

        mpFactory->AddClassification();
        // add some pretty print description
        mpFactory->AddAnnotation();
        // misc feature checking
        mpFactory->TagFeature();

        // sort according to font style
        qsort( pXlfdList, nXlfdCount, sizeof(Xlfd), XlfdCompare );

#ifndef _USE_PRINT_EXTENSION_
        // create a list of fonts already managed by the fontmanager
        FontLookup::fl_hashset aSet;
        FontLookup::BuildSet (aSet);
#endif

        //
        // create a font list with merged encoding information
        //

        BitmapXlfdStorage   aBitmapList;
        ScalableXlfd       *pScalableFont = NULL;

        int nFrom = 0;
        for ( i = 0; i < nXlfdCount; i++ )
        {
            // exclude openlook glyph and cursor
            Attribute *pAttr = mpFactory->RetrieveFamily(pXlfdList[i].mnFamily);
            if ( pAttr->HasFeature(   XLFD_FEATURE_OL_GLYPH
                                    | XLFD_FEATURE_OL_CURSOR) )
                continue;
            // exclude fonts with unknown encoding
            if ( pXlfdList[i].GetEncoding() == RTL_TEXTENCODING_DONTKNOW )
                continue;
            // exclude "interface system" and "interface user"
            if (pAttr->HasFeature( XLFD_FEATURE_APPLICATION_FONT ) )
                continue;
            // exclude fonts already managed by fontmanager, anyway keep
            // gui fonts: they are candidates for GetInterfaceFont ()
            if (pXlfdList[i].Fonttype() == eTypeScalable)
                ((VirtualXlfd*)mpFallbackFactory)->FilterInterfaceFont (pXlfdList + i);
#ifndef _USE_PRINT_EXTENSION_
            if (FontLookup::InSet (aSet, pXlfdList[i]))
                 continue;
#endif
            Bool bSameOutline = pXlfdList[i].SameFontoutline(pXlfdList + nFrom);
            XlfdFonttype eType = pXlfdList[i].Fonttype();

            // flush the old merged font list if the name doesn't match any more
            if ( !bSameOutline )
            {
                mpFontList->Add( pScalableFont );
                mpFontList->Add( &aBitmapList );
                pScalableFont = NULL;
                aBitmapList.Reset();
            }

            // merge the font or generate a new one
            switch( eType )
            {
                case eTypeScalable:
                    if ( pScalableFont == NULL )
                        pScalableFont = new ScalableXlfd;
                    pScalableFont->AddEncoding(pXlfdList + i);
                    break;

                case eTypeBitmap:
                    aBitmapList.AddBitmapFont( pXlfdList + i );
                    break;

                case eTypeScalableBitmap:
                    // ignore scaled X11 bitmap fonts because they look too ugly
                default:
                    break;
            }

            nFrom = i;
        }

        // flush the merged list into the global list
        mpFontList->Add( pScalableFont );
        mpFontList->Add( &aBitmapList );
        if (mpFallbackFactory->NumEncodings() > 0)
            mpFontList->Add( mpFallbackFactory );
        // cleanup the list of simple font information
        if ( pXlfdList != NULL )
            free( pXlfdList );

        return mpFontList;
    }
}

// ---------------------------------------------------------------------------

ExtendedFontStruct*
SalDisplay::GetFont( const ExtendedXlfd *pRequestedFont,
    const Size& rPixelSize, sal_Bool bVertical ) const
{
    // TODO: either get rid of X11 fonts or get rid of the non-hashmapped cache
    if( !m_pFontCache )
    {
        m_pFontCache = new SalFontCache( 64, 64, 16 ); // ???
    }
    else
    {
        ExtendedFontStruct *pItem;
        for ( pItem  = m_pFontCache->First();
              pItem != NULL;
              pItem  = m_pFontCache->Next() )
        {
            if ( pItem->Match(pRequestedFont, rPixelSize, bVertical) )
            {
                if( m_pFontCache->GetCurPos() )
                {
                    m_pFontCache->Remove( pItem );
                    m_pFontCache->Insert( pItem, 0UL );
                }
                return pItem;
            }
        }
    }

    // before we expand the cache, we look for very old and unused items
    if( m_pFontCache->Count() >= 64 )
    {
        ExtendedFontStruct *pItem;
        for ( pItem = m_pFontCache->Last();
              pItem != NULL;
              pItem = m_pFontCache->Prev() )
        {
            if( 1 == pItem->GetRefCount() )
            {
                m_pFontCache->Remove( pItem );
                pItem->ReleaseRef();
                if( m_pFontCache->Count() < 64 )
                    break;
            }
        }
    }

    ExtendedFontStruct *pItem = new ExtendedFontStruct( GetDisplay(),
                                        rPixelSize, bVertical,
                                        const_cast<ExtendedXlfd*>(pRequestedFont) );
    m_pFontCache->Insert( pItem, 0UL );
    pItem->AddRef();

    return pItem;
}

// ---------------------------------------------------------------------------

void
SalDisplay::DestroyFontCache()
{
    if( m_pFontCache )
    {
        ExtendedFontStruct *pItem = m_pFontCache->First();
        while( pItem )
        {
            delete pItem;
            pItem = m_pFontCache->Next();
        }
        delete m_pFontCache;
    }
    if( mpFontList )
    {
        mpFontList->Dispose();
        delete mpFontList;
    }
    if ( mpFactory )
    {
        delete mpFactory;
    }

    m_pFontCache = (SalFontCache*)NULL;
    mpFontList = (XlfdStorage*)NULL;
    mpFactory  = (AttributeProvider*)NULL;
}

// ===========================================================================

// PspKernInfo allows on-demand-querying of psprint provided kerning info (#i29881#)
class PspKernInfo : public ExtraKernInfo
{
public:
    PspKernInfo( int nFontId ) : ExtraKernInfo(nFontId) {}
protected:
    virtual void Initialize() const;
};

//--------------------------------------------------------------------------

void PspKernInfo::Initialize() const
{
    mbInitialized = true;

    // get the kerning pairs from psprint
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    typedef std::list< psp::KernPair > PspKernPairs;
    const PspKernPairs& rKernPairs = rMgr.getKernPairs( mnFontId );
    if( rKernPairs.empty() )
        return;

    PspKernPairs::const_iterator it = rKernPairs.begin();
    for(; it != rKernPairs.end(); ++it )
    {
        ImplKernPairData aKernPair = { it->first, it->second, it->kern_x };
        maUnicodeKernPairs.insert( aKernPair );
    }
}

// ----------------------------------------------------------------------------
//
// X11SalGraphics
//
// ----------------------------------------------------------------------------

GC
X11SalGraphics::SelectFont()
{
    Display *pDisplay = GetXDisplay();

    if( !pFontGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;
        values.foreground           = nTextPixel_;
#ifdef _USE_PRINT_EXTENSION_
        values.background = xColormap_->GetWhitePixel();
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCBackground | GCForeground,
                              &values );
#else
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCForeground,
                              &values );
#endif
    }
    if( !bFontGC_ )
    {
        XSetForeground( pDisplay, pFontGC_, nTextPixel_ );
        SetClipRegion( pFontGC_ );
        bFontGC_ = TRUE;
    }

    return pFontGC_;
}

//--------------------------------------------------------------------------

bool X11SalGraphics::setFont( const ImplFontSelectData *pEntry, int nFallbackLevel )
{
#ifdef HDU_DEBUG
    ByteString aReqName( "NULL" );
    if( pEntry )
        aReqName = ByteString( pEntry->maName, RTL_TEXTENCODING_UTF8 );
    ByteString aUseName( "NULL" );
    if( pEntry && pEntry->mpFontData )
        aUseName = ByteString( pEntry->mpFontData->GetFamilyName(), RTL_TEXTENCODING_UTF8 );
    fprintf( stderr, "SetFont(lvl=%d,\"%s\", %d*%d, naa=%d,b=%d,i=%d) => \"%s\"\n",
        nFallbackLevel, aReqName.GetBuffer(),
    !pEntry?-1:pEntry->mnWidth, !pEntry?-1:pEntry->mnHeight,
        !pEntry?-1:pEntry->mbNonAntialiased,
    !pEntry?-1:pEntry->meWeight, !pEntry?-1:pEntry->meItalic,
        aUseName.GetBuffer() );
#endif

    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        mXFont[i] = NULL; // ->ReleaseRef()

        if( mpServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *mpServerFont[i] );
            mpServerFont[i] = NULL;
        }
    }

    // return early if there is no new font
    if( !pEntry )
    return false;

    bFontVertical_ = pEntry->mbVertical;

    // return early if this is not a valid font for this graphics
    if( !pEntry->mpFontData )
        return false;

    // handle the request for a native X11-font
    if( ImplX11FontData::CheckFontData( *pEntry->mpFontData ) )
    {
        const ImplX11FontData* pRequestedFont = static_cast<const ImplX11FontData*>( pEntry->mpFontData );
        const ExtendedXlfd& rX11Font = pRequestedFont->GetExtendedXlfd();

        Size aReqSize( pEntry->mnWidth, pEntry->mnHeight );
        mXFont[ nFallbackLevel ] = GetDisplay()->GetFont( &rX11Font, aReqSize, bFontVertical_ );
        bFontGC_ = FALSE;
        return true;
    }

    // handle the request for a non-native X11-font => use the GlyphCache
    ServerFont* pServerFont = GlyphCache::GetInstance().CacheFont( *pEntry );
    if( pServerFont != NULL )
    {
        // ignore fonts with e.g. corrupted font files
        if( !pServerFont->TestFont() )
        {
            GlyphCache::GetInstance().UncacheFont( *pServerFont );
            return false;
        }

        // register to use the font
        mpServerFont[ nFallbackLevel ] = pServerFont;

        // apply font specific-hint settings if needed
        // TODO: also disable it for reference devices
    if( !bPrinter_ )
    {
        ImplServerFontEntry* pSFE = static_cast<ImplServerFontEntry*>( pEntry->mpFontEntry );
        pSFE->HandleFontOptions();
        }

        return true;
    }

    return false;
}

void ImplServerFontEntry::HandleFontOptions( void )
{
    bool GetFCFontOptions( const ImplFontAttributes&, int nSize, ImplFontOptions& );

    if( !mpServerFont )
        return;
    if( !mbGotFontOptions )
    {
        // get and cache the font options
        mbGotFontOptions = true;
        mbValidFontOptions = GetFCFontOptions( *maFontSelData.mpFontData,
            maFontSelData.mnHeight, maFontOptions );
    }
    // apply the font options
    if( mbValidFontOptions )
        mpServerFont->SetFontOptions( maFontOptions );
}

//--------------------------------------------------------------------------

inline sal_Unicode SwapBytes( const sal_Unicode nIn )
{
    return ((nIn >> 8) & 0x00ff) | ((nIn & 0x00ff) << 8);
}

// draw string in a specific multibyte encoding
static void
ConvertTextItem16( XTextItem16* pTextItem, rtl_TextEncoding nEncoding )
{
    if ( (pTextItem == NULL) || (pTextItem->nchars <= 0) )
        return;

    SalConverterCache* pCvt = SalConverterCache::GetInstance();
    // convert the string into the font encoding
    sal_Size  nSize;
    sal_Size  nBufferSize = pTextItem->nchars * 2;
    sal_Char *pBuffer = (sal_Char*)alloca( nBufferSize );

    nSize = pCvt->ConvertStringUTF16( (sal_Unicode*)pTextItem->chars, pTextItem->nchars,
                    pBuffer, nBufferSize, nEncoding);

    sal_Char *pTextChars = (sal_Char*)pTextItem->chars;
    unsigned int n = 0, m = 0;

    if (   nEncoding == RTL_TEXTENCODING_GB_2312
        || nEncoding == RTL_TEXTENCODING_GBT_12345
        || nEncoding == RTL_TEXTENCODING_GBK
        || nEncoding == RTL_TEXTENCODING_BIG5 )
    {
        // GB and Big5 needs special treatment since chars can be single or
        // double byte: encoding is
        // [ 0x00 - 0x7f ] | [ 0x81 - 0xfe ] [ 0x40 - 0x7e 0x80 - 0xfe ]
        while ( n < nSize )
        {
            if ( (unsigned char)pBuffer[ n ] < 0x80 )
            {
                pTextChars[ m++ ] = 0x0;
                pTextChars[ m++ ] = pBuffer[ n++ ];
            }
            else
            {
                pTextChars[ m++ ] = pBuffer[ n++ ];
                pTextChars[ m++ ] = pBuffer[ n++ ];
            }
        }
        pTextItem->nchars = m / 2;
    }
    else
    if ( pCvt->IsSingleByteEncoding(nEncoding) )
    {
        // Single Byte encoding has to be padded
        while ( n < nSize )
        {
            pTextChars[ m++ ] = 0x0;
            pTextChars[ m++ ] = pBuffer[ n++ ];
        }
        pTextItem->nchars = nSize;
    }
    else
    {
        while ( n < nSize )
        {
            pTextChars[ m++ ] = pBuffer[ n++ ];
        }
        pTextItem->nchars = nSize / 2;
    }

    // XXX FIXME
    if (   (nEncoding == RTL_TEXTENCODING_GB_2312)
        || (nEncoding == RTL_TEXTENCODING_EUC_KR) )
    {
        for (unsigned int n_char = 0; n_char < m; n_char++ )
            pTextChars[ n_char ] &= 0x7F;
    }
}

//--------------------------------------------------------------------------
namespace {

class CairoWrapper
{
private:
    oslModule mpCairoLib;

    cairo_surface_t* (*mp_xlib_surface_create_with_xrender_format)(Display *, Drawable , Screen *, XRenderPictFormat *, int , int );
    void (*mp_surface_destroy)(cairo_surface_t *);
    cairo_t* (*mp_create)(cairo_surface_t *);
    void (*mp_destroy)(cairo_t*);
    void (*mp_clip)(cairo_t*);
    void (*mp_rectangle)(cairo_t*, double, double, double, double);
    cairo_font_face_t * (*mp_ft_font_face_create_for_ft_face)(FT_Face, int);
    void (*mp_set_font_face)(cairo_t *, cairo_font_face_t *);
    void (*mp_font_face_destroy)(cairo_font_face_t *);
    void (*mp_matrix_init_identity)(cairo_matrix_t *);
    void (*mp_matrix_scale)(cairo_matrix_t *, double, double);
    void (*mp_matrix_rotate)(cairo_matrix_t *, double);
    void (*mp_set_font_matrix)(cairo_t *, const cairo_matrix_t *);
    void (*mp_show_glyphs)(cairo_t *, const cairo_glyph_t *, int );
    void (*mp_set_source_rgb)(cairo_t *, double , double , double );
    void (*mp_set_font_options)(cairo_t *, const void *);
    void (*mp_ft_font_options_substitute)(const void*, void*);

    bool canEmbolden() const { return false; }

    CairoWrapper();
public:
    static CairoWrapper& get();
    bool isValid() const { return (mpCairoLib != NULL); }
    bool isCairoRenderable(const ServerFont& rFont);

    cairo_surface_t* xlib_surface_create_with_xrender_format(Display *pDisplay, Drawable drawable, Screen *pScreen, XRenderPictFormat *pFormat, int width, int height)
        { return (*mp_xlib_surface_create_with_xrender_format)(pDisplay, drawable, pScreen, pFormat, width, height); }
    void surface_destroy(cairo_surface_t *surface) { (*mp_surface_destroy)(surface); }
    cairo_t* create(cairo_surface_t *surface) { return (*mp_create)(surface); }
    void destroy(cairo_t *cr) { (*mp_destroy)(cr); }
    void clip(cairo_t *cr) { (*mp_clip)(cr); }
    void rectangle(cairo_t *cr, double x, double y, double width, double height)
        { (*mp_rectangle)(cr, x, y, width, height); }
    cairo_font_face_t* ft_font_face_create_for_ft_face(FT_Face face, int load_flags)
        { return (*mp_ft_font_face_create_for_ft_face)(face, load_flags); }
    void set_font_face(cairo_t *cr, cairo_font_face_t *font_face)
        { (*mp_set_font_face)(cr, font_face); }
    void font_face_destroy(cairo_font_face_t *font_face)
        { (*mp_font_face_destroy)(font_face); }
    void matrix_init_identity(cairo_matrix_t *matrix)
        { (*mp_matrix_init_identity)(matrix); }
    void matrix_scale(cairo_matrix_t *matrix, double sx, double sy)
        { (*mp_matrix_scale)(matrix, sx, sy); }
    void matrix_rotate(cairo_matrix_t *matrix, double radians)
        { (*mp_matrix_rotate)(matrix, radians); }
    void set_font_matrix(cairo_t *cr, const cairo_matrix_t *matrix)
        { (*mp_set_font_matrix)(cr, matrix); }
    void show_glyphs(cairo_t *cr, const cairo_glyph_t *glyphs, int no_glyphs)
        { (*mp_show_glyphs)(cr, glyphs, no_glyphs); }
    void set_source_rgb(cairo_t *cr, double red, double green, double blue)
        { (*mp_set_source_rgb)(cr, red, green, blue); }
    void set_font_options(cairo_t *cr, const void *options)
        { (*mp_set_font_options)(cr, options); }
    void ft_font_options_substitute(const void *options, void *pattern)
        { (*mp_ft_font_options_substitute)(options, pattern); }
};

static CairoWrapper* pCairoInstance = NULL;

CairoWrapper& CairoWrapper::get()
{
    if( ! pCairoInstance )
        pCairoInstance = new CairoWrapper();
    return *pCairoInstance;
}

CairoWrapper::CairoWrapper()
:   mpCairoLib( NULL )
{
    static const char* pDisableCairoText = getenv( "SAL_DISABLE_CAIROTEXT" );
    if( pDisableCairoText && (pDisableCairoText[0] != '0') )
        return;

    int nDummy;
    if( !XQueryExtension( GetX11SalData()->GetDisplay()->GetDisplay(), "RENDER", &nDummy, &nDummy, &nDummy ) )
        return;

#ifdef MACOSX
    OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libcairo.2.dylib" ));
#else
    OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libcairo.so.2" ));
#endif
    mpCairoLib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_DEFAULT );
    if( !mpCairoLib )
        return;

#ifdef DEBUG
    // check cairo version
    int (*p_version)();
    p_version = (int(*)()) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_version" );
    const int nVersion = p_version ? (*p_version)() : 0;
    fprintf( stderr, "CAIRO version=%d\n", nVersion );
#endif

    mp_xlib_surface_create_with_xrender_format = (cairo_surface_t* (*)(Display *, Drawable , Screen *, XRenderPictFormat *, int , int ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_xlib_surface_create_with_xrender_format" );
    mp_surface_destroy = (void(*)(cairo_surface_t*))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_surface_destroy" );
    mp_create = (cairo_t*(*)(cairo_surface_t*))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_create" );
    mp_destroy = (void(*)(cairo_t*))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_destroy" );
    mp_clip = (void(*)(cairo_t*))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_clip" );
    mp_rectangle = (void(*)(cairo_t*, double, double, double, double))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_rectangle" );
    mp_ft_font_face_create_for_ft_face = (cairo_font_face_t * (*)(FT_Face, int))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_ft_font_face_create_for_ft_face" );
    mp_set_font_face = (void (*)(cairo_t *, cairo_font_face_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_font_face" );
    mp_font_face_destroy = (void (*)(cairo_font_face_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_font_face_destroy" );
    mp_matrix_init_identity = (void (*)(cairo_matrix_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_init_identity" );
    mp_matrix_scale = (void (*)(cairo_matrix_t *, double, double))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_scale" );
    mp_matrix_rotate = (void (*)(cairo_matrix_t *, double))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_rotate" );
    mp_set_font_matrix = (void (*)(cairo_t *, const cairo_matrix_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_font_matrix" );
    mp_show_glyphs = (void (*)(cairo_t *, const cairo_glyph_t *, int ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_show_glyphs" );
    mp_set_source_rgb = (void (*)(cairo_t *, double , double , double ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_source_rgb" );
    mp_set_font_options = (void (*)(cairo_t *, const void *options ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_font_options" );
    mp_ft_font_options_substitute = (void (*)(const void *, void *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_ft_font_options_substitute" );

    if( !(
            mp_xlib_surface_create_with_xrender_format &&
            mp_surface_destroy &&
            mp_create &&
            mp_destroy &&
            mp_clip &&
            mp_rectangle &&
            mp_ft_font_face_create_for_ft_face &&
            mp_set_font_face &&
            mp_font_face_destroy &&
            mp_matrix_init_identity &&
            mp_matrix_scale &&
            mp_matrix_rotate &&
            mp_set_font_matrix &&
            mp_show_glyphs &&
            mp_set_source_rgb &&
            mp_set_font_options &&
            mp_ft_font_options_substitute
        ) )
    {
        osl_unloadModule( mpCairoLib );
    mpCairoLib = NULL;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "not all needed symbols were found\n" );
#endif
    }
}

bool CairoWrapper::isCairoRenderable(const ServerFont& rFont)
{
    return rFont.GetFtFace() && isValid() && rFont.GetAntialiasAdvice() &&
        (rFont.NeedsArtificialBold() ? canEmbolden() : true);
}

} //namespace

CairoFontsCache::LRUFonts CairoFontsCache::maLRUFonts;
int CairoFontsCache::mnRefCount = 0;

CairoFontsCache::CairoFontsCache()
{
    ++mnRefCount;
}

CairoFontsCache::~CairoFontsCache()
{
    --mnRefCount;
    if (!mnRefCount && !maLRUFonts.empty())
    {
        CairoWrapper &rCairo = CairoWrapper::get();
        LRUFonts::iterator aEnd = maLRUFonts.end();
        for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
            rCairo.font_face_destroy((cairo_font_face_t*)aI->first);
    }
}

void CairoFontsCache::CacheFont(void *pFont, void* pId)
{
    maLRUFonts.push_front( std::pair<void*, void *>(pFont, pId) );
    if (maLRUFonts.size() > 8)
    {
        CairoWrapper &rCairo = CairoWrapper::get();
        rCairo.font_face_destroy((cairo_font_face_t*)maLRUFonts.back().first);
        maLRUFonts.pop_back();
    }
}

void* CairoFontsCache::FindCachedFont(void *pId)
{
    LRUFonts::iterator aEnd = maLRUFonts.end();
    for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
        if (aI->second == pId)
            return aI->first;
    return NULL;
}

void X11SalGraphics::DrawCairoAAFontString( const ServerFontLayout& rLayout )
{
    std::vector<cairo_glyph_t> cairo_glyphs;
    cairo_glyphs.reserve( 256 );

    Point aPos;
    sal_GlyphId aGlyphId;
    for( int nStart = 0; rLayout.GetNextGlyphs( 1, &aGlyphId, aPos, nStart ); )
    {
        cairo_glyph_t aGlyph;
        aGlyph.index = aGlyphId & GF_IDXMASK;
        aGlyph.x = aPos.X();
        aGlyph.y = aPos.Y();
        cairo_glyphs.push_back(aGlyph);
    }

    if (cairo_glyphs.empty())
        return;

    // find a XRenderPictFormat compatible with the Drawable
    XRenderPictFormat* pVisualFormat = GetXRenderFormat();
    DBG_ASSERT( pVisualFormat!=NULL, "no matching XRenderPictFormat for text" );
    if( !pVisualFormat )
        return;

    CairoWrapper &rCairo = CairoWrapper::get();

    Display* pDisplay = GetXDisplay();

    cairo_surface_t *surface = rCairo.xlib_surface_create_with_xrender_format (pDisplay,
        hDrawable_, ScreenOfDisplay(pDisplay, m_nScreen), pVisualFormat, SAL_MAX_INT16, SAL_MAX_INT16);

    /*
     * It might be ideal to cache surface and cairo context between calls and
     * only destroy it when the drawable changes, but to do that we need to at
     * least change the SalFrame etc impls to dtor the SalGraphics *before* the
     * destruction of the windows they reference
    */
    cairo_t *cr = rCairo.create(surface);
    rCairo.surface_destroy(surface);

    if (const void *pOptions = Application::GetSettings().GetStyleSettings().GetCairoFontOptions())
        rCairo.set_font_options( cr, pOptions);

    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
    {
    for (long i = 0; i < pClipRegion_->numRects; ++i)
    {
            rCairo.rectangle(cr,
                pClipRegion_->rects[i].x1,
                pClipRegion_->rects[i].y1,
                pClipRegion_->rects[i].x2 - pClipRegion_->rects[i].x1,
                pClipRegion_->rects[i].y2 - pClipRegion_->rects[i].y1);
    }
        rCairo.clip(cr);
    }

    rCairo.set_source_rgb(cr,
        SALCOLOR_RED(nTextColor_)/255.0,
        SALCOLOR_GREEN(nTextColor_)/255.0,
        SALCOLOR_BLUE(nTextColor_)/255.0);

    ServerFont& rFont = rLayout.GetServerFont();

    cairo_font_face_t* font_face = NULL;

    void *pId = rFont.GetFtFace();
    font_face = (cairo_font_face_t*)m_aCairoFontsCache.FindCachedFont(pId);
    if (!font_face)
    {
        font_face = rCairo.ft_font_face_create_for_ft_face(pId, rFont.GetLoadFlags());
        m_aCairoFontsCache.CacheFont(font_face, pId);
    }

    rCairo.set_font_face(cr, font_face);

    cairo_matrix_t m;
    const ImplFontSelectData& rFSD = rFont.GetFontSelData();
    int nWidth = rFSD.mnWidth ? rFSD.mnWidth : rFSD.mnHeight;

    rCairo.matrix_init_identity(&m);

    if (rLayout.GetOrientation())
        rCairo.matrix_rotate(&m, (3600 - rLayout.GetOrientation()) * M_PI / 1800.0);

    rCairo.matrix_scale(&m, nWidth, rFSD.mnHeight);
    if (rFont.NeedsArtificialItalic())
        m.xy = -m.xx * 0x6000L / 0x10000L;

    rCairo.set_font_matrix(cr, &m);
    rCairo.show_glyphs(cr, &cairo_glyphs[0], cairo_glyphs.size());
    rCairo.destroy(cr);
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerAAFontString( const ServerFontLayout& rLayout )
{
    // get xrender target for this drawable
    Picture aDstPic = GetXRenderPicture();
    if( !aDstPic )
        return;

    // get a XRenderPicture for the font foreground
    // TODO: move into own method
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();
    XRenderPictFormat* pVisualFormat = (XRenderPictFormat*)GetXRenderFormat();
    DBG_ASSERT( pVisualFormat, "we already have a render picture, but XRenderPictFormat==NULL???");
    const int nVisualDepth = pVisualFormat->depth;
    SalDisplay::RenderEntry& rEntry = GetDisplay()->GetRenderEntries( m_nScreen )[ nVisualDepth ];
    if( !rEntry.m_aPicture )
    {
        // create and cache XRenderPicture for the font foreground
        Display* pDisplay = GetXDisplay();
#ifdef DEBUG
        int iDummy;
        unsigned uDummy;
        XLIB_Window wDummy;
        unsigned int nDrawDepth;
        ::XGetGeometry( pDisplay, hDrawable_, &wDummy, &iDummy, &iDummy,
                      &uDummy, &uDummy, &uDummy, &nDrawDepth );
        DBG_ASSERT( static_cast<unsigned>(nVisualDepth) == nDrawDepth, "depth messed up for XRender" );
#endif

        rEntry.m_aPixmap = ::XCreatePixmap( pDisplay, hDrawable_, 1, 1, nVisualDepth );

        XRenderPictureAttributes aAttr;
        aAttr.repeat = true;
        rEntry.m_aPicture = rRenderPeer.CreatePicture ( rEntry.m_aPixmap, pVisualFormat, CPRepeat, &aAttr );
    }

    // set font foreground color and opacity
    XRenderColor aRenderColor = GetXRenderColor( nTextColor_ );
    rRenderPeer.FillRectangle( PictOpSrc, rEntry.m_aPicture, &aRenderColor, 0, 0, 1, 1 );

    // set clipping
    // TODO: move into GetXRenderPicture()?
    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
        rRenderPeer.SetPictureClipRegion( aDstPic, pClipRegion_ );

    ServerFont& rFont = rLayout.GetServerFont();
    X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();
    GlyphSet aGlyphSet = rGlyphPeer.GetGlyphSet( rFont, m_nScreen );

    Point aPos;
    static const int MAXGLYPHS = 160;
    sal_GlyphId aGlyphAry[ MAXGLYPHS ];
    int nMaxGlyphs = rLayout.GetOrientation() ? 1 : MAXGLYPHS;
    for( int nStart = 0;;)
    {
        int nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart );
        if( !nGlyphs )
            break;

        // #i51924# avoid 32->16bit coordinate truncation problem in X11
        // TODO: reevaluate once displays with >30000 pixels are available
        if( aPos.X() >= 30000 || aPos.Y() >= 30000 )
            continue;

        unsigned int aRenderAry[ MAXGLYPHS ];
        for( int i = 0; i < nGlyphs; ++i )
             aRenderAry[ i ] = rGlyphPeer.GetGlyphId( rFont, aGlyphAry[i] );
        rRenderPeer.CompositeString32( rEntry.m_aPicture, aDstPic,
           aGlyphSet, aPos.X(), aPos.Y(), aRenderAry, nGlyphs );
    }
}

//--------------------------------------------------------------------------

bool X11SalGraphics::DrawServerAAForcedString( const ServerFontLayout& rLayout )
{
    ServerFont& rFont = rLayout.GetServerFont();

    // prepare glyphs and get extent of operation
    X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();
    int nXmin = 0;
    int nXmax = 0;
    int nYmin = 0;
    int nYmax = 0;
    int nStart = 0;
    Point aPos;
    sal_GlyphId nGlyph;
    for( bool bFirst=true; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = rGlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;
        const int nX2 = nX1 + pRawBitmap->mnWidth;
        const int nY2 = nY1 + pRawBitmap->mnHeight;

        if( bFirst )
        {
            bFirst = false;
            nXmin = nX1;
            nXmax = nX2;
            nYmin = nY1;
            nYmax = nY2;
        }
        else
        {
            if( nXmin > nX1 ) nXmin = nX1;
            if( nXmax < nX2 ) nXmax = nX2;
            if( nYmin > nY1 ) nYmin = nY1;
            if( nYmax < nY2 ) nYmax = nY2;
        }
    }

    // get XImage
    GetDisplay()->GetXLib()->PushXErrorLevel( true );
    Display* pDisplay = GetXDisplay();

    XRectangle aXRect;
    long nWidth = 1, nHeight = 1;
    if( m_pFrame )
        nWidth = m_pFrame->maGeometry.nWidth, nHeight = m_pFrame->maGeometry.nHeight;
    else if( m_pVDev )
        nWidth = m_pVDev->GetWidth(), nHeight = m_pVDev->GetHeight();

    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
    {
        // get bounding box
        XClipBox( pClipRegion_, &aXRect );
        // clip with window
        if( aXRect.x < 0 ) aXRect.x = 0;

        if( aXRect.y < 0 ) aXRect.y = 0;
        if( aXRect.width+aXRect.x > nWidth ) aXRect.width = nWidth-aXRect.x;
        if( aXRect.height+aXRect.y > nHeight ) aXRect.height = nHeight-aXRect.y;
    }
    else
    {
        aXRect.x = 0;
        aXRect.y = 0;
        aXRect.width = nWidth;
        aXRect.height = nHeight;
    }
    if( m_pFrame )
    {
        // clip with screen
        int nScreenX = m_pFrame->maGeometry.nX+aXRect.x;
        int nScreenY = m_pFrame->maGeometry.nY+aXRect.y;
        const Size& rScreenSize = GetDisplay()->getDataForScreen( m_nScreen ).m_aSize;
        int nScreenW = rScreenSize.Width();
        int nScreenH = rScreenSize.Height();
        if( nScreenX < 0 )
            aXRect.x -= nScreenX, aXRect.width += nScreenX;
        if( nScreenX+aXRect.width > nScreenW )
            aXRect.width = nScreenW-nScreenX;
        if( nScreenY < 0 )
            aXRect.y -= nScreenY, aXRect.height += nScreenY;
        if( nScreenY+aXRect.height > nScreenH )
            aXRect.height = nScreenH-nScreenY;
    }


    if( nXmin < aXRect.x )  nXmin = aXRect.x;
    if( nYmin < aXRect.y )  nYmin = aXRect.y;
    if( nXmax >= aXRect.x+aXRect.width )    nXmax = aXRect.x + aXRect.width - 1;
    if( nYmax >= aXRect.y+aXRect.height )   nYmax = aXRect.y + aXRect.height - 1;

    if( nXmin > nXmax )
        return false;
    if( nYmin > nYmax )
        return false;

    XImage* pImg = XGetImage( pDisplay, hDrawable_,
                              nXmin, nYmin,
                              (nXmax-nXmin+1), (nYmax-nYmin+1),
                              ~0, ZPixmap );
    if( pImg == NULL )
    {
        if( m_pFrame )
        {
            // the reason we did not get an image could be that the frame
            // geometry changed in the meantime; lets get the current geometry
            // and clip against the current window size as well as the screen
            // with the current frame position
            const Size& rScreenSize = GetDisplay()->getDataForScreen(m_nScreen).m_aSize;
            int nScreenW = rScreenSize.Width();
            int nScreenH = rScreenSize.Height();
            XLIB_Window aRoot = None;
            int x = 0, y = 0;
            unsigned int w = 0, h = 0, bw = 0, d;
            XGetGeometry( pDisplay, hDrawable_, &aRoot, &x, &y, &w, &h, &bw, &d );
            XTranslateCoordinates( pDisplay, hDrawable_, aRoot, 0, 0, &x, &y, &aRoot );
            if( nXmin + x < 0 ) // clip on left screen edge
                nXmin += x-nXmin;
            if( nYmin + y < 0 ) // clip on top screen edge
                nYmin += y-nYmin;
            if( nXmax >= int(w) ) // clip on right window egde
                nXmax = w-1;
            if( nYmax >= int(h) ) // clip on bottom window edge
                nYmax = h-1;
            if( nXmax + x >= nScreenW ) // clip on right screen edge
                nXmax -= (nXmax + x - nScreenW)+1;
            if( nYmax + y >= nScreenH ) // clip on bottom screen edge
                nYmax -= (nYmax + y - nScreenH)+1;
            if( nXmax >= nXmin && nYmax >= nYmin )
            {
                // try again to get the image
                pImg = XGetImage( pDisplay, hDrawable_,
                                  nXmin, nYmin,
                                  (nXmax-nXmin+1), (nYmax-nYmin+1),
                                  ~0, ZPixmap );
            }
        }
        if( pImg == NULL )
        {
            GetDisplay()->GetXLib()->PopXErrorLevel();
            return false;
        }
    }

    // prepare context
    GC nGC = SelectFont();
    XGCValues aGCVal;
    XGetGCValues( pDisplay, nGC, GCForeground, &aGCVal );

    unsigned long nOrigColor = XGetPixel( pImg, 0, 0 );
    XPutPixel( pImg, 0, 0, aGCVal.foreground );
    unsigned char aColor[4];
    aColor[0] = pImg->data[0];
    aColor[1] = pImg->data[1];
    aColor[2] = pImg->data[2];
    aColor[3] = pImg->data[3];
    XPutPixel( pImg, 0, 0, nOrigColor );

    // work on XImage
    const int bpp = pImg->bits_per_pixel >> 3;
    for( nStart = 0; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = rGlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;

        if( (nX1 <= nXmax) && (int(nX1 + pRawBitmap->mnWidth) > nXmin)
        &&  (nY1 <= nYmax) && (int(nY1 + pRawBitmap->mnHeight) > nYmin) )
        {
            const unsigned char* p10 = pRawBitmap->mpBits;
            unsigned char* p20 = (unsigned char*)pImg->data;                // dest left limit
            p20 += (nY1 - nYmin) * pImg->bytes_per_line;
            unsigned char* p21 = p20 + (nX1 - nXmin + pImg->xoffset) * bpp;
            int y = pRawBitmap->mnHeight;
            if( y > nYmax - nY1 )
                y = nYmax - nY1 + 1;
            while( --y >= 0 )
            {
                if( p20 >= (unsigned char*)pImg->data )
                {
                    unsigned char* const p22 = p20 + pImg->width * bpp; // dest right limit
                    unsigned char* pDst = p21;
                    const unsigned char* pSrc = p10;
                    for( int x = pRawBitmap->mnWidth; (--x >= 0) && (p22 > pDst); ++pSrc )
                    {
                        if( (*pSrc == 0) || (p20 > pDst) )          // keep background
                            pDst += bpp;
                        else if( *pSrc == 0xFF )                    // paint foreground
                        {
                            const unsigned char* pColor = aColor;
                            for( int z = bpp; --z >= 0; ++pColor, ++pDst )
                                *pDst = *pColor;
                        }
                        else                                        // blend fg into bg
                        {
                            const unsigned char* pColor = aColor;
                            for( int z = bpp; --z >= 0; ++pColor, ++pDst )
                                // theoretically it should be *257) >> 16
                                // but the error is <0.4% worst case and we are in
                                // the innermost loop of very perf-sensitive code

                                *pDst += (*pSrc * ((int)*pColor - *pDst)) >> 8;
                        }
                    }
                }
                p10 += pRawBitmap->mnScanlineSize;
                p20 += pImg->bytes_per_line;
                p21 += pImg->bytes_per_line;
            }
        }
    }

    // put XImage
    XPutImage( pDisplay, hDrawable_, nGC, pImg,
        0, 0, nXmin, nYmin, (nXmax - nXmin + 1), (nYmax - nYmin + 1) );
    XDestroyImage( pImg );

    GetDisplay()->GetXLib()->PopXErrorLevel();
    return true;
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerSimpleFontString( const ServerFontLayout& rSalLayout )
{
    ServerFont& rFont = rSalLayout.GetServerFont();
    X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();

    Display* pDisplay = GetXDisplay();
    GC nGC = SelectFont();

    XGCValues aGCVal;
    aGCVal.fill_style = FillStippled;
    aGCVal.line_width = 0;
    GC tmpGC = XCreateGC( pDisplay, hDrawable_, GCFillStyle|GCLineWidth, &aGCVal );
    XCopyGC( pDisplay, nGC, (1<<GCLastBit)-(1+GCFillStyle+GCLineWidth), tmpGC );

    Point aPos;
    sal_GlyphId nGlyph;
    for( int nStart = 0; rSalLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        // #i51924# avoid 32->16bit coordinate truncation problem in X11
        // TODO: reevaluate once displays with >30000 pixels are available
        if( aPos.X() >= 30000 || aPos.Y() >= 30000 )
            continue;

        Pixmap aStipple = rGlyphPeer.GetPixmap( rFont, nGlyph, m_nScreen );
        const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyph );

        if( aStipple != None )
        {
            const int nDestX    = aPos.X() + rGM.GetOffset().X();
            const int nDestY    = aPos.Y() + rGM.GetOffset().Y();

            aGCVal.stipple      = aStipple;
            aGCVal.ts_x_origin  = nDestX;
            aGCVal.ts_y_origin  = nDestY;
            XChangeGC( pDisplay, tmpGC, GCStipple|GCTileStipXOrigin|GCTileStipYOrigin, &aGCVal );

            const int nWidth    = rGM.GetSize().Width();
            const int nHeight   = rGM.GetSize().Height();
            XFillRectangle( pDisplay, hDrawable_, tmpGC, nDestX, nDestY, nWidth, nHeight );
        }
    }

    XFreeGC( pDisplay, tmpGC );
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    // draw complex text
    ServerFont& rFont = rLayout.GetServerFont();
    const bool bVertical = rFont.GetFontSelData().mbVertical;

    if( !bVertical && CairoWrapper::get().isCairoRenderable(rFont) )
        DrawCairoAAFontString( rLayout );
    else
    {
        X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();
        if( rGlyphPeer.GetGlyphSet( rFont, m_nScreen ) )
            DrawServerAAFontString( rLayout );
        else if( !rGlyphPeer.ForcedAntialiasing( rFont, m_nScreen ) )
            DrawServerSimpleFontString( rLayout );
        else
            DrawServerAAForcedString( rLayout );
    }
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawStringUCS2MB( ExtendedFontStruct& rFont,
    const Point& rPoint, const sal_Unicode* pStr, int nLength )
{
    Display* pDisplay   = GetXDisplay();
    GC       nGC        = SelectFont();

    if( rFont.GetAsciiEncoding() == RTL_TEXTENCODING_UNICODE )
    {
        // plain Unicode, can handle all chars and can be handled straight forward
        XFontStruct* pFontStruct = rFont.GetFontStruct( RTL_TEXTENCODING_UNICODE );
        if( !pFontStruct )
            return;

        XSetFont( pDisplay, nGC, pFontStruct->fid );

        #ifdef OSL_LITENDIAN
        sal_Unicode *pBuffer = (sal_Unicode*)alloca( nLength * sizeof(sal_Unicode) );
        for ( int i = 0; i < nLength; i++ )
            pBuffer[ i ] = SwapBytes(pStr[ i ]) ;
        #else
        sal_Unicode *pBuffer = const_cast<sal_Unicode*>(pStr);
        #endif

        XDrawString16( pDisplay, hDrawable_, nGC, rPoint.X(), rPoint.Y(), (XChar2b*)pBuffer, nLength );
    }
    else
    {
        XTextItem16 *pTextItem = (XTextItem16*)alloca( nLength * sizeof(XTextItem16) );
        XChar2b     *pMBChar   = (XChar2b*)pStr;
        int nItem = 0;

        DBG_ASSERT( nLength<=1, "#i49902# DrawStringUCS2MB with nLength>1 => problems with XOrg6.8.[0123]");

        for( int nChar = 0; nChar < nLength; ++nChar )
        {
            rtl_TextEncoding  nEnc;
            XFontStruct* pFontStruct = rFont.GetFontStruct( pStr[nChar], &nEnc );
            if( !pFontStruct )
                continue;

            pTextItem[ nItem ].chars  = pMBChar + nChar;
            pTextItem[ nItem ].delta  = 0;
            pTextItem[ nItem ].font   = pFontStruct->fid;
            pTextItem[ nItem ].nchars = 1;

            ConvertTextItem16( &pTextItem[ nItem ], nEnc );
            ++nItem;
        }

        XDrawText16( pDisplay, hDrawable_, nGC, rPoint.X(), rPoint.Y(), pTextItem, nItem );
    }
}

//--------------------------------------------------------------------------

ImplFontCharMap* X11SalGraphics::GetImplFontCharMap() const
{
    // TODO: get ImplFontCharMap directly from fonts
    if( !mpServerFont[0] )
        return NULL;

    CmapResult aCmapResult;
    if( !mpServerFont[0]->GetFontCodeRanges( aCmapResult ) )
        return NULL;
    return new ImplFontCharMap( aCmapResult );
}

// ----------------------------------------------------------------------------
//
// SalGraphics
//
// ----------------------------------------------------------------------------

USHORT X11SalGraphics::SetFont( ImplFontSelectData *pEntry, int nFallbackLevel )
{
    USHORT nRetVal = 0;
    if( !setFont( pEntry, nFallbackLevel ) )
        nRetVal |= SAL_SETFONT_BADFONT;
    if( bPrinter_ || (mpServerFont[ nFallbackLevel ] != NULL) )
        nRetVal |= SAL_SETFONT_USEDRAWTEXTARRAY;
    return nRetVal;
}

// ----------------------------------------------------------------------------

void
X11SalGraphics::SetTextColor( SalColor nSalColor )
{
    if( nTextColor_ != nSalColor )
    {
        nTextColor_     = nSalColor;
        nTextPixel_     = GetPixel( nSalColor );
        bFontGC_        = FALSE;
    }
}

// ----------------------------------------------------------------------------

bool X11SalGraphics::AddTempDevFont( ImplDevFontList* pFontList,
    const String& rFileURL, const String& rFontName )
{
    // inform PSP font manager
    rtl::OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFileURL, aUSystemPath ) );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aOFileName( OUStringToOString( aUSystemPath, aEncoding ) );
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    int nFontId = rMgr.addFontFile( aOFileName, 0 );
    if( !nFontId )
        return false;

    // prepare font data
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontFastInfo( nFontId, aInfo );
    aInfo.m_aFamilyName = rFontName;

    // inform glyph cache of new font
    ImplDevFontAttributes aDFA = PspGraphics::Info2DevFontAttributes( aInfo );
    aDFA.mnQuality += 5800;

    int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
    if( nFaceNum < 0 )
        nFaceNum = 0;

    GlyphCache& rGC = X11GlyphCache::GetInstance();
    const rtl::OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
    rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );

    // announce new font to device's font list
    rGC.AnnounceFonts( pFontList );
    return true;
}

// ----------------------------------------------------------------------------

void RegisterFontSubstitutors( ImplDevFontList* );

void X11SalGraphics::GetDevFontList( ImplDevFontList *pList )
{
    // allow disabling of native X11 fonts
    static const char* pEnableX11FontStr = getenv( "SAL_ENABLE_NATIVE_XFONTS" );
    if( pEnableX11FontStr && (pEnableX11FontStr[0] != '0') )
    {
        // announce X11 fonts
        XlfdStorage* pX11FontList = GetDisplay()->GetXlfdList();
        pX11FontList->AnnounceFonts( pList );
    }

    // prepare the GlyphCache using psprint's font infos
    X11GlyphCache& rGC = X11GlyphCache::GetInstance();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    ::std::list< psp::fontID > aList;
    ::std::list< psp::fontID >::iterator it;
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontList( aList );
    for( it = aList.begin(); it != aList.end(); ++it )
    {
        if( !rMgr.getFontFastInfo( *it, aInfo ) )
            continue;

        // the GlyphCache must not bother with builtin fonts because
        // it cannot access or use them anyway
        if( aInfo.m_eType == psp::fonttype::Builtin )
            continue;

        // normalize face number to the GlyphCache
        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
        if( nFaceNum < 0 )
            nFaceNum = 0;

        // for fonts where extra kerning info can be provided on demand
        // an ExtraKernInfo object is supplied
        const ExtraKernInfo* pExtraKernInfo = NULL;
        if( aInfo.m_eType == psp::fonttype::Type1 )
            pExtraKernInfo = new PspKernInfo( *it );

        // inform GlyphCache about this font provided by the PsPrint subsystem
        ImplDevFontAttributes aDFA = PspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 4096;
        const rtl::OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA, pExtraKernInfo );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pList );

    // register platform specific font substitutions if available
    if( rMgr.hasFontconfig() )
        RegisterFontSubstitutors( pList );

    ImplGetSVData()->maGDIData.mbNativeFontConfig = rMgr.hasFontconfig();
}

// ----------------------------------------------------------------------------

void X11SalGraphics::GetDevFontSubstList( OutputDevice* )
{
    // no device specific font substitutions on X11 needed
}

// ----------------------------------------------------------------------------

void cairosubcallback( void* pPattern )
{
    CairoWrapper& rCairo = CairoWrapper::get();
    if( !rCairo.isValid() )
        return;
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const void* pFontOptions = rStyleSettings.GetCairoFontOptions();
    if( !pFontOptions )
        return;
    rCairo.ft_font_options_substitute( pFontOptions, pPattern );
}

bool GetFCFontOptions( const ImplFontAttributes& rFontAttributes, int nSize,
    ImplFontOptions& rFontOptions)
{
    // TODO: get rid of these insane enum-conversions
    // e.g. by using the classic vclenum values inside VCL

    psp::FastPrintFontInfo aInfo;
    // set family name
    aInfo.m_aFamilyName = rFontAttributes.GetFamilyName();
    // set italic
    switch( rFontAttributes.GetSlant() )
    {
        case ITALIC_NONE:
            aInfo.m_eItalic = psp::italic::Upright;
            break;
        case ITALIC_NORMAL:
            aInfo.m_eItalic = psp::italic::Italic;
            break;
        case ITALIC_OBLIQUE:
            aInfo.m_eItalic = psp::italic::Oblique;
            break;
        default:
            aInfo.m_eItalic = psp::italic::Unknown;
            break;
    }
    // set weight
    switch( rFontAttributes.GetWeight() )
    {
        case WEIGHT_THIN:
            aInfo.m_eWeight = psp::weight::Thin;
            break;
        case WEIGHT_ULTRALIGHT:
            aInfo.m_eWeight = psp::weight::UltraLight;
            break;
        case WEIGHT_LIGHT:
            aInfo.m_eWeight = psp::weight::Light;
            break;
        case WEIGHT_SEMILIGHT:
            aInfo.m_eWeight = psp::weight::SemiLight;
            break;
        case WEIGHT_NORMAL:
            aInfo.m_eWeight = psp::weight::Normal;
            break;
        case WEIGHT_MEDIUM:
            aInfo.m_eWeight = psp::weight::Medium;
            break;
        case WEIGHT_SEMIBOLD:
            aInfo.m_eWeight = psp::weight::SemiBold;
            break;
        case WEIGHT_BOLD:
            aInfo.m_eWeight = psp::weight::Bold;
            break;
        case WEIGHT_ULTRABOLD:
            aInfo.m_eWeight = psp::weight::UltraBold;
            break;
        case WEIGHT_BLACK:
            aInfo.m_eWeight = psp::weight::Black;
            break;
        default:
            aInfo.m_eWeight = psp::weight::Unknown;
            break;
    }
    // set width
    switch( rFontAttributes.GetWidthType() )
    {
        case WIDTH_ULTRA_CONDENSED:
            aInfo.m_eWidth = psp::width::UltraCondensed;
            break;
        case WIDTH_EXTRA_CONDENSED:
            aInfo.m_eWidth = psp::width::ExtraCondensed;
            break;
        case WIDTH_CONDENSED:
            aInfo.m_eWidth = psp::width::Condensed;
            break;
        case WIDTH_SEMI_CONDENSED:
            aInfo.m_eWidth = psp::width::SemiCondensed;
            break;
        case WIDTH_NORMAL:
            aInfo.m_eWidth = psp::width::Normal;
            break;
        case WIDTH_SEMI_EXPANDED:
            aInfo.m_eWidth = psp::width::SemiExpanded;
            break;
        case WIDTH_EXPANDED:
            aInfo.m_eWidth = psp::width::Expanded;
            break;
        case WIDTH_EXTRA_EXPANDED:
            aInfo.m_eWidth = psp::width::ExtraExpanded;
            break;
        case WIDTH_ULTRA_EXPANDED:
            aInfo.m_eWidth = psp::width::UltraExpanded;
            break;
        default:
            aInfo.m_eWidth = psp::width::Unknown;
            break;
    }

    const psp::PrintFontManager& rPFM = psp::PrintFontManager::get();
    bool bOK = rPFM.getFontOptions( aInfo, nSize, cairosubcallback, rFontOptions);
    return bOK;
}

// ----------------------------------------------------------------------------

void
X11SalGraphics::GetFontMetric( ImplFontMetricData *pMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if( mpServerFont[nFallbackLevel] != NULL )
    {
        long rDummyFactor;
        mpServerFont[nFallbackLevel]->FetchFontMetric( *pMetric, rDummyFactor );
    }
    else if( mXFont[nFallbackLevel] != NULL )
    {
        mXFont[nFallbackLevel]->ToImplFontMetricData( pMetric );
        if ( bFontVertical_ )
            pMetric->mnOrientation = 0;
    }
}

// ---------------------------------------------------------------------------

ULONG
X11SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData *pKernPairs )
{
    if( ! bPrinter_ )
    {
        if( mpServerFont[0] != NULL )
        {
            ImplKernPairData* pTmpKernPairs;
            ULONG nGotPairs = mpServerFont[0]->GetKernPairs( &pTmpKernPairs );
            for( unsigned int i = 0; i < nPairs && i < nGotPairs; ++i )
                pKernPairs[ i ] = pTmpKernPairs[ i ];
            delete[] pTmpKernPairs;
            return nGotPairs;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------

BOOL X11SalGraphics::GetGlyphBoundRect( long nGlyphIndex, Rectangle& rRect )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return FALSE;

    ServerFont* pSF = mpServerFont[ nLevel ];
    if( !pSF )
        return FALSE;

    nGlyphIndex &= ~GF_FONTMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( nGlyphIndex );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return TRUE;
}

// ---------------------------------------------------------------------------

BOOL X11SalGraphics::GetGlyphOutline( long nGlyphIndex,
    ::basegfx::B2DPolyPolygon& rPolyPoly )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return FALSE;

    ServerFont* pSF = mpServerFont[ nLevel ];
    if( !pSF )
        return FALSE;

    nGlyphIndex &= ~GF_FONTMASK;
    if( pSF->GetGlyphOutline( nGlyphIndex, rPolyPoly ) )
        return TRUE;

    return FALSE;
}

//--------------------------------------------------------------------------

SalLayout* X11SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    SalLayout* pLayout = NULL;

    if( mpServerFont[ nFallbackLevel ]
    && !(rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
    {
#ifdef ENABLE_GRAPHITE
        // Is this a Graphite font?
        if (!bDisableGraphite_ &&
            GraphiteFontAdaptor::IsGraphiteEnabledFont(*mpServerFont[nFallbackLevel]))
        {
            sal_Int32 xdpi, ydpi;

            xdpi = GetDisplay()->GetResolution().A();
            ydpi = GetDisplay()->GetResolution().B();

            GraphiteFontAdaptor * pGrfont = new GraphiteFontAdaptor( *mpServerFont[nFallbackLevel], xdpi, ydpi);
            if (!pGrfont) return NULL;
            pLayout = new GraphiteServerFontLayout(pGrfont);
        }
        else
#endif
            pLayout = new ServerFontLayout( *mpServerFont[ nFallbackLevel ] );
    }
    else if( mXFont[ nFallbackLevel ] )
        pLayout = new X11FontLayout( *mXFont[ nFallbackLevel ] );
    else
        pLayout = NULL;

    return pLayout;
}

//--------------------------------------------------------------------------

SystemFontData X11SalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.nFontId = 0;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    if (mpServerFont[nFallbacklevel] != NULL)
    {
        ServerFont* rFont = mpServerFont[nFallbacklevel];
        aSysFontData.nFontId = rFont->GetFtFace();
        aSysFontData.nFontFlags = rFont->GetLoadFlags();
        aSysFontData.bFakeBold = rFont->NeedsArtificialBold();
        aSysFontData.bFakeItalic = rFont->NeedsArtificialItalic();
        aSysFontData.bAntialias = rFont->GetAntialiasAdvice();
        aSysFontData.bVerticalCharacterType = rFont->GetFontSelData().mbVertical;
    }

    return aSysFontData;
}

//--------------------------------------------------------------------------

BOOL X11SalGraphics::CreateFontSubset(
                                   const rtl::OUString& rToFile,
                                   const ImplFontData* pFont,
                                   sal_Int32* pGlyphIDs,
                                   sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphCount,
                                   FontSubsetInfo& rInfo
                                   )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    bool bSuccess = rMgr.createFontSubset( rInfo,
                                 aFont,
                                 rToFile,
                                 pGlyphIDs,
                                 pEncoding,
                                 pWidths,
                                 nGlyphCount );
    return bSuccess;
}

//--------------------------------------------------------------------------

const void* X11SalGraphics::GetEmbedFontData( const ImplFontData* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
#ifndef _USE_PRINT_EXTENSION_
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
#else
    return NULL;
#endif
}

//--------------------------------------------------------------------------

void X11SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
#ifndef _USE_PRINT_EXTENSION_
    PspGraphics::DoFreeEmbedFontData( pData, nLen );
#endif
}

//--------------------------------------------------------------------------

const Ucs2SIntMap* X11SalGraphics::GetFontEncodingVector( const ImplFontData* pFont, const Ucs2OStrMap** pNonEncoded )
{
#ifndef _USE_PRINT_EXTENSION_
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
#else
    return NULL;
#endif
}

//--------------------------------------------------------------------------

void X11SalGraphics::GetGlyphWidths( const ImplFontData* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    PspGraphics::DoGetGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}

// ===========================================================================
// platform specific font substitution hooks

class FcPreMatchSubstititution
:   public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( ImplFontSelectData& ) const;

private:
    typedef ::boost::unordered_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash >
        CachedFontMapType;
    mutable CachedFontMapType maCachedFontMap;
};

class FcGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
    // TODO: add a cache
public:
    bool FindFontSubstitute( ImplFontSelectData&, OUString& rMissingCodes ) const;
};

void RegisterFontSubstitutors( ImplDevFontList* pList )
{
    // init font substitution defaults
    int nDisableBits = 0;
#ifdef SOLARIS
    nDisableBits = 1; // disable "font fallback" here on default
#endif
    // apply the environment variable if any
    const char* pEnvStr = ::getenv( "SAL_DISABLE_FC_SUBST" );
    if( pEnvStr )
    {
        if( (*pEnvStr >= '0') && (*pEnvStr <= '9') )
            nDisableBits = (*pEnvStr - '0');
        else
            nDisableBits = ~0U; // no specific bits set: disable all
    }

    // register font fallback substitutions (unless disabled by bit0)
    if( (nDisableBits & 1) == 0 )
    {
        static FcPreMatchSubstititution aSubstPreMatch;
        pList->SetPreMatchHook( &aSubstPreMatch );
    }

    // register glyph fallback substitutions (unless disabled by bit1)
    if( (nDisableBits & 2) == 0 )
    {
        static FcGlyphFallbackSubstititution aSubstFallback;
        pList->SetFallbackHook( &aSubstFallback );
    }
}

// -----------------------------------------------------------------------

static ImplFontSelectData GetFcSubstitute(const ImplFontSelectData &rFontSelData, OUString& rMissingCodes )
{
    ImplFontSelectData aRet(rFontSelData);

    const rtl::OString aLangAttrib = MsLangId::convertLanguageToIsoByteString( rFontSelData.meLanguage );

    psp::italic::type eItalic = psp::italic::Unknown;
    if( rFontSelData.GetSlant() != ITALIC_DONTKNOW )
    {
        switch( rFontSelData.GetSlant() )
        {
            case ITALIC_NONE:    eItalic = psp::italic::Upright; break;
            case ITALIC_NORMAL:  eItalic = psp::italic::Italic; break;
            case ITALIC_OBLIQUE: eItalic = psp::italic::Oblique; break;
            default:
                break;
        }
    }

    psp::weight::type eWeight = psp::weight::Unknown;
    if( rFontSelData.GetWeight() != WEIGHT_DONTKNOW )
    {
        switch( rFontSelData.GetWeight() )
        {
            case WEIGHT_THIN:       eWeight = psp::weight::Thin; break;
            case WEIGHT_ULTRALIGHT: eWeight = psp::weight::UltraLight; break;
            case WEIGHT_LIGHT:      eWeight = psp::weight::Light; break;
            case WEIGHT_SEMILIGHT:  eWeight = psp::weight::SemiLight; break;
            case WEIGHT_NORMAL:     eWeight = psp::weight::Normal; break;
            case WEIGHT_MEDIUM:     eWeight = psp::weight::Medium; break;
            case WEIGHT_SEMIBOLD:   eWeight = psp::weight::SemiBold; break;
            case WEIGHT_BOLD:       eWeight = psp::weight::Bold; break;
            case WEIGHT_ULTRABOLD:  eWeight = psp::weight::UltraBold; break;
            case WEIGHT_BLACK:      eWeight = psp::weight::Black; break;
            default:
                break;
        }
    }

    psp::width::type eWidth = psp::width::Unknown;
    if( rFontSelData.GetWidthType() != WIDTH_DONTKNOW )
    {
        switch( rFontSelData.GetWidthType() )
        {
            case WIDTH_ULTRA_CONDENSED: eWidth = psp::width::UltraCondensed; break;
            case WIDTH_EXTRA_CONDENSED: eWidth = psp::width::ExtraCondensed; break;
            case WIDTH_CONDENSED:   eWidth = psp::width::Condensed; break;
            case WIDTH_SEMI_CONDENSED:  eWidth = psp::width::SemiCondensed; break;
            case WIDTH_NORMAL:      eWidth = psp::width::Normal; break;
            case WIDTH_SEMI_EXPANDED:   eWidth = psp::width::SemiExpanded; break;
            case WIDTH_EXPANDED:    eWidth = psp::width::Expanded; break;
            case WIDTH_EXTRA_EXPANDED:  eWidth = psp::width::ExtraExpanded; break;
            case WIDTH_ULTRA_EXPANDED:  eWidth = psp::width::UltraExpanded; break;
            default:
                break;
        }
    }

    psp::pitch::type ePitch = psp::pitch::Unknown;
    if( rFontSelData.GetPitch() != PITCH_DONTKNOW )
    {
        switch( rFontSelData.GetPitch() )
        {
            case PITCH_FIXED:    ePitch=psp::pitch::Fixed; break;
            case PITCH_VARIABLE: ePitch=psp::pitch::Variable; break;
            default:
                break;
        }
    }

    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    aRet.maSearchName = rMgr.Substitute( rFontSelData.maTargetName, rMissingCodes, aLangAttrib, eItalic, eWeight, eWidth, ePitch);

    switch (eItalic)
    {
        case psp::italic::Upright: aRet.meItalic = ITALIC_NONE; break;
        case psp::italic::Italic: aRet.meItalic = ITALIC_NORMAL; break;
        case psp::italic::Oblique: aRet.meItalic = ITALIC_OBLIQUE; break;
        default:
            break;
    }

    switch (eWeight)
    {
        case psp::weight::Thin: aRet.meWeight = WEIGHT_THIN; break;
        case psp::weight::UltraLight: aRet.meWeight = WEIGHT_ULTRALIGHT; break;
        case psp::weight::Light: aRet.meWeight = WEIGHT_LIGHT; break;
        case psp::weight::SemiLight: aRet.meWeight = WEIGHT_SEMILIGHT; break;
        case psp::weight::Normal: aRet.meWeight = WEIGHT_NORMAL; break;
        case psp::weight::Medium: aRet.meWeight = WEIGHT_MEDIUM; break;
        case psp::weight::SemiBold: aRet.meWeight = WEIGHT_SEMIBOLD; break;
        case psp::weight::Bold: aRet.meWeight = WEIGHT_BOLD; break;
        case psp::weight::UltraBold: aRet.meWeight = WEIGHT_ULTRABOLD; break;
        case psp::weight::Black: aRet.meWeight = WEIGHT_BLACK; break;
        default:
                break;
    }

    switch (eWidth)
    {
        case psp::width::UltraCondensed: aRet.meWidthType = WIDTH_ULTRA_CONDENSED; break;
        case psp::width::ExtraCondensed: aRet.meWidthType = WIDTH_EXTRA_CONDENSED; break;
        case psp::width::Condensed: aRet.meWidthType = WIDTH_CONDENSED; break;
        case psp::width::SemiCondensed: aRet.meWidthType = WIDTH_SEMI_CONDENSED; break;
        case psp::width::Normal: aRet.meWidthType = WIDTH_NORMAL; break;
        case psp::width::SemiExpanded: aRet.meWidthType = WIDTH_SEMI_EXPANDED; break;
        case psp::width::Expanded: aRet.meWidthType = WIDTH_EXPANDED; break;
        case psp::width::ExtraExpanded: aRet.meWidthType = WIDTH_EXTRA_EXPANDED; break;
        case psp::width::UltraExpanded: aRet.meWidthType = WIDTH_ULTRA_EXPANDED; break;
        default:
            break;
    }

    switch (ePitch)
    {
        case psp::pitch::Fixed: aRet.mePitch = PITCH_FIXED; break;
        case psp::pitch::Variable: aRet.mePitch = PITCH_VARIABLE; break;
        default:
            break;
    }

    return aRet;
}

namespace
{
    bool uselessmatch(const ImplFontSelectData &rOrig, const ImplFontSelectData &rNew)
    {
        return
          (
            rOrig.maTargetName == rNew.maSearchName &&
            rOrig.meWeight == rNew.meWeight &&
            rOrig.meItalic == rNew.meItalic &&
            rOrig.mePitch == rNew.mePitch &&
            rOrig.meWidthType == rNew.meWidthType
          );
    }
}

//--------------------------------------------------------------------------

bool FcPreMatchSubstititution::FindFontSubstitute( ImplFontSelectData &rFontSelData ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    CachedFontMapType::const_iterator itr = maCachedFontMap.find(rFontSelData.maTargetName);
    if (itr != maCachedFontMap.end())
    {
        // Cached substitution pair
        rFontSelData.maSearchName = itr->second;
        return true;
    }

    rtl::OUString aDummy;
    const ImplFontSelectData aOut = GetFcSubstitute( rFontSelData, aDummy );

    maCachedFontMap.insert(
        CachedFontMapType::value_type(rFontSelData.maTargetName, aOut.maSearchName));

    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const ByteString aOrigName( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    const ByteString aSubstName( aOut.maSearchName, RTL_TEXTENCODING_UTF8 );
    printf( "FcPreMatchSubstititution \"%s\" bipw=%d%d%d%d -> ",
        aOrigName.GetBuffer(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.GetBuffer(),
        aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// -----------------------------------------------------------------------

bool FcGlyphFallbackSubstititution::FindFontSubstitute( ImplFontSelectData& rFontSelData,
    rtl::OUString& rMissingCodes ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
    return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    const ImplFontSelectData aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    // TODO: cache the unicode + srcfont specific result
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const ByteString aOrigName( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    const ByteString aSubstName( aOut.maSearchName, RTL_TEXTENCODING_UTF8 );
    printf( "FcGFSubstititution \"%s\" bipw=%d%d%d%d ->",
        aOrigName.GetBuffer(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.GetBuffer(),
        aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// ===========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
