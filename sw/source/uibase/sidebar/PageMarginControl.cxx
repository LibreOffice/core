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

#include <cstdlib>

#include "PageMarginControl.hxx"
#include "strings.hrc"

#include <editeng/sizeitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <svx/pageitem.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>

#include <swtypes.hxx>
#include <cmdid.h>

#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <vcl/settings.hxx>

#define SWPAGE_LEFT_GVALUE      "Sw_Page_Left"
#define SWPAGE_RIGHT_GVALUE     "Sw_Page_Right"
#define SWPAGE_TOP_GVALUE       "Sw_Page_Top"
#define SWPAGE_DOWN_GVALUE      "Sw_Page_Down"
#define SWPAGE_MIRROR_GVALUE    "Sw_Page_Mirrored"

namespace
{
    FieldUnit lcl_GetFieldUnit()
    {
        FieldUnit eUnit = FUNIT_INCH;
        const SfxPoolItem* pItem = nullptr;
        SfxItemState eState = SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_METRIC, pItem );
        if ( pItem && eState >= SfxItemState::DEFAULT )
        {
            eUnit = (FieldUnit)static_cast<const SfxUInt16Item*>( pItem )->GetValue();
        }
        else
        {
            return SfxModule::GetCurrentFieldUnit();
        }

        return eUnit;
    }

    MapUnit lcl_GetUnit()
    {
        SfxItemPool &rPool = SfxGetpApp()->GetPool();
        sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
        return rPool.GetMetric( nWhich );
    }

    const css::uno::Reference< css::document::XUndoManager > getUndoManager( const css::uno::Reference< css::frame::XFrame >& rxFrame )
    {
        const css::uno::Reference< css::frame::XController >& xController = rxFrame->getController();
        if ( xController.is() )
        {
            const css::uno::Reference< css::frame::XModel >& xModel = xController->getModel();
            if ( xModel.is() )
            {
                const css::uno::Reference< css::document::XUndoManagerSupplier > xSuppUndo( xModel, css::uno::UNO_QUERY_THROW );
                if ( xSuppUndo.is() )
                {
                    const css::uno::Reference< css::document::XUndoManager > xUndoManager( xSuppUndo->getUndoManager(), css::uno::UNO_QUERY_THROW );
                    return xUndoManager;
                }
            }
        }

        return css::uno::Reference< css::document::XUndoManager > ();
    }
}

namespace sw { namespace sidebar {

PageMarginControl::PageMarginControl( sal_uInt16 nId )
    : SfxPopupWindow( nId, "PageMarginControl", "modules/swriter/ui/pagemargincontrol.ui" )
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
    bool bLandscape = false;
    const SfxPoolItem* pItem;
    const SvxSizeItem* pSize = nullptr;
    const SvxLongLRSpaceItem* pLRItem = nullptr;
    const SvxLongULSpaceItem* pULItem = nullptr;
    if ( SfxViewFrame::Current() )
    {
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE, pItem );
        bLandscape = static_cast<const SvxPageItem*>( pItem )->IsLandscape();
        m_bMirrored = static_cast<const SvxPageItem*>( pItem )->GetPageUsage() == SvxPageUsage::Mirror;
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE_SIZE, pItem );
        pSize = static_cast<const SvxSizeItem*>( pItem );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE_LRSPACE, pItem );
        pLRItem = static_cast<const SvxLongLRSpaceItem*>( pItem );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE_ULSPACE, pItem );
        pULItem = static_cast<const SvxLongULSpaceItem*>( pItem );
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
        get( m_pNarrow, "narrowL" );
        get( m_pNormal, "normalL" );
        get( m_pWide, "wideL" );
        get( m_pMirrored, "mirroredL" );
        get( m_pLast, "lastL" );
    }
    else
    {
        get( m_pNarrow, "narrow" );
        get( m_pNormal, "normal" );
        get( m_pWide, "wide" );
        get( m_pMirrored, "mirrored" );
        get( m_pLast, "last" );
    }

    m_pNarrow->Show();
    m_pNormal->Show();
    m_pWide->Show();
    m_pMirrored->Show();
    m_pLast->Show();

    m_pNarrow->SetClickHdl( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_pNormal->SetClickHdl( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_pWide->SetClickHdl( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_pMirrored->SetClickHdl( LINK( this, PageMarginControl, SelectMarginHdl ) );
    m_pLast->SetClickHdl( LINK( this, PageMarginControl, SelectMarginHdl ) );

    get( m_pContainer, "container" );
    m_pWidthHeightField = VclPtr<MetricField>::Create( m_pContainer.get(), (WinBits)0 );
    m_pWidthHeightField->Hide();
    m_pWidthHeightField->SetUnit( FUNIT_CM );
    m_pWidthHeightField->SetMax( 9999 );
    m_pWidthHeightField->SetDecimalDigits( 2 );
    m_pWidthHeightField->SetSpinSize( 10 );
    m_pWidthHeightField->SetLast( 9999 );
    SetFieldUnit( *m_pWidthHeightField.get(), lcl_GetFieldUnit() );

    m_bUserCustomValuesAvailable = GetUserCustomValues();

    FillHelpText( m_bUserCustomValuesAvailable );

    get( m_pLeftMarginEdit, "left" );
    get( m_pRightMarginEdit, "right" );
    get( m_pTopMarginEdit, "top" );
    get( m_pBottomMarginEdit, "bottom" );

    Link<Edit&,void> aLinkLR = LINK( this, PageMarginControl, ModifyLRMarginHdl );
    m_pLeftMarginEdit->SetModifyHdl( aLinkLR );
    SetMetricValue( *m_pLeftMarginEdit.get(), m_nPageLeftMargin, m_eUnit );

    m_pRightMarginEdit->SetModifyHdl( aLinkLR );
    SetMetricValue( *m_pRightMarginEdit.get(), m_nPageRightMargin, m_eUnit );

    Link<Edit&,void> aLinkUL = LINK( this, PageMarginControl, ModifyULMarginHdl );
    m_pTopMarginEdit->SetModifyHdl( aLinkUL );
    SetMetricValue( *m_pTopMarginEdit.get(), m_nPageTopMargin, m_eUnit );

    m_pBottomMarginEdit->SetModifyHdl( aLinkUL );
    SetMetricValue( *m_pBottomMarginEdit.get(), m_nPageBottomMargin, m_eUnit );

    m_aPageSize = pSize->GetSize();
    SetMetricFieldMaxValues( m_aPageSize );

    get( m_pLeft, "leftLabel" );
    get( m_pRight, "rightLabel" );
    get( m_pInner, "innerLabel" );
    get( m_pOuter, "outerLabel" );

    if ( m_bMirrored )
    {
        m_pLeft->Hide();
        m_pRight->Hide();
        m_pInner->Show();
        m_pOuter->Show();
    }
    else
    {
        m_pLeft->Show();
        m_pRight->Show();
        m_pInner->Hide();
        m_pOuter->Hide();
    }
}

PageMarginControl::~PageMarginControl()
{
    disposeOnce();
}

void PageMarginControl::dispose()
{
    StoreUserCustomValues();

    m_pLeft.disposeAndClear();
    m_pRight.disposeAndClear();
    m_pInner.disposeAndClear();
    m_pOuter.disposeAndClear();
    m_pLeftMarginEdit.disposeAndClear();
    m_pRightMarginEdit.disposeAndClear();
    m_pTopMarginEdit.disposeAndClear();
    m_pBottomMarginEdit.disposeAndClear();
    m_pNarrow.disposeAndClear();
    m_pNormal.disposeAndClear();
    m_pWide.disposeAndClear();
    m_pMirrored.disposeAndClear();
    m_pLast.disposeAndClear();

    m_pWidthHeightField.disposeAndClear();
    m_pContainer.disposeAndClear();

    SfxPopupWindow::dispose();
}

void PageMarginControl::SetMetricFieldMaxValues( const Size& rPageSize )
{
    const long nML = m_pLeftMarginEdit->Denormalize( m_pLeftMarginEdit->GetValue( FUNIT_TWIP ) );
    const long nMR = m_pRightMarginEdit->Denormalize( m_pRightMarginEdit->GetValue( FUNIT_TWIP ) );
    const long nMT = m_pTopMarginEdit->Denormalize( m_pTopMarginEdit->GetValue( FUNIT_TWIP ) );
    const long nMB = m_pBottomMarginEdit->Denormalize( m_pBottomMarginEdit->GetValue( FUNIT_TWIP ) );

    const long nPH  = LogicToLogic( rPageSize.Height(), (MapUnit)m_eUnit, MapUnit::MapTwip );
    const long nPW  = LogicToLogic( rPageSize.Width(),  (MapUnit)m_eUnit, MapUnit::MapTwip );

    // Left
    long nMax = nPW - nMR - MINBODY;
    m_pLeftMarginEdit->SetMax( m_pLeftMarginEdit->Normalize( nMax ), FUNIT_TWIP );

    // Right
    nMax = nPW - nML - MINBODY;
    m_pRightMarginEdit->SetMax( m_pRightMarginEdit->Normalize( nMax ), FUNIT_TWIP );

    //Top
    nMax = nPH - nMB - MINBODY;
    m_pTopMarginEdit->SetMax( m_pTopMarginEdit->Normalize( nMax ), FUNIT_TWIP );

    //Bottom
    nMax = nPH - nMT -  MINBODY;
    m_pBottomMarginEdit->SetMax( m_pTopMarginEdit->Normalize( nMax ), FUNIT_TWIP );
}

void PageMarginControl::FillHelpText( const bool bUserCustomValuesAvailable )
{
    const OUString aLeft = SwResId( STR_MARGIN_TOOLTIP_LEFT );
    const OUString aRight = SwResId( STR_MARGIN_TOOLTIP_RIGHT );
    const OUString aTop = SwResId( STR_MARGIN_TOOLTIP_TOP );
    const OUString aBottom = SwResId( STR_MARGIN_TOOLTIP_BOT );

    SetMetricValue( *m_pWidthHeightField.get(), SWPAGE_NARROW_VALUE, m_eUnit );
    const OUString aNarrowValText = m_pWidthHeightField->GetText();
    OUString aHelpText = aLeft;
    aHelpText += aNarrowValText;
    aHelpText += aRight;
    aHelpText += aNarrowValText;
    aHelpText += aTop;
    aHelpText += aNarrowValText;
    aHelpText += aBottom;
    aHelpText += aNarrowValText;
    m_pNarrow->SetQuickHelpText( aHelpText );

    SetMetricValue( *m_pWidthHeightField.get(), SWPAGE_NORMAL_VALUE, m_eUnit );
    const OUString aNormalValText = m_pWidthHeightField->GetText();
    aHelpText = aLeft;
    aHelpText += aNormalValText;
    aHelpText += aRight;
    aHelpText += aNormalValText;
    aHelpText += aTop;
    aHelpText += aNormalValText;
    aHelpText += aBottom;
    aHelpText += aNormalValText;
    m_pNormal->SetQuickHelpText( aHelpText );

    SetMetricValue( *m_pWidthHeightField.get(), SWPAGE_WIDE_VALUE1, m_eUnit );
    const OUString aWide1ValText = m_pWidthHeightField->GetText();
    SetMetricValue( *m_pWidthHeightField.get(), SWPAGE_WIDE_VALUE2, m_eUnit );
    const OUString aWide2ValText = m_pWidthHeightField->GetText();
    aHelpText = aLeft;
    aHelpText += aWide2ValText;
    aHelpText += aRight;
    aHelpText += aWide2ValText;
    aHelpText += aTop;
    aHelpText += aWide1ValText;
    aHelpText += aBottom;
    aHelpText += aWide1ValText;
    m_pWide->SetQuickHelpText( aHelpText );

    const OUString aInner = SwResId( STR_MARGIN_TOOLTIP_INNER );
    const OUString aOuter = SwResId( STR_MARGIN_TOOLTIP_OUTER );

    SetMetricValue( *m_pWidthHeightField.get(), SWPAGE_WIDE_VALUE3, m_eUnit );
    const OUString aWide3ValText = m_pWidthHeightField->GetText();
    aHelpText = aInner;
    aHelpText += aWide3ValText;
    aHelpText += aOuter;
    aHelpText += aWide1ValText;
    aHelpText += aTop;
    aHelpText += aWide1ValText;
    aHelpText += aBottom;
    aHelpText += aWide1ValText;
    m_pMirrored->SetQuickHelpText( aHelpText );

    if ( bUserCustomValuesAvailable )
    {
        aHelpText = m_bUserCustomMirrored ? aInner : aLeft;
        SetMetricValue( *m_pWidthHeightField.get(), m_nUserCustomPageLeftMargin, m_eUnit );
        aHelpText += m_pWidthHeightField->GetText();
        aHelpText += m_bUserCustomMirrored ? aOuter : aRight;
        SetMetricValue( *m_pWidthHeightField.get(), m_nUserCustomPageRightMargin, m_eUnit );
        aHelpText += m_pWidthHeightField->GetText();
        aHelpText += aTop;
        SetMetricValue( *m_pWidthHeightField.get(), m_nUserCustomPageTopMargin, m_eUnit );
        aHelpText += m_pWidthHeightField->GetText();
        aHelpText += aBottom;
        SetMetricValue( *m_pWidthHeightField.get(), m_nUserCustomPageBottomMargin, m_eUnit );
        aHelpText += m_pWidthHeightField->GetText();
    }
    else
    {
        aHelpText.clear();
    }
    m_pLast->SetQuickHelpText( aHelpText );
}

IMPL_LINK( PageMarginControl, SelectMarginHdl, Button*, pControl, void )
{
    bool bMirrored = false;
    bool bApplyNewPageMargins = true;
    if( pControl == m_pNarrow.get() )
    {
        m_nPageLeftMargin = SWPAGE_NARROW_VALUE;
        m_nPageRightMargin = SWPAGE_NARROW_VALUE;
        m_nPageTopMargin = SWPAGE_NARROW_VALUE;
        m_nPageBottomMargin = SWPAGE_NARROW_VALUE;
        bMirrored = false;
    }
    if( pControl == m_pNormal.get() )
    {
        m_nPageLeftMargin = SWPAGE_NORMAL_VALUE;
        m_nPageRightMargin = SWPAGE_NORMAL_VALUE;
        m_nPageTopMargin = SWPAGE_NORMAL_VALUE;
        m_nPageBottomMargin = SWPAGE_NORMAL_VALUE;
        bMirrored = false;
    }
    if( pControl == m_pWide.get() )
    {
        m_nPageLeftMargin = SWPAGE_WIDE_VALUE2;
        m_nPageRightMargin = SWPAGE_WIDE_VALUE2;
        m_nPageTopMargin = SWPAGE_WIDE_VALUE1;
        m_nPageBottomMargin = SWPAGE_WIDE_VALUE1;
        bMirrored = false;
    }
    if( pControl == m_pMirrored.get() )
    {
        m_nPageLeftMargin = SWPAGE_WIDE_VALUE3;
        m_nPageRightMargin = SWPAGE_WIDE_VALUE1;
        m_nPageTopMargin = SWPAGE_WIDE_VALUE1;
        m_nPageBottomMargin = SWPAGE_WIDE_VALUE1;
        bMirrored = true;
    }
    if( pControl == m_pLast.get() )
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

    if ( bApplyNewPageMargins )
    {
        const css::uno::Reference< css::document::XUndoManager > xUndoManager( getUndoManager( SfxViewFrame::Current()->GetFrame().GetFrameInterface() ) );
        if ( xUndoManager.is() )
            xUndoManager->enterUndoContext( "" );

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
        EndPopupMode();
    }
}

void PageMarginControl::ExecuteMarginLRChange(
    const long nPageLeftMargin,
    const long nPageRightMargin )
{
    if ( SfxViewFrame::Current() )
    {
        std::unique_ptr<SvxLongLRSpaceItem> pPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) );
        pPageLRMarginItem->SetLeft( nPageLeftMargin );
        pPageLRMarginItem->SetRight( nPageRightMargin );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE,
                SfxCallMode::RECORD, { pPageLRMarginItem.get() } );
        pPageLRMarginItem.reset();
    }
}

void PageMarginControl::ExecuteMarginULChange(
    const long nPageTopMargin,
    const long nPageBottomMargin )
{
    if ( SfxViewFrame::Current() )
    {
        std::unique_ptr<SvxLongULSpaceItem> pPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) );
        pPageULMarginItem->SetUpper( nPageTopMargin );
        pPageULMarginItem->SetLower( nPageBottomMargin );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList( SID_ATTR_PAGE_ULSPACE,
                SfxCallMode::RECORD, { pPageULMarginItem.get() } );
        pPageULMarginItem.reset();
    }
}

void PageMarginControl::ExecutePageLayoutChange( const bool bMirrored )
{
    if ( SfxViewFrame::Current() )
    {
        std::unique_ptr<SvxPageItem> pPageItem( new SvxPageItem( SID_ATTR_PAGE ) );
        pPageItem->SetPageUsage( bMirrored ? SvxPageUsage::Mirror : SvxPageUsage::All );
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->ExecuteList( SID_ATTR_PAGE,
                SfxCallMode::RECORD, { pPageItem.get() } );
        pPageItem.reset();
    }
}

IMPL_LINK_NOARG( PageMarginControl, ModifyLRMarginHdl, Edit&, void )
{
    m_nPageLeftMargin = GetCoreValue( *m_pLeftMarginEdit.get(), m_eUnit );
    m_nPageRightMargin = GetCoreValue( *m_pRightMarginEdit.get(), m_eUnit );
    ExecuteMarginLRChange( m_nPageLeftMargin, m_nPageRightMargin );
    SetMetricFieldMaxValues( m_aPageSize );
    m_bCustomValuesUsed = true;
}

IMPL_LINK_NOARG( PageMarginControl, ModifyULMarginHdl, Edit&, void )
{
    m_nPageTopMargin = GetCoreValue( *m_pTopMarginEdit.get(), m_eUnit );
    m_nPageBottomMargin = GetCoreValue( *m_pBottomMarginEdit.get(), m_eUnit );
    ExecuteMarginULChange( m_nPageTopMargin, m_nPageBottomMargin );
    SetMetricFieldMaxValues( m_aPageSize );
    m_bCustomValuesUsed = true;
}

bool PageMarginControl::GetUserCustomValues()
{
    bool bUserCustomValuesAvailable = false;

    SvtViewOptions aWinOpt( EViewType::Window, SWPAGE_LEFT_GVALUE );
    if ( aWinOpt.Exists() )
    {
        css::uno::Sequence < css::beans::NamedValue > aSeq = aWinOpt.GetUserData();
        OUString aTmp;
        if ( aSeq.getLength())
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
        if ( aSeq.getLength())
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
        if ( aSeq.getLength() )
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
        if ( aSeq.getLength())
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
        if ( aSeq.getLength())
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
    SvtViewOptions aWinOpt( EViewType::Window, SWPAGE_LEFT_GVALUE );

    aSeq[0].Name = "mnPageLeftMargin";
    aSeq[0].Value <<= OUString::number( m_nPageLeftMargin );
    aWinOpt.SetUserData( aSeq );

    SvtViewOptions aWinOpt2( EViewType::Window, SWPAGE_RIGHT_GVALUE );
    aSeq[0].Name = "mnPageRightMargin";
    aSeq[0].Value <<= OUString::number( m_nPageRightMargin );
    aWinOpt2.SetUserData( aSeq );

    SvtViewOptions aWinOpt3( EViewType::Window, SWPAGE_TOP_GVALUE );
    aSeq[0].Name = "mnPageTopMargin";
    aSeq[0].Value <<= OUString::number( m_nPageTopMargin );
    aWinOpt3.SetUserData( aSeq );

    SvtViewOptions aWinOpt4( EViewType::Window, SWPAGE_DOWN_GVALUE );
    aSeq[0].Name = "mnPageBottomMargin";
    aSeq[0].Value <<= OUString::number( m_nPageBottomMargin );
    aWinOpt4.SetUserData( aSeq );

    SvtViewOptions aWinOpt5( EViewType::Window, SWPAGE_MIRROR_GVALUE );
    aSeq[0].Name = "mbMirrored";
    aSeq[0].Value <<= OUString::number( (m_bMirrored ? 1 : 0) );
    aWinOpt5.SetUserData( aSeq );
}

} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
