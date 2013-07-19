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

#include "file/FDatabaseMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/ucb/SearchRecursion.hpp>
#include <com/sun/star/ucb/SearchCommandArgument.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/SortedDynamicResultSetFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <tools/urlobj.hxx>
#include "file/FDriver.hxx"
#include "file/FTable.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
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

DBG_NAME( file_ODatabaseMetaData )
ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon) : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
                        ,m_pConnection(_pCon)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::ODatabaseMetaData" );
    DBG_CTOR( file_ODatabaseMetaData, NULL );
}
// -------------------------------------------------------------------------
ODatabaseMetaData::~ODatabaseMetaData()
{
    DBG_DTOR( file_ODatabaseMetaData, NULL );
}
// -------------------------------------------------------------------------
Reference< XResultSet > ODatabaseMetaData::impl_getTypeInfo_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_getTypeInfo_throw" );
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTypeInfo );
}
// -------------------------------------------------------------------------
OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_getCatalogSeparator_throw" );
    return OUString();
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*tableNamePattern*/,
        const OUString& /*columnNamePattern*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getColumns" );
    SAL_WARN( "connectivity.drivers","Should be overloaded!");
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumns );
}

// -------------------------------------------------------------------------
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
                    aContentURL = INetURLObject( _rFolderOrDoc, INetURLObject::WAS_ENCODED );
                    aContentURL.Append( _rDocName );
                    aContent1 = ::ucbhelper::Content( aContentURL.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
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
            if ( aURL2.GetMainURL(INetURLObject::NO_DECODE) == aContentURL.GetMainURL(INetURLObject::NO_DECODE) )
                return -1;

            // the second context
            sal_Bool bCanAccess = sal_False;
            ::ucbhelper::Content aContent2;
            try
            {
                aContent2 = ::ucbhelper::Content( aURL2.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext() );
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
            SAL_WARN( "connectivity.drivers", "isCaseSensitiveParentFolder: caught an unexpected exception!" );
        }

        return nIsCS;
    }
}

// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern, const Sequence< OUString >& types ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getTables" );
    ::osl::MutexGuard aGuard( m_aMutex );


    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTables );
    Reference< XResultSet > xRef = pResult;

    // check if any type is given
    // when no types are given then we have to return all tables e.g. TABLE

    static const OUString aTable("TABLE");

    sal_Bool bTableFound = sal_True;
    sal_Int32 nLength = types.getLength();
    if(nLength)
    {
        bTableFound = sal_False;

        const OUString* pBegin = types.getConstArray();
        const OUString* pEnd = pBegin + nLength;
        for(;pBegin != pEnd;++pBegin)
        {
            if(*pBegin == aTable)
            {
                bTableFound = sal_True;
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
    pInfo[ 0 ].Ascending   = sal_True;

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

    sal_Bool bKnowCaseSensivity = sal_False;
    sal_Bool bCaseSensitiveDir = sal_True;
    sal_Bool bCheckEnabled = m_pConnection->isCheckEnabled();

    while(xResultSet->next())
    {
        aName = xRow->getString(1);
        aURL.SetSmartProtocol(INET_PROT_FILE);
        OUString sUrl = m_pConnection->getURL() + "/" + aName;
        aURL.SetSmartURL( sUrl );
        sThisContentExtension = aURL.getExtension();

        ODatabaseMetaDataResultSet::ORow aRow(3);
        aRow.reserve(6);
        sal_Bool bNewRow = sal_False;

        if ( !bKnowCaseSensivity )
        {
            bKnowCaseSensivity = sal_True;
            sal_Int16 nCase = isCaseSensitiveParentFolder( m_pConnection->getURL(), aURL.getName() );
            switch( nCase )
            {
            case 1:
                bCaseSensitiveDir = sal_True;
                break;
            case -1:
                bKnowCaseSensivity = sal_False;
                /** run through */
            case 0:
                bCaseSensitiveDir = sal_False;
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
                if ( match(tableNamePattern,aName,'\0') && ( !bCheckEnabled || ( bCheckEnabled && ((nChar < '0' || nChar > '9')))) )
                {
                    aRow.push_back(new ORowSetValueDecorator(aName));
                    bNewRow = sal_True;
                }
            }
        }
        else // no extension, filter myself
        {
            sal_Bool bErg = sal_False;
            do
            {
                if (aURL.getExtension().isEmpty())
                {
                    sal_Unicode nChar = aURL.getBase().getStr()[0];
                    if(match(tableNamePattern,aURL.getBase(),'\0') && ( !bCheckEnabled || ( bCheckEnabled && ((nChar < '0' || nChar > '9')))) )
                    {
                        aRow.push_back(new ORowSetValueDecorator(OUString(aURL.getBase())));
                        bNewRow = sal_True;
                    }
                    break;
                }
                else if ( ( bErg = xResultSet->next() ) != sal_False )
                {
                    aName = xRow->getString(1);
                    aURL.SetSmartURL(aName);
                }
            } while (bErg);
        }
        if(bNewRow)
        {
            aRow.push_back(new ORowSetValueDecorator(aTable));
            aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());

            aRows.push_back(aRow);
        }
    }

    pResult->setRows(aRows);

    return xRef;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxBinaryLiteralLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxRowSize" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxCatalogNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxCharLiteralLength" );
    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxColumnNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxColumnsInIndex" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxCursorNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxConnections" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxColumnsInTable" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_getMaxStatements_throw" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxTableNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_getMaxTablesInSelect_throw" );
    return 1;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getTablePrivileges" );
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
                static ODatabaseMetaDataResultSet::ORow aRow(8);

                aRow[2] = new ORowSetValueDecorator(*pBegin);
                aRow[6] = ODatabaseMetaDataResultSet::getSelectValue();
                aRow[7] = new ORowSetValueDecorator(OUString("NO"));
                aRows.push_back(aRow);

                Reference< XPropertySet> xTable;
                ::cppu::extractInterface(xTable,xNames->getByName(*pBegin));
                if(xTable.is())
                {
                    Reference<XUnoTunnel> xTunnel(xTable,UNO_QUERY);
                    if(xTunnel.is())
                    {
                        OFileTable* pTable = reinterpret_cast< OFileTable* >( xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId()) );
                        if(pTable)
                        {
                            if(!pTable->isReadOnly())
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
    }

    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::doesMaxRowSizeIncludeBlobs(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::doesMaxRowSizeIncludeBlobs" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::storesLowerCaseQuotedIdentifiers" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::storesLowerCaseIdentifiers" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::storesMixedCaseIdentifiers" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::storesUpperCaseQuotedIdentifiers" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::storesUpperCaseIdentifiers" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxIndexLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsNonNullableColumns" );
    return sal_False;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getCatalogTerm" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_getIdentifierQuoteString_throw" );
    static const OUString sQuote("\"");
    return sQuote;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getExtraNameCharacters" );
    return OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsDifferentTableCorrelationNames" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_isCatalogAtStart_throw" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::dataDefinitionIgnoredInTransactions" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::dataDefinitionCausesTransactionCommit" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsDataManipulationTransactionsOnly" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsPositionedDelete" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsPositionedUpdate" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsOpenStatementsAcrossRollback" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsOpenStatementsAcrossCommit" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsOpenCursorsAcrossCommit" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsOpenCursorsAcrossRollback" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsTransactionIsolationLevel" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsANSI92FullSQL" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsANSI92EntryLevelSQL" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsIntegrityEnhancementFacility" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSchemasInIndexDefinitions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsCatalogsInIndexDefinitions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsOuterJoins" );
    return sal_False;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getTableTypes" );
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
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxStatementLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxProcedureNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxSchemaNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsTransactions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::allProceduresAreCallable" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsStoredProcedures" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSelectForUpdate" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::allTablesAreSelectable" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::isReadOnly" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::usesLocalFiles" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::usesLocalFilePerTable" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsTypeConversion" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::nullPlusNonNullIsNull" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsColumnAliasing" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsTableCorrelationNames" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsConvert" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsExpressionsInOrderBy" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsGroupBy" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsGroupByBeyondSelect" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsGroupByUnrelated" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsMultipleTransactions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsMultipleResultSets" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsLikeEscapeClause" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsOrderByUnrelated" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsUnion" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsUnionAll" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsMixedCaseIdentifiers" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::nullsAreSortedAtEnd" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::nullsAreSortedAtStart" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::nullsAreSortedHigh" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::nullsAreSortedLow" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSchemasInProcedureCalls" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsCatalogsInProcedureCalls" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsCorrelatedSubqueries" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSubqueriesInComparisons" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSubqueriesInExists" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSubqueriesInIns" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsSubqueriesInQuantifieds" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsANSI92IntermediateSQL" );
    return sal_False;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getURL" );
    static const OUString aValue(  "sdbc:file:" );
    return aValue;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getUserName" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDriverName" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDriverVersion" );
    return OUString::valueOf((sal_Int32)1);
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDatabaseProductVersion" );
    return OUString::valueOf((sal_Int32)0);
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDatabaseProductName" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getProcedureTerm" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getSchemaTerm" );
    return OUString();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDriverMajorVersion" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDefaultTransactionIsolation" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getDriverMinorVersion" );
    return 0;
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getSQLKeywords" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getSearchStringEscape" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getStringFunctions" );
    return OUString("UCASE,LCASE,ASCII,LENGTH,OCTET_LENGTH,CHAR_LENGTH,CHARACTER_LENGTH,CHAR,CONCAT,LOCATE,SUBSTRING,LTRIM,RTRIM,SPACE,REPLACE,REPEAT,INSERT,LEFT,RIGHT");
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getTimeDateFunctions" );
    return OUString("DAYOFWEEK,DAYOFMONTH,DAYOFYEAR,MONTH,DAYNAME,MONTHNAME,QUARTER,WEEK,YEAR,HOUR,MINUTE,SECOND,CURDATE,CURTIME,NOW");
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getSystemFunctions" );
    return OUString();
}
// -------------------------------------------------------------------------
OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getNumericFunctions" );
    return OUString("ABS,SIGN,MOD,FLOOR,CEILING,ROUND,EXP,LN,LOG,LOG10,POWER,SQRT,PI,COS,SIN,TAN,ACOS,ASIN,ATAN,ATAN2,DEGREES,RADIANS");
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsExtendedSQLGrammar" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsCoreSQLGrammar" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsMinimumSQLGrammar" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsFullOuterJoins" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsLimitedOuterJoins" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxColumnsInGroupBy" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxColumnsInOrderBy" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxColumnsInSelect" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getMaxUserNameLength" );
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsResultSetType" );
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            return sal_True;
        case ResultSetType::SCROLL_INSENSITIVE:
        case ResultSetType::SCROLL_SENSITIVE:
            break;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetConcurrency( sal_Int32 setType, sal_Int32 /*concurrency*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsResultSetConcurrency" );
    switch(setType)
    {
        case ResultSetType::FORWARD_ONLY:
            return sal_True;
        case ResultSetType::SCROLL_INSENSITIVE:
        case ResultSetType::SCROLL_SENSITIVE:
            break;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::ownUpdatesAreVisible" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::ownDeletesAreVisible" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::ownInsertsAreVisible" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::othersUpdatesAreVisible" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::othersDeletesAreVisible" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::othersInsertsAreVisible" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::updatesAreDetected" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::deletesAreDetected" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::insertsAreDetected" );
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::supportsBatchUpdates" );
    return sal_False;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com ODatabaseMetaData::getUDTs" );
    return NULL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
