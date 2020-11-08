/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "autocmpledit.hxx"

AutocompleteEdit::AutocompleteEdit(std::unique_ptr<weld::Entry> xEntry)
    : m_xEntry(std::move(xEntry))
{
    m_xEntry->connect_changed(LINK(this, AutocompleteEdit, ChangedHdl));

    m_aChangedIdle.SetInvokeHandler(LINK(this, AutocompleteEdit, TryAutoComplete));
    m_aChangedIdle.SetDebugName("fpicker::AutocompleteEdit m_aChangedIdle");
}

IMPL_LINK_NOARG(AutocompleteEdit, ChangedHdl, weld::Entry&, void)
{
    m_aChangeHdl.Call(*m_xEntry);
    m_aChangedIdle.Start(); //launch this to happen on idle after cursor position will have been set
}

void AutocompleteEdit::AddEntry( const OUString& rEntry )
{
    m_aEntries.push_back( rEntry );
}

void AutocompleteEdit::ClearEntries()
{
    m_aEntries.clear();
    m_aMatching.clear();
}

IMPL_LINK_NOARG(AutocompleteEdit, TryAutoComplete, Timer *, void)
{
    OUString aCurText = m_xEntry->get_text();

    int nStartPos, nEndPos;
    m_xEntry->get_selection_bounds(nStartPos, nEndPos);
    if (std::max(nStartPos, nEndPos) != aCurText.getLength())
        return;

    auto nLen = std::min(nStartPos, nEndPos);
    aCurText = aCurText.copy( 0, nLen );
    if( aCurText.isEmpty() )
        return;

    if( !m_aEntries.empty() )
    {
        if( Match( aCurText ) )
        {
            m_xEntry->set_text(m_aMatching[0]);
            auto nNewLen = m_aMatching[0].getLength();
            m_xEntry->select_region(nLen, nNewLen);
        }
    }
}

bool AutocompleteEdit::Match( const OUString& rText )
{
    bool bRet = false;

    m_aMatching.clear();

    for(const OUString & rEntry : m_aEntries)
    {
        if( rEntry.startsWithIgnoreAsciiCase( rText ) )
        {
            m_aMatching.push_back( rEntry );
            bRet = true;
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
