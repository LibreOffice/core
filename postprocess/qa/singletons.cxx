/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Tries to get every interface-based singleton registered in the type description manager using its
// type name. See tdf#170930 for an example of a mistake that can make this fail.

#include <sal/config.h>

#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/XSingletonTypeDescription2.hpp>
#include <test/bootstrapfixture.hxx>

namespace
{
constexpr OUString TYPE_DESCRIPTION_MANAGER
    = u"/singletons/com.sun.star.reflection.theTypeDescriptionManager"_ustr;

class Test : public test::BootstrapFixture
{
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test()
{
    css::uno::Reference<css::reflection::XTypeDescriptionEnumerationAccess> xTDMgr;

    m_xContext->getValueByName(TYPE_DESCRIPTION_MANAGER) >>= xTDMgr;

    if (!xTDMgr.is())
        throw css::uno::RuntimeException("cannot get singleton " + TYPE_DESCRIPTION_MANAGER);

    css::uno::Sequence<css::uno::TypeClass> aTypes = { css::uno::TypeClass_SINGLETON };

    css::uno::Reference<css::reflection::XTypeDescriptionEnumeration> xTypeEnum
        = xTDMgr->createTypeDescriptionEnumeration(
            "", aTypes, css::reflection::TypeDescriptionSearchDepth_INFINITE);

    while (true)
    {
        css::uno::Reference<css::reflection::XTypeDescription> xType;

        try
        {
            xType = xTypeEnum->nextTypeDescription();
        }
        catch (css::container::NoSuchElementException&)
        {
            break;
        }

        css::uno::Reference<css::reflection::XSingletonTypeDescription2> xSingleton(
            xType, css::uno::UNO_QUERY);

        // Skip singletons that are not interface-based
        if (!xSingleton.is() || !xSingleton->isInterfaceBased())
            continue;

        OUString sName = "/singletons/" + xSingleton->getName();

        css::uno::Reference<css::uno::XInterface> xImpl;

        m_xContext->getValueByName(sName) >>= xImpl;

        if (!xImpl.is())
            throw css::uno::RuntimeException("cannot get singleton " + sName);

        css::uno::Reference<css::reflection::XTypeDescription> xInterface
            = xSingleton->getInterface();

        css::uno::Type interfaceType(xInterface->getTypeClass(), xInterface->getName());

        // Make sure that the returned object supports the interface of the singleton
        if (!xImpl->queryInterface(interfaceType).hasValue())
        {
            throw css::uno::RuntimeException("Singleton " + xSingleton->getName()
                                             + " doesn’t support the " + xInterface->getName()
                                             + " interface");
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
