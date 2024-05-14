/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::awt;
using namespace css::container;
using namespace css::lang;
using namespace css::script;
using namespace css::uno;

namespace
{
/// Test EventContainer class
class EventContainerTest : public test::BootstrapFixture
{
protected:
    Reference<XComponentContext> mxContext;

public:
    virtual void setUp() override;
};

void EventContainerTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
}

// Make sure that EventContainer keeps insertion order, and does not reorder its elements.
// Otherwise this would break macro signatures.
CPPUNIT_TEST_FIXTURE(EventContainerTest, testInsertOrder)
{
    Reference<XMultiComponentFactory> xFactory(mxContext->getServiceManager(), UNO_SET_THROW);
    Reference<XControlModel> xControlModel(
        xFactory->createInstanceWithContext(u"com.sun.star.awt.UnoControlDialogModel"_ustr,
                                            mxContext),
        UNO_QUERY_THROW);

    Reference<beans::XPropertySet> xPropSet(xControlModel, UNO_QUERY_THROW);

    Reference<XScriptEventsSupplier> xEventsSupplier(xPropSet, UNO_QUERY_THROW);
    Reference<XNameContainer> xEvents(xEventsSupplier->getEvents(), UNO_SET_THROW);
    script::ScriptEventDescriptor descr1;
    script::ScriptEventDescriptor descr2;
    script::ScriptEventDescriptor descr3;
    script::ScriptEventDescriptor descr4;
    xEvents->insertByName(u"b"_ustr, Any(descr1));
    xEvents->insertByName(u"a"_ustr, Any(descr2));
    xEvents->insertByName(u"1"_ustr, Any(descr3));
    xEvents->insertByName(u"A"_ustr, Any(descr4));

    Sequence<OUString> aEventNames(xEvents->getElementNames());
    sal_Int32 nEventCount = aEventNames.getLength();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), nEventCount);

    CPPUNIT_ASSERT_EQUAL(u"b"_ustr, aEventNames[0]);
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, aEventNames[1]);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, aEventNames[2]);
    CPPUNIT_ASSERT_EQUAL(u"A"_ustr, aEventNames[3]);

    css::uno::Reference<css::lang::XComponent>(xControlModel, css::uno::UNO_QUERY_THROW)->dispose();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
