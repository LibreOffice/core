/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FDatabaseMetaData.cxx,v $
 * $Revision: 1.36 $
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
#include "precompiled_connectivity.hxx"
#include "file/FDatabaseMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/ucb/SearchRecursion.hpp>
#include <com/sun/star/ucb/SearchCommandArgument.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <tools/urlobj.hxx>
#include "file/FDriver.hxx"
#include "file/FTable.hxx"
#include <comphelper/extract.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <tools/debug.hxx>


using namespace com::sun::star::ucb;
using namespace connectivity::file;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;

DBG_NAME( file_ODatabaseMetaData )
ODatabaseMetaData::ODatabaseMetaData(OConnection* _pCon) : ::connectivity::ODatabaseMetaDataBase(_pCon,_pCon->getConnectionInfo())
                        ,m_pConnection(_pCon)
{
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
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTypeInfo );
}
// -------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaData::impl_getCatalogSeparator_throw(  )
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getColumns(
        const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& /*tableNamePattern*/,
        const ::rtl::OUString& /*columnNamePattern*/ ) throw(SQLException, RuntimeException)
{
    OSL_ENSURE(0,"Should be overloaded!");
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumns );
}

// -------------------------------------------------------------------------
namespace
{
    sal_Int16 isCaseSensitiveParentFolder( const String& _rFolderOrDoc, const String& _rDocName )
    {
        sal_Int16 nIsCS = 1;
        try
        {
            // first get the real content for the URL
            INetURLObject aContentURL( _rFolderOrDoc );
            ::ucbhelper::Content aContent1;
            {
                ::ucbhelper::Content aFolderOrDoc( _rFolderOrDoc, Reference< XCommandEnvironment >() );
                if ( aFolderOrDoc.isDocument() )
                    aContent1 = aFolderOrDoc;
                else
                {
                    aContentURL = INetURLObject( _rFolderOrDoc, INetURLObject::WAS_ENCODED );
                    aContentURL.Append( _rDocName );
                    aContent1 = ::ucbhelper::Content( aContentURL.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );
                }
            }

            // get two extensions which differ by case only
            String sExtension1 = aContentURL.getExtension();
            String sExtension2( sExtension1 );
            sExtension2.ToLowerAscii();
            if ( sExtension2 == sExtension1 )
                // the extension was already in lower case
                sExtension2.ToUpperAscii();

            // the complete URL for the second extension
            INetURLObject aURL2( aContentURL );
            if ( sExtension2.Len() )
                aURL2.SetExtension( sExtension2 );
            if ( aURL2.GetMainURL(INetURLObject::NO_DECODE) == aContentURL.GetMainURL(INetURLObject::NO_DECODE) )
                return -1;

            // the second context
            sal_Bool bCanAccess = sal_False;
            ::ucbhelper::Content aContent2;
            try
            {
                aContent2 = ::ucbhelper::Content( aURL2.GetMainURL( INetURLObject::NO_DECODE ), Reference< XCommandEnvironment >() );
                bCanAccess = aContent2.isDocument();
            }
            catch( const Exception& )
            {
            }

            if ( bCanAccess )
            {
                // here we have two contents which's URLs differ by case only.
                // Now let's check if both really refer to the same object ....
                Reference< XContent > xContent1 = aContent1.get();
                Reference< XContent > xContent2 = aContent2.get();
                OSL_ENSURE( xContent1.is() && xContent2.is(), "isCaseSensitiveParentFolder: invalid content interfaces!" );
                if ( xContent1.is() && xContent2.is() )
                {
                    Reference< XContentIdentifier > xID1 = xContent1->getIdentifier();
                    Reference< XContentIdentifier > xID2 = xContent2->getIdentifier();
                    OSL_ENSURE( xID1.is() && xID2.is(), "isCaseSensitiveParentFolder: invalid ID interfaces!" );
                    if ( xID1.is() && xID2.is() )
                    {
                        // get a generic content provider
                        ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
                        Reference< XContentProvider > xProvider;
                        if ( pBroker )
                            xProvider = pBroker->getContentProviderInterface();
                        OSL_ENSURE( xProvider.is(), "isCaseSensitiveParentFolder: invalid content broker!" );
                        if ( xProvider.is() )
                        {
                            if ( 0 == xProvider->compareContentIds( xID1, xID2 ) )
                                // finally, we know that the folder is not case-sensitive ....
                                nIsCS = 0;
                        }
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "isCaseSensitiveParentFolder: caught an unexpected exception!" );
        }

        return nIsCS;
    }
}

// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTables(
        const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/,
        const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& types ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );


    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTables );
    Reference< XResultSet > xRef = pResult;

    // check if any type is given
    // when no types are given then we have to return all tables e.g. TABLE

    static const ::rtl::OUString aTable(::rtl::OUString::createFromAscii("TABLE"));

    sal_Bool bTableFound = sal_True;
    sal_Int32 nLength = types.getLength();
    if(nLength)
    {
        bTableFound = sal_False;

        const ::rtl::OUString* pBegin = types.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + nLength;
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
    Reference < XSortedDynamicResultSetFactory > xSRSFac(
                m_pConnection->getDriver()->getFactory()->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.ucb.SortedDynamicResultSetFactory") ), UNO_QUERY );

    Sequence< NumberedSortingInfo > aSortInfo( 1 );
    NumberedSortingInfo* pInfo = aSortInfo.getArray();
    pInfo[ 0 ].ColumnIndex = 1;
    pInfo[ 0 ].Ascending   = sal_True;

    Reference < XAnyCompareFactory > xFactory;
    Reference< XDynamicResultSet > xDynamicResultSet;
    xDynamicResultSet = xSRSFac->createSortedDynamicResultSet( xContent, aSortInfo, xFactory );
    Reference<XResultSet> xResultSet = xDynamicResultSet->getStaticResultSet();

    Reference<XRow> xRow(xResultSet,UNO_QUERY);

    String aFilenameExtension = m_pConnection->getExtension();
    String sThisContentExtension;
    ODatabaseMetaDataResultSet::ORows aRows;
    // scan the directory for tables
    ::rtl::OUString aName;
    INetURLObject aURL;
    xResultSet->beforeFirst();

    sal_Bool bKnowCaseSensivity = sal_False;
    sal_Bool bCaseSensitiveDir = sal_True;
    sal_Bool bCheckEnabled = m_pConnection->isCheckEnabled();

    while(xResultSet->next())
    {
        aName = xRow->getString(1);
        aURL.SetSmartProtocol(INET_PROT_FILE);
        String sUrl = m_pConnection->getURL() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + aName;
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
                    aFilenameExtension.ToLowerAscii();
            }
        }

        if (aFilenameExtension.Len())
        {
            if ( !bCaseSensitiveDir )
                sThisContentExtension.ToLowerAscii();

            if ( sThisContentExtension == aFilenameExtension )
            {
                aName = aName.replaceAt(aName.getLength()-(aFilenameExtension.Len()+1),aFilenameExtension.Len()+1,::rtl::OUString());
                sal_Unicode nChar = aName.toChar();
                if ( match(tableNamePattern,aName.getStr(),'\0') && ( !bCheckEnabled || ( bCheckEnabled && ((nChar < '0' || nChar > '9')))) )
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
                if (!aURL.getExtension().getLength())
                {
                    sal_Unicode nChar = aURL.getBase().getStr()[0];
                    if(match(tableNamePattern,aURL.getBase().getStr(),'\0') && ( !bCheckEnabled || ( bCheckEnabled && ((nChar < '0' || nChar > '9')))) )
                    {
                        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString(aURL.getBase())));
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
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxRowSize(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCatalogNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxCursorNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxConnections(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::impl_getMaxStatements_throw(  )
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxTableNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::impl_getMaxTablesInSelect_throw(  )
{
    return 1;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTablePrivileges(
        const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
    Reference< XResultSet > xRef = pResult;
    ODatabaseMetaDataResultSet::ORows aRows;


    Reference< XTablesSupplier > xTabSup = m_pConnection->createCatalog();
    if( xTabSup.is())
    {
        Reference< XNameAccess> xNames      = xTabSup->getTables();
        Sequence< ::rtl::OUString > aNames  = xNames->getElementNames();
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if(match(tableNamePattern,pBegin->getStr(),'\0'))
            {
                static ODatabaseMetaDataResultSet::ORow aRow(8);

                aRow[2] = new ORowSetValueDecorator(*pBegin);
                aRow[6] = ODatabaseMetaDataResultSet::getSelectValue();
                aRow[7] = new ORowSetValueDecorator(::rtl::OUString::createFromAscii("NO"));
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
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesLowerCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseQuotedIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::storesUpperCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsAlterTableWithAddColumn_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsAlterTableWithDropColumn_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxIndexLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsNonNullableColumns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getCatalogTerm(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaData::impl_getIdentifierQuoteString_throw(  )
{
    static const ::rtl::OUString sQuote = ::rtl::OUString::createFromAscii("\"");
    return sQuote;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getExtraNameCharacters(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDifferentTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_isCatalogAtStart_throw(  )
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionIgnoredInTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::dataDefinitionCausesTransactionCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataManipulationTransactionsOnly(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsDataDefinitionAndDataManipulationTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedDelete(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsPositionedUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenStatementsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossCommit(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOpenCursorsAcrossRollback(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactionIsolationLevel( sal_Int32 /*level*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsSchemasInDataManipulation_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92FullSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92EntryLevelSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsIntegrityEnhancementFacility(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsSchemasInTableDefinitions_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsCatalogsInTableDefinitions_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInIndexDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsCatalogsInDataManipulation_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getTableTypes(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTableTypes );
    Reference< XResultSet > xRef = pResult;
    static ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString::createFromAscii("TABLE")));
        aRows.push_back(aRow);
    }
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxStatementLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxProcedureNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxSchemaNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allProceduresAreCallable(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsStoredProcedures(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSelectForUpdate(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::allTablesAreSelectable(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFiles(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::usesLocalFilePerTable(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTypeConversion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullPlusNonNullIsNull(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsColumnAliasing(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsTableCorrelationNames(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsConvert( sal_Int32 /*fromType*/, sal_Int32 /*toType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExpressionsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupBy(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByBeyondSelect(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsGroupByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleTransactions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMultipleResultSets(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLikeEscapeClause(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsOrderByUnrelated(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnion(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsUnionAll(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMixedCaseIdentifiers(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtEnd(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedAtStart(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedHigh(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::nullsAreSortedLow(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSchemasInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInProcedureCalls(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCatalogsInPrivilegeDefinitions(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCorrelatedSubqueries(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInComparisons(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInExists(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInIns(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsSubqueriesInQuantifieds(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsANSI92IntermediateSQL(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    static const ::rtl::OUString aValue = ::rtl::OUString::createFromAscii("sdbc:file:");
    return aValue;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getUserName(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverName(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDriverVersion(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString::valueOf((sal_Int32)1);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductVersion(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString::valueOf((sal_Int32)0);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getDatabaseProductName(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getProcedureTerm(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSchemaTerm(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMajorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDefaultTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getDriverMinorVersion(  ) throw(RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSQLKeywords(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSearchStringEscape(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getStringFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UCASE,LCASE,ASCII,LENGTH,OCTET_LENGTH,CHAR_LENGTH,CHARACTER_LENGTH,CHAR,CONCAT,LOCATE,SUBSTRING,LTRIM,RTRIM,SPACE,REPLACE,REPEAT,INSERT,LEFT,RIGHT"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getTimeDateFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DAYOFWEEK,DAYOFMONTH,DAYOFYEAR,MONTH,DAYNAME,MONTHNAME,QUARTER,WEEK,YEAR,HOUR,MINUTE,SECOND,CURDATE,CURTIME,NOW"));
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getSystemFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaData::getNumericFunctions(  ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ABS,SIGN,MOD,FLOOR,CEILING,ROUND,EXP,LN,LOG,LOG10,POWER,SQRT,PI,COS,SIN,TAN,ACOS,ASIN,ATAN,ATAN2,DEGREES,RADIANS"));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsExtendedSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsCoreSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsMinimumSQLGrammar(  ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsFullOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsLimitedOuterJoins(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInGroupBy(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInOrderBy(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxColumnsInSelect(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODatabaseMetaData::getMaxUserNameLength(  ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsResultSetType( sal_Int32 setType ) throw(SQLException, RuntimeException)
{
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
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::ownInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersUpdatesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersDeletesAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::othersInsertsAreVisible( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::updatesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::deletesAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::insertsAreDetected( sal_Int32 /*setType*/ ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaData::supportsBatchUpdates(  ) throw(SQLException, RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODatabaseMetaData::getUDTs( const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& /*typeNamePattern*/, const Sequence< sal_Int32 >& /*types*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}


