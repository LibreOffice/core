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
#include <swtypes.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include "PageMarginPanel.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include "cmdid.h"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

namespace sw { namespace sidebar{

VclPtr<vcl::Window> PageMarginPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( !pParent )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageMarginPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageMarginPanel::Create", nullptr, 0);
    if( !pBindings )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageMarginPanel::Create", nullptr, 0);

    return VclPtr<PageMarginPanel>::Create(pParent, rxFrame, pBindings);
}

PageMarginPanel::PageMarginPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, "PageMarginPanel" , "modules/swriter/ui/pagemarginpanel.ui", rxFrame),
    mpBindings(pBindings),
    meFUnit(),
    meUnit(),
    mnPageLeftMargin(0),
    mnPageRightMargin(0),
    mnPageTopMargin(0),
    mnPageBottomMargin(0),
    mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) ),
    mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) ),
    maSwPageLRControl(SID_ATTR_PAGE_LRSPACE, *pBindings, *this),
    maSwPageULControl(SID_ATTR_PAGE_ULSPACE, *pBindings, *this),
    maSwPageSizeControl(SID_ATTR_PAGE_SIZE, *pBindings, *this),
    maSwPageMetricControl(SID_ATTR_METRIC, *pBindings, *this)
{
    get(mpLeftMarginEdit, "leftmargin");
    get(mpRightMarginEdit, "rightmargin");
    get(mpTopMarginEdit, "topmargin");
    get(mpBottomMarginEdit, "bottommargin");

    Initialize();
}

PageMarginPanel::~PageMarginPanel()
{
    disposeOnce();
}

void PageMarginPanel::dispose()
{

    mpLeftMarginEdit.disposeAndClear();
    mpRightMarginEdit.disposeAndClear();
    mpTopMarginEdit.disposeAndClear();
    mpBottomMarginEdit.disposeAndClear();

    mpPageLRMarginItem.reset();
    mpPageULMarginItem.reset();

    maSwPageULControl.dispose();
    maSwPageLRControl.dispose();
    maSwPageSizeControl.dispose();
    maSwPageMetricControl.dispose();

    PanelLayout::dispose();
}

void PageMarginPanel::Initialize()
{
    meUnit = maSwPageSizeControl.GetCoreMetric();
    mpLeftMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyLRMarginHdl) );
    mpRightMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyLRMarginHdl) );
    mpTopMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyULMarginHdl) );
    mpBottomMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyULMarginHdl) );
}

void PageMarginPanel::MetricState( SfxItemState eState, const SfxPoolItem* pState )
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
                SAL_WARN("sw.ui", "<PageMarginPanel::MetricState(..)>: no module found");
            }
        }
    }
/*
    SetFieldUnit( *mpLeftMarginEdit.get(), meFUnit );
    SetFieldUnit( *mpRightMarginEdit.get(), meFUnit );
    SetFieldUnit( *mpTopMarginEdit.get(), meFUnit );
    SetFieldUnit( *mpBottomMarginEdit.get(), meFUnit );
*/
}

void PageMarginPanel::ExecuteMarginLRChange(
    const long nPageLeftMargin,
    const long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE, SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PageMarginPanel::ExecuteMarginULChange(
    const long nPageTopMargin,
    const long nPageBottomMargin )
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_ULSPACE, SfxCallMode::RECORD, { mpPageULMarginItem.get() });
}

void PageMarginPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    switch( nSId )
    {
        case SID_ATTR_PAGE_LRSPACE:
        {
        }
        break;
        case SID_ATTR_PAGE_ULSPACE:
        {
        }
        break;
        case SID_ATTR_METRIC:
            MetricState(eState, pState);
        break;
    }
}



IMPL_LINK_NOARG_TYPED( PageMarginPanel, ModifyLRMarginHdl, Edit&, void )
{
    mnPageLeftMargin = GetCoreValue( *mpLeftMarginEdit.get(), meUnit );
    mnPageRightMargin = GetCoreValue( *mpRightMarginEdit.get(), meUnit );
    ExecuteMarginLRChange( mnPageLeftMargin, mnPageRightMargin );
}

IMPL_LINK_NOARG_TYPED( PageMarginPanel, ModifyULMarginHdl, Edit&, void )
{
    mnPageTopMargin = GetCoreValue( *mpTopMarginEdit.get(), meUnit );
    mnPageBottomMargin = GetCoreValue( *mpBottomMarginEdit.get(), meUnit );
    ExecuteMarginULChange( mnPageTopMargin, mnPageBottomMargin );
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
