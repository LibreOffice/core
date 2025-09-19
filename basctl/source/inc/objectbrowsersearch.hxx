/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL.2.0/.
 */

#pragma once

#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

namespace basctl
{
class ObjectBrowser;

class ObjectBrowserSearch
{
public:
    explicit ObjectBrowserSearch(ObjectBrowser& rBrowser);
    ~ObjectBrowserSearch();

    void Initialize();

private:
    // Event handlers for the UI widgets
    DECL_STATIC_LINK(ObjectBrowserSearch, OnFilterChanged, weld::Entry&, void);
    DECL_STATIC_LINK(ObjectBrowserSearch, OnClearSearchClicked, weld::Button&, void);
    DECL_STATIC_LINK(ObjectBrowserSearch, DebounceTimerHandler, Timer*, void);

    Timer m_aDebounceTimer;
    ObjectBrowser& m_rBrowser;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
