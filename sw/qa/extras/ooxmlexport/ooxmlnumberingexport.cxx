/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
    bool CjkNumberedListTestHelper(sal_Int16& nValue)
    {
        bool isNumber = false;
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
        properties->getPropertyValue("NumberingIsNumber") >>= isNumber;
        if (!isNumber)
            return false;
        uno::Reference<container::XIndexAccess> xLevels(
            properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aPropertyValue;
        xLevels->getByIndex(0) >>= aPropertyValue;
        for (int j = 0; j < aPropertyValue.getLength(); ++j)
        {
            beans::PropertyValue aProp = aPropertyValue[j];
            if (aProp.Name == "NumberingType")
            {
                nValue = aProp.Value.get<sal_Int16>();
                return true;
            }
        }
        return false;
    }
};

DECLARE_OOXMLEXPORT_TEST(testNumberingFont, "numbering-font.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles("CharacterStyles")->getByName("ListLabel 1"), uno::UNO_QUERY);
    // This was Calibri, i.e. custom font of the numbering itself ("1.\t") was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("Verdana"), getProperty<OUString>(xStyle, "CharFontName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106541_noinheritChapterNumbering,
                         "tdf106541_noinheritChapterNumbering.odt")
{
    // in LO, it appears that styles based on the Chapter Numbering style explicitly set the
    // numbering style/outline level to 0 by default, and that LO prevents inheriting directly from "Outline" style.
    // Adding this preventative unit test to ensure that any fix for tdf106541 doesn't make incorrect assumptions.

    //reverting tdf#76817 hard-codes the numbering style on the paragraph, preventing RT of "Outline" style
    //    CPPUNIT_ASSERT_EQUAL(OUString("Outline"), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));

    OUString sPara3NumberingStyle = getProperty<OUString>(getParagraph(3), "NumberingStyleName");
    CPPUNIT_ASSERT_EQUAL(sPara3NumberingStyle,
                         getProperty<OUString>(getParagraph(4), "NumberingStyleName"));

    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//body/txt/Special", 3); //three of the four paragraphs have numbering
    assertXPath(pXmlDoc, "//body/txt[1]/Special", "rText", "1");
    assertXPath(pXmlDoc, "//body/txt[2]/Special", 0); //second paragraph style disables numbering
    assertXPath(pXmlDoc, "//body/txt[3]/Special", "rText", "I.");
    assertXPath(pXmlDoc, "//body/txt[4]/Special", "rText", "II.");
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist30, "cjklist30.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TIAN_GAN_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist31, "cjklist31.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::DI_ZI_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist34, "cjklist34.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_UPPER_ZH_TW, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist35, "cjklist35.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_LOWER_ZH, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlCjklist44, "cjklist44.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::NUMBER_HANGUL_KO, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTextNumberList, "text_number_list.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TEXT_NUMBER, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTextCardinalList, "text_cardinal_list.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TEXT_CARDINAL, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlTextOrdinalList, "text_ordinal_list.docx")
{
    sal_Int16 numFormat;
    CPPUNIT_ASSERT(CjkNumberedListTestHelper(numFormat));
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::TEXT_ORDINAL, numFormat);
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlNumListZHTW, "numlist-zhtw.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt", "val",
                "taiwaneseCountingThousand");
}

DECLARE_OOXMLEXPORT_TEST(testOoxmlNumListZHCN, "numlist-zhcn.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt", "val",
                "chineseCountingThousand");
}

DECLARE_OOXMLEXPORT_TEST(testOOxmlOutlineNumberTypes, "outline-number-types.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:pStyle", "val", "Heading1");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt", "val", "none");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[2]/w:numFmt", "val",
                "decimalEnclosedCircle");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[3]/w:numFmt", "val",
                "decimal"); // CHARS_GREEK_UPPER_LETTER fallback to decimal
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[4]/w:numFmt", "val",
                "decimal"); // CHARS_GREEK_LOWER_LETTER fallback to decimal
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[5]/w:numFmt", "val", "arabicAlpha");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[6]/w:numFmt", "val", "hindiVowels");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[7]/w:numFmt", "val", "thaiLetters");

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[1]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[2]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[3]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[4]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[5]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[6]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[7]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[2]/w:lvl[8]/w:numFmt", "val", "decimal");

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[1]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[2]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[3]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[4]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[5]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[6]/w:numFmt", "val", "decimal");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[3]/w:lvl[7]/w:numFmt", "val", "decimal");
}

DECLARE_OOXMLEXPORT_TEST(testNumParentStyle, "num-parent-style.docx")
{
    //reverting tdf#76817 hard-codes the numbering style on the paragraph, preventing RT of "Outline" style
    //I think this unit test is wrong, but I will revert to its original claim.
    CPPUNIT_ASSERT(
        getProperty<OUString>(getParagraph(4), "NumberingStyleName").startsWith("WWNum"));
}

DECLARE_OOXMLEXPORT_TEST(testNumOverrideLvltext, "num-override-lvltext.docx")
{
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 1, i.e. the numbering on the second level was "1", not "1.1".
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(2),
        comphelper::SequenceAsHashMap(xRules->getByIndex(1))["ParentNumbering"].get<sal_Int16>());

    // The paragraph marker's red font color was inherited by the number portion, this was ff0000.
    CPPUNIT_ASSERT_EQUAL(OUString("ffffffff"),
                         parseDump("//Special[@nType='POR_NUMBER']/SwFont", "color"));
}

DECLARE_OOXMLEXPORT_TEST(testNumOverrideStart, "num-override-start.docx")
{
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 1, i.e. the numbering on the second level was "1.1", not "1.3".
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(3),
        comphelper::SequenceAsHashMap(xRules->getByIndex(1))["StartWith"].get<sal_Int16>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf89890, "tdf89890.docx")
{
    // Numbering picture bullet was too large.
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bFound = false;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "GraphicSize")
        {
            // Height of the graphic was too large: 4382 after import, then 2485 after roundtrip.
            CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(279), rProp.Value.get<awt::Size>().Height);
            bFound = true;
        }
    }
    CPPUNIT_ASSERT(bFound);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106953, "tdf106953.docx")
{
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was -635, so the tab of the numbering expanded to a small value instead of matching Word's larger value.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        comphelper::SequenceAsHashMap(xRules->getByIndex(0))["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testI120928, "i120928.docx")
{
    // w:numPicBullet was ignored, leading to missing graphic bullet in numbering.
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    uno::Reference<awt::XBitmap> xBitmap;
    sal_Int16 nNumberingType = -1;

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "NumberingType")
            nNumberingType = rProp.Value.get<sal_Int16>();
        else if (rProp.Name == "GraphicBitmap")
            xBitmap = rProp.Value.get<uno::Reference<awt::XBitmap>>();
    }
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::BITMAP, nNumberingType);
    CPPUNIT_ASSERT(xBitmap.is());
}

DECLARE_OOXMLEXPORT_TEST(testFdo66781, "fdo66781.docx")
{
    // The problem was that bullets with level=0 were shown in LO as normal bullets,
    // and when saved back to DOCX were saved with level=1 (so hidden bullets became visible)
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "BulletChar")
        {
            CPPUNIT_ASSERT_EQUAL(OUString("\x0", 1, RTL_TEXTENCODING_ASCII_US),
                                 rProp.Value.get<OUString>());
            return;
        }
    }

    // Shouldn't reach here
    CPPUNIT_FAIL("Did not find bullet with level 0");
}

DECLARE_OOXMLEXPORT_TEST(testFDO74106, "FDO74106.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:numFmt", "val", "hebrew1");
}

DECLARE_OOXMLEXPORT_TEST(testFDO74215, "FDO74215.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;
        // tdf#106849 NumPicBullet xShape should not to be resized.

        // Seems this is dependent on the running system, which is - unfortunate
        // see: MSWordExportBase::BulletDefinitions
        // FIXME: the size of a bullet is defined by GraphicSize property
        // (stored in SvxNumberFormat::aGraphicSize) so use that for the size
        // (properly convert from 100mm to pt (1 inch is 72 pt, 1 pt is 20 twips).
#if !defined(MACOSX)
    assertXPath(pXmlDoc, "/w:numbering/w:numPicBullet[2]/w:pict/v:shape", "style",
                "width:11.25pt;height:11.25pt");
#endif
}

DECLARE_OOXMLEXPORT_TEST(testNumberedLists_StartingWithZero, "FDO74105.docx")
{
    /* Issue : Numbered lists Starting with value '0' is not preserved after RT.
    * In numbering.xml, an XML tag <w:start> is optional. If not mentioned,
    * the Numbered list should start from 0.
    * Problem was LO was writing <w:start> for all levels 0-8 with default value "1".
    */
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;

    // Check that we do _not_ export w:start for <w:lvl w:ilvl="0">.
    assertXPath(pXmlDoc, "w:numbering/w:abstractNum[1]/w:lvl[1]/w:start", 0);
}

DECLARE_OOXMLEXPORT_TEST(testfdo76589, "fdo76589.docx")
{
    /* Numbered list was not preserve after RT.
    * In numbering.xml, when NumberingType is "decimal" and level is zero,
    * w:val of w:lvlText was empty.
    * It should be <w:lvlText w:val="%1" />
    */
    xmlDocPtr pXmlDoc = parseExport("word/numbering.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText", "val", "%1");
}

DECLARE_OOXMLEXPORT_TEST(testDecimalNumberingNoLeveltext, "decimal-numbering-no-leveltext.docx")
{
    // This was "%1", not empty: we turned a kind-of-none numbering into a decimal one.
    if (xmlDocPtr pXmlDoc = parseExport("word/numbering.xml"))
        assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText", "val", "");
}

DECLARE_OOXMLEXPORT_TEST(testNumberedList, "NumberedList.docx")
{
    //fdo74150:In document.xml, for pStyle = "NumberedList1", iLvl and numId was not preserved
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:pStyle", "val",
                "NumberedList1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:ilvl",
                "val", "0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p[1]/w:pPr[1]/w:numPr/w:numId",
                "val", "0");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:pStyle",
                "val", "NumberedList1");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:ilvl", "val",
                "0");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[3]/w:pPr[1]/w:numPr/w:numId",
                "val", "0");
}

DECLARE_OOXMLEXPORT_TEST(testNumbering1, "numbering1.docx")
{
    /* <w:numPr> in the paragraph itself was overridden by <w:numpr> introduced by the paragraph's <w:pStyle>
    enum = ThisComponent.Text.createEnumeration
    para = enum.NextElement
    xray para.NumberingStyleName
    numberingstyle = ThisComponent.NumberingRules.getByIndex(6)
    xray numberingstyle.name   - should match name above
    numbering = numberingstyle.getByIndex(0)
    xray numbering(11)  - should be 4, arabic
    note that the indexes may get off as the implementation evolves, C++ code searches in loops
    */
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> paragraph(getParagraph(1, "Text1."));
    OUString numberingStyleName = getProperty<OUString>(paragraph, "NumberingStyleName");
    uno::Reference<text::XNumberingRulesSupplier> xNumberingRulesSupplier(mxComponent,
                                                                          uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> numberingRules(
        xNumberingRulesSupplier->getNumberingRules(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> numberingRule;
    for (int i = 0; i < numberingRules->getCount(); ++i)
    {
        OUString name = getProperty<OUString>(numberingRules->getByIndex(i), "Name");
        if (name == numberingStyleName)
        {
            numberingRule.set(numberingRules->getByIndex(i), uno::UNO_QUERY);
            break;
        }
    }
    CPPUNIT_ASSERT(numberingRule.is());
    uno::Sequence<beans::PropertyValue> numbering;
    numberingRule->getByIndex(0) >>= numbering;
    sal_Int16 numberingType = style::NumberingType::NUMBER_NONE;
    for (int i = 0; i < numbering.getLength(); ++i)
    {
        if (numbering[i].Name == "NumberingType")
        {
            numbering[i].Value >>= numberingType;
            break;
        }
    }
    CPPUNIT_ASSERT_EQUAL(style::NumberingType::ARABIC, numberingType);
}

DECLARE_OOXMLEXPORT_TEST(testFdo59638, "fdo59638.docx")
{
    // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletChar")
        {
            // Was '*', should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString(u"\uF0B7"), rProp.Value.get<OUString>());
            return;
        }
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_OOXMLEXPORT_TEST(testToolsLineNumbering, "tools-line-numbering.docx")
{
    /*
    * Test the line numbering feature import (Tools->Line Numbering ...)
    *
    * xray ThisComponent.getLineNumberingProperties().IsOn == True
    * xray ThisComponent.getLineNumberingProperties().CountEmptyLines == True
    * xray ThisComponent.getLineNumberingProperties().NumberPosition == 0
    * xray ThisComponent.getLineNumberingProperties().NumberingType == 4
    * xray ThisComponent.getLineNumberingProperties().SeparatorInterval == 3
    */

    bool bValue = false;
    sal_Int32 nValue = -1;

    uno::Reference<text::XTextDocument> xtextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XLineNumberingProperties> xLineProperties(xtextDocument,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet
        = xLineProperties->getLineNumberingProperties();

    xPropertySet->getPropertyValue("IsOn") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);

    xPropertySet->getPropertyValue("CountEmptyLines") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);

    xPropertySet->getPropertyValue("NumberPosition") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nValue);

    xPropertySet->getPropertyValue("NumberingType") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nValue);

    xPropertySet->getPropertyValue("SeparatorInterval") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nValue);
}

DECLARE_OOXMLEXPORT_TEST(testRhbz988516, "rhbz988516.docx")
{
    // The problem was that the list properties of the footer leaked into body
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Enclosure 3"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(3), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(4), "NumberingStyleName"));

    // tdf#103975 The problem was that an empty paragraph with page break info was removed.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

//Outline
DECLARE_OOXMLEXPORT_TEST(testTdf107684, "tdf107684.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/styles.xml"))
        // This was 1, <w:outlineLvl> was duplicated for Heading1.
        assertXPath(pXmlDoc, "//w:style[@w:styleId='Heading1']/w:pPr/w:outlineLvl", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf89702, "tdf89702.docx")
{
    // Get the first paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aCharStyleName = std::find_if(aLevel.begin(), aLevel.end(),
                                           [](const beans::PropertyValue& rValue) {
                                               return rValue.Name == "CharStyleName";
                                           })
                                  ->Value.get<OUString>();

    // Make sure that the font name is Arial, this was Verdana.
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xStyle, "CharFontName"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
