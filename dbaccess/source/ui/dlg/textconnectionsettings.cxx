/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textconnectionsettings.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:23:51 $
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
