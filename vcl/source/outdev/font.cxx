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

#include <i18nlangtag/mslangid.hxx>

#include <unotools/configmgr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>

#include "sallayout.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"

#include "outdev.h"
#include "window.h"

#include "PhysicalFontCollection.hxx"

#include "svids.hrc"

FontMetric OutputDevice::GetDevFont( int nDevFontIndex ) const
{
    FontMetric aFontMetric;

    ImplInitFontList();

    int nCount = GetDevFontCount();
    if( nDevFontIndex < nCount )
    {
        const PhysicalFontFace& rData = *mpDeviceFontList->Get( nDevFontIndex );
        aFontMetric.SetFamilyName( rData.GetFamilyName() );
        aFontMetric.SetStyleName( rData.GetStyleName() );
        aFontMetric.SetCharSet( rData.GetCharSet() );
        aFontMetric.SetFamily( rData.GetFamilyType() );
        aFontMetric.SetPitch( rData.GetPitch() );
        aFontMetric.SetWeight( rData.GetWeight() );
        aFontMetric.SetItalic( rData.GetItalic() );
        aFontMetric.SetAlignment( TextAlign::ALIGN_TOP );
        aFontMetric.SetWidthType( rData.GetWidthType() );
        aFontMetric.SetQuality( rData.GetQuality() );
        aFontMetric.SetMapNames( rData.GetMapNames() );
    }

    return aFontMetric;
}

int OutputDevice::GetDevFontCount() const
{
    if( !mpDeviceFontList )
    {
        if (!mpFontCollection)
        {
            return 0;
        }

        mpDeviceFontList = mpFontCollection->GetDeviceFontList();

        if (!mpDeviceFontList->Count())
        {
            delete mpDeviceFontList;
            mpDeviceFontList = nullptr;

            return 0;
        }
    }
    return mpDeviceFontList->Count();
}

bool OutputDevice::IsFontAvailable( const OUString& rFontName ) const
{
    PhysicalFontFamily* pFound = mpFontCollection->FindFontFamily( rFontName );
    return (pFound != nullptr);
}

int OutputDevice::GetDevFontSizeCount( const vcl::Font& rFont ) const
{
    delete mpDeviceFontSizeList;

    ImplInitFontList();
    mpDeviceFontSizeList = mpFontCollection->GetDeviceFontSizeList( rFont.GetFamilyName() );
    return mpDeviceFontSizeList->Count();
}

Size OutputDevice::GetDevFontSize( const vcl::Font& rFont, int nSizeIndex ) const
{
    // check range
    int nCount = GetDevFontSizeCount( rFont );
    if ( nSizeIndex >= nCount )
        return Size();

    // when mapping is enabled round to .5 points
    Size aSize( 0, mpDeviceFontSizeList->Get( nSizeIndex ) );
    if ( mbMap )
    {
        aSize.Height() *= 10;
        MapMode aMap( MapUnit::Map10thInch, Point(), Fraction( 1, 72 ), Fraction( 1, 72 ) );
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

    LogicalFontInstance* pFontInstance = mpFontInstance;
    ImplFontMetricDataRef xFontMetric = pFontInstance->mxFontMetric;

    // prepare metric
    aMetric.Font::operator=( maFont );

    // set aMetric with info from font
    aMetric.SetFamilyName( maFont.GetFamilyName() );
    aMetric.SetStyleName( xFontMetric->GetStyleName() );
    aMetric.SetFontSize( PixelToLogic( Size( xFontMetric->GetWidth(), xFontMetric->GetAscent() + xFontMetric->GetDescent() - xFontMetric->GetInternalLeading() ) ) );
    aMetric.SetCharSet( xFontMetric->IsSymbolFont() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
    aMetric.SetFamily( xFontMetric->GetFamilyType() );
    aMetric.SetPitch( xFontMetric->GetPitch() );
    aMetric.SetWeight( xFontMetric->GetWeight() );
    aMetric.SetItalic( xFontMetric->GetItalic() );
    aMetric.SetAlignment( TextAlign::ALIGN_TOP );
    aMetric.SetWidthType( xFontMetric->GetWidthType() );
    if ( pFontInstance->mnOwnOrientation )
        aMetric.SetOrientation( pFontInstance->mnOwnOrientation );
    else
        aMetric.SetOrientation( xFontMetric->GetOrientation() );

    // set remaining metric fields
    aMetric.SetFullstopCenteredFlag( xFontMetric->IsFullstopCentered() );
    aMetric.SetBulletOffset( xFontMetric->GetBulletOffset() );
    aMetric.SetAscent( ImplDevicePixelToLogicHeight( xFontMetric->GetAscent() + mnEmphasisAscent ) );
    aMetric.SetDescent( ImplDevicePixelToLogicHeight( xFontMetric->GetDescent() + mnEmphasisDescent ) );
    aMetric.SetInternalLeading( ImplDevicePixelToLogicHeight( xFontMetric->GetInternalLeading() + mnEmphasisAscent ) );
    // OutputDevice has its own external leading function due to #i60945#
    aMetric.SetExternalLeading( ImplDevicePixelToLogicHeight( GetFontExtLeading() ) );
    aMetric.SetLineHeight( ImplDevicePixelToLogicHeight( xFontMetric->GetAscent() + xFontMetric->GetDescent() + mnEmphasisAscent + mnEmphasisDescent ) );
    aMetric.SetSlant( ImplDevicePixelToLogicHeight( xFontMetric->GetSlant() ) );

    // get miscellaneous data
    aMetric.SetQuality( xFontMetric->GetQuality() );
    aMetric.SetMapNames( xFontMetric->GetMapNames() );

    SAL_INFO("vcl.gdi.fontmetric", "OutputDevice::GetFontMetric:" << aMetric);

    xFontMetric = nullptr;

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

bool OutputDevice::GetFontCharMap( FontCharMapRef& rxFontCharMap ) const
{
    // we need a graphics
    if( !mpGraphics && !AcquireGraphics() )
        return false;

    if( mbNewFont )
        ImplNewFont();
    if( mbInitFont )
        InitFont();
    if( !mpFontInstance )
        return false;

    FontCharMapRef xFontCharMap ( mpGraphics->GetFontCharMap() );
    if (!xFontCharMap.is())
    {
        FontCharMapRef xDefaultMap( new FontCharMap() );
        rxFontCharMap = xDefaultMap;
    }
    else
        rxFontCharMap = xFontCharMap;

    if( rxFontCharMap->IsDefaultMap() )
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
    if( !mpFontInstance )
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
    static const PolyFlags aAccentPolyFlags[24] =
    {
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Normal,  PolyFlags::Control,
        PolyFlags::Normal, PolyFlags::Control, PolyFlags::Control
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

    FontEmphasisMark    nEmphasisStyle = eEmphasis & FontEmphasisMark::Style;
    long                nDotSize = 0;
    switch ( nEmphasisStyle )
    {
        case FontEmphasisMark::Dot:
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

        case FontEmphasisMark::Circle:
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

        case FontEmphasisMark::Disc:
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

        case FontEmphasisMark::Accent:
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
        default: break;
    }

    // calculate position
    long nOffY = 1+(mnDPIY/300); // one visible pixel space
    long nSpaceY = nHeight-nDotSize;
    if ( nSpaceY >= nOffY*2 )
        rYOff += nOffY;
    if ( !(eEmphasis & FontEmphasisMark::PosBelow) )
        rYOff += nDotSize;
}

FontEmphasisMark OutputDevice::ImplGetEmphasisMarkStyle( const vcl::Font& rFont )
{
    FontEmphasisMark nEmphasisMark = rFont.GetEmphasisMark();

    // If no Position is set, then calculate the default position, which
    // depends on the language
    if ( !(nEmphasisMark & (FontEmphasisMark::PosAbove | FontEmphasisMark::PosBelow)) )
    {
        LanguageType eLang = rFont.GetLanguage();
        // In Chinese Simplified the EmphasisMarks are below/left
        if (MsLangId::isSimplifiedChinese(eLang))
            nEmphasisMark |= FontEmphasisMark::PosBelow;
        else
        {
            eLang = rFont.GetCJKContextLanguage();
            // In Chinese Simplified the EmphasisMarks are below/left
            if (MsLangId::isSimplifiedChinese(eLang))
                nEmphasisMark |= FontEmphasisMark::PosBelow;
            else
                nEmphasisMark |= FontEmphasisMark::PosAbove;
        }
    }

    return nEmphasisMark;
}

long OutputDevice::GetFontExtLeading() const
{
    return mpFontInstance->mxFontMetric->GetExternalLeading();
}

void OutputDevice::ImplClearFontData( const bool bNewFontLists )
{
    // the currently selected logical font is no longer needed
    if ( mpFontInstance )
    {
        mpFontCache->Release( mpFontInstance );
        mpFontInstance = nullptr;
    }

    mbInitFont = true;
    mbNewFont = true;

    if ( bNewFontLists )
    {
        if ( mpDeviceFontList )
        {
            delete mpDeviceFontList;
            mpDeviceFontList = nullptr;
        }
        if ( mpDeviceFontSizeList )
        {
            delete mpDeviceFontSizeList;
            mpDeviceFontSizeList = nullptr;
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

void OutputDevice::RefreshFontData( const bool bNewFontLists )
{
    ImplRefreshFontData( bNewFontLists );
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
                    mpFontCollection = pSVData->maGDIData.mpScreenFontList->Clone();
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

void OutputDevice::ImplUpdateFontData()
{
    ImplClearFontData( true/*bNewFontLists*/ );
    ImplRefreshFontData( true/*bNewFontLists*/ );
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
                OutputDevice *pDevice = pFrame;
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
    std::list<ImplFontSubstEntry>::iterator it = maFontSubstList.begin();
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
    const OUString& rSearchName ) const
{
    // TODO: get rid of O(N) searches
    std::list<ImplFontSubstEntry>::const_iterator it = maFontSubstList.begin();
    for(; it != maFontSubstList.end(); ++it )
    {
        const ImplFontSubstEntry& rEntry = *it;
        if( (rEntry.mnFlags & AddFontSubstituteFlags::ALWAYS) && rEntry.maSearchName == rSearchName )
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
    if( pSubst && pSubst->FindFontSubstitute( aSubstFontName, rFontName ) )
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
        aFont.SetFontHeight( 12 ); // corresponds to nDefaultHeight
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
            sal_Int32     nIndex = 0;
            do
            {
                PhysicalFontFamily* pFontFamily = pOutDev->mpFontCollection->FindFontFamily( GetNextFontToken( aSearch, nIndex ) );
                if( pFontFamily )
                {
                    AddTokenFontName( aName, pFontFamily->GetFamilyName() );
                    if( nFlags & GetDefaultFontFlags::OnlyOne )
                        break;
                }
            }
            while ( nIndex != -1 );
            aFont.SetFamilyName( aName );
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
                    aFont.SetFamilyName( aSearch.getToken( 0, ';', nIndex ) );
                }
                else
                {
                    pOutDev->ImplInitFontList();

                    aFont.SetFamilyName( aSearch );

                    // convert to pixel height
                    Size aSize = pOutDev->ImplLogicToDevicePixel( aFont.GetFontSize() );
                    if ( !aSize.Height() )
                    {
                        // use default pixel height only when logical height is zero
                        if ( aFont.GetFontHeight() )
                            aSize.Height() = 1;
                        else
                            aSize.Height() = (12*pOutDev->mnDPIY)/72;
                    }

                    // use default width only when logical width is zero
                    if( (0 == aSize.Width()) && (0 != aFont.GetFontSize().Width()) )
                        aSize.Width() = 1;

                    // get the name of the first available font
                    float fExactHeight = static_cast<float>(aSize.Height());
                    LogicalFontInstance* pFontInstance = pOutDev->mpFontCache->GetFontInstance( pOutDev->mpFontCollection, aFont, aSize, fExactHeight );
                    if (pFontInstance)
                    {
                        if( pFontInstance->maFontSelData.mpFontData )
                            aFont.SetFamilyName( pFontInstance->maFontSelData.mpFontData->GetFamilyName() );
                        else
                            aFont.SetFamilyName( pFontInstance->maFontSelData.maTargetName );
                        pOutDev->mpFontCache->Release(pFontInstance);
                    }
                }
            }
            else
                aFont.SetFamilyName( aSearch );
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
    SAL_INFO("vcl.gdi",
             "OutputDevice::GetDefaultFont() Type=\"" << s
             << "\" lang=" << eLang
             << " flags=" << nFlags
             << " FontName=\"" << OUStringToOString( aFont.GetName(), RTL_TEXTENCODING_UTF8 ).getStr());
#endif

    return aFont;
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

    if (!mpFontInstance)
        return;

    if ( mbInitFont )
    {
        // decide if antialiasing is appropriate
        bool bNonAntialiased(GetAntialiasing() & AntialiasingFlags::DisableText);
        if (!utl::ConfigManager::IsAvoidConfig())
        {
            const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
            bNonAntialiased |= bool(rStyleSettings.GetDisplayOptions() & DisplayOptions::AADisable);
            bNonAntialiased |= (int(rStyleSettings.GetAntialiasingMinPixelHeight()) > mpFontInstance->maFontSelData.mnHeight);
        }
        mpFontInstance->maFontSelData.mbNonAntialiased = bNonAntialiased;

        // select font in the device layers
        mpGraphics->SetFont( &(mpFontInstance->maFontSelData), 0 );
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
            const_cast<OutputDevice&>(*this).ImplUpdateFontData();
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
    float fExactHeight = ImplFloatLogicHeightToDevicePixel( static_cast<float>(maFont.GetFontHeight()) );
    Size aSize = ImplLogicToDevicePixel( maFont.GetFontSize() );
    if ( !aSize.Height() )
    {
        // use default pixel height only when logical height is zero
        if ( maFont.GetFontSize().Height() )
            aSize.Height() = 1;
        else
            aSize.Height() = (12*mnDPIY)/72;
        fExactHeight =  static_cast<float>(aSize.Height());
    }

    // select the default width only when logical width is zero
    if( (0 == aSize.Width()) && (0 != maFont.GetFontSize().Width()) )
        aSize.Width() = 1;

    // get font entry
    LogicalFontInstance* pOldFontInstance = mpFontInstance;
    mpFontInstance = mpFontCache->GetFontInstance( mpFontCollection, maFont, aSize, fExactHeight );
    if( pOldFontInstance )
        mpFontCache->Release( pOldFontInstance );

    LogicalFontInstance* pFontInstance = mpFontInstance;

    if (!pFontInstance)
    {
        SAL_WARN("vcl.gdi", "OutputDevice::ImplNewFont(): no LogicalFontInstance, no Font");
        return false;
    }

    // mark when lower layers need to get involved
    mbNewFont = false;
    if( pFontInstance != pOldFontInstance )
        mbInitFont = true;

    // select font when it has not been initialized yet
    if ( !pFontInstance->mbInit )
    {
        InitFont();

        // get metric data from device layers
        if ( pGraphics )
        {
            pFontInstance->mbInit = true;

            pFontInstance->mxFontMetric->SetOrientation( sal::static_int_cast<short>(pFontInstance->maFontSelData.mnOrientation) );
            pGraphics->GetFontMetric( pFontInstance->mxFontMetric, 0 );

            pFontInstance->mxFontMetric->ImplInitTextLineSize( this );
            pFontInstance->mxFontMetric->ImplInitAboveTextLineSize();

            pFontInstance->mnLineHeight = pFontInstance->mxFontMetric->GetAscent() + pFontInstance->mxFontMetric->GetDescent();

            SetFontOrientation( pFontInstance );
        }
    }

    // enable kerning array if requested
    if ( maFont.GetKerning() & FontKerning::FontSpecific )
    {
        mbKerning = true;
    }
    else
    {
        mbKerning = false;
    }

    if ( maFont.GetKerning() & FontKerning::Asian )
        mbKerning = true;

    // calculate EmphasisArea
    mnEmphasisAscent = 0;
    mnEmphasisDescent = 0;
    if ( maFont.GetEmphasisMark() & FontEmphasisMark::Style )
    {
        FontEmphasisMark    nEmphasisMark = ImplGetEmphasisMarkStyle( maFont );
        long                nEmphasisHeight = (pFontInstance->mnLineHeight*250)/1000;
        if ( nEmphasisHeight < 1 )
            nEmphasisHeight = 1;
        if ( nEmphasisMark & FontEmphasisMark::PosBelow )
            mnEmphasisDescent = nEmphasisHeight;
        else
            mnEmphasisAscent = nEmphasisHeight;
    }

    // calculate text offset depending on TextAlignment
    TextAlign eAlign = maFont.GetAlignment();
    if ( eAlign == ALIGN_BASELINE )
    {
        mnTextOffX = 0;
        mnTextOffY = 0;
    }
    else if ( eAlign == ALIGN_TOP )
    {
        mnTextOffX = 0;
        mnTextOffY = +pFontInstance->mxFontMetric->GetAscent() + mnEmphasisAscent;
        if ( pFontInstance->mnOrientation )
        {
            Point aOriginPt(0, 0);
            aOriginPt.RotateAround( mnTextOffX, mnTextOffY, pFontInstance->mnOrientation );
        }
    }
    else // eAlign == ALIGN_BOTTOM
    {
        mnTextOffX = 0;
        mnTextOffY = -pFontInstance->mxFontMetric->GetDescent() + mnEmphasisDescent;
        if ( pFontInstance->mnOrientation )
        {
            Point aOriginPt(0, 0);
            aOriginPt.RotateAround( mnTextOffX, mnTextOffY, pFontInstance->mnOrientation );
        }
    }

    mbTextLines     = ((maFont.GetUnderline() != LINESTYLE_NONE) && (maFont.GetUnderline() != LINESTYLE_DONTKNOW)) ||
                      ((maFont.GetOverline()  != LINESTYLE_NONE) && (maFont.GetOverline()  != LINESTYLE_DONTKNOW)) ||
                      ((maFont.GetStrikeout() != STRIKEOUT_NONE) && (maFont.GetStrikeout() != STRIKEOUT_DONTKNOW));
    mbTextSpecial   = maFont.IsShadow() || maFont.IsOutline() ||
                      (maFont.GetRelief() != FontRelief::NONE);


    // #95414# fix for OLE objects which use scale factors very creatively
    if( mbMap && !aSize.Width() )
    {
        int nOrigWidth = pFontInstance->mxFontMetric->GetWidth();
        float fStretch = (float)maMapRes.mnMapScNumX * maMapRes.mnMapScDenomY;
        fStretch /= (float)maMapRes.mnMapScNumY * maMapRes.mnMapScDenomX;
        int nNewWidth = (int)(nOrigWidth * fStretch + 0.5);
        if( (nNewWidth != nOrigWidth) && (nNewWidth != 0) )
        {
            Size aOrigSize = maFont.GetFontSize();
            const_cast<vcl::Font&>(maFont).SetFontSize( Size( nNewWidth, aSize.Height() ) );
            mbMap = false;
            mbNewFont = true;
            ImplNewFont();  // recurse once using stretched width
            mbMap = true;
            const_cast<vcl::Font&>(maFont).SetFontSize( aOrigSize );
        }
    }

    return true;
}

void OutputDevice::SetFontOrientation( LogicalFontInstance* const pFontInstance ) const
{
    if( pFontInstance->maFontSelData.mnOrientation && !pFontInstance->mxFontMetric->GetOrientation() )
    {
        pFontInstance->mnOwnOrientation = sal::static_int_cast<short>(pFontInstance->maFontSelData.mnOrientation);
        pFontInstance->mnOrientation = pFontInstance->mnOwnOrientation;
    }
    else
    {
        pFontInstance->mnOrientation = pFontInstance->mxFontMetric->GetOrientation();
    }
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

    if ( nEmphasisMark & FontEmphasisMark::PosBelow )
        nEmphasisHeight = mnEmphasisDescent;
    else
        nEmphasisHeight = mnEmphasisAscent;

    ImplGetEmphasisMark( aPolyPoly, bPolyLine,
                         aRect1, aRect2,
                         nEmphasisYOff, nEmphasisWidth,
                         nEmphasisMark,
                         nEmphasisHeight, mpFontInstance->mnOrientation );

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

    if ( nEmphasisMark & FontEmphasisMark::PosBelow )
        aOffset.Y() += mpFontInstance->mxFontMetric->GetDescent() + nEmphasisYOff;
    else
        aOffset.Y() -= mpFontInstance->mxFontMetric->GetAscent() + nEmphasisYOff;

    long nEmphasisWidth2  = nEmphasisWidth / 2;
    long nEmphasisHeight2 = nEmphasisHeight / 2;
    aOffset += Point( nEmphasisWidth2, nEmphasisHeight2 );

    Point aOutPoint;
    Rectangle aRectangle;
    const GlyphItem* pGlyph;
    int nStart = 0;
    while (rSalLayout.GetNextGlyphs(1, &pGlyph, aOutPoint, nStart))
    {
        if (!mpGraphics->GetGlyphBoundRect(*pGlyph, aRectangle ) )
            continue;

        if (!pGlyph->IsSpacing())
        {
            Point aAdjPoint = aOffset;
            aAdjPoint.X() += aRectangle.Left() + (aRectangle.GetWidth() - nEmphasisWidth) / 2;
            if ( mpFontInstance->mnOrientation )
            {
                Point aOriginPt(0, 0);
                aOriginPt.RotateAround( aAdjPoint.X(), aAdjPoint.Y(), mpFontInstance->mnOrientation );
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

SalLayout* OutputDevice::getFallbackFont(
    FontSelectPattern &rFontSelData, int nFallbackLevel,
    ImplLayoutArgs& rLayoutArgs) const
{
    // we need a graphics
    if (!mpGraphics && !AcquireGraphics())
        return nullptr;

    assert(mpGraphics != nullptr);
    mpGraphics->SetFont( &rFontSelData, nFallbackLevel );

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
    // This function relies on a valid mpFontInstance, if it doesn't exist bail out
    // - we'd have crashed later on anyway. At least here we can catch the error in debug
    // mode.
    if ( !mpFontInstance )
    {
        SAL_WARN ("vcl.gdi", "No font entry set in OutputDevice");
        assert(mpFontInstance);
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

    FontSelectPattern aFontSelData = mpFontInstance->maFontSelData;

    // try if fallback fonts support the missing unicodes
    for( int nFallbackLevel = 1; nFallbackLevel < MAX_FALLBACK; ++nFallbackLevel )
    {
        // find a font family suited for glyph fallback
        // GetGlyphFallbackFont() needs a valid aFontSelData.mpFontInstance
        // if the system-specific glyph fallback is active
        aFontSelData.mpFontInstance = mpFontInstance; // reset the fontinstance to base-level

        LogicalFontInstance* pFallbackFont = mpFontCache->GetGlyphFallbackFont( mpFontCollection,
            aFontSelData, nFallbackLevel, aMissingCodes );
        if( !pFallbackFont )
            break;

        aFontSelData.mpFontInstance = pFallbackFont;
        aFontSelData.mpFontData = pFallbackFont->maFontSelData.mpFontData;
        if( nFallbackLevel < MAX_FALLBACK-1)
        {
            // ignore fallback font if it is the same as the original font
            // unless we are looking for a substitution for 0x202F, in which
            // case we'll just use a normal space
            if( mpFontInstance->maFontSelData.mpFontData == aFontSelData.mpFontData &&
                aMissingCodes.indexOf(0x202F) == -1 )
            {
                mpFontCache->Release( pFallbackFont );
                continue;
            }
        }

        // create and add glyph fallback layout to multilayout
        SalLayout* pFallback = getFallbackFont(aFontSelData,
            nFallbackLevel, rLayoutArgs);
        if (pFallback)
        {
            if( !pMultiSalLayout )
                pMultiSalLayout = new MultiSalLayout( *pSalLayout );
            pMultiSalLayout->AddFallback( *pFallback,
                rLayoutArgs.maRuns, aFontSelData.mpFontData );
            if (nFallbackLevel == MAX_FALLBACK-1)
                pMultiSalLayout->SetIncomplete(true);
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

    return ImplDevicePixelToLogicWidth( mpFontInstance->mxFontMetric->GetMinKashida() );
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

    SAL_WARN_IF( nIndex >= nEnd, "vcl.gdi", "StartPos >= EndPos?" );
    SAL_WARN_IF( nEnd > rStr.getLength(), "vcl.gdi", "String too short" );

    // to get the map temporarily set font
    const vcl::Font aOrigFont = GetFont();
    const_cast<OutputDevice&>(*this).SetFont( rTempFont );
    FontCharMapRef xFontCharMap ( new FontCharMap() );
    bool bRet = GetFontCharMap( xFontCharMap );
    const_cast<OutputDevice&>(*this).SetFont( aOrigFont );

    // if fontmap is unknown assume it doesn't have the glyphs
    if( !bRet )
        return nIndex;

    for( sal_Int32 i = nIndex; nIndex < nEnd; ++i, ++nIndex )
        if( ! xFontCharMap->HasChar( rStr[i] ) )
            return nIndex;

    return -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
