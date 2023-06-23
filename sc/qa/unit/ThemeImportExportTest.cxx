/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <scitems.hxx>
#include <editeng/brushitem.hxx>

using namespace css;

class ThemeImportExportTest : public ScModelTestBase
{
public:
    ThemeImportExportTest()
        : ScModelTestBase("sc/qa/unit/data")
    {
    }
};

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testThemeExport)
{
    loadFromURL(u"xlsx/CalcThemeTest.xlsx");

    save("Calc Office Open XML");

    xmlDocUniquePtr pXmlDoc = parseExport("xl/styles.xml");

    assertXPath(pXmlDoc, "/x:styleSheet", 1);

    // Fonts
    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font", 6);

    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font[5]/x:color", "rgb",
                "FFFFC000"); // need to be theme probably

    assertXPath(pXmlDoc, "/x:styleSheet/x:fonts/x:font[6]/x:color", "rgb",
                "FF9C5700"); // need to be theme probably

    // Fills
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill", 4);

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[1]/x:patternFill", "patternType", "none");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[2]/x:patternFill", "patternType", "gray125");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[3]/x:patternFill", "patternType", "solid");
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[3]/x:patternFill/x:fgColor", "rgb",
                "FFFFEB9C");

    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[4]/x:patternFill", "patternType", "solid");
    assertXPath(pXmlDoc, "/x:styleSheet/x:fills/x:fill[4]/x:patternFill/x:fgColor", "theme", "4");
}

CPPUNIT_TEST_FIXTURE(ThemeImportExportTest, testCellBackgroundThemeColor)
{
    loadFromURL(u"xlsx/Test_ThemeColor_Text_Background_Border.xlsx");

    ScDocument* pDoc = getScDoc();

    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(0, 0, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0x27ced7), pBrushItem->GetColor());
        auto aComplexColor = pBrushItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getSchemeType());
        CPPUNIT_ASSERT_EQUAL(size_t(0), aComplexColor.getTransformations().size());
    }

    {
        const ScPatternAttr* pPattern = pDoc->GetPattern(0, 1, 0);
        const SfxPoolItem* pItem = nullptr;
        pPattern->GetItemSet().HasItem(ATTR_BACKGROUND, &pItem);
        CPPUNIT_ASSERT(pItem);

        auto* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        CPPUNIT_ASSERT_EQUAL(Color(0xd4f5f7), pBrushItem->GetColor());
        auto aComplexColor = pBrushItem->getComplexColor();
        CPPUNIT_ASSERT_EQUAL(model::ThemeColorType::Accent3, aComplexColor.getSchemeType());
        auto& rTransformations = aComplexColor.getTransformations();
        CPPUNIT_ASSERT_EQUAL(size_t(2), rTransformations.size());
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumMod, rTransformations[0].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2000), rTransformations[0].mnValue);
        CPPUNIT_ASSERT_EQUAL(model::TransformationType::LumOff, rTransformations[1].meType);
        CPPUNIT_ASSERT_EQUAL(sal_Int16(7999), rTransformations[1].mnValue);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
