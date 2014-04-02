/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "i18nlangtag/mslangid.hxx"
#include "i18nlangtag/languagetag.hxx"

#include "rtl/tencinfo.h"
#include "rtl/logfile.hxx"

#include "tools/debug.hxx"
#include "tools/poly.hxx"

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"

#include "vcl/metric.hxx"
#include "vcl/metaact.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/virdev.hxx"
#include "vcl/print.hxx"
#include "vcl/event.hxx"
#include "vcl/window.hxx"
#include "vcl/svapp.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/outdev.hxx"
#include "vcl/edit.hxx"
#include <vcl/settings.hxx>
// declare system types in sysdata.hxx
#include <svsys.h>
#include "vcl/sysdata.hxx"
#include "vcl/unohelp.hxx"
#include "vcl/controllayout.hxx"

#include "salgdi.hxx"
#include "sallayout.hxx"
#include "svdata.hxx"
#include "impfont.hxx"
#include "outdata.hxx"
#include "outfont.hxx"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "PhysicalFontFamily.hxx"

#include "textlayout.hxx"
#include "svids.hrc"
#include "window.h"

#include "unotools/fontcvt.hxx"
#include "unotools/fontcfg.hxx"

#include "osl/file.h"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

#include "pdfwriter_impl.hxx"

#include "com/sun/star/beans/PropertyValues.hpp"
#include "com/sun/star/i18n/XBreakIterator.hpp"
#include "com/sun/star/i18n/WordType.hpp"
#include "com/sun/star/linguistic2/LinguServiceManager.hpp"
#include <comphelper/processfactory.hxx>

#if defined UNX
#define GLYPH_FONT_HEIGHT   128
#else
#define GLYPH_FONT_HEIGHT   256
#endif

#include "sal/alloca.h"

#include <cmath>
#include <cstring>

#include <memory>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::vcl;
using namespace ::utl;

#define TEXT_DRAW_ELLIPSIS  (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS)

#define UNDERLINE_LAST      UNDERLINE_BOLDWAVE
#define STRIKEOUT_LAST      STRIKEOUT_X

static void ImplRotatePos( long nOriginX, long nOriginY, long& rX, long& rY,
                           int nOrientation )
{
    if ( (nOrientation >= 0) && !(nOrientation % 900) )
    {
        if ( (nOrientation >= 3600) )
            nOrientation %= 3600;

        if ( nOrientation )
        {
            rX -= nOriginX;
            rY -= nOriginY;

            if ( nOrientation == 900 )
            {
                long nTemp = rX;
                rX = rY;
                rY = -nTemp;
            }
            else if ( nOrientation == 1800 )
            {
                rX = -rX;
                rY = -rY;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = rX;
                rX = -rY;
                rY = nTemp;
            }

            rX += nOriginX;
            rY += nOriginY;
        }
    }
    else
    {
        double nRealOrientation = nOrientation*F_PI1800;
        double nCos = cos( nRealOrientation );
        double nSin = sin( nRealOrientation );

        // Translation...
        long nX = rX-nOriginX;
        long nY = rY-nOriginY;

        // Rotation...
        rX = +((long)(nCos*nX + nSin*nY)) + nOriginX;
        rY = -((long)(nSin*nX - nCos*nY)) + nOriginY;
    }
}

void OutputDevice::ImplClearFontData( const bool bNewFontLists )
{
    // the currently selected logical font is no longer needed
    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = NULL;
    }

    mbInitFont = true;
    mbNewFont = true;

    if ( bNewFontLists )
    {
        if ( mpGetDevFontList )
        {
            delete mpGetDevFontList;
            mpGetDevFontList = NULL;
        }
        if ( mpGetDevSizeList )
        {
            delete mpGetDevSizeList;
            mpGetDevSizeList = NULL;
        }

        // release all physically selected fonts on this device
        if( ImplGetGraphics() )
            mpGraphics->ReleaseFonts();
    }

//    if ( GetOutDevType() == OUTDEV_PRINTER || mpPDFWriter )
    {
        ImplSVData* pSVData = ImplGetSVData();

        if( mpFontCache && mpFontCache != pSVData->maGDIData.mpScreenFontCache )
            mpFontCache->Invalidate();

        if ( bNewFontLists )
        {
            // we need a graphics
            if ( ImplGetGraphics() )
            {
                if( mpFontCollection && mpFontCollection != pSVData->maGDIData.mpScreenFontList )
                    mpFontCollection->Clear();

                if( mpPDFWriter )
                {
                    if( mpFontCollection && mpFontCollection != pSVData->maGDIData.mpScreenFontList )
                        delete mpFontCollection;
                    if( mpFontCache && mpFontCache != pSVData->maGDIData.mpScreenFontCache )
                        delete mpFontCache;
                    mpFontCollection = 0;
                    mpFontCache = 0;
                }
            }
        }
    }

    // also update child windows if needed
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pChild = ((Window*)this)->mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplClearFontData( true );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void OutputDevice::ImplRefreshFontData( const bool bNewFontLists )
{
//    if ( GetOutDevType() == OUTDEV_PRINTER || mpPDFWriter )
    {
        ImplSVData* pSVData = ImplGetSVData();

        if ( bNewFontLists )
        {
            // we need a graphics
            if ( ImplGetGraphics() )
            {
                if( mpPDFWriter )
                {
                    mpFontCollection = pSVData->maGDIData.mpScreenFontList->Clone( true, true );
                    mpFontCache = new ImplFontCache();
                }
                else
                {
                    mpGraphics->GetDevFontList( mpFontCollection );
                }
            }
        }
    }

    // also update child windows if needed
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        Window* pChild = ((Window*)this)->mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplRefreshFontData( true );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void OutputDevice::ImplUpdateFontData( bool bNewFontLists )
{
    ImplClearFontData( bNewFontLists );
    ImplRefreshFontData( bNewFontLists );
}

void OutputDevice::ImplUpdateAllFontData( bool bNewFontLists )
{
    ImplSVData* pSVData = ImplGetSVData();

    ImplUpdateFontDataForAllFrames( &OutputDevice::ImplClearFontData, bNewFontLists );

    // clear global font lists to have them updated
    pSVData->maGDIData.mpScreenFontCache->Invalidate();
    if ( bNewFontLists )
    {
        pSVData->maGDIData.mpScreenFontList->Clear();
        Window * pFrame = pSVData->maWinData.mpFirstFrame;
        if ( pFrame )
        {
            if ( pFrame->ImplGetGraphics() )
            {
                // Stupid typecast here and somewhere ((OutputDevice*)&aVDev)->, because bug in .NET2002 compiler
                OutputDevice *pDevice = (OutputDevice*)pFrame;
                pDevice->mpGraphics->ClearDevFontCache();
                pDevice->mpGraphics->GetDevFontList(pFrame->mpWindowImpl->mpFrameData->mpFontCollection);
            }
        }
    }

    ImplUpdateFontDataForAllFrames( &OutputDevice::ImplRefreshFontData, bNewFontLists );
}

void OutputDevice::ImplUpdateFontDataForAllFrames( const FontUpdateHandler_t pHdl, const bool bNewFontLists )
{
    ImplSVData* const pSVData = ImplGetSVData();

    // update all windows
    Window* pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        ( pFrame->*pHdl )( bNewFontLists );

        Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            ( pSysWin->*pHdl )( bNewFontLists );
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    // update all virtual devices
    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
    while ( pVirDev )
    {
        ( pVirDev->*pHdl )( bNewFontLists );
        pVirDev = pVirDev->mpNext;
    }

    // update all printers
    Printer* pPrinter = pSVData->maGDIData.mpFirstPrinter;
    while ( pPrinter )
    {
        ( pPrinter->*pHdl )( bNewFontLists );
        pPrinter = pPrinter->mpNext;
    }
}

void OutputDevice::BeginFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maGDIData.mbFontSubChanged = false;
}

void OutputDevice::EndFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maGDIData.mbFontSubChanged )
    {
        ImplUpdateAllFontData( false );

        Application* pApp = GetpApp();
        DataChangedEvent aDCEvt( DATACHANGED_FONTSUBSTITUTION );
        pApp->DataChanged( aDCEvt );
        pApp->NotifyAllWindows( aDCEvt );
        pSVData->maGDIData.mbFontSubChanged = false;
    }
}

void OutputDevice::AddFontSubstitute( const OUString& rFontName,
                                      const OUString& rReplaceFontName,
                                      sal_uInt16 nFlags )
{
    ImplDirectFontSubstitution*& rpSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( !rpSubst )
        rpSubst = new ImplDirectFontSubstitution();
    rpSubst->AddFontSubstitute( rFontName, rReplaceFontName, nFlags );
    ImplGetSVData()->maGDIData.mbFontSubChanged = true;
}

void ImplDirectFontSubstitution::AddFontSubstitute( const OUString& rFontName,
    const OUString& rSubstFontName, sal_uInt16 nFlags )
{
    maFontSubstList.push_back( ImplFontSubstEntry( rFontName, rSubstFontName, nFlags ) );
}

ImplFontSubstEntry::ImplFontSubstEntry( const OUString& rFontName,
    const OUString& rSubstFontName, sal_uInt16 nSubstFlags )
:   maName( rFontName )
,   maReplaceName( rSubstFontName )
,   mnFlags( nSubstFlags )
{
    maSearchName        = rFontName;
    maSearchReplaceName = rSubstFontName;
    GetEnglishSearchFontName( maSearchName );
    GetEnglishSearchFontName( maSearchReplaceName );
}

void OutputDevice::RemoveFontSubstitute( sal_uInt16 n )
{
    ImplDirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( pSubst )
        pSubst->RemoveFontSubstitute( n );
}

void ImplDirectFontSubstitution::RemoveFontSubstitute( int nIndex )
{
    FontSubstList::iterator it = maFontSubstList.begin();
    for( int nCount = 0; (it != maFontSubstList.end()) && (nCount++ != nIndex); ++it ) ;
    if( it != maFontSubstList.end() )
        maFontSubstList.erase( it );
}

sal_uInt16 OutputDevice::GetFontSubstituteCount()
{
    const ImplDirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( !pSubst )
        return 0;
    int nCount =  pSubst->GetFontSubstituteCount();
    return (sal_uInt16)nCount;
}

bool ImplDirectFontSubstitution::FindFontSubstitute( OUString& rSubstName,
    const OUString& rSearchName, sal_uInt16 nFlags ) const
{
    // TODO: get rid of O(N) searches
    FontSubstList::const_iterator it = maFontSubstList.begin();
    for(; it != maFontSubstList.end(); ++it )
    {
        const ImplFontSubstEntry& rEntry = *it;
        if( ((rEntry.mnFlags & nFlags) || !nFlags)
        &&   (rEntry.maSearchName == rSearchName) )
        {
            rSubstName = rEntry.maSearchReplaceName;
            return true;
        }
    }

    return false;
}

void ImplFontSubstitute( OUString& rFontName )
{
#ifdef DBG_UTIL
    OUString aTempName = rFontName;
    GetEnglishSearchFontName( aTempName );
    DBG_ASSERT( aTempName == rFontName, "ImplFontSubstitute() called without a searchname" );
#endif

    OUString aSubstFontName;

    // apply user-configurable font replacement (eg, from the list in Tools->Options)
    const ImplDirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( pSubst && pSubst->FindFontSubstitute( aSubstFontName, rFontName, FONT_SUBSTITUTE_ALWAYS ) )
    {
        rFontName = aSubstFontName;
        return;
    }
}

//hidpi TODO: This routine has hard-coded font-sizes that break places such as DialControl
Font OutputDevice::GetDefaultFont( sal_uInt16 nType, LanguageType eLang,
                                   sal_uLong nFlags, const OutputDevice* pOutDev )
{
    if (!pOutDev) // default is NULL
        pOutDev = Application::GetDefaultDevice();

    LanguageTag aLanguageTag(
            ( eLang == LANGUAGE_NONE || eLang == LANGUAGE_SYSTEM || eLang == LANGUAGE_DONTKNOW ) ?
            Application::GetSettings().GetUILanguageTag() :
            LanguageTag( eLang ));

    utl::DefaultFontConfiguration& rDefaults = utl::DefaultFontConfiguration::get();
    OUString aDefault = rDefaults.getDefaultFont( aLanguageTag, nType );
    OUString aSearch;

    if( !aDefault.isEmpty() )
        aSearch = aDefault;
    else
        aSearch = rDefaults.getUserInterfaceFont( aLanguageTag ); // use the UI font as a fallback

    Font aFont;
    aFont.SetPitch( PITCH_VARIABLE );

    switch ( nType )
    {
        case DEFAULTFONT_SANS_UNICODE:
        case DEFAULTFONT_UI_SANS:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DEFAULTFONT_SANS:
        case DEFAULTFONT_LATIN_HEADING:
        case DEFAULTFONT_LATIN_SPREADSHEET:
        case DEFAULTFONT_LATIN_DISPLAY:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DEFAULTFONT_SERIF:
        case DEFAULTFONT_LATIN_TEXT:
        case DEFAULTFONT_LATIN_PRESENTATION:
            aFont.SetFamily( FAMILY_ROMAN );
            break;

        case DEFAULTFONT_FIXED:
        case DEFAULTFONT_LATIN_FIXED:
        case DEFAULTFONT_UI_FIXED:
            aFont.SetPitch( PITCH_FIXED );
            aFont.SetFamily( FAMILY_MODERN );
            break;

        case DEFAULTFONT_SYMBOL:
            aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
            break;

        case DEFAULTFONT_CJK_TEXT:
        case DEFAULTFONT_CJK_PRESENTATION:
        case DEFAULTFONT_CJK_SPREADSHEET:
        case DEFAULTFONT_CJK_HEADING:
        case DEFAULTFONT_CJK_DISPLAY:
            aFont.SetFamily( FAMILY_SYSTEM ); // don't care, but don't use font subst config later...
            break;

        case DEFAULTFONT_CTL_TEXT:
        case DEFAULTFONT_CTL_PRESENTATION:
        case DEFAULTFONT_CTL_SPREADSHEET:
        case DEFAULTFONT_CTL_HEADING:
        case DEFAULTFONT_CTL_DISPLAY:
            aFont.SetFamily( FAMILY_SYSTEM ); // don't care, but don't use font subst config later...
            break;
    }

    if ( !aSearch.isEmpty() )
    {
        aFont.SetHeight( 12 ); // corresponds to nDefaultHeight
        aFont.SetWeight( WEIGHT_NORMAL );
        aFont.SetLanguage( eLang );

        if ( aFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW )
            aFont.SetCharSet( osl_getThreadTextEncoding() );

        // Should we only return available fonts on the given device
        if ( pOutDev )
        {
            pOutDev->ImplInitFontList();

            // Search Font in the FontList
            OUString      aName;
            OUString      aSearchName;
            sal_Int32     nIndex = 0;
            do
            {
                aSearchName = GetNextFontToken( aSearch, nIndex );
                GetEnglishSearchFontName( aSearchName );
                PhysicalFontFamily* pFontFamily = pOutDev->mpFontCollection->ImplFindBySearchName( aSearchName );
                if( pFontFamily )
                {
                    AddTokenFontName( aName, pFontFamily->GetFamilyName() );
                    if( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
                        break;
                }
            }
            while ( nIndex != -1 );
            aFont.SetName( aName );
        }

        // No Name, than set all names
        if ( aFont.GetName().isEmpty() )
        {
            if ( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
            {
                if( !pOutDev )
                {
                    SAL_WARN ("vcl.gdi", "No default window has been set for the application - we really shouldn't be able to get here");
                    sal_Int32 nIndex = 0;
                    aFont.SetName( aSearch.getToken( 0, ';', nIndex ) );
                }
                else
                {
                    pOutDev->ImplInitFontList();

                    aFont.SetName( aSearch );

                    // convert to pixel height
                    Size aSize = pOutDev->ImplLogicToDevicePixel( aFont.GetSize() );
                    if ( !aSize.Height() )
                    {
                        // use default pixel height only when logical height is zero
                        if ( aFont.GetHeight() )
                            aSize.Height() = 1;
                        else
                            aSize.Height() = (12*pOutDev->mnDPIY)/72;
                    }

                    // use default width only when logical width is zero
                    if( (0 == aSize.Width()) && (0 != aFont.GetSize().Width()) )
                        aSize.Width() = 1;

                    // get the name of the first available font
                    float fExactHeight = static_cast<float>(aSize.Height());
                    ImplFontEntry* pEntry = pOutDev->mpFontCache->GetFontEntry( pOutDev->mpFontCollection, aFont, aSize, fExactHeight );
                    if (pEntry)
                    {
                        if( pEntry->maFontSelData.mpFontData )
                            aFont.SetName( pEntry->maFontSelData.mpFontData->GetFamilyName() );
                        else
                            aFont.SetName( pEntry->maFontSelData.maTargetName );
                    }
                }
            }
            else
                aFont.SetName( aSearch );
        }
    }

#if OSL_DEBUG_LEVEL > 2
    const char* s = "DEFAULTFONT_SANS_UNKNOWN";
    switch ( nType )
    {
    case DEFAULTFONT_SANS_UNICODE:  s = "DEFAULTFONT_SANS_UNICODE"; break;
    case DEFAULTFONT_UI_SANS:   s = "DEFAULTFONT_UI_SANS"; break;

    case DEFAULTFONT_SANS:  s = "DEFAULTFONT_SANS"; break;
    case DEFAULTFONT_LATIN_HEADING: s = "DEFAULTFONT_LATIN_HEADING"; break;
    case DEFAULTFONT_LATIN_SPREADSHEET: s = "DEFAULTFONT_LATIN_SPREADSHEET"; break;
    case DEFAULTFONT_LATIN_DISPLAY: s = "DEFAULTFONT_LATIN_DISPLAY"; break;

    case DEFAULTFONT_SERIF: s = "DEFAULTFONT_SERIF"; break;
    case DEFAULTFONT_LATIN_TEXT:    s = "DEFAULTFONT_LATIN_TEXT"; break;
    case DEFAULTFONT_LATIN_PRESENTATION:    s = "DEFAULTFONT_LATIN_PRESENTATION"; break;

    case DEFAULTFONT_FIXED: s = "DEFAULTFONT_FIXED"; break;
    case DEFAULTFONT_LATIN_FIXED:   s = "DEFAULTFONT_LATIN_FIXED"; break;
    case DEFAULTFONT_UI_FIXED:  s = "DEFAULTFONT_UI_FIXED"; break;

    case DEFAULTFONT_SYMBOL:    s = "DEFAULTFONT_SYMBOL"; break;

    case DEFAULTFONT_CJK_TEXT:  s = "DEFAULTFONT_CJK_TEXT"; break;
    case DEFAULTFONT_CJK_PRESENTATION:  s = "DEFAULTFONT_CJK_PRESENTATION"; break;
    case DEFAULTFONT_CJK_SPREADSHEET:   s = "DEFAULTFONT_CJK_SPREADSHEET"; break;
    case DEFAULTFONT_CJK_HEADING:   s = "DEFAULTFONT_CJK_HEADING"; break;
    case DEFAULTFONT_CJK_DISPLAY:   s = "DEFAULTFONT_CJK_DISPLAY"; break;

    case DEFAULTFONT_CTL_TEXT:  s = "DEFAULTFONT_CTL_TEXT"; break;
    case DEFAULTFONT_CTL_PRESENTATION:  s = "DEFAULTFONT_CTL_PRESENTATION"; break;
    case DEFAULTFONT_CTL_SPREADSHEET:   s = "DEFAULTFONT_CTL_SPREADSHEET"; break;
    case DEFAULTFONT_CTL_HEADING:   s = "DEFAULTFONT_CTL_HEADING"; break;
    case DEFAULTFONT_CTL_DISPLAY:   s = "DEFAULTFONT_CTL_DISPLAY"; break;
    }
    fprintf( stderr, "   OutputDevice::GetDefaultFont() Type=\"%s\" lang=%d flags=%ld FontName=\"%s\"\n",
         s, eLang, nFlags,
         OUStringToOString( aFont.GetName(), RTL_TEXTENCODING_UTF8 ).getStr()
         );
#endif

    return aFont;
}

ImplFontEntry::ImplFontEntry( const FontSelectPattern& rFontSelData )
    : maFontSelData( rFontSelData )
    , maMetric( rFontSelData )
    , mpConversion( NULL )
    , mnLineHeight( 0 )
    , mnRefCount( 1 )
    , mnSetFontFlags( 0 )
    , mnOwnOrientation( 0 )
    , mnOrientation( 0 )
    , mbInit( false )
    , mpUnicodeFallbackList( NULL )
{
    maFontSelData.mpFontEntry = this;
}

ImplFontEntry::~ImplFontEntry()
{
    delete mpUnicodeFallbackList;
}

size_t ImplFontEntry::GFBCacheKey_Hash::operator()( const GFBCacheKey& rData ) const
{
    boost::hash<sal_UCS4> a;
    boost::hash<int > b;
    return a(rData.first) ^ b(rData.second);
}

void ImplFontEntry::AddFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName )
{
    if( !mpUnicodeFallbackList )
        mpUnicodeFallbackList = new UnicodeFallbackList;
    (*mpUnicodeFallbackList)[ GFBCacheKey(cChar,eWeight) ] = rFontName;
}

bool ImplFontEntry::GetFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, OUString* pFontName ) const
{
    if( !mpUnicodeFallbackList )
        return false;

    UnicodeFallbackList::const_iterator it = mpUnicodeFallbackList->find( GFBCacheKey(cChar,eWeight) );
    if( it == mpUnicodeFallbackList->end() )
        return false;

    *pFontName = (*it).second;
    return true;
}

void ImplFontEntry::IgnoreFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const OUString& rFontName )
{
//  DBG_ASSERT( mpUnicodeFallbackList, "ImplFontEntry::IgnoreFallbackForUnicode no list" );
    UnicodeFallbackList::iterator it = mpUnicodeFallbackList->find( GFBCacheKey(cChar,eWeight) );
//  DBG_ASSERT( it != mpUnicodeFallbackList->end(), "ImplFontEntry::IgnoreFallbackForUnicode no match" );
    if( it == mpUnicodeFallbackList->end() )
        return;
    if( (*it).second == rFontName )
        mpUnicodeFallbackList->erase( it );
}

FontSelectPatternAttributes::FontSelectPatternAttributes( const Font& rFont,
    const OUString& rSearchName, const Size& rSize, float fExactHeight )
    : maSearchName( rSearchName )
    , mnWidth( rSize.Width() )
    , mnHeight( rSize.Height() )
    , mfExactHeight( fExactHeight)
    , mnOrientation( rFont.GetOrientation() )
    , meLanguage( rFont.GetLanguage() )
    , mbVertical( rFont.IsVertical() )
    , mbNonAntialiased( false )
    , mbEmbolden( false )
{
    maTargetName = GetFamilyName();

    rFont.GetFontAttributes( *this );

    // normalize orientation between 0 and 3600
    if( 3600 <= (unsigned)mnOrientation )
    {
        if( mnOrientation >= 0 )
            mnOrientation %= 3600;
        else
            mnOrientation = 3600 - (-mnOrientation % 3600);
    }

    // normalize width and height
    if( mnHeight < 0 )
        mnHeight = -mnHeight;
    if( mnWidth < 0 )
        mnWidth = -mnWidth;
}

FontSelectPattern::FontSelectPattern( const Font& rFont,
    const OUString& rSearchName, const Size& rSize, float fExactHeight)
    : FontSelectPatternAttributes(rFont, rSearchName, rSize, fExactHeight)
    , mpFontData( NULL )
    , mpFontEntry( NULL )
{
}

// NOTE: this ctor is still used on Windows. Do not remove.
#ifdef WNT
FontSelectPatternAttributes::FontSelectPatternAttributes( const PhysicalFontFace& rFontData,
    const Size& rSize, float fExactHeight, int nOrientation, bool bVertical )
    : ImplFontAttributes( rFontData )
    , mnWidth( rSize.Width() )
    , mnHeight( rSize.Height() )
    , mfExactHeight( fExactHeight )
    , mnOrientation( nOrientation )
    , meLanguage( 0 )
    , mbVertical( bVertical )
    , mbNonAntialiased( false )
    , mbEmbolden( false )
{
    maTargetName = maSearchName = GetFamilyName();
    // NOTE: no normalization for width/height/orientation
}

FontSelectPattern::FontSelectPattern( const PhysicalFontFace& rFontData,
    const Size& rSize, float fExactHeight, int nOrientation, bool bVertical )
    : FontSelectPatternAttributes(rFontData, rSize, fExactHeight, nOrientation, bVertical)
    , mpFontData( &rFontData )
    , mpFontEntry( NULL )
{
}
#endif

void FontSelectPattern::copyAttributes(const FontSelectPatternAttributes &rAttributes)
{
    static_cast<FontSelectPatternAttributes&>(*this) = rAttributes;
}

size_t ImplFontCache::IFSD_Hash::operator()( const FontSelectPattern& rFSD ) const
{
    return rFSD.hashCode();
}

size_t FontSelectPatternAttributes::hashCode() const
{
    // TODO: does it pay off to improve this hash function?
    static FontNameHash aFontNameHash;
    size_t nHash = aFontNameHash( maSearchName );
#if ENABLE_GRAPHITE
    // check for features and generate a unique hash if necessary
    if (maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
        != -1)
    {
        nHash = aFontNameHash( maTargetName );
    }
#endif
    nHash += 11 * mnHeight;
    nHash += 19 * GetWeight();
    nHash += 29 * GetSlant();
    nHash += 37 * mnOrientation;
    nHash += 41 * meLanguage;
    if( mbVertical )
        nHash += 53;
    return nHash;
}

bool FontSelectPatternAttributes::operator==(const FontSelectPatternAttributes& rOther) const
{
    if (static_cast<const ImplFontAttributes&>(*this) != static_cast<const ImplFontAttributes&>(rOther))
        return false;

    if (maTargetName != rOther.maTargetName)
        return false;

    if (maSearchName != rOther.maSearchName)
        return false;

    if (mnWidth != rOther.mnWidth)
        return false;

    if (mnHeight != rOther.mnHeight)
        return false;

    if (mfExactHeight != rOther.mfExactHeight)
        return false;

    if (mnOrientation != rOther.mnOrientation)
        return false;

    if (meLanguage != rOther.meLanguage)
        return false;

    if (mbVertical != rOther.mbVertical)
        return false;

    if (mbNonAntialiased != rOther.mbNonAntialiased)
        return false;

    if (mbEmbolden != rOther.mbEmbolden)
        return false;

    if (maItalicMatrix != rOther.maItalicMatrix)
        return false;

    return true;
}

bool ImplFontCache::IFSD_Equal::operator()(const FontSelectPattern& rA, const FontSelectPattern& rB) const
{
    // check normalized font family name
    if( rA.maSearchName != rB.maSearchName )
        return false;

    // check font transformation
    if( (rA.mnHeight       != rB.mnHeight)
    ||  (rA.mnWidth        != rB.mnWidth)
    ||  (rA.mnOrientation  != rB.mnOrientation) )
        return false;

    // check mapping relevant attributes
    if( (rA.mbVertical     != rB.mbVertical)
    ||  (rA.meLanguage     != rB.meLanguage) )
        return false;

    // check font face attributes
    if( (rA.GetWeight()       != rB.GetWeight())
    ||  (rA.GetSlant()       != rB.GetSlant())
//  ||  (rA.meFamily       != rB.meFamily) // TODO: remove this mostly obsolete member
    ||  (rA.GetPitch()     != rB.GetPitch()) )
        return false;

    // check style name
    if( rA.GetStyleName() != rB.GetStyleName() )
        return false;

    // Symbol fonts may recode from one type to another So they are only
    // safely equivalent for equal targets
    if (
        (rA.mpFontData && rA.mpFontData->IsSymbolFont()) ||
        (rB.mpFontData && rB.mpFontData->IsSymbolFont())
       )
    {
        if (rA.maTargetName != rB.maTargetName)
            return false;
    }

#if ENABLE_GRAPHITE
    // check for features
    if ((rA.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
         != -1 ||
         rB.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
         != -1) && rA.maTargetName != rB.maTargetName)
        return false;
#endif

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

ImplFontCache::ImplFontCache()
:   mpFirstEntry( NULL ),
    mnRef0Count( 0 )
{}

ImplFontCache::~ImplFontCache()
{
    FontInstanceList::iterator it = maFontInstanceList.begin();
    for(; it != maFontInstanceList.end(); ++it )
    {
        ImplFontEntry* pEntry = (*it).second;
        delete pEntry;
    }
}

ImplFontEntry* ImplFontCache::GetFontEntry( PhysicalFontCollection* pFontList,
    const Font& rFont, const Size& rSize, float fExactHeight )
{
    OUString aSearchName = rFont.GetName();

    // initialize internal font request object
    FontSelectPattern aFontSelData( rFont, aSearchName, rSize, fExactHeight );
    return GetFontEntry( pFontList, aFontSelData );
}

ImplFontEntry* ImplFontCache::GetFontEntry( PhysicalFontCollection* pFontList,
    FontSelectPattern& aFontSelData )
{
    // check if a directly matching logical font instance is already cached,
    // the most recently used font usually has a hit rate of >50%
    ImplFontEntry *pEntry = NULL;
    PhysicalFontFamily* pFontFamily = NULL;
    IFSD_Equal aIFSD_Equal;
    if( mpFirstEntry && aIFSD_Equal( aFontSelData, mpFirstEntry->maFontSelData ) )
        pEntry = mpFirstEntry;
    else
    {
        FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
        if( it != maFontInstanceList.end() )
            pEntry = (*it).second;
    }

    if( !pEntry ) // no direct cache hit
    {
        // find the best matching logical font family and update font selector accordingly
        pFontFamily = pFontList->ImplFindByFont( aFontSelData );
        DBG_ASSERT( (pFontFamily != NULL), "ImplFontCache::Get() No logical font found!" );
        if( pFontFamily )
            aFontSelData.maSearchName = pFontFamily->GetSearchName();

        // check if an indirectly matching logical font instance is already cached
        FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
        if( it != maFontInstanceList.end() )
        {
            // we have an indirect cache hit
            pEntry = (*it).second;
        }
    }

    PhysicalFontFace* pFontData = NULL;

    if (!pEntry && pFontFamily)// no cache hit => find the best matching physical font face
    {
        bool bOrigWasSymbol = aFontSelData.mpFontData && aFontSelData.mpFontData->IsSymbolFont();
        pFontData = pFontFamily->FindBestFontFace( aFontSelData );
        aFontSelData.mpFontData = pFontData;
        bool bNewIsSymbol = aFontSelData.mpFontData && aFontSelData.mpFontData->IsSymbolFont();

        if (bNewIsSymbol != bOrigWasSymbol)
        {
            // it is possible, though generally unlikely, that at this point we
            // will attempt to use a symbol font as a last-ditch fallback for a
            // non-symbol font request or vice versa, and by changing
            // aFontSelData.mpFontData to/from a symbol font we may now find
            // something in the cache that can be reused which previously
            // wasn't a candidate
            FontInstanceList::iterator it = maFontInstanceList.find( aFontSelData );
            if( it != maFontInstanceList.end() )
                pEntry = (*it).second;
        }
    }

    if( pEntry ) // cache hit => use existing font instance
    {
        // increase the font instance's reference count
        if( !pEntry->mnRefCount++ )
            --mnRef0Count;
    }

    if (!pEntry && pFontData)// still no cache hit => create a new font instance
    {
        // create a new logical font instance from this physical font face
        pEntry = pFontData->CreateFontInstance( aFontSelData );

        // if we're subtituting from or to a symbol font we may need a symbol
        // conversion table
        if( pFontData->IsSymbolFont() || aFontSelData.IsSymbolFont() )
        {
            if( aFontSelData.maTargetName != aFontSelData.maSearchName )
                pEntry->mpConversion = ConvertChar::GetRecodeData( aFontSelData.maTargetName, aFontSelData.maSearchName );
        }

#ifdef MACOSX
        //It might be better to dig out the font version of the target font
        //to see if it's a modern re-coded apple symbol font in case that
        //font shows up on a different platform
        if (!pEntry->mpConversion &&
            aFontSelData.maTargetName.equalsIgnoreAsciiCase("symbol") &&
            aFontSelData.maSearchName.equalsIgnoreAsciiCase("symbol"))
        {
            pEntry->mpConversion = ConvertChar::GetRecodeData( OUString("Symbol"), OUString("AppleSymbol") );
        }
#endif

        // add the new entry to the cache
        maFontInstanceList[ aFontSelData ] = pEntry;
    }

    mpFirstEntry = pEntry;
    return pEntry;
}

ImplFontEntry* ImplFontCache::GetGlyphFallbackFont( PhysicalFontCollection* pFontCollection,
    FontSelectPattern& rFontSelData, int nFallbackLevel, OUString& rMissingCodes )
{
    // get a candidate font for glyph fallback
    // unless the previously selected font got a device specific substitution
    // e.g. PsPrint Arial->Helvetica for udiaeresis when Helvetica doesn't support it
    if( nFallbackLevel >= 1)
    {
        PhysicalFontFamily* pFallbackData = NULL;

        //fdo#33898 If someone has EUDC installed then they really want that to
        //be used as the first-choice glyph fallback seeing as it's filled with
        //private area codes with don't make any sense in any other font so
        //prioritise it here if it's available. Ideally we would remove from
        //rMissingCodes all the glyphs which it is able to resolve as an
        //optimization, but that's tricky to achieve cross-platform without
        //sufficient heavy-weight code that's likely to undo the value of the
        //optimization
        if (nFallbackLevel == 1)
            pFallbackData = pFontCollection->FindFontFamily(OUString("EUDC"));
        if (!pFallbackData)
            pFallbackData = pFontCollection->GetGlyphFallbackFont(rFontSelData, rMissingCodes, nFallbackLevel-1);
        // escape when there are no font candidates
        if( !pFallbackData  )
            return NULL;
        // override the font name
        rFontSelData.SetFamilyName( pFallbackData->GetFamilyName() );
        // clear the cached normalized name
        rFontSelData.maSearchName = "";
    }

    ImplFontEntry* pFallbackFont = GetFontEntry( pFontCollection, rFontSelData );
    return pFallbackFont;
}

void ImplFontCache::Release( ImplFontEntry* pEntry )
{
    static const int FONTCACHE_MAX = 50;

    DBG_ASSERT( (pEntry->mnRefCount > 0), "ImplFontCache::Release() - font refcount underflow" );
    if( --pEntry->mnRefCount > 0 )
        return;

    if( ++mnRef0Count < FONTCACHE_MAX )
        return;

    // remove unused entries from font instance cache
    FontInstanceList::iterator it_next = maFontInstanceList.begin();
    while( it_next != maFontInstanceList.end() )
    {
        FontInstanceList::iterator it = it_next++;
        ImplFontEntry* pFontEntry = (*it).second;
        if( pFontEntry->mnRefCount > 0 )
            continue;

        maFontInstanceList.erase( it );
        delete pFontEntry;
        --mnRef0Count;
        DBG_ASSERT( (mnRef0Count>=0), "ImplFontCache::Release() - refcount0 underflow" );

        if( mpFirstEntry == pFontEntry )
            mpFirstEntry = NULL;
    }

    DBG_ASSERT( (mnRef0Count==0), "ImplFontCache::Release() - refcount0 mismatch" );
}

void ImplFontCache::Invalidate()
{
    // delete unreferenced entries
    FontInstanceList::iterator it = maFontInstanceList.begin();
    for(; it != maFontInstanceList.end(); ++it )
    {
        ImplFontEntry* pFontEntry = (*it).second;
        if( pFontEntry->mnRefCount > 0 )
            continue;

        delete pFontEntry;
        --mnRef0Count;
    }

    // #112304# make sure the font cache is really clean
    mpFirstEntry = NULL;
    maFontInstanceList.clear();

    DBG_ASSERT( (mnRef0Count==0), "ImplFontCache::Invalidate() - mnRef0Count non-zero" );
}

ImplMultiTextLineInfo::ImplMultiTextLineInfo()
{
    mpLines = new PImplTextLineInfo[MULTITEXTLINEINFO_RESIZE];
    mnLines = 0;
    mnSize  = MULTITEXTLINEINFO_RESIZE;
}

ImplMultiTextLineInfo::~ImplMultiTextLineInfo()
{
    for( sal_Int32 i = 0; i < mnLines; i++ )
        delete mpLines[i];
    delete [] mpLines;
}

void ImplMultiTextLineInfo::AddLine( ImplTextLineInfo* pLine )
{
    if ( mnSize == mnLines )
    {
        mnSize += MULTITEXTLINEINFO_RESIZE;
        PImplTextLineInfo* pNewLines = new PImplTextLineInfo[mnSize];
        memcpy( pNewLines, mpLines, mnLines*sizeof(PImplTextLineInfo) );
        mpLines = pNewLines;
    }

    mpLines[mnLines] = pLine;
    mnLines++;
}

void ImplMultiTextLineInfo::Clear()
{
    for( sal_Int32 i = 0; i < mnLines; i++ )
        delete mpLines[i];
    mnLines = 0;
}

FontEmphasisMark OutputDevice::ImplGetEmphasisMarkStyle( const Font& rFont )
{
    FontEmphasisMark nEmphasisMark = rFont.GetEmphasisMark();

    // If no Position is set, then calculate the default position, which
    // depends on the language
    if ( !(nEmphasisMark & (EMPHASISMARK_POS_ABOVE | EMPHASISMARK_POS_BELOW)) )
    {
        LanguageType eLang = rFont.GetLanguage();
        // In Chinese Simplified the EmphasisMarks are below/left
        if (MsLangId::isSimplifiedChinese(eLang))
            nEmphasisMark |= EMPHASISMARK_POS_BELOW;
        else
        {
            eLang = rFont.GetCJKContextLanguage();
            // In Chinese Simplified the EmphasisMarks are below/left
            if (MsLangId::isSimplifiedChinese(eLang))
                nEmphasisMark |= EMPHASISMARK_POS_BELOW;
            else
                nEmphasisMark |= EMPHASISMARK_POS_ABOVE;
        }
    }

    return nEmphasisMark;
}

bool OutputDevice::ImplIsUnderlineAbove( const Font& rFont )
{
    if ( !rFont.IsVertical() )
        return false;

    if( (LANGUAGE_JAPANESE == rFont.GetLanguage())
    ||  (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()) )
        // the underline is right for Japanese only
        return true;

    return false;
}

void OutputDevice::ImplInitFontList() const
{
    if( !mpFontCollection->Count() )
    {
        if( mpGraphics || ImplGetGraphics() )
        {
            SAL_INFO( "vcl.gdi", "OutputDevice::ImplInitFontList()" );
            mpGraphics->GetDevFontList( mpFontCollection );

            // There is absolutely no way there should be no fonts available on the device
            if( !mpFontCollection->Count() )
            {
                OUString aError( "Application error: no fonts and no vcl resource found on your system" );
                ResMgr* pMgr = ImplGetResMgr();
                if( pMgr )
                {
                    OUString aResStr(ResId(SV_ACCESSERROR_NO_FONTS, *pMgr).toString());
                    if( !aResStr.isEmpty() )
                        aError = aResStr;
                }
                Application::Abort( aError );
            }
        }
    }
}

void OutputDevice::ImplInitFont() const
{
    DBG_TESTSOLARMUTEX();

    if (!mpFontEntry)
        return;

    if ( mbInitFont )
    {
        if ( meOutDevType != OUTDEV_PRINTER )
        {
            // decide if antialiasing is appropriate
            bool bNonAntialiased = (GetAntialiasing() & ANTIALIASING_DISABLE_TEXT) != 0;
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            bNonAntialiased |= ((rStyleSettings.GetDisplayOptions() & DISPLAY_OPTION_AA_DISABLE) != 0);
            bNonAntialiased |= (int(rStyleSettings.GetAntialiasingMinPixelHeight()) > mpFontEntry->maFontSelData.mnHeight);
            mpFontEntry->maFontSelData.mbNonAntialiased = bNonAntialiased;
        }

        // select font in the device layers
        mpFontEntry->mnSetFontFlags = mpGraphics->SetFont( &(mpFontEntry->maFontSelData), 0 );
        mbInitFont = false;
    }
}

void OutputDevice::ImplInitTextColor()
{
    DBG_TESTSOLARMUTEX();

    if ( mbInitTextColor )
    {
        mpGraphics->SetTextColor( ImplColorToSal( GetTextColor() ) );
        mbInitTextColor = false;
    }
}

bool OutputDevice::ImplNewFont() const
{
    DBG_TESTSOLARMUTEX();

    // get correct font list on the PDF writer if necessary
    if( mpPDFWriter )
    {
        const ImplSVData* pSVData = ImplGetSVData();
        if( mpFontCollection == pSVData->maGDIData.mpScreenFontList
        ||  mpFontCache == pSVData->maGDIData.mpScreenFontCache )
            const_cast<OutputDevice&>(*this).ImplUpdateFontData( true );
    }

    if ( !mbNewFont )
        return true;

    // we need a graphics
    if ( !mpGraphics && !ImplGetGraphics() )
        return false;
    SalGraphics* pGraphics = mpGraphics;
    ImplInitFontList();

    // convert to pixel height
    // TODO: replace integer based aSize completely with subpixel accurate type
    float fExactHeight = ImplFloatLogicHeightToDevicePixel( static_cast<float>(maFont.GetHeight()) );
    Size aSize = ImplLogicToDevicePixel( maFont.GetSize() );
    if ( !aSize.Height() )
    {
        // use default pixel height only when logical height is zero
        if ( maFont.GetSize().Height() )
            aSize.Height() = 1;
        else
            aSize.Height() = (12*mnDPIY)/72;
        fExactHeight =  static_cast<float>(aSize.Height());
    }

    // select the default width only when logical width is zero
    if( (0 == aSize.Width()) && (0 != maFont.GetSize().Width()) )
        aSize.Width() = 1;

    // get font entry
    ImplFontEntry* pOldEntry = mpFontEntry;
    mpFontEntry = mpFontCache->GetFontEntry( mpFontCollection, maFont, aSize, fExactHeight );
    if( pOldEntry )
        mpFontCache->Release( pOldEntry );

    ImplFontEntry* pFontEntry = mpFontEntry;

    if (!pFontEntry)
        return false;

    // mark when lower layers need to get involved
    mbNewFont = false;
    if( pFontEntry != pOldEntry )
        mbInitFont = true;

    // select font when it has not been initialized yet
    if ( !pFontEntry->mbInit )
    {
        ImplInitFont();

        // get metric data from device layers
        if ( pGraphics )
        {
            pFontEntry->mbInit = true;

            pFontEntry->maMetric.mnOrientation  = sal::static_int_cast<short>(pFontEntry->maFontSelData.mnOrientation);
            pGraphics->GetFontMetric( &(pFontEntry->maMetric) );

            pFontEntry->maMetric.ImplInitTextLineSize( this );
            pFontEntry->maMetric.ImplInitAboveTextLineSize();

            pFontEntry->mnLineHeight = pFontEntry->maMetric.mnAscent + pFontEntry->maMetric.mnDescent;

            if( pFontEntry->maFontSelData.mnOrientation
            && !pFontEntry->maMetric.mnOrientation
            && (meOutDevType != OUTDEV_PRINTER) )
            {
                pFontEntry->mnOwnOrientation = sal::static_int_cast<short>(pFontEntry->maFontSelData.mnOrientation);
                pFontEntry->mnOrientation = pFontEntry->mnOwnOrientation;
            }
            else
                pFontEntry->mnOrientation = pFontEntry->maMetric.mnOrientation;
        }
    }

    // enable kerning array if requested
    if ( maFont.GetKerning() & KERNING_FONTSPECIFIC )
    {
        // TODO: test if physical font supports kerning and disable if not
        if( pFontEntry->maMetric.mbKernableFont )
            mbKerning = true;
    }
    else
        mbKerning = false;
    if ( maFont.GetKerning() & KERNING_ASIAN )
        mbKerning = true;

    // calculate EmphasisArea
    mnEmphasisAscent = 0;
    mnEmphasisDescent = 0;
    if ( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
    {
        FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
        long                nEmphasisHeight = (pFontEntry->mnLineHeight*250)/1000;
        if ( nEmphasisHeight < 1 )
            nEmphasisHeight = 1;
        if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
            mnEmphasisDescent = nEmphasisHeight;
        else
            mnEmphasisAscent = nEmphasisHeight;
    }

    // calculate text offset depending on TextAlignment
    TextAlign eAlign = maFont.GetAlign();
    if ( eAlign == ALIGN_BASELINE )
    {
        mnTextOffX = 0;
        mnTextOffY = 0;
    }
    else if ( eAlign == ALIGN_TOP )
    {
        mnTextOffX = 0;
        mnTextOffY = +pFontEntry->maMetric.mnAscent + mnEmphasisAscent;
        if ( pFontEntry->mnOrientation )
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
    }
    else // eAlign == ALIGN_BOTTOM
    {
        mnTextOffX = 0;
        mnTextOffY = -pFontEntry->maMetric.mnDescent + mnEmphasisDescent;
        if ( pFontEntry->mnOrientation )
            ImplRotatePos( 0, 0, mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
    }

    mbTextLines     = ((maFont.GetUnderline() != UNDERLINE_NONE) && (maFont.GetUnderline() != UNDERLINE_DONTKNOW)) ||
                      ((maFont.GetOverline()  != UNDERLINE_NONE) && (maFont.GetOverline()  != UNDERLINE_DONTKNOW)) ||
                      ((maFont.GetStrikeout() != STRIKEOUT_NONE) && (maFont.GetStrikeout() != STRIKEOUT_DONTKNOW));
    mbTextSpecial   = maFont.IsShadow() || maFont.IsOutline() ||
                      (maFont.GetRelief() != RELIEF_NONE);

    // #95414# fix for OLE objects which use scale factors very creatively
    if( mbMap && !aSize.Width() )
    {
        int nOrigWidth = pFontEntry->maMetric.mnWidth;
        float fStretch = (float)maMapRes.mnMapScNumX * maMapRes.mnMapScDenomY;
        fStretch /= (float)maMapRes.mnMapScNumY * maMapRes.mnMapScDenomX;
        int nNewWidth = (int)(nOrigWidth * fStretch + 0.5);
        if( (nNewWidth != nOrigWidth) && (nNewWidth != 0) )
        {
            Size aOrigSize = maFont.GetSize();
            const_cast<Font&>(maFont).SetSize( Size( nNewWidth, aSize.Height() ) );
            mbMap = false;
            mbNewFont = true;
            ImplNewFont();  // recurse once using stretched width
            mbMap = true;
            const_cast<Font&>(maFont).SetSize( aOrigSize );
        }
    }

    return true;
}

void OutputDevice::ImplDrawTextRect( long nBaseX, long nBaseY,
                                     long nDistX, long nDistY, long nWidth, long nHeight )
{
    long nX = nDistX;
    long nY = nDistY;

    short nOrientation = mpFontEntry->mnOrientation;
    if ( nOrientation )
    {
        // Rotate rect without rounding problems for 90 degree rotations
        if ( !(nOrientation % 900) )
        {
            if ( nOrientation == 900 )
            {
                long nTemp = nX;
                nX = nY;
                nY = -nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nY -= nHeight;
            }
            else if ( nOrientation == 1800 )
            {
                nX = -nX;
                nY = -nY;
                nX -= nWidth;
                nY -= nHeight;
            }
            else /* ( nOrientation == 2700 ) */
            {
                long nTemp = nX;
                nX = -nY;
                nY = nTemp;
                nTemp = nWidth;
                nWidth = nHeight;
                nHeight = nTemp;
                nX -= nWidth;
            }
        }
        else
        {
            nX += nBaseX;
            nY += nBaseY;
            // inflate because polygons are drawn smaller
            Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
            ImplDrawPolygon( aPoly );
            return;
        }
    }

    nX += nBaseX;
    nY += nBaseY;
    mpGraphics->DrawRect( nX, nY, nWidth, nHeight, this );
}

void OutputDevice::ImplDrawTextBackground( const SalLayout& rSalLayout )
{
    const long nWidth = rSalLayout.GetTextWidth() / rSalLayout.GetUnitsPerPixel();
    const Point aBase = rSalLayout.DrawBase();
    const long nX = aBase.X();
    const long nY = aBase.Y();

    if ( mbLineColor || mbInitLineColor )
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = true;
    }
    mpGraphics->SetFillColor( ImplColorToSal( GetTextFillColor() ) );
    mbInitFillColor = true;

    ImplDrawTextRect( nX, nY, 0, -(mpFontEntry->maMetric.mnAscent + mnEmphasisAscent),
                      nWidth,
                      mpFontEntry->mnLineHeight+mnEmphasisAscent+mnEmphasisDescent );
}

Rectangle OutputDevice::ImplGetTextBoundRect( const SalLayout& rSalLayout )
{
    Point aPoint = rSalLayout.GetDrawPosition();
    long nX = aPoint.X();
    long nY = aPoint.Y();

    long nWidth = rSalLayout.GetTextWidth();
    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    nY -= mpFontEntry->maMetric.mnAscent + mnEmphasisAscent;

    if ( mpFontEntry->mnOrientation )
    {
        long nBaseX = nX, nBaseY = nY;
        if ( !(mpFontEntry->mnOrientation % 900) )
        {
            long nX2 = nX+nWidth;
            long nY2 = nY+nHeight;
            ImplRotatePos( nBaseX, nBaseY, nX, nY, mpFontEntry->mnOrientation );
            ImplRotatePos( nBaseX, nBaseY, nX2, nY2, mpFontEntry->mnOrientation );
            nWidth = nX2-nX;
            nHeight = nY2-nY;
        }
        else
        {
            // inflate by +1+1 because polygons are drawn smaller
            Rectangle aRect( Point( nX, nY ), Size( nWidth+1, nHeight+1 ) );
            Polygon   aPoly( aRect );
            aPoly.Rotate( Point( nBaseX, nBaseY ), mpFontEntry->mnOrientation );
            return aPoly.GetBoundRect();
        }
    }

    return Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );
}

void OutputDevice::ImplInitTextLineSize()
{
    mpFontEntry->maMetric.ImplInitTextLineSize( this );
}

void OutputDevice::ImplInitAboveTextLineSize()
{
    mpFontEntry->maMetric.ImplInitAboveTextLineSize();
}

bool ImplFontAttributes::operator==(const ImplFontAttributes& rOther) const
{
    if (maName != rOther.maName)
        return false;

    if (maStyleName != rOther.maStyleName)
        return false;

    if (meWeight != rOther.meWeight)
        return false;

    if (meItalic != rOther.meItalic)
        return false;

    if (meFamily != rOther.meFamily)
        return false;

    if (mePitch != rOther.mePitch)
        return false;

    if (meWidthType != rOther.meWidthType)
        return false;

    if (mbSymbolFlag != rOther.mbSymbolFlag)
        return false;

    return true;
}

ImplFontMetricData::ImplFontMetricData( const FontSelectPattern& rFontSelData )
    : ImplFontAttributes( rFontSelData )
    , mnWidth ( rFontSelData.mnWidth)
    , mnOrientation( (short)(rFontSelData.mnOrientation))
    , mnAscent( 0 )
    , mnDescent( 0 )
    , mnIntLeading( 0 )
    , mnExtLeading( 0 )
    , mnSlant( 0 )
    , mnMinKashida( 0 )
    , mnUnderlineSize( 0 )
    , mnUnderlineOffset( 0 )
    , mnBUnderlineSize( 0 )
    , mnBUnderlineOffset( 0 )
    , mnDUnderlineSize( 0 )
    , mnDUnderlineOffset1( 0 )
    , mnDUnderlineOffset2( 0 )
    , mnWUnderlineSize( 0 )
    , mnWUnderlineOffset( 0 )
    , mnAboveUnderlineSize( 0 )
    , mnAboveUnderlineOffset( 0 )
    , mnAboveBUnderlineSize( 0 )
    , mnAboveBUnderlineOffset( 0 )
    , mnAboveDUnderlineSize( 0 )
    , mnAboveDUnderlineOffset1( 0 )
    , mnAboveDUnderlineOffset2( 0 )
    , mnAboveWUnderlineSize( 0 )
    , mnAboveWUnderlineOffset( 0 )
    , mnStrikeoutSize( 0 )
    , mnStrikeoutOffset( 0 )
    , mnBStrikeoutSize( 0 )
    , mnBStrikeoutOffset( 0 )
    , mnDStrikeoutSize( 0 )
    , mnDStrikeoutOffset1( 0 )
    , mnDStrikeoutOffset2( 0 )
{
    // intialize the used font name
    if( rFontSelData.mpFontData )
    {
        SetFamilyName( rFontSelData.mpFontData->GetFamilyName() );
        SetStyleName( rFontSelData.mpFontData->GetStyleName() );
        mbDevice   = rFontSelData.mpFontData->mbDevice;
        mbKernableFont = true;
    }
    else
    {
        sal_Int32 nTokenPos = 0;
        SetFamilyName( GetNextFontToken( rFontSelData.GetFamilyName(), nTokenPos ) );
        SetStyleName( rFontSelData.GetStyleName() );
        mbDevice   = false;
        mbKernableFont = false;
    }
}

void ImplFontMetricData::ImplInitTextLineSize( const OutputDevice* pDev )
{
    long nDescent = mnDescent;
    if ( nDescent <= 0 )
    {
        nDescent = mnAscent / 10;
        if ( !nDescent )
            nDescent = 1;
    }

    // #i55341# for some fonts it is not a good idea to calculate
    // their text line metrics from the real font descent
    // => work around this problem just for these fonts
    if( 3*nDescent > mnAscent )
        nDescent = mnAscent / 3;

    long nLineHeight = ((nDescent*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;
    long nLineHeight2 = nLineHeight / 2;
    if ( !nLineHeight2 )
        nLineHeight2 = 1;

    long nBLineHeight = ((nDescent*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;
    long nBLineHeight2 = nBLineHeight/2;
    if ( !nBLineHeight2 )
        nBLineHeight2 = 1;

    long n2LineHeight = ((nDescent*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;
    long n2LineDY = n2LineHeight;
     /* #117909#
      * add some pixels to minimum double line distance on higher resolution devices
      */
    long nMin2LineDY = 1 + pDev->ImplGetDPIY()/150;
    if ( n2LineDY < nMin2LineDY )
        n2LineDY = nMin2LineDY;
    long n2LineDY2 = n2LineDY/2;
    if ( !n2LineDY2 )
        n2LineDY2 = 1;

    long nUnderlineOffset = mnDescent/2 + 1;
    long nStrikeoutOffset = -((mnAscent - mnIntLeading) / 3);

    mnUnderlineSize        = nLineHeight;
    mnUnderlineOffset      = nUnderlineOffset - nLineHeight2;

    mnBUnderlineSize       = nBLineHeight;
    mnBUnderlineOffset     = nUnderlineOffset - nBLineHeight2;

    mnDUnderlineSize       = n2LineHeight;
    mnDUnderlineOffset1    = nUnderlineOffset - n2LineDY2 - n2LineHeight;
    mnDUnderlineOffset2    = mnDUnderlineOffset1 + n2LineDY + n2LineHeight;

    long nWCalcSize = mnDescent;
    if ( nWCalcSize < 6 )
    {
        if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
            mnWUnderlineSize = nWCalcSize;
        else
            mnWUnderlineSize = 3;
    }
    else
        mnWUnderlineSize = ((nWCalcSize*50)+50) / 100;

    // #109280# the following line assures that wavelnes are never placed below the descent, however
    // for most fonts the waveline then is drawn into the text, so we better keep the old solution
    // pFontEntry->maMetric.mnWUnderlineOffset     = pFontEntry->maMetric.mnDescent + 1 - pFontEntry->maMetric.mnWUnderlineSize;
    mnWUnderlineOffset     = nUnderlineOffset;

    mnStrikeoutSize        = nLineHeight;
    mnStrikeoutOffset      = nStrikeoutOffset - nLineHeight2;

    mnBStrikeoutSize       = nBLineHeight;
    mnBStrikeoutOffset     = nStrikeoutOffset - nBLineHeight2;

    mnDStrikeoutSize       = n2LineHeight;
    mnDStrikeoutOffset1    = nStrikeoutOffset - n2LineDY2 - n2LineHeight;
    mnDStrikeoutOffset2    = mnDStrikeoutOffset1 + n2LineDY + n2LineHeight;
}

void ImplFontMetricData::ImplInitAboveTextLineSize()
{
    long nIntLeading = mnIntLeading;
    // TODO: assess usage of nLeading below (changed in extleading CWS)
    // if no leading is available, we assume 15% of the ascent
    if ( nIntLeading <= 0 )
    {
        nIntLeading = mnAscent*15/100;
        if ( !nIntLeading )
            nIntLeading = 1;
    }

    long nLineHeight = ((nIntLeading*25)+50) / 100;
    if ( !nLineHeight )
        nLineHeight = 1;

    long nBLineHeight = ((nIntLeading*50)+50) / 100;
    if ( nBLineHeight == nLineHeight )
        nBLineHeight++;

    long n2LineHeight = ((nIntLeading*16)+50) / 100;
    if ( !n2LineHeight )
        n2LineHeight = 1;

    long nCeiling = -mnAscent;

    mnAboveUnderlineSize       = nLineHeight;
    mnAboveUnderlineOffset     = nCeiling + (nIntLeading - nLineHeight + 1) / 2;

    mnAboveBUnderlineSize      = nBLineHeight;
    mnAboveBUnderlineOffset    = nCeiling + (nIntLeading - nBLineHeight + 1) / 2;

    mnAboveDUnderlineSize      = n2LineHeight;
    mnAboveDUnderlineOffset1   = nCeiling + (nIntLeading - 3*n2LineHeight + 1) / 2;
    mnAboveDUnderlineOffset2   = nCeiling + (nIntLeading +   n2LineHeight + 1) / 2;

    long nWCalcSize = nIntLeading;
    if ( nWCalcSize < 6 )
    {
        if ( (nWCalcSize == 1) || (nWCalcSize == 2) )
            mnAboveWUnderlineSize = nWCalcSize;
        else
            mnAboveWUnderlineSize = 3;
    }
    else
        mnAboveWUnderlineSize = ((nWCalcSize*50)+50) / 100;

    mnAboveWUnderlineOffset = nCeiling + (nIntLeading + 1) / 2;
}

static void ImplDrawWavePixel( long nOriginX, long nOriginY,
                               long nCurX, long nCurY,
                               short nOrientation,
                               SalGraphics* pGraphics,
                               OutputDevice* pOutDev,
                               bool bDrawPixAsRect,

                               long nPixWidth, long nPixHeight )
{
    if ( nOrientation )
        ImplRotatePos( nOriginX, nOriginY, nCurX, nCurY, nOrientation );

    if ( bDrawPixAsRect )
    {

        pGraphics->DrawRect( nCurX, nCurY, nPixWidth, nPixHeight, pOutDev );
    }
    else
    {
        pGraphics->DrawPixel( nCurX, nCurY, pOutDev );
    }
}

void OutputDevice::ImplDrawWaveLine( long nBaseX, long nBaseY,
                                     long nDistX, long nDistY,
                                     long nWidth, long nHeight,
                                     long nLineWidth, short nOrientation,
                                     const Color& rColor )
{
    if ( !nHeight )
        return;

    long nStartX = nBaseX + nDistX;
    long nStartY = nBaseY + nDistY;

    // If the height is 1 pixel, it's enough ouput a line
    if ( (nLineWidth == 1) && (nHeight == 1) )
    {
        mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
        mbInitLineColor = true;

        long nEndX = nStartX+nWidth;
        long nEndY = nStartY;
        if ( nOrientation )
        {
            ImplRotatePos( nBaseX, nBaseY, nStartX, nStartY, nOrientation );
            ImplRotatePos( nBaseX, nBaseY, nEndX, nEndY, nOrientation );
        }
        mpGraphics->DrawLine( nStartX, nStartY, nEndX, nEndY, this );
    }
    else
    {
        long    nCurX = nStartX;
        long    nCurY = nStartY;
        long    nDiffX = 2;
        long    nDiffY = nHeight-1;
        long    nCount = nWidth;
        long    nOffY = -1;
        long    nFreq;
        long    i;
        long    nPixWidth;
        long    nPixHeight;
        bool    bDrawPixAsRect;
        // On printers that ouput pixel via DrawRect()
        if ( (GetOutDevType() == OUTDEV_PRINTER) || (nLineWidth > 1) )
        {
            if ( mbLineColor || mbInitLineColor )
            {
                mpGraphics->SetLineColor();
                mbInitLineColor = true;
            }
            mpGraphics->SetFillColor( ImplColorToSal( rColor ) );
            mbInitFillColor = true;
            bDrawPixAsRect  = true;
            nPixWidth       = nLineWidth;
            nPixHeight      = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        }
        else
        {
            mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
            mbInitLineColor = true;
            nPixWidth       = 1;
            nPixHeight      = 1;
            bDrawPixAsRect  = false;
        }

        if ( !nDiffY )
        {
            while ( nWidth )
            {
                ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                   mpGraphics, this,
                                   bDrawPixAsRect, nPixWidth, nPixHeight );
                nCurX++;
                nWidth--;
            }
        }
        else
        {
            nCurY += nDiffY;
            nFreq = nCount / (nDiffX+nDiffY);
            while ( nFreq-- )
            {
                for( i = nDiffY; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;
                }
                for( i = nDiffX; i; --i )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
                nOffY = -nOffY;
            }
            nFreq = nCount % (nDiffX+nDiffY);
            if ( nFreq )
            {
                for( i = nDiffY; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                    nCurY += nOffY;

                }
                for( i = nDiffX; i && nFreq; --i, --nFreq )
                {
                    ImplDrawWavePixel( nBaseX, nBaseY, nCurX, nCurY, nOrientation,
                                       mpGraphics, this,
                                       bDrawPixAsRect, nPixWidth, nPixHeight );
                    nCurX++;
                }
            }
        }

    }
}

void OutputDevice::ImplDrawWaveTextLine( long nBaseX, long nBaseY,
                                         long nDistX, long nDistY, long nWidth,
                                         FontUnderline eTextLine,
                                         Color aColor,
                                         bool bIsAbove )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight;
    long            nLinePos;

    if ( bIsAbove )
    {
        nLineHeight = pFontEntry->maMetric.mnAboveWUnderlineSize;
        nLinePos = pFontEntry->maMetric.mnAboveWUnderlineOffset;
    }
    else
    {
        nLineHeight = pFontEntry->maMetric.mnWUnderlineSize;
        nLinePos = pFontEntry->maMetric.mnWUnderlineOffset;
    }
    if ( (eTextLine == UNDERLINE_SMALLWAVE) && (nLineHeight > 3) )
        nLineHeight = 3;
    long nLineWidth = (mnDPIX/300);
    if ( !nLineWidth )
        nLineWidth = 1;
    if ( eTextLine == UNDERLINE_BOLDWAVE )
        nLineWidth *= 2;
    nLinePos += nDistY - (nLineHeight / 2);
    long nLineWidthHeight = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
    if ( eTextLine == UNDERLINE_DOUBLEWAVE )
    {
        long nOrgLineHeight = nLineHeight;
        nLineHeight /= 3;
        if ( nLineHeight < 2 )
        {
            if ( nOrgLineHeight > 1 )
                nLineHeight = 2;
            else
                nLineHeight = 1;
        }
        long nLineDY = nOrgLineHeight-(nLineHeight*2);
        if ( nLineDY < nLineWidthHeight )
            nLineDY = nLineWidthHeight;
        long nLineDY2 = nLineDY/2;
        if ( !nLineDY2 )
            nLineDY2 = 1;

        nLinePos -= nLineWidthHeight-nLineDY2;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontEntry->mnOrientation, aColor );
        nLinePos += nLineWidthHeight+nLineDY;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontEntry->mnOrientation, aColor );
    }
    else
    {
        nLinePos -= nLineWidthHeight/2;
        ImplDrawWaveLine( nBaseX, nBaseY, nDistX, nLinePos, nWidth, nLineHeight,
                          nLineWidth, mpFontEntry->mnOrientation, aColor );
    }
}

void OutputDevice::ImplDrawStraightTextLine( long nBaseX, long nBaseY,
                                             long nDistX, long nDistY, long nWidth,
                                             FontUnderline eTextLine,
                                             Color aColor,
                                             bool bIsAbove )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight = 0;
    long            nLinePos  = 0;
    long            nLinePos2 = 0;

    const long nY = nDistY;

    if ( eTextLine > UNDERLINE_LAST )
        eTextLine = UNDERLINE_SINGLE;

    switch ( eTextLine )
    {
        case UNDERLINE_SINGLE:
        case UNDERLINE_DOTTED:
        case UNDERLINE_DASH:
        case UNDERLINE_LONGDASH:
        case UNDERLINE_DASHDOT:
        case UNDERLINE_DASHDOTDOT:
            if ( bIsAbove )
            {
                nLineHeight = pFontEntry->maMetric.mnAboveUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveUnderlineOffset;
            }
            else
            {
                nLineHeight = pFontEntry->maMetric.mnUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnUnderlineOffset;
            }
            break;
        case UNDERLINE_BOLD:
        case UNDERLINE_BOLDDOTTED:
        case UNDERLINE_BOLDDASH:
        case UNDERLINE_BOLDLONGDASH:
        case UNDERLINE_BOLDDASHDOT:
        case UNDERLINE_BOLDDASHDOTDOT:
            if ( bIsAbove )
            {
                nLineHeight = pFontEntry->maMetric.mnAboveBUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveBUnderlineOffset;
            }
            else
            {
                nLineHeight = pFontEntry->maMetric.mnBUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnBUnderlineOffset;
            }
            break;
        case UNDERLINE_DOUBLE:
            if ( bIsAbove )
            {
                nLineHeight = pFontEntry->maMetric.mnAboveDUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset1;
                nLinePos2   = nY + pFontEntry->maMetric.mnAboveDUnderlineOffset2;
            }
            else
            {
                nLineHeight = pFontEntry->maMetric.mnDUnderlineSize;
                nLinePos    = nY + pFontEntry->maMetric.mnDUnderlineOffset1;
                nLinePos2   = nY + pFontEntry->maMetric.mnDUnderlineOffset2;
            }
            break;
        default:
            break;
    }

    if ( nLineHeight )
    {
        if ( mbLineColor || mbInitLineColor )
        {
            mpGraphics->SetLineColor();
            mbInitLineColor = true;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aColor ) );
        mbInitFillColor = true;

        long nLeft = nDistX;

        switch ( eTextLine )
        {
            case UNDERLINE_SINGLE:
            case UNDERLINE_BOLD:
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                break;
            case UNDERLINE_DOUBLE:
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos,  nWidth, nLineHeight );
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
                break;
            case UNDERLINE_DOTTED:
            case UNDERLINE_BOLDDOTTED:
                {
                    long nDotWidth = nLineHeight*mnDPIY;
                    nDotWidth += mnDPIY/2;
                    nDotWidth /= mnDPIY;
                    long nTempWidth = nDotWidth;
                    long nEnd = nLeft+nWidth;
                    while ( nLeft < nEnd )
                    {
                        if ( nLeft+nTempWidth > nEnd )
                            nTempWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                        nLeft += nDotWidth*2;
                    }
                }
                break;
            case UNDERLINE_DASH:
            case UNDERLINE_LONGDASH:
            case UNDERLINE_BOLDDASH:
            case UNDERLINE_BOLDLONGDASH:
                {
                    long nDotWidth = nLineHeight*mnDPIY;
                    nDotWidth += mnDPIY/2;
                    nDotWidth /= mnDPIY;
                    long nMinDashWidth;
                    long nMinSpaceWidth;
                    long nSpaceWidth;
                    long nDashWidth;
                    if ( (eTextLine == UNDERLINE_LONGDASH) ||
                         (eTextLine == UNDERLINE_BOLDLONGDASH) )
                    {
                        nMinDashWidth = nDotWidth*6;
                        nMinSpaceWidth = nDotWidth*2;
                        nDashWidth = 200;
                        nSpaceWidth = 100;
                    }
                    else
                    {
                        nMinDashWidth = nDotWidth*4;
                        nMinSpaceWidth = (nDotWidth*150)/100;
                        nDashWidth = 100;
                        nSpaceWidth = 50;
                    }
                    nDashWidth = ((nDashWidth*mnDPIX)+1270)/2540;
                    nSpaceWidth = ((nSpaceWidth*mnDPIX)+1270)/2540;
                    // DashWidth will be increased if the line is getting too thick
                    // in proportion to the line's length
                    if ( nDashWidth < nMinDashWidth )
                        nDashWidth = nMinDashWidth;
                    if ( nSpaceWidth < nMinSpaceWidth )
                        nSpaceWidth = nMinSpaceWidth;
                    long nTempWidth = nDashWidth;
                    long nEnd = nLeft+nWidth;
                    while ( nLeft < nEnd )
                    {
                        if ( nLeft+nTempWidth > nEnd )
                            nTempWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempWidth, nLineHeight );
                        nLeft += nDashWidth+nSpaceWidth;
                    }
                }
                break;
            case UNDERLINE_DASHDOT:
            case UNDERLINE_BOLDDASHDOT:
                {
                    long nDotWidth = nLineHeight*mnDPIY;
                    nDotWidth += mnDPIY/2;
                    nDotWidth /= mnDPIY;
                    long nDashWidth = ((100*mnDPIX)+1270)/2540;
                    long nMinDashWidth = nDotWidth*4;
                    // DashWidth will be increased if the line is getting too thick
                    // in proportion to the line's length
                    if ( nDashWidth < nMinDashWidth )
                        nDashWidth = nMinDashWidth;
                    long nTempDotWidth = nDotWidth;
                    long nTempDashWidth = nDashWidth;
                    long nEnd = nLeft+nWidth;
                    while ( nLeft < nEnd )
                    {
                        if ( nLeft+nTempDotWidth > nEnd )
                            nTempDotWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                        nLeft += nDotWidth*2;
                        if ( nLeft > nEnd )
                            break;
                        if ( nLeft+nTempDashWidth > nEnd )
                            nTempDashWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                        nLeft += nDashWidth+nDotWidth;
                    }
                }
                break;
            case UNDERLINE_DASHDOTDOT:
            case UNDERLINE_BOLDDASHDOTDOT:
                {
                    long nDotWidth = nLineHeight*mnDPIY;
                    nDotWidth += mnDPIY/2;
                    nDotWidth /= mnDPIY;
                    long nDashWidth = ((100*mnDPIX)+1270)/2540;
                    long nMinDashWidth = nDotWidth*4;
                    // DashWidth will be increased if the line is getting too thick
                    // in proportion to the line's length
                    if ( nDashWidth < nMinDashWidth )
                        nDashWidth = nMinDashWidth;
                    long nTempDotWidth = nDotWidth;
                    long nTempDashWidth = nDashWidth;
                    long nEnd = nLeft+nWidth;
                    while ( nLeft < nEnd )
                    {
                        if ( nLeft+nTempDotWidth > nEnd )
                            nTempDotWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                        nLeft += nDotWidth*2;
                        if ( nLeft > nEnd )
                            break;
                        if ( nLeft+nTempDotWidth > nEnd )
                            nTempDotWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDotWidth, nLineHeight );
                        nLeft += nDotWidth*2;
                        if ( nLeft > nEnd )
                            break;
                        if ( nLeft+nTempDashWidth > nEnd )
                            nTempDashWidth = nEnd-nLeft;
                        ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nTempDashWidth, nLineHeight );
                        nLeft += nDashWidth+nDotWidth;
                    }
                }
                break;
            default:
                break;
        }
    }
}

void OutputDevice::ImplDrawStrikeoutLine( long nBaseX, long nBaseY,
                                          long nDistX, long nDistY, long nWidth,
                                          FontStrikeout eStrikeout,
                                          Color aColor )
{
    ImplFontEntry*  pFontEntry = mpFontEntry;
    long            nLineHeight = 0;
    long            nLinePos  = 0;
    long            nLinePos2 = 0;

    long nY = nDistY;

    if ( eStrikeout > STRIKEOUT_LAST )
        eStrikeout = STRIKEOUT_SINGLE;

    switch ( eStrikeout )
    {
        case STRIKEOUT_SINGLE:
            nLineHeight = pFontEntry->maMetric.mnStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnStrikeoutOffset;
            break;
        case STRIKEOUT_BOLD:
            nLineHeight = pFontEntry->maMetric.mnBStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnBStrikeoutOffset;
            break;
        case STRIKEOUT_DOUBLE:
            nLineHeight = pFontEntry->maMetric.mnDStrikeoutSize;
            nLinePos    = nY + pFontEntry->maMetric.mnDStrikeoutOffset1;
            nLinePos2   = nY + pFontEntry->maMetric.mnDStrikeoutOffset2;
            break;
        default:
            break;
    }

    if ( nLineHeight )
    {
        if ( mbLineColor || mbInitLineColor )
        {
            mpGraphics->SetLineColor();
            mbInitLineColor = true;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aColor ) );
        mbInitFillColor = true;

        const long& nLeft = nDistX;

        switch ( eStrikeout )
        {
            case STRIKEOUT_SINGLE:
            case STRIKEOUT_BOLD:
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                break;
            case STRIKEOUT_DOUBLE:
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos, nWidth, nLineHeight );
                ImplDrawTextRect( nBaseX, nBaseY, nLeft, nLinePos2, nWidth, nLineHeight );
                break;
            default:
                break;
        }
    }
}

void OutputDevice::ImplDrawStrikeoutChar( long nBaseX, long nBaseY,
                                          long nDistX, long nDistY, long nWidth,
                                          FontStrikeout eStrikeout,
                                          Color aColor )
{
    // See qadevOOo/testdocs/StrikeThrough.odt for examples if you need
    // to tweak this
    if (!nWidth)
        return;

    // prepare string for strikeout measurement
    static char cStrikeoutChar;
    if ( eStrikeout == STRIKEOUT_SLASH )
        cStrikeoutChar = '/';
    else // ( eStrikeout == STRIKEOUT_X )
        cStrikeoutChar = 'X';
    static const int nTestStrLen = 4;
    static const int nMaxStrikeStrLen = 2048;
    sal_Unicode aChars[nMaxStrikeStrLen+1]; // +1 for valgrind...
    for( int i = 0; i < nTestStrLen; ++i)
        aChars[i] = cStrikeoutChar;
    const OUString aStrikeoutTest(aChars, nTestStrLen);

    // calculate approximation of strikeout atom size
    long nStrikeoutWidth = 0;
    SalLayout* pLayout = ImplLayout( aStrikeoutTest, 0, nTestStrLen );
    if( pLayout )
    {
        nStrikeoutWidth = pLayout->GetTextWidth() / (nTestStrLen * pLayout->GetUnitsPerPixel());
        pLayout->Release();
    }
    if( nStrikeoutWidth <= 0 ) // sanity check
        return;

    int nStrikeStrLen = (nWidth+(nStrikeoutWidth-1)) / nStrikeoutWidth;
    if( nStrikeStrLen > nMaxStrikeStrLen )
        nStrikeStrLen = nMaxStrikeStrLen;

    // build the strikeout string
    for( int i = nTestStrLen; i < nStrikeStrLen; ++i)
        aChars[i] = cStrikeoutChar;
    const OUString aStrikeoutText(aChars, nStrikeStrLen);

    if( mpFontEntry->mnOrientation )
        ImplRotatePos( 0, 0, nDistX, nDistY, mpFontEntry->mnOrientation );
    nBaseX += nDistX;
    nBaseY += nDistY;

    // strikeout text has to be left aligned
    sal_uLong nOrigTLM = mnTextLayoutMode;
    mnTextLayoutMode = TEXT_LAYOUT_BIDI_STRONG | TEXT_LAYOUT_COMPLEX_DISABLED;
    pLayout = ImplLayout( aStrikeoutText, 0, aStrikeoutText.getLength() );
    mnTextLayoutMode = nOrigTLM;

    if( !pLayout )
        return;

    // draw the strikeout text
    const Color aOldColor = GetTextColor();
    SetTextColor( aColor );
    ImplInitTextColor();

    pLayout->DrawBase() = Point( nBaseX+mnTextOffX, nBaseY+mnTextOffY );

    Rectangle aPixelRect;
    aPixelRect.Left() = nBaseX+mnTextOffX;
    aPixelRect.Right() = aPixelRect.Left()+nWidth;
    aPixelRect.Bottom() = nBaseY+mpFontEntry->maMetric.mnDescent;
    aPixelRect.Top() = nBaseY-mpFontEntry->maMetric.mnAscent;

    if (mpFontEntry->mnOrientation)
    {
        Polygon aPoly( aPixelRect );
        aPoly.Rotate( Point(nBaseX+mnTextOffX, nBaseY+mnTextOffY), mpFontEntry->mnOrientation);
        aPixelRect = aPoly.GetBoundRect();
    }

    Push( PUSH_CLIPREGION );
    IntersectClipRegion( PixelToLogic(aPixelRect) );
    if( mbInitClipRegion )
        ImplInitClipRegion();

    pLayout->DrawText( *mpGraphics );

    pLayout->Release();
    Pop();

    SetTextColor( aOldColor );
    ImplInitTextColor();
}

void OutputDevice::ImplDrawTextLine( long nX, long nY,
                                     long nDistX, long nWidth,
                                     FontStrikeout eStrikeout,
                                     FontUnderline eUnderline,
                                     FontUnderline eOverline,
                                     bool bUnderlineAbove )
{
    if ( !nWidth )
        return;

    Color           aStrikeoutColor = GetTextColor();
    Color           aUnderlineColor = GetTextLineColor();
    Color           aOverlineColor  = GetOverlineColor();
    bool            bStrikeoutDone = false;
    bool            bUnderlineDone = false;
    bool            bOverlineDone  = false;

    if ( IsRTLEnabled() )
    {
        // --- RTL --- mirror at basex
        long nXAdd = nWidth - nDistX;
        if( mpFontEntry->mnOrientation )
            nXAdd = FRound( nXAdd * cos( mpFontEntry->mnOrientation * F_PI1800 ) );
        nX += nXAdd - 1;
    }

    if ( !IsTextLineColor() )
        aUnderlineColor = GetTextColor();

    if ( !IsOverlineColor() )
        aOverlineColor = GetTextColor();

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        ImplDrawWaveTextLine( nX, nY, nDistX, 0, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );
        bUnderlineDone = true;
    }
    if ( (eOverline == UNDERLINE_SMALLWAVE) ||
         (eOverline == UNDERLINE_WAVE) ||
         (eOverline == UNDERLINE_DOUBLEWAVE) ||
         (eOverline == UNDERLINE_BOLDWAVE) )
    {
        ImplDrawWaveTextLine( nX, nY, nDistX, 0, nWidth, eOverline, aOverlineColor, true );
        bOverlineDone = true;
    }

    if ( (eStrikeout == STRIKEOUT_SLASH) ||
         (eStrikeout == STRIKEOUT_X) )
    {
        ImplDrawStrikeoutChar( nX, nY, nDistX, 0, nWidth, eStrikeout, aStrikeoutColor );
        bStrikeoutDone = true;
    }

    if ( !bUnderlineDone )
        ImplDrawStraightTextLine( nX, nY, nDistX, 0, nWidth, eUnderline, aUnderlineColor, bUnderlineAbove );

    if ( !bOverlineDone )
        ImplDrawStraightTextLine( nX, nY, nDistX, 0, nWidth, eOverline, aOverlineColor, true );

    if ( !bStrikeoutDone )
        ImplDrawStrikeoutLine( nX, nY, nDistX, 0, nWidth, eStrikeout, aStrikeoutColor );
}

void OutputDevice::ImplDrawTextLines( SalLayout& rSalLayout,
    FontStrikeout eStrikeout, FontUnderline eUnderline, FontUnderline eOverline, bool bWordLine, bool bUnderlineAbove )
{
    if( bWordLine )
    {
        // draw everything relative to the layout base point
        const Point aStartPt = rSalLayout.DrawBase();

        // calculate distance of each word from the base point
        Point aPos;
        sal_Int32 nDist = 0, nWidth = 0, nAdvance=0;
        for( int nStart = 0;;)
        {
            // iterate through the layouted glyphs
            sal_GlyphId aGlyphId;
            if( !rSalLayout.GetNextGlyphs( 1, &aGlyphId, aPos, nStart, &nAdvance ) )
                break;

            // calculate the boundaries of each word
            if( !rSalLayout.IsSpacingGlyph( aGlyphId ) )
            {
                if( !nWidth )
                {
                    // get the distance to the base point (as projected to baseline)
                    nDist = aPos.X() - aStartPt.X();
                    if( mpFontEntry->mnOrientation )
                    {
                        const long nDY = aPos.Y() - aStartPt.Y();
                        const double fRad = mpFontEntry->mnOrientation * F_PI1800;
                        nDist = FRound( nDist*cos(fRad) - nDY*sin(fRad) );
                    }
                }

                // update the length of the textline
                nWidth += nAdvance;
            }
            else if( nWidth > 0 )
            {
                // draw the textline for each word
                ImplDrawTextLine( aStartPt.X(), aStartPt.Y(), nDist, nWidth,
                    eStrikeout, eUnderline, eOverline, bUnderlineAbove );
                nWidth = 0;
            }
        }

        // draw textline for the last word
        if( nWidth > 0 )
        {
            ImplDrawTextLine( aStartPt.X(), aStartPt.Y(), nDist, nWidth,
                eStrikeout, eUnderline, eOverline, bUnderlineAbove );
        }
    }
    else
    {
        Point aStartPt = rSalLayout.GetDrawPosition();
        int nWidth = rSalLayout.GetTextWidth() / rSalLayout.GetUnitsPerPixel();
        ImplDrawTextLine( aStartPt.X(), aStartPt.Y(), 0, nWidth,
            eStrikeout, eUnderline, eOverline, bUnderlineAbove );
    }
}

void OutputDevice::ImplDrawMnemonicLine( long nX, long nY, long nWidth )
{
    long nBaseX = nX;
    if( /*HasMirroredGraphics() &&*/ IsRTLEnabled() )
    {
        // --- RTL ---
        // add some strange offset
        nX += 2;
        // revert the hack that will be done later in ImplDrawTextLine
        nX = nBaseX - nWidth - (nX - nBaseX - 1);
    }

    ImplDrawTextLine( nX, nY, 0, nWidth, STRIKEOUT_NONE, UNDERLINE_SINGLE, UNDERLINE_NONE, false );
}

void OutputDevice::ImplGetEmphasisMark( PolyPolygon& rPolyPoly, bool& rPolyLine,
                                        Rectangle& rRect1, Rectangle& rRect2,
                                        long& rYOff, long& rWidth,
                                        FontEmphasisMark eEmphasis,
                                        long nHeight, short /*nOrient*/ )
{
    static const sal_uInt8 aAccentPolyFlags[24] =
    {
        0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 0, 2, 0, 2, 2
    };

    static const long aAccentPos[48] =
    {
         78,      0,
        348,     79,
        599,    235,
        843,    469,
        938,    574,
        990,    669,
        990,    773,
        990,    843,
        964,    895,
        921,    947,
        886,    982,
        860,    999,
        825,    999,
        764,    999,
        721,    964,
        686,    895,
        625,    791,
        556,    660,
        469,    504,
        400,    400,
        261,    252,
         61,     61,
          0,     27,
          9,      0
    };

    rWidth      = 0;
    rYOff       = 0;
    rPolyLine   = false;

    if ( !nHeight )
        return;

    FontEmphasisMark    nEmphasisStyle = eEmphasis & EMPHASISMARK_STYLE;
    long                nDotSize = 0;
    switch ( nEmphasisStyle )
    {
        case EMPHASISMARK_DOT:
            // Dot has 55% of the height
            nDotSize = (nHeight*550)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
            }
            rYOff = ((nHeight*250)/1000)/2; // Center to the another EmphasisMarks
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_CIRCLE:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
                // BorderWidth is 15%
                long nBorder = (nDotSize*150)/1000;
                if ( nBorder <= 1 )
                    rPolyLine = true;
                else
                {
                    Polygon aPoly2( Point( nRad, nRad ),
                                    nRad-nBorder, nRad-nBorder );
                    rPolyPoly.Insert( aPoly2 );
                }
            }
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_DISC:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
                rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
            else
            {
                long nRad = nDotSize/2;
                Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
            }
            rWidth = nDotSize;
            break;

        case EMPHASISMARK_ACCENT:
            // Dot has 80% of the height
            nDotSize = (nHeight*800)/1000;
            if ( !nDotSize )
                nDotSize = 1;
            if ( nDotSize <= 2 )
            {
                if ( nDotSize == 1 )
                {
                    rRect1 = Rectangle( Point(), Size( nDotSize, nDotSize ) );
                    rWidth = nDotSize;
                }
                else
                {
                    rRect1 = Rectangle( Point(), Size( 1, 1 ) );
                    rRect2 = Rectangle( Point( 1, 1 ), Size( 1, 1 ) );
                }
            }
            else
            {
                Polygon aPoly( sizeof( aAccentPos ) / sizeof( long ) / 2,
                               (const Point*)aAccentPos,
                               aAccentPolyFlags );
                double dScale = ((double)nDotSize)/1000.0;
                aPoly.Scale( dScale, dScale );
                Polygon aTemp;
                aPoly.AdaptiveSubdivide( aTemp );
                Rectangle aBoundRect = aTemp.GetBoundRect();
                rWidth = aBoundRect.GetWidth();
                nDotSize = aBoundRect.GetHeight();
                rPolyPoly.Insert( aTemp );
            }
            break;
    }

    // calculate position
    long nOffY = 1+(mnDPIY/300); // one visible pixel space
    long nSpaceY = nHeight-nDotSize;
    if ( nSpaceY >= nOffY*2 )
        rYOff += nOffY;
    if ( !(eEmphasis & EMPHASISMARK_POS_BELOW) )
        rYOff += nDotSize;
}

void OutputDevice::ImplDrawEmphasisMark( long nBaseX, long nX, long nY,
                                         const PolyPolygon& rPolyPoly, bool bPolyLine,
                                         const Rectangle& rRect1, const Rectangle& rRect2 )
{
    if( IsRTLEnabled() )
        // --- RTL --- mirror at basex
        nX = nBaseX - (nX - nBaseX - 1);

    nX -= mnOutOffX;
    nY -= mnOutOffY;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            DrawPolyLine( aPoly );
        }
        else
        {
            PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            DrawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        DrawRect( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        DrawRect( aRect );
    }
}

void OutputDevice::ImplDrawEmphasisMarks( SalLayout& rSalLayout )
{
    Color               aOldLineColor   = GetLineColor();
    Color               aOldFillColor   = GetFillColor();
    bool                bOldMap         = mbMap;
    GDIMetaFile*        pOldMetaFile    = mpMetaFile;
    mpMetaFile = NULL;
    EnableMapMode( false );

    FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
    PolyPolygon         aPolyPoly;
    Rectangle           aRect1;
    Rectangle           aRect2;
    long                nEmphasisYOff;
    long                nEmphasisWidth;
    long                nEmphasisHeight;
    bool                bPolyLine;

    if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
        nEmphasisHeight = mnEmphasisDescent;
    else
        nEmphasisHeight = mnEmphasisAscent;

    ImplGetEmphasisMark( aPolyPoly, bPolyLine,
                         aRect1, aRect2,
                         nEmphasisYOff, nEmphasisWidth,
                         nEmphasisMark,
                         nEmphasisHeight, mpFontEntry->mnOrientation );

    if ( bPolyLine )
    {
        SetLineColor( GetTextColor() );
        SetFillColor();
    }
    else
    {
        SetLineColor();
        SetFillColor( GetTextColor() );
    }

    Point aOffset = Point(0,0);

    if ( nEmphasisMark & EMPHASISMARK_POS_BELOW )
        aOffset.Y() += mpFontEntry->maMetric.mnDescent + nEmphasisYOff;
    else
        aOffset.Y() -= mpFontEntry->maMetric.mnAscent + nEmphasisYOff;

    long nEmphasisWidth2  = nEmphasisWidth / 2;
    long nEmphasisHeight2 = nEmphasisHeight / 2;
    aOffset += Point( nEmphasisWidth2, nEmphasisHeight2 );

    Point aOutPoint;
    Rectangle aRectangle;
    for( int nStart = 0;;)
    {
        sal_GlyphId aGlyphId;
        if( !rSalLayout.GetNextGlyphs( 1, &aGlyphId, aOutPoint, nStart ) )
            break;

        if( !mpGraphics->GetGlyphBoundRect( aGlyphId, aRectangle ) )
            continue;

        if( !rSalLayout.IsSpacingGlyph( aGlyphId ) )
        {
            Point aAdjPoint = aOffset;
            aAdjPoint.X() += aRectangle.Left() + (aRectangle.GetWidth() - nEmphasisWidth) / 2;
            if ( mpFontEntry->mnOrientation )
                ImplRotatePos( 0, 0, aAdjPoint.X(), aAdjPoint.Y(), mpFontEntry->mnOrientation );
            aOutPoint += aAdjPoint;
            aOutPoint -= Point( nEmphasisWidth2, nEmphasisHeight2 );
            ImplDrawEmphasisMark( rSalLayout.DrawBase().X(),
                                  aOutPoint.X(), aOutPoint.Y(),
                                  aPolyPoly, bPolyLine, aRect1, aRect2 );
        }
    }

    SetLineColor( aOldLineColor );
    SetFillColor( aOldFillColor );
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

bool OutputDevice::ImplDrawRotateText( SalLayout& rSalLayout )
{
    int nX = rSalLayout.DrawBase().X();
    int nY = rSalLayout.DrawBase().Y();

    Rectangle aBoundRect;
    rSalLayout.DrawBase() = Point( 0, 0 );
    rSalLayout.DrawOffset() = Point( 0, 0 );
    if( !rSalLayout.GetBoundRect( *mpGraphics, aBoundRect ) )
    {
        // guess vertical text extents if GetBoundRect failed
        int nRight = rSalLayout.GetTextWidth();
        int nTop = mpFontEntry->maMetric.mnAscent + mnEmphasisAscent;
        long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
        aBoundRect = Rectangle( 0, -nTop, nRight, nHeight - nTop );
    }

    // cache virtual device for rotation
    if ( !mpOutDevData )
        ImplInitOutDevData();
    if ( !mpOutDevData->mpRotateDev )
        mpOutDevData->mpRotateDev = new VirtualDevice( *this, 1 );
    VirtualDevice* pVDev = mpOutDevData->mpRotateDev;

    // size it accordingly
    if( !pVDev->SetOutputSizePixel( aBoundRect.GetSize() ) )
        return false;

    Font aFont( GetFont() );
    aFont.SetOrientation( 0 );
    aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
    pVDev->SetFont( aFont );
    pVDev->SetTextColor( Color( COL_BLACK ) );
    pVDev->SetTextFillColor();
    pVDev->ImplNewFont();
    pVDev->ImplInitFont();
    pVDev->ImplInitTextColor();

    // draw text into upper left corner
    rSalLayout.DrawBase() -= aBoundRect.TopLeft();
    rSalLayout.DrawText( *((OutputDevice*)pVDev)->mpGraphics );

    Bitmap aBmp = pVDev->GetBitmap( Point(), aBoundRect.GetSize() );
    if ( !aBmp || !aBmp.Rotate( mpFontEntry->mnOwnOrientation, COL_WHITE ) )
        return false;

    // calculate rotation offset
    Polygon aPoly( aBoundRect );
    aPoly.Rotate( Point(), mpFontEntry->mnOwnOrientation );
    Point aPoint = aPoly.GetBoundRect().TopLeft();
    aPoint += Point( nX, nY );

    // mask output with text colored bitmap
    GDIMetaFile* pOldMetaFile = mpMetaFile;
    long nOldOffX = mnOutOffX;
    long nOldOffY = mnOutOffY;
    bool bOldMap = mbMap;

    mnOutOffX   = 0L;
    mnOutOffY   = 0L;
    mpMetaFile  = NULL;
    EnableMapMode( false );

    DrawMask( aPoint, aBmp, GetTextColor() );

    EnableMapMode( bOldMap );
    mnOutOffX   = nOldOffX;
    mnOutOffY   = nOldOffY;
    mpMetaFile  = pOldMetaFile;

    return true;
}

bool OutputDevice::ImplDrawTextDirect( SalLayout& rSalLayout, bool bTextLines, sal_uInt32 flags )
{
    if( mpFontEntry->mnOwnOrientation )
        if( ImplDrawRotateText( rSalLayout ) )
            return true;

    long nOldX = rSalLayout.DrawBase().X();
    if( HasMirroredGraphics() )
    {
        long w = meOutDevType == OUTDEV_VIRDEV ? mnOutWidth : mpGraphics->GetGraphicsWidth();
        long x = rSalLayout.DrawBase().X();
           rSalLayout.DrawBase().X() = w - 1 - x;
        if( !IsRTLEnabled() )
        {
            OutputDevice *pOutDevRef = (OutputDevice *)this;
            // mirror this window back
            long devX = w-pOutDevRef->mnOutWidth-pOutDevRef->mnOutOffX;   // re-mirrored mnOutOffX
            rSalLayout.DrawBase().X() = devX + ( pOutDevRef->mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) ) ;
        }
    }
    else if( IsRTLEnabled() )
    {
        OutputDevice *pOutDevRef = (OutputDevice *)this;

        // mirror this window back
        long devX = pOutDevRef->mnOutOffX;   // re-mirrored mnOutOffX
        rSalLayout.DrawBase().X() = pOutDevRef->mnOutWidth - 1 - (rSalLayout.DrawBase().X() - devX) + devX;
    }

    if(flags)
    {
        if( ! rSalLayout.DrawTextSpecial( *mpGraphics, flags ))
        {
            rSalLayout.DrawBase().X() = nOldX;
            return false;
        }
    }
    else
    {
        rSalLayout.DrawText( *mpGraphics );
    }
    rSalLayout.DrawBase().X() = nOldX;

    if( bTextLines )
        ImplDrawTextLines( rSalLayout,
            maFont.GetStrikeout(), maFont.GetUnderline(), maFont.GetOverline(),
            maFont.IsWordLineMode(), ImplIsUnderlineAbove( maFont ) );

    // emphasis marks
    if( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
        ImplDrawEmphasisMarks( rSalLayout );

    return true;
}

void OutputDevice::ImplDrawSpecialText( SalLayout& rSalLayout )
{
    Color       aOldColor           = GetTextColor();
    Color       aOldTextLineColor   = GetTextLineColor();
    Color       aOldOverlineColor   = GetOverlineColor();
    FontRelief  eRelief             = maFont.GetRelief();

    Point aOrigPos = rSalLayout.DrawBase();
    if ( eRelief != RELIEF_NONE )
    {
        Color   aReliefColor( COL_LIGHTGRAY );
        Color   aTextColor( aOldColor );

        Color   aTextLineColor( aOldTextLineColor );
        Color   aOverlineColor( aOldOverlineColor );

        // we don't have a automatic color, so black is always drawn on white
        if ( aTextColor.GetColor() == COL_BLACK )
            aTextColor = Color( COL_WHITE );
        if ( aTextLineColor.GetColor() == COL_BLACK )
            aTextLineColor = Color( COL_WHITE );
        if ( aOverlineColor.GetColor() == COL_BLACK )
            aOverlineColor = Color( COL_WHITE );

        // relief-color is black for white text, in all other cases
        // we set this to LightGray
        if ( aTextColor.GetColor() == COL_WHITE )
            aReliefColor = Color( COL_BLACK );
        SetTextLineColor( aReliefColor );
        SetOverlineColor( aReliefColor );
        SetTextColor( aReliefColor );
        ImplInitTextColor();

        // calculate offset - for high resolution printers the offset
        // should be greater so that the effect is visible
        long nOff = 1;
        nOff += mnDPIX/300;

        if ( eRelief == RELIEF_ENGRAVED )
            nOff = -nOff;
        rSalLayout.DrawOffset() += Point( nOff, nOff);
        ImplDrawTextDirect( rSalLayout, mbTextLines );
        rSalLayout.DrawOffset() -= Point( nOff, nOff);

        SetTextLineColor( aTextLineColor );
        SetOverlineColor( aOverlineColor );
        SetTextColor( aTextColor );
        ImplInitTextColor();
        ImplDrawTextDirect( rSalLayout, mbTextLines );

        SetTextLineColor( aOldTextLineColor );
        SetOverlineColor( aOldOverlineColor );

        if ( aTextColor != aOldColor )
        {
            SetTextColor( aOldColor );
            ImplInitTextColor();
        }
    }
    else
    {
        if ( maFont.IsShadow() )
        {
            long nOff = 1 + ((mpFontEntry->mnLineHeight-24)/24);
            if ( maFont.IsOutline() )
                nOff++;
            SetTextLineColor();
            SetOverlineColor();
            if ( (GetTextColor().GetColor() == COL_BLACK)
            ||   (GetTextColor().GetLuminance() < 8) )
                SetTextColor( Color( COL_LIGHTGRAY ) );
            else
                SetTextColor( Color( COL_BLACK ) );
            ImplInitTextColor();
            rSalLayout.DrawBase() += Point( nOff, nOff );
            ImplDrawTextDirect( rSalLayout, mbTextLines );
            rSalLayout.DrawBase() -= Point( nOff, nOff );
            SetTextColor( aOldColor );
            SetTextLineColor( aOldTextLineColor );
            SetOverlineColor( aOldOverlineColor );
            ImplInitTextColor();

            if ( !maFont.IsOutline() )
                ImplDrawTextDirect( rSalLayout, mbTextLines );
        }

        if ( maFont.IsOutline() )
        {
            if(! ImplDrawTextDirect( rSalLayout, mbTextLines, DRAWTEXT_F_OUTLINE))
            {
                rSalLayout.DrawBase() = aOrigPos + Point(-1,-1);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(+1,+1);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(-1,+0);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(-1,+1);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(+0,+1);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(+0,-1);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(+1,-1);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos + Point(+1,+0);
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                rSalLayout.DrawBase() = aOrigPos;

                SetTextColor( Color( COL_WHITE ) );
                SetTextLineColor( Color( COL_WHITE ) );
                SetOverlineColor( Color( COL_WHITE ) );
                ImplInitTextColor();
                ImplDrawTextDirect( rSalLayout, mbTextLines );
                SetTextColor( aOldColor );
                SetTextLineColor( aOldTextLineColor );
                SetOverlineColor( aOldOverlineColor );
                ImplInitTextColor();
            }
        }
    }
}

void OutputDevice::ImplDrawText( SalLayout& rSalLayout )
{
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;
    if( mbInitTextColor )
        ImplInitTextColor();

    rSalLayout.DrawBase() += Point( mnTextOffX, mnTextOffY );

    if( IsTextFillColor() )
        ImplDrawTextBackground( rSalLayout );

    if( mbTextSpecial )
        ImplDrawSpecialText( rSalLayout );
    else
        ImplDrawTextDirect( rSalLayout, mbTextLines );
}

long OutputDevice::ImplGetTextLines( ImplMultiTextLineInfo& rLineInfo,
                                     long nWidth, const OUString& rStr,
                                     sal_uInt16 nStyle, const ::vcl::ITextLayout& _rLayout )
{
    DBG_ASSERTWARNING( nWidth >= 0, "ImplGetTextLines: nWidth <= 0!" );

    if ( nWidth <= 0 )
        nWidth = 1;

    long nMaxLineWidth  = 0;
    rLineInfo.Clear();
    if ( !rStr.isEmpty() && (nWidth > 0) )
    {
        uno::Reference < i18n::XBreakIterator > xBI;
        // get service provider
        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        bool bHyphenate = (nStyle & TEXT_DRAW_WORDBREAK_HYPHENATION)
            == TEXT_DRAW_WORDBREAK_HYPHENATION;
        uno::Reference< linguistic2::XHyphenator > xHyph;
        if ( bHyphenate )
        {
            uno::Reference< linguistic2::XLinguServiceManager2> xLinguMgr = linguistic2::LinguServiceManager::create(xContext);
            xHyph = xLinguMgr->getHyphenator();
        }

        sal_Int32 nPos = 0;
        sal_Int32 nLen = rStr.getLength();
        while ( nPos < nLen )
        {
            sal_Int32 nBreakPos = nPos;

            while ( ( nBreakPos < nLen ) && ( rStr[ nBreakPos ] != '\r' ) && ( rStr[ nBreakPos ] != '\n' ) )
                nBreakPos++;

            long nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
            if ( ( nLineWidth > nWidth ) && ( nStyle & TEXT_DRAW_WORDBREAK ) )
            {
                if ( !xBI.is() )
                    xBI = vcl::unohelper::CreateBreakIterator();

                if ( xBI.is() )
                {
                    const com::sun::star::lang::Locale& rDefLocale(Application::GetSettings().GetUILanguageTag().getLocale());
                    sal_Int32 nSoftBreak = _rLayout.GetTextBreak( rStr, nWidth, nPos, nBreakPos - nPos );
                    DBG_ASSERT( nSoftBreak < nBreakPos, "Break?!" );
                    i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, uno::Sequence <beans::PropertyValue>(), 1 );
                    i18n::LineBreakUserOptions aUserOptions;
                    i18n::LineBreakResults aLBR = xBI->getLineBreak( rStr, nSoftBreak, rDefLocale, nPos, aHyphOptions, aUserOptions );
                    nBreakPos = aLBR.breakIndex;
                    if ( nBreakPos <= nPos )
                        nBreakPos = nSoftBreak;
                    if ( bHyphenate )
                    {
                        // Whether hyphen or not: Put the word after the hyphen through
                        // word boundary.

                        // nMaxBreakPos the last char that fits into the line
                        // nBreakPos is the word's start

                        // We run into a problem if the doc is so narrow, that a word
                        // is broken into more than two lines ...
                        if ( xHyph.is() )
                        {
                            sal_Unicode cAlternateReplChar = 0;
                            i18n::Boundary aBoundary = xBI->getWordBoundary( rStr, nBreakPos, rDefLocale, ::com::sun::star::i18n::WordType::DICTIONARY_WORD, sal_True );
                            sal_Int32 nWordStart = nPos;
                            sal_Int32 nWordEnd = aBoundary.endPos;
                            DBG_ASSERT( nWordEnd > nWordStart, "ImpBreakLine: Start >= End?" );

                            sal_Int32 nWordLen = nWordEnd - nWordStart;
                            if ( ( nWordEnd >= nSoftBreak ) && ( nWordLen > 3 ) )
                            {
                                // #104415# May happen, because getLineBreak may differ from getWordBoudary with DICTIONARY_WORD
                                // DBG_ASSERT( nWordEnd >= nMaxBreakPos, "Hyph: Break?" );
                                OUString aWord = rStr.copy( nWordStart, nWordLen );
                                sal_Int32 nMinTrail = nWordEnd-nSoftBreak+1;  //+1: Before the "broken off" char
                                uno::Reference< linguistic2::XHyphenatedWord > xHyphWord;
                                if (xHyph.is())
                                    xHyphWord = xHyph->hyphenate( aWord, rDefLocale, aWord.getLength() - nMinTrail, uno::Sequence< beans::PropertyValue >() );
                                if (xHyphWord.is())
                                {
                                    bool bAlternate = xHyphWord->isAlternativeSpelling();
                                    sal_Int32 _nWordLen = 1 + xHyphWord->getHyphenPos();

                                    if ( ( _nWordLen >= 2 ) && ( (nWordStart+_nWordLen) >= ( 2 ) ) )
                                    {
                                        if ( !bAlternate )
                                        {
                                            nBreakPos = nWordStart + _nWordLen;
                                        }
                                        else
                                        {
                                            OUString aAlt( xHyphWord->getHyphenatedWord() );

                                            // We can have two cases:
                                            // 1) "packen" turns into "pak-ken"
                                            // 2) "Schiffahrt" turns into "Schiff-fahrt"

                                            // In case 1 we need to replace a char
                                            // In case 2 we add a char

                                            // Correct recognition is made harder by words such as
                                            // "Schiffahrtsbrennesseln", as the Hyphenator splits all
                                            // positions of the word and comes up with "Schifffahrtsbrennnesseln"
                                            // Thus, we cannot infer the aWord from the AlternativWord's
                                            // index.
                                            // TODO: The whole junk will be made easier by a function in
                                            // the Hyphenator, as soon as AMA adds it.
                                            sal_Int32 nAltStart = _nWordLen - 1;
                                            sal_Int32 nTxtStart = nAltStart - (aAlt.getLength() - aWord.getLength());
                                            sal_Int32 nTxtEnd = nTxtStart;
                                            sal_Int32 nAltEnd = nAltStart;

                                            // The area between nStart and nEnd is the difference
                                            // between AlternativString and OriginalString
                                            while( nTxtEnd < aWord.getLength() && nAltEnd < aAlt.getLength() &&
                                                   aWord[nTxtEnd] != aAlt[nAltEnd] )
                                            {
                                                ++nTxtEnd;
                                                ++nAltEnd;
                                            }

                                            // If a char was added, we notice it now:
                                            if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
                                                aWord[ nTxtEnd ] == aAlt[nAltEnd] )
                                            {
                                                ++nAltEnd;
                                                ++nTxtStart;
                                                ++nTxtEnd;
                                            }

                                            DBG_ASSERT( ( nAltEnd - nAltStart ) == 1, "Alternate: Wrong assumption!" );

                                            if ( nTxtEnd > nTxtStart )
                                                cAlternateReplChar = aAlt[ nAltStart ];

                                            nBreakPos = nWordStart + nTxtStart;
                                            if ( cAlternateReplChar )
                                                nBreakPos++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
                }
                else
                {
                    // fallback to something really simple
                    sal_Int32 nSpacePos = rStr.getLength();
                    long nW = 0;
                    do
                    {
                        nSpacePos = rStr.lastIndexOf( ' ', nSpacePos );
                        if( nSpacePos != -1 )
                        {
                            if( nSpacePos > nPos )
                                nSpacePos--;
                            nW = _rLayout.GetTextWidth( rStr, nPos, nSpacePos-nPos );
                        }
                    } while( nW > nWidth );

                    if( nSpacePos != -1 )
                    {
                        nBreakPos = nSpacePos;
                        nLineWidth = _rLayout.GetTextWidth( rStr, nPos, nBreakPos-nPos );
                        if( nBreakPos < rStr.getLength()-1 )
                            nBreakPos++;
                    }
                }
            }

            if ( nLineWidth > nMaxLineWidth )
                nMaxLineWidth = nLineWidth;

            rLineInfo.AddLine( new ImplTextLineInfo( nLineWidth, nPos, nBreakPos-nPos ) );

            if ( nBreakPos == nPos )
                nBreakPos++;
            nPos = nBreakPos;

            if ( nPos < nLen && ( ( rStr[ nPos ] == '\r' ) || ( rStr[ nPos ] == '\n' ) ) )
            {
                nPos++;
                // CR/LF?
                if ( ( nPos < nLen ) && ( rStr[ nPos ] == '\n' ) && ( rStr[ nPos-1 ] == '\r' ) )
                    nPos++;
            }
        }
    }
#ifdef DBG_UTIL
    for ( sal_uInt16 nL = 0; nL < rLineInfo.Count(); nL++ )
    {
        ImplTextLineInfo* pLine = rLineInfo.GetLine( nL );
        OUString aLine = rStr.copy( pLine->GetIndex(), pLine->GetLen() );
        DBG_ASSERT( aLine.indexOf( '\r' ) == -1, "ImplGetTextLines - Found CR!" );
        DBG_ASSERT( aLine.indexOf( '\n' ) == -1, "ImplGetTextLines - Found LF!" );
    }
#endif

    return nMaxLineWidth;
}

void OutputDevice::SetAntialiasing( sal_uInt16 nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = true;

        if(mpGraphics)
        {
            mpGraphics->setAntiAliasB2DDraw(mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW);
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetAntialiasing( nMode );
}

void OutputDevice::SetFont( const Font& rNewFont )
{

    Font aFont( rNewFont );
    aFont.SetLanguage(rNewFont.GetLanguage());
    if ( mnDrawMode & (DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT | DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT | DRAWMODE_SETTINGSTEXT |
                       DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL | DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                       DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
    {
        Color aTextColor( aFont.GetColor() );

        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aTextColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aTextColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const sal_uInt8 cLum = aTextColor.GetLuminance();
            aTextColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aTextColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aTextColor = Color( (aTextColor.GetRed() >> 1 ) | 0x80,
                                (aTextColor.GetGreen() >> 1 ) | 0x80,
                                (aTextColor.GetBlue() >> 1 ) | 0x80 );
        }

        aFont.SetColor( aTextColor );

        bool bTransFill = aFont.IsTransparent();
        if ( !bTransFill )
        {
            Color aTextFillColor( aFont.GetFillColor() );

            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aTextFillColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aTextFillColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const sal_uInt8 cLum = aTextFillColor.GetLuminance();
                aTextFillColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
                aTextFillColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aTextFillColor = Color( COL_TRANSPARENT );
                bTransFill = true;
            }

            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aTextFillColor = Color( (aTextFillColor.GetRed() >> 1) | 0x80,
                                        (aTextFillColor.GetGreen() >> 1) | 0x80,
                                        (aTextFillColor.GetBlue() >> 1) | 0x80 );
            }

            aFont.SetFillColor( aTextFillColor );
        }
    }

    if ( mpMetaFile )
    {
        mpMetaFile->AddAction( new MetaFontAction( aFont ) );
        // the color and alignment actions don't belong here
        // TODO: get rid of them without breaking anything...
        mpMetaFile->AddAction( new MetaTextAlignAction( aFont.GetAlign() ) );
        mpMetaFile->AddAction( new MetaTextFillColorAction( aFont.GetFillColor(), !aFont.IsTransparent() ) );
    }

    if ( !maFont.IsSameInstance( aFont ) )
    {
        // Optimization MT/HDU: COL_TRANSPARENT means SetFont should ignore the font color,
        // because SetTextColor() is used for this.
        // #i28759# maTextColor might have been changed behind our back, commit then, too.
        if( aFont.GetColor() != COL_TRANSPARENT
        && (aFont.GetColor() != maFont.GetColor() || aFont.GetColor() != maTextColor ) )
        {
            maTextColor = aFont.GetColor();
            mbInitTextColor = true;
            if( mpMetaFile )
                mpMetaFile->AddAction( new MetaTextColorAction( aFont.GetColor() ) );
        }
        maFont      = aFont;
        mbNewFont   = true;

        if( mpAlphaVDev )
        {
            // #i30463#
            // Since SetFont might change the text color, apply that only
            // selectively to alpha vdev (which normally paints opaque text
            // with COL_BLACK)
            if( aFont.GetColor() != COL_TRANSPARENT )
            {
                mpAlphaVDev->SetTextColor( COL_BLACK );
                aFont.SetColor( COL_TRANSPARENT );
            }

            mpAlphaVDev->SetFont( aFont );
        }
    }
}

void OutputDevice::SetLayoutMode( sal_uLong nTextLayoutMode )
{
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaLayoutModeAction( nTextLayoutMode ) );

    mnTextLayoutMode = nTextLayoutMode;

    if( mpAlphaVDev )
        mpAlphaVDev->SetLayoutMode( nTextLayoutMode );
}

void OutputDevice::SetDigitLanguage( LanguageType eTextLanguage )
{
    if( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLanguageAction( eTextLanguage ) );

    meTextLanguage = eTextLanguage;

    if( mpAlphaVDev )
        mpAlphaVDev->SetDigitLanguage( eTextLanguage );
}

void OutputDevice::SetTextColor( const Color& rColor )
{

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT |
                        DRAWMODE_SETTINGSTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aColor = GetSettings().GetStyleSettings().GetFontColor();

        if ( mnDrawMode & DRAWMODE_GHOSTEDTEXT )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextColorAction( aColor ) );

    if ( maTextColor != aColor )
    {
        maTextColor = aColor;
        mbInitTextColor = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextColor( COL_BLACK );
}

void OutputDevice::SetTextFillColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( Color(), false ) );

    if ( maFont.GetColor() != Color( COL_TRANSPARENT ) )
        maFont.SetFillColor( Color( COL_TRANSPARENT ) );
    if ( !maFont.IsTransparent() )
        maFont.SetTransparent( true );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor();
}

void OutputDevice::SetTextFillColor( const Color& rColor )
{

    Color aColor( rColor );
    bool bTransFill = ImplIsColorTransparent( aColor ) ? sal_True : sal_False;

    if ( !bTransFill )
    {
        if ( mnDrawMode & ( DRAWMODE_BLACKFILL | DRAWMODE_WHITEFILL |
                            DRAWMODE_GRAYFILL | DRAWMODE_NOFILL |
                            DRAWMODE_GHOSTEDFILL | DRAWMODE_SETTINGSFILL ) )
        {
            if ( mnDrawMode & DRAWMODE_BLACKFILL )
                aColor = Color( COL_BLACK );
            else if ( mnDrawMode & DRAWMODE_WHITEFILL )
                aColor = Color( COL_WHITE );
            else if ( mnDrawMode & DRAWMODE_GRAYFILL )
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color( cLum, cLum, cLum );
            }
            else if( mnDrawMode & DRAWMODE_SETTINGSFILL )
                aColor = GetSettings().GetStyleSettings().GetWindowColor();
            else if ( mnDrawMode & DRAWMODE_NOFILL )
            {
                aColor = Color( COL_TRANSPARENT );
                bTransFill = true;
            }

            if ( !bTransFill && (mnDrawMode & DRAWMODE_GHOSTEDFILL) )
            {
                aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                                (aColor.GetGreen() >> 1) | 0x80,
                                (aColor.GetBlue() >> 1) | 0x80 );
            }
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( aColor, true ) );

    if ( maFont.GetFillColor() != aColor )
        maFont.SetFillColor( aColor );
    if ( maFont.IsTransparent() != bTransFill )
        maFont.SetTransparent( bTransFill );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor( COL_BLACK );
}

Color OutputDevice::GetTextFillColor() const
{
    if ( maFont.IsTransparent() )
        return Color( COL_TRANSPARENT );
    else
        return maFont.GetFillColor();
}

void OutputDevice::SetTextLineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( Color(), false ) );

    maTextLineColor = Color( COL_TRANSPARENT );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor();
}

void OutputDevice::SetTextLineColor( const Color& rColor )
{

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT |
                        DRAWMODE_SETTINGSTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aColor = GetSettings().GetStyleSettings().GetFontColor();

        if( (mnDrawMode & DRAWMODE_GHOSTEDTEXT)
        &&  (aColor.GetColor() != COL_TRANSPARENT) )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( aColor, true ) );

    maTextLineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor( COL_BLACK );
}

void OutputDevice::SetOverlineColor()
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( Color(), false ) );

    maOverlineColor = Color( COL_TRANSPARENT );

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor();
}

void OutputDevice::SetOverlineColor( const Color& rColor )
{

    Color aColor( rColor );

    if ( mnDrawMode & ( DRAWMODE_BLACKTEXT | DRAWMODE_WHITETEXT |
                        DRAWMODE_GRAYTEXT | DRAWMODE_GHOSTEDTEXT |
                        DRAWMODE_SETTINGSTEXT ) )
    {
        if ( mnDrawMode & DRAWMODE_BLACKTEXT )
            aColor = Color( COL_BLACK );
        else if ( mnDrawMode & DRAWMODE_WHITETEXT )
            aColor = Color( COL_WHITE );
        else if ( mnDrawMode & DRAWMODE_GRAYTEXT )
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color( cLum, cLum, cLum );
        }
        else if ( mnDrawMode & DRAWMODE_SETTINGSTEXT )
            aColor = GetSettings().GetStyleSettings().GetFontColor();

        if( (mnDrawMode & DRAWMODE_GHOSTEDTEXT)
        &&  (aColor.GetColor() != COL_TRANSPARENT) )
        {
            aColor = Color( (aColor.GetRed() >> 1) | 0x80,
                            (aColor.GetGreen() >> 1) | 0x80,
                            (aColor.GetBlue() >> 1) | 0x80 );
        }
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( aColor, true ) );

    maOverlineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor( COL_BLACK );
}

void OutputDevice::SetTextAlign( TextAlign eAlign )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAlignAction( eAlign ) );

    if ( maFont.GetAlign() != eAlign )
    {
        maFont.SetAlign( eAlign );
        mbNewFont = true;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextAlign( eAlign );
}

void OutputDevice::DrawTextLine( const Point& rPos, long nWidth,
                                 FontStrikeout eStrikeout,
                                 FontUnderline eUnderline,
                                 FontUnderline eOverline,
                                 bool bUnderlineAbove )
{

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineAction( rPos, nWidth, eStrikeout, eUnderline, eOverline ) );

    if ( ((eUnderline == UNDERLINE_NONE) || (eUnderline == UNDERLINE_DONTKNOW)) &&
         ((eOverline  == UNDERLINE_NONE) || (eOverline  == UNDERLINE_DONTKNOW)) &&
         ((eStrikeout == STRIKEOUT_NONE) || (eStrikeout == STRIKEOUT_DONTKNOW)) )
        return;

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;

    // initialize font if needed to get text offsets
    // TODO: only needed for mnTextOff!=(0,0)
    if( mbNewFont )
        if( !ImplNewFont() )
            return;
    if( mbInitFont )
        ImplInitFont();

    Point aPos = ImplLogicToDevicePixel( rPos );
    nWidth = ImplLogicWidthToDevicePixel( nWidth );
    aPos += Point( mnTextOffX, mnTextOffY );
    ImplDrawTextLine( aPos.X(), aPos.X(), 0, nWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );

    if( mpAlphaVDev )
        mpAlphaVDev->DrawTextLine( rPos, nWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );
}

void OutputDevice::DrawWaveLine( const Point& rStartPos, const Point& rEndPos )
{

    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if( mbNewFont )
        if( !ImplNewFont() )
            return;

    Point   aStartPt = ImplLogicToDevicePixel( rStartPos );
    Point   aEndPt = ImplLogicToDevicePixel( rEndPos );
    long    nStartX = aStartPt.X();
    long    nStartY = aStartPt.Y();
    long    nEndX = aEndPt.X();
    long    nEndY = aEndPt.Y();
    short   nOrientation = 0;

    // when rotated
    if ( (nStartY != nEndY) || (nStartX > nEndX) )
    {
        long nDX = nEndX - nStartX;
        double nO = atan2( -nEndY + nStartY, ((nDX == 0L) ? 0.000000001 : nDX) );
        nO /= F_PI1800;
        nOrientation = (short)nO;
        ImplRotatePos( nStartX, nStartY, nEndX, nEndY, -nOrientation );
    }

    long nWaveHeight;

    nWaveHeight = 3;
    nStartY++;
    nEndY++;

    if (mnDPIScaleFactor > 1)
    {
        nWaveHeight *= mnDPIScaleFactor;

        nStartY += mnDPIScaleFactor - 1; // Shift down additional pixel(s) to create more visual separation.

        // odd heights look better than even
        if (mnDPIScaleFactor % 2 == 0)
        {
            nWaveHeight--;
        }
    }

    // #109280# make sure the waveline does not exceed the descent to avoid paint problems
    ImplFontEntry* pFontEntry = mpFontEntry;
    if( nWaveHeight > pFontEntry->maMetric.mnWUnderlineSize )
        nWaveHeight = pFontEntry->maMetric.mnWUnderlineSize;

    ImplDrawWaveLine(nStartX, nStartY, 0, 0,
            nEndX-nStartX, nWaveHeight,
            mnDPIScaleFactor, nOrientation, GetLineColor());

    if( mpAlphaVDev )
        mpAlphaVDev->DrawWaveLine( rStartPos, rEndPos );
}

void OutputDevice::DrawText( const Point& rStartPt, const OUString& rStr,
                             sal_Int32 nIndex, sal_Int32 nLen,
                             MetricVector* pVector, OUString* pDisplayText
                             )
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutlines Suspicious arguments nLen:" << nLen);
    }
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if( mpOutDevData && mpOutDevData->mpRecordLayout )
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "   OutputDevice::DrawText(\"%s\")\n",
         OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );
    if( pVector )
    {
        Region aClip( GetClipRegion() );
        if( meOutDevType == OUTDEV_WINDOW )
            aClip.Intersect( Rectangle( Point(), GetOutputSize() ) );
        if( mpOutDevData && mpOutDevData->mpRecordLayout )
        {
            mpOutDevData->mpRecordLayout->m_aLineIndices.push_back( mpOutDevData->mpRecordLayout->m_aDisplayText.getLength() );
            aClip.Intersect( mpOutDevData->maRecordRect );
        }
        if( ! aClip.IsNull() )
        {
            MetricVector aTmp;
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, nIndex, aTmp );

            bool bInserted = false;
            for( MetricVector::const_iterator it = aTmp.begin(); it != aTmp.end(); ++it, nIndex++ )
            {
                bool bAppend = false;

                if( aClip.IsOver( *it ) )
                    bAppend = true;
                else if( rStr[ nIndex ] == ' ' && bInserted )
                {
                    MetricVector::const_iterator next = it;
                    ++next;
                    if( next != aTmp.end() && aClip.IsOver( *next ) )
                        bAppend = true;
                }

                if( bAppend )
                {
                    pVector->push_back( *it );
                    if( pDisplayText )
                        *pDisplayText += OUString(rStr[ nIndex ]);
                    bInserted = true;
                }
            }
        }
        else
        {
            GetGlyphBoundRects( rStartPt, rStr, nIndex, nLen, nIndex, *pVector );
            if( pDisplayText )
                *pDisplayText += rStr.copy( nIndex, nLen );
        }
    }

    if ( !IsDeviceOutputNecessary() || pVector )
        return;

    SalLayout* pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, NULL);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawText( rStartPt, rStr, nIndex, nLen, pVector, pDisplayText );
}

long OutputDevice::GetTextWidth( const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen ) const
{

    long nWidth = GetTextArray( rStr, NULL, nIndex, nLen );

    return nWidth;
}

long OutputDevice::GetTextHeight() const
{

    if( mbNewFont )
        if( !ImplNewFont() )
            return 0;
    if( mbInitFont )
        if( !ImplNewFont() )
            return 0;

    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;

    if ( mbMap )
        nHeight = ImplDevicePixelToLogicHeight( nHeight );

    return nHeight;
}

float OutputDevice::approximate_char_width() const
{
    return GetTextWidth("aemnnxEM") / 8.0;
}

void OutputDevice::DrawTextArray( const Point& rStartPt, const OUString& rStr,
                                  const sal_Int32* pDXAry,
                                  sal_Int32 nIndex, sal_Int32 nLen )
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "DrawTextArray Suspicious arguments nLen:" << nLen);
    }
    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }
    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped )
        return;

    SalLayout* pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, pDXAry);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen );
}

long OutputDevice::GetTextArray( const OUString& rStr, sal_Int32* pDXAry,
                                 sal_Int32 nIndex, sal_Int32 nLen ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextArray Suspicious arguments nLen:" << nLen);
    }

    if( nIndex >= rStr.getLength() )
        return 0;

    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }
    // do layout
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    if( !pSalLayout )
        return 0;

    long nWidth = pSalLayout->FillDXArray( pDXAry );
    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->Release();

    // convert virtual char widths to virtual absolute positions
    if( pDXAry )
        for( int i = 1; i < nLen; ++i )
            pDXAry[ i ] += pDXAry[ i-1 ];

    // convert from font units to logical units
    if( mbMap )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                pDXAry[i] = ImplDevicePixelToLogicWidth( pDXAry[i] );
        nWidth = ImplDevicePixelToLogicWidth( nWidth );
    }

    if( nWidthFactor > 1 )
    {
        if( pDXAry )
            for( int i = 0; i < nLen; ++i )
                pDXAry[i] /= nWidthFactor;
        nWidth /= nWidthFactor;
    }

    return nWidth;
}

bool OutputDevice::GetCaretPositions( const OUString& rStr, sal_Int32* pCaretXArray,
    sal_Int32 nIndex, sal_Int32 nLen,
    sal_Int32* pDXAry, long nLayoutWidth,
    bool bCellBreaking ) const
{

    if( nIndex >= rStr.getLength() )
        return false;
    if( nIndex+nLen >= rStr.getLength() )
        nLen = rStr.getLength() - nIndex;

    // layout complex text
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen,
        Point(0,0), nLayoutWidth, pDXAry );
    if( !pSalLayout )
        return false;

    int nWidthFactor = pSalLayout->GetUnitsPerPixel();
    pSalLayout->GetCaretPositions( 2*nLen, pCaretXArray );
    long nWidth = pSalLayout->GetTextWidth();
    pSalLayout->Release();

    // fixup unknown caret positions
    int i;
    for( i = 0; i < 2 * nLen; ++i )
        if( pCaretXArray[ i ] >= 0 )
            break;
    long nXPos = pCaretXArray[ i ];
    for( i = 0; i < 2 * nLen; ++i )
    {
        if( pCaretXArray[ i ] >= 0 )
            nXPos = pCaretXArray[ i ];
        else
            pCaretXArray[ i ] = nXPos;
    }

    // handle window mirroring
    if( IsRTLEnabled() )
    {
        for( i = 0; i < 2 * nLen; ++i )
            pCaretXArray[i] = nWidth - pCaretXArray[i] - 1;
    }

    // convert from font units to logical units
    if( mbMap )
    {
        for( i = 0; i < 2*nLen; ++i )
            pCaretXArray[i] = ImplDevicePixelToLogicWidth( pCaretXArray[i] );
    }

    if( nWidthFactor != 1 )
    {
        for( i = 0; i < 2*nLen; ++i )
            pCaretXArray[i] /= nWidthFactor;
    }

    // if requested move caret position to cell limits
    if( bCellBreaking )
    {
        ; // FIXME
    }

    return true;
}

void OutputDevice::DrawStretchText( const Point& rStartPt, sal_uLong nWidth,
                                    const OUString& rStr,
                                    sal_Int32 nIndex, sal_Int32 nLen )
{
    if(nIndex < 0 || nIndex == 0x0FFFF || nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "DrawStretchText Suspicious arguments nIndex:" << nIndex << " nLen:" << nLen);
    }
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    SalLayout* pSalLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, nWidth, NULL);
    if( pSalLayout )
    {
        ImplDrawText( *pSalLayout );
        pSalLayout->Release();
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

ImplLayoutArgs OutputDevice::ImplPrepareLayoutArgs( OUString& rStr,
                                       const sal_Int32 nMinIndex, const sal_Int32 nLen,
                                       long nPixelWidth, const sal_Int32* pDXArray ) const
{
    assert(nMinIndex >= 0);
    assert(nLen >= 0);

    // get string length for calculating extents
    sal_Int32 nEndIndex = rStr.getLength();
    if( nMinIndex + nLen < nEndIndex )
        nEndIndex = nMinIndex + nLen;

    // don't bother if there is nothing to do
    if( nEndIndex < nMinIndex )
        nEndIndex = nMinIndex;

    int nLayoutFlags = 0;
    if( mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL )
        nLayoutFlags |= SAL_LAYOUT_BIDI_RTL;
    if( mnTextLayoutMode & TEXT_LAYOUT_BIDI_STRONG )
        nLayoutFlags |= SAL_LAYOUT_BIDI_STRONG;
    else if( 0 == (mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL) )
    {
        // disable Bidi if no RTL hint and no RTL codes used
        const sal_Unicode* pStr = rStr.getStr() + nMinIndex;
        const sal_Unicode* pEnd = rStr.getStr() + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
            if( ((*pStr >= 0x0580) && (*pStr < 0x0800))   // middle eastern scripts
            ||  ((*pStr >= 0xFB18) && (*pStr < 0xFE00))   // hebrew + arabic A presentation forms
            ||  ((*pStr >= 0xFE70) && (*pStr < 0xFEFF)) ) // arabic presentation forms B
                break;
        if( pStr >= pEnd )
            nLayoutFlags |= SAL_LAYOUT_BIDI_STRONG;
    }

    if( mbKerning )
        nLayoutFlags |= SAL_LAYOUT_KERNING_PAIRS;
    if( maFont.GetKerning() & KERNING_ASIAN )
        nLayoutFlags |= SAL_LAYOUT_KERNING_ASIAN;
    if( maFont.IsVertical() )
        nLayoutFlags |= SAL_LAYOUT_VERTICAL;

    if( mnTextLayoutMode & TEXT_LAYOUT_ENABLE_LIGATURES )
        nLayoutFlags |= SAL_LAYOUT_ENABLE_LIGATURES;
    else if( mnTextLayoutMode & TEXT_LAYOUT_COMPLEX_DISABLED )
        nLayoutFlags |= SAL_LAYOUT_COMPLEX_DISABLED;
    else
    {
        // disable CTL for non-CTL text
        const sal_Unicode* pStr = rStr.getStr() + nMinIndex;
        const sal_Unicode* pEnd = rStr.getStr() + nEndIndex;
        for( ; pStr < pEnd; ++pStr )
            if( ((*pStr >= 0x0300) && (*pStr < 0x0370))   // diacritical marks
            ||  ((*pStr >= 0x0590) && (*pStr < 0x10A0))   // many CTL scripts
            ||  ((*pStr >= 0x1100) && (*pStr < 0x1200))   // hangul jamo
            ||  ((*pStr >= 0x1700) && (*pStr < 0x1900))   // many CTL scripts
            ||  ((*pStr >= 0xFB1D) && (*pStr < 0xFE00))   // middle east presentation
            ||  ((*pStr >= 0xFE70) && (*pStr < 0xFEFF))   // arabic presentation B
            ||  ((*pStr >= 0xFE00) && (*pStr < 0xFE10))   // variation selectors in BMP
            ||  ((pStr + 1 < pEnd) && (pStr[0] == 0xDB40) && (0xDD00 <= pStr[1]) && (pStr[1] < 0xDEF0)) // variation selector supplement
            )
                break;
        if( pStr >= pEnd )
            nLayoutFlags |= SAL_LAYOUT_COMPLEX_DISABLED;
    }

    if( meTextLanguage ) //TODO: (mnTextLayoutMode & TEXT_LAYOUT_SUBSTITUTE_DIGITS)
    {
        // disable character localization when no digits used
        const sal_Unicode* pBase = rStr.getStr();
        const sal_Unicode* pStr = pBase + nMinIndex;
        const sal_Unicode* pEnd = pBase + nEndIndex;
        OUStringBuffer sTmpStr(rStr);
        for( ; pStr < pEnd; ++pStr )
        {
            // TODO: are there non-digit localizations?
            if( (*pStr >= '0') && (*pStr <= '9') )
            {
                // translate characters to local preference
                sal_UCS4 cChar = GetLocalizedChar( *pStr, meTextLanguage );
                if( cChar != *pStr )
                    // TODO: are the localized digit surrogates?
                    sTmpStr[pStr - pBase] = cChar;
            }
        }
        rStr = sTmpStr.makeStringAndClear();
    }

    // right align for RTL text, DRAWPOS_REVERSED, RTL window style
    bool bRightAlign = ((mnTextLayoutMode & TEXT_LAYOUT_BIDI_RTL) != 0);
    if( mnTextLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT )
        bRightAlign = false;
    else if ( mnTextLayoutMode & TEXT_LAYOUT_TEXTORIGIN_RIGHT )
        bRightAlign = true;
    // SSA: hack for western office, ie text get right aligned
    //      for debugging purposes of mirrored UI
    bool bRTLWindow = IsRTLEnabled();
    bRightAlign ^= bRTLWindow;
    if( bRightAlign )
        nLayoutFlags |= SAL_LAYOUT_RIGHT_ALIGN;

    // set layout options
    ImplLayoutArgs aLayoutArgs( rStr.getStr(), rStr.getLength(), nMinIndex, nEndIndex, nLayoutFlags, maFont.GetLanguageTag() );

    int nOrientation = mpFontEntry ? mpFontEntry->mnOrientation : 0;
    aLayoutArgs.SetOrientation( nOrientation );

    aLayoutArgs.SetLayoutWidth( nPixelWidth );
    aLayoutArgs.SetDXArray( pDXArray );

    return aLayoutArgs;
}

SalLayout* OutputDevice::ImplLayout(const OUString& rOrigStr,
    sal_Int32 nMinIndex, sal_Int32 nLen,
    const Point& rLogicalPos, long nLogicalWidth,
    const sal_Int32* pDXArray) const
{
    // we need a graphics
    if( !mpGraphics )
        if( !ImplGetGraphics() )
            return NULL;

    // initialize font if needed
    if( mbNewFont )
        if( !ImplNewFont() )
            return NULL;
    if( mbInitFont )
        ImplInitFont();

    // check string index and length
    if( -1 == nLen || nMinIndex + nLen > rOrigStr.getLength() )
    {
        const sal_Int32 nNewLen = rOrigStr.getLength() - nMinIndex;
        if( nNewLen <= 0 )
            return NULL;
        nLen = nNewLen;
    }

    OUString aStr = rOrigStr;

    // convert from logical units to physical units
    // recode string if needed
    if( mpFontEntry->mpConversion ) {
        mpFontEntry->mpConversion->RecodeString( aStr, 0, aStr.getLength() );
    }

    long nPixelWidth = nLogicalWidth;
    if( nLogicalWidth && mbMap )
        nPixelWidth = ImplLogicWidthToDevicePixel( nLogicalWidth );
    if( pDXArray && mbMap )
    {
        // convert from logical units to font units using a temporary array
        sal_Int32* pTempDXAry = (sal_Int32*)alloca( nLen * sizeof(sal_Int32) );
        // using base position for better rounding a.k.a. "dancing characters"
        int nPixelXOfs = ImplLogicWidthToDevicePixel( rLogicalPos.X() );
        for( int i = 0; i < nLen; ++i )
            pTempDXAry[i] = ImplLogicWidthToDevicePixel( rLogicalPos.X() + pDXArray[i] ) - nPixelXOfs;

        pDXArray = pTempDXAry;
    }

    ImplLayoutArgs aLayoutArgs = ImplPrepareLayoutArgs( aStr, nMinIndex, nLen, nPixelWidth, pDXArray );

#if defined(MACOSX) || defined(IOS)
    // CoreText layouts are immutable and already contain the text color
    // so we need to provide the color already for the layout request
    // even if this layout will never be drawn
    if( mbInitTextColor )
        const_cast<OutputDevice&>(*this).ImplInitTextColor();
#endif

    // get matching layout object for base font
    SalLayout* pSalLayout = mpGraphics->GetTextLayout( aLayoutArgs, 0 );

    // layout text
    if( pSalLayout && !pSalLayout->LayoutText( aLayoutArgs ) )
    {
        pSalLayout->Release();
        pSalLayout = NULL;
    }

    if( !pSalLayout )
        return NULL;

    // do glyph fallback if needed
    // #105768# avoid fallback for very small font sizes
    if (aLayoutArgs.NeedFallback() && mpFontEntry->maFontSelData.mnHeight >= 3)
        pSalLayout = ImplGlyphFallbackLayout(pSalLayout, aLayoutArgs);

    // position, justify, etc. the layout
    pSalLayout->AdjustLayout( aLayoutArgs );
    pSalLayout->DrawBase() = ImplLogicToDevicePixel( rLogicalPos );
    // adjust to right alignment if necessary
    if( aLayoutArgs.mnFlags & SAL_LAYOUT_RIGHT_ALIGN )
    {
        long nRTLOffset;
        if( pDXArray )
            nRTLOffset = pDXArray[ nLen - 1 ];
        else if( nPixelWidth )
            nRTLOffset = nPixelWidth;
        else
            nRTLOffset = pSalLayout->GetTextWidth() / pSalLayout->GetUnitsPerPixel();
        pSalLayout->DrawOffset().X() = 1 - nRTLOffset;
    }

    return pSalLayout;
}

SalLayout* OutputDevice::getFallbackFont(ImplFontEntry &rFallbackFont,
    FontSelectPattern &rFontSelData, int nFallbackLevel,
    ImplLayoutArgs& rLayoutArgs) const
{
    rFallbackFont.mnSetFontFlags = mpGraphics->SetFont( &rFontSelData, nFallbackLevel );

    rLayoutArgs.ResetPos();
    SalLayout* pFallback = mpGraphics->GetTextLayout( rLayoutArgs, nFallbackLevel );

    if (!pFallback)
        return NULL;

    if (!pFallback->LayoutText(rLayoutArgs))
    {
        // there is no need for a font that couldn't resolve anything
        pFallback->Release();
        return NULL;
    }

    pFallback->AdjustLayout( rLayoutArgs );

    return pFallback;
}

SalLayout* OutputDevice::ImplGlyphFallbackLayout( SalLayout* pSalLayout, ImplLayoutArgs& rLayoutArgs ) const
{
    // prepare multi level glyph fallback
    MultiSalLayout* pMultiSalLayout = NULL;
    ImplLayoutRuns aLayoutRuns = rLayoutArgs.maRuns;
    rLayoutArgs.PrepareFallback();
    rLayoutArgs.mnFlags |= SAL_LAYOUT_FOR_FALLBACK;

    // get list of unicodes that need glyph fallback
    int nCharPos = -1;
    bool bRTL = false;
    OUStringBuffer aMissingCodeBuf;
    while( rLayoutArgs.GetNextPos( &nCharPos, &bRTL) )
        aMissingCodeBuf.append( rLayoutArgs.mpStr[ nCharPos ] );
    rLayoutArgs.ResetPos();
    OUString aMissingCodes = aMissingCodeBuf.makeStringAndClear();

    FontSelectPattern aFontSelData = mpFontEntry->maFontSelData;

    // try if fallback fonts support the missing unicodes
    for( int nFallbackLevel = 1; nFallbackLevel < MAX_FALLBACK; ++nFallbackLevel )
    {
        // find a font family suited for glyph fallback
#ifndef FONTFALLBACK_HOOKS_DISABLED
        // GetGlyphFallbackFont() needs a valid aFontSelData.mpFontEntry
        // if the system-specific glyph fallback is active
        aFontSelData.mpFontEntry = mpFontEntry; // reset the fontentry to base-level
#endif
        ImplFontEntry* pFallbackFont = mpFontCache->GetGlyphFallbackFont( mpFontCollection,
            aFontSelData, nFallbackLevel, aMissingCodes );
        if( !pFallbackFont )
            break;

        aFontSelData.mpFontEntry = pFallbackFont;
        aFontSelData.mpFontData = pFallbackFont->maFontSelData.mpFontData;
        if( mpFontEntry && nFallbackLevel < MAX_FALLBACK-1)
        {
            // ignore fallback font if it is the same as the original font
            if( mpFontEntry->maFontSelData.mpFontData == aFontSelData.mpFontData )
            {
                mpFontCache->Release( pFallbackFont );
                continue;
            }
        }

        // create and add glyph fallback layout to multilayout
        SalLayout* pFallback = getFallbackFont(*pFallbackFont, aFontSelData,
            nFallbackLevel, rLayoutArgs);
        if (pFallback)
        {
            if( !pMultiSalLayout )
                pMultiSalLayout = new MultiSalLayout( *pSalLayout );
            pMultiSalLayout->AddFallback( *pFallback,
                rLayoutArgs.maRuns, aFontSelData.mpFontData );
            if (nFallbackLevel == MAX_FALLBACK-1)
                pMultiSalLayout->SetInComplete();
        }

        mpFontCache->Release( pFallbackFont );

        // break when this fallback was sufficient
        if( !rLayoutArgs.PrepareFallback() )
            break;
    }

    if( pMultiSalLayout && pMultiSalLayout->LayoutText( rLayoutArgs ) )
        pSalLayout = pMultiSalLayout;

    // restore orig font settings
    pSalLayout->InitFont();
    rLayoutArgs.maRuns = aLayoutRuns;

    return pSalLayout;
}

bool OutputDevice::GetTextIsRTL( const OUString& rString, sal_Int32 nIndex, sal_Int32 nLen ) const
{
    OUString aStr( rString );
    ImplLayoutArgs aArgs = ImplPrepareLayoutArgs( aStr, nIndex, nLen, 0, NULL );
    bool bRTL = false;
    int nCharPos = -1;
    aArgs.GetNextPos( &nCharPos, &bRTL );
    return (nCharPos != nIndex) ? sal_True : sal_False;
}

sal_Int32 OutputDevice::GetTextBreak( const OUString& rStr, long nTextWidth,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       long nCharExtra ) const
{
    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    sal_Int32 nRetVal = -1;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;
        nTextWidth *= nWidthFactor * nSubPixelFactor;
        long nTextPixelWidth = ImplLogicWidthToDevicePixel( nTextWidth );
        long nExtraPixelWidth = 0;
        if( nCharExtra != 0 )
        {
            nCharExtra *= nWidthFactor * nSubPixelFactor;
            nExtraPixelWidth = ImplLogicWidthToDevicePixel( nCharExtra );
        }
        nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

        pSalLayout->Release();
    }

    return nRetVal;
}

sal_Int32 OutputDevice::GetTextBreak( const OUString& rStr, long nTextWidth,
                                       sal_Unicode nHyphenChar, sal_Int32& rHyphenPos,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       long nCharExtra ) const
{
    rHyphenPos = -1;

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    sal_Int32 nRetVal = -1;
    if( pSalLayout )
    {
        // convert logical widths into layout units
        // NOTE: be very careful to avoid rounding errors for nCharExtra case
        // problem with rounding errors especially for small nCharExtras
        // TODO: remove when layout units have subpixel granularity
        long nWidthFactor = pSalLayout->GetUnitsPerPixel();
        long nSubPixelFactor = (nWidthFactor < 64 ) ? 64 : 1;

        nTextWidth *= nWidthFactor * nSubPixelFactor;
        long nTextPixelWidth = ImplLogicWidthToDevicePixel( nTextWidth );
        long nExtraPixelWidth = 0;
        if( nCharExtra != 0 )
        {
            nCharExtra *= nWidthFactor * nSubPixelFactor;
            nExtraPixelWidth = ImplLogicWidthToDevicePixel( nCharExtra );
        }

        // calculate un-hyphenated break position
        nRetVal = pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor );

        // calculate hyphenated break position
        OUString aHyphenStr(nHyphenChar);
        sal_Int32 nTempLen = 1;
        SalLayout* pHyphenLayout = ImplLayout( aHyphenStr, 0, nTempLen );
        if( pHyphenLayout )
        {
            // calculate subpixel width of hyphenation character
            long nHyphenPixelWidth = pHyphenLayout->GetTextWidth() * nSubPixelFactor;
            pHyphenLayout->Release();

            // calculate hyphenated break position
            nTextPixelWidth -= nHyphenPixelWidth;
            if( nExtraPixelWidth > 0 )
                nTextPixelWidth -= nExtraPixelWidth;

            rHyphenPos = pSalLayout->GetTextBreak(nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor);

            if( rHyphenPos > nRetVal )
                rHyphenPos = nRetVal;
        }

        pSalLayout->Release();
    }

    return nRetVal;
}

void OutputDevice::ImplDrawText( OutputDevice& rTargetDevice, const Rectangle& rRect,
                                 const OUString& rOrigStr, sal_uInt16 nStyle,
                                 MetricVector* pVector, OUString* pDisplayText,
                                 ::vcl::ITextLayout& _rLayout )
{
    Color aOldTextColor;
    Color aOldTextFillColor;
    bool  bRestoreFillColor = false;
    if ( (nStyle & TEXT_DRAW_DISABLE) && ! pVector )
    {
        bool  bHighContrastBlack = false;
        bool  bHighContrastWhite = false;
        const StyleSettings& rStyleSettings( rTargetDevice.GetSettings().GetStyleSettings() );
        if( rStyleSettings.GetHighContrastMode() )
        {
            Color aCol;
            if( rTargetDevice.IsBackground() )
                aCol = rTargetDevice.GetBackground().GetColor();
            else
                // best guess is the face color here
                // but it may be totally wrong. the background color
                // was typically already reset
                aCol = rStyleSettings.GetFaceColor();

            bHighContrastBlack = aCol.IsDark();
            bHighContrastWhite = aCol.IsBright();
        }

        aOldTextColor = rTargetDevice.GetTextColor();
        if ( rTargetDevice.IsTextFillColor() )
        {
            bRestoreFillColor = true;
            aOldTextFillColor = rTargetDevice.GetTextFillColor();
        }
        if( bHighContrastBlack )
            rTargetDevice.SetTextColor( COL_GREEN );
        else if( bHighContrastWhite )
            rTargetDevice.SetTextColor( COL_LIGHTGREEN );
        else
        {
            // draw disabled text always without shadow
            // as it fits better with native look
            rTargetDevice.SetTextColor( rTargetDevice.GetSettings().GetStyleSettings().GetDisableColor() );
        }
    }

    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( ((nWidth <= 0) || (nHeight <= 0)) && (nStyle & TEXT_DRAW_CLIP) )
        return;

    Point       aPos            = rRect.TopLeft();

    long        nTextHeight     = rTargetDevice.GetTextHeight();
    TextAlign   eAlign          = rTargetDevice.GetTextAlign();
    sal_Int32   nMnemonicPos    = -1;

    OUString aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );

    const bool bDrawMnemonics = !(rTargetDevice.GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector;

    // We treat multiline text differently
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {

        OUString                aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        sal_Int32               i;
        sal_Int32               nLines;
        sal_Int32               nFormatLines;

        if ( nTextHeight )
        {
            long nMaxTextWidth = ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle, _rLayout );
            nLines = (sal_Int32)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
                {
                    // Create last line and shorten it
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = convertLineEnd(aStr.copy(pLineInfo->GetIndex()), LINEEND_LF);
                    // Replace all LineFeeds with Spaces
                    OUStringBuffer aLastLineBuffer(aLastLine);
                    sal_Int32 nLastLineLen = aLastLineBuffer.getLength();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLineBuffer[ i ] == '\n' )
                            aLastLineBuffer[ i ] = ' ';
                    }
                    aLastLine = aLastLineBuffer.makeStringAndClear();
                    aLastLine = ImplGetEllipsisString( rTargetDevice, aLastLine, nWidth, nStyle, _rLayout );
                    nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                    nStyle |= TEXT_DRAW_TOP;
                }
            }
            else
            {
                if ( nMaxTextWidth <= nWidth )
                    nStyle &= ~TEXT_DRAW_CLIP;
            }

            // Do we need to clip the height?
            if ( nFormatLines*nTextHeight > nHeight )
                nStyle |= TEXT_DRAW_CLIP;

            // Set clipping
            if ( nStyle & TEXT_DRAW_CLIP )
            {
                rTargetDevice.Push( PUSH_CLIPREGION );
                rTargetDevice.IntersectClipRegion( rRect );
            }

            // Vertical alignment
            if ( nStyle & TEXT_DRAW_BOTTOM )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & TEXT_DRAW_VCENTER )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // Font alignment
            if ( eAlign == ALIGN_BOTTOM )
                aPos.Y() += nTextHeight;
            else if ( eAlign == ALIGN_BASELINE )
                aPos.Y() += rTargetDevice.GetFontMetric().GetAscent();

            // Output all lines except for the last one
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & TEXT_DRAW_RIGHT )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & TEXT_DRAW_CENTER )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                sal_Int32 nIndex   = pLineInfo->GetIndex();
                sal_Int32 nLineLen = pLineInfo->GetLen();
                _rLayout.DrawText( aPos, aStr, nIndex, nLineLen, pVector, pDisplayText );
                if ( bDrawMnemonics )
                {
                    if ( (nMnemonicPos >= nIndex) && (nMnemonicPos < nIndex+nLineLen) )
                    {
                        long        nMnemonicX;
                        long        nMnemonicY;
                        long        nMnemonicWidth;

                        sal_Int32* pCaretXArray = (sal_Int32*) alloca( 2 * sizeof(sal_Int32) * nLineLen );
                        /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray,
                                                nIndex, nLineLen );
                        long lc_x1 = pCaretXArray[2*(nMnemonicPos - nIndex)];
                        long lc_x2 = pCaretXArray[2*(nMnemonicPos - nIndex)+1];
                        nMnemonicWidth = rTargetDevice.ImplLogicWidthToDevicePixel( ::abs((int)(lc_x1 - lc_x2)) );

                        Point       aTempPos = rTargetDevice.LogicToPixel( aPos );
                        nMnemonicX = rTargetDevice.GetOutOffXPixel() + aTempPos.X() + rTargetDevice.ImplLogicWidthToDevicePixel( std::min( lc_x1, lc_x2 ) );
                        nMnemonicY = rTargetDevice.GetOutOffYPixel() + aTempPos.Y() + rTargetDevice.ImplLogicWidthToDevicePixel( rTargetDevice.GetFontMetric().GetAscent() );
                        rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
                    }
                }
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
            }

            // If there still is a last line, we output it left-aligned as the line would be clipped
            if ( !aLastLine.isEmpty() )
                _rLayout.DrawText( aPos, aLastLine, 0, aLastLine.getLength(), pVector, pDisplayText );

            // Reset clipping
            if ( nStyle & TEXT_DRAW_CLIP )
                rTargetDevice.Pop();
        }
    }
    else
    {
        long nTextWidth = _rLayout.GetTextWidth( aStr, 0, -1 );

        // Clip text if needed
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & TEXT_DRAW_ELLIPSIS )
            {
                aStr = ImplGetEllipsisString( rTargetDevice, aStr, nWidth, nStyle, _rLayout );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                nTextWidth = _rLayout.GetTextWidth( aStr, 0, aStr.getLength() );
            }
        }
        else
        {
            if ( nTextHeight <= nHeight )
                nStyle &= ~TEXT_DRAW_CLIP;
        }

        // horizontal text alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-nTextWidth)/2;

        // vertical font alignment
        if ( eAlign == ALIGN_BOTTOM )
            aPos.Y() += nTextHeight;
        else if ( eAlign == ALIGN_BASELINE )
            aPos.Y() += rTargetDevice.GetFontMetric().GetAscent();

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-nTextHeight)/2;

        long nMnemonicX = 0;
        long nMnemonicY = 0;
        long nMnemonicWidth = 0;
        if ( nMnemonicPos != -1 )
        {
            sal_Int32* pCaretXArray = (sal_Int32*) alloca( 2 * sizeof(sal_Int32) * aStr.getLength() );
            /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray, 0, aStr.getLength() );
            long lc_x1 = pCaretXArray[2*(nMnemonicPos)];
            long lc_x2 = pCaretXArray[2*(nMnemonicPos)+1];
            nMnemonicWidth = rTargetDevice.ImplLogicWidthToDevicePixel( ::abs((int)(lc_x1 - lc_x2)) );

            Point aTempPos = rTargetDevice.LogicToPixel( aPos );
            nMnemonicX = rTargetDevice.GetOutOffXPixel() + aTempPos.X() + rTargetDevice.ImplLogicWidthToDevicePixel( std::min(lc_x1, lc_x2) );
            nMnemonicY = rTargetDevice.GetOutOffYPixel() + aTempPos.Y() + rTargetDevice.ImplLogicWidthToDevicePixel( rTargetDevice.GetFontMetric().GetAscent() );
        }

        if ( nStyle & TEXT_DRAW_CLIP )
        {
            rTargetDevice.Push( PUSH_CLIPREGION );
            rTargetDevice.IntersectClipRegion( rRect );
            _rLayout.DrawText( aPos, aStr, 0, aStr.getLength(), pVector, pDisplayText );
            if ( bDrawMnemonics )
            {
                if ( nMnemonicPos != -1 )
                    rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
            rTargetDevice.Pop();
        }
        else
        {
            _rLayout.DrawText( aPos, aStr, 0, aStr.getLength(), pVector, pDisplayText );
            if ( bDrawMnemonics )
            {
                if ( nMnemonicPos != -1 )
                    rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
        }
    }

    if ( nStyle & TEXT_DRAW_DISABLE && !pVector )
    {
        rTargetDevice.SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            rTargetDevice.SetTextFillColor( aOldTextFillColor );
    }
}

void OutputDevice::AddTextRectActions( const Rectangle& rRect,
                                       const OUString&  rOrigStr,
                                       sal_uInt16       nStyle,
                                       GDIMetaFile&     rMtf )
{

    if ( rOrigStr.isEmpty() || rRect.IsEmpty() )
        return;

    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();

    // temporarily swap in passed mtf for action generation, and
    // disable output generation.
    const bool bOutputEnabled( IsOutputEnabled() );
    GDIMetaFile* pMtf = mpMetaFile;

    mpMetaFile = &rMtf;
    EnableOutput( false );

    // #i47157# Factored out to ImplDrawTextRect(), to be shared
    // between us and DrawText()
    DefaultTextLayout aLayout( *this );
    ImplDrawText( *this, rRect, rOrigStr, nStyle, NULL, NULL, aLayout );

    // and restore again
    EnableOutput( bOutputEnabled );
    mpMetaFile = pMtf;
}

void OutputDevice::DrawText( const Rectangle& rRect, const OUString& rOrigStr, sal_uInt16 nStyle,
                             MetricVector* pVector, OUString* pDisplayText,
                             ::vcl::ITextLayout* _pTextLayout )
{
    if( mpOutDevData && mpOutDevData->mpRecordLayout )
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

    bool bDecomposeTextRectAction = ( _pTextLayout != NULL ) && _pTextLayout->DecomposeTextRectAction();
    if ( mpMetaFile && !bDecomposeTextRectAction )
        mpMetaFile->AddAction( new MetaTextRectAction( rRect, rOrigStr, nStyle ) );

    if ( ( !IsDeviceOutputNecessary() && !pVector && !bDecomposeTextRectAction ) || rOrigStr.isEmpty() || rRect.IsEmpty() )
        return;

    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if( mbOutputClipped && !bDecomposeTextRectAction )
        return;

    // temporarily disable mtf action generation (ImplDrawText _does_
    // create META_TEXT_ACTIONs otherwise)
    GDIMetaFile* pMtf = mpMetaFile;
    if ( !bDecomposeTextRectAction )
        mpMetaFile = NULL;

    // #i47157# Factored out to ImplDrawText(), to be used also
    // from AddTextRectActions()
    DefaultTextLayout aDefaultLayout( *this );
    ImplDrawText( *this, rRect, rOrigStr, nStyle, pVector, pDisplayText, _pTextLayout ? *_pTextLayout : aDefaultLayout );

    // and enable again
    mpMetaFile = pMtf;

    if( mpAlphaVDev )
        mpAlphaVDev->DrawText( rRect, rOrigStr, nStyle, pVector, pDisplayText );
}

Rectangle OutputDevice::GetTextRect( const Rectangle& rRect,
                                     const OUString& rStr, sal_uInt16 nStyle,
                                     TextRectInfo* pInfo,
                                     const ::vcl::ITextLayout* _pTextLayout ) const
{

    Rectangle           aRect = rRect;
    sal_Int32           nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight = GetTextHeight();

    OUString aStr = rStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr );

    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        sal_Int32               nFormatLines;
        sal_Int32               i;

        nMaxWidth = 0;
        DefaultTextLayout aDefaultLayout( *const_cast< OutputDevice* >( this ) );
        ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle, _pTextLayout ? *_pTextLayout : aDefaultLayout );
        nFormatLines = aMultiLineInfo.Count();
        if ( !nTextHeight )
            nTextHeight = 1;
        nLines = (sal_uInt16)(aRect.GetHeight()/nTextHeight);
        if ( pInfo )
            pInfo->mnLineCount = nFormatLines;
        if ( !nLines )
            nLines = 1;
        if ( nFormatLines <= nLines )
            nLines = nFormatLines;
        else
        {
            if ( !(nStyle & TEXT_DRAW_ENDELLIPSIS) )
                nLines = nFormatLines;
            else
            {
                if ( pInfo )
                    pInfo->mbEllipsis = true;
                nMaxWidth = nWidth;
            }
        }
        if ( pInfo )
        {
            bool bMaxWidth = nMaxWidth == 0;
            pInfo->mnMaxWidth = 0;
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( bMaxWidth && (pLineInfo->GetWidth() > nMaxWidth) )
                    nMaxWidth = pLineInfo->GetWidth();
                if ( pLineInfo->GetWidth() > pInfo->mnMaxWidth )
                    pInfo->mnMaxWidth = pLineInfo->GetWidth();
            }
        }
        else if ( !nMaxWidth )
        {
            for ( i = 0; i < nLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( pLineInfo->GetWidth() > nMaxWidth )
                    nMaxWidth = pLineInfo->GetWidth();
            }
        }
    }
    else
    {
        nLines      = 1;
        nMaxWidth   = _pTextLayout ? _pTextLayout->GetTextWidth( aStr, 0, aStr.getLength() ) : GetTextWidth( aStr );

        if ( pInfo )
        {
            pInfo->mnLineCount  = 1;
            pInfo->mnMaxWidth   = nMaxWidth;
        }

        if ( (nMaxWidth > nWidth) && (nStyle & TEXT_DRAW_ELLIPSIS) )
        {
            if ( pInfo )
                pInfo->mbEllipsis = true;
            nMaxWidth = nWidth;
        }
    }

    if ( nStyle & TEXT_DRAW_RIGHT )
        aRect.Left() = aRect.Right()-nMaxWidth+1;
    else if ( nStyle & TEXT_DRAW_CENTER )
    {
        aRect.Left() += (nWidth-nMaxWidth)/2;
        aRect.Right() = aRect.Left()+nMaxWidth-1;
    }
    else
        aRect.Right() = aRect.Left()+nMaxWidth-1;

    if ( nStyle & TEXT_DRAW_BOTTOM )
        aRect.Top() = aRect.Bottom()-(nTextHeight*nLines)+1;
    else if ( nStyle & TEXT_DRAW_VCENTER )
    {
        aRect.Top()   += (aRect.GetHeight()-(nTextHeight*nLines))/2;
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;
    }
    else
        aRect.Bottom() = aRect.Top()+(nTextHeight*nLines)-1;

    // #99188# get rid of rounding problems when using this rect later
    if (nStyle & TEXT_DRAW_RIGHT)
        aRect.Left()--;
    else
        aRect.Right()++;
    return aRect;
}

static bool ImplIsCharIn( sal_Unicode c, const sal_Char* pStr )
{
    while ( *pStr )
    {
        if ( *pStr == c )
            return true;
        pStr++;
    }

    return false;
}

OUString OutputDevice::GetEllipsisString( const OUString& rOrigStr, long nMaxWidth,
                                        sal_uInt16 nStyle ) const
{
    DefaultTextLayout aTextLayout( *const_cast< OutputDevice* >( this ) );
    return ImplGetEllipsisString( *this, rOrigStr, nMaxWidth, nStyle, aTextLayout );
}

OUString OutputDevice::ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rOrigStr, long nMaxWidth,
                                               sal_uInt16 nStyle, const ::vcl::ITextLayout& _rLayout )
{
    OUString aStr = rOrigStr;
    sal_Int32 nIndex = _rLayout.GetTextBreak( aStr, nMaxWidth, 0, aStr.getLength() );

    if ( nIndex != -1 )
    {
        if( (nStyle & TEXT_DRAW_CENTERELLIPSIS) == TEXT_DRAW_CENTERELLIPSIS )
        {
            OUStringBuffer aTmpStr( aStr );
            sal_Int32 nEraseChars = 4;
            while( nEraseChars < aStr.getLength() && _rLayout.GetTextWidth( aTmpStr.toString(), 0, aTmpStr.getLength() ) > nMaxWidth )
            {
                aTmpStr = OUStringBuffer(aStr);
                sal_Int32 i = (aTmpStr.getLength() - nEraseChars)/2;
                aTmpStr.remove(i, nEraseChars++);
                aTmpStr.insert(i, "...");
            }
            aStr = aTmpStr.makeStringAndClear();
        }
        else if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
        {
            aStr = aStr.copy(0, nIndex);
            if ( nIndex > 1 )
            {
                aStr += "...";
                while ( !aStr.isEmpty() && (_rLayout.GetTextWidth( aStr, 0, aStr.getLength() ) > nMaxWidth) )
                {
                    if ( (nIndex > 1) || (nIndex == aStr.getLength()) )
                        nIndex--;
                    aStr = aStr.replaceAt( nIndex, 1, "");
                }
            }

            if ( aStr.isEmpty() && (nStyle & TEXT_DRAW_CLIP) )
                aStr += OUString(rOrigStr[ 0 ]);
        }
        else if ( nStyle & TEXT_DRAW_PATHELLIPSIS )
        {
            OUString aPath( rOrigStr );
            OUString aAbbreviatedPath;
            osl_abbreviateSystemPath( aPath.pData, &aAbbreviatedPath.pData, nIndex, NULL );
            aStr = aAbbreviatedPath;
        }
        else if ( nStyle & TEXT_DRAW_NEWSELLIPSIS )
        {
            static sal_Char const   pSepChars[] = ".";
            // Determine last section
            sal_Int32 nLastContent = aStr.getLength();
            while ( nLastContent )
            {
                nLastContent--;
                if ( ImplIsCharIn( aStr[ nLastContent ], pSepChars ) )
                    break;
            }
            while ( nLastContent &&
                    ImplIsCharIn( aStr[ nLastContent-1 ], pSepChars ) )
                nLastContent--;

            OUString aLastStr = aStr.copy(nLastContent);
            OUString aTempLastStr1( "..." );
            aTempLastStr1 += aLastStr;
            if ( _rLayout.GetTextWidth( aTempLastStr1, 0, aTempLastStr1.getLength() ) > nMaxWidth )
                aStr = OutputDevice::ImplGetEllipsisString( rTargetDevice, aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS, _rLayout );
            else
            {
                sal_Int32 nFirstContent = 0;
                while ( nFirstContent < nLastContent )
                {
                    nFirstContent++;
                    if ( ImplIsCharIn( aStr[ nFirstContent ], pSepChars ) )
                        break;
                }
                while ( (nFirstContent < nLastContent) &&
                        ImplIsCharIn( aStr[ nFirstContent ], pSepChars ) )
                    nFirstContent++;
                // MEM continue here
                if ( nFirstContent >= nLastContent )
                    aStr = OutputDevice::ImplGetEllipsisString( rTargetDevice, aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS, _rLayout );
                else
                {
                    if ( nFirstContent > 4 )
                        nFirstContent = 4;
                    OUString aFirstStr = aStr.copy( 0, nFirstContent );
                    aFirstStr += "...";
                    OUString aTempStr = aFirstStr + aLastStr;
                    if ( _rLayout.GetTextWidth( aTempStr, 0, aTempStr.getLength() ) > nMaxWidth )
                        aStr = OutputDevice::ImplGetEllipsisString( rTargetDevice, aStr, nMaxWidth, nStyle | TEXT_DRAW_ENDELLIPSIS, _rLayout );
                    else
                    {
                        do
                        {
                            aStr = aTempStr;
                            if( nLastContent > aStr.getLength() )
                                nLastContent = aStr.getLength();
                            while ( nFirstContent < nLastContent )
                            {
                                nLastContent--;
                                if ( ImplIsCharIn( aStr[ nLastContent ], pSepChars ) )
                                    break;

                            }
                            while ( (nFirstContent < nLastContent) &&
                                    ImplIsCharIn( aStr[ nLastContent-1 ], pSepChars ) )
                                nLastContent--;

                            if ( nFirstContent < nLastContent )
                            {
                                OUString aTempLastStr = aStr.copy( nLastContent );
                                aTempStr = aFirstStr + aTempLastStr;

                                if ( _rLayout.GetTextWidth( aTempStr, 0, aTempStr.getLength() ) > nMaxWidth )
                                    break;
                            }
                        }
                        while ( nFirstContent < nLastContent );
                    }
                }
            }
        }
    }

    return aStr;
}

void OutputDevice::DrawCtrlText( const Point& rPos, const OUString& rStr,
                                 sal_Int32 nIndex, sal_Int32 nLen,
                                 sal_uInt16 nStyle, MetricVector* pVector, OUString* pDisplayText )
{

    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "DrawCtrlText Suspicious arguments nLen:" << nLen);
    }
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( !IsDeviceOutputNecessary() || (nIndex >= rStr.getLength()) )
        return;

    // better get graphics here because ImplDrawMnemonicLine() will not
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if( nIndex >= rStr.getLength() )
        return;

    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }
    OUString   aStr = rStr;
    sal_Int32  nMnemonicPos = -1;

    long        nMnemonicX = 0;
    long        nMnemonicY = 0;
    long        nMnemonicWidth = 0;
    if ( (nStyle & TEXT_DRAW_MNEMONIC) && nLen > 1 )
    {
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );
        if ( nMnemonicPos != -1 )
        {
            if( nMnemonicPos < nIndex )
            {
                --nIndex;
            }
            else
            {
                if( nMnemonicPos < (nIndex+nLen) )
                    --nLen;
                DBG_ASSERT( nMnemonicPos < (nIndex+nLen), "Mnemonic underline marker after last character" );
            }
            bool bInvalidPos = false;

            if( nMnemonicPos >= nLen )
            {
                // #106952#
                // may occur in BiDi-Strings: the '~' is sometimes found behind the last char
                // due to some strange BiDi text editors
                // -> place the underline behind the string to indicate a failure
                bInvalidPos = true;
                nMnemonicPos = nLen-1;
            }

            sal_Int32* pCaretXArray = (sal_Int32*)alloca( 2 * sizeof(sal_Int32) * nLen );
            /*sal_Bool bRet =*/ GetCaretPositions( aStr, pCaretXArray, nIndex, nLen );
            long lc_x1 = pCaretXArray[ 2*(nMnemonicPos - nIndex) ];
            long lc_x2 = pCaretXArray[ 2*(nMnemonicPos - nIndex)+1 ];
            nMnemonicWidth = ::abs((int)(lc_x1 - lc_x2));

            Point aTempPos( std::min(lc_x1,lc_x2), GetFontMetric().GetAscent() );
            if( bInvalidPos )  // #106952#, place behind the (last) character
                aTempPos = Point( std::max(lc_x1,lc_x2), GetFontMetric().GetAscent() );

            aTempPos += rPos;
            aTempPos = LogicToPixel( aTempPos );
            nMnemonicX = mnOutOffX + aTempPos.X();
            nMnemonicY = mnOutOffY + aTempPos.Y();
        }
    }

    if ( nStyle & TEXT_DRAW_DISABLE && ! pVector )
    {
        Color aOldTextColor;
        Color aOldTextFillColor;
        bool  bRestoreFillColor;
        bool  bHighContrastBlack = false;
        bool  bHighContrastWhite = false;
        const StyleSettings& rStyleSettings( GetSettings().GetStyleSettings() );
        if( rStyleSettings.GetHighContrastMode() )
        {
            if( IsBackground() )
            {
                Wallpaper aWall = GetBackground();
                Color aCol = aWall.GetColor();
                bHighContrastBlack = aCol.IsDark();
                bHighContrastWhite = aCol.IsBright();
            }
        }

        aOldTextColor = GetTextColor();
        if ( IsTextFillColor() )
        {
            bRestoreFillColor = true;
            aOldTextFillColor = GetTextFillColor();
        }
        else
            bRestoreFillColor = false;

        if( bHighContrastBlack )
            SetTextColor( COL_GREEN );
        else if( bHighContrastWhite )
            SetTextColor( COL_LIGHTGREEN );
        else
            SetTextColor( GetSettings().GetStyleSettings().GetDisableColor() );

        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
        {
            if ( nMnemonicPos != -1 )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
        SetTextColor( aOldTextColor );
        if ( bRestoreFillColor )
            SetTextFillColor( aOldTextFillColor );
    }
    else
    {
        DrawText( rPos, aStr, nIndex, nLen, pVector, pDisplayText );
        if ( !(GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector )
        {
            if ( nMnemonicPos != -1 )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawCtrlText( rPos, rStr, nIndex, nLen, nStyle, pVector, pDisplayText );
}

long OutputDevice::GetCtrlTextWidth( const OUString& rStr,
                                     sal_Int32 nIndex, sal_Int32 nLen,
                                     sal_uInt16 nStyle ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetCtrlTextWidth Suspicious arguments nLen:" << nLen);
    }
    /* defensive code */
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    if ( nStyle & TEXT_DRAW_MNEMONIC )
    {
        sal_Int32  nMnemonicPos;
        OUString   aStr = GetNonMnemonicString( rStr, nMnemonicPos );
        if ( nMnemonicPos != -1 )
        {
            if ( nMnemonicPos < nIndex )
                nIndex--;
            else if ( (nMnemonicPos >= nIndex) && ((sal_uLong)nMnemonicPos < (sal_uLong)(nIndex+nLen)) )
                nLen--;
        }
        return GetTextWidth( aStr, nIndex, nLen );
    }
    else
        return GetTextWidth( rStr, nIndex, nLen );
}

OUString OutputDevice::GetNonMnemonicString( const OUString& rStr, sal_Int32& rMnemonicPos )
{
    OUString   aStr    = rStr;
    sal_Int32  nLen    = aStr.getLength();
    sal_Int32  i       = 0;

    rMnemonicPos = -1;
    while ( i < nLen )
    {
        if ( aStr[ i ] == '~' )
        {
            if ( nLen <= i+1 )
                break;

            if ( aStr[ i+1 ] != '~' )
            {
                if ( rMnemonicPos == -1 )
                    rMnemonicPos = i;
                aStr = aStr.replaceAt( i, 1, "" );
                nLen--;
            }
            else
            {
                aStr = aStr.replaceAt( i, 1, "" );
                nLen--;
                i++;
            }
        }
        else
            i++;
    }

    return aStr;
}

int OutputDevice::GetDevFontCount() const
{

    if( !mpGetDevFontList )
        mpGetDevFontList = mpFontCollection->GetDevFontList();
    return mpGetDevFontList->Count();
}

FontInfo OutputDevice::GetDevFont( int nDevFontIndex ) const
{

    FontInfo aFontInfo;

    ImplInitFontList();

    int nCount = GetDevFontCount();
    if( nDevFontIndex < nCount )
    {
        const PhysicalFontFace& rData = *mpGetDevFontList->Get( nDevFontIndex );
        aFontInfo.SetName( rData.GetFamilyName() );
        aFontInfo.SetStyleName( rData.GetStyleName() );
        aFontInfo.SetCharSet( rData.IsSymbolFont() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        aFontInfo.SetFamily( rData.GetFamilyType() );
        aFontInfo.SetPitch( rData.GetPitch() );
        aFontInfo.SetWeight( rData.GetWeight() );
        aFontInfo.SetItalic( rData.GetSlant() );
        aFontInfo.SetWidthType( rData.GetWidthType() );
        if( rData.IsScalable() )
            aFontInfo.mpImplMetric->mnMiscFlags |= ImplFontMetric::SCALABLE_FLAG;
        if( rData.mbDevice )
            aFontInfo.mpImplMetric->mnMiscFlags |= ImplFontMetric::DEVICE_FLAG;
    }

    return aFontInfo;
}

bool OutputDevice::AddTempDevFont( const OUString& rFileURL, const OUString& rFontName )
{

    ImplInitFontList();

    if( !mpGraphics && !ImplGetGraphics() )
        return false;

    bool bRC = mpGraphics->AddTempDevFont( mpFontCollection, rFileURL, rFontName );
    if( !bRC )
        return false;

    if( mpAlphaVDev )
        mpAlphaVDev->AddTempDevFont( rFileURL, rFontName );

    mpFontCache->Invalidate();
    return true;
}

int OutputDevice::GetDevFontSizeCount( const Font& rFont ) const
{

    delete mpGetDevSizeList;

    ImplInitFontList();
    mpGetDevSizeList = mpFontCollection->GetDevSizeList( rFont.GetName() );
    return mpGetDevSizeList->Count();
}

Size OutputDevice::GetDevFontSize( const Font& rFont, int nSizeIndex ) const
{

    // check range
    int nCount = GetDevFontSizeCount( rFont );
    if ( nSizeIndex >= nCount )
        return Size();

    // when mapping is enabled round to .5 points
    Size aSize( 0, mpGetDevSizeList->Get( nSizeIndex ) );
    if ( mbMap )
    {
        aSize.Height() *= 10;
        MapMode aMap( MAP_10TH_INCH, Point(), Fraction( 1, 72 ), Fraction( 1, 72 ) );
        aSize = PixelToLogic( aSize, aMap );
        aSize.Height() += 5;
        aSize.Height() /= 10;
        long nRound = aSize.Height() % 5;
        if ( nRound >= 3 )
            aSize.Height() += (5-nRound);
        else
            aSize.Height() -= nRound;
        aSize.Height() *= 10;
        aSize = LogicToPixel( aSize, aMap );
        aSize = PixelToLogic( aSize );
        aSize.Height() += 5;
        aSize.Height() /= 10;
    }
    return aSize;
}

bool OutputDevice::IsFontAvailable( const OUString& rFontName ) const
{

    PhysicalFontFamily* pFound = mpFontCollection->FindFontFamily( rFontName );
    return (pFound != NULL);
}

FontMetric OutputDevice::GetFontMetric() const
{

    FontMetric aMetric;
    if( mbNewFont && !ImplNewFont() )
        return aMetric;

    ImplFontEntry*      pEntry = mpFontEntry;
    ImplFontMetricData* pMetric = &(pEntry->maMetric);

    // prepare metric
    aMetric.Font::operator=( maFont );

    // set aMetric with info from font
    aMetric.SetName( maFont.GetName() );
    aMetric.SetStyleName( pMetric->GetStyleName() );
    aMetric.SetSize( PixelToLogic( Size( pMetric->mnWidth, pMetric->mnAscent+pMetric->mnDescent-pMetric->mnIntLeading ) ) );
    aMetric.SetCharSet( pMetric->IsSymbolFont() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
    aMetric.SetFamily( pMetric->GetFamilyType() );
    aMetric.SetPitch( pMetric->GetPitch() );
    aMetric.SetWeight( pMetric->GetWeight() );
    aMetric.SetItalic( pMetric->GetSlant() );
    aMetric.SetWidthType( pMetric->GetWidthType() );
    if ( pEntry->mnOwnOrientation )
        aMetric.SetOrientation( pEntry->mnOwnOrientation );
    else
        aMetric.SetOrientation( pMetric->mnOrientation );
    if( !pEntry->maMetric.mbKernableFont )
         aMetric.SetKerning( maFont.GetKerning() & ~KERNING_FONTSPECIFIC );

    // set remaining metric fields
    aMetric.mpImplMetric->mnMiscFlags   = 0;
    if( pMetric->mbDevice )
            aMetric.mpImplMetric->mnMiscFlags |= ImplFontMetric::DEVICE_FLAG;
    if( pMetric->mbScalableFont )
            aMetric.mpImplMetric->mnMiscFlags |= ImplFontMetric::SCALABLE_FLAG;
    aMetric.mpImplMetric->mnAscent      = ImplDevicePixelToLogicHeight( pMetric->mnAscent+mnEmphasisAscent );
    aMetric.mpImplMetric->mnDescent     = ImplDevicePixelToLogicHeight( pMetric->mnDescent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnIntLeading  = ImplDevicePixelToLogicHeight( pMetric->mnIntLeading+mnEmphasisAscent );
    aMetric.mpImplMetric->mnExtLeading  = ImplDevicePixelToLogicHeight( pMetric->mnExtLeading );
    aMetric.mpImplMetric->mnLineHeight  = ImplDevicePixelToLogicHeight( pMetric->mnAscent+pMetric->mnDescent+mnEmphasisAscent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnSlant       = ImplDevicePixelToLogicHeight( pMetric->mnSlant );

#ifdef UNX
    // backwards compatible line metrics after fixing #i60945#
    if( (meOutDevType == OUTDEV_VIRDEV)
    &&  static_cast<const VirtualDevice*>(this)->ForceZeroExtleadBug() )
        aMetric.mpImplMetric->mnExtLeading = 0;
#endif

    return aMetric;
}

FontMetric OutputDevice::GetFontMetric( const Font& rFont ) const
{
    // select font, query metrics, select original font again
    Font aOldFont = GetFont();
    const_cast<OutputDevice*>(this)->SetFont( rFont );
    FontMetric aMetric( GetFontMetric() );
    const_cast<OutputDevice*>(this)->SetFont( aOldFont );
    return aMetric;
}

/** OutputDevice::GetSysFontData
 *
 * @param nFallbacklevel Fallback font level (0 = best matching font)
 *
 * Retrieve detailed font information in platform independent structure
 *
 * @return SystemFontData
 **/
SystemFontData OutputDevice::GetSysFontData(int nFallbacklevel) const
{
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof(aSysFontData);

    if (!mpGraphics) ImplGetGraphics();
    if (mpGraphics) aSysFontData = mpGraphics->GetSysFontData(nFallbacklevel);

    return aSysFontData;
}

/** OutputDevice::GetSysTextLayoutData
 *
 * @param rStartPt Start point of the text
 * @param rStr Text string that will be transformed into layout of glyphs
 * @param nIndex Position in the string from where layout will be done
 * @param nLen Length of the string
 * @param pDXAry Custom layout adjustment data
 *
 * Export finalized glyph layout data as platform independent SystemTextLayoutData
 * (see vcl/inc/vcl/sysdata.hxx)
 *
 * Only parameters rStartPt and rStr are mandatory, the rest is optional
 * (default values will be used)
 *
 * @return SystemTextLayoutData
 **/
SystemTextLayoutData OutputDevice::GetSysTextLayoutData(const Point& rStartPt, const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen,
                                                        const sal_Int32* pDXAry) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetSysTextLayoutData Suspicious arguments nLen:" << nLen);
    }
    if( (nLen < 0) || (nIndex + nLen >= rStr.getLength()))
    {
        nLen = rStr.getLength() - nIndex;
    }

    SystemTextLayoutData aSysLayoutData;
    aSysLayoutData.nSize = sizeof(aSysLayoutData);
    aSysLayoutData.rGlyphData.reserve( 256 );
    aSysLayoutData.orientation = 0;

    if ( mpMetaFile )
    {
        if (pDXAry)
            mpMetaFile->AddAction( new MetaTextArrayAction( rStartPt, rStr, pDXAry, nIndex, nLen ) );
        else
            mpMetaFile->AddAction( new MetaTextAction( rStartPt, rStr, nIndex, nLen ) );
    }

    if ( !IsDeviceOutputNecessary() ) return aSysLayoutData;

    SalLayout* pLayout = ImplLayout(rStr, nIndex, nLen, rStartPt, 0, pDXAry);

    if ( !pLayout ) return aSysLayoutData;

    // setup glyphs
    Point aPos;
    sal_GlyphId aGlyphId;
    for( int nStart = 0; pLayout->GetNextGlyphs( 1, &aGlyphId, aPos, nStart ); )
    {
        // NOTE: Windows backend is producing unicode chars (ucs4), so on windows,
        //       ETO_GLYPH_INDEX is unusable, unless extra glyph conversion is made.

        SystemGlyphData aGlyph;
        aGlyph.index = static_cast<unsigned long> (aGlyphId & GF_IDXMASK);
        aGlyph.x = aPos.X();
        aGlyph.y = aPos.Y();
        int nLevel = (aGlyphId & GF_FONTMASK) >> GF_FONTSHIFT;
        aGlyph.fallbacklevel = nLevel < MAX_FALLBACK ? nLevel : 0;
        aSysLayoutData.rGlyphData.push_back(aGlyph);
    }

    // Get font data
    aSysLayoutData.orientation = pLayout->GetOrientation();

    pLayout->Release();

    return aSysLayoutData;
}

long OutputDevice::GetMinKashida() const
{
    if( mbNewFont && !ImplNewFont() )
        return 0;

    ImplFontEntry*      pEntry = mpFontEntry;
    ImplFontMetricData* pMetric = &(pEntry->maMetric);
    return ImplDevicePixelToLogicWidth( pMetric->mnMinKashida );
}

sal_Int32 OutputDevice::ValidateKashidas ( const OUString& rTxt,
                                            sal_Int32 nIdx, sal_Int32 nLen,
                                            sal_Int32 nKashCount,
                                            const sal_Int32* pKashidaPos,
                                            sal_Int32* pKashidaPosDropped ) const
{
   // do layout
    SalLayout* pSalLayout = ImplLayout( rTxt, nIdx, nLen );
    if( !pSalLayout )
        return 0;
    sal_Int32 nDropped = 0;
    for( int i = 0; i < nKashCount; ++i )
    {
        if( !pSalLayout->IsKashidaPosValid( pKashidaPos[ i ] ))
        {
            pKashidaPosDropped[ nDropped ] = pKashidaPos [ i ];
            ++nDropped;
        }
    }
    pSalLayout->Release();
    return nDropped;
}

bool OutputDevice::GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr,
                                           int nIndex, int nLen, int nBase, MetricVector& rVector )
{

    rVector.clear();

    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetGlyphBoundRects Suspicious arguments nLen:" << nLen);
    }

    if( nIndex >= rStr.getLength() )
        return false;

    if( nLen < 0 || nIndex + nLen >= rStr.getLength() )
    {
        nLen = rStr.getLength() - nIndex;
    }

    Rectangle aRect;
    for( int i = 0; i < nLen; i++ )
    {
        if( !GetTextBoundRect( aRect, rStr, nBase, nIndex + i, 1 ) )
            break;
        aRect.Move( rOrigin.X(), rOrigin.Y() );
        rVector.push_back( aRect );
    }

    return (nLen == (int)rVector.size());
}

bool OutputDevice::GetTextBoundRect( Rectangle& rRect,
                                         const OUString& rStr, sal_Int32 nBase,
                                         sal_Int32 nIndex, sal_Int32 nLen,
                                         sal_uLong nLayoutWidth, const sal_Int32* pDXAry ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextBoundRect Suspicious arguments nLen:" << nLen);
    }

    bool bRet = false;
    rRect.SetEmpty();

    SalLayout* pSalLayout = NULL;
    const Point aPoint;
    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart = std::min( nBase, nIndex );
        sal_Int32 nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, aPoint, nLayoutWidth, pDXAry );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            nXOffset /= pSalLayout->GetUnitsPerPixel();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase < nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, aPoint, nLayoutWidth, pDXAry );
    Rectangle aPixelRect;
    if( pSalLayout )
    {
        bRet = pSalLayout->GetBoundRect( *mpGraphics, aPixelRect );

        if( bRet )
        {
            int nWidthFactor = pSalLayout->GetUnitsPerPixel();

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                aPixelRect.Left()
                    = static_cast< long >(aPixelRect.Left() * fFactor);
                aPixelRect.Right()
                    = static_cast< long >(aPixelRect.Right() * fFactor);
                aPixelRect.Top()
                    = static_cast< long >(aPixelRect.Top() * fFactor);
                aPixelRect.Bottom()
                    = static_cast< long >(aPixelRect.Bottom() * fFactor);
            }

            Point aRotatedOfs( mnTextOffX, mnTextOffY );
            aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
            aPixelRect += aRotatedOfs;
            rRect = PixelToLogic( aPixelRect );
            if( mbMap )
                rRect += Point( maMapRes.mnMapOfsX, maMapRes.mnMapOfsY );
        }

        pSalLayout->Release();
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) || !mpFontEntry )
        return bRet;

    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    VirtualDevice aVDev( 1 );
    Font aFont( GetFont() );
    aFont.SetShadow( false );
    aFont.SetOutline( false );
    aFont.SetRelief( RELIEF_NONE );
    aFont.SetOrientation( 0 );
    aFont.SetSize( Size( mpFontEntry->maFontSelData.mnWidth, mpFontEntry->maFontSelData.mnHeight ) );
    aVDev.SetFont( aFont );
    aVDev.SetTextAlign( ALIGN_TOP );

    // layout the text on the virtual device
    pSalLayout = aVDev.ImplLayout( rStr, nIndex, nLen, aPoint, nLayoutWidth, pDXAry );
    if( !pSalLayout )
        return false;

    // make the bitmap big enough
    // TODO: use factors when it would get too big
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent + mnEmphasisDescent;
    Point aOffset( nWidth/2, 8 );
    Size aOutSize( nWidth + 2*aOffset.X(), nHeight + 2*aOffset.Y() );
    if( !nWidth || !aVDev.SetOutputSizePixel( aOutSize ) )
        return false;

    // draw text in black
    pSalLayout->DrawBase() = aOffset;
    aVDev.SetTextColor( Color( COL_BLACK ) );
    aVDev.SetTextFillColor();
    aVDev.ImplInitTextColor();
    aVDev.ImplDrawText( *pSalLayout );
    pSalLayout->Release();

    // find extents using the bitmap
    Bitmap aBmp = aVDev.GetBitmap( Point(), aOutSize );
    BitmapReadAccess* pAcc = aBmp.AcquireReadAccess();
    if( !pAcc )
        return false;
    const BitmapColor aBlack( pAcc->GetBestMatchingColor( Color( COL_BLACK ) ) );
    const long nW = pAcc->Width();
    const long nH = pAcc->Height();
    long nLeft = 0;
    long nRight = 0;

    // find top left point
    long nTop = 0;
    for(; nTop < nH; ++nTop )
    {
        for( nLeft = 0; nLeft < nW; ++nLeft )
            if( pAcc->GetPixel( nTop, nLeft ) == aBlack )
                break;
        if( nLeft < nW )
            break;
    }

    // find bottom right point
    long nBottom = nH;
    while( --nBottom >= nTop )
    {
        for( nRight = nW; --nRight >= 0; )
            if( pAcc->GetPixel( nBottom, nRight ) == aBlack )
                break;
        if( nRight >= 0 )
            break;
    }
    if( nRight < nLeft )
    {
        long nX = nRight;
        nRight = nLeft;
        nLeft  = nX;
    }

    for( long nY = nTop; nY <= nBottom; ++nY )
    {
        // find leftmost point
        long nX;
        for( nX = 0; nX < nLeft; ++nX )
            if( pAcc->GetPixel( nY, nX ) == aBlack )
                break;
        nLeft = nX;

        // find rightmost point
        for( nX = nW; --nX > nRight; )
            if( pAcc->GetPixel( nY, nX ) == aBlack )
                break;
        nRight = nX;
    }

    aBmp.ReleaseAccess( pAcc );

    if( nTop <= nBottom )
    {
        Size aSize( nRight - nLeft + 1, nBottom - nTop + 1 );
        Point aTopLeft( nLeft, nTop );
        aTopLeft -= aOffset;
        // adjust to text alignment
        aTopLeft.Y()+= mnTextOffY - (mpFontEntry->maMetric.mnAscent + mnEmphasisAscent);
        // convert to logical coordinates
        aSize = PixelToLogic( aSize );
        aTopLeft.X() = ImplDevicePixelToLogicWidth( aTopLeft.X() );
        aTopLeft.Y() = ImplDevicePixelToLogicHeight( aTopLeft.Y() );
        rRect = Rectangle( aTopLeft, aSize );
        return true;
    }

    return false;
}

bool OutputDevice::GetTextOutlines( ::basegfx::B2DPolyPolygonVector& rVector,
                                        const OUString& rStr, sal_Int32 nBase,
                                        sal_Int32 nIndex, sal_Int32 nLen,
                                        bool bOptimize, sal_uLong nLayoutWidth, const sal_Int32* pDXArray ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutlines Suspicious arguments nLen:" << nLen);
    }
    // the fonts need to be initialized
    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        ImplInitFont();
    if( !mpFontEntry )
        return false;

    bool bRet = false;
    rVector.clear();
    if( nLen < 0 )
    {
        nLen = rStr.getLength() - nIndex;
    }
    rVector.reserve( nLen );

    // we want to get the Rectangle in logical units, so to
    // avoid rounding errors we just size the font in logical units
    bool bOldMap = mbMap;
    if( bOldMap )
    {
        const_cast<OutputDevice&>(*this).mbMap = false;
        const_cast<OutputDevice&>(*this).mbNewFont = true;
    }

    SalLayout* pSalLayout = NULL;

    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart = std::min( nBase, nIndex );
        sal_Int32 nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, Point(0,0), nLayoutWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if( pSalLayout )
    {
        bRet = pSalLayout->GetOutline( *mpGraphics, rVector );
        if( bRet )
        {
            // transform polygon to pixel units
            ::basegfx::B2DHomMatrix aMatrix;

            int nWidthFactor = pSalLayout->GetUnitsPerPixel();
            if( nXOffset | mnTextOffX | mnTextOffY )
            {
                Point aRotatedOfs( mnTextOffX*nWidthFactor, mnTextOffY*nWidthFactor );
                aRotatedOfs -= pSalLayout->GetDrawPosition( Point( nXOffset, 0 ) );
                aMatrix.translate( aRotatedOfs.X(), aRotatedOfs.Y() );
            }

            if( nWidthFactor > 1 )
            {
                double fFactor = 1.0 / nWidthFactor;
                aMatrix.scale( fFactor, fFactor );
            }

            if( !aMatrix.isIdentity() )
            {
                ::basegfx::B2DPolyPolygonVector::iterator aIt = rVector.begin();
                for(; aIt != rVector.end(); ++aIt )
                    (*aIt).transform( aMatrix );
            }
        }

        pSalLayout->Release();
    }

    if( bOldMap )
    {
        // restore original font size and map mode
        const_cast<OutputDevice&>(*this).mbMap = bOldMap;
        const_cast<OutputDevice&>(*this).mbNewFont = true;
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) || !mpFontEntry )
        return bRet;

    // fall back to bitmap conversion
    // Here, we can savely assume that the mapping between characters and glyphs
    // is one-to-one. This is most probably valid for the old bitmap fonts.
    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if (pSalLayout == 0)
        return false;
    long nOrgWidth = pSalLayout->GetTextWidth();
    long nOrgHeight = mpFontEntry->mnLineHeight + mnEmphasisAscent
        + mnEmphasisDescent;
    pSalLayout->Release();

    VirtualDevice aVDev(1);

    Font aFont(GetFont());
    aFont.SetShadow(false);
    aFont.SetOutline(false);
    aFont.SetRelief(RELIEF_NONE);
    aFont.SetOrientation(0);
    if( bOptimize )
    {
        aFont.SetSize( Size( 0, GLYPH_FONT_HEIGHT ) );
        aVDev.SetMapMode( MAP_PIXEL );
    }
    aVDev.SetFont( aFont );
    aVDev.SetTextAlign( ALIGN_TOP );
    aVDev.SetTextColor( Color(COL_BLACK) );
    aVDev.SetTextFillColor();

    pSalLayout = aVDev.ImplLayout( rStr, nIndex, nLen, Point(0,0), nLayoutWidth, pDXArray );
    if (pSalLayout == 0)
        return false;
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = ((OutputDevice*)&aVDev)->mpFontEntry->mnLineHeight + ((OutputDevice*)&aVDev)->mnEmphasisAscent
        + ((OutputDevice*)&aVDev)->mnEmphasisDescent;
    pSalLayout->Release();

    if( !nWidth || !nHeight )
        return true;
    double fScaleX = static_cast< double >(nOrgWidth) / nWidth;
    double fScaleY = static_cast< double >(nOrgHeight) / nHeight;

    // calculate offset when nBase!=nIndex
    // TODO: fix offset calculation for Bidi case
    nXOffset = 0;
    if( nBase != nIndex )
    {
        sal_Int32 nStart  = ((nBase < nIndex) ? nBase : nIndex);
        sal_Int32 nLength = ((nBase > nIndex) ? nBase : nIndex) - nStart;
        pSalLayout = aVDev.ImplLayout( rStr, nStart, nLength, Point(0,0), nLayoutWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    bRet = true;
    bool bRTL = false;
    OUString aStr( rStr ); // prepare for e.g. localized digits
    sal_Int32 nIndex2 = nIndex; // only needed until nIndex is sal_Int32
    sal_Int32 nLen2 = nLen; // only needed until nLen is sal_Int32
    ImplLayoutArgs aLayoutArgs = ImplPrepareLayoutArgs( aStr, nIndex2, nLen2, 0, NULL );
    for( int nCharPos = -1; aLayoutArgs.GetNextPos( &nCharPos, &bRTL);)
    {
        bool bSuccess = false;

        // draw character into virtual device
        pSalLayout = aVDev.ImplLayout( rStr, nCharPos, 1, Point(0,0), nLayoutWidth, pDXArray );
        if (pSalLayout == 0)
            return false;
        long nCharWidth = pSalLayout->GetTextWidth();

        Point aOffset(nCharWidth / 2, 8);
        Size aSize(nCharWidth + 2 * aOffset.X(), nHeight + 2 * aOffset.Y());
        bSuccess = (bool)aVDev.SetOutputSizePixel(aSize);
        if( bSuccess )
        {
            // draw glyph into virtual device
            aVDev.Erase();
            pSalLayout->DrawBase() += aOffset;
            pSalLayout->DrawBase() += Point( ((OutputDevice*)&aVDev)->mnTextOffX, ((OutputDevice*)&aVDev)->mnTextOffY );
            pSalLayout->DrawText( *((OutputDevice*)&aVDev)->mpGraphics );
            pSalLayout->Release();

            // convert character image into outline
            Bitmap aBmp( aVDev.GetBitmap(Point(0, 0), aSize));

            PolyPolygon aPolyPoly;
            bool bVectorized = aBmp.Vectorize(aPolyPoly, BMP_VECTORIZE_OUTER | BMP_VECTORIZE_REDUCE_EDGES);
            if( !bVectorized )
                bSuccess = false;
            else
            {
                // convert units to logical width
                for (sal_uInt16 j = 0; j < aPolyPoly.Count(); ++j)
                {
                    Polygon& rPoly = aPolyPoly[j];
                    for (sal_uInt16 k = 0; k < rPoly.GetSize(); ++k)
                    {
                        Point& rPt = rPoly[k];
                        rPt -= aOffset;
                        int nPixelX = rPt.X() - ((OutputDevice&)aVDev).mnTextOffX + nXOffset;
                        int nPixelY = rPt.Y() - ((OutputDevice&)aVDev).mnTextOffY;
                        rPt.X() = ImplDevicePixelToLogicWidth( nPixelX );
                        rPt.Y() = ImplDevicePixelToLogicHeight( nPixelY );
                    }
                }

                // ignore "empty" glyphs:
                if( aPolyPoly.Count() > 0 )
                {
                    // convert  to B2DPolyPolygon
                    // TODO: get rid of intermediate tool's PolyPolygon
                    ::basegfx::B2DPolyPolygon aB2DPolyPoly = aPolyPoly.getB2DPolyPolygon();
                    ::basegfx::B2DHomMatrix aMatrix;
                    aMatrix.scale( fScaleX, fScaleY );
                    int nAngle = GetFont().GetOrientation();
                    if( nAngle )
                        aMatrix.rotate( nAngle * F_PI1800 );
                    aB2DPolyPoly.transform( aMatrix );
                    rVector.push_back( aB2DPolyPoly );
                }
            }
        }

        nXOffset += nCharWidth;
        bRet = bRet && bSuccess;
    }

    return bRet;
}

bool OutputDevice::GetTextOutlines( PolyPolyVector& rResultVector,
                                        const OUString& rStr, sal_Int32 nBase,
                                        sal_Int32 nIndex, sal_Int32 nLen, bool bOptimize,
                                        sal_uLong nTWidth, const sal_Int32* pDXArray ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutlines Suspicious arguments  nLen:" << nLen);
    }

    rResultVector.clear();

    // get the basegfx polypolygon vector
    ::basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         bOptimize, nTWidth, pDXArray ) )
        return false;

    // convert to a tool polypolygon vector
    rResultVector.reserve( aB2DPolyPolyVector.size() );
    ::basegfx::B2DPolyPolygonVector::const_iterator aIt = aB2DPolyPolyVector.begin();
    for(; aIt != aB2DPolyPolyVector.end(); ++aIt )
        rResultVector.push_back(PolyPolygon(*aIt)); // #i76339#

    return true;
}

bool OutputDevice::GetTextOutline( PolyPolygon& rPolyPoly, const OUString& rStr,
                                       sal_Int32 nBase, sal_Int32 nIndex, sal_Int32 nLen,
                                       bool bOptimize, sal_uLong nTWidth, const sal_Int32* pDXArray ) const
{
    if(nLen == 0x0FFFF)
    {
        SAL_INFO("sal.rtl.xub",
                 "GetTextOutline Suspicious arguments nLen:" << nLen);
    }
    rPolyPoly.Clear();

    // get the basegfx polypolygon vector
    ::basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         bOptimize, nTWidth, pDXArray ) )
        return false;

    // convert and merge into a tool polypolygon
    ::basegfx::B2DPolyPolygonVector::const_iterator aIt = aB2DPolyPolyVector.begin();
    for(; aIt != aB2DPolyPolyVector.end(); ++aIt )
        for( unsigned int i = 0; i < aIt->count(); ++i )
            rPolyPoly.Insert(Polygon((*aIt).getB2DPolygon( i ))); // #i76339#

    return true;
}

bool OutputDevice::GetFontCapabilities( FontCapabilities& rFontCapabilities ) const
{
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return false;

    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        ImplInitFont();
    if( !mpFontEntry )
        return false;

    return mpGraphics->GetImplFontCapabilities(rFontCapabilities);
}

bool OutputDevice::GetFontCharMap( FontCharMap& rFontCharMap ) const
{
    rFontCharMap.Reset();

    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return false;

    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        ImplInitFont();
    if( !mpFontEntry )
        return false;

#ifdef ENABLE_IFC_CACHE // a little font charmap cache helps considerably
    static const int NMAXITEMS = 16;
    static int nUsedItems = 0, nCurItem = 0;

    struct CharMapCacheItem { const PhysicalFontFace* mpFontData; FontCharMap maCharMap; };
    static CharMapCacheItem aCache[ NMAXITEMS ];

    const PhysicalFontFace* pFontData = mpFontEntry->maFontSelData.mpFontData;

    int i;
    for( i = nUsedItems; --i >= 0; )
        if( pFontData == aCache[i].mpFontData )
            break;
    if( i >= 0 ) // found in cache
    {
        rFontCharMap.Reset( aCache[i].maCharMap.mpImpl );
    }
    else // need to cache
#endif // ENABLE_IFC_CACHE
    {
        const ImplFontCharMap* pNewMap = mpGraphics->GetImplFontCharMap();
        rFontCharMap.Reset( pNewMap );

#ifdef ENABLE_IFC_CACHE
        // manage cache round-robin and insert data
        CharMapCacheItem& rItem = aCache[ nCurItem ];
        rItem.mpFontData = pFontData;
        rItem.maCharMap.Reset( pNewMap );

        if( ++nCurItem >= NMAXITEMS )
            nCurItem = 0;

        if( ++nUsedItems >= NMAXITEMS )
            nUsedItems = NMAXITEMS;
#endif // ENABLE_IFC_CACHE
    }

    if( rFontCharMap.IsDefaultMap() )
        return false;
    return true;
}

sal_Int32 OutputDevice::HasGlyphs( const Font& rTempFont, const OUString& rStr,
    sal_Int32 nIndex, sal_Int32 nLen ) const
{
    if( nIndex >= rStr.getLength() )
        return nIndex;
    sal_Int32 nEnd;
    if( nLen == -1 )
        nEnd = rStr.getLength();
    else
        nEnd = std::min( rStr.getLength(), nIndex + nLen );

    DBG_ASSERT( nIndex < nEnd, "StartPos >= EndPos?" );
    DBG_ASSERT( nEnd <= rStr.getLength(), "String too short" );

    // to get the map temporarily set font
    const Font aOrigFont = GetFont();
    const_cast<OutputDevice&>(*this).SetFont( rTempFont );
    FontCharMap aFontCharMap;
    bool bRet = GetFontCharMap( aFontCharMap );
    const_cast<OutputDevice&>(*this).SetFont( aOrigFont );

    // if fontmap is unknown assume it doesn't have the glyphs
    if( !bRet )
        return nIndex;

    for( sal_Int32 i = nIndex; nIndex < nEnd; ++i, ++nIndex )
        if( ! aFontCharMap.HasChar( rStr[i] ) )
            return nIndex;

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
