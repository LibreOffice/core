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

#include <dlgsave.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <connectivity/dbtools.hxx>
#include <UITools.hxx>
#include <objectnamecheck.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;


IMPL_LINK(OSaveAsDlg, TextFilterHdl, OUString&, rTest, bool)
{
    OUString sCorrected;
    if (m_aChecker.checkString(rTest, sCorrected))
        rTest = sCorrected;
    return true;
}

namespace
{
typedef Reference< XResultSet > (SAL_CALL XDatabaseMetaData::*FGetMetaStrings)();

void lcl_fillComboList( weld::ComboBox& _rList, const Reference< XConnection >& _rxConnection,
                        FGetMetaStrings GetAll, const OUString& _rCurrent )
{
    try {
        Reference< XDatabaseMetaData > xMetaData( _rxConnection->getMetaData(), UNO_SET_THROW );

        Reference< XResultSet > xRes = (xMetaData.get()->*GetAll)();
        Reference< XRow > xRow( xRes, UNO_QUERY_THROW );
        OUString sValue;
        while ( xRes->next() ) {
            sValue = xRow->getString( 1 );
            if ( !xRow->wasNull() )
                _rList.append_text( sValue );
        }

        int nPos = _rList.find_text( _rCurrent );
        if (nPos != -1)
            _rList.set_active( nPos );
        else
            _rList.set_active( 0 );
    } catch( const Exception& ) {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}
}

OSaveAsDlg::OSaveAsDlg( weld::Window * pParent,
                        sal_Int32 _rType,
                        const Reference< XComponentContext >& _rxContext,
                        const Reference< XConnection>& _xConnection,
                        const OUString& rDefault,
                        const IObjectNameCheck& _rObjectNameCheck,
                        SADFlags _nFlags)
    : GenericDialogController(pParent, "dbaccess/ui/savedialog.ui", "SaveDialog")
    , m_xContext( _rxContext )
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(_rType)
    , m_nFlags(_nFlags)
    , m_aChecker(OUString())
    , m_xDescription(m_xBuilder->weld_label("descriptionft"))
    , m_xCatalogLbl(m_xBuilder->weld_label("catalogft"))
    , m_xCatalog(m_xBuilder->weld_combo_box("catalog"))
    , m_xSchemaLbl(m_xBuilder->weld_label("schemaft"))
    , m_xSchema(m_xBuilder->weld_combo_box("schema"))
    , m_xLabel(m_xBuilder->weld_label("titleft"))
    , m_xTitle(m_xBuilder->weld_entry("title"))
    , m_xPB_OK(m_xBuilder->weld_button("ok"))
{
    if ( _xConnection.is() )
        m_xMetaData = _xConnection->getMetaData();

    if (m_xMetaData.is())
    {
        OUString sExtraNameChars(m_xMetaData->getExtraNameCharacters());
        m_aChecker.setAllowedChars(sExtraNameChars);
    }

    m_xTitle->connect_insert_text(LINK(this, OSaveAsDlg, TextFilterHdl));
    m_xSchema->connect_entry_insert_text(LINK(this, OSaveAsDlg, TextFilterHdl));
    m_xCatalog->connect_entry_insert_text(LINK(this, OSaveAsDlg, TextFilterHdl));

    switch (_rType) {
    case CommandType::QUERY:
        implInitOnlyTitle(DBA_RES(STR_QRY_LABEL));
        break;

    case CommandType::TABLE:
        OSL_ENSURE( m_xMetaData.is(), "OSaveAsDlg::OSaveAsDlg: no meta data for entering table names: this will crash!" );
        {
            m_xLabel->set_label(DBA_RES(STR_TBL_LABEL));
            if(m_xMetaData.is() && !m_xMetaData->supportsCatalogsInTableDefinitions()) {
                m_xCatalogLbl->hide();
                m_xCatalog->hide();
            } else {
                // now fill the catalogs
                lcl_fillComboList( *m_xCatalog, _xConnection,
                                   &XDatabaseMetaData::getCatalogs, _xConnection->getCatalog() );
            }

            if ( !m_xMetaData->supportsSchemasInTableDefinitions()) {
                m_xSchemaLbl->hide();
                m_xSchema->hide();
            } else {
                lcl_fillComboList( *m_xSchema, _xConnection,
                                   &XDatabaseMetaData::getSchemas, m_xMetaData->getUserName() );
            }

            OSL_ENSURE(m_xMetaData.is(),"The metadata can not be null!");
            if(m_aName.indexOf('.') != -1) {
                OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(m_xMetaData,
                                                   m_aName,
                                                   sCatalog,
                                                   sSchema,
                                                   sTable,
                                                   ::dbtools::EComposeRule::InDataManipulation);

                int nPos = m_xCatalog->find_text(sCatalog);
                if (nPos != -1)
                    m_xCatalog->set_active(nPos);

                if ( !sSchema.isEmpty() ) {
                    nPos = m_xSchema->find_text(sSchema);
                    if (nPos != -1)
                        m_xSchema->set_active(nPos);
                }
                m_xTitle->set_text(sTable);
            } else
                m_xTitle->set_text(m_aName);
            m_xTitle->select_region(0, -1);

            sal_Int32 nLength =  m_xMetaData.is() ? m_xMetaData->getMaxTableNameLength() : 0;
            if (nLength)
            {
                m_xTitle->set_max_length(nLength);
                m_xSchema->set_entry_max_length(nLength);
                m_xCatalog->set_entry_max_length(nLength);
            }

            bool bCheck = _xConnection.is() && isSQL92CheckEnabled(_xConnection);
            m_aChecker.setCheck(bCheck); // enable non valid sql chars as well
        }
        break;

    default:
        OSL_FAIL( "OSaveAsDlg::OSaveAsDlg: Type not supported yet!" );
    }

    implInit();
}

OSaveAsDlg::OSaveAsDlg(weld::Window * pParent,
                       const Reference< XComponentContext >& _rxContext,
                       const OUString& rDefault,
                       const OUString& _sLabel,
                       const IObjectNameCheck& _rObjectNameCheck,
                       SADFlags _nFlags)
    : GenericDialogController(pParent, "dbaccess/ui/savedialog.ui", "SaveDialog")
    , m_xContext( _rxContext )
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(CommandType::COMMAND)
    , m_nFlags(_nFlags)
    , m_aChecker(OUString())
    , m_xDescription(m_xBuilder->weld_label("descriptionft"))
    , m_xCatalogLbl(m_xBuilder->weld_label("catalogft"))
    , m_xCatalog(m_xBuilder->weld_combo_box("catalog"))
    , m_xSchemaLbl(m_xBuilder->weld_label("schemaft"))
    , m_xSchema(m_xBuilder->weld_combo_box("schema"))
    , m_xLabel(m_xBuilder->weld_label("titleft"))
    , m_xTitle(m_xBuilder->weld_entry("title"))
    , m_xPB_OK(m_xBuilder->weld_button("ok"))
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
