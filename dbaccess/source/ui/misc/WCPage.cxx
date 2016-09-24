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

#include "WCPage.hxx"
#include "WCopyTable.hxx"
#include "WColumnSelect.hxx"
#include "WExtendPages.hxx"

#include "defaultobjectnamecheck.hxx"
#include <tools/debug.hxx>
#include "dbaccess_helpid.hrc"
#include "dbu_misc.hrc"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include <vcl/msgbox.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include "UITools.hxx"
#include "moduledbu.hxx"
#include <cppuhelper/exc_hlp.hxx>

using namespace ::dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

namespace CopyTableOperation = css::sdb::application::CopyTableOperation;

// Klasse OCopyTable
OCopyTable::OCopyTable(vcl::Window * pParent)
    : OWizardPage(pParent, "CopyTablePage", "dbaccess/ui/copytablepage.ui")
    , m_nOldOperation(0)
    , m_pPage2(nullptr)
    , m_pPage3(nullptr)
    , m_bPKeyAllowed(false)
    , m_bUseHeaderAllowed(true)
{
    get(m_pEdTableName, "name");
    get(m_pRB_DefData, "defdata");
    get(m_pRB_Def, "def");
    get(m_pRB_View, "view");
    get(m_pRB_AppendData, "data");
    get(m_pCB_UseHeaderLine, "firstline");
    get(m_pCB_PrimaryColumn, "primarykey");
    get(m_pFT_KeyName, "keynamelabel");
    get(m_pEdKeyName, "keyname");

    m_pEdTableName->SetMaxTextLen(EDIT_NOLIMIT);

    if ( m_pParent->m_xDestConnection.is() )
    {
        if ( !m_pParent->supportsViews() )
            m_pRB_View->Disable();

        m_pCB_UseHeaderLine->Check();
        m_bPKeyAllowed = m_pParent->supportsPrimaryKey();

        m_pCB_PrimaryColumn->Enable(m_bPKeyAllowed);

        m_pRB_AppendData->SetClickHdl(   LINK( this, OCopyTable, AppendDataClickHdl  ) );

        m_pRB_DefData->SetClickHdl(      LINK( this, OCopyTable, RadioChangeHdl      ) );
        m_pRB_Def->SetClickHdl(          LINK( this, OCopyTable, RadioChangeHdl      ) );
        m_pRB_View->SetClickHdl(         LINK( this, OCopyTable, RadioChangeHdl      ) );

        m_pCB_PrimaryColumn->SetClickHdl(LINK( this, OCopyTable, KeyClickHdl         ) );

        m_pFT_KeyName->Enable(false);
        m_pEdKeyName->Enable(false);
        m_pEdKeyName->SetText(m_pParent->createUniqueName("ID"));

        const sal_Int32 nMaxLen = m_pParent->getMaxColumnNameLength();
        m_pEdKeyName->SetMaxTextLen(nMaxLen ? nMaxLen : EDIT_NOLIMIT);
    }

    SetText(ModuleRes(STR_COPYTABLE_TITLE_COPY));
}

OCopyTable::~OCopyTable()
{
    disposeOnce();
}

void OCopyTable::dispose()
{
    m_pEdTableName.clear();
    m_pRB_DefData.clear();
    m_pRB_Def.clear();
    m_pRB_View.clear();
    m_pRB_AppendData.clear();
    m_pCB_UseHeaderLine.clear();
    m_pCB_PrimaryColumn.clear();
    m_pFT_KeyName.clear();
    m_pEdKeyName.clear();
    m_pPage2.clear();
    m_pPage3.clear();
    OWizardPage::dispose();
}

IMPL_LINK_NOARG_TYPED( OCopyTable, AppendDataClickHdl, Button*, void )
{
    SetAppendDataRadio();
}

void OCopyTable::SetAppendDataRadio()
{
    m_pParent->EnableNextButton(true);
    m_pFT_KeyName->Enable(false);
    m_pCB_PrimaryColumn->Enable(false);
    m_pEdKeyName->Enable(false);
    m_pParent->setOperation(CopyTableOperation::AppendData);
}

IMPL_LINK_TYPED( OCopyTable, RadioChangeHdl, Button*, pButton, void )
{
    m_pParent->EnableNextButton(pButton != m_pRB_View);
    bool bKey = m_bPKeyAllowed && pButton != m_pRB_View;
    m_pFT_KeyName->Enable(bKey && m_pCB_PrimaryColumn->IsChecked());
    m_pEdKeyName->Enable(bKey && m_pCB_PrimaryColumn->IsChecked());
    m_pCB_PrimaryColumn->Enable(bKey);
    m_pCB_UseHeaderLine->Enable(m_bUseHeaderAllowed && IsOptionDefData());

    // set typ what to do
    if( IsOptionDefData() )
        m_pParent->setOperation( CopyTableOperation::CopyDefinitionAndData );
    else if( IsOptionDef() )
        m_pParent->setOperation( CopyTableOperation::CopyDefinitionOnly );
    else if( IsOptionView() )
        m_pParent->setOperation( CopyTableOperation::CreateAsView );
}

IMPL_LINK_NOARG_TYPED( OCopyTable, KeyClickHdl, Button*, void )
{
    m_pEdKeyName->Enable(m_pCB_PrimaryColumn->IsChecked());
    m_pFT_KeyName->Enable(m_pCB_PrimaryColumn->IsChecked());
}

bool OCopyTable::LeavePage()
{
    m_pParent->m_bCreatePrimaryKeyColumn    = m_bPKeyAllowed && m_pCB_PrimaryColumn->IsEnabled() && m_pCB_PrimaryColumn->IsChecked();
    m_pParent->m_aKeyName                   = m_pParent->m_bCreatePrimaryKeyColumn ? m_pEdKeyName->GetText() : OUString();
    m_pParent->setUseHeaderLine( m_pCB_UseHeaderLine->IsChecked() );

    // first check if the table already exists in the database
    if( m_pParent->getOperation() != CopyTableOperation::AppendData )
    {
        m_pParent->clearDestColumns();
        DynamicTableOrQueryNameCheck aNameCheck( m_pParent->m_xDestConnection, CommandType::TABLE );
        SQLExceptionInfo aErrorInfo;
        if ( !aNameCheck.isNameValid( m_pEdTableName->GetText(), aErrorInfo ) )
        {
            aErrorInfo.append( SQLExceptionInfo::TYPE::SQLContext, ModuleRes( STR_SUGGEST_APPEND_TABLE_DATA ) );
            m_pParent->showError(aErrorInfo.get());

            return false;
        }

        // have to check the length of the table name
        Reference< XDatabaseMetaData > xMeta = m_pParent->m_xDestConnection->getMetaData();
        OUString sCatalog;
        OUString sSchema;
        OUString sTable;
        ::dbtools::qualifiedNameComponents( xMeta,
                                            m_pEdTableName->GetText(),
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::EComposeRule::InDataManipulation);
        sal_Int32 nMaxLength = xMeta->getMaxTableNameLength();
        if ( nMaxLength && sTable.getLength() > nMaxLength )
        {
            m_pParent->showError(ModuleRes(STR_INVALID_TABLE_NAME_LENGTH));
            return false;
        }

        // now we have to check if the name of the primary key already exists
        if (    m_pParent->m_bCreatePrimaryKeyColumn
            &&  m_pParent->m_aKeyName != m_pParent->createUniqueName(m_pParent->m_aKeyName) )
        {
            m_pParent->showError(ModuleRes(STR_WIZ_NAME_ALREADY_DEFINED).toString()+" "+m_pParent->m_aKeyName);
            return false;
        }
    }

    if ( m_pEdTableName->IsValueChangedFromSaved() )
    { // table exists and name has changed
        if ( m_pParent->getOperation() == CopyTableOperation::AppendData )
        {
            if(!checkAppendData())
                return false;
        }
        else if ( m_nOldOperation == CopyTableOperation::AppendData )
        {
            m_pEdTableName->SaveValue();
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
    m_pParent->m_sName = m_pEdTableName->GetText();
    m_pEdTableName->SaveValue();

    if(m_pParent->m_sName.isEmpty())
    {
        m_pParent->showError(ModuleRes(STR_INVALID_TABLE_NAME));
        return false;
    }

    return true;
}

void OCopyTable::ActivatePage()
{
    m_pParent->GetOKButton().Enable();
    m_nOldOperation = m_pParent->getOperation();
    m_pEdTableName->GrabFocus();
    m_pCB_UseHeaderLine->Check(m_pParent->UseHeaderLine());
}

OUString OCopyTable::GetTitle() const
{
    return ModuleRes(STR_WIZ_TABLE_COPY);
}

void OCopyTable::Reset()
{
    m_bFirstTime = false;

    m_pEdTableName->SetText( m_pParent->m_sName );
    m_pEdTableName->SaveValue();
}

bool OCopyTable::checkAppendData()
{
    m_pParent->clearDestColumns();
    Reference< XPropertySet > xTable;
    Reference< XTablesSupplier > xSup( m_pParent->m_xDestConnection, UNO_QUERY );
    Reference<XNameAccess> xTables;
    if(xSup.is())
        xTables = xSup->getTables();
    if(xTables.is() && xTables->hasByName(m_pEdTableName->GetText()))
    {
        const ODatabaseExport::TColumnVector& rSrcColumns = m_pParent->getSrcVector();
        const sal_uInt32 nSrcSize = rSrcColumns.size();
        m_pParent->m_vColumnPos.resize( nSrcSize, ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
        m_pParent->m_vColumnTypes.resize( nSrcSize , COLUMN_POSITION_NOT_FOUND );

        // set new destination
        xTables->getByName( m_pEdTableName->GetText() ) >>= xTable;
        ObjectCopySource aTableCopySource( m_pParent->m_xDestConnection, xTable );
        m_pParent->loadData( aTableCopySource, m_pParent->m_vDestColumns, m_pParent->m_aDestVec );
        const ODatabaseExport::TColumnVector& rDestColumns          = m_pParent->getDestVector();
        ODatabaseExport::TColumnVector::const_iterator aDestIter    = rDestColumns.begin();
        ODatabaseExport::TColumnVector::const_iterator aDestEnd     = rDestColumns.end();
        const sal_uInt32 nDestSize = rDestColumns.size();
        sal_uInt32 i = 0;
        for(sal_Int32 nPos = 1;aDestIter != aDestEnd && i < nDestSize && i < nSrcSize;++aDestIter,++nPos,++i)
        {
            bool bNotConvert = true;
            m_pParent->m_vColumnPos[i] = ODatabaseExport::TPositions::value_type(nPos,nPos);
            TOTypeInfoSP pTypeInfo = m_pParent->convertType((*aDestIter)->second->getSpecialTypeInfo(),bNotConvert);
            if ( !bNotConvert )
            {
                m_pParent->showColumnTypeNotSupported((*aDestIter)->first);
                return false;
            }

            if ( pTypeInfo.get() )
                m_pParent->m_vColumnTypes[i] = pTypeInfo->nType;
            else
                m_pParent->m_vColumnTypes[i] = DataType::VARCHAR;
        }

    }

    if ( !xTable.is() )
    {
        m_pParent->showError(ModuleRes(STR_INVALID_TABLE_NAME));
        return false;
    }
    return true;
}

void OCopyTable::setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName )
{
    bool bCreatePK = m_bPKeyAllowed && _bDoCreate;
    m_pCB_PrimaryColumn->Check( bCreatePK );
    m_pEdKeyName->SetText( _rSuggestedName );

    m_pFT_KeyName->Enable( bCreatePK );
    m_pEdKeyName->Enable( bCreatePK );
}

void OCopyTable::setCreateStyleAction()
{
    // reselect the last action before
    switch(m_pParent->getOperation())
    {
        case CopyTableOperation::CopyDefinitionAndData:
            m_pRB_DefData->Check();
            RadioChangeHdl(m_pRB_DefData);
            break;
        case CopyTableOperation::CopyDefinitionOnly:
            m_pRB_Def->Check();
            RadioChangeHdl(m_pRB_Def);
            break;
        case CopyTableOperation::AppendData:
            m_pRB_AppendData->Check();
            SetAppendDataRadio();
            break;
        case CopyTableOperation::CreateAsView:
            if ( m_pRB_View->IsEnabled() )
            {
                m_pRB_View->Check();
                RadioChangeHdl(m_pRB_View);
            }
            else
            {
                m_pRB_DefData->Check();
                RadioChangeHdl(m_pRB_DefData);
            }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
