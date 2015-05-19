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
#include "svx/sidebar/Popup.hxx"
#include "svx/sidebar/PopupContainer.hxx"
#include "svx/sidebar/PopupControl.hxx"

#include <vcl/toolbox.hxx>


namespace svx { namespace sidebar {

Popup::Popup (
    vcl::Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator,
    const ::rtl::OUString& rsAccessibleName)
    : mxControl(),
      mpParent(pParent),
      maControlCreator(rControlCreator),
      maPopupModeEndCallback(),
      msAccessibleName(rsAccessibleName),
      mxContainer()
{
    OSL_ASSERT(mpParent!=nullptr);
    OSL_ASSERT(maControlCreator);
}

Popup::~Popup()
{
    mxControl.disposeAndClear();
    mxContainer.disposeAndClear();
}

void Popup::Show (ToolBox& rToolBox)
{
    rToolBox.SetItemDown(rToolBox.GetCurItemId(), true);

    ProvideContainerAndControl();
    if ( ! (mxContainer && mxControl))
    {
        OSL_ASSERT(mxContainer);
        OSL_ASSERT(mxControl);
        return;
    }

    if ( !mxContainer->IsInPopupMode() )
    {
        mxContainer->SetSizePixel(mxControl->GetOutputSizePixel());

        const Point aPos (rToolBox.GetParent()->OutputToScreenPixel(rToolBox.GetPosPixel()));
        const Size aSize (rToolBox.GetSizePixel());
        const Rectangle aRect (aPos, aSize);

        mxContainer->StartPopupMode(
            aRect,
            FloatWinPopupFlags::NoFocusClose|FloatWinPopupFlags::Down);
        mxContainer->SetPopupModeFlags(
            mxContainer->GetPopupModeFlags()
                | FloatWinPopupFlags::NoAppFocusClose);

        mxControl->GetFocus();
    }
}

void Popup::Hide()
{
    if (mxContainer)
        if (mxContainer->IsInPopupMode())
            mxContainer->EndPopupMode();
}

void Popup::SetPopupModeEndHandler (const ::boost::function<void()>& rCallback)
{
    maPopupModeEndCallback = rCallback;
    if (mxContainer)
        mxContainer->SetPopupModeEndHdl(LINK(this, Popup, PopupModeEndHandler));
}

void Popup::ProvideContainerAndControl()
{
    if ( ! (mxContainer && mxControl)
         && mpParent != nullptr
         && maControlCreator)
    {
        CreateContainerAndControl();
    }
}

void Popup::CreateContainerAndControl()
{
    // Clean previous components, if any
    mxControl.disposeAndClear();
    mxContainer.disposeAndClear();

    mxContainer.set(VclPtr<PopupContainer>::Create(mpParent));
    mxContainer->SetAccessibleName(msAccessibleName);
    mxContainer->SetPopupModeEndHdl(LINK(this, Popup, PopupModeEndHandler));
    mxContainer->SetBorderStyle(mxContainer->GetBorderStyle() | WindowBorderStyle::MENU);

    mxControl.set(maControlCreator(mxContainer.get()));
}

IMPL_LINK_NOARG(Popup, PopupModeEndHandler)
{
    if (maPopupModeEndCallback)
        maPopupModeEndCallback();

    // Popup control is no longer needed and can be destroyed.
    mxControl.disposeAndClear();
    mxContainer.disposeAndClear();

    return 0;
}



} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
