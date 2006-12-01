/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testhelper.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:20:07 $
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

#if !defined(OSL_DEBUG_LEVEL) || OSL_DEBUG_LEVEL == 0
# undef OSL_DEBUG_LEVEL
# define OSL_DEBUG_LEVEL 2
#endif

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include <sal/main.h>

#include <stdio.h>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <cppuhelper/component_context.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include "testhelper.hxx"

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

SAL_IMPLEMENT_MAIN()
{
    try
    {
        Reference< XMultiComponentFactory > xMgr( createRegistryServiceFactory(
                                                      OUString( RTL_CONSTASCII_USTRINGPARAM("cpputest.rdb") ) ), UNO_QUERY );
        Reference< XComponentContext > xInitialContext;
        OSL_VERIFY( Reference< beans::XPropertySet >( xMgr, UNO_QUERY )->getPropertyValue(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xInitialContext );

        ContextEntry_Init aEntry;
        aEntry.bLateInitService = false;
        aEntry.name = OUString( RTL_CONSTASCII_USTRINGPARAM("bla, bla") );
        aEntry.value = makeAny( (sal_Int32)5 );
        Reference< XComponentContext > xContext( createComponentContext( &aEntry, 1, xInitialContext ) );
        OSL_ASSERT( xContext->getServiceManager() != xMgr ); // must be wrapped one
        OSL_ASSERT(
            Reference< beans::XPropertySet >(
                xContext->getServiceManager(), UNO_QUERY )->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) != xInitialContext );

        Reference< XMultiServiceFactory > x( xMgr, UNO_QUERY );
        test_ImplHelper( x );
        testPropertyTypeHelper();
        testidlclass( x );
         test_PropertySetHelper();
        test_interfacecontainer();

        OSL_VERIFY( xContext->getValueByName(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("bla, bla") ) ) == (sal_Int32)5 );
        OSL_VERIFY( ! xInitialContext->getValueByName(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("bla, bla") ) ).hasValue() );
        Reference< XComponent >( xInitialContext, UNO_QUERY )->dispose();
        xMgr.clear();
        xContext.clear();
        xInitialContext.clear();
    }
    catch (Exception & exc)
    {
        OString cstr_msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_ENSURE( ! "exception occured: ", cstr_msg.getStr() );
    }

    printf( "Test finished\n" );
    return 0;
}
