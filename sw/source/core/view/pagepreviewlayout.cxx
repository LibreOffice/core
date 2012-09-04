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

#include "vcl/svapp.hxx"

#include <pagepreviewlayout.hxx>
#include <prevwpage.hxx>

#include <algorithm>
#include <vcl/window.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <viewsh.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <swregion.hxx>
#include <comcore.hrc>
// OD 19.02.2003 #107369# - method <SwAlignRect(..)>
#include <frmtool.hxx>
// OD 24.09.2003 #i19975#
#include <sfx2/zoomitem.hxx>
#include <printdata.hxx>

#include <IDocumentDeviceAccess.hxx>

// OD 20.02.2003 #107369# - method to update statics for paint
// Note: method defined in '/sw/source/core/layout/paintfrm.cxx'
extern void SwCalcPixStatics( OutputDevice *pOut );

// =============================================================================
// methods to initialize page preview layout
// =============================================================================
SwPagePreviewLayout::SwPagePreviewLayout( ViewShell& _rParentViewShell,
                                          const SwRootFrm& _rLayoutRootFrm )
    : mnXFree ( 4*142 ),
      mnYFree ( 4*142 ),
      mrParentViewShell( _rParentViewShell ),
      mrLayoutRootFrm ( _rLayoutRootFrm )
{
    _Clear();

    // OD 2004-03-05 #i18143#
    mbBookPreview = false;
    mbBookPreviewModeToggled = false;

    mbPrintEmptyPages = mrParentViewShell.getIDocumentDeviceAccess()->getPrintData().IsPrintEmptyPages();
}

void SwPagePreviewLayout::_Clear()
{
    mbLayoutInfoValid = mbLayoutSizesValid = mbPaintInfoValid = false;

    maWinSize.Width() = 0;
    maWinSize.Height() = 0;
    mnCols = mnRows = 0;

    _ClearPrevwLayoutSizes();

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
    maPaintedPrevwDocRect.Left() = 0;
    maPaintedPrevwDocRect.Top() = 0;
    maPaintedPrevwDocRect.Right() = 0;
    maPaintedPrevwDocRect.Bottom() = 0;
    mnSelectedPageNum = 0;
    _ClearPrevwPageData();

    // OD 07.11.2003 #i22014#
    mbInPaint = false;
    mbNewLayoutDuringPaint = false;
}

void SwPagePreviewLayout::_ClearPrevwLayoutSizes()
{
    mnPages = 0;

    maMaxPageSize.Width() = 0;
    maMaxPageSize.Height() = 0;
    maPreviewDocRect.Left() = maPreviewDocRect.Top() = 0;
    maPreviewDocRect.Right() = maPreviewDocRect.Bottom() = 0;
    mnColWidth = mnRowHeight = 0;
    mnPrevwLayoutWidth = mnPrevwLayoutHeight = 0;
}

void SwPagePreviewLayout::_ClearPrevwPageData()
{
    for ( std::vector<PrevwPage*>::iterator aPageDelIter = maPrevwPages.begin();
          aPageDelIter != maPrevwPages.end();
          ++aPageDelIter )
    {
        delete (*aPageDelIter);
    }
    maPrevwPages.clear();
}

/** calculate page preview layout sizes

    OD 18.12.2002 #103492#

    @author OD
*/
void SwPagePreviewLayout::_CalcPrevwLayoutSizes()
{
    // calculate maximal page size; calculate also number of pages

    const SwPageFrm* pPage = static_cast<const SwPageFrm*>(mrLayoutRootFrm.Lower());
    while ( pPage )
    {
        if ( !mbBookPreview && !mbPrintEmptyPages && pPage->IsEmptyPage() )
        {
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
            continue;
        }

        ++mnPages;
        pPage->Calc();
        const Size& rPageSize = pPage->Frm().SSize();
        if ( rPageSize.Width() > maMaxPageSize.Width() )
            maMaxPageSize.Width() = rPageSize.Width();
        if ( rPageSize.Height() > maMaxPageSize.Height() )
            maMaxPageSize.Height() = rPageSize.Height();
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
    }
    // calculate and set column width and row height
    mnColWidth = maMaxPageSize.Width() + mnXFree;
    mnRowHeight = maMaxPageSize.Height() + mnYFree;

    // calculate and set preview layout width and height
    mnPrevwLayoutWidth = mnCols * mnColWidth + mnXFree;
    mnPrevwLayoutHeight = mnRows * mnRowHeight + mnYFree;

    // calculate document rectangle in preview layout
    {
        Size aDocSize;
        // document width
        aDocSize.Width() = mnPrevwLayoutWidth;

        // document height
        // determine number of rows needed for <nPages> in preview layout
        // OD 19.02.2003 #107369# - use method <GetRowOfPage(..)>.
        sal_uInt16 nDocRows = GetRowOfPage( mnPages );
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

    @author OD
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

    _CalcPrevwLayoutSizes();

    // validate layout information
    mbLayoutInfoValid = true;

    if ( _bCalcScale )
    {
        // calculate scaling
        MapMode aMapMode( MAP_TWIP );
        Size aWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize, aMapMode );
        Fraction aXScale( aWinSize.Width(), mnPrevwLayoutWidth );
        Fraction aYScale( aWinSize.Height(), mnPrevwLayoutHeight );
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

/** apply new zoom at given view shell

    OD 11.12.2002 #103492# - implementation of <_ApplyNewZoomAtViewShell>

    @author OD
*/
void SwPagePreviewLayout::_ApplyNewZoomAtViewShell( sal_uInt8 _aNewZoom )
{
    SwViewOption aNewViewOptions = *(mrParentViewShell.GetViewOptions());
    if ( aNewViewOptions.GetZoom() != _aNewZoom )
    {
        aNewViewOptions.SetZoom( _aNewZoom );
        //#i19975# - consider zoom type.
        enum SvxZoomType eZoomType = SVX_ZOOM_PERCENT;
        aNewViewOptions.SetZoomType( eZoomType );
        mrParentViewShell.ApplyViewOptions( aNewViewOptions );
    }
}

/** method to adjust page preview layout to document changes

    OD 18.12.2002 #103492#

    @author OD
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

    _ClearPrevwLayoutSizes();
    _CalcPrevwLayoutSizes();

    return true;
}

// =============================================================================
// methods to prepare paint of page preview
// =============================================================================
/** prepare paint of page preview

    OD 12.12.2002 #103492#
    OD 21.03.2003 #108282# - delete parameter _onStartPageVirtNum

    @author OD, _nProposedStartPageNum, _onStartPageNum are absolute
*/
bool SwPagePreviewLayout::Prepare( const sal_uInt16 _nProposedStartPageNum,
                                   const Point      _aProposedStartPos,
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
                _aProposedStartPos.X() >= 0 && _aProposedStartPos.Y() >= 0 &&
                _aProposedStartPos.X() <= maPreviewDocRect.Right() &&
                _aProposedStartPos.Y() <= maPreviewDocRect.Bottom();
        OSL_ENSURE( bStartPosRangeValid,
                "proposed start position out of range - no prepare of preview paint");
        if ( !bStartPosRangeValid )
            return false;

        bool bWinSizeValid = _rPxWinSize.Width() != 0 && _rPxWinSize.Height() != 0;
       OSL_ENSURE( bWinSizeValid, "no window size - no prepare of preview paint");
        if ( !bWinSizeValid )
            return false;

        bool bStartInfoValid = _nProposedStartPageNum > 0 ||
                               _aProposedStartPos != Point(0,0);
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
        sal_uInt16 nColOfProposed = GetColOfPage( nProposedStartPageNum );
        sal_uInt16 nRowOfProposed = GetRowOfPage( nProposedStartPageNum );
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
        sal_uInt16 nColOfProposed =
                static_cast<sal_uInt16>(_aProposedStartPos.X() / mnColWidth) + 1;
        sal_uInt16 nRowOfProposed =
                static_cast<sal_uInt16>(_aProposedStartPos.Y() / mnRowHeight) + 1;
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
                (_aProposedStartPos.X() % mnColWidth) - mnXFree;
        maPaintStartPageOffset.Y() =
                (_aProposedStartPos.Y() % mnRowHeight) - mnYFree;
        // virtual preview document offset.
        maPaintPreviewDocOffset = _aProposedStartPos;
    }

    // determine additional paint offset, if preview layout fits into window.
    _CalcAdditionalPaintOffset();

    // determine rectangle to be painted from document preview
    _CalcDocPrevwPaintRect();
    _orDocPreviewPaintRect = maPaintedPrevwDocRect;

    // OD 20.01.2003 #103492# - shift visible preview document area to the left,
    // if on the right is an area left blank.
    if ( !mbDoesLayoutColsFitIntoWindow &&
         maPaintedPrevwDocRect.GetWidth() < maWinSize.Width() )
    {
        maPaintedPrevwDocRect.Move(
                -(maWinSize.Width() - maPaintedPrevwDocRect.GetWidth()), 0 );
        Prepare( 0, maPaintedPrevwDocRect.TopLeft(),
                 _rPxWinSize, _onStartPageNum,
                 _orDocPreviewPaintRect, _bStartWithPageAtFirstCol );
    }

    // OD 20.01.2003 #103492# - shift visible preview document area to the top,
    // if on the botton is an area left blank.
    if ( mbBookPreviewModeToggled &&
         maPaintedPrevwDocRect.Bottom() == maPreviewDocRect.Bottom() &&
         maPaintedPrevwDocRect.GetHeight() < maWinSize.Height() )
    {
        if ( mbDoesLayoutRowsFitIntoWindow )
        {
            if ( maPaintedPrevwDocRect.GetHeight() < mnPrevwLayoutHeight)
            {
                maPaintedPrevwDocRect.Move(
                        0, -(mnPrevwLayoutHeight - maPaintedPrevwDocRect.GetHeight()) );
                Prepare( 0, maPaintedPrevwDocRect.TopLeft(),
                         _rPxWinSize, _onStartPageNum,
                         _orDocPreviewPaintRect, _bStartWithPageAtFirstCol );
            }
        }
        else
        {
            maPaintedPrevwDocRect.Move(
                    0, -(maWinSize.Height() - maPaintedPrevwDocRect.GetHeight()) );
            Prepare( 0, maPaintedPrevwDocRect.TopLeft(),
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

    @author OD
*/
void SwPagePreviewLayout::_CalcAdditionalPaintOffset()
{
    if ( mnPrevwLayoutWidth <= maWinSize.Width() &&
         maPaintStartPageOffset.X() <= 0 )
    {
        mbDoesLayoutColsFitIntoWindow = true;
        maAdditionalPaintOffset.X() = (maWinSize.Width() - mnPrevwLayoutWidth) / 2;
    }
    else
    {
        mbDoesLayoutColsFitIntoWindow = false;
        maAdditionalPaintOffset.X() = 0;
    }

    if ( mnPrevwLayoutHeight <= maWinSize.Height() &&
         maPaintStartPageOffset.Y() <= 0 )
    {
        mbDoesLayoutRowsFitIntoWindow = true;
        maAdditionalPaintOffset.Y() = (maWinSize.Height() - mnPrevwLayoutHeight) / 2;
    }
    else
    {
        mbDoesLayoutRowsFitIntoWindow = false;
        maAdditionalPaintOffset.Y() = 0;
    }
}

/** calculate painted preview document rectangle

    OD 12.12.2002 #103492#

    @author OD
*/
void SwPagePreviewLayout::_CalcDocPrevwPaintRect()
{
    Point aTopLeftPos = maPaintPreviewDocOffset;
    maPaintedPrevwDocRect.SetPos( aTopLeftPos );

    Size aSize;
    if ( mbDoesLayoutColsFitIntoWindow )
        //aSize.Width() = mnPrevwLayoutWidth;
        aSize.Width() = Min( mnPrevwLayoutWidth,
                             maPreviewDocRect.GetWidth() - aTopLeftPos.X() );
    else
        aSize.Width() = Min( maPreviewDocRect.GetWidth() - aTopLeftPos.X(),
                             maWinSize.Width() - maAdditionalPaintOffset.X() );
    if ( mbDoesLayoutRowsFitIntoWindow )
        //aSize.Height() = mnPrevwLayoutHeight;
        aSize.Height() = Min( mnPrevwLayoutHeight,
                              maPreviewDocRect.GetHeight() - aTopLeftPos.Y() );
    else
        aSize.Height() = Min( maPreviewDocRect.GetHeight() - aTopLeftPos.Y(),
                              maWinSize.Height() - maAdditionalPaintOffset.Y() );
    maPaintedPrevwDocRect.SetSize( aSize );
}

/** calculate preview pages

    OD 12.12.2002 #103492#

    @author OD
*/
void SwPagePreviewLayout::_CalcPreviewPages()
{
    _ClearPrevwPageData();

    if ( mbNoPageVisible )
        return;

    // determine start page frame
    const SwPageFrm* pStartPage = mrLayoutRootFrm.GetPageByPageNum( mnPaintPhyStartPageNum );

    // calculate initial paint offset
    Point aInitialPaintOffset;
    /// check whether RTL interface or not
    if(!Application::GetSettings().GetLayoutRTL()){
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
    const SwPageFrm* pPage = pStartPage;
    sal_uInt16 nCurrCol = mnPaintStartCol;
    sal_uInt16 nConsideredRows = 0;
    Point aCurrPaintOffset = aInitialPaintOffset;
    // loop on pages to determine preview background retangles
    while ( pPage &&
            (!mbDoesLayoutRowsFitIntoWindow || nConsideredRows < mnRows) &&
            aCurrPaintOffset.Y() < maWinSize.Height()
          )
    {
        if ( !mbBookPreview && !mbPrintEmptyPages && pPage->IsEmptyPage() )
        {
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
            continue;
        }

        pPage->Calc();

        // consider only pages, which have to be painted.
        if ( nCurrCol < mnPaintStartCol )
        {
            // calculate data of unvisible page needed for accessibility
            PrevwPage* pPrevwPage = new PrevwPage;
            Point aCurrAccOffset = aCurrPaintOffset -
                           Point( (mnPaintStartCol-nCurrCol) * mnColWidth, 0 );
            _CalcPreviewDataForPage( *(pPage), aCurrAccOffset, pPrevwPage );
            pPrevwPage->bVisible = false;
            maPrevwPages.push_back( pPrevwPage );
            // continue with next page and next column
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
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
                if(!Application::GetSettings().GetLayoutRTL())
                    aCurrPaintOffset.X() += mnColWidth;
                else aCurrPaintOffset.X() -= mnColWidth;
                ++nCurrCol;
                continue;
            }

            // calculate data of visible page
            PrevwPage* pPrevwPage = new PrevwPage;
            _CalcPreviewDataForPage( *(pPage), aCurrPaintOffset, pPrevwPage );
            pPrevwPage->bVisible = true;
            maPrevwPages.push_back( pPrevwPage );
        }
        else
        {
            // calculate data of unvisible page needed for accessibility
            PrevwPage* pPrevwPage = new PrevwPage;
            _CalcPreviewDataForPage( *(pPage), aCurrPaintOffset, pPrevwPage );
            pPrevwPage->bVisible = false;
            maPrevwPages.push_back( pPrevwPage );
        }

        // prepare data for next loop
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());

        /// check whether RTL interface or not
        if(!Application::GetSettings().GetLayoutRTL())
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

    @author OD
*/
bool SwPagePreviewLayout::_CalcPreviewDataForPage( const SwPageFrm& _rPage,
                                                   const Point& _rPrevwOffset,
                                                   PrevwPage* _opPrevwPage )
{
    // page frame
    _opPrevwPage->pPage = &_rPage;
    // size of page frame
    if ( _rPage.IsEmptyPage() )
    {
        if ( _rPage.GetPhyPageNum() % 2 == 0 )
            _opPrevwPage->aPageSize = _rPage.GetPrev()->Frm().SSize();
        else
            _opPrevwPage->aPageSize = _rPage.GetNext()->Frm().SSize();
    }
    else
        _opPrevwPage->aPageSize = _rPage.Frm().SSize();
    // position of page in preview window
    Point aPrevwWinOffset( _rPrevwOffset );
    if ( _opPrevwPage->aPageSize.Width() < maMaxPageSize.Width() )
        aPrevwWinOffset.X() += ( maMaxPageSize.Width() - _opPrevwPage->aPageSize.Width() ) / 2;
    if ( _opPrevwPage->aPageSize.Height() < maMaxPageSize.Height() )
        aPrevwWinOffset.Y() += ( maMaxPageSize.Height() - _opPrevwPage->aPageSize.Height() ) / 2;
    _opPrevwPage->aPrevwWinPos = aPrevwWinOffset;
    // logic position of page and mapping offset for paint
    if ( _rPage.IsEmptyPage() )
    {
        _opPrevwPage->aLogicPos = _opPrevwPage->aPrevwWinPos;
        _opPrevwPage->aMapOffset = Point( 0, 0 );
    }
    else
    {
        _opPrevwPage->aLogicPos = _rPage.Frm().Pos();
        _opPrevwPage->aMapOffset = _opPrevwPage->aPrevwWinPos - _opPrevwPage->aLogicPos;
    }

    return true;
}

/** enable/disable book preview

    OD 2004-03-04 #i18143#

    @author OD
*/
bool SwPagePreviewLayout::SetBookPreviewMode( const bool _bEnableBookPreview,
                                              sal_uInt16& _onStartPageNum,
                                              Rectangle&  _orDocPreviewPaintRect )
{
    bool bRet = false;

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

        bRet = true;
    }

    return bRet;
}

// =============================================================================
// methods to determine new data for changing the current shown part of the
// document preview.
// =============================================================================
/** calculate start position for new scale

    OD 12.12.2002 #103492#

    @author OD
*/
Point SwPagePreviewLayout::GetPreviewStartPosForNewScale(
                          const Fraction& _aNewScale,
                          const Fraction& _aOldScale,
                          const Size&     _aNewWinSize ) const
{
    Point aNewPaintStartPos = maPaintedPrevwDocRect.TopLeft();
    if ( _aNewScale < _aOldScale )
    {
        // increase paint width by moving start point to left.
        if ( mnPrevwLayoutWidth < _aNewWinSize.Width() )
            aNewPaintStartPos.X() = 0;
        else if ( maPaintedPrevwDocRect.GetWidth() < _aNewWinSize.Width() )
        {
            aNewPaintStartPos.X() -=
                (_aNewWinSize.Width() - maPaintedPrevwDocRect.GetWidth()) / 2;
            if ( aNewPaintStartPos.X() < 0)
                aNewPaintStartPos.X() = 0;
        }

        if ( !mbDoesLayoutRowsFitIntoWindow )
        {
            // increase paint height by moving start point to top.
            if ( mnPrevwLayoutHeight < _aNewWinSize.Height() )
            {
                aNewPaintStartPos.Y() =
                    ( (mnPaintStartRow - 1) * mnRowHeight );
            }
            else if ( maPaintedPrevwDocRect.GetHeight() < _aNewWinSize.Height() )
            {
                aNewPaintStartPos.Y() -=
                    (_aNewWinSize.Height() - maPaintedPrevwDocRect.GetHeight()) / 2;
                if ( aNewPaintStartPos.Y() < 0)
                    aNewPaintStartPos.Y() = 0;
            }
        }
    }
    else
    {
        // decrease paint width by moving start point to right
        if ( maPaintedPrevwDocRect.GetWidth() > _aNewWinSize.Width() )
            aNewPaintStartPos.X() +=
                (maPaintedPrevwDocRect.GetWidth() - _aNewWinSize.Width()) / 2;
        // decrease paint height by moving start point to bottom
        if ( maPaintedPrevwDocRect.GetHeight() > _aNewWinSize.Height() )
        {
            aNewPaintStartPos.Y() +=
                (maPaintedPrevwDocRect.GetHeight() - _aNewWinSize.Height()) / 2;
            // check, if new y-position is outside document preview
            if ( aNewPaintStartPos.Y() > maPreviewDocRect.Bottom() )
                aNewPaintStartPos.Y() =
                        Max( 0L, maPreviewDocRect.Bottom() - mnPrevwLayoutHeight );
        }
    }

    return aNewPaintStartPos;
}

/** determines, if page with given page number is visible in preview

    @author OD, _nPageNum is absolut!
*/
bool SwPagePreviewLayout::IsPageVisible( const sal_uInt16 _nPageNum ) const
{
    const PrevwPage* pPrevwPage = _GetPrevwPageByPageNum( _nPageNum );
    return pPrevwPage && pPrevwPage->bVisible;
}

/** calculate data to bring new selected page into view.

    @author OD, IN/OUT parameters are absolute page numbers!!!
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

    // leaving left-top-corner blank is controlled
    // by <mbBookPreview>.
    if ( mbBookPreview )
    {
        // Note: consider that left-top-corner is left blank --> +1
        ++nTmpRelSelPageNum;
    }
    sal_uInt16 nTmpCol = nTmpRelSelPageNum % mnCols;
    sal_uInt16 nCurrRow = nTmpRelSelPageNum / mnCols;
    if ( nTmpCol > 0 )
        ++nCurrRow;

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

    sal_uInt16 nNewAbsSelectedPageNum = ConvertRelativeToAbsolutePageNum( nNewRelSelectedPageNum );
    if ( !IsPageVisible( nNewAbsSelectedPageNum ) )
    {
        if ( _nHoriMove != 0 && _nVertMove != 0 )
        {
            OSL_FAIL( "missing implementation for moving preview selected page horizontal AND vertical");
            return false;
        }

        // new selected page has to be brought into view considering current
        // visible preview.
        sal_Int16 nTotalRows = GetRowOfPage( mnPages );
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

/** checks, if given position is inside a shown document page

    @author OD
*/
struct PrevwPosInsidePagePred
{
    const Point mnPrevwPos;
    PrevwPosInsidePagePred( const Point _nPrevwPos ) : mnPrevwPos( _nPrevwPos ) {};
    bool operator() ( const PrevwPage* _pPrevwPage )
    {
        if ( _pPrevwPage->bVisible )
        {
            Rectangle aPrevwPageRect( _pPrevwPage->aPrevwWinPos, _pPrevwPage->aPageSize );
            return aPrevwPageRect.IsInside( mnPrevwPos ) ? true : false;
        }
        else
            return false;
    }
};

bool SwPagePreviewLayout::IsPrevwPosInDocPrevwPage( const Point  _aPrevwPos,
                                                    Point&       _orDocPos,
                                                    bool&        _obPosInEmptyPage,
                                                    sal_uInt16&  _onPageNum ) const
{
    bool bIsPosInsideDoc;

    // initialize variable parameter values.
    _orDocPos.X() = 0;
    _orDocPos.Y() = 0;
    _obPosInEmptyPage = false;
    _onPageNum = 0;

    std::vector<PrevwPage*>::const_iterator aFoundPrevwPageIter =
            std::find_if( maPrevwPages.begin(), maPrevwPages.end(),
                          PrevwPosInsidePagePred( _aPrevwPos ) );

    if ( aFoundPrevwPageIter == maPrevwPages.end() )
        // given preview position outside a document page.
        bIsPosInsideDoc = false;
    else
    {
        _onPageNum = (*aFoundPrevwPageIter)->pPage->GetPhyPageNum();
        if ( (*aFoundPrevwPageIter)->pPage->IsEmptyPage() )
        {
            // given preview position inside an empty page
            bIsPosInsideDoc = false;
            _obPosInEmptyPage = true;
        }
        else
        {
            // given preview position inside a normal page
            bIsPosInsideDoc = true;
            _orDocPos = _aPrevwPos -
                        (*aFoundPrevwPageIter)->aPrevwWinPos +
                        (*aFoundPrevwPageIter)->aLogicPos;
        }
    }

    return bIsPosInsideDoc;
}

/** determine window page scroll amount

    @author OD
*/
SwTwips SwPagePreviewLayout::GetWinPagesScrollAmount(
                                const sal_Int16 _nWinPagesToScroll ) const
{
    SwTwips nScrollAmount;
    if ( mbDoesLayoutRowsFitIntoWindow )
    {
        nScrollAmount = (mnPrevwLayoutHeight - mnYFree) * _nWinPagesToScroll;
    }
    else
        nScrollAmount = _nWinPagesToScroll * maPaintedPrevwDocRect.GetHeight();

    // check, if preview layout size values are valid.
    // If not, the checks for an adjustment of the scroll amount aren't useful.
    if ( mbLayoutSizesValid )
    {
        if ( (maPaintedPrevwDocRect.Top() + nScrollAmount) <= 0 )
            nScrollAmount = -maPaintedPrevwDocRect.Top();

        // correct scroll amount
        if ( nScrollAmount > 0 &&
             maPaintedPrevwDocRect.Bottom() == maPreviewDocRect.Bottom()
           )
        {
            nScrollAmount = 0;
        }
        else
        {
            while ( (maPaintedPrevwDocRect.Top() + nScrollAmount + mnYFree) >= maPreviewDocRect.GetHeight() )
            {
                nScrollAmount -= mnRowHeight;
            }
        }
    }

    return nScrollAmount;
}

// =============================================================================
// methods to paint page preview layout
// =============================================================================
/** paint prepared preview

    OD 12.12.2002 #103492#

    @author OD
*/
bool SwPagePreviewLayout::Paint( const Rectangle  _aOutRect ) const
{
    // check environment and parameters
    {
        if ( !mrParentViewShell.GetWin() &&
             !mrParentViewShell.GetOut()->GetConnectMetaFile() )
            return false;

        OSL_ENSURE( mbPaintInfoValid,
                "invalid preview settings - no paint of preview" );
        if ( !mbPaintInfoValid )
            return false;
    }

    // OD 17.11.2003 #i22014# - no paint, if <superfluous> flag is set at layout
    if ( mrLayoutRootFrm.IsSuperfluous() )
    {
        return true;
    }

    // environment and parameter ok

    // OD 07.11.2003 #i22014#
    if ( mbInPaint )
    {
        return false;
    }
    mbInPaint = true;

    OutputDevice* pOutputDev = mrParentViewShell.GetOut();

    // prepare paint
    if ( maPrevwPages.size() > 0 )
    {
        mrParentViewShell.Imp()->bFirstPageInvalid = sal_False;
        mrParentViewShell.Imp()->pFirstVisPage =
                const_cast<SwPageFrm*>(maPrevwPages[0]->pPage);
    }

    // paint preview background
    {
        SwRegionRects aPreviewBackgrdRegion( _aOutRect );
        // calculate preview background rectangles
        for ( std::vector<PrevwPage*>::const_iterator aPageIter = maPrevwPages.begin();
              aPageIter != maPrevwPages.end();
              ++aPageIter )
        {
            if ( (*aPageIter)->bVisible )
            {
                aPreviewBackgrdRegion -=
                        SwRect( (*aPageIter)->aPrevwWinPos, (*aPageIter)->aPageSize );
            }
        }
        // paint preview background rectangles
        mrParentViewShell._PaintDesktop( aPreviewBackgrdRegion );
    }

    // prepare data for paint of pages
    const Rectangle aPxOutRect( pOutputDev->LogicToPixel( _aOutRect ) );

    MapMode aMapMode( pOutputDev->GetMapMode() );
    MapMode aSavedMapMode = aMapMode;

    const Font& rEmptyPgFont = SwPageFrm::GetEmptyPageFont();

    for ( std::vector<PrevwPage*>::const_iterator aPageIter = maPrevwPages.begin();
          aPageIter != maPrevwPages.end();
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
                    ::SwAlignRect( aTmpPageRect, &mrParentViewShell);
                    aPageRect = aTmpPageRect.SVRect();
                }
                pOutputDev->DrawRect( aPageRect );

                // paint empty page text
                Font aOldFont( pOutputDev->GetFont() );
                pOutputDev->SetFont( rEmptyPgFont );
                pOutputDev->DrawText( aPageRect, SW_RESSTR( STR_EMPTYPAGE ),
                                    TEXT_DRAW_VCENTER |
                                    TEXT_DRAW_CENTER |
                                    TEXT_DRAW_CLIP );
                pOutputDev->SetFont( aOldFont );
                // paint shadow and border for empty page
                // OD 19.02.2003 #107369# - use new method to paint page border and shadow
                SwPageFrm::PaintBorderAndShadow( aPageRect, &mrParentViewShell, true, false, true );
            }
            else
            {
                mrParentViewShell.aVisArea = aPageRect;
                aPxPaintRect.Intersection( aPxOutRect );
                Rectangle aPaintRect = pOutputDev->PixelToLogic( aPxPaintRect );
                mrParentViewShell.Paint( aPaintRect );
                // --> OD 2007-08-15 #i80691#
                // paint page border and shadow
                {
                    SwRect aPageBorderRect;
                    SwPageFrm::GetBorderAndShadowBoundRect( SwRect( aPageRect ), &mrParentViewShell, aPageBorderRect,
                        (*aPageIter)->pPage->IsLeftShadowNeeded(), (*aPageIter)->pPage->IsRightShadowNeeded(), true );
                    const Region aDLRegion(aPageBorderRect.SVRect());
                    mrParentViewShell.DLPrePaint2(aDLRegion);
                    SwPageFrm::PaintBorderAndShadow( aPageRect, &mrParentViewShell, true, false, true );
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
                _PaintSelectMarkAtPage( (*aPageIter) );
            }

        }
    }

    // OD 17.11.2003 #i22014# - no update of accessible preview, if a new
    // print preview layout is created during paint.
    if ( !mbNewLayoutDuringPaint )
    {
        // update at accessiblilty interface
        mrParentViewShell.Imp()->UpdateAccessiblePreview(
                        maPrevwPages,
                        aMapMode.GetScaleX(),
                        mrLayoutRootFrm.GetPageByPageNum( mnSelectedPageNum ),
                        maWinSize );
    }

    pOutputDev->SetMapMode( aSavedMapMode );
    mrParentViewShell.aVisArea.Clear();

    // OD 07.11.2003 #i22014#
    mbInPaint = false;
    mbNewLayoutDuringPaint = false;

    return true;
}

/** repaint pages on page preview

    OD 18.12.2002 #103492#

    @author OD
*/
void SwPagePreviewLayout::Repaint( const Rectangle _aInvalidCoreRect ) const
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
    if ( maPrevwPages.size() > 0 )
    {
        mrParentViewShell.Imp()->bFirstPageInvalid = sal_False;
        mrParentViewShell.Imp()->pFirstVisPage =
                const_cast<SwPageFrm*>(maPrevwPages[0]->pPage);
    }

    // invalidate visible pages, which overlap the invalid core rectangle
    for ( std::vector<PrevwPage*>::const_iterator aPageIter = maPrevwPages.begin();
          aPageIter != maPrevwPages.end();
          ++aPageIter )
    {
        if ( !(*aPageIter)->bVisible )
            continue;

        Rectangle aPageRect( (*aPageIter)->aLogicPos, (*aPageIter)->aPageSize );
        if ( _aInvalidCoreRect.IsOver( aPageRect ) )
        {
            aPageRect.Intersection( _aInvalidCoreRect );
            Rectangle aInvalidPrevwRect = aPageRect;
            aInvalidPrevwRect.SetPos( aInvalidPrevwRect.TopLeft() -
                                      (*aPageIter)->aLogicPos +
                                      (*aPageIter)->aPrevwWinPos );
            mrParentViewShell.GetWin()->Invalidate( aInvalidPrevwRect );
        }
    }
}

/** paint selection mark at page

    OD 17.12.2002 #103492#

    @author OD
*/
void SwPagePreviewLayout::_PaintSelectMarkAtPage(
                                    const PrevwPage* _aSelectedPrevwPage ) const
{
    OutputDevice* pOutputDev = mrParentViewShell.GetOut();
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
    aMapMode.SetOrigin( _aSelectedPrevwPage->aMapOffset );
    pOutputDev->SetMapMode( aMapMode );

    // calculate page rectangle in pixel coordinates
    SwRect aPageRect( _aSelectedPrevwPage->aLogicPos,
                         _aSelectedPrevwPage->aPageSize );
    // OD 19.02.2003 #107369# - use aligned page rectangle, as it is used for
    // page border and shadow paint - see <SwPageFrm::PaintBorderAndShadow(..)>
    ::SwAlignRect( aPageRect, &mrParentViewShell);
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

    @author OD, _nSelectedPage, mnSelectedPage are absolut
*/
void SwPagePreviewLayout::MarkNewSelectedPage( const sal_uInt16 _nSelectedPage )
{
    sal_uInt16 nOldSelectedPageNum = mnSelectedPageNum;
    mnSelectedPageNum = _nSelectedPage;

    // re-paint for current selected page in order to umark it.
    const PrevwPage* pOldSelectedPrevwPage = _GetPrevwPageByPageNum( nOldSelectedPageNum );
    if ( pOldSelectedPrevwPage && pOldSelectedPrevwPage->bVisible )
    {
        // OD 20.02.2003 #107369# - invalidate only areas of selection mark.
        SwRect aPageRect( pOldSelectedPrevwPage->aPrevwWinPos,
                              pOldSelectedPrevwPage->aPageSize );
        ::SwAlignRect( aPageRect, &mrParentViewShell);
        OutputDevice* pOutputDev = mrParentViewShell.GetOut();
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
    const PrevwPage* pNewSelectedPrevwPage = _GetPrevwPageByPageNum( _nSelectedPage );
    if ( pNewSelectedPrevwPage && pNewSelectedPrevwPage->bVisible )
        _PaintSelectMarkAtPage( pNewSelectedPrevwPage );
}


// =============================================================================
// helper methods
// =============================================================================
/** get preview page by physical page number

    OD 17.12.2002 #103492#

    @author OD
*/
struct EqualsPageNumPred
{
    const sal_uInt16 mnPageNum;
    EqualsPageNumPred( const sal_uInt16 _nPageNum ) : mnPageNum( _nPageNum ) {};
    bool operator() ( const PrevwPage* _pPrevwPage )
    {
        return _pPrevwPage->pPage->GetPhyPageNum() == mnPageNum;
    }
};

const PrevwPage* SwPagePreviewLayout::_GetPrevwPageByPageNum( const sal_uInt16 _nPageNum ) const
{
    std::vector<PrevwPage*>::const_iterator aFoundPrevwPageIter =
            std::find_if( maPrevwPages.begin(), maPrevwPages.end(),
                          EqualsPageNumPred( _nPageNum ) );

    if ( aFoundPrevwPageIter == maPrevwPages.end() )
        return 0;
    else
        return (*aFoundPrevwPageIter);
}

/** determine row the page with the given number is in

    OD 17.01.2003 #103492#

    @author OD, _nPageNum is relative
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

    sal_uInt16 nRow = (_nPageNum) / mnCols;
    if ( ( (_nPageNum) % mnCols ) > 0 )
        ++nRow;

    return nRow;
}

/** determine column the page with the given number is in

    OD 17.01.2003 #103492#

    @author OD, _nPageNum is relative
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

    sal_uInt16 nCol = (_nPageNum) % mnCols;
    if ( nCol == 0 )
        nCol = mnCols;

    return nCol;
}

Size SwPagePreviewLayout::GetPrevwDocSize() const
{
    OSL_ENSURE( PreviewLayoutValid(), "PagePreviewLayout not valid" );
    return maPreviewDocRect.GetSize();
}

/** get size of a preview page by its physical page number

    OD 15.01.2003 #103492#

    @author OD
*/
Size SwPagePreviewLayout::GetPrevwPageSizeByPageNum( sal_uInt16 _nPageNum ) const
{
    const PrevwPage* pPrevwPage = _GetPrevwPageByPageNum( _nPageNum );
    if ( pPrevwPage )
    {
        return pPrevwPage->aPageSize;
    }
    else
    {
        return Size( 0, 0 );
    }
}

/** get virtual page number by its physical page number

    OD 21.03.2003 #108282#

    @author OD
*/
sal_uInt16 SwPagePreviewLayout::GetVirtPageNumByPageNum( sal_uInt16 _nPageNum ) const
{
    const PrevwPage* pPrevwPage = _GetPrevwPageByPageNum( _nPageNum );
    if ( pPrevwPage )
    {
        return pPrevwPage->pPage->GetVirtPageNum();
    }
    else
    {
        return 0;
    }
}

/** Convert absolute to relative page numbers (see PrintEmptyPages)

    @author FME
*/
sal_uInt16 SwPagePreviewLayout::ConvertAbsoluteToRelativePageNum( sal_uInt16 _nAbsPageNum ) const
{
    if ( mbBookPreview || mbPrintEmptyPages || !_nAbsPageNum )
    {
        return _nAbsPageNum;
    }

    const SwPageFrm* pTmpPage = static_cast<const SwPageFrm*>(mrLayoutRootFrm.Lower());

    sal_uInt16 nRet = 1;

    while ( pTmpPage && pTmpPage->GetPhyPageNum() != _nAbsPageNum )
    {
        if ( !pTmpPage->IsEmptyPage() )
            ++nRet;

        pTmpPage = static_cast<const SwPageFrm*>( pTmpPage->GetNext() );
    }

    return nRet;
}

/** Convert relative to absolute page numbers (see PrintEmptyPages)

    @author FME
*/
sal_uInt16 SwPagePreviewLayout::ConvertRelativeToAbsolutePageNum( sal_uInt16 _nRelPageNum ) const
{
    if ( mbBookPreview || mbPrintEmptyPages || !_nRelPageNum )
    {
        return _nRelPageNum;
    }

    const SwPageFrm* pTmpPage = static_cast<const SwPageFrm*>(mrLayoutRootFrm.Lower());
    const SwPageFrm* pRet = 0;

    sal_uInt16 i = 0;
    while( pTmpPage && i != _nRelPageNum )
    {
        if ( !pTmpPage->IsEmptyPage() )
            ++i;

        pRet = pTmpPage;
        pTmpPage = static_cast<const SwPageFrm*>( pTmpPage->GetNext() );
    }

    return pRet->GetPhyPageNum();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
