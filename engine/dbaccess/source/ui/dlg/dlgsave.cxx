/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <dlgsave.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <connectivity/dbtools.hxx>
#include <UITools.hxx>
#include <objectnamecheck.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;


IMPL_LINK(OSaveAsDlg, TextFilterHdl, OUString&, rTest, bool)
{
    OUString sCorrected;
    if (m_aChecker.checkString(rTest, sCorrected))
        rTest = sCorrected;
    return true;
}

OSaveAsDlg::OSaveAsDlg(weld::Window * pParent,
                       const Reference< XComponentContext >& _rxContext,
                       const OUString& rDefault,
                       const OUString& _sLabel,
                       const IObjectNameCheck& _rObjectNameCheck,
                       SADFlags _nFlags)
    : GenericDialogController(pParent, u"dbaccess/ui/savedialog.ui"_ustr, u"SaveDialog"_ustr)
    , m_xContext( _rxContext )
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(CommandType::COMMAND)
    , m_nFlags(_nFlags)
    , m_aChecker(OUString())
    , m_xDescription(m_xBuilder->weld_label(u"descriptionft"_ustr))
    , m_xCatalogLbl(m_xBuilder->weld_label(u"catalogft"_ustr))
    , m_xCatalog(m_xBuilder->weld_combo_box(u"catalog"_ustr))
    , m_xSchemaLbl(m_xBuilder->weld_label(u"schemaft"_ustr))
    , m_xSchema(m_xBuilder->weld_combo_box(u"schema"_ustr))
    , m_xLabel(m_xBuilder->weld_label(u"titleft"_ustr))
    , m_xTitle(m_xBuilder->weld_entry(u"title"_ustr))
    , m_xPB_OK(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xTitle->connect_insert_text(LINK(this, OSaveAsDlg, TextFilterHdl));
    m_xSchema->connect_entry_insert_text(LINK(this, OSaveAsDlg, TextFilterHdl));
    m_xCatalog->connect_entry_insert_text(LINK(this, OSaveAsDlg, TextFilterHdl));
    implInitOnlyTitle(_sLabel);
    implInit();
}

OSaveAsDlg::~OSaveAsDlg()
{
}

IMPL_LINK_NOARG(OSaveAsDlg, ButtonClickHdl, weld::Button&, void)
{
    m_aName = m_xTitle->get_text();

    OUString sNameToCheck( m_aName );

    if ( m_nType == CommandType::TABLE ) {
        sNameToCheck = ::dbtools::composeTableName(
                           m_xMetaData,
                           getCatalog(),
                           getSchema(),
                           sNameToCheck,
                           false,  // no quoting
                           ::dbtools::EComposeRule::InDataManipulation
                       );
    }

    SQLExceptionInfo aNameError;
    if ( m_rObjectNameCheck.isNameValid( sNameToCheck, aNameError ) )
        m_xDialog->response(RET_OK);

    showError(aNameError, m_xDialog->GetXWindow(), m_xContext);
    m_xTitle->grab_focus();
}

IMPL_LINK_NOARG(OSaveAsDlg, EditModifyHdl, weld::Entry&, void)
{
    m_xPB_OK->set_sensitive(!m_xTitle->get_text().isEmpty());
}

void OSaveAsDlg::implInitOnlyTitle(const OUString& _rLabel)
{
    m_xLabel->set_label(_rLabel);
    m_xCatalogLbl->hide();
    m_xCatalog->hide();
    m_xSchemaLbl->hide();
    m_xSchema->hide();

    m_xTitle->set_text(m_aName);
    m_aChecker.setCheck(false); // enable non valid sql chars as well
}

void OSaveAsDlg::implInit()
{
    if ( !( m_nFlags & SADFlags::AdditionalDescription ) ) {
        // hide the description window
        m_xDescription->hide();
    }

    if ( SADFlags::TitlePasteAs == ( m_nFlags & SADFlags::TitlePasteAs ) )
        m_xDialog->set_title( DBA_RES( STR_TITLE_PASTE_AS ) );
    else if ( SADFlags::TitleRename == ( m_nFlags & SADFlags::TitleRename ) )
        m_xDialog->set_title( DBA_RES( STR_TITLE_RENAME ) );

    m_xPB_OK->connect_clicked(LINK(this,OSaveAsDlg,ButtonClickHdl));
    m_xTitle->connect_changed(LINK(this,OSaveAsDlg,EditModifyHdl));
    m_xTitle->grab_focus();
}

const OUString& OSaveAsDlg::getName() const
{
    return m_aName;
}
OUString OSaveAsDlg::getCatalog() const
{
    return m_xCatalog->get_visible() ? m_xCatalog->get_active_text() : OUString();
}
OUString OSaveAsDlg::getSchema() const
{
    return m_xSchema->get_visible() ? m_xSchema->get_active_text() : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
