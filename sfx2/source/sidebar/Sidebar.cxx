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
#include "precompiled_sfx2.hxx"

#include "Sidebar.hxx"
#include "ResourceManager.hxx"

using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {

Sidebar::Sidebar(
    Window& rParentWindow,
    const Reference<frame::XFrame>& rxDocumentFrame)
    : Window(&rParentWindow, WB_DIALOGCONTROL)
{
    ContentPanelManager::Instance();
}




Sidebar::~Sidebar (void)
{
}




void Sidebar::Resize (void)
{
    Window::Resize();
    //    m_pImpl->OnResize();
}




void Sidebar::GetFocus (void)
{
    Window::GetFocus();
    //    m_pImpl->OnGetFocus();
}




void Sidebar::DataChanged (const DataChangedEvent& rDataChangedEvent)
{
    if  (rDataChangedEvent.GetType() == DATACHANGED_SETTINGS
        &&  (rDataChangedEvent.GetFlags() & SETTINGS_STYLE)!= 0)
    {
        Invalidate();
    }
    else
        Window::DataChanged(rDataChangedEvent);
}



} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
