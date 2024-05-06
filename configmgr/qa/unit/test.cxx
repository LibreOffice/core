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

#include <sal/config.h>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/configuration/ReadOnlyAccess.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/time.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/configurationlistener.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <officecfg/Office/Math.hxx>

namespace {

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp() override;

    void testKeyFetch();
    void testKeySet();
    void testKeyReset();
    void testSetSetMemberName();
    void testInsertSetMember();
    void testLocalizedProperty();
    void testReadCommands();
    void testListener();
    void testRecursive();
    void testCrossThreads();

    css::uno::Any getKey(
        OUString const & path, OUString const & relative) const;

    void setKey(
        OUString const & path, OUString const & name,
        css::uno::Any const & value) const;

    bool resetKey(OUString const & path, OUString const & name) const;

    css::uno::Reference< css::uno::XInterface > createViewAccess(
        OUString const & path) const;

    css::uno::Reference< css::uno::XInterface > createUpdateAccess(
        OUString const & path) const;

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testKeyFetch);
    CPPUNIT_TEST(testKeySet);
    CPPUNIT_TEST(testKeyReset);
    CPPUNIT_TEST(testSetSetMemberName);
    CPPUNIT_TEST(testInsertSetMember);
    CPPUNIT_TEST(testLocalizedProperty);
    CPPUNIT_TEST(testReadCommands);
    CPPUNIT_TEST(testListener);
    CPPUNIT_TEST(testRecursive);
    CPPUNIT_TEST(testCrossThreads);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference< css::lang::XMultiServiceFactory > provider_;
};

class RecursiveTest:
    public cppu::WeakImplHelper< css::beans::XPropertyChangeListener >
{
public:
    RecursiveTest(Test const & theTest, int count, bool * destroyed);

    void test();

protected:
    virtual ~RecursiveTest() override;

    virtual void step() const = 0;

    Test const & test_;

private:
    virtual void SAL_CALL disposing(css::lang::EventObject const &) override;

    virtual void SAL_CALL propertyChange(
        css::beans::PropertyChangeEvent const &) override;

    int count_;
    bool * destroyed_;
    css::uno::Reference< css::beans::XPropertySet > properties_;
};

RecursiveTest::RecursiveTest(
    Test const & theTest, int count, bool * destroyed):
    test_(theTest), count_(count), destroyed_(destroyed)
{}

void RecursiveTest::test()
{
    properties_.set(
        test_.createUpdateAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                     ".uno:WebHtml"_ustr),
        css::uno::UNO_QUERY_THROW);
    properties_->addPropertyChangeListener(u"Label"_ustr, this);
    step();
    CPPUNIT_ASSERT_EQUAL(0, count_);
    css::uno::Reference< css::lang::XComponent >(
        properties_, css::uno::UNO_QUERY_THROW)->dispose();
}

RecursiveTest::~RecursiveTest()
{
    *destroyed_ = true;
}

void RecursiveTest::disposing(css::lang::EventObject const & Source)
{
    CPPUNIT_ASSERT(properties_.is());
    CPPUNIT_ASSERT_EQUAL(
        css::uno::Reference<css::uno::XInterface>(
            properties_, css::uno::UNO_QUERY_THROW),
        Source.Source);
    properties_.clear();
}

void RecursiveTest::propertyChange(css::beans::PropertyChangeEvent const & evt)
{
    CPPUNIT_ASSERT_EQUAL(
        css::uno::Reference<css::uno::XInterface>(
            properties_, css::uno::UNO_QUERY_THROW),
        evt.Source);
    CPPUNIT_ASSERT_EQUAL( u"Label"_ustr, evt.PropertyName );
    if (count_ > 0) {
        --count_;
        step();
    }
}

class SimpleRecursiveTest: public RecursiveTest {
public:
    SimpleRecursiveTest(Test const & theTest, int count, bool * destroyed);

private:
    virtual void step() const override;
};

SimpleRecursiveTest::SimpleRecursiveTest(
    Test const & theTest, int count, bool * destroyed):
    RecursiveTest(theTest, count, destroyed)
{}

void SimpleRecursiveTest::step() const
{
    test_.setKey(
        u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                 ".uno:WebHtml"_ustr,
        u"Label"_ustr,
        css::uno::Any(u"step"_ustr));
}

void Test::setUp()
{
    provider_ = css::configuration::theDefaultProvider::get(
                        comphelper::getProcessComponentContext() );
}

void Test::testKeyFetch()
{
    {
        OUString s;
        CPPUNIT_ASSERT(
            getKey(
                u"/org.openoffice.System"_ustr,
                u"L10N/Locale"_ustr) >>=
            s);
    }
    {
        auto const v = getKey(u"/org.openoffice.System"_ustr, u"L10N/['Locale']"_ustr);
        CPPUNIT_ASSERT_EQUAL(cppu::UnoType<OUString>::get(), v.getValueType());
    }
}

void Test::testKeySet()
{
    setKey(
        u"/org.openoffice.System/L10N"_ustr,
        u"Locale"_ustr,
        css::uno::Any(u"com.sun.star.configuration.backend.LocaleBackend UILocale"_ustr));
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            u"/org.openoffice.System/L10N"_ustr,
            u"Locale"_ustr) >>=
        s);
    CPPUNIT_ASSERT_EQUAL( u"com.sun.star.configuration.backend.LocaleBackend UILocale"_ustr, s );
}

void Test::testKeyReset()
{
    if (resetKey(
            u"/org.openoffice.System/L10N"_ustr,
            u"Locale"_ustr))
    {
        OUString s;
        CPPUNIT_ASSERT(
            getKey(
                u"/org.openoffice.System/L10N"_ustr,
                u"Locale"_ustr) >>=
            s);
        CPPUNIT_ASSERT_EQUAL( u"com.sun.star.configuration.backend.LocaleBackend Locale"_ustr, s );
    }
}

void Test::testSetSetMemberName()
{
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                     ".uno:FontworkShapeType"_ustr,
            u"Label"_ustr) >>=
        s);
    CPPUNIT_ASSERT_EQUAL( u"Fontwork Shape"_ustr, s );

    css::uno::Reference< css::container::XNameAccess > access(
        createUpdateAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/"
                     "Commands"_ustr),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNamed > member;
    access->getByName(u".uno:FontworkGalleryFloater"_ustr) >>= member;
    CPPUNIT_ASSERT(member.is());
    member->setName(u".uno:FontworkShapeType"_ustr);
    css::uno::Reference< css::util::XChangesBatch >(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();

    CPPUNIT_ASSERT(
        getKey(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType"_ustr,
            u"Label"_ustr) >>=
        s);
    CPPUNIT_ASSERT_EQUAL( u"Insert Fontwork"_ustr, s );
}

void Test::testInsertSetMember() {
    css::uno::Reference<css::container::XNameContainer> access(
        createUpdateAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands"_ustr),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::uno::XInterface> member;
    member.set(
        css::uno::Reference<css::lang::XSingleServiceFactory>(
            access, css::uno::UNO_QUERY_THROW)->createInstance());
    CPPUNIT_ASSERT(member.is());
    access->insertByName(u"A"_ustr, css::uno::Any(member));
    member.set(
        css::uno::Reference<css::lang::XSingleServiceFactory>(
            access, css::uno::UNO_QUERY_THROW)->createInstance());
    CPPUNIT_ASSERT(member.is());
    try {
        access->insertByName(u""_ustr, css::uno::Any(member));
        CPPUNIT_FAIL("expected IllegalArgumentException");
    } catch (css::lang::IllegalArgumentException &) {}
    try {
        access->insertByName(u"\x01"_ustr, css::uno::Any(member));
        CPPUNIT_FAIL("expected IllegalArgumentException");
    } catch (css::lang::IllegalArgumentException &) {}
    try {
        access->insertByName(u"a/b"_ustr, css::uno::Any(member));
    } catch (css::lang::IllegalArgumentException &) {
        CPPUNIT_FAIL("unexpected IllegalArgumentException");
    }
    css::uno::Reference<css::util::XChangesBatch>(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference<css::lang::XComponent>(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testLocalizedProperty() {
    auto const access = css::configuration::ReadOnlyAccess::create(
        comphelper::getProcessComponentContext(), u"*"_ustr);
    {
        // See <https://bugs.documentfoundation.org/show_bug.cgi?id=33638> "Pagination extension
        // not localized in LibreOffice", which wants to retrieve the non-canonical xml:lang="pt-PT"
        // value for the passed-in "pt" locale:
        OUString v;
        CPPUNIT_ASSERT(
            access->getByHierarchicalName(u"/org.libreoffice.unittest/localized/*pt"_ustr) >>= v);
        CPPUNIT_ASSERT_EQUAL(u"pt-PT"_ustr, v);
    }
    {
        // See <https://gerrit.libreoffice.org/c/core/+/147089> "configmgr: fix no longer found
        // es-419 -> es fallback", which wants to retrieve the xml:lang="es" value for the passed-in
        // "es-419" locale:
        OUString v;
        CPPUNIT_ASSERT(
            access->getByHierarchicalName(u"/org.libreoffice.unittest/localized/*es-419"_ustr) >>= v);
        CPPUNIT_ASSERT_EQUAL(u"es"_ustr, v);
    }
    {
        // See <https://git.libreoffice.org/core/+/dfc28be2487c13be36a90efd778b8d8f179c589d%5E%21>
        // "configmgr: Use a proper LanguageTag-based locale fallback mechanism":
        OUString v;
        CPPUNIT_ASSERT(
            access->getByHierarchicalName(u"/org.libreoffice.unittest/localized/*zh-Hant-TW"_ustr) >>= v);
        CPPUNIT_ASSERT_EQUAL(u"zh-TW"_ustr, v);
    }
    {
        // Make sure a degenerate passed-in "-" locale is handled gracefully:
        OUString v;
        CPPUNIT_ASSERT(
            access->getByHierarchicalName(u"/org.libreoffice.unittest/localized/*-"_ustr) >>= v);
        CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, v);
    }
    {
        // Make sure a degenerate passed-in "-" locale is handled gracefully:
        OUString v;
        CPPUNIT_ASSERT(
            access->getByHierarchicalName(u"/org.libreoffice.unittest/noDefaultLang/*-"_ustr) >>= v);
        CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, v);
    }
}

void Test::testReadCommands()
{
    css::uno::Reference< css::container::XNameAccess > access(
        createViewAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/"
                     "Commands"_ustr),
        css::uno::UNO_QUERY_THROW);
    const css::uno::Sequence< OUString > names(access->getElementNames());

    /*CPPUNIT_ASSERT_EQUAL(749, names.getLength());*/
    // testSetSetMemberName() already removed ".uno:FontworkGalleryFloater"
    sal_uInt32 n = osl_getGlobalTimer();
    for (int i = 0; i < 8; ++i) {
        for (OUString const & childName : names) {
            css::uno::Reference< css::container::XNameAccess > child;
            if (access->getByName(childName) >>= child) {
                CPPUNIT_ASSERT(child.is());
                child->getByName(u"Label"_ustr);
                child->getByName(u"ContextLabel"_ustr);
                child->getByName(u"Properties"_ustr);
            }
        }
    }
    n = osl_getGlobalTimer() - n;
    printf("Reading elements took %" SAL_PRIuUINT32 " ms\n", n);
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testListener()
{
    OUString aRandomPath = u"/org.openoffice.Office.Math/View"_ustr;

    // test with no props.
    {
        rtl::Reference xListener(
            new comphelper::ConfigurationListener(aRandomPath));
        xListener->dispose();
    }

    // test some changes
    {
        rtl::Reference xListener(
            new comphelper::ConfigurationListener(aRandomPath));

        comphelper::ConfigurationListenerProperty<bool> aSetting(xListener, u"AutoRedraw"_ustr);
        CPPUNIT_ASSERT_MESSAGE("check AutoRedraw defaults to true", aSetting.get());

        // set to false
        {
            std::shared_ptr< comphelper::ConfigurationChanges > xChanges(
                comphelper::ConfigurationChanges::create());
            officecfg::Office::Math::View::AutoRedraw::set(false, xChanges);
            xChanges->commit();
        }
        CPPUNIT_ASSERT_MESSAGE("listener failed to trigger", !aSetting.get());

        // set to true
        {
            std::shared_ptr< comphelper::ConfigurationChanges > xChanges(
                comphelper::ConfigurationChanges::create());
            officecfg::Office::Math::View::AutoRedraw::set(true, xChanges);
            xChanges->commit();
        }
        CPPUNIT_ASSERT_MESSAGE("listener failed to trigger", aSetting.get());

        xListener->dispose();
    }
}

void Test::testRecursive()
{
    bool destroyed = false;
    rtl::Reference< RecursiveTest >(
        new SimpleRecursiveTest(*this, 100, &destroyed))->test();
    CPPUNIT_ASSERT(destroyed);
}

void Test::testCrossThreads()
{
    bool destroyed = false;
    rtl::Reference< RecursiveTest >(
        new SimpleRecursiveTest(*this, 10, &destroyed))->test();
    CPPUNIT_ASSERT(destroyed);
}

css::uno::Any Test::getKey(
    OUString const & path, OUString const & relative) const
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > access(
        createViewAccess(path), css::uno::UNO_QUERY_THROW);
    css::uno::Any value(access->getByHierarchicalName(relative));
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
    return value;
}

void Test::setKey(
    OUString const & path, OUString const & name,
    css::uno::Any const & value) const
{
    css::uno::Reference< css::container::XNameReplace > access(
        createUpdateAccess(path), css::uno::UNO_QUERY_THROW);
    access->replaceByName(name, value);
    css::uno::Reference< css::util::XChangesBatch >(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

bool Test::resetKey(OUString const & path, OUString const & name)
    const
{
    //TODO: support setPropertyToDefault
    css::uno::Reference< css::util::XChangesBatch > access(
        createUpdateAccess(path), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::beans::XPropertyState > state(
        access, css::uno::UNO_QUERY);
    if (!state.is()) {
        return false;
    }
    state->setPropertyToDefault(name);
    access->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
    return true;
}

css::uno::Reference< css::uno::XInterface > Test::createViewAccess(
    OUString const & path) const
{
    css::uno::Any arg(
            css::beans::NamedValue(
                u"nodepath"_ustr,
                css::uno::Any(path)));
    return provider_->createInstanceWithArguments(
        u"com.sun.star.configuration.ConfigurationAccess"_ustr,
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

css::uno::Reference< css::uno::XInterface > Test::createUpdateAccess(
    OUString const & path) const
{
    css::uno::Any arg(
            css::beans::NamedValue(
                u"nodepath"_ustr,
                css::uno::Any(path)));
    return provider_->createInstanceWithArguments(
        u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
