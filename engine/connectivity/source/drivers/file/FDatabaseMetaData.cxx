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

#include <config_fuzzers.h>

#include <file/FDatabaseMetaData.hxx>
#include <FDatabaseMetaDataResultSet.hxx>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/SortedDynamicResultSetFactory.hpp>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <file/FDriver.hxx>
#include <file/FTable.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <ucbhelper/content.hxx>

using namespace com::sun::star::ucb;
using namespace connectivity::file;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace cpo::uno;
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

Reference< XResultSet > ODatabaseMetaData::getColumns(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*tableNamePattern*/,
        const OUString& /*columnNamePattern*/ )
{
    SAL_WARN("connectivity.drivers", "ODatabaseMetaData::getColumns() should be overridden!");
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumns );
}


namespace
{
#if !ENABLE_FUZZERS
    sal_Int16 isCaseSensitiveParentFolder( const OUString& _rFolderOrDoc, std::u16string_view _rDocName )
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
                    aContent1 = std::move(aFolderOrDoc);
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
                // Now let's check if both really refer to the same object...
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
                        // finally, we know that the folder is not case-sensitive...
                        nIsCS = 0;
                    }
                }
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "connectivity.drivers", "isCaseSensitiveParentFolder" );
        }

        return nIsCS;
    }
#endif
}


Reference< XResultSet > ODatabaseMetaData::getTables(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTables );

    // check if any type is given
    // when no types are given then we have to return all tables e.g. TABLE

    static constexpr OUString aTable = u"TABLE"_ustr;

    bool bTableFound
        = !types.hasElements() || std::find(types.begin(), types.end(), aTable) != types.end();
    if(!bTableFound)
        return pResult;

    Reference<XDynamicResultSet> xContent = m_pConnection->getDir();
    Reference < XSortedDynamicResultSetFactory > xSRSFac =
                SortedDynamicResultSetFactory::create( m_pConnection->getDriver()->getComponentContext() );

    Sequence< NumberedSortingInfo > aSortInfo( 1 );
    NumberedSortingInfo* pInfo = aSortInfo.getArray();
    pInfo[ 0 ].ColumnIndex = 1;
    pInfo[ 0 ].Ascending   = true;

    Reference < XAnyCompareFactory > xFactory;
    Reference< XDynamicResultSet > xDynamicResultSet = xSRSFac->createSortedDynamicResultSet( xContent, aSortInfo, xFactory );
    Reference<XResultSet> xResultSet = xDynamicResultSet->getStaticResultSet();

    Reference<XRow> xRow(xResultSet,UNO_QUERY);

    OUString aFilenameExtension = m_pConnection->getExtension();
    OUString sThisContentExtension;
    ODatabaseMetaDataResultSet::ORows aRows;
    // scan the directory for tables
    OUString aName;
    INetURLObject aURL;
    xResultSet->beforeFirst();

    bool bKnowCaseSensitivity = false;
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

        if ( !bKnowCaseSensitivity )
        {
            bKnowCaseSensitivity = true;
#if ENABLE_FUZZERS
            sal_Int16 nCase = 1;
#else
            sal_Int16 nCase = isCaseSensitiveParentFolder( m_pConnection->getURL(), aURL.getName() );
#endif
            switch( nCase )
            {
            case 1:
                bCaseSensitiveDir = true;
                break;
            case -1:
                bKnowCaseSensitivity = false;
                [[fallthrough]];
            case 0:
                bCaseSensitiveDir = false;
            }
            if ( bKnowCaseSensitivity )
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
            aRow.push_back(new ORowSetValueDecorator(aTable));
            aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());

            aRows.push_back(std::move(aRow));
        }
    }

    pResult->setRows(std::move(aRows));

    return pResult;
}

sal_Int32 ODatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxRowSize(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxCatalogNameLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxCharLiteralLength(  )
{
    return SAL_MAX_INT32;
}

sal_Int32 ODatabaseMetaData::getMaxColumnNameLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxCursorNameLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxConnections(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInTable(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxTableNameLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return 1;
}

Reference< XResultSet > ODatabaseMetaData::getTablePrivileges(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
    ODatabaseMetaDataResultSet::ORows aRows;

    Reference< XTablesSupplier > xTabSup = m_pConnection->createCatalog();
    if( xTabSup.is())
    {
        Reference< XNameAccess> xNames      = xTabSup->getTables();
        for (const OUString& rName : xNames->getElementNames())
        {
            if (match(tableNamePattern, rName, '\0'))
            {
                ODatabaseMetaDataResultSet::ORow aRow(8);

                aRow[2] = new ORowSetValueDecorator(rName);
                aRow[6] = ODatabaseMetaDataResultSet::getSelectValue();
                aRow[7] = new ORowSetValueDecorator(u"NO"_ustr);
                aRows.push_back(aRow);

                if (auto xTable = xNames->getByName(rName).query<XPropertySet>())
                {
                    auto pTable = dynamic_cast<OFileTable*>(xTable.get());
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
                        aRows.push_back(std::move(aRow));
                    }
                }
            }
        }
    }

    pResult->setRows(std::move(aRows));
    return pResult;
}

bool ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  )
{
    return true;
}

bool ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::storesLowerCaseIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return false;
}

bool ODatabaseMetaData::storesMixedCaseIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  )
{
    return false;
}

bool ODatabaseMetaData::storesUpperCaseIdentifiers(  )
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

sal_Int32 ODatabaseMetaData::getMaxIndexLength(  )
{
    return 0;
}

bool ODatabaseMetaData::supportsNonNullableColumns(  )
{
    return false;
}

OUString ODatabaseMetaData::getCatalogTerm(  )
{
    return OUString();
}

OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    return u"\""_ustr;
}

OUString ODatabaseMetaData::getExtraNameCharacters(  )
{
    return OUString();
}

bool ODatabaseMetaData::supportsDifferentTableCorrelationNames(  )
{
    return true;
}

bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return true;
}

bool ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  )
{
    return true;
}

bool ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  )
{
    return true;
}

bool ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  )
{
    return false;
}

bool ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  )
{
    return false;
}

bool ODatabaseMetaData::supportsPositionedDelete(  )
{
    return false;
}

bool ODatabaseMetaData::supportsPositionedUpdate(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  )
{
    return false;
}

bool ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return false;
}

bool ODatabaseMetaData::supportsANSI92FullSQL(  )
{
    return false;
}

bool ODatabaseMetaData::supportsANSI92EntryLevelSQL(  )
{
    return false;
}

bool ODatabaseMetaData::supportsIntegrityEnhancementFacility(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSchemasInIndexDefinitions(  )
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

bool ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOuterJoins(  )
{
    return false;
}

Reference< XResultSet > ODatabaseMetaData::getTableTypes(  )
{
    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTableTypes );
    ODatabaseMetaDataResultSet::ORows aRows;
    aRows.push_back( { ODatabaseMetaDataResultSet::getEmptyValue(), new ORowSetValueDecorator(u"TABLE"_ustr) } );
    pResult->setRows(std::move(aRows));
    return pResult;
}

sal_Int32 ODatabaseMetaData::getMaxStatementLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxProcedureNameLength(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxSchemaNameLength(  )
{
    return 0;
}

bool ODatabaseMetaData::supportsTransactions(  )
{
    return false;
}

bool ODatabaseMetaData::allProceduresAreCallable(  )
{
    return false;
}

bool ODatabaseMetaData::supportsStoredProcedures(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSelectForUpdate(  )
{
    return false;
}

bool ODatabaseMetaData::allTablesAreSelectable(  )
{
    return true;
}

bool ODatabaseMetaData::isReadOnly(  )
{
    return true;
}

bool ODatabaseMetaData::usesLocalFiles(  )
{
    return true;
}

bool ODatabaseMetaData::usesLocalFilePerTable(  )
{
    return true;
}

bool ODatabaseMetaData::supportsTypeConversion(  )
{
    return false;
}

bool ODatabaseMetaData::nullPlusNonNullIsNull(  )
{
    return true;
}

bool ODatabaseMetaData::supportsColumnAliasing(  )
{
    return true;
}

bool ODatabaseMetaData::supportsTableCorrelationNames(  )
{
    return true;
}

bool ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ )
{
    return false;
}

bool ODatabaseMetaData::supportsExpressionsInOrderBy(  )
{
    return false;
}

bool ODatabaseMetaData::supportsGroupBy(  )
{
    return false;
}

bool ODatabaseMetaData::supportsGroupByBeyondSelect(  )
{
    return false;
}

bool ODatabaseMetaData::supportsGroupByUnrelated(  )
{
    return false;
}

bool ODatabaseMetaData::supportsMultipleTransactions(  )
{
    return false;
}

bool ODatabaseMetaData::supportsMultipleResultSets(  )
{
    return false;
}

bool ODatabaseMetaData::supportsLikeEscapeClause(  )
{
    return false;
}

bool ODatabaseMetaData::supportsOrderByUnrelated(  )
{
    return true;
}

bool ODatabaseMetaData::supportsUnion(  )
{
    return false;
}

bool ODatabaseMetaData::supportsUnionAll(  )
{
    return false;
}

bool ODatabaseMetaData::supportsMixedCaseIdentifiers(  )
{
    return true;
}

bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return false;
}

bool ODatabaseMetaData::nullsAreSortedAtEnd(  )
{
    return false;
}

bool ODatabaseMetaData::nullsAreSortedAtStart(  )
{
    return true;
}

bool ODatabaseMetaData::nullsAreSortedHigh(  )
{
    return false;
}

bool ODatabaseMetaData::nullsAreSortedLow(  )
{
    return true;
}

bool ODatabaseMetaData::supportsSchemasInProcedureCalls(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCatalogsInProcedureCalls(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCorrelatedSubqueries(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSubqueriesInComparisons(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSubqueriesInExists(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSubqueriesInIns(  )
{
    return false;
}

bool ODatabaseMetaData::supportsSubqueriesInQuantifieds(  )
{
    return false;
}

bool ODatabaseMetaData::supportsANSI92IntermediateSQL(  )
{
    return false;
}

OUString ODatabaseMetaData::getURL(  )
{
    return u"sdbc:file:"_ustr;
}

OUString ODatabaseMetaData::getUserName(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getDriverName(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getDriverVersion(  )
{
    return OUString::number(1);
}

OUString ODatabaseMetaData::getDatabaseProductVersion(  )
{
    return OUString::number(0);
}

OUString ODatabaseMetaData::getDatabaseProductName(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getProcedureTerm(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getSchemaTerm(  )
{
    return OUString();
}

sal_Int32 ODatabaseMetaData::getDriverMajorVersion(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getDefaultTransactionIsolation(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getDriverMinorVersion(  )
{
    return 0;
}

OUString ODatabaseMetaData::getSQLKeywords(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getSearchStringEscape(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getStringFunctions(  )
{
    return u"UCASE,LCASE,ASCII,LENGTH,OCTET_LENGTH,CHAR_LENGTH,CHARACTER_LENGTH,CHAR,CONCAT,LOCATE,SUBSTRING,LTRIM,RTRIM,SPACE,REPLACE,REPEAT,INSERT,LEFT,RIGHT"_ustr;
}

OUString ODatabaseMetaData::getTimeDateFunctions(  )
{
    return u"DAYOFWEEK,DAYOFMONTH,DAYOFYEAR,MONTH,DAYNAME,MONTHNAME,QUARTER,WEEK,YEAR,HOUR,MINUTE,SECOND,CURDATE,CURTIME,NOW"_ustr;
}

OUString ODatabaseMetaData::getSystemFunctions(  )
{
    return OUString();
}

OUString ODatabaseMetaData::getNumericFunctions(  )
{
    return u"ABS,SIGN,MOD,FLOOR,CEILING,ROUND,EXP,LN,LOG,LOG10,POWER,SQRT,PI,COS,SIN,TAN,ACOS,ASIN,ATAN,ATAN2,DEGREES,RADIANS"_ustr;
}

bool ODatabaseMetaData::supportsExtendedSQLGrammar(  )
{
    return false;
}

bool ODatabaseMetaData::supportsCoreSQLGrammar(  )
{
    return false;
}

bool ODatabaseMetaData::supportsMinimumSQLGrammar(  )
{
    return true;
}

bool ODatabaseMetaData::supportsFullOuterJoins(  )
{
    return false;
}

bool ODatabaseMetaData::supportsLimitedOuterJoins(  )
{
    return false;
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInGroupBy(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInOrderBy(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxColumnsInSelect(  )
{
    return 0;
}

sal_Int32 ODatabaseMetaData::getMaxUserNameLength(  )
{
    return 0;
}

bool ODatabaseMetaData::supportsResultSetType( sal_Int32 setType )
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

bool ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 /*concurrency*/ )
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

bool ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ )
{
    return true;
}

bool ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

bool ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

bool ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ )
{
    return false;
}

bool ODatabaseMetaData::supportsBatchUpdates(  )
{
    return false;
}

Reference< XResultSet > ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ )
{
    return nullptr;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
