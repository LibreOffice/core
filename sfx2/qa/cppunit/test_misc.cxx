/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <memory>

#ifdef LINUX
#include <fstream>
#include <sstream>
#include <string>
#include <limits.h>
#endif

#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/DocumentProperties.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <test/unoapi_test.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <osl/file.hxx>

#include <libxml/xpathInternals.h>

using namespace ::com::sun::star;

namespace {

class MiscTest
    : public UnoApiTest
{
public:
    MiscTest()
        : UnoApiTest(u"/sfx2/qa/cppunit/data/"_ustr)
    {
    }

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override
    {
        // ODF
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("office"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("meta"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
        // used in testCustomMetadata
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("foo"), BAD_CAST("http://foo.net"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("baz"), BAD_CAST("http://baz.net"));
    }
};

class Listener : public ::cppu::WeakImplHelper<css::util::XModifyListener>
{
public:
    Listener()
        : bCalled(false)
    {
    }

    bool reset()
    {
        bool bOld = bCalled;
        bCalled = false;
        return bOld;
    }

private:
    bool bCalled;

    virtual void SAL_CALL disposing(const lang::EventObject&) noexcept override {}
    virtual void SAL_CALL modified(const lang::EventObject&) noexcept override { bCalled = true; }
};

CPPUNIT_TEST_FIXTURE(MiscTest, testODFCustomMetadata)
{
    uno::Reference<document::XDocumentProperties> const xProps(
        ::com::sun::star::document::DocumentProperties::create(m_xContext));

    OUString const url(m_directories.getURLFromSrc(u"/sfx2/qa/complex/sfx2/testdocuments/CUSTOM.odt"));
    xProps->loadFromMedium(url, uno::Sequence<beans::PropertyValue>());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xProps->getAuthor());
    uno::Sequence<beans::PropertyValue> mimeArgs({
        beans::PropertyValue(u"MediaType"_ustr, -1, uno::Any(u"application/vnd.oasis.opendocument.text"_ustr), beans::PropertyState_DIRECT_VALUE)
        });
    xProps->storeToMedium(maTempFile.GetURL(), mimeArgs);

    // check that custom metadata is preserved
    xmlDocUniquePtr pXmlDoc = parseExport(u"meta.xml"_ustr);
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/bork", u"bork");
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar", 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo", 1);
    assertXPath(pXmlDoc, "/office:document-meta/office:meta/foo:bar/baz:foo[@baz:bar='foo']");
    assertXPathContent(pXmlDoc, "/office:document-meta/office:meta/foo:bar/foo:baz", u"bar");
}

/**
 * Test case for the service com.sun.star.document.DocumentProperties.
 * Currently, this service is implemented in
 * sfx2/source/doc/SfxDocumentMetaData.cxx.
 *
 */
CPPUNIT_TEST_FIXTURE(MiscTest, testDocumentProperties)
{
    uno::Reference<document::XDocumentProperties> const xProps(
        ::com::sun::star::document::DocumentProperties::create(m_xContext));

    OUString const url(
        m_directories.getURLFromSrc(u"/sfx2/qa/complex/sfx2/testdocuments/TEST.odt"));
    xProps->loadFromMedium(url, uno::Sequence<beans::PropertyValue>());
    CPPUNIT_ASSERT_EQUAL(u"Karl-Heinz Mustermann"_ustr, xProps->getAuthor());
    CPPUNIT_ASSERT_EQUAL(u"StarOffice/8$Solaris_x86 OpenOffice.org_project/680m232$Build-9227"_ustr,
                         xProps->getGenerator());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2007), xProps->getCreationDate().Year);
    CPPUNIT_ASSERT_EQUAL(u"Urgent Memo"_ustr, xProps->getTitle());
    CPPUNIT_ASSERT_EQUAL(u"Wichtige Mitteilung"_ustr, xProps->getSubject());
    CPPUNIT_ASSERT_EQUAL(u"Modern internal company memorandum in full-blocked style"_ustr,
                         xProps->getDescription());
    CPPUNIT_ASSERT_EQUAL(u"Karl-Heinz Mustermann"_ustr, xProps->getModifiedBy());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(10), xProps->getModificationDate().Month);
    CPPUNIT_ASSERT_EQUAL(u"Karl-Heinz Mustermann"_ustr, xProps->getPrintedBy());
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(29), xProps->getPrintDate().Day);
    CPPUNIT_ASSERT_EQUAL(u"Modern Memo"_ustr, xProps->getTemplateName());
    CPPUNIT_ASSERT(xProps->getTemplateURL().endsWith("memmodern.ott"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(17), xProps->getTemplateDate().Hours);
    CPPUNIT_ASSERT(xProps->getAutoloadURL().endsWith("/TEST.odt"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xProps->getAutoloadSecs());
    CPPUNIT_ASSERT_EQUAL(u"_blank"_ustr, xProps->getDefaultTarget());
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3), xProps->getEditingCycles());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(320), xProps->getEditingDuration());

    uno::Sequence<OUString> aKeywords(xProps->getKeywords());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(u"Memo"_ustr, aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(u"Asien"_ustr, aKeywords[1]);
    CPPUNIT_ASSERT_EQUAL(u"Reis"_ustr, aKeywords[2]);

    uno::Sequence<beans::NamedValue> aDocStats = xProps->getDocumentStatistics();
    auto it = std::find_if(std::cbegin(aDocStats), std::cend(aDocStats),
                           [](const css::beans::NamedValue& val) {
                               return val.Name == "WordCount" && val.Value.get<sal_uInt32>() == 23;
                           });
    CPPUNIT_ASSERT(it != std::cend(aDocStats));

    it = std::find_if(std::cbegin(aDocStats), std::cend(aDocStats),
                      [](const css::beans::NamedValue& val) {
                          return val.Name == "PageCount" && val.Value.get<sal_uInt32>() == 1;
                      });
    CPPUNIT_ASSERT(it != std::cend(aDocStats));

    uno::Reference<beans::XPropertyContainer> xUDP = xProps->getUserDefinedProperties();
    uno::Reference<beans::XPropertySet> xPropertySet(xUDP, uno::UNO_QUERY);
    uno::Any aAny = xPropertySet->getPropertyValue(u"Hinweis"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Dies ist ein wichtiger Hinweis"_ustr, aAny.get<OUString>());

    aAny = xPropertySet->getPropertyValue(u"Warnung"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Kann Spuren von N\u00FCssen enthalten"_ustr, aAny.get<OUString>());

    xProps->setAuthor(u"me"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"me"_ustr, xProps->getAuthor());

    xProps->setGenerator(u"the computa"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"the computa"_ustr, xProps->getGenerator());

    css::util::DateTime aDateTime;
    aDateTime.Day = 1;
    aDateTime.Month = 1;
    aDateTime.Year = 2038;
    xProps->setCreationDate(aDateTime);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2038), xProps->getCreationDate().Year);

    xProps->setTitle(u"El t'itulo"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"El t'itulo"_ustr, xProps->getTitle());

    xProps->setSubject(u"Ein verkommenes Subjekt"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Ein verkommenes Subjekt"_ustr, xProps->getSubject());

    xProps->setDescription(u"Este descripci'on no es importante"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Este descripci'on no es importante"_ustr, xProps->getDescription());

    lang::Locale aLang;
    aLang.Language = u"en"_ustr;
    aLang.Country = u"GB"_ustr;

    xProps->setLanguage(aLang);
    CPPUNIT_ASSERT_EQUAL(aLang.Language, xProps->getLanguage().Language);
    CPPUNIT_ASSERT_EQUAL(aLang.Country, xProps->getLanguage().Country);

    xProps->setModifiedBy(u"myself"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"myself"_ustr, xProps->getModifiedBy());

    aDateTime.Year = 2042;
    xProps->setModificationDate(aDateTime);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2042), xProps->getModificationDate().Year);

    xProps->setPrintedBy(u"i did not do it"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"i did not do it"_ustr, xProps->getPrintedBy());

    aDateTime.Year = 2024;
    xProps->setPrintDate(aDateTime);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2024), xProps->getPrintDate().Year);

    xProps->setTemplateName(u"blah"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"blah"_ustr, xProps->getTemplateName());

    xProps->setTemplateURL(u"gopher://some-hole-in-the-ground/"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"gopher://some-hole-in-the-ground/"_ustr, xProps->getTemplateURL());

    aDateTime.Year = 2043;
    xProps->setTemplateDate(aDateTime);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2043), xProps->getTemplateDate().Year);

    xProps->setAutoloadURL(u"http://nowhere/"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"http://nowhere/"_ustr, xProps->getAutoloadURL());

    xProps->setAutoloadSecs(3661);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3661), xProps->getAutoloadSecs());

    xProps->setDefaultTarget(u"_blank"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"_blank"_ustr, xProps->getDefaultTarget());

    xProps->setEditingCycles(42);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(42), xProps->getEditingCycles());

    xProps->setEditingDuration(84);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(84), xProps->getEditingDuration());

    uno::Sequence<OUString> aKeywords2{ u"keywordly"_ustr, u"keywordlike"_ustr,
                                        u"keywordalicious"_ustr };
    xProps->setKeywords(aKeywords2);
    aKeywords = xProps->getKeywords();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aKeywords.getLength());
    CPPUNIT_ASSERT_EQUAL(u"keywordly"_ustr, aKeywords[0]);
    CPPUNIT_ASSERT_EQUAL(u"keywordlike"_ustr, aKeywords[1]);
    CPPUNIT_ASSERT_EQUAL(u"keywordalicious"_ustr, aKeywords[2]);

    uno::Sequence<beans::NamedValue> aDocStats2{ { u"SyllableCount"_ustr, uno::Any(sal_Int16(9)) },
                                                 { u"FrameCount"_ustr, uno::Any(sal_Int16(2)) },
                                                 { u"SentenceCount"_ustr,
                                                   uno::Any(sal_Int16(7)) } };

    xProps->setDocumentStatistics(aDocStats2);
    aDocStats = xProps->getDocumentStatistics();

    it = std::find_if(std::cbegin(aDocStats), std::cend(aDocStats),
                      [](const css::beans::NamedValue& val) {
                          return val.Name == "SyllableCount" && val.Value.get<sal_uInt32>() == 9;
                      });
    CPPUNIT_ASSERT(it != std::cend(aDocStats));

    it = std::find_if(std::cbegin(aDocStats), std::cend(aDocStats),
                      [](const css::beans::NamedValue& val) {
                          return val.Name == "FrameCount" && val.Value.get<sal_uInt32>() == 2;
                      });
    CPPUNIT_ASSERT(it != std::cend(aDocStats));

    it = std::find_if(std::cbegin(aDocStats), std::cend(aDocStats),
                      [](const css::beans::NamedValue& val) {
                          return val.Name == "SentenceCount" && val.Value.get<sal_uInt32>() == 7;
                      });
    CPPUNIT_ASSERT(it != std::cend(aDocStats));

    // actually, this tests the PropertyBag service
    // but maybe the DocumentProperties service will be implemented
    // differently some day...

    // note that Time is only supported for backward compatibility!
    css::util::Time aTime;
    aTime.Hours = 1;
    aTime.Minutes = 16;

    css::util::Date aDate;
    aDate.Year = 2071;
    aDate.Month = 2;
    aDate.Day = 3;

    aDateTime.Year = 2065;

    css::util::Duration aDur;
    aDur.Negative = true;
    aDur.Years = 1001;
    aDur.Months = 999;
    aDur.Days = 888;
    aDur.Hours = 777;
    aDur.Minutes = 666;
    aDur.Seconds = 555;
    aDur.NanoSeconds = 444444444;

    xUDP->addProperty("Frobnicate", beans::PropertyAttribute::REMOVABLE, uno::Any(true));
    xUDP->addProperty("FrobDuration", beans::PropertyAttribute::REMOVABLE, uno::Any(aDur));
    xUDP->addProperty("FrobDuration2", beans::PropertyAttribute::REMOVABLE, uno::Any(aTime));
    xUDP->addProperty("FrobEndDate", beans::PropertyAttribute::REMOVABLE, uno::Any(aDate));
    xUDP->addProperty("FrobStartTime", beans::PropertyAttribute::REMOVABLE, uno::Any(aDateTime));
    xUDP->addProperty("Pi", beans::PropertyAttribute::REMOVABLE, uno::Any(3.1415));
    xUDP->addProperty("Foo", beans::PropertyAttribute::REMOVABLE, uno::Any(u"bar"_ustr));
    xUDP->addProperty("Removed", beans::PropertyAttribute::REMOVABLE, uno::Any(u"bar"_ustr));
    // #i94175#: empty property name is valid ODF 1.1
    xUDP->addProperty("", beans::PropertyAttribute::REMOVABLE, uno::Any(u"eeeeek"_ustr));

    try
    {
        xUDP->removeProperty(u"Info 1"_ustr);
        xUDP->removeProperty(u"Removed"_ustr);
    }
    catch (beans::UnknownPropertyException&)
    {
        CPPUNIT_FAIL("removeProperty failed");
    }

    try
    {
        xUDP->addProperty("Forbidden", beans::PropertyAttribute::REMOVABLE,
                          uno::Any(uno::Sequence<OUString>{ u"foo"_ustr, u"bar"_ustr }));
        CPPUNIT_FAIL("inserting value of non-supported type did not fail");
    }
    catch (beans::IllegalTypeException&)
    {
    }

    aAny = xPropertySet->getPropertyValue(u"Frobnicate"_ustr);
    CPPUNIT_ASSERT_EQUAL(true, aAny.get<bool>());

    aAny = xPropertySet->getPropertyValue(u"FrobDuration"_ustr);
    css::util::Duration aDur2 = aAny.get<css::util::Duration>();
    CPPUNIT_ASSERT_EQUAL(aDur.Negative, aDur2.Negative);
    CPPUNIT_ASSERT_EQUAL(aDur.Years, aDur2.Years);
    CPPUNIT_ASSERT_EQUAL(aDur.Months, aDur2.Months);
    CPPUNIT_ASSERT_EQUAL(aDur.Days, aDur2.Days);
    CPPUNIT_ASSERT_EQUAL(aDur.Hours, aDur2.Hours);
    CPPUNIT_ASSERT_EQUAL(aDur.Minutes, aDur2.Minutes);
    CPPUNIT_ASSERT_EQUAL(aDur.Seconds, aDur2.Seconds);
    CPPUNIT_ASSERT_EQUAL(aDur.NanoSeconds, aDur2.NanoSeconds);

    aAny = xPropertySet->getPropertyValue(u"FrobDuration2"_ustr);
    css::util::Time aTime2 = aAny.get<css::util::Time>();
    CPPUNIT_ASSERT_EQUAL(aTime.Hours, aTime2.Hours);
    CPPUNIT_ASSERT_EQUAL(aTime.Minutes, aTime2.Minutes);
    CPPUNIT_ASSERT_EQUAL(aTime.Seconds, aTime2.Seconds);
    CPPUNIT_ASSERT_EQUAL(aTime.NanoSeconds, aTime2.NanoSeconds);

    aAny = xPropertySet->getPropertyValue(u"FrobEndDate"_ustr);
    css::util::Date aDate2 = aAny.get<css::util::Date>();
    CPPUNIT_ASSERT_EQUAL(aDate.Day, aDate2.Day);
    CPPUNIT_ASSERT_EQUAL(aDate.Month, aDate2.Month);
    CPPUNIT_ASSERT_EQUAL(aDate.Year, aDate2.Year);

    aAny = xPropertySet->getPropertyValue(u"FrobStartTime"_ustr);
    css::util::DateTime aDateTime2 = aAny.get<css::util::DateTime>();
    CPPUNIT_ASSERT_EQUAL(aDateTime.Year, aDateTime2.Year);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Month, aDateTime2.Month);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Day, aDateTime2.Day);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Hours, aDateTime2.Hours);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Minutes, aDateTime2.Minutes);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Seconds, aDateTime2.Seconds);
    CPPUNIT_ASSERT_EQUAL(aDateTime.NanoSeconds, aDateTime2.NanoSeconds);

    aAny = xPropertySet->getPropertyValue(u"Pi"_ustr);
    double aPi = aAny.get<double>();
    CPPUNIT_ASSERT_EQUAL(3.1415, aPi);

    aAny = xPropertySet->getPropertyValue(u"Foo"_ustr);
    OUString aFoo = aAny.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, aFoo);

    aAny = xPropertySet->getPropertyValue(u""_ustr);
    OUString aEmpty = aAny.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"eeeeek"_ustr, aEmpty);

    try
    {
        xPropertySet->getPropertyValue(u"Removed"_ustr);
        CPPUNIT_FAIL("UserDefined remove didn't");
    }
    catch (beans::UnknownPropertyException&)
    {
    }

    uno::Sequence<beans::PropertyValue> mimeArgs({ beans::PropertyValue(
        u"MediaType"_ustr, -1, uno::Any(u"application/vnd.oasis.opendocument.text"_ustr),
        beans::PropertyState_DIRECT_VALUE) });
    xProps->storeToMedium(maTempFile.GetURL(), mimeArgs);

    xProps->loadFromMedium(maTempFile.GetURL(), uno::Sequence<beans::PropertyValue>());

    xUDP = xProps->getUserDefinedProperties();
    xPropertySet.set(xUDP, uno::UNO_QUERY);

    aAny = xPropertySet->getPropertyValue(u"Frobnicate"_ustr);
    CPPUNIT_ASSERT_EQUAL(true, aAny.get<bool>());

    aAny = xPropertySet->getPropertyValue(u"FrobDuration"_ustr);
    aDur2 = aAny.get<css::util::Duration>();
    CPPUNIT_ASSERT_EQUAL(aDur.Negative, aDur2.Negative);
    CPPUNIT_ASSERT_EQUAL(aDur.Years, aDur2.Years);
    CPPUNIT_ASSERT_EQUAL(aDur.Months, aDur2.Months);
    CPPUNIT_ASSERT_EQUAL(aDur.Days, aDur2.Days);
    CPPUNIT_ASSERT_EQUAL(aDur.Hours, aDur2.Hours);
    CPPUNIT_ASSERT_EQUAL(aDur.Minutes, aDur2.Minutes);
    CPPUNIT_ASSERT_EQUAL(aDur.Seconds, aDur2.Seconds);
    CPPUNIT_ASSERT_EQUAL(aDur.NanoSeconds, aDur2.NanoSeconds);

    css::util::Duration aDur3(false, 0, 0, 0, aTime.Hours, aTime.Minutes, aTime.Seconds,
                              aTime.NanoSeconds);
    // this is now a Duration!
    aAny = xPropertySet->getPropertyValue(u"FrobDuration2"_ustr);
    aDur2 = aAny.get<css::util::Duration>();
    CPPUNIT_ASSERT_EQUAL(aDur3.Negative, aDur2.Negative);
    CPPUNIT_ASSERT_EQUAL(aDur3.Years, aDur2.Years);
    CPPUNIT_ASSERT_EQUAL(aDur3.Months, aDur2.Months);
    CPPUNIT_ASSERT_EQUAL(aDur3.Days, aDur2.Days);
    CPPUNIT_ASSERT_EQUAL(aDur3.Hours, aDur2.Hours);
    CPPUNIT_ASSERT_EQUAL(aDur3.Minutes, aDur2.Minutes);
    CPPUNIT_ASSERT_EQUAL(aDur3.Seconds, aDur2.Seconds);
    CPPUNIT_ASSERT_EQUAL(aDur3.NanoSeconds, aDur2.NanoSeconds);

    aAny = xPropertySet->getPropertyValue(u"FrobEndDate"_ustr);
    aDate2 = aAny.get<css::util::Date>();
    CPPUNIT_ASSERT_EQUAL(aDate.Day, aDate2.Day);
    CPPUNIT_ASSERT_EQUAL(aDate.Month, aDate2.Month);
    CPPUNIT_ASSERT_EQUAL(aDate.Year, aDate2.Year);

    aAny = xPropertySet->getPropertyValue(u"FrobStartTime"_ustr);
    aDateTime2 = aAny.get<css::util::DateTime>();
    CPPUNIT_ASSERT_EQUAL(aDateTime.Year, aDateTime2.Year);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Month, aDateTime2.Month);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Day, aDateTime2.Day);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Hours, aDateTime2.Hours);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Minutes, aDateTime2.Minutes);
    CPPUNIT_ASSERT_EQUAL(aDateTime.Seconds, aDateTime2.Seconds);
    CPPUNIT_ASSERT_EQUAL(aDateTime.NanoSeconds, aDateTime2.NanoSeconds);

    aAny = xPropertySet->getPropertyValue(u"Pi"_ustr);
    aPi = aAny.get<double>();
    CPPUNIT_ASSERT_EQUAL(3.1415, aPi);

    aAny = xPropertySet->getPropertyValue(u"Foo"_ustr);
    aFoo = aAny.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"bar"_ustr, aFoo);

    aAny = xPropertySet->getPropertyValue(u""_ustr);
    aEmpty = aAny.get<OUString>();
    CPPUNIT_ASSERT_EQUAL(u"eeeeek"_ustr, aEmpty);

    try
    {
        xPropertySet->getPropertyValue(u"Removed"_ustr);
        CPPUNIT_FAIL("UserDefined remove didn't");
    }
    catch (beans::UnknownPropertyException&)
    {
    }

    rtl::Reference<Listener> xListener = new Listener();
    css::uno::Reference<css::util::XModifyBroadcaster> xBroadcaster(xProps, css::uno::UNO_QUERY);
    xBroadcaster->addModifyListener(xListener);
    xProps->setAuthor(u"not me"_ustr);
    CPPUNIT_ASSERT(xListener->reset());
    xUDP->addProperty(u"Listener"_ustr, beans::PropertyAttribute::REMOVABLE, uno::Any(u"foo"_ustr));
    CPPUNIT_ASSERT(xListener->reset());
    xPropertySet->setPropertyValue(u"Listener"_ustr, uno::Any(u"bar"_ustr));
    CPPUNIT_ASSERT(xListener->reset());
    xUDP->removeProperty(u"Listener"_ustr);
    CPPUNIT_ASSERT(xListener->reset());

    xBroadcaster->removeModifyListener(xListener);
    xUDP->addProperty(u"Listener2"_ustr, beans::PropertyAttribute::REMOVABLE,
                      uno::Any(u"foo"_ustr));
    CPPUNIT_ASSERT(!xListener->reset());
}

CPPUNIT_TEST_FIXTURE(MiscTest, testNoThumbnail)
{
    // Load a document.
    loadFromFile(u"hello.odt");

    // Save it with the NoThumbnail option and assert that it has no thumbnail.
#ifndef _WIN32
    mode_t nMask = umask(022);
#endif
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStorable.is());
    uno::Sequence<beans::PropertyValue> aProperties(
        comphelper::InitPropertySequence({ { "NoThumbnail", uno::Any(true) } }));
    osl::File::remove(maTempFile.GetURL());
    xStorable->storeToURL(maTempFile.GetURL(), aProperties);
    uno::Reference<packages::zip::XZipFileAccess2> xZipFile
        = packages::zip::ZipFileAccess::createWithURL(m_xContext, maTempFile.GetURL());
    CPPUNIT_ASSERT(!xZipFile->hasByName(u"Thumbnails/thumbnail.png"_ustr));

#ifndef _WIN32
    // Check permissions of the URL after store.
    osl::DirectoryItem aItem;
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None,
                         osl::DirectoryItem::get(maTempFile.GetURL(), aItem));

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None, aItem.getFileStatus(aStatus));

    // The following checks used to fail in the past, osl_File_Attribute_GrpRead was not set even if
    // umask requested so:
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_GrpRead);
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_OthRead);

    // Now "save as" again to trigger the "overwrite" case.
    xStorable->storeToURL(maTempFile.GetURL(), {});
    CPPUNIT_ASSERT_EQUAL(osl::DirectoryItem::E_None, aItem.getFileStatus(aStatus));
    // The following check used to fail in the past, result had temp file
    // permissions.
    CPPUNIT_ASSERT(aStatus.getAttributes() & osl_File_Attribute_GrpRead);

    umask(nMask);
#endif
}

#ifdef LINUX

std::string get_filesystem_type(const char* path)
{
    char resolved[PATH_MAX];
    if (!realpath(path, resolved))
        return "";

    std::ifstream file("/proc/self/mountinfo");
    std::string line;

    std::string bestMatch;
    std::string fsType;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;

        // Skip first fields until mount point
        for (int i = 0; i < 4; ++i)
            iss >> token;

        std::string mountPoint;
        iss >> mountPoint;

        // Find separator "-"
        while (iss >> token && token != "-")
            ;

        if (!(iss >> token))
            continue;

        std::string type = token;

        if (std::string(resolved).starts_with(mountPoint) &&
            mountPoint.length() > bestMatch.length()) {
            bestMatch = mountPoint;
            fsType = type;
        }
    }

    return fsType;
}

#endif


CPPUNIT_TEST_FIXTURE(MiscTest, testHardLinks)
{
#ifndef _WIN32
    OUString aTargetDir = m_directories.getURLFromWorkdir(u"/CppunitTest/sfx2_misc.test.user/");
    const OUString aURL(aTargetDir + "hello.odt");
    osl::File::copy(createFileURL(u"hello.odt"), aURL);
    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aURL, aTargetPath);
    OString aOld = aTargetPath.toUtf8();
    aTargetPath += ".2";
    OString aNew = aTargetPath.toUtf8();
    int nRet = link(aOld.getStr(), aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);

    loadFromURL(aURL);

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->store();

    struct stat buf;
    // coverity[fs_check_call] - this is legitimate in the context of this test
    nRet = stat(aOld.getStr(), &buf);
    CPPUNIT_ASSERT_EQUAL(0, nRet);

    // This failed: hard link count was 1, the hard link broke on store.
    bool doCheckNlink(true);

#ifdef LINUX
    // Some file system types are know to have quite non-POSIX semantics.
    std::string fsType = get_filesystem_type(aOld.getStr());
    if (fsType == "fuse.prl_fsd")
        doCheckNlink = false;
#endif

    CPPUNIT_ASSERT(!doCheckNlink || buf.st_nlink > 1);

    // Test that symlinks are preserved as well.
    nRet = remove(aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    nRet = symlink(aOld.getStr(), aNew.getStr());
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    xStorable->storeToURL(aURL + ".2", {});
    nRet = lstat(aNew.getStr(), &buf);
    CPPUNIT_ASSERT_EQUAL(0, nRet);
    // This failed, the hello.odt.2 symlink was replaced with a real file.
    CPPUNIT_ASSERT(bool(S_ISLNK(buf.st_mode)));
#endif
}

CPPUNIT_TEST_FIXTURE(MiscTest, testtestOverwriteReadOnly)
{
#ifdef UNX
    // Given a read-only, already created file:
    OUString aTargetDir = m_directories.getURLFromWorkdir(u"/CppunitTest/sfx2_misc.test.user/");
    OUString aURL(aTargetDir + "read-only.odt");
    osl::File aFile(aURL);
    aFile.open(osl_File_OpenFlag_Create);
    aFile.close();
    osl::File::setAttributes(aURL, osl_File_Attribute_OwnRead);
    loadFromURL(aURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    bool bFail = false;

    // When trying to overwrite that:
    try
    {
        xStorable->storeToURL(aURL, { comphelper::makePropertyValue("Overwrite", true) });
    }
    catch (const io::IOException&)
    {
        bFail = true;
    }

    // Then make sure we fail:
    // Without the accompanying fix in place, this test would have failed, the overwrite would
    // silently ignore the read-only attribute of the file.
    CPPUNIT_ASSERT(bFail);
#endif
}

CPPUNIT_TEST_FIXTURE(MiscTest, testOverwrite)
{
    // tdf#60237 - try to overwrite an existing file using the different settings of the Overwrite option
    loadFromURL(maTempFile.GetURL());
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xStorable.is());

    // overwrite the file using the default case of the Overwrite option (true)
    CPPUNIT_ASSERT_NO_THROW(xStorable->storeToURL(maTempFile.GetURL(), {}));

    // explicitly overwrite the file using the Overwrite option
    CPPUNIT_ASSERT_NO_THROW(xStorable->storeToURL(
        maTempFile.GetURL(),
        comphelper::InitPropertySequence({ { "Overwrite", uno::Any(true) } })));

    try
    {
        // overwrite an existing file with the Overwrite flag set to false
        xStorable->storeToURL(maTempFile.GetURL(), comphelper::InitPropertySequence(
                                                      { { "Overwrite", uno::Any(false) } }));
        CPPUNIT_ASSERT_MESSAGE("We expect an exception on overwriting an existing file", false);
    }
    catch (const css::uno::Exception&)
    {
    }
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
