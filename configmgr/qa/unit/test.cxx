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

#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertyState.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/container/XHierarchicalNameAccess.hpp"
#include "com/sun/star/container/XNameReplace.hpp"
#include "com/sun/star/container/XNamed.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "cppuhelper/implbase1.hxx"
#include "osl/conditn.hxx"
#include "osl/thread.h"
#include "osl/thread.hxx"
#include "osl/time.h"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/textcvt.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include <comphelper/processfactory.hxx>
#include <unotest/bootstrapfixturebase.hxx>

namespace {

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
        buf.append(sal_Unicode('/'));
        buf.append(relative.copy(0, i));
        *normalizedPath = buf.makeStringAndClear();
        *name = relative.copy(i + 1);
    }
}

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp();
    virtual void tearDown();

    void testKeyFetch();
    void testKeySet();
    void testKeyReset();
    void testSetSetMemberName();
    void testReadCommands();
    void testThreads();
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
    CPPUNIT_TEST(testReadCommands);
    CPPUNIT_TEST(testThreads);
    CPPUNIT_TEST(testRecursive);
    CPPUNIT_TEST(testCrossThreads);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference< css::uno::XComponentContext > context_;
    css::uno::Reference< css::lang::XMultiServiceFactory > provider_;
};

class TestThread: public osl::Thread {
public:
    TestThread(osl::Condition & stop);

    bool getSuccess() const;

protected:
    virtual bool iteration() = 0;

private:
    virtual void SAL_CALL run();

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
    virtual bool iteration();

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

class WriterThread: public TestThread {
public:
    WriterThread(
        osl::Condition & stop, Test const & test, OUString const & path,
        OUString const & relative);

private:
    virtual bool iteration();

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
    test_.setKey(path_, name_, css::uno::makeAny(options[index_]));
    index_ = (index_ + 1) % (sizeof options / sizeof (OUString));
    return true;
}

class RecursiveTest:
    public cppu::WeakImplHelper1< css::beans::XPropertyChangeListener >
{
public:
    RecursiveTest(Test const & theTest, int count, bool * destroyed);

    void test();

protected:
    virtual ~RecursiveTest();

    virtual void step() const = 0;

    Test const & test_;

private:
    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL propertyChange(
        css::beans::PropertyChangeEvent const &)
        throw (css::uno::RuntimeException);

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
    properties_ = css::uno::Reference< css::beans::XPropertySet >(
        test_.createUpdateAccess(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    "dotuno:WebHtml"))),
        css::uno::UNO_QUERY_THROW);
    properties_->addPropertyChangeListener(
        OUString("Label"), this);
    step();
    CPPUNIT_ASSERT(count_ == 0);
    css::uno::Reference< css::lang::XComponent >(
        properties_, css::uno::UNO_QUERY_THROW)->dispose();
}

RecursiveTest::~RecursiveTest()
{
    *destroyed_ = true;
}

void RecursiveTest::disposing(css::lang::EventObject const & Source)
    throw (css::uno::RuntimeException)
{
    CPPUNIT_ASSERT(properties_.is() && Source.Source == properties_);
    properties_.clear();
}

void RecursiveTest::propertyChange(css::beans::PropertyChangeEvent const & evt)
    throw (css::uno::RuntimeException)
{
    CPPUNIT_ASSERT( evt.Source == properties_ && evt.PropertyName == "Label" );
    if (count_ > 0) {
        --count_;
        step();
    }
}

class SimpleRecursiveTest: public RecursiveTest {
public:
    SimpleRecursiveTest(Test const & theTest, int count, bool * destroyed);

private:
    virtual void step() const;
};

SimpleRecursiveTest::SimpleRecursiveTest(
    Test const & theTest, int count, bool * destroyed):
    RecursiveTest(theTest, count, destroyed)
{}

void SimpleRecursiveTest::step() const
{
    test_.setKey(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                "dotuno:WebHtml")),
        OUString("Label"),
        css::uno::makeAny(OUString("step")));
}

class CrossThreadTest: public RecursiveTest {
public:
    CrossThreadTest(Test const & theTest, int count, bool * destroyed);

private:
    virtual void step() const;
};

CrossThreadTest::CrossThreadTest(
    Test const & theTest, int count, bool * destroyed):
    RecursiveTest(theTest, count, destroyed)
{}

void CrossThreadTest::step() const
{
    osl::Condition stop;
    stop.set();
    WriterThread(
        stop, test_,
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                "dotuno:WebHtml")),
        OUString("Label")).join();
    test_.resetKey(
        OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                "dotuno:WebHtml")),
        OUString("Label"));
}

void Test::setUp()
{
    provider_ = css::configuration::theDefaultProvider::get(
                        comphelper::getProcessComponentContext() );
}

void Test::tearDown()
{
}

void Test::testKeyFetch()
{
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            OUString("/org.openoffice.Setup"),
            OUString("L10N/ooLocale")) >>=
        s);
    CPPUNIT_ASSERT(
        getKey(
            OUString("/org.openoffice.Setup"),
            OUString("Test/AString")) >>=
        s);
}

void Test::testKeySet()
{
    setKey(
        OUString("/org.openoffice.Setup/Test"),
        OUString("AString"),
        css::uno::makeAny(OUString("baa")));
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            OUString("/org.openoffice.Setup/Test"),
            OUString("AString")) >>=
        s);
    CPPUNIT_ASSERT( s == "baa" );
}

void Test::testKeyReset()
{
    if (resetKey(
            OUString("/org.openoffice.Setup/Test"),
            OUString("AString")))
    {
        OUString s;
        CPPUNIT_ASSERT(
            getKey(
                OUString("/org.openoffice.Setup/Test"),
                OUString("AString")) >>=
            s);
        CPPUNIT_ASSERT( s == "Foo" );
    }
}

void Test::testSetSetMemberName()
{
    OUString s;
    CPPUNIT_ASSERT(
        getKey(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType")),
            OUString("Label")) >>=
        s);
    CPPUNIT_ASSERT( s == "Fontwork Shape" );

    css::uno::Reference< css::container::XNameAccess > access(
        createUpdateAccess(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/"
                    "Commands"))),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNamed > member;
    access->getByName(
        OUString(".uno:FontworkGalleryFloater")) >>=
        member;
    CPPUNIT_ASSERT(member.is());
    member->setName(
        OUString(".uno:FontworkShapeType"));
    css::uno::Reference< css::util::XChangesBatch >(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();

    CPPUNIT_ASSERT(
        getKey(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType")),
            OUString("Label")) >>=
        s);
    CPPUNIT_ASSERT( s == "Fontwork Gallery" );
}

void Test::testReadCommands()
{
    css::uno::Reference< css::container::XNameAccess > access(
        createViewAccess(
            OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/"
                    "Commands"))),
        css::uno::UNO_QUERY_THROW);
    css::uno::Sequence< OUString > names(access->getElementNames());
    CPPUNIT_ASSERT(names.getLength() == 695);
        // testSetSetMemberName() already removed ".uno:FontworkGalleryFloater"
    sal_uInt32 n = osl_getGlobalTimer();
    for (int i = 0; i < 8; ++i) {
        for (sal_Int32 j = 0; j < names.getLength(); ++j) {
            css::uno::Reference< css::container::XNameAccess > child;
            if (access->getByName(names[j]) >>= child) {
                CPPUNIT_ASSERT(child.is());
                child->getByName(
                    OUString("Label"));
                child->getByName(
                    OUString("ContextLabel"));
                child->getByName(
                    OUString("Properties"));
            }
        }
    }
    n = osl_getGlobalTimer() - n;
    printf("Reading elements took %" SAL_PRIuUINT32 " ms\n", n);
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testThreads()
{
    struct Entry { OUString path; OUString relative; };
    Entry list[] = {
        { OUString("/org.openoffice.Setup"),
          OUString("Test/AString") },
        { OUString("/org.openoffice.Setup"),
          OUString("Test/AString") },
        { OUString(
                  "/org.openoffice.UI.GenericCommands"),
          OUString(
                  "UserInterface/Commands/dotuno:WebHtml/Label") },
        { OUString(
                  "/org.openoffice.UI.GenericCommands"),
          OUString(
                  "UserInterface/Commands/dotuno:NewPresentation/Label") },
        { OUString(
                  "/org.openoffice.UI.GenericCommands"),
          OUString(
                  "UserInterface/Commands/dotuno:RecentFileList/Label") },
        { OUString("/org.openoffice.Setup"),
          OUString("L10N/ooLocale") },
        { OUString("/org.openoffice.Setup"),
          OUString("Test/ABoolean") }
    };
    std::size_t const numReaders = sizeof list / sizeof (Entry);
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
        success = success && readers[i]->getSuccess();
        delete readers[i];
    }
    for (std::size_t i = 0; i < numWriters; ++i) {
        writers[i]->join();
        success = success && writers[i]->getSuccess();
        delete writers[i];
    }
    CPPUNIT_ASSERT(success);
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
        css::uno::makeAny(
            css::beans::NamedValue(
                OUString("nodepath"),
                css::uno::makeAny(path))));
    return provider_->createInstanceWithArguments(
        OUString(
                "com.sun.star.configuration.ConfigurationAccess"),
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

css::uno::Reference< css::uno::XInterface > Test::createUpdateAccess(
    OUString const & path) const
{
    css::uno::Any arg(
        css::uno::makeAny(
            css::beans::NamedValue(
                OUString("nodepath"),
                css::uno::makeAny(path))));
    return provider_->createInstanceWithArguments(
        OUString(
                "com.sun.star.configuration.ConfigurationUpdateAccess"),
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
