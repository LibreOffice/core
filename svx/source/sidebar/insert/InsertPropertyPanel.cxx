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
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/XStatusListener.hpp>

using namespace css;
using namespace cssu;
using ::rtl::OUString;
using ::sfx2::sidebar::SidebarToolBox;

namespace svx { namespace sidebar {


InsertPropertyPanel::InsertPropertyPanel (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame)
    :   PanelLayout(pParent, "InsertPropertyPanel", "svx/ui/sidebarinsert.ui", rxFrame),
        mxFrame(rxFrame)
{
    get(mpStandardShapesToolBox, "standardshapes");
    get(mpCustomShapesToolBox,   "customshapes");

    mpStandardShapesToolBox->Show();
    mpCustomShapesToolBox->Show();

    // Listen to all tool box selection events.
    Window* pTopWindow = pParent;
    while (pTopWindow->GetParent() != NULL)
        pTopWindow = pTopWindow->GetParent();
    pTopWindow->AddChildEventListener(LINK(this, InsertPropertyPanel, WindowEventListener));
}




InsertPropertyPanel::~InsertPropertyPanel (void)
{
    // Remove window child listener.
    Window* pTopWindow = this;
    while (pTopWindow->GetParent() != NULL)
        pTopWindow = pTopWindow->GetParent();
    pTopWindow->RemoveChildEventListener(LINK(this, InsertPropertyPanel, WindowEventListener));
}




IMPL_LINK(InsertPropertyPanel, WindowEventListener, VclSimpleEvent*, pEvent)
{
    // We will be getting a lot of window events (well, basically all
    // of them), so reject early everything that is not connected to
    // toolbox selection.
    if (pEvent == NULL)
        return 1;
    if ( ! pEvent->ISA(VclWindowEvent))
        return 1;
    if (pEvent->GetId() != VCLEVENT_TOOLBOX_SELECT)
        return 1;

    Window* pWindow = dynamic_cast<VclWindowEvent*>(pEvent)->GetWindow();
    ToolBox* pToolBox = dynamic_cast<ToolBox*>(pWindow);
    if (pToolBox == NULL)
        return 1;

    // Extract name of (sub)toolbar from help id.
    OUString sToolbarName (rtl::OStringToOUString(pToolBox->GetHelpId(), RTL_TEXTENCODING_UTF8));
    if (sToolbarName.getLength() == 0)
        return 1;
    const util::URL aURL (sfx2::sidebar::Tools::GetURL(sToolbarName));
    if (aURL.Path.getLength() == 0)
        return 1;

    // Get item id.
    sal_uInt16 nId = pToolBox->GetCurItemId();
    if (nId == 0)
        return 1;

    SidebarToolBox* pSidebarToolBox = dynamic_cast<SidebarToolBox*>(mpStandardShapesToolBox);
    if (pSidebarToolBox == NULL)
        return 1;
    sal_uInt16 nItemId (pSidebarToolBox->GetItemIdForSubToolbarName(aURL.Path));
    if (nItemId == 0)
    {
        pSidebarToolBox = dynamic_cast<SidebarToolBox*>(mpCustomShapesToolBox);
        if (pSidebarToolBox == NULL)
            return 1;
        nItemId = pSidebarToolBox->GetItemIdForSubToolbarName(aURL.Path);
        if (nItemId == 0)
            return 1;
    }

    Reference<frame::XSubToolbarController> xController (pSidebarToolBox->GetControllerForItemId(nItemId), UNO_QUERY);
    if ( ! xController.is() )
        return 1;

    const OUString sCommand (pToolBox->GetItemCommand(nId));
    xController->functionSelected(sCommand);

    return 1;
}


} } // end of namespace svx::sidebar
