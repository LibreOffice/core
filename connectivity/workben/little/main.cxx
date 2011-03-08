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
#include "connectivity/sqlnode.hxx"

using namespace connectivity;
using namespace com::sun::star::sdbc;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace cppu;

using ::rtl::OUString;


#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif

{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver>      m_xDriver;

    try{
        Reference< ::com::sun::star::lang::XMultiServiceFactory > xFac =
                createRegistryServiceFactory(OUString(RTL_CONSTASCII_USTRINGPARAM("g:\\office50\\program\\applicat.rdb")),OUString());
        if(!xFac.is())
            return;

        m_xDriver = Reference<XDriver>(xFac->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.driver.dbase.Driver"))),UNO_QUERY);
        if(m_xDriver.is())
        {

            Sequence<PropertyValue> aValue;
    //      aValue.getArray()[0] = PropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("user")),0,makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM("TEST1"))),PropertyState_DIRECT_VALUE);
    //      aValue.getArray()[1] = PropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("password")),0,makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM("TEST1"))),PropertyState_DIRECT_VALUE);
    //
            m_xConnection = m_xDriver->connect(OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:dbase:g:\\")),aValue);
            if(m_xConnection.is())
            {
                Reference<XStatement> xStmt = m_xConnection->createStatement();
                if(xStmt.is())
                {
                    Reference<XResultSet> xRes = xStmt->executeQuery(OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * FROM Tele")));
                    if(xRes.is())
                    {
                        ::rtl::OUString aPat( RTL_CONSTASCII_USTRINGPARAM( "%s\t" ));
                        Reference<XRow> xRow(xRes,UNO_QUERY);
                        Reference<XResultSetMetaData> xMeta = Reference<XResultSetMetaDataSupplier>(xRes,UNO_QUERY)->getMetaData();
                        for(sal_Int32 i=1;i<xMeta->getColumnCount();++i)
                        {
                            wprintf(aPat.getStr(), xMeta->getColumnName(i).getStr());
                        }
                        printf("----------------------------------------------------------------------\n");
                        while(xRes->next())
                        {
                            for(sal_Int32 j=1;j<xMeta->getColumnCount();++j)
                                wprintf(aPat.getStr(), xRow->getString(j).getStr());
                            printf("\n");
                        }
                    }
                }
            }

        }
    }
    catch(...)
    {
        printf("Exception thrown!\n");

    }
    sal_Int32 d;
    scanf("%d",&d);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
