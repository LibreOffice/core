/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "vcl/svapp.hxx"

#include <pagepreviewlayout.hxx>
#include <prevwpage.hxx>

#include <algorithm>
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

#include <IDocumentDeviceAccess.hxx>



extern void SwCalcPixStatics( OutputDevice *pOut );



SwPagePreviewLayout::SwPagePreviewLayout( SwViewShell& _rParentViewShell,
                                          const SwRootFrm& _rLayoutRootFrm )
    : mnXFree ( 4*142 ),
      mnYFree ( 4*142 ),
      mrParentViewShell( _rParentViewShell ),
      mrLayoutRootFrm ( _rLayoutRootFrm )
{
    _Clear();

    
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
    
    mnColWidth = maMaxPageSize.Width() + mnXFree;
    mnRowHeight = maMaxPageSize.Height() + mnYFree;

    
    mnPreviewLayoutWidth = mnCols * mnColWidth + mnXFree;
    mnPreviewLayoutHeight = mnRows * mnRowHeight + mnYFree;

    
    {
        Size aDocSize;
        
        aDocSize.Width() = mnPreviewLayoutWidth;

        
        
        
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
*/
bool SwPagePreviewLayout::Init( const sal_uInt16 _nCols,
                                const sal_uInt16 _nRows,
                                const Size&      _rPxWinSize,
                                const bool       _bCalcScale
                              )
{
    
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

    

    
    _Clear();

    
    mnCols = _nCols;
    mnRows = _nRows;

    _CalcPreviewLayoutSizes();

    
    mbLayoutInfoValid = true;

    if ( _bCalcScale )
    {
        
        MapMode aMapMode( MAP_TWIP );
        Size aWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize, aMapMode );
        Fraction aXScale( aWinSize.Width(), mnPreviewLayoutWidth );
        Fraction aYScale( aWinSize.Height(), mnPreviewLayoutHeight );
        if( aXScale < aYScale )
            aYScale = aXScale;
        {
            
            aYScale *= Fraction( 1000, 1 );
            long nNewNuminator = aYScale.operator long();
            if( nNewNuminator < 1 )
                nNewNuminator = 1;
            aYScale = Fraction( nNewNuminator, 1000 );
            
            _ApplyNewZoomAtViewShell( static_cast<sal_uInt8>(nNewNuminator/10) );
        }
        aMapMode.SetScaleY( aYScale );
        aMapMode.SetScaleX( aYScale );
        
        mrParentViewShell.GetOut()->SetMapMode( aMapMode );
        
        ::SwCalcPixStatics( mrParentViewShell.GetOut() );
    }

    
    maWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize );
    
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
        
        enum SvxZoomType eZoomType = SVX_ZOOM_PERCENT;
        aNewViewOptions.SetZoomType( eZoomType );
        mrParentViewShell.ApplyViewOptions( aNewViewOptions );
    }
}

/** method to adjust page preview layout to document changes

    OD 18.12.2002 #103492#
*/
bool SwPagePreviewLayout::ReInit()
{
    
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



/** prepare paint of page preview

    OD 12.12.2002 #103492#
    OD 21.03.2003 #108282# - delete parameter _onStartPageVirtNum

    @note _nProposedStartPageNum, _onStartPageNum are absolute
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

    

    
    maWinSize = mrParentViewShell.GetOut()->PixelToLogic( _rPxWinSize );

    mbNoPageVisible = false;
    if ( nProposedStartPageNum > 0 )
    {
        
        sal_uInt16 nColOfProposed = GetColOfPage( nProposedStartPageNum );
        sal_uInt16 nRowOfProposed = GetRowOfPage( nProposedStartPageNum );
        
        if ( _bStartWithPageAtFirstCol )
        {
            
            
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

        
        if ( _bStartWithPageAtFirstCol )
            mnPaintStartCol = 1;
        else
            mnPaintStartCol = nColOfProposed;
        
        mnPaintStartRow = nRowOfProposed;
        
        maPaintStartPageOffset.X() = -1;
        maPaintStartPageOffset.Y() = -1;
        
        if ( _bStartWithPageAtFirstCol )
            maPaintPreviewDocOffset.X() = 0;
        else
            maPaintPreviewDocOffset.X() = (nColOfProposed-1) * mnColWidth;
        maPaintPreviewDocOffset.Y() = (nRowOfProposed-1) * mnRowHeight;
    }
    else
    {
        
        
        sal_uInt16 nColOfProposed =
                static_cast<sal_uInt16>(_aProposedStartPos.X() / mnColWidth) + 1;
        sal_uInt16 nRowOfProposed =
                static_cast<sal_uInt16>(_aProposedStartPos.Y() / mnRowHeight) + 1;
        
        
        
        if ( mbBookPreview &&
             ( nRowOfProposed == 1 && nColOfProposed == 1 )
           )
            mnPaintPhyStartPageNum = 1;
        else
        {
            
            
            mnPaintPhyStartPageNum = (nRowOfProposed-1) * mnCols + nColOfProposed;
            if ( mbBookPreview )
                --mnPaintPhyStartPageNum;
            if ( mnPaintPhyStartPageNum > mnPages )
            {
                
                
                mnPaintPhyStartPageNum = mnPages;
                mbNoPageVisible = true;
            }
        }
        
        mnPaintStartCol = nColOfProposed;
        mnPaintStartRow = nRowOfProposed;
        
        maPaintStartPageOffset.X() =
                (_aProposedStartPos.X() % mnColWidth) - mnXFree;
        maPaintStartPageOffset.Y() =
                (_aProposedStartPos.Y() % mnRowHeight) - mnYFree;
        
        maPaintPreviewDocOffset = _aProposedStartPos;
    }

    
    _CalcAdditionalPaintOffset();

    
    _CalcDocPreviewPaintRect();
    _orDocPreviewPaintRect = maPaintedPreviewDocRect;

    
    
    if ( !mbDoesLayoutColsFitIntoWindow &&
         maPaintedPreviewDocRect.GetWidth() < maWinSize.Width() )
    {
        maPaintedPreviewDocRect.Move(
                -(maWinSize.Width() - maPaintedPreviewDocRect.GetWidth()), 0 );
        Prepare( 0, maPaintedPreviewDocRect.TopLeft(),
                 _rPxWinSize, _onStartPageNum,
                 _orDocPreviewPaintRect, _bStartWithPageAtFirstCol );
    }

    
    
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

    
    
    _CalcPreviewPages();

    
    if ( mbInPaint )
    {
        mbNewLayoutDuringPaint = true;
    }

    
    mbPaintInfoValid = true;

    
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
    _ClearPreviewPageData();

    if ( mbNoPageVisible )
        return;

    
    const SwPageFrm* pStartPage = mrLayoutRootFrm.GetPageByPageNum( mnPaintPhyStartPageNum );

    
    Point aInitialPaintOffset;
    
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

    
    const SwPageFrm* pPage = pStartPage;
    sal_uInt16 nCurrCol = mnPaintStartCol;
    sal_uInt16 nConsideredRows = 0;
    Point aCurrPaintOffset = aInitialPaintOffset;
    
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

        
        if ( nCurrCol < mnPaintStartCol )
        {
            
            PreviewPage* pPreviewPage = new PreviewPage;
            Point aCurrAccOffset = aCurrPaintOffset -
                           Point( (mnPaintStartCol-nCurrCol) * mnColWidth, 0 );
            _CalcPreviewDataForPage( *(pPage), aCurrAccOffset, pPreviewPage );
            pPreviewPage->bVisible = false;
            maPreviewPages.push_back( pPreviewPage );
            
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
            ++nCurrCol;
            continue;
        }
        if ( aCurrPaintOffset.X() < maWinSize.Width() )
        {
            
            
            if ( mbBookPreview && pPage->GetPhyPageNum() == 1 && mnCols != 1 && nCurrCol == 1
               )
            {
                
                
                
                if(!Application::GetSettings().GetLayoutRTL())
                    aCurrPaintOffset.X() += mnColWidth;
                else aCurrPaintOffset.X() -= mnColWidth;
                ++nCurrCol;
                continue;
            }

            
            PreviewPage* pPreviewPage = new PreviewPage;
            _CalcPreviewDataForPage( *(pPage), aCurrPaintOffset, pPreviewPage );
            pPreviewPage->bVisible = true;
            maPreviewPages.push_back( pPreviewPage );
        }
        else
        {
            
            PreviewPage* pPreviewPage = new PreviewPage;
            _CalcPreviewDataForPage( *(pPage), aCurrPaintOffset, pPreviewPage );
            pPreviewPage->bVisible = false;
            maPreviewPages.push_back( pPreviewPage );
        }

        
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());

        
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
*/
bool SwPagePreviewLayout::_CalcPreviewDataForPage( const SwPageFrm& _rPage,
                                                   const Point& _rPreviewOffset,
                                                   PreviewPage* _opPreviewPage )
{
    
    _opPreviewPage->pPage = &_rPage;
    
    if ( _rPage.IsEmptyPage() )
    {
        if ( _rPage.GetPhyPageNum() % 2 == 0 )
            _opPreviewPage->aPageSize = _rPage.GetPrev()->Frm().SSize();
        else
            _opPreviewPage->aPageSize = _rPage.GetNext()->Frm().SSize();
    }
    else
        _opPreviewPage->aPageSize = _rPage.Frm().SSize();
    
    Point aPreviewWinOffset( _rPreviewOffset );
    if ( _opPreviewPage->aPageSize.Width() < maMaxPageSize.Width() )
        aPreviewWinOffset.X() += ( maMaxPageSize.Width() - _opPreviewPage->aPageSize.Width() ) / 2;
    if ( _opPreviewPage->aPageSize.Height() < maMaxPageSize.Height() )
        aPreviewWinOffset.Y() += ( maMaxPageSize.Height() - _opPreviewPage->aPageSize.Height() ) / 2;
    _opPreviewPage->aPreviewWinPos = aPreviewWinOffset;
    
    if ( _rPage.IsEmptyPage() )
    {
        _opPreviewPage->aLogicPos = _opPreviewPage->aPreviewWinPos;
        _opPreviewPage->aMapOffset = Point( 0, 0 );
    }
    else
    {
        _opPreviewPage->aLogicPos = _rPage.Frm().Pos();
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
    bool bRet = false;

    if ( mbBookPreview != _bEnableBookPreview)
    {
        mbBookPreview = _bEnableBookPreview;
        
        ReInit();
        
        {
            mbBookPreviewModeToggled = true;
            Point aProposedStartPos( maPaintPreviewDocOffset );
            
            
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
        
        if ( maPaintedPreviewDocRect.GetWidth() > _aNewWinSize.Width() )
            aNewPaintStartPos.X() +=
                (maPaintedPreviewDocRect.GetWidth() - _aNewWinSize.Width()) / 2;
        
        if ( maPaintedPreviewDocRect.GetHeight() > _aNewWinSize.Height() )
        {
            aNewPaintStartPos.Y() +=
                (maPaintedPreviewDocRect.GetHeight() - _aNewWinSize.Height()) / 2;
            
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
    
    sal_uInt16 nTmpRelSelPageNum = ConvertAbsoluteToRelativePageNum( mnSelectedPageNum );
    sal_uInt16 nNewRelSelectedPageNum = nTmpRelSelPageNum;

    
    
    if ( mbBookPreview )
    {
        
        ++nTmpRelSelPageNum;
    }
    sal_uInt16 nTmpCol = nTmpRelSelPageNum % mnCols;
    sal_uInt16 nCurrRow = nTmpRelSelPageNum / mnCols;
    if ( nTmpCol > 0 )
        ++nCurrRow;

    
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

        
        
        sal_Int16 nTotalRows = GetRowOfPage( mnPages );
        if ( (_nHoriMove > 0 || _nVertMove > 0) &&
             mbDoesLayoutRowsFitIntoWindow &&
             mbDoesLayoutColsFitIntoWindow &&
             nCurrRow > nTotalRows - mnRows )
        {
            
            
            nNewStartPage = (nTotalRows - mnRows) * mnCols + 1;
            
            
            if ( mbBookPreview )
            {
                
                
                --nNewStartPage;
            }
            nNewStartPage = ConvertRelativeToAbsolutePageNum( nNewStartPage );
        }
        else
        {
            
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
    PreviewPosInsidePagePred( const Point _nPreviewPos ) : mnPreviewPos( _nPreviewPos ) {};
    bool operator() ( const PreviewPage* _pPreviewPage )
    {
        if ( _pPreviewPage->bVisible )
        {
            Rectangle aPreviewPageRect( _pPreviewPage->aPreviewWinPos, _pPreviewPage->aPageSize );
            return aPreviewPageRect.IsInside( mnPreviewPos ) ? true : false;
        }
        else
            return false;
    }
};

bool SwPagePreviewLayout::IsPreviewPosInDocPreviewPage( const Point  _aPreviewPos,
                                                    Point&       _orDocPos,
                                                    bool&        _obPosInEmptyPage,
                                                    sal_uInt16&  _onPageNum ) const
{
    bool bIsPosInsideDoc;

    
    _orDocPos.X() = 0;
    _orDocPos.Y() = 0;
    _obPosInEmptyPage = false;
    _onPageNum = 0;

    std::vector<PreviewPage*>::const_iterator aFoundPreviewPageIter =
            std::find_if( maPreviewPages.begin(), maPreviewPages.end(),
                          PreviewPosInsidePagePred( _aPreviewPos ) );

    if ( aFoundPreviewPageIter == maPreviewPages.end() )
        
        bIsPosInsideDoc = false;
    else
    {
        _onPageNum = (*aFoundPreviewPageIter)->pPage->GetPhyPageNum();
        if ( (*aFoundPreviewPageIter)->pPage->IsEmptyPage() )
        {
            
            bIsPosInsideDoc = false;
            _obPosInEmptyPage = true;
        }
        else
        {
            
            bIsPosInsideDoc = true;
            _orDocPos = _aPreviewPos -
                        (*aFoundPreviewPageIter)->aPreviewWinPos +
                        (*aFoundPreviewPageIter)->aLogicPos;
        }
    }

    return bIsPosInsideDoc;
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

    
    
    if ( mbLayoutSizesValid )
    {
        if ( (maPaintedPreviewDocRect.Top() + nScrollAmount) <= 0 )
            nScrollAmount = -maPaintedPreviewDocRect.Top();

        
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



/** paint prepared preview

    OD 12.12.2002 #103492#
*/
bool SwPagePreviewLayout::Paint( const Rectangle  _aOutRect ) const
{
    
    {
        if ( !mrParentViewShell.GetWin() &&
             !mrParentViewShell.GetOut()->GetConnectMetaFile() )
            return false;

        OSL_ENSURE( mbPaintInfoValid,
                "invalid preview settings - no paint of preview" );
        if ( !mbPaintInfoValid )
            return false;
    }

    
    if ( mrLayoutRootFrm.IsSuperfluous() )
    {
        return true;
    }

    

    
    if ( mbInPaint )
    {
        return false;
    }
    mbInPaint = true;

    OutputDevice* pOutputDev = mrParentViewShell.GetOut();

    
    if ( maPreviewPages.size() > 0 )
    {
        mrParentViewShell.Imp()->bFirstPageInvalid = sal_False;
        mrParentViewShell.Imp()->pFirstVisPage =
                const_cast<SwPageFrm*>(maPreviewPages[0]->pPage);
    }

    
    {
        SwRegionRects aPreviewBackgrdRegion( _aOutRect );
        
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
        
        mrParentViewShell._PaintDesktop( aPreviewBackgrdRegion );
    }

    
    const Rectangle aPxOutRect( pOutputDev->LogicToPixel( _aOutRect ) );

    MapMode aMapMode( pOutputDev->GetMapMode() );
    MapMode aSavedMapMode = aMapMode;

    const Font& rEmptyPgFont = SwPageFrm::GetEmptyPageFont();

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
                pOutputDev->SetLineColor(); 
                
                {
                    SwRect aTmpPageRect( aPageRect );
                    ::SwAlignRect( aTmpPageRect, &mrParentViewShell);
                    aPageRect = aTmpPageRect.SVRect();
                }
                pOutputDev->DrawRect( aPageRect );

                
                Font aOldFont( pOutputDev->GetFont() );
                pOutputDev->SetFont( rEmptyPgFont );
                pOutputDev->DrawText( aPageRect, SW_RESSTR( STR_EMPTYPAGE ),
                                    TEXT_DRAW_VCENTER |
                                    TEXT_DRAW_CENTER |
                                    TEXT_DRAW_CLIP );
                pOutputDev->SetFont( aOldFont );
                
                
                SwPageFrm::PaintBorderAndShadow( aPageRect, &mrParentViewShell, true, false, true );
            }
            else
            {
                mrParentViewShell.maVisArea = aPageRect;
                aPxPaintRect.Intersection( aPxOutRect );
                Rectangle aPaintRect = pOutputDev->PixelToLogic( aPxPaintRect );
                mrParentViewShell.Paint( aPaintRect );
                
                
                {
                    SwRect aPageBorderRect;
                    SwPageFrm::GetBorderAndShadowBoundRect( SwRect( aPageRect ), &mrParentViewShell, aPageBorderRect,
                        (*aPageIter)->pPage->IsLeftShadowNeeded(), (*aPageIter)->pPage->IsRightShadowNeeded(), true );
                    const Region aDLRegion(aPageBorderRect.SVRect());
                    mrParentViewShell.DLPrePaint2(aDLRegion);
                    SwPageFrm::PaintBorderAndShadow( aPageRect, &mrParentViewShell, true, false, true );
                    mrParentViewShell.DLPostPaint2(true);
                }
                
            }
            
            
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

    
    
    if ( !mbNewLayoutDuringPaint )
    {
        
        mrParentViewShell.Imp()->UpdateAccessiblePreview(
                        maPreviewPages,
                        aMapMode.GetScaleX(),
                        mrLayoutRootFrm.GetPageByPageNum( mnSelectedPageNum ),
                        maWinSize );
    }

    pOutputDev->SetMapMode( aSavedMapMode );
    mrParentViewShell.maVisArea.Clear();

    
    mbInPaint = false;
    mbNewLayoutDuringPaint = false;

    return true;
}

/** repaint pages on page preview

    OD 18.12.2002 #103492#
*/
void SwPagePreviewLayout::Repaint( const Rectangle _aInvalidCoreRect ) const
{
    
    {
        if ( !mrParentViewShell.GetWin() &&
             !mrParentViewShell.GetOut()->GetConnectMetaFile() )
            return;

        OSL_ENSURE( mbPaintInfoValid,
                "invalid preview settings - no paint of preview" );
        if ( !mbPaintInfoValid )
            return;
    }

    

    
    if ( maPreviewPages.size() > 0 )
    {
        mrParentViewShell.Imp()->bFirstPageInvalid = sal_False;
        mrParentViewShell.Imp()->pFirstVisPage =
                const_cast<SwPageFrm*>(maPreviewPages[0]->pPage);
    }

    
    for ( std::vector<PreviewPage*>::const_iterator aPageIter = maPreviewPages.begin();
          aPageIter != maPreviewPages.end();
          ++aPageIter )
    {
        if ( !(*aPageIter)->bVisible )
            continue;

        Rectangle aPageRect( (*aPageIter)->aLogicPos, (*aPageIter)->aPageSize );
        if ( _aInvalidCoreRect.IsOver( aPageRect ) )
        {
            aPageRect.Intersection( _aInvalidCoreRect );
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
void SwPagePreviewLayout::_PaintSelectMarkAtPage(
                                    const PreviewPage* _aSelectedPreviewPage ) const
{
    OutputDevice* pOutputDev = mrParentViewShell.GetOut();
    MapMode aMapMode( pOutputDev->GetMapMode() );
    
    MapMode aSavedMapMode = aMapMode;
    
    Color aFill( pOutputDev->GetFillColor() );
    Color aLine( pOutputDev->GetLineColor() );

    
    Color aSelPgLineColor(COL_LIGHTBLUE);
    const StyleSettings& rSettings =
        mrParentViewShell.GetWin()->GetSettings().GetStyleSettings();
    if ( rSettings.GetHighContrastMode() )
        aSelPgLineColor = rSettings.GetHighlightTextColor();

    
    aMapMode.SetOrigin( _aSelectedPreviewPage->aMapOffset );
    pOutputDev->SetMapMode( aMapMode );

    
    SwRect aPageRect( _aSelectedPreviewPage->aLogicPos,
                         _aSelectedPreviewPage->aPageSize );
    
    
    ::SwAlignRect( aPageRect, &mrParentViewShell);
    Rectangle aPxPageRect = pOutputDev->LogicToPixel( aPageRect.SVRect() );

    
    
    Rectangle aRect( aPxPageRect.Left(), aPxPageRect.Top(),
                       aPxPageRect.Right(), aPxPageRect.Bottom() );
    aRect = pOutputDev->PixelToLogic( aRect );
    pOutputDev->SetFillColor(); 
    pOutputDev->SetLineColor( aSelPgLineColor );
    pOutputDev->DrawRect( aRect );
    
    aRect = Rectangle( aPxPageRect.Left()+1, aPxPageRect.Top()+1,
                       aPxPageRect.Right()-1, aPxPageRect.Bottom()-1 );
    aRect = pOutputDev->PixelToLogic( aRect );
    pOutputDev->DrawRect( aRect );

    
    pOutputDev->SetFillColor( aFill );
    pOutputDev->SetLineColor( aLine );

    
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
    sal_uInt16 nOldSelectedPageNum = mnSelectedPageNum;
    mnSelectedPageNum = _nSelectedPage;

    
    const PreviewPage* pOldSelectedPreviewPage = _GetPreviewPageByPageNum( nOldSelectedPageNum );
    if ( pOldSelectedPreviewPage && pOldSelectedPreviewPage->bVisible )
    {
        
        SwRect aPageRect( pOldSelectedPreviewPage->aPreviewWinPos,
                              pOldSelectedPreviewPage->aPageSize );
        ::SwAlignRect( aPageRect, &mrParentViewShell);
        OutputDevice* pOutputDev = mrParentViewShell.GetOut();
        Rectangle aPxPageRect = pOutputDev->LogicToPixel( aPageRect.SVRect() );
        
        Rectangle aInvalPxRect( aPxPageRect.Left(), aPxPageRect.Top(),
                                aPxPageRect.Right(), aPxPageRect.Top()+1 );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
        
        aInvalPxRect = Rectangle( aPxPageRect.Right()-1, aPxPageRect.Top(),
                                  aPxPageRect.Right(), aPxPageRect.Bottom() );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
        
        aInvalPxRect = Rectangle( aPxPageRect.Left(), aPxPageRect.Bottom()-1,
                                  aPxPageRect.Right(), aPxPageRect.Bottom() );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
        
        aInvalPxRect = Rectangle( aPxPageRect.Left(), aPxPageRect.Top(),
                                  aPxPageRect.Left()+1, aPxPageRect.Bottom() );
        mrParentViewShell.GetWin()->Invalidate( pOutputDev->PixelToLogic( aInvalPxRect ) );
    }

    
    const PreviewPage* pNewSelectedPreviewPage = _GetPreviewPageByPageNum( _nSelectedPage );
    if ( pNewSelectedPreviewPage && pNewSelectedPreviewPage->bVisible )
        _PaintSelectMarkAtPage( pNewSelectedPreviewPage );
}



/** get preview page by physical page number

    OD 17.12.2002 #103492#
*/
struct EqualsPageNumPred
{
    const sal_uInt16 mnPageNum;
    EqualsPageNumPred( const sal_uInt16 _nPageNum ) : mnPageNum( _nPageNum ) {};
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
        return 0;
    else
        return (*aFoundPreviewPageIter);
}

/** determine row the page with the given number is in

    OD 17.01.2003 #103492#

    @note _nPageNum is relative
*/
sal_uInt16 SwPagePreviewLayout::GetRowOfPage( sal_uInt16 _nPageNum ) const
{
    
    
    if ( mbBookPreview )
    {
        
        
        ++_nPageNum;
    }

    sal_uInt16 nRow = (_nPageNum) / mnCols;
    if ( ( (_nPageNum) % mnCols ) > 0 )
        ++nRow;

    return nRow;
}

/** determine column the page with the given number is in

    OD 17.01.2003 #103492#

    @note _nPageNum is relative
*/
sal_uInt16 SwPagePreviewLayout::GetColOfPage( sal_uInt16 _nPageNum ) const
{
    
    
    if ( mbBookPreview )
    {
        
        
        ++_nPageNum;
    }

    sal_uInt16 nCol = (_nPageNum) % mnCols;
    if ( nCol == 0 )
        nCol = mnCols;

    return nCol;
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
    else
    {
        return Size( 0, 0 );
    }
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
    else
    {
        return 0;
    }
}

/** Convert absolute to relative page numbers (see PrintEmptyPages) */
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

/** Convert relative to absolute page numbers (see PrintEmptyPages) */
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
