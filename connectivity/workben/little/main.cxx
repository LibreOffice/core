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
#include <connectivity/sqliterator.hxx>
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
#include <connectivity/sqlnode.hxx>

using namespace connectivity;
using namespace com::sun::star::sdbc;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace cppu;



void main( int argc, char * argv[] )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>  m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver>      m_xDriver;

    try{
        Reference< ::com::sun::star::lang::XMultiServiceFactory > xFac =
                createRegistryServiceFactory(OUString("g:\\office50\\program\\applicat.rdb"),OUString());
        if(!xFac.is())
            return;

        m_xDriver.set(xFac->createInstance("com.sun.star.sdbc.driver.dbase.Driver"),UNO_QUERY);
        if(m_xDriver.is())
        {

            Sequence<PropertyValue> aValue;
    //      aValue.getArray()[0] = PropertyValue( OUString("user"),0,makeAny(OUString("TEST1")),PropertyState_DIRECT_VALUE);
    //      aValue.getArray()[1] = PropertyValue( OUString("password"),0,makeAny(OUString("TEST1")),PropertyState_DIRECT_VALUE);

            m_xConnection = m_xDriver->connect(OUString("sdbc:dbase:g:\\"),aValue);
            if(m_xConnection.is())
            {
                Reference<XStatement> xStmt = m_xConnection->createStatement();
                if(xStmt.is())
                {
                    Reference<XResultSet> xRes = xStmt->executeQuery(OUString("SELECT * FROM Tele"));
                    if(xRes.is())
                    {
                        OUString aPat( "%s\t" );
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
