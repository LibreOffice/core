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

#include <vcl/builderfactory.hxx>
#include <vcl/contexttabctrl.hxx>
#include <vcl/tabpage.hxx>

VCL_BUILDER_FACTORY(ContextTabControl);

ContextTabControl::ContextTabControl( vcl::Window* pParent, WinBits nStyle ) :
    TabControl( pParent, nStyle )
    , eLastContext(vcl::EnumContext::Context::Context_Any)
{
    HideDisabledTabs();
}

void ContextTabControl::SetContext( vcl::EnumContext::Context eContext )
{
    if (eLastContext != eContext)
    {
        for (int nChild = 0; nChild < GetChildCount(); ++nChild)
        {
            TabPage* pPage = static_cast<TabPage*>(GetChild(nChild));

            if (pPage->HasContext(eContext) || pPage->HasContext(vcl::EnumContext::Context::Context_Any))
                EnablePage(nChild + 1);
            else
                EnablePage(nChild + 1, false);

            if (pPage->HasContext(eContext) && eContext != vcl::EnumContext::Context::Context_Any)
                SetCurPageId(nChild + 1);
        }

        eLastContext = eContext;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
