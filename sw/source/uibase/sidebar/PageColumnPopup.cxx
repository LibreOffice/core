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
#include <PageColumnPopup.hxx>
#include "PageColumnControl.hxx"
#include <vcl/toolbox.hxx>

PageColumnPopup::PageColumnPopup(const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

void PageColumnPopup::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (getToolboxId(nId, &pToolBox))
        pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWNONLY | pToolBox->GetItemBits(nId));
}

PageColumnPopup::~PageColumnPopup()
{
}

std::unique_ptr<WeldToolbarPopup> PageColumnPopup::weldPopupWindow()
{
    return std::make_unique<sw::sidebar::PageColumnControl>(this, m_pToolbar);
}

VclPtr<vcl::Window> PageColumnPopup::createVclPopupWindow( vcl::Window* pParent )
{
    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::make_unique<sw::sidebar::PageColumnControl>(this, pParent->GetFrameWeld()));

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString PageColumnPopup::getImplementationName()
{
    return "lo.writer.PageColumnToolBoxControl";
}

css::uno::Sequence<OUString> PageColumnPopup::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
lo_writer_PageColumnToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire(new PageColumnPopup(rContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
