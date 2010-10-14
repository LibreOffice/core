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
#include "precompiled_stoc.hxx"

#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XSet.hpp"
#include "com/sun/star/lang/XMain.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/reflection/XIndirectTypeDescription.hpp"
#include "com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp"
#include "com/sun/star/reflection/XPublished.hpp"
#include "com/sun/star/reflection/XStructTypeDescription.hpp"
#include "com/sun/star/reflection/XTypeDescription.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/registry/XSimpleRegistry.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/file.h"
#include "osl/thread.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include /*MSVC trouble: <cstdlib>*/ <stdlib.h>
#include <iostream>
#include <ostream>

namespace css = com::sun::star;

namespace {

class Service: public cppu::WeakImplHelper1< css::lang::XMain > {
public:
    virtual sal_Int32 SAL_CALL
    run(css::uno::Sequence< rtl::OUString > const & arguments)
        throw (css::uno::RuntimeException);

    static rtl::OUString getImplementationName();

    static css::uno::Sequence< rtl::OUString > getSupportedServiceNames();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        css::uno::Reference< css::uno::XComponentContext > const & context)
        throw (css::uno::Exception);

private:
    explicit Service(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context)
    {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

}

namespace {

std::ostream & operator <<(std::ostream & out, rtl::OUString const & value) {
    return out << rtl::OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr();
}

void assertTrue(bool argument) {
    if (!argument) {
        std::cerr
            << "assertTrue(" << argument << ") failed" << std::endl;
        /*MSVC trouble: std::*/abort();
    }
}

void assertFalse(bool argument) {
    if (argument) {
        std::cerr
            << "assertFalse(" << argument << ") failed" << std::endl;
        /*MSVC trouble: std::*/abort();
    }
}

template< typename T > void assertEqual(T const & value, T const & argument) {
    if (argument != value) {
        std::cerr
            << "assertEqual(" << value << ", " << argument << ") failed"
            << std::endl;
        /*MSVC trouble: std::*/abort();
    }
}

}

sal_Int32 Service::run(css::uno::Sequence< rtl::OUString > const & arguments)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > factory(
        m_context->getServiceManager());
    assertTrue(factory.is());
    css::uno::Reference< css::container::XHierarchicalNameAccess > manager(
        m_context->getValueByName(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/"
                    "com.sun.star.reflection.theTypeDescriptionManager"))),
        css::uno::UNO_QUERY_THROW);

    ////////////////////////////////////////
    // test: add cmd line rdbs to manager
    ////////////////////////////////////////

    OSL_ASSERT( arguments.getLength() > 0 );
    css::uno::Reference<css::container::XSet> xSet(
        manager, css::uno::UNO_QUERY_THROW );
    for ( sal_Int32 argPos = 0; argPos < arguments.getLength(); ++argPos ) {
        rtl::OUString url;
        OSL_VERIFY( osl_File_E_None == osl_getFileURLFromSystemPath(
                        arguments[argPos].pData, &url.pData ) );
        bool supposedToBeCompatible = ! url.endsWithIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM("_incomp.rdb") );

        css::uno::Reference<css::registry::XSimpleRegistry> xReg(
            m_context->getServiceManager()->createInstanceWithContext(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                   "com.sun.star.registry.SimpleRegistry") ),
                m_context ), css::uno::UNO_QUERY_THROW );
        xReg->open( url, true /* read-only */, false /* ! create */ );
        css::uno::Any arg( css::uno::makeAny(xReg) );
        css::uno::Reference<css::container::XHierarchicalNameAccess> xTDprov(
            m_context->getServiceManager()->
            createInstanceWithArgumentsAndContext(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                   "com.sun.star.comp.stoc."
                                   "RegistryTypeDescriptionProvider") ),
                css::uno::Sequence<css::uno::Any>( &arg, 1 ), m_context ),
            css::uno::UNO_QUERY_THROW );
        try {
            xSet->insert( css::uno::makeAny(xTDprov) );
            if (! supposedToBeCompatible)
                std::cerr << "current rdb file: " <<
                    rtl::OUStringToOString(
                        url, osl_getThreadTextEncoding()).getStr() << std::endl;
            assertTrue(supposedToBeCompatible);
        } catch (css::lang::IllegalArgumentException &) {
            if (supposedToBeCompatible)
                throw;
            assertFalse(supposedToBeCompatible);
        }
    }

    ///////

    css::uno::Reference< css::reflection::XIndirectTypeDescription > sequence(
        manager->getByHierarchicalName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[][]boolean"))),
        css::uno::UNO_QUERY_THROW);
    assertEqual(css::uno::TypeClass_SEQUENCE, sequence->getTypeClass());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[][]boolean")),
        sequence->getName());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[]boolean")),
        sequence->getReferencedType()->getName());

    css::uno::Reference< css::reflection::XStructTypeDescription > structure(
        manager->getByHierarchicalName(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "test.tdmanager.Struct<boolean,test.tdmanager.Struct<"
                    "any,com.sun.star.uno.XInterface>>"))),
        css::uno::UNO_QUERY_THROW);
    assertEqual(css::uno::TypeClass_STRUCT, structure->getTypeClass());
    assertEqual(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "test.tdmanager.Struct<boolean,test.tdmanager.Struct<"
                "any,com.sun.star.uno.XInterface>>")),
        structure->getName());
    assertEqual< bool >(false, structure->getBaseType().is());
    assertEqual< sal_Int32 >(1, structure->getMemberTypes().getLength());
    assertEqual(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "test.tdmanager.Struct<any,com.sun.star.uno.XInterface>")),
        structure->getMemberTypes()[0]->getName());
    assertEqual< sal_Int32 >(1, structure->getMemberNames().getLength());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("s")),
        structure->getMemberNames()[0]);
    assertEqual< sal_Int32 >(0, structure->getTypeParameters().getLength());
    assertEqual< sal_Int32 >(2, structure->getTypeArguments().getLength());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("boolean")),
        structure->getTypeArguments()[0]->getName());
    assertEqual(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "test.tdmanager.Struct<any,com.sun.star.uno.XInterface>")),
        structure->getTypeArguments()[1]->getName());

    css::uno::Reference< css::reflection::XInterfaceMethodTypeDescription >
        method(
            manager->getByHierarchicalName(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.uno.XComponentContext::getValueByName"))),
            css::uno::UNO_QUERY_THROW);
    assertEqual(css::uno::TypeClass_INTERFACE_METHOD, method->getTypeClass());
    assertEqual(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.uno.XComponentContext::getValueByName")),
        method->getName());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getValueByName")),
        method->getMemberName());
    assertEqual< sal_Int32 >(3, method->getPosition());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any")),
        method->getReturnType()->getName());
    assertEqual< bool >(false, method->isOneway());
    assertEqual< sal_Int32 >(1, method->getParameters().getLength());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")),
        method->getParameters()[0]->getName());
    assertEqual(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("string")),
        method->getParameters()[0]->getType()->getName());
    assertEqual< bool >(true, method->getParameters()[0]->isIn());
    assertEqual< bool >(false, method->getParameters()[0]->isOut());
    assertEqual< sal_Int32 >(0, method->getParameters()[0]->getPosition());
    assertEqual< sal_Int32 >(0, method->getExceptions().getLength());

    assertFalse(
        css::uno::Reference< css::reflection::XPublished >(
            css::uno::Reference< css::reflection::XTypeDescription >(
                manager->getByHierarchicalName(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("[]boolean"))),
                css::uno::UNO_QUERY_THROW),
            css::uno::UNO_QUERY).is());
    assertFalse(
        css::uno::Reference< css::reflection::XPublished >(
            css::uno::Reference< css::reflection::XTypeDescription >(
                manager->getByHierarchicalName(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.beans.XIntroTest::ObjectName"))),
                css::uno::UNO_QUERY_THROW),
            css::uno::UNO_QUERY).is());
    assertFalse(
        css::uno::Reference< css::reflection::XPublished >(
            css::uno::Reference< css::reflection::XTypeDescription >(
                manager->getByHierarchicalName(
                    rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.beans.XIntroTest::writeln"))),
                css::uno::UNO_QUERY_THROW),
            css::uno::UNO_QUERY).is());
    //TODO: check that the reflection of a property of an accumulation-based
    // service does not support XPublished

    return 0;
}

rtl::OUString Service::getImplementationName() {
    return rtl::OUString::createFromAscii("test.tdmanager.impl");
}

css::uno::Sequence< rtl::OUString > Service::getSupportedServiceNames() {
    return css::uno::Sequence< rtl::OUString >();
}

css::uno::Reference< css::uno::XInterface > Service::createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    throw (css::uno::Exception)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    if (envTypeName != 0) {
        *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }
}

extern "C" void * SAL_CALL component_getFactory(char const * implName,
                                                void * serviceManager, void *) {
    void * p = 0;
    if (serviceManager != 0) {
        css::uno::Reference< css::lang::XSingleComponentFactory > f;
        if (Service::getImplementationName().equalsAscii(implName)) {
            f = cppu::createSingleComponentFactory(
                &Service::createInstance, Service::getImplementationName(),
                Service::getSupportedServiceNames());
        }
        if (f.is()) {
            f->acquire();
            p = f.get();
        }
    }
    return p;
}

namespace {

bool writeInfo(void * registryKey, rtl::OUString const & implementationName,
               css::uno::Sequence< rtl::OUString > const & serviceNames) {
    rtl::OUString keyName(rtl::OUString::createFromAscii("/"));
    keyName += implementationName;
    keyName += rtl::OUString::createFromAscii("/UNO/SERVICES");
    css::uno::Reference< css::registry::XRegistryKey > key;
    try {
        key = static_cast< css::registry::XRegistryKey * >(registryKey)->
            createKey(keyName);
    } catch (css::registry::InvalidRegistryException &) {}
    if (!key.is()) {
        return false;
    }
    bool success = true;
    for (sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        try {
            key->createKey(serviceNames[i]);
        } catch (css::registry::InvalidRegistryException &) {
            success = false;
            break;
        }
    }
    return success;
}

}

extern "C" sal_Bool SAL_CALL component_writeInfo(void *, void * registryKey) {
    return registryKey
        && writeInfo(registryKey, Service::getImplementationName(),
                     Service::getSupportedServiceNames());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
