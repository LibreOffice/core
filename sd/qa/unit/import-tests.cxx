/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <tools/color.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>

#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdotable.hxx>
#include <o3tl/environment.hxx>

#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

#include <stlpool.hxx>

#include <unotools/syslocaleoptions.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <osl/file.hxx>

using namespace ::com::sun::star;

/// Impress import filters tests.
class SdImportTest : public SdModelTestBase
{
public:
    SdImportTest()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }

    void testDocumentLayout(std::u16string_view sInput, std::u16string_view sDump,
                            TestFilter sExportType, bool bUpdateMe = false);
};

/** Test document against a reference XML dump of shapes.

If you want to update one of these tests, or add a new one, set the bUpdateMe
to true, and the dump XML's will be created (or rewritten)
instead of checking. Use with care - when the test is failing, first find out
why, instead of just updating .xml's blindly.

Example: Let's say you are adding a test called fdoABCD.pptx.  You'll place it
to the data/ subdirectory, and will add a new test below,
When the 3rd parameter is set TestFilter::NONE, it doesn't export.
Temporarily you'll set bUpdateMe to true, and run

make CppunitTest_sd_import_tests

This will generate the sd/qa/unit/data/xml/fdoABCD_*.xml for you.  Now you
will change bUpdateMe back to false, and commit your fdoABCD.pptx test, the
xml/fdoABCD_*.xml dumps, and the addition in this file in one commit.

As the last step, you will revert your fix and do 'make CppunitTest_sd_import_tests' again, to check
that without your fix, the unit test breaks.  Then clean up, and push :-)

NOTE: This approach is suitable only for tests of fixes that actually change
the layout - best to check by reverting your fix locally after having added
the test, and re-running; it should break.
*/
void SdImportTest::testDocumentLayout(std::u16string_view sInput, std::u16string_view sDump,
                                      TestFilter sExportType, bool bUpdateMe)
{
    // Use fallback document theme, which uses previous default
    // fill and line colors as accent colors.
    o3tl::setEnvironment(u"LO_FORCE_FALLBACK_DOCUMENT_THEME"_ustr, u"1"_ustr);

    loadFromFile(sInput);
    if (sExportType != TestFilter::NONE)
        saveAndReload(sExportType);

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
    CPPUNIT_ASSERT(xDrawPages.is());

    sal_Int32 nLength = xDrawPages->getCount();
    for (sal_Int32 j = 0; j < nLength; ++j)
    {
        uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(j), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xDrawPage.is());
        uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);
        OUString aString = XShapeDumper::dump(xShapes);

        OString aFileName = OUStringToOString(createFileURL(sDump), RTL_TEXTENCODING_UTF8)
                            + OString::number(j) + ".xml";

        if (bUpdateMe)
        {
            // had to adapt this, std::ofstream annot write to a URL but needs a
            // filesystem path. Seems as if no one had to adapt any of the cases
            // for some years :-/
            OUString sTempFilePath;
            osl::FileBase::getSystemPathFromFileURL(OUString::fromUtf8(aFileName), sTempFilePath);
            std::ofstream aStream(sTempFilePath.toUtf8().getStr(),
                                  std::ofstream::out | std::ofstream::binary);
            aStream << aString;
            aStream.close();
        }
        else
        {
            doXMLDiff(
                aFileName.getStr(), OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr(),
                static_cast<int>(aString.getLength()),
                OUStringToOString(createFileURL(u"tolerance.xml"), RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testShapesTest)
{
    testDocumentLayout(u"odp/shapes-test.odp", u"xml/shapes-test_page", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo47434)
{
    testDocumentLayout(u"fdo47434.pptx", u"xml/fdo47434_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN758621)
{
    testDocumentLayout(u"n758621.ppt", u"xml/n758621_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo64586)
{
    testDocumentLayout(u"fdo64586.ppt", u"xml/fdo64586_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN819614)
{
    // needed to adapt this, the border parameter is no longer
    // exported with MCGRs due to oox neither needing nor
    // supporting it with now freely definable gradients
    testDocumentLayout(u"n819614.pptx", u"xml/n819614_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN820786)
{
    testDocumentLayout(u"n820786.pptx", u"xml/n820786_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN762695)
{
    testDocumentLayout(u"n762695.pptx", u"xml/n762695_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN593612)
{
    testDocumentLayout(u"n593612.pptx", u"xml/n593612_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo71434)
{
    testDocumentLayout(u"fdo71434.pptx", u"xml/fdo71434_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN902652)
{
    testDocumentLayout(u"n902652.pptx", u"xml/n902652_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf90403)
{
    testDocumentLayout(u"tdf90403.pptx", u"xml/tdf90403_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf90338)
{
    testDocumentLayout(u"tdf90338.odp", u"xml/tdf90338_", TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf92001)
{
    testDocumentLayout(u"tdf92001.odp", u"xml/tdf92001_", TestFilter::PPTX);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf100491)
{
    testDocumentLayout(u"tdf100491.pptx", u"xml/tdf100491_", TestFilter::NONE);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf109317)
{
    testDocumentLayout(u"tdf109317.pptx", u"xml/tdf109317_", TestFilter::ODP);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf157216)
{
    createSdImpressDoc("pptx/tdf157216.pptx");
    uno::Reference<beans::XPropertySet> xFlowchartShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aProps;
    xFlowchartShape->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aProps;

    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "GluePoints")
        {
            seqGluePoints
                = rProp.Value.get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>();
        }
    }

    sal_Int32 nCountGluePoints = seqGluePoints.getLength();
    // The Flowchart: Punched Tape has 4 glue points.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nCountGluePoints);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTableStyle)
{
    createSdImpressDoc("pptx/tdf156718.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCellPropSet;
    uno::Reference<beans::XPropertySet> xRunPropSet;
    uno::Reference<text::XTextRange> xParagraph;
    uno::Reference<text::XTextRange> xRun;
    table::BorderLine2 aBorderLine;
    Color nFillColor, nCharColor;
    float nFontWeight;

    xCellPropSet.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    xRunPropSet->getPropertyValue(u"CharWeight"_ustr) >>= nFontWeight;
    xCellPropSet->getPropertyValue(u"BottomBorder"_ustr) >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The bottom border is missing!", true, aBorderLine.LineWidth > 0);

    xCellPropSet.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    xRunPropSet->getPropertyValue(u"CharWeight"_ustr) >>= nFontWeight;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);

    xCellPropSet.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    xRunPropSet->getPropertyValue(u"CharWeight"_ustr) >>= nFontWeight;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);

    xCellPropSet.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    xRunPropSet->getPropertyValue(u"CharWeight"_ustr) >>= nFontWeight;
    xCellPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nFillColor);

    xCellPropSet.set(xTable->getCellByPosition(2, 1), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue(u"CharColor"_ustr) >>= nCharColor;
    xRunPropSet->getPropertyValue(u"CharWeight"_ustr) >>= nFontWeight;
    xCellPropSet->getPropertyValue(u"FillColor"_ustr) >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nFillColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFreeformShapeGluePoints)
{
    createSdImpressDoc("pptx/tdf156829.pptx");
    uno::Reference<beans::XPropertySet> xFreeformShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aProps;
    xFreeformShape->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aProps;

    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "GluePoints")
        {
            seqGluePoints
                = rProp.Value.get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>();
        }
    }

    sal_Int32 nCountGluePoints = seqGluePoints.getLength();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nCountGluePoints);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf154363)
{
    sal_Int32 nGlueId;
    createSdImpressDoc("pptx/tdf154363.pptx");
    {
        uno::Reference<beans::XPropertySet> xConnector1(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
        uno::Reference<beans::XPropertySet> xConnector2(getShapeFromPage(3, 0), uno::UNO_SET_THROW);
        nGlueId = xConnector1->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nGlueId);
        nGlueId = xConnector2->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nGlueId);
    }

    saveAndReload(TestFilter::PPTX_2007);
    {
        uno::Reference<beans::XPropertySet> xConnector1(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
        uno::Reference<beans::XPropertySet> xConnector2(getShapeFromPage(3, 0), uno::UNO_SET_THROW);
        nGlueId = xConnector1->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nGlueId);
        nGlueId = xConnector2->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nGlueId);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf154858)
{
    createSdImpressDoc("pptx/tdf154858.pptx");
    uno::Reference<beans::XPropertySet> xShapeProp(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    awt::Gradient2 aGradient;
    CPPUNIT_ASSERT(xShapeProp->getPropertyValue(u"FillGradient"_ustr) >>= aGradient);
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_RADIAL, aGradient.Style);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf153466)
{
    createSdImpressDoc("pptx/tdf153466.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPageSet(xPage, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xBackground(
        xPageSet->getPropertyValue(u"Background"_ustr).get<uno::Reference<beans::XPropertySet>>());

    css::drawing::RectanglePoint aRectanglePoint;
    xBackground->getPropertyValue(u"FillBitmapRectanglePoint"_ustr) >>= aRectanglePoint;
    CPPUNIT_ASSERT_EQUAL(drawing::RectanglePoint::RectanglePoint_RIGHT_BOTTOM, aRectanglePoint);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    xShape->getPropertyValue(u"FillBitmapRectanglePoint"_ustr) >>= aRectanglePoint;
    CPPUNIT_ASSERT_EQUAL(drawing::RectanglePoint::RectanglePoint_LEFT_MIDDLE, aRectanglePoint);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf152434)
{
    createSdImpressDoc("pptx/tdf152434.pptx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    const SdrPage* pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testConnectors)
{
    createSdImpressDoc("pptx/connectors.pptx");

    sal_Int32 aEdgeValue[] = { -1167, -1167, -1591, 1476,  1356, -1357, 1604,  -1540,
                               607,   1296,  -1638, -1060, -522, 1578,  -1291, 333 };

    sal_Int32 nCount = 0;
    for (size_t i = 0; i < 18; i++)
    {
        uno::Reference<beans::XPropertySet> xConnector(getShapeFromPage(i, 0));
        bool bConnector
            = xConnector->getPropertySetInfo()->hasPropertyByName(u"EdgeLine1Delta"_ustr);
        if (bConnector)
        {
            sal_Int32 nEdgeLine
                = xConnector->getPropertyValue(u"EdgeLine1Delta"_ustr).get<sal_Int32>();
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("edge index " + OString::number(nCount)).getStr(),
                                         aEdgeValue[nCount], nEdgeLine);
            nCount++;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf153036_resizedConnectorL)
{
    createSdImpressDoc("pptx/tdf153036_resizedConnectorL.pptx");

    // TODO: If you are working on improvement related to connectors import, then
    // expect this unit test to fail.
    // This is a "L" shape, imported as a special "Z" shape that looks like a "L" shape.
    uno::Reference<beans::XPropertySet> xConnector(getShapeFromPage(1, 0));
    CPPUNIT_ASSERT(xConnector->getPropertySetInfo()->hasPropertyByName(u"EdgeLine1Delta"_ustr));

    sal_Int32 nEdgeLine = xConnector->getPropertyValue(u"EdgeLine1Delta"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-3243), nEdgeLine);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf150719)
{
    createSdImpressDoc("pptx/tdf150719.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(1, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun, uno::UNO_QUERY_THROW);
    sal_Int16 nUnderline;
    xPropSet1->getPropertyValue(u"CharUnderline"_ustr) >>= nUnderline;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The underline is missing!", sal_Int16(1), nUnderline);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf149314)
{
    createSdImpressDoc("pptx/tdf149314.pptx");

    OUString aURL;
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 1));

    uno::Reference<text::XTextRange> const xParagraph1(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun1(getRunFromParagraph(0, xParagraph1));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun1, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextField> xField1;
    xPropSet1->getPropertyValue(u"TextField"_ustr) >>= xField1;
    xPropSet1.set(xField1, uno::UNO_QUERY);
    xPropSet1->getPropertyValue(u"URL"_ustr) >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", u"#Slide 1"_ustr, aURL);

    uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(1, xShape));
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextField> xField2;
    xPropSet2->getPropertyValue(u"TextField"_ustr) >>= xField2;
    xPropSet2.set(xField2, uno::UNO_QUERY);
    xPropSet2->getPropertyValue(u"URL"_ustr) >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", u"#Slide 3"_ustr, aURL);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf149124)
{
    createSdImpressDoc("pptx/tdf149124.pptx");

    uno::Reference<container::XIndexAccess> xGroupShape(getShapeFromPage(0, 0),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xStandardConnector(xGroupShape->getByIndex(2),
                                                           uno::UNO_QUERY_THROW);

    sal_Int32 nStartGlueId
        = xStandardConnector->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), nStartGlueId);
    sal_Int32 nEndGlueId
        = xStandardConnector->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nEndGlueId);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf148965)
{
    // Set the system user interface to Hungarian
    SvtSysLocaleOptions aOptions;
    OUString sUIConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetUILocaleConfigString(u"hu-HU"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g([&aOptions, &sUIConfigString] {
        aOptions.SetUILocaleConfigString(sUIConfigString);
        aOptions.Commit();
    });

    createSdImpressDoc("pptx/tdf148965.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName(u"OnClick"_ustr) >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find(u"Bookmark"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"page1"_ustr, iter1->second.get<OUString>());

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName(u"OnClick"_ustr) >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find(u"Bookmark"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"page3"_ustr, iter2->second.get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf89449)
{
    createSdImpressDoc("pptx/tdf89449.pptx");

    sal_Int32 nStartGlueId;
    sal_Int32 nEndGlueId;
    css::drawing::ConnectorType aConnectorType;

    uno::Reference<beans::XPropertySet> xCurvedConnector(getShapeFromPage(3, 0));
    xCurvedConnector->getPropertyValue(u"EdgeKind"_ustr) >>= aConnectorType;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_CURVE, aConnectorType);
    nStartGlueId = xCurvedConnector->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), nStartGlueId);
    nEndGlueId = xCurvedConnector->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nEndGlueId);

    uno::Reference<beans::XPropertySet> xStraightConnector(getShapeFromPage(4, 0));
    xStraightConnector->getPropertyValue(u"EdgeKind"_ustr) >>= aConnectorType;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_LINE, aConnectorType);
    nStartGlueId
        = xStraightConnector->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7), nStartGlueId);
    nEndGlueId = xStraightConnector->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nEndGlueId);

    uno::Reference<beans::XPropertySet> xStandardConnector(getShapeFromPage(5, 0));
    xStandardConnector->getPropertyValue(u"EdgeKind"_ustr) >>= aConnectorType;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_STANDARD, aConnectorType);
    nStartGlueId
        = xStandardConnector->getPropertyValue(u"StartGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nStartGlueId);
    nEndGlueId = xStandardConnector->getPropertyValue(u"EndGluePointIndex"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), nEndGlueId);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testGluePointLeavingDirections)
{
    createSdImpressDoc("pptx/glue_point_leaving_directions.pptx");
    uno::Reference<beans::XPropertySet> xEllipseShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aProps;
    xEllipseShape->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aProps;

    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    uno::Sequence<double> seqGluePointLeavingDirections;
    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "GluePointLeavingDirections")
        {
            rProp.Value >>= seqGluePointLeavingDirections;
        }
    }

    sal_Int32 nCountGluePointLeavingDirections = seqGluePointLeavingDirections.getLength();
    // The ellipse has 8 glue point leaving directions
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), nCountGluePointLeavingDirections);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf147459)
{
    createSdImpressDoc("pptx/tdf147459.pptx");
    uno::Reference<beans::XPropertySet> xTriangleShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aProps;
    xTriangleShape->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aProps;

    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
    for (beans::PropertyValue const& rProp : aPathProps)
    {
        if (rProp.Name == "GluePoints")
        {
            seqGluePoints
                = rProp.Value.get<uno::Sequence<drawing::EnhancedCustomShapeParameterPair>>();
        }
    }

    sal_Int32 nCountGluePoints = seqGluePoints.getLength();
    // The triangle has 6 glue points.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), nCountGluePoints);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf146223)
{
    createSdImpressDoc("pptx/tdf146223.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSet(xPage1, uno::UNO_QUERY_THROW);

    bool bBackgroundObjectsVisible;
    xSet->getPropertyValue(u"IsBackgroundObjectsVisible"_ustr) >>= bBackgroundObjectsVisible;
    CPPUNIT_ASSERT_EQUAL(false, bBackgroundObjectsVisible);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf144918)
{
    createSdImpressDoc("pptx/tdf144918.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName(u"OnClick"_ustr) >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find(u"Bookmark"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"First slide"_ustr, iter1->second.get<OUString>());

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName(u"OnClick"_ustr) >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find(u"Bookmark"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Third slide"_ustr, iter2->second.get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf144917)
{
    createSdImpressDoc("pptx/tdf144917.pptx");

    uno::Reference<container::XIndexAccess> xGroupShape(getShapeFromPage(0, 0),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xGroupShape->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<document::XEventsSupplier> xEventsSupplier(xShape, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents(xEventsSupplier->getEvents());
    uno::Sequence<beans::PropertyValue> props;
    xEvents->getByName(u"OnClick"_ustr) >>= props;
    comphelper::SequenceAsHashMap map(props);
    auto iter(map.find(u"Bookmark"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"http://www.example.com/"_ustr, iter->second.get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testHyperlinkOnImage)
{
    createSdImpressDoc("pptx/hyperlinkOnImage.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(1, 0));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName(u"OnClick"_ustr) >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find(u"ClickAction"_ustr));
    CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_LASTPAGE,
                         iter1->second.get<css::presentation::ClickAction>());

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName(u"OnClick"_ustr) >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find(u"ClickAction"_ustr));
    CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_NONE,
                         iter2->second.get<css::presentation::ClickAction>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142645)
{
    createSdImpressDoc("pptx/tdf142645.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed1(xPage1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xNamed1->getName());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf141704)
{
    createSdImpressDoc("pptx/tdf141704.pptx");

    for (sal_Int32 i = 0; i < 7; i++)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, i));
        uno::Reference<document::XEventsSupplier> xEventsSupplier(xShape, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xEvents(xEventsSupplier->getEvents());

        uno::Sequence<beans::PropertyValue> props;
        xEvents->getByName(u"OnClick"_ustr) >>= props;
        comphelper::SequenceAsHashMap map(props);
        auto iter(map.find(u"ClickAction"_ustr));
        switch (i)
        {
            case 0:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_LASTPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 1:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_NEXTPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 2:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_PREVPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 3:
            {
                auto iter1(map.find(u"Bookmark"_ustr));
                CPPUNIT_ASSERT_EQUAL(u"http://www.example.com/"_ustr,
                                     iter1->second.get<OUString>());
            }
            break;
            case 4:
            {
                auto iter2(map.find(u"Bookmark"_ustr));
                CPPUNIT_ASSERT_EQUAL(u"End Show"_ustr, iter2->second.get<OUString>());
            }
            break;
            case 5:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_STOPPRESENTATION,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            case 6:
                CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_FIRSTPAGE,
                                     iter->second.get<css::presentation::ClickAction>());
                break;
            default:
                break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142915)
{
    createSdImpressDoc("pptx/tdf142915.pptx");

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    bool bChangeManually = xPresentationProps->getPropertyValue(u"IsAutomatic"_ustr).get<bool>();

    CPPUNIT_ASSERT_EQUAL(true, bChangeManually);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142913)
{
    createSdImpressDoc("pptx/tdf142913.pptx");

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    OUString sFirstPage = xPresentationProps->getPropertyValue(u"FirstPage"_ustr).get<OUString>();

    CPPUNIT_ASSERT_EQUAL(u"Second"_ustr, sFirstPage);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142590)
{
    createSdImpressDoc("pptx/tdf142590.pptx");

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);
    const OUString sCustomShowId
        = xPresentationProps->getPropertyValue(u"CustomShow"_ustr).get<OUString>();

    CPPUNIT_ASSERT(!sCustomShowId.isEmpty());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testCustomSlideShow)
{
    createSdImpressDoc("pptx/tdf131390.pptx");

    css::uno::Reference<css::presentation::XCustomPresentationSupplier> aXCPSup(
        mxComponent, css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameContainer> aXCont(aXCPSup->getCustomPresentations());
    const css::uno::Sequence<OUString> aNameSeq(aXCont->getElementNames());

    // In the document, there are two custom presentations.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), aNameSeq.size());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testInternalHyperlink)
{
    createSdImpressDoc("pptx/tdf65724.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 0));

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // first chunk of text
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue(u"TextField"_ustr) >>= xField;
    CPPUNIT_ASSERT_MESSAGE("The text field is missing!", xField.is());

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue(u"URL"_ustr) >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", u"#Slide2"_ustr, aURL);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testHyperlinkColor)
{
    createSdImpressDoc("pptx/tdf137367.pptx");

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph1(getParagraphFromShape(0, xShape));
    // Get second paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(1, xShape));
    // Get third paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph3(getParagraphFromShape(2, xShape));

    // Get run of the first paragraph
    uno::Reference<text::XTextRange> xRun1(getRunFromParagraph(0, xParagraph1));
    uno::Reference<beans::XPropertySet> xPropSet1(xRun1, uno::UNO_QUERY_THROW);
    Color nCharColorBlue;
    xPropSet1->getPropertyValue(u"CharColor"_ustr) >>= nCharColorBlue;

    // Get run of the second paragraph
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(1, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    Color nCharColorRed;
    xPropSet2->getPropertyValue(u"CharColor"_ustr) >>= nCharColorRed;

    // Get run of the third paragraph
    uno::Reference<text::XTextRange> xRun3(getRunFromParagraph(2, xParagraph3));
    uno::Reference<beans::XPropertySet> xPropSet3(xRun3, uno::UNO_QUERY_THROW);
    Color nCharColorGreen;
    xPropSet3->getPropertyValue(u"CharColor"_ustr) >>= nCharColorGreen;

    // Hyperlink colors should be blue, red, green.
    CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), nCharColorBlue);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, nCharColorRed);
    CPPUNIT_ASSERT_EQUAL(Color(0x548235), nCharColorGreen);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testSmoketest)
{
    createSdImpressDoc("smoketest.pptx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // cf. SdrModel svx/svdmodel.hxx ...

    CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong page count", static_cast<sal_uInt16>(3),
                                 pDoc->GetPageCount());

    const SdrPage* pPage = pDoc->GetPage(1);
    CPPUNIT_ASSERT_MESSAGE("no page", pPage != nullptr);

    CPPUNIT_ASSERT_MESSAGE("changed", !pDoc->IsChanged());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf131269)
{
    createSdImpressDoc("tdf131269.ppt");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 115
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(115), pDoc->GetPageCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN759180)
{
    createSdImpressDoc("n759180.pptx");

    const SdrPage* pPage = GetPage(1);

    // Get the object
    SdrObject* pObj = pPage->GetObj(0);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT(pTxtObj);
    std::vector<EECharAttrib> rLst;
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxULSpaceItem* pULSpace = aEdit.GetParaAttribs(0).GetItem(EE_PARA_ULSPACE);
    CPPUNIT_ASSERT(pULSpace);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Para bottom spacing is wrong!", static_cast<sal_uInt16>(0),
                                 pULSpace->GetLower());
    aEdit.GetCharAttribs(1, rLst);
    auto it = std::find_if(rLst.rbegin(), rLst.rend(), [](const EECharAttrib& rCharAttr) {
        return dynamic_cast<const SvxFontHeightItem*>(rCharAttr.pAttr) != nullptr;
    });
    if (it != rLst.rend())
    {
        const SvxFontHeightItem* pFontHeight = dynamic_cast<const SvxFontHeightItem*>((*it).pAttr);
        // nStart == 9
        // font height = 5 => 5*2540/72
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Font height is wrong", static_cast<sal_uInt32>(176),
                                     pFontHeight->GetHeight());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN862510_1)
{
    createSdImpressDoc("pptx/n862510_1.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        std::vector<EECharAttrib> rLst;
        SdrObject* pObj = pPage->GetObj(0);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT(pTxtObj);
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        for (std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend();
             ++it)
        {
            const SvxEscapementItem* pFontEscapement
                = dynamic_cast<const SvxEscapementItem*>((*it).pAttr);
            CPPUNIT_ASSERT_MESSAGE(
                "Baseline attribute not handled properly",
                !(pFontEscapement && pFontEscapement->GetProportionalHeight() != 100));
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN862510_2)
{
    createSdImpressDoc("pptx/n862510_2.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
        SdrObjGroup* pGrpObj = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pGrpObj);
        SdrObjCustomShape* pObj
            = dynamic_cast<SdrObjCustomShape*>(pGrpObj->GetSubList()->GetObj(1));
        CPPUNIT_ASSERT(pObj);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Text Rotation!", 90.0,
                                     pObj->GetExtraTextRotation(false));
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN862510_4)
{
    createSdImpressDoc("pptx/n862510_4.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        std::vector<EECharAttrib> rLst;
        SdrObject* pObj = pPage->GetObj(0);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT(pTxtObj);
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs(0, rLst);
        for (std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend();
             ++it)
        {
            const SvxColorItem* pC = dynamic_cast<const SvxColorItem*>((*it).pAttr);
            CPPUNIT_ASSERT_MESSAGE("gradfill for text color not handled!",
                                   !(pC && pC->GetValue() == COL_BLACK));
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN828390_2)
{
    createSdImpressDoc("pptx/n828390_2.pptx");
    const SdrPage* pPage = GetPage(1);

    SdrObject* pObj = pPage->GetObj(0);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT(pTxtObj);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(u"Linux  "_ustr, aEdit.GetText(0));
    CPPUNIT_ASSERT_EQUAL(u"Standard Platform"_ustr, aEdit.GetText(1));
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN828390_3)
{
    createSdImpressDoc("pptx/n828390_3.pptx");
    const SdrPage* pPage = GetPage(1);

    SdrObject* pObj = pPage->GetObj(0);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT(pTxtObj);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(1, rLst);
    bool bPassed = std::none_of(rLst.rbegin(), rLst.rend(), [](const EECharAttrib& rCharAttr) {
        const SvxEscapementItem* pFontEscapement
            = dynamic_cast<const SvxEscapementItem*>(rCharAttr.pAttr);
        return pFontEscapement && (pFontEscapement->GetEsc() != 0);
    });
    CPPUNIT_ASSERT_MESSAGE("CharEscapment not imported properly", bPassed);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testMasterPageStyleParent)
{
    createSdImpressDoc("odp/masterpage_style_parent.odp");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    SdStyleSheetPool* const pPool(pDoc->GetSdStyleSheetPool());

    int parents(0);
    SfxStyleSheetIterator iter(pPool, SfxStyleFamily::Page);
    for (SfxStyleSheetBase* pStyle = iter.First(); pStyle; pStyle = iter.Next())
    {
        OUString const name(pStyle->GetName());
        OUString const parent(pStyle->GetParent());
        if (!parent.isEmpty())
        {
            ++parents;
            // check that parent exists
            SfxStyleSheetBase* const pParentStyle(pPool->Find(parent, SfxStyleFamily::Page));
            CPPUNIT_ASSERT(pParentStyle);
            CPPUNIT_ASSERT_EQUAL(pParentStyle->GetName(), parent);
            // check that parent has the same master page as pStyle
            CPPUNIT_ASSERT(parent.indexOf(SD_LT_SEPARATOR) != -1);
            CPPUNIT_ASSERT(name.indexOf(SD_LT_SEPARATOR) != -1);
            CPPUNIT_ASSERT_EQUAL(parent.copy(0, parent.indexOf(SD_LT_SEPARATOR)),
                                 name.copy(0, name.indexOf(SD_LT_SEPARATOR)));
        }
    }
    // check that there are actually parents...
    CPPUNIT_ASSERT_EQUAL(16, parents);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
