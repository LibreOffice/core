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
#include <dbu_dlg.hxx>
#include <strings.hrc>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <sqlmessage.hxx>
#include <connectivity/dbtools.hxx>
#include <UITools.hxx>
#include <SqlNameEdit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <objectnamecheck.hxx>
#include <tools/diagnose_ex.h>

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
namespace dbaui
{

class OSaveAsDlgImpl
{
public:
    OUString                   m_aQryLabel;
    OUString                   m_sTblLabel;
    OUString                   m_aName;
    const IObjectNameCheck&    m_rObjectNameCheck;
    css::uno::Reference< css::sdbc::XDatabaseMetaData>            m_xMetaData;
    sal_Int32                  m_nType;
    SADFlags                   m_nFlags;

    OSQLNameChecker            m_aChecker;

    std::unique_ptr<weld::Label> m_xDescription;
    std::unique_ptr<weld::Label> m_xCatalogLbl;
    std::unique_ptr<weld::ComboBox> m_xCatalog;
    std::unique_ptr<weld::Label> m_xSchemaLbl;
    std::unique_ptr<weld::ComboBox> m_xSchema;
    std::unique_ptr<weld::Label> m_xLabel;
    std::unique_ptr<weld::Entry> m_xTitle;
    std::unique_ptr<weld::Button> m_xPB_OK;

    DECL_LINK(TextFilterHdl, OUString&, bool);

    OSaveAsDlgImpl( weld::Builder* pParent, sal_Int32 _rType,
                    const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                    const OUString& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    SADFlags _nFlags);
    OSaveAsDlgImpl( weld::Builder* pParent,
                    const OUString& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    SADFlags _nFlags);
};

} // dbaui

IMPL_LINK(OSaveAsDlgImpl, TextFilterHdl, OUString&, rTest, bool)
{
    OUString sCorrected;
    if (m_aChecker.checkString(rTest, sCorrected))
        rTest = sCorrected;
    return true;
}

OSaveAsDlgImpl::OSaveAsDlgImpl(weld::Builder* pBuilder,
                               sal_Int32 _rType,
                               const Reference< XConnection>& _xConnection,
                               const OUString& rDefault,
                               const IObjectNameCheck& _rObjectNameCheck,
                               SADFlags _nFlags)
    : m_aQryLabel(DBA_RES(STR_QRY_LABEL))
    , m_sTblLabel(DBA_RES(STR_TBL_LABEL))
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(_rType)
    , m_nFlags(_nFlags)
    , m_aChecker(OUString())
    , m_xDescription(pBuilder->weld_label("descriptionft"))
    , m_xCatalogLbl(pBuilder->weld_label("catalogft"))
    , m_xCatalog(pBuilder->weld_combo_box("catalog"))
    , m_xSchemaLbl(pBuilder->weld_label("schemaft"))
    , m_xSchema(pBuilder->weld_combo_box("schema"))
    , m_xLabel(pBuilder->weld_label("titleft"))
    , m_xTitle(pBuilder->weld_entry("title"))
    , m_xPB_OK(pBuilder->weld_button("ok"))
{
    if ( _xConnection.is() )
        m_xMetaData = _xConnection->getMetaData();

    if (m_xMetaData.is())
    {
        OUString sExtraNameChars(m_xMetaData->getExtraNameCharacters());
        m_aChecker.setAllowedChars(sExtraNameChars);
    }

    m_xTitle->connect_insert_text(LINK(this, OSaveAsDlgImpl, TextFilterHdl));
    m_xSchema->connect_entry_insert_text(LINK(this, OSaveAsDlgImpl, TextFilterHdl));
    m_xCatalog->connect_entry_insert_text(LINK(this, OSaveAsDlgImpl, TextFilterHdl));
}

OSaveAsDlgImpl::OSaveAsDlgImpl(weld::Builder* pBuilder,
                               const OUString& rDefault,
                               const IObjectNameCheck& _rObjectNameCheck,
                               SADFlags _nFlags)
    : m_aQryLabel(DBA_RES(STR_QRY_LABEL))
    , m_sTblLabel(DBA_RES(STR_TBL_LABEL))
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(CommandType::COMMAND)
    , m_nFlags(_nFlags)
    , m_aChecker(OUString())
    , m_xDescription(pBuilder->weld_label("descriptionft"))
    , m_xCatalogLbl(pBuilder->weld_label("catalogft"))
    , m_xCatalog(pBuilder->weld_combo_box("catalog"))
    , m_xSchemaLbl(pBuilder->weld_label("schemaft"))
    , m_xSchema(pBuilder->weld_combo_box("schema"))
    , m_xLabel(pBuilder->weld_label("titleft"))
    , m_xTitle(pBuilder->weld_entry("title"))
    , m_xPB_OK(pBuilder->weld_button("ok"))
{
    m_xTitle->connect_insert_text(LINK(this, OSaveAsDlgImpl, TextFilterHdl));
    m_xSchema->connect_entry_insert_text(LINK(this, OSaveAsDlgImpl, TextFilterHdl));
    m_xCatalog->connect_entry_insert_text(LINK(this, OSaveAsDlgImpl, TextFilterHdl));
}

using namespace ::com::sun::star::lang;

namespace
{
typedef Reference< XResultSet > (SAL_CALL XDatabaseMetaData::*FGetMetaStrings)();

void lcl_fillComboList( weld::ComboBox& _rList, const Reference< XConnection >& _rxConnection,
                        FGetMetaStrings GetAll, const OUString& _rCurrent )
{
    try {
        Reference< XDatabaseMetaData > xMetaData( _rxConnection->getMetaData(), UNO_QUERY_THROW );

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
{
    m_pImpl.reset( new OSaveAsDlgImpl(m_xBuilder.get(),_rType,_xConnection,rDefault,_rObjectNameCheck,_nFlags) );

    switch (_rType) {
    case CommandType::QUERY:
        implInitOnlyTitle(m_pImpl->m_aQryLabel);
        break;

    case CommandType::TABLE:
        OSL_ENSURE( m_pImpl->m_xMetaData.is(), "OSaveAsDlg::OSaveAsDlg: no meta data for entering table names: this will crash!" );
        {
            m_pImpl->m_xLabel->set_label(m_pImpl->m_sTblLabel);
            if(m_pImpl->m_xMetaData.is() && !m_pImpl->m_xMetaData->supportsCatalogsInTableDefinitions()) {
                m_pImpl->m_xCatalogLbl->hide();
                m_pImpl->m_xCatalog->hide();
            } else {
                // now fill the catalogs
                lcl_fillComboList( *m_pImpl->m_xCatalog, _xConnection,
                                   &XDatabaseMetaData::getCatalogs, _xConnection->getCatalog() );
            }

            if ( !m_pImpl->m_xMetaData->supportsSchemasInTableDefinitions()) {
                m_pImpl->m_xSchemaLbl->hide();
                m_pImpl->m_xSchema->hide();
            } else {
                lcl_fillComboList( *m_pImpl->m_xSchema, _xConnection,
                                   &XDatabaseMetaData::getSchemas, m_pImpl->m_xMetaData->getUserName() );
            }

            OSL_ENSURE(m_pImpl->m_xMetaData.is(),"The metadata can not be null!");
            if(m_pImpl->m_aName.indexOf('.') != -1) {
                OUString sCatalog,sSchema,sTable;
                ::dbtools::qualifiedNameComponents(m_pImpl->m_xMetaData,
                                                   m_pImpl->m_aName,
                                                   sCatalog,
                                                   sSchema,
                                                   sTable,
                                                   ::dbtools::EComposeRule::InDataManipulation);

                int nPos = m_pImpl->m_xCatalog->find_text(sCatalog);
                if (nPos != -1)
                    m_pImpl->m_xCatalog->set_active(nPos);

                if ( !sSchema.isEmpty() ) {
                    nPos = m_pImpl->m_xSchema->find_text(sSchema);
                    if (nPos != -1)
                        m_pImpl->m_xSchema->set_active(nPos);
                }
                m_pImpl->m_xTitle->set_text(sTable);
            } else
                m_pImpl->m_xTitle->set_text(m_pImpl->m_aName);
            m_pImpl->m_xTitle->select_region(0, -1);

            sal_Int32 nLength =  m_pImpl->m_xMetaData.is() ? m_pImpl->m_xMetaData->getMaxTableNameLength() : 0;
            if (nLength)
            {
                m_pImpl->m_xTitle->set_max_length(nLength);
                m_pImpl->m_xSchema->set_entry_max_length(nLength);
                m_pImpl->m_xCatalog->set_entry_max_length(nLength);
            }

            bool bCheck = _xConnection.is() && isSQL92CheckEnabled(_xConnection);
            m_pImpl->m_aChecker.setCheck(bCheck); // enable non valid sql chars as well
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
{
    m_pImpl.reset( new OSaveAsDlgImpl(m_xBuilder.get(),rDefault,_rObjectNameCheck,_nFlags) );
    implInitOnlyTitle(_sLabel);
    implInit();
}

OSaveAsDlg::~OSaveAsDlg()
{
}

IMPL_LINK_NOARG(OSaveAsDlg, ButtonClickHdl, weld::Button&, void)
{
    m_pImpl->m_aName = m_pImpl->m_xTitle->get_text();

    OUString sNameToCheck( m_pImpl->m_aName );

    if ( m_pImpl->m_nType == CommandType::TABLE ) {
        sNameToCheck = ::dbtools::composeTableName(
                           m_pImpl->m_xMetaData,
                           getCatalog(),
                           getSchema(),
                           sNameToCheck,
                           false,  // no quoting
                           ::dbtools::EComposeRule::InDataManipulation
                       );
    }

    SQLExceptionInfo aNameError;
    if ( m_pImpl->m_rObjectNameCheck.isNameValid( sNameToCheck, aNameError ) )
        m_xDialog->response(RET_OK);

    showError(aNameError, m_xDialog->GetXWindow(), m_xContext);
    m_pImpl->m_xTitle->grab_focus();
}

IMPL_LINK_NOARG(OSaveAsDlg, EditModifyHdl, weld::Entry&, void)
{
    m_pImpl->m_xPB_OK->set_sensitive(!m_pImpl->m_xTitle->get_text().isEmpty());
}

void OSaveAsDlg::implInitOnlyTitle(const OUString& _rLabel)
{
    m_pImpl->m_xLabel->set_label(_rLabel);
    m_pImpl->m_xCatalogLbl->hide();
    m_pImpl->m_xCatalog->hide();
    m_pImpl->m_xSchemaLbl->hide();
    m_pImpl->m_xSchema->hide();

    m_pImpl->m_xTitle->set_text(m_pImpl->m_aName);
    m_pImpl->m_aChecker.setCheck(false); // enable non valid sql chars as well
}

void OSaveAsDlg::implInit()
{
    if ( !( m_pImpl->m_nFlags & SADFlags::AdditionalDescription ) ) {
        // hide the description window
        m_pImpl->m_xDescription->hide();
    }

    if ( SADFlags::TitlePasteAs == ( m_pImpl->m_nFlags & SADFlags::TitlePasteAs ) )
        m_xDialog->set_title( DBA_RES( STR_TITLE_PASTE_AS ) );
    else if ( SADFlags::TitleRename == ( m_pImpl->m_nFlags & SADFlags::TitleRename ) )
        m_xDialog->set_title( DBA_RES( STR_TITLE_RENAME ) );

    m_pImpl->m_xPB_OK->connect_clicked(LINK(this,OSaveAsDlg,ButtonClickHdl));
    m_pImpl->m_xTitle->connect_changed(LINK(this,OSaveAsDlg,EditModifyHdl));
    m_pImpl->m_xTitle->grab_focus();
}

const OUString& OSaveAsDlg::getName() const
{
    return m_pImpl->m_aName;
}
OUString OSaveAsDlg::getCatalog() const
{
    return m_pImpl->m_xCatalog->get_visible() ? m_pImpl->m_xCatalog->get_active_text() : OUString();
}
OUString OSaveAsDlg::getSchema() const
{
    return m_pImpl->m_xSchema->get_visible() ? m_pImpl->m_xSchema->get_active_text() : OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
