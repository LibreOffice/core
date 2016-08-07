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

#include "textconnectionsettings.hxx"
#include "TextConnectionHelper.hxx"
#include "dbu_resource.hrc"
#include "moduledbu.hxx"
#include "dsitems.hxx"
#include "dbustrings.hrc"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <vcl/msgbox.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= TextConnectionSettingsDialog
    //====================================================================
    //--------------------------------------------------------------------
    TextConnectionSettingsDialog::TextConnectionSettingsDialog( Window* _pParent, SfxItemSet& _rItems )
        :ModalDialog( _pParent, ModuleRes( DLG_TEXT_CONNECTION_SETTINGS ) )
        ,m_aOK( this, ModuleRes( 1 ) )
        ,m_aCancel( this, ModuleRes( 1 ) )
        ,m_rItems( _rItems )
    {
        m_pTextConnectionHelper.reset( new OTextConnectionHelper( this, TC_HEADER | TC_SEPARATORS | TC_CHARSET ) );
        FreeResource();

        m_aOK.SetClickHdl( LINK( this, TextConnectionSettingsDialog, OnOK ) );
    }

    //--------------------------------------------------------------------
    TextConnectionSettingsDialog::~TextConnectionSettingsDialog()
    {
    }

    //--------------------------------------------------------------------
    void TextConnectionSettingsDialog::bindItemStorages( SfxItemSet& _rSet, PropertyValues& _rValues )
    {
        _rValues[ PROPERTY_ID_HEADER_LINE ].reset( new SetItemPropertyStorage( _rSet, DSID_TEXTFILEHEADER ) );
        _rValues[ PROPERTY_ID_FIELD_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_FIELDDELIMITER ) );
        _rValues[ PROPERTY_ID_STRING_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_TEXTDELIMITER ) );
        _rValues[ PROPERTY_ID_DECIMAL_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_DECIMALDELIMITER ) );
        _rValues[ PROPERTY_ID_THOUSAND_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_THOUSANDSDELIMITER ) );
        _rValues[ PROPERTY_ID_ENCODING ].reset( new SetItemPropertyStorage( _rSet, DSID_CHARSET ) );
    }

    //--------------------------------------------------------------------
    short TextConnectionSettingsDialog::Execute()
    {
        m_pTextConnectionHelper->implInitControls( m_rItems, sal_True );
        return ModalDialog::Execute();
    }

    //--------------------------------------------------------------------
    IMPL_LINK( TextConnectionSettingsDialog, OnOK, PushButton*, /*_pButton*/ )
    {
        if ( m_pTextConnectionHelper->prepareLeave() )
        {
            sal_Bool bUnused = sal_False;
            m_pTextConnectionHelper->FillItemSet( m_rItems, bUnused );
            EndDialog( RET_OK );
            return 1L;
        }

        return 0L;
    }

//........................................................................
} // namespace dbaui
//........................................................................
