/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
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
    WpftLoader createCalcLoader(const rtl::OUString &rFile) const;

    WpftLoader createLoader(const rtl::OUString &rUrl, const rtl::OUString &rFactoryUrl) const;

    rtl::OUString makeUrl(const rtl::OUString &rFile) const;

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
    WpftLoader aLoader(createCalcLoader("SOLVE.WK3"));
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
#if 0 // broken by commit 8154953add163554c00935486a1cf5677cef2609
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x0000ff), nCharColor); // blue text
#endif
}

WpftLoader ImportTest::createCalcLoader(const rtl::OUString &rFile) const
{
    return createLoader(makeUrl(rFile), "private:factory/scalc");
}

WpftLoader ImportTest::createLoader(const rtl::OUString &rUrl, const rtl::OUString &rFactoryUrl) const
{
    utl::MediaDescriptor aDesc;
    aDesc[utl::MediaDescriptor::PROP_URL()] <<= rUrl;
    aDesc[utl::MediaDescriptor::PROP_READONLY()] <<= true;
    uno::Sequence<beans::PropertyValue> lDesc(aDesc.getAsConstPropertyValueList());
    const rtl::OUString sType = m_xTypeDetection->queryTypeByDescriptor(lDesc, true);
    CPPUNIT_ASSERT(!sType.isEmpty());
    const uno::Reference<document::XFilter> xFilter(m_xFilterFactory->createInstance(sType), UNO_QUERY);
    CPPUNIT_ASSERT(xFilter.is());
    return WpftLoader(rUrl, xFilter, rFactoryUrl, m_xDesktop, m_xTypeMap, m_xContext);
}

rtl::OUString ImportTest::makeUrl(const rtl::OUString &rFile) const
{
    return const_cast<ImportTest *>(this)->m_directories.getURLFromSrc("/" TEST_DIR "/" + rFile);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ImportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
