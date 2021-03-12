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
#pragma once

#include <sfx2/sidebar/Panel.hxx>

namespace tools { class Rectangle; }

namespace sfx2::sidebar {

class DeckTitleBar;
class SidebarDockingWindow;

/** Helper for layouting the direct and indirect children of a
    deck like title bars, panels, and scroll bars.
*/
namespace DeckLayouter
{
    void LayoutDeck (
        SidebarDockingWindow* pDockingWindow,
        const tools::Rectangle& rContentArea,
        sal_Int32& rMinimalWidth,
        sal_Int32& rMinimalHeight,
        SharedPanelContainer& rPanels,
        DeckTitleBar& pDeckTitleBar,
        weld::ScrolledWindow& pVerticalScrollBar);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
