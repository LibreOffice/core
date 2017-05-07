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


#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/reflection/XIndirectTypeDescription.hpp>
#include <com/sun/star/reflection/XInterfaceMethodTypeDescription.hpp>
#include <com/sun/star/reflection/XPublished.hpp>
#include <com/sun/star/reflection/XStructTypeDescription.hpp>
#include <com/sun/star/reflection/XTypeDescription.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/textenc.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/environment.h>

#include /*MSVC trouble: <cstdlib>*/ <stdlib.h>
#include <iostream>
#include <ostream>

namespace {

class Service: public cppu::WeakImplHelper< css::lang::XMain > {
public:
    virtual sal_Int32 SAL_CALL
    run(css::uno::Sequence< OUString > const & arguments)
        throw (css::uno::RuntimeException);

    static OUString getImplementationName();

    static css::uno::Sequence< OUString > getSupportedServiceNames();

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

std::ostream & operator <<(std::ostream & out, OUString const & value) {
    return out << OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr();
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

sal_Int32 Service::run(css::uno::Sequence< OUString > const & arguments)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > factory(
        m_context->getServiceManager());
    assertTrue(factory.is());
    css::uno::Reference< css::container::XHierarchicalNameAccess > manager(
        m_context->getValueByName(
            OUString(
                    "/singletons/"
                    "com.sun.star.reflection.theTypeDescriptionManager")),
        css::uno::UNO_QUERY_THROW);


    // test: add cmd line rdbs to manager


    OSL_ASSERT( arguments.getLength() > 0 );
    css::uno::Reference<css::container::XSet> xSet(
        manager, css::uno::UNO_QUERY_THROW );
    for ( sal_Int32 argPos = 0; argPos < arguments.getLength(); ++argPos ) {
        OUString url;
        OSL_VERIFY( osl_File_E_None == osl_getFileURLFromSystemPath(
                        arguments[argPos].pData, &url.pData ) );
        bool supposedToBeCompatible = ! url.endsWithIgnoreAsciiCase("_incomp.rdb");

        css::uno::Reference<css::registry::XSimpleRegistry> xReg(
            m_context->getServiceManager()->createInstanceWithContext(
                "com.sun.star.registry.SimpleRegistry",
                m_context ), css::uno::UNO_QUERY_THROW );
        xReg->open( url, true /* read-only */, false /* ! create */ );
        css::uno::Any arg( css::uno::makeAny(xReg) );
        css::uno::Reference<css::container::XHierarchicalNameAccess> xTDprov(
            m_context->getServiceManager()->
            createInstanceWithArgumentsAndContext(
                "com.sun.star.comp.stoc.RegistryTypeDescriptionProvider",
                css::uno::Sequence<css::uno::Any>( &arg, 1 ), m_context ),
            css::uno::UNO_QUERY_THROW );
        try {
            xSet->insert( css::uno::makeAny(xTDprov) );
            if (! supposedToBeCompatible)
                std::cerr << "current rdb file: " <<
                    OUStringToOString(
                        url, osl_getThreadTextEncoding()).getStr() << std::endl;
            assertTrue(supposedToBeCompatible);
        } catch (css::lang::IllegalArgumentException &) {
            if (supposedToBeCompatible)
                throw;
            assertFalse(supposedToBeCompatible);
        }
    }

    /

    css::uno::Reference< css::reflection::XIndirectTypeDescription > sequence(
        manager->getByHierarchicalName(
            OUString("[][]boolean")),
        css::uno::UNO_QUERY_THROW);
    assertEqual(css::uno::TypeClass_SEQUENCE, sequence->getTypeClass());
    assertEqual(
        OUString("[][]boolean"),
        sequence->getName());
    assertEqual(
        OUString("[]boolean"),
        sequence->getReferencedType()->getName());

    css::uno::Reference< css::reflection::XStructTypeDescription > structure(
        manager->getByHierarchicalName(
            OUString( "test.tdmanager.Struct<boolean,test.tdmanager.Struct<"
                    "any,com.sun.star.uno.XInterface>>")),
        css::uno::UNO_QUERY_THROW);
    assertEqual(css::uno::TypeClass_STRUCT, structure->getTypeClass());
    assertEqual(
        OUString( "test.tdmanager.Struct<boolean,test.tdmanager.Struct<"
                "any,com.sun.star.uno.XInterface>>"),
        structure->getName());
    assertEqual< bool >(false, structure->getBaseType().is());
    assertEqual< sal_Int32 >(1, structure->getMemberTypes().getLength());
    assertEqual(
        OUString( "test.tdmanager.Struct<any,com.sun.star.uno.XInterface>"),
        structure->getMemberTypes()[0]->getName());
    assertEqual< sal_Int32 >(1, structure->getMemberNames().getLength());
    assertEqual(
        OUString("s"),
        structure->getMemberNames()[0]);
    assertEqual< sal_Int32 >(0, structure->getTypeParameters().getLength());
    assertEqual< sal_Int32 >(2, structure->getTypeArguments().getLength());
    assertEqual(
        OUString("boolean"),
        structure->getTypeArguments()[0]->getName());
    assertEqual(
        OUString( "test.tdmanager.Struct<any,com.sun.star.uno.XInterface>"),
        structure->getTypeArguments()[1]->getName());

    css::uno::Reference< css::reflection::XInterfaceMethodTypeDescription >
        method(
            manager->getByHierarchicalName(
                OUString( "com.sun.star.uno.XComponentContext::getValueByName")),
            css::uno::UNO_QUERY_THROW);
    assertEqual(css::uno::TypeClass_INTERFACE_METHOD, method->getTypeClass());
    assertEqual(
        OUString( "com.sun.star.uno.XComponentContext::getValueByName"),
        method->getName());
    assertEqual(
        OUString("getValueByName"),
        method->getMemberName());
    assertEqual< sal_Int32 >(3, method->getPosition());
    assertEqual(
        OUString("any"),
        method->getReturnType()->getName());
    assertEqual< bool >(false, method->isOneway());
    assertEqual< sal_Int32 >(1, method->getParameters().getLength());
    assertEqual(
        OUString("Name"),
        method->getParameters()[0]->getName());
    assertEqual(
        OUString("string"),
        method->getParameters()[0]->getType()->getName());
    assertEqual< bool >(true, method->getParameters()[0]->isIn());
    assertEqual< bool >(false, method->getParameters()[0]->isOut());
    assertEqual< sal_Int32 >(0, method->getParameters()[0]->getPosition());
    assertEqual< sal_Int32 >(0, method->getExceptions().getLength());

    assertFalse(
        css::uno::Reference< css::reflection::XPublished >(
            css::uno::Reference< css::reflection::XTypeDescription >(
                manager->getByHierarchicalName(
                    OUString("[]boolean")),
                css::uno::UNO_QUERY_THROW),
            css::uno::UNO_QUERY).is());
    assertFalse(
        css::uno::Reference< css::reflection::XPublished >(
            css::uno::Reference< css::reflection::XTypeDescription >(
                manager->getByHierarchicalName(
                    OUString( "com.sun.star.beans.XIntroTest::ObjectName")),
                css::uno::UNO_QUERY_THROW),
            css::uno::UNO_QUERY).is());
    assertFalse(
        css::uno::Reference< css::reflection::XPublished >(
            css::uno::Reference< css::reflection::XTypeDescription >(
                manager->getByHierarchicalName(
                    OUString( "com.sun.star.beans.XIntroTest::writeln")),
                css::uno::UNO_QUERY_THROW),
            css::uno::UNO_QUERY).is());
    //TODO: check that the reflection of a property of an accumulation-based
    // service does not support XPublished

    return 0;
}

OUString Service::getImplementationName() {
    return OUString("test.tdmanager.impl");
}

css::uno::Sequence< OUString > Service::getSupportedServiceNames() {
    return css::uno::Sequence< OUString >();
}

css::uno::Reference< css::uno::XInterface > Service::createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    throw (css::uno::Exception)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(char const * implName,
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

bool writeInfo(void * registryKey, OUString const & implementationName,
               css::uno::Sequence< OUString > const & serviceNames) {
    OUString keyName = "/" + implementationName + "/UNO/SERVICES";
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
