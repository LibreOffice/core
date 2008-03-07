/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: charsetlistbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:20:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "charsetlistbox.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svtools/itemset.hxx>
#include <svtools/stritem.hxx>

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
            DBG_ERROR( "CharSetListBox::SelectEntryByIanaName: unknown charset falling back to system language!" );
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
                DBG_ERROR( "CharSetListBox::SelectEntryByIanaName: invalid character set!" );
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
            DBG_ASSERT( aFind != m_aCharSets.end(), "CharSetListBox::StoreSelectedCharSet: could not translate the selected character set!" );
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
