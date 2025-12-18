/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/EntryTreeView.hxx>

namespace weld
{
EntryTreeView::EntryTreeView(std::unique_ptr<Entry> xEntry, std::unique_ptr<TreeView> xTreeView)
    : m_xEntry(std::move(xEntry))
    , m_xTreeView(std::move(xTreeView))
{
    m_xTreeView->connect_selection_changed(LINK(this, EntryTreeView, ClickHdl));
    m_xEntry->connect_changed(LINK(this, EntryTreeView, ModifyHdl));
}

IMPL_LINK(EntryTreeView, ClickHdl, weld::TreeView&, rView, void)
{
    m_xEntry->set_text(rView.get_selected_text());
    m_aChangeHdl.Call(*this);
}

IMPL_LINK_NOARG(EntryTreeView, ModifyHdl, weld::Entry&, void) { m_aChangeHdl.Call(*this); }

void EntryTreeView::set_height_request_by_rows(int nRows)
{
    int nHeight = nRows == -1 ? -1 : m_xTreeView->get_height_rows(nRows);
    m_xTreeView->set_size_request(m_xTreeView->get_size_request().Width(), nHeight);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
