/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <cstddef>
#include <cstdlib>

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
#include "osl/process.h"
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
#include "gtest/gtest.h"

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

class Test: public ::testing::Test {
public:
    virtual void SetUp();

    virtual void TearDown();

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
    ASSERT_TRUE(count_ == 0);
    css::uno::Reference< css::lang::XComponent >(
        properties_, css::uno::UNO_QUERY_THROW)->dispose();
}

RecursiveTest::~RecursiveTest() {
    *destroyed_ = true;
}

void RecursiveTest::disposing(css::lang::EventObject const & Source)
    throw (css::uno::RuntimeException)
{
    ASSERT_TRUE(properties_.is() && Source.Source == properties_);
    properties_.clear();
}

void RecursiveTest::propertyChange(css::beans::PropertyChangeEvent const & evt)
    throw (css::uno::RuntimeException)
{
    ASSERT_TRUE(
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

void Test::SetUp() {
    char const * forward = getenv("CONFIGMGR_UNIT_FORWARD_STRING");
    rtl_uString * registry = 0;
    ASSERT_TRUE(
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
    ASSERT_TRUE(
        context_->getValueByName(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/"
                    "com.sun.star.configuration.theDefaultProvider"))) >>=
        provider_);
}

void Test::TearDown() {
    css::uno::Reference< css::lang::XComponent >(
        context_, css::uno::UNO_QUERY_THROW)->dispose();
}

TEST_F(Test, testKeyFetch) {
    rtl::OUString s;
    ASSERT_TRUE(
        getKey(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("L10N/ooLocale"))) >>=
        s);
    ASSERT_TRUE(
        getKey(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Test/AString"))) >>=
        s);
}

TEST_F(Test, testKeySet) {
    setKey(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString")),
        css::uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("baa"))));
    rtl::OUString s;
    ASSERT_TRUE(
        getKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString"))) >>=
        s);
    ASSERT_TRUE(s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("baa")));
}

TEST_F(Test, testKeyReset) {
    if (resetKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString"))))
    {
        rtl::OUString s;
        ASSERT_TRUE(
            getKey(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup/Test")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AString"))) >>=
            s);
        ASSERT_TRUE(s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Foo")));
    }
}

TEST_F(Test, testSetSetMemberName) {
    rtl::OUString s;
    ASSERT_TRUE(
        getKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label"))) >>=
        s);
    ASSERT_TRUE(
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
    ASSERT_TRUE(member.is());
    member->setName(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(".uno:FontworkShapeType")));
    css::uno::Reference< css::util::XChangesBatch >(
        access, css::uno::UNO_QUERY_THROW)->commitChanges();
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();

    ASSERT_TRUE(
        getKey(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label"))) >>=
        s);
    ASSERT_TRUE(
        s.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Fontwork Gallery")));
}

TEST_F(Test, testReadCommands) {
    css::uno::Reference< css::container::XNameAccess > access(
        createViewAccess(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "/org.openoffice.UI.GenericCommands/UserInterface/"
                    "Commands"))),
        css::uno::UNO_QUERY_THROW);
    css::uno::Sequence< rtl::OUString > names(access->getElementNames());
    ASSERT_TRUE(names.getLength() == 695);
        // testSetSetMemberName() already removed ".uno:FontworkGalleryFloater"
    sal_uInt32 n = osl_getGlobalTimer();
    for (int i = 0; i < 8; ++i) {
        for (sal_Int32 j = 0; j < names.getLength(); ++j) {
            css::uno::Reference< css::container::XNameAccess > child;
            if (access->getByName(names[j]) >>= child) {
                ASSERT_TRUE(child.is());
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
    printf("Reading elements took %" SAL_PRIuUINT32 " ms\n", n);
    css::uno::Reference< css::lang::XComponent >(
        access, css::uno::UNO_QUERY_THROW)->dispose();
}

TEST_F(Test, testThreads) {
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
        ASSERT_TRUE(getKey(list[i].path, list[i].relative).hasValue());
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
    ASSERT_TRUE(success);
}

TEST_F(Test, testRecursive) {
    bool destroyed = false;
    rtl::Reference< RecursiveTest >(
        new SimpleRecursiveTest(*this, 100, &destroyed))->test();
    ASSERT_TRUE(destroyed);
}

TEST_F(Test, testCrossThreads) {
    bool destroyed = false;
    rtl::Reference< RecursiveTest >(
        new SimpleRecursiveTest(*this, 10, &destroyed))->test();
    ASSERT_TRUE(destroyed);
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


}

int main(int argc, char **argv)
{
    osl_setCommandArgs(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
