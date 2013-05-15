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

#include "sfx2/sidebar/SidebarToolBox.hxx"
#include "ToolBoxBackground.hxx"
#include "sfx2/sidebar/ControllerFactory.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sidebar/Tools.hxx"

#include <vcl/gradient.hxx>
#include <svtools/miscopt.hxx>
#include <framework/imageproducer.hxx>
#include <com/sun/star/frame/XSubToolbarController.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;


namespace sfx2 { namespace sidebar {


SidebarToolBox::SidebarToolBox (
    Window* pParentWindow,
    const ResId& rResId,
    const cssu::Reference<css::frame::XFrame>& rxFrame)
    : ToolBox(pParentWindow, rResId),
      mbParentIsBorder(false),
      maItemSeparator(Theme::GetImage(Theme::Image_ToolBoxItemSeparator))
{
    SetBackground(Wallpaper());
    SetPaintTransparent(true);

    if (rxFrame.is())
    {
        const sal_uInt16 nItemCount (GetItemCount());
        for (sal_uInt16 nItemIndex=0; nItemIndex<nItemCount; ++nItemIndex)
            CreateController(GetItemId(nItemIndex), rxFrame);
        UpdateIcons(rxFrame);

        SetSizePixel(CalcWindowSizePixel());

        SetDropdownClickHdl(LINK(this, SidebarToolBox, DropDownClickHandler));
        SetClickHdl(LINK(this, SidebarToolBox, ClickHandler));
        SetDoubleClickHdl(LINK(this, SidebarToolBox, DoubleClickHandler));
        SetSelectHdl(LINK(this, SidebarToolBox, SelectHandler));
        SetActivateHdl(LINK(this, SidebarToolBox, ActivateToolBox));
        SetDeactivateHdl(LINK(this, SidebarToolBox, DeactivateToolBox));
    }

#ifdef DEBUG
    SetText(A2S("SidebarToolBox"));
#endif
}




SidebarToolBox::~SidebarToolBox (void)
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

    SetDropdownClickHdl(Link());
    SetClickHdl(Link());
    SetDoubleClickHdl(Link());
    SetSelectHdl(Link());
    SetActivateHdl(Link());
    SetDeactivateHdl(Link());

}




void SidebarToolBox::SetBorderWindow (const Window* pBorderWindow)
{
    if (pBorderWindow != GetParent())
    {
        OSL_ASSERT("SetBorderWindow can only handle parent as border window");
        return;
    }

    if ( ! mbParentIsBorder)
    {
        mbParentIsBorder = true;

        setPosSizePixel (
            GetPosPixel().X(),
            GetPosPixel().Y(),
            GetSizePixel().Width(),
            GetSizePixel().Height(),
            WINDOW_POSSIZE_ALL);
    }
}




void SidebarToolBox::Paint (const Rectangle& rRect)
{
    ToolBox::Paint(rRect);

    if (Theme::GetBoolean(Theme::Bool_UseToolBoxItemSeparator))
    {
        const sal_Int32 nSeparatorY ((GetSizePixel().Height() - maItemSeparator.GetSizePixel().Height())/2);
        const sal_uInt16 nItemCount (GetItemCount());
        int nLastRight (-1);
        for (sal_uInt16 nIndex=0; nIndex<nItemCount; ++nIndex)
        {
            const Rectangle aItemBoundingBox (GetItemPosRect(nIndex));
            if (nLastRight >= 0)
            {
                const int nSeparatorX ((nLastRight + aItemBoundingBox.Left() - 1) / 2);
                DrawImage(Point(nSeparatorX,nSeparatorY), maItemSeparator);
            }

            nLastRight = aItemBoundingBox.Right();
        }
    }
}




Point SidebarToolBox::GetPosPixel (void) const
{
    if (mbParentIsBorder)
    {
        const Point aParentPoint (GetParent()->GetPosPixel());
        const Point aChildPoint (ToolBox::GetPosPixel());
        return Point(
            aParentPoint.X() + aChildPoint.X(),
            aParentPoint.Y() + aChildPoint.Y());
    }
    else
        return ToolBox::GetPosPixel();
}




void SidebarToolBox::setPosSizePixel (
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    sal_uInt16 nFlags)
{
    if (mbParentIsBorder)
    {
        const Point aRelativePosition (static_cast<ToolBoxBackground*>(GetParent())->SetToolBoxChild(
                this,
                nX,
                nY,
                nWidth,
                nHeight,
                nFlags));
        ToolBox::setPosSizePixel(
            aRelativePosition.X(),
            aRelativePosition.Y(),
            nWidth,
            nHeight,
            nFlags);
    }
    else
        ToolBox::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);
}




long SidebarToolBox::Notify (NotifyEvent& rEvent)
{
    if (rEvent.GetType() == EVENT_KEYINPUT)
    {
        if (rEvent.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB)
        {
            // Special handling for transferring handling of KEY_TAB
            // that becomes necessary because of our parent that is
            // not the dialog but a background control.
            return DockingWindow::Notify(rEvent);
        }
    }
    return ToolBox::Notify(rEvent);
}




void SidebarToolBox::CreateController (
    const sal_uInt16 nItemId,
    const cssu::Reference<css::frame::XFrame>& rxFrame)
{
    ItemDescriptor aDescriptor;

    const OUString sCommandName (GetItemCommand(nItemId));

    aDescriptor.mxController = sfx2::sidebar::ControllerFactory::CreateToolBoxController(
        this,
        nItemId,
        sCommandName,
        rxFrame);
    aDescriptor.maURL = sfx2::sidebar::Tools::GetURL(sCommandName);
    aDescriptor.msCurrentCommand = sCommandName;
    aDescriptor.mxDispatch = sfx2::sidebar::Tools::GetDispatch(rxFrame, aDescriptor.maURL);

    if (aDescriptor.mxController.is() && aDescriptor.mxDispatch.is())
        maControllers.insert(::std::make_pair(nItemId, aDescriptor));
}




Reference<frame::XToolbarController> SidebarToolBox::GetControllerForItemId (const sal_uInt16 nItemId) const
{
    ControllerContainer::const_iterator iController (maControllers.find(nItemId));
    if (iController != maControllers.end())
        return iController->second.mxController;
    else
        return NULL;
}




void SidebarToolBox::UpdateIcons (const Reference<frame::XFrame>& rxFrame)
{
    const sal_Bool bBigImages (SvtMiscOptions().AreCurrentSymbolsLarge());

    for (ControllerContainer::iterator iController(maControllers.begin()), iEnd(maControllers.end());
         iController!=iEnd;
         ++iController)
    {
        const ::rtl::OUString sCommandURL (iController->second.msCurrentCommand);
        Image aImage (framework::GetImageFromURL(rxFrame, sCommandURL, bBigImages));
        SetItemImage(iController->first, aImage);
    }
}




sal_uInt16 SidebarToolBox::GetItemIdForSubToolbarName (const OUString& rsSubToolbarName) const
{
    for (ControllerContainer::const_iterator iController(maControllers.begin()), iEnd(maControllers.end());
         iController!=iEnd;
         ++iController)
    {
        Reference<frame::XToolbarController> xController (iController->second.mxController);
        Reference<frame::XSubToolbarController> xSubToolbarController (xController, UNO_QUERY);
        if (xSubToolbarController.is())
        {
            const OUString sName (xSubToolbarController->getSubToolbarName());
            if (sName.equals(rsSubToolbarName))
                return iController->first;
        }
    }
    return 0;
}



IMPL_LINK(SidebarToolBox, DropDownClickHandler, ToolBox*, pToolBox)
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




IMPL_LINK(SidebarToolBox, ClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->click();

    return 1;
}




IMPL_LINK(SidebarToolBox, DoubleClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->doubleClick();

    return 1;
}




IMPL_LINK(SidebarToolBox, SelectHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->execute((sal_Int16)pToolBox->GetModifier());

    return 1;
}




IMPL_LINK(SidebarToolBox, ActivateToolBox, ToolBox*, EMPTYARG)
{
    return 1;
}




IMPL_LINK(SidebarToolBox, DeactivateToolBox, ToolBox*, EMPTYARG)
{
    return 1;
}



} } // end of namespace sfx2::sidebar
