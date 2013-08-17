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


#include "dbu_misc.hrc"
#include "dbustrings.hrc"
#include "moduledbu.hxx"
#include "sqlmessage.hxx"
#include "UITools.hxx"
#include "WColumnSelect.hxx"
#include "WCopyTable.hxx"
#include "WCPage.hxx"
#include "WExtendPages.hxx"
#include "WizardPages.hrc"
#include "WNameMatch.hxx"
#include "WTypeSelect.hxx"

#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/types.hxx>
#include <comphelper/interaction.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbexception.hxx>

#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

#include <functional>
#include <o3tl/compat_functional.hxx>

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace dbtools;

namespace CopyTableOperation = ::com::sun::star::sdb::application::CopyTableOperation;

#define MAX_PAGES   4   // max. number of pages, which are shown

DBG_NAME(OCopyTableWizard)
namespace
{
    //....................................................................
    void clearColumns(ODatabaseExport::TColumns& _rColumns, ODatabaseExport::TColumnVector& _rColumnsVec)
    {
        ODatabaseExport::TColumns::iterator aIter = _rColumns.begin();
        ODatabaseExport::TColumns::iterator aEnd  = _rColumns.end();

        for(;aIter != aEnd;++aIter)
            delete aIter->second;

        _rColumnsVec.clear();
        _rColumns.clear();
    }
}

//========================================================================
//= ICopyTableSourceObject
//========================================================================
//------------------------------------------------------------------------
ICopyTableSourceObject::~ICopyTableSourceObject()
{
}

//========================================================================
//= ObjectCopySource
//========================================================================
//------------------------------------------------------------------------
ObjectCopySource::ObjectCopySource( const Reference< XConnection >& _rxConnection, const Reference< XPropertySet >& _rxObject )
    :m_xConnection( _rxConnection, UNO_SET_THROW )
    ,m_xMetaData( _rxConnection->getMetaData(), UNO_SET_THROW )
    ,m_xObject( _rxObject, UNO_SET_THROW )
    ,m_xObjectPSI( _rxObject->getPropertySetInfo(), UNO_SET_THROW )
    ,m_xObjectColumns( Reference< XColumnsSupplier >( _rxObject, UNO_QUERY_THROW )->getColumns(), UNO_SET_THROW )
{
}

//------------------------------------------------------------------------
OUString ObjectCopySource::getQualifiedObjectName() const
{
    OUString sName;

    if ( !m_xObjectPSI->hasPropertyByName( PROPERTY_COMMAND ) )
        sName = ::dbtools::composeTableName( m_xMetaData, m_xObject, ::dbtools::eInDataManipulation, false, false, false );
    else
        m_xObject->getPropertyValue( PROPERTY_NAME ) >>= sName;
    return sName;
}

//------------------------------------------------------------------------
bool ObjectCopySource::isView() const
{
    bool bIsView = false;
    try
    {
        if ( m_xObjectPSI->hasPropertyByName( PROPERTY_TYPE ) )
        {
            OUString sObjectType;
            OSL_VERIFY( m_xObject->getPropertyValue( PROPERTY_TYPE ) >>= sObjectType );
            bIsView = sObjectType == "VIEW";
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bIsView;
}

//------------------------------------------------------------------------
void ObjectCopySource::copyUISettingsTo( const Reference< XPropertySet >& _rxObject ) const
{
    const OUString aCopyProperties[] = {
        OUString(PROPERTY_FONT), OUString(PROPERTY_ROW_HEIGHT), OUString(PROPERTY_TEXTCOLOR),OUString(PROPERTY_TEXTLINECOLOR),OUString(PROPERTY_TEXTEMPHASIS),OUString(PROPERTY_TEXTRELIEF)
    };
    for ( size_t i=0; i < sizeof( aCopyProperties ) / sizeof( aCopyProperties[0] ); ++i )
    {
        if ( m_xObjectPSI->hasPropertyByName( aCopyProperties[i] ) )
            _rxObject->setPropertyValue( aCopyProperties[i], m_xObject->getPropertyValue( aCopyProperties[i] ) );
    }
}
//------------------------------------------------------------------------
void ObjectCopySource::copyFilterAndSortingTo( const Reference< XConnection >& _xConnection,const Reference< XPropertySet >& _rxObject ) const
{
    ::std::pair< OUString, OUString > aProperties[] = {
                 ::std::pair< OUString, OUString >(PROPERTY_FILTER,OUString(" AND "))
                ,::std::pair< OUString, OUString >(PROPERTY_ORDER,OUString(" ORDER BY "))
    };

    try
    {
        const String sSourceName = (::dbtools::composeTableNameForSelect(m_xConnection,m_xObject) + OUString("."));
        const OUString sTargetName = ::dbtools::composeTableNameForSelect(_xConnection,_rxObject);
        const String sTargetNameTemp = (sTargetName + OUString("."));

        OUString sStatement("SELECT * FROM ");
        sStatement += sTargetName;
        sStatement += OUString(" WHERE 0=1");


        for ( size_t i=0; i < SAL_N_ELEMENTS(aProperties); ++i )
        {
            if ( m_xObjectPSI->hasPropertyByName( aProperties[i].first ) )
            {
                OUString sFilter;
                m_xObject->getPropertyValue( aProperties[i].first ) >>= sFilter;
                if ( !sFilter.isEmpty() )
                {
                    sStatement += aProperties[i].second;
                    String sReplace = sFilter;
                    sReplace.SearchAndReplace(sSourceName,sTargetNameTemp);
                    sFilter = sReplace;
                    _rxObject->setPropertyValue( aProperties[i].first, makeAny(sFilter) );
                    sStatement += sFilter;
                }
            }
        }

        _xConnection->createStatement()->executeQuery(sStatement);

        if ( m_xObjectPSI->hasPropertyByName( PROPERTY_APPLYFILTER ) )
            _rxObject->setPropertyValue( PROPERTY_APPLYFILTER, m_xObject->getPropertyValue( PROPERTY_APPLYFILTER ) );
    }
    catch(Exception&)
    {
    }
}
//------------------------------------------------------------------------
Sequence< OUString > ObjectCopySource::getColumnNames() const
{
    return m_xObjectColumns->getElementNames();
}

//------------------------------------------------------------------------
Sequence< OUString > ObjectCopySource::getPrimaryKeyColumnNames() const
{
    const Reference<XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(m_xObject);
    Sequence< OUString > aKeyColNames;
    if ( xPrimaryKeyColumns.is() )
        aKeyColNames = xPrimaryKeyColumns->getElementNames();
    return aKeyColNames;
}

//------------------------------------------------------------------------
OFieldDescription* ObjectCopySource::createFieldDescription( const OUString& _rColumnName ) const
{
    Reference< XPropertySet > xColumn( m_xObjectColumns->getByName( _rColumnName ), UNO_QUERY_THROW );
    return new OFieldDescription( xColumn );
}
//------------------------------------------------------------------------
OUString ObjectCopySource::getSelectStatement() const
{
    OUString sSelectStatement;
    if ( m_xObjectPSI->hasPropertyByName( PROPERTY_COMMAND ) )
    {   // query
        OSL_VERIFY( m_xObject->getPropertyValue( PROPERTY_COMMAND ) >>= sSelectStatement );
    }
    else
    {   // table
        OUStringBuffer aSQL;
        aSQL.appendAscii( "SELECT " );

        // we need to create the sql stmt with column names
        // otherwise it is possible that names don't match
        const OUString sQuote = m_xMetaData->getIdentifierQuoteString();

        Sequence< OUString > aColumnNames = getColumnNames();
        const OUString* pColumnName = aColumnNames.getConstArray();
        const OUString* pEnd = pColumnName + aColumnNames.getLength();
        for ( ; pColumnName != pEnd; )
        {
            aSQL.append( ::dbtools::quoteName( sQuote, *pColumnName++ ) );

            if ( pColumnName == pEnd )
                aSQL.appendAscii( " " );
            else
                aSQL.appendAscii( ", " );
        }

        aSQL.append( "FROM " + ::dbtools::composeTableNameForSelect( m_xConnection, m_xObject ) );

        sSelectStatement = aSQL.makeStringAndClear();
    }

    return sSelectStatement;
}

//------------------------------------------------------------------------
::utl::SharedUNOComponent< XPreparedStatement > ObjectCopySource::getPreparedSelectStatement() const
{
    ::utl::SharedUNOComponent< XPreparedStatement > xStatement(
        m_xConnection->prepareStatement( getSelectStatement() ),
        ::utl::SharedUNOComponent< XPreparedStatement >::TakeOwnership
    );
    return xStatement;
}

//========================================================================
//= NamedTableCopySource
//========================================================================
//------------------------------------------------------------------------
NamedTableCopySource::NamedTableCopySource( const Reference< XConnection >& _rxConnection, const OUString& _rTableName )
    :m_xConnection( _rxConnection, UNO_SET_THROW )
    ,m_xMetaData( _rxConnection->getMetaData(), UNO_SET_THROW )
    ,m_sTableName( _rTableName )
    ,m_aColumnInfo()
{
    ::dbtools::qualifiedNameComponents( m_xMetaData, m_sTableName, m_sTableCatalog, m_sTableSchema, m_sTableBareName, ::dbtools::eComplete );
    impl_ensureColumnInfo_throw();
}

//------------------------------------------------------------------------
OUString NamedTableCopySource::getQualifiedObjectName() const
{
    return m_sTableName;
}

//------------------------------------------------------------------------
bool NamedTableCopySource::isView() const
{
    OUString sTableType;
    try
    {
        Reference< XResultSet > xTableDesc( m_xMetaData->getTables( makeAny( m_sTableCatalog ), m_sTableSchema, m_sTableBareName,
            Sequence< OUString >() ) );
        Reference< XRow > xTableDescRow( xTableDesc, UNO_QUERY_THROW );
        OSL_VERIFY( xTableDesc->next() );
        sTableType = xTableDescRow->getString( 4 );
        OSL_ENSURE( !xTableDescRow->wasNull(), "NamedTableCopySource::isView: invalid table type!" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return sTableType == "VIEW";
}

//------------------------------------------------------------------------
void NamedTableCopySource::copyUISettingsTo( const Reference< XPropertySet >& /*_rxObject*/ ) const
{
    // not supported: we do not have UI settings to copy
}
// -----------------------------------------------------------------------------
void NamedTableCopySource::copyFilterAndSortingTo( const Reference< XConnection >& ,const Reference< XPropertySet >& /*_rxObject*/ ) const
{
}
//------------------------------------------------------------------------
void NamedTableCopySource::impl_ensureColumnInfo_throw()
{
    if ( !m_aColumnInfo.empty() )
        return;

    Reference< XResultSetMetaDataSupplier > xStatementMetaSupp( impl_ensureStatement_throw().getTyped(), UNO_QUERY_THROW );
    Reference< XResultSetMetaData > xStatementMeta( xStatementMetaSupp->getMetaData(), UNO_SET_THROW );

    sal_Int32 nColCount( xStatementMeta->getColumnCount() );
    for ( sal_Int32 i = 1; i <= nColCount; ++i )
    {
        OFieldDescription aDesc;

        aDesc.SetName(          xStatementMeta->getColumnName(      i ) );
        aDesc.SetHelpText(      xStatementMeta->getColumnLabel(     i ) );
        aDesc.SetTypeValue(     xStatementMeta->getColumnType(      i ) );
        aDesc.SetTypeName(      xStatementMeta->getColumnTypeName(  i ) );
        aDesc.SetPrecision(     xStatementMeta->getPrecision(       i ) );
        aDesc.SetScale(         xStatementMeta->getScale(           i ) );
        aDesc.SetIsNullable(    xStatementMeta->isNullable(         i ) );
        aDesc.SetCurrency(      xStatementMeta->isCurrency(         i ) );
        aDesc.SetAutoIncrement( xStatementMeta->isAutoIncrement(    i ) );

        m_aColumnInfo.push_back( aDesc );
    }
}

//------------------------------------------------------------------------
::utl::SharedUNOComponent< XPreparedStatement > NamedTableCopySource::impl_ensureStatement_throw()
{
    if ( !m_xStatement.is() )
        m_xStatement.set( m_xConnection->prepareStatement( getSelectStatement() ), UNO_SET_THROW );
    return m_xStatement;
}

//------------------------------------------------------------------------
Sequence< OUString > NamedTableCopySource::getColumnNames() const
{
    Sequence< OUString > aNames( m_aColumnInfo.size() );
    for (   ::std::vector< OFieldDescription >::const_iterator col = m_aColumnInfo.begin();
            col != m_aColumnInfo.end();
            ++col
        )
        aNames[ col - m_aColumnInfo.begin() ] = col->GetName();

    return aNames;
}

//------------------------------------------------------------------------
Sequence< OUString > NamedTableCopySource::getPrimaryKeyColumnNames() const
{
    Sequence< OUString > aPKColNames;

    try
    {
        Reference< XResultSet > xPKDesc( m_xMetaData->getPrimaryKeys( makeAny( m_sTableCatalog ), m_sTableSchema, m_sTableBareName ) );
        Reference< XRow > xPKDescRow( xPKDesc, UNO_QUERY_THROW );
        while ( xPKDesc->next() )
        {
            sal_Int32 len( aPKColNames.getLength() );
            aPKColNames.realloc( len + 1 );
            aPKColNames[ len ] = xPKDescRow->getString( 4 );    // COLUMN_NAME
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aPKColNames;
}

//------------------------------------------------------------------------
OFieldDescription* NamedTableCopySource::createFieldDescription( const OUString& _rColumnName ) const
{
    for (   ::std::vector< OFieldDescription >::const_iterator col = m_aColumnInfo.begin();
            col != m_aColumnInfo.end();
            ++col
        )
        if ( col->GetName() == _rColumnName )
            return new OFieldDescription( *col );

    return NULL;
}
//------------------------------------------------------------------------
OUString NamedTableCopySource::getSelectStatement() const
{
    OUStringBuffer aSQL;
    aSQL.appendAscii( "SELECT * FROM " );

    aSQL.append( ::dbtools::composeTableNameForSelect( m_xConnection, m_sTableCatalog, m_sTableSchema, m_sTableBareName ) );

    return aSQL.makeStringAndClear();
}

//------------------------------------------------------------------------
::utl::SharedUNOComponent< XPreparedStatement > NamedTableCopySource::getPreparedSelectStatement() const
{
    return const_cast< NamedTableCopySource* >( this )->impl_ensureStatement_throw();
}

// ========================================================
// DummyCopySource
// ========================================================
class DummyCopySource : public ICopyTableSourceObject
{
public:
    DummyCopySource() { }

    static const DummyCopySource& Instance();

    // ICopyTableSourceObject overridables
    virtual OUString     getQualifiedObjectName() const;
    virtual bool                isView() const;
    virtual void                copyUISettingsTo( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject ) const;
    virtual void                copyFilterAndSortingTo(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject ) const;
    virtual ::com::sun::star::uno::Sequence< OUString >
                                getColumnNames() const;
    virtual ::com::sun::star::uno::Sequence< OUString >
                                getPrimaryKeyColumnNames() const;
    virtual OFieldDescription*  createFieldDescription( const OUString& _rColumnName ) const;
    virtual OUString     getSelectStatement() const;
    virtual ::utl::SharedUNOComponent< XPreparedStatement >
                                getPreparedSelectStatement() const;
};

//------------------------------------------------------------------------
const DummyCopySource& DummyCopySource::Instance()
{
    static DummyCopySource s_aTheInstance;
    return s_aTheInstance;
}

//------------------------------------------------------------------------
OUString DummyCopySource::getQualifiedObjectName() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getQualifiedObjectName: not to be called!" );
    return OUString();
}

//------------------------------------------------------------------------
bool DummyCopySource::isView() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::isView: not to be called!" );
    return false;
}

//------------------------------------------------------------------------
void DummyCopySource::copyUISettingsTo( const Reference< XPropertySet >& /*_rxObject*/ ) const
{
    // no support
}
// -----------------------------------------------------------------------------
void DummyCopySource::copyFilterAndSortingTo( const Reference< XConnection >& ,const Reference< XPropertySet >& /*_rxObject*/ ) const
{
}
//------------------------------------------------------------------------
Sequence< OUString > DummyCopySource::getColumnNames() const
{
    return Sequence< OUString >();
}

//------------------------------------------------------------------------
Sequence< OUString > DummyCopySource::getPrimaryKeyColumnNames() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getPrimaryKeyColumnNames: not to be called!" );
    return Sequence< OUString >();
}

//------------------------------------------------------------------------
OFieldDescription* DummyCopySource::createFieldDescription( const OUString& /*_rColumnName*/ ) const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::createFieldDescription: not to be called!" );
    return NULL;
}
//------------------------------------------------------------------------
OUString DummyCopySource::getSelectStatement() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getSelectStatement: not to be called!" );
    return OUString();
}

//------------------------------------------------------------------------
::utl::SharedUNOComponent< XPreparedStatement > DummyCopySource::getPreparedSelectStatement() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getPreparedSelectStatement: not to be called!" );
    return ::utl::SharedUNOComponent< XPreparedStatement >();
}

//------------------------------------------------------------------------
namespace
{
    bool lcl_canCreateViewFor_nothrow( const Reference< XConnection >& _rxConnection )
    {
        Reference< XViewsSupplier > xSup( _rxConnection, UNO_QUERY );
        Reference< XDataDescriptorFactory > xViewFac;
        if ( xSup.is() )
            xViewFac.set( xSup->getViews(), UNO_QUERY );
        return xViewFac.is();
    }

    bool lcl_sameConnection_throw( const Reference< XConnection >& _rxLHS, const Reference< XConnection >& _rxRHS )
    {
        Reference< XDatabaseMetaData > xMetaLHS( _rxLHS->getMetaData(), UNO_QUERY_THROW );
        Reference< XDatabaseMetaData > xMetaRHS( _rxRHS->getMetaData(), UNO_QUERY_THROW );
        return xMetaLHS->getURL().equals( xMetaRHS->getURL() );
    }
}

//========================================================================
//= OCopyTableWizard
//========================================================================
//------------------------------------------------------------------------
OCopyTableWizard::OCopyTableWizard( Window * pParent, const OUString& _rDefaultName, sal_Int16 _nOperation,
        const ICopyTableSourceObject& _rSourceObject, const Reference< XConnection >& _xSourceConnection,
        const Reference< XConnection >& _xConnection, const Reference< XComponentContext >& _rxContext,
        const Reference< XInteractionHandler>&   _xInteractionHandler)
    : WizardDialog( pParent, ModuleRes(WIZ_RTFCOPYTABLE))
    ,m_pbHelp( this , ModuleRes(PB_HELP))
    ,m_pbCancel( this , ModuleRes(PB_CANCEL))
    ,m_pbPrev( this , ModuleRes(PB_PREV))
    ,m_pbNext( this , ModuleRes(PB_NEXT))
    ,m_pbFinish( this , ModuleRes(PB_OK))
    ,m_mNameMapping(_xConnection->getMetaData().is() && _xConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_xDestConnection( _xConnection )
    ,m_rSourceObject( _rSourceObject )
    ,m_xFormatter( getNumberFormatter( _xConnection, _rxContext ) )
    ,m_xContext(_rxContext)
    ,m_xInteractionHandler(_xInteractionHandler)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_nPageCount(0)
    ,m_bDeleteSourceColumns(sal_True)
    ,m_bInterConnectionCopy( _xSourceConnection != _xConnection )
    ,m_sName( _rDefaultName )
    ,m_nOperation( _nOperation )
    ,m_ePressed( WIZARD_NONE )
    ,m_bCreatePrimaryKeyColumn(sal_False)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::OCopyTableWizard" );
    DBG_CTOR(OCopyTableWizard,NULL);
    construct();

    // extract table name
    OUString sInitialTableName( _rDefaultName );
    try
    {
        m_sSourceName = m_rSourceObject.getQualifiedObjectName();
        OSL_ENSURE( !m_sSourceName.isEmpty(), "OCopyTableWizard::OCopyTableWizard: unable to retrieve the source object's name!" );

        if ( sInitialTableName.isEmpty() )
            sInitialTableName = m_sSourceName;

        if ( m_sName.isEmpty() )
        {
            if ( _xSourceConnection == m_xDestConnection )
            {
                Reference< XTablesSupplier > xSup( m_xDestConnection, UNO_QUERY_THROW );
                m_sName = ::dbtools::createUniqueName( xSup->getTables(), sInitialTableName, sal_False );
            }
            else
                m_sName = sInitialTableName;
        }
    }
    catch ( const Exception& )
    {
        m_sName = sInitialTableName;
    }

    ::dbaui::fillTypeInfo( _xSourceConnection, m_sTypeNames, m_aTypeInfo, m_aTypeInfoIndex );
    ::dbaui::fillTypeInfo( m_xDestConnection, m_sTypeNames, m_aDestTypeInfo, m_aDestTypeInfoIndex );
    impl_loadSourceData();

    bool bAllowViews = true;
    // if the source is a, don't allow creating views
    if ( m_rSourceObject.isView() )
        bAllowViews = false;
    // no views if the target connection does not support creating them
    if ( !lcl_canCreateViewFor_nothrow( m_xDestConnection ) )
        bAllowViews = false;
    // no views if we're copying to a different database
    if ( !lcl_sameConnection_throw( _xSourceConnection, m_xDestConnection ) )
        bAllowViews = false;

    if ( m_bInterConnectionCopy )
    {
        Reference< XDatabaseMetaData > xSrcMeta = _xSourceConnection->getMetaData();
        OUString sCatalog;
        OUString sSchema;
        OUString sTable;
        ::dbtools::qualifiedNameComponents( xSrcMeta,
                                            m_sName,
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::eInDataManipulation);

        m_sName = ::dbtools::composeTableName(m_xDestConnection->getMetaData(),sCatalog,sSchema,sTable,sal_False,::dbtools::eInTableDefinitions);
    }

    OCopyTable* pPage1( new OCopyTable( this ) );
    pPage1->disallowUseHeaderLine();
    if ( !bAllowViews )
        pPage1->disallowViews();
    pPage1->setCreateStyleAction();
    AddWizardPage(pPage1);

    AddWizardPage( new OWizNameMatching( this ) );
    AddWizardPage( new OWizColumnSelect( this ) );
    AddWizardPage( new OWizNormalExtend( this ) );
    ActivatePage();
}

// -----------------------------------------------------------------------------
OCopyTableWizard::OCopyTableWizard( Window* pParent, const OUString& _rDefaultName, sal_Int16 _nOperation,
        const ODatabaseExport::TColumns& _rSourceColumns, const ODatabaseExport::TColumnVector& _rSourceColVec,
        const Reference< XConnection >& _xConnection, const Reference< XNumberFormatter >&  _xFormatter,
        TypeSelectionPageFactory _pTypeSelectionPageFactory, SvStream& _rTypeSelectionPageArg, const Reference< XComponentContext >& _rxContext )
    :WizardDialog( pParent, ModuleRes(WIZ_RTFCOPYTABLE))
    ,m_vSourceColumns(_rSourceColumns)
    ,m_pbHelp( this , ModuleRes(PB_HELP))
    ,m_pbCancel( this , ModuleRes(PB_CANCEL))
    ,m_pbPrev( this , ModuleRes(PB_PREV))
    ,m_pbNext( this , ModuleRes(PB_NEXT))
    ,m_pbFinish( this , ModuleRes(PB_OK))
    ,m_mNameMapping(_xConnection->getMetaData().is() && _xConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_xDestConnection( _xConnection )
    ,m_rSourceObject( DummyCopySource::Instance() )
    ,m_xFormatter(_xFormatter)
    ,m_xContext(_rxContext)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_nPageCount(0)
    ,m_bDeleteSourceColumns(sal_False)
    ,m_bInterConnectionCopy( false )
    ,m_sName(_rDefaultName)
    ,m_nOperation( _nOperation )
    ,m_ePressed( WIZARD_NONE )
    ,m_bCreatePrimaryKeyColumn(sal_False)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::OCopyTableWizard" );
    DBG_CTOR(OCopyTableWizard,NULL);
    construct();
    ODatabaseExport::TColumnVector::const_iterator aIter = _rSourceColVec.begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = _rSourceColVec.end();
    for (; aIter != aEnd ; ++aIter)
    {
        m_vSourceVec.push_back(m_vSourceColumns.find((*aIter)->first));
    }

    ::dbaui::fillTypeInfo( _xConnection, m_sTypeNames, m_aTypeInfo, m_aTypeInfoIndex );
    ::dbaui::fillTypeInfo( _xConnection, m_sTypeNames, m_aDestTypeInfo, m_aDestTypeInfoIndex );

    m_xInteractionHandler.set( InteractionHandler::createWithParent(m_xContext, 0), UNO_QUERY );

    OCopyTable* pPage1( new OCopyTable( this ) );
    pPage1->disallowViews();
    pPage1->setCreateStyleAction();
    AddWizardPage( pPage1 );

    AddWizardPage( new OWizNameMatching( this ) );
    AddWizardPage( new OWizColumnSelect( this ) );
    AddWizardPage( (*_pTypeSelectionPageFactory)( this, _rTypeSelectionPageArg ) );

    ActivatePage();
}

// -----------------------------------------------------------------------------
void OCopyTableWizard::construct()
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::construct" );
    AddButton( &m_pbHelp, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    AddButton( &m_pbCancel, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    AddButton( &m_pbPrev );
    AddButton( &m_pbNext, WIZARDDIALOG_BUTTON_STDOFFSET_X );
    AddButton( &m_pbFinish );

    m_pbPrev.SetClickHdl( LINK( this, OCopyTableWizard, ImplPrevHdl ) );
    m_pbNext.SetClickHdl( LINK( this, OCopyTableWizard, ImplNextHdl ) );
    m_pbFinish.SetClickHdl( LINK( this, OCopyTableWizard, ImplOKHdl ) );

    SetActivatePageHdl( LINK( this, OCopyTableWizard, ImplActivateHdl ) );

    SetPrevButton( &m_pbPrev );
    SetNextButton( &m_pbNext );

    ShowButtonFixedLine( sal_True );

    m_pbNext.GrabFocus();

    if (m_vDestColumns.size())
        // source is a html or rtf table
        m_pbNext.SetStyle(m_pbFinish.GetStyle() | WB_DEFBUTTON);
    else
        m_pbFinish.SetStyle(m_pbFinish.GetStyle() | WB_DEFBUTTON);

    FreeResource();

    m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
    m_pTypeInfo->aUIName = m_sTypeNames.GetToken(TYPE_OTHER);
    m_bAddPKFirstTime = sal_True;
}
//------------------------------------------------------------------------
OCopyTableWizard::~OCopyTableWizard()
{
    DBG_DTOR(OCopyTableWizard,NULL);
    for ( ;; )
    {
        TabPage *pPage = GetPage(0);
        if ( pPage == NULL )
            break;
        RemovePage( pPage );
        delete pPage;
    }

    if ( m_bDeleteSourceColumns )
        clearColumns(m_vSourceColumns,m_vSourceVec);

    clearColumns(m_vDestColumns,m_aDestVec);

    // clear the type information
    m_aTypeInfoIndex.clear();
    m_aTypeInfo.clear();
    m_aDestTypeInfoIndex.clear();
}
// -----------------------------------------------------------------------
IMPL_LINK_NOARG(OCopyTableWizard, ImplPrevHdl)
{
    m_ePressed = WIZARD_PREV;
    if ( GetCurLevel() )
    {
        if ( getOperation() != CopyTableOperation::AppendData )
        {
            if(GetCurLevel() == 2)
                ShowPage(GetCurLevel()-2);
            else
                ShowPrevPage();
        }
        else
            ShowPrevPage();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(OCopyTableWizard, ImplNextHdl)
{
    m_ePressed = WIZARD_NEXT;
    if ( GetCurLevel() < MAX_PAGES )
    {
        if ( getOperation() != CopyTableOperation::AppendData )
        {
            if(GetCurLevel() == 0)
                ShowPage(GetCurLevel()+2);
            else
                ShowNextPage();
        }
        else
            ShowNextPage();
    }
    return 0;
}
// -----------------------------------------------------------------------
sal_Bool OCopyTableWizard::CheckColumns(sal_Int32& _rnBreakPos)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::CheckColumns" );
    sal_Bool bRet = sal_True;
    m_vColumnPos.clear();
    m_vColumnTypes.clear();

    OSL_ENSURE( m_xDestConnection.is(), "OCopyTableWizard::CheckColumns: No connection!" );
    //////////////////////////////////////////////////////////////////////
    // If database is able to process PrimaryKeys, set PrimaryKey
    if ( m_xDestConnection.is() )
    {
        sal_Bool bPKeyAllowed = supportsPrimaryKey();

        sal_Bool bContainsColumns = !m_vDestColumns.empty();

        if ( bPKeyAllowed && shouldCreatePrimaryKey() )
        {
            // add extra column for the primary key
            TOTypeInfoSP pTypeInfo = queryPrimaryKeyType(m_aDestTypeInfo);
            if ( pTypeInfo.get() )
            {
                if ( m_bAddPKFirstTime )
                {
                    OFieldDescription* pField = new OFieldDescription();
                    pField->SetName(m_aKeyName);
                    pField->FillFromTypeInfo(pTypeInfo,sal_True,sal_True);
                    pField->SetPrimaryKey(sal_True);
                    m_bAddPKFirstTime = sal_False;
                    insertColumn(0,pField);
                }
                m_vColumnPos.push_back(ODatabaseExport::TPositions::value_type(1,1));
                m_vColumnTypes.push_back(pTypeInfo->nType);
            }
        }

        if ( bContainsColumns )
        {   // we have dest columns so look for the matching column
            ODatabaseExport::TColumnVector::const_iterator aSrcIter = m_vSourceVec.begin();
            ODatabaseExport::TColumnVector::const_iterator aSrcEnd = m_vSourceVec.end();
            for(;aSrcIter != aSrcEnd;++aSrcIter)
            {
                ODatabaseExport::TColumns::iterator aDestIter = m_vDestColumns.find(m_mNameMapping[(*aSrcIter)->first]);

                if ( aDestIter != m_vDestColumns.end() )
                {
                    ODatabaseExport::TColumnVector::const_iterator aFind = ::std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                    sal_Int32 nPos = (aFind - m_aDestVec.begin())+1;
                    m_vColumnPos.push_back(ODatabaseExport::TPositions::value_type(nPos,nPos));
                    m_vColumnTypes.push_back((*aFind)->second->GetType());
                }
                else
                {
                    m_vColumnPos.push_back( ODatabaseExport::TPositions::value_type( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND ) );
                    m_vColumnTypes.push_back(0);
                }
            }
        }
        else
        {
            Reference< XDatabaseMetaData > xMetaData( m_xDestConnection->getMetaData() );
            OUString sExtraChars = xMetaData->getExtraNameCharacters();
            sal_Int32 nMaxNameLen       = getMaxColumnNameLength();

            ODatabaseExport::TColumnVector::const_iterator aSrcIter = m_vSourceVec.begin();
            ODatabaseExport::TColumnVector::const_iterator aSrcEnd = m_vSourceVec.end();
            for(_rnBreakPos=0;aSrcIter != aSrcEnd && bRet ;++aSrcIter,++_rnBreakPos)
            {
                OFieldDescription* pField = new OFieldDescription(*(*aSrcIter)->second);
                pField->SetName(convertColumnName(TExportColumnFindFunctor(&m_vDestColumns),(*aSrcIter)->first,sExtraChars,nMaxNameLen));
                TOTypeInfoSP pType = convertType((*aSrcIter)->second->getSpecialTypeInfo(),bRet);
                pField->SetType(pType);
                if ( !bPKeyAllowed )
                    pField->SetPrimaryKey(sal_False);

                // now create a column
                insertColumn(m_vDestColumns.size(),pField);
                m_vColumnPos.push_back(ODatabaseExport::TPositions::value_type(m_vDestColumns.size(),m_vDestColumns.size()));
                m_vColumnTypes.push_back((*aSrcIter)->second->GetType());
            }
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------
IMPL_LINK_NOARG(OCopyTableWizard, ImplOKHdl)
{
    m_ePressed = WIZARD_FINISH;
    sal_Bool bFinish = DeactivatePage() != 0;

    if(bFinish)
    {
        WaitObject aWait(this);
        switch(getOperation())
        {
            case CopyTableOperation::CopyDefinitionAndData:
            case CopyTableOperation::CopyDefinitionOnly:
            {
                sal_Bool bOnFirstPage = GetCurLevel() == 0;
                if ( bOnFirstPage )
                {
                    // we came from the first page so we have to clear
                    // all column information already collected
                    clearDestColumns();
                    m_mNameMapping.clear();
                }
                sal_Int32 nBreakPos = 0;
                sal_Bool bCheckOk = CheckColumns(nBreakPos);
                if ( bOnFirstPage && !bCheckOk )
                {
                    showColumnTypeNotSupported(m_vSourceVec[nBreakPos-1]->first);
                    OWizTypeSelect* pPage = static_cast<OWizTypeSelect*>(GetPage(3));
                    if ( pPage )
                    {
                        m_mNameMapping.clear();
                        pPage->setDisplayRow(nBreakPos);
                        ShowPage(3);
                        return 0;
                    }
                }
                if ( m_xDestConnection.is() )
                {
                    if ( supportsPrimaryKey() )
                    {
                        ODatabaseExport::TColumns::iterator aFind = ::std::find_if(m_vDestColumns.begin(),m_vDestColumns.end()
                            ,::o3tl::compose1(::std::mem_fun(&OFieldDescription::IsPrimaryKey),::o3tl::select2nd<ODatabaseExport::TColumns::value_type>()));
                        if ( aFind == m_vDestColumns.end() && m_xInteractionHandler.is() )
                        {

                            String sMsg(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY));
                            SQLContext aError;
                            aError.Message = sMsg;
                            ::rtl::Reference< ::comphelper::OInteractionRequest > xRequest( new ::comphelper::OInteractionRequest( makeAny( aError ) ) );
                            ::rtl::Reference< ::comphelper::OInteractionApprove > xYes = new ::comphelper::OInteractionApprove;
                            xRequest->addContinuation( xYes.get() );
                            xRequest->addContinuation( new ::comphelper::OInteractionDisapprove );
                            ::rtl::Reference< ::comphelper::OInteractionAbort > xAbort = new ::comphelper::OInteractionAbort;
                            xRequest->addContinuation( xAbort.get() );

                            m_xInteractionHandler->handle( xRequest.get() );

                            if ( xYes->wasSelected() )
                            {
                                OCopyTable* pPage = static_cast<OCopyTable*>(GetPage(0));
                                m_bCreatePrimaryKeyColumn = sal_True;
                                m_aKeyName = pPage->GetKeyName();
                                if ( m_aKeyName.isEmpty() )
                                    m_aKeyName = OUString( "ID" );
                                m_aKeyName = createUniqueName( m_aKeyName );
                                sal_Int32 nBreakPos2 = 0;
                                CheckColumns(nBreakPos2);
                            }
                            else if ( xAbort->wasSelected() )
                            {
                                ShowPage(3);
                                return 0;
                            }
                        }
                    }
                }
                break;
            }
            case CopyTableOperation::AppendData:
            case CopyTableOperation::CreateAsView:
                break;
            default:
            {
                SAL_WARN("dbaccess.ui", "OCopyTableWizard::ImplOKHdl: invalid creation style!");
            }
        }

        EndDialog(RET_OK);
    }
    return bFinish;
}
//------------------------------------------------------------------------
sal_Bool OCopyTableWizard::shouldCreatePrimaryKey() const
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::shouldCreatePrimaryKey" );
    return m_bCreatePrimaryKeyColumn;
}

// -----------------------------------------------------------------------
void OCopyTableWizard::setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName )
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::setCreatePrimaryKey" );
    m_bCreatePrimaryKeyColumn = _bDoCreate;
    if ( !_rSuggestedName.isEmpty() )
        m_aKeyName = _rSuggestedName;

    OCopyTable* pSettingsPage = dynamic_cast< OCopyTable* >( GetPage( 0 ) );
    OSL_ENSURE( pSettingsPage, "OCopyTableWizard::setCreatePrimaryKey: page should have been added in the ctor!" );
    if ( pSettingsPage )
        pSettingsPage->setCreatePrimaryKey( _bDoCreate, _rSuggestedName );
}

// -----------------------------------------------------------------------
IMPL_LINK_NOARG(OCopyTableWizard, ImplActivateHdl)
{
    OWizardPage* pCurrent = (OWizardPage*)GetPage(GetCurLevel());
    if(pCurrent)
    {
        sal_Bool bFirstTime = pCurrent->IsFirstTime();
        if(bFirstTime)
            pCurrent->Reset();

        CheckButtons();

        SetText(pCurrent->GetTitle());

        Invalidate();
    }
    return 0;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::CheckButtons()
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::CheckButtons" );
    if(GetCurLevel() == 0) // the first page has no back button
    {
        if(m_nPageCount > 1)
            m_pbNext.Enable(sal_True);
        else
            m_pbNext.Enable(sal_False);

        m_pbPrev.Enable(sal_False);
    }
    else if(GetCurLevel() == m_nPageCount-1) // the last page has no next button
    {
        m_pbNext.Enable(sal_False);
        m_pbPrev.Enable(sal_True);
    }
    else
    {
        m_pbPrev.Enable(sal_True);
        // next already has its state
    }
}
// -----------------------------------------------------------------------
void OCopyTableWizard::EnableButton(Wizard_Button_Style eStyle,sal_Bool bEnable)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::EnableButton" );
    Button* pButton;
    if(eStyle == WIZARD_NEXT)
        pButton = &m_pbNext;
    else if(eStyle == WIZARD_PREV)
        pButton = &m_pbPrev;
    else
        pButton = &m_pbFinish;
    pButton->Enable(bEnable);

}
// -----------------------------------------------------------------------
long OCopyTableWizard::DeactivatePage()
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::DeactivatePage" );
    OWizardPage* pPage = (OWizardPage*)GetPage(GetCurLevel());
    return pPage ? pPage->LeavePage() : sal_False;
}
// -----------------------------------------------------------------------
void OCopyTableWizard::AddWizardPage(OWizardPage* pPage)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::AddWizardPage" );
    AddPage(pPage);
    ++m_nPageCount;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::insertColumn(sal_Int32 _nPos,OFieldDescription* _pField)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::insertColumn" );
    OSL_ENSURE(_pField,"FieldDescrioption is null!");
    if ( _pField )
    {
        ODatabaseExport::TColumns::iterator aFind = m_vDestColumns.find(_pField->GetName());
        if ( aFind != m_vDestColumns.end() )
        {
            delete aFind->second;
            m_vDestColumns.erase(aFind);
        }

        m_aDestVec.insert(m_aDestVec.begin() + _nPos,
            m_vDestColumns.insert(ODatabaseExport::TColumns::value_type(_pField->GetName(),_pField)).first);
        m_mNameMapping[_pField->GetName()] = _pField->GetName();
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::replaceColumn(sal_Int32 _nPos,OFieldDescription* _pField,const OUString& _sOldName)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::replaceColumn" );
    OSL_ENSURE(_pField,"FieldDescrioption is null!");
    if ( _pField )
    {
        m_vDestColumns.erase(_sOldName);
        OSL_ENSURE( m_vDestColumns.find(_pField->GetName()) == m_vDestColumns.end(),"Column with that name already exist!");

        m_aDestVec[_nPos] =
            m_vDestColumns.insert(ODatabaseExport::TColumns::value_type(_pField->GetName(),_pField)).first;
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::impl_loadSourceData()
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::impl_loadSourceData" );
    loadData( m_rSourceObject, m_vSourceColumns, m_vSourceVec );
}

// -----------------------------------------------------------------------------
void OCopyTableWizard::loadData(  const ICopyTableSourceObject& _rSourceObject, ODatabaseExport::TColumns& _rColumns, ODatabaseExport::TColumnVector& _rColVector )
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::loadData" );
    ODatabaseExport::TColumns::iterator colEnd = _rColumns.end();
    for ( ODatabaseExport::TColumns::iterator col = _rColumns.begin(); col != colEnd; ++col )
        delete col->second;

    _rColVector.clear();
    _rColumns.clear();

    OFieldDescription* pActFieldDescr = NULL;
    OUString sCreateParam("x");
    //////////////////////////////////////////////////////////////////////
    // ReadOnly-Flag
    // On drop no line must be editable.
    // On add only empty lines must be editable.
    // On Add and Drop all lines can be edited.
    Sequence< OUString > aColumns( _rSourceObject.getColumnNames() );
    const OUString* pColumn      = aColumns.getConstArray();
    const OUString* pColumnEnd   = pColumn + aColumns.getLength();

    for ( ; pColumn != pColumnEnd; ++pColumn )
    {
        // get the properties of the column
        pActFieldDescr = _rSourceObject.createFieldDescription( *pColumn );
        OSL_ENSURE( pActFieldDescr, "OCopyTableWizard::loadData: illegal field description!" );
        if ( !pActFieldDescr )
            continue;

        sal_Int32 nType           = pActFieldDescr->GetType();
        sal_Int32 nScale          = pActFieldDescr->GetScale();
        sal_Int32 nPrecision      = pActFieldDescr->GetPrecision();
        sal_Bool bAutoIncrement   = pActFieldDescr->IsAutoIncrement();
        OUString sTypeName = pActFieldDescr->GetTypeName();

        // search for type
        sal_Bool bForce;
        TOTypeInfoSP pTypeInfo = ::dbaui::getTypeInfoFromType(m_aTypeInfo,nType,sTypeName,sCreateParam,nPrecision,nScale,bAutoIncrement,bForce);
        if ( !pTypeInfo.get() )
            pTypeInfo = m_pTypeInfo;

        pActFieldDescr->FillFromTypeInfo(pTypeInfo,sal_True,sal_False);
        _rColVector.push_back(_rColumns.insert(ODatabaseExport::TColumns::value_type(pActFieldDescr->GetName(),pActFieldDescr)).first);
    }

    // determine which coumns belong to the primary key
    Sequence< OUString > aPrimaryKeyColumns( _rSourceObject.getPrimaryKeyColumnNames() );
    const OUString* pKeyColName  = aPrimaryKeyColumns.getConstArray();
    const OUString* pKeyColEnd   = pKeyColName + aPrimaryKeyColumns.getLength();

    for( ; pKeyColName != pKeyColEnd; ++pKeyColName )
    {
        ODatabaseExport::TColumns::iterator keyPos = _rColumns.find( *pKeyColName );
        if ( keyPos != _rColumns.end() )
        {
            keyPos->second->SetPrimaryKey( sal_True );
            keyPos->second->SetIsNullable( ColumnValue::NO_NULLS );
        }
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::clearDestColumns()
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::clearDestColumns" );
    clearColumns(m_vDestColumns,m_aDestVec);
    m_bAddPKFirstTime = sal_True;
    m_mNameMapping.clear();
}

// -----------------------------------------------------------------------------
void OCopyTableWizard::appendColumns( Reference<XColumnsSupplier>& _rxColSup, const ODatabaseExport::TColumnVector* _pVec, sal_Bool _bKeyColumns) const
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::appendColumns" );
    // now append the columns
    OSL_ENSURE(_rxColSup.is(),"No columns supplier");
    if(!_rxColSup.is())
        return;
    Reference<XNameAccess> xColumns = _rxColSup->getColumns();
    OSL_ENSURE(xColumns.is(),"No columns");
    Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);

    Reference<XAppend> xAppend(xColumns,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    ODatabaseExport::TColumnVector::const_iterator aIter = _pVec->begin();
    ODatabaseExport::TColumnVector::const_iterator aEnd = _pVec->end();
    for(;aIter != aEnd;++aIter)
    {
        OFieldDescription* pField = (*aIter)->second;
        if(!pField)
            continue;

        Reference<XPropertySet> xColumn;
        if(pField->IsPrimaryKey() || !_bKeyColumns)
            xColumn = xColumnFactory->createDataDescriptor();
        if(xColumn.is())
        {
            if(!_bKeyColumns)
                dbaui::setColumnProperties(xColumn,pField);
            else
                xColumn->setPropertyValue(PROPERTY_NAME,makeAny(pField->GetName()));

            xAppend->appendByDescriptor(xColumn);
            xColumn = NULL;
            // now only the settings are missing
            if(xColumns->hasByName(pField->GetName()))
            {
                xColumn.set(xColumns->getByName(pField->GetName()),UNO_QUERY);
                OSL_ENSURE(xColumn.is(),"OCopyTableWizard::appendColumns: Column is NULL!");
                if ( xColumn.is() )
                    pField->copyColumnSettingsTo(xColumn);
            }
            else
            {
                SAL_WARN("dbaccess.ui", "OCopyTableWizard::appendColumns: invalid field name!");
            }

        }
    }
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::appendKey( Reference<XKeysSupplier>& _rxSup, const ODatabaseExport::TColumnVector* _pVec) const
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::appendKey" );
    if(!_rxSup.is())
        return; // the database doesn't support keys
    OSL_ENSURE(_rxSup.is(),"No XKeysSupplier!");
    Reference<XDataDescriptorFactory> xKeyFactory(_rxSup->getKeys(),UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
    if ( !xKeyFactory.is() )
        return;
    Reference<XAppend> xAppend(xKeyFactory,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    Reference<XPropertySet> xKey = xKeyFactory->createDataDescriptor();
    OSL_ENSURE(xKey.is(),"Key is null!");
    xKey->setPropertyValue(PROPERTY_TYPE,makeAny(KeyType::PRIMARY));

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if(xColSup.is())
    {
        appendColumns(xColSup,_pVec,sal_True);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        if(xColumns.is() && xColumns->getElementNames().getLength())
            xAppend->appendByDescriptor(xKey);
    }

}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCopyTableWizard::createView() const
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::createView" );
    OUString sCommand( m_rSourceObject.getSelectStatement() );
    OSL_ENSURE( !sCommand.isEmpty(), "OCopyTableWizard::createView: no statement in the source object!" );
        // there are legitimate cases in which getSelectStatement does not provide a statement,
        // but in all those cases, this method here should never be called.
    return ::dbaui::createView( m_sName, m_xDestConnection, sCommand );
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCopyTableWizard::createTable()
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::createTable" );
    Reference< XPropertySet > xTable;

    Reference<XTablesSupplier> xSup( m_xDestConnection, UNO_QUERY );
    Reference< XNameAccess > xTables;
    if(xSup.is())
        xTables = xSup->getTables();
    if ( getOperation() != CopyTableOperation::AppendData )
    {
        Reference<XDataDescriptorFactory> xFact(xTables,UNO_QUERY);
        OSL_ENSURE(xFact.is(),"No XDataDescriptorFactory available!");
        if(!xFact.is())
            return NULL;

        xTable = xFact->createDataDescriptor();
        OSL_ENSURE(xTable.is(),"Could not create a new object!");
        if(!xTable.is())
            return NULL;

        OUString sCatalog,sSchema,sTable;
        Reference< XDatabaseMetaData> xMetaData = m_xDestConnection->getMetaData();
        ::dbtools::qualifiedNameComponents(xMetaData,
                                            m_sName,
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::eInDataManipulation);

        if ( sCatalog.isEmpty() && xMetaData->supportsCatalogsInTableDefinitions() )
        {
            sCatalog = m_xDestConnection->getCatalog();
        }

        if ( sSchema.isEmpty() && xMetaData->supportsSchemasInTableDefinitions() )
        {
            sSchema = xMetaData->getUserName();
        }

        xTable->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
        xTable->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
        xTable->setPropertyValue(PROPERTY_NAME,makeAny(sTable));

        Reference< XColumnsSupplier > xSuppDestinationColumns( xTable, UNO_QUERY );
        // now append the columns
        const ODatabaseExport::TColumnVector* pVec = getDestVector();
        appendColumns( xSuppDestinationColumns, pVec );
        // now append the primary key
        Reference<XKeysSupplier> xKeySup(xTable,UNO_QUERY);
        appendKey(xKeySup,pVec);

        Reference<XAppend> xAppend(xTables,UNO_QUERY);
        if(xAppend.is())
            xAppend->appendByDescriptor(xTable);

        //  xTable = NULL;
        // we need to reget the table because after appending it it is no longer valid
        if(xTables->hasByName(m_sName))
            xTables->getByName(m_sName) >>= xTable;
        else
        {
            OUString sComposedName(
                ::dbtools::composeTableName( m_xDestConnection->getMetaData(), xTable, ::dbtools::eInDataManipulation, false, false, false ) );
            if(xTables->hasByName(sComposedName))
            {
                xTables->getByName(sComposedName) >>= xTable;
                m_sName = sComposedName;
            }
            else
                xTable = NULL;
        }
        if(xTable.is())
        {
            xSuppDestinationColumns.set( xTable, UNO_QUERY_THROW );
            // insert new table name into table filter
            ::dbaui::appendToFilter( m_xDestConnection, m_sName, GetComponentContext(), this );

            // copy ui settings
            m_rSourceObject.copyUISettingsTo( xTable );
            //copy filter and sorting
            m_rSourceObject.copyFilterAndSortingTo(m_xDestConnection,xTable);
            // set column mappings
            Reference<XNameAccess> xNameAccess = xSuppDestinationColumns->getColumns();
            Sequence< OUString> aSeq = xNameAccess->getElementNames();
            const OUString* pIter = aSeq.getConstArray();
            const OUString* pEnd   = pIter + aSeq.getLength();

            for(sal_Int32 nNewPos=1;pIter != pEnd;++pIter,++nNewPos)
            {
                ODatabaseExport::TColumns::const_iterator aDestIter = m_vDestColumns.find(*pIter);

                if ( aDestIter != m_vDestColumns.end() )
                {
                    ODatabaseExport::TColumnVector::const_iterator aFind = ::std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                    sal_Int32 nPos = (aFind - m_aDestVec.begin())+1;

                    ODatabaseExport::TPositions::iterator aPosFind = ::std::find_if(
                        m_vColumnPos.begin(),
                        m_vColumnPos.end(),
                        ::o3tl::compose1(    ::std::bind2nd( ::std::equal_to< sal_Int32 >(), nPos ),
                                            ::o3tl::select1st< ODatabaseExport::TPositions::value_type >()
                        )
                    );

                    if ( m_vColumnPos.end() != aPosFind )
                    {
                        aPosFind->second = nNewPos;
                        OSL_ENSURE( m_vColumnTypes.size() > size_t( aPosFind - m_vColumnPos.begin() ),
                            "Invalid index for vector!" );
                        m_vColumnTypes[ aPosFind - m_vColumnPos.begin() ] = (*aFind)->second->GetType();
                    }
                }
            }
        }
    }
    else if(xTables.is() && xTables->hasByName(m_sName))
        xTables->getByName(m_sName) >>= xTable;

    return xTable;
}

// -----------------------------------------------------------------------------
bool OCopyTableWizard::supportsPrimaryKey( const Reference< XConnection >& _rxConnection )
{
    OSL_PRECOND( _rxConnection.is(), "OCopyTableWizard::supportsPrimaryKey: invalid connection!" );
    if ( !_rxConnection.is() )
        return false;

    ::dbtools::DatabaseMetaData aMetaData( _rxConnection );
    return aMetaData.supportsPrimaryKeys();
}

// -----------------------------------------------------------------------------
bool OCopyTableWizard::supportsViews( const Reference< XConnection >& _rxConnection )
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::supportsViews" );
    OSL_PRECOND( _rxConnection.is(), "OCopyTableWizard::supportsViews: invalid connection!" );
    if ( !_rxConnection.is() )
        return false;

    bool bSupportsViews( false );
    try
    {
        Reference< XDatabaseMetaData > xMetaData( _rxConnection->getMetaData(), UNO_SET_THROW );
        Reference< XViewsSupplier > xViewSups( _rxConnection, UNO_QUERY );
        bSupportsViews = xViewSups.is();
        if ( !bSupportsViews )
        {
            try
            {
                Reference< XResultSet > xRs( xMetaData->getTableTypes(), UNO_SET_THROW );
                Reference< XRow > xRow( xRs, UNO_QUERY_THROW );
                while ( xRs->next() )
                {
                    OUString sValue = xRow->getString( 1 );
                    if ( !xRow->wasNull() && sValue.equalsIgnoreAsciiCase("View") )
                    {
                        bSupportsViews = true;
                        break;
                    }
                }
            }
            catch( const SQLException& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bSupportsViews;
}

// -----------------------------------------------------------------------------
sal_Int32 OCopyTableWizard::getMaxColumnNameLength() const
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::getMaxColumnNameLength" );
    sal_Int32 nLen = 0;
    if ( m_xDestConnection.is() )
    {
        try
        {
            Reference< XDatabaseMetaData > xMetaData( m_xDestConnection->getMetaData(), UNO_SET_THROW );
            nLen = xMetaData->getMaxColumnNameLength();
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return nLen;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::setOperation( const sal_Int16 _nOperation )
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::setOperation" );
    m_nOperation = _nOperation;
}
// -----------------------------------------------------------------------------
sal_Int16 OCopyTableWizard::getOperation() const
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::getOperation" );
    return m_nOperation;
}
// -----------------------------------------------------------------------------
OUString OCopyTableWizard::convertColumnName(const TColumnFindFunctor&   _rCmpFunctor,
                                                    const OUString&  _sColumnName,
                                                    const OUString&  _sExtraChars,
                                                    sal_Int32               _nMaxNameLen)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::convertColumnName" );
    OUString sAlias = _sColumnName;
    if ( isSQL92CheckEnabled( m_xDestConnection ) )
        sAlias = ::dbtools::convertName2SQLName(_sColumnName,_sExtraChars);
    if((_nMaxNameLen && sAlias.getLength() > _nMaxNameLen) || _rCmpFunctor(sAlias))
    {
        sal_Int32 nDiff = 1;
        do
        {
            ++nDiff;
            if(_nMaxNameLen && sAlias.getLength() >= _nMaxNameLen)
                sAlias = sAlias.copy(0,sAlias.getLength() - (sAlias.getLength()-_nMaxNameLen+nDiff));

            OUString sName(sAlias);
            sal_Int32 nPos = 1;
            sName += OUString::valueOf(nPos);

            while(_rCmpFunctor(sName))
            {
                sName = sAlias;
                sName += OUString::valueOf(++nPos);
            }
            sAlias = sName;
            // we have to check again, it could happen that the name is already to long
        }
        while(_nMaxNameLen && sAlias.getLength() > _nMaxNameLen);
    }
    OSL_ENSURE(m_mNameMapping.find(_sColumnName) == m_mNameMapping.end(),"name doubled!");
    m_mNameMapping[_sColumnName] = sAlias;
    return sAlias;
}

// -----------------------------------------------------------------------------
void OCopyTableWizard::removeColumnNameFromNameMap(const OUString& _sName)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::removeColumnNameFromNameMap" );
    m_mNameMapping.erase(_sName);
}

// -----------------------------------------------------------------------------
sal_Bool OCopyTableWizard::supportsType(sal_Int32 _nDataType,sal_Int32& _rNewDataType)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::supportsType" );
    sal_Bool bRet = m_aDestTypeInfo.find(_nDataType) != m_aDestTypeInfo.end();
    if ( bRet )
        _rNewDataType = _nDataType;
    return bRet;
}

// -----------------------------------------------------------------------------
TOTypeInfoSP OCopyTableWizard::convertType(const TOTypeInfoSP& _pType,sal_Bool& _bNotConvert)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::convertType" );
    if ( !m_bInterConnectionCopy )
        // no need to convert if the source and destination connection are the same
        return _pType;

    sal_Bool bForce;
    TOTypeInfoSP pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,_pType->nType,_pType->aTypeName,_pType->aCreateParams,_pType->nPrecision,_pType->nMaximumScale,_pType->bAutoIncrement,bForce);
    if ( !pType.get() || bForce )
    { // no type found so we have to find the correct one ourself
        sal_Int32 nDefaultType = DataType::VARCHAR;
        switch(_pType->nType)
        {
            case DataType::TINYINT:
                if(supportsType(DataType::SMALLINT,nDefaultType))
                    break;
                // run through
            case DataType::SMALLINT:
                if(supportsType(DataType::INTEGER,nDefaultType))
                    break;
                // run through
            case DataType::INTEGER:
                if(supportsType(DataType::FLOAT,nDefaultType))
                    break;
                // run through
            case DataType::FLOAT:
                if(supportsType(DataType::REAL,nDefaultType))
                    break;
                // run through
            case DataType::DATE:
            case DataType::TIME:
                if( DataType::DATE == _pType->nType || DataType::TIME == _pType->nType )
                {
                    if(supportsType(DataType::TIMESTAMP,nDefaultType))
                        break;
                }
                // run through
            case DataType::TIMESTAMP:
            case DataType::REAL:
            case DataType::BIGINT:
                if ( supportsType(DataType::DOUBLE,nDefaultType) )
                    break;
                // run through
            case DataType::DOUBLE:
                if ( supportsType(DataType::NUMERIC,nDefaultType) )
                    break;
                // run through
            case DataType::NUMERIC:
                supportsType(DataType::DECIMAL,nDefaultType);
                break;
            case DataType::DECIMAL:
                if ( supportsType(DataType::NUMERIC,nDefaultType) )
                    break;
                if ( supportsType(DataType::DOUBLE,nDefaultType) )
                    break;
                break;
            case DataType::VARCHAR:
                if ( supportsType(DataType::LONGVARCHAR,nDefaultType) )
                    break;
                break;
            case DataType::LONGVARCHAR:
                if ( supportsType(DataType::CLOB,nDefaultType) )
                    break;
                break;
            case DataType::BINARY:
                if ( supportsType(DataType::VARBINARY,nDefaultType) )
                    break;
                break;
            case DataType::VARBINARY:
                if ( supportsType(DataType::LONGVARBINARY,nDefaultType) )
                    break;
                break;
            case DataType::LONGVARBINARY:
                if ( supportsType(DataType::BLOB,nDefaultType) )
                    break;
                if ( supportsType(DataType::LONGVARCHAR,nDefaultType) )
                    break;
                if ( supportsType(DataType::CLOB,nDefaultType) )
                    break;
                break;
            default:
                nDefaultType = DataType::VARCHAR;
        }
        pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,nDefaultType,_pType->aTypeName,_pType->aCreateParams,_pType->nPrecision,_pType->nMaximumScale,_pType->bAutoIncrement,bForce);
        if ( !pType.get() )
        {
            _bNotConvert = sal_False;
            OUString sCreate("x");
            pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,DataType::VARCHAR,_pType->aTypeName,sCreate,50,0,sal_False,bForce);
            if ( !pType.get() )
                pType = m_pTypeInfo;
        }
        else if ( bForce )
            _bNotConvert = sal_False;
    }
    return pType;
}
// -----------------------------------------------------------------------------
OUString OCopyTableWizard::createUniqueName(const OUString& _sName)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::createUniqueName" );
    OUString sName = _sName;
    Sequence< OUString > aColumnNames( m_rSourceObject.getColumnNames() );
    if ( aColumnNames.getLength() )
        sName = ::dbtools::createUniqueName( aColumnNames, sName, sal_False );
    else
    {
        if ( m_vSourceColumns.find(sName) != m_vSourceColumns.end())
        {
            sal_Int32 nPos = 0;
            while(m_vSourceColumns.find(sName) != m_vSourceColumns.end())
            {
                sName = _sName;
                sName += OUString::valueOf(++nPos);
            }
        }
    }
    return sName;
}
// -----------------------------------------------------------------------------
void OCopyTableWizard::showColumnTypeNotSupported(const OUString& _rColumnName)
{
    SAL_INFO("dbaccess.ui", "OCopyTableWizard::showColumnTypeNotSupported" );
    String sMessage( ModuleRes( STR_UNKNOWN_TYPE_FOUND ) );
    sMessage.SearchAndReplaceAscii("#1",_rColumnName);
    showError(sMessage);
}
//-------------------------------------------------------------------------------
void OCopyTableWizard::showError(const OUString& _sErrorMesage)
{
    SQLExceptionInfo aInfo(_sErrorMesage);
    showError(aInfo.get());
}
//-------------------------------------------------------------------------------
void OCopyTableWizard::showError(const Any& _aError)
{
    if ( _aError.hasValue() && m_xInteractionHandler.is() )
    {
        try
        {
            ::rtl::Reference< ::comphelper::OInteractionRequest > xRequest( new ::comphelper::OInteractionRequest( _aError ) );
            m_xInteractionHandler->handle( xRequest.get() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
