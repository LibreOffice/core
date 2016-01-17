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

#include <unotools/configmgr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <vcl/outdev.hxx>
#include <vcl/edit.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>

#include "sallayout.hxx"
#include "svdata.hxx"

#include "impfont.hxx"
#include "outdata.hxx"
#include "outfont.hxx"

#include "outdev.h"
#include "window.h"

#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "PhysicalFontFamily.hxx"

#include "svids.hrc"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include "graphite_features.hxx"
#endif

#include "../gdi/pdfwriter_impl.hxx"

#include <boost/functional/hash.hpp>
#include <cmath>
#include <cstring>
#include <memory>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::utl;

vcl::FontInfo OutputDevice::GetDevFont( int nDevFontIndex ) const
{
    vcl::FontInfo aFontInfo;

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

int OutputDevice::GetDevFontCount() const
{
    if( !mpGetDevFontList )
    {
        if (!mpFontCollection)
        {
            return 0;
        }

        mpGetDevFontList = mpFontCollection->GetDevFontList();

        if (!mpGetDevFontList->Count())
        {
            delete mpGetDevFontList;
            mpGetDevFontList = nullptr;

            return 0;
        }
    }
    return mpGetDevFontList->Count();
}

bool OutputDevice::IsFontAvailable( const OUString& rFontName ) const
{
    PhysicalFontFamily* pFound = mpFontCollection->FindFontFamily( rFontName );
    return (pFound != nullptr);
}

int OutputDevice::GetDevFontSizeCount( const vcl::Font& rFont ) const
{
    delete mpGetDevSizeList;

    ImplInitFontList();
    mpGetDevSizeList = mpFontCollection->GetDevSizeList( rFont.GetFamilyName() );
    return mpGetDevSizeList->Count();
}

Size OutputDevice::GetDevFontSize( const vcl::Font& rFont, int nSizeIndex ) const
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

namespace
{
    struct UpdateFontsGuard
    {
        UpdateFontsGuard()
        {
            OutputDevice::ImplClearAllFontData(true);
        }

        ~UpdateFontsGuard()
        {
            OutputDevice::ImplRefreshAllFontData(true);
        }
    };
}

bool OutputDevice::AddTempDevFont( const OUString& rFileURL, const OUString& rFontName )
{
    UpdateFontsGuard aUpdateFontsGuard;

    ImplInitFontList();

    if( !mpGraphics && !AcquireGraphics() )
        return false;

    bool bRC = mpGraphics->AddTempDevFont( mpFontCollection, rFileURL, rFontName );
    if( !bRC )
        return false;

    if( mpAlphaVDev )
        mpAlphaVDev->AddTempDevFont( rFileURL, rFontName );

    OutputDevice::ImplRefreshAllFontData(true);
    return true;
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
    aMetric.SetName( maFont.GetFamilyName() );
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
         aMetric.SetKerning( maFont.GetKerning() & ~FontKerning::FontSpecific );

    // set remaining metric fields
    aMetric.mpImplMetric->mnMiscFlags   = 0;
    if( pMetric->mbDevice )
            aMetric.mpImplMetric->mnMiscFlags |= ImplFontMetric::DEVICE_FLAG;
    if( pMetric->mbScalableFont )
            aMetric.mpImplMetric->mnMiscFlags |= ImplFontMetric::SCALABLE_FLAG;
    if ( pMetric->mbFullstopCentered)
            aMetric.mpImplMetric->mnMiscFlags |= ImplFontMetric::FULLSTOP_CENTERED_FLAG;
    aMetric.mpImplMetric->mnAscent      = ImplDevicePixelToLogicHeight( pMetric->mnAscent+mnEmphasisAscent );
    aMetric.mpImplMetric->mnDescent     = ImplDevicePixelToLogicHeight( pMetric->mnDescent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnIntLeading  = ImplDevicePixelToLogicHeight( pMetric->mnIntLeading+mnEmphasisAscent );
    aMetric.mpImplMetric->mnExtLeading  = ImplDevicePixelToLogicHeight( GetFontExtLeading() );
    aMetric.mpImplMetric->mnLineHeight  = ImplDevicePixelToLogicHeight( pMetric->mnAscent+pMetric->mnDescent+mnEmphasisAscent+mnEmphasisDescent );
    aMetric.mpImplMetric->mnSlant       = ImplDevicePixelToLogicHeight( pMetric->mnSlant );

    SAL_INFO("vcl.gdi.fontmetric", "OutputDevice::GetFontMetric:" << aMetric);

    return aMetric;
}

FontMetric OutputDevice::GetFontMetric( const vcl::Font& rFont ) const
{
    // select font, query metrics, select original font again
    vcl::Font aOldFont = GetFont();
    const_cast<OutputDevice*>(this)->SetFont( rFont );
    FontMetric aMetric( GetFontMetric() );
    const_cast<OutputDevice*>(this)->SetFont( aOldFont );
    return aMetric;
}

bool OutputDevice::GetFontCharMap( FontCharMapPtr& rFontCharMap ) const
{
    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return false;

    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        InitFont();
    if( !mpFontEntry )
        return false;

    FontCharMapPtr pFontCharMap ( mpGraphics->GetFontCharMap() );
    if (!pFontCharMap)
    {
        FontCharMapPtr pDefaultMap( new FontCharMap() );
        rFontCharMap = pDefaultMap;
    }
    else
        rFontCharMap = pFontCharMap;

    if( rFontCharMap->IsDefaultMap() )
        return false;
    return true;
}

bool OutputDevice::GetFontCapabilities( vcl::FontCapabilities& rFontCapabilities ) const
{
    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return false;

    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        InitFont();
    if( !mpFontEntry )
        return false;

    return mpGraphics->GetFontCapabilities(rFontCapabilities);
}

#if ENABLE_CAIRO_CANVAS

SystemFontData OutputDevice::GetSysFontData(int nFallbacklevel) const
{
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof(aSysFontData);

    if (!mpGraphics)
        (void) AcquireGraphics();

    if (mpGraphics)
        aSysFontData = mpGraphics->GetSysFontData(nFallbacklevel);

    return aSysFontData;
}

#endif // ENABLE_CAIRO_CANVAS

void OutputDevice::ImplGetEmphasisMark( tools::PolyPolygon& rPolyPoly, bool& rPolyLine,
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
                tools::Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
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
                tools::Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
                rPolyPoly.Insert( aPoly );
                // BorderWidth is 15%
                long nBorder = (nDotSize*150)/1000;
                if ( nBorder <= 1 )
                    rPolyLine = true;
                else
                {
                    tools::Polygon aPoly2( Point( nRad, nRad ),
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
                tools::Polygon aPoly( Point( nRad, nRad ), nRad, nRad );
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
                tools::Polygon aPoly( sizeof( aAccentPos ) / sizeof( long ) / 2,
                                      reinterpret_cast<const Point*>(aAccentPos),
                                      aAccentPolyFlags );
                double dScale = ((double)nDotSize)/1000.0;
                aPoly.Scale( dScale, dScale );
                tools::Polygon aTemp;
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

FontEmphasisMark OutputDevice::ImplGetEmphasisMarkStyle( const vcl::Font& rFont )
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

long OutputDevice::GetFontExtLeading() const
{
    ImplFontEntry*      pEntry = mpFontEntry;
    ImplFontMetricData* pMetric = &(pEntry->maMetric);

    return pMetric->mnExtLeading;
}

void OutputDevice::ImplClearFontData( const bool bNewFontLists )
{
    // the currently selected logical font is no longer needed
    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = nullptr;
    }

    mbInitFont = true;
    mbNewFont = true;

    if ( bNewFontLists )
    {
        if ( mpGetDevFontList )
        {
            delete mpGetDevFontList;
            mpGetDevFontList = nullptr;
        }
        if ( mpGetDevSizeList )
        {
            delete mpGetDevSizeList;
            mpGetDevSizeList = nullptr;
        }

        // release all physically selected fonts on this device
        if( AcquireGraphics() )
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
            if ( AcquireGraphics() )
            {
                if( mpFontCollection && mpFontCollection != pSVData->maGDIData.mpScreenFontList )
                    mpFontCollection->Clear();

                if( mpPDFWriter )
                {
                    if( mpFontCollection && mpFontCollection != pSVData->maGDIData.mpScreenFontList )
                        delete mpFontCollection;
                    if( mpFontCache && mpFontCache != pSVData->maGDIData.mpScreenFontCache )
                        delete mpFontCache;
                    mpFontCollection = nullptr;
                    mpFontCache = nullptr;
                }
            }
        }
    }

    // also update child windows if needed
    if ( GetOutDevType() == OUTDEV_WINDOW )
    {
        vcl::Window* pChild = static_cast<vcl::Window*>(this)->mpWindowImpl->mpFirstChild;
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
            if ( AcquireGraphics() )
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
        vcl::Window* pChild = static_cast<vcl::Window*>(this)->mpWindowImpl->mpFirstChild;
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

void OutputDevice::ImplClearAllFontData(bool bNewFontLists)
{
    ImplSVData* pSVData = ImplGetSVData();

    ImplUpdateFontDataForAllFrames( &OutputDevice::ImplClearFontData, bNewFontLists );

    // clear global font lists to have them updated
    pSVData->maGDIData.mpScreenFontCache->Invalidate();
    if ( bNewFontLists )
    {
        pSVData->maGDIData.mpScreenFontList->Clear();
        vcl::Window * pFrame = pSVData->maWinData.mpFirstFrame;
        if ( pFrame )
        {
            if ( pFrame->AcquireGraphics() )
            {
                // Stupid typecast here and somewhere ((OutputDevice*)&aVDev)->, because bug in .NET2002 compiler
                OutputDevice *pDevice = static_cast<OutputDevice*>(pFrame);
                pDevice->mpGraphics->ClearDevFontCache();
                pDevice->mpGraphics->GetDevFontList(pFrame->mpWindowImpl->mpFrameData->mpFontCollection);
            }
        }
    }
}

void OutputDevice::ImplRefreshAllFontData(bool bNewFontLists)
{
    ImplUpdateFontDataForAllFrames( &OutputDevice::ImplRefreshFontData, bNewFontLists );
}

void OutputDevice::ImplUpdateAllFontData(bool bNewFontLists)
{
    OutputDevice::ImplClearAllFontData(bNewFontLists);
    OutputDevice::ImplRefreshAllFontData(bNewFontLists);
}

void OutputDevice::ImplUpdateFontDataForAllFrames( const FontUpdateHandler_t pHdl, const bool bNewFontLists )
{
    ImplSVData* const pSVData = ImplGetSVData();

    // update all windows
    vcl::Window* pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        ( pFrame->*pHdl )( bNewFontLists );

        vcl::Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
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

        DataChangedEvent aDCEvt( DataChangedEventType::FONTSUBSTITUTION );
        Application::NotifyAllWindows( aDCEvt );
        pSVData->maGDIData.mbFontSubChanged = false;
    }
}

void OutputDevice::AddFontSubstitute( const OUString& rFontName,
                                      const OUString& rReplaceFontName,
                                      AddFontSubstituteFlags nFlags )
{
    ImplDirectFontSubstitution*& rpSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( !rpSubst )
        rpSubst = new ImplDirectFontSubstitution();
    rpSubst->AddFontSubstitute( rFontName, rReplaceFontName, nFlags );
    ImplGetSVData()->maGDIData.mbFontSubChanged = true;
}

void ImplDirectFontSubstitution::AddFontSubstitute( const OUString& rFontName,
    const OUString& rSubstFontName, AddFontSubstituteFlags nFlags )
{
    maFontSubstList.push_back( ImplFontSubstEntry( rFontName, rSubstFontName, nFlags ) );
}

ImplFontSubstEntry::ImplFontSubstEntry( const OUString& rFontName,
    const OUString& rSubstFontName, AddFontSubstituteFlags nSubstFlags )
:   mnFlags( nSubstFlags )
{
    maSearchName = GetEnglishSearchFontName( rFontName );
    maSearchReplaceName = GetEnglishSearchFontName( rSubstFontName );
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
    const OUString& rSearchName, AddFontSubstituteFlags nFlags ) const
{
    // TODO: get rid of O(N) searches
    FontSubstList::const_iterator it = maFontSubstList.begin();
    for(; it != maFontSubstList.end(); ++it )
    {
        const ImplFontSubstEntry& rEntry = *it;
        if( ((rEntry.mnFlags & nFlags) || nFlags == AddFontSubstituteFlags::NONE)
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
    // must be canonicalised
    assert( GetEnglishSearchFontName( rFontName ) == rFontName );

    OUString aSubstFontName;

    // apply user-configurable font replacement (eg, from the list in Tools->Options)
    const ImplDirectFontSubstitution* pSubst = ImplGetSVData()->maGDIData.mpDirectFontSubst;
    if( pSubst && pSubst->FindFontSubstitute( aSubstFontName, rFontName, AddFontSubstituteFlags::ALWAYS ) )
    {
        rFontName = aSubstFontName;
        return;
    }
}

//hidpi TODO: This routine has hard-coded font-sizes that break places such as DialControl
vcl::Font OutputDevice::GetDefaultFont( DefaultFontType nType, LanguageType eLang,
                                        GetDefaultFontFlags nFlags, const OutputDevice* pOutDev )
{
    if (!pOutDev && !utl::ConfigManager::IsAvoidConfig()) // default is NULL
        pOutDev = Application::GetDefaultDevice();

    OUString aSearch;
    if (!utl::ConfigManager::IsAvoidConfig())
    {
        LanguageTag aLanguageTag(
                ( eLang == LANGUAGE_NONE || eLang == LANGUAGE_SYSTEM || eLang == LANGUAGE_DONTKNOW ) ?
                Application::GetSettings().GetUILanguageTag() :
                LanguageTag( eLang ));

        utl::DefaultFontConfiguration& rDefaults = utl::DefaultFontConfiguration::get();
        OUString aDefault = rDefaults.getDefaultFont( aLanguageTag, nType );

        if( !aDefault.isEmpty() )
            aSearch = aDefault;
        else
            aSearch = rDefaults.getUserInterfaceFont( aLanguageTag ); // use the UI font as a fallback
    }
    else
        aSearch = "Liberation Serif";

    vcl::Font aFont;
    aFont.SetPitch( PITCH_VARIABLE );

    switch ( nType )
    {
        case DefaultFontType::SANS_UNICODE:
        case DefaultFontType::UI_SANS:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DefaultFontType::SANS:
        case DefaultFontType::LATIN_HEADING:
        case DefaultFontType::LATIN_SPREADSHEET:
        case DefaultFontType::LATIN_DISPLAY:
            aFont.SetFamily( FAMILY_SWISS );
            break;

        case DefaultFontType::SERIF:
        case DefaultFontType::LATIN_TEXT:
        case DefaultFontType::LATIN_PRESENTATION:
            aFont.SetFamily( FAMILY_ROMAN );
            break;

        case DefaultFontType::FIXED:
        case DefaultFontType::LATIN_FIXED:
        case DefaultFontType::UI_FIXED:
            aFont.SetPitch( PITCH_FIXED );
            aFont.SetFamily( FAMILY_MODERN );
            break;

        case DefaultFontType::SYMBOL:
            aFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
            break;

        case DefaultFontType::CJK_TEXT:
        case DefaultFontType::CJK_PRESENTATION:
        case DefaultFontType::CJK_SPREADSHEET:
        case DefaultFontType::CJK_HEADING:
        case DefaultFontType::CJK_DISPLAY:
            aFont.SetFamily( FAMILY_SYSTEM ); // don't care, but don't use font subst config later...
            break;

        case DefaultFontType::CTL_TEXT:
        case DefaultFontType::CTL_PRESENTATION:
        case DefaultFontType::CTL_SPREADSHEET:
        case DefaultFontType::CTL_HEADING:
        case DefaultFontType::CTL_DISPLAY:
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
                aSearchName = GetEnglishSearchFontName( GetNextFontToken( aSearch, nIndex ) );

                PhysicalFontFamily* pFontFamily = pOutDev->mpFontCollection->ImplFindBySearchName( aSearchName );
                if( pFontFamily )
                {
                    AddTokenFontName( aName, pFontFamily->GetFamilyName() );
                    if( nFlags & GetDefaultFontFlags::OnlyOne )
                        break;
                }
            }
            while ( nIndex != -1 );
            aFont.SetName( aName );
        }

        // No Name, than set all names
        if ( aFont.GetFamilyName().isEmpty() )
        {
            if ( nFlags & GetDefaultFontFlags::OnlyOne )
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
    const char* s = "DefaultFontType::SANS_UNKNOWN";
    switch ( nType )
    {
    case DefaultFontType::SANS_UNICODE:  s = "DefaultFontType::SANS_UNICODE"; break;
    case DefaultFontType::UI_SANS:   s = "DefaultFontType::UI_SANS"; break;

    case DefaultFontType::SANS:  s = "DefaultFontType::SANS"; break;
    case DefaultFontType::LATIN_HEADING: s = "DefaultFontType::LATIN_HEADING"; break;
    case DefaultFontType::LATIN_SPREADSHEET: s = "DefaultFontType::LATIN_SPREADSHEET"; break;
    case DefaultFontType::LATIN_DISPLAY: s = "DefaultFontType::LATIN_DISPLAY"; break;

    case DefaultFontType::SERIF: s = "DefaultFontType::SERIF"; break;
    case DefaultFontType::LATIN_TEXT:    s = "DefaultFontType::LATIN_TEXT"; break;
    case DefaultFontType::LATIN_PRESENTATION:    s = "DefaultFontType::LATIN_PRESENTATION"; break;

    case DefaultFontType::FIXED: s = "DefaultFontType::FIXED"; break;
    case DefaultFontType::LATIN_FIXED:   s = "DefaultFontType::LATIN_FIXED"; break;
    case DefaultFontType::UI_FIXED:  s = "DefaultFontType::UI_FIXED"; break;

    case DefaultFontType::SYMBOL:    s = "DefaultFontType::SYMBOL"; break;

    case DefaultFontType::CJK_TEXT:  s = "DefaultFontType::CJK_TEXT"; break;
    case DefaultFontType::CJK_PRESENTATION:  s = "DefaultFontType::CJK_PRESENTATION"; break;
    case DefaultFontType::CJK_SPREADSHEET:   s = "DefaultFontType::CJK_SPREADSHEET"; break;
    case DefaultFontType::CJK_HEADING:   s = "DefaultFontType::CJK_HEADING"; break;
    case DefaultFontType::CJK_DISPLAY:   s = "DefaultFontType::CJK_DISPLAY"; break;

    case DefaultFontType::CTL_TEXT:  s = "DefaultFontType::CTL_TEXT"; break;
    case DefaultFontType::CTL_PRESENTATION:  s = "DefaultFontType::CTL_PRESENTATION"; break;
    case DefaultFontType::CTL_SPREADSHEET:   s = "DefaultFontType::CTL_SPREADSHEET"; break;
    case DefaultFontType::CTL_HEADING:   s = "DefaultFontType::CTL_HEADING"; break;
    case DefaultFontType::CTL_DISPLAY:   s = "DefaultFontType::CTL_DISPLAY"; break;
    }
    fprintf( stderr, "   OutputDevice::GetDefaultFont() Type=\"%s\" lang=%d flags=%ld FontName=\"%s\"\n",
         s, eLang, nFlags,
         OUStringToOString( aFont.GetName(), RTL_TEXTENCODING_UTF8 ).getStr()
         );
#endif

    return aFont;
}

ImplFontEntry::ImplFontEntry( const FontSelectPattern& rFontSelData )
    : m_pFontCache(nullptr)
    , maFontSelData( rFontSelData )
    , maMetric( rFontSelData )
    , mpConversion( nullptr )
    , mnLineHeight( 0 )
    , mnRefCount( 1 )
    , mnSetFontFlags( 0 )
    , mnOwnOrientation( 0 )
    , mnOrientation( 0 )
    , mbInit( false )
    , mpUnicodeFallbackList( nullptr )
{
    maFontSelData.mpFontEntry = this;
}

ImplFontEntry::~ImplFontEntry()
{
    delete mpUnicodeFallbackList;
    m_pFontCache = nullptr;
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

FontSelectPatternAttributes::FontSelectPatternAttributes( const vcl::Font& rFont,
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

FontSelectPattern::FontSelectPattern( const vcl::Font& rFont,
    const OUString& rSearchName, const Size& rSize, float fExactHeight)
    : FontSelectPatternAttributes(rFont, rSearchName, rSize, fExactHeight)
    , mpFontData( nullptr )
    , mpFontEntry( nullptr )
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
    size_t nHash;
#if ENABLE_GRAPHITE
    // check for features and generate a unique hash if necessary
    if (maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
        != -1)
    {
        nHash = maTargetName.hashCode();
    }
    else
#endif
    {
        nHash = maSearchName.hashCode();
    }
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
:   mpFirstEntry( nullptr ),
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
    const vcl::Font& rFont, const Size& rSize, float fExactHeight )
{
    OUString aSearchName = rFont.GetFamilyName();

    // initialize internal font request object
    FontSelectPattern aFontSelData( rFont, aSearchName, rSize, fExactHeight );
    return GetFontEntry( pFontList, aFontSelData );
}

ImplFontEntry* ImplFontCache::GetFontEntry( PhysicalFontCollection* pFontList,
    FontSelectPattern& aFontSelData )
{
    // check if a directly matching logical font instance is already cached,
    // the most recently used font usually has a hit rate of >50%
    ImplFontEntry *pEntry = nullptr;
    PhysicalFontFamily* pFontFamily = nullptr;
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
        DBG_ASSERT( (pFontFamily != nullptr), "ImplFontCache::Get() No logical font found!" );
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

    PhysicalFontFace* pFontData = nullptr;

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
        Acquire(pEntry);
    }

    if (!pEntry && pFontData)// still no cache hit => create a new font instance
    {
        // create a new logical font instance from this physical font face
        pEntry = pFontData->CreateFontInstance( aFontSelData );
        pEntry->m_pFontCache = this;

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
            pEntry->mpConversion = ConvertChar::GetRecodeData( "Symbol", "AppleSymbol" );
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
        PhysicalFontFamily* pFallbackData = nullptr;

        //fdo#33898 If someone has EUDC installed then they really want that to
        //be used as the first-choice glyph fallback seeing as it's filled with
        //private area codes with don't make any sense in any other font so
        //prioritise it here if it's available. Ideally we would remove from
        //rMissingCodes all the glyphs which it is able to resolve as an
        //optimization, but that's tricky to achieve cross-platform without
        //sufficient heavy-weight code that's likely to undo the value of the
        //optimization
        if (nFallbackLevel == 1)
            pFallbackData = pFontCollection->FindFontFamily("EUDC");
        if (!pFallbackData)
            pFallbackData = pFontCollection->GetGlyphFallbackFont(rFontSelData, rMissingCodes, nFallbackLevel-1);
        // escape when there are no font candidates
        if( !pFallbackData  )
            return nullptr;
        // override the font name
        rFontSelData.SetFamilyName( pFallbackData->GetFamilyName() );
        // clear the cached normalized name
        rFontSelData.maSearchName.clear();
    }

    ImplFontEntry* pFallbackFont = GetFontEntry( pFontCollection, rFontSelData );
    return pFallbackFont;
}

void ImplFontCache::Acquire(ImplFontEntry* pEntry)
{
    assert(pEntry->m_pFontCache == this);

    if (0 == pEntry->mnRefCount++)
        --mnRef0Count;
}

void ImplFontCache::Release(ImplFontEntry* pEntry)
{
    static const int FONTCACHE_MAX = getenv("LO_TESTNAME") ? 1 : 50;

    assert(pEntry->mnRefCount > 0 && "ImplFontCache::Release() - font refcount underflow");
    if( --pEntry->mnRefCount > 0 )
        return;

    if (++mnRef0Count < FONTCACHE_MAX)
        return;

    assert(CountUnreferencedEntries() == mnRef0Count);

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
        assert(mnRef0Count>=0 && "ImplFontCache::Release() - refcount0 underflow");

        if( mpFirstEntry == pFontEntry )
            mpFirstEntry = nullptr;
    }

    assert(mnRef0Count==0 && "ImplFontCache::Release() - refcount0 mismatch");
}

int ImplFontCache::CountUnreferencedEntries() const
{
    size_t nCount = 0;
    // count unreferenced entries
    for (FontInstanceList::const_iterator it = maFontInstanceList.begin();
         it != maFontInstanceList.end(); ++it)
    {
        const ImplFontEntry* pFontEntry = it->second;
        if (pFontEntry->mnRefCount > 0)
            continue;
        ++nCount;
    }
    return nCount;
}

void ImplFontCache::Invalidate()
{
    assert(CountUnreferencedEntries() == mnRef0Count);

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
    mpFirstEntry = nullptr;
    maFontInstanceList.clear();

    assert(mnRef0Count==0 && "ImplFontCache::Invalidate() - mnRef0Count non-zero");
}

void OutputDevice::ImplInitFontList() const
{
    if( !mpFontCollection->Count() )
    {
        if( mpGraphics || AcquireGraphics() )
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

void OutputDevice::InitFont() const
{
    DBG_TESTSOLARMUTEX();

    if (!mpFontEntry)
        return;

    if ( mbInitFont )
    {
        // decide if antialiasing is appropriate
        bool bNonAntialiased(GetAntialiasing() & AntialiasingFlags::DisableText);
        if (!utl::ConfigManager::IsAvoidConfig())
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            bNonAntialiased |= bool(rStyleSettings.GetDisplayOptions() & DisplayOptions::AADisable);
            bNonAntialiased |= (int(rStyleSettings.GetAntialiasingMinPixelHeight()) > mpFontEntry->maFontSelData.mnHeight);
        }
        mpFontEntry->maFontSelData.mbNonAntialiased = bNonAntialiased;

        // select font in the device layers
        mpFontEntry->mnSetFontFlags = mpGraphics->SetFont( &(mpFontEntry->maFontSelData), 0 );
        mbInitFont = false;
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
    if ( !mpGraphics && !AcquireGraphics() )
    {
        SAL_WARN("vcl.gdi", "OutputDevice::ImplNewFont(): no Graphics, no Font");
        return false;
    }
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
    {
        SAL_WARN("vcl.gdi", "OutputDevice::ImplNewFont(): no ImplFontEntry, no Font");
        return false;
    }

    // mark when lower layers need to get involved
    mbNewFont = false;
    if( pFontEntry != pOldEntry )
        mbInitFont = true;

    // select font when it has not been initialized yet
    if ( !pFontEntry->mbInit )
    {
        InitFont();

        // get metric data from device layers
        if ( pGraphics )
        {
            pFontEntry->mbInit = true;

            pFontEntry->maMetric.mnOrientation  = sal::static_int_cast<short>(pFontEntry->maFontSelData.mnOrientation);
            pGraphics->GetFontMetric( &(pFontEntry->maMetric) );

            pFontEntry->maMetric.ImplInitTextLineSize( this );
            pFontEntry->maMetric.ImplInitAboveTextLineSize();

            pFontEntry->mnLineHeight = pFontEntry->maMetric.mnAscent + pFontEntry->maMetric.mnDescent;

            SetFontOrientation( pFontEntry );
        }
    }

    // enable kerning array if requested
    if ( maFont.GetKerning() & FontKerning::FontSpecific )
    {
        // TODO: test if physical font supports kerning and disable if not
        if( pFontEntry->maMetric.mbKernableFont )
            mbKerning = true;
    }
    else
        mbKerning = false;
    if ( maFont.GetKerning() & FontKerning::Asian )
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
        {
            Point aOriginPt(0, 0);
            aOriginPt.RotateAround( mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
        }
    }
    else // eAlign == ALIGN_BOTTOM
    {
        mnTextOffX = 0;
        mnTextOffY = -pFontEntry->maMetric.mnDescent + mnEmphasisDescent;
        if ( pFontEntry->mnOrientation )
        {
            Point aOriginPt(0, 0);
            aOriginPt.RotateAround( mnTextOffX, mnTextOffY, pFontEntry->mnOrientation );
        }
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
            const_cast<vcl::Font&>(maFont).SetSize( Size( nNewWidth, aSize.Height() ) );
            mbMap = false;
            mbNewFont = true;
            ImplNewFont();  // recurse once using stretched width
            mbMap = true;
            const_cast<vcl::Font&>(maFont).SetSize( aOrigSize );
        }
    }

    return true;
}

void OutputDevice::SetFontOrientation( ImplFontEntry* const pFontEntry ) const
{
    if( pFontEntry->maFontSelData.mnOrientation && !pFontEntry->maMetric.mnOrientation )
    {
        pFontEntry->mnOwnOrientation = sal::static_int_cast<short>(pFontEntry->maFontSelData.mnOrientation);
        pFontEntry->mnOrientation = pFontEntry->mnOwnOrientation;
    }
    else
    {
        pFontEntry->mnOrientation = pFontEntry->maMetric.mnOrientation;
    }
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
    , meFamilyType(FAMILY_DONTKNOW)
    , mbScalableFont(false)
    , mbTrueTypeFont(false)
    , mbFullstopCentered(false)
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
    long nMin2LineDY = 1 + pDev->GetDPIY()/150;
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

    // #109280# the following line assures that wavelines are never placed below the descent, however
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

    const vcl::Font& rFont ( pDev->GetFont() );
    bool bCentered = true;
    if (MsLangId::isCJK(rFont.GetLanguage()))
    {
        const OUString sFullstop( sal_Unicode( 0x3001 ) ); // Fullwidth fullstop
        Rectangle aRect;
        pDev->GetTextBoundRect( aRect, sFullstop );
        const sal_uInt16 nH = rFont.GetSize().Height();
        const sal_uInt16 nB = aRect.Left();
        // Use 18.75% as a threshold to define a centered fullwidth fullstop.
        // In general, nB/nH < 5% for most Japanese fonts.
        bCentered = nB > (((nH >> 1)+nH)>>3);
    }
    mbFullstopCentered = bCentered ;
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

void OutputDevice::ImplDrawEmphasisMark( long nBaseX, long nX, long nY,
                                         const tools::PolyPolygon& rPolyPoly, bool bPolyLine,
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
            tools::Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            DrawPolyLine( aPoly );
        }
        else
        {
            tools::PolyPolygon aPolyPoly = rPolyPoly;
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
    mpMetaFile = nullptr;
    EnableMapMode( false );

    FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
    tools::PolyPolygon  aPolyPoly;
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

        if( !SalLayout::IsSpacingGlyph( aGlyphId ) )
        {
            Point aAdjPoint = aOffset;
            aAdjPoint.X() += aRectangle.Left() + (aRectangle.GetWidth() - nEmphasisWidth) / 2;
            if ( mpFontEntry->mnOrientation )
            {
                Point aOriginPt(0, 0);
                aOriginPt.RotateAround( aAdjPoint.X(), aAdjPoint.Y(), mpFontEntry->mnOrientation );
            }
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

SalLayout* OutputDevice::getFallbackFont(ImplFontEntry &rFallbackFont,
    FontSelectPattern &rFontSelData, int nFallbackLevel,
    ImplLayoutArgs& rLayoutArgs) const
{
    // we need a graphics
    if (!mpGraphics && !AcquireGraphics())
        return nullptr;

    assert(mpGraphics != nullptr);
    rFallbackFont.mnSetFontFlags = mpGraphics->SetFont( &rFontSelData, nFallbackLevel );

    rLayoutArgs.ResetPos();
    SalLayout* pFallback = mpGraphics->GetTextLayout( rLayoutArgs, nFallbackLevel );

    if (!pFallback)
        return nullptr;

    if (!pFallback->LayoutText(rLayoutArgs))
    {
        // there is no need for a font that couldn't resolve anything
        pFallback->Release();
        return nullptr;
    }

    pFallback->AdjustLayout( rLayoutArgs );

    return pFallback;
}

SalLayout* OutputDevice::ImplGlyphFallbackLayout( SalLayout* pSalLayout, ImplLayoutArgs& rLayoutArgs ) const
{
    // This function relies on a valid mpFontEntry, if it doesn't exist bail out
    // - we'd have crashed later on anyway. At least here we can catch the error in debug
    // mode.
    if ( !mpFontEntry )
    {
        SAL_WARN ("vcl.gdi", "No font entry set in OutputDevice");
        assert(mpFontEntry);
        return nullptr;
    }

    // prepare multi level glyph fallback
    MultiSalLayout* pMultiSalLayout = nullptr;
    ImplLayoutRuns aLayoutRuns = rLayoutArgs.maRuns;
    rLayoutArgs.PrepareFallback();
    rLayoutArgs.mnFlags |= SalLayoutFlags::ForFallback;

    // get list of unicodes that need glyph fallback
    int nCharPos = -1;
    bool bRTL = false;
    OUStringBuffer aMissingCodeBuf;
    while (rLayoutArgs.GetNextPos( &nCharPos, &bRTL))
        aMissingCodeBuf.append(rLayoutArgs.mrStr[nCharPos]);
    rLayoutArgs.ResetPos();
    OUString aMissingCodes = aMissingCodeBuf.makeStringAndClear();

    FontSelectPattern aFontSelData = mpFontEntry->maFontSelData;

    // try if fallback fonts support the missing unicodes
    for( int nFallbackLevel = 1; nFallbackLevel < MAX_FALLBACK; ++nFallbackLevel )
    {
        // find a font family suited for glyph fallback
        // GetGlyphFallbackFont() needs a valid aFontSelData.mpFontEntry
        // if the system-specific glyph fallback is active
        aFontSelData.mpFontEntry = mpFontEntry; // reset the fontentry to base-level

        ImplFontEntry* pFallbackFont = mpFontCache->GetGlyphFallbackFont( mpFontCollection,
            aFontSelData, nFallbackLevel, aMissingCodes );
        if( !pFallbackFont )
            break;

        aFontSelData.mpFontEntry = pFallbackFont;
        aFontSelData.mpFontData = pFallbackFont->maFontSelData.mpFontData;
        if( nFallbackLevel < MAX_FALLBACK-1)
        {
            // ignore fallback font if it is the same as the original font
            // unless we are looking for a substitution for 0x202F, in which
            // case we'll just use a normal space
            if( mpFontEntry->maFontSelData.mpFontData == aFontSelData.mpFontData &&
                aMissingCodes.indexOf(0x202F) == -1 )
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
                pMultiSalLayout->SetIncomplete();
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

sal_Int32 OutputDevice::HasGlyphs( const vcl::Font& rTempFont, const OUString& rStr,
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
    const vcl::Font aOrigFont = GetFont();
    const_cast<OutputDevice&>(*this).SetFont( rTempFont );
    FontCharMapPtr pFontCharMap ( new FontCharMap() );
    bool bRet = GetFontCharMap( pFontCharMap );
    const_cast<OutputDevice&>(*this).SetFont( aOrigFont );

    // if fontmap is unknown assume it doesn't have the glyphs
    if( !bRet )
        return nIndex;

    for( sal_Int32 i = nIndex; nIndex < nEnd; ++i, ++nIndex )
        if( ! pFontCharMap->HasChar( rStr[i] ) )
            return nIndex;

    pFontCharMap = nullptr;

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
