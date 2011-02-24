/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "charsetlistbox.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <osl/diagnose.h>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= CharSetListBox
    //====================================================================
    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    CharSetListBox::~CharSetListBox()
    {
    }

    //--------------------------------------------------------------------
    void CharSetListBox::SelectEntryByIanaName( const String& _rIanaName )
    {
        OCharsetDisplay::const_iterator aFind = m_aCharSets.findIanaName( _rIanaName );
        if (aFind == m_aCharSets.end())
        {
            OSL_ASSERT( "CharSetListBox::SelectEntryByIanaName: unknown charset falling back to system language!" );
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
                OSL_ASSERT( "CharSetListBox::SelectEntryByIanaName: invalid character set!" );
                sDisplayName = String();
            }

            SelectEntry( sDisplayName );
        }
    }

    //--------------------------------------------------------------------
    bool CharSetListBox::StoreSelectedCharSet( SfxItemSet& _rSet, const USHORT _nItemId )
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

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
