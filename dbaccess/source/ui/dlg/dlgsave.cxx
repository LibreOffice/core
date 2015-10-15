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

#include "dlgsave.hxx"
#include <vcl/msgbox.hxx>
#include "dbu_dlg.hrc"
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include "sqlmessage.hxx"
#include <connectivity/dbtools.hxx>
#include "UITools.hxx"
#include "dbaccess_helpid.hrc"
#include "SqlNameEdit.hxx"
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "moduledbu.hxx"
#include "objectnamecheck.hxx"
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
    VclPtr<FixedText>          m_pDescription;
    VclPtr<FixedText>          m_pCatalogLbl;
    VclPtr<OSQLNameComboBox>   m_pCatalog;
    VclPtr<FixedText>          m_pSchemaLbl;
    VclPtr<OSQLNameComboBox>   m_pSchema;
    VclPtr<FixedText>          m_pLabel;
    VclPtr<OSQLNameEdit>       m_pTitle;
    VclPtr<OKButton>           m_pPB_OK;
    OUString                   m_aQryLabel;
    OUString                   m_sTblLabel;
    OUString                   m_aName;
    const IObjectNameCheck&    m_rObjectNameCheck;
    OUString                   m_sParentURL;
    css::uno::Reference< css::sdbc::XDatabaseMetaData>            m_xMetaData;
    sal_Int32                  m_nType;
    sal_Int32                  m_nFlags;

    OSaveAsDlgImpl( OSaveAsDlg* pParent,const sal_Int32& _rType,
                    const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                    const OUString& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags);
    OSaveAsDlgImpl( OSaveAsDlg* pParent,
                    const OUString& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags);
};

} // dbaui

OSaveAsDlgImpl::OSaveAsDlgImpl(OSaveAsDlg* pParent,
                        const sal_Int32& _rType,
                        const Reference< XConnection>& _xConnection,
                        const OUString& rDefault,
                        const IObjectNameCheck& _rObjectNameCheck,
                        sal_Int32 _nFlags)
    : m_aQryLabel(ModuleRes(STR_QRY_LABEL))
    , m_sTblLabel(ModuleRes(STR_TBL_LABEL))
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(_rType)
    , m_nFlags(_nFlags)
{
    pParent->get(m_pDescription, "descriptionft");
    pParent->get(m_pCatalogLbl, "catalogft");
    pParent->get(m_pCatalog, "catalog");
    pParent->get(m_pSchemaLbl, "schemaft");
    pParent->get(m_pSchema, "schema");
    pParent->get(m_pLabel, "titleft");
    pParent->get(m_pTitle, "title");
    pParent->get(m_pPB_OK, "ok");

    if ( _xConnection.is() )
        m_xMetaData = _xConnection->getMetaData();

    if ( m_xMetaData.is() )
    {
        OUString sExtraNameChars( m_xMetaData->getExtraNameCharacters() );
        m_pCatalog->setAllowedChars( sExtraNameChars );
        m_pSchema->setAllowedChars( sExtraNameChars );
        m_pTitle->setAllowedChars( sExtraNameChars );
    }

    m_pCatalog->SetDropDownLineCount( 10 );
    m_pSchema->SetDropDownLineCount( 10 );
}

OSaveAsDlgImpl::OSaveAsDlgImpl(OSaveAsDlg* pParent,
                        const OUString& rDefault,
                        const IObjectNameCheck& _rObjectNameCheck,
                        sal_Int32 _nFlags)
    : m_aQryLabel(ModuleRes(STR_QRY_LABEL))
    , m_sTblLabel(ModuleRes(STR_TBL_LABEL))
    , m_aName(rDefault)
    , m_rObjectNameCheck( _rObjectNameCheck )
    , m_nType(CommandType::COMMAND)
    , m_nFlags(_nFlags)
{
    pParent->get(m_pDescription, "descriptionft");
    pParent->get(m_pCatalogLbl, "catalogft");
    pParent->get(m_pCatalog, "catalog");
    pParent->get(m_pSchemaLbl, "schemaft");
    pParent->get(m_pSchema, "schema");
    pParent->get(m_pLabel, "titleft");
    pParent->get(m_pTitle, "title");
    pParent->get(m_pPB_OK, "ok");

    m_pCatalog->SetDropDownLineCount( 10 );
    m_pSchema->SetDropDownLineCount( 10 );
}

using namespace ::com::sun::star::lang;

namespace
{
    typedef Reference< XResultSet > (SAL_CALL XDatabaseMetaData::*FGetMetaStrings)();

    void lcl_fillComboList( ComboBox& _rList, const Reference< XConnection >& _rxConnection,
        FGetMetaStrings _GetAll, const OUString& _rCurrent )
    {
        try
        {
            Reference< XDatabaseMetaData > xMetaData( _rxConnection->getMetaData(), UNO_QUERY_THROW );

            Reference< XResultSet > xRes = (xMetaData.get()->*_GetAll)();
            Reference< XRow > xRow( xRes, UNO_QUERY_THROW );
            OUString sValue;
            while ( xRes->next() )
            {
                sValue = xRow->getString( 1 );
                if ( !xRow->wasNull() )
                    _rList.InsertEntry( sValue );
            }

            sal_Int32 nPos = _rList.GetEntryPos( OUString( _rCurrent ) );
            if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                _rList.SelectEntryPos( nPos );
            else
                _rList.SelectEntryPos( 0 );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

OSaveAsDlg::OSaveAsDlg( vcl::Window * pParent,
                        const sal_Int32& _rType,
                        const Reference< XComponentContext >& _rxContext,
                        const Reference< XConnection>& _xConnection,
                        const OUString& rDefault,
                        const IObjectNameCheck& _rObjectNameCheck,
                        sal_Int32 _nFlags)
    : ModalDialog(pParent, "SaveDialog", "dbaccess/ui/savedialog.ui")
    , m_xContext( _rxContext )
{
    m_pImpl = new OSaveAsDlgImpl(this,_rType,_xConnection,rDefault,_rObjectNameCheck,_nFlags);

    switch (_rType)
    {
        case CommandType::QUERY:
            implInitOnlyTitle(m_pImpl->m_aQryLabel);
            break;

        case CommandType::TABLE:
            OSL_ENSURE( m_pImpl->m_xMetaData.is(), "OSaveAsDlg::OSaveAsDlg: no meta data for entering table names: this will crash!" );
            {
                m_pImpl->m_pLabel->SetText(m_pImpl->m_sTblLabel);
                if(m_pImpl->m_xMetaData.is() && !m_pImpl->m_xMetaData->supportsCatalogsInTableDefinitions())
                {
                    m_pImpl->m_pCatalogLbl->Hide();
                    m_pImpl->m_pCatalog->Hide();
                }
                else
                {
                    // now fill the catalogs
                    lcl_fillComboList( *m_pImpl->m_pCatalog, _xConnection,
                        &XDatabaseMetaData::getCatalogs, _xConnection->getCatalog() );
                }

                if ( !m_pImpl->m_xMetaData->supportsSchemasInTableDefinitions())
                {
                    m_pImpl->m_pSchemaLbl->Hide();
                    m_pImpl->m_pSchema->Hide();
                }
                else
                {
                    lcl_fillComboList( *m_pImpl->m_pSchema, _xConnection,
                        &XDatabaseMetaData::getSchemas, m_pImpl->m_xMetaData->getUserName() );
                }

                OSL_ENSURE(m_pImpl->m_xMetaData.is(),"The metadata can not be null!");
                if(m_pImpl->m_aName.indexOf('.') != -1)
                {
                    OUString sCatalog,sSchema,sTable;
                    ::dbtools::qualifiedNameComponents(m_pImpl->m_xMetaData,
                                                        m_pImpl->m_aName,
                                                        sCatalog,
                                                        sSchema,
                                                        sTable,
                                                        ::dbtools::eInDataManipulation);

                    sal_Int32 nPos = m_pImpl->m_pCatalog->GetEntryPos(OUString(sCatalog));
                    if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                        m_pImpl->m_pCatalog->SelectEntryPos(nPos);

                    if ( !sSchema.isEmpty() )
                    {
                        nPos = m_pImpl->m_pSchema->GetEntryPos(OUString(sSchema));
                        if ( nPos != COMBOBOX_ENTRY_NOTFOUND )
                            m_pImpl->m_pSchema->SelectEntryPos(nPos);
                    }
                    m_pImpl->m_pTitle->SetText(sTable);
                }
                else
                    m_pImpl->m_pTitle->SetText(m_pImpl->m_aName);
                m_pImpl->m_pTitle->SetSelection( Selection( SELECTION_MIN, SELECTION_MAX ) );

                sal_Int32 nLength =  m_pImpl->m_xMetaData.is() ? m_pImpl->m_xMetaData->getMaxTableNameLength() : 0;
                nLength = nLength ? nLength : EDIT_NOLIMIT;

                m_pImpl->m_pTitle->SetMaxTextLen(nLength);
                m_pImpl->m_pSchema->SetMaxTextLen(nLength);
                m_pImpl->m_pCatalog->SetMaxTextLen(nLength);

                bool bCheck = _xConnection.is() && isSQL92CheckEnabled(_xConnection);
                m_pImpl->m_pTitle->setCheck(bCheck); // enable non valid sql chars as well
                m_pImpl->m_pSchema->setCheck(bCheck); // enable non valid sql chars as well
                m_pImpl->m_pCatalog->setCheck(bCheck); // enable non valid sql chars as well
            }
            break;

        default:
            OSL_FAIL( "OSaveAsDlg::OSaveAsDlg: Type not supported yet!" );
    }

    implInit();
}

OSaveAsDlg::OSaveAsDlg( vcl::Window * pParent,
                        const Reference< XComponentContext >& _rxContext,
                        const OUString& rDefault,
                        const OUString& _sLabel,
                        const IObjectNameCheck& _rObjectNameCheck,
                        sal_Int32 _nFlags)
    : ModalDialog(pParent, "SaveDialog", "dbaccess/ui/savedialog.ui")
    , m_xContext( _rxContext )
{
    m_pImpl = new OSaveAsDlgImpl(this,rDefault,_rObjectNameCheck,_nFlags);
    implInitOnlyTitle(_sLabel);
    implInit();
}

OSaveAsDlg::~OSaveAsDlg()
{
    disposeOnce();
}

void OSaveAsDlg::dispose()
{
    DELETEZ(m_pImpl);
    ModalDialog::dispose();
}

IMPL_LINK_TYPED(OSaveAsDlg, ButtonClickHdl, Button *, pButton, void)
{
    if (pButton == m_pImpl->m_pPB_OK)
    {
        m_pImpl->m_aName = m_pImpl->m_pTitle->GetText();

        OUString sNameToCheck( m_pImpl->m_aName );

        if ( m_pImpl->m_nType == CommandType::TABLE )
        {
            sNameToCheck = ::dbtools::composeTableName(
                m_pImpl->m_xMetaData,
                getCatalog(),
                getSchema(),
                sNameToCheck,
                false,  // no quoting
                ::dbtools::eInDataManipulation
            );
        }

        SQLExceptionInfo aNameError;
        if ( m_pImpl->m_rObjectNameCheck.isNameValid( sNameToCheck, aNameError ) )
            EndDialog( RET_OK );

        showError( aNameError, this, m_xContext );
        m_pImpl->m_pTitle->GrabFocus();
    }
}

IMPL_LINK_TYPED(OSaveAsDlg, EditModifyHdl, Edit&, rEdit, void )
{
    if (&rEdit == m_pImpl->m_pTitle)
        m_pImpl->m_pPB_OK->Enable(!m_pImpl->m_pTitle->GetText().isEmpty());
}

void OSaveAsDlg::implInitOnlyTitle(const OUString& _rLabel)
{
    m_pImpl->m_pLabel->SetText(_rLabel);
    m_pImpl->m_pCatalogLbl->Hide();
    m_pImpl->m_pCatalog->Hide();
    m_pImpl->m_pSchemaLbl->Hide();
    m_pImpl->m_pSchema->Hide();

    m_pImpl->m_pTitle->SetText(m_pImpl->m_aName);
    m_pImpl->m_pTitle->setCheck(false); // enable non valid sql chars as well
}

void OSaveAsDlg::implInit()
{
    if ( 0 == ( m_pImpl->m_nFlags & SAD_ADDITIONAL_DESCRIPTION ) )
    {
        // hide the description window
        m_pImpl->m_pDescription->Hide();
    }

    if ( SAD_TITLE_PASTE_AS == ( m_pImpl->m_nFlags & SAD_TITLE_PASTE_AS ) )
        SetText( ModuleRes( STR_TITLE_PASTE_AS ) );
    else if ( SAD_TITLE_RENAME == ( m_pImpl->m_nFlags & SAD_TITLE_RENAME ) )
        SetText( ModuleRes( STR_TITLE_RENAME ) );

    m_pImpl->m_pPB_OK->SetClickHdl(LINK(this,OSaveAsDlg,ButtonClickHdl));
    m_pImpl->m_pTitle->SetModifyHdl(LINK(this,OSaveAsDlg,EditModifyHdl));
    m_pImpl->m_pTitle->GrabFocus();
}

OUString OSaveAsDlg::getName() const      { return m_pImpl->m_aName; }
OUString OSaveAsDlg::getCatalog() const   { return m_pImpl->m_pCatalog->IsVisible() ? m_pImpl->m_pCatalog->GetText() : OUString(); }
OUString OSaveAsDlg::getSchema() const    { return m_pImpl->m_pSchema->IsVisible() ? m_pImpl->m_pSchema->GetText() : OUString(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
