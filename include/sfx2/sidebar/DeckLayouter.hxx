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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_DECKLAYOUTER_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_DECKLAYOUTER_HXX

#include <sfx2/sidebar/Panel.hxx>

#include <tools/gen.hxx>

#include <com/sun/star/ui/LayoutSize.hpp>

#include <vector>

class ScrollBar;
namespace vcl { class Window; }

namespace sfx2 { namespace sidebar {

class Panel;

/** Helper for layouting the direct and indirect children of a
    deck like title bars, panels, and scroll bars.
*/
namespace DeckLayouter
{
    void LayoutDeck (
        const Rectangle& rContentArea,
        sal_Int32& rMinimalWidth,
        SharedPanelContainer& rPanels,
        vcl::Window& pDeckTitleBar,
        vcl::Window& pScrollClipWindow,
        vcl::Window& pScrollContainer,
        vcl::Window& pFiller,
        ScrollBar& pVerticalScrollBar);
}

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
