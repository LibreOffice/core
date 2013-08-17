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

#include "charsetlistbox.hxx"

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <osl/diagnose.h>

namespace dbaui
{
    // CharSetListBox
    CharSetListBox::CharSetListBox( Window* _pParent, const ResId& _rResId )
        :ListBox( _pParent, _rResId )
    {
        SetDropDownLineCount( 20 );

        OCharsetDisplay::const_iterator charSet = m_aCharSets.begin();
        while ( charSet != m_aCharSets.end() )
        {
            InsertEntry( (*charSet).getDisplayName() );
            ++charSet;
        }
    }

    CharSetListBox::~CharSetListBox()
    {
    }

    void CharSetListBox::SelectEntryByIanaName( const String& _rIanaName )
    {
        OCharsetDisplay::const_iterator aFind = m_aCharSets.findIanaName( _rIanaName );
        if (aFind == m_aCharSets.end())
        {
            OSL_FAIL( "CharSetListBox::SelectEntryByIanaName: unknown charset falling back to system language!" );
            aFind = m_aCharSets.findEncoding( RTL_TEXTENCODING_DONTKNOW );
        }

        if ( aFind == m_aCharSets.end() )
        {
            SelectEntry( String() );
        }
        else
        {
            String sDisplayName = (*aFind).getDisplayName();
            if ( LISTBOX_ENTRY_NOTFOUND == GetEntryPos( sDisplayName ) )
            {
                // in our settings, there was an encoding selected which is not valid for the current
                // data source type
                // This is worth at least an assertion.
                OSL_FAIL( "CharSetListBox::SelectEntryByIanaName: invalid character set!" );
                sDisplayName = String();
            }

            SelectEntry( sDisplayName );
        }
    }

    bool CharSetListBox::StoreSelectedCharSet( SfxItemSet& _rSet, const sal_uInt16 _nItemId )
    {
        bool bChangedSomething = false;
        if ( GetSelectEntryPos() != GetSavedValue() )
        {
            OCharsetDisplay::const_iterator aFind = m_aCharSets.findDisplayName( GetSelectEntry() );
            OSL_ENSURE( aFind != m_aCharSets.end(), "CharSetListBox::StoreSelectedCharSet: could not translate the selected character set!" );
            if ( aFind != m_aCharSets.end() )
            {
                _rSet.Put( SfxStringItem( _nItemId, (*aFind).getIanaName() ) );
                bChangedSomething = true;
            }
        }
        return bChangedSomething;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
