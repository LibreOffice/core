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

#include <WCPage.hxx>
#include <WCopyTable.hxx>

#include <defaultobjectnamecheck.hxx>
#include <strings.hrc>
#include <core_resource.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>

using namespace ::dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

namespace CopyTableOperation = css::sdb::application::CopyTableOperation;

OCopyTable::OCopyTable(weld::Container* pPage, OCopyTableWizard* pWizard)
    : OWizardPage(pPage, pWizard, u"dbaccess/ui/copytablepage.ui"_ustr, u"CopyTablePage"_ustr)
    , m_bPKeyAllowed(false)
    , m_bUseHeaderAllowed(true)
    , m_nOldOperation(0)
    , m_xEdTableName(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xRB_DefData(m_xBuilder->weld_radio_button(u"defdata"_ustr))
    , m_xRB_Def(m_xBuilder->weld_radio_button(u"def"_ustr))
    , m_xRB_View(m_xBuilder->weld_radio_button(u"view"_ustr))
    , m_xRB_AppendData(m_xBuilder->weld_radio_button(u"data"_ustr))
    , m_xCB_UseHeaderLine(m_xBuilder->weld_check_button(u"firstline"_ustr))
    , m_xCB_PrimaryColumn(m_xBuilder->weld_check_button(u"primarykey"_ustr))
    , m_xFT_KeyName(m_xBuilder->weld_label(u"keynamelabel"_ustr))
    , m_xEdKeyName(m_xBuilder->weld_entry(u"keyname"_ustr))
{
    if ( m_pParent->m_xDestConnection.is() )
    {
        if (!m_pParent->supportsViews())
            m_xRB_View->set_sensitive(false);

        m_xCB_UseHeaderLine->set_active(true);
        m_bPKeyAllowed = m_pParent->supportsPrimaryKey();

        m_xCB_PrimaryColumn->set_sensitive(m_bPKeyAllowed);

        m_xRB_AppendData->connect_toggled(  LINK( this, OCopyTable, RadioChangeHdl ) );
        m_xRB_DefData->connect_toggled(     LINK( this, OCopyTable, RadioChangeHdl ) );
        m_xRB_Def->connect_toggled(         LINK( this, OCopyTable, RadioChangeHdl ) );
        m_xRB_View->connect_toggled(        LINK( this, OCopyTable, RadioChangeHdl ) );

        m_xCB_PrimaryColumn->connect_toggled(LINK( this, OCopyTable, KeyClickHdl ) );

        m_xFT_KeyName->set_sensitive(false);
        m_xEdKeyName->set_sensitive(false);
        m_xEdKeyName->set_text(m_pParent->createUniqueName(u"ID"_ustr));

        const sal_Int32 nMaxLen = m_pParent->getMaxColumnNameLength();
        m_xEdKeyName->set_max_length(nMaxLen);
    }

    SetPageTitle(DBA_RES(STR_COPYTABLE_TITLE_COPY));
}

OCopyTable::~OCopyTable()
{
}

void OCopyTable::SetAppendDataRadio()
{
    m_pParent->EnableNextButton(true);
    m_xFT_KeyName->set_sensitive(false);
    m_xCB_PrimaryColumn->set_sensitive(false);
    m_xEdKeyName->set_sensitive(false);
    m_pParent->setOperation(CopyTableOperation::AppendData);
}

IMPL_LINK(OCopyTable, RadioChangeHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    if (m_xRB_AppendData->get_active())
    {
        SetAppendDataRadio();
        return;
    }
    m_pParent->EnableNextButton(!m_xRB_View->get_active());
    bool bKey = m_bPKeyAllowed && !m_xRB_View->get_active();
    m_xFT_KeyName->set_sensitive(bKey && m_xCB_PrimaryColumn->get_active());
    m_xEdKeyName->set_sensitive(bKey && m_xCB_PrimaryColumn->get_active());
    m_xCB_PrimaryColumn->set_sensitive(bKey);
    m_xCB_UseHeaderLine->set_sensitive(m_bUseHeaderAllowed && IsOptionDefData());

    // set type what to do
    if( IsOptionDefData() )
        m_pParent->setOperation( CopyTableOperation::CopyDefinitionAndData );
    else if( IsOptionDef() )
        m_pParent->setOperation( CopyTableOperation::CopyDefinitionOnly );
    else if( IsOptionView() )
        m_pParent->setOperation( CopyTableOperation::CreateAsView );
}

IMPL_LINK_NOARG( OCopyTable, KeyClickHdl, weld::Toggleable&, void )
{
    m_xEdKeyName->set_sensitive(m_xCB_PrimaryColumn->get_active());
    m_xFT_KeyName->set_sensitive(m_xCB_PrimaryColumn->get_active());
}

bool OCopyTable::LeavePage()
{
    m_pParent->m_bCreatePrimaryKeyColumn    = m_bPKeyAllowed && m_xCB_PrimaryColumn->get_sensitive() && m_xCB_PrimaryColumn->get_active();
    m_pParent->m_aKeyName                   = m_pParent->m_bCreatePrimaryKeyColumn ? m_xEdKeyName->get_text() : OUString();
    m_pParent->setUseHeaderLine( m_xCB_UseHeaderLine->get_active() );

    // first check if the table already exists in the database
    if( m_pParent->getOperation() != CopyTableOperation::AppendData )
    {
        m_pParent->clearDestColumns();
        DynamicTableOrQueryNameCheck aNameCheck( m_pParent->m_xDestConnection, CommandType::TABLE );
        SQLExceptionInfo aErrorInfo;
        if ( !aNameCheck.isNameValid( m_xEdTableName->get_text(), aErrorInfo ) )
        {
            aErrorInfo.append( SQLExceptionInfo::TYPE::SQLContext, DBA_RES( STR_SUGGEST_APPEND_TABLE_DATA ) );
            m_pParent->showError(aErrorInfo.get());

            return false;
        }

        // have to check the length of the table name
        Reference< XDatabaseMetaData > xMeta = m_pParent->m_xDestConnection->getMetaData();
        OUString sCatalog;
        OUString sSchema;
        OUString sTable;
        ::dbtools::qualifiedNameComponents( xMeta,
                                            m_xEdTableName->get_text(),
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::EComposeRule::InDataManipulation);
        sal_Int32 nMaxLength = xMeta->getMaxTableNameLength();
        if ( nMaxLength && sTable.getLength() > nMaxLength )
        {
            m_pParent->showError(DBA_RES(STR_INVALID_TABLE_NAME_LENGTH));
            return false;
        }

        // now we have to check if the name of the primary key already exists
        if (    m_pParent->m_bCreatePrimaryKeyColumn
            &&  m_pParent->m_aKeyName != m_pParent->createUniqueName(m_pParent->m_aKeyName) )
        {
            m_pParent->showError(DBA_RES(STR_WIZ_NAME_ALREADY_DEFINED) + " " + m_pParent->m_aKeyName);
            return false;
        }
    }

    if (m_xEdTableName->get_value_changed_from_saved())
    { // table exists and name has changed
        if ( m_pParent->getOperation() == CopyTableOperation::AppendData )
        {
            if(!checkAppendData())
                return false;
        }
        else if ( m_nOldOperation == CopyTableOperation::AppendData )
        {
            m_xEdTableName->save_value();
            return LeavePage();
        }
    }
    else
    { // table exist and is not new or doesn't exist and so on
        if ( CopyTableOperation::AppendData == m_pParent->getOperation() )
        {
            if( !checkAppendData() )
                return false;
        }
    }
    m_pParent->m_sName = m_xEdTableName->get_text();
    m_xEdTableName->save_value();

    if(m_pParent->m_sName.isEmpty())
    {
        m_pParent->showError(DBA_RES(STR_INVALID_TABLE_NAME));
        return false;
    }

    return true;
}

void OCopyTable::Activate()
{
    m_pParent->GetOKButton().set_sensitive(true);
    m_nOldOperation = m_pParent->getOperation();
    m_xEdTableName->grab_focus();
    m_xCB_UseHeaderLine->set_active(m_pParent->UseHeaderLine());
}

OUString OCopyTable::GetTitle() const
{
    return DBA_RES(STR_WIZ_TABLE_COPY);
}

void OCopyTable::Reset()
{
    m_bFirstTime = false;

    m_xEdTableName->set_text( m_pParent->m_sName );
    m_xEdTableName->save_value();
}

bool OCopyTable::checkAppendData()
{
    m_pParent->clearDestColumns();
    Reference< XPropertySet > xTable;
    Reference< XTablesSupplier > xSup( m_pParent->m_xDestConnection, UNO_QUERY );
    Reference<XNameAccess> xTables;
    if (xSup.is())
        xTables = xSup->getTables();
    if (xTables.is() && xTables->hasByName(m_xEdTableName->get_text()))
    {
        const ODatabaseExport::TColumnVector& rSrcColumns = m_pParent->getSrcVector();
        const sal_uInt32 nSrcSize = rSrcColumns.size();
        m_pParent->m_vColumnPositions.resize( nSrcSize, ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
        m_pParent->m_vColumnTypes.resize( nSrcSize , COLUMN_POSITION_NOT_FOUND );

        // set new destination
        xTables->getByName( m_xEdTableName->get_text() ) >>= xTable;
        ObjectCopySource aTableCopySource( m_pParent->m_xDestConnection, xTable );
        m_pParent->loadData( aTableCopySource, m_pParent->m_vDestColumns, m_pParent->m_aDestVec );
        const ODatabaseExport::TColumnVector& rDestColumns          = m_pParent->getDestVector();
        const sal_uInt32 nMinSrcDestSize = std::min<sal_uInt32>(nSrcSize, rDestColumns.size());
        sal_uInt32 i = 0;
        for (auto const& column : rDestColumns)
        {
            if (i >= nMinSrcDestSize)
                break;
            bool bNotConvert = true;
            m_pParent->m_vColumnPositions[i] = ODatabaseExport::TPositions::value_type(i+1,i+1);
            TOTypeInfoSP pTypeInfo = m_pParent->convertType(column->second->getSpecialTypeInfo(),bNotConvert);
            if ( !bNotConvert )
            {
                m_pParent->showColumnTypeNotSupported(column->first);
                return false;
            }

            if ( pTypeInfo )
                m_pParent->m_vColumnTypes[i] = pTypeInfo->nType;
            else
                m_pParent->m_vColumnTypes[i] = DataType::VARCHAR;
            ++i;
        }

    }

    if ( !xTable.is() )
    {
        m_pParent->showError(DBA_RES(STR_INVALID_TABLE_NAME));
        return false;
    }
    return true;
}

void OCopyTable::setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName )
{
    bool bCreatePK = m_bPKeyAllowed && _bDoCreate;
    m_xCB_PrimaryColumn->set_active( bCreatePK );
    m_xEdKeyName->set_text( _rSuggestedName );

    m_xFT_KeyName->set_sensitive( bCreatePK );
    m_xEdKeyName->set_sensitive( bCreatePK );
}

void OCopyTable::setCreateStyleAction()
{
    // reselect the last action before
    switch (m_pParent->getOperation())
    {
        case CopyTableOperation::CopyDefinitionAndData:
            m_xRB_DefData->set_active(true);
            RadioChangeHdl(*m_xRB_DefData);
            break;
        case CopyTableOperation::CopyDefinitionOnly:
            m_xRB_Def->set_active(true);
            RadioChangeHdl(*m_xRB_Def);
            break;
        case CopyTableOperation::AppendData:
            m_xRB_AppendData->set_active(true);
            SetAppendDataRadio();
            break;
        case CopyTableOperation::CreateAsView:
            if (m_xRB_View->get_sensitive())
            {
                m_xRB_View->set_active(true);
                RadioChangeHdl(*m_xRB_View);
            }
            else
            {
                m_xRB_DefData->set_active(true);
                RadioChangeHdl(*m_xRB_DefData);
            }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
