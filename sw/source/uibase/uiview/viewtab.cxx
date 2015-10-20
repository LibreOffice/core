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

#include <hintids.hxx>
#include "uitool.hxx"
#include <sfx2/app.hxx>
#include <svx/rulritem.hxx>
#include <editeng/tstpitem.hxx>
#include <sfx2/request.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <svx/ruler.hxx>
#include <editeng/protitem.hxx>
#include <svl/rectitem.hxx>
#include <sfx2/bindings.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <edtwin.hxx>
#include "view.hxx"
#include "wrtsh.hxx"
#include "basesh.hxx"
#include "cmdid.h"
#include "viewopt.hxx"
#include "tabcol.hxx"
#include "frmfmt.hxx"
#include "pagedesc.hxx"
#include "wview.hxx"
#include "fmtcol.hxx"
#include "section.hxx"

#include "ndtxt.hxx"
#include "pam.hxx"

#include <IDocumentSettingAccess.hxx>

using namespace ::com::sun::star;

// Pack columns
static void lcl_FillSvxColumn(const SwFormatCol& rCol,
                          long nTotalWidth,
                          SvxColumnItem& rColItem,
                          long nDistance)
{
    const SwColumns& rCols = rCol.GetColumns();

    bool bOrtho = rCol.IsOrtho() && rCols.size();
    long nInnerWidth = 0;
    if( bOrtho )
    {
        nInnerWidth = nTotalWidth;
        for ( size_t i = 0; i < rCols.size(); ++i )
        {
            const SwColumn* pCol = &rCols[i];
            nInnerWidth -= pCol->GetLeft() + pCol->GetRight();
        }
        if( nInnerWidth < 0 )
            nInnerWidth = 0;
        else
            nInnerWidth /= rCols.size();
    }

    long nWidth = 0;
    for ( size_t i = 0; i < rCols.size(); ++i )
    {
        const SwColumn* pCol = &rCols[i];
        const long nStart = pCol->GetLeft() + nWidth + nDistance;
        if( bOrtho )
            nWidth += nInnerWidth + pCol->GetLeft() + pCol->GetRight();
        else
            nWidth += rCol.CalcColWidth(i, static_cast< sal_uInt16 >(nTotalWidth));
        const long nEnd = nWidth - pCol->GetRight() + nDistance;

        SvxColumnDescription aColDesc(nStart, nEnd, true);
        rColItem.Append(aColDesc);
    }
}

// Transfer ColumnItem in ColumnInfo
static void lcl_ConvertToCols(const SvxColumnItem& rColItem,
                          long nTotalWidth,
                          SwFormatCol& rCols)
{
    OSL_ENSURE( rCols.GetNumCols() == rColItem.Count(), "Column count mismatch" );
    // ruler executes that change the columns shortly after the selection has changed
    // can result in a crash
    if(rCols.GetNumCols() != rColItem.Count())
        return;

    sal_uInt16 nLeft    = 0;
    SwTwips nSumAll= 0;  // Sum up all columns and margins

    SwColumns& rArr = rCols.GetColumns();

    // Tabcols sequentially
    for( sal_uInt16 i=0; i < rColItem.Count()-1; ++i )
    {
        OSL_ENSURE(rColItem[i+1].nStart >= rColItem[i].nEnd,"overlapping columns" );
        const long nStart = std::max(rColItem[i+1].nStart, rColItem[i].nEnd);
        const sal_uInt16 nRight = static_cast<sal_uInt16>((nStart - rColItem[i].nEnd) / 2);

        const long nWidth = rColItem[i].nEnd - rColItem[i].nStart + nLeft + nRight;

        SwColumn* pCol = &rArr[i];
        pCol->SetWishWidth( sal_uInt16(long(rCols.GetWishWidth()) * nWidth / nTotalWidth ));
        pCol->SetLeft( nLeft );
        pCol->SetRight( nRight );
        nSumAll += pCol->GetWishWidth();

        nLeft = nRight;
    }
    rArr[rColItem.Count()-1].SetLeft( nLeft );

    // The difference between the total sum of the desired width and the so far
    // calculated columns and margins should result in the width of the last column.
    rArr[rColItem.Count()-1].SetWishWidth( rCols.GetWishWidth() - (sal_uInt16)nSumAll );

    rCols.SetOrtho(false, 0, 0 );
}

// Delete tabs
static void lcl_EraseDefTabs(SvxTabStopItem& rTabStops)
{
    // Delete DefTabs
    for ( sal_uInt16 i = 0; i < rTabStops.Count(); )
    {
        // Here also throw out the DefTab to zero
        if ( SVX_TAB_ADJUST_DEFAULT == rTabStops[i].GetAdjustment() ||
            rTabStops[i].GetTabPos() == 0 )
        {
            rTabStops.Remove(i);
            continue;
        }
        ++i;
    }
}

// Flip page margin
void SwView::SwapPageMargin(const SwPageDesc& rDesc, SvxLRSpaceItem& rLRSpace)
{
    sal_uInt16 nPhyPage, nVirPage;
    GetWrtShell().GetPageNum( nPhyPage, nVirPage );

    if ( rDesc.GetUseOn() == nsUseOnPage::PD_MIRROR && (nPhyPage % 2) == 0 )
    {
        long nTmp = rLRSpace.GetRight();
        rLRSpace.SetRight( rLRSpace.GetLeft() );
        rLRSpace.SetLeft( nTmp );
    }
}

// If the frame border is moved, the column separator
// should stay in the same absolute position.
static void lcl_Scale(long& nVal, long nScale)
{
    nVal *= nScale;
    nVal >>= 8;
}

void ResizeFrameCols(SwFormatCol& rCol,
                    long nOldWidth,
                    long nNewWidth,
                    long nLeftDelta )
{
    SwColumns& rArr = rCol.GetColumns();
    long nWishSum = (long)rCol.GetWishWidth();
    long nWishDiff = (nWishSum * 100/nOldWidth * nNewWidth) / 100 - nWishSum;
    long nNewWishWidth = nWishSum + nWishDiff;
    if(nNewWishWidth > 0xffffl)
    {
        // If the desired width is getting too large, then all values
        // must be scaled appropriately.
        long nScale = (0xffffl << 8)/ nNewWishWidth;
        for(size_t i = 0; i < rArr.size(); ++i)
        {
            SwColumn* pCol = &rArr[i];
            long nVal = pCol->GetWishWidth();
            lcl_Scale(nVal, nScale);
            pCol->SetWishWidth((sal_uInt16) nVal);
            nVal = pCol->GetLeft();
            lcl_Scale(nVal, nScale);
            pCol->SetLeft((sal_uInt16) nVal);
            nVal = pCol->GetRight();
            lcl_Scale(nVal, nScale);
            pCol->SetRight((sal_uInt16) nVal);
        }
        lcl_Scale(nNewWishWidth, nScale);
        lcl_Scale(nWishDiff, nScale);
    }
    rCol.SetWishWidth( (sal_uInt16) (nNewWishWidth) );

    if( nLeftDelta >= 2 || nLeftDelta <= -2)
        rArr.front().SetWishWidth(rArr.front().GetWishWidth() + (sal_uInt16)nWishDiff);
    else
        rArr.back().SetWishWidth(rArr.back().GetWishWidth() + (sal_uInt16)nWishDiff);
    // Reset auto width
    rCol.SetOrtho(false, 0, 0 );
}

// Here all changes to the tab bar will be shot again into the model.
void SwView::ExecTabWin( SfxRequest& rReq )
{
    SwWrtShell &rSh         = GetWrtShell();
    const FrmTypeFlags nFrmType   = rSh.IsObjSelected() ?
                                    FrmTypeFlags::DRAWOBJ :
                                        rSh.GetFrmType(0,true);
    const bool bFrmSelection = rSh.IsFrmSelected();
    const bool bBrowse = rSh.GetViewOptions()->getBrowseMode();

    const sal_uInt16 nSlot      = rReq.GetSlot();
    const SfxItemSet* pReqArgs = rReq.GetArgs();
    const size_t nDescId    = rSh.GetCurPageDesc();
    const SwPageDesc& rDesc = rSh.GetPageDesc( nDescId );

    const bool bVerticalWriting = rSh.IsInVerticalText();
    const SwFormatHeader& rHeaderFormat = rDesc.GetMaster().GetHeader();
    SwFrameFormat *pHeaderFormat = const_cast<SwFrameFormat*>(rHeaderFormat.GetHeaderFormat());

    const SwFormatFooter& rFooterFormat = rDesc.GetMaster().GetFooter();
    SwFrameFormat *pFooterFormat = const_cast<SwFrameFormat*>(rFooterFormat.GetFooterFormat());

    const SwFormatFrmSize &rFrmSize = rDesc.GetMaster().GetFrmSize();

    const SwRect& rPageRect = rSh.GetAnyCurRect(RECT_PAGE);
    const long nPageWidth  = bBrowse ? rPageRect.Width() : rFrmSize.GetWidth();
    const long nPageHeight = bBrowse ? rPageRect.Height() : rFrmSize.GetHeight();

    bool bUnlockView = false;
    rSh.StartAllAction();
    bool bSect = bool(nFrmType & FrmTypeFlags::COLSECT);

    switch  (nSlot)
    {
    case SID_ATTR_LONG_LRSPACE:
        if ( pReqArgs )
        {
            SvxLongLRSpaceItem aLongLR( static_cast<const SvxLongLRSpaceItem&>(pReqArgs->
                                                        Get( SID_ATTR_LONG_LRSPACE )) );
            SvxLRSpaceItem aLR(RES_LR_SPACE);
            if ( !bSect && (bFrmSelection || nFrmType & FrmTypeFlags::FLY_ANY) )
            {
                SwFrameFormat* pFormat = static_cast<SwFrameFormat*>(rSh.GetFlyFrameFormat());
                const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED);

                bool bVerticalFrame(false);
                {
                    bool bRTL;
                    bool bVertL2R;
                    bVerticalFrame = ( bFrmSelection &&
                                       rSh.IsFrmVertical(true, bRTL, bVertL2R) ) ||
                                     ( !bFrmSelection && bVerticalWriting);
                }
                long nDeltaX = bVerticalFrame ?
                    rRect.Right() - rPageRect.Right() + aLongLR.GetRight() :
                    rPageRect.Left() + aLongLR.GetLeft() - rRect.Left();

                SfxItemSet aSet( GetPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_VERT_ORIENT, RES_HORI_ORIENT,
                                            RES_COL, RES_COL, 0 );

                if(bVerticalFrame)
                {
                    SwFormatVertOrient aVertOrient(pFormat->GetVertOrient());
                    aVertOrient.SetVertOrient(text::VertOrientation::NONE);
                    aVertOrient.SetPos(aVertOrient.GetPos() + nDeltaX );
                    aSet.Put( aVertOrient );
                }
                else
                {
                    SwFormatHoriOrient aHoriOrient( pFormat->GetHoriOrient() );
                    aHoriOrient.SetHoriOrient( text::HoriOrientation::NONE );
                    aHoriOrient.SetPos( aHoriOrient.GetPos() + nDeltaX );
                    aSet.Put( aHoriOrient );
                }

                SwFormatFrmSize aSize( pFormat->GetFrmSize() );
                long nOldWidth = (long) aSize.GetWidth();

                if(aSize.GetWidthPercent())
                {
                    SwRect aRect;
                    rSh.CalcBoundRect(aRect, FLY_AS_CHAR);
                    long nPrtWidth = aRect.Width();
                    aSize.SetWidthPercent(sal_uInt8((nPageWidth - aLongLR.GetLeft() - aLongLR.GetRight()) * 100 /nPrtWidth));
                }
                else
                    aSize.SetWidth( nPageWidth -
                            (aLongLR.GetLeft() + aLongLR.GetRight()));

                if( nFrmType & FrmTypeFlags::COLUMN )
                {
                    SwFormatCol aCol(pFormat->GetCol());

                    ::ResizeFrameCols(aCol, nOldWidth, (long)aSize.GetWidth(), nDeltaX );
                    aSet.Put(aCol);
                }

                aSet.Put( aSize );

                rSh.StartAction();
                rSh.Push();
                rSh.SetFlyFrmAttr( aSet );
                // Cancel the frame selection
                if(!bFrmSelection && rSh.IsFrmSelected())
                {
                    rSh.UnSelectFrm();
                    rSh.LeaveSelFrmMode();
                }
                rSh.Pop();
                rSh.EndAction();
            }
            else if ( nFrmType & ( FrmTypeFlags::HEADER | FrmTypeFlags::FOOTER ))
            {
                // Subtract out page margins
                long nOld = rDesc.GetMaster().GetLRSpace().GetLeft();
                aLongLR.SetLeft( nOld > aLongLR.GetLeft() ? 0 : aLongLR.GetLeft() - nOld );

                nOld = rDesc.GetMaster().GetLRSpace().GetRight();
                aLongLR.SetRight( nOld > aLongLR.GetRight() ? 0 : aLongLR.GetRight() - nOld );
                aLR.SetLeft(aLongLR.GetLeft());
                aLR.SetRight(aLongLR.GetRight());

                if ( nFrmType & FrmTypeFlags::HEADER && pHeaderFormat )
                    pHeaderFormat->SetFormatAttr( aLR );
                else if( nFrmType & FrmTypeFlags::FOOTER && pFooterFormat )
                    pFooterFormat->SetFormatAttr( aLR );
            }
            else if( nFrmType == FrmTypeFlags::DRAWOBJ)
            {
                SwRect aRect( rSh.GetObjRect() );
                aRect.Left( aLongLR.GetLeft() + rPageRect.Left() );
                aRect.Right( rPageRect.Right() - aLongLR.GetRight());
                rSh.SetObjRect( aRect );
            }
            else if(bSect || rSh.IsDirectlyInSection())
            {
                //change the section indents and the columns if available
                //at first determine the changes
                SwRect aSectRect = rSh.GetAnyCurRect(RECT_SECTION_PRT);
                const SwRect aTmpRect = rSh.GetAnyCurRect(RECT_SECTION);
                aSectRect.Pos() += aTmpRect.Pos();
                long nLeftDiff = aLongLR.GetLeft() - (long)(aSectRect.Left() - rPageRect.Left() );
                long nRightDiff = aLongLR.GetRight() - (long)( rPageRect.Right() - aSectRect.Right());
                //change the LRSpaceItem of the section accordingly
                const SwSection* pCurrSect = rSh.GetCurrSection();
                const SwSectionFormat* pSectFormat = pCurrSect->GetFormat();
                SvxLRSpaceItem aLRTmp = pSectFormat->GetLRSpace();
                aLRTmp.SetLeft(aLRTmp.GetLeft() + nLeftDiff);
                aLRTmp.SetRight(aLRTmp.GetRight() + nRightDiff);
                SfxItemSet aSet(rSh.GetAttrPool(), RES_LR_SPACE, RES_LR_SPACE, RES_COL, RES_COL, 0L);
                aSet.Put(aLRTmp);
                //change the first/last column
                if(bSect)
                {
                    SwFormatCol aCols( pSectFormat->GetCol() );
                    long nDiffWidth = nLeftDiff + nRightDiff;
                    ::ResizeFrameCols(aCols, aSectRect.Width(), aSectRect.Width() - nDiffWidth, nLeftDiff );
                    aSet.Put( aCols );
                }
                SwSectionData aData(*pCurrSect);
                rSh.UpdateSection(rSh.GetSectionFormatPos(*pSectFormat), aData, &aSet);
            }
            else
            {   // Adjust page margins
                aLR.SetLeft(aLongLR.GetLeft());
                aLR.SetRight(aLongLR.GetRight());
                SwapPageMargin( rDesc, aLR );
                SwPageDesc aDesc( rDesc );
                aDesc.GetMaster().SetFormatAttr( aLR );
                rSh.ChgPageDesc( nDescId, aDesc );
            }
        }
        break;

    // apply new left and right margins to current page style
    case SID_ATTR_PAGE_LRSPACE:
        if ( pReqArgs )
        {
            const SvxLongLRSpaceItem aLongLR( static_cast<const SvxLongLRSpaceItem&>(pReqArgs->Get( SID_ATTR_PAGE_LRSPACE )) );

            SwPageDesc aDesc( rDesc );
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                aLR.SetLeft(aLongLR.GetLeft());
                aLR.SetRight(aLongLR.GetRight());
                SwapPageMargin( rDesc, aLR );
                aDesc.GetMaster().SetFormatAttr( aLR );
            }
            rSh.ChgPageDesc( nDescId, aDesc );
        }
        break;

    case SID_ATTR_LONG_ULSPACE:
        if ( pReqArgs )
        {
            SvxLongULSpaceItem aLongULSpace( static_cast<const SvxLongULSpaceItem&>(pReqArgs->
                                                            Get( SID_ATTR_LONG_ULSPACE )));

            if( bFrmSelection || nFrmType & FrmTypeFlags::FLY_ANY )
            {
                SwFrameFormat* pFormat = static_cast<SwFrameFormat*>(rSh.GetFlyFrameFormat());
                const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED);
                const long nDeltaY = rPageRect.Top() + aLongULSpace.GetUpper() - rRect.Top();
                const long nHeight = nPageHeight - (aLongULSpace.GetUpper() + aLongULSpace.GetLower());

                SfxItemSet aSet( GetPool(), RES_FRM_SIZE, RES_FRM_SIZE,
                                            RES_VERT_ORIENT, RES_HORI_ORIENT, 0 );
                //which of the orientation attributes is to be put depends on the frame's environment
                bool bRTL;
                bool bVertL2R;
                if ( ( bFrmSelection &&
                       rSh.IsFrmVertical(true, bRTL, bVertL2R ) ) ||
                     ( !bFrmSelection && bVerticalWriting ) )
                {
                    SwFormatHoriOrient aHoriOrient(pFormat->GetHoriOrient());
                    aHoriOrient.SetHoriOrient(text::HoriOrientation::NONE);
                    aHoriOrient.SetPos(aHoriOrient.GetPos() + nDeltaY );
                    aSet.Put( aHoriOrient );
                }
                else
                {
                    SwFormatVertOrient aVertOrient(pFormat->GetVertOrient());
                    aVertOrient.SetVertOrient(text::VertOrientation::NONE);
                    aVertOrient.SetPos(aVertOrient.GetPos() + nDeltaY );
                    aSet.Put( aVertOrient );
                }
                SwFormatFrmSize aSize(pFormat->GetFrmSize());
                if(aSize.GetHeightPercent())
                {
                    SwRect aRect;
                    rSh.CalcBoundRect(aRect, FLY_AS_CHAR);
                    long nPrtHeight = aRect.Height();
                    aSize.SetHeightPercent(sal_uInt8(nHeight * 100 /nPrtHeight));
                }
                else
                    aSize.SetHeight(nHeight );

                aSet.Put( aSize );
                rSh.SetFlyFrmAttr( aSet );
            }
            else if( nFrmType == FrmTypeFlags::DRAWOBJ )
            {
                SwRect aRect( rSh.GetObjRect() );
                aRect.Top( aLongULSpace.GetUpper() + rPageRect.Top() );
                aRect.Bottom( rPageRect.Bottom() - aLongULSpace.GetLower() );
                rSh.SetObjRect( aRect ) ;
            }
            else if(bVerticalWriting && (bSect || rSh.IsDirectlyInSection()))
            {
                //change the section indents and the columns if available
                //at first determine the changes
                SwRect aSectRect = rSh.GetAnyCurRect(RECT_SECTION_PRT);
                const SwRect aTmpRect = rSh.GetAnyCurRect(RECT_SECTION);
                aSectRect.Pos() += aTmpRect.Pos();
                const long nLeftDiff = aLongULSpace.GetUpper() - (long)(aSectRect.Top() - rPageRect.Top());
                const long nRightDiff = aLongULSpace.GetLower() - (long)(nPageHeight - aSectRect.Bottom() + rPageRect.Top());
                //change the LRSpaceItem of the section accordingly
                const SwSection* pCurrSect = rSh.GetCurrSection();
                const SwSectionFormat* pSectFormat = pCurrSect->GetFormat();
                SvxLRSpaceItem aLR = pSectFormat->GetLRSpace();
                aLR.SetLeft(aLR.GetLeft() + nLeftDiff);
                aLR.SetRight(aLR.GetRight() + nRightDiff);
                SfxItemSet aSet(rSh.GetAttrPool(), RES_LR_SPACE, RES_LR_SPACE, RES_COL, RES_COL, 0L);
                aSet.Put(aLR);
                //change the first/last column
                if(bSect)
                {
                    SwFormatCol aCols( pSectFormat->GetCol() );
                    long nDiffWidth = nLeftDiff + nRightDiff;
                    ::ResizeFrameCols(aCols, aSectRect.Height(), aSectRect.Height() - nDiffWidth, nLeftDiff );
                    aSet.Put( aCols );
                }
                SwSectionData aData(*pCurrSect);
                rSh.UpdateSection(rSh.GetSectionFormatPos(*pSectFormat), aData, &aSet);
            }
            else
            {   SwPageDesc aDesc( rDesc );

                if ( nFrmType & ( FrmTypeFlags::HEADER | FrmTypeFlags::FOOTER ))
                {

                    const bool bHead = bool(nFrmType & FrmTypeFlags::HEADER);
                    SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
                    if ( bHead )
                        aUL.SetUpper( (sal_uInt16)aLongULSpace.GetUpper() );
                    else
                        aUL.SetLower( (sal_uInt16)aLongULSpace.GetLower() );
                    aDesc.GetMaster().SetFormatAttr( aUL );

                    if( (bHead && pHeaderFormat) || (!bHead && pFooterFormat) )
                    {
                        SwFormatFrmSize aSz( bHead ? pHeaderFormat->GetFrmSize() :
                                                  pFooterFormat->GetFrmSize() );
                        aSz.SetHeightSizeType( ATT_FIX_SIZE );
                        aSz.SetHeight(nPageHeight - aLongULSpace.GetLower() -
                                                    aLongULSpace.GetUpper() );
                        if ( bHead )
                            pHeaderFormat->SetFormatAttr( aSz );
                        else
                            pFooterFormat->SetFormatAttr( aSz );
                    }
                }
                else
                {
                    SvxULSpaceItem aUL(RES_UL_SPACE);
                    aUL.SetUpper((sal_uInt16)aLongULSpace.GetUpper());
                    aUL.SetLower((sal_uInt16)aLongULSpace.GetLower());
                    aDesc.GetMaster().SetFormatAttr(aUL);
                }

                rSh.ChgPageDesc( nDescId, aDesc );
            }
        }
        break;

    // apply new top and bottom margins to current page style
    case SID_ATTR_PAGE_ULSPACE:
        if ( pReqArgs )
        {
            SvxLongULSpaceItem aLongULSpace(
                static_cast<const SvxLongULSpaceItem&>(pReqArgs->Get( SID_ATTR_PAGE_ULSPACE ) ) );

            SwPageDesc aDesc( rDesc );
            {
                SvxULSpaceItem aUL(RES_UL_SPACE);
                aUL.SetUpper((sal_uInt16)aLongULSpace.GetUpper());
                aUL.SetLower((sal_uInt16)aLongULSpace.GetLower());
                aDesc.GetMaster().SetFormatAttr(aUL);
            }
            rSh.ChgPageDesc( nDescId, aDesc );
        }
        break;

    case SID_ATTR_PAGE_COLUMN:
        if ( pReqArgs )
        {
            const SfxInt16Item aColumnItem( static_cast<const SfxInt16Item&>(pReqArgs->Get(nSlot)) );
            const sal_uInt16 nPageColumnType = aColumnItem.GetValue();

            // nPageColumnType =
            // 1 - single-columned page
            // 2 - two-columned page
            // 3 - three-columned page
            // 4 - two-columned page with left column width of 2/3 of page width
            // 5 - two-columned page with right column width of 2/3 of page width

            sal_uInt16 nCount = 2;
            if ( nPageColumnType == 1 )
            {
                nCount = 0;
            }
            else if ( nPageColumnType == 3 )
            {
                nCount = 3;
            }

            const sal_uInt16 nGutterWidth = 0;

            const SvxLRSpaceItem aLR( rDesc.GetMaster().GetLRSpace() );
            const long nLeft = aLR.GetLeft();
            const long nRight = aLR.GetRight();
            const long nWidth = nPageWidth - nLeft - nRight;

            SwFormatCol aCols( rDesc.GetMaster().GetCol() );
            aCols.Init( nCount, nGutterWidth, nWidth );
            aCols.SetWishWidth( nWidth );
            aCols.SetGutterWidth( nGutterWidth, nWidth );
            aCols.SetOrtho( false, nGutterWidth, nWidth );

            long nColumnLeft = 0;
            long nColumnRight = 0;
            if ( nPageColumnType == 4 )
            {
                nColumnRight = (long)(nWidth/3);
                nColumnLeft = nWidth - nColumnRight;
                aCols.GetColumns()[0].SetWishWidth( nColumnLeft );
                aCols.GetColumns()[1].SetWishWidth( nColumnRight );
            }
            else if ( nPageColumnType == 5 )
            {
                nColumnLeft = (long)(nWidth/3);
                nColumnRight = nWidth - nColumnLeft;
                aCols.GetColumns()[0].SetWishWidth( nColumnLeft );
                aCols.GetColumns()[1].SetWishWidth( nColumnRight );
            }

            SwPageDesc aDesc( rDesc );
            aDesc.GetMaster().SetFormatAttr( aCols );
            rSh.ChgPageDesc( rSh.GetCurPageDesc(), aDesc );
        }
        break;

    case SID_ATTR_TABSTOP_VERTICAL:
    case SID_ATTR_TABSTOP:
        if (pReqArgs)
        {
            const sal_uInt16 nWhich = GetPool().GetWhich(nSlot);
            SvxTabStopItem aTabStops( static_cast<const SvxTabStopItem&>(pReqArgs->
                                                        Get( nWhich )));
            aTabStops.SetWhich(RES_PARATR_TABSTOP);
             const SvxTabStopItem& rDefTabs =
                        static_cast<const SvxTabStopItem&>(rSh.GetDefault(RES_PARATR_TABSTOP));

            // Default tab at pos 0
            SfxItemSet aSet( GetPool(), RES_LR_SPACE, RES_LR_SPACE );
            rSh.GetCurAttr( aSet );
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(aSet.Get(RES_LR_SPACE));

            if ( rLR.GetTextFirstLineOfst() < 0 )
            {
                SvxTabStop aSwTabStop( 0, SVX_TAB_ADJUST_DEFAULT );
                aTabStops.Insert( aSwTabStop );
            }

            // Populate with default tabs.
            ::MakeDefTabs( ::GetTabDist( rDefTabs ), aTabStops );

            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();
            if( pColl && pColl->IsAutoUpdateFormat() )
            {
                SfxItemSet aTmp(GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP);
                aTmp.Put(aTabStops);
                rSh.AutoUpdatePara( pColl, aTmp );
            }
            else
                rSh.SetAttrItem( aTabStops );
        }
        break;

    case SID_ATTR_PARA_LRSPACE_VERTICAL:
    case SID_ATTR_PARA_LRSPACE:
        if ( pReqArgs )
        {
            SvxLRSpaceItem aParaMargin(static_cast<const SvxLRSpaceItem&>(pReqArgs->Get(nSlot)));

            aParaMargin.SetRight( aParaMargin.GetRight() - m_nRightBorderDistance );
            aParaMargin.SetTextLeft(aParaMargin.GetTextLeft() - m_nLeftBorderDistance );

            aParaMargin.SetWhich( RES_LR_SPACE );
            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();

            // #i23726#
            if (m_pNumRuleNodeFromDoc)
            {
                // --> #i42922# Mouse move of numbering label
                // has to consider the left indent of the paragraph
                SfxItemSet aSet( GetPool(), RES_LR_SPACE, RES_LR_SPACE );
                rSh.GetCurAttr( aSet );
                const SvxLRSpaceItem& rLR =
                        static_cast<const SvxLRSpaceItem&>(aSet.Get(RES_LR_SPACE));

                SwPosition aPos(*m_pNumRuleNodeFromDoc);
                // #i90078#
                rSh.SetIndent( static_cast< short >(aParaMargin.GetTextLeft() - rLR.GetTextLeft()), aPos);
                // #i42921# invalidate state of indent in order to get a ruler update.
                aParaMargin.SetWhich( nSlot );
                GetViewFrame()->GetBindings().SetState( aParaMargin );
            }
            else if( pColl && pColl->IsAutoUpdateFormat() )
            {
                SfxItemSet aSet(GetPool(), RES_LR_SPACE, RES_LR_SPACE);
                aSet.Put(aParaMargin);
                rSh.AutoUpdatePara( pColl, aSet);
            }
            else
                rSh.SetAttrItem( aParaMargin );

            if ( aParaMargin.GetTextFirstLineOfst() < 0 )
            {
                SfxItemSet aSet( GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP );

                rSh.GetCurAttr( aSet );
                const SvxTabStopItem&  rTabStops = static_cast<const SvxTabStopItem&>(aSet.Get(RES_PARATR_TABSTOP));

                // Do we have a tab at position zero?
                sal_uInt16 i;

                for ( i = 0; i < rTabStops.Count(); ++i )
                    if ( rTabStops[i].GetTabPos() == 0 )
                        break;

                if ( i >= rTabStops.Count() )
                {
                    // No DefTab
                    SvxTabStopItem aTabStops( RES_PARATR_TABSTOP );
                    aTabStops = rTabStops;

                    ::lcl_EraseDefTabs(aTabStops);

                    SvxTabStop aSwTabStop( 0, SVX_TAB_ADJUST_DEFAULT );
                    aTabStops.Insert(aSwTabStop);

                    const SvxTabStopItem& rDefTabs =
                        static_cast<const SvxTabStopItem&>(rSh.GetDefault(RES_PARATR_TABSTOP));
                    ::MakeDefTabs( ::GetTabDist(rDefTabs), aTabStops );

                    if( pColl && pColl->IsAutoUpdateFormat())
                    {
                        SfxItemSet aSetTmp(GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP);
                        aSetTmp.Put(aTabStops);
                        rSh.AutoUpdatePara( pColl, aSetTmp );
                    }
                    else
                        rSh.SetAttrItem( aTabStops );
                }
            }
        }
        break;

    case SID_ATTR_PARA_ULSPACE:
        if ( pReqArgs )
        {
            SvxULSpaceItem aParaMargin(static_cast<const SvxULSpaceItem&>(pReqArgs->Get(nSlot)));

            long nUDist = 0;
            long nLDist = 0;
            aParaMargin.SetUpper( aParaMargin.GetUpper() - nUDist );
            aParaMargin.SetLower(aParaMargin.GetLower() - nLDist);

            aParaMargin.SetWhich( RES_UL_SPACE );
            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();
            if( pColl && pColl->IsAutoUpdateFormat() )
            {
                SfxItemSet aSet(GetPool(), RES_UL_SPACE, RES_UL_SPACE);
                aSet.Put(aParaMargin);
                rSh.AutoUpdatePara( pColl, aSet);
            }
            else
                rSh.SetAttrItem( aParaMargin );
        }
        break;
    case SID_PARASPACE_INCREASE:
    case SID_PARASPACE_DECREASE:
        {
            SfxItemSet aULSpaceSet( GetPool(), RES_UL_SPACE, RES_UL_SPACE );
            rSh.GetCurAttr( aULSpaceSet );
            SvxULSpaceItem aULSpace(
                static_cast< const SvxULSpaceItem& >( aULSpaceSet.Get( RES_UL_SPACE ) ) );
            sal_uInt16 nUpper = aULSpace.GetUpper();
            sal_uInt16 nLower = aULSpace.GetLower();

            if ( nSlot == SID_PARASPACE_INCREASE )
            {
                nUpper = std::min< sal_uInt16 >( nUpper + 57, 5670 );
                nLower = std::min< sal_uInt16 >( nLower + 57, 5670 );
            }
            else
            {
                nUpper = std::max< sal_Int16 >( nUpper - 57, 0 );
                nLower = std::max< sal_Int16 >( nLower - 57, 0 );
            }

            aULSpace.SetUpper( nUpper );
            aULSpace.SetLower( nLower );

            SwTextFormatColl* pColl = rSh.GetCurTextFormatColl();
            if( pColl && pColl->IsAutoUpdateFormat() )
            {
                aULSpaceSet.Put( aULSpace );
                rSh.AutoUpdatePara( pColl, aULSpaceSet );
            }
            else
                rSh.SetAttrItem( aULSpace );
        }
        break;
    case SID_RULER_BORDERS_VERTICAL:
    case SID_RULER_BORDERS:
        if ( pReqArgs )
        {
            SvxColumnItem aColItem(static_cast<const SvxColumnItem&>(pReqArgs->Get(nSlot)));

            if( m_bSetTabColFromDoc || (!bSect && rSh.GetTableFormat()) )
            {
                OSL_ENSURE(aColItem.Count(), "ColDesc is empty!!");

                const bool bSingleLine = static_cast<const SfxBoolItem&>(rReq.
                                GetArgs()->Get(SID_RULER_ACT_LINE_ONLY)).GetValue();

                SwTabCols aTabCols;
                if ( m_bSetTabColFromDoc )
                    rSh.GetMouseTabCols( aTabCols, m_aTabColFromDocPos );
                else
                    rSh.GetTabCols(aTabCols);

                // left table border
                long nBorder = (long)(aColItem.GetLeft() - aTabCols.GetLeftMin());
                aTabCols.SetLeft( nBorder );

                nBorder = (bVerticalWriting ? nPageHeight : nPageWidth) - aTabCols.GetLeftMin() - aColItem.GetRight();

                if ( aColItem.GetRight() > 0 )
                    aTabCols.SetRight( nBorder );

                // Tabcols sequentially
                // The last column is defined by the edge.
                // Columns in right-to-left tables need to be mirrored
                bool bIsTableRTL =
                    IsTabColFromDoc() ?
                          rSh.IsMouseTableRightToLeft(m_aTabColFromDocPos)
                        : rSh.IsTableRightToLeft();
                const size_t nColCount = aColItem.Count() - 1;
                if(bIsTableRTL)
                {
                    for ( size_t i = 0; i < nColCount && i < aTabCols.Count(); ++i )
                    {
                        const SvxColumnDescription& rCol = aColItem[nColCount - i];
                        aTabCols[i] = aTabCols.GetRight() - rCol.nStart;
                        aTabCols.SetHidden( i, !rCol.bVisible );
                    }
                }
                else
                {
                    for ( size_t i = 0; i < nColCount && i < aTabCols.Count(); ++i )
                    {
                        const SvxColumnDescription& rCol = aColItem[i];
                        aTabCols[i] = rCol.nEnd + aTabCols.GetLeft();
                        aTabCols.SetHidden( i, !rCol.bVisible );
                    }
                }

                if ( m_bSetTabColFromDoc )
                {
                    if( !rSh.IsViewLocked() )
                    {
                        bUnlockView = true;
                        rSh.LockView( true );
                    }
                    rSh.SetMouseTabCols( aTabCols, bSingleLine,
                                                   m_aTabColFromDocPos );
                }
                else
                    rSh.SetTabCols(aTabCols, bSingleLine);

            }
            else
            {
                if ( bFrmSelection || nFrmType & FrmTypeFlags::FLY_ANY || bSect)
                {
                    SwSectionFormat *pSectFormat = 0;
                    SfxItemSet aSet( GetPool(), RES_COL, RES_COL );
                    if(bSect)
                    {
                        SwSection *pSect = rSh.GetAnySection();
                        OSL_ENSURE( pSect, "Which section?");
                        pSectFormat = pSect->GetFormat();
                    }
                    else
                    {
                        rSh.GetFlyFrmAttr( aSet );
                    }
                    SwFormatCol aCols(
                        bSect ?
                            pSectFormat->GetCol() :
                                static_cast<const SwFormatCol&>(aSet.Get( RES_COL, false )));
                    SwRect aCurRect = rSh.GetAnyCurRect(bSect ? RECT_SECTION_PRT : RECT_FLY_PRT_EMBEDDED);
                    const long lWidth = bVerticalWriting ? aCurRect.Height() : aCurRect.Width();
                    ::lcl_ConvertToCols( aColItem, lWidth, aCols );
                    aSet.Put( aCols );
                    if(bSect)
                        rSh.SetSectionAttr( aSet, pSectFormat );
                    else
                    {
                        rSh.StartAction();
                        rSh.Push();
                        rSh.SetFlyFrmAttr( aSet );
                        // Cancel the frame selection again
                        if(!bFrmSelection && rSh.IsFrmSelected())
                        {
                            rSh.UnSelectFrm();
                            rSh.LeaveSelFrmMode();
                        }
                        rSh.Pop();
                        rSh.EndAction();
                    }
                }
                else
                {
                    SwFormatCol aCols( rDesc.GetMaster().GetCol() );
                    const SwRect aPrtRect = rSh.GetAnyCurRect(RECT_PAGE_PRT);
                    ::lcl_ConvertToCols( aColItem,
                                    bVerticalWriting ? aPrtRect.Height() : aPrtRect.Width(),
                                    aCols );
                    SwPageDesc aDesc( rDesc );
                    aDesc.GetMaster().SetFormatAttr( aCols );
                    rSh.ChgPageDesc( rSh.GetCurPageDesc(), aDesc );
                }
            }
        }
        break;

    case SID_RULER_ROWS :
    case SID_RULER_ROWS_VERTICAL:
        if (pReqArgs)
        {
            SvxColumnItem aColItem(static_cast<const SvxColumnItem&>(pReqArgs->Get(nSlot)));

            if( m_bSetTabColFromDoc || (!bSect && rSh.GetTableFormat()) )
            {
                OSL_ENSURE(aColItem.Count(), "ColDesc is empty!!");

                SwTabCols aTabCols;
                if ( m_bSetTabRowFromDoc )
                    rSh.GetMouseTabRows( aTabCols, m_aTabColFromDocPos );
                else
                    rSh.GetTabRows(aTabCols);

                if ( bVerticalWriting )
                {
                    aTabCols.SetRight(nPageWidth - aColItem.GetRight() - aColItem.GetLeft());
                    aTabCols.SetLeftMin(aColItem.GetLeft());
                }
                else
                {
                    long nBorder = nPageHeight - aTabCols.GetLeftMin() - aColItem.GetRight();
                    aTabCols.SetRight( nBorder );
                }

                const size_t nColItems = aColItem.Count() - 1;
                if(bVerticalWriting)
                {
                    for ( size_t i = nColItems; i; --i )
                    {
                        const SvxColumnDescription& rCol = aColItem[i - 1];
                        long nColumnPos = aTabCols.GetRight() - rCol.nEnd ;
                        aTabCols[i - 1] = nColumnPos;
                        aTabCols.SetHidden( i - 1, !rCol.bVisible );
                    }
                }
                else
                {
                    for ( size_t i = 0; i < nColItems; ++i )
                    {
                        const SvxColumnDescription& rCol = aColItem[i];
                        aTabCols[i] = rCol.nEnd + aTabCols.GetLeft();
                        aTabCols.SetHidden( i, !rCol.bVisible );
                    }
                }
                bool bSingleLine = false;
                const SfxPoolItem* pSingleLine;
                if( SfxItemState::SET == rReq.GetArgs()->GetItemState(SID_RULER_ACT_LINE_ONLY, false, &pSingleLine))
                    bSingleLine = static_cast<const SfxBoolItem*>(pSingleLine)->GetValue();
                if ( m_bSetTabRowFromDoc )
                {
                    if( !rSh.IsViewLocked() )
                    {
                        bUnlockView = true;
                        rSh.LockView( true );
                    }
                    rSh.SetMouseTabRows( aTabCols, bSingleLine, m_aTabColFromDocPos );
                }
                else
                    rSh.SetTabRows(aTabCols, bSingleLine);
            }
        }
        break;

    default:
        OSL_ENSURE( false, "wrong SlotId");
    }
    rSh.EndAllAction();

    if( bUnlockView )
        rSh.LockView( false );

    m_bSetTabColFromDoc = m_bSetTabRowFromDoc = m_bTabColFromDoc = m_bTabRowFromDoc = false;
    SetNumRuleNodeFromDoc(NULL);
}

// Here the status of the tab bar will be determined.
// This means that all relevant attributes at the CursorPos
// will be submittet to the tab bar.
void SwView::StateTabWin(SfxItemSet& rSet)
{
    SwWrtShell &rSh         = GetWrtShell();

    const Point* pPt = IsTabColFromDoc() || IsTabRowFromDoc() ? &m_aTabColFromDocPos : 0;
    const FrmTypeFlags nFrmType   = rSh.IsObjSelected()
                ? FrmTypeFlags::DRAWOBJ
                : rSh.GetFrmType( pPt, true );

    const bool  bFrmSelection = rSh.IsFrmSelected();
    const bool bBrowse = rSh.GetViewOptions()->getBrowseMode();
    // PageOffset/limiter
    const SwRect& rPageRect = rSh.GetAnyCurRect( RECT_PAGE, pPt );
    const SwRect& rPagePrtRect = rSh.GetAnyCurRect( RECT_PAGE_PRT, pPt );
    const long nPageWidth  = rPageRect.Width();
    const long nPageHeight = rPageRect.Height();

    const SwPageDesc& rDesc = rSh.GetPageDesc(
                IsTabColFromDoc() || m_bTabRowFromDoc ?
                    rSh.GetMousePageDesc(m_aTabColFromDocPos) : rSh.GetCurPageDesc() );

    const SvxFrameDirectionItem& rFrameDir = rDesc.GetMaster().GetFrmDir();
    const bool bVerticalWriting = rSh.IsInVerticalText();

    //enable tab stop display on the rulers depending on the writing direction
    WinBits nRulerStyle = m_pHRuler->GetStyle() & ~WB_EXTRAFIELD;
    m_pHRuler->SetStyle(bVerticalWriting||bBrowse ? nRulerStyle : nRulerStyle|WB_EXTRAFIELD);
    nRulerStyle = m_pVRuler->GetStyle() & ~WB_EXTRAFIELD;
    m_pVRuler->SetStyle(bVerticalWriting ? nRulerStyle|WB_EXTRAFIELD : nRulerStyle);

    //#i24363# tab stops relative to indent
    bool bRelative = rSh.getIDocumentSettingAccess().get(DocumentSettingId::TABS_RELATIVE_TO_INDENT);
    m_pHRuler->SetTabsRelativeToIndent( bRelative );
    m_pVRuler->SetTabsRelativeToIndent( bRelative );

    SvxLRSpaceItem aPageLRSpace( rDesc.GetMaster().GetLRSpace() );
    SwapPageMargin( rDesc, aPageLRSpace );

    SfxItemSet aCoreSet( GetPool(), RES_PARATR_TABSTOP, RES_PARATR_TABSTOP,
                                    RES_LR_SPACE,        RES_UL_SPACE, 0 );
    // get also the list level indent values merged as LR-SPACE item, if needed.
    rSh.GetCurAttr( aCoreSet, true );
    const SelectionType nSelType = rSh.GetSelectionType();

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    bool bPutContentProtection = false;

    while ( nWhich )
    {
        switch ( nWhich )
        {

        case SID_ATTR_PAGE_COLUMN:
        {
            sal_uInt16 nColumnType = 0;

            const SwFrameFormat& rMaster = rDesc.GetMaster();
            SwFormatCol aCol(rMaster.GetCol());
            const sal_uInt16 nCols = aCol.GetNumCols();
            if ( nCols == 0 )
            {
                nColumnType = 1;
            }
            else if ( nCols == 2 )
            {
                const sal_uInt16 nColLeft = aCol.CalcPrtColWidth(0, aCol.GetWishWidth());
                const sal_uInt16 nColRight = aCol.CalcPrtColWidth(1, aCol.GetWishWidth());

                if ( abs(nColLeft - nColRight) <= 10 )
                {
                    nColumnType = 2;
                }
                else if( abs(nColLeft - nColRight*2) < 20 )
                {
                    nColumnType = 4;
                }
                else if( abs(nColLeft*2 - nColRight) < 20 )
                {
                    nColumnType = 5;
                }
            }
            else if( nCols == 3 )
            {
                nColumnType = 3;
            }

            rSet.Put( SfxInt16Item( SID_ATTR_PAGE_COLUMN, nColumnType ) );
        }
        break;

        case SID_ATTR_LONG_LRSPACE:
        {
            SvxLongLRSpaceItem aLongLR( (long)aPageLRSpace.GetLeft(),
                                        (long)aPageLRSpace.GetRight(),
                                        SID_ATTR_LONG_LRSPACE);
            if(bBrowse)
            {
                aLongLR.SetLeft(rPagePrtRect.Left());
                aLongLR.SetRight(nPageWidth - rPagePrtRect.Right());
            }
            if ( ( nFrmType & FrmTypeFlags::HEADER || nFrmType & FrmTypeFlags::FOOTER ) &&
                 !(nFrmType & FrmTypeFlags::COLSECT) )
            {
                SwFrameFormat *pFormat = const_cast<SwFrameFormat*>(nFrmType & FrmTypeFlags::HEADER ?
                                rDesc.GetMaster().GetHeader().GetHeaderFormat() :
                                rDesc.GetMaster().GetFooter().GetFooterFormat());
                if( pFormat )// #i80890# if rDesc is not the one belonging to the current page is might crash
                {
                    SwRect aRect( rSh.GetAnyCurRect( RECT_HEADERFOOTER, pPt));
                    aRect.Pos() -= rSh.GetAnyCurRect( RECT_PAGE, pPt ).Pos();
                    const SvxLRSpaceItem& aLR = pFormat->GetLRSpace();
                    aLongLR.SetLeft ( (long)aLR.GetLeft() + (long)aRect.Left() );
                    aLongLR.SetRight( (nPageWidth -
                                        (long)aRect.Right() + (long)aLR.GetRight()));
                }
            }
            else
            {
                SwRect aRect;
                if( !bFrmSelection && ((nFrmType & FrmTypeFlags::COLSECT) || rSh.IsDirectlyInSection()) )
                {
                    aRect = rSh.GetAnyCurRect(RECT_SECTION_PRT, pPt);
                    const SwRect aTmpRect = rSh.GetAnyCurRect(RECT_SECTION, pPt);
                    aRect.Pos() += aTmpRect.Pos();
                }

                else if ( bFrmSelection || nFrmType & FrmTypeFlags::FLY_ANY )
                    aRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED, pPt);
                else if( nFrmType & FrmTypeFlags::DRAWOBJ)
                    aRect = rSh.GetObjRect();

                if( aRect.Width() )
                {
                    // make relative to page position:
                    aLongLR.SetLeft(aRect.Left() - rPageRect.Left());
                    aLongLR.SetRight(rPageRect.Right() - aRect.Right());
                }
            }
            if( nWhich == SID_ATTR_LONG_LRSPACE )
                rSet.Put( aLongLR );
            else
            {
                SvxLRSpaceItem aLR( aLongLR.GetLeft(),
                                    aLongLR.GetRight(),
                                    0, 0,
                                    nWhich);
                rSet.Put(aLR);
            }
            break;
        }

        // provide left and right margins of current page style
        case SID_ATTR_PAGE_LRSPACE:
        {
            const SvxLRSpaceItem aTmpPageLRSpace( rDesc.GetMaster().GetLRSpace() );
            const SvxLongLRSpaceItem aLongLR(
                (long)aTmpPageLRSpace.GetLeft(),
                (long)aTmpPageLRSpace.GetRight(),
                SID_ATTR_PAGE_LRSPACE );
            rSet.Put( aLongLR );
        }
        break;

        case SID_ATTR_LONG_ULSPACE:
        {
            // Page margin top bottom
            SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
            SvxLongULSpaceItem aLongUL( (long)aUL.GetUpper(),
                                        (long)aUL.GetLower(),
                                        SID_ATTR_LONG_ULSPACE);

            if ( bFrmSelection || nFrmType & FrmTypeFlags::FLY_ANY )
            {
                // Convert document coordinates into page coordinates.
                const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED, pPt);
                aLongUL.SetUpper(rRect.Top() - rPageRect.Top());
                aLongUL.SetLower(rPageRect.Bottom() - rRect.Bottom());
            }
            else if ( nFrmType & FrmTypeFlags::HEADER || nFrmType & FrmTypeFlags::FOOTER )
            {
                SwRect aRect( rSh.GetAnyCurRect( RECT_HEADERFOOTER, pPt));
                aRect.Pos() -= rSh.GetAnyCurRect( RECT_PAGE, pPt ).Pos();
                aLongUL.SetUpper( aRect.Top() );
                aLongUL.SetLower( nPageHeight - aRect.Bottom() );
            }
            else if( nFrmType & FrmTypeFlags::DRAWOBJ)
            {
                const SwRect &rRect = rSh.GetObjRect();
                aLongUL.SetUpper((rRect.Top() - rPageRect.Top()));
                aLongUL.SetLower((rPageRect.Bottom() - rRect.Bottom()));
            }
            else if(bBrowse)
            {
                aLongUL.SetUpper(rPagePrtRect.Top());
                aLongUL.SetLower(nPageHeight - rPagePrtRect.Bottom());
            }
            if( nWhich == SID_ATTR_LONG_ULSPACE )
                rSet.Put( aLongUL );
            else
            {
                SvxULSpaceItem aULTmp((sal_uInt16)aLongUL.GetUpper(),
                                      (sal_uInt16)aLongUL.GetLower(),
                                      nWhich);
                rSet.Put(aULTmp);
            }
            break;
        }

        // provide top and bottom margins of current page style
        case SID_ATTR_PAGE_ULSPACE:
        {
            const SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
            SvxLongULSpaceItem aLongUL(
                (long)aUL.GetUpper(),
                (long)aUL.GetLower(),
                SID_ATTR_PAGE_ULSPACE );

            rSet.Put( aLongUL );
        }
        break;

        case SID_ATTR_TABSTOP_VERTICAL :
        case RES_PARATR_TABSTOP:
        {
            if ( dynamic_cast< const SwWebView *>( this ) !=  nullptr ||
                 IsTabColFromDoc() ||
                 IsTabRowFromDoc() ||
                 ( nSelType & nsSelectionType::SEL_GRF ) ||
                 ( nSelType & nsSelectionType::SEL_FRM ) ||
                 ( nSelType & nsSelectionType::SEL_OLE ) ||
                 ( SfxItemState::DEFAULT > aCoreSet.GetItemState(RES_LR_SPACE) ) ||
                 (!bVerticalWriting && (SID_ATTR_TABSTOP_VERTICAL == nWhich) ) ||
                 ( bVerticalWriting && (RES_PARATR_TABSTOP == nWhich))
               )
                rSet.DisableItem( nWhich );
            else
            {
                SvxTabStopItem aTabStops(static_cast<const SvxTabStopItem&>(
                                            aCoreSet.Get( RES_PARATR_TABSTOP )));

                const SvxTabStopItem& rDefTabs = static_cast<const SvxTabStopItem&>(
                                            rSh.GetDefault(RES_PARATR_TABSTOP));

                OSL_ENSURE(m_pHRuler, "why is there no ruler?");
                const long nDefTabDist = ::GetTabDist(rDefTabs);
                m_pHRuler->SetDefTabDist( nDefTabDist );
                m_pVRuler->SetDefTabDist( nDefTabDist );
                ::lcl_EraseDefTabs(aTabStops);
                rSet.Put(aTabStops, nWhich);
            }
            break;
        }

        case SID_ATTR_PARA_LRSPACE_VERTICAL:
        case SID_ATTR_PARA_LRSPACE:
        {
            if ( nSelType & nsSelectionType::SEL_GRF ||
                 nSelType & nsSelectionType::SEL_FRM ||
                 nSelType & nsSelectionType::SEL_OLE ||
                 nFrmType == FrmTypeFlags::DRAWOBJ ||
                 (!bVerticalWriting && (SID_ATTR_PARA_LRSPACE_VERTICAL == nWhich)) ||
                 ( bVerticalWriting && (SID_ATTR_PARA_LRSPACE == nWhich))
                )
            {
                rSet.DisableItem(nWhich);
            }
            else
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                if ( !IsTabColFromDoc() )
                {
                    aLR = static_cast<const SvxLRSpaceItem&>(aCoreSet.Get(RES_LR_SPACE));

                    // #i23726#
                    if (m_pNumRuleNodeFromDoc)
                    {
                        short nOffset = static_cast< short >(aLR.GetTextLeft() +
                                        // #i42922# Mouse move of numbering label
                                        // has to consider the left indent of the paragraph
                                        m_pNumRuleNodeFromDoc->GetLeftMarginWithNum( true ) );

                        short nFLOffset;
                        m_pNumRuleNodeFromDoc->GetFirstLineOfsWithNum( nFLOffset );

                        aLR.SetLeft( nOffset + nFLOffset );
                    }
                }
                aLR.SetWhich(nWhich);
                rSet.Put(aLR);
            }
        break;
        }

        case SID_ATTR_PARA_ULSPACE:
        case SID_PARASPACE_INCREASE:
        case SID_PARASPACE_DECREASE:
        {
            SvxULSpaceItem aUL = static_cast<const SvxULSpaceItem&>(aCoreSet.Get(RES_UL_SPACE));
            SfxItemState e = aCoreSet.GetItemState(RES_UL_SPACE);
            if( e >= SfxItemState::DEFAULT )
            {
                if ( !aUL.GetUpper() && !aUL.GetLower() )
                    rSet.DisableItem( SID_PARASPACE_DECREASE );
                else if ( aUL.GetUpper() >= 5670 && aUL.GetLower() >= 5670 )
                    rSet.DisableItem( SID_PARASPACE_INCREASE );
                if ( nWhich == SID_ATTR_PARA_ULSPACE )
                {
                    aUL.SetWhich( SID_ATTR_PARA_ULSPACE );
                    rSet.Put( aUL );
                }
            }
            else
            {
                rSet.DisableItem( SID_PARASPACE_INCREASE );
                rSet.DisableItem( SID_PARASPACE_DECREASE );
                rSet.InvalidateItem( SID_ATTR_PARA_ULSPACE );
            }
        }
        break;

        case SID_RULER_BORDER_DISTANCE:
        {
            m_nLeftBorderDistance = 0;
            m_nRightBorderDistance = 0;
            if ( nSelType & nsSelectionType::SEL_GRF ||
                    nSelType & nsSelectionType::SEL_FRM ||
                    nSelType & nsSelectionType::SEL_OLE ||
                    nFrmType == FrmTypeFlags::DRAWOBJ )
                rSet.DisableItem(SID_RULER_BORDER_DISTANCE);
            else
            {
                SvxLRSpaceItem aDistLR(SID_RULER_BORDER_DISTANCE);
                if(nFrmType & FrmTypeFlags::FLY_ANY)
                {
                    if( IsTabColFromDoc() )
                    {
                        const SwRect& rFlyPrtRect = rSh.GetAnyCurRect( RECT_FLY_PRT_EMBEDDED, pPt );
                        aDistLR.SetLeft(rFlyPrtRect.Left());
                        aDistLR.SetRight(rFlyPrtRect.Left());
                    }
                    else
                    {
                        SfxItemSet aCoreSet2( GetPool(),
                                                RES_BOX, RES_BOX,
                                                SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
                        SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                        aCoreSet.Put( aBoxInfo );
                        rSh.GetFlyFrmAttr( aCoreSet );
                        const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(aCoreSet.Get(RES_BOX));
                        aDistLR.SetLeft(rBox.GetDistance(SvxBoxItemLine::LEFT));
                        aDistLR.SetRight(rBox.GetDistance(SvxBoxItemLine::RIGHT));

                        //add the paragraph border distance
                        SfxItemSet aCoreSet1( GetPool(),
                                                RES_BOX, RES_BOX,
                                                0 );
                        rSh.GetCurAttr( aCoreSet1 );
                        const SvxBoxItem& rParaBox = static_cast<const SvxBoxItem&>(aCoreSet1.Get(RES_BOX));
                        aDistLR.SetLeft(aDistLR.GetLeft() + rParaBox.GetDistance(SvxBoxItemLine::LEFT));
                        aDistLR.SetRight(aDistLR.GetRight() + rParaBox.GetDistance(SvxBoxItemLine::RIGHT));
                    }
                    rSet.Put(aDistLR);
                    m_nLeftBorderDistance  = static_cast< sal_uInt16 >(aDistLR.GetLeft());
                    m_nRightBorderDistance = static_cast< sal_uInt16 >(aDistLR.GetRight());
                }
                else if ( IsTabColFromDoc() ||
                    ( rSh.GetTableFormat() && !bFrmSelection &&
                    !(nFrmType & FrmTypeFlags::COLSECT ) ) )
                {
                    SfxItemSet aCoreSet2( GetPool(),
                                            RES_BOX, RES_BOX,
                                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
                    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                    aBoxInfo.SetTable(false);
                    aBoxInfo.SetDist(true);
                    aCoreSet2.Put(aBoxInfo);
                    rSh.GetTabBorders( aCoreSet2 );
                    const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(aCoreSet2.Get(RES_BOX));
                    aDistLR.SetLeft(rBox.GetDistance(SvxBoxItemLine::LEFT));
                    aDistLR.SetRight(rBox.GetDistance(SvxBoxItemLine::RIGHT));

                    //add the border distance of the paragraph
                    SfxItemSet aCoreSet1( GetPool(), RES_BOX, RES_BOX );
                    rSh.GetCurAttr( aCoreSet1 );
                    const SvxBoxItem& rParaBox = static_cast<const SvxBoxItem&>(aCoreSet1.Get(RES_BOX));
                    aDistLR.SetLeft(aDistLR.GetLeft() + rParaBox.GetDistance(SvxBoxItemLine::LEFT));
                    aDistLR.SetRight(aDistLR.GetRight() + rParaBox.GetDistance(SvxBoxItemLine::RIGHT));
                    rSet.Put(aDistLR);
                    m_nLeftBorderDistance  = static_cast< sal_uInt16 >(aDistLR.GetLeft());
                    m_nRightBorderDistance = static_cast< sal_uInt16 >(aDistLR.GetRight());
                }
                else if ( !rSh.IsDirectlyInSection() )
                {
                    //get the page/header/footer border distance
                    const SwFrameFormat& rMaster = rDesc.GetMaster();
                    const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(rMaster.GetAttrSet().Get(RES_BOX));
                    aDistLR.SetLeft(rBox.GetDistance(SvxBoxItemLine::LEFT));
                    aDistLR.SetRight(rBox.GetDistance(SvxBoxItemLine::RIGHT));

                    const SvxBoxItem* pBox = 0;
                    if(nFrmType & FrmTypeFlags::HEADER)
                    {
                        rMaster.GetHeader();
                        const SwFormatHeader& rHeaderFormat = rMaster.GetHeader();
                        SwFrameFormat *pHeaderFormat = const_cast<SwFrameFormat*>(rHeaderFormat.GetHeaderFormat());
                        if( pHeaderFormat )// #i80890# if rDesc is not the one belonging to the current page is might crash
                            pBox = & (const SvxBoxItem&)pHeaderFormat->GetBox();
                    }
                    else if(nFrmType & FrmTypeFlags::FOOTER )
                    {
                        const SwFormatFooter& rFooterFormat = rMaster.GetFooter();
                        SwFrameFormat *pFooterFormat = const_cast<SwFrameFormat*>(rFooterFormat.GetFooterFormat());
                        if( pFooterFormat )// #i80890# if rDesc is not the one belonging to the current page is might crash
                            pBox = & (const SvxBoxItem&)pFooterFormat->GetBox();
                    }
                    if(pBox)
                    {
                        aDistLR.SetLeft(pBox->GetDistance(SvxBoxItemLine::LEFT));
                        aDistLR.SetRight(pBox->GetDistance(SvxBoxItemLine::RIGHT));
                    }

                    //add the border distance of the paragraph
                    SfxItemSet aCoreSetTmp( GetPool(),
                                            RES_BOX, RES_BOX,
                                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
                    rSh.GetCurAttr( aCoreSetTmp );
                    const SvxBoxItem& rParaBox = static_cast<const SvxBoxItem&>(aCoreSetTmp.Get(RES_BOX));
                    aDistLR.SetLeft(aDistLR.GetLeft() + rParaBox.GetDistance(SvxBoxItemLine::LEFT));
                    aDistLR.SetRight(aDistLR.GetRight() + rParaBox.GetDistance(SvxBoxItemLine::RIGHT));
                    rSet.Put(aDistLR);
                    m_nLeftBorderDistance  = static_cast< sal_uInt16 >(aDistLR.GetLeft());
                    m_nRightBorderDistance = static_cast< sal_uInt16 >(aDistLR.GetRight());
                }
            }
        }
        break;

        case SID_RULER_TEXT_RIGHT_TO_LEFT:
        {
            if ( nSelType & nsSelectionType::SEL_GRF ||
                    nSelType & nsSelectionType::SEL_FRM ||
                    nSelType & nsSelectionType::SEL_OLE ||
                    nFrmType == FrmTypeFlags::DRAWOBJ)
                rSet.DisableItem(nWhich);
            else
            {
                bool bFlag = rSh.IsInRightToLeftText();
                rSet.Put(SfxBoolItem(nWhich, bFlag));
            }
        }
        break;

        case SID_RULER_BORDERS_VERTICAL:
        case SID_RULER_BORDERS:
        {
            bool bFrameHasVerticalColumns(false);
            {
                bool bFrameRTL;
                bool bFrameVertL2R;
                bFrameHasVerticalColumns = rSh.IsFrmVertical(false, bFrameRTL, bFrameVertL2R) &&
                                           bFrmSelection;
            }
            bool bHasTable = ( IsTabColFromDoc() ||
                    ( rSh.GetTableFormat() && !bFrmSelection &&
                    !(nFrmType & FrmTypeFlags::COLSECT ) ) );

            bool bTableVertical = bHasTable && rSh.IsTableVertical();

            if(((SID_RULER_BORDERS_VERTICAL == nWhich) &&
                ((bHasTable && !bTableVertical) ||
                 (!bVerticalWriting && !bFrmSelection && !bHasTable ) ||
                 ( bFrmSelection && !bFrameHasVerticalColumns))) ||
               ((SID_RULER_BORDERS == nWhich) &&
                ((bHasTable && bTableVertical) ||
                 (bVerticalWriting && !bFrmSelection&& !bHasTable) || bFrameHasVerticalColumns)))
                rSet.DisableItem(nWhich);
            else if ( bHasTable )
            {
                SwTabCols aTabCols;
                size_t nNum = 0;
                if ( ( m_bSetTabColFromDoc = IsTabColFromDoc() ) )
                {
                    rSh.GetMouseTabCols( aTabCols, m_aTabColFromDocPos );
                    nNum = rSh.GetCurMouseTabColNum( m_aTabColFromDocPos );
                }
                else
                {
                    rSh.GetTabCols( aTabCols );
                    nNum = rSh.GetCurTabColNum();
                    if(rSh.IsTableRightToLeft())
                        nNum = aTabCols.Count() - nNum;
                }

                OSL_ENSURE(nNum <= aTabCols.Count(), "TabCol not found");
                const int nLft = aTabCols.GetLeftMin() + aTabCols.GetLeft();
                const int nRgt = (bTableVertical ? nPageHeight : nPageWidth) -
                                 (aTabCols.GetLeftMin() + aTabCols.GetRight());

                const sal_uInt16 nL = static_cast< sal_uInt16 >(nLft > 0 ? nLft : 0);
                const sal_uInt16 nR = static_cast< sal_uInt16 >(nRgt > 0 ? nRgt : 0);

                SvxColumnItem aColItem(nNum, nL, nR);

                long nStart = 0;
                long nEnd = 0;

                //columns in right-to-left tables need to be mirrored
                bool bIsTableRTL =
                    IsTabColFromDoc() ?
                          rSh.IsMouseTableRightToLeft(m_aTabColFromDocPos)
                        : rSh.IsTableRightToLeft();
                if(bIsTableRTL)
                {
                    for ( size_t i = aTabCols.Count(); i; --i )
                    {
                        const SwTabColsEntry& rEntry = aTabCols.GetEntry( i - 1 );
                        nEnd = aTabCols.GetRight() - rEntry.nPos;
                        SvxColumnDescription aColDesc( nStart, nEnd,
                                                    aTabCols.GetRight() - rEntry.nMax,
                                                    aTabCols.GetRight() - rEntry.nMin,
                                                    !aTabCols.IsHidden(i - 1) );
                        aColItem.Append(aColDesc);
                        nStart = nEnd;
                    }
                    SvxColumnDescription aColDesc(nStart,
                                    aTabCols.GetRight() - aTabCols.GetLeft(), true);
                    aColItem.Append(aColDesc);
                }
                else
                {
                    for ( size_t i = 0; i < aTabCols.Count(); ++i )
                    {
                        const SwTabColsEntry& rEntry = aTabCols.GetEntry( i );
                        nEnd = rEntry.nPos - aTabCols.GetLeft();
                        SvxColumnDescription aColDesc( nStart, nEnd,
                                rEntry.nMin - aTabCols.GetLeft(), rEntry.nMax - aTabCols.GetLeft(),
                                                    !aTabCols.IsHidden(i) );
                        aColItem.Append(aColDesc);
                        nStart = nEnd;
                    }
                    SvxColumnDescription aColDesc(nStart, aTabCols.GetRight() - aTabCols.GetLeft(),
                                0, 0, true);
                    aColItem.Append(aColDesc);
                }
                rSet.Put(aColItem, nWhich);
            }
            else if ( bFrmSelection || nFrmType & ( FrmTypeFlags::COLUMN | FrmTypeFlags::COLSECT ) )
            {
                // Out of frame or page?
                sal_uInt16 nNum = 0;
                if(bFrmSelection)
                {
                    const SwFrameFormat* pFormat = rSh.GetFlyFrameFormat();
                    if(pFormat)
                        nNum = pFormat->GetCol().GetNumCols();
                }
                else
                    nNum = rSh.GetCurColNum();

                if(
                    // For that matter FrmTypeFlags::COLSECT should not be included
                    // if the border is selected!
                    !bFrmSelection &&
                     nFrmType & FrmTypeFlags::COLSECT )
                {
                    const SwSection *pSect = rSh.GetAnySection(false, pPt);
                    OSL_ENSURE( pSect, "Which section?");
                    if( pSect )
                    {
                        SwSectionFormat const *pFormat = pSect->GetFormat();
                        const SwFormatCol& rCol = pFormat->GetCol();
                        if(rSh.IsInRightToLeftText())
                            nNum = rCol.GetColumns().size() - nNum;
                        else
                            --nNum;
                        SvxColumnItem aColItem(nNum);
                        SwRect aRect = rSh.GetAnyCurRect(RECT_SECTION_PRT, pPt);
                        const SwRect aTmpRect = rSh.GetAnyCurRect(RECT_SECTION, pPt);

                        ::lcl_FillSvxColumn(rCol, bVerticalWriting ? aRect.Height() : aRect.Width(), aColItem, 0);

                        if(bVerticalWriting)
                        {
                            aRect.Pos() += Point(aTmpRect.Left(), aTmpRect.Top());
                            aRect.Pos().Y() -= rPageRect.Top();
                            aColItem.SetLeft(aRect.Top());
                            aColItem.SetRight(nPageHeight - aRect.Bottom());
                        }
                        else
                        {
                            aRect.Pos() += aTmpRect.Pos();

                            // make relative to page position:
                            aColItem.SetLeft ((sal_uInt16)( aRect.Left() - rPageRect.Left() ));
                            aColItem.SetRight((sal_uInt16)( rPageRect.Right() - aRect.Right()));
                        }
                        aColItem.SetOrtho(aColItem.CalcOrtho());

                        rSet.Put(aColItem, nWhich);
                    }
                }
                else if( bFrmSelection || nFrmType & FrmTypeFlags::FLY_ANY )
                {
                    // Columns in frame
                    if ( nNum  )
                    {
                        const SwFrameFormat* pFormat = rSh.GetFlyFrameFormat() ;

                        const SwFormatCol& rCol = pFormat->GetCol();
                        if(rSh.IsInRightToLeftText())
                            nNum = rCol.GetColumns().size() - nNum;
                        else
                            nNum--;
                        SvxColumnItem aColItem(nNum);
                        const SwRect &rSizeRect = rSh.GetAnyCurRect(RECT_FLY_PRT_EMBEDDED, pPt);

                        bool bUseVertical = bFrameHasVerticalColumns || (!bFrmSelection && bVerticalWriting);
                        const long lWidth = bUseVertical ? rSizeRect.Height() : rSizeRect.Width();
                        const SwRect &rRect = rSh.GetAnyCurRect(RECT_FLY_EMBEDDED, pPt);
                        long nDist2 = ((bUseVertical ? rRect.Height() : rRect.Width()) - lWidth) /2;
                        ::lcl_FillSvxColumn(rCol, lWidth, aColItem, nDist2);

                        SfxItemSet aFrameSet(GetPool(), RES_LR_SPACE, RES_LR_SPACE);
                        rSh.GetFlyFrmAttr( aFrameSet );

                        if(bUseVertical)
                        {
                            aColItem.SetLeft(rRect.Top()- rPageRect.Top());
                            aColItem.SetRight(nPageHeight + rPageRect.Top() - rRect.Bottom());
                        }
                        else
                        {
                            aColItem.SetLeft(rRect.Left() - rPageRect.Left());
                            aColItem.SetRight(rPageRect.Right() - rRect.Right());
                        }

                        aColItem.SetOrtho(aColItem.CalcOrtho());

                        rSet.Put(aColItem, nWhich);
                    }
                    else
                        rSet.DisableItem(nWhich);
                }
                else
                {   // Columns on the page
                    const SwFrameFormat& rMaster = rDesc.GetMaster();
                    SwFormatCol aCol(rMaster.GetCol());
                    if(rFrameDir.GetValue() == FRMDIR_HORI_RIGHT_TOP)
                        nNum = aCol.GetColumns().size() - nNum;
                    else
                        nNum--;

                    SvxColumnItem aColItem(nNum);
                    const SwRect aPrtRect = rSh.GetAnyCurRect(RECT_PAGE_PRT, pPt);
                    const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(rMaster.GetFormatAttr(RES_BOX));
                    long nDist = rBox.GetDistance();

                    lcl_FillSvxColumn(
                        aCol,
                        bVerticalWriting ? aPrtRect.Height() : aPrtRect.Width(),
                        aColItem, nDist);

                    if(bBrowse)
                    {
                        if (bVerticalWriting)
                        {
                            aColItem.SetLeft((sal_uInt16)rPagePrtRect.Top());
                            aColItem.SetRight(sal_uInt16(nPageHeight - rPagePrtRect.Bottom()));
                        }
                        else
                        {
                            aColItem.SetLeft((sal_uInt16)rPagePrtRect.Left());
                            aColItem.SetRight(sal_uInt16(nPageWidth - rPagePrtRect.Right()));
                        }
                    }
                    else
                    {
                        if (bVerticalWriting)
                        {
                            SvxULSpaceItem aUL( rDesc.GetMaster().GetULSpace() );
                            aColItem.SetLeft (aUL.GetUpper());
                            aColItem.SetRight(aUL.GetLower());
                        }
                        else
                        {
                            aColItem.SetLeft (aPageLRSpace.GetLeft());
                            aColItem.SetRight(aPageLRSpace.GetRight());
                        }
                    }
                    aColItem.SetOrtho(aColItem.CalcOrtho());

                    rSet.Put(aColItem, nWhich);
                }
            }
            else
                rSet.DisableItem(nWhich);
            break;
        }

        case SID_RULER_ROWS :
        case SID_RULER_ROWS_VERTICAL:
        {
            bool bFrameHasVerticalColumns(false);
            {
                bool bFrameRTL;
                bool bFrameVertL2R;
                bFrameHasVerticalColumns = rSh.IsFrmVertical(false, bFrameRTL, bFrameVertL2R) &&
                                           bFrmSelection;
            }

            if(((SID_RULER_ROWS == nWhich) &&
                ((!bVerticalWriting && !bFrmSelection) || (bFrmSelection && !bFrameHasVerticalColumns))) ||
               ((SID_RULER_ROWS_VERTICAL == nWhich) &&
                ((bVerticalWriting && !bFrmSelection) || bFrameHasVerticalColumns)))
                rSet.DisableItem(nWhich);
            else if ( IsTabRowFromDoc() ||
                    ( rSh.GetTableFormat() && !bFrmSelection &&
                    !(nFrmType & FrmTypeFlags::COLSECT ) ) )
            {
                SwTabCols aTabCols;
                if ( ( m_bSetTabRowFromDoc = IsTabRowFromDoc() ) )
                {
                    rSh.GetMouseTabRows( aTabCols, m_aTabColFromDocPos );
                }
                else
                {
                    rSh.GetTabRows( aTabCols );
                }

                const int nLft = aTabCols.GetLeftMin();
                const int nRgt = (bVerticalWriting ? nPageWidth : nPageHeight) -
                                 (aTabCols.GetLeftMin() + aTabCols.GetRight());

                const sal_uInt16 nL = static_cast< sal_uInt16 >(nLft > 0 ? nLft : 0);
                const sal_uInt16 nR = static_cast< sal_uInt16 >(nRgt > 0 ? nRgt : 0);

                SvxColumnItem aColItem(0, nL, nR);

                long nStart = 0;
                long nEnd = 0;

                for ( size_t i = 0; i < aTabCols.Count(); ++i )
                {
                    const SwTabColsEntry& rEntry = aTabCols.GetEntry( i );
                    if(bVerticalWriting)
                    {
                        nEnd = aTabCols.GetRight() - rEntry.nPos;
                        SvxColumnDescription aColDesc( nStart, nEnd,
                                                    aTabCols.GetRight() - rEntry.nMax,
                                                    aTabCols.GetRight() - rEntry.nMin,
                                                    !aTabCols.IsHidden(i) );
                        aColItem.Append(aColDesc);
                    }
                    else
                    {
                        nEnd  = rEntry.nPos - aTabCols.GetLeft();
                        SvxColumnDescription aColDesc( nStart, nEnd,
                                                    rEntry.nMin - aTabCols.GetLeft(),
                                                    rEntry.nMax - aTabCols.GetLeft(),
                                                    !aTabCols.IsHidden(i) );
                        aColItem.Append(aColDesc);
                    }
                    nStart = nEnd;
                }
                if(bVerticalWriting)
                    nEnd = aTabCols.GetRight();
                else
                    nEnd = aTabCols.GetLeft();
                // put a position protection when the last row cannot be moved
                // due to a page break inside of a row
                if(!aTabCols.IsLastRowAllowedToChange())
                    bPutContentProtection = true;

                SvxColumnDescription aColDesc( nStart, nEnd,
                                            aTabCols.GetRight(),
                                            aTabCols.GetRight(),
                                            false );
                aColItem.Append(aColDesc);

                rSet.Put(aColItem, nWhich);
            }
            else
                rSet.DisableItem(nWhich);
        }
        break;

        case SID_RULER_PAGE_POS:
        {
            SvxPagePosSizeItem aPagePosSize(
                    Point( rPageRect.Left(), rPageRect.Top()) , nPageWidth, nPageHeight);

            rSet.Put(aPagePosSize);
            break;
        }

        case SID_RULER_LR_MIN_MAX:
        {
            Rectangle aRectangle;
            if( ( nFrmType & FrmTypeFlags::COLSECT ) && !IsTabColFromDoc() &&
                ( nFrmType & ( FrmTypeFlags::TABLE|FrmTypeFlags::COLUMN ) ) )
            {
                if( nFrmType & FrmTypeFlags::TABLE )
                {
                    const size_t nNum = rSh.GetCurTabColNum();
                    SwTabCols aTabCols;
                    rSh.GetTabCols( aTabCols );

                    const int nLft = aTabCols.GetLeftMin() + aTabCols.GetLeft();
                    const int nRgt = nPageWidth -(aTabCols.GetLeftMin() + aTabCols.GetRight());

                    const sal_uInt16 nL = static_cast< sal_uInt16 >(nLft > 0 ? nLft : 0);
                    const sal_uInt16 nR = static_cast< sal_uInt16 >(nRgt > 0 ? nRgt : 0);

                    aRectangle.Left() = nL;
                    if(nNum > 1)
                        aRectangle.Left() += aTabCols[nNum - 2];
                    if(nNum)
                        aRectangle.Left() += MINLAY;
                    if(aTabCols.Count() <= nNum + 1 )
                        aRectangle.Right() = nR;
                    else
                        aRectangle.Right() = nPageWidth - (nL + aTabCols[nNum + 1]);

                    if(nNum < aTabCols.Count())
                        aRectangle.Right() += MINLAY;
                }
                else
                {
                    const SwFrameFormat* pFormat =  rSh.GetFlyFrameFormat();
                    const SwFormatCol* pCols = pFormat ? &pFormat->GetCol():
                                                   &rDesc.GetMaster().GetCol();
                    const SwColumns& rCols = pCols->GetColumns();
                    sal_uInt16 nNum = rSh.GetCurOutColNum();
                    const sal_uInt16 nCount = std::min(sal_uInt16(nNum + 1), sal_uInt16(rCols.size()));
                    const SwRect aRect( rSh.GetAnyCurRect( pFormat
                                                    ? RECT_FLY_PRT_EMBEDDED
                                                    : RECT_PAGE_PRT, pPt ));
                    const SwRect aAbsRect( rSh.GetAnyCurRect( pFormat
                                                    ? RECT_FLY_EMBEDDED
                                                    : RECT_PAGE, pPt ));

                    // The width of the frame or within the page margins.
                    const sal_uInt16 nTotalWidth = (sal_uInt16)aRect.Width();
                    // The entire frame width - The difference is twice the distance to the edge.
                    const sal_uInt16 nOuterWidth = (sal_uInt16)aAbsRect.Width();
                    int nWidth = 0,
                        nEnd = 0;
                    aRectangle.Left() = 0;
                    for ( sal_uInt16 i = 0; i < nCount; ++i )
                    {
                        const SwColumn* pCol = &rCols[i];
                        const int nStart = pCol->GetLeft() + nWidth;
                        if(i == nNum - 2)
                            aRectangle.Left() = nStart;
                        nWidth += pCols->CalcColWidth( i, nTotalWidth );
                        nEnd = nWidth - pCol->GetRight();
                    }
                    aRectangle.Right() = rPageRect.Right() - nEnd;
                    aRectangle.Left() -= rPageRect.Left();

                    if(nNum > 1)
                    {
                        aRectangle.Left() += MINLAY;
                        aRectangle.Left() += aRect.Left();
                    }
                    if(pFormat) // Range in frame - here you may up to the edge
                        aRectangle.Left()  = aRectangle.Right() = 0;
                    else
                    {
                        // Move the rectangle to the correct absolute position.
                        aRectangle.Left() += aAbsRect.Left();
                        aRectangle.Right() -= aAbsRect.Left();
                        // Include distance to the border.
                        aRectangle.Right() -= (nOuterWidth - nTotalWidth) / 2;
                    }

                    if(nNum < rCols.size())
                    {
                        aRectangle.Right() += MINLAY;
                    }
                    else
                        // Right is only the margin now.
                        aRectangle.Right() = 0;

                }
            }
            else if ( ((nFrmType & FrmTypeFlags::TABLE) || IsTabColFromDoc()) &&
                 !bFrmSelection )
            {
                bool bColumn;
                if ( IsTabColFromDoc() )
                    bColumn = rSh.GetCurMouseColNum( m_aTabColFromDocPos ) != 0;
                else
                    bColumn = bool(nFrmType & (FrmTypeFlags::COLUMN|FrmTypeFlags::FLY_ANY|FrmTypeFlags::COLSECTOUTTAB));

                if ( !bColumn )
                {
                    if( nFrmType & FrmTypeFlags::FLY_ANY && IsTabColFromDoc() )
                    {
                        SwRect aRect( rSh.GetAnyCurRect(
                                            RECT_FLY_PRT_EMBEDDED, pPt ) );
                        aRect.Pos() += rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,
                                                                pPt ).Pos();

                        aRectangle.Left()  = aRect.Left() - rPageRect.Left();
                        aRectangle.Right() = rPageRect.Right() - aRect.Right();
                    }
                    else if( bBrowse )
                    {
                        aRectangle.Left()  = rPagePrtRect.Left();
                        aRectangle.Right() = nPageWidth - rPagePrtRect.Right();
                    }
                    else
                    {
                        aRectangle.Left()  = aPageLRSpace.GetLeft();
                        aRectangle.Right() = aPageLRSpace.GetRight();
                    }
                }
                else
                {   // Here only for table in multi-column pages and borders.
                    bool bSectOutTable = bool(nFrmType & FrmTypeFlags::TABLE);
                    bool bFrame = bool(nFrmType & FrmTypeFlags::FLY_ANY);
                    bool bColSct = bool(nFrmType & ( bSectOutTable
                                                    ? FrmTypeFlags::COLSECTOUTTAB
                                                    : FrmTypeFlags::COLSECT )
                                                );
                    //So you can also drag with the mouse, without being in the table.
                    CurRectType eRecType = RECT_PAGE_PRT;
                    size_t nNum = IsTabColFromDoc() ?
                                rSh.GetCurMouseColNum( m_aTabColFromDocPos ):
                                rSh.GetCurOutColNum();
                    const SwFrameFormat* pFormat = NULL;
                    if( bColSct )
                    {
                        eRecType = bSectOutTable ? RECT_OUTTABSECTION
                                               : RECT_SECTION;
                        const SwSection *pSect = rSh.GetAnySection( bSectOutTable, pPt );
                        OSL_ENSURE( pSect, "Which section?");
                        pFormat = pSect->GetFormat();
                    }
                    else if( bFrame )
                    {
                        pFormat = rSh.GetFlyFrameFormat();
                        eRecType = RECT_FLY_PRT_EMBEDDED;
                    }

                    const SwFormatCol* pCols = pFormat ? &pFormat->GetCol():
                                                   &rDesc.GetMaster().GetCol();
                    const SwColumns& rCols = pCols->GetColumns();
                    const sal_uInt16 nBorder = pFormat ? pFormat->GetBox().GetDistance() :
                                                  rDesc.GetMaster().GetBox().GetDistance();

                    // RECT_FLY_PRT_EMBEDDED returns the relative position to RECT_FLY_EMBEDDED
                    // the absolute position must be added here

                    SwRect aRect( rSh.GetAnyCurRect( eRecType, pPt ) );
                    if(RECT_FLY_PRT_EMBEDDED == eRecType)
                        aRect.Pos() += rSh.GetAnyCurRect( RECT_FLY_EMBEDDED,
                                                                pPt ).Pos();

                    const sal_uInt16 nTotalWidth = (sal_uInt16)aRect.Width();
                    // Initialize nStart and nEnd initialisieren for nNum == 0
                    int nWidth = 0,
                        nStart = 0,
                        nEnd = nTotalWidth;

                    if( nNum > rCols.size() )
                    {
                        OSL_ENSURE( false, "wrong FormatCol is being edited!" );
                        nNum = rCols.size();
                    }

                    for( size_t i = 0; i < nNum; ++i )
                    {
                        const SwColumn* pCol = &rCols[i];
                        nStart = pCol->GetLeft() + nWidth;
                        nWidth += pCols->CalcColWidth( static_cast<sal_uInt16>(i), nTotalWidth );
                        nEnd = nWidth - pCol->GetRight();
                    }
                    if( bFrame || bColSct )
                    {
                        aRectangle.Left()  = aRect.Left() - rPageRect.Left() + nStart;
                        aRectangle.Right() = nPageWidth - aRectangle.Left() - nEnd + nStart;
                    }
                    else if(!bBrowse)
                    {
                        aRectangle.Left()  = aPageLRSpace.GetLeft() + nStart;
                        aRectangle.Right() = nPageWidth - nEnd - aPageLRSpace.GetLeft();
                    }
                    else
                    {
                        long nLeft = rPagePrtRect.Left();
                        aRectangle.Left()  = nStart + nLeft;
                        aRectangle.Right() = nPageWidth - nEnd - nLeft;
                    }
                    if(!bFrame)
                    {
                        aRectangle.Left() += nBorder;
                        aRectangle.Right() -= nBorder;
                    }
                }
            }
            else if ( nFrmType & ( FrmTypeFlags::HEADER  | FrmTypeFlags::FOOTER ))
            {
                aRectangle.Left()  = aPageLRSpace.GetLeft();
                aRectangle.Right() = aPageLRSpace.GetRight();
            }
            else
                aRectangle.Left()  = aRectangle.Right() = 0;

            SfxRectangleItem aLR( SID_RULER_LR_MIN_MAX , aRectangle);
            rSet.Put(aLR);
        }
        break;

        case SID_RULER_PROTECT:
        {
            if(bFrmSelection)
            {
                FlyProtectFlags nProtect = m_pWrtShell->IsSelObjProtected( FlyProtectFlags::Size|FlyProtectFlags::Pos|FlyProtectFlags::Content );

                SvxProtectItem aProt(SID_RULER_PROTECT);
                aProt.SetContentProtect(bool(nProtect & FlyProtectFlags::Content));
                aProt.SetSizeProtect   (bool(nProtect & FlyProtectFlags::Size));
                aProt.SetPosProtect    (bool(nProtect & FlyProtectFlags::Pos));
                rSet.Put(aProt);
            }
            else
            {
                SvxProtectItem aProtect(SID_RULER_PROTECT);
                if(bBrowse && !(nFrmType & (FrmTypeFlags::DRAWOBJ|FrmTypeFlags::COLUMN)) && !rSh.GetTableFormat())
                {
                    aProtect.SetSizeProtect(true);
                    aProtect.SetPosProtect(true);
                }
                rSet.Put(aProtect);
            }
        }
        break;
        }
        nWhich = aIter.NextWhich();
    }
    if(bPutContentProtection)
    {
        SvxProtectItem aProtect(SID_RULER_PROTECT);
        aProtect.SetContentProtect(true);
        rSet.Put(aProtect);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
