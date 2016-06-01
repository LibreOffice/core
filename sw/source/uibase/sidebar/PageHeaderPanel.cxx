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
    mpBindings( pBindings ),
    maHeaderController(SID_ATTR_PAGE_HEADERSET, *pBindings, *this),
    maHFToggleController(SID_ATTR_PAGE_HEADER, *pBindings, *this),
    mpHeaderItem( new SvxSetItem(SID_ATTR_PAGE_HEADER) )
{
    get(mpHeaderToggle, "headertoggle");
    get(mpHeaderSpacing, "spacingpreset");
    get(mpSameContentLB, "samecontentLB");
    get(mpHeaderMarginPresetBox, "headermarginpreset");

    Initialize();
}

PageHeaderPanel::~PageHeaderPanel()
{
    disposeOnce();
}

void PageHeaderPanel::dispose()
{
    mpHeaderToggle.disposeAndClear();
    mpHeaderSpacing.disposeAndClear();
    mpSameContentLB.disposeAndClear();
    mpHeaderMarginPresetBox.disposeAndClear();

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
        mpHeaderSpacing->Enable();
        mpSameContentLB->Enable();
        mpHeaderMarginPresetBox->Enable();
    }
    else
    {
        mpHeaderSpacing->Disable();
        mpSameContentLB->Disable();
        mpHeaderMarginPresetBox->Disable();
    }
}

void PageHeaderPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    if (IsDisposed())
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_HEADERSET:
        {
        }
        break;
        case SID_ATTR_PAGE_HEADER:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SvxSetItem*>( pState) !=  nullptr )
            {
                const SvxSetItem* pItem = static_cast<const SvxSetItem*>(pState);
                const SfxItemSet rHeaderSet = pItem->GetItemSet();
//                 const SfxBoolItem& rHeaderOn = static_cast<const SfxBoolItem&>(rHeaderSet.Get( SID_ATTR_PAGE_HEADER ));
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED( PageHeaderPanel, HeaderToggleHdl, Button*, void )
{
//     bool IsChecked = mpHeaderToggle->IsChecked();
//     SfxBoolItem aItem(SID_ATTR_PAGE_ON, IsChecked);
//     GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER, SfxCallMode::RECORD, { &aItem } );
//     UpdateControls();
}



} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
