/*************************************************************************
 *
 *  $RCSfile: mozthread.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 18:35:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include "connectivity/sqliterator.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDELETEROWS_HPP_
#include <com/sun/star/sdbcx/XDeleteRows.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/sdbc/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include "connectivity/sqlnode.hxx"
#include <ucbhelper/contentbroker.hxx>
#include <comphelper/regpathhelper.hxx>
#include <rtl/ustring>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>

#include <ucbhelper/content.hxx>
#include <osl/module.h>
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif

#include <stdio.h>
#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif
#include <osl/diagnose.h>
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#include <time.h>


using namespace comphelper;
using namespace cppu;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;

//using namespace com::sun::star;
using namespace connectivity;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace com::sun::star::registry;

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
                                makeAny(OUString::createFromAscii("")), // Catalog
                                OUString::createFromAscii("%"),          // Schema
                                OUString::createFromAscii("%"),          // TabName
                                OUString::createFromAscii("%")
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
                    makeAny(OUString::createFromAscii("")), // Catalog
                    OUString::createFromAscii("%"),          // Schema
                    OUString::createFromAscii("%"),          // TabName
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
        OUString sqlPrefix= OUString::createFromAscii("SELECT  \"First Name\", \"Display Name\", \"E-mail\" FROM ");
        try
        {
            sal_Int32 times=0;
            Reference< XDatabaseMetaData > xDmd = pConnection->getMetaData();
            if ( xDmd.is() )
            {
                OSL_TRACE("getTables() : START\n");
                OUString qut      = xDmd->getIdentifierQuoteString();

                Reference<XResultSet> xRes = xDmd->getTables(
                    makeAny(OUString::createFromAscii("")), // Catalog
                    OUString::createFromAscii("%"),          // Schema
                    OUString::createFromAscii("%"),          // TabName
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
        url=OUString::createFromAscii("sdbc:address:ldap://");
        aValue.realloc(2);
        aValue[0].Name = ::rtl::OUString::createFromAscii("HostName");
        aValue[0].Value <<= rtl::OUString::createFromAscii("sun-ds");
        aValue[1].Name = ::rtl::OUString::createFromAscii("BaseDN");
        aValue[1].Value <<= rtl::OUString::createFromAscii("dc=sun,dc=com");
        break;
    case testMozilla:
        url=OUString::createFromAscii("sdbc:address:mozilla://");
        break;
    case testOp:
        url=OUString::createFromAscii("sdbc:address:outlook://");
        break;
    case testOe:
        url=OUString::createFromAscii("sdbc:address:outlookexp://");
        break;
    default:
        url=OUString::createFromAscii("sdbc:address:mozilla://");
        break;
    }
    pConnection =
        pDriver->connect(url,aValue);
   return pConnection;
 }

int autoTest(Reference<XResultSet> &xRes)
{
    sal_Int32 nRows = 0;
    printColumns(xRes);
    if(xRes.is())
    {
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
           OUString::createFromAscii("com.sun.star.comp.sdbc.MozabDriver")),
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

