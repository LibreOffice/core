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

#include <textconnectionsettings.hxx>
#include "TextConnectionHelper.hxx"
#include <dsitems.hxx>
#include <stringconstants.hxx>

namespace dbaui
{
    // TextConnectionSettingsDialog
    TextConnectionSettingsDialog::TextConnectionSettingsDialog(weld::Window* pParent, SfxItemSet& rItems)
        : GenericDialogController(pParent, u"dbaccess/ui/textconnectionsettings.ui"_ustr, u"TextConnectionSettingsDialog"_ustr)
        , m_rItems(rItems)
        , m_xContainer(m_xBuilder->weld_widget(u"TextPageContainer"_ustr))
        , m_xOK(m_xBuilder->weld_button(u"ok"_ustr))
        , m_xTextConnectionHelper(new OTextConnectionHelper(m_xContainer.get(), TC_HEADER | TC_SEPARATORS | TC_CHARSET))
    {
        m_xOK->connect_clicked(LINK(this, TextConnectionSettingsDialog, OnOK));
    }

    TextConnectionSettingsDialog::~TextConnectionSettingsDialog()
    {
    }

    void TextConnectionSettingsDialog::bindItemStorages( SfxItemSet& _rSet, PropertyValues& _rValues )
    {
        _rValues[ PROPERTY_ID_HEADER_LINE ] = std::make_shared<SetItemPropertyStorage>( _rSet, DSID_TEXTFILEHEADER );
        _rValues[ PROPERTY_ID_FIELD_DELIMITER ] = std::make_shared<SetItemPropertyStorage>( _rSet, DSID_FIELDDELIMITER );
        _rValues[ PROPERTY_ID_STRING_DELIMITER ] = std::make_shared<SetItemPropertyStorage>( _rSet, DSID_TEXTDELIMITER );
        _rValues[ PROPERTY_ID_DECIMAL_DELIMITER ] = std::make_shared<SetItemPropertyStorage>( _rSet, DSID_DECIMALDELIMITER );
        _rValues[ PROPERTY_ID_THOUSAND_DELIMITER ] = std::make_shared<SetItemPropertyStorage>( _rSet, DSID_THOUSANDSDELIMITER );
        _rValues[ PROPERTY_ID_ENCODING ] = std::make_shared<SetItemPropertyStorage>( _rSet, DSID_CHARSET );
    }

    short TextConnectionSettingsDialog::run()
    {
        m_xTextConnectionHelper->implInitControls(m_rItems, true);
        return GenericDialogController::run();
    }

    IMPL_LINK_NOARG(TextConnectionSettingsDialog, OnOK, weld::Button&, void)
    {
        if (m_xTextConnectionHelper->prepareLeave())
        {
            m_xTextConnectionHelper->FillItemSet( m_rItems, false/*bUnused*/ );
            m_xDialog->response(RET_OK);
        }
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
