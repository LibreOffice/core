/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.120 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:38:35 $
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#if !( defined(FREEBSD) || defined(NETBSD) || defined(MACOSX) )
#include <alloca.h>
#endif

#include <gcach_xpeer.hxx>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _VCL_PSPGRAPHICS_H
#include <pspgraphics.h>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef EXTENDED_FONTSTRUCT_HXX
#include "xfont.hxx"
#endif
#ifndef _SV_IMPFONT_HXX
#include <impfont.hxx>
#endif


#include <tools/debug.hxx>
#include <tools/stream.hxx>

#ifndef _USE_PRINT_EXTENSION_
#include <psprint/printergfx.hxx>
#include <psprint/fontmanager.hxx>
#include <psprint/jobdata.hxx>
#include <psprint/printerinfomanager.hxx>
#include <svapp.hxx>
#endif

#ifndef XLFD_ATTRIBUTE_HXX
#include "xlfd_attr.hxx"
#endif
#ifndef XLFD_SIMPLE_HXX
#include "xlfd_smpl.hxx"
#endif
#ifndef XLFD_EXTENDED_HXX
#include "xlfd_extd.hxx"
#endif
#ifndef SAL_CONVERTER_CACHE_HXX_
#include "salcvt.hxx"
#endif

#include <hash_set>

// -----------------------------------------------------------------------

static X11GlyphPeer aX11GlyphPeer;

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
        typedef ::std::hash_set< FontLookup,
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

    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aIdList, NULL );

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
SalDisplay::GetXlfdList()
{
    if ( mpFontList != NULL )
    {
        return mpFontList;
    }
    else
    {
        mpFactory  = new AttributeProvider;
        mpFontList = new XlfdStorage;
        mpFallbackFactory = new VirtualXlfd;

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
    const Size& rPixelSize, sal_Bool bVertical )
{
    // TODO: either get rid of X11 fonts or get rid of the non-hashmapped cache
    if( !pFontCache_ )
    {
        pFontCache_ = new SalFontCache( 64, 64, 16 ); // ???
    }
    else
    {
        ExtendedFontStruct *pItem;
        for ( pItem  = pFontCache_->First();
              pItem != NULL;
              pItem  = pFontCache_->Next() )
        {
            if ( pItem->Match(pRequestedFont, rPixelSize, bVertical) )
            {
                if( pFontCache_->GetCurPos() )
                {
                    pFontCache_->Remove( pItem );
                    pFontCache_->Insert( pItem, (sal_uInt32)0 );
                }
                return pItem;
            }
        }
    }

    // before we expand the cache, we look for very old and unused items
    if( pFontCache_->Count() >= 64 )
    {
        ExtendedFontStruct *pItem;
        for ( pItem = pFontCache_->Last();
              pItem != NULL;
              pItem = pFontCache_->Prev() )
        {
            if( 1 == pItem->GetRefCount() )
            {
                pFontCache_->Remove( pItem );
                pItem->ReleaseRef();
                if( pFontCache_->Count() < 64 )
                    break;
            }
        }
    }

    ExtendedFontStruct *pItem = new ExtendedFontStruct( GetDisplay(),
                                        rPixelSize, bVertical,
                                        const_cast<ExtendedXlfd*>(pRequestedFont) );
    pFontCache_->Insert( pItem, (sal_uInt32)0 );
    pItem->AddRef();

    return pItem;
}

// ---------------------------------------------------------------------------

void
SalDisplay::DestroyFontCache()
{
    if( pFontCache_ )
    {
        ExtendedFontStruct *pItem = pFontCache_->First();
        while( pItem )
        {
            delete pItem;
            pItem = pFontCache_->Next();
        }
        delete pFontCache_;
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

    pFontCache_ = (SalFontCache*)NULL;
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

    // feed psprint's kerning list into a lookup-friendly container
    maUnicodeKernPairs.resize( rKernPairs.size() );
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
        values.graphics_exposures   = True;
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
    bFontVertical_      = pEntry->mbVertical;

#ifdef HDU_DEBUG
    ByteString aName( pEntry->maName, osl_getThreadTextEncoding() );
    fprintf( stderr, "SetFont(lvl=%d,\"%s\", %d*%d, naa=%d,b=%d,i=%d)\n", nFallbackLevel, aName.GetBuffer(), pEntry->mnWidth, pEntry->mnHeight, pEntry->mbNonAntialiased, pEntry->meWeight, pEntry->meItalic );
#endif

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

    if( !pEntry->mpFontData )
        return false;

    // requesting a font provided by builtin rasterizer
    ServerFont* pServerFont = GlyphCache::GetInstance().CacheFont( *pEntry );
    if( pServerFont != NULL )
    {
        if( !pServerFont->TestFont() )
        {
            GlyphCache::GetInstance().UncacheFont( *pServerFont );
            return false;
        }
        mpServerFont[ nFallbackLevel ] = pServerFont;
        return true;
    }

    // requesting a native X11-font
    if( !ImplX11FontData::CheckFontData( *pEntry->mpFontData ) )
        return false;

    ImplX11FontData* pRequestedFont = static_cast<ImplX11FontData*>( pEntry->mpFontData );
    const ExtendedXlfd& rX11Font = pRequestedFont->GetExtendedXlfd();

    Size aReqSize( pEntry->mnWidth, pEntry->mnHeight );
    mXFont[ nFallbackLevel ] = GetDisplay()->GetFont( &rX11Font, aReqSize, bFontVertical_ );
    bFontGC_ = FALSE;
    return true;
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
    int n = 0, m = 0;

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
        for (int n_char = 0; n_char < m; n_char++ )
            pTextChars[ n_char ] &= 0x7F;
    }
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerAAFontString( const ServerFontLayout& rLayout )
{
    Display* pDisplay = GetXDisplay();
    Visual* pVisual = GetDisplay()->GetVisual()->GetVisual();
    XRenderPictFormat* pVisualFormat = (*aX11GlyphPeer.pXRenderFindVisualFormat)( pDisplay, pVisual );

    // create xrender Picture for font foreground
    static Pixmap aPixmap;
    static Picture aSrc = 0;
    if( !aSrc )
    {
        int iDummy;
        unsigned uDummy, nDepth;
        XLIB_Window wDummy;
        XGetGeometry( pDisplay, hDrawable_, &wDummy, &iDummy, &iDummy,
            &uDummy, &uDummy, &uDummy, &nDepth );
        aPixmap = XCreatePixmap( pDisplay, hDrawable_, 1, 1, nDepth );

        XRenderPictureAttributes aAttr;
        aAttr.repeat = true;
        aSrc = (*aX11GlyphPeer.pXRenderCreatePicture)( pDisplay, aPixmap, pVisualFormat, CPRepeat, &aAttr );
    }

    // set font foreground
    GC nGC = SelectFont();
    XGCValues aGCVal;
    XGetGCValues( pDisplay, nGC, GCForeground, &aGCVal );
    aGCVal.clip_mask = None;
    GC tmpGC = XCreateGC( pDisplay, aPixmap, GCForeground | GCClipMask, &aGCVal );
    XDrawPoint( pDisplay, aPixmap, tmpGC, 0, 0 );
    XFreeGC( pDisplay, tmpGC );

    // notify xrender of target drawable
    XRenderPictureAttributes aAttr;
    Picture aDst = (*aX11GlyphPeer.pXRenderCreatePicture)( pDisplay, hDrawable_, pVisualFormat, 0, &aAttr );
    // set clipping
    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
        (*aX11GlyphPeer.pXRenderSetPictureClipRegion)( pDisplay, aDst, pClipRegion_ );

    ServerFont& rFont = rLayout.GetServerFont();
    GlyphSet aGlyphSet = aX11GlyphPeer.GetGlyphSet( rFont );

    Point aPos;
    static const int MAXGLYPHS = 160;
    sal_Int32 aGlyphAry[ MAXGLYPHS ];
    int nMaxGlyphs = rLayout.GetOrientation() ? 1 : MAXGLYPHS;
    for( int nStart = 0;;)
    {
        int nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart );
        if( !nGlyphs )
            break;

        unsigned int aRenderAry[ MAXGLYPHS ];
        for( int i = 0; i < nGlyphs; ++i )
             aRenderAry[ i ] = aX11GlyphPeer.GetGlyphId( rFont, aGlyphAry[i] );
        (*aX11GlyphPeer.pXRenderCompositeString32)( pDisplay, PictOpOver,
            aSrc, aDst, 0, aGlyphSet, 0, 0, aPos.X(), aPos.Y(), aRenderAry, nGlyphs );
    }

    // cleanup
    (*aX11GlyphPeer.pXRenderFreePicture)( pDisplay, aDst );
}

//--------------------------------------------------------------------------

bool X11SalGraphics::DrawServerAAForcedString( const ServerFontLayout& rLayout )
{
    ServerFont& rFont = rLayout.GetServerFont();

    // prepare glyphs and get extent of operation
    int nXmin, nXmax, nYmin, nYmax;
    int nStart = 0;
    Point aPos;
    sal_Int32 nGlyph;
    for( bool bFirst=true; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = aX11GlyphPeer.GetRawBitmap( rFont, nGlyph );
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
    bool bOldXErrorEnabled = GetDisplay()->GetXLib()->GetIgnoreXErrors();
    GetDisplay()->GetXLib()->SetIgnoreXErrors( true );
    Display* pDisplay = GetXDisplay();

    XRectangle aXRect;
    unsigned long nWidth = 1, nHeight = 1;
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
        int nScreenW = GetDisplay()->GetScreenSize().Width();
        int nScreenH = GetDisplay()->GetScreenSize().Height();
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

    XImage* const pImg = XGetImage( pDisplay, hDrawable_,
        nXmin, nYmin, (nXmax-nXmin+1), (nYmax-nYmin+1), ~0, ZPixmap );
    if( pImg == NULL )
        return false;

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
        const RawBitmap* const pRawBitmap = aX11GlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;

        if( (nX1 <= nXmax) && ((nX1 + pRawBitmap->mnWidth) > nXmin)
        &&  (nY1 <= nYmax) && ((nY1 + pRawBitmap->mnHeight) > nYmin) )
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

    GetDisplay()->GetXLib()->SetIgnoreXErrors( bOldXErrorEnabled );
    return true;
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerSimpleFontString( const ServerFontLayout& rSalLayout )
{
    ServerFont& rFont = rSalLayout.GetServerFont();

    Display* pDisplay = GetXDisplay();
    GC nGC = SelectFont();

    XGCValues aGCVal;
    aGCVal.fill_style = FillStippled;
    aGCVal.line_width = 0;
    GC tmpGC = XCreateGC( pDisplay, hDrawable_, GCFillStyle|GCLineWidth, &aGCVal );
    XCopyGC( pDisplay, nGC, (1<<GCLastBit)-(1+GCFillStyle+GCLineWidth), tmpGC );

    Point aPos;
    sal_Int32 nGlyph;
    for( int nStart = 0; rSalLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        Pixmap aStipple = aX11GlyphPeer.GetPixmap( rFont, nGlyph );
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

    if( aX11GlyphPeer.GetGlyphSet( rFont ) )
        DrawServerAAFontString( rLayout );
#ifndef MACOSX        /* ignore X11 fonts on MACOSX */
    else if( !aX11GlyphPeer.ForcedAntialiasing( rFont ) )
        DrawServerSimpleFontString( rLayout );
#endif // MACOSX
    else
        DrawServerAAForcedString( rLayout );
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
    int nPairCount = 0;
    if( mpServerFont[0] )
        nPairCount = mpServerFont[0]->GetFontCodeRanges( NULL );
    else if( mXFont[0] )
        nPairCount = mXFont[0]->GetFontCodeRanges( NULL );

    if( !nPairCount )
        return NULL;

    sal_uInt32* pCodePairs = new sal_uInt32[ 2 * nPairCount ];
    if( mpServerFont[0] )
        mpServerFont[0]->GetFontCodeRanges( pCodePairs );
    else if( mXFont[0] )
        mXFont[0]->GetFontCodeRanges( pCodePairs );
    return new ImplFontCharMap( nPairCount, pCodePairs );
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

    GlyphCache::EnsureInstance( aX11GlyphPeer, false );
    GlyphCache& rGC = GlyphCache::GetInstance();
    const rtl::OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
    rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );

    // announce new font to device's font list
    rGC.AnnounceFonts( pFontList );
    return true;
}

// ----------------------------------------------------------------------------

void X11SalGraphics::GetDevFontList( ImplDevFontList *pList )
{
    // announce X11 fonts
    XlfdStorage* pX11FontList = GetDisplay()->GetXlfdList();
    pX11FontList->AnnounceFonts( pList );

    // prepare the GlyphCache using psprint's font infos
    aX11GlyphPeer.SetDisplay( GetXDisplay(),
        GetDisplay()->GetVisual()->GetVisual() );

#ifdef MACOSX
    GlyphCache::EnsureInstance( aX11GlyphPeer, true );
#else
    GlyphCache::EnsureInstance( aX11GlyphPeer, false );
#endif
    GlyphCache& rGC = GlyphCache::GetInstance();

    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
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
}

// ----------------------------------------------------------------------------

void X11SalGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{}

// ----------------------------------------------------------------------------

void
X11SalGraphics::GetFontMetric( ImplFontMetricData *pMetric )
{
    if( mpServerFont[0] != NULL )
    {
        long rDummyFactor;
        mpServerFont[0]->FetchFontMetric( *pMetric, rDummyFactor );
    }
    else if( mXFont[0] != NULL )
    {
        mXFont[0]->ToImplFontMetricData( pMetric );
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
            for( int i = 0; i < nPairs && i < nGotPairs; ++i )
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

BOOL X11SalGraphics::GetGlyphOutline( long nGlyphIndex, PolyPolygon& rPolyPoly )
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
    GenericSalLayout* pLayout = NULL;

    if( mpServerFont[ nFallbackLevel ]
    && !(rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
        pLayout = new ServerFontLayout( *mpServerFont[ nFallbackLevel ] );
    else if( mXFont[ nFallbackLevel ] )
        pLayout = new X11FontLayout( *mXFont[ nFallbackLevel ] );
    else
        pLayout = NULL;

    return pLayout;
}

//--------------------------------------------------------------------------

BOOL X11SalGraphics::CreateFontSubset(
                                   const rtl::OUString& rToFile,
                                   ImplFontData* pFont,
                                   sal_Int32* pGlyphIDs,
                                   sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphs,
                                   FontSubsetInfo& rInfo
                                   )
{
#ifndef _USE_PRINT_EXTENSION_
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoCreateFontSubset( rToFile, aFont, pGlyphIDs, pEncoding, pWidths, nGlyphs, rInfo );
#else
    return FALSE;
#endif
}

//--------------------------------------------------------------------------

const void* X11SalGraphics::GetEmbedFontData( ImplFontData* pFont, const sal_Unicode* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
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

const std::map< sal_Unicode, sal_Int32 >* X11SalGraphics::GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
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

// ===========================================================================
