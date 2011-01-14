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

#include "WCPage.hxx"
#include "WCopyTable.hxx"

#ifndef DBACCESS_SOURCE_UI_MISC_DEFAULTOBJECTNAMECHECK_HXX
#include "defaultobjectnamecheck.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef DBAUI_WIZARD_PAGES_HRC
#include "WizardPages.hrc"
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_APPLICATION_COPYTABLEOPERATION_HPP_
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif

using namespace ::dbaui;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

namespace CopyTableOperation = ::com::sun::star::sdb::application::CopyTableOperation;

//========================================================================
// Klasse OCopyTable
//========================================================================
DBG_NAME(OCopyTable)
//------------------------------------------------------------------------
OCopyTable::OCopyTable( Window * pParent )
    :OWizardPage( pParent, ModuleRes(TAB_WIZ_COPYTABLE) )
    ,m_ftTableName(         this, ModuleRes( FT_TABLENAME       ) )
    ,m_edTableName(         this, ModuleRes( ET_TABLENAME       ) )
    ,m_aFL_Options(         this, ModuleRes( FL_OPTIONS         ) )
    ,m_aRB_DefData(         this, ModuleRes( RB_DEFDATA         ) )
    ,m_aRB_Def(             this, ModuleRes( RB_DEF             ) )
    ,m_aRB_View(            this, ModuleRes( RB_VIEW            ) )
    ,m_aRB_AppendData(      this, ModuleRes( RB_APPENDDATA      ) )
    ,m_aCB_UseHeaderLine(   this, ModuleRes( CB_USEHEADERLINE   ) )
    ,m_aCB_PrimaryColumn(   this, ModuleRes( CB_PRIMARY_COLUMN  ) )
    ,m_aFT_KeyName(         this, ModuleRes( FT_KEYNAME         ) )
    ,m_edKeyName(           this, ModuleRes( ET_KEYNAME         ) )
    ,m_pPage2(NULL)
    ,m_pPage3(NULL)
    ,m_bUseHeaderAllowed(sal_True)
{
    DBG_CTOR(OCopyTable,NULL);

    m_edTableName.SetMaxTextLen( EDIT_NOLIMIT );

    if ( m_pParent->m_xDestConnection.is() )
    {
        if ( !m_pParent->supportsViews() )
            m_aRB_View.Disable();

        m_aCB_UseHeaderLine.Check(sal_True);
        m_bPKeyAllowed = m_pParent->supportsPrimaryKey();

        m_aCB_PrimaryColumn.Enable(m_bPKeyAllowed);

        m_aRB_AppendData.SetClickHdl(   LINK( this, OCopyTable, AppendDataClickHdl  ) );

        m_aRB_DefData.SetClickHdl(      LINK( this, OCopyTable, RadioChangeHdl      ) );
        m_aRB_Def.SetClickHdl(          LINK( this, OCopyTable, RadioChangeHdl      ) );
        m_aRB_View.SetClickHdl(         LINK( this, OCopyTable, RadioChangeHdl      ) );

        m_aCB_PrimaryColumn.SetClickHdl(LINK( this, OCopyTable, KeyClickHdl         ) );

        m_aFT_KeyName.Enable(sal_False);
        m_edKeyName.Enable(sal_False);
        ::rtl::OUString sKeyName(RTL_CONSTASCII_USTRINGPARAM("ID"));
        sKeyName = m_pParent->createUniqueName(sKeyName);
        m_edKeyName.SetText(sKeyName);

        sal_Int32 nMaxLen = m_pParent->getMaxColumnNameLength();
        m_edKeyName.SetMaxTextLen(nMaxLen ? (xub_StrLen)nMaxLen : EDIT_NOLIMIT);
    }

    FreeResource();

    SetText(String(ModuleRes(STR_COPYTABLE_TITLE_COPY)));
}

//------------------------------------------------------------------------
OCopyTable::~OCopyTable()
{
    DBG_DTOR(OCopyTable,NULL);
}
//------------------------------------------------------------------------
IMPL_LINK( OCopyTable, AppendDataClickHdl, Button*, /*pButton*/ )
{
    DBG_CHKTHIS(OCopyTable,NULL);

    SetAppendDataRadio();
    return 0;
}
//--------dyf ADD
void OCopyTable::SetAppendDataRadio()
{
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,sal_True);
    m_aFT_KeyName.Enable(sal_False);
    m_aCB_PrimaryColumn.Enable(sal_False);
    m_edKeyName.Enable(sal_False);
    m_pParent->setOperation(CopyTableOperation::AppendData);
}

//--------add end
//------------------------------------------------------------------------
IMPL_LINK( OCopyTable, RadioChangeHdl, Button*, pButton )
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->EnableButton(OCopyTableWizard::WIZARD_NEXT,pButton != &m_aRB_View);
    sal_Bool bKey = m_bPKeyAllowed && pButton != &m_aRB_View;
    m_aFT_KeyName.Enable(bKey && m_aCB_PrimaryColumn.IsChecked());
    m_edKeyName.Enable(bKey && m_aCB_PrimaryColumn.IsChecked());
    m_aCB_PrimaryColumn.Enable(bKey);
    m_aCB_UseHeaderLine.Enable(m_bUseHeaderAllowed && IsOptionDefData());

    // set typ what to do
    if( IsOptionDefData() )
        m_pParent->setOperation( CopyTableOperation::CopyDefinitionAndData );
    else if( IsOptionDef() )
        m_pParent->setOperation( CopyTableOperation::CopyDefinitionOnly );
    else if( IsOptionView() )
        m_pParent->setOperation( CopyTableOperation::CreateAsView );

    return 0;
}
//------------------------------------------------------------------------
IMPL_LINK( OCopyTable, KeyClickHdl, Button*, /*pButton*/ )
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_edKeyName.Enable(m_aCB_PrimaryColumn.IsChecked());
    m_aFT_KeyName.Enable(m_aCB_PrimaryColumn.IsChecked());
    return 0;
}
//------------------------------------------------------------------------
sal_Bool OCopyTable::LeavePage()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->m_bCreatePrimaryKeyColumn    = (m_bPKeyAllowed && m_aCB_PrimaryColumn.IsEnabled()) ? m_aCB_PrimaryColumn.IsChecked() : sal_False;
    m_pParent->m_aKeyName                   = m_pParent->m_bCreatePrimaryKeyColumn ? m_edKeyName.GetText() : String();
    m_pParent->setUseHeaderLine( m_aCB_UseHeaderLine.IsChecked() );

    // first check if the table already exists in the database
    if( m_pParent->getOperation() != CopyTableOperation::AppendData )
    {
        m_pParent->clearDestColumns();
        DynamicTableOrQueryNameCheck aNameCheck( m_pParent->m_xDestConnection, CommandType::TABLE );
        SQLExceptionInfo aErrorInfo;
        if ( !aNameCheck.isNameValid( m_edTableName.GetText(), aErrorInfo ) )
        {
            aErrorInfo.append( SQLExceptionInfo::SQL_CONTEXT, String( ModuleRes( STR_SUGGEST_APPEND_TABLE_DATA ) ) );
            m_pParent->showError(aErrorInfo.get());

            return sal_False;
        }

        // have to check the length of the table name
        Reference< XDatabaseMetaData > xMeta = m_pParent->m_xDestConnection->getMetaData();
        ::rtl::OUString sCatalog;
        ::rtl::OUString sSchema;
        ::rtl::OUString sTable;
        ::dbtools::qualifiedNameComponents( xMeta,
                                            m_edTableName.GetText(),
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::eInDataManipulation);
        sal_Int32 nMaxLength = xMeta->getMaxTableNameLength();
        if ( nMaxLength && sTable.getLength() > nMaxLength )
        {
            String sError(ModuleRes(STR_INVALID_TABLE_NAME_LENGTH));
            m_pParent->showError(sError);
            return sal_False;
        }

        // now we have to check if the name of the primary key already exists
        if (    m_pParent->m_bCreatePrimaryKeyColumn
            &&  m_pParent->m_aKeyName != m_pParent->createUniqueName(m_pParent->m_aKeyName) )
        {
            String aInfoString( ModuleRes(STR_WIZ_PKEY_ALREADY_DEFINED) );
            aInfoString += String(' ');
            aInfoString += String(m_pParent->m_aKeyName);
            m_pParent->showError(aInfoString);
            return sal_False;
        }
    }

    if ( !m_edTableName.GetSavedValue().Equals(m_edTableName.GetText()) )
    { // table exists and name has changed
        if ( m_pParent->getOperation() == CopyTableOperation::AppendData )
        {
            if(!checkAppendData())
                return sal_False;
        }
        else if ( m_nOldOperation == CopyTableOperation::AppendData )
        {
            m_edTableName.SaveValue();
            return LeavePage();
        }
    }
    else
    { // table exist and is not new or doesn't exist and so on
        if ( CopyTableOperation::AppendData == m_pParent->getOperation() )
        {
            if( !checkAppendData() )
                return sal_False;
        }
    }
    m_pParent->m_sName = m_edTableName.GetText();
    m_edTableName.SaveValue();

    if(!m_pParent->m_sName.getLength())
    {
        String sError(ModuleRes(STR_INVALID_TABLE_NAME));
        m_pParent->showError(sError);
        return sal_False;
    }

    return sal_True;
}
//------------------------------------------------------------------------
void OCopyTable::ActivatePage()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->GetOKButton().Enable( sal_True );
    m_nOldOperation = m_pParent->getOperation();
    m_edTableName.GrabFocus();
    m_aCB_UseHeaderLine.Check(m_pParent->UseHeaderLine());
}
//------------------------------------------------------------------------
String OCopyTable::GetTitle() const
{
    DBG_CHKTHIS(OCopyTable,NULL);
    return String(ModuleRes(STR_WIZ_TABLE_COPY));
}
//------------------------------------------------------------------------
void OCopyTable::Reset()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_bFirstTime = sal_False;

    m_edTableName.SetText( m_pParent->m_sName );
    m_edTableName.SaveValue();
}
//------------------------------------------------------------------------
sal_Bool OCopyTable::checkAppendData()
{
    DBG_CHKTHIS(OCopyTable,NULL);
    m_pParent->clearDestColumns();
    Reference< XPropertySet > xTable;
    Reference< XTablesSupplier > xSup( m_pParent->m_xDestConnection, UNO_QUERY );
    Reference<XNameAccess> xTables;
    if(xSup.is())
        xTables = xSup->getTables();
    if(xTables.is() && xTables->hasByName(m_edTableName.GetText()))
    {
        const ODatabaseExport::TColumnVector* pSrcColumns = m_pParent->getSrcVector();
        const sal_uInt32 nSrcSize = pSrcColumns->size();
        m_pParent->m_vColumnPos.resize( nSrcSize, ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
        m_pParent->m_vColumnTypes.resize( nSrcSize , COLUMN_POSITION_NOT_FOUND );

        // set new destination
        xTables->getByName( m_edTableName.GetText() ) >>= xTable;
        ObjectCopySource aTableCopySource( m_pParent->m_xDestConnection, xTable );
        m_pParent->loadData( aTableCopySource, m_pParent->m_vDestColumns, m_pParent->m_aDestVec );
        // #90027#
        const ODatabaseExport::TColumnVector* pDestColumns          = m_pParent->getDestVector();
        ODatabaseExport::TColumnVector::const_iterator aDestIter    = pDestColumns->begin();
        ODatabaseExport::TColumnVector::const_iterator aDestEnd     = pDestColumns->end();
        const sal_uInt32 nDestSize = pDestColumns->size();
        sal_Bool bNotConvert;
        sal_uInt32 i = 0;
        for(sal_Int32 nPos = 1;aDestIter != aDestEnd && i < nDestSize && i < nSrcSize;++aDestIter,++nPos,++i)
        {
            bNotConvert = sal_True;
            m_pParent->m_vColumnPos[i] = ODatabaseExport::TPositions::value_type(nPos,nPos);
            TOTypeInfoSP pTypeInfo = m_pParent->convertType((*aDestIter)->second->getSpecialTypeInfo(),bNotConvert);
            if ( !bNotConvert )
            {
                m_pParent->showColumnTypeNotSupported((*aDestIter)->first);
                return sal_False;
            }

            if ( pTypeInfo.get() )
                m_pParent->m_vColumnTypes[i] = pTypeInfo->nType;
            else
                m_pParent->m_vColumnTypes[i] = DataType::VARCHAR;
        }

    }

    if ( !xTable.is() )
    {
        String sError(ModuleRes(STR_INVALID_TABLE_NAME));
        m_pParent->showError(sError);
        return sal_False;
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
void OCopyTable::setCreatePrimaryKey( bool _bDoCreate, const ::rtl::OUString& _rSuggestedName )
{
    bool bCreatePK = m_bPKeyAllowed && _bDoCreate;
    m_aCB_PrimaryColumn.Check( bCreatePK );
    m_edKeyName.SetText( _rSuggestedName );

    m_aFT_KeyName.Enable( bCreatePK );
    m_edKeyName.Enable( bCreatePK );
}

// -----------------------------------------------------------------------------
//---dyf add 2006/7/10
void OCopyTable::setCreateStyleAction()
{
    // reselect the last action before
    switch(m_pParent->getOperation())
    {
        case CopyTableOperation::CopyDefinitionAndData:
            m_aRB_DefData.Check(sal_True);
            RadioChangeHdl(&m_aRB_DefData);
            break;
        case CopyTableOperation::CopyDefinitionOnly:
            m_aRB_Def.Check(sal_True);
            RadioChangeHdl(&m_aRB_Def);
            break;
        case CopyTableOperation::AppendData:
            m_aRB_AppendData.Check(sal_True);
            SetAppendDataRadio();
            break;
        case CopyTableOperation::CreateAsView:
            if ( m_aRB_View.IsEnabled() )
            {
                m_aRB_View.Check(sal_True);
                RadioChangeHdl(&m_aRB_View);
            }
            else
            {
                m_aRB_DefData.Check(sal_True);
                RadioChangeHdl(&m_aRB_DefData);
            }
    }
}
//---add end
