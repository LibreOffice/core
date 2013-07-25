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


DBG_NAMEEX( OutputDevice )
DBG_NAMEEX( Font )

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

void OutputDevice::ImplUpdateFontData( bool bNewFontLists )
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
                if( mpFontList && mpFontList != pSVData->maGDIData.mpScreenFontList )
                    mpFontList->Clear();

                if( mpPDFWriter )
                {
                    if( mpFontList && mpFontList != pSVData->maGDIData.mpScreenFontList )
                        delete mpFontList;
                    if( mpFontCache && mpFontCache != pSVData->maGDIData.mpScreenFontCache )
                        delete mpFontCache;
                    mpFontList = pSVData->maGDIData.mpScreenFontList->Clone( true, true );
                    mpFontCache = new ImplFontCache( sal_False );
                }
                else
                {
                    if( mpOutDevData )
                        mpOutDevData->maDevFontSubst.Clear();
                    mpGraphics->GetDevFontList( mpFontList );
                    mpGraphics->GetDevFontSubstList( this );
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
            pChild->ImplUpdateFontData( true );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void OutputDevice::ImplUpdateAllFontData( bool bNewFontLists )
{
    ImplSVData* pSVData = ImplGetSVData();

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
                pDevice->mpGraphics->GetDevFontList(pFrame->mpWindowImpl->mpFrameData->mpFontList);
            }
        }
    }

    // update all windows
    Window* pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->ImplUpdateFontData( bNewFontLists );

        Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->ImplUpdateFontData( bNewFontLists );
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    // update all virtual devices
    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
    while ( pVirDev )
    {
        pVirDev->ImplUpdateFontData( bNewFontLists );
        pVirDev = pVirDev->mpNext;
    }

    // update all printers
    Printer* pPrinter = pSVData->maGDIData.mpFirstPrinter;
    while ( pPrinter )
    {
        pPrinter->ImplUpdateFontData( bNewFontLists );
        pPrinter = pPrinter->mpNext;
    }
}

// TODO: remove this method when the CWS-gfbfcfg dust has settled
void ImplFreeOutDevFontData()
{}

void OutputDevice::BeginFontSubstitution()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maGDIData.mbFontSubChanged = sal_False;
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
        pSVData->maGDIData.mbFontSubChanged = sal_False;
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
    ImplGetSVData()->maGDIData.mbFontSubChanged = sal_True;
}

void ImplDirectFontSubstitution::AddFontSubstitute( const String& rFontName,
    const String& rSubstFontName, sal_uInt16 nFlags )
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

void OutputDevice::ImplAddDevFontSubstitute( const OUString& rFontName,
                                             const OUString& rReplaceFontName,
                                             sal_uInt16 nFlags )
{
    ImplInitOutDevData();
    mpOutDevData->maDevFontSubst.AddFontSubstitute( rFontName, rReplaceFontName, nFlags );
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

bool ImplDirectFontSubstitution::FindFontSubstitute( String& rSubstName,
    const String& rSearchName, sal_uInt16 nFlags ) const
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

static void ImplFontSubstitute( OUString& rFontName,
    sal_uInt16 nFlags, ImplDirectFontSubstitution* pDevSpecific )
{
#ifdef DBG_UTIL
    OUString aTempName = rFontName;
    GetEnglishSearchFontName( aTempName );
    DBG_ASSERT( aTempName == rFontName, "ImplFontSubstitute() called without a searchname" );
#endif

    String aSubstFontName;

    // apply user-configurable font replacement (eg, from the list in Tools->Options)
    const ImplDirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( pSubst && pSubst->FindFontSubstitute( aSubstFontName, rFontName, FONT_SUBSTITUTE_ALWAYS ) )
    {
        rFontName = aSubstFontName;
        return;
    }

    // apply device specific font replacement (e.g. to use printer builtin fonts)
    if( !pDevSpecific )
        return;

    if( pDevSpecific->FindFontSubstitute( aSubstFontName, rFontName, nFlags ) )
    {
        rFontName = aSubstFontName;
        return;
    }
}

Font OutputDevice::GetDefaultFont( sal_uInt16 nType, LanguageType eLang,
                                   sal_uLong nFlags, const OutputDevice* pOutDev )
{
    LanguageTag aLanguageTag(
            ( eLang == LANGUAGE_NONE || eLang == LANGUAGE_SYSTEM || eLang == LANGUAGE_DONTKNOW ) ?
            Application::GetSettings().GetUILanguageTag() :
            LanguageTag( eLang ));

    utl::DefaultFontConfiguration& rDefaults = utl::DefaultFontConfiguration::get();
    String aSearch = rDefaults.getUserInterfaceFont( aLanguageTag ); // ensure a fallback
    String aDefault = rDefaults.getDefaultFont( aLanguageTag, nType );
    if( aDefault.Len() )
        aSearch = aDefault;

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

    if ( aSearch.Len() )
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
                ImplDevFontListData* pFontFamily = pOutDev->mpFontList->ImplFindBySearchName( aSearchName );
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
        if ( !aFont.GetName().Len() )
        {
            if ( nFlags & DEFAULTFONT_FLAGS_ONLYONE )
            {

                if( !pOutDev )
                    pOutDev = (const OutputDevice *)ImplGetSVData()->mpDefaultWin;
                if( !pOutDev )
                {
                    sal_Int32 nIndex = 0;
                    aFont.SetName( aSearch.GetToken( 0, ';', nIndex ) );
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
                    ImplFontEntry* pEntry = pOutDev->mpFontCache->GetFontEntry( pOutDev->mpFontList, aFont, aSize, fExactHeight, pOutDev->mpOutDevData ? &pOutDev->mpOutDevData->maDevFontSubst : NULL );
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

static unsigned ImplIsCJKFont( const String& rFontName )
{
    // Test, if Fontname includes CJK characters --> In this case we
    // mention that it is a CJK font
    const sal_Unicode* pStr = rFontName.GetBuffer();
    while ( *pStr )
    {
        // japanese
        if ( ((*pStr >= 0x3040) && (*pStr <= 0x30FF)) ||
             ((*pStr >= 0x3190) && (*pStr <= 0x319F)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_JP;

        // korean
        if ( ((*pStr >= 0xAC00) && (*pStr <= 0xD7AF)) ||
             ((*pStr >= 0x3130) && (*pStr <= 0x318F)) ||
             ((*pStr >= 0x1100) && (*pStr <= 0x11FF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_KR;

        // chinese
        if ( ((*pStr >= 0x3400) && (*pStr <= 0x9FFF)) )
            return IMPL_FONT_ATTR_CJK|IMPL_FONT_ATTR_CJK_TC|IMPL_FONT_ATTR_CJK_SC;

        // cjk
        if ( ((*pStr >= 0x3000) && (*pStr <= 0xD7AF)) ||
             ((*pStr >= 0xFF00) && (*pStr <= 0xFFEE)) )
            return IMPL_FONT_ATTR_CJK;

        pStr++;
    }

    return 0;
}

static void ImplCalcType( sal_uLong& rType, FontWeight& rWeight, FontWidth& rWidth,
                          FontFamily eFamily, const FontNameAttr* pFontAttr )
{
    if ( eFamily != FAMILY_DONTKNOW )
    {
        if ( eFamily == FAMILY_SWISS )
            rType |= IMPL_FONT_ATTR_SANSSERIF;
        else if ( eFamily == FAMILY_ROMAN )
            rType |= IMPL_FONT_ATTR_SERIF;
        else if ( eFamily == FAMILY_SCRIPT )
            rType |= IMPL_FONT_ATTR_SCRIPT;
        else if ( eFamily == FAMILY_MODERN )
            rType |= IMPL_FONT_ATTR_FIXED;
        else if ( eFamily == FAMILY_DECORATIVE )
            rType |= IMPL_FONT_ATTR_DECORATIVE;
    }

    if ( pFontAttr )
    {
        rType |= pFontAttr->Type;

        if ( ((rWeight == WEIGHT_DONTKNOW) || (rWeight == WEIGHT_NORMAL)) &&
             (pFontAttr->Weight != WEIGHT_DONTKNOW) )
            rWeight = pFontAttr->Weight;
        if ( ((rWidth == WIDTH_DONTKNOW) || (rWidth == WIDTH_NORMAL)) &&
             (pFontAttr->Width != WIDTH_DONTKNOW) )
            rWidth = pFontAttr->Width;
    }
}

PhysicalFontFace::PhysicalFontFace( const ImplDevFontAttributes& rDFA, int nMagic )
:   ImplDevFontAttributes( rDFA ),
    mnWidth(0),
    mnHeight(0),
    mnMagic( nMagic ),
    mpNext( NULL )
{
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( !IsSymbolFont() )
        if ( IsStarSymbol( GetFamilyName() ) )
            SetSymbolFlag( true );
}

sal_Int32 PhysicalFontFace::CompareIgnoreSize( const PhysicalFontFace& rOther ) const
{
    // compare their width, weight, italic and style name
    if( GetWidthType() < rOther.GetWidthType() )
        return COMPARE_LESS;
    else if( GetWidthType() > rOther.GetWidthType() )
        return COMPARE_GREATER;

    if( GetWeight() < rOther.GetWeight() )
        return COMPARE_LESS;
    else if( GetWeight() > rOther.GetWeight() )
        return COMPARE_GREATER;

    if( GetSlant() < rOther.GetSlant() )
        return COMPARE_LESS;
    else if( GetSlant() > rOther.GetSlant() )
        return COMPARE_GREATER;

    sal_Int32 eCompare = GetFamilyName().compareTo( rOther.GetFamilyName() );
    return eCompare;
}

sal_Int32 PhysicalFontFace::CompareWithSize( const PhysicalFontFace& rOther ) const
{
    sal_Int32 eCompare = CompareIgnoreSize( rOther );
    if( eCompare != COMPARE_EQUAL )
        return eCompare;

    if( mnHeight < rOther.mnHeight )
        return COMPARE_LESS;
    else if( mnHeight > rOther.mnHeight )
        return COMPARE_GREATER;

    if( mnWidth < rOther.mnWidth )
        return COMPARE_LESS;
    else if( mnWidth > rOther.mnWidth )
        return COMPARE_GREATER;

    return COMPARE_EQUAL;
}

struct FontMatchStatus
{
public:
    int                 mnFaceMatch;
    int                 mnHeightMatch;
    int                 mnWidthMatch;
    const sal_Unicode*  mpTargetStyleName;
};

bool PhysicalFontFace::IsBetterMatch( const FontSelectPattern& rFSD, FontMatchStatus& rStatus ) const
{
    int nMatch = 0;

    const OUString& rFontName = rFSD.maTargetName;
    if( rFontName.equalsIgnoreAsciiCase( GetFamilyName() ) )
        nMatch += 240000;

    if( rStatus.mpTargetStyleName
    &&  GetStyleName().equalsIgnoreAsciiCase( rStatus.mpTargetStyleName ) )
        nMatch += 120000;

    if( (rFSD.GetPitch() != PITCH_DONTKNOW) && (rFSD.GetPitch() == GetPitch()) )
        nMatch += 20000;

    // prefer NORMAL font width
    // TODO: change when the upper layers can tell their width preference
    if( GetWidthType() == WIDTH_NORMAL )
        nMatch += 400;
    else if( (GetWidthType() == WIDTH_SEMI_EXPANDED) || (GetWidthType() == WIDTH_SEMI_CONDENSED) )
        nMatch += 300;

    if( rFSD.GetWeight() != WEIGHT_DONTKNOW )
    {
        // if not bold or requiring emboldening prefer light fonts to bold fonts
        FontWeight ePatternWeight = rFSD.mbEmbolden ? WEIGHT_NORMAL : rFSD.GetWeight();

        int nReqWeight = (int)ePatternWeight;
        if ( ePatternWeight > WEIGHT_MEDIUM )
            nReqWeight += 100;

        int nGivenWeight = (int)GetWeight();
        if( GetWeight() > WEIGHT_MEDIUM )
            nGivenWeight += 100;

        int nWeightDiff = nReqWeight - nGivenWeight;

        if ( nWeightDiff == 0 )
            nMatch += 1000;
        else if ( nWeightDiff == +1 || nWeightDiff == -1 )
            nMatch += 700;
        else if ( nWeightDiff < +50 && nWeightDiff > -50)
            nMatch += 200;
    }
    else // requested weight == WEIGHT_DONTKNOW
    {
        // prefer NORMAL font weight
        // TODO: change when the upper layers can tell their weight preference
        if( GetWeight() == WEIGHT_NORMAL )
            nMatch += 450;
        else if( GetWeight() == WEIGHT_MEDIUM )
            nMatch += 350;
        else if( (GetWeight() == WEIGHT_SEMILIGHT) || (GetWeight() == WEIGHT_SEMIBOLD) )
            nMatch += 200;
        else if( GetWeight() == WEIGHT_LIGHT )
            nMatch += 150;
    }

    // if requiring custom matrix to fake italic, prefer upright font
    FontItalic ePatternItalic = rFSD.maItalicMatrix != ItalicMatrix() ? ITALIC_NONE : rFSD.GetSlant();

    if ( ePatternItalic == ITALIC_NONE )
    {
        if( GetSlant() == ITALIC_NONE )
            nMatch += 900;
    }
    else
    {
        if( ePatternItalic == GetSlant() )
            nMatch += 900;
        else if( GetSlant() != ITALIC_NONE )
            nMatch += 600;
    }

    if( mbDevice )
        nMatch += 1;

    int nHeightMatch = 0;
    int nWidthMatch = 0;

    if( IsScalable() )
    {
        if( rFSD.mnOrientation != 0 )
            nMatch += 80;
        else if( rFSD.mnWidth != 0 )
            nMatch += 25;
        else
            nMatch += 5;
    }
    else
    {
        if( rFSD.mnHeight == mnHeight )
        {
            nMatch += 20;
            if( rFSD.mnWidth == mnWidth )
                nMatch += 10;
        }
        else
        {
            // for non-scalable fonts the size difference is very important
            // prefer the smaller font face because of clipping/overlapping issues
            int nHeightDiff = (rFSD.mnHeight - mnHeight) * 1000;
            nHeightMatch = (nHeightDiff >= 0) ? -nHeightDiff : 100+nHeightDiff;
            if( rFSD.mnHeight )
                nHeightMatch /= rFSD.mnHeight;

            if( (rFSD.mnWidth != 0) && (mnWidth != 0) && (rFSD.mnWidth != mnWidth) )
            {
                int nWidthDiff = (rFSD.mnWidth - mnWidth) * 100;
                nWidthMatch = (nWidthDiff >= 0) ? -nWidthDiff : +nWidthDiff;
            }
        }
    }

    if( rStatus.mnFaceMatch > nMatch )
        return false;
    else if( rStatus.mnFaceMatch < nMatch )
    {
        rStatus.mnFaceMatch      = nMatch;
        rStatus.mnHeightMatch    = nHeightMatch;
        rStatus.mnWidthMatch     = nWidthMatch;
        return true;
    }

    // when two fonts are still competing prefer the
    // one with the best matching height
    if( rStatus.mnHeightMatch > nHeightMatch )
        return false;
    else if( rStatus.mnHeightMatch < nHeightMatch )
    {
        rStatus.mnHeightMatch    = nHeightMatch;
        rStatus.mnWidthMatch     = nWidthMatch;
        return true;
    }

    if( rStatus.mnWidthMatch > nWidthMatch )
        return false;

    rStatus.mnWidthMatch = nWidthMatch;
    return true;
}

ImplFontEntry::ImplFontEntry( const FontSelectPattern& rFontSelData )
:   maFontSelData( rFontSelData ),
    maMetric( rFontSelData ),
    mpConversion( NULL ),
    mnRefCount( 1 ),
    mnSetFontFlags( 0 ),
    mnOwnOrientation( 0 ),
    mnOrientation( 0 ),
    mbInit( false ),
    mpUnicodeFallbackList( NULL )
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

inline void ImplFontEntry::AddFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const String& rFontName )
{
    if( !mpUnicodeFallbackList )
        mpUnicodeFallbackList = new UnicodeFallbackList;
    (*mpUnicodeFallbackList)[ GFBCacheKey(cChar,eWeight) ] = rFontName;
}

inline bool ImplFontEntry::GetFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, OUString* pFontName ) const
{
    if( !mpUnicodeFallbackList )
        return false;

    UnicodeFallbackList::const_iterator it = mpUnicodeFallbackList->find( GFBCacheKey(cChar,eWeight) );
    if( it == mpUnicodeFallbackList->end() )
        return false;

    *pFontName = (*it).second;
    return true;
}

inline void ImplFontEntry::IgnoreFallbackForUnicode( sal_UCS4 cChar, FontWeight eWeight, const String& rFontName )
{
//  DBG_ASSERT( mpUnicodeFallbackList, "ImplFontEntry::IgnoreFallbackForUnicode no list" );
    UnicodeFallbackList::iterator it = mpUnicodeFallbackList->find( GFBCacheKey(cChar,eWeight) );
//  DBG_ASSERT( it != mpUnicodeFallbackList->end(), "ImplFontEntry::IgnoreFallbackForUnicode no match" );
    if( it == mpUnicodeFallbackList->end() )
        return;
    if( (*it).second == rFontName )
        mpUnicodeFallbackList->erase( it );
}

ImplDevFontListData::ImplDevFontListData( const String& rSearchName )
:   mpFirst( NULL ),
    maSearchName( rSearchName ),
    mnTypeFaces( 0 ),
    mnMatchType( 0 ),
    meMatchWeight( WEIGHT_DONTKNOW ),
    meMatchWidth( WIDTH_DONTKNOW ),
    meFamily( FAMILY_DONTKNOW ),
    mePitch( PITCH_DONTKNOW ),
    mnMinQuality( -1 )
{}

ImplDevFontListData::~ImplDevFontListData()
{
    // release all physical font faces
    while( mpFirst )
    {
        PhysicalFontFace* pFace = mpFirst;
        mpFirst = pFace->GetNextFace();
        delete pFace;
    }
}

bool ImplDevFontListData::AddFontFace( PhysicalFontFace* pNewData )
{
    pNewData->mpNext = NULL;

    if( !mpFirst )
    {
        maName         = pNewData->GetFamilyName();
        maMapNames     = pNewData->maMapNames;
        meFamily       = pNewData->GetFamilyType();
        mePitch        = pNewData->GetPitch();
        mnMinQuality   = pNewData->mnQuality;
    }
    else
    {
        if( meFamily == FAMILY_DONTKNOW )
            meFamily = pNewData->GetFamilyType();
        if( mePitch == PITCH_DONTKNOW )
            mePitch = pNewData->GetPitch();
        if( mnMinQuality > pNewData->mnQuality )
            mnMinQuality = pNewData->mnQuality;
    }

    // set attributes for attribute based font matching
    if( pNewData->IsScalable() )
        mnTypeFaces |= IMPL_DEVFONT_SCALABLE;

    if( pNewData->IsSymbolFont() )
        mnTypeFaces |= IMPL_DEVFONT_SYMBOL;
    else
        mnTypeFaces |= IMPL_DEVFONT_NONESYMBOL;

    if( pNewData->GetWeight() != WEIGHT_DONTKNOW )
    {
        if( pNewData->GetWeight() >= WEIGHT_SEMIBOLD )
            mnTypeFaces |= IMPL_DEVFONT_BOLD;
        else if( pNewData->GetWeight() <= WEIGHT_SEMILIGHT )
            mnTypeFaces |= IMPL_DEVFONT_LIGHT;
        else
            mnTypeFaces |= IMPL_DEVFONT_NORMAL;
    }

    if( pNewData->GetSlant() == ITALIC_NONE )
        mnTypeFaces |= IMPL_DEVFONT_NONEITALIC;
    else if( (pNewData->GetSlant() == ITALIC_NORMAL)
         ||  (pNewData->GetSlant() == ITALIC_OBLIQUE) )
        mnTypeFaces |= IMPL_DEVFONT_ITALIC;

    if( (meMatchWeight == WEIGHT_DONTKNOW)
    ||  (meMatchWidth  == WIDTH_DONTKNOW)
    ||  (mnMatchType   == 0) )
    {
        // TODO: is it cheaper to calc matching attributes now or on demand?
        // calc matching attributes if other entries are already initialized

        // Do lazy, quite expensive, not needed in start-up!
        // const FontSubstConfiguration& rFontSubst = *FontSubstConfiguration::get();
        // InitMatchData( rFontSubst, maSearchName );
        // mbMatchData=true; // Somewhere else???
    }

    // reassign name (sharing saves memory)
    if( pNewData->GetFamilyName() == GetFamilyName() )
        pNewData->SetFamilyName( GetFamilyName() );

    // insert new physical font face into linked list
    // TODO: get rid of linear search?
    PhysicalFontFace* pData;
    PhysicalFontFace** ppHere = &mpFirst;
    for(; (pData=*ppHere) != NULL; ppHere=&pData->mpNext )
    {
        sal_Int32 eComp = pNewData->CompareWithSize( *pData );
        if( eComp == COMPARE_GREATER )
            continue;
        if( eComp == COMPARE_LESS )
            break;

        // ignore duplicate if its quality is worse
        if( pNewData->mnQuality < pData->mnQuality )
            return false;

        // keep the device font if its quality is good enough
        if( (pNewData->mnQuality == pData->mnQuality)
        &&  (pData->mbDevice || !pNewData->mbDevice) )
            return false;

        // replace existing font face with a better one
        pNewData->mpNext = pData->mpNext;
        *ppHere = pNewData;
        delete pData;
        return true;
    }

    // insert into or append to list of physical font faces
    pNewData->mpNext = pData;
    *ppHere = pNewData;
    return true;
}

// get font attributes using the normalized font family name
void ImplDevFontListData::InitMatchData( const utl::FontSubstConfiguration& rFontSubst,
    const String& rSearchName )
{
    OUString aShortName;
    OUString aMatchFamilyName(maMatchFamilyName);
    // get font attributes from the decorated font name
    rFontSubst.getMapName( rSearchName, aShortName, aMatchFamilyName,
                            meMatchWeight, meMatchWidth, mnMatchType );
    maMatchFamilyName = aMatchFamilyName;
    const FontNameAttr* pFontAttr = rFontSubst.getSubstInfo( rSearchName );
    // eventually use the stripped name
    if( !pFontAttr )
        if( aShortName != rSearchName )
            pFontAttr = rFontSubst.getSubstInfo( aShortName );
    ImplCalcType( mnMatchType, meMatchWeight, meMatchWidth, meFamily, pFontAttr );
    mnMatchType |= ImplIsCJKFont( maName );
}

PhysicalFontFace* ImplDevFontListData::FindBestFontFace( const FontSelectPattern& rFSD ) const
{
    if( !mpFirst )
        return NULL;
    if( !mpFirst->GetNextFace() )
        return mpFirst;

    // FontName+StyleName should map to FamilyName+StyleName
    const String& rSearchName = rFSD.maTargetName;
    const sal_Unicode* pTargetStyleName = NULL;
    if( (rSearchName.Len() > maSearchName.Len())
    &&   rSearchName.Equals( maSearchName, 0, maSearchName.Len() ) )
        pTargetStyleName = rSearchName.GetBuffer() + maSearchName.Len() + 1;

    // TODO: linear search improve!
    PhysicalFontFace* pFontFace = mpFirst;
    PhysicalFontFace* pBestFontFace = pFontFace;
    FontMatchStatus aFontMatchStatus = {0,0,0, pTargetStyleName};
    for(; pFontFace; pFontFace = pFontFace->GetNextFace() )
        if( pFontFace->IsBetterMatch( rFSD, aFontMatchStatus ) )
            pBestFontFace = pFontFace;

    return pBestFontFace;
}

// update device font list with unique font faces, with uniqueness
// meaning different font attributes, but not different fonts sizes
void ImplDevFontListData::UpdateDevFontList( ImplGetDevFontList& rDevFontList ) const
{
    PhysicalFontFace* pPrevFace = NULL;
    for( PhysicalFontFace* pFace = mpFirst; pFace; pFace = pFace->GetNextFace() )
    {
        if( !pPrevFace || pFace->CompareIgnoreSize( *pPrevFace ) )
            rDevFontList.Add( pFace );
        pPrevFace = pFace;
    }
}

void ImplDevFontListData::GetFontHeights( std::set<int>& rHeights ) const
{
    // add all available font heights
    for( const PhysicalFontFace* pFace = mpFirst; pFace; pFace = pFace->GetNextFace() )
        rHeights.insert( pFace->GetHeight() );
}

void ImplDevFontListData::UpdateCloneFontList( ImplDevFontList& rDevFontList,
    bool bScalable, bool bEmbeddable ) const
{
    for( PhysicalFontFace* pFace = mpFirst; pFace; pFace = pFace->GetNextFace() )
    {
        if( bScalable && !pFace->IsScalable() )
            continue;
        if( bEmbeddable && !pFace->IsEmbeddable() && !pFace->IsSubsettable() )
            continue;

        PhysicalFontFace* pClonedFace = pFace->Clone();
        rDevFontList.Add( pClonedFace );
    }
}

ImplDevFontList::ImplDevFontList()
:   mbMatchData( false )
,   mbMapNames( false )
,   mpPreMatchHook( NULL )
,   mpFallbackHook( NULL )
,   mpFallbackList( NULL )
,   mnFallbackCount( -1 )
{}

ImplDevFontList::~ImplDevFontList()
{
    Clear();
}

void ImplDevFontList::SetPreMatchHook( ImplPreMatchFontSubstitution* pHook )
{
    mpPreMatchHook = pHook;
}

void ImplDevFontList::SetFallbackHook( ImplGlyphFallbackFontSubstitution* pHook )
{
    mpFallbackHook = pHook;
}

void ImplDevFontList::Clear()
{
    // remove fallback lists
    delete[] mpFallbackList;
    mpFallbackList = NULL;
    mnFallbackCount = -1;

    // clear all entries in the device font list
    DevFontList::iterator it = maDevFontList.begin();
    for(; it != maDevFontList.end(); ++it )
    {
        ImplDevFontListData* pEntry = (*it).second;
        delete pEntry;
    }

    maDevFontList.clear();

    // match data must be recalculated too
    mbMatchData = false;
}

void ImplDevFontList::InitGenericGlyphFallback( void ) const
{
    // normalized family names of fonts suited for glyph fallback
    // if a font is available related fonts can be ignored
    // TODO: implement dynamic lists
    static const char* aGlyphFallbackList[] = {
        // empty strings separate the names of unrelated fonts
        "eudc", "",
        "arialunicodems", "cyberbit", "code2000", "",
        "andalesansui", "",
        "starsymbol", "opensymbol", "",
        "msmincho", "fzmingti", "fzheiti", "ipamincho", "sazanamimincho", "kochimincho", "",
        "sunbatang", "sundotum", "baekmukdotum", "gulim", "batang", "dotum", "",
        "hgmincholightj", "msunglightsc", "msunglighttc", "hymyeongjolightk", "",
        "tahoma", "dejavusans", "timesnewroman", "liberationsans", "",
        "shree", "mangal", "",
        "raavi", "shruti", "tunga", "",
        "latha", "gautami", "kartika", "vrinda", "",
        "shayyalmt", "naskmt", "scheherazade", "",
        "david", "nachlieli", "lucidagrande", "",
        "norasi", "angsanaupc", "",
        "khmerossystem", "",
        "muktinarrow", "",
        "phetsarathot", "",
        "padauk", "pinlonmyanmar", "",
        "iskoolapota", "lklug", "",
        0
    };

    bool bHasEudc = false;
    int nMaxLevel = 0;
    int nBestQuality = 0;
    ImplDevFontListData** pFallbackList = NULL;
    for( const char** ppNames = &aGlyphFallbackList[0];; ++ppNames )
    {
        // advance to next sub-list when end-of-sublist marker
        if( !**ppNames ) // #i46456# check for empty string, i.e., deref string itself not only ptr to it
        {
            if( nBestQuality > 0 )
                if( ++nMaxLevel >= MAX_FALLBACK )
                    break;
            if( !ppNames[1] )
                break;
            nBestQuality = 0;
            continue;
        }

        // test if the glyph fallback candidate font is available and scalable
        String aTokenName( *ppNames, RTL_TEXTENCODING_UTF8 );
        ImplDevFontListData* pFallbackFont = FindFontFamily( aTokenName );
        if( !pFallbackFont )
            continue;
        if( !pFallbackFont->IsScalable() )
            continue;

        // keep the best font of the glyph fallback sub-list
        if( nBestQuality < pFallbackFont->GetMinQuality() )
        {
            nBestQuality = pFallbackFont->GetMinQuality();
            // store available glyph fallback fonts
            if( !pFallbackList )
                pFallbackList = new ImplDevFontListData*[ MAX_FALLBACK ];
            pFallbackList[ nMaxLevel ] = pFallbackFont;
            if( !bHasEudc && !nMaxLevel )
                bHasEudc = !strncmp( *ppNames, "eudc", 5 );
        }
    }

#ifdef SAL_FONTENUM_STABLE_ON_PLATFORM // #i113472#
    // sort the list of fonts for glyph fallback by quality (highest first)
    // #i33947# keep the EUDC font at the front of the list
    // an insertion sort is good enough for this short list
    const int nSortStart = bHasEudc ? 1 : 0;
    for( int i = nSortStart+1, j; i < nMaxLevel; ++i )
    {
        ImplDevFontListData* pTestFont = pFallbackList[ i ];
        int nTestQuality = pTestFont->GetMinQuality();
        for( j = i; --j >= nSortStart; )
            if( nTestQuality > pFallbackList[j]->GetMinQuality() )
                pFallbackList[ j+1 ] = pFallbackList[ j ];
            else
                break;
        pFallbackList[ j+1 ] = pTestFont;
    }
#endif

    mnFallbackCount = nMaxLevel;
    mpFallbackList  = pFallbackList;
}

ImplDevFontListData* ImplDevFontList::GetGlyphFallbackFont( FontSelectPattern& rFontSelData,
    OUString& rMissingCodes, int nFallbackLevel ) const
{
    ImplDevFontListData* pFallbackData = NULL;

    // find a matching font candidate for platform specific glyph fallback
    if( mpFallbackHook )
    {
        // check cache for the first matching entry
        // to avoid calling the expensive fallback hook (#i83491#)
        sal_UCS4 cChar = 0;
        bool bCached = true;
        sal_Int32 nStrIndex = 0;
        while( nStrIndex < rMissingCodes.getLength() )
        {
            cChar = rMissingCodes.iterateCodePoints( &nStrIndex );
            bCached = rFontSelData.mpFontEntry->GetFallbackForUnicode( cChar, rFontSelData.GetWeight(), &rFontSelData.maSearchName );
            // ignore entries which don't have a fallback
            if( !bCached || !rFontSelData.maSearchName.isEmpty() )
                break;
        }

        if( bCached )
        {
            // there is a matching fallback in the cache
            // so update rMissingCodes with codepoints not yet resolved by this fallback
            int nRemainingLength = 0;
            sal_UCS4* pRemainingCodes = (sal_UCS4*)alloca( rMissingCodes.getLength() * sizeof(sal_UCS4) );
            OUString aFontName;
            while( nStrIndex < rMissingCodes.getLength() )
            {
                cChar = rMissingCodes.iterateCodePoints( &nStrIndex );
                bCached = rFontSelData.mpFontEntry->GetFallbackForUnicode( cChar, rFontSelData.GetWeight(), &aFontName );
                if( !bCached || (rFontSelData.maSearchName != aFontName) )
                    pRemainingCodes[ nRemainingLength++ ] = cChar;
            }
            rMissingCodes = OUString( pRemainingCodes, nRemainingLength );
        }
        else
        {
            OUString aOldMissingCodes = rMissingCodes;
            // call the hook to query the best matching glyph fallback font
            if( mpFallbackHook->FindFontSubstitute( rFontSelData, rMissingCodes ) )
                // apply outdev3.cxx specific fontname normalization
                GetEnglishSearchFontName( rFontSelData.maSearchName );
            else
                rFontSelData.maSearchName = OUString();

            // See fdo#32665 for an example. FreeSerif that has glyphs in normal
            // font, but not in the italic or bold version
            bool bSubSetOfFontRequiresPropertyFaking = rFontSelData.mbEmbolden || rFontSelData.maItalicMatrix != ItalicMatrix();

            // Cache the result even if there was no match, unless its from part of a font for which the properties need
            // to be faked. We need to rework this cache to take into account that fontconfig can return different fonts
            // for different input sizes, weights, etc. Basically the cache is way to naive
            if (!bSubSetOfFontRequiresPropertyFaking)
            {
                for(;;)
                {
                     if( !rFontSelData.mpFontEntry->GetFallbackForUnicode( cChar, rFontSelData.GetWeight(), &rFontSelData.maSearchName ) )
                         rFontSelData.mpFontEntry->AddFallbackForUnicode( cChar, rFontSelData.GetWeight(), rFontSelData.maSearchName );
                     if( nStrIndex >= aOldMissingCodes.getLength() )
                         break;
                     cChar = aOldMissingCodes.iterateCodePoints( &nStrIndex );
                }
                if( !rFontSelData.maSearchName.isEmpty() )
                {
                    // remove cache entries that were still not resolved
                    for( nStrIndex = 0; nStrIndex < rMissingCodes.getLength(); )
                    {
                        cChar = rMissingCodes.iterateCodePoints( &nStrIndex );
                        rFontSelData.mpFontEntry->IgnoreFallbackForUnicode( cChar, rFontSelData.GetWeight(), rFontSelData.maSearchName );
                    }
                }
            }
        }

        // find the matching device font
        if( !rFontSelData.maSearchName.isEmpty() )
            pFallbackData = FindFontFamily( rFontSelData.maSearchName );
    }

    // else find a matching font candidate for generic glyph fallback
    if( !pFallbackData )
    {
        // initialize font candidates for generic glyph fallback if needed
        if( mnFallbackCount < 0 )
            InitGenericGlyphFallback();
        // TODO: adjust nFallbackLevel by number of levels resolved by the fallback hook
        if( nFallbackLevel < mnFallbackCount )
            pFallbackData = mpFallbackList[ nFallbackLevel ];
    }

    return pFallbackData;
}

void ImplDevFontList::Add( PhysicalFontFace* pNewData )
{
    OUString aSearchName = pNewData->GetFamilyName();
    GetEnglishSearchFontName( aSearchName );

    DevFontList::const_iterator it = maDevFontList.find( aSearchName );
    ImplDevFontListData* pFoundData = NULL;
    if( it != maDevFontList.end() )
        pFoundData = (*it).second;

    if( !pFoundData )
    {
        pFoundData = new ImplDevFontListData( aSearchName );
        maDevFontList[ aSearchName ] = pFoundData;
    }

    bool bKeepNewData = pFoundData->AddFontFace( pNewData );

    if( !bKeepNewData )
        delete pNewData;
}

// find the font from the normalized font family name
ImplDevFontListData* ImplDevFontList::ImplFindBySearchName( const OUString& rSearchName ) const
{
#ifdef DEBUG
    OUString aTempName = rSearchName;
    GetEnglishSearchFontName( aTempName );
    DBG_ASSERT( aTempName == rSearchName, "ImplDevFontList::ImplFindBySearchName() called with non-normalized name" );
#endif

    DevFontList::const_iterator it = maDevFontList.find( rSearchName );
    if( it == maDevFontList.end() )
        return NULL;

    ImplDevFontListData* pFoundData = (*it).second;
    return pFoundData;
}

ImplDevFontListData* ImplDevFontList::ImplFindByAliasName(const OUString& rSearchName,
    const OUString& rShortName) const
{
    // short circuit for impossible font name alias
    if (rSearchName.isEmpty())
        return NULL;

    // short circuit if no alias names are available
    if (!mbMapNames)
        return NULL;

    // use the font's alias names to find the font
    // TODO: get rid of linear search
    DevFontList::const_iterator it = maDevFontList.begin();
    while( it != maDevFontList.end() )
    {
        ImplDevFontListData* pData = (*it).second;
        if( pData->maMapNames.isEmpty() )
            continue;

        // if one alias name matches we found a matching font
        OUString aTempName;
        sal_Int32 nIndex = 0;
        do
        {
           aTempName = GetNextFontToken( pData->maMapNames, nIndex );
           // Test, if the Font name match with one of the mapping names
           if ( (aTempName == rSearchName) || (aTempName == rShortName) )
              return pData;
        }
        while ( nIndex != -1 );
     }

     return NULL;
}

ImplDevFontListData* ImplDevFontList::FindFontFamily( const String& rFontName ) const
{
    // normalize the font family name and
    OUString aName = rFontName;
    GetEnglishSearchFontName( aName );
    ImplDevFontListData* pFound = ImplFindBySearchName( aName );
    return pFound;
}

ImplDevFontListData* ImplDevFontList::ImplFindByTokenNames(const OUString& rTokenStr) const
{
    ImplDevFontListData* pFoundData = NULL;

    // use normalized font name tokens to find the font
    for( sal_Int32 nTokenPos = 0; nTokenPos != -1; )
    {
        OUString aSearchName = GetNextFontToken( rTokenStr, nTokenPos );
        if( aSearchName.isEmpty() )
            continue;
        GetEnglishSearchFontName( aSearchName );
        pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            break;
    }

    return pFoundData;
}

ImplDevFontListData* ImplDevFontList::ImplFindBySubstFontAttr( const utl::FontNameAttr& rFontAttr ) const
{
    ImplDevFontListData* pFoundData = NULL;

    // use the font substitutions suggested by the FontNameAttr to find the font
    ::std::vector< OUString >::const_iterator it = rFontAttr.Substitutions.begin();
    for(; it != rFontAttr.Substitutions.end(); ++it )
    {
        OUString aSearchName( *it );
        GetEnglishSearchFontName( aSearchName );

        pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    // use known attributes from the configuration to find a matching substitute
    const sal_uLong nSearchType = rFontAttr.Type;
    if( nSearchType != 0 )
    {
        const FontWeight eSearchWeight = rFontAttr.Weight;
        const FontWidth  eSearchWidth  = rFontAttr.Width;
        const FontItalic eSearchSlant  = ITALIC_DONTKNOW;
        const String aSearchName;
        pFoundData = ImplFindByAttributes( nSearchType,
            eSearchWeight, eSearchWidth, eSearchSlant, aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    return NULL;
}

void ImplDevFontList::InitMatchData() const
{
    // short circuit if already done
    if( mbMatchData )
        return;
    mbMatchData = true;

    // calculate MatchData for all entries
    const FontSubstConfiguration& rFontSubst = FontSubstConfiguration::get();

    DevFontList::const_iterator it = maDevFontList.begin();
    for(; it != maDevFontList.end(); ++it )
    {
        const String& rSearchName = (*it).first;
        ImplDevFontListData* pEntry = (*it).second;

        pEntry->InitMatchData( rFontSubst, rSearchName );
    }
}

ImplDevFontListData* ImplDevFontList::ImplFindByAttributes( sal_uLong nSearchType,
    FontWeight eSearchWeight, FontWidth eSearchWidth,
    FontItalic eSearchItalic, const OUString& rSearchFamilyName ) const
{
    if( (eSearchItalic != ITALIC_NONE) && (eSearchItalic != ITALIC_DONTKNOW) )
        nSearchType |= IMPL_FONT_ATTR_ITALIC;

    // don't bother to match attributes if the attributes aren't worth matching
    if( !nSearchType
    && ((eSearchWeight == WEIGHT_DONTKNOW) || (eSearchWeight == WEIGHT_NORMAL))
    && ((eSearchWidth == WIDTH_DONTKNOW) || (eSearchWidth == WIDTH_NORMAL)) )
        return NULL;

    InitMatchData();
    ImplDevFontListData* pFoundData = NULL;

    long    nTestMatch;
    long    nBestMatch = 40000;
    sal_uLong   nBestType = 0;

    DevFontList::const_iterator it = maDevFontList.begin();
    for(; it != maDevFontList.end(); ++it )
    {
        ImplDevFontListData* pData = (*it).second;

        // Get all information about the matching font
        sal_uLong       nMatchType  = pData->mnMatchType;
        FontWeight  eMatchWeight= pData->meMatchWeight;
        FontWidth   eMatchWidth = pData->meMatchWidth;

        // Calculate Match Value
        // 1000000000
        //  100000000
        //   10000000   CJK, CTL, None-Latin, Symbol
        //    1000000   FamilyName, Script, Fixed, -Special, -Decorative,
        //              Titling, Capitals, Outline, Shadow
        //     100000   Match FamilyName, Serif, SansSerif, Italic,
        //              Width, Weight
        //      10000   Scalable, Standard, Default,
        //              full, Normal, Knownfont,
        //              Otherstyle, +Special, +Decorative,
        //       1000   Typewriter, Rounded, Gothic, Schollbook
        //        100
        nTestMatch = 0;

        // test CJK script attributes
        if ( nSearchType & IMPL_FONT_ATTR_CJK )
        {
            // Matching language
            if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_CJK_ALLLANG) )
                nTestMatch += 10000000*3;
            if( nMatchType & IMPL_FONT_ATTR_CJK )
                nTestMatch += 10000000*2;
            if( nMatchType & IMPL_FONT_ATTR_FULL )
                nTestMatch += 10000000;
        }
        else if ( nMatchType & IMPL_FONT_ATTR_CJK )
            nTestMatch -= 10000000;

        // test CTL script attributes
        if( nSearchType & IMPL_FONT_ATTR_CTL )
        {
            if( nMatchType & IMPL_FONT_ATTR_CTL )
                nTestMatch += 10000000*2;
            if( nMatchType & IMPL_FONT_ATTR_FULL )
                nTestMatch += 10000000;
        }
        else if ( nMatchType & IMPL_FONT_ATTR_CTL )
            nTestMatch -= 10000000;

        // test LATIN script attributes
        if( nSearchType & IMPL_FONT_ATTR_NONELATIN )
        {
            if( nMatchType & IMPL_FONT_ATTR_NONELATIN )
                nTestMatch += 10000000*2;
            if( nMatchType & IMPL_FONT_ATTR_FULL )
                nTestMatch += 10000000;
        }

        // test SYMBOL attributes
        if ( nSearchType & IMPL_FONT_ATTR_SYMBOL )
        {
            const String& rSearchName = it->first;
            // prefer some special known symbol fonts
            if ( rSearchName.EqualsAscii( "starsymbol" ) )
                nTestMatch += 10000000*6+(10000*3);
            else if ( rSearchName.EqualsAscii( "opensymbol" ) )
                nTestMatch += 10000000*6;
            else if ( rSearchName.EqualsAscii( "starbats" )
            ||        rSearchName.EqualsAscii( "wingdings" )
            ||        rSearchName.EqualsAscii( "monotypesorts" )
            ||        rSearchName.EqualsAscii( "dingbats" )
            ||        rSearchName.EqualsAscii( "zapfdingbats" ) )
                nTestMatch += 10000000*5;
            else if ( pData->mnTypeFaces & IMPL_DEVFONT_SYMBOL )
                nTestMatch += 10000000*4;
            else
            {
                if( nMatchType & IMPL_FONT_ATTR_SYMBOL )
                    nTestMatch += 10000000*2;
                if( nMatchType & IMPL_FONT_ATTR_FULL )
                    nTestMatch += 10000000;
            }
        }
        else if ( (pData->mnTypeFaces & (IMPL_DEVFONT_SYMBOL | IMPL_DEVFONT_NONESYMBOL)) == IMPL_DEVFONT_SYMBOL )
            nTestMatch -= 10000000;
        else if ( nMatchType & IMPL_FONT_ATTR_SYMBOL )
            nTestMatch -= 10000;

        // match stripped family name
        if( !rSearchFamilyName.isEmpty() && (rSearchFamilyName.equals(pData->maMatchFamilyName)) )
            nTestMatch += 1000000*3;

        // match ALLSCRIPT? attribute
        if( nSearchType & IMPL_FONT_ATTR_ALLSCRIPT )
        {
            if( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
                nTestMatch += 1000000*2;
            if( nSearchType & IMPL_FONT_ATTR_ALLSUBSCRIPT )
            {
                if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_ALLSUBSCRIPT) )
                    nTestMatch += 1000000*2;
                if( 0 != ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_BRUSHSCRIPT) )
                    nTestMatch -= 1000000;
            }
        }
        else if( nMatchType & IMPL_FONT_ATTR_ALLSCRIPT )
            nTestMatch -= 1000000;

        // test MONOSPACE+TYPEWRITER attributes
        if( nSearchType & IMPL_FONT_ATTR_FIXED )
        {
            if( nMatchType & IMPL_FONT_ATTR_FIXED )
                nTestMatch += 1000000*2;
            // a typewriter attribute is even better
            if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_TYPEWRITER) )
                nTestMatch += 10000*2;
        }
        else if( nMatchType & IMPL_FONT_ATTR_FIXED )
            nTestMatch -= 1000000;

        // test SPECIAL attribute
        if( nSearchType & IMPL_FONT_ATTR_SPECIAL )
        {
            if( nMatchType & IMPL_FONT_ATTR_SPECIAL )
                nTestMatch += 10000;
            else if( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
            {
                 if( nMatchType & IMPL_FONT_ATTR_SERIF )
                     nTestMatch += 1000*2;
                 else if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                     nTestMatch += 1000;
             }
        }
        else if( (nMatchType & IMPL_FONT_ATTR_SPECIAL) && !(nSearchType & IMPL_FONT_ATTR_SYMBOL) )
            nTestMatch -= 1000000;

        // test DECORATIVE attribute
        if( nSearchType & IMPL_FONT_ATTR_DECORATIVE )
        {
            if( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
                nTestMatch += 10000;
            else if( !(nSearchType & IMPL_FONT_ATTR_ALLSERIFSTYLE) )
            {
                if( nMatchType & IMPL_FONT_ATTR_SERIF )
                    nTestMatch += 1000*2;
                else if ( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                    nTestMatch += 1000;
            }
        }
        else if( nMatchType & IMPL_FONT_ATTR_DECORATIVE )
            nTestMatch -= 1000000;

        // test TITLE+CAPITALS attributes
        if( nSearchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
        {
            if( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
                nTestMatch += 1000000*2;
            if( 0 == ((nSearchType^nMatchType) & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS)))
                nTestMatch += 1000000;
            else if( (nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS))
            &&       (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
                nTestMatch += 1000000;
        }
        else if( nMatchType & (IMPL_FONT_ATTR_TITLING | IMPL_FONT_ATTR_CAPITALS) )
            nTestMatch -= 1000000;

        // test OUTLINE+SHADOW attributes
        if( nSearchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
        {
            if( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
                nTestMatch += 1000000*2;
            if( 0 == ((nSearchType ^ nMatchType) & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW)) )
                nTestMatch += 1000000;
            else if( (nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW))
            &&       (nMatchType & (IMPL_FONT_ATTR_STANDARD | IMPL_FONT_ATTR_DEFAULT)) )
                nTestMatch += 1000000;
        }
        else if ( nMatchType & (IMPL_FONT_ATTR_OUTLINE | IMPL_FONT_ATTR_SHADOW) )
            nTestMatch -= 1000000;

        // test font name substrings
        // TODO: calculate name matching score using e.g. Levenstein distance
        if( (rSearchFamilyName.getLength() >= 4) && (pData->maMatchFamilyName.Len() >= 4)
        &&    ((rSearchFamilyName.indexOf( pData->maMatchFamilyName ) != -1)
            || (pData->maMatchFamilyName.Search( rSearchFamilyName ) != STRING_NOTFOUND)) )
                    nTestMatch += 5000;

        // test SERIF attribute
        if( nSearchType & IMPL_FONT_ATTR_SERIF )
        {
            if( nMatchType & IMPL_FONT_ATTR_SERIF )
                nTestMatch += 1000000*2;
            else if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                nTestMatch -= 1000000;
        }

        // test SANSERIF attribute
        if( nSearchType & IMPL_FONT_ATTR_SANSSERIF )
        {
            if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                nTestMatch += 1000000;
            else if ( nMatchType & IMPL_FONT_ATTR_SERIF )
                nTestMatch -= 1000000;
        }

        // test ITALIC attribute
        if( nSearchType & IMPL_FONT_ATTR_ITALIC )
        {
            if( pData->mnTypeFaces & IMPL_DEVFONT_ITALIC )
                nTestMatch += 1000000*3;
            if( nMatchType & IMPL_FONT_ATTR_ITALIC )
                nTestMatch += 1000000;
        }
        else if( !(nSearchType & IMPL_FONT_ATTR_ALLSCRIPT)
            &&  ((nMatchType & IMPL_FONT_ATTR_ITALIC)
                || !(pData->mnTypeFaces & IMPL_DEVFONT_NONEITALIC)) )
            nTestMatch -= 1000000*2;

        // test WIDTH attribute
        if( (eSearchWidth != WIDTH_DONTKNOW) && (eSearchWidth != WIDTH_NORMAL) )
        {
            if( eSearchWidth < WIDTH_NORMAL )
            {
                if( eSearchWidth == eMatchWidth )
                    nTestMatch += 1000000*3;
                else if( (eMatchWidth < WIDTH_NORMAL) && (eMatchWidth != WIDTH_DONTKNOW) )
                    nTestMatch += 1000000;
            }
            else
            {
                if( eSearchWidth == eMatchWidth )
                    nTestMatch += 1000000*3;
                else if( eMatchWidth > WIDTH_NORMAL )
                    nTestMatch += 1000000;
            }
        }
        else if( (eMatchWidth != WIDTH_DONTKNOW) && (eMatchWidth != WIDTH_NORMAL) )
            nTestMatch -= 1000000;

        // test WEIGHT attribute
        if( (eSearchWeight != WEIGHT_DONTKNOW) && (eSearchWeight != WEIGHT_NORMAL) && (eSearchWeight != WEIGHT_MEDIUM) )
        {
            if( eSearchWeight < WEIGHT_NORMAL )
            {
                if( pData->mnTypeFaces & IMPL_DEVFONT_LIGHT )
                    nTestMatch += 1000000;
                if( (eMatchWeight < WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_DONTKNOW) )
                    nTestMatch += 1000000;
            }
            else
            {
                if( pData->mnTypeFaces & IMPL_DEVFONT_BOLD )
                    nTestMatch += 1000000;
                if( eMatchWeight > WEIGHT_BOLD )
                    nTestMatch += 1000000;
            }
        }
        else if( ((eMatchWeight != WEIGHT_DONTKNOW) && (eMatchWeight != WEIGHT_NORMAL) && (eMatchWeight != WEIGHT_MEDIUM))
            || !(pData->mnTypeFaces & IMPL_DEVFONT_NORMAL) )
            nTestMatch -= 1000000;

        // prefer scalable fonts
        if( pData->mnTypeFaces & IMPL_DEVFONT_SCALABLE )
            nTestMatch += 10000*4;
        else
            nTestMatch -= 10000*4;

        // test STANDARD+DEFAULT+FULL+NORMAL attributes
        if( nMatchType & IMPL_FONT_ATTR_STANDARD )
            nTestMatch += 10000*2;
        if( nMatchType & IMPL_FONT_ATTR_DEFAULT )
            nTestMatch += 10000;
        if( nMatchType & IMPL_FONT_ATTR_FULL )
            nTestMatch += 10000;
        if( nMatchType & IMPL_FONT_ATTR_NORMAL )
            nTestMatch += 10000;

        // test OTHERSTYLE attribute
        if( ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_OTHERSTYLE) != 0 )
        {
            nTestMatch -= 10000;
        }

        // test ROUNDED attribute
        if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_ROUNDED) )
            nTestMatch += 1000;

        // test TYPEWRITER attribute
        if( 0 == ((nSearchType ^ nMatchType) & IMPL_FONT_ATTR_TYPEWRITER) )
            nTestMatch += 1000;

        // test GOTHIC attribute
        if( nSearchType & IMPL_FONT_ATTR_GOTHIC )
        {
            if( nMatchType & IMPL_FONT_ATTR_GOTHIC )
                nTestMatch += 1000*3;
            if( nMatchType & IMPL_FONT_ATTR_SANSSERIF )
                nTestMatch += 1000*2;
        }

        // test SCHOOLBOOK attribute
        if( nSearchType & IMPL_FONT_ATTR_SCHOOLBOOK )
        {
            if( nMatchType & IMPL_FONT_ATTR_SCHOOLBOOK )
                nTestMatch += 1000*3;
            if( nMatchType & IMPL_FONT_ATTR_SERIF )
                nTestMatch += 1000*2;
        }

        // compare with best matching font yet
        if ( nTestMatch > nBestMatch )
        {
            pFoundData  = pData;
            nBestMatch  = nTestMatch;
            nBestType   = nMatchType;
        }
        else if( nTestMatch == nBestMatch )
        {
            // some fonts are more suitable defaults
            if( nMatchType & IMPL_FONT_ATTR_DEFAULT )
            {
                pFoundData  = pData;
                nBestType   = nMatchType;
            }
            else if( (nMatchType & IMPL_FONT_ATTR_STANDARD) &&
                    !(nBestType & IMPL_FONT_ATTR_DEFAULT) )
            {
                 pFoundData  = pData;
                 nBestType   = nMatchType;
            }
        }
    }

    return pFoundData;
}

ImplDevFontListData* ImplDevFontList::FindDefaultFont() const
{
    // try to find one of the default fonts of the
    // UNICODE, SANSSERIF, SERIF or FIXED default font lists
    const DefaultFontConfiguration& rDefaults = DefaultFontConfiguration::get();
    LanguageTag aLanguageTag( OUString( "en"));
    String aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_SANS_UNICODE );
    ImplDevFontListData* pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_SANS );
    pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_SERIF );
    pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    aFontname = rDefaults.getDefaultFont( aLanguageTag, DEFAULTFONT_FIXED );
    pFoundData = ImplFindByTokenNames( aFontname );
    if( pFoundData )
        return pFoundData;

    // now try to find a reasonable non-symbol font

    InitMatchData();

    DevFontList::const_iterator it = maDevFontList.begin();
    for(; it !=  maDevFontList.end(); ++it )
    {
        ImplDevFontListData* pData = (*it).second;
        if( pData->mnMatchType & IMPL_FONT_ATTR_SYMBOL )
            continue;
        pFoundData = pData;
        if( pData->mnMatchType & (IMPL_FONT_ATTR_DEFAULT|IMPL_FONT_ATTR_STANDARD) )
            break;
    }
    if( pFoundData )
        return pFoundData;

    // finding any font is better than finding no font at all
    it = maDevFontList.begin();
    if( it !=  maDevFontList.end() )
        pFoundData = (*it).second;

    return pFoundData;
}

ImplDevFontList* ImplDevFontList::Clone( bool bScalable, bool bEmbeddable ) const
{
    ImplDevFontList* pClonedList = new ImplDevFontList;
    pClonedList->mbMapNames     = mbMapNames;
    pClonedList->mpPreMatchHook = mpPreMatchHook;
    pClonedList->mpFallbackHook = mpFallbackHook;

    // TODO: clone the config-font attributes too?
    pClonedList->mbMatchData    = false;

    DevFontList::const_iterator it = maDevFontList.begin();
    for(; it != maDevFontList.end(); ++it )
    {
        const ImplDevFontListData* pFontFace = (*it).second;
        pFontFace->UpdateCloneFontList( *pClonedList, bScalable, bEmbeddable );
    }

    return pClonedList;
}

ImplGetDevFontList* ImplDevFontList::GetDevFontList() const
{
    ImplGetDevFontList* pGetDevFontList = new ImplGetDevFontList;

    DevFontList::const_iterator it = maDevFontList.begin();
    for(; it != maDevFontList.end(); ++it )
    {
        const ImplDevFontListData* pFontFamily = (*it).second;
        pFontFamily->UpdateDevFontList( *pGetDevFontList );
    }

    return pGetDevFontList;
}

ImplGetDevSizeList* ImplDevFontList::GetDevSizeList( const String& rFontName ) const
{
    ImplGetDevSizeList* pGetDevSizeList = new ImplGetDevSizeList( rFontName );

    ImplDevFontListData* pFontFamily = FindFontFamily( rFontName );
    if( pFontFamily != NULL )
    {
        std::set<int> rHeights;
        pFontFamily->GetFontHeights( rHeights );

        std::set<int>::const_iterator it = rHeights.begin();
        for(; it != rHeights.begin(); ++it )
            pGetDevSizeList->Add( *it );
    }

    return pGetDevSizeList;
}

FontSelectPatternAttributes::FontSelectPatternAttributes( const Font& rFont,
    const String& rSearchName, const Size& rSize, float fExactHeight )
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
    const String& rSearchName, const Size& rSize, float fExactHeight)
    : FontSelectPatternAttributes(rFont, rSearchName, rSize, fExactHeight)
    , mpFontData( NULL )
    , mpFontEntry( NULL )
{
}

// NOTE: this ctor is still used on Windows. Do not remove.
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

#ifdef WNT
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

ImplFontCache::ImplFontCache( bool bPrinter )
:   mpFirstEntry( NULL ),
    mnRef0Count( 0 ),
    mbPrinter( bPrinter )
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

ImplFontEntry* ImplFontCache::GetFontEntry( ImplDevFontList* pFontList,
    const Font& rFont, const Size& rSize, float fExactHeight, ImplDirectFontSubstitution* pDevSpecific )
{
    String aSearchName = rFont.GetName();

    // initialize internal font request object
    FontSelectPattern aFontSelData( rFont, aSearchName, rSize, fExactHeight );
    return GetFontEntry( pFontList, aFontSelData, pDevSpecific );
}

ImplFontEntry* ImplFontCache::GetFontEntry( ImplDevFontList* pFontList,
    FontSelectPattern& aFontSelData, ImplDirectFontSubstitution* pDevSpecific )
{
    // check if a directly matching logical font instance is already cached,
    // the most recently used font usually has a hit rate of >50%
    ImplFontEntry *pEntry = NULL;
    ImplDevFontListData* pFontFamily = NULL;
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
        pFontFamily = pFontList->ImplFindByFont( aFontSelData, mbPrinter, pDevSpecific );
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
        }

        // add the new entry to the cache
        maFontInstanceList[ aFontSelData ] = pEntry;
    }

    mpFirstEntry = pEntry;
    return pEntry;
}

namespace
{
    OUString stripCharSetFromName(OUString aName)
    {
        // I worry that someone will have a font which *does* have
        // e.g. "Greek" legitimately at the end of its name :-(
        const char*suffixes[] =
        {
            " baltic",
            " ce",
            " cyr",
            " greek",
            " tur",
            " (arabic)",
            " (hebrew)",
            " (thai)",
            " (vietnamese)"
        };

        // These can be crazily piled up, e.g. Times New Roman CYR Greek
        bool bFinished = false;
        while (!bFinished)
        {
            bFinished = true;
            for (size_t i = 0; i < SAL_N_ELEMENTS(suffixes); ++i)
            {
                size_t nLen = strlen(suffixes[i]);
                if (aName.endsWithIgnoreAsciiCaseAsciiL(suffixes[i], nLen))
                {
                    bFinished = false;
                    aName = aName.copy(0, aName.getLength() - nLen);
                }
            }
        }
        return aName;
    }
}

ImplDevFontListData* ImplDevFontList::ImplFindByFont( FontSelectPattern& rFSD,
    bool bPrinter, ImplDirectFontSubstitution* pDevSpecific ) const
{
    // give up if no fonts are available
    if( !Count() )
        return NULL;

    // test if a font in the token list is available
    // substitute the font if this was requested
    sal_uInt16 nSubstFlags = FONT_SUBSTITUTE_ALWAYS;
    if ( bPrinter )
        nSubstFlags |= FONT_SUBSTITUTE_SCREENONLY;

    bool bMultiToken = false;
    sal_Int32 nTokenPos = 0;
    OUString& aSearchName = rFSD.maSearchName; // TODO: get rid of reference
    for(;;)
    {
        rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
        aSearchName = rFSD.maTargetName;

#if ENABLE_GRAPHITE
        // Until features are properly supported, they are appended to the
        // font name, so we need to strip them off so the font is found.
        sal_Int32 nFeat = aSearchName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX);
        String aOrigName = rFSD.maTargetName;
        String aBaseFontName(aSearchName, 0, (nFeat != -1)?nFeat:aSearchName.getLength());
        if (nFeat != -1 && -1 !=
            aSearchName.indexOf(grutils::GrFeatureParser::FEAT_ID_VALUE_SEPARATOR, nFeat))
        {
            aSearchName = aBaseFontName;
            rFSD.maTargetName = aBaseFontName;
        }

#endif

        GetEnglishSearchFontName( aSearchName );
        ImplFontSubstitute( aSearchName, nSubstFlags, pDevSpecific );
        // #114999# special emboldening for Ricoh fonts
        // TODO: smarter check for special cases by using PreMatch infrastructure?
        if( (rFSD.GetWeight() > WEIGHT_MEDIUM)
        &&  aSearchName.startsWithIgnoreAsciiCase( "hg" ) )
        {
            OUString aBoldName;
            if( aSearchName.startsWithIgnoreAsciiCase( "hggothicb" ) )
                aBoldName = OUString("hggothice");
            else if( aSearchName.startsWithIgnoreAsciiCase( "hgpgothicb" ) )
                aBoldName = OUString("hgpgothice");
            else if( aSearchName.startsWithIgnoreAsciiCase( "hgminchol" ) )
                aBoldName = OUString("hgminchob");
            else if( aSearchName.startsWithIgnoreAsciiCase( "hgpminchol" ) )
                aBoldName = OUString("hgpminchob");
            else if( aSearchName.equalsIgnoreAsciiCase( "hgminchob" ) )
                aBoldName = OUString("hgminchoe");
            else if( aSearchName.equalsIgnoreAsciiCase( "hgpminchob" ) )
                aBoldName = OUString("hgpminchoe");

            if( !aBoldName.isEmpty() && ImplFindBySearchName( aBoldName ) )
            {
                // the other font is available => use it
                aSearchName = aBoldName;
                // prevent synthetic emboldening of bold version
                rFSD.SetWeight(WEIGHT_DONTKNOW);
            }
        }

#if ENABLE_GRAPHITE
        // restore the features to make the font selection data unique
        rFSD.maTargetName = aOrigName;
#endif
        // check if the current font name token or its substitute is valid
        ImplDevFontListData* pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;

        // some systems provide special customization
        // e.g. they suggest "serif" as UI-font, but this name cannot be used directly
        //      because the system wants to map it to another font first, e.g. "Helvetica"
#if ENABLE_GRAPHITE
        // use the target name to search in the prematch hook
        rFSD.maTargetName = aBaseFontName;
#endif

        // Related: fdo#49271 RTF files often contain weird-ass
        // Win 3.1/Win95 style fontnames which attempt to put the
        // charset encoding into the filename
        // http://www.webcenter.ru/~kazarn/eng/fonts_ttf.htm
        OUString sStrippedName = stripCharSetFromName(rFSD.maTargetName);
        if (!sStrippedName.equals(rFSD.maTargetName))
        {
            rFSD.maTargetName = sStrippedName;
            aSearchName = rFSD.maTargetName;
            GetEnglishSearchFontName(aSearchName);
            pFoundData = ImplFindBySearchName(aSearchName);
            if( pFoundData )
                return pFoundData;
        }

        if( mpPreMatchHook )
        {
            if( mpPreMatchHook->FindFontSubstitute( rFSD ) )
                GetEnglishSearchFontName( aSearchName );
        }
#if ENABLE_GRAPHITE
        // the prematch hook uses the target name to search, but we now need
        // to restore the features to make the font selection data unique
        rFSD.maTargetName = aOrigName;
#endif
        pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;

        // break after last font name token was checked unsuccessfully
        if( nTokenPos == -1)
            break;
        bMultiToken = true;
    }

    // if the first font was not available find the next available font in
    // the semicolon separated list of font names. A font is also considered
    // available when there is a matching entry in the Tools->Options->Fonts
    // dialog witho neither ALWAYS nor SCREENONLY flags set and the substitution
    // font is available
    for( nTokenPos = 0; nTokenPos != -1; )
    {
        if( bMultiToken )
        {
            rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
            aSearchName = rFSD.maTargetName;
            GetEnglishSearchFontName( aSearchName );
        }
        else
            nTokenPos = -1;
        if( mpPreMatchHook )
            if( mpPreMatchHook->FindFontSubstitute( rFSD ) )
                GetEnglishSearchFontName( aSearchName );
        ImplFontSubstitute( aSearchName, nSubstFlags, pDevSpecific );
        ImplDevFontListData* pFoundData = ImplFindBySearchName( aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    // if no font with a directly matching name is available use the
    // first font name token and get its attributes to find a replacement
    if ( bMultiToken )
    {
        nTokenPos = 0;
        rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
        aSearchName = rFSD.maTargetName;
        GetEnglishSearchFontName( aSearchName );
    }

    OUString      aSearchShortName;
    OUString      aSearchFamilyName;
    FontWeight  eSearchWeight   = rFSD.GetWeight();
    FontWidth   eSearchWidth    = rFSD.GetWidthType();
    sal_uLong   nSearchType     = 0;
    FontSubstConfiguration::getMapName( aSearchName, aSearchShortName, aSearchFamilyName,
                                        eSearchWeight, eSearchWidth, nSearchType );

    // note: the search name was already translated to english (if possible)
    // use the font's shortened name if needed
    if ( aSearchShortName != aSearchName )
    {
       ImplDevFontListData* pFoundData = ImplFindBySearchName( aSearchShortName );
       if( pFoundData )
       {
#ifdef UNX
            /* #96738# don't use mincho as an replacement for "MS Mincho" on X11: Mincho is
            a korean bitmap font that is not suitable here. Use the font replacement table,
            that automatically leads to the desired "HG Mincho Light J". Same story for
            MS Gothic, there are thai and korean "Gothic" fonts, so we even prefer Andale */
            static OUString aMS_Mincho( "msmincho" );
            static OUString aMS_Gothic( "msgothic" );
            if ((aSearchName != aMS_Mincho) && (aSearchName != aMS_Gothic))
                // TODO: add heuristic to only throw out the fake ms* fonts
#endif
            {
                return pFoundData;
            }
        }
    }

    // use font fallback
    const FontNameAttr* pFontAttr = NULL;
    if( !aSearchName.isEmpty() )
    {
        // get fallback info using FontSubstConfiguration and
        // the target name, it's shortened name and family name in that order
        const FontSubstConfiguration& rFontSubst = FontSubstConfiguration::get();
        pFontAttr = rFontSubst.getSubstInfo( aSearchName );
        if ( !pFontAttr && (aSearchShortName != aSearchName) )
            pFontAttr = rFontSubst.getSubstInfo( aSearchShortName );
        if ( !pFontAttr && (aSearchFamilyName != aSearchShortName) )
            pFontAttr = rFontSubst.getSubstInfo( aSearchFamilyName );

        // try the font substitutions suggested by the fallback info
        if( pFontAttr )
        {
            ImplDevFontListData* pFoundData = ImplFindBySubstFontAttr( *pFontAttr );
            if( pFoundData )
                return pFoundData;
        }
    }

    // if a target symbol font is not available use a default symbol font
    if( rFSD.IsSymbolFont() )
    {
        LanguageTag aDefaultLanguageTag( OUString( "en"));
        aSearchName = DefaultFontConfiguration::get().getDefaultFont( aDefaultLanguageTag, DEFAULTFONT_SYMBOL );
        ImplDevFontListData* pFoundData = ImplFindByTokenNames( aSearchName );
        if( pFoundData )
            return pFoundData;
    }

    // now try the other font name tokens
    while( nTokenPos != -1 )
    {
        rFSD.maTargetName = GetNextFontToken( rFSD.GetFamilyName(), nTokenPos );
        if( rFSD.maTargetName.isEmpty() )
            continue;

        aSearchName = rFSD.maTargetName;
        GetEnglishSearchFontName( aSearchName );

        OUString      aTempShortName;
        OUString      aTempFamilyName;
        sal_uLong   nTempType   = 0;
        FontWeight  eTempWeight = rFSD.GetWeight();
        FontWidth   eTempWidth  = WIDTH_DONTKNOW;
        FontSubstConfiguration::getMapName( aSearchName, aTempShortName, aTempFamilyName,
                                            eTempWeight, eTempWidth, nTempType );

        // use a shortend token name if available
        if( aTempShortName != aSearchName )
        {
            ImplDevFontListData* pFoundData = ImplFindBySearchName( aTempShortName );
            if( pFoundData )
                return pFoundData;
        }

        // use a font name from font fallback list to determine font attributes
        // get fallback info using FontSubstConfiguration and
        // the target name, it's shortened name and family name in that order
        const FontSubstConfiguration& rFontSubst = FontSubstConfiguration::get();
        const FontNameAttr* pTempFontAttr = rFontSubst.getSubstInfo( aSearchName );
        if ( !pTempFontAttr && (aTempShortName != aSearchName) )
            pTempFontAttr = rFontSubst.getSubstInfo( aTempShortName );
        if ( !pTempFontAttr && (aTempFamilyName != aTempShortName) )
            pTempFontAttr = rFontSubst.getSubstInfo( aTempFamilyName );

        // try the font substitutions suggested by the fallback info
        if( pTempFontAttr )
        {
            ImplDevFontListData* pFoundData = ImplFindBySubstFontAttr( *pTempFontAttr );
            if( pFoundData )
                return pFoundData;
            if( !pFontAttr )
                pFontAttr = pTempFontAttr;
        }
    }

    // if still needed use the alias names of the installed fonts
    if( mbMapNames )
    {
        ImplDevFontListData* pFoundData = ImplFindByAliasName( rFSD.maTargetName, aSearchShortName );
        if( pFoundData )
            return pFoundData;
    }

    // if still needed use the font request's attributes to find a good match
    if (MsLangId::isSimplifiedChinese(rFSD.meLanguage))
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_SC;
    else if (MsLangId::isTraditionalChinese(rFSD.meLanguage))
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_TC;
    else if (MsLangId::isKorean(rFSD.meLanguage))
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_KR;
    else if (rFSD.meLanguage == LANGUAGE_JAPANESE)
        nSearchType |= IMPL_FONT_ATTR_CJK | IMPL_FONT_ATTR_CJK_JP;
    else
    {
        nSearchType |= ImplIsCJKFont( rFSD.GetFamilyName() );
        if( rFSD.IsSymbolFont() )
            nSearchType |= IMPL_FONT_ATTR_SYMBOL;
    }

    ImplCalcType( nSearchType, eSearchWeight, eSearchWidth, rFSD.GetFamilyType(), pFontAttr );
    ImplDevFontListData* pFoundData = ImplFindByAttributes( nSearchType,
        eSearchWeight, eSearchWidth, rFSD.GetSlant(), aSearchFamilyName );

    if( pFoundData )
    {
        // overwrite font selection attributes using info from the typeface flags
        if( (eSearchWeight >= WEIGHT_BOLD)
        &&  (eSearchWeight > rFSD.GetWeight())
        &&  (pFoundData->mnTypeFaces & IMPL_DEVFONT_BOLD) )
            rFSD.SetWeight( eSearchWeight );
        else if( (eSearchWeight < WEIGHT_NORMAL)
        &&  (eSearchWeight < rFSD.GetWeight())
        &&  (eSearchWeight != WEIGHT_DONTKNOW)
        &&  (pFoundData->mnTypeFaces & IMPL_DEVFONT_LIGHT) )
            rFSD.SetWeight( eSearchWeight );

        if( (nSearchType & IMPL_FONT_ATTR_ITALIC)
        &&  ((rFSD.GetSlant() == ITALIC_DONTKNOW) || (rFSD.GetSlant() == ITALIC_NONE))
        &&  (pFoundData->mnTypeFaces & IMPL_DEVFONT_ITALIC) )
            rFSD.SetItalic( ITALIC_NORMAL );
    }
    else
    {
        // if still needed fall back to default fonts
        pFoundData = FindDefaultFont();
    }

    return pFoundData;
}

ImplFontEntry* ImplFontCache::GetGlyphFallbackFont( ImplDevFontList* pFontList,
    FontSelectPattern& rFontSelData, int nFallbackLevel, OUString& rMissingCodes )
{
    // get a candidate font for glyph fallback
    // unless the previously selected font got a device specific substitution
    // e.g. PsPrint Arial->Helvetica for udiaeresis when Helvetica doesn't support it
    if( nFallbackLevel >= 1)
    {
        ImplDevFontListData* pFallbackData = NULL;

        //fdo#33898 If someone has EUDC installed then they really want that to
        //be used as the first-choice glyph fallback seeing as it's filled with
        //private area codes with don't make any sense in any other font so
        //prioritise it here if it's available. Ideally we would remove from
        //rMissingCodes all the glyphs which it is able to resolve as an
        //optimization, but that's tricky to achieve cross-platform without
        //sufficient heavy-weight code that's likely to undo the value of the
        //optimization
        if (nFallbackLevel == 1)
            pFallbackData = pFontList->FindFontFamily(OUString("EUDC"));
        if (!pFallbackData)
            pFallbackData = pFontList->GetGlyphFallbackFont(rFontSelData, rMissingCodes, nFallbackLevel-1);
        // escape when there are no font candidates
        if( !pFallbackData  )
            return NULL;
        // override the font name
        rFontSelData.SetFamilyName( pFallbackData->GetFamilyName() );
        // clear the cached normalized name
        rFontSelData.maSearchName = String();
    }

    // get device font without doing device specific substitutions
    ImplFontEntry* pFallbackFont = GetFontEntry( pFontList, rFontSelData, NULL );
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
    for ( xub_StrLen i = 0; i < mnLines; i++ )
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
    for ( xub_StrLen i = 0; i < mnLines; i++ )
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

sal_Bool OutputDevice::ImplIsUnderlineAbove( const Font& rFont )
{
    if ( !rFont.IsVertical() )
        return sal_False;

    if( (LANGUAGE_JAPANESE == rFont.GetLanguage())
    ||  (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage()) )
        // the underline is right for Japanese only
        return sal_True;

    return sal_False;
}

void OutputDevice::ImplInitFontList() const
{
    if( ! mpFontList->Count() )
    {
        if( mpGraphics || ImplGetGraphics() )
        {
            SAL_INFO( "vcl.gdi", "OutputDevice::ImplInitFontList()" );
            mpGraphics->GetDevFontList( mpFontList );
        }
    }
    if( meOutDevType == OUTDEV_WINDOW && ! mpFontList->Count() )
    {
        String aError( "Application error: no fonts and no vcl resource found on your system" );
        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr )
        {
            String aResStr(ResId(SV_ACCESSERROR_NO_FONTS, *pMgr).toString());
            if( aResStr.Len() )
                aError = aResStr;
        }
        Application::Abort( aError );
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
        mbInitTextColor = sal_False;
    }
}

bool OutputDevice::ImplNewFont() const
{
    DBG_TESTSOLARMUTEX();

    // get correct font list on the PDF writer if necessary
    if( mpPDFWriter )
    {
        const ImplSVData* pSVData = ImplGetSVData();
        if( mpFontList == pSVData->maGDIData.mpScreenFontList
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
    ImplDirectFontSubstitution* pDevSpecificSubst = NULL;
    if( mpOutDevData )
        pDevSpecificSubst = &mpOutDevData->maDevFontSubst;
    ImplFontEntry* pOldEntry = mpFontEntry;
    mpFontEntry = mpFontCache->GetFontEntry( mpFontList, maFont, aSize, fExactHeight, pDevSpecificSubst );
    if( pOldEntry )
        mpFontCache->Release( pOldEntry );

    ImplFontEntry* pFontEntry = mpFontEntry;

    if (!pFontEntry)
        return false;

    // mark when lower layers need to get involved
    mbNewFont = sal_False;
    if( pFontEntry != pOldEntry )
        mbInitFont = sal_True;

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
            mbMap = sal_False;
            mbNewFont = sal_True;
            ImplNewFont();  // recurse once using stretched width
            mbMap = sal_True;
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
        mbInitLineColor = sal_True;
    }
    mpGraphics->SetFillColor( ImplColorToSal( GetTextFillColor() ) );
    mbInitFillColor = sal_True;

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
:   ImplFontAttributes( rFontSelData )
{
    // initialize the members provided by the font request
    mnWidth        = rFontSelData.mnWidth;
    mnSlant        = rFontSelData.GetSlant();
    mnOrientation  = sal::static_int_cast<short>(rFontSelData.mnOrientation);

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

    // reset metrics that are usually measured for the font instance
    mnAscent       = 0;
    mnDescent      = 0;
    mnIntLeading   = 0;
    mnExtLeading   = 0;
    mnMinKashida   = 0;

    // reset metrics that are usually derived from the measurements
    mnUnderlineSize            = 0;
    mnUnderlineOffset          = 0;
    mnBUnderlineSize           = 0;
    mnBUnderlineOffset         = 0;
    mnDUnderlineSize           = 0;
    mnDUnderlineOffset1        = 0;
    mnDUnderlineOffset2        = 0;
    mnWUnderlineSize           = 0;
    mnWUnderlineOffset         = 0;
    mnAboveUnderlineSize       = 0;
    mnAboveUnderlineOffset     = 0;
    mnAboveBUnderlineSize      = 0;
    mnAboveBUnderlineOffset    = 0;
    mnAboveDUnderlineSize      = 0;
    mnAboveDUnderlineOffset1   = 0;
    mnAboveDUnderlineOffset2   = 0;
    mnAboveWUnderlineSize      = 0;
    mnAboveWUnderlineOffset    = 0;
    mnStrikeoutSize            = 0;
    mnStrikeoutOffset          = 0;
    mnBStrikeoutSize           = 0;
    mnBStrikeoutOffset         = 0;
    mnDStrikeoutSize           = 0;
    mnDStrikeoutOffset1        = 0;
    mnDStrikeoutOffset2        = 0;
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
        mbInitLineColor = sal_True;

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
                mbInitLineColor = sal_True;
            }
            mpGraphics->SetFillColor( ImplColorToSal( rColor ) );
            mbInitFillColor = sal_True;
            bDrawPixAsRect  = true;
            nPixWidth       = nLineWidth;
            nPixHeight      = ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY;
        }
        else
        {
            mpGraphics->SetLineColor( ImplColorToSal( rColor ) );
            mbInitLineColor = sal_True;
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
                                         sal_Bool bIsAbove )
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
                                             sal_Bool bIsAbove )
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
            mbInitLineColor = sal_True;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aColor ) );
        mbInitFillColor = sal_True;

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
            mbInitLineColor = sal_True;
        }
        mpGraphics->SetFillColor( ImplColorToSal( aColor ) );
        mbInitFillColor = sal_True;

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
    pLayout = ImplLayout( aStrikeoutText, 0, STRING_LEN );
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
                                     sal_Bool bUnderlineAbove )
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
        ImplDrawWaveTextLine( nX, nY, nDistX, 0, nWidth, eOverline, aOverlineColor, sal_True );
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
        ImplDrawStraightTextLine( nX, nY, nDistX, 0, nWidth, eOverline, aOverlineColor, sal_True );

    if ( !bStrikeoutDone )
        ImplDrawStrikeoutLine( nX, nY, nDistX, 0, nWidth, eStrikeout, aStrikeoutColor );
}

void OutputDevice::ImplDrawTextLines( SalLayout& rSalLayout,
    FontStrikeout eStrikeout, FontUnderline eUnderline, FontUnderline eOverline, sal_Bool bWordLine, sal_Bool bUnderlineAbove )
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
            sal_GlyphId nGlyphIndex;
            if( !rSalLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                break;

            // calculate the boundaries of each word
            if( !rSalLayout.IsSpacingGlyph( nGlyphIndex ) )
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
    if( /*ImplHasMirroredGraphics() &&*/ IsRTLEnabled() )
    {
        // --- RTL ---
        // add some strange offset
        nX += 2;
        // revert the hack that will be done later in ImplDrawTextLine
        nX = nBaseX - nWidth - (nX - nBaseX - 1);
    }

    ImplDrawTextLine( nX, nY, 0, nWidth, STRIKEOUT_NONE, UNDERLINE_SINGLE, UNDERLINE_NONE, sal_False );
}

void OutputDevice::ImplGetEmphasisMark( PolyPolygon& rPolyPoly, sal_Bool& rPolyLine,
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
    rPolyLine   = sal_False;

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
                    rPolyLine = sal_True;
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
                                         const PolyPolygon& rPolyPoly, sal_Bool bPolyLine,
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
    sal_Bool                bOldMap         = mbMap;
    GDIMetaFile*        pOldMetaFile    = mpMetaFile;
    mpMetaFile = NULL;
    EnableMapMode( sal_False );

    FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
    PolyPolygon         aPolyPoly;
    Rectangle           aRect1;
    Rectangle           aRect2;
    long                nEmphasisYOff;
    long                nEmphasisWidth;
    long                nEmphasisHeight;
    sal_Bool                bPolyLine;

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
        sal_GlyphId nGlyphIndex;
        if( !rSalLayout.GetNextGlyphs( 1, &nGlyphIndex, aOutPoint, nStart ) )
            break;

        if( !mpGraphics->GetGlyphBoundRect( nGlyphIndex, aRectangle ) )
            continue;

        if( !rSalLayout.IsSpacingGlyph( nGlyphIndex ) )
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
    sal_Bool bOldMap = mbMap;

    mnOutOffX   = 0L;
    mnOutOffY   = 0L;
    mpMetaFile  = NULL;
    EnableMapMode( sal_False );

    DrawMask( aPoint, aBmp, GetTextColor() );

    EnableMapMode( bOldMap );
    mnOutOffX   = nOldOffX;
    mnOutOffY   = nOldOffY;
    mpMetaFile  = pOldMetaFile;

    return true;
}

void OutputDevice::ImplDrawTextDirect( SalLayout& rSalLayout, sal_Bool bTextLines )
{
    if( mpFontEntry->mnOwnOrientation )
        if( ImplDrawRotateText( rSalLayout ) )
            return;

    long nOldX = rSalLayout.DrawBase().X();
    if( ImplHasMirroredGraphics() )
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

    rSalLayout.DrawText( *mpGraphics );

    rSalLayout.DrawBase().X() = nOldX;

    if( bTextLines )
        ImplDrawTextLines( rSalLayout,
            maFont.GetStrikeout(), maFont.GetUnderline(), maFont.GetOverline(),
            maFont.IsWordLineMode(), ImplIsUnderlineAbove( maFont ) );

    // emphasis marks
    if( maFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
        ImplDrawEmphasisMarks( rSalLayout );
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
        OUString aText( rStr );
        uno::Reference < i18n::XBreakIterator > xBI;
        // get service provider
        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        uno::Reference< linguistic2::XLinguServiceManager2> xLinguMgr = linguistic2::LinguServiceManager::create(xContext);
        uno::Reference< linguistic2::XHyphenator > xHyph = xLinguMgr->getHyphenator();

        i18n::LineBreakHyphenationOptions aHyphOptions( xHyph, uno::Sequence <beans::PropertyValue>(), 1 );
        i18n::LineBreakUserOptions aUserOptions;

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
                    xub_StrLen nSoftBreak = _rLayout.GetTextBreak( rStr, nWidth, nPos, nBreakPos - nPos );
                    DBG_ASSERT( nSoftBreak < nBreakPos, "Break?!" );
                    i18n::LineBreakResults aLBR = xBI->getLineBreak( aText, nSoftBreak, rDefLocale, nPos, aHyphOptions, aUserOptions );
                    nBreakPos = (xub_StrLen)aLBR.breakIndex;
                    if ( nBreakPos <= nPos )
                        nBreakPos = nSoftBreak;
                    if ( (nStyle & TEXT_DRAW_WORDBREAK_HYPHENATION) == TEXT_DRAW_WORDBREAK_HYPHENATION )
                    {
                        // Whether hyphen or not: Put the word after the hyphen through
                        // word boundary.
                        //
                        // nMaxBreakPos the last char that fits into the line
                        // nBreakPos is the word's start
                        //
                        // We run into a problem if the doc is so narrow, that a word
                        // is broken into more than two lines ...
                        if ( xHyph.is() )
                        {
                            sal_Unicode cAlternateReplChar = 0;
                            i18n::Boundary aBoundary = xBI->getWordBoundary( aText, nBreakPos, rDefLocale, ::com::sun::star::i18n::WordType::DICTIONARY_WORD, sal_True );
                            sal_Int32 nWordStart = nPos;
                            sal_Int32 nWordEnd = (sal_Int32) aBoundary.endPos;
                            DBG_ASSERT( nWordEnd > nWordStart, "ImpBreakLine: Start >= End?" );

                            sal_Int32 nWordLen = nWordEnd - nWordStart;
                            if ( ( nWordEnd >= nSoftBreak ) && ( nWordLen > 3 ) )
                            {
                                // #104415# May happen, because getLineBreak may differ from getWordBoudary with DICTIONARY_WORD
                                // DBG_ASSERT( nWordEnd >= nMaxBreakPos, "Hyph: Break?" );
                                String aWord( aText, nWordStart, nWordLen );
                                sal_uInt16 nMinTrail = static_cast<sal_uInt16>(nWordEnd-nSoftBreak+1);  //+1: Before the "broken off" char
                                uno::Reference< linguistic2::XHyphenatedWord > xHyphWord;
                                if (xHyph.is())
                                    xHyphWord = xHyph->hyphenate( aWord, rDefLocale, aWord.Len() - nMinTrail, uno::Sequence< beans::PropertyValue >() );
                                if (xHyphWord.is())
                                {
                                    sal_Bool bAlternate = xHyphWord->isAlternativeSpelling();
                                    sal_uInt16 _nWordLen = 1 + xHyphWord->getHyphenPos();

                                    if ( ( _nWordLen >= 2 ) && ( (nWordStart+_nWordLen) >= ( 2 ) ) )
                                    {
                                        if ( !bAlternate )
                                        {
                                            nBreakPos = nWordStart + _nWordLen;
                                        }
                                        else
                                        {
                                            String aAlt( xHyphWord->getHyphenatedWord() );

                                            // We can have two cases:
                                            // 1) "packen" turns into "pak-ken"
                                            // 2) "Schiffahrt" turns into "Schiff-fahrt"
                                            //
                                            // In case 1 we need to replace a char
                                            // In case 2 we add a char
                                            //
                                            // Correct recognition is made harder by words such as
                                            // "Schiffahrtsbrennesseln", as the Hyphenator splits all
                                            // positions of the word and comes up with "Schifffahrtsbrennnesseln"
                                            // Thus, we cannot infer the aWord from the AlternativWord's
                                            // index.
                                            // TODO: The whole junk will be made easier by a function in
                                            // the Hyphenator, as soon as AMA adds it.
                                            sal_uInt16 nAltStart = _nWordLen - 1;
                                            sal_uInt16 nTxtStart = nAltStart - (aAlt.Len() - aWord.Len());
                                            sal_uInt16 nTxtEnd = nTxtStart;
                                            sal_uInt16 nAltEnd = nAltStart;

                                            // The area between nStart and nEnd is the difference
                                            // between AlternativString and OriginalString
                                            while( nTxtEnd < aWord.Len() && nAltEnd < aAlt.Len() &&
                                                   aWord.GetChar(nTxtEnd) != aAlt.GetChar(nAltEnd) )
                                            {
                                                ++nTxtEnd;
                                                ++nAltEnd;
                                            }

                                            // If a char was added, we notice it now:
                                            if( nAltEnd > nTxtEnd && nAltStart == nAltEnd &&
                                                aWord.GetChar( nTxtEnd ) == aAlt.GetChar(nAltEnd) )
                                            {
                                                ++nAltEnd;
                                                ++nTxtStart;
                                                ++nTxtEnd;
                                            }

                                            DBG_ASSERT( ( nAltEnd - nAltStart ) == 1, "Alternate: Wrong assumption!" );

                                            if ( nTxtEnd > nTxtStart )
                                                cAlternateReplChar = aAlt.GetChar( nAltStart );

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
                        nSpacePos = rStr.lastIndexOf( sal_Unicode(' '), nSpacePos );
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

            if ( ( rStr[ nPos ] == '\r' ) || ( rStr[ nPos ] == '\n' ) )
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
        String aLine( rStr, pLine->GetIndex(), pLine->GetLen() );
        DBG_ASSERT( aLine.Search( '\r' ) == STRING_NOTFOUND, "ImplGetTextLines - Found CR!" );
        DBG_ASSERT( aLine.Search( '\n' ) == STRING_NOTFOUND, "ImplGetTextLines - Found LF!" );
    }
#endif

    return nMaxLineWidth;
}

void OutputDevice::SetAntialiasing( sal_uInt16 nMode )
{
    if ( mnAntialiasing != nMode )
    {
        mnAntialiasing = nMode;
        mbInitFont = sal_True;

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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DBG_CHKOBJ( &rNewFont, Font, NULL );

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

        sal_Bool bTransFill = aFont.IsTransparent();
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
                bTransFill = sal_True;
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
            mbInitTextColor = sal_True;
            if( mpMetaFile )
                mpMetaFile->AddAction( new MetaTextColorAction( aFont.GetColor() ) );
        }
        maFont      = aFont;
        mbNewFont   = sal_True;

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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
        mbInitTextColor = sal_True;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextColor( COL_BLACK );
}

void OutputDevice::SetTextFillColor()
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextFillColorAction( Color(), sal_False ) );

    if ( maFont.GetColor() != Color( COL_TRANSPARENT ) )
        maFont.SetFillColor( Color( COL_TRANSPARENT ) );
    if ( !maFont.IsTransparent() )
        maFont.SetTransparent( sal_True );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextFillColor();
}

void OutputDevice::SetTextFillColor( const Color& rColor )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Color aColor( rColor );
    sal_Bool bTransFill = ImplIsColorTransparent( aColor ) ? sal_True : sal_False;

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
                bTransFill = sal_True;
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
        mpMetaFile->AddAction( new MetaTextFillColorAction( aColor, sal_True ) );

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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextLineColorAction( Color(), sal_False ) );

    maTextLineColor = Color( COL_TRANSPARENT );

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor();
}

void OutputDevice::SetTextLineColor( const Color& rColor )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
        mpMetaFile->AddAction( new MetaTextLineColorAction( aColor, sal_True ) );

    maTextLineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextLineColor( COL_BLACK );
}

void OutputDevice::SetOverlineColor()
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaOverlineColorAction( Color(), sal_False ) );

    maOverlineColor = Color( COL_TRANSPARENT );

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor();
}

void OutputDevice::SetOverlineColor( const Color& rColor )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
        mpMetaFile->AddAction( new MetaOverlineColorAction( aColor, sal_True ) );

    maOverlineColor = aColor;

    if( mpAlphaVDev )
        mpAlphaVDev->SetOverlineColor( COL_BLACK );
}

void OutputDevice::SetTextAlign( TextAlign eAlign )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaTextAlignAction( eAlign ) );

    if ( maFont.GetAlign() != eAlign )
    {
        maFont.SetAlign( eAlign );
        mbNewFont = sal_True;
    }

    if( mpAlphaVDev )
        mpAlphaVDev->SetTextAlign( eAlign );
}

void OutputDevice::DrawTextLine( const Point& rPos, long nWidth,
                                 FontStrikeout eStrikeout,
                                 FontUnderline eUnderline,
                                 FontUnderline eOverline,
                                 sal_Bool bUnderlineAbove )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

void OutputDevice::DrawWaveLine( const Point& rStartPos, const Point& rEndPos,
                                 sal_uInt16 nStyle )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
    if ( nStyle == WAVE_NORMAL )
    {
        nWaveHeight = 3;
        nStartY++;
        nEndY++;
    }
    else if( nStyle == WAVE_SMALL )
    {
        nWaveHeight = 2;
        nStartY++;
        nEndY++;
    }
    else // WAVE_FLAT
        nWaveHeight = 1;

     // #109280# make sure the waveline does not exceed the descent to avoid paint problems
     ImplFontEntry* pFontEntry = mpFontEntry;
     if( nWaveHeight > pFontEntry->maMetric.mnWUnderlineSize )
         nWaveHeight = pFontEntry->maMetric.mnWUnderlineSize;

     ImplDrawWaveLine( nStartX, nStartY, 0, 0,
                      nEndX-nStartX, nWaveHeight, 1,
                      nOrientation, GetLineColor() );
    if( mpAlphaVDev )
        mpAlphaVDev->DrawWaveLine( rStartPos, rEndPos, nStyle );
}

void OutputDevice::DrawText( const Point& rStartPt, const OUString& rStr,
                             xub_StrLen nIndex, xub_StrLen nLen,
                             MetricVector* pVector, OUString* pDisplayText
                             )
{
    if( mpOutDevData && mpOutDevData->mpRecordLayout )
    {
        pVector = &mpOutDevData->mpRecordLayout->m_aUnicodeBoundRects;
        pDisplayText = &mpOutDevData->mpRecordLayout->m_aDisplayText;
    }

    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen, rStartPt, 0, NULL, true );
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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    long nWidth = GetTextArray( rStr, NULL, nIndex, nLen );

    return nWidth;
}

long OutputDevice::GetTextHeight() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
                                  xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen, rStartPt, 0, pDXAry, true );
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
    // MEM: default nLen = STRING_LENGTH
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( nIndex >= rStr.getLength() )
        return 0;

    if( nLen < 0 || nIndex+nLen >= rStr.getLength() )
        nLen = rStr.getLength() - nIndex;

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
    sal_Bool bCellBreaking ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
                                    xub_StrLen nIndex, xub_StrLen nLen )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( mpMetaFile )
        mpMetaFile->AddAction( new MetaStretchTextAction( rStartPt, nWidth, rStr, nIndex, nLen ) );

    if ( !IsDeviceOutputNecessary() )
        return;

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen, rStartPt, nWidth, NULL, true );
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
    ImplLayoutArgs aLayoutArgs( rStr.getStr(), rStr.getLength(), nMinIndex, nEndIndex, nLayoutFlags, maFont.GetLanguage() );

    int nOrientation = mpFontEntry ? mpFontEntry->mnOrientation : 0;
    aLayoutArgs.SetOrientation( nOrientation );

    aLayoutArgs.SetLayoutWidth( nPixelWidth );
    aLayoutArgs.SetDXArray( pDXArray );

    return aLayoutArgs;
}

SalLayout* OutputDevice::ImplLayout( const OUString& rOrigStr, sal_Int32 nMinIndex, sal_Int32 nLen,
                                     const Point& rLogicalPos, long nLogicalWidth, const sal_Int32* pDXArray,
                                     bool bFilter ) const
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

    // filter out special markers
    if( bFilter )
    {
        sal_Int32 nCutStart, nCutStop, nOrgLen = nLen;
        bool bFiltered = mpGraphics->filterText( rOrigStr, aStr, nMinIndex, nLen, nCutStart, nCutStop );
        if( !nLen )
            return NULL;

        if( bFiltered && nCutStop != nCutStart && pDXArray )
        {
            if( !nLen )
                pDXArray = NULL;
            else
            {
                sal_Int32* pAry = (sal_Int32*)alloca(sizeof(sal_Int32)*nLen);
                if( nCutStart > nMinIndex )
                    memcpy( pAry, pDXArray, sizeof(sal_Int32)*(nCutStart-nMinIndex) );
                // note: nCutStart will never be smaller than nMinIndex
                memcpy( pAry+nCutStart-nMinIndex,
                        pDXArray + nOrgLen - (nCutStop-nMinIndex),
                        sizeof(sal_Int32)*(nLen - (nCutStart-nMinIndex)) );
                pDXArray = pAry;
            }
        }
    }

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

#ifdef MACOSX
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
    if( aLayoutArgs.NeedFallback() )
        if( mpFontEntry && (mpFontEntry->maFontSelData.mnHeight >= 3) )
            pSalLayout = ImplGlyphFallbackLayout( pSalLayout, aLayoutArgs );

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

    // when device specific font substitution may have been performed for
    // the originally selected font then make sure that a fallback to that
    // font is performed first
    int nDevSpecificFallback = 0;
    if( mpOutDevData && !mpOutDevData->maDevFontSubst.Empty() )
        nDevSpecificFallback = 1;

    // try if fallback fonts support the missing unicodes
    for( int nFallbackLevel = 1; nFallbackLevel < MAX_FALLBACK; ++nFallbackLevel )
    {
        // find a font family suited for glyph fallback
#ifndef FONTFALLBACK_HOOKS_DISABLED
        // GetGlyphFallbackFont() needs a valid aFontSelData.mpFontEntry
        // if the system-specific glyph fallback is active
        aFontSelData.mpFontEntry = mpFontEntry; // reset the fontentry to base-level
#endif
        ImplFontEntry* pFallbackFont = mpFontCache->GetGlyphFallbackFont( mpFontList,
            aFontSelData, nFallbackLevel-nDevSpecificFallback, aMissingCodes );
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

sal_Bool OutputDevice::GetTextIsRTL( const OUString& rString, sal_Int32 nIndex, sal_Int32 nLen ) const
{
    OUString aStr( rString );
    ImplLayoutArgs aArgs = ImplPrepareLayoutArgs( aStr, nIndex, nLen, 0, NULL );
    bool bRTL = false;
    int nCharPos = -1;
    aArgs.GetNextPos( &nCharPos, &bRTL );
    return (nCharPos != nIndex) ? sal_True : sal_False;
}

xub_StrLen OutputDevice::GetTextBreak( const OUString& rStr, long nTextWidth,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       long nCharExtra, sal_Bool /*TODO: bCellBreaking*/ ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    xub_StrLen nRetVal = STRING_LEN;
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
        nRetVal = sal::static_int_cast<xub_StrLen>(pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor ));

        pSalLayout->Release();
    }

    return nRetVal;
}

xub_StrLen OutputDevice::GetTextBreak( const OUString& rStr, long nTextWidth,
                                       sal_Unicode nHyphenatorChar, sal_Int32& rHyphenatorPos,
                                       sal_Int32 nIndex, sal_Int32 nLen,
                                       long nCharExtra ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    rHyphenatorPos = STRING_LEN;

    SalLayout* pSalLayout = ImplLayout( rStr, nIndex, nLen );
    if( !pSalLayout )
        return STRING_LEN;

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
    xub_StrLen nRetVal = sal::static_int_cast<xub_StrLen>(pSalLayout->GetTextBreak( nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor ));

    // calculate hyphenated break position
    OUString aHyphenatorStr(nHyphenatorChar);
    sal_Int32 nTempLen = 1;
    SalLayout* pHyphenatorLayout = ImplLayout( aHyphenatorStr, 0, nTempLen );
    if( pHyphenatorLayout )
    {
        // calculate subpixel width of hyphenation character
        long nHyphenatorPixelWidth = pHyphenatorLayout->GetTextWidth() * nSubPixelFactor;
        pHyphenatorLayout->Release();

        // calculate hyphenated break position
        nTextPixelWidth -= nHyphenatorPixelWidth;
        if( nExtraPixelWidth > 0 )
            nTextPixelWidth -= nExtraPixelWidth;

        // why does this return "int" and use STRING_LEN for errors???
        xub_StrLen nTmp = sal::static_int_cast<xub_StrLen>(
            pSalLayout->GetTextBreak(nTextPixelWidth, nExtraPixelWidth, nSubPixelFactor));

        nTmp = std::min(nTmp, nRetVal);

        // TODO: remove nTmp when GetTextBreak sal_Int32
        rHyphenatorPos = (nTmp == STRING_LEN) ? -1 : nTmp;
    }

    pSalLayout->Release();
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
        sal_Bool  bHighContrastBlack = sal_False;
        sal_Bool  bHighContrastWhite = sal_False;
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
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    String aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );

    const bool bDrawMnemonics = !(rTargetDevice.GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_NOMNEMONICS) && !pVector;

    // We treat multiline text differently
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {

        XubString               aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        xub_StrLen              i;
        xub_StrLen              nLines;
        xub_StrLen              nFormatLines;

        if ( nTextHeight )
        {
            long nMaxTextWidth = ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle, _rLayout );
            nLines = (xub_StrLen)(nHeight/nTextHeight);
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
                    aLastLine = convertLineEnd(aStr.Copy(pLineInfo->GetIndex()), LINEEND_LF);
                    // Replace all LineFeeds with Spaces
                    xub_StrLen nLastLineLen = aLastLine.Len();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLine.GetChar( i ) == '\n' )
                            aLastLine.SetChar( i, ' ' );
                    }
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
                xub_StrLen nIndex   = pLineInfo->GetIndex();
                xub_StrLen nLineLen = pLineInfo->GetLen();
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
            if ( aLastLine.Len() )
                _rLayout.DrawText( aPos, aLastLine, 0, STRING_LEN, pVector, pDisplayText );

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
                nTextWidth = _rLayout.GetTextWidth( aStr, 0, aStr.Len() );
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
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            sal_Int32* pCaretXArray = (sal_Int32*) alloca( 2 * sizeof(sal_Int32) * aStr.Len() );
            /*sal_Bool bRet =*/ _rLayout.GetCaretPositions( aStr, pCaretXArray, 0, aStr.Len() );
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
            _rLayout.DrawText( aPos, aStr, 0, STRING_LEN, pVector, pDisplayText );
            if ( bDrawMnemonics )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
                    rTargetDevice.ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
            }
            rTargetDevice.Pop();
        }
        else
        {
            _rLayout.DrawText( aPos, aStr, 0, STRING_LEN, pVector, pDisplayText );
            if ( bDrawMnemonics )
            {
                if ( nMnemonicPos != STRING_NOTFOUND )
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
                                       const String&    rOrigStr,
                                       sal_uInt16           nStyle,
                                       GDIMetaFile&     rMtf )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( !rOrigStr.Len() || rRect.IsEmpty() )
        return;

    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();

    // temporarily swap in passed mtf for action generation, and
    // disable output generation.
    const sal_Bool bOutputEnabled( IsOutputEnabled() );
    GDIMetaFile* pMtf = mpMetaFile;

    mpMetaFile = &rMtf;
    EnableOutput( sal_False );

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

    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
                                     const XubString& rStr, sal_uInt16 nStyle,
                                     TextRectInfo* pInfo,
                                     const ::vcl::ITextLayout* _pTextLayout ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    Rectangle           aRect = rRect;
    xub_StrLen          nLines;
    long                nWidth = rRect.GetWidth();
    long                nMaxWidth;
    long                nTextHeight = GetTextHeight();

    String aStr = rStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = GetNonMnemonicString( aStr );

    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        xub_StrLen              nFormatLines;
        xub_StrLen              i;

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
                    pInfo->mbEllipsis = sal_True;
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
        nMaxWidth   = _pTextLayout ? _pTextLayout->GetTextWidth( aStr, 0, aStr.Len() ) : GetTextWidth( aStr );

        if ( pInfo )
        {
            pInfo->mnLineCount  = 1;
            pInfo->mnMaxWidth   = nMaxWidth;
        }

        if ( (nMaxWidth > nWidth) && (nStyle & TEXT_DRAW_ELLIPSIS) )
        {
            if ( pInfo )
                pInfo->mbEllipsis = sal_True;
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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    DefaultTextLayout aTextLayout( *const_cast< OutputDevice* >( this ) );
    return ImplGetEllipsisString( *this, rOrigStr, nMaxWidth, nStyle, aTextLayout );
}

OUString OutputDevice::ImplGetEllipsisString( const OutputDevice& rTargetDevice, const OUString& rOrigStr, long nMaxWidth,
                                               sal_uInt16 nStyle, const ::vcl::ITextLayout& _rLayout )
{
    OUString aStr = rOrigStr;
    sal_Int32 nIndex = _rLayout.GetTextBreak( aStr, nMaxWidth, 0, aStr.getLength() );

    if ( nIndex != STRING_LEN )
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

void OutputDevice::DrawCtrlText( const Point& rPos, const XubString& rStr,
                                 xub_StrLen nIndex, xub_StrLen nLen,
                                 sal_uInt16 nStyle, MetricVector* pVector, OUString* pDisplayText )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( !IsDeviceOutputNecessary() || (nIndex >= rStr.Len()) )
        return;

    // better get graphics here because ImplDrawMnemonicLine() will not
    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return;
    if( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return;

    if( nIndex >= rStr.Len() )
        return;
    if( (sal_uLong)nIndex+nLen >= rStr.Len() )
        nLen = rStr.Len() - nIndex;

    XubString   aStr = rStr;
    xub_StrLen  nMnemonicPos = STRING_NOTFOUND;

    long        nMnemonicX = 0;
    long        nMnemonicY = 0;
    long        nMnemonicWidth = 0;
    if ( (nStyle & TEXT_DRAW_MNEMONIC) && nLen > 1 )
    {
        aStr = GetNonMnemonicString( aStr, nMnemonicPos );
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            if( nMnemonicPos < nIndex )
                --nIndex;
            else if( nLen < STRING_LEN )
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
        sal_Bool  bHighContrastBlack = sal_False;
        sal_Bool  bHighContrastWhite = sal_False;
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
            if ( nMnemonicPos != STRING_NOTFOUND )
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
            if ( nMnemonicPos != STRING_NOTFOUND )
                ImplDrawMnemonicLine( nMnemonicX, nMnemonicY, nMnemonicWidth );
        }
    }

    if( mpAlphaVDev )
        mpAlphaVDev->DrawCtrlText( rPos, rStr, nIndex, nLen, nStyle, pVector, pDisplayText );
}

long OutputDevice::GetCtrlTextWidth( const String& rStr,
                                     xub_StrLen nIndex, xub_StrLen nLen,
                                     sal_uInt16 nStyle ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if ( nStyle & TEXT_DRAW_MNEMONIC )
    {
        xub_StrLen  nMnemonicPos;
        XubString   aStr = GetNonMnemonicString( rStr, nMnemonicPos );
        if ( nMnemonicPos != STRING_NOTFOUND )
        {
            if ( nMnemonicPos < nIndex )
                nIndex--;
            else if ( (nLen < STRING_LEN) &&
                      (nMnemonicPos >= nIndex) && (nMnemonicPos < (sal_uLong)(nIndex+nLen)) )
                nLen--;
        }
        return GetTextWidth( aStr, nIndex, nLen );
    }
    else
        return GetTextWidth( rStr, nIndex, nLen );
}

String OutputDevice::GetNonMnemonicString( const String& rStr, xub_StrLen& rMnemonicPos )
{
    String   aStr    = rStr;
    xub_StrLen  nLen    = aStr.Len();
    xub_StrLen  i       = 0;

    rMnemonicPos = STRING_NOTFOUND;
    while ( i < nLen )
    {
        if ( aStr.GetChar( i ) == '~' )
        {
            if ( aStr.GetChar( i+1 ) != '~' )
            {
                if ( rMnemonicPos == STRING_NOTFOUND )
                    rMnemonicPos = i;
                aStr.Erase( i, 1 );
                nLen--;
            }
            else
            {
                aStr.Erase( i, 1 );
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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    if( !mpGetDevFontList )
        mpGetDevFontList = mpFontList->GetDevFontList();
    return mpGetDevFontList->Count();
}

FontInfo OutputDevice::GetDevFont( int nDevFontIndex ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

sal_Bool OutputDevice::AddTempDevFont( const String& rFileURL, const String& rFontName )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    ImplInitFontList();

    if( !mpGraphics && !ImplGetGraphics() )
        return sal_False;

    bool bRC = mpGraphics->AddTempDevFont( mpFontList, rFileURL, rFontName );
    if( !bRC )
        return sal_False;

    if( mpAlphaVDev )
        mpAlphaVDev->AddTempDevFont( rFileURL, rFontName );

    mpFontCache->Invalidate();
    return sal_True;
}

int OutputDevice::GetDevFontSizeCount( const Font& rFont ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    delete mpGetDevSizeList;

    ImplInitFontList();
    mpGetDevSizeList = mpFontList->GetDevSizeList( rFont.GetName() );
    return mpGetDevSizeList->Count();
}

Size OutputDevice::GetDevFontSize( const Font& rFont, int nSizeIndex ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

sal_Bool OutputDevice::IsFontAvailable( const String& rFontName ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    ImplDevFontListData* pFound = mpFontList->FindFontFamily( rFontName );
    return (pFound != NULL);
}

FontMetric OutputDevice::GetFontMetric() const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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
SystemTextLayoutData OutputDevice::GetSysTextLayoutData(const Point& rStartPt, const XubString& rStr, xub_StrLen nIndex, xub_StrLen nLen,
                                                        const sal_Int32* pDXAry) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

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

    SalLayout* pLayout = ImplLayout( rStr, nIndex, nLen, rStartPt, 0, pDXAry, true );

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
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );
    if( mbNewFont && !ImplNewFont() )
        return 0;

    ImplFontEntry*      pEntry = mpFontEntry;
    ImplFontMetricData* pMetric = &(pEntry->maMetric);
    return ImplDevicePixelToLogicWidth( pMetric->mnMinKashida );
}

xub_StrLen OutputDevice::ValidateKashidas ( const String& rTxt,
                                            xub_StrLen nIdx, xub_StrLen nLen,
                                            xub_StrLen nKashCount,
                                            const xub_StrLen* pKashidaPos,
                                            xub_StrLen* pKashidaPosDropped ) const
{
   // do layout
    SalLayout* pSalLayout = ImplLayout( rTxt, nIdx, nLen );
    if( !pSalLayout )
        return 0;
    xub_StrLen nDropped = 0;
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

sal_Bool OutputDevice::GetGlyphBoundRects( const Point& rOrigin, const String& rStr,
    int nIndex, int nLen, int nBase, MetricVector& rVector )
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    rVector.clear();

    if( nLen == STRING_LEN )
        nLen = rStr.Len() - nIndex;

    Rectangle aRect;
    for( int i = 0; i < nLen; i++ )
    {
        if( !GetTextBoundRect( aRect, rStr, sal::static_int_cast<xub_StrLen>(nBase), sal::static_int_cast<xub_StrLen>(nIndex+i), 1 ) )
            break;
        aRect.Move( rOrigin.X(), rOrigin.Y() );
        rVector.push_back( aRect );
    }

    return (nLen == (int)rVector.size());
}

sal_Bool OutputDevice::GetTextBoundRect( Rectangle& rRect,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex, xub_StrLen nLen,
    sal_uLong nLayoutWidth, const sal_Int32* pDXAry ) const
{
    DBG_CHKTHIS( OutputDevice, ImplDbgCheckOutputDevice );

    sal_Bool bRet = sal_False;
    rRect.SetEmpty();

    SalLayout* pSalLayout = NULL;
    const Point aPoint;
    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
        xub_StrLen nStart = std::min( nBase, nIndex );
        xub_StrLen nOfsLen = std::max( nBase, nIndex ) - nStart;
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
    aFont.SetShadow( sal_False );
    aFont.SetOutline( sal_False );
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
        return sal_False;
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
        return sal_True;
    }

    return sal_False;
}

sal_Bool OutputDevice::GetTextOutlines( ::basegfx::B2DPolyPolygonVector& rVector,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex, xub_StrLen nLen,
    sal_Bool bOptimize, sal_uLong nTWidth, const sal_Int32* pDXArray ) const
{
    // the fonts need to be initialized
    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        ImplInitFont();
    if( !mpFontEntry )
        return sal_False;

    sal_Bool bRet = sal_False;
    rVector.clear();
    if( nLen == STRING_LEN )
        nLen = rStr.Len() - nIndex;
    rVector.reserve( nLen );

    // we want to get the Rectangle in logical units, so to
    // avoid rounding errors we just size the font in logical units
    sal_Bool bOldMap = mbMap;
    if( bOldMap )
    {
        const_cast<OutputDevice&>(*this).mbMap = sal_False;
        const_cast<OutputDevice&>(*this).mbNewFont = sal_True;
    }

    SalLayout* pSalLayout = NULL;

    // calculate offset when nBase!=nIndex
    long nXOffset = 0;
    if( nBase != nIndex )
    {
        xub_StrLen nStart = std::min( nBase, nIndex );
        xub_StrLen nOfsLen = std::max( nBase, nIndex ) - nStart;
        pSalLayout = ImplLayout( rStr, nStart, nOfsLen, Point(0,0), nTWidth, pDXArray );
        if( pSalLayout )
        {
            nXOffset = pSalLayout->GetTextWidth();
            pSalLayout->Release();
            // TODO: fix offset calculation for Bidi case
            if( nBase > nIndex)
                nXOffset = -nXOffset;
        }
    }

    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nTWidth, pDXArray );
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
        const_cast<OutputDevice&>(*this).mbNewFont = sal_True;
    }

    if( bRet || (OUTDEV_PRINTER == meOutDevType) || !mpFontEntry )
        return bRet;

    // fall back to bitmap conversion
    // Here, we can savely assume that the mapping between characters and glyphs
    // is one-to-one. This is most probably valid for the old bitmap fonts.
    // fall back to bitmap method to get the bounding rectangle,
    // so we need a monochrome virtual device with matching font
    pSalLayout = ImplLayout( rStr, nIndex, nLen, Point(0,0), nTWidth, pDXArray );
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

    pSalLayout = aVDev.ImplLayout( rStr, nIndex, nLen, Point(0,0), nTWidth, pDXArray );
    if (pSalLayout == 0)
        return false;
    long nWidth = pSalLayout->GetTextWidth();
    long nHeight = ((OutputDevice*)&aVDev)->mpFontEntry->mnLineHeight + ((OutputDevice*)&aVDev)->mnEmphasisAscent
        + ((OutputDevice*)&aVDev)->mnEmphasisDescent;
    pSalLayout->Release();

    if( !nWidth || !nHeight )
        return sal_True;
    double fScaleX = static_cast< double >(nOrgWidth) / nWidth;
    double fScaleY = static_cast< double >(nOrgHeight) / nHeight;

    // calculate offset when nBase!=nIndex
    // TODO: fix offset calculation for Bidi case
    nXOffset = 0;
    if( nBase != nIndex )
    {
        xub_StrLen nStart  = ((nBase < nIndex) ? nBase : nIndex);
        xub_StrLen nLength = ((nBase > nIndex) ? nBase : nIndex) - nStart;
        pSalLayout = aVDev.ImplLayout( rStr, nStart, nLength, Point(0,0), nTWidth, pDXArray );
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
        pSalLayout = aVDev.ImplLayout( rStr, static_cast< xub_StrLen >(nCharPos), 1, Point(0,0), nTWidth, pDXArray );
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

sal_Bool OutputDevice::GetTextOutlines( PolyPolyVector& rResultVector,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex,
    xub_StrLen nLen, sal_Bool bOptimize, sal_uLong nTWidth, const sal_Int32* pDXArray ) const
{
    rResultVector.clear();

    // get the basegfx polypolygon vector
    ::basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         bOptimize, nTWidth, pDXArray ) )
        return sal_False;

    // convert to a tool polypolygon vector
    rResultVector.reserve( aB2DPolyPolyVector.size() );
    ::basegfx::B2DPolyPolygonVector::const_iterator aIt = aB2DPolyPolyVector.begin();
    for(; aIt != aB2DPolyPolyVector.end(); ++aIt )
        rResultVector.push_back(PolyPolygon(*aIt)); // #i76339#

    return sal_True;
}

sal_Bool OutputDevice::GetTextOutline( PolyPolygon& rPolyPoly,
    const String& rStr, xub_StrLen nBase, xub_StrLen nIndex, xub_StrLen nLen,
    sal_Bool bOptimize, sal_uLong nTWidth, const sal_Int32* pDXArray ) const
{
    rPolyPoly.Clear();

    // get the basegfx polypolygon vector
    ::basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    if( !GetTextOutlines( aB2DPolyPolyVector, rStr, nBase, nIndex, nLen,
                         bOptimize, nTWidth, pDXArray ) )
        return sal_False;

    // convert and merge into a tool polypolygon
    ::basegfx::B2DPolyPolygonVector::const_iterator aIt = aB2DPolyPolyVector.begin();
    for(; aIt != aB2DPolyPolyVector.end(); ++aIt )
        for( unsigned int i = 0; i < aIt->count(); ++i )
            rPolyPoly.Insert(Polygon((*aIt).getB2DPolygon( i ))); // #i76339#

    return sal_True;
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

sal_Bool OutputDevice::GetFontCharMap( FontCharMap& rFontCharMap ) const
{
    rFontCharMap.Reset();

    // we need a graphics
    if( !mpGraphics && !ImplGetGraphics() )
        return sal_False;

    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        ImplInitFont();
    if( !mpFontEntry )
        return sal_False;

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
        return sal_False;
    return sal_True;
}

xub_StrLen OutputDevice::HasGlyphs( const Font& rTempFont, const String& rStr,
    xub_StrLen nIndex, xub_StrLen nLen ) const
{
    if( nIndex >= rStr.Len() )
        return nIndex;
    xub_StrLen nEnd = nIndex + nLen;
    if( (sal_uLong)nIndex+nLen > rStr.Len() )
        nEnd = rStr.Len();

    DBG_ASSERT( nIndex < nEnd, "StartPos >= EndPos?" );
    DBG_ASSERT( nEnd <= rStr.Len(), "String too short" );

    // to get the map temporarily set font
    const Font aOrigFont = GetFont();
    const_cast<OutputDevice&>(*this).SetFont( rTempFont );
    FontCharMap aFontCharMap;
    sal_Bool bRet = GetFontCharMap( aFontCharMap );
    const_cast<OutputDevice&>(*this).SetFont( aOrigFont );

    // if fontmap is unknown assume it doesn't have the glyphs
    if( bRet == sal_False )
        return nIndex;

    const sal_Unicode* pStr = rStr.GetBuffer();
    for( pStr += nIndex; nIndex < nEnd; ++pStr, ++nIndex )
        if( ! aFontCharMap.HasChar( *pStr ) )
            return nIndex;

    return STRING_LEN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
