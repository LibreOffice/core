/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: main.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:50:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include "connectivity/sqlnode.hxx"

using namespace connectivity;
using namespace com::sun::star::sdbc;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace rtl;
using namespace cppu;


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
                createRegistryServiceFactory(OUString::createFromAscii("g:\\office50\\program\\applicat.rdb"),OUString());
        if(!xFac.is())
            return;

        m_xDriver = Reference<XDriver>(xFac->createInstance(OUString::createFromAscii("com.sun.star.sdbc.driver.dbase.Driver")),UNO_QUERY);
        if(m_xDriver.is())
        {

            Sequence<PropertyValue> aValue;
    //      aValue.getArray()[0] = PropertyValue( OUString::createFromAscii("user"),0,makeAny(OUString::createFromAscii("TEST1")),PropertyState_DIRECT_VALUE);
    //      aValue.getArray()[1] = PropertyValue( OUString::createFromAscii("password"),0,makeAny(OUString::createFromAscii("TEST1")),PropertyState_DIRECT_VALUE);
    //
            m_xConnection = m_xDriver->connect(OUString::createFromAscii("sdbc:dbase:g:\\"),aValue);
            if(m_xConnection.is())
            {
                Reference<XStatement> xStmt = m_xConnection->createStatement();
                if(xStmt.is())
                {
                    Reference<XResultSet> xRes = xStmt->executeQuery(OUString::createFromAscii("SELECT * FROM Tele"));
                    if(xRes.is())
                    {
                        ::rtl::OUString aPat = ::rtl::OUString::createFromAscii("%s\t");
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

