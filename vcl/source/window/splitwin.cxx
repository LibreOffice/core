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

#include <string.h>

#include <tools/debug.hxx>
#include <sal/log.hxx>

#include <vcl/event.hxx>
#include <vcl/wall.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/help.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>

#include <vcl/vclenum.hxx>

#include <svdata.hxx>
#include <strings.hrc>


#define SPLITWIN_SPLITSIZE              4
#define SPLITWIN_SPLITSIZEEX            4
#define SPLITWIN_SPLITSIZEEXLN          7
#define SPLITWIN_SPLITSIZEAUTOHIDE      72
#define SPLITWIN_SPLITSIZEFADE          72

#define SPLIT_HORZ              (sal_uInt16(0x0001))
#define SPLIT_VERT              (sal_uInt16(0x0002))
#define SPLIT_WINDOW            (sal_uInt16(0x0004))
#define SPLIT_NOSPLIT           (sal_uInt16(0x8000))

class ImplSplitItem
{
public:
    ImplSplitItem();

    long                mnSize;
    long                mnPixSize;
    long                mnLeft;
    long                mnTop;
    long                mnWidth;
    long                mnHeight;
    long                mnSplitPos;
    long                mnSplitSize;
    long                mnOldSplitPos;
    long                mnOldSplitSize;
    long                mnOldWidth;
    long                mnOldHeight;
    std::unique_ptr<ImplSplitSet> mpSet;
    VclPtr<vcl::Window> mpWindow;
    VclPtr<vcl::Window> mpOrgParent;
    sal_uInt16          mnId;
    SplitWindowItemFlags mnBits;
    bool                mbFixed;
    bool                mbSubSize;
    /// Minimal width or height of the item.  -1 means no restriction.
    long                mnMinSize;
    /// Maximal width or height of the item.  -1 means no restriction.
    long                mnMaxSize;
};


class ImplSplitSet
{
public:
    ImplSplitSet();
    ~ImplSplitSet();

    std::vector< ImplSplitItem > mvItems;
    std::unique_ptr<Wallpaper>   mpWallpaper;
    long                mnLastSize;
    long                mnSplitSize;
    sal_uInt16          mnId;
    bool                mbCalcPix;
};

ImplSplitItem::ImplSplitItem()
    : mnSize(0)
    , mnPixSize(0)
    , mnLeft(0)
    , mnTop(0)
    , mnWidth(0)
    , mnHeight(0)
    , mnSplitPos(0)
    , mnSplitSize(0)
    , mnOldSplitPos(0)
    , mnOldSplitSize(0)
    , mnOldWidth(0)
    , mnOldHeight(0)
    , mnId(0)
    , mnBits(SplitWindowItemFlags::NONE)
    , mbFixed(false)
    , mbSubSize(false)
    , mnMinSize(-1)
    , mnMaxSize(-1)
{
}

ImplSplitSet::ImplSplitSet() :
    mnLastSize( 0 ),
    mnSplitSize( SPLITWIN_SPLITSIZE ),
    mnId( 0 ),
    mbCalcPix( true )
{
}

ImplSplitSet::~ImplSplitSet()
{
    mpWallpaper.reset();
}

/** Check whether the given size is inside the valid range defined by
    [rItem.mnMinSize,rItem.mnMaxSize].  When it is not inside it then return
    the upper or lower bound, respectively. Otherwise return the given size
    unmodified.
    Note that either mnMinSize and/or mnMaxSize can be -1 in which case the
    size has not lower or upper bound.
*/
namespace {
    long ValidateSize (const long nSize, const ImplSplitItem & rItem)
    {
        if (rItem.mnMinSize>=0 && nSize<rItem.mnMinSize)
            return rItem.mnMinSize;
        else if (rItem.mnMaxSize>0 && nSize>rItem.mnMaxSize)
            return rItem.mnMaxSize;
        else
            return nSize;
    }
}

static void ImplCalcBorder( WindowAlign eAlign,
                            long& rLeft, long& rTop,
                            long& rRight, long& rBottom )
{
    switch ( eAlign )
    {
    case WindowAlign::Top:
        rLeft   = 2;
        rTop    = 2;
        rRight  = 2;
        rBottom = 0;
        break;
    case WindowAlign::Left:
        rLeft   = 0;
        rTop    = 2;
        rRight  = 2;
        rBottom = 2;
        break;
    case WindowAlign::Bottom:
        rLeft   = 2;
        rTop    = 0;
        rRight  = 2;
        rBottom = 2;
        break;
    default:
        rLeft   = 0;
        rTop    = 2;
        rRight  = 2;
        rBottom = 2;
        break;
    }
}

void SplitWindow::ImplDrawBorder(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    long nDX = mnDX;
    long nDY = mnDY;

    switch (meAlign)
    {
    case WindowAlign::Bottom:
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
        rRenderContext.DrawLine(Point(0, 0), Point(nDX - 1, 0));
        rRenderContext.DrawLine(Point(0, nDY - 2), Point(nDX - 1, nDY - 2));

        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        rRenderContext.DrawLine(Point(0, 1), Point(nDX - 1, 1));
        rRenderContext.DrawLine(Point(0, nDY - 1), Point(nDX - 1, nDY - 1));
        break;
    case WindowAlign::Top:
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
        rRenderContext.DrawLine(Point(0, nDY - 2), Point(nDX - 1, nDY - 2));
        rRenderContext.DrawLine(Point(0, 0), Point(nDX - 1, 0));

        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        rRenderContext.DrawLine(Point(0, nDY - 1), Point(nDX - 1, nDY - 1));
        rRenderContext.DrawLine(Point(0, 1), Point(nDX - 1, 1));
        break;
    case WindowAlign::Left:
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
        rRenderContext.DrawLine(Point(nDX - 2, 0), Point(nDX - 2, nDY - 2));
        rRenderContext.DrawLine(Point(0, 0), Point(nDX - 1, 0));
        rRenderContext.DrawLine(Point(0, nDY - 2), Point(nDX - 2, nDY - 2));

        rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
        rRenderContext.DrawLine(Point(nDX - 1, 0), Point(nDX - 1, nDY - 1));
        rRenderContext.DrawLine(Point(0, 1), Point(nDX - 3, 1));
        rRenderContext.DrawLine(Point(0, nDY - 1), Point(nDX - 2, nDY - 1));
        break;
    default:
        rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
        rRenderContext.DrawLine(Point(0, 0), Point( 0, nDY - 2));
        rRenderContext.DrawLine(Point(0, 0), Point( nDX - 1, 0));
        rRenderContext.DrawLine(Point(0, nDY - 2), Point(nDX - 1, nDY - 2));

        rRenderContext.SetLineColor( rStyleSettings.GetLightColor());
        rRenderContext.DrawLine(Point(1, 1), Point(1, nDY - 3));
        rRenderContext.DrawLine(Point(1, 1), Point(nDX - 1, 1));
        rRenderContext.DrawLine(Point(0, nDY - 1), Point(nDX - 1, nDY - 1));
    }
}

void SplitWindow::ImplDrawBorderLine(vcl::RenderContext& rRenderContext)
{
    if (mbFadeOut)
    {
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        long nDX = mnDX;
        long  nDY = mnDY;

        switch (meAlign)
        {
        case WindowAlign::Left:
            rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
            rRenderContext.DrawLine( Point( nDX-SPLITWIN_SPLITSIZEEXLN-1, 1 ), Point( nDX-SPLITWIN_SPLITSIZEEXLN-1, nDY-2 ) );

            rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
            rRenderContext.DrawLine( Point( nDX-SPLITWIN_SPLITSIZEEXLN, 1 ), Point( nDX-SPLITWIN_SPLITSIZEEXLN, nDY-3 ) );
            break;
        case WindowAlign::Right:
            rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
            rRenderContext.DrawLine( Point( SPLITWIN_SPLITSIZEEXLN-1, 0 ), Point( SPLITWIN_SPLITSIZEEXLN-1, nDY-2 ) );

            rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
            rRenderContext.DrawLine( Point( SPLITWIN_SPLITSIZEEXLN, 1 ), Point( SPLITWIN_SPLITSIZEEXLN, nDY-3 ) );
            break;
        case WindowAlign::Top:
            rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
            rRenderContext.DrawLine( Point( 0, nDY-SPLITWIN_SPLITSIZEEXLN-1 ), Point( nDX-1, nDY-SPLITWIN_SPLITSIZEEXLN-1 ) );

            rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
            rRenderContext.DrawLine( Point( 0, nDY-SPLITWIN_SPLITSIZEEXLN ), Point( nDX-1, nDY-SPLITWIN_SPLITSIZEEXLN ) );
            break;
        case WindowAlign::Bottom:
            rRenderContext.SetLineColor( rStyleSettings.GetShadowColor() );
            rRenderContext.DrawLine( Point( 0, 5 ), Point( nDX-1, 5 ) );

            rRenderContext.SetLineColor( rStyleSettings.GetLightColor() );
            rRenderContext.DrawLine( Point( 0, SPLITWIN_SPLITSIZEEXLN ), Point( nDX-1, SPLITWIN_SPLITSIZEEXLN ) );
            break;
        }
    }
}

static ImplSplitSet* ImplFindSet( ImplSplitSet* pSet, sal_uInt16 nId )
{
    if ( pSet->mnId == nId )
        return pSet;

    std::vector< ImplSplitItem >&     rItems = pSet->mvItems;

    for ( auto& rItem : rItems )
    {
        if ( rItem.mnId == nId )
            return rItem.mpSet.get();
    }

    for ( auto& rItem : rItems )
    {
        if ( rItem.mpSet )
        {
            ImplSplitSet* pFindSet = ImplFindSet( rItem.mpSet.get(), nId );
            if ( pFindSet )
                return pFindSet;
        }
    }

    return nullptr;
}

static ImplSplitSet* ImplFindItem( ImplSplitSet* pSet, sal_uInt16 nId, sal_uInt16& rPos )
{
    size_t              nItems = pSet->mvItems.size();
    std::vector< ImplSplitItem >&     rItems = pSet->mvItems;

    for ( size_t i = 0; i < nItems; i++ )
    {
        if ( rItems[i].mnId == nId )
        {
            rPos = i;
            return pSet;
        }
    }

    for ( auto& rItem : rItems )
    {
        if ( rItem.mpSet )
        {
            ImplSplitSet* pFindSet = ImplFindItem( rItem.mpSet.get(), nId, rPos );
            if ( pFindSet )
                return pFindSet;
        }
    }

    return nullptr;
}

static sal_uInt16 ImplFindItem( ImplSplitSet* pSet, vcl::Window* pWindow )
{
    std::vector< ImplSplitItem >&     rItems = pSet->mvItems;

    for ( auto& rItem : rItems )
    {
        if ( rItem.mpWindow == pWindow )
            return rItem.mnId;
        else
        {
            if ( rItem.mpSet )
            {
                sal_uInt16 nId = ImplFindItem( rItem.mpSet.get(), pWindow );
                if ( nId )
                    return nId;
            }
        }
    }

    return 0;
}

static sal_uInt16 ImplFindItem( ImplSplitSet* pSet, const Point& rPos,
                            bool bRows, bool bDown = true )
{
    std::vector< ImplSplitItem >&     rItems = pSet->mvItems;

    for ( auto& rItem : rItems )
    {
        if ( rItem.mnWidth && rItem.mnHeight )
        {
            Point       aPoint( rItem.mnLeft, rItem.mnTop );
            Size        aSize( rItem.mnWidth, rItem.mnHeight );
            tools::Rectangle   aRect( aPoint, aSize );
            if ( bRows )
            {
                if ( bDown )
                    aRect.AdjustBottom(pSet->mnSplitSize );
                else
                    aRect.AdjustTop( -(pSet->mnSplitSize) );
            }
            else
            {
                if ( bDown )
                    aRect.AdjustRight(pSet->mnSplitSize );
                else
                    aRect.AdjustLeft( -(pSet->mnSplitSize) );
            }

            if ( aRect.IsInside( rPos ) )
            {
                if ( rItem.mpSet && !rItem.mpSet->mvItems.empty() )
                {
                    return ImplFindItem( rItem.mpSet.get(), rPos,
                                        !(rItem.mnBits & SplitWindowItemFlags::ColSet) );
                }
                else
                    return rItem.mnId;
            }
        }
    }

    return 0;
}

static void ImplCalcSet( ImplSplitSet* pSet,
                         long nSetLeft, long nSetTop,
                         long nSetWidth, long nSetHeight,
                         bool bRows, bool bDown = true )
{
    if ( pSet->mvItems.empty() )
        return;

    sal_uInt16          nMins;
    sal_uInt16          nCalcItems;
    size_t              nItems = pSet->mvItems.size();
    sal_uInt16          nAbsItems;
    long                nCalcSize;
    long                nPos;
    long                nMaxPos;
    std::vector< ImplSplitItem >&     rItems = pSet->mvItems;
    bool                bEmpty;

    // calculate sizes
    if ( bRows )
        nCalcSize = nSetHeight;
    else
        nCalcSize = nSetWidth;
    nCalcSize -= (rItems.size()-1)*pSet->mnSplitSize;
    if ( pSet->mbCalcPix || (pSet->mnLastSize != nCalcSize) )
    {
        long nPercentFactor = 10;
        long nRelCount      = 0;
        long nPercent       = 0;
        long nRelPercent    = 0;
        long nAbsSize       = 0;
        long nCurSize       = 0;
        for ( auto& rItem : rItems )
        {
            if ( rItem.mnBits & SplitWindowItemFlags::RelativeSize )
                nRelCount += rItem.mnSize;
            else if ( rItem.mnBits & SplitWindowItemFlags::PercentSize )
                nPercent += rItem.mnSize;
            else
                nAbsSize += rItem.mnSize;
        }
        // map relative values to percentages (percentage here one tenth of a procent)
        nPercent *= nPercentFactor;
        if ( nRelCount )
        {
            long nRelPercentBase = 1000;
            while ( (nRelCount > nRelPercentBase) && (nPercentFactor < 100000) )
            {
                nRelPercentBase *= 10;
                nPercentFactor *= 10;
            }
            if ( nPercent < nRelPercentBase )
            {
                nRelPercent = (nRelPercentBase-nPercent)/nRelCount;
                nPercent += nRelPercent*nRelCount;
            }
            else
                nRelPercent = 0;
        }
        if ( !nPercent )
            nPercent = 1;
        long nSizeDelta = nCalcSize-nAbsSize;
        for ( auto& rItem : rItems )
        {
            if ( rItem.mnBits & SplitWindowItemFlags::RelativeSize )
            {
                if ( nSizeDelta <= 0 )
                    rItem.mnPixSize = 0;
                else
                    rItem.mnPixSize = (nSizeDelta*rItem.mnSize*nRelPercent)/nPercent;
            }
            else if ( rItem.mnBits & SplitWindowItemFlags::PercentSize )
            {
                if ( nSizeDelta <= 0 )
                    rItem.mnPixSize = 0;
                else
                    rItem.mnPixSize = (nSizeDelta*rItem.mnSize*nPercentFactor)/nPercent;
            }
            else
                rItem.mnPixSize = rItem.mnSize;
            nCurSize += rItem.mnPixSize;
        }

        pSet->mbCalcPix  = false;
        pSet->mnLastSize = nCalcSize;

        // adapt window
        nSizeDelta  = nCalcSize-nCurSize;
        if ( nSizeDelta )
        {
            nAbsItems       = 0;
            long nSizeWinSize    = 0;

            // first resize absolute items relative
            for ( auto& rItem : rItems )
            {
                if ( !(rItem.mnBits & (SplitWindowItemFlags::RelativeSize | SplitWindowItemFlags::PercentSize)) )
                {
                    nAbsItems++;
                    nSizeWinSize += rItem.mnPixSize;
                }
            }
            // do not compensate rounding errors here
            if ( (nAbsItems < static_cast<sal_uInt16>(std::abs( nSizeDelta ))) && nSizeWinSize )
            {
                long nNewSizeWinSize = 0;

                for ( auto& rItem : rItems )
                {
                    if ( !(rItem.mnBits & (SplitWindowItemFlags::RelativeSize | SplitWindowItemFlags::PercentSize)) )
                    {
                        rItem.mnPixSize += (nSizeDelta*rItem.mnPixSize)/nSizeWinSize;
                        nNewSizeWinSize += rItem.mnPixSize;
                    }
                }

                nSizeDelta -= nNewSizeWinSize-nSizeWinSize;
            }

            // compensate rounding errors now
            sal_uInt16 j = 0;
            nMins       = 0;
            while ( nSizeDelta && (nItems != nMins) )
            {
                // determine which items we can calculate
                nCalcItems = 0;
                while ( !nCalcItems )
                {
                    for ( auto& rItem : rItems )
                    {
                        rItem.mbSubSize = false;

                        if ( j >= 2 )
                            rItem.mbSubSize = true;
                        else
                        {
                            if ( (nSizeDelta > 0) || rItem.mnPixSize )
                            {
                                if ( j >= 1 )
                                    rItem.mbSubSize = true;
                                else
                                {
                                    if ( (j == 0) && (rItem.mnBits & (SplitWindowItemFlags::RelativeSize | SplitWindowItemFlags::PercentSize)) )
                                        rItem.mbSubSize = true;
                                }
                            }
                        }

                        if ( rItem.mbSubSize )
                            nCalcItems++;
                    }

                    j++;
                }

                // subtract size of individual items
                long nErrorSum       = nSizeDelta % nCalcItems;
                long nCurSizeDelta   = nSizeDelta / nCalcItems;
                nMins           = 0;
                for ( auto& rItem : rItems )
                {
                    if ( rItem.mbSubSize )
                    {
                        long* pSize = &(rItem.mnPixSize);
                        long  nTempErr;

                        if ( nErrorSum )
                        {
                            if ( nErrorSum < 0 )
                                nTempErr = -1;
                            else
                                nTempErr = 1;
                        }
                        else
                            nTempErr = 0;

                        if ( (*pSize+nCurSizeDelta+nTempErr) <= 0 )
                        {
                            long nTemp = *pSize;
                            if ( nTemp )
                            {
                                *pSize -= nTemp;
                                nSizeDelta += nTemp;
                            }
                            nMins++;
                        }
                        else
                        {
                            *pSize += nCurSizeDelta;
                            nSizeDelta -= nCurSizeDelta;
                            if ( nTempErr && (*pSize || (nTempErr > 0)) )
                            {
                                *pSize += nTempErr;
                                nSizeDelta -= nTempErr;
                                nErrorSum -= nTempErr;
                            }
                        }
                    }
                }
            }
        }
    }

    // calculate maximum size
    if ( bRows )
    {
        nPos = nSetTop;
        if ( !bDown )
            nMaxPos = nSetTop-nSetHeight;
        else
            nMaxPos = nSetTop+nSetHeight;
    }
    else
    {
        nPos = nSetLeft;
        if ( !bDown )
            nMaxPos = nSetLeft-nSetWidth;
        else
            nMaxPos = nSetLeft+nSetWidth;
    }

    // order windows and adept values
    for ( size_t i = 0; i < nItems; i++ )
    {
        rItems[i].mnOldSplitPos    = rItems[i].mnSplitPos;
        rItems[i].mnOldSplitSize   = rItems[i].mnSplitSize;
        rItems[i].mnOldWidth       = rItems[i].mnWidth;
        rItems[i].mnOldHeight      = rItems[i].mnHeight;

        bEmpty = false;
        if ( bDown )
        {
            if ( nPos+rItems[i].mnPixSize > nMaxPos )
                bEmpty = true;
        }
        else
        {
            nPos -= rItems[i].mnPixSize;
            if ( nPos < nMaxPos )
                bEmpty = true;
        }

        if ( bEmpty )
        {
            rItems[i].mnWidth     = 0;
            rItems[i].mnHeight    = 0;
            rItems[i].mnSplitSize = 0;
        }
        else
        {
            if ( bRows )
            {
                rItems[i].mnLeft   = nSetLeft;
                rItems[i].mnTop    = nPos;
                rItems[i].mnWidth  = nSetWidth;
                rItems[i].mnHeight = rItems[i].mnPixSize;
            }
            else
            {
                rItems[i].mnLeft   = nPos;
                rItems[i].mnTop    = nSetTop;
                rItems[i].mnWidth  = rItems[i].mnPixSize;
                rItems[i].mnHeight = nSetHeight;
            }

            if ( i > nItems-1 )
                rItems[i].mnSplitSize = 0;
            else
            {
                rItems[i].mnSplitSize = pSet->mnSplitSize;
                if ( bDown )
                {
                    rItems[i].mnSplitPos  = nPos+rItems[i].mnPixSize;
                    if ( rItems[i].mnSplitPos+rItems[i].mnSplitSize > nMaxPos )
                        rItems[i].mnSplitSize = nMaxPos-rItems[i].mnSplitPos;
                }
                else
                {
                    rItems[i].mnSplitPos = nPos-pSet->mnSplitSize;
                    if ( rItems[i].mnSplitPos < nMaxPos )
                        rItems[i].mnSplitSize = rItems[i].mnSplitPos+pSet->mnSplitSize-nMaxPos;
                }
            }
        }

        if ( !bDown )
            nPos -= pSet->mnSplitSize;
        else
            nPos += rItems[i].mnPixSize+pSet->mnSplitSize;
    }

    // calculate Sub-Set's
    for ( auto& rItem : rItems )
    {
        if ( rItem.mpSet && rItem.mnWidth && rItem.mnHeight )
        {
            ImplCalcSet( rItem.mpSet.get(),
                         rItem.mnLeft, rItem.mnTop,
                         rItem.mnWidth, rItem.mnHeight,
                         !(rItem.mnBits & SplitWindowItemFlags::ColSet) );
        }
    }

    // set fixed
    for ( auto& rItem : rItems )
    {
        rItem.mbFixed = false;
        if ( rItem.mnBits & SplitWindowItemFlags::Fixed )
            rItem.mbFixed = true;
        else
        {
            // this item is also fixed if Child-Set is available,
            // if a child is fixed
            if ( rItem.mpSet )
            {
                for ( auto const & j: rItem.mpSet->mvItems )
                {
                    if ( j.mbFixed )
                    {
                        rItem.mbFixed = true;
                        break;
                    }
                }
            }
        }
    }
}

void SplitWindow::ImplCalcSet2( SplitWindow* pWindow, ImplSplitSet* pSet, bool bHide,
                                bool bRows )
{
    std::vector< ImplSplitItem >&     rItems = pSet->mvItems;

    if ( pWindow->IsReallyVisible() && pWindow->IsUpdateMode() && pWindow->mbInvalidate )
    {
        for ( auto& rItem : rItems )
        {
            if ( rItem.mnSplitSize )
            {
                // invalidate all, if applicable or only a small part
                if ( (rItem.mnOldSplitPos  != rItem.mnSplitPos)  ||
                     (rItem.mnOldSplitSize != rItem.mnSplitSize) ||
                     (rItem.mnOldWidth     != rItem.mnWidth)     ||
                     (rItem.mnOldHeight    != rItem.mnHeight) )
                {
                    tools::Rectangle aRect;

                    // invalidate old rectangle
                    if ( bRows )
                    {
                        aRect.SetLeft( rItem.mnLeft );
                        aRect.SetRight( rItem.mnLeft+rItem.mnOldWidth-1 );
                        aRect.SetTop( rItem.mnOldSplitPos );
                        aRect.SetBottom( aRect.Top() + rItem.mnOldSplitSize );
                    }
                    else
                    {
                        aRect.SetTop( rItem.mnTop );
                        aRect.SetBottom( rItem.mnTop+rItem.mnOldHeight-1 );
                        aRect.SetLeft( rItem.mnOldSplitPos );
                        aRect.SetRight( aRect.Left() + rItem.mnOldSplitSize );
                    }
                    pWindow->Invalidate( aRect );
                    // invalidate new rectangle
                    if ( bRows )
                    {
                        aRect.SetLeft( rItem.mnLeft );
                        aRect.SetRight( rItem.mnLeft+rItem.mnWidth-1 );
                        aRect.SetTop( rItem.mnSplitPos );
                        aRect.SetBottom( aRect.Top() + rItem.mnSplitSize );
                    }
                    else
                    {
                        aRect.SetTop( rItem.mnTop );
                        aRect.SetBottom( rItem.mnTop+rItem.mnHeight-1 );
                        aRect.SetLeft( rItem.mnSplitPos );
                        aRect.SetRight( aRect.Left() + rItem.mnSplitSize );
                    }
                    pWindow->Invalidate( aRect );

                    // invalidate complete set, as these areas
                    // are not cluttered by windows
                    if ( rItem.mpSet && rItem.mpSet->mvItems.empty() )
                    {
                        aRect.SetLeft( rItem.mnLeft );
                        aRect.SetTop( rItem.mnTop );
                        aRect.SetRight( rItem.mnLeft+rItem.mnWidth-1 );
                        aRect.SetBottom( rItem.mnTop+rItem.mnHeight-1 );
                        pWindow->Invalidate( aRect );
                    }
                }
            }
        }
    }

    // position windows
    for ( auto& rItem : rItems )
    {
        if ( rItem.mpSet )
        {
            bool bTempHide = bHide;
            if ( !rItem.mnWidth || !rItem.mnHeight )
                bTempHide = true;
            ImplCalcSet2( pWindow, rItem.mpSet.get(), bTempHide,
                          !(rItem.mnBits & SplitWindowItemFlags::ColSet) );
        }
        else
        {
            if ( rItem.mnWidth && rItem.mnHeight && !bHide )
            {
                Point aPos( rItem.mnLeft, rItem.mnTop );
                Size  aSize( rItem.mnWidth, rItem.mnHeight );
                rItem.mpWindow->SetPosSizePixel( aPos, aSize );
            }
            else
                rItem.mpWindow->Hide();
        }
    }

    // show windows and reset flag
    for ( auto& rItem : rItems )
    {
        if ( rItem.mpWindow && rItem.mnWidth && rItem.mnHeight && !bHide )
            rItem.mpWindow->Show();
    }
}

static void ImplCalcLogSize( std::vector< ImplSplitItem > & rItems, size_t nItems )
{
    // update original sizes
    size_t  i;
    long    nRelSize = 0;
    long    nPerSize = 0;

    for ( i = 0; i < nItems; i++ )
    {
        if ( rItems[i].mnBits & SplitWindowItemFlags::RelativeSize )
            nRelSize += rItems[i].mnPixSize;
        else if ( rItems[i].mnBits & SplitWindowItemFlags::PercentSize )
            nPerSize += rItems[i].mnPixSize;
    }
    nPerSize += nRelSize;
    for ( i = 0; i < nItems; i++ )
    {
        if ( rItems[i].mnBits & SplitWindowItemFlags::RelativeSize )
        {
            if ( nRelSize )
                rItems[i].mnSize = (rItems[i].mnPixSize+(nRelSize/2))/nRelSize;
            else
                rItems[i].mnSize = 1;
        }
        else if ( rItems[i].mnBits & SplitWindowItemFlags::PercentSize )
        {
            if ( nPerSize )
                rItems[i].mnSize = (rItems[i].mnPixSize*100)/nPerSize;
            else
                rItems[i].mnSize = 1;
        }
        else
            rItems[i].mnSize = rItems[i].mnPixSize;
    }
}

void SplitWindow::ImplDrawBack(vcl::RenderContext& rRenderContext, ImplSplitSet* pSet)
{
    std::vector< ImplSplitItem >& rItems = pSet->mvItems;

    for ( auto& rItem : rItems )
    {
        pSet = rItem.mpSet.get();
        if (pSet && pSet->mpWallpaper)
        {
            Point aPoint(rItem.mnLeft, rItem.mnTop);
            Size aSize(rItem.mnWidth, rItem.mnHeight);
            tools::Rectangle aRect(aPoint, aSize);
            rRenderContext.DrawWallpaper(aRect, *pSet->mpWallpaper);
        }
    }

    for ( auto& rItem : rItems )
    {
        if (rItem.mpSet)
            ImplDrawBack(rRenderContext, rItem.mpSet.get());
    }
}

static void ImplDrawSplit(vcl::RenderContext& rRenderContext, ImplSplitSet* pSet, bool bRows, bool bDown)
{
    if (pSet->mvItems.empty())
        return;

    size_t     nItems = pSet->mvItems.size();
    long       nPos;
    long       nTop;
    long       nBottom;
    std::vector< ImplSplitItem >& rItems = pSet->mvItems;
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    for (size_t i = 0; i < nItems-1; i++)
    {
        if (rItems[i].mnSplitSize)
        {
            nPos = rItems[i].mnSplitPos;

            long nItemSplitSize = rItems[i].mnSplitSize;
            long nSplitSize = pSet->mnSplitSize;
            if (bRows)
            {
                nTop    = rItems[i].mnLeft;
                nBottom = rItems[i].mnLeft+rItems[i].mnWidth-1;

                if (bDown || (nItemSplitSize >= nSplitSize))
                {
                    rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
                    rRenderContext.DrawLine(Point(nTop, nPos + 1), Point(nBottom, nPos + 1));
                }
                nPos += nSplitSize-2;
                if ((!bDown && (nItemSplitSize >= 2)) ||
                    (bDown  && (nItemSplitSize >= nSplitSize - 1)))
                {
                    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                    rRenderContext.DrawLine(Point(nTop, nPos), Point(nBottom, nPos));
                }
                nPos++;
                if (!bDown || (nItemSplitSize >= nSplitSize))
                {
                    rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
                    rRenderContext.DrawLine(Point(nTop, nPos), Point(nBottom, nPos));
                }
            }
            else
            {
                nTop    = rItems[i].mnTop;
                nBottom = rItems[i].mnTop+pSet->mvItems[i].mnHeight-1;

                if (bDown || (nItemSplitSize >= nSplitSize))
                {
                    rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
                    rRenderContext.DrawLine(Point(nPos + 1, nTop), Point(nPos+1, nBottom));
                }
                nPos += pSet->mnSplitSize - 2;
                if ((!bDown && (nItemSplitSize >= 2)) ||
                    (bDown  && (nItemSplitSize >= nSplitSize - 1)))
                {
                    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
                    rRenderContext.DrawLine(Point(nPos, nTop), Point(nPos, nBottom));
                }
                nPos++;
                if (!bDown || (nItemSplitSize >= nSplitSize))
                {
                    rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
                    rRenderContext.DrawLine(Point(nPos, nTop), Point(nPos, nBottom));
                }
            }
        }
    }

    for ( auto& rItem : rItems )
    {
        if (rItem.mpSet && rItem.mnWidth && rItem.mnHeight)
        {
            ImplDrawSplit(rRenderContext, rItem.mpSet.get(), !(rItem.mnBits & SplitWindowItemFlags::ColSet), true/*bDown*/);
        }
    }
}

sal_uInt16 SplitWindow::ImplTestSplit( ImplSplitSet* pSet, const Point& rPos,
                                   long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos,
                                   bool bRows )
{
    if ( pSet->mvItems.empty() )
        return 0;

    sal_uInt16      nSplitTest;
    size_t          nItems = pSet->mvItems.size();
    long            nMPos1;
    long            nMPos2;
    long            nPos;
    long            nTop;
    long            nBottom;
    std::vector< ImplSplitItem >& rItems = pSet->mvItems;

    if ( bRows )
    {
        nMPos1 = rPos.X();
        nMPos2 = rPos.Y();
    }
    else
    {
        nMPos1 = rPos.Y();
        nMPos2 = rPos.X();
    }

    for ( size_t i = 0; i < nItems-1; i++ )
    {
        if ( rItems[i].mnSplitSize )
        {
            if ( bRows )
            {
                nTop    = rItems[i].mnLeft;
                nBottom = rItems[i].mnLeft+rItems[i].mnWidth-1;
            }
            else
            {
                nTop    = rItems[i].mnTop;
                nBottom = rItems[i].mnTop+rItems[i].mnHeight-1;
            }
            nPos = rItems[i].mnSplitPos;

            if ( (nMPos1 >= nTop) && (nMPos1 <= nBottom) &&
                 (nMPos2 >= nPos) && (nMPos2 <= nPos+rItems[i].mnSplitSize) )
            {
                if ( !rItems[i].mbFixed && !rItems[i+1].mbFixed )
                {
                    rMouseOff = nMPos2-nPos;
                    *ppFoundSet = pSet;
                    rFoundPos = i;
                    if ( bRows )
                        return SPLIT_VERT;
                    else
                        return SPLIT_HORZ;
                }
                else
                    return SPLIT_NOSPLIT;
            }
        }
    }

    for ( auto& rItem : rItems )
    {
        if ( rItem.mpSet )
        {
            nSplitTest = ImplTestSplit( rItem.mpSet.get(), rPos,
                                       rMouseOff, ppFoundSet, rFoundPos,
                                       !(rItem.mnBits & SplitWindowItemFlags::ColSet) );
            if ( nSplitTest )
                return nSplitTest;
        }
    }

    return 0;
}

sal_uInt16 SplitWindow::ImplTestSplit( const SplitWindow* pWindow, const Point& rPos,
                                   long& rMouseOff, ImplSplitSet** ppFoundSet, sal_uInt16& rFoundPos )
{
    // Resizable SplitWindow should be treated different
    if ( pWindow->mnWinStyle & WB_SIZEABLE )
    {
        long    nTPos;
        long    nPos;
        long    nBorder;

        if ( pWindow->mbHorz )
        {
            if ( pWindow->mbBottomRight )
            {
                nBorder = pWindow->mnBottomBorder;
                nPos = 0;
            }
            else
            {
                nBorder = pWindow->mnTopBorder;
                nPos = pWindow->mnDY-nBorder;
            }
            nTPos = rPos.Y();
        }
        else
        {
            if ( pWindow->mbBottomRight )
            {
                nBorder = pWindow->mnRightBorder;
                nPos = 0;
            }
            else
            {
                nBorder = pWindow->mnLeftBorder;
                nPos = pWindow->mnDX-nBorder;
            }
            nTPos = rPos.X();
        }
        long nSplitSize = pWindow->mpMainSet->mnSplitSize-2;
        if (pWindow->mbFadeOut)
            nSplitSize += SPLITWIN_SPLITSIZEEXLN;
        if ( !pWindow->mbBottomRight )
            nPos -= nSplitSize;
        if ( (nTPos >= nPos) && (nTPos <= nPos+nSplitSize+nBorder) )
        {
            rMouseOff = nTPos-nPos;
            *ppFoundSet = pWindow->mpMainSet.get();
            if ( !pWindow->mpMainSet->mvItems.empty() )
                rFoundPos = pWindow->mpMainSet->mvItems.size() - 1;
            else
                rFoundPos = 0;
            if ( pWindow->mbHorz )
                return SPLIT_VERT | SPLIT_WINDOW;
            else
                return SPLIT_HORZ | SPLIT_WINDOW;
        }
    }

    return ImplTestSplit( pWindow->mpMainSet.get(), rPos, rMouseOff, ppFoundSet, rFoundPos,
                         pWindow->mbHorz );
}

void SplitWindow::ImplDrawSplitTracking(const Point& rPos)
{
    tools::Rectangle aRect;

    if (mnSplitTest & SPLIT_HORZ)
    {
        aRect.SetTop( maDragRect.Top() );
        aRect.SetBottom( maDragRect.Bottom() );
        aRect.SetLeft( rPos.X() );
        aRect.SetRight( aRect.Left() + mpSplitSet->mnSplitSize - 1 );
        if (!(mnWinStyle & WB_NOSPLITDRAW))
            aRect.AdjustRight( -1 );
        if ((mnSplitTest & SPLIT_WINDOW) && mbFadeOut)
        {
            aRect.AdjustLeft(SPLITWIN_SPLITSIZEEXLN );
            aRect.AdjustRight(SPLITWIN_SPLITSIZEEXLN );
        }
    }
    else
    {
        aRect.SetLeft( maDragRect.Left() );
        aRect.SetRight( maDragRect.Right() );
        aRect.SetTop( rPos.Y() );
        aRect.SetBottom( aRect.Top() + mpSplitSet->mnSplitSize - 1 );
        if (!(mnWinStyle & WB_NOSPLITDRAW))
            aRect.AdjustBottom( -1 );
        if ((mnSplitTest & SPLIT_WINDOW) && mbFadeOut)
        {
            aRect.AdjustTop(SPLITWIN_SPLITSIZEEXLN );
            aRect.AdjustBottom(SPLITWIN_SPLITSIZEEXLN );
        }
    }
    ShowTracking(aRect, ShowTrackFlags::Split);
}

void SplitWindow::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    mpMainSet.reset(new ImplSplitSet());
    mpBaseSet               = mpMainSet.get();
    mpSplitSet              = nullptr;
    mpLastSizes             = nullptr;
    mnDX                    = 0;
    mnDY                    = 0;
    mnLeftBorder            = 0;
    mnTopBorder             = 0;
    mnRightBorder           = 0;
    mnBottomBorder          = 0;
    mnMaxSize               = 0;
    mnMouseOff              = 0;
    meAlign                 = WindowAlign::Top;
    mnWinStyle              = nStyle;
    mnSplitTest             = 0;
    mnSplitPos              = 0;
    mnMouseModifier         = 0;
    mnMStartPos             = 0;
    mnMSplitPos             = 0;
    mbDragFull              = false;
    mbHorz                  = true;
    mbBottomRight           = false;
    mbCalc                  = false;
    mbRecalc                = true;
    mbInvalidate            = true;
    mbFadeIn                = false;
    mbFadeOut               = false;
    mbFadeInDown            = false;
    mbFadeOutDown           = false;
    mbFadeInPressed         = false;
    mbFadeOutPressed        = false;
    mbFadeNoButtonMode      = false;

    if ( nStyle & WB_NOSPLITDRAW )
    {
        mpMainSet->mnSplitSize -= 2;
        mbInvalidate = false;
    }

    if ( nStyle & WB_BORDER )
    {
        ImplCalcBorder( meAlign, mnLeftBorder, mnTopBorder,
                        mnRightBorder, mnBottomBorder );
    }
    else
    {
        mnLeftBorder   = 0;
        mnTopBorder    = 0;
        mnRightBorder  = 0;
        mnBottomBorder = 0;
    }

    DockingWindow::ImplInit( pParent, (nStyle | WB_CLIPCHILDREN) & ~(WB_BORDER | WB_SIZEABLE) );

    ImplInitSettings();
}

void SplitWindow::ImplInitSettings()
{
    // If MainSet has a Wallpaper, this is the background,
    // otherwise it is the standard colour
    if ( mpMainSet->mpWallpaper )
        SetBackground( *mpMainSet->mpWallpaper );
    else
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else if ( Window::GetStyle() & WB_3DLOOK )
            aColor = rStyleSettings.GetFaceColor();
        else
            aColor = rStyleSettings.GetWindowColor();
        SetBackground( aColor );
    }
}

SplitWindow::SplitWindow( vcl::Window* pParent, WinBits nStyle ) :
    DockingWindow( WindowType::SPLITWINDOW )
{
    ImplInit( pParent, nStyle );
    DockingWindow::SetIdleDebugName( "vcl::SplitWindow maLayoutIdle" );
}

SplitWindow::~SplitWindow()
{
    disposeOnce();
}

void SplitWindow::dispose()
{
    // delete Sets
    mpMainSet.reset();
    DockingWindow::dispose();
}

void SplitWindow::ImplSetWindowSize( long nDelta )
{
    if ( !nDelta )
        return;

    Size aSize = GetSizePixel();
    switch ( meAlign )
    {
    case WindowAlign::Top:
        aSize.AdjustHeight(nDelta );
        SetSizePixel( aSize );
        break;
    case WindowAlign::Bottom:
    {
        maDragRect.AdjustTop(nDelta );
        Point aPos = GetPosPixel();
        aPos.AdjustY( -nDelta );
        aSize.AdjustHeight(nDelta );
        SetPosSizePixel( aPos, aSize );
        break;
    }
    case WindowAlign::Left:
        aSize.AdjustWidth(nDelta );
        SetSizePixel( aSize );
        break;
    case WindowAlign::Right:
    default:
    {
        maDragRect.AdjustLeft(nDelta );
        Point aPos = GetPosPixel();
        aPos.AdjustX( -nDelta );
        aSize.AdjustWidth(nDelta );
        SetPosSizePixel( aPos, aSize );
        break;
    }
    }

    SplitResize();
}

Size SplitWindow::CalcLayoutSizePixel( const Size& aNewSize )
{
    Size aSize( aNewSize );
    long nSplitSize = mpMainSet->mnSplitSize-2;

    if (mbFadeOut)
        nSplitSize += SPLITWIN_SPLITSIZEEXLN;

    // if the window is sizeable and if it does not contain a relative window,
    // the size is determined according to MainSet
    if ( mnWinStyle & WB_SIZEABLE )
    {
        long    nCalcSize = 0;
        std::vector< ImplSplitItem* >::size_type i;

        for ( i = 0; i < mpMainSet->mvItems.size(); i++ )
        {
            if ( mpMainSet->mvItems[i].mnBits & (SplitWindowItemFlags::RelativeSize | SplitWindowItemFlags::PercentSize) )
                break;
            else
                nCalcSize += mpMainSet->mvItems[i].mnSize;
        }

        if ( i == mpMainSet->mvItems.size() )
        {
            long    nDelta = 0;
            long    nCurSize;

            if ( mbHorz )
                nCurSize = aNewSize.Height()-mnTopBorder-mnBottomBorder;
            else
                nCurSize = aNewSize.Width()-mnLeftBorder-mnRightBorder;
            nCurSize -= nSplitSize;
            nCurSize -= (mpMainSet->mvItems.size()-1)*mpMainSet->mnSplitSize;

            nDelta = nCalcSize-nCurSize;
            if ( !nDelta )
                return aSize;

            switch ( meAlign )
            {
            case WindowAlign::Top:
                aSize.AdjustHeight(nDelta );
                break;
            case WindowAlign::Bottom:
                aSize.AdjustHeight(nDelta );
                break;
            case WindowAlign::Left:
                aSize.AdjustWidth(nDelta );
                break;
            case WindowAlign::Right:
            default:
                aSize.AdjustWidth(nDelta );
                break;
            }
        }
    }

    return aSize;
}

void SplitWindow::ImplCalcLayout()
{
    if ( !mbCalc || !mbRecalc || mpMainSet->mvItems.empty() )
        return;

    long nSplitSize = mpMainSet->mnSplitSize-2;
    if (mbFadeOut)
        nSplitSize += SPLITWIN_SPLITSIZEEXLN;

    // if the window is sizeable and if it does not contain a relative window,
    // the size is determined according to MainSet
    if ( mnWinStyle & WB_SIZEABLE )
    {
        long    nCalcSize = 0;
        std::vector<ImplSplitItem *>::size_type i;

        for ( i = 0; i < mpMainSet->mvItems.size(); i++ )
        {
            if ( mpMainSet->mvItems[i].mnBits & (SplitWindowItemFlags::RelativeSize | SplitWindowItemFlags::PercentSize) )
                break;
            else
                nCalcSize += mpMainSet->mvItems[i].mnSize;
        }

        if ( i == mpMainSet->mvItems.size() )
        {
            long    nCurSize;
            if ( mbHorz )
                nCurSize = mnDY-mnTopBorder-mnBottomBorder;
            else
                nCurSize = mnDX-mnLeftBorder-mnRightBorder;
            nCurSize -= nSplitSize;
            nCurSize -= (mpMainSet->mvItems.size()-1)*mpMainSet->mnSplitSize;

            mbRecalc = false;
            ImplSetWindowSize( nCalcSize-nCurSize );
            mbRecalc = true;
        }
    }

    if ( (mnDX <= 0) || (mnDY <= 0) )
        return;

    // pre-calculate sizes/position
    long    nL;
    long    nT;
    long    nW;
    long    nH;

    if ( mbHorz )
    {
        if ( mbBottomRight )
            nT = mnDY-mnBottomBorder;
        else
            nT = mnTopBorder;
        nL = mnLeftBorder;
    }
    else
    {
        if ( mbBottomRight )
            nL = mnDX-mnRightBorder;
        else
            nL = mnLeftBorder;
        nT = mnTopBorder;
    }
    nW = mnDX-mnLeftBorder-mnRightBorder;
    nH = mnDY-mnTopBorder-mnBottomBorder;
    if ( mnWinStyle & WB_SIZEABLE )
    {
        if ( mbHorz )
            nH -= nSplitSize;
        else
            nW -= nSplitSize;
    }

    // calculate sets recursive
    ImplCalcSet( mpMainSet.get(), nL, nT, nW, nH, mbHorz, !mbBottomRight );
    ImplCalcSet2( this, mpMainSet.get(), false, mbHorz );
    mbCalc = false;
}

void SplitWindow::ImplUpdate()
{
    mbCalc = true;

    if ( IsReallyShown() && IsUpdateMode() && mbRecalc )
    {
        if ( !mpMainSet->mvItems.empty() )
            ImplCalcLayout();
        else
            Invalidate();
    }
}

void SplitWindow::ImplSplitMousePos( Point& rMousePos )
{
    if ( mnSplitTest & SPLIT_HORZ )
    {
        rMousePos.AdjustX( -mnMouseOff );
        if ( rMousePos.X() < maDragRect.Left() )
            rMousePos.setX( maDragRect.Left() );
        else if ( rMousePos.X()+mpSplitSet->mnSplitSize+1 > maDragRect.Right() )
            rMousePos.setX( maDragRect.Right()-mpSplitSet->mnSplitSize+1 );
        // store in screen coordinates due to FullDrag
        mnMSplitPos = OutputToScreenPixel( rMousePos ).X();
    }
    else
    {
        rMousePos.AdjustY( -mnMouseOff );
        if ( rMousePos.Y() < maDragRect.Top() )
            rMousePos.setY( maDragRect.Top() );
        else if ( rMousePos.Y()+mpSplitSet->mnSplitSize+1 > maDragRect.Bottom() )
            rMousePos.setY( maDragRect.Bottom()-mpSplitSet->mnSplitSize+1 );
        mnMSplitPos = OutputToScreenPixel( rMousePos ).Y();
    }
}

void SplitWindow::ImplGetButtonRect( tools::Rectangle& rRect, bool bTest ) const
{
    long nSplitSize = mpMainSet->mnSplitSize-1;
    if (mbFadeOut || mbFadeIn)
        nSplitSize += SPLITWIN_SPLITSIZEEX;

    long nButtonSize = 0;
    if ( mbFadeIn )
        nButtonSize += SPLITWIN_SPLITSIZEFADE+1;
    if ( mbFadeOut )
        nButtonSize += SPLITWIN_SPLITSIZEFADE+1;
    long nCenterEx = 0;
    if ( mbHorz )
        nCenterEx += ((mnDX-mnLeftBorder-mnRightBorder)-nButtonSize)/2;
    else
        nCenterEx += ((mnDY-mnTopBorder-mnBottomBorder)-nButtonSize)/2;
    long nEx = 0;
    if ( nCenterEx > 0 )
        nEx += nCenterEx;

    switch ( meAlign )
    {
    case WindowAlign::Top:
        rRect.SetLeft( mnLeftBorder+nEx );
        rRect.SetTop( mnDY-mnBottomBorder-nSplitSize );
        rRect.SetRight( rRect.Left()+SPLITWIN_SPLITSIZEAUTOHIDE );
        rRect.SetBottom( mnDY-mnBottomBorder-1 );
        if ( bTest )
        {
            rRect.AdjustTop( -mnTopBorder );
            rRect.AdjustBottom(mnBottomBorder );
        }
        break;
    case WindowAlign::Bottom:
        rRect.SetLeft( mnLeftBorder+nEx );
        rRect.SetTop( mnTopBorder );
        rRect.SetRight( rRect.Left()+SPLITWIN_SPLITSIZEAUTOHIDE );
        rRect.SetBottom( mnTopBorder+nSplitSize-1 );
        if ( bTest )
        {
            rRect.AdjustTop( -mnTopBorder );
            rRect.AdjustBottom(mnBottomBorder );
        }
        break;
    case WindowAlign::Left:
        rRect.SetLeft( mnDX-mnRightBorder-nSplitSize );
        rRect.SetTop( mnTopBorder+nEx );
        rRect.SetRight( mnDX-mnRightBorder-1 );
        rRect.SetBottom( rRect.Top()+SPLITWIN_SPLITSIZEAUTOHIDE );
        if ( bTest )
        {
            rRect.AdjustLeft( -mnLeftBorder );
            rRect.AdjustRight(mnRightBorder );
        }
        break;
    case WindowAlign::Right:
        rRect.SetLeft( mnLeftBorder );
        rRect.SetTop( mnTopBorder+nEx );
        rRect.SetRight( mnLeftBorder+nSplitSize-1 );
        rRect.SetBottom( rRect.Top()+SPLITWIN_SPLITSIZEAUTOHIDE );
        if ( bTest )
        {
            rRect.AdjustLeft( -mnLeftBorder );
            rRect.AdjustRight(mnRightBorder );
        }
        break;
    }
}

void SplitWindow::ImplGetFadeInRect( tools::Rectangle& rRect, bool bTest ) const
{
    tools::Rectangle aRect;

    if ( mbFadeIn )
        ImplGetButtonRect( aRect, bTest );

    rRect = aRect;
}

void SplitWindow::ImplGetFadeOutRect( tools::Rectangle& rRect ) const
{
    tools::Rectangle aRect;

    if ( mbFadeOut )
        ImplGetButtonRect( aRect, false );

    rRect = aRect;
}

void SplitWindow::ImplDrawGrip(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect, bool bHorizontal, bool bLeft)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    Color aColor;

    if (rRect.IsInside(GetPointerPosPixel()))
    {
        vcl::RenderTools::DrawSelectionBackground(rRenderContext, *this, rRect, 2, false, false, false);

        aColor = rStyleSettings.GetDarkShadowColor();
    }
    else
    {
        rRenderContext.SetLineColor(rStyleSettings.GetDarkShadowColor());
        rRenderContext.SetFillColor(rStyleSettings.GetDarkShadowColor());

        rRenderContext.DrawRect(rRect);

        aColor = rStyleSettings.GetFaceColor();
    }

    AntialiasingFlags nAA = rRenderContext.GetAntialiasing();
    rRenderContext.SetAntialiasing(nAA | AntialiasingFlags::PixelSnapHairline | AntialiasingFlags::EnableB2dDraw);

    long nWidth = rRect.getWidth();
    long nWidthHalf = nWidth / 2;
    long nHeight = rRect.getHeight();
    long nHeightHalf = nHeight / 2;

    long nLeft = rRect.Left();
    long nRight = rRect.Right();
    long nTop = rRect.Top();
    long nBottom = rRect.Bottom();
    long nMargin = 1;

    rRenderContext.SetLineColor(aColor);
    rRenderContext.SetFillColor(aColor);

    tools::Polygon aPoly(3);

    if (bHorizontal)
    {
        long nCenter = nLeft + nWidthHalf;

        if (bLeft)
        {
            aPoly.SetPoint(Point(nCenter,               nTop    + nMargin), 0);
            aPoly.SetPoint(Point(nCenter - nHeightHalf, nBottom - nMargin), 1);
            aPoly.SetPoint(Point(nCenter - nHeightHalf, nBottom - nMargin), 2);
        }
        else
        {
            aPoly.SetPoint(Point(nCenter,               nBottom - nMargin), 0);
            aPoly.SetPoint(Point(nCenter - nHeightHalf, nTop    + nMargin), 1);
            aPoly.SetPoint(Point(nCenter + nHeightHalf, nTop    + nMargin), 2);
        }
        rRenderContext.DrawPolygon(aPoly);
    }
    else
    {
        long nCenter = nTop + nHeightHalf;

        if (bLeft)
        {
            aPoly.SetPoint(Point(nLeft  + nMargin, nCenter), 0);
            aPoly.SetPoint(Point(nRight - nMargin, nCenter - nWidthHalf), 1);
            aPoly.SetPoint(Point(nRight - nMargin, nCenter + nWidthHalf), 2);
        }
        else
        {
            aPoly.SetPoint(Point(nRight - nMargin, nCenter), 0);
            aPoly.SetPoint(Point(nLeft  + nMargin, nCenter - nWidthHalf), 1);
            aPoly.SetPoint(Point(nLeft  + nMargin, nCenter + nWidthHalf), 2);
        }
        rRenderContext.DrawPolygon(aPoly);
    }

    rRenderContext.SetAntialiasing(nAA);
}

void SplitWindow::ImplDrawFadeIn(vcl::RenderContext& rRenderContext)
{
    if (mbFadeIn)
    {
        tools::Rectangle aTempRect;
        ImplGetFadeInRect(aTempRect);

        bool bLeft = true;
        switch (meAlign)
        {
        case WindowAlign::Top:
        case WindowAlign::Left:
            bLeft = false;
            break;
        case WindowAlign::Bottom:
        case WindowAlign::Right:
        default:
            bLeft = true;
            break;
        }

        ImplDrawGrip(rRenderContext, aTempRect, (meAlign == WindowAlign::Top) || (meAlign == WindowAlign::Bottom), bLeft);
    }
}

void SplitWindow::ImplDrawFadeOut(vcl::RenderContext& rRenderContext)
{
    if (mbFadeOut)
    {
        tools::Rectangle aTempRect;
        ImplGetFadeOutRect(aTempRect);

        bool bLeft = true;
        switch (meAlign)
        {
        case WindowAlign::Bottom:
        case WindowAlign::Right:
            bLeft = false;
            break;
        case WindowAlign::Top:
        case WindowAlign::Left:
        default:
            bLeft = true;
            break;
        }

        ImplDrawGrip(rRenderContext, aTempRect, (meAlign == WindowAlign::Top) || (meAlign == WindowAlign::Bottom), bLeft);
    }
}

void SplitWindow::ImplStartSplit( const MouseEvent& rMEvt )
{
    Point aMousePosPixel = rMEvt.GetPosPixel();
    mnSplitTest = ImplTestSplit( this, aMousePosPixel, mnMouseOff, &mpSplitSet, mnSplitPos );

    if ( !mnSplitTest || (mnSplitTest & SPLIT_NOSPLIT) )
        return;

    ImplSplitItem*  pSplitItem;
    long            nCurMaxSize;
    bool            bPropSmaller;

    mnMouseModifier = rMEvt.GetModifier();
    bPropSmaller = (mnMouseModifier & KEY_SHIFT) && (static_cast<sal_uInt16>(mnSplitPos+1) < mpSplitSet->mvItems.size());

    // here we can set the maximum size
    StartSplit();

    if ( mnMaxSize )
        nCurMaxSize = mnMaxSize;
    else
    {
        Size aSize = GetParent()->GetOutputSizePixel();
        if ( mbHorz )
            nCurMaxSize = aSize.Height();
        else
            nCurMaxSize = aSize.Width();
    }

    if ( !mpSplitSet->mvItems.empty() )
    {
        bool bDown = true;
        if ( (mpSplitSet == mpMainSet.get()) && mbBottomRight )
            bDown = false;

        pSplitItem          = &mpSplitSet->mvItems[mnSplitPos];
        maDragRect.SetLeft( pSplitItem->mnLeft );
        maDragRect.SetTop( pSplitItem->mnTop );
        maDragRect.SetRight( pSplitItem->mnLeft+pSplitItem->mnWidth-1 );
        maDragRect.SetBottom( pSplitItem->mnTop+pSplitItem->mnHeight-1 );

        if ( mnSplitTest & SPLIT_HORZ )
        {
            if ( bDown )
                maDragRect.AdjustRight(mpSplitSet->mnSplitSize );
            else
                maDragRect.AdjustLeft( -(mpSplitSet->mnSplitSize) );
        }
        else
        {
            if ( bDown )
                maDragRect.AdjustBottom(mpSplitSet->mnSplitSize );
            else
                maDragRect.AdjustTop( -(mpSplitSet->mnSplitSize) );
        }

        if ( mnSplitPos )
        {
            long nTemp = mnSplitPos;
            while ( nTemp )
            {
                pSplitItem = &mpSplitSet->mvItems[nTemp-1];
                if ( pSplitItem->mbFixed )
                    break;
                else
                {
                    if ( mnSplitTest & SPLIT_HORZ )
                    {
                        if ( bDown )
                            maDragRect.AdjustLeft( -(pSplitItem->mnPixSize) );
                        else
                            maDragRect.AdjustRight(pSplitItem->mnPixSize );
                    }
                    else
                    {
                        if ( bDown )
                            maDragRect.AdjustTop( -(pSplitItem->mnPixSize) );
                        else
                            maDragRect.AdjustBottom(pSplitItem->mnPixSize );
                    }
                }
                nTemp--;
            }
        }

        if ( (mpSplitSet == mpMainSet.get()) && (mnWinStyle & WB_SIZEABLE) && !bPropSmaller )
        {
            if ( bDown )
            {
                if ( mbHorz )
                    maDragRect.AdjustBottom(nCurMaxSize-mnDY-mnTopBorder );
                else
                    maDragRect.AdjustRight(nCurMaxSize-mnDX-mnLeftBorder );
            }
            else
            {
                if ( mbHorz )
                    maDragRect.AdjustTop( -(nCurMaxSize-mnDY-mnBottomBorder) );
                else
                    maDragRect.AdjustLeft( -(nCurMaxSize-mnDX-mnRightBorder) );
            }
        }
        else
        {
            std::vector<ImplSplitItem *>::size_type nTemp = mnSplitPos+1;
            while ( nTemp < mpSplitSet->mvItems.size() )
            {
                pSplitItem = &mpSplitSet->mvItems[nTemp];
                if ( pSplitItem->mbFixed )
                    break;
                else
                {
                    if ( mnSplitTest & SPLIT_HORZ )
                    {
                        if ( bDown )
                            maDragRect.AdjustRight(pSplitItem->mnPixSize );
                        else
                            maDragRect.AdjustLeft( -(pSplitItem->mnPixSize) );
                    }
                    else
                    {
                        if ( bDown )
                            maDragRect.AdjustBottom(pSplitItem->mnPixSize );
                        else
                            maDragRect.AdjustTop( -(pSplitItem->mnPixSize) );
                    }
                }
                nTemp++;
            }
        }
    }
    else
    {
        maDragRect.SetLeft( mnLeftBorder );
        maDragRect.SetTop( mnTopBorder );
        maDragRect.SetRight( mnDX-mnRightBorder-1 );
        maDragRect.SetBottom( mnDY-mnBottomBorder-1 );
        if ( mbHorz )
        {
            if ( mbBottomRight )
                maDragRect.AdjustTop( -(nCurMaxSize-mnDY-mnBottomBorder) );
            else
                maDragRect.AdjustBottom(nCurMaxSize-mnDY-mnTopBorder );
        }
        else
        {
            if ( mbBottomRight )
                maDragRect.AdjustLeft( -(nCurMaxSize-mnDX-mnRightBorder) );
            else
                maDragRect.AdjustRight(nCurMaxSize-mnDX-mnLeftBorder );
        }
    }

    StartTracking();

    mbDragFull = bool(GetSettings().GetStyleSettings().GetDragFullOptions() & DragFullOptions::Split);

    ImplSplitMousePos( aMousePosPixel );

    if (!mbDragFull)
    {
        ImplDrawSplitTracking(aMousePosPixel);
    }
    else
    {
        std::vector< ImplSplitItem >&  rItems = mpSplitSet->mvItems;
        sal_uInt16       nItems = mpSplitSet->mvItems.size();
        mpLastSizes = new long[nItems*2];
        for ( sal_uInt16 i = 0; i < nItems; i++ )
        {
            mpLastSizes[i*2]   = rItems[i].mnSize;
            mpLastSizes[i*2+1] = rItems[i].mnPixSize;
        }
    }
    mnMStartPos = mnMSplitPos;

    PointerStyle eStyle = PointerStyle::Arrow;
    if ( mnSplitTest & SPLIT_HORZ )
        eStyle = PointerStyle::HSplit;
    else if ( mnSplitTest & SPLIT_VERT )
        eStyle = PointerStyle::VSplit;

    SetPointer( eStyle );
}

void SplitWindow::StartSplit()
{
}

void SplitWindow::Split()
{
    maSplitHdl.Call( this );
}

void SplitWindow::SplitResize()
{
}

void SplitWindow::FadeIn()
{
}

void SplitWindow::FadeOut()
{
}

void SplitWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() || rMEvt.IsMod2() )
    {
        DockingWindow::MouseButtonDown( rMEvt );
        return;
    }

    Point           aMousePosPixel = rMEvt.GetPosPixel();
    tools::Rectangle       aTestRect;

    mbFadeNoButtonMode = false;

    ImplGetFadeOutRect( aTestRect );
    if ( aTestRect.IsInside( aMousePosPixel ) )
    {
        mbFadeOutDown = true;
        mbFadeOutPressed = true;
        Invalidate();
    }
    else
    {
        ImplGetFadeInRect( aTestRect, true );
        if ( aTestRect.IsInside( aMousePosPixel ) )
        {
            mbFadeInDown = true;
            mbFadeInPressed = true;
            Invalidate();
        }
        else if ( !aTestRect.IsEmpty() && !(mnWinStyle & WB_SIZEABLE) )
        {
            mbFadeNoButtonMode = true;
            FadeIn();
            return;
        }
    }

    if ( mbFadeInDown || mbFadeOutDown )
        StartTracking();
    else
        ImplStartSplit( rMEvt );
}

void SplitWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( IsTracking() )
        return;

    Point           aPos = rMEvt.GetPosPixel();
    long            nTemp;
    ImplSplitSet*   pTempSplitSet;
    sal_uInt16          nTempSplitPos;
    sal_uInt16          nSplitTest = ImplTestSplit( this, aPos, nTemp, &pTempSplitSet, nTempSplitPos );
    PointerStyle    eStyle = PointerStyle::Arrow;
    tools::Rectangle       aFadeInRect;
    tools::Rectangle       aFadeOutRect;

    ImplGetFadeInRect( aFadeInRect );
    ImplGetFadeOutRect( aFadeOutRect );
    if ( !aFadeInRect.IsInside( aPos ) &&
         !aFadeOutRect.IsInside( aPos ) )
    {
        if ( nSplitTest && !(nSplitTest & SPLIT_NOSPLIT) )
        {
            if ( nSplitTest & SPLIT_HORZ )
                eStyle = PointerStyle::HSplit;
            else if ( nSplitTest & SPLIT_VERT )
                eStyle = PointerStyle::VSplit;
        }
    }

    SetPointer( eStyle );
}

void SplitWindow::Tracking( const TrackingEvent& rTEvt )
{
    Point aMousePosPixel = rTEvt.GetMouseEvent().GetPosPixel();

    if ( mbFadeInDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbFadeInDown = false;
            if ( mbFadeInPressed )
            {
                mbFadeInPressed = false;
                Invalidate();

                if ( !rTEvt.IsTrackingCanceled() )
                    FadeIn();
            }
        }
        else
        {
            tools::Rectangle aTestRect;
            ImplGetFadeInRect( aTestRect, true );
            bool bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( bNewPressed != mbFadeInPressed )
            {
                mbFadeInPressed = bNewPressed;
                Invalidate();
            }
        }
    }
    else if ( mbFadeOutDown )
    {
        if ( rTEvt.IsTrackingEnded() )
        {
            mbFadeOutDown = false;
            if ( mbFadeOutPressed )
            {
                mbFadeOutPressed = false;
                Invalidate();

                if ( !rTEvt.IsTrackingCanceled() )
                    FadeOut();
            }
        }
        else
        {
            tools::Rectangle aTestRect;
            ImplGetFadeOutRect( aTestRect );
            bool bNewPressed = aTestRect.IsInside( aMousePosPixel );
            if ( !bNewPressed )
            {
                mbFadeOutPressed = bNewPressed;
                Invalidate();

                // We need a mouseevent with a position inside the button for the
                // ImplStartSplit function!
                MouseEvent aOrgMEvt = rTEvt.GetMouseEvent();
                MouseEvent aNewMEvt = MouseEvent( aTestRect.Center(), aOrgMEvt.GetClicks(),
                                                  aOrgMEvt.GetMode(), aOrgMEvt.GetButtons(),
                                                  aOrgMEvt.GetModifier() );

                ImplStartSplit( aNewMEvt );
                mbFadeOutDown = false;
            }
        }
    }
    else
    {
        ImplSplitMousePos( aMousePosPixel );
        bool bSplit = true;
        if ( mbDragFull )
        {
            if ( rTEvt.IsTrackingEnded() )
            {
                if ( rTEvt.IsTrackingCanceled() )
                {
                    std::vector< ImplSplitItem >& rItems = mpSplitSet->mvItems;
                    size_t          nItems = rItems.size();
                    for ( size_t i = 0; i < nItems; i++ )
                    {
                        rItems[i].mnSize     = mpLastSizes[i*2];
                        rItems[i].mnPixSize  = mpLastSizes[i*2+1];
                    }
                    ImplUpdate();
                    Split();
                }
                bSplit = false;
            }
        }
        else
        {
            if ( rTEvt.IsTrackingEnded() )
            {
                HideTracking();
                bSplit = !rTEvt.IsTrackingCanceled();
            }
            else
            {
                ImplDrawSplitTracking(aMousePosPixel);
                bSplit = false;
            }
        }

        if ( bSplit )
        {
            bool    bPropSmaller = (mnMouseModifier & KEY_SHIFT) != 0;
            bool    bPropGreater = (mnMouseModifier & KEY_MOD1) != 0;
            long    nDelta = mnMSplitPos-mnMStartPos;

            if ( (mnSplitTest & SPLIT_WINDOW) && mpMainSet->mvItems.empty() )
            {
                if ( (mpSplitSet == mpMainSet.get()) && mbBottomRight )
                    nDelta *= -1;
                ImplSetWindowSize( nDelta );
            }
            else
            {
                long nNewSize = mpSplitSet->mvItems[mnSplitPos].mnPixSize;
                if ( (mpSplitSet == mpMainSet.get()) && mbBottomRight )
                    nNewSize -= nDelta;
                else
                    nNewSize += nDelta;
                SplitItem( mpSplitSet->mvItems[mnSplitPos].mnId, nNewSize,
                           bPropSmaller, bPropGreater );
            }

            Split();

            if ( mbDragFull )
            {
                Update();
                mnMStartPos = mnMSplitPos;
            }
        }

        if ( rTEvt.IsTrackingEnded() )
        {
            delete [] mpLastSizes;
            mpLastSizes     = nullptr;
            mpSplitSet      = nullptr;
            mnMouseOff      = 0;
            mnMStartPos     = 0;
            mnMSplitPos     = 0;
            mnMouseModifier = 0;
            mnSplitTest     = 0;
            mnSplitPos      = 0;
        }
    }
}

bool SplitWindow::PreNotify( NotifyEvent& rNEvt )
{
    const MouseEvent* pMouseEvt = nullptr;

    if( (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE) && (pMouseEvt = rNEvt.GetMouseEvent()) != nullptr )
    {
        if( !pMouseEvt->GetButtons() && !pMouseEvt->IsSynthetic() && !pMouseEvt->IsModifierChanged() )
        {
            // trigger redraw if mouse over state has changed
            tools::Rectangle aFadeInRect;
            tools::Rectangle aFadeOutRect;
            ImplGetFadeInRect( aFadeInRect );
            ImplGetFadeOutRect( aFadeOutRect );

            if ( aFadeInRect.IsInside( GetPointerPosPixel() ) != aFadeInRect.IsInside( GetLastPointerPosPixel() ) )
                Invalidate( aFadeInRect );
            if ( aFadeOutRect.IsInside( GetPointerPosPixel() ) != aFadeOutRect.IsInside( GetLastPointerPosPixel() ) )
                Invalidate( aFadeOutRect );

            if( pMouseEvt->IsLeaveWindow() || pMouseEvt->IsEnterWindow() )
            {
                Invalidate( aFadeInRect );
                Invalidate( aFadeOutRect );
            }
        }
    }
    return Window::PreNotify( rNEvt );
}

void SplitWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (mnWinStyle & WB_BORDER)
        ImplDrawBorder(rRenderContext);

    ImplDrawBorderLine(rRenderContext);
    ImplDrawFadeOut(rRenderContext);
    ImplDrawFadeIn(rRenderContext);

    // draw FrameSet-backgrounds
    ImplDrawBack(rRenderContext, mpMainSet.get());

    // draw splitter
    if (!(mnWinStyle & WB_NOSPLITDRAW))
    {
        ImplDrawSplit(rRenderContext, mpMainSet.get(), mbHorz, !mbBottomRight);
    }
}

void SplitWindow::Resize()
{
    Size aSize = GetOutputSizePixel();
    mnDX = aSize.Width();
    mnDY = aSize.Height();

    ImplUpdate();
    Invalidate();
}

void SplitWindow::RequestHelp( const HelpEvent& rHEvt )
{
    // no keyboard help for splitwin
    if ( rHEvt.GetMode() & (HelpEventMode::BALLOON | HelpEventMode::QUICK) && !rHEvt.KeyboardActivated() )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        tools::Rectangle   aHelpRect;
        const char* pHelpResId = nullptr;

        ImplGetFadeInRect( aHelpRect, true );
        if ( aHelpRect.IsInside( aMousePosPixel ) )
            pHelpResId = SV_HELPTEXT_FADEIN;
        else
        {
            ImplGetFadeOutRect( aHelpRect );
            if ( aHelpRect.IsInside( aMousePosPixel ) )
                pHelpResId = SV_HELPTEXT_FADEOUT;
        }

        // get rectangle
        if (pHelpResId)
        {
            Point aPt = OutputToScreenPixel( aHelpRect.TopLeft() );
            aHelpRect.SetLeft( aPt.X() );
            aHelpRect.SetTop( aPt.Y() );
            aPt = OutputToScreenPixel( aHelpRect.BottomRight() );
            aHelpRect.SetRight( aPt.X() );
            aHelpRect.SetBottom( aPt.Y() );

            // get and draw text
            OUString aStr = VclResId(pHelpResId);
            if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                Help::ShowBalloon( this, aHelpRect.Center(), aHelpRect, aStr );
            else
                Help::ShowQuickHelp( this, aHelpRect, aStr );
            return;
        }
    }

    DockingWindow::RequestHelp( rHEvt );
}

void SplitWindow::StateChanged( StateChangedType nType )
{
    switch ( nType )
    {
    case StateChangedType::InitShow:
        if ( IsUpdateMode() )
            ImplCalcLayout();
        break;
    case StateChangedType::UpdateMode:
        if ( IsUpdateMode() && IsReallyShown() )
            ImplCalcLayout();
        break;
    case StateChangedType::ControlBackground:
        ImplInitSettings();
        Invalidate();
        break;
    default:;
    }

    DockingWindow::StateChanged( nType );
}

void SplitWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
    else
        DockingWindow::DataChanged( rDCEvt );
}

void SplitWindow::InsertItem( sal_uInt16 nId, vcl::Window* pWindow, long nSize,
                              sal_uInt16 nPos, sal_uInt16 nIntoSetId,
                              SplitWindowItemFlags nBits )
{
#ifdef DBG_UTIL
    sal_uInt16 nDbgDummy;
    SAL_WARN_IF( ImplFindItem( mpMainSet.get(), nId, nDbgDummy ), "vcl", "SplitWindow::InsertItem() - Id already exists" );
#endif

    // Size has to be at least 1.
    if ( nSize < 1 )
        nSize = 1;

    ImplSplitSet* pSet       = ImplFindSet( mpMainSet.get(), nIntoSetId );
#ifdef DBG_UTIL
    SAL_WARN_IF( !pSet, "vcl", "SplitWindow::InsertItem() - Set not exists" );
#endif
    if(!pSet)
    {
        return;
    }

    // Don't insert further than the end
    if ( nPos > pSet->mvItems.size() )
        nPos = pSet->mvItems.size();

    // Insert in set
    pSet->mvItems.emplace( pSet->mvItems.begin() + nPos );

    // init new item
    ImplSplitItem & aItem = pSet->mvItems[nPos];
    aItem.mnSize   = nSize;
    aItem.mnPixSize = 0;
    aItem.mnId     = nId;
    aItem.mnBits   = nBits;
    aItem.mnMinSize=-1;
    aItem.mnMaxSize=-1;

    if ( pWindow )
    {
        // New VclPtr reference
        aItem.mpWindow         = pWindow;
        aItem.mpOrgParent      = pWindow->GetParent();

        // Attach window to SplitWindow.
        pWindow->Hide();
        pWindow->SetParent( this );
    }
    else
    {
        ImplSplitSet * pNewSet  = new ImplSplitSet();
        pNewSet->mnId           = nId;
        pNewSet->mnSplitSize    = pSet->mnSplitSize;

        aItem.mpSet.reset(pNewSet);
    }

    pSet->mbCalcPix = true;

    ImplUpdate();
}

void SplitWindow::InsertItem( sal_uInt16 nId, long nSize,
                              sal_uInt16 nPos, sal_uInt16 nIntoSetId,
                              SplitWindowItemFlags nBits )
{
    InsertItem( nId, nullptr, nSize, nPos, nIntoSetId, nBits );
}

void SplitWindow::RemoveItem( sal_uInt16 nId )
{
#ifdef DBG_UTIL
    sal_uInt16 nDbgDummy;
    SAL_WARN_IF( !ImplFindItem( mpMainSet.get(), nId, nDbgDummy ), "vcl", "SplitWindow::RemoveItem() - Id not found" );
#endif

    // search set
    sal_uInt16     nPos;
    ImplSplitSet*  pSet    = ImplFindItem( mpMainSet.get(), nId, nPos );

    if (!pSet)
        return;

    ImplSplitItem* pItem = &pSet->mvItems[nPos];
    VclPtr<vcl::Window> pWindow = pItem->mpWindow;
    VclPtr<vcl::Window> pOrgParent = pItem->mpOrgParent;

    // delete set if required
    if ( !pWindow )
        pItem->mpSet.reset();

    // remove item
    pSet->mbCalcPix = true;
    pSet->mvItems.erase( pSet->mvItems.begin() + nPos );

    ImplUpdate();

    // to have the least amounts of paints delete window only here
    if ( pWindow )
    {
        // restore window
        pWindow->Hide();
        pWindow->SetParent( pOrgParent );
    }

    // Clear and delete
    pWindow.clear();
    pOrgParent.clear();
}

void SplitWindow::Clear()
{
    // create Main-Set again
    mpMainSet.reset(new ImplSplitSet());
    if ( mnWinStyle & WB_NOSPLITDRAW )
        mpMainSet->mnSplitSize -= 2;
    mpBaseSet = mpMainSet.get();

    // and invalidate again
    ImplUpdate();
}

void SplitWindow::SplitItem( sal_uInt16 nId, long nNewSize,
                             bool bPropSmall, bool bPropGreat )
{
    sal_uInt16      nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if (!pSet)
        return;

    size_t           nItems = pSet->mvItems.size();
    std::vector< ImplSplitItem >&  rItems = pSet->mvItems;

    // When there is an explicit minimum or maximum size then move nNewSize
    // into that range (when it is not yet already in it.)
    nNewSize = ValidateSize(nNewSize, rItems[nPos]);

    if ( mbCalc )
    {
        rItems[nPos].mnSize = nNewSize;
        return;
    }

    long nDelta = nNewSize-rItems[nPos].mnPixSize;
    if ( !nDelta )
        return;

    // calculate area, which could be affected by splitting
    sal_uInt16 nMin = 0;
    sal_uInt16 nMax = nItems;
    for (size_t i = 0; i < nItems; ++i)
    {
        if ( rItems[i].mbFixed )
        {
            if ( i < nPos )
                nMin = i+1;
            else
                nMax = i;
        }
    }

    // treat TopSet different if the window is sizeable
    bool bSmall  = true;
    bool bGreat  = true;
    if ( (pSet == mpMainSet.get()) && (mnWinStyle & WB_SIZEABLE) )
    {
        if ( nPos < pSet->mvItems.size()-1 )
        {
            if ( !((bPropSmall && bPropGreat) ||
                   ((nDelta > 0) && bPropSmall) ||
                   ((nDelta < 0) && bPropGreat)) )
            {
                if ( nDelta < 0 )
                    bGreat = false;
                else
                    bSmall = false;
            }
        }
        else
        {
            if ( nDelta < 0 )
                bGreat = false;
            else
                bSmall = false;
        }
    }
    else if ( nPos >= nMax )
    {
        bSmall = false;
        bGreat = false;
    }
    else if ( nPos && (nPos >= pSet->mvItems.size()-1) )
    {
        nPos--;
        nDelta *= -1;
        bool bTemp = bPropSmall;
        bPropSmall = bPropGreat;
        bPropGreat = bTemp;
    }

    sal_uInt16          n;
    // now splitt the windows
    if ( nDelta < 0 )
    {
        if ( bGreat )
        {
            if ( bPropGreat )
            {
                long nTempDelta = nDelta;
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nTempDelta )
                        {
                            rItems[n].mnPixSize++;
                            nTempDelta++;
                        }
                        n++;
                    }
                    while ( n < nMax );
                }
                while ( nTempDelta );
            }
            else
                rItems[nPos+1].mnPixSize -= nDelta;
        }

        if ( bSmall )
        {
            if ( bPropSmall )
            {
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nDelta && rItems[n-1].mnPixSize )
                        {
                            rItems[n-1].mnPixSize--;
                            nDelta++;
                        }

                        n--;
                    }
                    while ( n > nMin );
                }
                while ( nDelta );
            }
            else
            {
                n = nPos+1;
                do
                {
                    if ( rItems[n-1].mnPixSize+nDelta < 0 )
                    {
                        nDelta += rItems[n-1].mnPixSize;
                        rItems[n-1].mnPixSize = 0;
                    }
                    else
                    {
                        rItems[n-1].mnPixSize += nDelta;
                        break;
                    }
                    n--;
                }
                while ( n > nMin );
            }
        }
    }
    else
    {
        if ( bGreat )
        {
            if ( bPropGreat )
            {
                long nTempDelta = nDelta;
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nTempDelta )
                        {
                            rItems[n-1].mnPixSize++;
                            nTempDelta--;
                        }
                        n--;
                    }
                    while ( n > nMin );
                }
                while ( nTempDelta );
            }
            else
                rItems[nPos].mnPixSize += nDelta;
        }

        if ( bSmall )
        {
            if ( bPropSmall )
            {
                do
                {
                    n = nPos+1;
                    do
                    {
                        if ( nDelta && rItems[n].mnPixSize )
                        {
                            rItems[n].mnPixSize--;
                            nDelta--;
                        }

                        n++;
                    }
                    while ( n < nMax );
                }
                while ( nDelta );
            }
            else
            {
                n = nPos+1;
                do
                {
                    if ( rItems[n].mnPixSize-nDelta < 0 )
                    {
                        nDelta -= rItems[n].mnPixSize;
                        rItems[n].mnPixSize = 0;
                    }
                    else
                    {
                        rItems[n].mnPixSize -= nDelta;
                        break;
                    }
                    n++;
                }
                while ( n < nMax );
            }
        }
    }

    // update original sizes
    ImplCalcLogSize( rItems, nItems );

    ImplUpdate();
}

void SplitWindow::SetItemSize( sal_uInt16 nId, long nNewSize )
{
    sal_uInt16          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );
    ImplSplitItem*  pItem;

    if ( !pSet )
        return;

    // check if size is changed
    pItem = &pSet->mvItems[nPos];
    if ( pItem->mnSize != nNewSize )
    {
        // set new size and re-calculate
        pItem->mnSize = nNewSize;
        pSet->mbCalcPix = true;
        ImplUpdate();
    }
}

long SplitWindow::GetItemSize( sal_uInt16 nId ) const
{
    sal_uInt16          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mvItems[nPos].mnSize;
    else
        return 0;
}

long SplitWindow::GetItemSize( sal_uInt16 nId, SplitWindowItemFlags nBits ) const
{
    sal_uInt16          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
    {
        if ( nBits == pSet->mvItems[nPos].mnBits )
            return pSet->mvItems[nPos].mnSize;
        else
        {
            const_cast<SplitWindow*>(this)->ImplCalcLayout();

            long                nRelSize = 0;
            long                nPerSize = 0;
            size_t              nItems;
            SplitWindowItemFlags nTempBits;
            nItems = pSet->mvItems.size();
            std::vector< ImplSplitItem >& rItems = pSet->mvItems;
            for ( size_t i = 0; i < nItems; i++ )
            {
                if ( i == nPos )
                    nTempBits = nBits;
                else
                    nTempBits = rItems[i].mnBits;
                if ( nTempBits & SplitWindowItemFlags::RelativeSize )
                    nRelSize += rItems[i].mnPixSize;
                else if ( nTempBits & SplitWindowItemFlags::PercentSize )
                    nPerSize += rItems[i].mnPixSize;
            }
            nPerSize += nRelSize;
            if ( nBits & SplitWindowItemFlags::RelativeSize )
            {
                if ( nRelSize )
                    return (rItems[nPos].mnPixSize+(nRelSize/2))/nRelSize;
                else
                    return 1;
            }
            else if ( nBits & SplitWindowItemFlags::PercentSize )
            {
                if ( nPerSize )
                    return (rItems[nPos].mnPixSize*100)/nPerSize;
                else
                    return 1;
            }
            else
                return rItems[nPos].mnPixSize;
        }
    }
    else
        return 0;
}

void SplitWindow::SetItemSizeRange (sal_uInt16 nId, const Range& rRange)
{
    sal_uInt16 nPos;
    ImplSplitSet* pSet = ImplFindItem(mpBaseSet, nId, nPos);

    if (pSet != nullptr)
    {
        pSet->mvItems[nPos].mnMinSize = rRange.Min();
        pSet->mvItems[nPos].mnMaxSize = rRange.Max();
    }
}

sal_uInt16 SplitWindow::GetSet( sal_uInt16 nId ) const
{
    sal_uInt16          nPos;
    ImplSplitSet*   pSet = ImplFindItem( mpBaseSet, nId, nPos );

    if ( pSet )
        return pSet->mnId;
    else
        return 0;
}

bool SplitWindow::IsItemValid( sal_uInt16 nId ) const
{
    sal_uInt16          nPos;
    ImplSplitSet* pSet = mpBaseSet ? ImplFindItem(mpBaseSet, nId, nPos) : nullptr;

    return pSet != nullptr;
}

sal_uInt16 SplitWindow::GetItemId( vcl::Window* pWindow ) const
{
    return ImplFindItem( mpBaseSet, pWindow );
}

sal_uInt16 SplitWindow::GetItemId( const Point& rPos ) const
{
    return ImplFindItem( mpBaseSet, rPos, mbHorz, !mbBottomRight );
}

sal_uInt16 SplitWindow::GetItemPos( sal_uInt16 nId, sal_uInt16 nSetId ) const
{
    ImplSplitSet*   pSet = ImplFindSet( mpBaseSet, nSetId );
    sal_uInt16          nPos = SPLITWINDOW_ITEM_NOTFOUND;

    if ( pSet )
    {
        for ( size_t i = 0; i < pSet->mvItems.size(); i++ )
        {
            if ( pSet->mvItems[i].mnId == nId )
            {
                nPos = i;
                break;
            }
        }
    }

    return nPos;
}

sal_uInt16 SplitWindow::GetItemId( sal_uInt16 nPos ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpBaseSet, 0/*nSetId*/ );
    if ( pSet && (nPos < pSet->mvItems.size()) )
        return pSet->mvItems[nPos].mnId;
    else
        return 0;
}

sal_uInt16 SplitWindow::GetItemCount( sal_uInt16 nSetId ) const
{
    ImplSplitSet* pSet = ImplFindSet( mpBaseSet, nSetId );
    if ( pSet )
        return pSet->mvItems.size();
    else
        return 0;
}

void SplitWindow::ImplNewAlign()
{
    switch ( meAlign )
    {
    case WindowAlign::Top:
        mbHorz        = true;
        mbBottomRight = false;
        break;
    case WindowAlign::Bottom:
        mbHorz        = true;
        mbBottomRight = true;
        break;
    case WindowAlign::Left:
        mbHorz        = false;
        mbBottomRight = false;
        break;
    case WindowAlign::Right:
        mbHorz        = false;
        mbBottomRight = true;
        break;
    }

    if ( mnWinStyle & WB_BORDER )
    {
        ImplCalcBorder( meAlign, mnLeftBorder, mnTopBorder,
                        mnRightBorder, mnBottomBorder );
    }

    if ( IsReallyVisible() && IsUpdateMode() )
        Invalidate();
    ImplUpdate();
}

void SplitWindow::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;
        ImplNewAlign();
    }
}

void SplitWindow::ShowFadeInHideButton()
{
    mbFadeIn = true;
    ImplUpdate();
}

void SplitWindow::ShowFadeOutButton()
{
    mbFadeOut = true;
    ImplUpdate();
}

long SplitWindow::GetFadeInSize() const
{
    long n = 0;

    if ( mbHorz )
        n = mnTopBorder+mnBottomBorder;
    else
        n = mnLeftBorder+mnRightBorder;

    return n+SPLITWIN_SPLITSIZE+SPLITWIN_SPLITSIZEEX-2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
