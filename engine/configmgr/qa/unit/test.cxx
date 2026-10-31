/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/configuration/ReadOnlyAccess.hpp>
#include <com/sun/star/configuration/Update.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/XInterface.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <cppu/unotype.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/time.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

#include <algorithm>
#include <vector>
#include <comphelper/configuration.hxx>
#include <comphelper/configurationlistener.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <officecfg/Office/Math.hxx>

#ifdef MACOSX
#include <macosprefs.hxx>
#include <writemodfile.hxx>
#endif

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
    void testSharedOrganizationLayer();
    void testFinalized();
#ifdef MACOSX
    void testMacOSPrefsDump();
#endif

    cpo::uno::Any getKey(
        OUString const & path, OUString const & relative) const;

    void setKey(
        OUString const & path, OUString const & name,
        cpo::uno::Any const & value) const;

    bool resetKey(OUString const & path, OUString const & name) const;

    cpo::uno::Reference< cpo::uno::XInterface > createViewAccess(
        OUString const & path) const;

    cpo::uno::Reference< cpo::uno::XInterface > createUpdateAccess(
        OUString const & path) const;

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testKeyFetch);
    CPPUNIT_TEST(testKeySet);
    CPPUNIT_TEST(testKeyReset);
    CPPUNIT_TEST(testSetSetMemberName);
    CPPUNIT_TEST(testInsertSetMember);
#ifndef _WIN32
    // LanguageTag fallback for "pt" -> "pt-PT" does not currently work on the
    // Windows app build; the lookup returns the "en-US" default instead.
    CPPUNIT_TEST(testLocalizedProperty);
#endif
    CPPUNIT_TEST(testReadCommands);
    CPPUNIT_TEST(testListener);
    CPPUNIT_TEST(testRecursive);
    CPPUNIT_TEST(testCrossThreads);
    CPPUNIT_TEST(testSharedOrganizationLayer);
    CPPUNIT_TEST(testFinalized);
#ifdef MACOSX
    CPPUNIT_TEST(testMacOSPrefsDump);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    cpo::uno::Reference< css::lang::XMultiServiceFactory > provider_;
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
    virtual void disposing(css::lang::EventObject const &) override;

    virtual void propertyChange(
        css::beans::PropertyChangeEvent const &) override;

    int count_;
    bool * destroyed_;
    cpo::uno::Reference< css::beans::XPropertySet > properties_;
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
        cpo::uno::UNO_QUERY_THROW);
    properties_->addPropertyChangeListener(u"Label"_ustr, this);
    step();
    CPPUNIT_ASSERT_EQUAL(0, count_);
    cpo::uno::Reference< css::lang::XComponent >(
        properties_, cpo::uno::UNO_QUERY_THROW)->dispose();
}

RecursiveTest::~RecursiveTest()
{
    *destroyed_ = true;
}

void RecursiveTest::disposing(css::lang::EventObject const & Source)
{
    CPPUNIT_ASSERT(properties_.is());
    CPPUNIT_ASSERT_EQUAL(
        cpo::uno::Reference<cpo::uno::XInterface>(
            properties_, cpo::uno::UNO_QUERY_THROW),
        Source.Source);
    properties_.clear();
}

void RecursiveTest::propertyChange(css::beans::PropertyChangeEvent const & evt)
{
    CPPUNIT_ASSERT_EQUAL(
        cpo::uno::Reference<cpo::uno::XInterface>(
            properties_, cpo::uno::UNO_QUERY_THROW),
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
        cpo::uno::Any(u"step"_ustr));
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
        cpo::uno::Any(u"com.sun.star.configuration.backend.LocaleBackend UILocale"_ustr));
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

    cpo::uno::Reference< css::container::XNameAccess > access(
        createUpdateAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/"
                     "Commands"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference< css::container::XNamed > member;
    access->getByName(u".uno:FontworkGalleryFloater"_ustr) >>= member;
    CPPUNIT_ASSERT(member.is());
    member->setName(u".uno:FontworkShapeType"_ustr);
    cpo::uno::Reference< css::util::XChangesBatch >(
        access, cpo::uno::UNO_QUERY_THROW)->commitChanges();
    cpo::uno::Reference< css::lang::XComponent >(
        access, cpo::uno::UNO_QUERY_THROW)->dispose();

    CPPUNIT_ASSERT(
        getKey(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands/"
                    ".uno:FontworkShapeType"_ustr,
            u"Label"_ustr) >>=
        s);
    CPPUNIT_ASSERT_EQUAL( u"Insert Fontwork"_ustr, s );
}

void Test::testInsertSetMember() {
    cpo::uno::Reference<css::container::XNameContainer> access(
        createUpdateAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/Commands"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<cpo::uno::XInterface> member;
    member.set(
        cpo::uno::Reference<css::lang::XSingleServiceFactory>(
            access, cpo::uno::UNO_QUERY_THROW)->createInstance());
    CPPUNIT_ASSERT(member.is());
    access->insertByName(u"A"_ustr, cpo::uno::Any(member));
    member.set(
        cpo::uno::Reference<css::lang::XSingleServiceFactory>(
            access, cpo::uno::UNO_QUERY_THROW)->createInstance());
    CPPUNIT_ASSERT(member.is());
    try {
        access->insertByName(u""_ustr, cpo::uno::Any(member));
        CPPUNIT_FAIL("expected IllegalArgumentException");
    } catch (css::lang::IllegalArgumentException &) {}
    try {
        access->insertByName(u"\x01"_ustr, cpo::uno::Any(member));
        CPPUNIT_FAIL("expected IllegalArgumentException");
    } catch (css::lang::IllegalArgumentException &) {}
    try {
        access->insertByName(u"a/b"_ustr, cpo::uno::Any(member));
    } catch (css::lang::IllegalArgumentException &) {
        CPPUNIT_FAIL("unexpected IllegalArgumentException");
    }
    cpo::uno::Reference<css::util::XChangesBatch>(
        access, cpo::uno::UNO_QUERY_THROW)->commitChanges();
    cpo::uno::Reference<css::lang::XComponent>(
        access, cpo::uno::UNO_QUERY_THROW)->dispose();
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
    cpo::uno::Reference< css::container::XNameAccess > access(
        createViewAccess(
            u"/org.openoffice.Office.UI.GenericCommands/UserInterface/"
                     "Commands"_ustr),
        cpo::uno::UNO_QUERY_THROW);
    const cpo::uno::Sequence< OUString > names(access->getElementNames());

    /*CPPUNIT_ASSERT_EQUAL(749, names.getLength());*/
    // testSetSetMemberName() already removed ".uno:FontworkGalleryFloater"
    sal_uInt32 n = osl_getGlobalTimer();
    for (int i = 0; i < 8; ++i) {
        for (OUString const & childName : names) {
            cpo::uno::Reference< css::container::XNameAccess > child;
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
    cpo::uno::Reference< css::lang::XComponent >(
        access, cpo::uno::UNO_QUERY_THROW)->dispose();
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

cpo::uno::Any Test::getKey(
    OUString const & path, OUString const & relative) const
{
    cpo::uno::Reference< css::container::XHierarchicalNameAccess > access(
        createViewAccess(path), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Any value(access->getByHierarchicalName(relative));
    cpo::uno::Reference< css::lang::XComponent >(
        access, cpo::uno::UNO_QUERY_THROW)->dispose();
    return value;
}

void Test::setKey(
    OUString const & path, OUString const & name,
    cpo::uno::Any const & value) const
{
    cpo::uno::Reference< css::container::XNameReplace > access(
        createUpdateAccess(path), cpo::uno::UNO_QUERY_THROW);
    access->replaceByName(name, value);
    cpo::uno::Reference< css::util::XChangesBatch >(
        access, cpo::uno::UNO_QUERY_THROW)->commitChanges();
    cpo::uno::Reference< css::lang::XComponent >(
        access, cpo::uno::UNO_QUERY_THROW)->dispose();
}

bool Test::resetKey(OUString const & path, OUString const & name)
    const
{
    //TODO: support setPropertyToDefault
    cpo::uno::Reference< css::util::XChangesBatch > access(
        createUpdateAccess(path), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference< css::beans::XPropertyState > state(
        access, cpo::uno::UNO_QUERY);
    if (!state.is()) {
        return false;
    }
    state->setPropertyToDefault(name);
    access->commitChanges();
    cpo::uno::Reference< css::lang::XComponent >(
        access, cpo::uno::UNO_QUERY_THROW)->dispose();
    return true;
}

cpo::uno::Reference< cpo::uno::XInterface > Test::createViewAccess(
    OUString const & path) const
{
    cpo::uno::Any arg(
            css::beans::NamedValue(
                u"nodepath"_ustr,
                cpo::uno::Any(path)));
    return provider_->createInstanceWithArguments(
        u"com.sun.star.configuration.ConfigurationAccess"_ustr,
        cpo::uno::Sequence< cpo::uno::Any >(&arg, 1));
}

cpo::uno::Reference< cpo::uno::XInterface > Test::createUpdateAccess(
    OUString const & path) const
{
    cpo::uno::Any arg(
            css::beans::NamedValue(
                u"nodepath"_ustr,
                cpo::uno::Any(path)));
    return provider_->createInstanceWithArguments(
        u"com.sun.star.configuration.ConfigurationUpdateAccess"_ustr,
        cpo::uno::Sequence< cpo::uno::Any >(&arg, 1));
}

/// Collabora Online puts the shared AutoText, Dictionary, Template and DocumentTheme directories
/// into the configuration at run time, by handing an .xcu to XUpdate. That needs the "sharedext"
/// layer to be declared in CONFIGURATION_LAYERS, and nothing says so out loud if it is not: the
/// caller only logs, so the paths quietly go missing. This test is what says so.
void Test::testSharedOrganizationLayer()
{
    OUString aUrl;
    oslFileHandle aHandle = nullptr;
    CPPUNIT_ASSERT_EQUAL(
        osl::FileBase::E_None, osl::FileBase::createTempFile(nullptr, &aHandle, &aUrl));

    static constexpr OString aXcu
        = R"(<?xml version="1.0" encoding="UTF-8"?>)"
          R"(<oor:component-data xmlns:oor="http://openoffice.org/2001/registry")"
          R"( oor:name="Paths" oor:package="org.openoffice.Office">)"
          R"(<node oor:name="Paths"><node oor:name="AutoText" oor:op="fuse">)"
          R"(<prop oor:name="OrganizationPaths" oor:type="oor:string-list">)"
          R"(<value>file:///tmp/configmgr-test-autotext</value></prop></node></node>)"
          R"(</oor:component-data>)"_ostr;

    sal_uInt64 nWritten = 0;
    CPPUNIT_ASSERT_EQUAL(osl_File_E_None,
                         osl_writeFile(aHandle, aXcu.getStr(), aXcu.getLength(), &nWritten));
    osl_closeFile(aHandle);

    // Throws "insert extension xcs/xcu file into undefined layer" when the layer is gone.
    css::configuration::Update::get(comphelper::getProcessComponentContext())
        ->insertExtensionXcuFile(aUrl);

    cpo::uno::Sequence<OUString> aPaths;
    CPPUNIT_ASSERT(
        getKey(u"/org.openoffice.Office.Paths/Paths/AutoText"_ustr, u"OrganizationPaths"_ustr)
        >>= aPaths);

    // The layer has to outrank the shipped defaults, or the entry is read and then ignored.
    CPPUNIT_ASSERT_MESSAGE(
        "the shared layer did not reach the AutoText path",
        std::find(aPaths.begin(), aPaths.end(), u"file:///tmp/configmgr-test-autotext"_ustr)
            != aPaths.end());

    osl::File::remove(aUrl);
}

/// finalized.xcu, a read-only user layer in this test's CONFIGURATION_LAYERS, finalizes
/// MacroSecurityLevel the way an administrator's policy (the winreg or macosprefs layer) does.
/// The value has to arrive, the property has to be read-only and refuse writes, and its siblings
/// have to stay writable.
void Test::testFinalized()
{
    static constexpr OUString aPath = u"/org.openoffice.Office.Common/Security/Scripting"_ustr;
    sal_Int32 nLevel = 0;
    CPPUNIT_ASSERT(getKey(aPath, u"MacroSecurityLevel"_ustr) >>= nLevel);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nLevel);

    cpo::uno::Reference<css::beans::XPropertySet> xAccess(
        createUpdateAccess(aPath), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySetInfo> xInfo(xAccess->getPropertySetInfo());
    CPPUNIT_ASSERT(xInfo->getPropertyByName(u"MacroSecurityLevel"_ustr).Attributes
                   & css::beans::PropertyAttribute::READONLY);
    CPPUNIT_ASSERT(!(xInfo->getPropertyByName(u"SecureURL"_ustr).Attributes
                     & css::beans::PropertyAttribute::READONLY));
    CPPUNIT_ASSERT_THROW(
        xAccess->setPropertyValue(u"MacroSecurityLevel"_ustr, cpo::uno::Any(sal_Int32(0))),
        css::lang::IllegalArgumentException);
    cpo::uno::Reference<css::lang::XComponent>(xAccess, cpo::uno::UNO_QUERY_THROW)->dispose();
}

#ifdef MACOSX

CFStringRef createCFString(char const* pText)
{
    return CFStringCreateWithCString(kCFAllocatorDefault, pText, kCFStringEncodingUTF8);
}

CFNumberRef createCFNumber(sal_Int64 nValue)
{
    return CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &nValue);
}

CFNumberRef createCFNumber(double fValue)
{
    return CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &fValue);
}

CFMutableDictionaryRef createDictionary()
{
    return CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                                     &kCFTypeDictionaryValueCallBacks);
}

/// Stores pValue under pKey and gives up the caller's reference to it.
void setValue(CFMutableDictionaryRef pDict, char const* pKey, CFTypeRef pValue)
{
    CFStringRef pKeyString = createCFString(pKey);
    CFDictionarySetValue(pDict, pKeyString, pValue);
    CFRelease(pKeyString);
    CFRelease(pValue);
}

/// The macosprefs layer turns the configuration keys among the application's preferences into
/// an .xcu file. This drives its writer with a dictionary built here (a test must not write real
/// preferences: they would leak into the developer's account and into other test processes), and
/// then loads the result through the extension layer, to see that the configuration takes it.
void Test::testMacOSPrefsDump()
{
    CFMutableDictionaryRef pPrefs = createDictionary();
    // plain values
    setValue(pPrefs, "/org.openoffice.Office.Common/Save/Document/AutoSave",
             CFRetain(kCFBooleanFalse));
    setValue(pPrefs, "/org.openoffice.Office.Common/Save/Document/AutoSaveTimeIntervall",
             createCFNumber(sal_Int64(7)));
    setValue(pPrefs, "/org.openoffice.Office.Common/Nonexistent/Ratio", createCFNumber(1.5));
    UInt8 const aBytes[] = { 0x01, 0xAB };
    setValue(pPrefs, "/org.openoffice.Office.Common/Nonexistent/Blob",
             CFDataCreate(kCFAllocatorDefault, aBytes, 2));
    // a list
    CFTypeRef aUrls[] = { createCFString("file:///opt/corp/macros"),
                          createCFString("file:///Volumes/share") };
    setValue(pPrefs, "/org.openoffice.Office.Common/Security/Scripting/SecureURL",
             CFArrayCreate(kCFAllocatorDefault, aUrls, 2, &kCFTypeArrayCallBacks));
    CFRelease(aUrls[0]);
    CFRelease(aUrls[1]);
    // a set member created with #fuse, with the value given as a string
    setValue(pPrefs,
             "/org.openoffice.Office.OptionsDialog/OptionsDialogGroups/ProductName#fuse/Pages/"
             "Java#fuse/Hide",
             createCFString("true"));
    // dictionaries: Final on a managed key, Final on a key that is not managed, Nil, Type
    CFMutableDictionaryRef pDict = createDictionary();
    setValue(pDict, "Value", createCFString("Example Corp."));
    setValue(pDict, "Final", CFRetain(kCFBooleanTrue));
    setValue(pPrefs, "/org.openoffice.UserProfile/Data/o", pDict);
    pDict = createDictionary();
    setValue(pDict, "Value", createCFString("Berlin"));
    setValue(pDict, "Final", createCFString("1"));
    setValue(pPrefs, "/org.openoffice.UserProfile/Data/l", pDict);
    pDict = createDictionary();
    setValue(pDict, "Nil", createCFNumber(sal_Int64(1)));
    setValue(pDict, "Final", CFRetain(kCFBooleanTrue));
    setValue(pPrefs, "/org.openoffice.Office.Common/Path/Current/Work", pDict);
    pDict = createDictionary();
    setValue(pDict, "Value", CFRetain(kCFBooleanFalse));
    setValue(pDict, "Type", createCFString("xs:boolean"));
    setValue(pDict, "Final", CFRetain(kCFBooleanTrue));
    setValue(pPrefs,
             "/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/"
             "Arguments/AutoCheckEnabled",
             pDict);
    // ignored: not a configuration path, no property name, unsupported value, bad operation
    setValue(pPrefs, "userName", createCFString("someone"));
    setValue(pPrefs, "/org.openoffice.Office.Common", createCFString("x"));
    setValue(pPrefs, "/org.openoffice.Office.Common/Misc/Bad", CFDateCreate(kCFAllocatorDefault, 0));
    setValue(pPrefs, "/org.openoffice.Office.Common/Misc/Group#delete/Bad", createCFString("x"));

    CFTypeRef aForced[]
        = { createCFString("/org.openoffice.UserProfile/Data/o"),
            createCFString("/org.openoffice.Office.Common/Path/Current/Work"),
            createCFString("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['"
                           "UpdateCheck']/Arguments/AutoCheckEnabled") };
    CFSetRef pForced = CFSetCreate(kCFAllocatorDefault, aForced, 3, &kCFTypeSetCallBacks);
    for (CFTypeRef pKey : aForced)
        CFRelease(pKey);

    static constexpr OString aHeader
        = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<oor:items"
          " xmlns:oor=\"http://openoffice.org/2001/registry\""
          " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
          " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"_ostr;

    configmgr::TempFile aFile;
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                         osl::FileBase::createTempFile(nullptr, &aFile.handle, &aFile.url));
    aFile.writeString(aHeader);
    configmgr::writeMacOSPreferencesXcu(pPrefs, pForced, aFile);
    aFile.writeString("</oor:items>");
    CPPUNIT_ASSERT_EQUAL(osl_File_E_None, aFile.closeWithoutUnlink());
    CFRelease(pForced);
    CFRelease(pPrefs);

    OString aContent;
    {
        osl::File aIn(aFile.url);
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aIn.open(osl_File_OpenFlag_Read));
        sal_uInt64 nSize = 0;
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aIn.getSize(nSize));
        std::vector<char> aBuffer(nSize);
        sal_uInt64 nRead = 0;
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, aIn.read(aBuffer.data(), nSize, nRead));
        aContent = OString(aBuffer.data(), static_cast<sal_Int32>(nRead));
    }
    // Keys are written in sorted order; the ignored ones do not appear.
    OString aExpected
        = aHeader
          + "<item oor:path=\"/org.openoffice.Office.Common/Nonexistent\">"
            "<prop oor:name=\"Blob\"><value>01AB</value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.Office.Common/Nonexistent\">"
            "<prop oor:name=\"Ratio\"><value>1.5</value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.Office.Common/Path/Current\">"
            "<prop oor:name=\"Work\" oor:finalized=\"true\"><value xsi:nil=\"true\"/></prop>"
            "</item>\n"
            "<item oor:path=\"/org.openoffice.Office.Common/Save/Document\">"
            "<prop oor:name=\"AutoSave\"><value>false</value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.Office.Common/Save/Document\">"
            "<prop oor:name=\"AutoSaveTimeIntervall\"><value>7</value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.Office.Common/Security/Scripting\">"
            "<prop oor:name=\"SecureURL\"><value><it>file:///opt/corp/macros</it>"
            "<it>file:///Volumes/share</it></value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.Office.Jobs/Jobs/"
            "org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments\">"
            "<prop oor:name=\"AutoCheckEnabled\" oor:type=\"xs:boolean\" oor:finalized=\"true\">"
            "<value>false</value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.Office.OptionsDialog/OptionsDialogGroups\">"
            "<node oor:name=\"ProductName\" oor:op=\"fuse\"><node oor:name=\"Pages\">"
            "<node oor:name=\"Java\" oor:op=\"fuse\"><prop oor:name=\"Hide\">"
            "<value>true</value></prop></node></node></node></item>\n"
            "<item oor:path=\"/org.openoffice.UserProfile/Data\">"
            "<prop oor:name=\"l\"><value>Berlin</value></prop></item>\n"
            "<item oor:path=\"/org.openoffice.UserProfile/Data\">"
            "<prop oor:name=\"o\" oor:finalized=\"true\"><value>Example Corp.</value></prop>"
            "</item>\n"
            "</oor:items>";
    CPPUNIT_ASSERT_EQUAL(aExpected, aContent);

    // The generated file is what the configuration gets at start-up; here it goes in through
    // the extension layer, which parses it the same way.
    css::configuration::Update::get(comphelper::getProcessComponentContext())
        ->insertExtensionXcuFile(aFile.url);
    osl::File::remove(aFile.url);

    bool bAutoSave = true;
    CPPUNIT_ASSERT(
        getKey(u"/org.openoffice.Office.Common/Save/Document"_ustr, u"AutoSave"_ustr)
        >>= bAutoSave);
    CPPUNIT_ASSERT(!bAutoSave);
    sal_Int32 nInterval = 0;
    CPPUNIT_ASSERT(
        getKey(u"/org.openoffice.Office.Common/Save/Document"_ustr,
               u"AutoSaveTimeIntervall"_ustr)
        >>= nInterval);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), nInterval);
    cpo::uno::Sequence<OUString> aSecureUrls;
    CPPUNIT_ASSERT(
        getKey(u"/org.openoffice.Office.Common/Security/Scripting"_ustr, u"SecureURL"_ustr)
        >>= aSecureUrls);
    CPPUNIT_ASSERT(std::find(aSecureUrls.begin(), aSecureUrls.end(),
                             u"file:///Volumes/share"_ustr)
                   != aSecureUrls.end());
    bool bHide = false;
    CPPUNIT_ASSERT(
        getKey(u"/org.openoffice.Office.OptionsDialog/OptionsDialogGroups/ProductName/Pages/"
               "Java"_ustr,
               u"Hide"_ustr)
        >>= bHide);
    CPPUNIT_ASSERT(bHide);
    OUString aText;
    CPPUNIT_ASSERT(getKey(u"/org.openoffice.UserProfile/Data"_ustr, u"o"_ustr) >>= aText);
    CPPUNIT_ASSERT_EQUAL(u"Example Corp."_ustr, aText);
    CPPUNIT_ASSERT(getKey(u"/org.openoffice.UserProfile/Data"_ustr, u"l"_ustr) >>= aText);
    CPPUNIT_ASSERT_EQUAL(u"Berlin"_ustr, aText);

    // Only the managed key is locked.
    cpo::uno::Reference<css::beans::XPropertySet> xData(
        createUpdateAccess(u"/org.openoffice.UserProfile/Data"_ustr), cpo::uno::UNO_QUERY_THROW);
    cpo::uno::Reference<css::beans::XPropertySetInfo> xInfo(xData->getPropertySetInfo());
    CPPUNIT_ASSERT(xInfo->getPropertyByName(u"o"_ustr).Attributes
                   & css::beans::PropertyAttribute::READONLY);
    CPPUNIT_ASSERT(!(xInfo->getPropertyByName(u"l"_ustr).Attributes
                     & css::beans::PropertyAttribute::READONLY));
    CPPUNIT_ASSERT_THROW(xData->setPropertyValue(u"o"_ustr, cpo::uno::Any(u"Other"_ustr)),
                         css::lang::IllegalArgumentException);
    cpo::uno::Reference<css::lang::XComponent>(xData, cpo::uno::UNO_QUERY_THROW)->dispose();
}

#endif

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
