/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_connectivity.hxx"
#include <connectivity/sqlparse.hxx>
#include "connectivity/sqliterator.hxx"
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#include <com/sun/star/uno/Any.hxx>

#include "connectivity/sqlnode.hxx"
#include <ucbhelper/contentbroker.hxx>
#include <comphelper/regpathhelper.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/process.h>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>

#include <ucbhelper/content.hxx>
#include <osl/module.h>
#include <tools/config.hxx>

#include <stdio.h>
#include <osl/thread.hxx>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <time.h>


using namespace comphelper;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;

using namespace connectivity;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace com::sun::star::registry;

using ::rtl::OUString;
using ::rtl::OUStringToOString;

extern Reference< XMultiServiceFactory > InitializeFac( void );
Reference< XMultiServiceFactory > mMgr;

#define OUtoCStr( x ) (OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US ).getStr())
#define PRINTSTR(x) printf("%s",x);
#define PRINTLN(x) printf("%s\n",x);

const int testLDAP=0;
const int testMozilla=1;
const int testOp=2;
const int testOe=3;

static int testCount = 0;
static int testList[4] = {0,0,0,0};

int autoTest(Reference<XResultSet> &xRes);

void printColumns( Reference<XResultSet> &xRes )
{
    if(xRes.is())
    {
        char* aPat = " %-22s ";
        char* aPat_Short = " %-12s ";
        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
        OSL_TRACE( "ColumnCount = %d\n", xMeta->getColumnCount());
        for(sal_Int32 i=1;i<=xMeta->getColumnCount();++i)
        {
            const char *str = OUtoCStr(xMeta->getColumnName(i));
            OSL_TRACE( aPat, str );
            }
        OSL_TRACE("\n");
        OSL_TRACE("------------------------------------------------------------------------------------------\n");
    }
    else
    {
        OSL_TRACE(": FAILED to get a ResultSet \n");
        }
}
void printXResultSet( Reference<XResultSet> &xRes )
{
    if(xRes.is())
    {
        char* aPat = " %-22s ";
        char* aPat_Short = " %-12s ";
        Reference<XRow> xRow(xRes,UNO_QUERY);
        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
        for(sal_Int32 j=1;j<=xMeta->getColumnCount();++j)
        {
            try
            {
                const char *str = OUtoCStr(xRow->getString(j));
                OSL_TRACE( aPat_Short, str );
            } catch (...) {
            OSL_TRACE(" Ex ");
            }
        }
        OSL_TRACE("\n");
    }
    else
        OSL_TRACE("FAILED to get a ResultSet \n");
}

void printXResultSets( Reference<XResultSet> &xRes )
{
    if(xRes.is())
    {
        printColumns(xRes);
        sal_Int32 nRows = 0;
        while( xRes.is() && xRes->next())
        {
            printXResultSet(xRes);
            nRows++;
        }
        OSL_TRACE( "%d Row(s)\n", nRows);
    }else
        OSL_TRACE("FAILED to get a ResultSet \n");
}



int TestMetaData(Reference< ::com::sun::star::sdbc::XConnection> &pConnection)
{
    // Test some metadata
    Reference< XDatabaseMetaData > xDmd = pConnection->getMetaData();
    if ( xDmd.is() )
     {
        OSL_TRACE(": got DatabaseMetaData \n");

        OUString sQuoteStr = xDmd->getIdentifierQuoteString();
        OSL_TRACE( "Quote String : '%s'\n", OUtoCStr( sQuoteStr ) );

        OUString sSQLCmds = xDmd->getSQLKeywords();
        OSL_TRACE( "SQL Commands : '%s'\n", OUtoCStr( sSQLCmds ) );

        OSL_TRACE("Testing getColumns() : START\n");
        {
            Reference<XResultSet> xRes = xDmd->getColumns(
                                makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM(""))), // Catalog
                                OUString(RTL_CONSTASCII_USTRINGPARAM("%")),          // Schema
                                OUString(RTL_CONSTASCII_USTRINGPARAM("%")),          // TabName
                                OUString(RTL_CONSTASCII_USTRINGPARAM("%"))
                                );
            printXResultSets( xRes );
        }
        OSL_TRACE("Testing getColumns() : END\n");

        OSL_TRACE("Testing  getTypeInfo() : START\n");
        {
            Reference<XResultSet> xRes = xDmd-> getTypeInfo();
            printXResultSets( xRes );
        }
        OSL_TRACE("Testing  getTypeInfo() : END\n");

        OSL_TRACE("Testing getTables() : START\n");
         {
            Reference<XResultSet> xRes = xDmd->getTables(
                    makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM(""))), // Catalog
                    OUString(RTL_CONSTASCII_USTRINGPARAM("%")),          // Schema
                    OUString(RTL_CONSTASCII_USTRINGPARAM("%")),          // TabName
                    Sequence<rtl::OUString>() );
            printXResultSets( xRes );
        }
        OSL_TRACE("Testing getTables() : END\n");

    }
    else
        OSL_TRACE(": FAILED to get DatabaseMetaData \n");
    return 0;
}

void TestQuery(Reference< ::com::sun::star::sdbc::XConnection> &pConnection)
{
    // Try a query
    OSL_TRACE("Testing createStatement() & executeQuery() : START\n");
    Reference<XStatement> xStmt = pConnection->createStatement();
    Reference<XResultSet> xRes;
    if(xStmt.is())
    {
        OSL_TRACE(": got statement\n");
        OSL_TRACE(":   excuteQuery() : START \n");
//      SELECT "First Name", "Display Name", "E-mail" FROM tablename
        OUString sqlPrefix(RTL_CONSTASCII_USTRINGPARAM("SELECT  \"First Name\", \"Display Name\", \"E-mail\" FROM "));
        try
        {
            sal_Int32 times=0;
            Reference< XDatabaseMetaData > xDmd = pConnection->getMetaData();
            if ( xDmd.is() )
            {
                OSL_TRACE("getTables() : START\n");
                OUString qut      = xDmd->getIdentifierQuoteString();

                Reference<XResultSet> xRes = xDmd->getTables(
                    makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM(""))), // Catalog
                    OUString(RTL_CONSTASCII_USTRINGPARAM("%")),          // Schema
                    OUString(RTL_CONSTASCII_USTRINGPARAM("%")),          // TabName
                        Sequence<rtl::OUString>() );
                sal_Int32 nTables = 0;
                while( xRes.is() && xRes->next())
                {
                    Reference<XRow> xRow(xRes,UNO_QUERY);
                    const char *strTableName = OUtoCStr(xRow->getString(3));
                    OSL_TRACE("Testing Table:%s\n",strTableName);

                    Reference<XResultSet> tmpRes =
                        xStmt->executeQuery(sqlPrefix + qut + xRow->getString(3) + qut);
                    autoTest( tmpRes );
                    Reference<XCloseable> clsRes(tmpRes,UNO_QUERY);
                    clsRes->close();
                    nTables++;
                }
                OSL_TRACE("Tested Tables:%d\n",nTables);
            }
        } catch ( Exception &e ) {
            OSL_TRACE( "Exception caught : %s\n", OUtoCStr( e.Message) );
        }
//      catch (...) {
//          OSL_TRACE( "Non-UNO Exception caught\n" );
//      }
        OSL_TRACE("excuteQuery() : END \n");
    }
    else
    {
        OSL_TRACE(": FAILED to get statement\n");
    }
    OSL_TRACE("Testing createStatement() & executeQuery() : END\n");
}
Reference< ::com::sun::star::sdbc::XConnection> TestConnected
        (Reference< ::com::sun::star::sdbc::XDriver> &pDriver)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  pConnection;
    OSL_TRACE("Begin Connect!\n");
    OUString url;
    Sequence<PropertyValue> aValue;
    int nType=0;
    srand( (unsigned)time( NULL ) );
    nType = rand() % testCount + 1;
    int nIndex=0,nCount=0;
    for ( nIndex = 0; nIndex< 4;nIndex++)
    {
        if (testList[nIndex])
            nCount++;
        if (nCount == nType)
            break;
    }
    switch( nIndex)
    {
    case testLDAP:
        url=OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:ldap://"));
        aValue.realloc(2);
        aValue[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HostName"));
        aValue[0].Value <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sun-ds"));
        aValue[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseDN"));
        aValue[1].Value <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("dc=sun,dc=com"));
        break;
    case testMozilla:
        url=OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:mozilla://"));
        break;
    case testOp:
        url=OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:outlook://"));
        break;
    case testOe:
        url=OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:outlookexp://"));
        break;
    default:
        url=OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:mozilla://"));
        break;
    }
    pConnection =
        pDriver->connect(url,aValue);
   return pConnection;
 }

int autoTest(Reference<XResultSet> &xRes)
{
    printColumns(xRes);
    if(xRes.is())
    {
        sal_Int32 nRows = 0;
        while( xRes.is() && xRes->next())
        {
            nRows++;
        }
        OSL_TRACE( "%d Row(s)\n", nRows);
        sal_Int32 times;
        sal_Int32 pos;
        if (nRows)
        {
            for(times = 1;times < 10; times ++)
            {
                pos= rand() % nRows+1;
                OSL_TRACE("pos:%d\n",pos);
                xRes->absolute(pos);
                printXResultSet(xRes);
            }
        }
    }
    else
    {
        OSL_TRACE(": FAILED to get a ResultSet \n");
    }
    TimeValue               timeValue = { 1, 0 };  //sleep 1  Seconds to give time to other threads
    osl_waitThread(&timeValue);
   return 0;
}
void SAL_CALL mozThread(void*)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  m_xConnection;
    try
    {
        Reference< ::com::sun::star::sdbc::XDriver>
        m_xDriver(mMgr->createInstance(
           OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.sdbc.MozabDriver"))),
             UNO_QUERY);
       if(m_xDriver.is())
        {

           m_xConnection = TestConnected(m_xDriver);
           if(m_xConnection.is())
                {
                if (!TestMetaData(m_xConnection))
                {
                    TestQuery(m_xConnection);
                }
            }
            else
                OSL_TRACE("Can't connected!\n");

        }
        else
        {
            OSL_TRACE("No driver!\n");
        }
    } catch ( Exception &e ) {
        OSL_TRACE( "Exception caught : %s\n", OUtoCStr( e.Message) );
    }
//  catch (...) {
//         OSL_TRACE( "Non-UNO Exception caught\n" );
//    }
}
const int THREAD_COUNT=100;


void usage()
{
    PRINTLN("mozThread [opts] threadcount");
    PRINTLN("opts:");
    PRINTLN("-l test ldap");
    PRINTLN("-m test mozilla");
    PRINTLN("-p test outlook");
    PRINTLN("-e test outlook express");
    PRINTLN("0 < threadcount <= 100, default 100");
}
#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif

{
    OSL_TRACE("Init UNO\n");
    Reference< XMultiServiceFactory > xMgr =InitializeFac();
    int threadCount=THREAD_COUNT;
    int nAc;
    for (nAc = 1; nAc < argc; nAc ++)
    {
        if (strcmp(argv[nAc],"-l") ==0)
        {
            testList[testLDAP] = 1;
        }else if(strcmp(argv[nAc],"-m") ==0)
        {
            testList[testMozilla]=1;
        }else if(strcmp(argv[nAc],"-p") ==0)
        {
            testList[testOp]=1;
        }else if(strcmp(argv[nAc],"-e") ==0)
        {
            testList[testOe]=1;
        }else if(strcmp(argv[nAc],"-h") ==0 || strcmp(argv[nAc],"--help") ==0)
        {
            usage();
            return 0;
        }else
        {
            int tmpCount = atoi(argv[nAc]);
            if (tmpCount > 0 && tmpCount < threadCount)
            {
                threadCount = tmpCount;
            }
        }
    }
    testCount = testList[testLDAP] + testList[testMozilla] + testList[testOp] + testList[testOe];
    if ( testCount == 0)
    {
        testList[testLDAP] = 1;
        testCount = 1;
    }

    if (!xMgr.is())
    {
        OSL_TRACE("Error init UNO\n");
        return 1;
    }
    else
        OSL_TRACE("UNO initted\n");

    mMgr = xMgr;
    oslThread xThreads[THREAD_COUNT];
    int index=0;
    for(index=0;index < threadCount; index++)
    {
        xThreads[index] = osl_createThread(mozThread,(void*)NULL);
        TimeValue               timeValue = { 1, 0 };  //sleep 1  Seconds to give time to other threads
        osl_waitThread(&timeValue);
    }
    for(index=0;index < threadCount; index++)
    {
        if (osl_isThreadRunning(xThreads[index]))
            osl_joinWithThread(xThreads[index]);
    }
    OSL_TRACE("Exiting...\n");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
