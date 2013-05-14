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
#include "InsertPropertyPanel.hrc"
#include "sfx2/sidebar/CommandInfoProvider.hxx"

#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>

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


namespace svx { namespace sidebar {


InsertPropertyPanel::InsertPropertyPanel (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame)
    :   Control(pParent, SVX_RES(RID_SIDEBAR_INSERT_PANEL)),
        mpStandardShapesBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
        mpStandardShapesToolBox(sfx2::sidebar::ControlFactory::CreateToolBox(
                mpStandardShapesBackground.get(),
                SVX_RES(TB_INSERT_STANDARD))),
        mpCustomShapesBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
        mpCustomShapesToolBox(sfx2::sidebar::ControlFactory::CreateToolBox(
                mpCustomShapesBackground.get(),
                SVX_RES(TB_INSERT_CUSTOM))),
        maControllers(),
        mxFrame(rxFrame)
{
    SetupToolBox(*mpStandardShapesToolBox);
    SetupToolBox(*mpCustomShapesToolBox);
    FreeResource();

    UpdateIcons();

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
    ControllerContainer aControllers;
    aControllers.swap(maControllers);
    for (ControllerContainer::iterator iController(aControllers.begin()), iEnd(aControllers.end());
         iController!=iEnd;
         ++iController)
    {
        Reference<lang::XComponent> xComponent (iController->second.mxController, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }

    // Remove window child listener.
    Window* pTopWindow = this;
    while (pTopWindow->GetParent() != NULL)
        pTopWindow = pTopWindow->GetParent();
    pTopWindow->RemoveChildEventListener(LINK(this, InsertPropertyPanel, WindowEventListener));

    mpStandardShapesToolBox.reset();
    mpCustomShapesToolBox.reset();
    mpStandardShapesBackground.reset();
    mpCustomShapesBackground.reset();
}




void InsertPropertyPanel::SetupToolBox (ToolBox& rToolBox)
{
    const sal_uInt16 nItemCount (rToolBox.GetItemCount());
    for (sal_uInt16 nItemIndex=0; nItemIndex<nItemCount; ++nItemIndex)
        CreateController(rToolBox.GetItemId(nItemIndex));

    rToolBox.SetDropdownClickHdl(LINK(this, InsertPropertyPanel, DropDownClickHandler));
    rToolBox.SetClickHdl(LINK(this, InsertPropertyPanel, ClickHandler));
    rToolBox.SetDoubleClickHdl(LINK(this, InsertPropertyPanel, DoubleClickHandler));
    rToolBox.SetSelectHdl(LINK(this, InsertPropertyPanel, SelectHandler));
    rToolBox.SetActivateHdl(LINK(this, InsertPropertyPanel, ActivateToolBox));
    rToolBox.SetDeactivateHdl(LINK(this, InsertPropertyPanel, DeactivateToolBox));

    rToolBox.SetSizePixel(rToolBox.CalcWindowSizePixel());
}




IMPL_LINK(InsertPropertyPanel, DropDownClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox != NULL)
    {
        Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
        if (xController.is())
        {
            Reference<awt::XWindow> xWindow = xController->createPopupWindow();
            if (xWindow.is() )
                xWindow->setFocus();
        }
    }
    return 1;
}




IMPL_LINK(InsertPropertyPanel, ClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->click();

    return 1;
}




IMPL_LINK(InsertPropertyPanel, DoubleClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->doubleClick();

    return 1;
}




IMPL_LINK(InsertPropertyPanel, SelectHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->execute((sal_Int16)pToolBox->GetModifier());

    return 1;
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

    ToolBox* pToolBox = dynamic_cast<ToolBox*>(dynamic_cast<VclWindowEvent*>(pEvent)->GetWindow());
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

    // Get toolbar controller.
    const sal_uInt16 nItemId (GetItemIdForSubToolbarName(aURL.Path));
    Reference<frame::XSubToolbarController> xController (GetControllerForItemId(nItemId), UNO_QUERY);
    if ( ! xController.is())
        return 1;

    const OUString sCommand (pToolBox->GetItemCommand(nId));
    ControllerContainer::iterator iController (maControllers.find(nItemId));
    if (iController != maControllers.end())
        iController->second.msCurrentCommand = sCommand;
    xController->functionSelected(sCommand);

    const sal_Bool bBigImages (SvtMiscOptions().AreCurrentSymbolsLarge());
    Image aImage (framework::GetImageFromURL(mxFrame, sCommand, bBigImages));
    pToolBox->SetItemImage(iController->first, aImage);

    return 1;
}




IMPL_LINK(InsertPropertyPanel, ActivateToolBox, ToolBox*, EMPTYARG)
{
    return 1;
}




IMPL_LINK(InsertPropertyPanel, DeactivateToolBox, ToolBox*, EMPTYARG)
{
    return 1;
}




void InsertPropertyPanel::CreateController (
    const sal_uInt16 nItemId)
{
    ToolBox* pToolBox = GetToolBoxForItemId(nItemId);
    if (pToolBox != NULL)
    {
        ItemDescriptor aDescriptor;

        const OUString sCommandName (pToolBox->GetItemCommand(nItemId));

        // Create a controller for the new item.
        aDescriptor.mxController.set(
            static_cast<XWeak*>(::framework::CreateToolBoxController(
                    mxFrame,
                    pToolBox,
                    nItemId,
                    sCommandName)),
            UNO_QUERY);
        if ( ! aDescriptor.mxController.is())
            aDescriptor.mxController.set(
                static_cast<XWeak*>(new svt::GenericToolboxController(
                        ::comphelper::getProcessComponentContext(),
                        mxFrame,
                        pToolBox,
                        nItemId,
                        sCommandName)),
                UNO_QUERY);
        if ( ! aDescriptor.mxController.is())
            return;

        // Get dispatch object for the command.
        aDescriptor.maURL = sfx2::sidebar::Tools::GetURL(sCommandName);
        aDescriptor.msCurrentCommand = sCommandName;
        aDescriptor.mxDispatch = sfx2::sidebar::Tools::GetDispatch(mxFrame, aDescriptor.maURL);
        if ( ! aDescriptor.mxDispatch.is())
            return;

        // Initialize the controller with eg a service factory.
        Reference<lang::XInitialization> xInitialization (aDescriptor.mxController, UNO_QUERY);
        if (xInitialization.is())
        {
            beans::PropertyValue aPropValue;
            std::vector<Any> aPropertyVector;

            aPropValue.Name = A2S("Frame");
            aPropValue.Value <<= mxFrame;
            aPropertyVector.push_back(makeAny(aPropValue));

            aPropValue.Name = A2S("ServiceManager");
            aPropValue.Value <<= ::comphelper::getProcessServiceFactory();
            aPropertyVector.push_back(makeAny(aPropValue));

            aPropValue.Name = A2S("CommandURL");
            aPropValue.Value <<= sCommandName;
            aPropertyVector.push_back(makeAny(aPropValue));

            Sequence<Any> aArgs (comphelper::containerToSequence(aPropertyVector));
            xInitialization->initialize(aArgs);
        }

        Reference<util::XUpdatable> xUpdatable (aDescriptor.mxController, UNO_QUERY);
        if (xUpdatable.is())
            xUpdatable->update();

        // Add label.
        const OUString sLabel (sfx2::sidebar::CommandInfoProvider::Instance().GetLabelForCommand(
                sCommandName,
                mxFrame));
        pToolBox->SetQuickHelpText(nItemId, sLabel);

        // Add item to toolbox.
        pToolBox->EnableItem(nItemId);
        maControllers.insert(::std::make_pair(nItemId, aDescriptor));
    }
}




ToolBox* InsertPropertyPanel::GetToolBoxForItemId (const sal_uInt16 nItemId) const
{
    switch(nItemId)
    {
        case TBI_STANDARD_LINE:
        case TBI_STANDARD_ARROW:
        case TBI_STANDARD_RECTANGLE:
        case TBI_STANDARD_ELLIPSE:
        case TBI_STANDARD_TEXT:
        case TBI_STANDARD_LINES:
        case TBI_STANDARD_CONNECTORS:
        case TBI_STANDARD_ARROWS:
            return mpStandardShapesToolBox.get();

        case TBI_CUSTOM_BASICS:
        case TBI_CUSTOM_SYMBOLS:
        case TBI_CUSTOM_ARROWS:
        case TBI_CUSTOM_FLOWCHARTS:
        case TBI_CUSTOM_CALLOUTS:
        case TBI_CUSTOM_STARS:
            return mpCustomShapesToolBox.get();

        default:
            return NULL;
    }
}




Reference<frame::XToolbarController> InsertPropertyPanel::GetControllerForItemId (const sal_uInt16 nItemId) const
{
    ControllerContainer::const_iterator iController (maControllers.find(nItemId));
    if (iController != maControllers.end())
        return iController->second.mxController;
    else
        return NULL;
}




sal_uInt16 InsertPropertyPanel::GetItemIdForSubToolbarName (const OUString& rsSubToolbarName) const
{
    for (ControllerContainer::const_iterator iController(maControllers.begin()), iEnd(maControllers.end());
         iController!=iEnd;
         ++iController)
    {
        Reference<frame::XSubToolbarController> xSubToolbarController (iController->second.mxController, UNO_QUERY);
        if (xSubToolbarController.is())
            if (xSubToolbarController->getSubToolbarName().equals(rsSubToolbarName))
                return iController->first;
    }
    return 0;
}




void InsertPropertyPanel::UpdateIcons (void)
{
    const sal_Bool bBigImages (SvtMiscOptions().AreCurrentSymbolsLarge());

    for (ControllerContainer::iterator iController(maControllers.begin()), iEnd(maControllers.end());
         iController!=iEnd;
         ++iController)
    {
        const ::rtl::OUString sCommandURL (iController->second.msCurrentCommand);
        Image aImage (framework::GetImageFromURL(mxFrame, sCommandURL, bBigImages));
        ToolBox* pToolBox = GetToolBoxForItemId(iController->first);
        if (pToolBox != NULL)
            pToolBox->SetItemImage(iController->first, aImage);
    }
}




} } // end of namespace svx::sidebar
