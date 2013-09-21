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

#include "PagePropertyPanel.hxx"
#include "PagePropertyPanel.hrc"

#include "PropertyPanel.hrc"

#include <svx/sidebar/PopupContainer.hxx>
#include "PageOrientationControl.hxx"
#include "PageMarginControl.hxx"
#include "PageSizeControl.hxx"
#include "PageColumnControl.hxx"

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

#include <boost/bind.hpp>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

const char UNO_ORIENTATION[] = ".uno:Orientation";
const char UNO_MARGIN[]      = ".uno:Margin";
const char UNO_SIZE[]        = ".uno:Size";
const char UNO_COLUMN[]      = ".uno:Column";

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace {
    const cssu::Reference< css::document::XUndoManager > getUndoManager( const cssu::Reference< css::frame::XFrame >& rxFrame )
    {
        const cssu::Reference< css::frame::XController >& xController = rxFrame->getController();
        if ( xController.is() )
        {
            const cssu::Reference< css::frame::XModel >& xModel = xController->getModel();
            if ( xModel.is() )
            {
                const cssu::Reference< css::document::XUndoManagerSupplier > xSuppUndo( xModel, cssu::UNO_QUERY_THROW );
                if ( xSuppUndo.is() )
                {
                    const cssu::Reference< css::document::XUndoManager > xUndoManager( xSuppUndo->getUndoManager(), cssu::UNO_QUERY_THROW );
                    return xUndoManager;
                }
            }
        }

        return cssu::Reference< css::document::XUndoManager > ();
    }
}


namespace sw { namespace sidebar {

PagePropertyPanel* PagePropertyPanel::Create (
    Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw ::com::sun::star::lang::IllegalArgumentException(A2S("no parent Window given to PagePropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw ::com::sun::star::lang::IllegalArgumentException(A2S("no XFrame given to PagePropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw ::com::sun::star::lang::IllegalArgumentException(A2S("no SfxBindings given to PagePropertyPanel::Create"), NULL, 2);

    return new PagePropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}

PagePropertyPanel::PagePropertyPanel(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
            SfxBindings* pBindings)
    : PanelLayout(pParent, "PagePropertyPanel", "modules/swriter/ui/sidebarpage.ui", rxFrame)
    , mpBindings(pBindings)

    // image resources
    , maImgSize                 (NULL)
    , maImgSize_L                   (NULL)
    , mImgPortrait              (SW_RES(IMG_PAGE_PORTRAIT))
    , mImgLandscape             (SW_RES(IMG_PAGE_LANDSCAPE))
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
    , mImgA3                        (SW_RES(IMG_PAGE_A3))
    , mImgA4                        (SW_RES(IMG_PAGE_A4))
    , mImgA5                        (SW_RES(IMG_PAGE_A5))
    , mImgB4                        (SW_RES(IMG_PAGE_B4))
    , mImgB5                        (SW_RES(IMG_PAGE_B5))
    , mImgC5                        (SW_RES(IMG_PAGE_C5))
    , mImgLetter                    (SW_RES(IMG_PAGE_LETTER))
    , mImgLegal                 (SW_RES(IMG_PAGE_LEGAL))
    , mImgSizeNone              (SW_RES(IMG_PAGE_SIZE_NONE))
    , mImgA3_L                  (SW_RES(IMG_PAGE_A3_L))
    , mImgA4_L                  (SW_RES(IMG_PAGE_A4_L))
    , mImgA5_L                  (SW_RES(IMG_PAGE_A5_L))
    , mImgB4_L                  (SW_RES(IMG_PAGE_B4_L))
    , mImgB5_L                  (SW_RES(IMG_PAGE_B5_L))
    , mImgC5_L                  (SW_RES(IMG_PAGE_C5_L))
    , mImgLetter_L              (SW_RES(IMG_PAGE_LETTER_L))
    , mImgLegal_L                   (SW_RES(IMG_PAGE_LEGAL_L))
    , mImgSizeNone_L                (SW_RES(IMG_PAGE_SIZE_NONE_L))
    , mImgColumn1                   (SW_RES(IMG_PAGE_COLUMN_1))
    , mImgColumn2                   (SW_RES(IMG_PAGE_COLUMN_2))
    , mImgColumn3                   (SW_RES(IMG_PAGE_COLUMN_3))
    , mImgLeft                  (SW_RES(IMG_PAGE_COLUMN_LEFT))
    , mImgRight                 (SW_RES(IMG_PAGE_COLUMN_RIGHT))
    , mImgColumnNone                (SW_RES(IMG_PAGE_COLUMN_NONE))
    , mImgColumn1_L             (SW_RES(IMG_PAGE_COLUMN_1_L))
    , mImgColumn2_L             (SW_RES(IMG_PAGE_COLUMN_2_L))
    , mImgColumn3_L             (SW_RES(IMG_PAGE_COLUMN_3_L))
    , mImgLeft_L                    (SW_RES(IMG_PAGE_COLUMN_LEFT_L))
    , mImgRight_L                   (SW_RES(IMG_PAGE_COLUMN_RIGHT_L))
    , mImgColumnNone_L          (SW_RES(IMG_PAGE_COLUMN_NONE_L))

    , mpPageItem( new SvxPageItem(SID_ATTR_PAGE) )
    , mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) )
    , mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) )
    , mpPageSizeItem( new SvxSizeItem(SID_ATTR_PAGE_SIZE) )
    , mePaper( PAPER_USER )
    , mpPageColumnTypeItem( new SfxInt16Item(SID_ATTR_PAGE_COLUMN) )

    , meFUnit()
    , meUnit()

    , m_aSwPagePgULControl(SID_ATTR_PAGE_ULSPACE, *pBindings, *this)
    , m_aSwPagePgLRControl(SID_ATTR_PAGE_LRSPACE, *pBindings, *this)
    , m_aSwPagePgSizeControl(SID_ATTR_PAGE_SIZE, *pBindings, *this)
    , m_aSwPagePgControl(SID_ATTR_PAGE, *pBindings, *this)
    , m_aSwPageColControl(SID_ATTR_PAGE_COLUMN, *pBindings, *this)
    , m_aSwPagePgMetricControl(SID_ATTR_METRIC, *pBindings, *this)

    , maOrientationPopup( this,
                          ::boost::bind( &PagePropertyPanel::CreatePageOrientationControl, this, _1 ),
                          A2S("Page orientation") )
    , maMarginPopup( this,
                     ::boost::bind( &PagePropertyPanel::CreatePageMarginControl, this, _1 ),
                     A2S("Page margins") )
    , maSizePopup( this,
                   ::boost::bind( &PagePropertyPanel::CreatePageSizeControl, this, _1 ),
                   A2S("Page size") )
    , maColumnPopup( this,
                     ::boost::bind( &PagePropertyPanel::CreatePageColumnControl, this, _1 ),
                     A2S("Page columns") )

    , mxUndoManager( getUndoManager( rxFrame ) )

    , mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify( false )
{
    // visible controls
    get(mpToolBoxOrientation, "selectorientation");
    get(mpToolBoxMargin, "selectmargin");
    get(mpToolBoxSize, "selectsize");
    get(mpToolBoxColumn, "selectcolumn");

    Initialize();
    mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify = true;
}

PagePropertyPanel::~PagePropertyPanel()
{
    delete[] maImgSize;
    delete[] maImgSize_L;
}

void PagePropertyPanel::Initialize()
{
    // popup for page orientation
    const sal_uInt16 nIdOrientation = mpToolBoxOrientation->GetItemId(UNO_ORIENTATION);
    Link aLink = LINK( this, PagePropertyPanel, ClickOrientationHdl );
    mpToolBoxOrientation->SetDropdownClickHdl( aLink );
    mpToolBoxOrientation->SetSelectHdl( aLink );
    mpToolBoxOrientation->SetItemImage( nIdOrientation, mImgPortrait);
    mpToolBoxOrientation->SetItemBits( nIdOrientation, mpToolBoxOrientation->GetItemBits( nIdOrientation ) | TIB_DROPDOWNONLY );

    // popup for page margins
    const sal_uInt16 nIdMargin = mpToolBoxMargin->GetItemId(UNO_MARGIN);
    aLink = LINK( this, PagePropertyPanel, ClickMarginHdl );
    mpToolBoxMargin->SetDropdownClickHdl( aLink );
    mpToolBoxMargin->SetSelectHdl( aLink );
    mpToolBoxMargin->SetItemImage(nIdMargin, mImgNormal);
    mpToolBoxMargin->SetItemBits( nIdMargin, mpToolBoxMargin->GetItemBits( nIdMargin ) | TIB_DROPDOWNONLY );

    // popup for page size
    const sal_uInt16 nIdSize = mpToolBoxSize->GetItemId(UNO_SIZE);
    aLink = LINK( this, PagePropertyPanel, ClickSizeHdl );
    mpToolBoxSize->SetDropdownClickHdl( aLink );
    mpToolBoxSize->SetSelectHdl( aLink );
    mpToolBoxSize->SetItemImage(nIdSize, mImgLetter);
    mpToolBoxSize->SetItemBits( nIdSize, mpToolBoxSize->GetItemBits( nIdSize ) | TIB_DROPDOWNONLY );
    maImgSize = new Image[8];
    maImgSize[0] = mImgA3;
    maImgSize[1] = mImgA4;
    maImgSize[2] = mImgA5;
    maImgSize[3] = mImgB4;
    maImgSize[4] = mImgB5;
    maImgSize[5] = mImgC5;
    maImgSize[6] = mImgLetter;
    maImgSize[7] = mImgLegal;
    maImgSize_L = new Image[8];
    maImgSize_L[0] = mImgA3_L;
    maImgSize_L[1] = mImgA4_L;
    maImgSize_L[2] = mImgA5_L;
    maImgSize_L[3] = mImgB4_L;
    maImgSize_L[4] = mImgB5_L;
    maImgSize_L[5] = mImgC5_L;
    maImgSize_L[6] = mImgLetter_L;
    maImgSize_L[7] = mImgLegal_L;

    // popup for page column property
    const sal_uInt16 nIdColumn = mpToolBoxColumn->GetItemId(UNO_COLUMN);
    aLink = LINK( this, PagePropertyPanel, ClickColumnHdl );
    mpToolBoxColumn->SetDropdownClickHdl( aLink );
    mpToolBoxColumn->SetSelectHdl( aLink );
    mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn1);
    mpToolBoxColumn->SetItemBits( nIdColumn, mpToolBoxColumn->GetItemBits( nIdColumn ) | TIB_DROPDOWNONLY );

    meFUnit = GetModuleFieldUnit();
    meUnit  = m_aSwPagePgSizeControl.GetCoreMetric();

    // 'pull' for page style's attribute values
    mpBindings->Update( SID_ATTR_PAGE_LRSPACE );
    mpBindings->Update( SID_ATTR_PAGE_ULSPACE );
    mpBindings->Update( SID_ATTR_PAGE );
    mpBindings->Update( SID_ATTR_PAGE_SIZE );
}


::svx::sidebar::PopupControl* PagePropertyPanel::CreatePageOrientationControl( ::svx::sidebar::PopupContainer* pParent )
{
    return new PageOrientationControl( pParent, *this , mpPageItem->IsLandscape() );
}


IMPL_LINK( PagePropertyPanel, ClickOrientationHdl, ToolBox*, pToolBox )
{
    maOrientationPopup.Show( *pToolBox );

    return 0L;
}


void PagePropertyPanel::ExecuteOrientationChange( const sal_Bool bLandscape )
{
    StartUndo();

    {
        // set new page orientation
        mpPageItem->SetLandscape( bLandscape );

        // swap the width and height of the page size
        mpPageSizeItem->SetSize( Size( mpPageSizeItem->GetSize().Height(), mpPageSizeItem->GetSize().Width() ) );

        // apply changed attributes
        GetBindings()->GetDispatcher()->Execute( SID_ATTR_PAGE_SIZE, SFX_CALLMODE_RECORD, mpPageSizeItem.get(), mpPageItem.get(), 0L );
    }

    // check, if margin values still fit to the changed page size.
    // if not, adjust margin values
    {
        const long nML = mpPageLRMarginItem->GetLeft();
        const long nMR = mpPageLRMarginItem->GetRight();
        const long nTmpPW = nML + nMR + MINBODY;

        const long nPW  = mpPageSizeItem->GetSize().Width();

        if ( nTmpPW > nPW )
        {
            if ( nML <= nMR )
            {
                ExecuteMarginLRChange( mpPageLRMarginItem->GetLeft(), nMR - (nTmpPW - nPW ) );
            }
            else
            {
                ExecuteMarginLRChange( nML - (nTmpPW - nPW ), mpPageLRMarginItem->GetRight() );
            }
        }

        const long nMT = mpPageULMarginItem->GetUpper();
        const long nMB = mpPageULMarginItem->GetLower();
        const long nTmpPH = nMT + nMB + MINBODY;

        const long nPH  = mpPageSizeItem->GetSize().Height();

        if ( nTmpPH > nPH )
        {
            if ( nMT <= nMB )
            {
                ExecuteMarginULChange( mpPageULMarginItem->GetUpper(), nMB - ( nTmpPH - nPH ) );
            }
            else
            {
                ExecuteMarginULChange( nMT - ( nTmpPH - nPH ), mpPageULMarginItem->GetLower() );
            }
        }
    }

    EndUndo();
}


void PagePropertyPanel::ClosePageOrientationPopup()
{
    maOrientationPopup.Hide();
}




::svx::sidebar::PopupControl* PagePropertyPanel::CreatePageMarginControl( ::svx::sidebar::PopupContainer* pParent )
{
    return new PageMarginControl(
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
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_PAGE_LRSPACE, SFX_CALLMODE_RECORD, mpPageLRMarginItem.get(),  0L );
}

void PagePropertyPanel::ExecuteMarginULChange(
    const long nPageTopMargin,
    const long nPageBottomMargin )
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_PAGE_ULSPACE, SFX_CALLMODE_RECORD, mpPageULMarginItem.get(),  0L );
}


void PagePropertyPanel::ExecutePageLayoutChange( const bool bMirrored )
{
    mpPageItem->SetPageUsage( bMirrored ? SVX_PAGE_MIRROR : SVX_PAGE_ALL );
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_PAGE, SFX_CALLMODE_RECORD, mpPageItem.get(),  0L );
}


IMPL_LINK( PagePropertyPanel, ClickMarginHdl, ToolBox*, pToolBox )
{
    maMarginPopup.Show( *pToolBox );

    return 0L;
}


void PagePropertyPanel::ClosePageMarginPopup()
{
    maMarginPopup.Hide();
}




::svx::sidebar::PopupControl* PagePropertyPanel::CreatePageSizeControl( ::svx::sidebar::PopupContainer* pParent )
{
    return new PageSizeControl(
        pParent,
        *this,
        mePaper,
        mpPageItem->IsLandscape(),
        meFUnit );
}


void PagePropertyPanel::ExecuteSizeChange( const Paper ePaper )
{
    Size aPageSize = SvxPaperInfo::GetPaperSize( ePaper, (MapUnit)(meUnit) );
    if ( mpPageItem->IsLandscape() )
    {
        Swap( aPageSize );
    }
    mpPageSizeItem->SetSize( aPageSize );

    mpBindings->GetDispatcher()->Execute(SID_ATTR_PAGE_SIZE, SFX_CALLMODE_RECORD, mpPageSizeItem.get(),  0L );
}


IMPL_LINK( PagePropertyPanel, ClickSizeHdl, ToolBox*, pToolBox )
{
    maSizePopup.Show( *pToolBox );

    return 0L;
}


void PagePropertyPanel::ClosePageSizePopup()
{
    maSizePopup.Hide();
}




::svx::sidebar::PopupControl* PagePropertyPanel::CreatePageColumnControl( ::svx::sidebar::PopupContainer* pParent )
{
    return new PageColumnControl(
        pParent,
        *this,
        mpPageColumnTypeItem->GetValue(),
        mpPageItem->IsLandscape() );
}


void PagePropertyPanel::ExecuteColumnChange( const sal_uInt16 nColumnType )
{
    mpPageColumnTypeItem->SetValue( nColumnType );
    mpBindings->GetDispatcher()->Execute(SID_ATTR_PAGE_COLUMN, SFX_CALLMODE_RECORD, mpPageColumnTypeItem.get(),  0L );
}


IMPL_LINK( PagePropertyPanel, ClickColumnHdl, ToolBox*, pToolBox )
{
    maColumnPopup.Show( *pToolBox );

    return 0L;
}


void PagePropertyPanel::ClosePageColumnPopup()
{
    maColumnPopup.Hide();
}




void PagePropertyPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch( nSId )
    {
    case SID_ATTR_PAGE_COLUMN:
        {
            if ( eState >= SFX_ITEM_AVAILABLE &&
                 pState && pState->ISA(SfxInt16Item) )
            {
                mpPageColumnTypeItem.reset( static_cast<SfxInt16Item*>(pState->Clone()) );
                ChangeColumnImage( mpPageColumnTypeItem->GetValue() );
            }
        }
        break;
    case SID_ATTR_PAGE_LRSPACE:
        if ( eState >= SFX_ITEM_AVAILABLE &&
             pState && pState->ISA(SvxLongLRSpaceItem) )
        {
            mpPageLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
            ChangeMarginImage();
        }
        break;

    case SID_ATTR_PAGE_ULSPACE:
        if ( eState >= SFX_ITEM_AVAILABLE &&
             pState && pState->ISA(SvxLongULSpaceItem) )
        {
            mpPageULMarginItem.reset( static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
            ChangeMarginImage();
        }
        break;

    case SID_ATTR_PAGE:
        if ( eState >= SFX_ITEM_AVAILABLE &&
             pState && pState->ISA(SvxPageItem) )
        {
            const sal_uInt16 nIdOrientation = mpToolBoxOrientation->GetItemId(UNO_ORIENTATION);
            mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
            if ( mpPageItem->IsLandscape() )
            {
                mpToolBoxOrientation->SetItemImage(nIdOrientation, mImgLandscape);
            }
            else
            {
                mpToolBoxOrientation->SetItemImage(nIdOrientation, mImgPortrait);
            }
            ChangeMarginImage();
            ChangeSizeImage();
            ChangeColumnImage( mpPageColumnTypeItem->GetValue() );
        }
        break;

    case SID_ATTR_PAGE_SIZE:
        if ( mbInvalidateSIDAttrPageOnSIDAttrPageSizeNotify )
        {
            mpBindings->Invalidate( SID_ATTR_PAGE, sal_True, sal_False );
        }
        if ( eState >= SFX_ITEM_AVAILABLE &&
             pState && pState->ISA(SvxSizeItem) )
        {
            mpPageSizeItem.reset( static_cast<SvxSizeItem*>(pState->Clone()) );
            ChangeSizeImage();
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
    if ( pState && eState >= SFX_ITEM_DEFAULT )
    {
        meFUnit = (FieldUnit)( (const SfxUInt16Item*)pState )->GetValue();
    }
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = NULL;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    meFUnit = (FieldUnit)( (SfxUInt16Item*)pItem )->GetValue();
            }
            else
            {
                DBG_ERRORFILE( "<PagePropertyPanel::MetricState(..)>: no module found" );
            }
        }
    }
}




void PagePropertyPanel::ChangeMarginImage()
{
    if ( mpPageLRMarginItem.get() == 0 ||
         mpPageULMarginItem.get() == 0 ||
         mpPageItem.get() == 0 )
    {
        return;
    }

    const long cTolerance = 5;
    const sal_uInt16 nIdMargin = mpToolBoxMargin->GetItemId(UNO_MARGIN);

    if( abs(mpPageLRMarginItem->GetLeft() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        abs(mpPageLRMarginItem->GetRight() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        abs(mpPageULMarginItem->GetUpper() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        abs(mpPageULMarginItem->GetLower() - SWPAGE_NARROW_VALUE) <= cTolerance &&
        mpPageItem->GetPageUsage() != SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgNarrow_L : mImgNarrow );

    else if( abs(mpPageLRMarginItem->GetLeft() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        abs(mpPageLRMarginItem->GetRight() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        abs(mpPageULMarginItem->GetUpper() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        abs(mpPageULMarginItem->GetLower() - SWPAGE_NORMAL_VALUE) <= cTolerance &&
        mpPageItem->GetPageUsage() != SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgNormal_L : mImgNormal );

    else if( abs(mpPageLRMarginItem->GetLeft() - SWPAGE_WIDE_VALUE2) <= cTolerance &&
        abs(mpPageLRMarginItem->GetRight() - SWPAGE_WIDE_VALUE2) <= cTolerance &&
        abs(mpPageULMarginItem->GetUpper() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        abs(mpPageULMarginItem->GetLower() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        mpPageItem->GetPageUsage() != SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgWide_L : mImgWide );

    else if( abs(mpPageLRMarginItem->GetLeft() - SWPAGE_WIDE_VALUE3) <= cTolerance &&
        abs(mpPageLRMarginItem->GetRight() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        abs(mpPageULMarginItem->GetUpper() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        abs(mpPageULMarginItem->GetLower() - SWPAGE_WIDE_VALUE1) <= cTolerance &&
        mpPageItem->GetPageUsage() == SVX_PAGE_MIRROR )
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgMirrored_L : mImgMirrored );

    else
        mpToolBoxMargin->SetItemImage( nIdMargin, mpPageItem->IsLandscape() ? mImgMarginCustom_L : mImgMarginCustom );
}


void PagePropertyPanel::ChangeSizeImage()
{
    if ( mpPageSizeItem.get() == 0 ||
         mpPageItem.get() == 0 )
    {
        return;
    }

    Size aTmpPaperSize = mpPageSizeItem->GetSize();
    if ( mpPageItem->IsLandscape() )
    {
        Swap( aTmpPaperSize ); // Swap(..) defined in editeng/paperinf.hxx
    }

    mePaper = SvxPaperInfo::GetSvxPaper( aTmpPaperSize, static_cast<MapUnit>(meUnit), sal_True );

    sal_uInt16 nImageIdx = 0;
    switch ( mePaper )
    {
    case PAPER_A3:
        nImageIdx = 1;
        break;
    case PAPER_A4:
        nImageIdx = 2;
        break;
    case PAPER_A5:
        nImageIdx = 3;
        break;
    case PAPER_B4_ISO:
        nImageIdx = 4;
        break;
    case PAPER_B5_ISO:
        nImageIdx = 5;
        break;
    case PAPER_ENV_C5:
        nImageIdx = 6;
        break;
    case PAPER_LETTER:
        nImageIdx = 7;
        break;
    case PAPER_LEGAL:
        nImageIdx = 8;
        break;
    default:
        nImageIdx = 0;
        mePaper = PAPER_USER;
        break;
    }

    const sal_uInt16 nIdSize = mpToolBoxSize->GetItemId(UNO_SIZE);

    if ( nImageIdx == 0 )
    {
        mpToolBoxSize->SetItemImage( nIdSize,
                                     ( mpPageItem->IsLandscape() ? mImgSizeNone_L : mImgSizeNone  ) );
    }
    else
    {
        mpToolBoxSize->SetItemImage( nIdSize,
                                     ( mpPageItem->IsLandscape() ? maImgSize_L[nImageIdx-1] : maImgSize[nImageIdx-1] ) );
    }
}


void PagePropertyPanel::ChangeColumnImage( const sal_uInt16 nColumnType )
{
    if ( mpPageItem.get() == 0 )
    {
        return;
    }

    const sal_uInt16 nIdColumn = mpToolBoxColumn->GetItemId(UNO_COLUMN);
    if ( !mpPageItem->IsLandscape() )
    {
        switch( nColumnType )
        {
        case 1:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn1);
            break;
        case 2:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn2);
            break;
        case 3:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn3);
            break;
        case 4:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgLeft);
            break;
        case 5:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgRight);
            break;
        default:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumnNone);
        }
    }
    else
    {
        switch( nColumnType )
        {
        case 1:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn1_L);
            break;
        case 2:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn2_L);
            break;
        case 3:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumn3_L);
            break;
        case 4:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgLeft_L);
            break;
        case 5:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgRight_L);
            break;
        default:
            mpToolBoxColumn->SetItemImage(nIdColumn, mImgColumnNone_L);
        }
    }
}


void PagePropertyPanel::StartUndo()
{
    if ( mxUndoManager.is() )
    {
        mxUndoManager->enterUndoContext( A2S("") );
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
