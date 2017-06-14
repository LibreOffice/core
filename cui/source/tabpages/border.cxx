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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <cuires.hrc>
#include <bitmaps.hlst>
#include "helpid.hrc"

#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include "border.hxx"
#include <svx/dlgutil.hxx>
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include "svx/flagsdef.hxx"
#include <sfx2/request.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>
#include <svl/int64item.hxx>
#include <sfx2/itemconnect.hxx>
#include <sal/macros.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "borderconn.hxx"

using namespace ::editeng;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XServiceInfo;
using ::com::sun::star::uno::UNO_QUERY;


/*
 * [Description:]
 * TabPage for setting the border attributes.
 * Needs
 *      a SvxShadowItem: shadow
 *      a SvxBoxItem:    lines left, right, top, bottom,
 *      a SvxBoxInfo:    lines vertical, horizontal, distance, flags
 *
 * Lines can have three conditions:
 *      1. Show     ( -> valid values )
 *      2. Hide     ( -> NULL-Pointer )
 *      3. DontCare ( -> special Valid-Flags in the InfoItem )
 */

// static ----------------------------------------------------------------

const sal_uInt16 SvxBorderTabPage::pRanges[] =
{
    SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_SHADOW,
    SID_ATTR_ALIGN_MARGIN,      SID_ATTR_ALIGN_MARGIN,
    SID_ATTR_BORDER_CONNECT,    SID_ATTR_BORDER_CONNECT,
    SID_SW_COLLAPSING_BORDERS,  SID_SW_COLLAPSING_BORDERS,
    SID_ATTR_BORDER_DIAG_TLBR,  SID_ATTR_BORDER_DIAG_BLTR,
    0
};

static void lcl_SetDecimalDigitsTo1(MetricField& rField)
{
    sal_Int64 nMin = rField.Denormalize( rField.GetMin( FUNIT_TWIP ) );
    rField.SetDecimalDigits(1);
    rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
}

// number of preset images to show
const sal_uInt16 SVX_BORDER_PRESET_COUNT = 5;

// number of shadow images to show
const sal_uInt16 SVX_BORDER_SHADOW_COUNT = 5;

SvxBorderTabPage::SvxBorderTabPage(vcl::Window* pParent, const SfxItemSet& rCoreAttrs)
    : SfxTabPage(pParent, "BorderPage", "cui/ui/borderpage.ui", &rCoreAttrs)
    , nMinValue(0)
    , nSWMode(SwBorderModes::NONE)
    , mbHorEnabled(false)
    , mbVerEnabled(false)
    , mbTLBREnabled(false)
    , mbBLTREnabled(false)
    , mbUseMarginItem(false)
    , mbAllowPaddingWithoutBorders(true)
    , mbSync(true)
    , mbRemoveAdjacentCellBorders(false)
    , bIsCalcDoc(false)
{
    get(m_pWndPresets, "presets");
    get(m_pUserDefFT, "userdefft");
    get(m_pFrameSel, "framesel");
    get(m_pLbLineStyle, "linestylelb");
    get(m_pLbLineColor, "linecolorlb");
    get(m_pLineWidthMF, "linewidthmf");

    get(m_pSpacingFrame, "spacing");
    get(m_pLeftFT, "leftft");
    get(m_pLeftMF, "leftmf");
    get(m_pRightFT, "rightft");
    get(m_pRightMF, "rightmf");
    get(m_pTopFT, "topft");
    get(m_pTopMF, "topmf");
    get(m_pBottomFT, "bottomft");
    get(m_pBottomMF, "bottommf");
    get(m_pSynchronizeCB, "sync");

    get(m_pShadowFrame, "shadow");
    get(m_pWndShadows, "shadows");
    get(m_pFtShadowSize, "distanceft");
    get(m_pEdShadowSize, "distancemf");
    get(m_pFtShadowColor, "shadowcolorft");
    get(m_pLbShadowColor, "shadowcolorlb");

    get(m_pPropertiesFrame, "properties");
    get(m_pMergeWithNextCB, "mergewithnext");
    get(m_pMergeAdjacentBordersCB, "mergeadjacent");
    get(m_pRemoveAdjcentCellBordersCB, "rmadjcellborders");
    get(m_pRemoveAdjcentCellBordersFT, "rmadjcellbordersft");

    static const OUStringLiteral pnBorderImgIds[] =
    {
        RID_SVXBMP_CELL_NONE,
        RID_SVXBMP_CELL_ALL,
        RID_SVXBMP_CELL_LR,
        RID_SVXBMP_CELL_TB,
        RID_SVXBMP_CELL_L,
        RID_SVXBMP_CELL_DIAG,
        RID_SVXBMP_HOR_NONE,
        RID_SVXBMP_HOR_OUTER,
        RID_SVXBMP_HOR_HOR,
        RID_SVXBMP_HOR_ALL,
        RID_SVXBMP_HOR_OUTER2,
        RID_SVXBMP_VER_NONE,
        RID_SVXBMP_VER_OUTER,
        RID_SVXBMP_VER_VER,
        RID_SVXBMP_VER_ALL,
        RID_SVXBMP_VER_OUTER2,
        RID_SVXBMP_TABLE_NONE,
        RID_SVXBMP_TABLE_OUTER,
        RID_SVXBMP_TABLE_OUTERH,
        RID_SVXBMP_TABLE_ALL,
        RID_SVXBMP_TABLE_OUTER2
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(pnBorderImgIds); ++i)
        m_aBorderImgVec.push_back(BitmapEx(pnBorderImgIds[i]));

    static const OUStringLiteral pnShadowImgIds[SVX_BORDER_SHADOW_COUNT] =
    {
        RID_SVXBMP_SHADOWNONE,
        RID_SVXBMP_SHADOW_BOT_RIGHT,
        RID_SVXBMP_SHADOW_BOT_LEFT,
        RID_SVXBMP_SHADOW_TOP_RIGHT,
        RID_SVXBMP_SHADOW_TOP_LEFT
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(pnShadowImgIds); ++i)
        m_aShadowImgVec.push_back(BitmapEx(pnShadowImgIds[i]));
    assert(m_aShadowImgVec.size() == SVX_BORDER_SHADOW_COUNT);

    if ( GetDPIScaleFactor() > 1 )
    {
        for (size_t i = 0; i < m_aBorderImgVec.size(); ++i)
            m_aBorderImgVec[i].Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BmpScaleFlag::Fast);

        for (size_t i = 0; i < m_aShadowImgVec.size(); ++i)
            m_aShadowImgVec[i].Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BmpScaleFlag::Fast);
    }

    // this page needs ExchangeSupport
    SetExchangeSupport();

    /*  Use SvxMarginItem instead of margins from SvxBoxItem, if present.
        ->  Remember this state in mbUseMarginItem, because other special handling
            is needed across various functions... */
    mbUseMarginItem = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_MARGIN)) != SfxItemState::UNKNOWN;

    const SfxPoolItem* pItem = nullptr;
    if (rCoreAttrs.HasItem(SID_ATTR_BORDER_STYLES, &pItem))
    {
        const SfxIntegerListItem* p = static_cast<const SfxIntegerListItem*>(pItem);
        std::vector<sal_Int32> aUsedStyles = p->GetList();
        for (int aUsedStyle : aUsedStyles)
            maUsedBorderStyles.insert(static_cast<SvxBorderLineStyle>(aUsedStyle));
    }

    if (rCoreAttrs.HasItem(SID_ATTR_BORDER_DEFAULT_WIDTH, &pItem))
    {
        // The caller specifies default line width.  Honor it.
        const SfxInt64Item* p = static_cast<const SfxInt64Item*>(pItem);
        m_pLineWidthMF->SetValue(p->GetValue());
    }

    // set metric
    FieldUnit eFUnit = GetModuleFieldUnit( rCoreAttrs );

    if( mbUseMarginItem )
    {
        // copied from SvxAlignmentTabPage
        switch ( eFUnit )
        {
            //  #103396# the default value (1pt) can't be accurately represented in
            //  inches or pica with two decimals, so point is used instead.
            case FUNIT_PICA:
            case FUNIT_INCH:
            case FUNIT_FOOT:
            case FUNIT_MILE:
                eFUnit = FUNIT_POINT;
                break;

            case FUNIT_CM:
            case FUNIT_M:
            case FUNIT_KM:
                eFUnit = FUNIT_MM;
                break;
            default: ;//prevent warning
        }
    }
    else
    {
        switch ( eFUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eFUnit = FUNIT_MM;
                break;
            default: ; //prevent warning
        }
    }

    SetFieldUnit(*m_pEdShadowSize, eFUnit);

    sal_uInt16 nWhich = GetWhich( SID_ATTR_BORDER_INNER, false );
    bool bIsDontCare = true;

    if ( rCoreAttrs.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        // paragraph or table
        const SvxBoxInfoItem* pBoxInfo =
            static_cast<const SvxBoxInfoItem*>(&( rCoreAttrs.Get( nWhich ) ));

        mbHorEnabled = pBoxInfo->IsHorEnabled();
        mbVerEnabled = pBoxInfo->IsVerEnabled();
        mbTLBREnabled = sfx::ItemWrapperHelper::IsKnownItem( rCoreAttrs, SID_ATTR_BORDER_DIAG_TLBR );
        mbBLTREnabled = sfx::ItemWrapperHelper::IsKnownItem( rCoreAttrs, SID_ATTR_BORDER_DIAG_BLTR );

        if(pBoxInfo->IsDist())
        {
            SetFieldUnit(*m_pLeftMF, eFUnit);
            SetFieldUnit(*m_pRightMF, eFUnit);
            SetFieldUnit(*m_pTopMF, eFUnit);
            SetFieldUnit(*m_pBottomMF, eFUnit);
            m_pSynchronizeCB->SetClickHdl(LINK(this, SvxBorderTabPage, SyncHdl_Impl));
            m_pLeftMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_pRightMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_pTopMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            m_pBottomMF->SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
        }
        else
        {
            m_pSpacingFrame->Hide();
        }
        bIsDontCare = !pBoxInfo->IsValid( SvxBoxInfoItemValidFlags::DISABLE );
    }
    if(!mbUseMarginItem && eFUnit == FUNIT_MM && MapUnit::MapTwip == rCoreAttrs.GetPool()->GetMetric( GetWhich( SID_ATTR_BORDER_INNER ) ))
    {
        //#i91548# changing the number of decimal digits changes the minimum values, too
        lcl_SetDecimalDigitsTo1(*m_pLeftMF);
        lcl_SetDecimalDigitsTo1(*m_pRightMF);
        lcl_SetDecimalDigitsTo1(*m_pTopMF);
        lcl_SetDecimalDigitsTo1(*m_pBottomMF);
        lcl_SetDecimalDigitsTo1(*m_pEdShadowSize);
    }

    FrameSelFlags nFlags = FrameSelFlags::Outer;
    if( mbHorEnabled )
        nFlags |= FrameSelFlags::InnerHorizontal;
    if( mbVerEnabled )
        nFlags |= FrameSelFlags::InnerVertical;
    if( mbTLBREnabled )
        nFlags |= FrameSelFlags::DiagonalTLBR;
    if( mbBLTREnabled )
        nFlags |= FrameSelFlags::DiagonalBLTR;
    if( bIsDontCare )
        nFlags |= FrameSelFlags::DontCare;
    m_pFrameSel->Initialize( nFlags );

    m_pFrameSel->SetSelectHdl(LINK(this, SvxBorderTabPage, LinesChanged_Impl));
    m_pLbLineStyle->SetSelectHdl( LINK( this, SvxBorderTabPage, SelStyleHdl_Impl ) );
    m_pLbLineColor->SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    m_pLineWidthMF->SetModifyHdl( LINK( this, SvxBorderTabPage, ModifyWidthHdl_Impl ) );
    m_pWndPresets->SetSelectHdl( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    m_pWndShadows->SetSelectHdl( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );

    FillValueSets();
    FillLineListBox_Impl();

    // connections
    bool bSupportsShadow = !SfxItemPool::IsSlot( GetWhich( SID_ATTR_BORDER_SHADOW ) );
    if( bSupportsShadow )
        AddItemConnection( svx::CreateShadowConnection( rCoreAttrs, *m_pWndShadows, *m_pEdShadowSize, *m_pLbShadowColor ) );
    else
        HideShadowControls();

    if( mbUseMarginItem )
        AddItemConnection( svx::CreateMarginConnection( rCoreAttrs, *m_pLeftMF, *m_pRightMF, *m_pTopMF, *m_pBottomMF ) );
    if( m_pFrameSel->IsBorderEnabled( svx::FrameBorderType::TLBR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_TLBR, *m_pFrameSel, svx::FrameBorderType::TLBR ) );
    if( m_pFrameSel->IsBorderEnabled( svx::FrameBorderType::BLTR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_BLTR, *m_pFrameSel, svx::FrameBorderType::BLTR ) );
    // #i43593# - item connection doesn't work for Writer,
    // because the Writer item sets contain these items
    // checkbox "Merge with next paragraph" only visible for Writer dialog format.paragraph
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_BORDER_CONNECT, *m_pMergeWithNextCB, ItemConnFlags::NONE ) );
    m_pMergeWithNextCB->Hide();
    // checkbox "Merge adjacent line styles" only visible for Writer dialog format.table
    AddItemConnection( new sfx::CheckBoxConnection( SID_SW_COLLAPSING_BORDERS, *m_pMergeAdjacentBordersCB, ItemConnFlags::NONE ) );
    m_pMergeAdjacentBordersCB->Hide();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if (pDocSh)
    {
        Reference< XServiceInfo > xSI( pDocSh->GetModel(), UNO_QUERY );
        if ( xSI.is() )
            bIsCalcDoc = xSI->supportsService("com.sun.star.sheet.SpreadsheetDocument");
    }
    if( bIsCalcDoc )
    {
        m_pRemoveAdjcentCellBordersCB->SetClickHdl(LINK(this, SvxBorderTabPage, RemoveAdjacentCellBorderHdl_Impl));
        m_pRemoveAdjcentCellBordersCB->Show();
        m_pRemoveAdjcentCellBordersCB->Enable( false );
    }
    else
    {
        m_pRemoveAdjcentCellBordersCB->Hide();
        m_pRemoveAdjcentCellBordersFT->Hide();
    }
}

SvxBorderTabPage::~SvxBorderTabPage()
{
    disposeOnce();
}

void SvxBorderTabPage::dispose()
{
    m_pWndPresets.clear();
    m_pUserDefFT.clear();
    m_pFrameSel.clear();
    m_pLbLineStyle.clear();
    m_pLbLineColor.clear();
    m_pLineWidthMF.clear();
    m_pSpacingFrame.clear();
    m_pLeftFT.clear();
    m_pLeftMF.clear();
    m_pRightFT.clear();
    m_pRightMF.clear();
    m_pTopFT.clear();
    m_pTopMF.clear();
    m_pBottomFT.clear();
    m_pBottomMF.clear();
    m_pSynchronizeCB.clear();
    m_pShadowFrame.clear();
    m_pWndShadows.clear();
    m_pFtShadowSize.clear();
    m_pEdShadowSize.clear();
    m_pFtShadowColor.clear();
    m_pLbShadowColor.clear();
    m_pPropertiesFrame.clear();
    m_pMergeWithNextCB.clear();
    m_pMergeAdjacentBordersCB.clear();
    m_pRemoveAdjcentCellBordersCB.clear();
    m_pRemoveAdjcentCellBordersFT.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxBorderTabPage::Create( vcl::Window* pParent,
                                             const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxBorderTabPage>::Create( pParent, *rAttrSet );
}

void SvxBorderTabPage::ResetFrameLine_Impl( svx::FrameBorderType eBorder, const SvxBorderLine* pCoreLine, bool bValid )
{
    if( m_pFrameSel->IsBorderEnabled( eBorder ) )
    {
        if( bValid )
            m_pFrameSel->ShowBorder( eBorder, pCoreLine );
        else
            m_pFrameSel->SetBorderDontCare( eBorder );
    }
}

bool SvxBorderTabPage::IsBorderLineStyleAllowed( SvxBorderLineStyle nStyle ) const
{
    if (maUsedBorderStyles.empty())
        // All border styles are allowed.
        return true;

    return maUsedBorderStyles.count(nStyle) > 0;
}


void SvxBorderTabPage::Reset( const SfxItemSet* rSet )
{
    SfxTabPage::Reset( rSet );

    const SvxBoxItem*       pBoxItem;
    const SvxBoxInfoItem*   pBoxInfoItem;
    sal_uInt16              nWhichBox       = GetWhich(SID_ATTR_BORDER_OUTER);
    MapUnit                 eCoreUnit;

    pBoxItem  = static_cast<const SvxBoxItem*>(GetItem( *rSet, SID_ATTR_BORDER_OUTER ));

    pBoxInfoItem = static_cast<const SvxBoxInfoItem*>(GetItem( *rSet, SID_ATTR_BORDER_INNER, false ));

    eCoreUnit = rSet->GetPool()->GetMetric( nWhichBox );

    if ( pBoxItem && pBoxInfoItem ) // -> Don't Care
    {
        ResetFrameLine_Impl( svx::FrameBorderType::Left,   pBoxItem->GetLeft(),     pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::LEFT ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Right,  pBoxItem->GetRight(),    pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::RIGHT ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Top,    pBoxItem->GetTop(),      pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::TOP ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Bottom, pBoxItem->GetBottom(),   pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::BOTTOM ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Vertical,    pBoxInfoItem->GetVert(), pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::VERT ) );
        ResetFrameLine_Impl( svx::FrameBorderType::Horizontal,    pBoxInfoItem->GetHori(), pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::HORI ) );


        // distance inside

        if( !mbUseMarginItem )
        {
            if ( m_pLeftMF->IsVisible() )
            {
                SetMetricValue(*m_pLeftMF,    pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_pRightMF,   pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_pTopMF,     pBoxInfoItem->GetDefDist(), eCoreUnit);
                SetMetricValue(*m_pBottomMF,  pBoxInfoItem->GetDefDist(), eCoreUnit);

                nMinValue = static_cast<long>(m_pLeftMF->GetValue());

                if ( pBoxInfoItem->IsMinDist() )
                {
                    m_pLeftMF->SetFirst( nMinValue );
                    m_pRightMF->SetFirst( nMinValue );
                    m_pTopMF->SetFirst( nMinValue );
                    m_pBottomMF->SetFirst( nMinValue );
                }

                if ( pBoxInfoItem->IsDist() )
                {
                    if( rSet->GetItemState( nWhichBox ) >= SfxItemState::DEFAULT )
                    {
                        bool bIsAnyBorderVisible = m_pFrameSel->IsAnyBorderVisible();
                        if( !bIsAnyBorderVisible || !pBoxInfoItem->IsMinDist() )
                        {
                            m_pLeftMF->SetMin( 0 );
                            m_pLeftMF->SetFirst( 0 );
                            m_pRightMF->SetMin( 0 );
                            m_pRightMF->SetFirst( 0 );
                            m_pTopMF->SetMin( 0 );
                            m_pTopMF->SetFirst( 0 );
                            m_pBottomMF->SetMin( 0 );
                            m_pBottomMF->SetFirst( 0 );
                        }
                        long nLeftDist = pBoxItem->GetDistance( SvxBoxItemLine::LEFT);
                        SetMetricValue(*m_pLeftMF, nLeftDist, eCoreUnit);
                        long nRightDist = pBoxItem->GetDistance( SvxBoxItemLine::RIGHT);
                        SetMetricValue(*m_pRightMF, nRightDist, eCoreUnit);
                        long nTopDist = pBoxItem->GetDistance( SvxBoxItemLine::TOP);
                        SetMetricValue( *m_pTopMF, nTopDist, eCoreUnit );
                        long nBottomDist = pBoxItem->GetDistance( SvxBoxItemLine::BOTTOM);
                        SetMetricValue( *m_pBottomMF, nBottomDist, eCoreUnit );

                        // if the distance is set with no active border line
                        // or it is null with an active border line
                        // no automatic changes should be made
                        const long nDefDist = bIsAnyBorderVisible ? pBoxInfoItem->GetDefDist() : 0;
                        bool bDiffDist = (nDefDist != nLeftDist ||
                                    nDefDist != nRightDist ||
                                    nDefDist != nTopDist   ||
                                    nDefDist != nBottomDist);
                        if ((pBoxItem->GetSmallestDistance() ||
                                bIsAnyBorderVisible) && bDiffDist )
                        {
                            m_pLeftMF->SetModifyFlag();
                            m_pRightMF->SetModifyFlag();
                            m_pTopMF->SetModifyFlag();
                            m_pBottomMF->SetModifyFlag();
                        }
                    }
                    else
                    {
                        // #106224# different margins -> do not fill the edits
                        m_pLeftMF->SetText( OUString() );
                        m_pRightMF->SetText( OUString() );
                        m_pTopMF->SetText( OUString() );
                        m_pBottomMF->SetText( OUString() );
                    }
                }
                m_pLeftMF->SaveValue();
                m_pRightMF->SaveValue();
                m_pTopMF->SaveValue();
                m_pBottomMF->SaveValue();
            }
        }
    }
    else
    {
        // avoid ResetFrameLine-calls:
        m_pFrameSel->HideAllBorders();
    }

    if( !m_pFrameSel->IsAnyBorderVisible() )
        m_pFrameSel->DeselectAllBorders();

    // depict line (color) in controllers if unambiguous:

    {
        // Do all visible lines show the same line widths?
        long nWidth;
        SvxBorderLineStyle nStyle;
        bool bWidthEq = m_pFrameSel->GetVisibleWidth( nWidth, nStyle );
        if( bWidthEq )
        {
            // Determine the width first as some styles can be missing depending on it
            sal_Int64 nWidthPt =  static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                        sal_Int64( nWidth ), m_pLineWidthMF->GetDecimalDigits( ),
                        MapUnit::MapTwip, m_pLineWidthMF->GetUnit() ));
            m_pLineWidthMF->SetValue( nWidthPt );
            m_pLbLineStyle->SetWidth( nWidth );

            // then set the style
            m_pLbLineStyle->SelectEntry( nStyle );
        }
        else
            m_pLbLineStyle->SelectEntryPos( 1 );

        // Do all visible lines show the same line color?
        Color aColor;
        bool bColorEq = m_pFrameSel->GetVisibleColor( aColor );
        if( !bColorEq )
            aColor.SetColor( COL_BLACK );

        m_pLbLineColor->SelectEntry(aColor);
        m_pLbLineStyle->SetColor(aColor);

        // Select all visible lines, if they are all equal.
        if( bWidthEq && bColorEq )
            m_pFrameSel->SelectAllVisibleBorders();

        // set the current style and color (caches style in control even if nothing is selected)
        SelStyleHdl_Impl(*m_pLbLineStyle);
        SelColHdl_Impl(*m_pLbLineColor);
    }

    bool bEnable = m_pWndShadows->GetSelectItemId() > 1 ;
    m_pFtShadowSize->Enable(bEnable);
    m_pEdShadowSize->Enable(bEnable);
    m_pFtShadowColor->Enable(bEnable);
    m_pLbShadowColor->Enable(bEnable);

    m_pWndPresets->SetNoSelection();

    // - no line - should not be selected

    if ( m_pLbLineStyle->GetSelectEntryPos() == 0 )
    {
        m_pLbLineStyle->SelectEntryPos( 1 );
        SelStyleHdl_Impl(*m_pLbLineStyle);
    }

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SfxItemState::SET == rSet->GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != (pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            // there are no shadows in Html-mode and only complete borders
            m_pShadowFrame->Disable();

            if( !(nSWMode & SwBorderModes::TABLE) )
            {
                m_pUserDefFT->Disable();
                m_pFrameSel->Disable();
                m_pWndPresets->RemoveItem(3);
                m_pWndPresets->RemoveItem(4);
                m_pWndPresets->RemoveItem(5);
            }
        }
    }

    LinesChanged_Impl( nullptr );
    if(m_pLeftMF->GetValue() == m_pRightMF->GetValue() && m_pTopMF->GetValue() == m_pBottomMF->GetValue() && m_pTopMF->GetValue() == m_pLeftMF->GetValue())
        mbSync = true;
    else
        mbSync = false;
    m_pSynchronizeCB->Check(mbSync);

    mbRemoveAdjacentCellBorders = false;
    m_pRemoveAdjcentCellBordersCB->Check( false );
    m_pRemoveAdjcentCellBordersCB->Enable( false );
}

void SvxBorderTabPage::ChangesApplied()
{
    m_pLeftMF->SaveValue();
    m_pRightMF->SaveValue();
    m_pTopMF->SaveValue();
    m_pBottomMF->SaveValue();
}


DeactivateRC SvxBorderTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxBorderTabPage::FillItemSet( SfxItemSet* rCoreAttrs )
{
    bool bAttrsChanged = SfxTabPage::FillItemSet( rCoreAttrs );

    bool                  bPut          = true;
    sal_uInt16            nBoxWhich     = GetWhich( SID_ATTR_BORDER_OUTER );
    sal_uInt16            nBoxInfoWhich = rCoreAttrs->GetPool()->GetWhich( SID_ATTR_BORDER_INNER, false );
    const SfxItemSet&     rOldSet       = GetItemSet();
    SvxBoxItem            aBoxItem      ( nBoxWhich );
    SvxBoxInfoItem        aBoxInfoItem  ( nBoxInfoWhich );
    const SvxBoxItem*     pOldBoxItem = static_cast<const SvxBoxItem*>(GetOldItem( *rCoreAttrs, SID_ATTR_BORDER_OUTER ));

    MapUnit eCoreUnit = rOldSet.GetPool()->GetMetric( nBoxWhich );


    // outer border:

    std::pair<svx::FrameBorderType,SvxBoxItemLine> eTypes1[] = {
                                { svx::FrameBorderType::Top,SvxBoxItemLine::TOP },
                                { svx::FrameBorderType::Bottom,SvxBoxItemLine::BOTTOM },
                                { svx::FrameBorderType::Left,SvxBoxItemLine::LEFT },
                                { svx::FrameBorderType::Right,SvxBoxItemLine::RIGHT },
                            };

    for (std::pair<svx::FrameBorderType,SvxBoxItemLine> const & i : eTypes1)
        aBoxItem.SetLine( m_pFrameSel->GetFrameBorderStyle( i.first ), i.second );


    aBoxItem.SetRemoveAdjacentCellBorder( mbRemoveAdjacentCellBorders );
    // border hor/ver and TableFlag

    std::pair<svx::FrameBorderType,SvxBoxInfoItemLine> eTypes2[] = {
                                { svx::FrameBorderType::Horizontal,SvxBoxInfoItemLine::HORI },
                                { svx::FrameBorderType::Vertical,SvxBoxInfoItemLine::VERT }
                            };
    for (std::pair<svx::FrameBorderType,SvxBoxInfoItemLine> const & j : eTypes2)
        aBoxInfoItem.SetLine( m_pFrameSel->GetFrameBorderStyle( j.first ), j.second );

    aBoxInfoItem.EnableHor( mbHorEnabled );
    aBoxInfoItem.EnableVer( mbVerEnabled );


    // inner distance

    if( m_pLeftMF->IsVisible() )
    {
        // #i40405# enable distance controls for next dialog call
        aBoxInfoItem.SetDist( true );

        if( !mbUseMarginItem )
        {
            // #106224# all edits empty: do nothing
            if( !m_pLeftMF->GetText().isEmpty() || !m_pRightMF->GetText().isEmpty() ||
                !m_pTopMF->GetText().isEmpty() || !m_pBottomMF->GetText().isEmpty() )
            {
                if ( mbAllowPaddingWithoutBorders
                     || ((mbHorEnabled || mbVerEnabled || (nSWMode & SwBorderModes::TABLE)) &&
                         (m_pLeftMF->IsModified()||m_pRightMF->IsModified()||
                             m_pTopMF->IsModified()||m_pBottomMF->IsModified()) )
                     || m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Top ) != svx::FrameBorderState::Hide
                     || m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Bottom ) != svx::FrameBorderState::Hide
                     || m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Left ) != svx::FrameBorderState::Hide
                     || m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Right ) != svx::FrameBorderState::Hide )
                {
                    const SvxBoxInfoItem* pOldBoxInfoItem = static_cast<const SvxBoxInfoItem*>(GetOldItem(
                                                        *rCoreAttrs, SID_ATTR_BORDER_INNER ));
                    if (
                        !pOldBoxItem ||
                        m_pLeftMF->IsValueChangedFromSaved() ||
                        m_pRightMF->IsValueChangedFromSaved() ||
                        m_pTopMF->IsValueChangedFromSaved() ||
                        m_pBottomMF->IsValueChangedFromSaved() ||
                        nMinValue == m_pLeftMF->GetValue() ||
                        nMinValue == m_pRightMF->GetValue() ||
                        nMinValue == m_pTopMF->GetValue() ||
                        nMinValue == m_pBottomMF->GetValue() ||
                        (pOldBoxInfoItem && !pOldBoxInfoItem->IsValid(SvxBoxInfoItemValidFlags::DISTANCE))
                       )
                    {
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pLeftMF, eCoreUnit ), SvxBoxItemLine::LEFT  );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pRightMF, eCoreUnit ), SvxBoxItemLine::RIGHT );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pTopMF, eCoreUnit ), SvxBoxItemLine::TOP   );
                        aBoxItem.SetDistance( (sal_uInt16)GetCoreValue(*m_pBottomMF, eCoreUnit ), SvxBoxItemLine::BOTTOM);
                    }
                    else
                    {
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::LEFT ), SvxBoxItemLine::LEFT);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::RIGHT),  SvxBoxItemLine::RIGHT);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::TOP  ), SvxBoxItemLine::TOP);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(SvxBoxItemLine::BOTTOM), SvxBoxItemLine::BOTTOM);
                    }
                    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::DISTANCE );
                }
                else
                    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::DISTANCE, false );
            }
        }
    }


    // note Don't Care Status in the Info-Item:

    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::TOP,    m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Top )    != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::BOTTOM, m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Bottom ) != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::LEFT,   m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Left )   != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::RIGHT,  m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Right )  != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::HORI,   m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Horizontal )    != svx::FrameBorderState::DontCare );
    aBoxInfoItem.SetValid( SvxBoxInfoItemValidFlags::VERT,   m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Vertical )    != svx::FrameBorderState::DontCare );


    // Put or Clear of the border?

    bPut = true;

    if (   SfxItemState::DEFAULT == rOldSet.GetItemState( nBoxWhich,     false ))
    {
        bPut = aBoxItem != static_cast<const SvxBoxItem&>(rOldSet.Get(nBoxWhich));
    }
    if(  SfxItemState::DEFAULT == rOldSet.GetItemState( nBoxInfoWhich, false ) )
    {
        const SvxBoxInfoItem& rOldBoxInfo = static_cast<const SvxBoxInfoItem&>(
                                rOldSet.Get(nBoxInfoWhich));

        aBoxInfoItem.SetMinDist( rOldBoxInfo.IsMinDist() );
        aBoxInfoItem.SetDefDist( rOldBoxInfo.GetDefDist() );
        bPut |= (aBoxInfoItem != rOldBoxInfo );
    }

    if ( bPut )
    {
        if ( !pOldBoxItem || !( *pOldBoxItem == aBoxItem ) )
        {
            rCoreAttrs->Put( aBoxItem );
            bAttrsChanged = true;
        }
        const SfxPoolItem* pOld = GetOldItem( *rCoreAttrs, SID_ATTR_BORDER_INNER, false );

        if ( !pOld || !( *static_cast<const SvxBoxInfoItem*>(pOld) == aBoxInfoItem ) )
        {
            rCoreAttrs->Put( aBoxInfoItem );
            bAttrsChanged = true;
        }
    }
    else
    {
        rCoreAttrs->ClearItem( nBoxWhich );
        rCoreAttrs->ClearItem( nBoxInfoWhich );
    }

    return bAttrsChanged;
}

void SvxBorderTabPage::HideShadowControls()
{
    m_pShadowFrame->Hide();
}

#define IID_PRE_CELL_NONE       1
#define IID_PRE_CELL_ALL        2
#define IID_PRE_CELL_LR         3
#define IID_PRE_CELL_TB         4
#define IID_PRE_CELL_L          5
#define IID_PRE_CELL_DIAG       6
#define IID_PRE_HOR_NONE        7
#define IID_PRE_HOR_OUTER       8
#define IID_PRE_HOR_HOR         9
#define IID_PRE_HOR_ALL         10
#define IID_PRE_HOR_OUTER2      11
#define IID_PRE_VER_NONE        12
#define IID_PRE_VER_OUTER       13
#define IID_PRE_VER_VER         14
#define IID_PRE_VER_ALL         15
#define IID_PRE_VER_OUTER2      16
#define IID_PRE_TABLE_NONE      17
#define IID_PRE_TABLE_OUTER     18
#define IID_PRE_TABLE_OUTERH    19
#define IID_PRE_TABLE_ALL       20
#define IID_PRE_TABLE_OUTER2    21

IMPL_LINK_NOARG(SvxBorderTabPage, SelPreHdl_Impl, ValueSet*, void)
{
    const svx::FrameBorderState SHOW = svx::FrameBorderState::Show;
    const svx::FrameBorderState HIDE = svx::FrameBorderState::Hide;
    const svx::FrameBorderState DONT = svx::FrameBorderState::DontCare;

    static const svx::FrameBorderState ppeStates[][ svx::FRAMEBORDERTYPE_COUNT ] =
    {                   /*    Left  Right Top   Bot   Hor   Ver   TLBR  BLTR */
/* ---------------------+--------------------------------------------------- */
/* IID_PRE_CELL_NONE    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_ALL     */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_LR      */  { SHOW, SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_TB      */  { HIDE, HIDE, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_L       */  { SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_DIAG    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, SHOW, SHOW },
/* IID_PRE_HOR_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_HOR      */  { HIDE, HIDE, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_ALL      */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, DONT, HIDE, HIDE, HIDE },
/* IID_PRE_VER_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_VER      */  { SHOW, SHOW, HIDE, HIDE, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_ALL      */  { SHOW, SHOW, SHOW, SHOW, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, HIDE, DONT, HIDE, HIDE },
/* IID_PRE_TABLE_NONE   */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER  */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTERH */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_ALL    */  { SHOW, SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER2 */  { SHOW, SHOW, SHOW, SHOW, DONT, DONT, HIDE, HIDE }
    };

    // first hide and deselect all frame borders
    m_pFrameSel->HideAllBorders();
    m_pFrameSel->DeselectAllBorders();

    // Using image ID to find correct line in table above.
    sal_uInt16 nLine = GetPresetImageId( m_pWndPresets->GetSelectItemId() ) - 1;

    // Apply all styles from the table
    for( int nBorder = 0; nBorder < svx::FRAMEBORDERTYPE_COUNT; ++nBorder )
    {
        svx::FrameBorderType eBorder = svx::GetFrameBorderTypeFromIndex( nBorder );
        switch( ppeStates[ nLine ][ nBorder ] )
        {
            case SHOW:  m_pFrameSel->SelectBorder( eBorder );      break;
            case HIDE:  /* nothing to do */                     break;
            case DONT:  m_pFrameSel->SetBorderDontCare( eBorder ); break;
        }
    }

    // Show all lines that have been selected above
    if( m_pFrameSel->IsAnyBorderSelected() )
    {
        // any visible style, but "no-line" in line list box? -> use hair-line
        if( (m_pLbLineStyle->GetSelectEntryPos() == 0) || (m_pLbLineStyle->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND) )
            m_pLbLineStyle->SelectEntryPos( 1 );

        // set current style to all previously selected lines
        SelStyleHdl_Impl(*m_pLbLineStyle);
        SelColHdl_Impl(*m_pLbLineColor);
    }

    // Presets ValueSet does not show a selection (used as push buttons).
    m_pWndPresets->SetNoSelection();

    LinesChanged_Impl( nullptr );
    UpdateRemoveAdjCellBorderCB( nLine + 1 );
}


IMPL_LINK_NOARG(SvxBorderTabPage, SelSdwHdl_Impl, ValueSet*, void)
{
    bool bEnable = m_pWndShadows->GetSelectItemId() > 1;
    m_pFtShadowSize->Enable(bEnable);
    m_pEdShadowSize->Enable(bEnable);
    m_pFtShadowColor->Enable(bEnable);
    m_pLbShadowColor->Enable(bEnable);
}

IMPL_LINK(SvxBorderTabPage, SelColHdl_Impl, SvxColorListBox&, rColorBox, void)
{
    Color aColor = rColorBox.GetSelectEntryColor();
    m_pFrameSel->SetColorToSelection(aColor);
    m_pLbLineStyle->SetColor(aColor);
}

IMPL_LINK_NOARG(SvxBorderTabPage, ModifyWidthHdl_Impl, Edit&, void)
{
    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                m_pLineWidthMF->GetValue( ),
                m_pLineWidthMF->GetDecimalDigits( ),
                m_pLineWidthMF->GetUnit(), MapUnit::MapTwip ));
    m_pLbLineStyle->SetWidth( nVal );

    m_pFrameSel->SetStyleToSelection( nVal,
        m_pLbLineStyle->GetSelectEntryStyle() );
}


IMPL_LINK( SvxBorderTabPage, SelStyleHdl_Impl, ListBox&, rLb, void )
{
    if (&rLb == m_pLbLineStyle)
    {
        sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                    m_pLineWidthMF->GetValue( ),
                    m_pLineWidthMF->GetDecimalDigits( ),
                    m_pLineWidthMF->GetUnit(), MapUnit::MapTwip ));
        m_pFrameSel->SetStyleToSelection ( nVal,
            m_pLbLineStyle->GetSelectEntryStyle() );
    }
}


// ValueSet handling
sal_uInt16 SvxBorderTabPage::GetPresetImageId( sal_uInt16 nValueSetIdx ) const
{
    // table with all sets of predefined border styles
    static const sal_uInt16 ppnImgIds[][ SVX_BORDER_PRESET_COUNT ] =
    {
        // simple cell without diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_L          },
        // simple cell with diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_DIAG       },
        // with horizontal inner frame border
        {   IID_PRE_HOR_NONE,   IID_PRE_HOR_OUTER,      IID_PRE_HOR_HOR,        IID_PRE_HOR_ALL,    IID_PRE_HOR_OUTER2      },
        // with vertical inner frame border
        {   IID_PRE_VER_NONE,   IID_PRE_VER_OUTER,      IID_PRE_VER_VER,        IID_PRE_VER_ALL,    IID_PRE_VER_OUTER2      },
        // with horizontal and vertical inner frame borders
        {   IID_PRE_TABLE_NONE, IID_PRE_TABLE_OUTER,    IID_PRE_TABLE_OUTERH,   IID_PRE_TABLE_ALL,  IID_PRE_TABLE_OUTER2    }
    };

    // find correct set of presets
    int nLine = 0;
    if( !mbHorEnabled && !mbVerEnabled )
        nLine = (mbTLBREnabled || mbBLTREnabled) ? 1 : 0;
    else if( mbHorEnabled && !mbVerEnabled )
        nLine = 2;
    else if( !mbHorEnabled && mbVerEnabled )
        nLine = 3;
    else
        nLine = 4;

    DBG_ASSERT( (1 <= nValueSetIdx) && (nValueSetIdx <= SVX_BORDER_PRESET_COUNT),
        "SvxBorderTabPage::GetPresetImageId - wrong index" );
    return ppnImgIds[ nLine ][ nValueSetIdx - 1 ];
}

sal_uInt16 SvxBorderTabPage::GetPresetStringId( sal_uInt16 nValueSetIdx ) const
{
    // string resource IDs for each image (in order of the IID_PRE_* image IDs)
    static const sal_uInt16 pnStrIds[] =
    {
        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_PARA_PRESET_ALL,
        RID_SVXSTR_PARA_PRESET_LEFTRIGHT,
        RID_SVXSTR_PARA_PRESET_TOPBOTTOM,
        RID_SVXSTR_PARA_PRESET_ONLYLEFT,
        RID_SVXSTR_PARA_PRESET_DIAGONAL,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_HOR_PRESET_ONLYHOR,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_VER_PRESET_ONLYVER,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_TABLE_PRESET_OUTERHORI,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER
    };
    return pnStrIds[ GetPresetImageId( nValueSetIdx ) - 1 ];
}

void SvxBorderTabPage::FillPresetVS()
{
    // basic initialization of the ValueSet
    m_pWndPresets->SetStyle( m_pWndPresets->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    m_pWndPresets->SetColCount( SVX_BORDER_PRESET_COUNT );

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_PRESET_COUNT; ++nVSIdx )
    {
        m_pWndPresets->InsertItem( nVSIdx );
        m_pWndPresets->SetItemImage(nVSIdx, Image(m_aBorderImgVec[nVSIdx-1]));
        m_pWndPresets->SetItemText( nVSIdx, CuiResId( GetPresetStringId( nVSIdx ) ) );
    }

    // show the control
    m_pWndPresets->SetNoSelection();
    m_pWndPresets->Show();
}

void SvxBorderTabPage::FillShadowVS()
{
    // basic initialization of the ValueSet
    m_pWndShadows->SetStyle( m_pWndShadows->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    m_pWndShadows->SetColCount( SVX_BORDER_SHADOW_COUNT );

    // string resource IDs for each image
    static const sal_uInt16 pnStrIds[ SVX_BORDER_SHADOW_COUNT ] =
        { RID_SVXSTR_SHADOW_STYLE_NONE, RID_SVXSTR_SHADOW_STYLE_BOTTOMRIGHT, RID_SVXSTR_SHADOW_STYLE_TOPRIGHT, RID_SVXSTR_SHADOW_STYLE_BOTTOMLEFT, RID_SVXSTR_SHADOW_STYLE_TOPLEFT };

    // insert images and help texts
    for( sal_uInt16 nVSIdx = 1; nVSIdx <= SVX_BORDER_SHADOW_COUNT; ++nVSIdx )
    {
        m_pWndShadows->InsertItem( nVSIdx );
        m_pWndShadows->SetItemImage(nVSIdx, Image(m_aShadowImgVec[nVSIdx-1]));
        m_pWndShadows->SetItemText( nVSIdx, CuiResId( pnStrIds[ nVSIdx - 1 ] ) );
    }

    // show the control
    m_pWndShadows->SelectItem( 1 );
    m_pWndShadows->Show();
}


void SvxBorderTabPage::FillValueSets()
{
    FillPresetVS();
    FillShadowVS();
}


static Color lcl_mediumColor( Color aMain, Color /*aDefault*/ )
{
    return SvxBorderLine::threeDMediumColor( aMain );
}

void SvxBorderTabPage::FillLineListBox_Impl()
{
    using namespace ::com::sun::star::table::BorderLineStyle;

    struct {
        SvxBorderLineStyle mnStyle;
        long mnMinWidth;
        LineListBox::ColorFunc mpColor1Fn;
        LineListBox::ColorFunc mpColor2Fn;
        LineListBox::ColorDistFunc mpColorDistFn;
    } aLines[] = {
        // Simple lines
        { SvxBorderLineStyle::SOLID,        0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DOTTED,       0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DASHED,       0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::FINE_DASHED,  0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DASH_DOT,     0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DASH_DOT_DOT, 0, &sameColor, &sameColor, &sameDistColor },

        // Double lines
        { SvxBorderLineStyle::DOUBLE,              10, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::DOUBLE_THIN,         10, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THINTHICK_SMALLGAP,  20, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THINTHICK_MEDIUMGAP,  0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THINTHICK_LARGEGAP,   0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THICKTHIN_SMALLGAP,  20, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THICKTHIN_MEDIUMGAP,  0, &sameColor, &sameColor, &sameDistColor },
        { SvxBorderLineStyle::THICKTHIN_LARGEGAP,   0, &sameColor, &sameColor, &sameDistColor },

        { SvxBorderLineStyle::EMBOSSED, 15, &SvxBorderLine::threeDLightColor, &SvxBorderLine::threeDDarkColor, &lcl_mediumColor },
        { SvxBorderLineStyle::ENGRAVED, 15, &SvxBorderLine::threeDDarkColor, &SvxBorderLine::threeDLightColor, &lcl_mediumColor },

        { SvxBorderLineStyle::OUTSET, 10, &SvxBorderLine::lightColor, &SvxBorderLine::darkColor, &sameDistColor },
        { SvxBorderLineStyle::INSET,  10, &SvxBorderLine::darkColor, &SvxBorderLine::lightColor, &sameDistColor }
    };

    m_pLbLineStyle->SetSourceUnit( FUNIT_TWIP );

    m_pLbLineStyle->SetNone( SvxResId( RID_SVXSTR_NONE ) );

    for (size_t i = 0; i < SAL_N_ELEMENTS(aLines); ++i)
    {
        if (!IsBorderLineStyleAllowed(aLines[i].mnStyle))
            continue;

        m_pLbLineStyle->InsertEntry(
            SvxBorderLine::getWidthImpl(aLines[i].mnStyle), aLines[i].mnStyle,
            aLines[i].mnMinWidth, aLines[i].mpColor1Fn, aLines[i].mpColor2Fn, aLines[i].mpColorDistFn);
    }

    sal_Int64 nVal = static_cast<sal_Int64>(MetricField::ConvertDoubleValue(
                m_pLineWidthMF->GetValue( ),
                m_pLineWidthMF->GetDecimalDigits( ),
                m_pLineWidthMF->GetUnit(), MapUnit::MapTwip ));
    m_pLbLineStyle->SetWidth( nVal );
}


IMPL_LINK_NOARG(SvxBorderTabPage, LinesChanged_Impl, LinkParamNone*, void)
{
    if(!mbUseMarginItem && m_pLeftMF->IsVisible())
    {
        bool bLineSet = m_pFrameSel->IsAnyBorderVisible();
        bool bMinAllowed = bool(nSWMode & (SwBorderModes::FRAME|SwBorderModes::TABLE));
        bool bSpaceModified =   m_pLeftMF->IsModified()||
                                m_pRightMF->IsModified()||
                                m_pTopMF->IsModified()||
                                m_pBottomMF->IsModified();

        if(bLineSet)
        {
            if(!bMinAllowed)
            {
                m_pLeftMF->SetFirst(nMinValue);
                m_pRightMF->SetFirst(nMinValue);
                m_pTopMF->SetFirst(nMinValue);
                m_pBottomMF->SetFirst(nMinValue);
            }
            if(!bSpaceModified)
            {
                m_pLeftMF->SetValue(nMinValue);
                m_pRightMF->SetValue(nMinValue);
                m_pTopMF->SetValue(nMinValue);
                m_pBottomMF->SetValue(nMinValue);
            }
        }
        else
        {
            m_pLeftMF->SetMin(0);
            m_pRightMF->SetMin(0);
            m_pTopMF->SetMin(0);
            m_pBottomMF->SetMin(0);
            m_pLeftMF->SetFirst(0);
            m_pRightMF->SetFirst(0);
            m_pTopMF->SetFirst(0);
            m_pBottomMF->SetFirst(0);
            if(!bSpaceModified && !mbAllowPaddingWithoutBorders)
            {
                m_pLeftMF->SetValue(0);
                m_pRightMF->SetValue(0);
                m_pTopMF->SetValue(0);
                m_pBottomMF->SetValue(0);
            }
        }
        // for tables everything is allowed
        SvxBoxInfoItemValidFlags nValid = SvxBoxInfoItemValidFlags::TOP|SvxBoxInfoItemValidFlags::BOTTOM|SvxBoxInfoItemValidFlags::LEFT|SvxBoxInfoItemValidFlags::RIGHT;

        // for other objects (paragraph, page, frame, character) the edit is disabled, if there's no border set
        if(!(nSWMode & SwBorderModes::TABLE) && !mbAllowPaddingWithoutBorders)
        {
            if(bLineSet)
            {
                nValid  = (m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Top)    == svx::FrameBorderState::Show) ? SvxBoxInfoItemValidFlags::TOP : SvxBoxInfoItemValidFlags::NONE;
                nValid |= (m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Bottom) == svx::FrameBorderState::Show) ? SvxBoxInfoItemValidFlags::BOTTOM : SvxBoxInfoItemValidFlags::NONE;
                nValid |= (m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Left)   == svx::FrameBorderState::Show) ? SvxBoxInfoItemValidFlags::LEFT : SvxBoxInfoItemValidFlags::NONE;
                nValid |= (m_pFrameSel->GetFrameBorderState( svx::FrameBorderType::Right ) == svx::FrameBorderState::Show) ? SvxBoxInfoItemValidFlags::RIGHT : SvxBoxInfoItemValidFlags::NONE;
            }
            else
                nValid = SvxBoxInfoItemValidFlags::NONE;
        }
        m_pLeftFT->Enable( bool(nValid & SvxBoxInfoItemValidFlags::LEFT) );
        m_pRightFT->Enable( bool(nValid & SvxBoxInfoItemValidFlags::RIGHT) );
        m_pTopFT->Enable( bool(nValid & SvxBoxInfoItemValidFlags::TOP) );
        m_pBottomFT->Enable( bool(nValid & SvxBoxInfoItemValidFlags::BOTTOM) );
        m_pLeftMF->Enable( bool(nValid & SvxBoxInfoItemValidFlags::LEFT) );
        m_pRightMF->Enable( bool(nValid & SvxBoxInfoItemValidFlags::RIGHT) );
        m_pTopMF->Enable( bool(nValid & SvxBoxInfoItemValidFlags::TOP) );
        m_pBottomMF->Enable( bool(nValid & SvxBoxInfoItemValidFlags::BOTTOM) );
        m_pSynchronizeCB->Enable( m_pRightMF->IsEnabled() || m_pTopMF->IsEnabled() ||
                               m_pBottomMF->IsEnabled() || m_pLeftMF->IsEnabled() );
    }
    UpdateRemoveAdjCellBorderCB( SAL_MAX_UINT16 );
}


IMPL_LINK( SvxBorderTabPage, ModifyDistanceHdl_Impl, Edit&, rField, void)
{
    if ( mbSync )
    {
        sal_Int64 nVal = static_cast<MetricField&>(rField).GetValue();
        if(&rField != m_pLeftMF)
            m_pLeftMF->SetValue(nVal);
        if(&rField != m_pRightMF)
            m_pRightMF->SetValue(nVal);
        if(&rField != m_pTopMF)
            m_pTopMF->SetValue(nVal);
        if(&rField != m_pBottomMF)
            m_pBottomMF->SetValue(nVal);
    }
}

IMPL_LINK( SvxBorderTabPage, SyncHdl_Impl, Button*, pBox, void)
{
    mbSync = static_cast<CheckBox*>(pBox)->IsChecked();
}

IMPL_LINK( SvxBorderTabPage, RemoveAdjacentCellBorderHdl_Impl, Button*, pBox, void)
{
    mbRemoveAdjacentCellBorders = static_cast<CheckBox*>(pBox)->IsChecked();
}

void SvxBorderTabPage::UpdateRemoveAdjCellBorderCB( sal_uInt16 nPreset )
{
    if( !bIsCalcDoc )
        return;
    const SfxItemSet&     rOldSet         = GetItemSet();
    const SvxBoxInfoItem* pOldBoxInfoItem = static_cast<const SvxBoxInfoItem*>(GetOldItem( rOldSet, SID_ATTR_BORDER_INNER ));
    const SvxBoxItem*     pOldBoxItem     = static_cast<const SvxBoxItem*>(GetOldItem( rOldSet, SID_ATTR_BORDER_OUTER ));
    if( !pOldBoxInfoItem || !pOldBoxItem )
        return;
    std::pair<svx::FrameBorderType, SvxBoxInfoItemValidFlags> eTypes1[] = {
        { svx::FrameBorderType::Top,SvxBoxInfoItemValidFlags::TOP },
        { svx::FrameBorderType::Bottom,SvxBoxInfoItemValidFlags::BOTTOM },
        { svx::FrameBorderType::Left,SvxBoxInfoItemValidFlags::LEFT },
        { svx::FrameBorderType::Right,SvxBoxInfoItemValidFlags::RIGHT },
    };
    SvxBoxItemLine eTypes2[] = {
        SvxBoxItemLine::TOP,
        SvxBoxItemLine::BOTTOM,
        SvxBoxItemLine::LEFT,
        SvxBoxItemLine::RIGHT,
    };

    // Check if current selection involves deletion of at least one border
    bool bBorderDeletionReq = false;
    for ( sal_uInt32 i=0; i < SAL_N_ELEMENTS( eTypes1 ); ++i )
    {
        if( pOldBoxItem->GetLine( eTypes2[i] ) || !( pOldBoxInfoItem->IsValid( eTypes1[i].second ) ) )
        {
            if( m_pFrameSel->GetFrameBorderState( eTypes1[i].first ) == svx::FrameBorderState::Hide )
            {
                bBorderDeletionReq = true;
                break;
            }
        }
    }

    if( !bBorderDeletionReq && ( nPreset == IID_PRE_CELL_NONE || nPreset == IID_PRE_TABLE_NONE ) )
        bBorderDeletionReq = true;

    m_pRemoveAdjcentCellBordersCB->Enable( bBorderDeletionReq );

    if( !bBorderDeletionReq )
    {
        mbRemoveAdjacentCellBorders = false;
        m_pRemoveAdjcentCellBordersCB->Check( false );
    }
}

void SvxBorderTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
        FillValueSets();

    SfxTabPage::DataChanged( rDCEvt );
}

void SvxBorderTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pSWModeItem = aSet.GetItem<SfxUInt16Item>(SID_SWMODE_TYPE, false);
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pSWModeItem)
    {
        nSWMode = static_cast<SwBorderModes>(pSWModeItem->GetValue());
        // #i43593#
        // show checkbox <m_pMergeWithNextCB> for format.paragraph
        if ( nSWMode == SwBorderModes::PARA )
        {
            m_pMergeWithNextCB->Show();
            m_pPropertiesFrame->Show();
        }
        // show checkbox <m_pMergeAdjacentBordersCB> for format.paragraph
        else if ( nSWMode == SwBorderModes::TABLE )
        {
            m_pMergeAdjacentBordersCB->Show();
            m_pPropertiesFrame->Show();
        }
    }
    if (pFlagItem)
        if ( ( pFlagItem->GetValue() & SVX_HIDESHADOWCTL ) == SVX_HIDESHADOWCTL )
            HideShadowControls();
}

void SvxBorderTabPage::SetTableMode()
{
    nSWMode = SwBorderModes::TABLE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
