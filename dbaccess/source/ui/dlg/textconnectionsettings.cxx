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

#include "textconnectionsettings.hxx"
#include "TextConnectionHelper.hxx"
#include "dbu_pageids.hxx"
#include "core_resource.hxx"
#include "dsitems.hxx"
#include "stringconstants.hxx"

#include <vcl/msgbox.hxx>

namespace dbaui
{
    // TextConnectionSettingsDialog
    TextConnectionSettingsDialog::TextConnectionSettingsDialog( vcl::Window* _pParent, SfxItemSet& _rItems )
        :ModalDialog( _pParent, "TextConnectionSettingsDialog", "dbaccess/ui/textconnectionsettings.ui" )
        ,m_rItems( _rItems )
    {
        get(m_pOK, "ok");
        m_pTextConnectionHelper.reset( VclPtr<OTextConnectionHelper>::Create( get<VclVBox>("TextPageContainer"), TC_HEADER | TC_SEPARATORS | TC_CHARSET ) );

        m_pOK->SetClickHdl( LINK( this, TextConnectionSettingsDialog, OnOK ) );
    }

    TextConnectionSettingsDialog::~TextConnectionSettingsDialog()
    {
        disposeOnce();
    }

    void TextConnectionSettingsDialog::dispose()
    {
        m_pOK.clear();
        m_pTextConnectionHelper.disposeAndClear();
        ModalDialog::dispose();
    }

    void TextConnectionSettingsDialog::bindItemStorages( SfxItemSet& _rSet, PropertyValues& _rValues )
    {
        _rValues[ PROPERTY_ID_HEADER_LINE ].reset( new SetItemPropertyStorage( _rSet, DSID_TEXTFILEHEADER ) );
        _rValues[ PROPERTY_ID_FIELD_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_FIELDDELIMITER ) );
        _rValues[ PROPERTY_ID_STRING_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_TEXTDELIMITER ) );
        _rValues[ PROPERTY_ID_DECIMAL_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_DECIMALDELIMITER ) );
        _rValues[ PROPERTY_ID_THOUSAND_DELIMITER ].reset( new SetItemPropertyStorage( _rSet, DSID_THOUSANDSDELIMITER ) );
        _rValues[ PROPERTY_ID_ENCODING ].reset( new SetItemPropertyStorage( _rSet, DSID_CHARSET ) );
    }

    short TextConnectionSettingsDialog::Execute()
    {
        m_pTextConnectionHelper->implInitControls( m_rItems, true );
        return ModalDialog::Execute();
    }

    IMPL_LINK_NOARG( TextConnectionSettingsDialog, OnOK, Button*, void )
    {
        if ( m_pTextConnectionHelper->prepareLeave() )
        {
            m_pTextConnectionHelper->FillItemSet( m_rItems, false/*bUnused*/ );
            EndDialog( RET_OK );
        }
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
