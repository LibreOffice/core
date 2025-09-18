/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>

#include <test/unoapi_test.hxx>

#include <array>

using namespace css;

class AcceleratorsTest : public UnoApiTest
{
public:
    AcceleratorsTest()
        : UnoApiTest(u"/framework/qa/cppunit/data"_ustr)
    {
    }

    void configNotification();

    CPPUNIT_TEST_SUITE(AcceleratorsTest);
    CPPUNIT_TEST(configNotification);
    CPPUNIT_TEST_SUITE_END();
};

void AcceleratorsTest::configNotification()
{
    uno::Reference<lang::XMultiServiceFactory> xFactory = getMultiServiceFactory();

    uno::Reference<ui::XAcceleratorConfiguration> xGlobalAccelCfg(
        xFactory->createInstance(u"com.sun.star.ui.GlobalAcceleratorConfiguration"_ustr),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGlobalAccelCfg.is());

    // Create two instances of the Writer module accelerator config so
    // we can test that updates are copied between them
    std::array<uno::Reference<ui::XAcceleratorConfiguration>, 2> xSwAccelCfgs;

    for (auto& xSwAccelCfg : xSwAccelCfgs)
    {
        uno::Sequence<uno::Any> args(1);
        args.getArray()[0] <<= u"com.sun.star.text.TextDocument"_ustr;

        xSwAccelCfg.set(xFactory->createInstanceWithArguments(
                            u"com.sun.star.ui.ModuleAcceleratorConfiguration"_ustr, args),
                        uno::UNO_QUERY);
        CPPUNIT_ASSERT(xSwAccelCfg.is());
    }

    // The two modules configs shouldn’t be the same object, otherwise
    // the test is not really testing anything
    CPPUNIT_ASSERT(xSwAccelCfgs[0] != xSwAccelCfgs[1]);

    // Create an instance of another module
    uno::Sequence<uno::Any> args(1);
    args.getArray()[0] <<= u"com.sun.star.sheet.SpreadsheetDocument"_ustr;
    uno::Reference<ui::XAcceleratorConfiguration> xScAccelCfg(
        xFactory->createInstanceWithArguments(
            u"com.sun.star.ui.ModuleAcceleratorConfiguration"_ustr, args),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT(xScAccelCfg.is());

    awt::KeyEvent aKeyEvent;
    aKeyEvent.KeyCode = awt::Key::F16;

    // Make sure the key we’re going to test isn’t already defined
    CPPUNIT_ASSERT_THROW(xGlobalAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);
    for (const auto& xSwAccelCfg : xSwAccelCfgs)
    {
        CPPUNIT_ASSERT_THROW(xSwAccelCfg->getCommandByKeyEvent(aKeyEvent),
                             css::container::NoSuchElementException);
    }
    CPPUNIT_ASSERT_THROW(xScAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);

    // Set the key event on the module
    xSwAccelCfgs[0]->setKeyEvent(aKeyEvent, u".uno:Bold"_ustr);

    // Store the changed configuration. This should trigger a change event
    uno::Reference<ui::XUIConfigurationPersistence> xConfPersistence(xSwAccelCfgs[0],
                                                                     uno::UNO_QUERY);
    xConfPersistence->store();

    // The key shouldn’t have been copied to the global configuration
    CPPUNIT_ASSERT_THROW(xGlobalAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);
    // Nor the Calc module
    CPPUNIT_ASSERT_THROW(xScAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);

    // Make sure the key was copied to all of the Writer module
    // configurations
    for (const auto& xSwAccelCfg : xSwAccelCfgs)
        CPPUNIT_ASSERT_EQUAL(u".uno:Bold"_ustr, xSwAccelCfg->getCommandByKeyEvent(aKeyEvent));

    // Modify the key to a different command
    xSwAccelCfgs[0]->setKeyEvent(aKeyEvent, u".uno:Italic"_ustr);
    xConfPersistence->store();

    // Check the propagation worked correctly
    CPPUNIT_ASSERT_THROW(xGlobalAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);
    CPPUNIT_ASSERT_THROW(xScAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);
    for (const auto& xSwAccelCfg : xSwAccelCfgs)
        CPPUNIT_ASSERT_EQUAL(u".uno:Italic"_ustr, xSwAccelCfg->getCommandByKeyEvent(aKeyEvent));

    // Remove the key binding
    xSwAccelCfgs[0]->removeKeyEvent(aKeyEvent);
    xConfPersistence->store();

    // Now none of the configs should have the key
    CPPUNIT_ASSERT_THROW(xGlobalAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);
    CPPUNIT_ASSERT_THROW(xScAccelCfg->getCommandByKeyEvent(aKeyEvent),
                         css::container::NoSuchElementException);
    for (const auto& xSwAccelCfg : xSwAccelCfgs)
    {
        CPPUNIT_ASSERT_THROW(xSwAccelCfg->getCommandByKeyEvent(aKeyEvent),
                             css::container::NoSuchElementException);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(AcceleratorsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
