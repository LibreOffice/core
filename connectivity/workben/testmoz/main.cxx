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
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/process.h>

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>

#include <osl/module.h>

#include <stdio.h>

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
using ::rtl::OUStringBuffer;
using ::rtl::OUStringToOString;

#define OUtoCStr( x ) (OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US ).getStr())
#define PRINTSTR(x) printf("%s",x);

int autoTest(Reference<XResultSet> &xRes);

void printColumns( Reference<XResultSet> &xRes )
{
    if(xRes.is()) {
        char* aPat = " %-22s ";
        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
        printf( "ColumnCount = %d\n", xMeta->getColumnCount());
        for(sal_Int32 i=1;i<=xMeta->getColumnCount();++i)
        {
            const char *str = OUtoCStr(xMeta->getColumnName(i));
            printf( aPat, str );
        }
        printf("\n");
        printf("------------------------------------------------------------------------------------------\n");
    } else {
        printf(": FAILED to get a ResultSet \n");
    }
}
void printXResultSet( Reference<XResultSet> &xRes )
{
    if(xRes.is()) {
        char* aPat_Short = " %-12s ";
        Reference<XRow> xRow(xRes,UNO_QUERY);
        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
        for(sal_Int32 j=1;j<=xMeta->getColumnCount();++j)
        {
            try {
                const char *str = OUtoCStr(xRow->getString(j));
                printf( aPat_Short, str );
            } catch (...) {
                  printf(" Ex ");
            }
        }
        printf("\n");
    } else {
        printf(": FAILED to get a ResultSet \n");
    }
}

void printXResultSets( Reference<XResultSet> &xRes )
{
    if(xRes.is()) {
        printColumns(xRes);
        sal_Int32 nRows = 0;
        while( xRes.is() && xRes->next())
        {
            printXResultSet(xRes);
            nRows++;
        }
        printf( "%d Row(s)\n", nRows);
    } else {
        printf(": FAILED to get a ResultSet \n");
    }
}

static const char * const components[] =
{
    SAL_MODULENAME( "ucb1" )    // KSO, ABI
    , SAL_MODULENAME( "ucpfile1" )
    , SAL_MODULENAME( "cfgmgr2" )
    , "sax.uno" SAL_DLLEXTENSION
    , "stocservices.uno" SAL_DLLEXTENSION
    , SAL_MODULENAME( "fileacc" )
    , SAL_MODULENAME( "mcnttype" )          //Clipboard   Ask Oliver Braun
    , "i18npool.uno" SAL_DLLEXTENSION
        // Reading of files in specific encodings like UTF-8 using
        // createUnoService( "com.sun.star.io.TextInputStream" ) and such
    , "textinstream.uno" SAL_DLLEXTENSION
    , "textoutstream.uno" SAL_DLLEXTENSION
    , "introspection.uno" SAL_DLLEXTENSION
    , "corereflection.uno" SAL_DLLEXTENSION
        // RemoteUno
    , "connector.uno" SAL_DLLEXTENSION
    , "bridgefac.uno" SAL_DLLEXTENSION
    , "remotebridge.uno" SAL_DLLEXTENSION
    , "dbtools2" SAL_DLLEXTENSION
    , "mozab2" SAL_DLLEXTENSION
    , "mozabdrv2" SAL_DLLEXTENSION
    , "sdbc2" SAL_DLLEXTENSION
    , "dbpool2" SAL_DLLEXTENSION
#ifdef SAL_UNX
    , SVLIBRARY( "dtransX11" )        // OBR
#endif
#ifdef SAL_W32
    , SAL_MODULENAME( "sysdtrans" )
    , SAL_MODULENAME( "ftransl" )
    , SAL_MODULENAME( "dnd" )
#endif
    , 0
};

Reference< XMultiServiceFactory > InitializeFac( void )
{
    OUString path;
    if( osl_Process_E_None != osl_getExecutableFile( (rtl_uString**)&path ) )
    {
        printf("Exit.\n");
    exit( 1 );
    }
    OSL_ASSERT( path.lastIndexOf( '/' ) >= 0 );


    ::rtl::OUStringBuffer bufServices( path.copy( 0, path.lastIndexOf( '/' )+1 ) );
    bufServices.appendAscii("services.rdb");
    OUString services = bufServices.makeStringAndClear();

    ::rtl::OUStringBuffer bufTypes( path.copy( 0, path.lastIndexOf( '/' )+1 ) );
    bufTypes.appendAscii("types.rdb");
    OUString types = bufTypes.makeStringAndClear();

    printf("Create Registry.\n");

    Reference< XMultiServiceFactory > xSMgr;
    try
    {
        xSMgr = createRegistryServiceFactory( types, services, sal_True );
    }
    catch( com::sun::star::uno::Exception & )
    {
        try
        {
            {
                Reference< XMultiServiceFactory > interimSmgr =
                    createRegistryServiceFactory( types, sal_True );
                Reference< XImplementationRegistration > xIR(
                    interimSmgr->createInstance(
                        OUString::(
                            "com.sun.star.registry.ImplementationRegistration" ) ), UNO_QUERY );

                Reference< XSimpleRegistry > xReg(
                    interimSmgr->createInstance(
                        OUString::(
                            "com.sun.star.registry.SimpleRegistry" ) ), UNO_QUERY );
                if ( xReg.is() )
                {
                    xReg->open(services, sal_False, sal_True);
                    if ( xReg->isValid() )
                    {
                        OUString loader( "com.sun.star.loader.SharedLibrary" );
                        for( sal_Int32 i = 0; components[i] ; i ++ )
                        {
                            printf("Registering %s ... ", components[i]);
                            xIR->registerImplementation(
                                loader, OUString::createFromAscii(components[i]),xReg);
                            printf("done\n");
                        }
                        xReg->close();
                    } else
                    {
                        printf("Cannot open Registry. Terminating Program\n");
                        exit (1);
                    }
                }

                Reference< XComponent > xComp( interimSmgr, UNO_QUERY );
                if( xComp.is() )
                    xComp->dispose();
            }

            // now try it again readonly
            printf("Opening Registry readonly\n");
            xSMgr = createRegistryServiceFactory( types, services, sal_True );
        }
        catch( com::sun::star::uno::Exception & exc )
        {
            fprintf( stderr, "Couldn't bootstrap uno servicemanager for reason : %s\n" ,
                     OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }

    printf("set global factory.\n");

    //////////////////////////////////////////////////////////////////////
    // set global factory
    setProcessServiceFactory( xSMgr );


//  Create unconfigured Ucb:
    Reference< XUniversalContentBroker > xUcb
        ( xSMgr->createInstance( OUString("com.sun.star.ucb.UniversalContentBroker") ), UNO_QUERY_THROW );

    Reference< XContentProvider > xFileProvider
        ( xSMgr->createInstance( OUString("com.sun.star.ucb.FileContentProvider") ), UNO_QUERY_THROW );
    xUcb->registerContentProvider( xFileProvider, OUString("file"), sal_True );

    return xSMgr;
}

int TestMetaData(Reference< ::com::sun::star::sdbc::XConnection> &pConnection)
{
               // Test some metadata
                Reference< XDatabaseMetaData > xDmd = pConnection->getMetaData();
                if ( xDmd.is() ) {
                    printf(": got DatabaseMetaData \n");

                    OUString sQuoteStr = xDmd->getIdentifierQuoteString();
                    printf( "Quote String : '%s'\n", OUtoCStr( sQuoteStr ) );

                    OUString sSQLCmds = xDmd->getSQLKeywords();
                    printf( "SQL Commands : '%s'\n", OUtoCStr( sSQLCmds ) );

                    printf("Testing getColumns() : START\n");
                    {
                        Reference<XResultSet> xRes = xDmd->getColumns(
                                makeAny(OUString("")), // Catalog
                                OUString("%"),          // Schema
                                OUString("%"),          // TabName
                                OUString("%")
                                );
                        printXResultSets( xRes );
                    }
                    printf("Testing getColumns() : END\n");

                    printf("Testing  getTypeInfo() : START\n");
                    {
                        Reference<XResultSet> xRes = xDmd-> getTypeInfo();
                        printXResultSets( xRes );
                    }
                    printf("Testing  getTypeInfo() : END\n");

                printf("Testing getTables() : START\n");
                    {
                        Reference<XResultSet> xRes = xDmd->getTables(
                                makeAny(OUString("")), // Catalog
                                OUString("%"),          // Schema
                                OUString("%"),          // TabName
                                Sequence<rtl::OUString>()
                                );
                        printXResultSets( xRes );
                    }
                    printf("Testing getTables() : END\n");

                } else {
                    printf(": FAILED to get DatabaseMetaData \n");
                }

    return 0;
}
int TestBookMark(Reference<XResultSet> &xRes);
int TestRowUpdate(Reference<XResultSet> &xRes);

Reference<XResultSet> TestQuery(Reference< ::com::sun::star::sdbc::XConnection> &pConnection,sal_Int32 choice)
{
     // Try a query
     printf("Testing createStatement() & executeQuery() : START\n");
     Reference<XStatement> xStmt = pConnection->createStatement();
     Reference<XResultSet> xRes;
     if(xStmt.is())
     {
          printf(": got statement\n");
          printf(":   excuteQuery() : START \n");
//          SELECT "First Name", "Display Name", "E-mail" FROM "addr" "addr"
          char sql[256]="SELECT  \"First Name\", \"Display Name\", \"E-mail\" FROM \"addr\"";
          if (choice!=-1)
          {
              printf("Please input a query,end by \";\" and less then 256 char plz:\n");
              char ch=0;
              int index=0;
              while (ch != ';')
              {
                  scanf("%c",&ch);
                  sql[index++] = ch;
              }
              sql[index]= 0;
              printf("SQL:%s\n",sql);
          }
          try
          {
                  sal_Int32 times=0;
                  for (times = 0;times< 100;times ++)
                  {
     Reference<XResultSet> tmpRes =
                  xStmt->executeQuery(OUString::createFromAscii(sql));

                  autoTest( tmpRes );
                            Reference<XCloseable> clsRes(tmpRes,UNO_QUERY);
                            clsRes->close();
                  printf("Current Times:%d\n",times);
                }
                  TestBookMark(xRes);
                  TestRowUpdate(xRes);
          } catch ( Exception &e ) {
               printf( "Exception caught : %s\n", OUtoCStr( e.Message) );
          } catch (...) {
          printf( "Non-UNO Exception caught\n" );
           }
          printf(":   excuteQuery() : END \n");
     }
     else
     {
        printf(": FAILED to get statement\n");
     }
     printf("Testing createStatement() & executeQuery() : END\n");
     return xRes;
}
Reference< ::com::sun::star::sdbc::XConnection> TestConnected
        (Reference< ::com::sun::star::sdbc::XDriver> &pDriver,sal_Int32 choice)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  pConnection;
    printf("Begin Connect!\n");
    OUString url;
    Sequence<PropertyValue> aValue;

    switch (choice)
    {
        case -1:
        case 1: //mozilla
            url=OUString("sdbc:address:mozilla://");
            break;
        case 2:
            url=OUString("sdbc:address:ldap://");
            char hostname[40],basedn[40];
            scanf("%s %s",hostname,basedn);
            aValue.realloc(2);
            aValue[0].Name = ::rtl::OUString("HostName");
            aValue[0].Value <<= rtl::OUString::createFromAscii(hostname);
            aValue[1].Name = ::rtl::OUString("BaseDN");
            aValue[1].Value <<= rtl::OUString::createFromAscii(basedn);
            break;
        case 3:
        case 4:
            break;
        case 5:
            //Default LDAP AB
            url=OUString("sdbc:address:ldap://");
            aValue.realloc(2);
            aValue[0].Name = ::rtl::OUString("HostName");
            aValue[0].Value <<= rtl::OUString("sun-ds");
            aValue[1].Name = ::rtl::OUString("BaseDN");
            aValue[1].Value <<= rtl::OUString("dc=sun,dc=com");
            break;
        default:
            return pConnection;
            break;
    }
    pConnection =
        pDriver->connect(url,aValue);
   return pConnection;
 }

void menuBookmark()
{
    PRINTSTR("0 Exit\n")
    PRINTSTR("1 Show Max rowcount\n")
    PRINTSTR("2 Move First\n")
    PRINTSTR("3 Record bookmark 1\n")
    PRINTSTR("4 Record bookmark 2\n")
    PRINTSTR("5 Move to bookmark. Usage: 5 bookid\n")
    PRINTSTR("6 moveRelativeToBookmark, follow bookid rows plz\n")
    PRINTSTR("7 compareBookmark\n")
    PRINTSTR("8 print boomark hash. Usage: 8 bookid\n")
    PRINTSTR("9 print current row\n")
    PRINTSTR("10 Auto test\n")

    PRINTSTR("Please Input you choice:")
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
        printf( "%d Row(s)\n", nRows);
        sal_Int32 times;
        sal_Int32 pos;
        for(times = 1;times < 100; times ++)
        {
            pos= rand() % nRows+1;
            printf("pos:%d\n",pos);
            xRes->absolute(pos);
            printXResultSet(xRes);
        }
    } else {
        printf(": FAILED to get a ResultSet \n");
    }
    return 0;
}
int TestBookMark(Reference<XResultSet> &xRes)
{
    Reference<XResultSet> mRes(xRes);
    if (!mRes.is() || !mRes->first())
    {
        return sal_False;
    }

    Reference<XRow> mRow(xRes,UNO_QUERY);
    Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(mRes,UNO_QUERY)->getMetaData();
    printXResultSet(mRes);
    Reference< ::com::sun::star::sdbcx::XRowLocate> xRowLocate(xRes, UNO_QUERY);
    ::com::sun::star::uno::Any xBookMark[2];
    int index;
    if (xRowLocate.is())
    {
        int choice=-1;
        while (choice)
        {
            menuBookmark();

            scanf("%d",&choice);
            printf("Your choice is:%d\n",choice);
            switch (choice)
            {
                case 1:
                    printf("Rowcount:Current don't know\n");
                    break;
                case 2:
                    if (!mRes->isFirst()) mRes->first();
                    break;
                case 3:
                    xBookMark[0] = xRowLocate->getBookmark();
                    break;
                case 4:
                    xBookMark[1] = xRowLocate->getBookmark();
                    break;
                case 5:
                    scanf("%d",&index);
                    if (index == 1 || index == 2)
                        xRowLocate->moveToBookmark(xBookMark[index-1]);
                    break;
                case 6:
                    int rows;
                    scanf("%d %d",&index,&rows);
                    if (index == 1 || index == 2)
                        xRowLocate->moveRelativeToBookmark(xBookMark[index-1],rows);
                    break;
                case 7:
                    printf("compareBookmarks:%d\n",xRowLocate->compareBookmarks(xBookMark[0],xBookMark[1]));
                    break;
                case 8:
                    scanf("%d",&index);
                    printf("HashBookmark[%d]:%d\n",index,xRowLocate->hashBookmark(xBookMark[index-1]));
                    break;
                case 9:
                    printXResultSet(mRes);
                    break;
                case 10:
                    autoTest(mRes);
                    break;
                default:
                    break;
            }
        }
   }
   return 0;
}
void menuRowUpdate()
{
    PRINTSTR("0 Exit\n")
    PRINTSTR("1 Print Columns\n")
    PRINTSTR("2 Move to row. Usage:2 rowid\n")
    PRINTSTR("3 Print Row values\n")
    PRINTSTR("4 Change Column Value: 4 columnid newvalue\n")
    PRINTSTR("5 Commit changes\n")
    PRINTSTR("6 Delete Current Row\n")
    PRINTSTR("Please Input Your choice:")
}
int TestRowUpdate(Reference<XResultSet> &xRes)
{
    if (!xRes.is() || !xRes->first())
    {
        return sal_False;
    }
    printf("Test XRowUpdate START\n");
    Reference< ::com::sun::star::sdbc::XRowUpdate> xRowUpdate(xRes, UNO_QUERY);
    Reference< ::com::sun::star::sdbc::XResultSetUpdate> xResultSetUpdate(xRes, UNO_QUERY);
    int index;
    if (xRowUpdate.is())
    {
        int choice=-1;
        char newString[256];
        while (choice)
        {
            menuRowUpdate();

            scanf("%d",&choice);
            printf("Your choice is:%d\n",choice);
            switch (choice)
            {
                case 1:
                    printColumns(xRes);
                    break;
                case 2:
                    scanf("%d",&index);
                    xRes->absolute(index);
                    break;
                case 3:
                    printXResultSet(xRes);
                    break;
                case 4:
                    scanf("%d %s",&index,newString);
                    xRowUpdate->updateString(index,OUString::createFromAscii(newString));
                    printXResultSet(xRes);
                    break;
                case 5:
                    if (xResultSetUpdate.is())
                    {
                        xResultSetUpdate->updateRow();
                        printXResultSet(xRes);
                    }
                    else
                        printf("Can't update!\n");
                    break;
                case 6:
                    if (xResultSetUpdate.is())
                    {
                        xResultSetUpdate->deleteRow();
                        printXResultSet(xRes);
                    }
                    else
                        printf("Can't update!\n");
                    break;

                default:
                    break;
            }
        }
    }
   printf("Test XRowUpdate END\n");
   return 0;
}

void menuMain()
{
    PRINTSTR("0 Exit\n")
    PRINTSTR("1 Open Mozilla Address Book\n")
    PRINTSTR("2 Open LDAP. Pleae follow hostname and base dn.\n")
    PRINTSTR("3 Open Outlook Address Book\n")
    PRINTSTR("4 Open OE Address Book\n")
    PRINTSTR("Please Input your choice:")
}
#if (defined UNX)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif

{
    Reference< XMultiServiceFactory > xMgr = InitializeFac();
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  m_xConnection;
    try
    {
    Reference< ::com::sun::star::sdbc::XDriver>
    m_xDriver(xMgr->createInstance(
           OUString("com.sun.star.comp.sdbc.MozabDriver")),
             UNO_QUERY);
        if(m_xDriver.is())
    {
        int choice=-1;
        if (argc > 1 )
        {
            menuMain();
               scanf("%d",&choice);
        }

        while (choice)
        {

           m_xConnection = TestConnected(m_xDriver,choice);
           if(m_xConnection.is())
           {
            if (!TestMetaData(m_xConnection))
            {
                Reference<XResultSet> xRes=TestQuery(m_xConnection,choice);
                if (xRes.is())
                {
                }
            }
           }else  printf("Can't connected!\n");

           if (argc > 1)
           {
                   menuMain();
                scanf("%d",&choice);
           }
           else
                   choice = 0;

        }
    } else {
            printf("No driver!\n");
        }
    }
    catch(...)
    {
     printf("Exception thrown!\n");
    }
    printf("Exiting...\n");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
