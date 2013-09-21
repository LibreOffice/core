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

#include "TableDesignPanel.hxx"



namespace sd { namespace sidebar {


PanelBase::PanelBase (
    ::Window* pParentWindow,
    ViewShellBase& rViewShellBase)
    : Control(pParentWindow),
      mpWrappedControl(NULL),
      mxSidebar(),
      mrViewShellBase(rViewShellBase)
{
    OSL_TRACE("created PanelBase at %p for parent %p", this, pParentWindow);

#ifdef DEBUG
    SetText(OUString("sd:PanelBase"));
#endif
}




PanelBase::~PanelBase (void)
{
    OSL_TRACE("deleting wrapped control at %p", mpWrappedControl.get());
    mpWrappedControl.reset();
    OSL_TRACE("deleting PanelBase at %p from parent %p", this, GetParent());
}





void PanelBase::Dispose (void)
{
    OSL_TRACE("PanelBase::DisposeL: deleting wrapped control at %p", mpWrappedControl.get());
    mpWrappedControl.reset();
}




css::ui::LayoutSize PanelBase::GetHeightForWidth (const sal_Int32 /*nWidth*/)
{
    sal_Int32 nHeight (0);
    if (ProvideWrappedControl())
        nHeight = mpWrappedControl->GetSizePixel().Height();
    return css::ui::LayoutSize(nHeight,nHeight,nHeight);
}




void PanelBase::Resize (void)
{
    if (ProvideWrappedControl())
    {
        Size aNewSize (GetSizePixel());
        mpWrappedControl->SetOutputSizePixel(aNewSize);
    }
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> PanelBase::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    if (ProvideWrappedControl())
        return mpWrappedControl->GetAccessible();
    else
        return NULL;
}




void PanelBase::SetSidebar (const cssu::Reference<css::ui::XSidebar>& rxSidebar)
{
    mxSidebar = rxSidebar;
    if (mxSidebar.is() && mpWrappedControl!=0)
        mxSidebar->requestLayout();
}




bool PanelBase::ProvideWrappedControl (void)
{
    if ( ! mpWrappedControl)
    {
        mpWrappedControl.reset(CreateWrappedControl(this, mrViewShellBase));
        OSL_TRACE("created wrapped control at %p for parent PanelBase at %p", mpWrappedControl.get(), this);
        if (mpWrappedControl)
            mpWrappedControl->Show();
        if (mxSidebar.is())
            mxSidebar->requestLayout();
    }
    return mpWrappedControl.get() != NULL;
}

ISidebarReceiver::~ISidebarReceiver()
{
}

IDisposable::~IDisposable()
{
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
