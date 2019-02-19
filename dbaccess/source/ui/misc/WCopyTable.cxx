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

#include <strings.hrc>
#include <strings.hxx>
#include <stringconstants.hxx>
#include <core_resource.hxx>
#include <sqlmessage.hxx>
#include <UITools.hxx>
#include <WColumnSelect.hxx>
#include <WCopyTable.hxx>
#include <WCPage.hxx>
#include <WExtendPages.hxx>
#include <WNameMatch.hxx>
#include <WTypeSelect.hxx>

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

#include <comphelper/types.hxx>
#include <comphelper/interaction.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbmetadata.hxx>
#include <connectivity/dbexception.hxx>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/waitobj.hxx>

#include <functional>

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

namespace
{
    void clearColumns(ODatabaseExport::TColumns& _rColumns, ODatabaseExport::TColumnVector& _rColumnsVec)
    {
        for (auto const& column : _rColumns)
            delete column.second;

        _rColumnsVec.clear();
        _rColumns.clear();
    }
}

// ICopyTableSourceObject
ICopyTableSourceObject::~ICopyTableSourceObject()
{
}

// ObjectCopySource
ObjectCopySource::ObjectCopySource( const Reference< XConnection >& _rxConnection, const Reference< XPropertySet >& _rxObject )
    :m_xConnection( _rxConnection, UNO_SET_THROW )
    ,m_xMetaData( _rxConnection->getMetaData(), UNO_SET_THROW )
    ,m_xObject( _rxObject, UNO_SET_THROW )
    ,m_xObjectPSI( _rxObject->getPropertySetInfo(), UNO_SET_THROW )
    ,m_xObjectColumns( Reference< XColumnsSupplier >( _rxObject, UNO_QUERY_THROW )->getColumns(), UNO_SET_THROW )
{
}

OUString ObjectCopySource::getQualifiedObjectName() const
{
    OUString sName;

    if ( !m_xObjectPSI->hasPropertyByName( PROPERTY_COMMAND ) )
        sName = ::dbtools::composeTableName( m_xMetaData, m_xObject, ::dbtools::EComposeRule::InDataManipulation, false );
    else
        m_xObject->getPropertyValue( PROPERTY_NAME ) >>= sName;
    return sName;
}

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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return bIsView;
}

void ObjectCopySource::copyUISettingsTo( const Reference< XPropertySet >& _rxObject ) const
{
    const OUString aCopyProperties[] = {
        OUString(PROPERTY_FONT), OUString(PROPERTY_ROW_HEIGHT), OUString(PROPERTY_TEXTCOLOR),OUString(PROPERTY_TEXTLINECOLOR),OUString(PROPERTY_TEXTEMPHASIS),OUString(PROPERTY_TEXTRELIEF)
    };
    for (const auto & aCopyPropertie : aCopyProperties)
    {
        if ( m_xObjectPSI->hasPropertyByName( aCopyPropertie ) )
            _rxObject->setPropertyValue( aCopyPropertie, m_xObject->getPropertyValue( aCopyPropertie ) );
    }
}

void ObjectCopySource::copyFilterAndSortingTo( const Reference< XConnection >& _xConnection,const Reference< XPropertySet >& _rxObject ) const
{
    std::pair< OUString, OUString > aProperties[] = {
                 std::pair< OUString, OUString >(PROPERTY_FILTER,OUString(" AND "))
                ,std::pair< OUString, OUString >(PROPERTY_ORDER,OUString(" ORDER BY "))
    };

    try
    {
        const OUString sSourceName = ::dbtools::composeTableNameForSelect(m_xConnection,m_xObject) + ".";
        const OUString sTargetName = ::dbtools::composeTableNameForSelect(_xConnection,_rxObject);
        const OUString sTargetNameTemp = sTargetName + ".";

        OUStringBuffer sStatement = "SELECT * FROM " + sTargetName + " WHERE 0=1";

        for (const std::pair<OUString,OUString> & aPropertie : aProperties)
        {
            if ( m_xObjectPSI->hasPropertyByName( aPropertie.first ) )
            {
                OUString sFilter;
                m_xObject->getPropertyValue( aPropertie.first ) >>= sFilter;
                if ( !sFilter.isEmpty() )
                {
                    sStatement.append(aPropertie.second);
                    OUString sReplace = sFilter;
                    sReplace = sReplace.replaceFirst(sSourceName,sTargetNameTemp);
                    sFilter = sReplace;
                    _rxObject->setPropertyValue( aPropertie.first, makeAny(sFilter) );
                    sStatement.append(sFilter);
                }
            }
        }

        _xConnection->createStatement()->executeQuery(sStatement.makeStringAndClear());

        if ( m_xObjectPSI->hasPropertyByName( PROPERTY_APPLYFILTER ) )
            _rxObject->setPropertyValue( PROPERTY_APPLYFILTER, m_xObject->getPropertyValue( PROPERTY_APPLYFILTER ) );
    }
    catch(Exception&)
    {
    }
}

Sequence< OUString > ObjectCopySource::getColumnNames() const
{
    return m_xObjectColumns->getElementNames();
}

Sequence< OUString > ObjectCopySource::getPrimaryKeyColumnNames() const
{
    const Reference<XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(m_xObject);
    Sequence< OUString > aKeyColNames;
    if ( xPrimaryKeyColumns.is() )
        aKeyColNames = xPrimaryKeyColumns->getElementNames();
    return aKeyColNames;
}

OFieldDescription* ObjectCopySource::createFieldDescription( const OUString& _rColumnName ) const
{
    Reference< XPropertySet > xColumn( m_xObjectColumns->getByName( _rColumnName ), UNO_QUERY_THROW );
    return new OFieldDescription( xColumn );
}

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
        aSQL.append( "SELECT " );

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
                aSQL.append( " " );
            else
                aSQL.append( ", " );
        }

        aSQL.append( "FROM " ).append( ::dbtools::composeTableNameForSelect( m_xConnection, m_xObject ) );

        sSelectStatement = aSQL.makeStringAndClear();
    }

    return sSelectStatement;
}

::utl::SharedUNOComponent< XPreparedStatement > ObjectCopySource::getPreparedSelectStatement() const
{
    ::utl::SharedUNOComponent< XPreparedStatement > xStatement(
        m_xConnection->prepareStatement( getSelectStatement() ),
        ::utl::SharedUNOComponent< XPreparedStatement >::TakeOwnership
    );
    return xStatement;
}

// NamedTableCopySource
NamedTableCopySource::NamedTableCopySource( const Reference< XConnection >& _rxConnection, const OUString& _rTableName )
    :m_xConnection( _rxConnection, UNO_SET_THROW )
    ,m_xMetaData( _rxConnection->getMetaData(), UNO_SET_THROW )
    ,m_sTableName( _rTableName )
    ,m_aColumnInfo()
{
    ::dbtools::qualifiedNameComponents( m_xMetaData, m_sTableName, m_sTableCatalog, m_sTableSchema, m_sTableBareName, ::dbtools::EComposeRule::Complete );
    impl_ensureColumnInfo_throw();
}

OUString NamedTableCopySource::getQualifiedObjectName() const
{
    return m_sTableName;
}

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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return sTableType == "VIEW";
}

void NamedTableCopySource::copyUISettingsTo( const Reference< XPropertySet >& /*_rxObject*/ ) const
{
    // not supported: we do not have UI settings to copy
}

void NamedTableCopySource::copyFilterAndSortingTo( const Reference< XConnection >& ,const Reference< XPropertySet >& /*_rxObject*/ ) const
{
}

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

::utl::SharedUNOComponent< XPreparedStatement > const & NamedTableCopySource::impl_ensureStatement_throw()
{
    if ( !m_xStatement.is() )
        m_xStatement.set( m_xConnection->prepareStatement( getSelectStatement() ), UNO_SET_THROW );
    return m_xStatement;
}

Sequence< OUString > NamedTableCopySource::getColumnNames() const
{
    Sequence< OUString > aNames( m_aColumnInfo.size() );
    size_t nPos = 0;
    for (auto const& elem : m_aColumnInfo)
        aNames[ nPos++ ] = elem.GetName();

    return aNames;
}

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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    return aPKColNames;
}

OFieldDescription* NamedTableCopySource::createFieldDescription( const OUString& _rColumnName ) const
{
    for (auto const& elem : m_aColumnInfo)
        if ( elem.GetName() == _rColumnName )
            return new OFieldDescription(elem);

    return nullptr;
}

OUString NamedTableCopySource::getSelectStatement() const
{
    OUStringBuffer aSQL;
    aSQL.append( "SELECT * FROM " );

    aSQL.append( ::dbtools::composeTableNameForSelect( m_xConnection, m_sTableCatalog, m_sTableSchema, m_sTableBareName ) );

    return aSQL.makeStringAndClear();
}

::utl::SharedUNOComponent< XPreparedStatement > NamedTableCopySource::getPreparedSelectStatement() const
{
    return const_cast< NamedTableCopySource* >( this )->impl_ensureStatement_throw();
}

// DummyCopySource
class DummyCopySource : public ICopyTableSourceObject
{
public:
    DummyCopySource() { }

    static const DummyCopySource& Instance();

    // ICopyTableSourceObject overridables
    virtual OUString            getQualifiedObjectName() const override;
    virtual bool                isView() const override;
    virtual void                copyUISettingsTo( const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const override;
    virtual void                copyFilterAndSortingTo(const css::uno::Reference< css::sdbc::XConnection >& _xConnection, const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const override;
    virtual css::uno::Sequence< OUString >
                                getColumnNames() const override;
    virtual css::uno::Sequence< OUString >
                                getPrimaryKeyColumnNames() const override;
    virtual OFieldDescription*  createFieldDescription( const OUString& _rColumnName ) const override;
    virtual OUString            getSelectStatement() const override;
    virtual ::utl::SharedUNOComponent< XPreparedStatement >
                                getPreparedSelectStatement() const override;
};

const DummyCopySource& DummyCopySource::Instance()
{
    static DummyCopySource s_aTheInstance;
    return s_aTheInstance;
}

OUString DummyCopySource::getQualifiedObjectName() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getQualifiedObjectName: not to be called!" );
    return OUString();
}

bool DummyCopySource::isView() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::isView: not to be called!" );
    return false;
}

void DummyCopySource::copyUISettingsTo( const Reference< XPropertySet >& /*_rxObject*/ ) const
{
    // no support
}

void DummyCopySource::copyFilterAndSortingTo( const Reference< XConnection >& ,const Reference< XPropertySet >& /*_rxObject*/ ) const
{
}

Sequence< OUString > DummyCopySource::getColumnNames() const
{
    return Sequence< OUString >();
}

Sequence< OUString > DummyCopySource::getPrimaryKeyColumnNames() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getPrimaryKeyColumnNames: not to be called!" );
    return Sequence< OUString >();
}

OFieldDescription* DummyCopySource::createFieldDescription( const OUString& /*_rColumnName*/ ) const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::createFieldDescription: not to be called!" );
    return nullptr;
}

OUString DummyCopySource::getSelectStatement() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getSelectStatement: not to be called!" );
    return OUString();
}

::utl::SharedUNOComponent< XPreparedStatement > DummyCopySource::getPreparedSelectStatement() const
{
    SAL_WARN("dbaccess.ui",  "DummyCopySource::getPreparedSelectStatement: not to be called!" );
    return ::utl::SharedUNOComponent< XPreparedStatement >();
}

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
        return xMetaLHS->getURL() == xMetaRHS->getURL();
    }
}

// OCopyTableWizard
OCopyTableWizard::OCopyTableWizard( vcl::Window * pParent, const OUString& _rDefaultName, sal_Int16 _nOperation,
        const ICopyTableSourceObject& _rSourceObject, const Reference< XConnection >& _xSourceConnection,
        const Reference< XConnection >& _xConnection, const Reference< XComponentContext >& _rxContext,
        const Reference< XInteractionHandler>&   _xInteractionHandler)
    : WizardDialog( pParent, "RTFCopyTable", "dbaccess/ui/rtfcopytabledialog.ui")
    ,m_mNameMapping(_xConnection->getMetaData().is() && _xConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_xDestConnection( _xConnection )
    ,m_rSourceObject( _rSourceObject )
    ,m_xFormatter( getNumberFormatter( _xConnection, _rxContext ) )
    ,m_xContext(_rxContext)
    ,m_xInteractionHandler(_xInteractionHandler)
    ,m_sTypeNames(DBA_RES(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_nPageCount(0)
    ,m_bDeleteSourceColumns(true)
    ,m_bInterConnectionCopy( _xSourceConnection != _xConnection )
    ,m_sName( _rDefaultName )
    ,m_nOperation( _nOperation )
    ,m_ePressed( WIZARD_NONE )
    ,m_bCreatePrimaryKeyColumn(false)
{
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
                m_sName = ::dbtools::createUniqueName( xSup->getTables(), sInitialTableName, false );
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
    loadData( m_rSourceObject, m_vSourceColumns, m_vSourceVec );

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
                                            ::dbtools::EComposeRule::InDataManipulation);

        m_sName = ::dbtools::composeTableName(m_xDestConnection->getMetaData(),sCatalog,sSchema,sTable,false,::dbtools::EComposeRule::InTableDefinitions);
    }

    VclPtrInstance<OCopyTable> pPage1( this );
    pPage1->disallowUseHeaderLine();
    if ( !bAllowViews )
        pPage1->disallowViews();
    pPage1->setCreateStyleAction();
    AddWizardPage(pPage1);

    AddWizardPage( VclPtr<OWizNameMatching>::Create( this ) );
    AddWizardPage( VclPtr<OWizColumnSelect>::Create( this ) );
    AddWizardPage( VclPtr<OWizNormalExtend>::Create( this ) );
    ActivatePage();
}

OCopyTableWizard::OCopyTableWizard( vcl::Window* pParent, const OUString& _rDefaultName, sal_Int16 _nOperation,
        const ODatabaseExport::TColumns& _rSourceColumns, const ODatabaseExport::TColumnVector& _rSourceColVec,
        const Reference< XConnection >& _xConnection, const Reference< XNumberFormatter >&  _xFormatter,
        TypeSelectionPageFactory _pTypeSelectionPageFactory, SvStream& _rTypeSelectionPageArg, const Reference< XComponentContext >& _rxContext )
    :WizardDialog( pParent, "RTFCopyTable", "dbaccess/ui/rtfcopytabledialog.ui")
    ,m_vSourceColumns(_rSourceColumns)
    ,m_mNameMapping(_xConnection->getMetaData().is() && _xConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers())
    ,m_xDestConnection( _xConnection )
    ,m_rSourceObject( DummyCopySource::Instance() )
    ,m_xFormatter(_xFormatter)
    ,m_xContext(_rxContext)
    ,m_sTypeNames(DBA_RES(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_nPageCount(0)
    ,m_bDeleteSourceColumns(false)
    ,m_bInterConnectionCopy( false )
    ,m_sName(_rDefaultName)
    ,m_nOperation( _nOperation )
    ,m_ePressed( WIZARD_NONE )
    ,m_bCreatePrimaryKeyColumn(false)
{
    construct();
    for (auto const& sourceCol : _rSourceColVec)
    {
        m_vSourceVec.emplace_back(m_vSourceColumns.find(sourceCol->first));
    }

    ::dbaui::fillTypeInfo( _xConnection, m_sTypeNames, m_aTypeInfo, m_aTypeInfoIndex );
    ::dbaui::fillTypeInfo( _xConnection, m_sTypeNames, m_aDestTypeInfo, m_aDestTypeInfoIndex );

    m_xInteractionHandler.set( InteractionHandler::createWithParent(m_xContext, nullptr), UNO_QUERY );

    VclPtrInstance<OCopyTable> pPage1( this );
    pPage1->disallowViews();
    pPage1->setCreateStyleAction();
    AddWizardPage( pPage1 );

    AddWizardPage( VclPtr<OWizNameMatching>::Create( this ) );
    AddWizardPage( VclPtr<OWizColumnSelect>::Create( this ) );
    AddWizardPage( (*_pTypeSelectionPageFactory)( this, _rTypeSelectionPageArg ) );

    ActivatePage();
}

void OCopyTableWizard::construct()
{
    SetSizePixel(Size(700, 350));

    m_pbHelp = VclPtr<HelpButton>::Create(this, WB_TABSTOP);
    AddButton(m_pbHelp);
    m_pbCancel = VclPtr<CancelButton>::Create(this, WB_TABSTOP);
    AddButton(m_pbCancel);
    m_pbPrev = VclPtr<PushButton>::Create(this, WB_TABSTOP);
    AddButton(m_pbPrev);
    m_pbNext = VclPtr<PushButton>::Create(this, WB_TABSTOP);
    AddButton(m_pbNext);
    m_pbFinish = VclPtr<PushButton>::Create(this, WB_TABSTOP);
    AddButton(m_pbFinish);

    m_pbHelp->SetSizePixel( LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)) );
    m_pbCancel->SetSizePixel( LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)) );
    m_pbPrev->SetSizePixel( LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)) );
    m_pbNext->SetSizePixel( LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)) );
    m_pbFinish->SetSizePixel( LogicToPixel(Size(50, 14), MapMode(MapUnit::MapAppFont)) );

    m_pbPrev->SetText(DBA_RES(STR_WIZ_PB_PREV));
    m_pbNext->SetText(DBA_RES(STR_WIZ_PB_NEXT));
    m_pbFinish->SetText(DBA_RES(STR_WIZ_PB_OK));

    m_pbHelp->Show();
    m_pbCancel->Show();
    m_pbPrev->Show();
    m_pbNext->Show();
    m_pbFinish->Show();

    m_pbPrev->SetClickHdl( LINK( this, OCopyTableWizard, ImplPrevHdl ) );
    m_pbNext->SetClickHdl( LINK( this, OCopyTableWizard, ImplNextHdl ) );
    m_pbFinish->SetClickHdl( LINK( this, OCopyTableWizard, ImplOKHdl ) );

    SetActivatePageHdl( LINK( this, OCopyTableWizard, ImplActivateHdl ) );

    SetPrevButton( m_pbPrev );
    SetNextButton( m_pbNext );

    m_pbNext->GrabFocus();

    if (!m_vDestColumns.empty())
        // source is a html or rtf table
        m_pbNext->SetStyle(m_pbFinish->GetStyle() | WB_DEFBUTTON);
    else
        m_pbFinish->SetStyle(m_pbFinish->GetStyle() | WB_DEFBUTTON);

    m_pTypeInfo = std::make_shared<OTypeInfo>();
    m_pTypeInfo->aUIName = m_sTypeNames.getToken(TYPE_OTHER, ';');
    m_bAddPKFirstTime = true;
}

OCopyTableWizard::~OCopyTableWizard()
{
    disposeOnce();
}

void OCopyTableWizard::dispose()
{
    for ( ;; )
    {
        VclPtr<TabPage> pPage = GetPage(0);
        if ( pPage == nullptr )
            break;
        RemovePage( pPage );
        pPage.disposeAndClear();
    }

    if ( m_bDeleteSourceColumns )
        clearColumns(m_vSourceColumns,m_vSourceVec);

    clearColumns(m_vDestColumns,m_aDestVec);

    // clear the type information
    m_aTypeInfoIndex.clear();
    m_aTypeInfo.clear();
    m_aDestTypeInfoIndex.clear();

    m_pbHelp.disposeAndClear();
    m_pbCancel.disposeAndClear();
    m_pbPrev.disposeAndClear();
    m_pbNext.disposeAndClear();
    m_pbFinish.disposeAndClear();
    WizardDialog::dispose();
}

IMPL_LINK_NOARG(OCopyTableWizard, ImplPrevHdl, Button*, void)
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
}

IMPL_LINK_NOARG(OCopyTableWizard, ImplNextHdl, Button*, void)
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
}

bool OCopyTableWizard::CheckColumns(sal_Int32& _rnBreakPos)
{
    bool bRet = true;
    m_vColumnPositions.clear();
    m_vColumnTypes.clear();

    OSL_ENSURE( m_xDestConnection.is(), "OCopyTableWizard::CheckColumns: No connection!" );
    // If database is able to process PrimaryKeys, set PrimaryKey
    if ( m_xDestConnection.is() )
    {
        bool bPKeyAllowed = supportsPrimaryKey();

        bool bContainsColumns = !m_vDestColumns.empty();

        if ( bPKeyAllowed && shouldCreatePrimaryKey() )
        {
            // add extra column for the primary key
            TOTypeInfoSP pTypeInfo = queryPrimaryKeyType(m_aDestTypeInfo);
            if ( pTypeInfo.get() )
            {
                if ( m_bAddPKFirstTime )
                {
                    // tdf#114955: since we chose to create a primary key
                    // be sure all other columns won't be in primary key
                    for (auto const& elem : m_vDestColumns)
                        elem.second->SetPrimaryKey(false);
                    OFieldDescription* pField = new OFieldDescription();
                    pField->SetName(m_aKeyName);
                    pField->FillFromTypeInfo(pTypeInfo,true,true);
                    pField->SetPrimaryKey(true);
                    m_bAddPKFirstTime = false;
                    insertColumn(0,pField);
                }
                m_vColumnPositions.emplace_back(1,1);
                m_vColumnTypes.push_back(pTypeInfo->nType);
            }
        }

        if ( bContainsColumns )
        {   // we have dest columns so look for the matching column
            for (auto const& elemSource : m_vSourceVec)
            {
                ODatabaseExport::TColumns::const_iterator aDestIter = m_vDestColumns.find(m_mNameMapping[elemSource->first]);

                if ( aDestIter != m_vDestColumns.end() )
                {
                    ODatabaseExport::TColumnVector::const_iterator aFind = std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                    sal_Int32 nPos = (aFind - m_aDestVec.begin())+1;
                    m_vColumnPositions.emplace_back(nPos,nPos);
                    m_vColumnTypes.push_back((*aFind)->second->GetType());
                }
                else
                {
                    m_vColumnPositions.emplace_back( COLUMN_POSITION_NOT_FOUND, COLUMN_POSITION_NOT_FOUND );
                    m_vColumnTypes.push_back(0);
                }
            }
        }
        else
        {
            Reference< XDatabaseMetaData > xMetaData( m_xDestConnection->getMetaData() );
            OUString sExtraChars = xMetaData->getExtraNameCharacters();
            sal_Int32 nMaxNameLen       = getMaxColumnNameLength();

            _rnBreakPos=0;
            for (auto const& elemSource : m_vSourceVec)
            {
                OFieldDescription* pField = new OFieldDescription(*elemSource->second);
                pField->SetName(convertColumnName(TExportColumnFindFunctor(&m_vDestColumns),elemSource->first,sExtraChars,nMaxNameLen));
                TOTypeInfoSP pType = convertType(elemSource->second->getSpecialTypeInfo(),bRet);
                pField->SetType(pType);
                if ( !bPKeyAllowed )
                    pField->SetPrimaryKey(false);

                // now create a column
                insertColumn(m_vDestColumns.size(),pField);
                m_vColumnPositions.emplace_back(m_vDestColumns.size(),m_vDestColumns.size());
                m_vColumnTypes.push_back(elemSource->second->GetType());
                ++_rnBreakPos;
                if (!bRet)
                    break;
            }
        }
    }
    return bRet;
}

IMPL_LINK_NOARG(OCopyTableWizard, ImplOKHdl, Button*, void)
{
    m_ePressed = WIZARD_FINISH;
    bool bFinish = DeactivatePage();

    if(bFinish)
    {
        WaitObject aWait(this);
        switch(getOperation())
        {
            case CopyTableOperation::CopyDefinitionAndData:
            case CopyTableOperation::CopyDefinitionOnly:
            {
                bool bOnFirstPage = GetCurLevel() == 0;
                if ( bOnFirstPage )
                {
                    // we came from the first page so we have to clear
                    // all column information already collected
                    clearDestColumns();
                    m_mNameMapping.clear();
                }
                sal_Int32 nBreakPos = 0;
                bool bCheckOk = CheckColumns(nBreakPos);
                if ( bOnFirstPage && !bCheckOk )
                {
                    showColumnTypeNotSupported(m_vSourceVec[nBreakPos-1]->first);
                    OWizTypeSelect* pPage = static_cast<OWizTypeSelect*>(GetPage(3));
                    if ( pPage )
                    {
                        m_mNameMapping.clear();
                        pPage->setDisplayRow(nBreakPos);
                        ShowPage(3);
                        return;
                    }
                }
                if ( m_xDestConnection.is() )
                {
                    if ( supportsPrimaryKey() )
                    {
                        bool noPrimaryKey = std::none_of(m_vDestColumns.begin(),m_vDestColumns.end(),
                            [] (const ODatabaseExport::TColumns::value_type& tCol) { return tCol.second->IsPrimaryKey(); });
                        if ( noPrimaryKey && m_xInteractionHandler.is() )
                        {

                            OUString sMsg(DBA_RES(STR_TABLEDESIGN_NO_PRIM_KEY));
                            SQLContext aError;
                            aError.Message = sMsg;
                            ::rtl::Reference xRequest( new ::comphelper::OInteractionRequest( makeAny( aError ) ) );
                            ::rtl::Reference xYes = new ::comphelper::OInteractionApprove;
                            xRequest->addContinuation( xYes.get() );
                            xRequest->addContinuation( new ::comphelper::OInteractionDisapprove );
                            ::rtl::Reference< ::comphelper::OInteractionAbort > xAbort = new ::comphelper::OInteractionAbort;
                            xRequest->addContinuation( xAbort.get() );

                            m_xInteractionHandler->handle( xRequest.get() );

                            if ( xYes->wasSelected() )
                            {
                                OCopyTable* pPage = static_cast<OCopyTable*>(GetPage(0));
                                m_bCreatePrimaryKeyColumn = true;
                                m_aKeyName = pPage->GetKeyName();
                                if ( m_aKeyName.isEmpty() )
                                    m_aKeyName = "ID";
                                m_aKeyName = createUniqueName( m_aKeyName );
                                sal_Int32 nBreakPos2 = 0;
                                CheckColumns(nBreakPos2);
                            }
                            else if ( xAbort->wasSelected() )
                            {
                                ShowPage(3);
                                return;
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
}


void OCopyTableWizard::setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName )
{
    m_bCreatePrimaryKeyColumn = _bDoCreate;
    if ( !_rSuggestedName.isEmpty() )
        m_aKeyName = _rSuggestedName;

    OCopyTable* pSettingsPage = dynamic_cast< OCopyTable* >( GetPage( 0 ) );
    OSL_ENSURE( pSettingsPage, "OCopyTableWizard::setCreatePrimaryKey: page should have been added in the ctor!" );
    if ( pSettingsPage )
        pSettingsPage->setCreatePrimaryKey( _bDoCreate, _rSuggestedName );
}

IMPL_LINK_NOARG(OCopyTableWizard, ImplActivateHdl, WizardDialog*, void)
{
    OWizardPage* pCurrent = static_cast<OWizardPage*>(GetPage(GetCurLevel()));
    if(pCurrent)
    {
        bool bFirstTime = pCurrent->IsFirstTime();
        if(bFirstTime)
            pCurrent->Reset();

        CheckButtons();

        SetText(pCurrent->GetTitle());

        Invalidate();
    }
}

void OCopyTableWizard::CheckButtons()
{
    if(GetCurLevel() == 0) // the first page has no back button
    {
        if(m_nPageCount > 1)
            m_pbNext->Enable();
        else
            m_pbNext->Enable(false);

        m_pbPrev->Enable(false);
    }
    else if(GetCurLevel() == m_nPageCount-1) // the last page has no next button
    {
        m_pbNext->Enable(false);
        m_pbPrev->Enable();
    }
    else
    {
        m_pbPrev->Enable();
        // next already has its state
    }
}

void OCopyTableWizard::EnableNextButton(bool bEnable)
{
    m_pbNext->Enable(bEnable);
}

bool OCopyTableWizard::DeactivatePage()
{
    OWizardPage* pPage = static_cast<OWizardPage*>(GetPage(GetCurLevel()));
    return pPage && pPage->LeavePage();
}

void OCopyTableWizard::AddWizardPage(OWizardPage* pPage)
{
    AddPage(pPage);
    ++m_nPageCount;
}

void OCopyTableWizard::insertColumn(sal_Int32 _nPos,OFieldDescription* _pField)
{
    OSL_ENSURE(_pField,"FieldDescrioption is null!");
    if ( _pField )
    {
        ODatabaseExport::TColumns::const_iterator aFind = m_vDestColumns.find(_pField->GetName());
        if ( aFind != m_vDestColumns.end() )
        {
            delete aFind->second;
            m_vDestColumns.erase(aFind);
        }

        m_aDestVec.insert(m_aDestVec.begin() + _nPos,
            m_vDestColumns.emplace(_pField->GetName(),_pField).first);
        m_mNameMapping[_pField->GetName()] = _pField->GetName();
    }
}

void OCopyTableWizard::replaceColumn(sal_Int32 _nPos,OFieldDescription* _pField,const OUString& _sOldName)
{
    OSL_ENSURE(_pField,"FieldDescrioption is null!");
    if ( _pField )
    {
        m_vDestColumns.erase(_sOldName);
        OSL_ENSURE( m_vDestColumns.find(_pField->GetName()) == m_vDestColumns.end(),"Column with that name already exist!");

        m_aDestVec[_nPos] = m_vDestColumns.emplace(_pField->GetName(),_pField).first;
    }
}

void OCopyTableWizard::loadData(  const ICopyTableSourceObject& _rSourceObject, ODatabaseExport::TColumns& _rColumns, ODatabaseExport::TColumnVector& _rColVector )
{
    for (auto const& column : _rColumns)
        delete column.second;

    _rColVector.clear();
    _rColumns.clear();

    OFieldDescription* pActFieldDescr = nullptr;
    OUString const sCreateParam("x");
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
        bool bAutoIncrement   = pActFieldDescr->IsAutoIncrement();
        OUString sTypeName = pActFieldDescr->GetTypeName();

        // search for type
        bool bForce;
        TOTypeInfoSP pTypeInfo = ::dbaui::getTypeInfoFromType(m_aTypeInfo,nType,sTypeName,sCreateParam,nPrecision,nScale,bAutoIncrement,bForce);
        if ( !pTypeInfo.get() )
            pTypeInfo = m_pTypeInfo;

        pActFieldDescr->FillFromTypeInfo(pTypeInfo,true,false);
        _rColVector.emplace_back(_rColumns.emplace(pActFieldDescr->GetName(),pActFieldDescr).first);
    }

    // determine which columns belong to the primary key
    Sequence< OUString > aPrimaryKeyColumns( _rSourceObject.getPrimaryKeyColumnNames() );
    const OUString* pKeyColName  = aPrimaryKeyColumns.getConstArray();
    const OUString* pKeyColEnd   = pKeyColName + aPrimaryKeyColumns.getLength();

    for( ; pKeyColName != pKeyColEnd; ++pKeyColName )
    {
        ODatabaseExport::TColumns::const_iterator keyPos = _rColumns.find( *pKeyColName );
        if ( keyPos != _rColumns.end() )
        {
            keyPos->second->SetPrimaryKey( true );
            keyPos->second->SetIsNullable( ColumnValue::NO_NULLS );
        }
    }
}

void OCopyTableWizard::clearDestColumns()
{
    clearColumns(m_vDestColumns,m_aDestVec);
    m_bAddPKFirstTime = true;
    m_mNameMapping.clear();
}

void OCopyTableWizard::appendColumns( Reference<XColumnsSupplier> const & _rxColSup, const ODatabaseExport::TColumnVector* _pVec, bool _bKeyColumns)
{
    // now append the columns
    OSL_ENSURE(_rxColSup.is(),"No columns supplier");
    if(!_rxColSup.is())
        return;
    Reference<XNameAccess> xColumns = _rxColSup->getColumns();
    OSL_ENSURE(xColumns.is(),"No columns");
    Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);

    Reference<XAppend> xAppend(xColumns,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    for (auto const& elem : *_pVec)
    {
        OFieldDescription* pField = elem->second;
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
            xColumn = nullptr;
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

void OCopyTableWizard::appendKey( Reference<XKeysSupplier> const & _rxSup, const ODatabaseExport::TColumnVector* _pVec)
{
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
        appendColumns(xColSup,_pVec,true);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        if(xColumns.is() && xColumns->getElementNames().getLength())
            xAppend->appendByDescriptor(xKey);
    }

}

Reference< XPropertySet > OCopyTableWizard::createView() const
{
    OUString sCommand( m_rSourceObject.getSelectStatement() );
    OSL_ENSURE( !sCommand.isEmpty(), "OCopyTableWizard::createView: no statement in the source object!" );
        // there are legitimate cases in which getSelectStatement does not provide a statement,
        // but in all those cases, this method here should never be called.
    return ::dbaui::createView( m_sName, m_xDestConnection, sCommand );
}

Reference< XPropertySet > OCopyTableWizard::createTable()
{
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
            return nullptr;

        xTable = xFact->createDataDescriptor();
        OSL_ENSURE(xTable.is(),"Could not create a new object!");
        if(!xTable.is())
            return nullptr;

        OUString sCatalog,sSchema,sTable;
        Reference< XDatabaseMetaData> xMetaData = m_xDestConnection->getMetaData();
        ::dbtools::qualifiedNameComponents(xMetaData,
                                            m_sName,
                                            sCatalog,
                                            sSchema,
                                            sTable,
                                            ::dbtools::EComposeRule::InDataManipulation);

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
        const ODatabaseExport::TColumnVector& rVec = getDestVector();
        appendColumns( xSuppDestinationColumns, &rVec );
        // now append the primary key
        Reference<XKeysSupplier> xKeySup(xTable,UNO_QUERY);
        appendKey(xKeySup, &rVec);

        Reference<XAppend> xAppend(xTables,UNO_QUERY);
        if(xAppend.is())
            xAppend->appendByDescriptor(xTable);

        //  xTable = NULL;
        // we need to reget the table because after appending it, it is no longer valid
        if(xTables->hasByName(m_sName))
            xTables->getByName(m_sName) >>= xTable;
        else
        {
            OUString sComposedName(
                ::dbtools::composeTableName( m_xDestConnection->getMetaData(), xTable, ::dbtools::EComposeRule::InDataManipulation, false ) );
            if(xTables->hasByName(sComposedName))
            {
                xTables->getByName(sComposedName) >>= xTable;
                m_sName = sComposedName;
            }
            else
                xTable = nullptr;
        }
        if(xTable.is())
        {
            xSuppDestinationColumns.set( xTable, UNO_QUERY_THROW );
            // insert new table name into table filter
            ::dbaui::appendToFilter(m_xDestConnection, m_sName, GetComponentContext(), GetFrameWeld());

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
                    ODatabaseExport::TColumnVector::const_iterator aFind = std::find(m_aDestVec.begin(),m_aDestVec.end(),aDestIter);
                    sal_Int32 nPos = (aFind - m_aDestVec.begin())+1;

                    ODatabaseExport::TPositions::iterator aPosFind = std::find_if(
                        m_vColumnPositions.begin(),
                        m_vColumnPositions.end(),
                        [nPos] (const ODatabaseExport::TPositions::value_type& tPos) {
                            return tPos.first == nPos;
                        }
                    );

                    if ( m_vColumnPositions.end() != aPosFind )
                    {
                        aPosFind->second = nNewPos;
                        OSL_ENSURE( m_vColumnTypes.size() > size_t( aPosFind - m_vColumnPositions.begin() ),
                            "Invalid index for vector!" );
                        m_vColumnTypes[ aPosFind - m_vColumnPositions.begin() ] = (*aFind)->second->GetType();
                    }
                }
            }
        }
    }
    else if(xTables.is() && xTables->hasByName(m_sName))
        xTables->getByName(m_sName) >>= xTable;

    return xTable;
}

bool OCopyTableWizard::supportsPrimaryKey( const Reference< XConnection >& _rxConnection )
{
    OSL_PRECOND( _rxConnection.is(), "OCopyTableWizard::supportsPrimaryKey: invalid connection!" );
    if ( !_rxConnection.is() )
        return false;

    ::dbtools::DatabaseMetaData aMetaData( _rxConnection );
    return aMetaData.supportsPrimaryKeys();
}

bool OCopyTableWizard::supportsViews( const Reference< XConnection >& _rxConnection )
{
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
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    return bSupportsViews;
}

sal_Int32 OCopyTableWizard::getMaxColumnNameLength() const
{
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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
    return nLen;
}

void OCopyTableWizard::setOperation( const sal_Int16 _nOperation )
{
    m_nOperation = _nOperation;
}


OUString OCopyTableWizard::convertColumnName(const TColumnFindFunctor&   _rCmpFunctor,
                                                    const OUString&  _sColumnName,
                                                    const OUString&  _sExtraChars,
                                                    sal_Int32               _nMaxNameLen)
{
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
            sName += OUString::number(nPos);

            while(_rCmpFunctor(sName))
            {
                sName = sAlias + OUString::number(++nPos);
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

void OCopyTableWizard::removeColumnNameFromNameMap(const OUString& _sName)
{
    m_mNameMapping.erase(_sName);
}

bool OCopyTableWizard::supportsType(sal_Int32 _nDataType,   sal_Int32& _rNewDataType)
{
    bool bRet = m_aDestTypeInfo.find(_nDataType) != m_aDestTypeInfo.end();
    if ( bRet )
        _rNewDataType = _nDataType;
    return bRet;
}

TOTypeInfoSP OCopyTableWizard::convertType(const TOTypeInfoSP& _pType, bool& _bNotConvert)
{
    if ( !m_bInterConnectionCopy )
        // no need to convert if the source and destination connection are the same
        return _pType;

    bool bForce;
    TOTypeInfoSP pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,_pType->nType,_pType->aTypeName,_pType->aCreateParams,_pType->nPrecision,_pType->nMaximumScale,_pType->bAutoIncrement,bForce);
    if ( !pType.get() || bForce )
    { // no type found so we have to find the correct one ourself
        sal_Int32 nDefaultType = DataType::VARCHAR;
        switch(_pType->nType)
        {
            case DataType::TINYINT:
                if(supportsType(DataType::SMALLINT,nDefaultType))
                    break;
                [[fallthrough]];
            case DataType::SMALLINT:
                if(supportsType(DataType::INTEGER,nDefaultType))
                    break;
                [[fallthrough]];
            case DataType::INTEGER:
                if(supportsType(DataType::FLOAT,nDefaultType))
                    break;
                [[fallthrough]];
            case DataType::FLOAT:
                if(supportsType(DataType::REAL,nDefaultType))
                    break;
                [[fallthrough]];
            case DataType::DATE:
            case DataType::TIME:
                if( DataType::DATE == _pType->nType || DataType::TIME == _pType->nType )
                {
                    if(supportsType(DataType::TIMESTAMP,nDefaultType))
                        break;
                }
                [[fallthrough]];
            case DataType::TIMESTAMP:
            case DataType::REAL:
            case DataType::BIGINT:
                if ( supportsType(DataType::DOUBLE,nDefaultType) )
                    break;
                [[fallthrough]];
            case DataType::DOUBLE:
                if ( supportsType(DataType::NUMERIC,nDefaultType) )
                    break;
                [[fallthrough]];
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
            _bNotConvert = false;
            OUString const sCreate("x");
            pType = ::dbaui::getTypeInfoFromType(m_aDestTypeInfo,DataType::VARCHAR,_pType->aTypeName,sCreate,50,0,false,bForce);
            if ( !pType.get() )
                pType = m_pTypeInfo;
        }
        else if ( bForce )
            _bNotConvert = false;
    }
    return pType;
}

OUString OCopyTableWizard::createUniqueName(const OUString& _sName)
{
    OUString sName = _sName;
    Sequence< OUString > aColumnNames( m_rSourceObject.getColumnNames() );
    if ( aColumnNames.getLength() )
        sName = ::dbtools::createUniqueName( aColumnNames, sName, false );
    else
    {
        if ( m_vSourceColumns.find(sName) != m_vSourceColumns.end())
        {
            sal_Int32 nPos = 0;
            while(m_vSourceColumns.find(sName) != m_vSourceColumns.end())
            {
                sName = _sName + OUString::number(++nPos);
            }
        }
    }
    return sName;
}

void OCopyTableWizard::showColumnTypeNotSupported(const OUString& _rColumnName)
{
    OUString sMessage( DBA_RES( STR_UNKNOWN_TYPE_FOUND ) );
    sMessage = sMessage.replaceFirst("#1",_rColumnName);
    showError(sMessage);
}

void OCopyTableWizard::showError(const OUString& _sErrorMessage)
{
    SQLExceptionInfo aInfo(_sErrorMessage);
    showError(aInfo.get());
}

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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
