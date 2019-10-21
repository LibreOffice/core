/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "autocmpledit.hxx"
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>

AutocompleteEdit::AutocompleteEdit(std::unique_ptr<weld::Entry> xEntry)
    : m_xEntry(std::move(xEntry))
    , m_nCurrent(0)
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
            m_nCurrent = 0;
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

#if 0
bool AutocompleteEdit::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent& rEvent = *rNEvt.GetKeyEvent();
        const vcl::KeyCode& rKey = rEvent.GetKeyCode();
        vcl::KeyCode aCode( rKey.GetCode() );

        if( ( aCode == KEY_UP || aCode == KEY_DOWN ) && !rKey.IsMod2() )
        {
            Selection aSelection( GetSelection() );
            sal_uInt16 nLen = static_cast<sal_uInt16>(aSelection.Min());

            if( !m_aMatching.empty() &&
                ( ( aCode == KEY_DOWN && m_nCurrent + 1 < m_aMatching.size() )
                || ( aCode == KEY_UP && m_nCurrent > 0 ) ) )
            {
                SetText( m_aMatching[ aCode == KEY_DOWN ? ++m_nCurrent : --m_nCurrent ] );
                SetSelection( Selection( nLen, GetText().getLength() ) );
                return true;
            }
        }
    }

    return Edit::PreNotify( rNEvt );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
