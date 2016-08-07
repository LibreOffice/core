/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"

#include "charsetlistbox.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svl/itemset.hxx>
#include <svl/stritem.hxx>

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
    bool CharSetListBox::StoreSelectedCharSet( SfxItemSet& _rSet, const sal_uInt16 _nItemId )
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
