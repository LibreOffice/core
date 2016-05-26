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
#include <svl/eitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include "PageHeaderPanel.hxx"
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

VclPtr<vcl::Window> PageHeaderPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageHeaderPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageHeaderPanel::Create", nullptr, 0);
    if( pBindings == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageHeaderPanel::Create", nullptr, 0);

    return VclPtr<PageHeaderPanel>::Create(pParent, rxFrame, pBindings);
}

PageHeaderPanel::PageHeaderPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, "PageHeaderPanel", "modules/swriter/ui/pageheaderpanel.ui", rxFrame),
    mpBindings( pBindings )
{
    get(mpHeaderToggle, "headertoggle");
    get(mpHeaderHeightField, "heightspinfield");
    get(mpHeaderLMargin, "leftmargin");
    get(mpHeaderRMargin, "rightmargin");
    get(mpHeaderSpacing, "spacingspinfield");
    get(mpSameContentLB, "samecontentLB");

    Initialize();
}

PageHeaderPanel::~PageHeaderPanel()
{
    disposeOnce();
}

void PageHeaderPanel::dispose()
{
    mpHeaderToggle.disposeAndClear();
    mpHeaderHeightField.disposeAndClear();
    mpHeaderLMargin.disposeAndClear();
    mpHeaderRMargin.disposeAndClear();
    mpHeaderSpacing.disposeAndClear();
    mpSameContentLB.disposeAndClear();

    PanelLayout::dispose();
}

void PageHeaderPanel::Initialize()
{
    mpHeaderToggle->SetClickHdl( LINK(this, PageHeaderPanel, HeaderToggleHdl) );
}

void PageHeaderPanel::UpdateControls()
{
    bool bIsEnabled = (bool)mpHeaderToggle->IsChecked();
    if(bIsEnabled)
    {
        mpHeaderHeightField->Enable();
        mpHeaderLMargin->Enable();
        mpHeaderRMargin->Enable();
        mpHeaderSpacing->Enable();
        mpSameContentLB->Enable();
    }
    else
    {
        mpHeaderHeightField->Disable();
        mpHeaderLMargin->Disable();
        mpHeaderRMargin->Disable();
        mpHeaderSpacing->Disable();
        mpSameContentLB->Disable();
    }
}

void PageHeaderPanel::NotifyItemUpdate(
    const sal_uInt16 /*nSid*/,
    const SfxItemState /*eState*/,
    const SfxPoolItem* /*pState*/,
    const bool /*bIsEnabled*/)
{
}

IMPL_LINK_NOARG_TYPED( PageHeaderPanel, HeaderToggleHdl, Button*, void )
{
    bool IsChecked = mpHeaderToggle->IsChecked();
    SfxBoolItem aItem(SID_ATTR_PAGE_ON, IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADERSET, SfxCallMode::RECORD, { &aItem } );
    UpdateControls();
}



} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
