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

#include <vcl/svapp.hxx>

#include <pagepreviewlayout.hxx>
#include <prevwpage.hxx>

#include <algorithm>
#include <tools/fract.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>

#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <swregion.hxx>
#include <comcore.hrc>
#include <frmtool.hxx>
#include <sfx2/zoomitem.hxx>
#include <printdata.hxx>
#include <paintfrm.hxx>

#include <IDocumentDeviceAccess.hxx>

// methods to initialize page preview layout

SwPagePreviewLayout::SwPagePreviewLayout( SwViewShell& _rParentViewShell,
                                          const SwRootFrame& _rLayoutRootFrame )
    : mnXFree ( 4*142 ),
      mnYFree ( 4*142 ),
      mrParentViewShell( _rParentViewShell ),
      mrLayoutRootFrame ( _rLayoutRootFrame )
{
    _Clear();

    // OD 2004-03-05 #i18143#
    mbBookPreview = false;
    mbBookPreviewModeToggled = false;

    mbPrintEmptyPages = mrParentViewShell.getIDocumentDeviceAccess().getPrintData().IsPrintEmptyPages();
}

void SwPagePreviewLayout::_Clear()
{
    mbLayoutInfoValid = mbLayoutSizesValid = mbPaintInfoValid = false;

    maWinSize.Width() = 0;
    maWinSize.Height() = 0;
    mnCols = mnRows = 0;

    _ClearPreviewLayoutSizes();

    mbDoesLayoutRowsFitIntoWindow = false;
    mbDoesLayoutColsFitIntoWindow = false;

    mnPaintPhyStartPageNum = 0;
    mnPaintStartCol = mnPaintStartRow = 0;
    mbNoPageVisible = false;
    maPaintStartPageOffset.X() = 0;
    maPaintStartPageOffset.Y() = 0;
    maPaintPreviewDocOffset.X() = 0;
    maPaintPreviewDocOffset.Y() = 0;
    maAdditionalPaintOffset.X() = 0;
    maAdditionalPaintOffset.Y() = 0;
    maPaintedPreviewDocRect.Left() = 0;
    maPaintedPreviewDocRect.Top() = 0;
    maPaintedPreviewDocRect.Right() = 0;
    maPaintedPreviewDocRect.Bottom() = 0;
    mnSelectedPageNum = 0;
    _ClearPreviewPageData();

    // OD 07.11.2003 #i22014#
    mbInPaint = false;
    mbNewLayoutDuringPaint = false;
}

void SwPagePreviewLayout::_ClearPreviewLayoutSizes()
{
    mnPages = 0;

    maMaxPageSize.Width() = 0;
    maMaxPageSize.Height() = 0;
    maPreviewDocRect.Left() = maPreviewDocRect.Top() = 0;
    maPreviewDocRect.Right() = maPreviewDocRect.Bottom() = 0;
    mnColWidth = mnRowHeight = 0;
    mnPreviewLayoutWidth = mnPreviewLayoutHeight = 0;
}

void SwPagePreviewLayout::_ClearPreviewPageData()
{
    for ( std::vector<PreviewPage*>::iterator aPageDelIter = maPreviewPages.begin();
          aPageDelIter != maPreviewPages.end();
          ++aPageDelIter )
    {
        delete (*aPageDelIter);
    }
    maPreviewPages.clear();
}

/** calculate page preview layout sizes

    OD 18.12.2002 #103492#
*/
void SwPagePreviewLayout::_CalcPreviewLayoutSizes()
{
    vcl::RenderContext* pRenderContext = mrParentViewShell.GetOut();
    // calculate maximal page size; calculate also number of pages

    const SwPageFrame* pPage = static_cast<const SwPageFrame*>(mrLayoutRootFrame.Lower());
    while ( pPage )
    {
        if ( !mbBookPreview && !mbPrintEmptyPages && pPage->IsEmptyPage() )
        {
            pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
            continue;
        }

        ++mnPages;
        pPage->Calc(pRenderContext);
        const Size& rPageSize = pPage->Frame().SSize();
        if ( rPageSize.Width() > maMaxPageSize.Width() )
            maMaxPageSize.Width() = rPageSize.Width();
        if ( rPageSize.Height() > maMaxPageSize.Height() )
            maMaxPageSize.Height() = rPageSize.Height();
        pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
    }
    // calculate and set column width and row height
    mnColWidth = maMaxPageSize.Width() + mnXFree;
    mnRowHeight = maMaxPageSize.Height() + mnYFree;

    // calculate and set preview layout width and height
    mnPreviewLayoutWidth = mnCols * mnColWidth + mnXFree;
    mnPreviewLayoutHeight = mnRows * mnRowHeight + mnYFree;

    // calculate document rectangle in preview layout
    {
        Size aDocSize;
        // document width
        aDocSize.Width() = mnPreviewLayoutWidth;

        // document height
        // determine number of rows needed for <nPages> in preview layout
        // OD 19.02.2003 #107369# - use method <GetRowOfPage(..)>.
        const sal_uInt16 nDocRows = GetRowOfPage( mnPages );
        aDocSize.Height() = nDocRows * maMaxPageSize.Height() +
                            (nDocRows+1) * mnYFree;
        maPreviewDocRect.SetPos( Point( 0, 0 ) );
        maPreviewDocRect.SetSize( aDocSize );
    }
}

/** init page preview layout

    OD 11.12.2002 #103492#
    initialize the page preview settings for a given layout.

    side effects:
    (1) If parameter <_bCalcScale> is true, mapping mode with calculated
    scaling is set at the output device and the zoom at the view options of
    the given view shell is set with the calculated scaling.
*/
bool SwPagePreviewLayout::Init( const sal_uInt16 _nCols,
                                const sal_uInt16 _nRows,
                                const Size&      _rPxWinSize,
                                const bool       _bCalcScale
                              )
{
    // check environment and parameters
    {
        bool bColsRowsValid = (_nCols != 0) && (_nRows != 0);
        OSL_ENSURE( bColsRowsValid, "preview layout parameters not correct - preview layout can *not* be initialized" );
        if ( !bColsRowsValid )
            return false;

        bool bPxWinSizeValid = (_rPxWinSize.Width() >= 0) &&
                               (_rPxWinSize.Height() >= 0);
        OSL_ENSURE( bPxWinSizeValid, "no window size - preview layout can *not* be initialized" );
        if ( !bPxWinSizeValid )
            return false;
    }

    // environment and parameters ok

    // clear existing preview settings
    _Clear();

    // set layout information columns and rows
    mnCols = _nCols;
    mnRows = _nRows;

    _CalcPreviewLayoutSizes();

    // validate layout information
    mbLayoutInfoValid = true;

    if ( _bCalcScale )
    {
        // calculate scaling
        MapMode aMapMode( MAP_TWIP );
        Size aWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize, aMapMode );
        Fraction aXScale( aWinSize.Width(), mnPreviewLayoutWidth );
        Fraction aYScale( aWinSize.Height(), mnPreviewLayoutHeight );
        if( aXScale < aYScale )
            aYScale = aXScale;
        {
            // adjust scaling for Drawing layer.
            aYScale *= Fraction( 1000, 1 );
            long nNewNuminator = aYScale.operator long();
            if( nNewNuminator < 1 )
                nNewNuminator = 1;
            aYScale = Fraction( nNewNuminator, 1000 );
            // propagate scaling as zoom percentage to view options for font cache
            _ApplyNewZoomAtViewShell( static_cast<sal_uInt8>(nNewNuminator/10) );
        }
        aMapMode.SetScaleY( aYScale );
        aMapMode.SetScaleX( aYScale );
        // set created mapping mode with calculated scaling at output device.
        mrParentViewShell.GetOut()->SetMapMode( aMapMode );
        // OD 20.02.2003 #107369# - update statics for paint.
        ::SwCalcPixStatics( mrParentViewShell.GetOut() );
    }

    // set window size in twips
    maWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize );
    // validate layout sizes
    mbLayoutSizesValid = true;

    return true;
}

/** apply new zoom at given view shell */
void SwPagePreviewLayout::_ApplyNewZoomAtViewShell( sal_uInt8 _aNewZoom )
{
    SwViewOption aNewViewOptions = *(mrParentViewShell.GetViewOptions());
    if ( aNewViewOptions.GetZoom() != _aNewZoom )
    {
        aNewViewOptions.SetZoom( _aNewZoom );
        //#i19975# - consider zoom type.
        SvxZoomType eZoomType = SvxZoomType::PERCENT;
        aNewViewOptions.SetZoomType( eZoomType );
        mrParentViewShell.ApplyViewOptions( aNewViewOptions );
    }
}

/** method to adjust page preview layout to document changes

    OD 18.12.2002 #103492#
*/
bool SwPagePreviewLayout::ReInit()
{
    // check environment and parameters
    {
        bool bLayoutSettingsValid = mbLayoutInfoValid && mbLayoutSizesValid;
        OSL_ENSURE( bLayoutSettingsValid,
                "no valid preview layout info/sizes - no re-init of page preview layout");
        if ( !bLayoutSettingsValid )
            return false;
    }

    _ClearPreviewLayoutSizes();
    _CalcPreviewLayoutSizes();

    return true;
}

// methods to prepare paint of page preview

/** prepare paint of page preview

    OD 12.12.2002 #103492#
    OD 21.03.2003 #108282# - delete parameter _onStartPageVirtNum

    @note _nProposedStartPageNum, _onStartPageNum are absolute
*/
bool SwPagePreviewLayout::Prepare( const sal_uInt16 _nProposedStartPageNum,
                                   const Point&      rProposedStartPos,
                                   const Size&      _rPxWinSize,
                                   sal_uInt16&      _onStartPageNum,
                                   Rectangle&       _orDocPreviewPaintRect,
                                   const bool       _bStartWithPageAtFirstCol
                                 )
{
    sal_uInt16 nProposedStartPageNum = ConvertAbsoluteToRelativePageNum( _nProposedStartPageNum );
    // check environment and parameters
    {
        bool bLayoutSettingsValid = mbLayoutInfoValid && mbLayoutSizesValid;
        OSL_ENSURE( bLayoutSettingsValid,
                "no valid preview layout info/sizes - no prepare of preview paint");
        if ( !bLayoutSettingsValid )
            return false;

        bool bStartPageRangeValid = nProposedStartPageNum <= mnPages;
        OSL_ENSURE( bStartPageRangeValid,
                "proposed start page not existing - no prepare of preview paint");
        if ( !bStartPageRangeValid )
            return false;

        bool bStartPosRangeValid =
                rProposedStartPos.X() >= 0 && rProposedStartPos.Y() >= 0 &&
                rProposedStartPos.X() <= maPreviewDocRect.Right() &&
                rProposedStartPos.Y() <= maPreviewDocRect.Bottom();
        OSL_ENSURE( bStartPosRangeValid,
                "proposed start position out of range - no prepare of preview paint");
        if ( !bStartPosRangeValid )
            return false;

        bool bWinSizeValid = _rPxWinSize.Width() != 0 && _rPxWinSize.Height() != 0;
       OSL_ENSURE( bWinSizeValid, "no window size - no prepare of preview paint");
        if ( !bWinSizeValid )
            return false;

        bool bStartInfoValid = _nProposedStartPageNum > 0 ||
                               rProposedStartPos != Point(0,0);
        if ( !bStartInfoValid )
            nProposedStartPageNum = 1;
    }

    // environment and parameter ok

    // update window size at preview setting data
    maWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize );

    mbNoPageVisible = false;
    if ( nProposedStartPageNum > 0 )
    {
        // determine column and row of proposed start page in virtual preview layout
        const sal_uInt16 nColOfProposed = GetColOfPage( nProposedStartPageNum );
        const sal_uInt16 nRowOfProposed = GetRowOfPage( nProposedStartPageNum );
        // determine start page
        if ( _bStartWithPageAtFirstCol )
        {
            // OD 19.02.2003 #107369# - leaving left-top-corner blank is
            // controlled by <mbBookPreview>.
            if ( mbBookPreview &&
                 ( nProposedStartPageNum == 1 || nRowOfProposed == 1 )
               )
                mnPaintPhyStartPageNum = 1;
            else
                mnPaintPhyStartPageNum = nProposedStartPageNum - (nColOfProposed-1);
        }
        else
            mnPaintPhyStartPageNum = nProposedStartPageNum;

        mnPaintPhyStartPageNum = ConvertRelativeToAbsolutePageNum( mnPaintPhyStartPageNum );

        // set starting column
        if ( _bStartWithPageAtFirstCol )
            mnPaintStartCol = 1;
        else
            mnPaintStartCol = nColOfProposed;
        // set starting row
        mnPaintStartRow = nRowOfProposed;
        // page offset == (-1,-1), indicating no offset and paint of free space.
        maPaintStartPageOffset.X() = -1;
        maPaintStartPageOffset.Y() = -1;
        // virtual preview document offset.
        if ( _bStartWithPageAtFirstCol )
            maPaintPreviewDocOffset.X() = 0;
        else
            maPaintPreviewDocOffset.X() = (nColOfProposed-1) * mnColWidth;
        maPaintPreviewDocOffset.Y() = (nRowOfProposed-1) * mnRowHeight;
    }
    else
    {
        // determine column and row of proposed start position.
        // Note: paint starts at point (0,0)
        const sal_uInt16 nColOfProposed =
                static_cast<sal_uInt16>(rProposedStartPos.X() / mnColWidth) + 1;
        const sal_uInt16 nRowOfProposed =
                static_cast<sal_uInt16>(rProposedStartPos.Y() / mnRowHeight) + 1;
        // determine start page == page at proposed start position
        // OD 19.02.2003 #107369# - leaving left-top-corner blank is
        // controlled by <mbBookPreview>.
        if ( mbBookPreview &&
             ( nRowOfProposed == 1 && nColOfProposed == 1 )
           )
            mnPaintPhyStartPageNum = 1;
        else
        {
            // OD 19.02.2003 #107369# - leaving left-top-corner blank is
            // controlled by <mbBookPreview>.
            mnPaintPhyStartPageNum = (nRowOfProposed-1) * mnCols + nColOfProposed;
            if ( mbBookPreview )
                --mnPaintPhyStartPageNum;
            if ( mnPaintPhyStartPageNum > mnPages )
            {
                // no page will be visible, because shown part of document
                // preview is the last row to the right of the last page
                mnPaintPhyStartPageNum = mnPages;
                mbNoPageVisible = true;
            }
        }
        // set starting column and starting row
        mnPaintStartCol = nColOfProposed;
        mnPaintStartRow = nRowOfProposed;
        // page offset
        maPaintStartPageOffset.X() =
                (rProposedStartPos.X() % mnColWidth) - mnXFree;
        maPaintStartPageOffset.Y() =
                (rProposedStartPos.Y() % mnRowHeight) - mnYFree;
        // virtual preview document offset.
        maPaintPreviewDocOffset = rProposedStartPos;
    }

    // determine additional paint offset, if preview layout fits into window.
    _CalcAdditionalPaintOffset();

    // determine rectangle to be painted from document preview
    _CalcDocPreviewPaintRect();
    _orDocPreviewPaintRect = maPaintedPreviewDocRect;

    // OD 20.01.2003 #103492# - shift visible preview document area to the left,
    // if on the right is an area left blank.
    if ( !mbDoesLayoutColsFitIntoWindow &&
         maPaintedPreviewDocRect.GetWidth() < maWinSize.Width() )
    {
        maPaintedPreviewDocRect.Move(
                -(maWinSize.Width() - maPaintedPreviewDocRect.GetWidth()), 0 );
        Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                 _rPxWinSize, _onStartPageNum,
                 _orDocPreviewPaintRect, _bStartWithPageAtFirstCol );
    }

    // OD 20.01.2003 #103492# - shift visible preview document area to the top,
    // if on the botton is an area left blank.
    if ( mbBookPreviewModeToggled &&
         maPaintedPreviewDocRect.Bottom() == maPreviewDocRect.Bottom() &&
         maPaintedPreviewDocRect.GetHeight() < maWinSize.Height() )
    {
        if ( mbDoesLayoutRowsFitIntoWindow )
        {
            if ( maPaintedPreviewDocRect.GetHeight() < mnPreviewLayoutHeight)
            {
                maPaintedPreviewDocRect.Move(
                        0, -(mnPreviewLayoutHeight - maPaintedPreviewDocRect.GetHeight()) );
                Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                         _rPxWinSize, _onStartPageNum,
                         _orDocPreviewPaintRect, _bStartWithPageAtFirstCol );
            }
        }
        else
        {
            maPaintedPreviewDocRect.Move(
                    0, -(maWinSize.Height() - maPaintedPreviewDocRect.GetHeight()) );
            Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                     _rPxWinSize, _onStartPageNum,
                     _orDocPreviewPaintRect, _bStartWithPageAtFirstCol );
        }
    }

    // determine preview pages - visible pages with needed data for paint and
    // accessible pages with needed data.
    _CalcPreviewPages();

    // OD 07.11.2003 #i22014# - indicate new layout, if print preview is in paint
    if ( mbInPaint )
    {
        mbNewLayoutDuringPaint = true;
    }

    // validate paint data
    mbPaintInfoValid = true;

    // return start page
    _onStartPageNum = mnPaintPhyStartPageNum;

    return true;
}

/** calculate additional paint offset

    OD 12.12.2002 #103492#
*/
void SwPagePreviewLayout::_CalcAdditionalPaintOffset()
{
    if ( mnPreviewLayoutWidth <= maWinSize.Width() &&
         maPaintStartPageOffset.X() <= 0 )
    {
        mbDoesLayoutColsFitIntoWindow = true;
        maAdditionalPaintOffset.X() = (maWinSize.Width() - mnPreviewLayoutWidth) / 2;
    }
    else
    {
        mbDoesLayoutColsFitIntoWindow = false;
        maAdditionalPaintOffset.X() = 0;
    }

    if ( mnPreviewLayoutHeight <= maWinSize.Height() &&
         maPaintStartPageOffset.Y() <= 0 )
    {
        mbDoesLayoutRowsFitIntoWindow = true;
        maAdditionalPaintOffset.Y() = (maWinSize.Height() - mnPreviewLayoutHeight) / 2;
    }
    else
    {
        mbDoesLayoutRowsFitIntoWindow = false;
        maAdditionalPaintOffset.Y() = 0;
    }
}

/** calculate painted preview document rectangle

    OD 12.12.2002 #103492#
*/
void SwPagePreviewLayout::_CalcDocPreviewPaintRect()
{
    Point aTopLeftPos = maPaintPreviewDocOffset;
    maPaintedPreviewDocRect.SetPos( aTopLeftPos );

    Size aSize;
    if ( mbDoesLayoutColsFitIntoWindow )
        aSize.Width() = std::min( mnPreviewLayoutWidth,
                             maPreviewDocRect.GetWidth() - aTopLeftPos.X() );
    else
        aSize.Width() = std::min( maPreviewDocRect.GetWidth() - aTopLeftPos.X(),
                             maWinSize.Width() - maAdditionalPaintOffset.X() );
    if ( mbDoesLayoutRowsFitIntoWindow )
        aSize.Height() = std::min( mnPreviewLayoutHeight,
                              maPreviewDocRect.GetHeight() - aTopLeftPos.Y() );
    else
        aSize.Height() = std::min( maPreviewDocRect.GetHeight() - aTopLeftPos.Y(),
                              maWinSize.Height() - maAdditionalPaintOffset.Y() );
    maPaintedPreviewDocRect.SetSize( aSize );
}

/** calculate preview pages

    OD 12.12.2002 #103492#
*/
void SwPagePreviewLayout::_CalcPreviewPages()
{
    vcl::RenderContext* pRenderContext = mrParentViewShell.GetOut();
    _ClearPreviewPageData();

    if ( mbNoPageVisible )
        return;

    // determine start page frame
    const SwPageFrame* pStartPage = mrLayoutRootFrame.GetPageByPageNum( mnPaintPhyStartPageNum );

    // calculate initial paint offset
    Point aInitialPaintOffset;
    /// check whether RTL interface or not
    if(!AllSettings::GetLayoutRTL()){
        if ( maPaintStartPageOffset != Point( -1, -1 ) )
            aInitialPaintOffset = Point(0,0) - maPaintStartPageOffset;
        else
            aInitialPaintOffset = Point( mnXFree, mnYFree );
    }
    else {
        if ( maPaintStartPageOffset != Point( -1, -1 ) )
            aInitialPaintOffset = Point(0 + ((SwPagePreviewLayout::mnCols-1)*mnColWidth),0) - maPaintStartPageOffset;
        else
            aInitialPaintOffset = Point( mnXFree + ((SwPagePreviewLayout::mnCols-1)*mnColWidth), mnYFree );
    }
    aInitialPaintOffset += maAdditionalPaintOffset;

    // prepare loop data
    const SwPageFrame* pPage = pStartPage;
    sal_uInt16 nCurrCol = mnPaintStartCol;
    sal_uInt16 nConsideredRows = 0;
    Point aCurrPaintOffset = aInitialPaintOffset;
    // loop on pages to determine preview background rectangles
    while ( pPage &&
            (!mbDoesLayoutRowsFitIntoWindow || nConsideredRows < mnRows) &&
            aCurrPaintOffset.Y() < maWinSize.Height()
          )
    {
        if ( !mbBookPreview && !mbPrintEmptyPages && pPage->IsEmptyPage() )
        {
            pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
            continue;
        }

        pPage->Calc(pRenderContext);

        // consider only pages, which have to be painted.
        if ( nCurrCol < mnPaintStartCol )
        {
            // calculate data of unvisible page needed for accessibility
            PreviewPage* pPreviewPage = new PreviewPage;
            Point aCurrAccOffset = aCurrPaintOffset -
                           Point( (mnPaintStartCol-nCurrCol) * mnColWidth, 0 );
            _CalcPreviewDataForPage( *(pPage), aCurrAccOffset, pPreviewPage );
            pPreviewPage->bVisible = false;
            maPreviewPages.push_back( pPreviewPage );
            // continue with next page and next column
            pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
            ++nCurrCol;
            continue;
        }
        if ( aCurrPaintOffset.X() < maWinSize.Width() )
        {
            // OD 19.02.2003 #107369# - leaving left-top-corner blank is
            // controlled by <mbBookPreview>.
            if ( mbBookPreview && pPage->GetPhyPageNum() == 1 && mnCols != 1 && nCurrCol == 1
               )
            {
                // first page in 2nd column
                // --> continue with increased paint offset and next column
                /// check whether RTL interface or not
                if(!AllSettings::GetLayoutRTL())
                    aCurrPaintOffset.X() += mnColWidth;
                else aCurrPaintOffset.X() -= mnColWidth;
                ++nCurrCol;
                continue;
            }

            // calculate data of visible page
            PreviewPage* pPreviewPage = new PreviewPage;
            _CalcPreviewDataForPage( *(pPage), aCurrPaintOffset, pPreviewPage );
            pPreviewPage->bVisible = true;
            maPreviewPages.push_back( pPreviewPage );
        }
        else
        {
            // calculate data of unvisible page needed for accessibility
            PreviewPage* pPreviewPage = new PreviewPage;
            _CalcPreviewDataForPage( *(pPage), aCurrPaintOffset, pPreviewPage );
            pPreviewPage->bVisible = false;
            maPreviewPages.push_back( pPreviewPage );
        }

        // prepare data for next loop
        pPage = static_cast<const SwPageFrame*>(pPage->GetNext());

        /// check whether RTL interface or not
        if(!AllSettings::GetLayoutRTL())
            aCurrPaintOffset.X() += mnColWidth;
        else aCurrPaintOffset.X() -= mnColWidth;
        ++nCurrCol;
        if ( nCurrCol > mnCols )
        {
            ++nConsideredRows;
            aCurrPaintOffset.X() = aInitialPaintOffset.X();
            nCurrCol = 1;
            aCurrPaintOffset.Y() += mnRowHeight;
        }
    }
}

/** determines preview data for a given page and a given preview offset

    OD 13.12.2002 #103492#
*/
bool SwPagePreviewLayout::_CalcPreviewDataForPage( const SwPageFrame& _rPage,
                                                   const Point& _rPreviewOffset,
                                                   PreviewPage* _opPreviewPage )
{
    // page frame
    _opPreviewPage->pPage = &_rPage;
    // size of page frame
    if ( _rPage.IsEmptyPage() )
    {
        if ( _rPage.GetPhyPageNum() % 2 == 0 )
            _opPreviewPage->aPageSize = _rPage.GetPrev()->Frame().SSize();
        else
            _opPreviewPage->aPageSize = _rPage.GetNext()->Frame().SSize();
    }
    else
        _opPreviewPage->aPageSize = _rPage.Frame().SSize();
    // position of page in preview window
    Point aPreviewWinOffset( _rPreviewOffset );
    if ( _opPreviewPage->aPageSize.Width() < maMaxPageSize.Width() )
        aPreviewWinOffset.X() += ( maMaxPageSize.Width() - _opPreviewPage->aPageSize.Width() ) / 2;
    if ( _opPreviewPage->aPageSize.Height() < maMaxPageSize.Height() )
        aPreviewWinOffset.Y() += ( maMaxPageSize.Height() - _opPreviewPage->aPageSize.Height() ) / 2;
    _opPreviewPage->aPreviewWinPos = aPreviewWinOffset;
    // logic position of page and mapping offset for paint
    if ( _rPage.IsEmptyPage() )
    {
        _opPreviewPage->aLogicPos = _opPreviewPage->aPreviewWinPos;
        _opPreviewPage->aMapOffset = Point( 0, 0 );
    }
    else
    {
        _opPreviewPage->aLogicPos = _rPage.Frame().Pos();
        _opPreviewPage->aMapOffset = _opPreviewPage->aPreviewWinPos - _opPreviewPage->aLogicPos;
    }

    return true;
}

/** enable/disable book preview

    OD 2004-03-04 #i18143#
*/
bool SwPagePreviewLayout::SetBookPreviewMode( const bool _bEnableBookPreview,
                                              sal_uInt16& _onStartPageNum,
                                              Rectangle&  _orDocPreviewPaintRect )
{
    if ( mbBookPreview != _bEnableBookPreview)
    {
        mbBookPreview = _bEnableBookPreview;
        // re-initialize page preview layout
        ReInit();
        // re-prepare page preview layout
        {
            mbBookPreviewModeToggled = true;
            Point aProposedStartPos( maPaintPreviewDocOffset );
            // if proposed start position is below virtual preview document
            // bottom, adjust it to the virtual preview document bottom
            if ( aProposedStartPos.Y() > maPreviewDocRect.Bottom() )
            {
                aProposedStartPos.Y() = maPreviewDocRect.Bottom();
            }
            Prepare( 0, aProposedStartPos,
                     mrParentViewShell.GetOut()->LogicToPixel( maWinSize ),
                     _onStartPageNum, _orDocPreviewPaintRect );
            mbBookPreviewModeToggled = false;
        }

        return true;
    }

    return false;
}

// methods to determine new data for changing the current shown part of the
// document preview.

/** calculate start position for new scale

    OD 12.12.2002 #103492#
*/
Point SwPagePreviewLayout::GetPreviewStartPosForNewScale(
                          const Fraction& _aNewScale,
                          const Fraction& _aOldScale,
                          const Size&     _aNewWinSize ) const
{
    Point aNewPaintStartPos = maPaintedPreviewDocRect.TopLeft();
    if ( _aNewScale < _aOldScale )
    {
        // increase paint width by moving start point to left.
        if ( mnPreviewLayoutWidth < _aNewWinSize.Width() )
            aNewPaintStartPos.X() = 0;
        else if ( maPaintedPreviewDocRect.GetWidth() < _aNewWinSize.Width() )
        {
            aNewPaintStartPos.X() -=
                (_aNewWinSize.Width() - maPaintedPreviewDocRect.GetWidth()) / 2;
            if ( aNewPaintStartPos.X() < 0)
                aNewPaintStartPos.X() = 0;
        }

        if ( !mbDoesLayoutRowsFitIntoWindow )
        {
            // increase paint height by moving start point to top.
            if ( mnPreviewLayoutHeight < _aNewWinSize.Height() )
            {
                aNewPaintStartPos.Y() =
                    ( (mnPaintStartRow - 1) * mnRowHeight );
            }
            else if ( maPaintedPreviewDocRect.GetHeight() < _aNewWinSize.Height() )
            {
                aNewPaintStartPos.Y() -=
                    (_aNewWinSize.Height() - maPaintedPreviewDocRect.GetHeight()) / 2;
                if ( aNewPaintStartPos.Y() < 0)
                    aNewPaintStartPos.Y() = 0;
            }
        }
    }
    else
    {
        // decrease paint width by moving start point to right
        if ( maPaintedPreviewDocRect.GetWidth() > _aNewWinSize.Width() )
            aNewPaintStartPos.X() +=
                (maPaintedPreviewDocRect.GetWidth() - _aNewWinSize.Width()) / 2;
        // decrease paint height by moving start point to bottom
        if ( maPaintedPreviewDocRect.GetHeight() > _aNewWinSize.Height() )
        {
            aNewPaintStartPos.Y() +=
                (maPaintedPreviewDocRect.GetHeight() - _aNewWinSize.Height()) / 2;
            // check, if new y-position is outside document preview
            if ( aNewPaintStartPos.Y() > maPreviewDocRect.Bottom() )
                aNewPaintStartPos.Y() =
                        std::max( 0L, maPreviewDocRect.Bottom() - mnPreviewLayoutHeight );
        }
    }

    return aNewPaintStartPos;
}

/** determines, if page with given page number is visible in preview

    @note _nPageNum is absolut!
*/
bool SwPagePreviewLayout::IsPageVisible( const sal_uInt16 _nPageNum ) const
{
    const PreviewPage* pPreviewPage = _GetPreviewPageByPageNum( _nPageNum );
    return pPreviewPage && pPreviewPage->bVisible;
}

/** calculate data to bring new selected page into view.

    @note IN/OUT parameters are absolute page numbers!!!
*/
bool SwPagePreviewLayout::CalcStartValuesForSelectedPageMove(
                                const sal_Int16  _nHoriMove,
                                const sal_Int16  _nVertMove,
                                sal_uInt16&      _orNewSelectedPage,
                                sal_uInt16&      _orNewStartPage,
                                Point&           _orNewStartPos ) const
{
    // determine position of current selected page
    sal_uInt16 nTmpRelSelPageNum = ConvertAbsoluteToRelativePageNum( mnSelectedPageNum );
    sal_uInt16 nNewRelSelectedPageNum = nTmpRelSelPageNum;

    const sal_uInt16 nCurrRow = GetRowOfPage(nTmpRelSelPageNum);

    // determine new selected page number
    {
        if ( _nHoriMove != 0 )
        {
            if ( (nNewRelSelectedPageNum + _nHoriMove) < 1 )
                nNewRelSelectedPageNum = 1;
            else if ( (nNewRelSelectedPageNum + _nHoriMove) > mnPages )
                nNewRelSelectedPageNum = mnPages;
            else
                nNewRelSelectedPageNum = nNewRelSelectedPageNum + _nHoriMove;
        }
        if ( _nVertMove != 0 )
        {
            if ( (nNewRelSelectedPageNum + (_nVertMove * mnCols)) < 1 )
                nNewRelSelectedPageNum = 1;
            else if ( (nNewRelSelectedPageNum + (_nVertMove * mnCols)) > mnPages )
                nNewRelSelectedPageNum = mnPages;
            else
                nNewRelSelectedPageNum += ( _nVertMove * mnCols );
        }
    }

    sal_uInt16 nNewStartPage = mnPaintPhyStartPageNum;
    Point aNewStartPos = Point(0,0);

    const sal_uInt16 nNewAbsSelectedPageNum = ConvertRelativeToAbsolutePageNum( nNewRelSelectedPageNum );
    if ( !IsPageVisible( nNewAbsSelectedPageNum ) )
    {
        if ( _nHoriMove != 0 && _nVertMove != 0 )
        {
            OSL_FAIL( "missing implementation for moving preview selected page horizontal AND vertical");
            return false;
        }

        // new selected page has to be brought into view considering current
        // visible preview.
        const sal_uInt16 nTotalRows = GetRowOfPage( mnPages );
        if ( (_nHoriMove > 0 || _nVertMove > 0) &&
             mbDoesLayoutRowsFitIntoWindow &&
             mbDoesLayoutColsFitIntoWindow &&
             nCurrRow > nTotalRows - mnRows )
        {
            // new proposed start page = left-top-corner of last possible
            // preview page.
            nNewStartPage = (nTotalRows - mnRows) * mnCols + 1;
            // leaving left-top-corner blank is controlled
            // by <mbBookPreview>.
            if ( mbBookPreview )
            {
                // Note: decrease new proposed start page number by one,
                // because of blank left-top-corner
                --nNewStartPage;
            }
            nNewStartPage = ConvertRelativeToAbsolutePageNum( nNewStartPage );
        }
        else
        {
            // new proposed start page = new selected page.
            nNewStartPage = ConvertRelativeToAbsolutePageNum( nNewRelSelectedPageNum );
        }
    }

    _orNewSelectedPage = nNewAbsSelectedPageNum;
    _orNewStartPage = nNewStartPage;
    _orNewStartPos = aNewStartPos;

    return true;
}

/** checks, if given position is inside a shown document page */
struct PreviewPosInsidePagePred
{
    const Point mnPreviewPos;
    explicit PreviewPosInsidePagePred(const Point& rPreviewPos)
        : mnPreviewPos( rPreviewPos )
    {}
    bool operator() ( const PreviewPage* _pPreviewPage )
    {
        if ( _pPreviewPage->bVisible )
        {
            Rectangle aPreviewPageRect( _pPreviewPage->aPreviewWinPos, _pPreviewPage->aPageSize );
            return aPreviewPageRect.IsInside( mnPreviewPos );
        }
        return false;
    }
};

bool SwPagePreviewLayout::IsPreviewPosInDocPreviewPage( const Point&  rPreviewPos,
                                                    Point&       _orDocPos,
                                                    bool&        _obPosInEmptyPage,
                                                    sal_uInt16&  _onPageNum ) const
{
    // initialize variable parameter values.
    _orDocPos.X() = 0;
    _orDocPos.Y() = 0;
    _obPosInEmptyPage = false;
    _onPageNum = 0;

    std::vector<PreviewPage*>::const_iterator aFoundPreviewPageIter =
            std::find_if( maPreviewPages.begin(), maPreviewPages.end(),
                          PreviewPosInsidePagePred( rPreviewPos ) );

    if ( aFoundPreviewPageIter != maPreviewPages.end() )
    {
        // given preview position is inside a document page.
        _onPageNum = (*aFoundPreviewPageIter)->pPage->GetPhyPageNum();
        _obPosInEmptyPage = (*aFoundPreviewPageIter)->pPage->IsEmptyPage();
        if ( !_obPosInEmptyPage )
        {
            // given preview position inside a normal page
            _orDocPos = rPreviewPos -
                        (*aFoundPreviewPageIter)->aPreviewWinPos +
                        (*aFoundPreviewPageIter)->aLogicPos;
            return true;
        }
    }

    return false;
}

/** determine window page scroll amount */
SwTwips SwPagePreviewLayout::GetWinPagesScrollAmount(
                                const sal_Int16 _nWinPagesToScroll ) const
{
    SwTwips nScrollAmount;
    if ( mbDoesLayoutRowsFitIntoWindow )
    {
        nScrollAmount = (mnPreviewLayoutHeight - mnYFree) * _nWinPagesToScroll;
    }
    else
        nScrollAmount = _nWinPagesToScroll * maPaintedPreviewDocRect.GetHeight();

    // check, if preview layout size values are valid.
    // If not, the checks for an adjustment of the scroll amount aren't useful.
    if ( mbLayoutSizesValid )
    {
        if ( (maPaintedPreviewDocRect.Top() + nScrollAmount) <= 0 )
            nScrollAmount = -maPaintedPreviewDocRect.Top();

        // correct scroll amount
        if ( nScrollAmount > 0 &&
             maPaintedPreviewDocRect.Bottom() == maPreviewDocRect.Bottom()
           )
        {
            nScrollAmount = 0;
        }
        else
        {
            while ( (maPaintedPreviewDocRect.Top() + nScrollAmount + mnYFree) >= maPreviewDocRect.GetHeight() )
            {
                nScrollAmount -= mnRowHeight;
            }
        }
    }

    return nScrollAmount;
}

// methods to paint page preview layout

namespace
{
/// Similar to RenderContextGuard, but does not touch the draw view.
class PreviewRenderContextGuard
{
    VclPtr<vcl::RenderContext> m_pOriginalValue;
    SwViewShell& m_rShell;

public:
    PreviewRenderContextGuard(SwViewShell& rShell, vcl::RenderContext* pValue)
        : m_pOriginalValue(rShell.GetOut()),
        m_rShell(rShell)
    {
        m_rShell.SetOut(pValue);
    }

    ~PreviewRenderContextGuard()
    {
        m_rShell.SetOut(m_pOriginalValue);
    }
};
}

/** paint prepared preview

    OD 12.12.2002 #103492#
*/
bool SwPagePreviewLayout::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rOutRect) const
{
    PreviewRenderContextGuard aGuard(mrParentViewShell, &rRenderContext);
    // check environment and parameters
    {
        if (!mrParentViewShell.GetWin() && !mrParentViewShell.GetOut()->GetConnectMetaFile())
        {
            return false;
        }

        OSL_ENSURE(mbPaintInfoValid, "invalid preview settings - no paint of preview");
        if (!mbPaintInfoValid)
            return false;
    }

    // OD 17.11.2003 #i22014# - no paint, if <superfluous> flag is set at layout
    if (mrLayoutRootFrame.IsSuperfluous())
    {
        return true;
    }

    // environment and parameter ok

    // OD 07.11.2003 #i22014#
    if (mbInPaint)
    {
        return false;
    }
    mbInPaint = true;

    OutputDevice* pOutputDev = &rRenderContext;

    // prepare paint
    if ( maPreviewPages.size() > 0 )
    {
        mrParentViewShell.Imp()->m_bFirstPageInvalid = false;
        mrParentViewShell.Imp()->m_pFirstVisiblePage =
                const_cast<SwPageFrame*>(maPreviewPages[0]->pPage);
    }

    // paint preview background
    {
        SwRegionRects aPreviewBackgrdRegion(rOutRect);
        // calculate preview background rectangles
        for ( std::vector<PreviewPage*>::const_iterator aPageIter = maPreviewPages.begin();
              aPageIter != maPreviewPages.end();
              ++aPageIter )
        {
            if ( (*aPageIter)->bVisible )
            {
                aPreviewBackgrdRegion -=
                        SwRect( (*aPageIter)->aPreviewWinPos, (*aPageIter)->aPageSize );
            }
        }
        // paint preview background rectangles
        mrParentViewShell._PaintDesktop(rRenderContext, aPreviewBackgrdRegion);
    }

    // prepare data for paint of pages
    const Rectangle aPxOutRect( pOutputDev->LogicToPixel(rOutRect) );

    MapMode aMapMode( pOutputDev->GetMapMode() );
    MapMode aSavedMapMode = aMapMode;

    const vcl::Font& rEmptyPgFont = SwPageFrame::GetEmptyPageFont();

    for ( std::vector<PreviewPage*>::const_iterator aPageIter = maPreviewPages.begin();
          aPageIter != maPreviewPages.end();
          ++aPageIter )
    {
        if ( !(*aPageIter)->bVisible )
            continue;

        Rectangle aPageRect( (*aPageIter)->aLogicPos, (*aPageIter)->aPageSize );
        aMapMode.SetOrigin( (*aPageIter)->aMapOffset );
        pOutputDev->SetMapMode( aMapMode );
        Rectangle aPxPaintRect = pOutputDev->LogicToPixel( aPageRect );
        if ( aPxOutRect.IsOver( aPxPaintRect) )
        {
            if ( (*aPageIter)->pPage->IsEmptyPage() )
            {
                const Color aRetouche( mrParentViewShell.Imp()->GetRetoucheColor() );
                if( pOutputDev->GetFillColor() != aRetouche )
                    pOutputDev->SetFillColor( aRetouche );
                pOutputDev->SetLineColor(); // OD 20.02.2003 #107369# - no line color
                // OD 20.02.2003 #107369# - use aligned page rectangle
                {
                    SwRect aTmpPageRect( aPageRect );
                    ::SwAlignRect( aTmpPageRect, &mrParentViewShell, &rRenderContext );
                    aPageRect = aTmpPageRect.SVRect();
                }
                pOutputDev->DrawRect( aPageRect );

                // paint empty page text
                vcl::Font aOldFont( pOutputDev->GetFont() );
                pOutputDev->SetFont( rEmptyPgFont );
                pOutputDev->DrawText( aPageRect, SW_RESSTR( STR_EMPTYPAGE ),
                                    DrawTextFlags::VCenter |
                                    DrawTextFlags::Center |
                                    DrawTextFlags::Clip );
                pOutputDev->SetFont( aOldFont );
                // paint shadow and border for empty page
                // OD 19.02.2003 #107369# - use new method to paint page border and shadow
                SwPageFrame::PaintBorderAndShadow( aPageRect, &mrParentViewShell, true, false, true );
            }
            else
            {
                mrParentViewShell.maVisArea = aPageRect;
                aPxPaintRect.Intersection( aPxOutRect );
                Rectangle aPaintRect = pOutputDev->PixelToLogic( aPxPaintRect );
                mrParentViewShell.Paint(rRenderContext, aPaintRect);
                // --> OD 2007-08-15 #i80691#
                // paint page border and shadow
                {
                    SwRect aPageBorderRect;
                    SwPageFrame::GetBorderAndShadowBoundRect( SwRect( aPageRect ), &mrParentViewShell, &rRenderContext, aPageBorderRect,
                        (*aPageIter)->pPage->IsLeftShadowNeeded(), (*aPageIter)->pPage->IsRightShadowNeeded(), true );
                    const vcl::Region aDLRegion(aPageBorderRect.SVRect());
                    mrParentViewShell.DLPrePaint2(aDLRegion);
                    SwPageFrame::PaintBorderAndShadow( aPageRect, &mrParentViewShell, true, false, true );
                    mrParentViewShell.DLPostPaint2(true);
                }
                // <--
            }
            // OD 07.11.2003 #i22014# - stop painting, because new print
            // preview layout is created during paint.
            if ( mbNewLayoutDuringPaint )
            {
                break;
            }

            if ( (*aPageIter)->pPage->GetPhyPageNum() == mnSelectedPageNum )
            {
                _PaintSelectMarkAtPage(rRenderContext, *aPageIter);
            }

        }
    }

    // OD 17.11.2003 #i22014# - no update of accessible preview, if a new
    // print preview layout is created during paint.
    if ( !mbNewLayoutDuringPaint )
    {
        // update at accessibility interface
        mrParentViewShell.Imp()->UpdateAccessiblePreview(
                        maPreviewPages,
                        aMapMode.GetScaleX(),
                        mrLayoutRootFrame.GetPageByPageNum( mnSelectedPageNum ),
                        maWinSize );
    }

    pOutputDev->SetMapMode( aSavedMapMode );
    mrParentViewShell.maVisArea.Clear();

    // OD 07.11.2003 #i22014#
    mbInPaint = false;
    mbNewLayoutDuringPaint = false;

    return true;
}

/** repaint pages on page preview

    OD 18.12.2002 #103492#
*/
void SwPagePreviewLayout::Repaint( const Rectangle& rInvalidCoreRect ) const
{
    // check environment and parameters
    {
        if ( !mrParentViewShell.GetWin() &&
             !mrParentViewShell.GetOut()->GetConnectMetaFile() )
            return;

        OSL_ENSURE( mbPaintInfoValid,
                "invalid preview settings - no paint of preview" );
        if ( !mbPaintInfoValid )
            return;
    }

    // environment and parameter ok

    // prepare paint
    if ( maPreviewPages.size() > 0 )
    {
        mrParentViewShell.Imp()->m_bFirstPageInvalid = false;
        mrParentViewShell.Imp()->m_pFirstVisiblePage =
                const_cast<SwPageFrame*>(maPreviewPages[0]->pPage);
    }

    // invalidate visible pages, which overlap the invalid core rectangle
    for ( std::vector<PreviewPage*>::const_iterator aPageIter = maPreviewPages.begin();
          aPageIter != maPreviewPages.end();
          ++aPageIter )
    {
        if ( !(*aPageIter)->bVisible )
            continue;

        Rectangle aPageRect( (*aPageIter)->aLogicPos, (*aPageIter)->aPageSize );
        if ( rInvalidCoreRect.IsOver( aPageRect ) )
        {
            aPageRect.Intersection(rInvalidCoreRect);
            Rectangle aInvalidPreviewRect = aPageRect;
            aInvalidPreviewRect.SetPos( aInvalidPreviewRect.TopLeft() -
                                      (*aPageIter)->aLogicPos +
                                      (*aPageIter)->aPreviewWinPos );
            mrParentViewShell.GetWin()->Invalidate( aInvalidPreviewRect );
        }
    }
}

/** paint selection mark at page

    OD 17.12.2002 #103492#
*/
void SwPagePreviewLayout::_PaintSelectMarkAtPage(vcl::RenderContext& rRenderContext,
                                    const PreviewPage* _aSelectedPreviewPage ) const
{
    OutputDevice* pOutputDev = &rRenderContext;
    MapMode aMapMode( pOutputDev->GetMapMode() );
    // save mapping mode of output device
    MapMode aSavedMapMode = aMapMode;
    // save fill and line color of output device
    Color aFill( pOutputDev->GetFillColor() );
    Color aLine( pOutputDev->GetLineColor() );

    // determine selection mark color
    Color aSelPgLineColor(COL_LIGHTBLUE);
    const StyleSettings& rSettings =
        mrParentViewShell.GetWin()->GetSettings().GetStyleSettings();
    if ( rSettings.GetHighContrastMode() )
        aSelPgLineColor = rSettings.GetHighlightTextColor();

    // set needed mapping mode at output device
    aMapMode.SetOrigin( _aSelectedPreviewPage->aMapOffset );
    pOutputDev->SetMapMode( aMapMode );

    // calculate page rectangle in pixel coordinates
    SwRect aPageRect( _aSelectedPreviewPage->aLogicPos,
                         _aSelectedPreviewPage->aPageSize );
    // OD 19.02.2003 #107369# - use aligned page rectangle, as it is used for
    // page border and shadow paint - see <SwPageFrame::PaintBorderAndShadow(..)>
    ::SwAlignRect( aPageRect, &mrParentViewShell, pOutputDev );
    Rectangle aPxPageRect = pOutputDev->LogicToPixel( aPageRect.SVRect() );

    // draw two rectangle
    // OD 19.02.2003 #107369# - adjust position of select mark rectangle
    Rectangle aRect( aPxPageRect.Left(), aPxPageRect.Top(),
                       aPxPageRect.Right(), aPxPageRect.Bottom() );
    aRect = pOutputDev->PixelToLogic( aRect );
    pOutputDev->SetFillColor(); // OD 20.02.2003 #107369# - no fill color
    pOutputDev->SetLineColor( aSelPgLineColor );
    pOutputDev->DrawRect( aRect );
    // OD 19.02.2003 #107369# - adjust position of select mark rectangle
    aRect = Rectangle( aPxPageRect.Left()+1, aPxPageRect.Top()+1,
                       aPxPageRect.Right()-1, aPxPageRect.Bottom()-1 );
    aRect = pOutputDev->PixelToLogic( aRect );
    pOutputDev->DrawRect( aRect );

    // reset fill and line color of output device
    pOutputDev->SetFillColor( aFill );
    pOutputDev->SetLineColor( aLine );

    // reset mapping mode of output device
    pOutputDev->SetMapMode( aSavedMapMode );
}

/** paint to mark new selected page

    OD 17.12.2002 #103492#
    Perform paint for current selected page in order to unmark it.
    Set new selected page and perform paint to mark this page.

    @note _nSelectedPage, mnSelectedPage are absolut
*/
void SwPagePreviewLayout::MarkNewSelectedPage( const sal_uInt16 _nSelectedPage )
{
    const sal_uInt16 nOldSelectedPageNum = mnSelectedPageNum;
    mnSelectedPageNum = _nSelectedPage;

    // re-paint for current selected page in order to unmark it.
    const PreviewPage* pOldSelectedPreviewPage = _GetPreviewPageByPageNum( nOldSelectedPageNum );
    OutputDevice* pOutputDev = mrParentViewShell.GetOut();
    if ( pOldSelectedPreviewPage && pOldSelectedPreviewPage->bVisible )
    {
        // OD 20.02.2003 #107369# - invalidate only areas of selection mark.
        SwRect aPageRect( pOldSelectedPreviewPage->aPreviewWinPos,
                              pOldSelectedPreviewPage->aPageSize );
        ::SwAlignRect( aPageRect, &mrParentViewShell, pOutputDev );
        Rectangle aPxPageRect = pOutputDev->LogicToPixel( aPageRect.SVRect() );
        // invalidate top mark line
        Rectangle aInvalPxRect( aPxPageRect.Left(), aPxPageRect.Top(),
                                aPxPageRect.Right(), aPxPageRect.Top()+1 );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
        // invalidate right mark line
        aInvalPxRect = Rectangle( aPxPageRect.Right()-1, aPxPageRect.Top(),
                                  aPxPageRect.Right(), aPxPageRect.Bottom() );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
        // invalidate bottom mark line
        aInvalPxRect = Rectangle( aPxPageRect.Left(), aPxPageRect.Bottom()-1,
                                  aPxPageRect.Right(), aPxPageRect.Bottom() );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
        // invalidate left mark line
        aInvalPxRect = Rectangle( aPxPageRect.Left(), aPxPageRect.Top(),
                                  aPxPageRect.Left()+1, aPxPageRect.Bottom() );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
    }

    // re-paint for new selected page in order to mark it.
    const PreviewPage* pNewSelectedPreviewPage = _GetPreviewPageByPageNum( _nSelectedPage );
    if ( pNewSelectedPreviewPage && pNewSelectedPreviewPage->bVisible )
    {
        const PreviewPage* pSelectedPreviewPage = _GetPreviewPageByPageNum(mnSelectedPageNum);
        SwRect aPageRect(pSelectedPreviewPage->aPreviewWinPos, pSelectedPreviewPage->aPageSize);
        ::SwAlignRect(aPageRect, &mrParentViewShell, pOutputDev);
        mrParentViewShell.GetWin()->Invalidate(aPageRect.SVRect());
    }
}

// helper methods

/** get preview page by physical page number

    OD 17.12.2002 #103492#
*/
struct EqualsPageNumPred
{
    const sal_uInt16 mnPageNum;
    explicit EqualsPageNumPred(const sal_uInt16 _nPageNum)
        : mnPageNum( _nPageNum )
    {}
    bool operator() ( const PreviewPage* _pPreviewPage )
    {
        return _pPreviewPage->pPage->GetPhyPageNum() == mnPageNum;
    }
};

const PreviewPage* SwPagePreviewLayout::_GetPreviewPageByPageNum( const sal_uInt16 _nPageNum ) const
{
    std::vector<PreviewPage*>::const_iterator aFoundPreviewPageIter =
            std::find_if( maPreviewPages.begin(), maPreviewPages.end(),
                          EqualsPageNumPred( _nPageNum ) );

    if ( aFoundPreviewPageIter == maPreviewPages.end() )
        return nullptr;

    return (*aFoundPreviewPageIter);
}

/** determine row the page with the given number is in

    OD 17.01.2003 #103492#

    @note _nPageNum is relative
*/
sal_uInt16 SwPagePreviewLayout::GetRowOfPage( sal_uInt16 _nPageNum ) const
{
    // OD 19.02.2003 #107369# - leaving left-top-corner blank is controlled
    // by <mbBookPreview>.
    if ( mbBookPreview )
    {
        // Note: increase given physical page number by one, because left-top-corner
        //       in the preview layout is left blank.
        ++_nPageNum;
    }

    return _nPageNum / mnCols + ((_nPageNum % mnCols)>0 ? 1 : 0);
}

/** determine column the page with the given number is in

    OD 17.01.2003 #103492#

    @note _nPageNum is relative
*/
sal_uInt16 SwPagePreviewLayout::GetColOfPage( sal_uInt16 _nPageNum ) const
{
    // OD 19.02.2003 #107369# - leaving left-top-corner blank is controlled
    // by <mbBookPreview>.
    if ( mbBookPreview )
    {
        // Note: increase given physical page number by one, because left-top-corner
        //       in the preview layout is left blank.
        ++_nPageNum;
    }

    const sal_uInt16 nCol = _nPageNum % mnCols;
    return nCol ? nCol : mnCols;
}

Size SwPagePreviewLayout::GetPreviewDocSize() const
{
    OSL_ENSURE( PreviewLayoutValid(), "PagePreviewLayout not valid" );
    return maPreviewDocRect.GetSize();
}

/** get size of a preview page by its physical page number

    OD 15.01.2003 #103492#
*/
Size SwPagePreviewLayout::GetPreviewPageSizeByPageNum( sal_uInt16 _nPageNum ) const
{
    const PreviewPage* pPreviewPage = _GetPreviewPageByPageNum( _nPageNum );
    if ( pPreviewPage )
    {
        return pPreviewPage->aPageSize;
    }
    return Size( 0, 0 );
}

/** get virtual page number by its physical page number

    OD 21.03.2003 #108282#
*/
sal_uInt16 SwPagePreviewLayout::GetVirtPageNumByPageNum( sal_uInt16 _nPageNum ) const
{
    const PreviewPage* pPreviewPage = _GetPreviewPageByPageNum( _nPageNum );
    if ( pPreviewPage )
    {
        return pPreviewPage->pPage->GetVirtPageNum();
    }
    return 0;
}

/** Convert absolute to relative page numbers (see PrintEmptyPages) */
sal_uInt16 SwPagePreviewLayout::ConvertAbsoluteToRelativePageNum( sal_uInt16 _nAbsPageNum ) const
{
    if ( mbBookPreview || mbPrintEmptyPages || !_nAbsPageNum )
    {
        return _nAbsPageNum;
    }

    const SwPageFrame* pTmpPage = static_cast<const SwPageFrame*>(mrLayoutRootFrame.Lower());

    sal_uInt16 nRet = 1;

    while ( pTmpPage && pTmpPage->GetPhyPageNum() != _nAbsPageNum )
    {
        if ( !pTmpPage->IsEmptyPage() )
            ++nRet;

        pTmpPage = static_cast<const SwPageFrame*>( pTmpPage->GetNext() );
    }

    return nRet;
}

/** Convert relative to absolute page numbers (see PrintEmptyPages) */
sal_uInt16 SwPagePreviewLayout::ConvertRelativeToAbsolutePageNum( sal_uInt16 _nRelPageNum ) const
{
    if ( mbBookPreview || mbPrintEmptyPages || !_nRelPageNum )
    {
        return _nRelPageNum;
    }

    const SwPageFrame* pTmpPage = static_cast<const SwPageFrame*>(mrLayoutRootFrame.Lower());
    const SwPageFrame* pRet = nullptr;

    sal_uInt16 i = 0;
    while( pTmpPage && i != _nRelPageNum )
    {
        if ( !pTmpPage->IsEmptyPage() )
            ++i;

        pRet = pTmpPage;
        pTmpPage = static_cast<const SwPageFrame*>( pTmpPage->GetNext() );
    }

    return pRet->GetPhyPageNum();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
