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

#include <memory>
#include <sal/config.h>

#include "PageMarginControl.hxx"
#include <strings.hrc>

#include <editeng/sizeitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/pageitem.hxx>
#include <svx/rulritem.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>
#include <svtools/unitconv.hxx>
#include <unotools/viewoptions.hxx>

#include <swtypes.hxx>
#include <cmdid.h>
#include <PageMarginPopup.hxx>

#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/frame/XFrame.hpp>

constexpr OUString SWPAGE_LEFT_GVALUE = u"Sw_Page_Left"_ustr;
constexpr OUString SWPAGE_RIGHT_GVALUE = u"Sw_Page_Right"_ustr;
constexpr OUString SWPAGE_TOP_GVALUE = u"Sw_Page_Top"_ustr;
constexpr OUString SWPAGE_DOWN_GVALUE = u"Sw_Page_Down"_ustr;
constexpr OUString SWPAGE_MIRROR_GVALUE = u"Sw_Page_Mirrored"_ustr;

namespace
{
    FieldUnit lcl_GetFieldUnit()
    {
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        {
            SfxPoolItemHolder aResult;
            const SfxItemState eState(pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_METRIC, aResult));
            const SfxUInt16Item* pItem(static_cast<const SfxUInt16Item*>(aResult.getItem()));
            if (pItem && eState >= SfxItemState::DEFAULT)
                return static_cast<FieldUnit>(pItem->GetValue());
        }
        return SfxModule::GetCurrentFieldUnit();
    }

    MapUnit lcl_GetUnit()
    {
        SfxItemPool &rPool = SfxGetpApp()->GetPool();
        sal_uInt16 nWhich = rPool.GetWhichIDFromSlotID( SID_ATTR_PAGE_SIZE );
        return rPool.GetMetric( nWhich );
    }

    css::uno::Reference< css::document::XUndoManager > getUndoManager( const css::uno::Reference< css::frame::XFrame >& rxFrame )
    {
        const css::uno::Reference< css::frame::XController >& xController = rxFrame->getController();
        if ( xController.is() )
        {
            const css::uno::Reference< css::frame::XModel >& xModel = xController->getModel();
            if ( xModel.is() )
            {
                const css::uno::Reference< css::document::XUndoManagerSupplier > xSuppUndo( xModel, css::uno::UNO_QUERY_THROW );
                return css::uno::Reference< css::document::XUndoManager >( xSuppUndo->getUndoManager(), css::uno::UNO_SET_THROW );
            }
        }

        return css::uno::Reference< css::document::XUndoManager > ();
    }
}

namespace sw::sidebar {

PageMarginControl::PageMarginControl(PageMarginPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"modules/swriter/ui/pagemargincontrol.ui"_ustr, u"PageMarginControl"_ustr)
    , m_xMoreButton(m_xBuilder->weld_button(u"moreoptions"_ustr))
    , m_xLeft(m_xBuilder->weld_label(u"leftLabel"_ustr))
    , m_xRight(m_xBuilder->weld_label(u"rightLabel"_ustr))
    , m_xInner(m_xBuilder->weld_label(u"innerLabel"_ustr))
    , m_xOuter(m_xBuilder->weld_label(u"outerLabel"_ustr))
    , m_xLeftMarginEdit(m_xBuilder->weld_metric_spin_button(u"left"_ustr, FieldUnit::CM))
    , m_xRightMarginEdit(m_xBuilder->weld_metric_spin_button(u"right"_ustr, FieldUnit::CM))
    , m_xTopMarginEdit(m_xBuilder->weld_metric_spin_button(u"top"_ustr, FieldUnit::CM))
    , m_xBottomMarginEdit(m_xBuilder->weld_metric_spin_button(u"bottom"_ustr, FieldUnit::CM))
    , m_xWidthHeightField(m_xBuilder->weld_metric_spin_button(u"hidden"_ustr, FieldUnit::CM))
    , m_xControl(pControl)
    , m_nPageLeftMargin(0)
    , m_nPageRightMargin(0)
    , m_nPageTopMargin(0)
    , m_nPageBottomMargin(0)
    , m_bMirrored(false)
    , m_eUnit( lcl_GetUnit() )
    , m_bUserCustomValuesAvailable( false )
    , m_nUserCustomPageLeftMargin( 0 )
    , m_nUserCustomPageRightMargin( 0 )
    , m_nUserCustomPageTopMargin( 0 )
    , m_nUserCustomPageBottomMargin( 0 )
    , m_bUserCustomMirrored( false )
    , m_bCustomValuesUsed( false )
{
    m_xWidthHeightField->set_unit(FieldUnit::CM);
    m_xWidthHeightField->set_range(0, 9999, FieldUnit::NONE);
    m_xWidthHeightField->set_digits(2);
    m_xWidthHeightField->set_increments(10, 100, FieldUnit::NONE);
    SetFieldUnit( *m_xWidthHeightField, lcl_GetFieldUnit() );

    bool bLandscape = false;
    const SvxSizeItem* pSize = nullptr;
    const SvxLongLRSpaceItem* pLRItem = nullptr;
    const SvxLongULSpaceItem* pULItem = nullptr;
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        SfxPoolItemHolder aResult;
        pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE, aResult);
        const SvxPageItem* pPageItem(static_cast<const SvxPageItem*>(aResult.getItem()));
        bLandscape = pPageItem->IsLandscape();
        m_bMirrored = pPageItem->GetPageUsage() == SvxPageUsage::Mirror;

        pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_SIZE, aResult);
        pSize = static_cast<const SvxSizeItem*>(aResult.getItem());

        pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_LRSPACE, aResult);
        pLRItem = static_cast<const SvxLongLRSpaceItem*>(aResult.getItem());

        pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_ULSPACE, aResult);
        pULItem = static_cast<const SvxLongULSpaceItem*>(aResult.getItem());
    }

    if ( pLRItem )
    {
        m_nPageLeftMargin = pLRItem->GetLeft();
        m_nPageRightMargin = pLRItem->GetRight();
    }

    if ( pULItem )
    {
        m_nPageTopMargin = pULItem->GetUpper();
        m_nPageBottomMargin = pULItem->GetLower();
    }

    if ( bLandscape )
    {
        m_xNarrow = m_xBuilder->weld_button(u"narrowL"_ustr);
        m_xNormal = m_xBuilder->weld_button(u"normalL"_ustr);
        m_xWide = m_xBuilder->weld_button(u"wideL"_ustr);
        m_xMirrored = m_xBuilder->weld_button(u"mirroredL"_ustr);
        m_xLast = m_xBuilder->weld_button(u"lastL"_ustr);
    }
    else
    {
        m_xNarrow = m_xBuilder->weld_button(u"narrow"_ustr);
        m_xNormal = m_xBuilder->weld_button(u"normal"_ustr);
        m_xWide = m_xBuilder->weld_button(u"wide"_ustr);
        m_xMirrored = m_xBuilder->weld_button(u"mirrored"_ustr);
        m_xLast = m_xBuilder->weld_button(u"last"_ustr);
    }

    m_xNarrow->show();
    m_xNormal->show();
    m_xWide->show();
    m_xMirrored->show();
    m_xLast->show();
    m_xMoreButton->show();

    m_xNarrow->connect_clicked( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_xNormal->connect_clicked( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_xWide->connect_clicked( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_xMirrored->connect_clicked( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_xLast->connect_clicked( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_xMoreButton->connect_clicked( LINK(this, PageMarginControl, MoreButtonClickHdl_Impl));

    m_bUserCustomValuesAvailable = GetUserCustomValues();

    FillHelpText( m_bUserCustomValuesAvailable );

    Link<weld::MetricSpinButton&,void> aLinkLR = LINK( this, PageMarginControl, ModifyLRMarginHdl );
    m_xLeftMarginEdit->connect_value_changed( aLinkLR );
    SetMetricValue( *m_xLeftMarginEdit, m_nPageLeftMargin, m_eUnit );
    SetFieldUnit( *m_xLeftMarginEdit, lcl_GetFieldUnit() );

    m_xRightMarginEdit->connect_value_changed( aLinkLR );
    SetMetricValue( *m_xRightMarginEdit, m_nPageRightMargin, m_eUnit );
    SetFieldUnit( *m_xRightMarginEdit, lcl_GetFieldUnit() );

    Link<weld::MetricSpinButton&,void> aLinkUL = LINK( this, PageMarginControl, ModifyULMarginHdl );
    m_xTopMarginEdit->connect_value_changed( aLinkUL );
    SetMetricValue( *m_xTopMarginEdit, m_nPageTopMargin, m_eUnit );
    SetFieldUnit( *m_xTopMarginEdit, lcl_GetFieldUnit() );

    m_xBottomMarginEdit->connect_value_changed( aLinkUL );
    SetMetricValue( *m_xBottomMarginEdit, m_nPageBottomMargin, m_eUnit );
    SetFieldUnit( *m_xBottomMarginEdit, lcl_GetFieldUnit() );

    if (pSize)
    {
        m_aPageSize = pSize->GetSize();
        SetMetricFieldMaxValues( m_aPageSize );
    }

    if ( m_bMirrored )
    {
        m_xLeft->hide();
        m_xRight->hide();
        m_xInner->show();
        m_xOuter->show();
    }
    else
    {
        m_xLeft->show();
        m_xRight->show();
        m_xInner->hide();
        m_xOuter->hide();
    }
}

void PageMarginControl::GrabFocus()
{
    m_xMoreButton->grab_focus();
}

PageMarginControl::~PageMarginControl()
{
    StoreUserCustomValues();
}

void PageMarginControl::SetMetricFieldMaxValues( const Size& rPageSize )
{
    const tools::Long nML = m_xLeftMarginEdit->denormalize( m_xLeftMarginEdit->get_value( FieldUnit::TWIP ) );
    const tools::Long nMR = m_xRightMarginEdit->denormalize( m_xRightMarginEdit->get_value( FieldUnit::TWIP ) );
    const tools::Long nMT = m_xTopMarginEdit->denormalize( m_xTopMarginEdit->get_value( FieldUnit::TWIP ) );
    const tools::Long nMB = m_xBottomMarginEdit->denormalize( m_xBottomMarginEdit->get_value( FieldUnit::TWIP ) );

    const tools::Long nPH  = OutputDevice::LogicToLogic( rPageSize.Height(), m_eUnit, MapUnit::MapTwip );
    const tools::Long nPW  = OutputDevice::LogicToLogic( rPageSize.Width(),  m_eUnit, MapUnit::MapTwip );

    // Left
    tools::Long nMax = nPW - nMR - MINBODY;
    m_xLeftMarginEdit->set_max( m_xLeftMarginEdit->normalize( nMax ), FieldUnit::TWIP );

    // Right
    nMax = nPW - nML - MINBODY;
    m_xRightMarginEdit->set_max( m_xRightMarginEdit->normalize( nMax ), FieldUnit::TWIP );

    //Top
    nMax = nPH - nMB - MINBODY;
    m_xTopMarginEdit->set_max( m_xTopMarginEdit->normalize( nMax ), FieldUnit::TWIP );

    //Bottom
    nMax = nPH - nMT -  MINBODY;
    m_xBottomMarginEdit->set_max( m_xTopMarginEdit->normalize( nMax ), FieldUnit::TWIP );
}

void PageMarginControl::FillHelpText( const bool bUserCustomValuesAvailable )
{
    const OUString aLeft = SwResId( STR_MARGIN_TOOLTIP_LEFT );
    const OUString aRight = SwResId( STR_MARGIN_TOOLTIP_RIGHT );
    const OUString aTop = SwResId( STR_MARGIN_TOOLTIP_TOP );
    const OUString aBottom = SwResId( STR_MARGIN_TOOLTIP_BOT );

    SetMetricValue( *m_xWidthHeightField, SWPAGE_NARROW_VALUE, m_eUnit );
    const OUString aNarrowValText = m_xWidthHeightField->get_text();
    OUString aHelpText = aLeft +
        aNarrowValText +
        aRight +
        aNarrowValText +
        aTop +
        aNarrowValText +
        aBottom +
        aNarrowValText;
    m_xNarrow->set_tooltip_text( aHelpText );

    SetMetricValue( *m_xWidthHeightField, SWPAGE_NORMAL_VALUE, m_eUnit );
    const OUString aNormalValText = m_xWidthHeightField->get_text();
    aHelpText = aLeft +
        aNormalValText +
        aRight +
        aNormalValText +
        aTop +
        aNormalValText +
        aBottom +
        aNormalValText;
    m_xNormal->set_tooltip_text( aHelpText );

    SetMetricValue( *m_xWidthHeightField, SWPAGE_WIDE_VALUE1, m_eUnit );
    const OUString aWide1ValText = m_xWidthHeightField->get_text();
    SetMetricValue( *m_xWidthHeightField, SWPAGE_WIDE_VALUE2, m_eUnit );
    const OUString aWide2ValText = m_xWidthHeightField->get_text();
    aHelpText = aLeft +
        aWide2ValText +
        aRight +
        aWide2ValText +
        aTop +
        aWide1ValText +
        aBottom +
        aWide1ValText;
    m_xWide->set_tooltip_text( aHelpText );

    const OUString aInner = SwResId( STR_MARGIN_TOOLTIP_INNER );
    const OUString aOuter = SwResId( STR_MARGIN_TOOLTIP_OUTER );

    SetMetricValue( *m_xWidthHeightField, SWPAGE_WIDE_VALUE3, m_eUnit );
    const OUString aWide3ValText = m_xWidthHeightField->get_text();
    aHelpText = aInner +
        aWide3ValText +
        aOuter +
        aWide1ValText +
        aTop +
        aWide1ValText +
        aBottom +
        aWide1ValText;
    m_xMirrored->set_tooltip_text( aHelpText );

    if ( bUserCustomValuesAvailable )
    {
        aHelpText = m_bUserCustomMirrored ? aInner : aLeft;
        SetMetricValue( *m_xWidthHeightField, m_nUserCustomPageLeftMargin, m_eUnit );
        aHelpText += m_xWidthHeightField->get_text();
        aHelpText += m_bUserCustomMirrored ? aOuter : aRight;
        SetMetricValue( *m_xWidthHeightField, m_nUserCustomPageRightMargin, m_eUnit );
        aHelpText += m_xWidthHeightField->get_text() + aTop;
        SetMetricValue( *m_xWidthHeightField, m_nUserCustomPageTopMargin, m_eUnit );
        aHelpText += m_xWidthHeightField->get_text() + aBottom;
        SetMetricValue( *m_xWidthHeightField, m_nUserCustomPageBottomMargin, m_eUnit );
        aHelpText += m_xWidthHeightField->get_text();
    }
    else
    {
        aHelpText.clear();
    }
    m_xLast->set_tooltip_text( aHelpText );
}

IMPL_LINK( PageMarginControl, SelectMarginHdl, weld::Button&, rControl, void )
{
    bool bMirrored = false;
    bool bApplyNewPageMargins = true;
    if( &rControl == m_xNarrow.get() )
    {
        m_nPageLeftMargin = SWPAGE_NARROW_VALUE;
        m_nPageRightMargin = SWPAGE_NARROW_VALUE;
        m_nPageTopMargin = SWPAGE_NARROW_VALUE;
        m_nPageBottomMargin = SWPAGE_NARROW_VALUE;
        bMirrored = false;
    }
    if( &rControl == m_xNormal.get() )
    {
        m_nPageLeftMargin = SWPAGE_NORMAL_VALUE;
        m_nPageRightMargin = SWPAGE_NORMAL_VALUE;
        m_nPageTopMargin = SWPAGE_NORMAL_VALUE;
        m_nPageBottomMargin = SWPAGE_NORMAL_VALUE;
        bMirrored = false;
    }
    if( &rControl == m_xWide.get() )
    {
        m_nPageLeftMargin = SWPAGE_WIDE_VALUE2;
        m_nPageRightMargin = SWPAGE_WIDE_VALUE2;
        m_nPageTopMargin = SWPAGE_WIDE_VALUE1;
        m_nPageBottomMargin = SWPAGE_WIDE_VALUE1;
        bMirrored = false;
    }
    if( &rControl == m_xMirrored.get() )
    {
        m_nPageLeftMargin = SWPAGE_WIDE_VALUE3;
        m_nPageRightMargin = SWPAGE_WIDE_VALUE1;
        m_nPageTopMargin = SWPAGE_WIDE_VALUE1;
        m_nPageBottomMargin = SWPAGE_WIDE_VALUE1;
        bMirrored = true;
    }
    if( &rControl == m_xLast.get() )
    {
        if ( m_bUserCustomValuesAvailable )
        {
            m_nPageLeftMargin = m_nUserCustomPageLeftMargin;
            m_nPageRightMargin = m_nUserCustomPageRightMargin;
            m_nPageTopMargin = m_nUserCustomPageTopMargin;
            m_nPageBottomMargin = m_nUserCustomPageBottomMargin;
            bMirrored = m_bUserCustomMirrored;
        }
        else
        {
            bApplyNewPageMargins = false;
        }
    }

    if ( !bApplyNewPageMargins )
        return;

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    const css::uno::Reference<css::document::XUndoManager> xUndoManager(pViewFrm ? getUndoManager(pViewFrm->GetFrame().GetFrameInterface()) : nullptr);
    if ( xUndoManager.is() )
        xUndoManager->enterUndoContext( u""_ustr );

    ExecuteMarginLRChange( m_nPageLeftMargin, m_nPageRightMargin );
    ExecuteMarginULChange( m_nPageTopMargin, m_nPageBottomMargin );
    if ( m_bMirrored != bMirrored )
    {
        m_bMirrored = bMirrored;
        ExecutePageLayoutChange( m_bMirrored );
    }

    if ( xUndoManager.is() )
        xUndoManager->leaveUndoContext();

    m_bCustomValuesUsed = false;
    m_xControl->EndPopupMode();
}

void PageMarginControl::ExecuteMarginLRChange(
    const tools::Long nPageLeftMargin,
    const tools::Long nPageRightMargin )
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        SvxLongLRSpaceItem aPageLRMarginItem( 0, 0, SID_ATTR_PAGE_LRSPACE );
        aPageLRMarginItem.SetLeft( nPageLeftMargin );
        aPageLRMarginItem.SetRight( nPageRightMargin );
        pViewFrm->GetBindings().GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE,
                SfxCallMode::RECORD, { &aPageLRMarginItem } );
    }
}

void PageMarginControl::ExecuteMarginULChange(
    const tools::Long nPageTopMargin,
    const tools::Long nPageBottomMargin )
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        SvxLongULSpaceItem aPageULMarginItem( 0, 0, SID_ATTR_PAGE_ULSPACE );
        aPageULMarginItem.SetUpper( nPageTopMargin );
        aPageULMarginItem.SetLower( nPageBottomMargin );
        pViewFrm->GetBindings().GetDispatcher()->ExecuteList( SID_ATTR_PAGE_ULSPACE,
                SfxCallMode::RECORD, { &aPageULMarginItem } );
    }
}

void PageMarginControl::ExecutePageLayoutChange( const bool bMirrored )
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        SvxPageItem aPageItem( SID_ATTR_PAGE );
        aPageItem.SetPageUsage( bMirrored ? SvxPageUsage::Mirror : SvxPageUsage::All );
        pViewFrm->GetBindings().GetDispatcher()->ExecuteList( SID_ATTR_PAGE,
                SfxCallMode::RECORD, { &aPageItem } );
    }
}

IMPL_LINK_NOARG( PageMarginControl, ModifyLRMarginHdl, weld::MetricSpinButton&, void )
{
    m_nPageLeftMargin = GetCoreValue( *m_xLeftMarginEdit, m_eUnit );
    m_nPageRightMargin = GetCoreValue( *m_xRightMarginEdit, m_eUnit );
    ExecuteMarginLRChange( m_nPageLeftMargin, m_nPageRightMargin );
    SetMetricFieldMaxValues( m_aPageSize );
    m_bCustomValuesUsed = true;
}

IMPL_LINK_NOARG( PageMarginControl, ModifyULMarginHdl, weld::MetricSpinButton&, void )
{
    m_nPageTopMargin = GetCoreValue( *m_xTopMarginEdit, m_eUnit );
    m_nPageBottomMargin = GetCoreValue( *m_xBottomMarginEdit, m_eUnit );
    ExecuteMarginULChange( m_nPageTopMargin, m_nPageBottomMargin );
    SetMetricFieldMaxValues( m_aPageSize );
    m_bCustomValuesUsed = true;
}

IMPL_LINK_NOARG(PageMarginControl, MoreButtonClickHdl_Impl, weld::Button&, void)
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetBindings().GetDispatcher()->Execute(FN_FORMAT_PAGE_SETTING_DLG,
                                                         SfxCallMode::ASYNCHRON);
    m_xControl->EndPopupMode();
}

bool PageMarginControl::GetUserCustomValues()
{
    bool bUserCustomValuesAvailable = false;

    SvtViewOptions aWinOpt( EViewType::Window, SWPAGE_LEFT_GVALUE );
    if ( aWinOpt.Exists() )
    {
        css::uno::Sequence < css::beans::NamedValue > aSeq = aWinOpt.GetUserData();
        OUString aTmp;
        if ( aSeq.hasElements())
            aSeq[0].Value >>= aTmp;
        OUString aWinData( aTmp );
        m_nUserCustomPageLeftMargin = aWinData.toInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt2( EViewType::Window, SWPAGE_RIGHT_GVALUE );
    if ( aWinOpt2.Exists() )
    {
        css::uno::Sequence < css::beans::NamedValue > aSeq = aWinOpt2.GetUserData();
        OUString aTmp;
        if ( aSeq.hasElements())
            aSeq[0].Value >>= aTmp;
        OUString aWinData( aTmp );
        m_nUserCustomPageRightMargin = aWinData.toInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt3( EViewType::Window, SWPAGE_TOP_GVALUE );
    if ( aWinOpt3.Exists() )
    {
        css::uno::Sequence < css::beans::NamedValue > aSeq = aWinOpt3.GetUserData();
        OUString aTmp;
        if ( aSeq.hasElements() )
            aSeq[0].Value >>= aTmp;
        OUString aWinData( aTmp );
        m_nUserCustomPageTopMargin = aWinData.toInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt4( EViewType::Window, SWPAGE_DOWN_GVALUE );
    if ( aWinOpt4.Exists() )
    {
        css::uno::Sequence < css::beans::NamedValue > aSeq = aWinOpt4.GetUserData();
        OUString aTmp;
        if ( aSeq.hasElements())
            aSeq[0].Value >>= aTmp;
        OUString aWinData( aTmp );
        m_nUserCustomPageBottomMargin = aWinData.toInt32();
        bUserCustomValuesAvailable = true;
    }

    SvtViewOptions aWinOpt5( EViewType::Window, SWPAGE_MIRROR_GVALUE );
    if ( aWinOpt5.Exists() )
    {
        css::uno::Sequence < css::beans::NamedValue > aSeq = aWinOpt5.GetUserData();
        OUString aTmp;
        if ( aSeq.hasElements())
            aSeq[0].Value >>= aTmp;
        OUString aWinData( aTmp );
        m_bUserCustomMirrored = aWinData.toInt32() != 0;
        bUserCustomValuesAvailable = true;
    }

    return bUserCustomValuesAvailable;
}

void PageMarginControl::StoreUserCustomValues()
{
    if ( !m_bCustomValuesUsed )
    {
        return;
    }

    css::uno::Sequence < css::beans::NamedValue > aSeq( 1 );
    auto pSeq = aSeq.getArray();
    SvtViewOptions aWinOpt( EViewType::Window, SWPAGE_LEFT_GVALUE );

    pSeq[0].Name = "mnPageLeftMargin";
    pSeq[0].Value <<= OUString::number( m_nPageLeftMargin );
    aWinOpt.SetUserData( aSeq );

    SvtViewOptions aWinOpt2( EViewType::Window, SWPAGE_RIGHT_GVALUE );
    pSeq[0].Name = "mnPageRightMargin";
    pSeq[0].Value <<= OUString::number( m_nPageRightMargin );
    aWinOpt2.SetUserData( aSeq );

    SvtViewOptions aWinOpt3( EViewType::Window, SWPAGE_TOP_GVALUE );
    pSeq[0].Name = "mnPageTopMargin";
    pSeq[0].Value <<= OUString::number( m_nPageTopMargin );
    aWinOpt3.SetUserData( aSeq );

    SvtViewOptions aWinOpt4( EViewType::Window, SWPAGE_DOWN_GVALUE );
    pSeq[0].Name = "mnPageBottomMargin";
    pSeq[0].Value <<= OUString::number( m_nPageBottomMargin );
    aWinOpt4.SetUserData( aSeq );

    SvtViewOptions aWinOpt5( EViewType::Window, SWPAGE_MIRROR_GVALUE );
    pSeq[0].Name = "mbMirrored";
    pSeq[0].Value <<= OUString::number( m_bMirrored ? 1 : 0 );
    aWinOpt5.SetUserData( aSeq );
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
