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

#include <cmdid.h>
#include <hintids.hxx>
#include <algorithm>
#include <svl/eitem.hxx>
#include <tools/fract.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/pageitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/builder.hxx>
#include <vcl/graph.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <tgrditem.hxx>
#include <viewopt.hxx>
#include <colex.hxx>
#include <colmgr.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <svx/svxids.hrc>

// Taking the updated values from the set
void SwPageExample::UpdateExample( const SfxItemSet& rSet )
{
    if (SfxItemState::DEFAULT <= rSet.GetItemState(RES_FRAMEDIR))
    {
        const SvxFrameDirectionItem& rDirItem = rSet.Get(RES_FRAMEDIR);
        m_bVertical = rDirItem.GetValue() == SvxFrameDirection::Vertical_RL_TB||
                    rDirItem.GetValue() == SvxFrameDirection::Vertical_LR_TB;
    }

    SfxItemPool* pPool = rSet.GetPool();
    sal_uInt16 nWhich = pPool->GetWhich( SID_ATTR_PAGE );
    if ( rSet.GetItemState( nWhich, false ) == SfxItemState::SET )
    {
        // alignment
        const SvxPageItem* pPage = static_cast<const SvxPageItem*>(&rSet.Get( nWhich ));

        if ( pPage )
            SetUsage( pPage->GetPageUsage() );
    }

    nWhich = pPool->GetWhich( SID_ATTR_PAGE_SIZE );

    if ( rSet.GetItemState( nWhich, false ) == SfxItemState::SET )
    {
        // orientation and size from PageItem
        const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(rSet.Get( nWhich ));
        SetSize( rSize.GetSize() );
    }
    nWhich = RES_LR_SPACE;
    if ( rSet.GetItemState( nWhich, false ) == SfxItemState::SET )
    {
        // set left and right border
        const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(rSet.Get( nWhich ));

        SetLeft( rLRSpace.GetLeft() );
        SetRight( rLRSpace.GetRight() );
    }
    else
    {
        SetLeft( 0 );
        SetRight( 0 );
    }

    nWhich = RES_UL_SPACE;

    if ( rSet.GetItemState( nWhich, false ) == SfxItemState::SET )
    {
        // set upper and lower border
        const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(rSet.Get( nWhich ));

        SetTop( rULSpace.GetUpper() );
        SetBottom( rULSpace.GetLower() );
    }
    else
    {
        SetTop( 0 );
        SetBottom( 0 );
    }

    // evaluate header-attributes
    const SfxPoolItem* pItem;
    if( SfxItemState::SET == rSet.GetItemState( pPool->GetWhich( SID_ATTR_PAGE_HEADERSET),
            false, &pItem ) )
    {
        const SfxItemSet& rHeaderSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            static_cast<const SfxBoolItem&>(rHeaderSet.Get( pPool->GetWhich( SID_ATTR_PAGE_ON ) ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                static_cast<const SvxSizeItem&>(rHeaderSet.Get(pPool->GetWhich(SID_ATTR_PAGE_SIZE)));

            const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(rHeaderSet.Get(
                                        pPool->GetWhich(SID_ATTR_ULSPACE)));
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(rHeaderSet.Get(
                                        pPool->GetWhich(SID_ATTR_LRSPACE)));

            SetHdHeight( rSize.GetSize().Height() - rUL.GetLower());
            SetHdDist( rUL.GetLower() );
            SetHdLeft( rLR.GetLeft() );
            SetHdRight( rLR.GetRight() );
            SetHeader( true );

            if(SfxItemState::SET == rHeaderSet.GetItemState(RES_BACKGROUND))
            {
                // create FillAttributes from SvxBrushItem //SetHdColor(rItem.GetColor());
                const SvxBrushItem& rItem = rHeaderSet.Get(RES_BACKGROUND);
                SfxItemSet aTempSet(*rHeaderSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

                setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                setHeaderFillAttributes(
                    std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(
                            aTempSet));
            }
        }
        else
            SetHeader( false );
    }

    if( SfxItemState::SET == rSet.GetItemState( pPool->GetWhich( SID_ATTR_PAGE_FOOTERSET),
            false, &pItem ) )
    {
        const SfxItemSet& rFooterSet = static_cast<const SvxSetItem*>(pItem)->GetItemSet();
        const SfxBoolItem& rFooterOn = rFooterSet.Get( SID_ATTR_PAGE_ON );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                static_cast<const SvxSizeItem&>(rFooterSet.Get( pPool->GetWhich( SID_ATTR_PAGE_SIZE ) ));

            const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(rFooterSet.Get(
                                        pPool->GetWhich( SID_ATTR_ULSPACE ) ));
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(rFooterSet.Get(
                                        pPool->GetWhich( SID_ATTR_LRSPACE ) ));

            SetFtHeight( rSize.GetSize().Height() - rUL.GetUpper());
            SetFtDist( rUL.GetUpper() );
            SetFtLeft( rLR.GetLeft() );
            SetFtRight( rLR.GetRight() );
            SetFooter( true );

            if( rFooterSet.GetItemState( RES_BACKGROUND ) == SfxItemState::SET )
            {
                // create FillAttributes from SvxBrushItem //SetFtColor(rItem.GetColor());
                const SvxBrushItem& rItem = rFooterSet.Get(RES_BACKGROUND);
                SfxItemSet aTempSet(*rFooterSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

                setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                setFooterFillAttributes(
                    std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(
                            aTempSet));
            }
        }
        else
            SetFooter( false );
    }

    if(SfxItemState::SET == rSet.GetItemState(RES_BACKGROUND, false, &pItem))
    {
        // create FillAttributes from SvxBrushItem
        const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(*pItem);
        SfxItemSet aTempSet(*rSet.GetPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});

        setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
        setPageFillAttributes(
            std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(
                    aTempSet));
    }

    Invalidate();
}

void SwColExample::DrawPage(vcl::RenderContext& rRenderContext, const Point& rOrg,
                            const bool bSecond, const bool bEnabled)
{
    SwPageExample::DrawPage(rRenderContext, rOrg, bSecond, bEnabled);
    if (!pColMgr)
        return;
    sal_uInt16 nColumnCount = pColMgr->GetCount();
    if (!nColumnCount)
        return;

    long nL = GetLeft();
    long nR = GetRight();

    if (GetUsage() == SvxPageUsage::Mirror && !bSecond)
    {
        // swap for mirrored
        nL = GetRight();
        nR = GetLeft();
    }

    rRenderContext.SetFillColor(COL_LIGHTGRAY);
    tools::Rectangle aRect;
    aRect.SetRight( rOrg.X() + GetSize().Width() - nR );
    aRect.SetLeft( rOrg.X() + nL );
    aRect.SetTop( rOrg.Y() + GetTop() + GetHdHeight() + GetHdDist() );
    aRect.SetBottom( rOrg.Y() + GetSize().Height() - GetBottom() - GetFtHeight() - GetFtDist() );
    rRenderContext.DrawRect(aRect);

    const tools::Rectangle aDefineRect(aRect);
    const drawinglayer::attribute::SdrAllFillAttributesHelperPtr& rFillAttributes = getPageFillAttributes();

    if (!rFillAttributes.get() || !rFillAttributes->isUsed())
    {
        // If there is no fill, use fallback color
        const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
        const Color& rFieldColor = rStyleSettings.GetFieldColor();

        setPageFillAttributes(
            std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(
                rFieldColor));
    }

    // #97495# make sure that the automatic column width's are always equal
    bool bAutoWidth = pColMgr->IsAutoWidth();
    sal_Int32 nAutoColWidth = 0;
    if (bAutoWidth)
    {
        sal_Int32 nColumnWidthSum = 0;
        for (sal_uInt16 i = 0; i < nColumnCount; ++i)
            nColumnWidthSum += pColMgr->GetColWidth( i );
        nAutoColWidth = nColumnWidthSum / nColumnCount;
    }

    for (sal_uInt16 i = 0; i < nColumnCount; ++i)
    {
        if (!bAutoWidth)
            nAutoColWidth = pColMgr->GetColWidth(i);

        if (!m_bVertical)
            aRect.SetRight( aRect.Left() + nAutoColWidth );
        else
            aRect.SetBottom( aRect.Top() + nAutoColWidth );

        // use primitive draw command
        drawFillAttributes(rRenderContext, getPageFillAttributes(), aRect, aDefineRect);

        if (i < nColumnCount - 1)
        {
            if (!m_bVertical)
                aRect.SetLeft( aRect.Right() + pColMgr->GetGutterWidth(i) );
            else
                aRect.SetTop( aRect.Bottom() + pColMgr->GetGutterWidth(i) );
        }
    }
    if (pColMgr->HasLine())
    {
        Point aUp(rOrg.X() + nL, rOrg.Y() + GetTop());
        Point aDown(rOrg.X() + nL,
                        rOrg.Y() + GetSize().Height() - GetBottom() - GetFtHeight() - GetFtDist());

        if (pColMgr->GetLineHeightPercent() != 100)
        {
            long nLength = !m_bVertical ? aDown.Y() - aUp.Y() : aDown.X() - aUp.X();
            nLength -= nLength * pColMgr->GetLineHeightPercent() / 100;
            switch (pColMgr->GetAdjust())
            {
                case COLADJ_BOTTOM:
                    if (!m_bVertical)
                        aUp.AdjustY(nLength );
                    else
                        aUp.AdjustX(nLength );
                    break;
                case COLADJ_TOP:
                    if (!m_bVertical)
                        aDown.AdjustY( -nLength );
                    else
                        aDown.AdjustX( -nLength );
                    break;
                case COLADJ_CENTER:
                    if (!m_bVertical)
                    {
                        aUp.AdjustY(nLength / 2 );
                        aDown.AdjustY( -(nLength / 2) );
                    }
                    else
                    {
                        aUp.AdjustX(nLength / 2 );
                        aDown.AdjustX( -(nLength / 2) );
                    }
                    break;
                default:
                    break; // prevent warning
            }
        }

        for (sal_uInt16 i = 0; i < nColumnCount -  1; ++i)
        {
            int nGutter = pColMgr->GetGutterWidth(i);
            int nDist = pColMgr->GetColWidth( i ) + nGutter;
            nDist -= (i == 0) ? nGutter / 2 : 0;
            if (!m_bVertical)
            {
                aUp.AdjustX(nDist );
                aDown.AdjustX(nDist );
            }
            else
            {
                aUp.AdjustY(nDist );
                aDown.AdjustY(nDist );
            }

            rRenderContext.DrawLine(aUp, aDown);
        }
    }
}

VCL_BUILDER_FACTORY(SwColExample)

SwColumnOnlyExample::SwColumnOnlyExample(vcl::Window* pParent)
    : Window(pParent)
    , m_aFrameSize(1,1)
{
    SetMapMode( MapMode( MapUnit::MapTwip ) );
    m_aWinSize = GetOptimalSize();
    m_aWinSize.AdjustHeight( -4 );
    m_aWinSize.AdjustWidth( -4 );

    m_aWinSize = PixelToLogic( m_aWinSize );

    SetBorderStyle( WindowBorderStyle::MONO );

    m_aFrameSize  = SvxPaperInfo::GetPaperSize(PAPER_A4);// DIN A4
    ::FitToActualSize(m_aCols, static_cast<sal_uInt16>(m_aFrameSize.Width()));

    long nHeight = m_aFrameSize.Height();
    Fraction aScale( m_aWinSize.Height(), nHeight );
    MapMode aMapMode( GetMapMode() );
    aMapMode.SetScaleX( aScale );
    aMapMode.SetScaleY( aScale );
    SetMapMode( aMapMode );
}

VCL_BUILDER_FACTORY(SwColumnOnlyExample)

void SwColumnOnlyExample::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    const Color& rFieldColor = rStyleSettings.GetFieldColor();
    const Color& rDlgColor = rStyleSettings.GetDialogColor();
    const Color& rFieldTextColor = SwViewOption::GetFontColor();
    Color aGrayColor(COL_LIGHTGRAY);
    if (rFieldColor == aGrayColor)
        aGrayColor.Invert();

    Size aLogSize(rRenderContext.PixelToLogic(GetOutputSizePixel()));
    tools::Rectangle aCompleteRect(Point(0,0), aLogSize);
    rRenderContext.SetLineColor(rDlgColor);
    rRenderContext.SetFillColor(rDlgColor);
    rRenderContext.DrawRect(aCompleteRect);

    rRenderContext.SetLineColor(rFieldTextColor);
    Point aTL((aLogSize.Width() - m_aFrameSize.Width()) / 2,
              (aLogSize.Height() - m_aFrameSize.Height()) / 2);
    tools::Rectangle aRect(aTL, m_aFrameSize);

    //draw a shadow rectangle
    rRenderContext.SetFillColor(COL_GRAY);
    tools::Rectangle aShadowRect(aRect);
    aShadowRect.Move(aTL.Y(), aTL.Y());
    rRenderContext.DrawRect(aShadowRect);

    rRenderContext.SetFillColor(rFieldColor);
    rRenderContext.DrawRect(aRect);

    rRenderContext.SetFillColor(aGrayColor);

    //column separator?
    long nLength = aLogSize.Height() - 2 * aTL.Y();
    Point aUp(aTL);
    Point aDown(aTL.X(), nLength);
    bool bLines = false;
    if (m_aCols.GetLineAdj() != COLADJ_NONE)
    {
        bLines = true;

        sal_uInt16 nPercent = m_aCols.GetLineHeight();
        if (nPercent != 100)
        {
            nLength -= nLength * nPercent / 100;
            switch(m_aCols.GetLineAdj())
            {
                case COLADJ_BOTTOM: aUp.AdjustY(nLength ); break;
                case COLADJ_TOP: aDown.AdjustY( -nLength ); break;
                case COLADJ_CENTER:
                        aUp.AdjustY(nLength / 2 );
                        aDown.AdjustY( -(nLength / 2) );
                break;
                default:
                    break; //prevent warning
            }
        }

    }
    const SwColumns& rCols = m_aCols.GetColumns();
    sal_uInt16 nColCount = rCols.size();
    if (nColCount)
    {
        rRenderContext.DrawRect(aRect);
        rRenderContext.SetFillColor(rFieldColor);
        tools::Rectangle aFrameRect(aTL, m_aFrameSize);
        long nSum = aTL.X();
        for (sal_uInt16 i = 0; i < nColCount; i++)
        {
            const SwColumn* pCol = &rCols[i];
            aFrameRect.SetLeft( nSum + pCol->GetLeft() ); //nSum + pCol->GetLeft() + aTL.X();
            nSum += pCol->GetWishWidth();
            aFrameRect.SetRight( nSum - pCol->GetRight() );
            rRenderContext.DrawRect(aFrameRect);
        }
        if (bLines)
        {
            nSum = aTL.X();
            for (sal_uInt16 i = 0; i < nColCount - 1; i++)
            {
                nSum += rCols[i].GetWishWidth();
                aUp.setX( nSum );
                aDown.setX( nSum );
                rRenderContext.DrawLine(aUp, aDown);
            }
        }
    }
}

void  SwColumnOnlyExample::SetColumns(const SwFormatCol& rCol)
{
    m_aCols = rCol;
    sal_uInt16 nWishSum = m_aCols.GetWishWidth();
    long nFrameWidth = m_aFrameSize.Width();
    SwColumns& rCols = m_aCols.GetColumns();
    sal_uInt16 nColCount = rCols.size();

    for(sal_uInt16 i = 0; i < nColCount; i++)
    {
        SwColumn* pCol = &rCols[i];
        long nWish = pCol->GetWishWidth();
        nWish *= nFrameWidth;
        nWish /= nWishSum;
        pCol->SetWishWidth(static_cast<sal_uInt16>(nWish));
        long nLeft = pCol->GetLeft();
        nLeft *= nFrameWidth;
        nLeft /= nWishSum;
        pCol->SetLeft(static_cast<sal_uInt16>(nLeft));
        long nRight = pCol->GetRight();
        nRight *= nFrameWidth;
        nRight /= nWishSum;
        pCol->SetRight(static_cast<sal_uInt16>(nRight));
    }
    // #97495# make sure that the automatic column width's are always equal
    if(nColCount && m_aCols.IsOrtho())
    {
        sal_Int32 nColumnWidthSum = 0;
        sal_uInt16 i;
        for(i = 0; i < nColCount; ++i)
        {
            SwColumn* pCol = &rCols[i];
            nColumnWidthSum += pCol->GetWishWidth();
            nColumnWidthSum -= (pCol->GetRight() + pCol->GetLeft());
        }
        nColumnWidthSum /= nColCount;
        for(i = 0; i < nColCount; ++i)
        {
            SwColumn* pCol = &rCols[i];
            pCol->SetWishWidth( static_cast< sal_uInt16 >(nColumnWidthSum + pCol->GetRight() + pCol->GetLeft()));
        }
    }
}

Size SwColumnOnlyExample::GetOptimalSize() const
{
    return LogicToPixel(Size(75, 46), MapMode(MapUnit::MapAppFont));
}

SwPageGridExample::~SwPageGridExample()
{
    disposeOnce();
}

void SwPageGridExample::dispose()
{
    pGridItem.reset();
    SwPageExample::dispose();
}

void SwPageGridExample::DrawPage(vcl::RenderContext& rRenderContext, const Point& rOrg,
                                 const bool bSecond, const bool bEnabled)
{
    SwPageExample::DrawPage(rRenderContext, rOrg, bSecond, bEnabled);

    if (!pGridItem || !pGridItem->GetGridType())
        return;

    //paint the grid now
    Color aLineColor = pGridItem->GetColor();
    if (aLineColor == COL_AUTO)
    {
        aLineColor = rRenderContext.GetFillColor();
        aLineColor.Invert();
    }
    rRenderContext.SetLineColor(aLineColor);
    long nL = GetLeft();
    long nR = GetRight();

    if (GetUsage() == SvxPageUsage::Mirror && !bSecond)
    {
        // rotate for mirrored
        nL = GetRight();
        nR = GetLeft();
    }

    tools::Rectangle aRect;
    aRect.SetRight( rOrg.X() + GetSize().Width() - nR );
    aRect.SetLeft( rOrg.X() + nL );
    aRect.SetTop( rOrg.Y() + GetTop() + GetHdHeight() + GetHdDist() );
    aRect.SetBottom( rOrg.Y() + GetSize().Height() - GetBottom() - GetFtHeight() - GetFtDist() );

    //increase the values to get a 'viewable' preview
    sal_Int32 nBaseHeight = pGridItem->GetBaseHeight() * 3;
    sal_Int32 nRubyHeight = pGridItem->GetRubyHeight() * 3;

    //detect height of rectangles
    tools::Rectangle aRubyRect(aRect.TopLeft(),
                m_bVertical ?
                Size(nRubyHeight, aRect.GetHeight()) :
                Size(aRect.GetWidth(), nRubyHeight));
    tools::Rectangle aCharRect(aRect.TopLeft(),
                m_bVertical ?
                Size(nBaseHeight, aRect.GetHeight()) :
                Size(aRect.GetWidth(), nBaseHeight));

    sal_Int32 nLineHeight = nBaseHeight + nRubyHeight;

    //detect count of rectangles
    sal_Int32 nLines = (m_bVertical ? aRect.GetWidth(): aRect.GetHeight()) / nLineHeight;
    if (nLines > pGridItem->GetLines())
        nLines = pGridItem->GetLines();

    // determine start position
    if (m_bVertical)
    {
        sal_Int16 nXStart = static_cast<sal_Int16>(aRect.GetWidth() / 2 - nLineHeight * nLines /2);
        aRubyRect.Move(nXStart, 0);
        aCharRect.Move(nXStart, 0);
    }
    else
    {
        sal_Int16 nYStart = static_cast<sal_Int16>(aRect.GetHeight() / 2 - nLineHeight * nLines /2);
        aRubyRect.Move(0, nYStart);
        aCharRect.Move(0, nYStart);
    }

    if (pGridItem->IsRubyTextBelow())
        m_bVertical ? aRubyRect.Move(nBaseHeight, 0) : aRubyRect.Move(0, nBaseHeight);
    else
        m_bVertical ? aCharRect.Move(nRubyHeight, 0) : aCharRect.Move(0, nRubyHeight);

    //vertical lines
    bool bBothLines = pGridItem->GetGridType() == GRID_LINES_CHARS;
    rRenderContext.SetFillColor(COL_TRANSPARENT);
    sal_Int32 nXMove = m_bVertical ? nLineHeight : 0;
    sal_Int32 nYMove = m_bVertical ? 0 : nLineHeight;
    for (sal_Int32 nLine = 0; nLine < nLines; nLine++)
    {
        rRenderContext.DrawRect(aRubyRect);
        rRenderContext.DrawRect(aCharRect);
        if (bBothLines)
        {
            Point aStart = aCharRect.TopLeft();
            Point aEnd = m_bVertical ? aCharRect.TopRight() : aCharRect.BottomLeft();
            while (m_bVertical ? aStart.Y() < aRect.Bottom(): aStart.X() < aRect.Right())
            {
                rRenderContext.DrawLine(aStart, aEnd);
                if(m_bVertical)
                    aStart.setY( aEnd.AdjustY(nBaseHeight ) );
                else
                    aStart.setX( aEnd.AdjustX(nBaseHeight ) );
            }
        }
        aRubyRect.Move(nXMove, nYMove);
        aCharRect.Move(nXMove, nYMove);
    }

}

void SwPageGridExample::UpdateExample( const SfxItemSet& rSet )
{
    pGridItem.reset();
    //get the grid information
    if(SfxItemState::DEFAULT <= rSet.GetItemState(RES_TEXTGRID))
        pGridItem.reset(static_cast<SwTextGridItem*>(rSet.Get(RES_TEXTGRID).Clone()));
    SwPageExample::UpdateExample(rSet);
}

VCL_BUILDER_FACTORY(SwPageGridExample)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
