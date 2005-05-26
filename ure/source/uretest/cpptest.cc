/*************************************************************************
 *
 *  $RCSfile: cpptest.cc,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2005-05-26 09:37:20 $
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
 *  Copyright: 2005 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "sal/config.h"

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include "test/types/TestException.hpp"
#include "test/types/XTest.hpp"

namespace css = com::sun::star;

namespace {

class Service: public cppu::WeakImplHelper1< test::types::XTest > {
public:
    Service() {}

    virtual void SAL_CALL throwException()
        throw (test::types::TestException, css::uno::RuntimeException)
    {
        throw test::types::TestException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test")),
            static_cast< cppu::OWeakObject * >(this));
    }

private:
    Service(Service &); // not defined
    void operator =(Service &); // not defined

    virtual ~Service() {}
};

namespace CppTest {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< cppu::OWeakObject * >(new Service);
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("std::bad_alloc")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("test.cpp.cpptest.Component"));
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("test.types.CppTest"));
    return css::uno::Sequence< rtl::OUString >(&s, 1);
}

}

cppu::ImplementationEntry entries[] = {
    { CppTest::create, CppTest::getImplementationName,
      CppTest::getSupportedServiceNames, cppu::createSingleComponentFactory, 0,
      0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}

extern "C" void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
