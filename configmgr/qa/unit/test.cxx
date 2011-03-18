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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/beans/NamedValue.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertyState.hpp"
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
#include "cppuhelper/servicefactory.hxx"
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
#include "testshl/simpleheader.hxx"

namespace {

namespace css = com::sun::star;

void normalize(
    rtl::OUString const & path, rtl::OUString const & relative,
    rtl::OUString * normalizedPath, rtl::OUString * name)
{
    sal_Int32 i = relative.lastIndexOf('/');
    if (i == -1) {
        *normalizedPath = path;
        *name = relative;
    } else {
        rtl::OUStringBuffer buf(path);
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
        rtl::OUString const & path, rtl::OUString const & relative) const;

    void setKey(
        rtl::OUString const & path, rtl::OUString const & name,
        css::uno::Any const & value) const;

    bool resetKey(rtl::OUString const & path, rtl::OUString const & name) const;

    css::uno::Reference< css::uno::XInterface > createViewAccess(
        rtl::OUString const & path) const;

    css::uno::Reference< css::uno::XInterface > createUpdateAccess(
        rtl::OUString const & path) const;

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

bool TestThread::getSuccess() const {
    return success_;
}

void TestThread::run() {
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
        osl::Condition & stop, Test const & test, rtl::OUString const & path,
        rtl::OUString const & relative);

private:
    virtual bool iteration();

    Test const & test_;
    rtl::OUString path_;
    rtl::OUString relative_;
};

ReaderThread::ReaderThread(
    osl::Condition & stop, Test const & test, rtl::OUString const & path,
    rtl::OUString const & relative):
    TestThread(stop), test_(test), path_(path), relative_(relative)
{
    create();
}

bool ReaderThread::iteration() {
    return test_.getKey(path_, relative_).hasValue();
}

class WriterThread: public TestThread {
public:
    WriterThread(
        osl::Condition & stop, Test const & test, rtl::OUString const & path,
        rtl::OUString const & relative);

private:
    virtual bool iteration();

    Test const & test_;
    rtl::OUString path_;
    rtl::OUString name_;
    std::size_t index_;
};

WriterThread::WriterThread(
    osl::Condition & stop, Test const & test, rtl::OUString const & path,
    rtl::OUString const & relative):
    TestThread(stop), test_(test), index_(0)
{
    normalize(path, relative, &path_, &name_);
    create();
}

bool WriterThread::iteration() {
    rtl::OUString options[] = {
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("fish")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("chips")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("kippers")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bloaters")) };
    test_.setKey(path_, name_, css::uno::makeAny(options[index_]));
    index_ = (index_ + 1) % (sizeof options / sizeof (rtl::OUString));
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

void RecursiveTest::test() {
    properties_ = css::uno::Reference< css::beans::XPropertySet >(
        test_.createUpdateAccess(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    "dotuno:WebHtml"))),
        css::uno::UNO_QUERY_THROW);
    properties_->addPropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")), this);
    step();
    CPPUNIT_ASSERT(count_ == 0);
    css::uno::Reference< css::lang::XComponent >(
        properties_, css::uno::UNO_QUERY_THROW)->dispose();
}

RecursiveTest::~RecursiveTest() {
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
    CPPUNIT_ASSERT(
        evt.Source == properties_ &&
        evt.PropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Label")));
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

void SimpleRecursiveTest::step() const {
    test_.setKey(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                "dotuno:WebHtml")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")),
        css::uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("step"))));
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

void CrossThreadTest::step() const {
    osl::Condition stop;
    stop.set();
    WriterThread(
        stop, test_,
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                "dotuno:WebHtml")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label"))).join();
    test_.resetKey(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                "dotuno:WebHtml")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")));
}

void Test::setUp() {
    char const * forward = getForwardString();
    rtl_uString * registry = 0;
    CPPUNIT_ASSERT(
        rtl_convertStringToUString(
            &registry, forward, rtl_str_getLength(forward),
            osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)));
    context_ = css::uno::Reference< css::uno::XComponentContext >(
        css::uno::Reference< css::beans::XPropertySet >(
            cppu::createRegistryServiceFactory(
                rtl::OUString(registry, SAL_NO_ACQUIRE)),
            css::uno::UNO_QUERY_THROW)->getPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext"))),
        css::uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(
        context_->getValueByName(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/"
                    "com.sun.star.configuration.theDefaultProvider"))) >>=
        provider_);
}

void Test::tearDown() {
    css::uno::Reference< css::lang::XComponent >(
        context_, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testKeyFetch() {
    rtl::OUString s;
    CPPUNIT_ASSERT(
        getKey(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("L10N/ooLocale"))) >>=
        s);
    CPPUNIT_ASSERT(
        getKey(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Test/AString"))) >>=
        s);
}

void Test::testKeySet() {
    setKey(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString")),
        css::uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baa"))));
    rtl::OUString s;
    CPPUNIT_ASSERT(
        getKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString"))) >>=
        s);
    CPPUNIT_ASSERT(s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("baa")));
}

void Test::testKeyReset() {
    if (resetKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString"))))
    {
        rtl::OUString s;
        CPPUNIT_ASSERT(
            getKey(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString"))) >>=
            s);
        CPPUNIT_ASSERT(s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Foo")));
    }
}

void Test::testSetSetMemberName() {
    rtl::OUString s;
    CPPUNIT_ASSERT(
        getKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label"))) >>=
        s);
    CPPUNIT_ASSERT(
        s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Fontwork Shape")));

    css::uno::Reference< css::container::XNameAccess > access(
        createUpdateAccess(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/"
                    "Commands"))),
        css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNamed > member;
    access->getByName(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(".uno:FontworkGalleryFloater"))) >>=
        member;
    CPPUNIT_ASSERT(member.is());
    member->setName(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(".uno:FontworkShapeType")));
    css::uno::Reference< css::util::XChangesBatch >(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();

    CPPUNIT_ASSERT(
        getKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label"))) >>=
        s);
    CPPUNIT_ASSERT(
        s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Fontwork Gallery")));
}

void Test::testReadCommands() {
    css::uno::Reference< css::container::XNameAccess > access(
        createViewAccess(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/"
                    "Commands"))),
        css::uno::UNO_QUERY_THROW);
    css::uno::Sequence< rtl::OUString > names(access->getElementNames());
    CPPUNIT_ASSERT(names.getLength() == 695);
        // testSetSetMemberName() already removed ".uno:FontworkGalleryFloater"
    sal_uInt32 n = osl_getGlobalTimer();
    for (int i = 0; i < 8; ++i) {
        for (sal_Int32 j = 0; j < names.getLength(); ++j) {
            css::uno::Reference< css::container::XNameAccess > child;
            if (access->getByName(names[j]) >>= child) {
                CPPUNIT_ASSERT(child.is());
                child->getByName(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")));
                child->getByName(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContextLabel")));
                child->getByName(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Properties")));
            }
        }
    }
    n = osl_getGlobalTimer() - n;
    t_print("Reading elements took %" SAL_PRIuUINT32 " ms\n", n);
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testThreads() {
    struct Entry { rtl::OUString path; rtl::OUString relative; };
    Entry list[] = {
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Test/AString")) },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Test/AString")) },
        { rtl::OUString(
              RTL_CONSTASCII_USTRINGPARAM(
                  "/org.openoffice.UI.GenericCommands")),
          rtl::OUString(
              RTL_CONSTASCII_USTRINGPARAM(
                  "UserInterface/Commands/dotuno:WebHtml/Label")) },
        { rtl::OUString(
              RTL_CONSTASCII_USTRINGPARAM(
                  "/org.openoffice.UI.GenericCommands")),
          rtl::OUString(
              RTL_CONSTASCII_USTRINGPARAM(
                  "UserInterface/Commands/dotuno:NewPresentation/Label")) },
        { rtl::OUString(
              RTL_CONSTASCII_USTRINGPARAM(
                  "/org.openoffice.UI.GenericCommands")),
          rtl::OUString(
              RTL_CONSTASCII_USTRINGPARAM(
                  "UserInterface/Commands/dotuno:RecentFileList/Label")) },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("L10N/ooLocale")) },
        { rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
          rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Test/ABoolean")) }
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
            rtl::OUString path;
            rtl::OUString name;
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

void Test::testRecursive() {
    bool destroyed = false;
    rtl::Reference< RecursiveTest >(
        new SimpleRecursiveTest(*this, 100, &destroyed))->test();
    CPPUNIT_ASSERT(destroyed);
}

void Test::testCrossThreads() {
    bool destroyed = false;
    rtl::Reference< RecursiveTest >(
        new SimpleRecursiveTest(*this, 10, &destroyed))->test();
    CPPUNIT_ASSERT(destroyed);
}

css::uno::Any Test::getKey(
    rtl::OUString const & path, rtl::OUString const & relative) const
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > access(
        createViewAccess(path), css::uno::UNO_QUERY_THROW);
    css::uno::Any value(access->getByHierarchicalName(relative));
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
    return value;
}

void Test::setKey(
    rtl::OUString const & path, rtl::OUString const & name,
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

bool Test::resetKey(rtl::OUString const & path, rtl::OUString const & name)
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
    rtl::OUString const & path) const
{
    css::uno::Any arg(
        css::uno::makeAny(
            css::beans::NamedValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")),
                css::uno::makeAny(path))));
    return provider_->createInstanceWithArguments(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.ConfigurationAccess")),
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

css::uno::Reference< css::uno::XInterface > Test::createUpdateAccess(
    rtl::OUString const & path) const
{
    css::uno::Any arg(
        css::uno::makeAny(
            css::beans::NamedValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath")),
                css::uno::makeAny(path))));
    return provider_->createInstanceWithArguments(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "com.sun.star.configuration.ConfigurationUpdateAccess")),
        css::uno::Sequence< css::uno::Any >(&arg, 1));
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltest");

}

NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
