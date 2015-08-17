/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/autocmpledit.hxx>
#include <vcl/svapp.hxx>

AutocompleteEdit::AutocompleteEdit( vcl::Window* pParent )
    : Edit( pParent )
    , m_nCurrent( 0 )
{
    SignalConnectAutocomplete( nullptr,
            [this] ( Edit *const pEdit ) { this->AutoCompleteHandler( pEdit ); } );
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

void AutocompleteEdit::AutoCompleteHandler( Edit* )
{
    if( GetAutocompleteAction() != AUTOCOMPLETE_KEYINPUT )
        return;

    if( Application::AnyInput( VclInputFlags::KEYBOARD ) )
        return;

    OUString aCurText = GetText();
    Selection aSelection( GetSelection() );

    if( aSelection.Max() != aCurText.getLength() )
        return;

    sal_uInt16 nLen = ( sal_uInt16 )aSelection.Min();
    aCurText = aCurText.copy( 0, nLen );
    if( !aCurText.isEmpty() )
    {
        if( m_aEntries.size() )
        {
            if( Match( aCurText ) )
            {
                m_nCurrent = 0;
                SetText( m_aMatching[0] );
                sal_uInt16 nNewLen = m_aMatching[0].getLength();

                Selection aSel( nLen, nNewLen );
                SetSelection( aSel );
            }
        }
    }
}

bool AutocompleteEdit::Match( const OUString& rText )
{
    bool bRet = false;

    m_aMatching.clear();

    for( std::vector< OUString >::size_type i = 0; i < m_aEntries.size(); ++i )
    {
        if( m_aEntries[i].startsWith( rText ) )
        {
            m_aMatching.push_back( m_aEntries[i] );
            bRet = true;
        }
    }

    return bRet;
}

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
            sal_uInt16 nLen = ( sal_uInt16 )aSelection.Min();

            if( m_aMatching.size() &&
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
