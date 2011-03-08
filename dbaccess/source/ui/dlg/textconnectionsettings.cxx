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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
