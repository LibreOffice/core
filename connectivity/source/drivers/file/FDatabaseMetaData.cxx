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

#include <file/FDatabaseMetaData.hxx>
#include <FDatabaseMetaDataResultSet.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/ucb/SearchRecursion.hpp>
#include <com/sun/star/ucb/SearchCommandArgument.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/SortedDynamicResultSetFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <file/FDriver.hxx>
#include <file/FTable.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/content.hxx>

using namespace com::sun::star::ucb;
using namespace connectivity::file;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon) : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
                        ,m_pConnection(_pCon)
{
}

ODatabaseMetaData::~ODatabaseMetaData()
{
}

Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTypeInfo );
}

OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return OUString();
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*tableNamePattern*/,
        const OUString& /*columnNamePattern*/ )
{
    SAL_WARN("connectivity.drivers", "ODatabaseMetaData::getColumns() should be overridden!");
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumns );
}


namespace
{
    sal_Int16 isCaseSensitiveParentFolder( const OUString& _rFolderOrDoc, const OUString& _rDocName )
    {
        sal_Int16 nIsCS = 1;
        try
        {
            // first get the real content for the URL
            INetURLObject aContentURL( _rFolderOrDoc );
            ::ucbhelper::Content aContent1;
            {
                ::ucbhelper::Content aFolderOrDoc( _rFolderOrDoc, Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                if ( aFolderOrDoc.isDocument() )
                    aContent1 = aFolderOrDoc;
                else
                {
                    aContentURL = INetURLObject( _rFolderOrDoc, INetURLObject::EncodeMechanism::WasEncoded );
                    aContentURL.Append( _rDocName );
                    aContent1 = ::ucbhelper::Content( aContentURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                }
            }

            // get two extensions which differ by case only
            OUString sExtension1(aContentURL.getExtension());
            OUString sExtension2(sExtension1.toAsciiLowerCase());
            if (sExtension2 == sExtension1)
            {
                // the extension was already in lower case
                sExtension2 = sExtension2.toAsciiUpperCase();
            }

            // the complete URL for the second extension
            INetURLObject aURL2( aContentURL );
            if (!sExtension2.isEmpty())
                aURL2.SetExtension( sExtension2 );
            if ( aURL2.GetMainURL(INetURLObject::DecodeMechanism::NONE) == aContentURL.GetMainURL(INetURLObject::DecodeMechanism::NONE) )
                return -1;

            // the second context
            bool bCanAccess = false;
            ::ucbhelper::Content aContent2;
            try
            {
                aContent2 = ::ucbhelper::Content( aURL2.GetMainURL( INetURLObject::DecodeMechanism::NONE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                bCanAccess = aContent2.isDocument();
            }
            catch( const Exception& )
            {
            }

            if ( bCanAccess )
            {
                // here we have two contents whose URLs differ by case only.
                // Now let's check if both really refer to the same object ....
                Reference< XContent > xContent1 = aContent1.get();
                Reference< XContent > xContent2 = aContent2.get();
                OSL_ENSURE( xContent1.is() && xContent2.is(), "isCaseSensitiveParentFolder: invalid content interfaces!" );
                if ( xContent1.is() && xContent2.is() )
                {
                    Reference< XContentIdentifier > xID1 = xContent1->getIdentifier();
                    Reference< XContentIdentifier > xID2 = xContent2->getIdentifier();
                    OSL_ENSURE( xID1.is() && xID2.is(), "isCaseSensitiveParentFolder: invalid ID interfaces!" );
                    if ( xID1.is() && xID2.is()
                         && ( UniversalContentBroker::create(
                                  comphelper::getProcessComponentContext() )->
                              compareContentIds( xID1, xID2 ) == 0 ) )
                    {
                        // finally, we know that the folder is not case-sensitive ....
                        nIsCS = 0;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "connectivity.drivers", "isCaseSensitiveParentFolder: caught an unexpected exception! " << exceptionToString(ex) );
        }

        return nIsCS;
    }
}


Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    ::osl::MutexGuard aGuard( m_aMutex );


    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTables );
    Reference< XResultSet > xRef = pResult;

    // check if any type is given
    // when no types are given then we have to return all tables e.g. TABLE

    static const char aTable[] = "TABLE";

    bool bTableFound = true;
    sal_Int32 nLength = types.getLength();
    if(nLength)
    {
        bTableFound = false;

        const OUString* pBegin = types.getConstArray();
        const OUString* pEnd = pBegin + nLength;
        for(;pBegin != pEnd;++pBegin)
        {
            if(*pBegin == aTable)
            {
                bTableFound = true;
                break;
            }
        }
    }
    if(!bTableFound)
        return xRef;

    Reference<XDynamicResultSet> xContent = m_pConnection->getDir();
    Reference < XSortedDynamicResultSetFactory > xSRSFac =
                SortedDynamicResultSetFactory::create( m_pConnection->getDriver()->getComponentContext() );

    Sequence< NumberedSortingInfo > aSortInfo( 1 );
    NumberedSortingInfo* pInfo = aSortInfo.getArray();
    pInfo[ 0 ].ColumnIndex = 1;
    pInfo[ 0 ].Ascending   = true;

    Reference < XAnyCompareFactory > xFactory;
    Reference< XDynamicResultSet > xDynamicResultSet;
    xDynamicResultSet = xSRSFac->createSortedDynamicResultSet( xContent, aSortInfo, xFactory );
    Reference<XResultSet> xResultSet = xDynamicResultSet->getStaticResultSet();

    Reference<XRow> xRow(xResultSet,UNO_QUERY);

    OUString aFilenameExtension = m_pConnection->getExtension();
    OUString sThisContentExtension;
    ODatabaseMetaDataResultSet::ORows aRows;
    // scan the directory for tables
    OUString aName;
    INetURLObject aURL;
    xResultSet->beforeFirst();

    bool bKnowCaseSensivity = false;
    bool bCaseSensitiveDir = true;
    bool bCheckEnabled = m_pConnection->isCheckEnabled();

    while(xResultSet->next())
    {
        aName = xRow->getString(1);
        aURL.SetSmartProtocol(INetProtocol::File);
        OUString sUrl = m_pConnection->getURL() + "/" + aName;
        aURL.SetSmartURL( sUrl );
        sThisContentExtension = aURL.getExtension();

        ODatabaseMetaDataResultSet::ORow aRow { nullptr, nullptr, nullptr };
        aRow.reserve(6);
        bool bNewRow = false;

        if ( !bKnowCaseSensivity )
        {
            bKnowCaseSensivity = true;
            sal_Int16 nCase = isCaseSensitiveParentFolder( m_pConnection->getURL(), aURL.getName() );
            switch( nCase )
            {
            case 1:
                bCaseSensitiveDir = true;
                break;
            case -1:
                bKnowCaseSensivity = false;
                [[fallthrough]];
            case 0:
                bCaseSensitiveDir = false;
            }
            if ( bKnowCaseSensivity )
            {
                m_pConnection->setCaseSensitiveExtension( bCaseSensitiveDir, OConnection::GrantAccess() );
                if ( !bCaseSensitiveDir )
                {
                    aFilenameExtension = aFilenameExtension.toAsciiLowerCase();
                }
            }
        }

        if (!aFilenameExtension.isEmpty())
        {
            if ( !bCaseSensitiveDir )
            {
                sThisContentExtension = sThisContentExtension.toAsciiLowerCase();
            }

            if ( sThisContentExtension == aFilenameExtension )
            {
                aName = aName.copy(0, (aName.getLength()-(aFilenameExtension.getLength()+1)));
                sal_Unicode nChar = aName.toChar();
                if ( match(tableNamePattern,aName,'\0') && ( !bCheckEnabled || (nChar < '0' || nChar > '9')) )
                {
                    aRow.push_back(new ORowSetValueDecorator(aName));
                    bNewRow = true;
                }
            }
        }
        else // no extension, filter myself
        {
            for (;;)
            {
                if (aURL.getExtension().isEmpty())
                {
                    sal_Unicode nChar = aURL.getBase()[0];
                    if( match(tableNamePattern,aURL.getBase(),'\0') && ( !bCheckEnabled || nChar < '0' || nChar > '9' ) )
                    {
                        aRow.push_back(new ORowSetValueDecorator(aURL.getBase()));
                        bNewRow = true;
                    }
                    break;
                }
                if ( !xResultSet->next() )
                {
                    break;
                }
                aName = xRow->getString(1);
                aURL.SetSmartURL(aName);
            }
        }
        if(bNewRow)
        {
            aRow.push_back(new ORowSetValueDecorator(OUString(aTable)));
            aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());

            aRows.push_back(aRow);
        }
    }

    pResult->setRows(aRows);

    return xRef;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return 1;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
    Reference< XResultSet > xRef = pResult;
    ODatabaseMetaDataResultSet::ORows aRows;


    Reference< XTablesSupplier > xTabSup = m_pConnection->createCatalog();
    if( xTabSup.is())
    {
        Reference< XNameAccess> xNames      = xTabSup->getTables();
        Sequence< OUString > aNames  = xNames->getElementNames();
        const OUString* pBegin = aNames.getConstArray();
        const OUString* pEnd = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if(match(tableNamePattern,*pBegin,'\0'))
            {
                ODatabaseMetaDataResultSet::ORow aRow(8);

                aRow[2] = new ORowSetValueDecorator(*pBegin);
                aRow[6] = ODatabaseMetaDataResultSet::getSelectValue();
                aRow[7] = new ORowSetValueDecorator(OUString("NO"));
                aRows.push_back(aRow);

                Reference< XPropertySet> xTable(
                    xNames->getByName(*pBegin), css::uno::UNO_QUERY);
                if(xTable.is())
                {
                    Reference<XUnoTunnel> xTunnel(xTable,UNO_QUERY);
                    if(xTunnel.is())
                    {
                        OFileTable* pTable = reinterpret_cast< OFileTable* >( xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId()) );
                        if(pTable && !pTable->isReadOnly())
                        {
                            aRow[6] = ODatabaseMetaDataResultSet::getInsertValue();
                            aRows.push_back(aRow);
                            if(!m_pConnection->showDeleted())
                            {
                                aRow[6] = ODatabaseMetaDataResultSet::getDeleteValue();
                                aRows.push_back(aRow);
                            }
                            aRow[6] = ODatabaseMetaDataResultSet::getUpdateValue();
                            aRows.push_back(aRow);
                            aRow[6] = ODatabaseMetaDataResultSet::getCreateValue();
                            aRows.push_back(aRow);
                            aRow[6] = ODatabaseMetaDataResultSet::getReadValue();
                            aRows.push_back(aRow);
                            aRow[6] = ODatabaseMetaDataResultSet::getAlterValue();
                            aRows.push_back(aRow);
                            aRow[6] = ODatabaseMetaDataResultSet::getDropValue();
                            aRows.push_back(aRow);
                        }
                    }
                }
            }
        }
    }

    pResult->setRows(aRows);
    return xRef;
}

sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  )
{
    return 0;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return false;
}

OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  )
{
    return OUString();
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    return OUString("\"");
}

OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return true;
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  )
{
    return false;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTableTypes );
    Reference< XResultSet > xRef = pResult;
    static ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("TABLE")));
        aRows.push_back(aRow);
    }
    pResult->setRows(aRows);
    return xRef;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    return 0;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return true;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return false;
}

OUString SAL_CALL ODatabaseMetaData::getURL(  )
{
    return OUString(  "sdbc:file:" );
}

OUString SAL_CALL ODatabaseMetaData::getUserName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getDriverName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  )
{
    return OUString::number(1);
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString::number(0);
}

OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  )
{
    return OUString();
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  )
{
    return 0;
}

OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  )
{
    return OUString("UCASE,LCASE,ASCII,LENGTH,OCTET_LENGTH,CHAR_LENGTH,CHARACTER_LENGTH,CHAR,CONCAT,LOCATE,SUBSTRING,LTRIM,RTRIM,SPACE,REPLACE,REPEAT,INSERT,LEFT,RIGHT");
}

OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  )
{
    return OUString("DAYOFWEEK,DAYOFMONTH,DAYOFYEAR,MONTH,DAYNAME,MONTHNAME,QUARTER,WEEK,YEAR,HOUR,MINUTE,SECOND,CURDATE,CURTIME,NOW");
}

OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  )
{
    return OUString("ABS,SIGN,MOD,FLOOR,CEILING,ROUND,EXP,LN,LOG,LOG10,POWER,SQRT,PI,COS,SIN,TAN,ACOS,ASIN,ATAN,ATAN2,DEGREES,RADIANS");
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return false;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0;
}

sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  )
{
    return 0;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType )
{
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            return true;
        case ResultSetType::SCROLL_INSENSITIVE:
        case ResultSetType::SCROLL_SENSITIVE:
            break;
    }
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 /*concurrency*/ )
{
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            return true;
        case ResultSetType::SCROLL_INSENSITIVE:
        case ResultSetType::SCROLL_SENSITIVE:
            break;
    }
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
