/*************************************************************************
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dkenny $ $Date: 2001-05-31 07:22:02 $
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
#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#endif

#include "connectivity/sqlnode.hxx"
#include <ucbhelper/contentbroker.hxx>

#include <rtl/ustring>

#include <stdio.h>

using namespace connectivity;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdb;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace rtl;
using namespace cppu;

#define OUtoCStr( x ) (OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US ).getStr())

void printXResultSet( Reference<XResultSet> &xRes )
{
    if(xRes.is()) {
        char* aPat = " %-22s ";
        char* aPat_Short = " %-12s ";
        Reference<XRow> xRow(xRes,UNO_QUERY);
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
        sal_Int32 nRows = 0;
        while( xRes.is() && xRes->next())
        {
            for(sal_Int32 j=1;j<=xMeta->getColumnCount();++j)
                try {
                    const char *str = OUtoCStr(xRow->getString(j));
//                   if ( j < 3 ) {
//                       printf( aPat_Short, str );
//                   } else {
                        printf( aPat, str );
//                   }
                } catch (...) {
                  printf(" Ex ");
                }
            nRows++;
            printf("\n");
        }
        printf( "%d Row(s)\n", nRows);
    } else {
        printf(": FAILED to get a ResultSet \n");
    }
}

#define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())

#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif

{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver>      m_xDriver;
    uno::Sequence< uno::Any > aArgs(2);

    try{
        Reference< ::com::sun::star::lang::XMultiServiceFactory > xFac =
                createRegistryServiceFactory(OUString::createFromAscii("applicat.rdb"),OUString());
        if(!xFac.is()) {
            printf("No Factory\n");
            return;
        }
        comphelper::setProcessServiceFactory( xFac );

        aArgs[0] <<= OUString::createFromAscii( "Local" );
        aArgs[1] <<= OUString::createFromAscii( "Office" );

        ::ucb::ContentBroker::initialize( xFac, aArgs );

        m_xDriver =
        Reference<XDriver>(xFac->createInstance(OUString::createFromAscii("com.sun.star.sdbc.driver.mozaddressbook.Driver")),UNO_QUERY);
        if(m_xDriver.is())
        {

            Sequence<PropertyValue> aValue;


            // LDAP AB
            aValue.realloc(2);
            aValue[0].Name = ::rtl::OUString::createFromAscii("HostName");
            aValue[0].Value <<= rtl::OUString::createFromAscii("sunldap1.sun.com");
            aValue[1].Name = ::rtl::OUString::createFromAscii("BaseDN");
            aValue[1].Value <<= rtl::OUString::createFromAscii("dc=sun,dc=com");

            m_xConnection =
            m_xDriver->connect(OUString::createFromAscii("sdbc:address:ldap://"),aValue);

            // Mozilla ABs
            // m_xConnection =
            // m_xDriver->connect(OUString::createFromAscii("sdbc:address:mozilla://"),aValue);

            if(m_xConnection.is())
            {
                // Test some metadata
                printf("Testing getTables() : START\n");
                Reference< XDatabaseMetaData > xDmd = m_xConnection->getMetaData();
                if ( xDmd.is() ) {
                    printf(": got DatabaseMetaData \n");

                    OUString sQuoteStr = xDmd->getIdentifierQuoteString();
                    printf( "Quote String : '%s'\n", OUtoCStr( sQuoteStr ) );

                    OUString sSQLCmds = xDmd->getSQLKeywords();
                    printf( "SQL Commands : '%s'\n", OUtoCStr( sSQLCmds ) );

                    {
                        Reference<XResultSet> xRes = xDmd->getTables(
                                makeAny(OUString::createFromAscii("")), // Catalog
                                OUString::createFromAscii(""),          // Schema
                                OUString::createFromAscii(""),          // TabName
                                uno::Sequence<rtl::OUString>()
                                );
                        printXResultSet( xRes );
                    }

                    {
                        Reference<XResultSet> xRes = xDmd->getColumns(
                                makeAny(OUString::createFromAscii("")), // Catalog
                                OUString::createFromAscii(""),          // Schema
                                OUString::createFromAscii(""),          // TabName
                                OUString::createFromAscii("")
                                );
                        printXResultSet( xRes );
                    }
                } else {
                    printf(": FAILED to get DatabaseMetaData \n");
                }
                printf("Testing getTables() : END\n");

                // Try a query
                printf("Testing createStatement() & executeQuery() : START\n");
                Reference<XStatement> xStmt = m_xConnection->createStatement();
                if(xStmt.is())
                {
                    printf(": got statement\n");
                    printf(":   excuteQuery() : START \n");
                    try {
                        Reference<XResultSet> xRes =
                        xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"AddressBook\" WHERE ( PrimaryEmail LIKE \"Darren\" )"));
                        // xStmt->executeQuery(OUString::createFromAscii("SELECT DisplayName, PrimaryEmail FROM \"Personal Address Book\""));
                        // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"Personal Address Book\" WHERE ( PrimaryEmail IS NULL )"));
                        // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"Personal Address Book\" WHERE ( PrimaryEmail LIKE \"Darren\" )"));
                        // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"Personal Address Book\""));
                        // xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM \"myldap\" WHERE ( PrimaryEmail LIKE \"%Darren%\" OR DisplayName LIKE \"%Darren%\" )"));
                        printXResultSet( xRes );
                    } catch ( uno::Exception &e ) {
                        printf( "Exception caught : %s\n", OUtoCStr( e.Message) );
                    } catch (...) {
                        printf( "Non-UNO Exception caught\n" );
                    }
                    printf(":   excuteQuery() : END \n");
                } else {
                    printf(": FAILED to get statement\n");
                }
                printf("Testing createStatement() & executeQuery() : END\n");
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
}

