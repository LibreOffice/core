/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL.2.0/.
 */

#include <objectbrowser.hxx>
#include <objectbrowsersearch.hxx>
#include <sal/log.hxx>

namespace basctl
{
ObjectBrowserSearch::ObjectBrowserSearch(ObjectBrowser& rBrowser)
    : m_aDebounceTimer("ObjectBrowserSearchDebounce")
    , m_rBrowser(rBrowser)
{
    m_aDebounceTimer.SetInvokeHandler(LINK(this, ObjectBrowserSearch, DebounceTimerHandler));
    m_aDebounceTimer.SetTimeout(350);
}

ObjectBrowserSearch::~ObjectBrowserSearch() {}

void ObjectBrowserSearch::Initialize()
{
    SAL_INFO("basctl", "ObjectBrowserSearch::Initialize: Starting initialization");

    if (m_rBrowser.GetFilterBox())
    {
        m_rBrowser.GetFilterBox()->connect_changed(
            LINK(this, ObjectBrowserSearch, OnFilterChanged));
        SAL_INFO("basctl", "ObjectBrowserSearch::Initialize: Connected to filter box");
    }

    if (m_rBrowser.GetClearSearchButton())
    {
        m_rBrowser.GetClearSearchButton()->connect_clicked(
            LINK(this, ObjectBrowserSearch, OnClearSearchClicked));
        SAL_INFO("basctl", "ObjectBrowserSearch::Initialize: Connected to clear button");
    }
}

IMPL_STATIC_LINK(basctl::ObjectBrowserSearch, OnFilterChanged, weld::Entry&, /*rBox*/, void)
{
    // STUB
}

IMPL_STATIC_LINK(basctl::ObjectBrowserSearch, OnClearSearchClicked, weld::Button&, /*rButton*/,
                 void)
{
    // STUB
}

IMPL_STATIC_LINK(basctl::ObjectBrowserSearch, DebounceTimerHandler, Timer*, /*pTimer*/, void)
{
    // STUB
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
