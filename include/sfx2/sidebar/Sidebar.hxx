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

#include <sfx2/dllapi.h>
#include <sfx2/viewfrm.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::uno { template <typename > class Reference; }


namespace sfx2::sidebar {

/** Accept requests for switching to certain panels or decks.
*/
class SFX2_DLLPUBLIC Sidebar
{
public:
    static void ToggleDeck(const OUString& rsDeckId, SfxViewFrame *pViewFrame);

    /** Switch to the deck that contains the specified panel and make
        sure that the panel is visible (expanded and scrolled into the
        visible area.)
        Note that most of the work is done asynchronously and that
        this function probably returns before the requested panel is visible.
    */
    static void ShowPanel (
        const OUString& rsPanelId,
        const css::uno::Reference<css::frame::XFrame>& rxFrame, bool bFocus = false);

    /** Switch to the deck that contains the specified panel and toggle
        the visibility of the panel (expanded and scrolled into the
        visible area when visible)
        Note that most of the work is done asynchronously and that
        this function probably returns before the requested panel is visible.
    */
    static void TogglePanel (
        const OUString& rsPanelId,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static bool IsPanelVisible(
        const OUString& rsPanelId,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
};

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
