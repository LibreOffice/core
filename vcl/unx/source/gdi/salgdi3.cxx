/*************************************************************************
 *
 *  $RCSfile: salgdi3.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: cp $ $Date: 2001-04-06 08:13:36 $
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

#define _SV_SALGDI3_CXX

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#if !( defined(FREEBSD) || defined(NETBSD) )
#include <alloca.h>
#endif

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
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#include <tools/debug.hxx>
#include <tools/stream.hxx>

#if !defined(USE_PSPRINT) && !defined(PRINTER_DUMMY) && !defined(_USE_PRINT_EXTENSION_)
#define Font XLIB_Font
#define Region XLIB_Region
#include <xprinter/xp.h>
#undef Font
#undef Region
#endif

#if defined(USE_PSPRINT)
#include <psprint/printergfx.hxx>
#include <psprint/fontmanager.hxx>
#include <psprint/jobdata.hxx>
#endif

#ifndef ANSI1252_HXX_
#include "ansi1252.hxx"
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

#ifdef USE_BUILTIN_RASTERIZER
#include <gcach_xpeer.hxx>
#endif // USE_BUILTIN_RASTERIZER

// -----------------------------------------------------------------------

#ifdef USE_BUILTIN_RASTERIZER
static X11GlyphPeer aX11GlyphPeer;
#endif // USE_BUILTIN_RASTERIZER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SalGraphicsData::FaxPhoneComment( const sal_Unicode* pStr, USHORT nLen ) const
{
#ifdef USE_PSPRINT
    if( ! m_pPhoneNr )
        return;
#else
    if( ! bPrinter_ )
        return;
#endif

#define FAX_PHONE_TOKEN          "@@#"
#define FAX_PHONE_TOKEN_LENGTH   3
#define FAX_END_TOKEN            "@@"

    USHORT nPos;
    ByteString aPhone( pStr, nLen, gsl_getSystemTextEncoding() );

    static ByteString aPhoneNumber;
    static BOOL   bIsCollecting = FALSE;

    if( ! bIsCollecting )
    {
        if( ( nPos = aPhone.Search( FAX_PHONE_TOKEN ) ) != STRING_NOTFOUND )
        {
            aPhone.Erase( 0, nPos + FAX_PHONE_TOKEN_LENGTH );
            bIsCollecting = TRUE;
            aPhoneNumber.Erase();
        }
    }
    if( bIsCollecting )
    {
        if( ( nPos = aPhone.Search( FAX_END_TOKEN ) ) != STRING_NOTFOUND )
        {
            aPhone.Erase( nPos );
            bIsCollecting = FALSE;
        }
        aPhoneNumber += aPhone;
        if( ! bIsCollecting )
        {
#ifndef PRINTER_DUMMY
#ifndef USE_PSPRINT
            aPhone = "PhoneNumber(";
            aPhone += aPhoneNumber;
            aPhone += ")\n";
#if !defined(_USE_PRINT_EXTENSION_)
            XpPSComment( GetDisplay()->GetDisplay(), aPhone.GetBuffer() );
#endif
#else
            *m_pPhoneNr = String( aPhoneNumber, gsl_getSystemTextEncoding() );
#endif
#endif
            aPhoneNumber = ByteString();
        }
    }
    if( aPhoneNumber.Len() > 1024 )
    {
        bIsCollecting = FALSE;
        aPhoneNumber = ByteString();
    }
}

// ----------------------------------------------------------------------------
//
// manage X11 fonts and self rastered fonts
//
// ----------------------------------------------------------------------------

#if defined(USE_PSPRINT)

static FontItalic ToFontItalic (psp::italic::type eItalic);
static FontWeight ToFontWeight (psp::weight::type eWeight);

class FontLookup
{
    public:

        struct hash;
        struct equal;
        typedef ::std::hash_set< FontLookup,
                                 FontLookup::hash,
                                 FontLookup::equal > fl_hashset;

    private:

        FontWeight          mnWeight;
        FontItalic          mnItalic;
        rtl::OString        maName;

    public:

                            FontLookup ( ::std::list< psp::fontID >::iterator& it,
                                          const psp::PrintFontManager& rMgr );
                            FontLookup (const Xlfd& rFont);
                            FontLookup (const FontLookup &rRef) :
                                    mnWeight (rRef.mnWeight),
                                    mnItalic (rRef.mnItalic),
                                    maName   (rRef.maName)
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
                                        && (maName   == rRef.maName);
                            }
        FontLookup&         operator= (const FontLookup &rRef)
                            {
                                mnWeight = rRef.mnWeight;
                                mnItalic = rRef.mnItalic;
                                maName   = rRef.maName;

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
        mnItalic = ToFontItalic (aInfo.m_eItalic);
        mnWeight = ToFontWeight (aInfo.m_eWeight);
        maName   = rtl::OUStringToOString (aInfo.m_aFamilyName,
                                        RTL_TEXTENCODING_ISO_8859_1).toLowerCase();

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

// ----------------------------------------------------------------------------
//
// manage a fallback for raster fonts
//
// ----------------------------------------------------------------------------

class FontFallback
{
    private:

        inline bool         equalItalic (psp::italic::type from, psp::italic::type to);
        inline bool         equalWeight (psp::weight::type from, psp::weight::type to);
        ServerFont*         ImplFallbackFor (const ImplFontSelectData *pData);

        ::psp::fontID       mnId;
        void*               mpSysData;

    public:
                            FontFallback ();
        static ServerFont*  FallbackFor (const ImplFontSelectData *pData);
        static FontFallback* GetInstance ();
};

bool
FontFallback::equalWeight (psp::weight::type from, psp::weight::type to)
{
    return from > to ? (from - to) <= 3 : (to - from) <= 3;
}

bool
FontFallback::equalItalic (psp::italic::type from, psp::italic::type to)
{
    if ( (from == psp::italic::Italic) || (from == psp::italic::Oblique) )
        return (to == psp::italic::Italic) || (to == psp::italic::Oblique);
    return to == from;
}

FontFallback::FontFallback () :
        mnId (-1),
        mpSysData (NULL)
{
    // get static fontlist
    ::std::list< psp::fontID > aList;
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aList, NULL );

    // get fontid of andale
    ::rtl::OUString aName (RTL_CONSTASCII_USTRINGPARAM("Andale WT UI"));
    ::std::list< psp::fontID >::iterator it;
    for (it = aList.begin(); it != aList.end() && mnId == -1; ++it)
    {
        psp::FastPrintFontInfo aInfo;
        if (rMgr.getFontFastInfo (*it, aInfo))
        {
            if ( ! equalItalic (aInfo.m_eItalic, psp::italic::Upright) )
                continue;
            if ( ! equalWeight (aInfo.m_eWeight, psp::weight::Normal) )
                continue;
            if ( aName.equalsIgnoreCase (aInfo.m_aFamilyName) )
                mnId = *it;
        }
    }

    // get sysdata handle for andale
    if (mnId != -1)
    {
        GlyphCache& rGC = GlyphCache::GetInstance();
        mpSysData = rGC.GetFontHandle (mnId);
        if (mpSysData == NULL)
            mnId = -1;
    }
}

ServerFont*
FontFallback::ImplFallbackFor (const ImplFontSelectData *pData)
{
    if (mpSysData == NULL)
        return NULL;

    ImplFontSelectData  aFaksimile;
    ImplFontData        aFaksimileData;

    aFaksimile.mnHeight              = pData->mnHeight;
    aFaksimile.mnWidth               = 0;
    aFaksimile.mnOrientation         = pData->mnOrientation;
    aFaksimile.mbVertical            = pData->mbVertical;
    aFaksimile.mbNonAntialiased      = pData->mbNonAntialiased;
    aFaksimile.mpFontData            = &aFaksimileData;
    aFaksimile.mpFontData->mpSysData = mpSysData;

    return GlyphCache::GetInstance().CacheFont (aFaksimile);
}

ServerFont*
FontFallback::FallbackFor (const ImplFontSelectData *pData)
{
    FontFallback* pInstance = FontFallback::GetInstance();
    if (pInstance != NULL)
        return pInstance->ImplFallbackFor (pData);
    return NULL;
}

FontFallback*
FontFallback::GetInstance ()
{
    static FontFallback *pInstance = NULL;
    if (pInstance == NULL)
        pInstance = new FontFallback;

    return pInstance;
}

#endif /* USE_PSPRINT */

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

        #if defined(USE_PSPRINT)
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
            {
                continue;
            }
            // exclude fonts with unknown encoding
            if ( pXlfdList[i].GetEncoding() == RTL_TEXTENCODING_DONTKNOW )
            {
                continue;
            }
            // exclude "interface system" and "interface user"
            if (pAttr->HasFeature( XLFD_FEATURE_APPLICATION_FONT ) )
            {
                continue;
            }
            // exclude fonts already managed by fontmanager, anyway keep
            // gui fonts: they are candidates for GetInterfaceFont ()
            if (pXlfdList[i].Fonttype() == eTypeScalable)
                ((VirtualXlfd*)mpFallbackFactory)->FilterInterfaceFont (pXlfdList + i);
            #if defined(USE_PSPRINT) && defined(USE_BUILTIN_RASTERIZER)
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
SalDisplay::GetFont( const ExtendedXlfd *pRequestedFont, int nPixelSize, sal_Bool bVertical )
{
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
            if ( pItem->Match(pRequestedFont, nPixelSize, bVertical) )
            {
                if( pFontCache_->GetCurPos() )
                {
                    pFontCache_->Remove( pItem );
                    pFontCache_->Insert( pItem, 0UL );
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
                                        nPixelSize, bVertical,
                                        const_cast<ExtendedXlfd*>(pRequestedFont) );
    pFontCache_->Insert( pItem, 0UL );
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

// ----------------------------------------------------------------------------
//
// SalGraphicsData
//
// ----------------------------------------------------------------------------

GC
SalGraphicsData::SelectFont()
{
    Display *pDisplay = GetXDisplay();

    if( !pFontGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = IncludeInferiors;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = True;
        values.foreground           = nTextPixel_;
#ifdef _USE_PRINT_EXTENSION_
                values.background = xColormap_->GetWhitePixel();
#endif
#ifdef _USE_PRINT_EXTENSION_
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                                GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCBackground | GCForeground,
                              &values );
    }
#else
            pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                    GCSubwindowMode | GCFillRule
                    | GCGraphicsExposures | GCForeground,
                    &values );
        }
#endif
    if( !bFontGC_ )
    {
        XSetForeground( pDisplay, pFontGC_, nTextPixel_ );
        SetClipRegion( pFontGC_ );
        bFontGC_ = TRUE;
    }

    return pFontGC_;
}

//--------------------------------------------------------------------------

// Select the max size of a font, which is token for real
// This routine is (and should be) called only once, the result should be
// stored in some static variable

static int
GetMaxFontHeight()
{
    #define DEFAULT_MAXFONTHEIGHT 250

    int  nMaxFontHeight = 0;

    char *FontHeight = getenv ("SAL_MAXFONTHEIGHT");
    if (FontHeight)
        nMaxFontHeight = atoi (FontHeight);

    if (nMaxFontHeight <= 0)
        nMaxFontHeight = DEFAULT_MAXFONTHEIGHT;

    return nMaxFontHeight;
}

void
SalGraphicsData::SetFont( const ImplFontSelectData *pEntry )
{
    bFontGC_            = FALSE;
    xFont_              = NULL; // ->ReleaseRef()
    mxFallbackFont      = NULL;
    aScale_             = Fraction( 1, 1 );
    nFontOrientation_   = pEntry->mnOrientation;
    bFontVertical_      = pEntry->mbVertical;

#ifdef USE_BUILTIN_RASTERIZER
    if( mpServerSideFont != NULL )
    {
        // old server side font is no longer referenced
        GlyphCache::GetInstance().UncacheFont( *mpServerSideFont );
        mpServerSideFont = NULL;
    }
    if( mpSrvFallbackFont != NULL )
    {
        GlyphCache::GetInstance().UncacheFont( *mpSrvFallbackFont );
        mpSrvFallbackFont = NULL;
    }
#endif //USE_BUILTIN_RASTERIZER

    if( pEntry->mpFontData && pEntry->mpFontData->mpSysData )
    {
#ifdef USE_BUILTIN_RASTERIZER
        // requesting a font provided by builtin rasterizer
        mpServerSideFont = GlyphCache::GetInstance().CacheFont( *pEntry );
        if( mpServerSideFont != NULL )
        {
            if( mpServerSideFont->TestFont() )
            {
                #if defined(USE_PSPRINT) && defined(USE_BUILTIN_RASTERIZER)
                mpSrvFallbackFont = FontFallback::FallbackFor (pEntry);
                if ( mpSrvFallbackFont != NULL && ! mpSrvFallbackFont->TestFont() )
                {
                    GlyphCache::GetInstance().UncacheFont( *mpSrvFallbackFont );
                    mpSrvFallbackFont = NULL;
                }
                #endif
            }
            else
            {
                GlyphCache::GetInstance().UncacheFont( *mpServerSideFont );
                mpServerSideFont = NULL;
                xFont_ = mxFallbackFont;
            }

            return;
        }
#endif //USE_BUILTIN_RASTERIZER

        ExtendedXlfd *pSysFont = (ExtendedXlfd*)pEntry->mpFontData->mpSysData;
        static int nMaxFontHeight = GetMaxFontHeight();

        USHORT         nH, nW;
        if( bWindow_ )
        {
            // see BugId #44528# FontWork (-> #45038#) and as well Bug #47127#
            if( pEntry->mnHeight > nMaxFontHeight )
                nH = nMaxFontHeight;
            else if( pEntry->mnHeight > 2 )
                nH = pEntry->mnHeight;
            else
                nH = 2;
            nW = 0; // pEntry->mnWidth;
        }
        else
        {
            nH = pEntry->mnHeight;
            nW = pEntry->mnWidth;
        }

        xFont_ = GetDisplay()->GetFont( pSysFont, nH, bFontVertical_ );
        const ExtendedXlfd *pFactory = GetDisplay()->GetFallbackFactory();
        if ( pFactory != NULL)
            mxFallbackFont = GetDisplay()->GetFont(pFactory, nH, bFontVertical_);
        if( pEntry->mnHeight > nMaxFontHeight || pEntry->mnHeight < 2 )
            aScale_ = Fraction( pEntry->mnHeight, nH );
    }
    else
    {
        xFont_ = mxFallbackFont;
    }
}

//--------------------------------------------------------------------------

static sal_Unicode
SwapBytes( const sal_Unicode nIn )
{
    return ((nIn >> 8) & 0x00ff) | ((nIn << 8) & 0xff00);
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

#ifdef USE_BUILTIN_RASTERIZER
#ifdef USE_XRENDER

void SalGraphicsData::DrawServerAAFontString( int nX, int nY,
    ServerFont *pFont, const sal_uInt16* pGlyph, int nLength, const long* pDXAry )
{
    // translate unicode to glyph ids and make sure they are already on the server
    unsigned short* pGlyphString = (unsigned short*)alloca( 2*nLength );
    for( int i = 0; i < nLength; ++i )
    {
        const int nGlyphIndex = pGlyph[i];
        pGlyphString[ i ] = aX11GlyphPeer.GetGlyphId( *pFont, nGlyphIndex );
    }

    Display* pDisplay = GetXDisplay();
    Visual* pVisual = DefaultVisual( pDisplay, 0 );
    XRenderPictFormat* pVisualFormat = (*aX11GlyphPeer.pXRenderFindVisualFormat)( pDisplay, pVisual );

    static Pixmap aPixmap = NULL;
    static Picture aSrc = NULL;
    if( !aPixmap )
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

    // set foreground
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

    GlyphSet aGlyphSet = aX11GlyphPeer.GetGlyphSet( *pFont );
    if( pDXAry == NULL )
    {
        // draw the whole string
        (*aX11GlyphPeer.pXRenderCompositeString16)( pDisplay, PictOpOver,
            aSrc, aDst, 0, aGlyphSet, 0, 0, nX, nY, pGlyphString, nLength );
    }
    else
    {
        Point aPos( nX, nY );
        // draw the string glyph by glyph
        for( int i = 0; i < nLength; ++i )
        {
            (*aX11GlyphPeer.pXRenderCompositeString16)( pDisplay, PictOpOver,
                aSrc, aDst, 0, aGlyphSet, 0, 0, aPos.X(), aPos.Y(), pGlyphString+i, 1 );
            aPos = Point(nX,nY) + pFont->TransformPoint( Point(pDXAry[i],0) );
        }
    }

    // cleanup
    (*aX11GlyphPeer.pXRenderFreePicture)( pDisplay, aDst );
}

#endif // USE_XRENDER

//--------------------------------------------------------------------------

bool SalGraphicsData::DrawServerAAForcedString( int nX, int nY,
    ServerFont *pFont, const sal_uInt16* pGlyph, int nLength, const long* pDXAry )
{
    // prepare glyphs and get extent of operation
    int nXmin, nXmax, nYmin, nYmax;
    Point aPos( nX, nY );
    int i = 0;
    for( bool bFirst = true; i < nLength; ++i )
    {
        const int nGlyphIndex =  pGlyph[i] ;
        const RawBitmap* const pRawBitmap = aX11GlyphPeer.GetRawBitmap( *pFont, nGlyphIndex );
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

        if( pDXAry != NULL )
            aPos = Point(nX,nY) + pFont->TransformPoint( Point(pDXAry[i],0) );
        else {
            const GlyphMetric& rGM = pFont->GetGlyphMetric( nGlyphIndex );
            aPos += pFont->TransformPoint( Point(rGM.GetCharWidth(),0) );
        }
    }

    // get XImage
    GetDisplay()->GetXLib()->SetIgnoreXErrors( true );
    Display* pDisplay = GetXDisplay();

    if( pClipRegion_ && !XEmptyRegion( pClipRegion_ ) )
    {
        XRectangle aXRect;
        XClipBox( pClipRegion_, &aXRect );
        if( nXmin < aXRect.x )  nXmin = aXRect.x;
        if( nYmin < aXRect.y )  nYmin = aXRect.y;
        if( nXmax >= aXRect.x+aXRect.width )    nXmax = aXRect.x + aXRect.width - 1;
        if( nYmax >= aXRect.y+aXRect.height )   nYmax = aXRect.y + aXRect.height - 1;
    }

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
    aPos = Point( nX, nY );
    const int bpp = pImg->bits_per_pixel >> 3;
    for( i = 0; i < nLength; ++i )
    {
        const int nGlyphIndex = pGlyph [i];
        const RawBitmap* const pRawBitmap = aX11GlyphPeer.GetRawBitmap( *pFont, nGlyphIndex );
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
                    unsigned char* const p22 = p20 + pImg->bytes_per_line;  // dest right limit
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
        if( pDXAry != NULL )
            aPos = Point(nX,nY) + pFont->TransformPoint( Point(pDXAry[i],0) );
        else {
            const GlyphMetric& rGM = mpServerSideFont->GetGlyphMetric( nGlyphIndex );
            aPos += pFont->TransformPoint( Point(rGM.GetCharWidth(),0) );
        }
    }

    // put XImage
    XPutImage( pDisplay, hDrawable_, nGC, pImg,
        0, 0, nXmin, nYmin, (nXmax - nXmin + 1), (nYmax - nYmin + 1) );
    XDestroyImage( pImg );

    GetDisplay()->GetXLib()->SetIgnoreXErrors( false );
    return true;
}

//--------------------------------------------------------------------------

void SalGraphicsData::DrawServerSimpleFontString(
    int nX, int nY, ServerFont *pFont, const sal_uInt16* pGlyph, int nLength, const long* pDXAry )
{
    Display* pDisplay   = GetXDisplay();
    GC nGC = SelectFont();

    XGCValues aGCVal;
    aGCVal.fill_style = FillStippled;
    aGCVal.line_width = 0;
    GC tmpGC = XCreateGC( pDisplay, hDrawable_, GCFillStyle|GCLineWidth, &aGCVal );
    XCopyGC( pDisplay, nGC, (1<<GCLastBit)-(1+GCFillStyle+GCLineWidth), tmpGC );

    Point aPos( nX, nY );
    for( int i = 0; i < nLength; ++i )
    {
        const int nGlyphIndex = pGlyph[i];

        Pixmap aStipple = aX11GlyphPeer.GetPixmap( *pFont, nGlyphIndex );
        const GlyphMetric& rGM  = pFont->GetGlyphMetric( nGlyphIndex );

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
        if( pDXAry != NULL )
            aPos = Point(nX,nY) + pFont->TransformPoint( Point(pDXAry[i],0) );
        else
            aPos += pFont->TransformPoint( Point(rGM.GetCharWidth(),0) );
    }

    XFreeGC( pDisplay, tmpGC );
}

//--------------------------------------------------------------------------

void SalGraphicsData::DispatchServerFontString( int nX, int nY,
    ServerFont *pFont, const sal_uInt16* pGlyph, int nLen, const long* pDXAry )
{
    #ifdef USE_XRENDER
    if( aX11GlyphPeer.GetGlyphSet( *pFont ) )
        DrawServerAAFontString( nX, nY, pFont, pGlyph, nLen, pDXAry );
    else
    #endif // USE_XRENDER
    if( aX11GlyphPeer.ForcedAntialiasing( *pFont ) )
        DrawServerAAForcedString( nX, nY, pFont, pGlyph, nLen, pDXAry );
    else
        DrawServerSimpleFontString( nX, nY, pFont, pGlyph, nLen, pDXAry );
}

//--------------------------------------------------------------------------

void SalGraphicsData::DrawServerFontString(
    int nX, int nY, const sal_Unicode* pStr, int nLength, const long* pDXAry )
{
    sal_uInt16 *pFGlyph  = (sal_uInt16*)alloca( nLength * sizeof(sal_uInt16) );
    for( int i = 0; i < nLength; ++i )
    {
        pFGlyph[i] = mpServerSideFont->GetGlyphIndex( pStr[i] );
    }

    if ((mpSrvFallbackFont == NULL) || (pDXAry == 0) || (mnOrientation != 0))
    {
        DispatchServerFontString (nX, nY, mpServerSideFont, pFGlyph, nLength, pDXAry);
        return;
    }

    sal_uInt16 *pFFGlyph    = (sal_uInt16*)alloca( nLength * sizeof(sal_uInt16) );
    long       *pDeltaArray = (long*)      alloca( nLength * sizeof(long) );

    int nAdvance = 0;

    for (int nFrom = 0; nFrom < nLength; )
    {
        int nLen  = 0;

        if (pFGlyph[ nFrom + nLen ] != 0)
        {
            while (pFGlyph[ nFrom + nLen ] != 0 && (nFrom + nLen) < nLength)
            {
                pDeltaArray[ nLen ] = pDXAry[ nFrom + nLen ] - nAdvance;
                nLen++;
            }
            if (nLen)
            {
                DispatchServerFontString(nX + nAdvance, nY, mpServerSideFont,
                                        pFGlyph + nFrom, nLen, pDeltaArray);
            }
        }
        else
        {
            while (pFGlyph[ nFrom + nLen ] == 0 && (nFrom + nLen) < nLength)
            {
                pFFGlyph[ nFrom + nLen ] = mpSrvFallbackFont->GetGlyphIndex(pStr[nFrom + nLen]);
                if (pFFGlyph[ nFrom + nLen ] == 0)
                    pFFGlyph[ nFrom + nLen ] = mpSrvFallbackFont->GetGlyphIndex( '?' );
                pDeltaArray[ nLen ] = pDXAry[ nFrom + nLen ] - nAdvance;
                nLen++;
            }
            if (nLen)
            {
                DispatchServerFontString(nX + nAdvance, nY, mpSrvFallbackFont,
                                        pFFGlyph + nFrom, nLen, pDeltaArray);
            }
        }

        nFrom    += nLen;
        nAdvance  = pDXAry[ nFrom - 1];
    }

    return;
}

#endif // USE_BUILTIN_RASTERIZER

//--------------------------------------------------------------------------

void
SalGraphicsData::DrawStringUCS2( int nX, int nY, const sal_Unicode* pStr, int nLength )
{
    // plain Unicode, can handle all chars and can be handled straight forward
    XFontStruct* pFontStruct = xFont_->GetFontStruct( RTL_TEXTENCODING_UNICODE );

    if ( pFontStruct == NULL )
        return;

    Display* pDisplay   = GetXDisplay();
    GC       nGC        = SelectFont();

    XSetFont( pDisplay, nGC, pFontStruct->fid );

    #ifdef OSL_LITENDIAN
    sal_Unicode *pBuffer = (sal_Unicode*)alloca( nLength * sizeof(sal_Unicode) );
    for ( int i = 0; i < nLength; i++ )
        pBuffer[ i ] = SwapBytes(pStr[ i ]) ;
    #else
    sal_Unicode *pBuffer = const_cast<sal_Unicode*>(pStr);
    #endif

    XDrawString16( pDisplay, hDrawable_, nGC, nX, nY, (XChar2b*)pBuffer, nLength );
}

void
SalGraphicsData::DrawStringMB ( int nX, int nY, const sal_Unicode* pStr, int nLength )
{
    XTextItem16 *pTextItem = (XTextItem16*)alloca( nLength * sizeof(XTextItem16) );
    XChar2b     *pMBChar   = (XChar2b*)    alloca( nLength * sizeof(XChar2b) );

    memcpy( pMBChar, pStr, nLength * sizeof(XChar2b) );

    rtl_TextEncoding  nEnc;
    XFontStruct      *pFont;

    for ( int nChar = 0, nItem = 0; nChar < nLength; nChar++ )
    {
        if ( !(pFont = xFont_->GetFontStruct(pStr[nChar], &nEnc)) )
            if ( !(pFont = mxFallbackFont->GetFontStruct(pStr[nChar], &nEnc)) )
                if ( !(pFont = mxFallbackFont->GetFontStruct((sal_Unicode)'?', &nEnc)) )
                    continue;
                else
                    pMBChar[nChar].byte1 = 0, pMBChar[nChar].byte2 = '?';

        pTextItem[ nItem ].chars  = pMBChar + nChar;
        pTextItem[ nItem ].delta  = 0;
        pTextItem[ nItem ].font   = pFont->fid;
        pTextItem[ nItem ].nchars = 1;

        ConvertTextItem16( &pTextItem[ nItem ], nEnc );
        ++nItem;
    }

    Display* pDisplay   = GetXDisplay();
    GC       nGC        = SelectFont();
    XDrawText16( pDisplay, hDrawable_, nGC, nX, nY, pTextItem, nItem );
}


// draw string in one of the fonts / encodings that are available in the
// extended font
void
SalGraphicsData::DrawText( long nX, long nY, const sal_Unicode* pStr, USHORT nLength )
{
    if (nLength == 0)
        return;

    if( xFont_ != NULL )
    {
        if( xFont_->GetAsciiEncoding() == RTL_TEXTENCODING_UNICODE )
            DrawStringUCS2 ( nX, nY, pStr, nLength );
        else
            DrawStringMB ( nX, nY, pStr, nLength );
    }
}

//--------------------------------------------------------------------------

void
SalGraphics::DrawText( long nX, long nY, const xub_Unicode* pStr, USHORT nLen )
{
    maGraphicsData.FaxPhoneComment( pStr, nLen );
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
         maGraphicsData.m_pPrinterGfx->DrawText( Point(nX, nY), pStr, nLen);
    else
#endif
    {
#ifdef USE_BUILTIN_RASTERIZER
        if( maGraphicsData.mpServerSideFont != NULL )
            maGraphicsData.DrawServerFontString( nX, nY, pStr, nLen, NULL );
        else
#endif
            maGraphicsData.DrawText( nX, nY, pStr, nLen );
    }
}

//--------------------------------------------------------------------------

static BOOL
CheckNoNegativeCoordinateWorkaround()
{
    /* Motivation: one of our clients uses a Solaris 2.4 X86 system with an
       XServer for the Matrox Mystique graphics card. This board/server
       sometimes does not draw Text with negative x-coordinates into a
       virtual device (for unknown reasons). A stock X-server just clips the
       part in the negative area. */
    static int nCheck = -2;
    if( nCheck == -2 )
    {
        char* pCmp = getenv( "SAL_NO_NEGATIVE_TEXT_OFFSET" );
        if( pCmp && ! strncasecmp( pCmp, "true", 4 ) )
            nCheck = 1;
        else
            nCheck = 0;
    }
    return nCheck ? TRUE : FALSE;
}

void SalGraphicsData::DrawText( long nX, long nY,
        const sal_Unicode* pStr, USHORT nLen, const long* pDXAry )
{
    // workaround for problems with negative coordinates
    long* pTmpAry = NULL;
    if( nX < 0 && CheckNoNegativeCoordinateWorkaround() )
    {
        long nOldX = nX;
        while( nX < 0 )
        {
            nX = nOldX + *pDXAry;
            pStr++, pDXAry++, nLen--;
            if( nLen < 1 )
                return;
        }
        pTmpAry = new long[ nLen ];
        for( int q = 0; q < nLen-1; q++ )
            pTmpAry[q] = pDXAry[q] - ( nX - nOldX );
        pDXAry = pTmpAry;
    }

    // draw every single character
    DrawText( nX, nY, pStr, 1 );

    Polygon aPolygon(1);
    Point   aOrigin( nX, nY );
    for( int i = 1; i < nLen ; i++ )
    {
        Point aCharPos( aOrigin.X() + pDXAry[ i - 1 ], aOrigin.Y() );
        aPolygon.SetPoint( aCharPos, 0 );
        aPolygon.Rotate( aOrigin, nFontOrientation_ );
        aCharPos = aPolygon.GetPoint( 0 );
                                                  // when the display's visuals are

        DrawText( aCharPos.X(), aCharPos.Y(), pStr+i, 1 );
    }

    if( pTmpAry )
        delete pTmpAry;
}

// ----------------------------------------------------------------------------
//
// SalGraphics
//
// ----------------------------------------------------------------------------

USHORT
SalGraphics::SetFont( ImplFontSelectData *pEntry )
{
#ifdef USE_PSPRINT
    if( (maGraphicsData.m_pPrinterGfx != NULL) )
    {
        sal_Bool bVertical = pEntry->mbVertical;
        sal_Int32 nID = pEntry->mpFontData ? (sal_Int32)pEntry->mpFontData->mpSysData : 0;

        return maGraphicsData.m_pPrinterGfx->SetFont(
                                                     nID,
                                                     pEntry->mnHeight,
                                                     pEntry->mnWidth,
                                                     pEntry->mnOrientation,
                                                     pEntry->mbVertical
                                                     );
    }
    else
#endif
    {
        maGraphicsData.SetFont( pEntry );
        if( _IsPrinter() || (maGraphicsData.mpServerSideFont != NULL) )
            return SAL_SETFONT_USEDRAWTEXTARRAY;
        return 0;
    }
}

// ----------------------------------------------------------------------------

void
SalGraphics::DrawTextArray( long nX, long nY,
        const xub_Unicode *pStr, USHORT nLen, const long *pDXAry )
{
    maGraphicsData.FaxPhoneComment( pStr, nLen );
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
         maGraphicsData.m_pPrinterGfx->DrawText (Point(nX, nY), pStr, nLen, pDXAry);
    else
#endif
    {
#ifdef USE_BUILTIN_RASTERIZER
        if( maGraphicsData.mpServerSideFont != NULL )
            maGraphicsData.DrawServerFontString( nX, nY, pStr, nLen, pDXAry );
        else
#endif
            maGraphicsData.DrawText( nX, nY, pStr, nLen, pDXAry );
    }
}

// ----------------------------------------------------------------------------

void
SalGraphics::SetTextColor( SalColor nSalColor )
{
    #if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)   // when the display's visuals are

    {
        psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                                  SALCOLOR_GREEN (nSalColor),
                                  SALCOLOR_BLUE  (nSalColor));
        maGraphicsData.m_pPrinterGfx->SetTextColor (aColor);
    }
    else
    {
    #endif

    if( _GetTextColor() != nSalColor )
    {
        _GetTextColor()     = nSalColor;
        _GetTextPixel()     = _GetPixel( nSalColor );
        _IsFontGC()         = FALSE;
    }

    #if defined(USE_PSPRINT)
    }
    #endif
}

// ----------------------------------------------------------------------------

#if defined(USE_PSPRINT)

static FontWidth
ToFontWidth (psp::width::type eWidth)
{
    switch (eWidth)
    {
        case psp::width::UltraCondensed: return WIDTH_ULTRA_CONDENSED;
        case psp::width::ExtraCondensed: return WIDTH_EXTRA_CONDENSED;
        case psp::width::Condensed:      return WIDTH_CONDENSED;
        case psp::width::SemiCondensed:  return WIDTH_SEMI_CONDENSED;
        case psp::width::Normal:         return WIDTH_NORMAL;
        case psp::width::SemiExpanded:   return WIDTH_SEMI_EXPANDED;
        case psp::width::Expanded:       return WIDTH_EXPANDED;
        case psp::width::ExtraExpanded:  return WIDTH_EXTRA_EXPANDED;
        case psp::width::UltraExpanded:  return WIDTH_ULTRA_EXPANDED;
    }
    return WIDTH_DONTKNOW;
}

static FontWeight
ToFontWeight (psp::weight::type eWeight)
{
    switch (eWeight)
    {
        case psp::weight::Thin:       return WEIGHT_THIN;
        case psp::weight::UltraLight: return WEIGHT_ULTRALIGHT;
        case psp::weight::Light:      return WEIGHT_LIGHT;
        case psp::weight::SemiLight:  return WEIGHT_SEMILIGHT;
        case psp::weight::Normal:     return WEIGHT_NORMAL;
        case psp::weight::Medium:     return WEIGHT_MEDIUM;
        case psp::weight::SemiBold:   return WEIGHT_SEMIBOLD;
        case psp::weight::Bold:       return WEIGHT_BOLD;
        case psp::weight::UltraBold:  return WEIGHT_ULTRABOLD;
        case psp::weight::Black:      return WEIGHT_BLACK;
    }
    return WEIGHT_DONTKNOW;
}

static FontPitch
ToFontPitch (psp::pitch::type ePitch)
{
    switch (ePitch)
    {
        case psp::pitch::Fixed:     return PITCH_FIXED;
        case psp::pitch::Variable:  return PITCH_VARIABLE;
    }
    return PITCH_DONTKNOW;
}

static FontItalic
ToFontItalic (psp::italic::type eItalic)
{
    switch (eItalic)
    {
        case psp::italic::Upright:  return ITALIC_NONE;
        case psp::italic::Oblique:  return ITALIC_OBLIQUE;
        case psp::italic::Italic:   return ITALIC_NORMAL;
    }
    return ITALIC_DONTKNOW;
}

static FontFamily
ToFontFamily (psp::family::type eFamily)
{
    switch (eFamily)
    {
        case psp::family::Decorative: return FAMILY_DECORATIVE;
        case psp::family::Modern:     return FAMILY_MODERN;
        case psp::family::Roman:      return FAMILY_ROMAN;
        case psp::family::Script:     return FAMILY_SCRIPT;
        case psp::family::Swiss:      return FAMILY_SWISS;
        case psp::family::System:     return FAMILY_SYSTEM;
    }
    return FAMILY_DONTKNOW;
}

static void SetImplFontData( const psp::FastPrintFontInfo& aInfo, ImplFontData& rData )
{
    rData.meFamily      = ToFontFamily (aInfo.m_eFamilyStyle);
    rData.meWeight      = ToFontWeight (aInfo.m_eWeight);
    rData.meItalic      = ToFontItalic (aInfo.m_eItalic);
    rData.meWidthType   = ToFontWidth  (aInfo.m_eWidth);
    rData.mePitch       = ToFontPitch  (aInfo.m_ePitch);
    rData.meCharSet     = aInfo.m_aEncoding;
    rData.maName        = aInfo.m_aFamilyName;
    rData.meScript      = SCRIPT_DONTKNOW;
    /*rData.maStyleName = XXX */

    rData.mnWidth       = 0;
    rData.mnHeight      = 0;
    rData.mbOrientation = TRUE;
    rData.mnQuality     = aInfo.m_eType == psp::fonttype::Builtin ? 1024 : 0;
    rData.mnVerticalOrientation= 0;
    rData.meType        = TYPE_SCALABLE;
    rData.mbDevice      = aInfo.m_eType == psp::fonttype::Builtin;
}

#endif /* USE_PSPRINT */

// ----------------------------------------------------------------------------

void
SalGraphics::GetDevFontList( ImplDevFontList *pList )
{
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pJobData != NULL)
    {
        ::std::list< psp::fontID > aList;
        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        rMgr.getFontList( aList, maGraphicsData.m_pJobData->m_pParser );

        ::std::list< psp::fontID >::iterator it;
        for (it = aList.begin(); it != aList.end(); ++it)
        {
            psp::FastPrintFontInfo aInfo;
            if (rMgr.getFontFastInfo (*it, aInfo))
            {
                ImplFontData *pFontData = new ImplFontData;
                SetImplFontData( aInfo, *pFontData );
                pFontData->mpSysData = (void*)*it;
                pList->Add( pFontData );
            }
        }
    }
    else
#endif
    {
        XlfdStorage* pFonts = _GetDisplay()->GetXlfdList();

        for ( int nIdx = 0; nIdx < pFonts->GetCount(); nIdx++ )
        {
            ImplFontData *pFontData = new ImplFontData;
            pFonts->Get(nIdx)->ToImplFontData( pFontData );
            pList->Add( pFontData );
        }

        // store data to bootstrap an X fallback font

#ifdef USE_BUILTIN_RASTERIZER
        aX11GlyphPeer.SetDisplay( maGraphicsData.GetXDisplay(),
            maGraphicsData.GetDisplay()->GetVisual()->GetVisual() );
        GlyphCache::EnsureInstance( aX11GlyphPeer, false );
        GlyphCache& rGC = GlyphCache::GetInstance();

        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        ::std::list< psp::fontID > aList;
        ::std::list< psp::fontID >::iterator it;
        rMgr.getFontList( aList );
        for( it = aList.begin(); it != aList.end(); ++it )
        {
            psp::FastPrintFontInfo aInfo;
            if( rMgr.getFontFastInfo( *it, aInfo ) )
            {
                ImplFontData aFontData;
                SetImplFontData( aInfo, aFontData );
                aFontData.mnQuality += 4096;    // prefer to X11 fonts
                ::rtl::OUString aTmpName( rMgr.getFontFileSysPath( aInfo.m_nID ) );
                String aFontFileName( aTmpName.getStr(), aTmpName.getLength() );
                int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
                if( nFaceNum < 0 )
                    nFaceNum = 0;
                rGC.AddFontFile( aFontFileName, nFaceNum, aInfo.m_nID, &aFontData );
            }
        }

        rGC.FetchFontList( pList );

#endif // USE_BUILTIN_RASTERIZER

    }
}

// ----------------------------------------------------------------------------

inline long
sal_DivideNeg( long n1, long n2 )
{
    return ( n1 < 0 ) ? (n1 - n2 / 2) / n2 : (n1 + n2 / 2) / n2;
}

// ----------------------------------------------------------------------------

void
SalGraphics::GetFontMetric( ImplFontMetricData *pMetric )
{
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
    {
        const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        psp::PrintFontInfo aInfo;

        if (rMgr.getFontInfo (maGraphicsData.m_pPrinterGfx->GetFontID(), aInfo))
        {
            sal_Int32 nTextHeight   = maGraphicsData.m_pPrinterGfx->GetFontHeight();
            sal_Int32 nTextWidth    = maGraphicsData.m_pPrinterGfx->GetFontWidth();
            if( ! nTextWidth )
                nTextWidth = nTextHeight;

            pMetric->mnOrientation  = maGraphicsData.m_pPrinterGfx->GetFontAngle();
            pMetric->mnSlant        = 0;
            pMetric->mbDevice       = aInfo.m_eType == psp::fonttype::Builtin ? sal_True : sal_False;

            pMetric->meCharSet      = aInfo.m_aEncoding;

            pMetric->meFamily       = ToFontFamily (aInfo.m_eFamilyStyle);
            pMetric->meWeight       = ToFontWeight (aInfo.m_eWeight);
            pMetric->mePitch        = ToFontPitch  (aInfo.m_ePitch);
            pMetric->meItalic       = ToFontItalic (aInfo.m_eItalic);
            pMetric->meType         = TYPE_SCALABLE;

            pMetric->mnFirstChar    =   0;
            pMetric->mnLastChar     = 255;

            pMetric->mnWidth        = nTextWidth;
            pMetric->mnAscent       = ( aInfo.m_nAscend * nTextHeight + 500 ) / 1000;
            pMetric->mnDescent      = ( aInfo.m_nDescend * nTextHeight + 500 ) / 1000;
            pMetric->mnLeading      = ( aInfo.m_nLeading * nTextHeight + 500 ) / 1000;
        }
    }
    else
    {
#endif
#ifdef USE_BUILTIN_RASTERIZER
        if( maGraphicsData.mpServerSideFont != NULL )
        {
            long rDummyFactor;
            maGraphicsData.mpServerSideFont->FetchFontMetric( *pMetric,
                                                              rDummyFactor );
            return;
        }
#endif //USE_BUILTIN_RASTERIZER


    ExtendedFontStruct* pFont = maGraphicsData.xFont_;

    if ( pFont != NULL )
    {
        pFont->ToImplFontMetricData( pMetric );

#ifndef USE_PSPRINT
        if( XSalCanDrawRotString( maGraphicsData.GetXDisplay(), None ) )
            pMetric->mnOrientation = maGraphicsData.nFontOrientation_;
#endif
        if ( maGraphicsData.bFontVertical_ )
            pMetric->mnOrientation = 0;

        long n;

        n = maGraphicsData.aScale_.GetNumerator();
        if( n != 1 )
        {
            pMetric->mnWidth    *= n;
            pMetric->mnAscent   *= n;
            pMetric->mnDescent  *= n;
            pMetric->mnLeading  *= n;
            pMetric->mnSlant    *= n;
        }

        n = maGraphicsData.aScale_.GetDenominator();
        if( n != 1 )
        {
            pMetric->mnWidth    = Divide( pMetric->mnWidth, n );
            pMetric->mnAscent   = sal_DivideNeg( pMetric->mnAscent,  n );
            pMetric->mnDescent  = sal_DivideNeg( pMetric->mnDescent, n );
            pMetric->mnLeading  = sal_DivideNeg( pMetric->mnLeading, n );
            pMetric->mnSlant    = sal_DivideNeg( pMetric->mnSlant,   n );
        }
    }

#if defined(USE_PSPRINT)
    }
#endif
}

// ---------------------------------------------------------------------------

static long
InitializeWidthArray( long *pWidthArray, sal_Size nItems, int nValue = 0  )
{
    const long nPrecision = 1;

    for ( int i = 0; i < nItems; i++, pWidthArray++ )
        *pWidthArray = nValue;

    return nPrecision;
}

// ---------------------------------------------------------------------------

static int
GetCharWidth( ServerFont *pServerFont, int nChar, long* nWidth )
{
    int nGlyphIndex = pServerFont->GetGlyphIndex( nChar );

    const GlyphMetric& rGM = pServerFont->GetGlyphMetric( nGlyphIndex );
    *nWidth = rGM.GetDelta().X();

    return nGlyphIndex;
}

long
SalGraphics::GetCharWidth( USHORT nChar1, USHORT nChar2, long  *pWidthAry )
{
#if defined(USE_PSPRINT)
    if (maGraphicsData.m_pPrinterGfx != NULL)
        return maGraphicsData.m_pPrinterGfx->GetCharWidth(nChar1, nChar2, pWidthAry);
    else
    {
#endif
#ifdef USE_BUILTIN_RASTERIZER
        if( maGraphicsData.mpServerSideFont != NULL )
        {
            long nWidth;
            for( int i = nChar1; i <= nChar2; ++i )
            {
                if (! ::GetCharWidth (maGraphicsData.mpServerSideFont, i, &nWidth) )
                    if (! ::GetCharWidth (maGraphicsData.mpSrvFallbackFont, i, &nWidth) )
                        ::GetCharWidth (maGraphicsData.mpSrvFallbackFont, '?', &nWidth);

                pWidthAry[ i - nChar1 ] = nWidth;
            }
            return 1;
        }
#endif // USE_BUILTIN_RASTERIZER

    // return the precision of the calculated charwidth, e.g. 1000 = 3 digits
    // defaultet to 1 for now
    const long nPrecision = 1;
    int nRequestedWidth = nChar2 - nChar1 + 1;
    int nCharWidth;

    // XXX sanity check, this may happen if no font at all is installed
    // or no system font matches the requirements for the user interface
    if ( maGraphicsData.xFont_ == NULL )
        return InitializeWidthArray( pWidthAry, nRequestedWidth, 12 );

    // the font should know it's metrics best
    SalDisplay *pSalDisplay = maGraphicsData.GetDisplay();

    nCharWidth = maGraphicsData.xFont_->GetCharWidth( nChar1, nChar2, pWidthAry,                        maGraphicsData.mxFallbackFont );

    // XXX sanity check, this may happen if the font cannot be loaded/queried
    // either because of a garbled fontpath or because of invalid fontfile
    if ( nCharWidth != nRequestedWidth )
        InitializeWidthArray( pWidthAry + nCharWidth,
                nRequestedWidth - nCharWidth );

    // handle internal scaling
    const long nNumerator   = maGraphicsData.aScale_.GetNumerator();
    const long nDenominator = maGraphicsData.aScale_.GetDenominator();
    long *pPtr;
    sal_Unicode nChar;

    if ( nNumerator != 1 )
        for( pPtr = pWidthAry, nChar = nChar1; nChar <= nChar2; nChar++, pPtr++)
            *pPtr *= nNumerator;
    if ( nDenominator != 1 )
        for( pPtr = pWidthAry, nChar = nChar1; nChar <= nChar2; nChar++, pPtr++)
            *pPtr = Divide( *pPtr, nDenominator );

    // return
    return nPrecision;

#if defined(USE_PSPRINT)
    }
#endif
}

// ---------------------------------------------------------------------------

extern unsigned char TranslateCharName( char* );

ULONG
SalGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData *pKernPairs )
{
    if( ! _IsPrinter() )
#ifdef USE_BUILTIN_RASTERIZER
        if( maGraphicsData.mpServerSideFont != NULL )
        {
            ImplKernPairData* pTmpKernPairs;
            ULONG nGotPairs = maGraphicsData.mpServerSideFont->GetKernPairs( &pTmpKernPairs );
            for( int i = 0; i < nPairs && i < nGotPairs; ++i )
                pKernPairs[ i ] = pTmpKernPairs[ i ];
            delete[] pTmpKernPairs;
            return nGotPairs;
        }
#endif //USE_BUILTIN_RASTERIZER

#ifdef USE_PSPRINT
    const ::std::list< ::psp::KernPair >& rPairs( maGraphicsData.m_pPrinterGfx->getKernPairs() );
    ULONG nHavePairs = rPairs.size();
    if( pKernPairs && nPairs )
    {
        ::std::list< ::psp::KernPair >::const_iterator it;
        int i;
        int nTextScale = maGraphicsData.m_pPrinterGfx->GetFontWidth();
        if( ! nTextScale )
            nTextScale = maGraphicsData.m_pPrinterGfx->GetFontHeight();
        for( i = 0, it = rPairs.begin(); i < nPairs && i < nHavePairs; i++, ++it )
        {
            pKernPairs[i].mnChar1   = it->first;
            pKernPairs[i].mnChar2   = it->second;
            pKernPairs[i].mnKern    = it->kern_x * nTextScale / 1000;
        }
    }
    return nHavePairs;
#else
    // get pair kerning table ( internal data from xprinter )
    int i, nCurPair=0;

    // XXX Fix me, improve this to be multi encoding aware: merge kern
    // pair list for all encodings available in the xfont
    rtl_TextEncoding nEncoding = maGraphicsData.xFont_->GetAsciiEncoding();
    XFontStruct *pXFS = maGraphicsData.xFont_->GetFontStruct( nEncoding );
    XExtData    *pXES = pXFS->ext_data;

    for( i = 0; pXES && i < 2; i++, pXES = pXES->next );
    if( i < 2 )
        return 0;
    XpPairKernData* pXpPKD = (XpPairKernData*)(pXES->private_data);
    PairKernData*   pPKD   = pXpPKD->pkd;

    for( i = 0, nCurPair=0; i < pXpPKD->numOfPairs; i++ )
    {
        unsigned char c1 = TranslateCharName( pPKD[i].name1 );
        unsigned char c2 = TranslateCharName( pPKD[i].name2 );
        if( c1 && c2 )
        {
            if( pKernPairs && nCurPair < nPairs )

            {
                pKernPairs[ nCurPair ].mnChar1 = c1;
                pKernPairs[ nCurPair ].mnChar2 = c2;
                pKernPairs[ nCurPair ].mnKern =
                        (long)(pPKD[i].xamt * pXpPKD->pointsize / 1000 );
            }
            nCurPair++;
        }
    }

    return nCurPair;
#endif
    return 0;
}

// ---------------------------------------------------------------------------

BOOL
SalGraphics::GetGlyphBoundRect( xub_Unicode cChar,
        long *pX, long *pY, long *pDX, long *pDY )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        ServerFont& rSF = *maGraphicsData.mpServerSideFont;
        const int nGlyphIndex = rSF.GetGlyphIndex( cChar );
        const GlyphMetric& rGM = rSF.GetGlyphMetric( nGlyphIndex );

        *pX = rGM.GetOffset().X();
        *pY = rGM.GetOffset().Y();
        *pDX = rGM.GetSize().Width();
        *pDY = rGM.GetSize().Height();
        return TRUE;
    }
#endif //USE_BUILTIN_RASTERIZER

    return FALSE;
}

// ---------------------------------------------------------------------------

ULONG
SalGraphics::GetGlyphOutline( xub_Unicode cChar,
        USHORT **ppPolySizes, SalPoint **ppPoints, BYTE **ppFlags )
{
#ifdef USE_BUILTIN_RASTERIZER
    if( maGraphicsData.mpServerSideFont != NULL )
    {
        PolyPolygon rPolyPoly;
        ServerFont& rSF = *maGraphicsData.mpServerSideFont;
        const int nGlyphIndex = rSF.GetGlyphIndex( cChar );
        bool bRet = rSF.GetGlyphOutline( nGlyphIndex, rPolyPoly );
        if( !bRet )
            return 0;

        // translate PolyPolygon into ancient format
        const unsigned nPolygons = rPolyPoly.Count();
        unsigned nPoints = 0;
        unsigned i;
        for( i = 0; i < nPolygons; ++i )
            nPoints += rPolyPoly[ i].GetSize();

        USHORT* const pPolySizes    = new USHORT[ nPolygons ];
        SalPoint* const pPoints     = new SalPoint[ nPoints ];
        BYTE* const pFlags          = new BYTE[ nPoints ];

        *ppPolySizes = pPolySizes;
        *ppPoints   = pPoints;
        *ppFlags    = pFlags;

        unsigned nDestIndex = 0;
        for( i = 0; i < nPolygons; ++i )
        {
            const Polygon& rPolygon = rPolyPoly[ i ];
            const int nPoints = rPolygon.GetSize();
            pPolySizes[ i ] = nPoints;
            for( unsigned j = 0; j < nPoints; ++j )
            {
                const Point& aPoint = rPolygon[ j ];
                pPoints[ nDestIndex ].mnX   = aPoint.X();
                pPoints[ nDestIndex ].mnY   = aPoint.Y();
                pFlags[ nDestIndex ]        = rPolygon.GetFlags( j );
                ++nDestIndex;
            }
        }

        return nPolygons;
    }
#endif // USE_BUILTIN_RASTERIZER

    return 0;
}

// ===========================================================================
