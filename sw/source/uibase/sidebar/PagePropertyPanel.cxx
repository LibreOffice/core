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

#include <sal/config.h>

#include <cstdlib>

#include "PagePropertyPanel.hxx"
#include "PagePropertyPanel.hrc"

#include "PropertyPanel.hrc"

#include <svx/sidebar/PopupContainer.hxx>
#include "PageMarginControl.hxx"

#include <swtypes.hxx>
#include <cmdid.h>

#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>

#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

const char UNO_MARGIN[]      = ".uno:Margin";

namespace {
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

VclPtr<vcl::Window> PagePropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference< css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to PagePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to PagePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException("no SfxBindings given to PagePropertyPanel::Create", nullptr, 2);

    return VclPtr<PagePropertyPanel>::Create( pParent,
                                              rxFrame,
                                              pBindings);
}

PagePropertyPanel::PagePropertyPanel(
            vcl::Window* pParent,
            const css::uno::Reference< css::frame::XFrame>& rxFrame,
            SfxBindings* pBindings)
    : PanelLayout(pParent, "PagePropertyPanel", "modules/swriter/ui/sidebarpage.ui", rxFrame)
    , mpBindings(pBindings)

    // image resources
    , maImgSize                 (nullptr)
    , maImgSize_L                   (nullptr)
    , mImgNarrow                    (SW_RES(IMG_PAGE_NARROW))
    , mImgNormal                    (SW_RES(IMG_PAGE_NORMAL))
    , mImgWide                  (SW_RES(IMG_PAGE_WIDE))
    , mImgMirrored              (SW_RES(IMG_PAGE_MIRRORED))
    , mImgMarginCustom          (SW_RES(IMG_PAGE_MARGIN_CUSTOM))
    , mImgNarrow_L              (SW_RES(IMG_PAGE_NARROW_L))
    , mImgNormal_L              (SW_RES(IMG_PAGE_NORMAL_L))
    , mImgWide_L                    (SW_RES(IMG_PAGE_WIDE_L))
    , mImgMirrored_L                (SW_RES(IMG_PAGE_MIRRORED_L))
    , mImgMarginCustom_L            (SW_RES(IMG_PAGE_MARGIN_CUSTOM_L))

    , mpPageItem( new SvxPageItem(SID_ATTR_PAGE) )
    , mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) )
    , mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) )
    , mpPageSizeItem( new SvxSizeItem(SID_ATTR_PAGE_SIZE) )

    , meFUnit()
    , meUnit()

    , m_aSwPagePgULControl(SID_ATTR_PAGE_ULSPACE, *pBindings, *this)
    , m_aSwPagePgLRControl(SID_ATTR_PAGE_LRSPACE, *pBindings, *this)
    , m_aSwPagePgSizeControl(SID_ATTR_PAGE_SIZE, *pBindings, *this)
    , m_aSwPagePgMetricControl(SID_ATTR_METRIC, *pBindings, *this)

    , maMarginPopup( this,
            [this] (svx::sidebar::PopupContainer *parent) { return this->CreatePageMarginControl(parent); },
                     OUString("Page margins") )

    , mxUndoManager( getUndoManager( rxFrame ) )

    , mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify( false )
{
    // visible controls
    get(mpToolBoxMargin, "selectmargin");

    Initialize();
    mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify = true;
}

PagePropertyPanel::~PagePropertyPanel()
{
    disposeOnce();
}

void PagePropertyPanel::dispose()
{
    delete[] maImgSize;
    maImgSize = nullptr;
    delete[] maImgSize_L;
    maImgSize_L = nullptr;

    mpPageItem.reset();
    mpPageLRMarginItem.reset();
    mpPageULMarginItem.reset();
    mpPageSizeItem.reset();

    mpToolBoxMargin.clear();

    m_aSwPagePgULControl.dispose();
    m_aSwPagePgLRControl.dispose();
    m_aSwPagePgSizeControl.dispose();
    m_aSwPagePgMetricControl.dispose();

    maMarginPopup.dispose();

    PanelLayout::dispose();
}

void PagePropertyPanel::Initialize()
{
    // popup for page margins
    const sal_uInt16 nIdMargin = mpToolBoxMargin->GetItemId(UNO_MARGIN);
    Link<ToolBox *, void> aLink = LINK( this, PagePropertyPanel, ClickMarginHdl );
    mpToolBoxMargin->SetDropdownClickHdl( aLink );
    mpToolBoxMargin->SetSelectHdl( aLink );
    mpToolBoxMargin->SetItemImage(nIdMargin, mImgNormal);
    mpToolBoxMargin->SetItemBits( nIdMargin, mpToolBoxMargin->GetItemBits( nIdMargin ) | ToolBoxItemBits::DROPDOWNONLY );

    meFUnit = GetModuleFieldUnit();
    meUnit  = m_aSwPagePgSizeControl.GetCoreMetric();

    // 'pull' for page style's attribute values
    mpBindings->Update( SID_ATTR_PAGE_LRSPACE );
    mpBindings->Update( SID_ATTR_PAGE_ULSPACE );
    mpBindings->Update( SID_ATTR_PAGE );
    mpBindings->Update( SID_ATTR_PAGE_SIZE );
}

VclPtr< svx::sidebar::PopupControl> PagePropertyPanel::CreatePageMarginControl( svx::sidebar::PopupContainer* pParent )
{
    return VclPtr<PageMarginControl>::Create(

        pParent,
        *this,
        *mpPageLRMarginItem.get(),
        *mpPageULMarginItem.get(),
        mpPageItem->GetPageUsage() == SVX_PAGE_MIRROR,
        mpPageSizeItem->GetSize(),
        mpPageItem->IsLandscape(),
        meFUnit,
        meUnit );
}

void PagePropertyPanel::ExecuteMarginLRChange(
    const long nPageLeftMargin,
    const long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_LRSPACE,
            SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PagePropertyPanel::ExecuteMarginULChange(
    const long nPageTopMargin,
    const long nPageBottomMargin )
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_ULSPACE,
            SfxCallMode::RECORD, { mpPageULMarginItem.get() });
}

void PagePropertyPanel::ExecutePageLayoutChange( const bool bMirrored )
{
    mpPageItem->SetPageUsage( bMirrored ? SVX_PAGE_MIRROR : SVX_PAGE_ALL );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE,
            SfxCallMode::RECORD, { mpPageItem.get() });
}

IMPL_LINK_TYPED( PagePropertyPanel, ClickMarginHdl, ToolBox*, pToolBox, void )
{
    maMarginPopup.Show( *pToolBox );
}

void PagePropertyPanel::ClosePageMarginPopup()
{
    maMarginPopup.Hide();
}

void PagePropertyPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    if (IsDisposed())
        return;

    switch( nSId )
    {
    case SID_ATTR_PAGE_LRSPACE:
        if ( eState >= SfxItemState::DEFAULT &&
             pState && dynamic_cast< const SvxLongLRSpaceItem *>( pState ) !=  nullptr )
        {
            mpPageLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
            ChangeMarginImage();
        }
        break;

    case SID_ATTR_PAGE_ULSPACE:
        if ( eState >= SfxItemState::DEFAULT &&
             pState && dynamic_cast< const SvxLongULSpaceItem *>( pState ) !=  nullptr )
        {
            mpPageULMarginItem.reset( static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
            ChangeMarginImage();
        }
        break;

    case SID_ATTR_PAGE_SIZE:
        if ( mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify )
        {
            mpBindings->Invalidate( SID_ATTR_PAGE, true );
        }
        if ( eState >= SfxItemState::DEFAULT &&
             pState && dynamic_cast< const SvxSizeItem *>( pState ) !=  nullptr )
        {
            mpPageSizeItem.reset( static_cast<SvxSizeItem*>(pState->Clone()) );
        }
        break;
    case SID_ATTR_METRIC:
        MetricState( eState, pState );
        break;
    }
}

void PagePropertyPanel::MetricState( SfxItemState eState, const SfxPoolItem* pState )
{
    meFUnit = FUNIT_NONE;
    if ( pState && eState >= SfxItemState::DEFAULT )
    {
        meFUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pState )->GetValue();
    }
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = nullptr;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    meFUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pItem )->GetValue();
            }
            else
            {
                SAL_WARN("sw.ui", "<PagePropertyPanel::MetricState(..)>: no module found");
            }
        }
    }
}

void PagePropertyPanel::ChangeMarginImage()
{
    if ( mpPageLRMarginItem.get() == nullptr ||
         mpPageULMarginItem.get() == nullptr ||
         mpPageItem.get() == nullptr )
    {
        return;
    }

    const long cTolerance = 5;
    const sal_uInt16 nIdMargin = mpToolBoxMargin->GetItemId(UNO_MARGIN);

    if( std::abs(mpPageLRMarginItem->GetLeft() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        std::abs(mpPageLRMarginItem->GetRight() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetUpper() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetLower() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        mpPageItem->GetPageUsage() != SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgNarrow_L : mImgNarrow );

    else if( std::abs(mpPageLRMarginItem->GetLeft() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        std::abs(mpPageLRMarginItem->GetRight() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetUpper() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetLower() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        mpPageItem->GetPageUsage() != SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgNormal_L : mImgNormal );

    else if( std::abs(mpPageLRMarginItem->GetLeft() - SWPAGE_WIDE_VALUE2) <= cTolerance &&
        std::abs(mpPageLRMarginItem->GetRight() - SWPAGE_WIDE_VALUE2) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetUpper() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetLower() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        mpPageItem->GetPageUsage() != SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgWide_L : mImgWide );

    else if( std::abs(mpPageLRMarginItem->GetLeft() - SWPAGE_WIDE_VALUE3) <= cTolerance &&
        std::abs(mpPageLRMarginItem->GetRight() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetUpper() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        std::abs(mpPageULMarginItem->GetLower() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        mpPageItem->GetPageUsage() == SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgMirrored_L : mImgMirrored );

    else
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgMarginCustom_L : mImgMarginCustom );
}

void PagePropertyPanel::StartUndo()
{
    if ( mxUndoManager.is() )
    {
        mxUndoManager->enterUndoContext( "" );
    }
}

void PagePropertyPanel::EndUndo()
{
    if ( mxUndoManager.is() )
    {
        mxUndoManager->leaveUndoContext();
    }
}

} } // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
