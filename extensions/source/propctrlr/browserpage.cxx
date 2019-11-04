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

#include "browserpage.hxx"

namespace pcr
{
    OBrowserPage::OBrowserPage(weld::Container* pParent, weld::Container* pInitialControlContainer)
        : BuilderPage(pParent, /*TODO*/ nullptr, "modules/spropctrlr/ui/browserpage.ui", "BrowserPage")
        , m_pParent(pParent)
        , m_xListBox(std::make_unique<OBrowserListBox>(*m_xBuilder, pInitialControlContainer))
    {
    }

    OBrowserPage::~OBrowserPage()
    {
        m_pParent->move(m_xContainer.get(), nullptr);
    }

#if 0
    void OBrowserPage::StateChanged(StateChangedType nType)
    {
        Window::StateChanged( nType);
        if (StateChangedType::Visible == nType && m_xListBox)
            m_xListBox->ActivateListBox(IsVisible());
    }
#endif

} // namespace pcr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
