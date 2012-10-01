/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MDatabaseMetaDataHelper.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include <connectivity/dbexception.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/sequence.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>

// do we need it?
static ::osl::Mutex m_aMetaMutex;

#include <osl/diagnose.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <comphelper/processfactory.hxx>

using namespace connectivity;
using namespace connectivity::mork;

// -------------------------------------------------------------------------
MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()
{
    SAL_INFO("connectivity.mork", "=> MDatabaseMetaDataHelper::MDatabaseMetaDataHelper()" );
}

// -------------------------------------------------------------------------
MDatabaseMetaDataHelper::~MDatabaseMetaDataHelper()
{
}

sal_Bool MDatabaseMetaDataHelper::getTableStrings( OConnection*                      _pCon,
                                                   ::std::vector< ::rtl::OUString >& _rStrings)
{
    ::rtl::OUString                             sAbURI;
    ::rtl::OString                              sAbURIString;

    SAL_INFO("connectivity.mork", "=> MDatabaseMetaDataHelper::getTableStrings()");
    _pCon->getMorkParser();
    // TODO: retrieve Tables from MorkParser
    // only put for now the private adress book
    rtl::OUString table = rtl::OUString::createFromAscii( "AddressBook");
    _rStrings.push_back(table);
    return( sal_True );
}

sal_Bool MDatabaseMetaDataHelper::getTables( OConnection* _pCon,
                                             const ::rtl::OUString& tableNamePattern,
                                             ODatabaseMetaDataResultSet::ORows& _rRows)
{

    SAL_INFO("connectivity.mork", "=> MDatabaseMetaDataHelper::getTables()");

    static ODatabaseMetaDataResultSet::ORows    aRows;

    SAL_INFO("connectivity.mork", "=> MDatabaseMetaDataHelper::getTables()" );
    ::osl::MutexGuard aGuard( m_aMetaMutex );

    ODatabaseMetaDataResultSet::ORows().swap(aRows); // this makes real clear where memory is freed as well
    aRows.clear();

    ::std::vector< ::rtl::OUString > tables;
//    ::std::vector< ::rtl::OUString > tabletypes;
    ::rtl::OUString matchAny = rtl::OUString::createFromAscii("%");

    if ( !getTableStrings( _pCon, tables ) )
        return sal_False;

    for ( size_t i = 0; i < tables.size(); i++ ) {
        ODatabaseMetaDataResultSet::ORow aRow(3);

        ::rtl::OUString aTableName  = tables[i];
        //::rtl::OUString aTableType      = tabletypes[i];
        SAL_INFO("connectivity.mork", "TableName: " << aTableName );


        // return tables to caller
        if (match( tableNamePattern, aTableName, '\0' ))
    {
            if ( aTableName.isEmpty() ) {
                aTableName = rtl::OUString::createFromAscii("AddressBook");
            }

            SAL_INFO("connectivity.mork", "TableName: " << aTableName);

            aRow.push_back( new ORowSetValueDecorator( aTableName ) ); // Table name
            aRow.push_back( new ORowSetValueDecorator( rtl::OUString::createFromAscii("TABLE") ) ); // Table type
            aRow.push_back( ODatabaseMetaDataResultSet::getEmptyValue() ); // Remarks
            aRows.push_back(aRow);
        }
    }

    _rRows = aRows;
    return(sal_True);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
