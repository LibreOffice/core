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

#include <cstddef>
#include <string_view>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
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
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/conditn.hxx>
#include <osl/thread.h>
#include <osl/thread.hxx>
#include <osl/time.h>
#include <rtl/ref.hxx>
#include <rtl/string.h>
#include <rtl/textcvt.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/configuration.hxx>
#include <comphelper/configurationlistener.hxx>
#include <unotest/bootstrapfixturebase.hxx>
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
    void testReadCommands();
    void testListener();
#if 0
    void testThreads();
#endif
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
    CPPUNIT_TEST(testReadCommands);
    CPPUNIT_TEST(testListener);
#if 0
    CPPUNIT_TEST(testThreads);
#endif
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
            "/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                     ".uno:WebHtml"),
        css::uno::UNO_QUERY_THROW);
    properties_->addPropertyChangeListener("Label", this);
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
    CPPUNIT_ASSERT_EQUAL( OUString("Label"), evt.PropertyName );
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
        "/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                 ".uno:WebHtml",
        "Label",
        css::uno::Any(OUString("step")));
}

void Test::setUp()
{
    provider_ = css::configuration::theDefaultProvider::get(
                        comphelper::getProcessComponentContext() );
}

void Test::testKeyFetch()
{
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            "/org.openoffice.System",
            "L10N/Locale") >>=
        s);
}

void Test::testKeySet()
{
    setKey(
        "/org.openoffice.System/L10N",
        "Locale",
        css::uno::Any(OUString("com.sun.star.configuration.backend.LocaleBackend UILocale")));
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            "/org.openoffice.System/L10N",
            "Locale") >>=
        s);
    CPPUNIT_ASSERT_EQUAL( OUString("com.sun.star.configuration.backend.LocaleBackend UILocale"), s );
}

void Test::testKeyReset()
{
    if (resetKey(
            "/org.openoffice.System/L10N",
            "Locale"))
    {
        OUString s;
        CPPUNIT_ASSERT(
            getKey(
                "/org.openoffice.System/L10N",
                "Locale") >>=
            s);
        CPPUNIT_ASSERT_EQUAL( OUString("com.sun.star.configuration.backend.LocaleBackend Locale"), s );
    }
}

void Test::testSetSetMemberName()
{
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            "/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                     ".uno:FontworkShapeType",
            "Label") >>=
        s);
    CPPUNIT_ASSERT_EQUAL( OUString("Fontwork Shape"), s );

    css::uno::Reference< css::container::XNameAccess > access(
        createUpdateAccess(
            "/org.openoffice.Office.UI.GenericCommands/UserInterface/"
                     "Commands"),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNamed > member;
    access->getByName(".uno:FontworkGalleryFloater") >>= member;
    CPPUNIT_ASSERT(member.is());
    member->setName(".uno:FontworkShapeType");
    css::uno::Reference< css::util::XChangesBatch >(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();

    CPPUNIT_ASSERT(
        getKey(
            "/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType",
            "Label") >>=
        s);
    CPPUNIT_ASSERT_EQUAL( OUString("Fontwork Style"), s );
}

void Test::testInsertSetMember() {
    css::uno::Reference<css::container::XNameContainer> access(
        createUpdateAccess(
            "/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands"),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::uno::XInterface> member;
    member.set(
        css::uno::Reference<css::lang::XSingleServiceFactory>(
            access, css::uno::UNO_QUERY_THROW)->createInstance());
    CPPUNIT_ASSERT(member.is());
    access->insertByName("A", css::uno::Any(member));
    member.set(
        css::uno::Reference<css::lang::XSingleServiceFactory>(
            access, css::uno::UNO_QUERY_THROW)->createInstance());
    CPPUNIT_ASSERT(member.is());
    try {
        access->insertByName("", css::uno::Any(member));
        CPPUNIT_FAIL("expected IllegalArgumentException");
    } catch (css::lang::IllegalArgumentException &) {}
    try {
        access->insertByName("\x01", css::uno::Any(member));
        CPPUNIT_FAIL("expected IllegalArgumentException");
    } catch (css::lang::IllegalArgumentException &) {}
    try {
        access->insertByName("a/b", css::uno::Any(member));
    } catch (css::lang::IllegalArgumentException &) {
        CPPUNIT_FAIL("unexpected IllegalArgumentException");
    }
    css::uno::Reference<css::util::XChangesBatch>(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference<css::lang::XComponent>(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testReadCommands()
{
    css::uno::Reference< css::container::XNameAccess > access(
        createViewAccess(
            "/org.openoffice.Office.UI.GenericCommands/UserInterface/"
                     "Commands"),
        css::uno::UNO_QUERY_THROW);
    css::uno::Sequence< OUString > names(access->getElementNames());

    /*CPPUNIT_ASSERT_EQUAL(749, names.getLength());*/
    // testSetSetMemberName() already removed ".uno:FontworkGalleryFloater"
    sal_uInt32 n = osl_getGlobalTimer();
    for (int i = 0; i < 8; ++i) {
        for (sal_Int32 j = 0; j < names.getLength(); ++j) {
            css::uno::Reference< css::container::XNameAccess > child;
            if (access->getByName(names[j]) >>= child) {
                CPPUNIT_ASSERT(child.is());
                child->getByName("Label");
                child->getByName("ContextLabel");
                child->getByName("Properties");
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
    OUString aRandomPath = "/org.openoffice.Office.Math/View";

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

        comphelper::ConfigurationListenerProperty<bool> aSetting(xListener, "AutoRedraw");
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
                "nodepath",
                css::uno::Any(path)));
    return provider_->createInstanceWithArguments(
        "com.sun.star.configuration.ConfigurationAccess",
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

css::uno::Reference< css::uno::XInterface > Test::createUpdateAccess(
    OUString const & path) const
{
    css::uno::Any arg(
            css::beans::NamedValue(
                "nodepath",
                css::uno::Any(path)));
    return provider_->createInstanceWithArguments(
        "com.sun.star.configuration.ConfigurationUpdateAccess",
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

#if 0
class TestThread: public osl::Thread {
public:
    TestThread(osl::Condition & stop);

    bool getSuccess() const;

protected:
    virtual bool iteration() = 0;

private:
    virtual void SAL_CALL run() override;

    osl::Condition & stop_;
    bool success_;
};

TestThread::TestThread(
    osl::Condition & stop):
    stop_(stop), success_(true)
{}

bool TestThread::getSuccess() const
{
    return success_;
}

void TestThread::run()
{
    try {
        while (!stop_.check()) {
            if (!iteration()) {
                success_ = false;
            }
        }
    } catch (...) {
        success_ = false;
    }
}

class ReaderThread: public TestThread {
public:
    ReaderThread(
        osl::Condition & stop, Test const & test, OUString const & path,
        OUString const & relative);

private:
    virtual bool iteration() override;

    Test const & test_;
    OUString path_;
    OUString relative_;
};

ReaderThread::ReaderThread(
    osl::Condition & stop, Test const & test, OUString const & path,
    OUString const & relative):
    TestThread(stop), test_(test), path_(path), relative_(relative)
{
    create();
}

bool ReaderThread::iteration()
{
    return test_.getKey(path_, relative_).hasValue();
}

void normalize(
    OUString const & path, OUString const & relative,
    OUString * normalizedPath, OUString * name)
{
    sal_Int32 i = relative.lastIndexOf('/');
    if (i == -1) {
        *normalizedPath = path;
        *name = relative;
    } else {
        OUStringBuffer buf(path);
        buf.append('/');
        buf.append(std::u16string_view(relative).substr(0, i));
        *normalizedPath = buf.makeStringAndClear();
        *name = relative.copy(i + 1);
    }
}

class WriterThread: public TestThread {
public:
    WriterThread(
        osl::Condition & stop, Test const & test, OUString const & path,
        OUString const & relative);

private:
    virtual bool iteration() override;

    Test const & test_;
    OUString path_;
    OUString name_;
    std::size_t index_;
};

WriterThread::WriterThread(
    osl::Condition & stop, Test const & test, OUString const & path,
    OUString const & relative):
    TestThread(stop), test_(test), index_(0)
{
    normalize(path, relative, &path_, &name_);
    create();
}

bool WriterThread::iteration() {
    OUString options[] = {
        OUString("fish"),
        OUString("chips"),
        OUString("kippers"),
        OUString("bloaters") };

    test_.setKey(path_, name_, css::uno::Any(options[index_]));
    index_ = (index_ + 1) % SAL_N_ELEMENTS(options);
    return true;
}

void Test::testThreads()
{
    struct Entry { OUString path; OUString relative; };
    Entry list[] = {
        { OUString(
                  "/org.openoffice.Office.UI.GenericCommands"),
          OUString(
                  "UserInterface/Commands/.uno:WebHtml") },
        { OUString(
                  "/org.openoffice.Office.UI.GenericCommands"),
          OUString(
                  "UserInterface/Commands/.uno:NewPresentation") },
        { OUString(
                  "/org.openoffice.Office.UI.GenericCommands"),
          OUString(
                  "UserInterface/Commands/.uno:RecentFileList") },
        { OUString("/org.openoffice.System"),
          OUString("L10N/Locale") }
    };
    std::size_t const numReaders = SAL_N_ELEMENTS(list);
    std::size_t const numWriters = numReaders - 2;
    ReaderThread * readers[numReaders];
    WriterThread * writers[numWriters];
    osl::Condition stop;
    for (std::size_t i = 0; i < numReaders; ++i) {
        CPPUNIT_ASSERT(getKey(list[i].path, list[i].relative).hasValue());
        readers[i] = new ReaderThread(
            stop, *this, list[i].path, list[i].relative);
    }
    for (std::size_t i = 0; i < numWriters; ++i) {
        writers[i] = new WriterThread(
            stop, *this, list[i].path, list[i].relative);
    }
    for (int i = 0; i < 5; ++i) {
        for (std::size_t j = 0; j < numReaders; ++j) {
            OUString path;
            OUString name;
            normalize(list[j].path, list[j].relative, &path, &name);
            resetKey(path, name);
            osl::Thread::yield();
        }
    }
    stop.set();
    bool success = true;
    for (std::size_t i = 0; i < numReaders; ++i) {
        readers[i]->join();
        CPPUNIT_ASSERT(readers[i]->getSuccess());
        delete readers[i];
    }
    for (std::size_t i = 0; i < numWriters; ++i) {
        writers[i]->join();
        CPPUNIT_ASSERT(writers[i]->getSuccess());
        delete writers[i];
    }

    CPPUNIT_ASSERT(success);
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
