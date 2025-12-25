/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <config_features.h>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/cjk/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf101729)
{
    createSwDoc("tdf101729.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 l
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "left").toInt32();
    sal_Int32 w
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32();
    sal_Int32 x
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt/infos/bounds", "left").toInt32();
    // Make sure the text does not go outside and verify it is centered roughly
    CPPUNIT_ASSERT(l + w / 4 < x);
    CPPUNIT_ASSERT(x < l + 3 * w / 4);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115079)
{
    createSwDoc("tdf115079.odt");
    // This document caused segfault when layouting
}

CPPUNIT_TEST_FIXTURE(Test, testMsWordUlTrailSpace)
{
    createSwDoc("UnderlineTrailingSpace.docx");
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true),
                             xSettings->getPropertyValue(u"MsWordUlTrailSpace"_ustr));
    }

    // Test also after save-and-reload:
    saveAndReload(TestFilter::DOCX);
    {
        uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xSettings(
            xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(uno::Any(true),
                             xSettings->getPropertyValue(u"MsWordUlTrailSpace"_ustr));
    }

    // Check that the compat setting is exported in OOXML
    xmlDocUniquePtr pXmlSettings = parseExport("word/settings.xml");
    assertXPath(pXmlSettings, "/w:settings/w:compat/w:ulTrailSpace");
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist30, "cjklist30.doc")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist31, "cjklist31.doc")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist34, "cjklist34.doc")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_WW8EXPORT_TEST(testWw8Cjklist35, "cjklist35.doc")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(1);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_WW8EXPORT_TEST(testTdf118564, "tdf118564.doc")
{
    sal_Int16 numFormat = getNumberingTypeOfParagraph(3);
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77964)
{
    createSwDoc("tdf77964.doc");
    saveAndReload(TestFilter::DOC);
    // both images were loading as AT_PARA instead of AS_CHAR. Image2 visually had text wrapping.
    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType_AS_CHARACTER,
        getProperty<text::TextContentAnchorType>(getShapeByName(u"Image2"), u"AnchorType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf167455)
{
    // crashes at import time on macOS
    createSwDoc("tdf167455.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
