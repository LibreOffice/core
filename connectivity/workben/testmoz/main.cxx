/*************************************************************************
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 18:35:22 $
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

#define OUtoCStr( x ) (OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US ).getStr())
Reference< XContentProviderManager > globalUcb;
#define PRINTSTR(x) printf("%s",x);

int autoTest(Reference<XResultSet> &xRes);

void printColumns( Reference<XResultSet> &xRes )
{
    if(xRes.is()) {
        char* aPat = " %-22s ";
        char* aPat_Short = " %-12s ";
        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
        printf( "ColumnCount = %d\n", xMeta->getColumnCount());
        for(sal_Int32 i=1;i<=xMeta->getColumnCount();++i)
        {
            // printf(aPat.getStr(), xMeta->getColumnName(i).getStr());
            const char *str = OUtoCStr(xMeta->getColumnName(i));
//           if ( i < 3 ) {
//               printf( aPat_Short, str );
//           } else {
                printf( aPat, str );
//           }
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
        char* aPat = " %-22s ";
        char* aPat_Short = " %-12s ";
        Reference<XRow> xRow(xRes,UNO_QUERY);
        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
        for(sal_Int32 j=1;j<=xMeta->getColumnCount();++j)
        {
            try {
                const char *str = OUtoCStr(xRow->getString(j));
//                   if ( j < 3 ) {
//                       printf( aPat_Short, str );
//                   } else {
                        printf( aPat_Short, str );
//                   }
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

//#define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
static const char * const components[] =
{
    SAL_MODULENAME( "ucb1" )    // KSO, ABI
    , SAL_MODULENAME( "ucpfile1" )
    , SAL_MODULENAME( "cfgmgr2" )
    , "sax.uno" SAL_DLLEXTENSION
    , "typeconverter.uno" SAL_DLLEXTENSION
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
                        OUString::createFromAscii(
                            "com.sun.star.registry.ImplementationRegistration" ) ), UNO_QUERY );

                Reference< XSimpleRegistry > xReg(
                    interimSmgr->createInstance(
                        OUString::createFromAscii(
                            "com.sun.star.registry.SimpleRegistry" ) ), UNO_QUERY );
                if ( xReg.is() )
                {
                    xReg->open(services, sal_False, sal_True);
                    if ( xReg->isValid() )
                    {
                        OUString loader( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.loader.SharedLibrary" ));
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

/*  // Create simple ConfigManager
    Sequence< Any > aConfArgs(3);
    aConfArgs[0] <<= PropertyValue( OUString::createFromAscii("servertype"), 0, makeAny( OUString::createFromAscii("local") ), ::com::sun::star::beans::PropertyState_DIRECT_VALUE );
    aConfArgs[1] <<= PropertyValue( OUString::createFromAscii("sourcepath"), 0, makeAny( OUString::createFromAscii("g:\\") ), ::com::sun::star::beans::PropertyState_DIRECT_VALUE );
    aConfArgs[2] <<= PropertyValue( OUString::createFromAscii("updatepath"), 0, makeAny( OUString::createFromAscii("g:\\") ), ::com::sun::star::beans::PropertyState_DIRECT_VALUE );

    Reference< XContentProvider > xConfProvider
        ( xSMgr->createInstanceWithArguments( OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" ), aConfArgs), UNO_QUERY );
*/


//  Create unconfigured Ucb:
/*  Sequence< Any > aArgs(1);
    aArgs[1] = makeAny ( xConfProvider );*/
    Sequence< Any > aArgs;
    ::ucb::ContentBroker::initialize( xSMgr, aArgs );
    Reference< XContentProviderManager > xUcb =
        ucb::ContentBroker::get()->getContentProviderManagerInterface();

    Reference< XContentProvider > xFileProvider
        ( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.ucb.FileContentProvider" ) ), UNO_QUERY );
    xUcb->registerContentProvider( xFileProvider, OUString::createFromAscii( "file" ), sal_True );


/*  Reference< XContentProvider > xPackageProvider
        ( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.ucb.PackageContentProvider" ) ), UNO_QUERY );
    xUcb->registerContentProvider( xPackageProvider, OUString::createFromAscii( "vnd.sun.star.pkg" ), sal_True );
    */
    globalUcb = xUcb;
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
                                makeAny(OUString::createFromAscii("")), // Catalog
                                OUString::createFromAscii("%"),          // Schema
                                OUString::createFromAscii("%"),          // TabName
                                OUString::createFromAscii("%")
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
                                makeAny(OUString::createFromAscii("")), // Catalog
                                OUString::createFromAscii("%"),          // Schema
                                OUString::createFromAscii("%"),          // TabName
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
                  //xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"addr\""));
                  xStmt->executeQuery(OUString::createFromAscii(sql));
                  // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"Personal Address Book\" WHERE ( PrimaryEmail IS NULL )"));
                  // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"Personal Address Book\" WHERE ( PrimaryEmail LIKE \"Darren\" )"));
                  // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"Personal Address Book\""));
                  // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"myldap\" WHERE ( PrimaryEmail LIKE \"%Darren%\" OR DisplayName LIKE \"%Darren%\" )"));

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
            url=OUString::createFromAscii("sdbc:address:mozilla://");
            break;
        case 2:
            url=OUString::createFromAscii("sdbc:address:ldap://");
            char hostname[40],basedn[40];
            scanf("%s %s",hostname,basedn);
            aValue.realloc(2);
            aValue[0].Name = ::rtl::OUString::createFromAscii("HostName");
            aValue[0].Value <<= rtl::OUString::createFromAscii(hostname);
            aValue[1].Name = ::rtl::OUString::createFromAscii("BaseDN");
            aValue[1].Value <<= rtl::OUString::createFromAscii(basedn);
            break;
        case 3:
        case 4:
            break;
        case 5:
            //Default LDAP AB
            url=OUString::createFromAscii("sdbc:address:ldap://");
            aValue.realloc(2);
            aValue[0].Name = ::rtl::OUString::createFromAscii("HostName");
            aValue[0].Value <<= rtl::OUString::createFromAscii("sun-ds");
            aValue[1].Name = ::rtl::OUString::createFromAscii("BaseDN");
            aValue[1].Value <<= rtl::OUString::createFromAscii("dc=sun,dc=com");
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
    sal_Int32 nRows = 0;
    printColumns(xRes);
    if(xRes.is()) {
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
#if (defined UNX) || (defined OS2)
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
           OUString::createFromAscii("com.sun.star.comp.sdbc.MozabDriver")),
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

