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
#include "InsertPropertyPanel.hxx"
#include "sfx2/sidebar/CommandInfoProvider.hxx"

#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/ControllerFactory.hxx>

#include <svx/dialmgr.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/generictoolboxcontroller.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/tbxctrl.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <framework/imageproducer.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/XStatusListener.hpp>

using namespace css;
using namespace css::uno;
using ::rtl::OUString;
using ::sfx2::sidebar::SidebarToolBox;

namespace svx { namespace sidebar {


InsertPropertyPanel::InsertPropertyPanel (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
    :   PanelLayout(pParent, "InsertPropertyPanel", "svx/ui/sidebarinsert.ui", rxFrame),
        mxFrame(rxFrame)
{
    get(mpStandardShapesToolBox, "standardshapes");
    get(mpCustomShapesToolBox,   "customshapes");

    mpStandardShapesToolBox->Show();
    mpCustomShapesToolBox->Show();

    // Listen to all tool box selection events.
    // FIXME: This is an incredibly ugly hack that we should kill at some
    // stage.  It is needed because the mpCustomShapesToolBox somehow does not
    // get the right controller, and so the images there are not updated when
    // the user selects eg. a callout.  But using the help id's to get/update
    // it (that is what functionSelected() does) is not the way to go in
    // general ;-)
    // In other words, we should find the underlying problem, and remove the
    // WindowEventListener for good.
    vcl::Window* pTopWindow = pParent;
    while (pTopWindow->GetParent() != NULL)
        pTopWindow = pTopWindow->GetParent();
    pTopWindow->AddChildEventListener(LINK(this, InsertPropertyPanel, WindowEventListener));
}

InsertPropertyPanel::~InsertPropertyPanel()
{
    disposeOnce();
}

void InsertPropertyPanel::dispose()
{
    // Remove window child listener.
    vcl::Window* pTopWindow = this;
    while (pTopWindow->GetParent() != NULL)
        pTopWindow = pTopWindow->GetParent();
    pTopWindow->RemoveChildEventListener(LINK(this, InsertPropertyPanel, WindowEventListener));
    mpStandardShapesToolBox.clear();
    mpCustomShapesToolBox.clear();
    PanelLayout::dispose();
}




IMPL_LINK_TYPED(InsertPropertyPanel, WindowEventListener, VclWindowEvent&, rEvent, void)
{
    // We will be getting a lot of window events (well, basically all
    // of them), so reject early everything that is not connected to
    // toolbox selection.
    if (rEvent.GetId() != VCLEVENT_TOOLBOX_SELECT)
        return;

    vcl::Window* pWindow = rEvent.GetWindow();
    ToolBox* pToolBox = dynamic_cast<ToolBox*>(pWindow);
    if (pToolBox == NULL)
        return;

    // Extract name of (sub)toolbar from help id.
    OUString sToolbarName (rtl::OStringToOUString(pToolBox->GetHelpId(), RTL_TEXTENCODING_UTF8));
    if (sToolbarName.getLength() == 0)
        return;
    const util::URL aURL (sfx2::sidebar::Tools::GetURL(sToolbarName));
    if (aURL.Path.getLength() == 0)
        return;

    // Get item id.
    sal_uInt16 nId = pToolBox->GetCurItemId();
    if (nId == 0)
        return;

    SidebarToolBox* pSidebarToolBox = dynamic_cast<SidebarToolBox*>(mpStandardShapesToolBox.get());
    if (pSidebarToolBox == NULL)
        return;
    sal_uInt16 nItemId (pSidebarToolBox->GetItemIdForSubToolbarName(aURL.Path));
    if (nItemId == 0)
    {
        pSidebarToolBox = dynamic_cast<SidebarToolBox*>(mpCustomShapesToolBox.get());
        if (pSidebarToolBox == NULL)
            return;
        nItemId = pSidebarToolBox->GetItemIdForSubToolbarName(aURL.Path);
        if (nItemId == 0)
            return;
    }

    Reference<frame::XSubToolbarController> xController (pSidebarToolBox->GetControllerForItemId(nItemId), UNO_QUERY);
    if ( ! xController.is() )
        return;

    const OUString sCommand (pToolBox->GetItemCommand(nId));
    xController->functionSelected(sCommand);
}


} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
