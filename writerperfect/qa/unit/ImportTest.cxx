/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <unotools/mediadescriptor.hxx>

#include "WpftFilterFixture.hxx"
#include "WpftLoader.hxx"

namespace
{
namespace beans = css::beans;
namespace container = css::container;
namespace document = css::document;
namespace lang = css::lang;
namespace sheet = css::sheet;
namespace table = css::table;
namespace uno = css::uno;

using uno::UNO_QUERY;

using writerperfect::test::WpftLoader;

class ImportTest : public writerperfect::test::WpftFilterFixture
{
public:
    virtual void setUp() override;

    void testWK3WithFM3();

    CPPUNIT_TEST_SUITE(ImportTest);
    CPPUNIT_TEST(testWK3WithFM3);
    CPPUNIT_TEST_SUITE_END();

private:
    WpftLoader createCalcLoader(std::u16string_view rFile) const;

    WpftLoader createLoader(const OUString& rUrl, const OUString& rFactoryUrl) const;

    OUString makeUrl(std::u16string_view rFile) const;

private:
    uno::Reference<lang::XMultiServiceFactory> m_xFilterFactory;
};

void ImportTest::setUp()
{
    writerperfect::test::WpftFilterFixture::setUp();

    m_xFilterFactory.set(
        m_xFactory->createInstanceWithContext("com.sun.star.document.FilterFactory", m_xContext),
        UNO_QUERY);
    assert(m_xFilterFactory.is());
}

void ImportTest::testWK3WithFM3()
{
    WpftLoader aLoader(createCalcLoader(u"SOLVE.WK3"));
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(aLoader.getDocument(), UNO_QUERY);
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<container::XIndexAccess> xSheets(xDoc->getSheets(), UNO_QUERY);
    CPPUNIT_ASSERT(xSheets.is());
    uno::Reference<table::XCellRange> xSheet(xSheets->getByIndex(0), UNO_QUERY);
    CPPUNIT_ASSERT(xSheet.is());
    uno::Reference<beans::XPropertySet> xCellProps(xSheet->getCellByPosition(1, 1), UNO_QUERY);
    CPPUNIT_ASSERT(xCellProps.is());
    sal_Int32 nCharColor = 0;
    CPPUNIT_ASSERT(xCellProps->getPropertyValue("CharColor") >>= nCharColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000ff), nCharColor); // blue text
}

WpftLoader ImportTest::createCalcLoader(std::u16string_view rFile) const
{
    return createLoader(makeUrl(rFile), "private:factory/scalc");
}

WpftLoader ImportTest::createLoader(const OUString& rUrl, const OUString& rFactoryUrl) const
{
    utl::MediaDescriptor aDesc;
    aDesc[utl::MediaDescriptor::PROP_URL()] <<= rUrl;
    aDesc[utl::MediaDescriptor::PROP_READONLY()] <<= true;
    uno::Sequence<beans::PropertyValue> lDesc(aDesc.getAsConstPropertyValueList());
    m_xTypeDetection->queryTypeByDescriptor(lDesc, true);
    aDesc = lDesc;
    OUString sFilter;
    aDesc[utl::MediaDescriptor::PROP_FILTERNAME()] >>= sFilter;
    CPPUNIT_ASSERT(!sFilter.isEmpty());
    const uno::Reference<document::XFilter> xFilter(m_xFilterFactory->createInstance(sFilter),
                                                    UNO_QUERY);
    CPPUNIT_ASSERT(xFilter.is());
    return WpftLoader(rUrl, xFilter, rFactoryUrl, m_xDesktop, m_xTypeMap, m_xContext);
}

OUString ImportTest::makeUrl(std::u16string_view rFile) const
{
    return const_cast<ImportTest*>(this)->m_directories.getURLFromSrc(
        OUString(OUString::Concat("/" TEST_DIR "/") + rFile));
}

CPPUNIT_TEST_SUITE_REGISTRATION(ImportTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
