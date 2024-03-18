/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_poppler.h>

#include "sdmodeltestbase.hxx"

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/numitem.hxx>

#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdasitm.hxx>
#include <svx/sdmetitm.hxx>
#include <animations/animationnodehelper.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

#include <stlpool.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdograf.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace com::sun::star::uno
{
template <class T>
static std::ostream& operator<<(std::ostream& rStrm, const uno::Reference<T>& xRef)
{
    rStrm << xRef.get();
    return rStrm;
}
}

/// Impress import filters tests.
class SdImportTest : public SdModelTestBase
{
public:
    SdImportTest()
        : SdModelTestBase("/sd/qa/unit/data/")
    {
    }
};

/** Test document against a reference XML dump of shapes.

If you want to update one of these tests, or add a new one, set the nUpdateMe
to the index of the test, and the dump XML's will be created (or rewritten)
instead of checking. Use with care - when the test is failing, first find out
why, instead of just updating .xml's blindly.

Example: Let's say you are adding a test called fdoABCD.pptx.  You'll place it
to the data/ subdirectory, and will add an entry to aFilesToCompare below,
the 3rd parameter is for export test - can be -1 (don't export), "impress8", "MS PowerPoint 97" or "Impress Office Open XML"
like:

        { "fdoABCD.pptx", "xml/fdoABCD_", "Impress Office Open XML" },

and will count the index in the aFilesToCompare structure (1st is 0, 2nd is 1,
etc.)  Temporarily you'll set nUpdateMe to this index (instead of -1), and run

make sd

This will generate the sd/qa/unit/data/xml/fdoABCD_*.xml for you.  Now you
will change nUpdateMe back to -1, and commit your fdoABCD.pptx test, the
xml/fdoABCD_*.xml dumps, and the aFilesToCompare addition in one commit.

As the last step, you will revert your fix and do 'make sd' again, to check
that without your fix, the unit test breaks.  Then clean up, and push :-)

NOTE: This approach is suitable only for tests of fixes that actually change
the layout - best to check by reverting your fix locally after having added
the test, and re-running; it should break.
*/
CPPUNIT_TEST_FIXTURE(SdImportTest, testDocumentLayout)
{
    static const struct
    {
        std::u16string_view sInput, sDump;
        OUString sExportType;
    } aFilesToCompare[]
        = { { u"odp/shapes-test.odp", u"xml/shapes-test_page", u"" },
            { u"fdo47434.pptx", u"xml/fdo47434_", u"" },
            { u"n758621.ppt", u"xml/n758621_", u"" },
            { u"fdo64586.ppt", u"xml/fdo64586_", u"" },

            // needed to adapt this, the border parameter is no longer
            // exported with MCGRs due to oox neither needing nor
            // supporting it with now freely definable gradients
            { u"n819614.pptx", u"xml/n819614_", u"" },

            { u"n820786.pptx", u"xml/n820786_", u"" },
            { u"n762695.pptx", u"xml/n762695_", u"" },
            { u"n593612.pptx", u"xml/n593612_", u"" },
            { u"fdo71434.pptx", u"xml/fdo71434_", u"" },
            { u"n902652.pptx", u"xml/n902652_", u"" },
            { u"tdf90403.pptx", u"xml/tdf90403_", u"" },
            { u"tdf90338.odp", u"xml/tdf90338_", u"Impress Office Open XML" },
            { u"tdf92001.odp", u"xml/tdf92001_", u"Impress Office Open XML" },
// GCC -mfpmath=387 rounding issues in lclPushMarkerProperties
// (oox/source/drawingml/lineproperties.cxx); see mail sub-thread starting at
// <https://lists.freedesktop.org/archives/libreoffice/2016-September/
// 075211.html> "Re: Test File: sc/qa/unit/data/functions/fods/chiinv.fods:
// fails with Assertion" for how "-mfpmath=sse -msse2" would fix that:
#if !(defined LINUX && defined X86)
            { u"tdf100491.pptx", u"xml/tdf100491_", u"" },
#endif
            { u"tdf109317.pptx", u"xml/tdf109317_", u"impress8" },
            // { u"pptx/n828390.pptx", u"pptx/xml/n828390_", "Impress Office Open XML" }, // Example
          };

    for (int i = 0; i < static_cast<int>(SAL_N_ELEMENTS(aFilesToCompare)); ++i)
    {
        int const nUpdateMe
            = -1; // index of test we want to update; supposedly only when the test is created

        loadFromURL(aFilesToCompare[i].sInput);
        if (!aFilesToCompare[i].sExportType.isEmpty())
            saveAndReload(aFilesToCompare[i].sExportType);
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                       uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();
        CPPUNIT_ASSERT(xDrawPages.is());

        sal_Int32 nLength = xDrawPages->getCount();
        for (sal_Int32 j = 0; j < nLength; ++j)
        {
            uno::Reference<drawing::XDrawPage> xDrawPage;
            uno::Any aAny = xDrawPages->getByIndex(j);
            aAny >>= xDrawPage;
            uno::Reference<drawing::XShapes> xShapes(xDrawPage, uno::UNO_QUERY_THROW);
            OUString aString = XShapeDumper::dump(xShapes);

            OString aFileName
                = OUStringToOString(createFileURL(aFilesToCompare[i].sDump), RTL_TEXTENCODING_UTF8)
                  + OString::number(j) + ".xml";

            if (nUpdateMe == i) // index was wrong here
            {
                // had to adapt this, std::ofstream annot write to an URL but needs a
                // filesystem path. Seems as if no one had to adapt any of the cases
                // for some years :-/
                OUString sTempFilePath;
                osl::FileBase::getSystemPathFromFileURL(OUString::fromUtf8(aFileName),
                                                        sTempFilePath);
                std::ofstream aStream(sTempFilePath.toUtf8().getStr(),
                                      std::ofstream::out | std::ofstream::binary);
                aStream << aString;
                aStream.close();
            }
            else
            {
                doXMLDiff(aFileName.getStr(),
                          OUStringToOString(aString, RTL_TEXTENCODING_UTF8).getStr(),
                          static_cast<int>(aString.getLength()),
                          OUStringToOString(createFileURL(u"tolerance.xml"), RTL_TEXTENCODING_UTF8)
                              .getStr());
            }
        }
    }
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
    xRunPropSet->getPropertyValue("CharColor") >>= nCharColor;
    xRunPropSet->getPropertyValue("CharWeight") >>= nFontWeight;
    xCellPropSet->getPropertyValue("BottomBorder") >>= aBorderLine;
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The bottom border is missing!", true, aBorderLine.LineWidth > 0);

    xCellPropSet.set(xTable->getCellByPosition(1, 0), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue("CharColor") >>= nCharColor;
    xRunPropSet->getPropertyValue("CharWeight") >>= nFontWeight;
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);

    xCellPropSet.set(xTable->getCellByPosition(2, 0), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue("CharColor") >>= nCharColor;
    xRunPropSet->getPropertyValue("CharWeight") >>= nFontWeight;
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);

    xCellPropSet.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue("CharColor") >>= nCharColor;
    xRunPropSet->getPropertyValue("CharWeight") >>= nFontWeight;
    xCellPropSet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nFillColor);

    xCellPropSet.set(xTable->getCellByPosition(2, 1), uno::UNO_QUERY_THROW);
    xParagraph.set(getParagraphFromShape(0, xCellPropSet));
    xRun.set(getRunFromParagraph(0, xParagraph));
    xRunPropSet.set(xRun, uno::UNO_QUERY_THROW);
    xRunPropSet->getPropertyValue("CharColor") >>= nCharColor;
    xRunPropSet->getPropertyValue("CharWeight") >>= nFontWeight;
    xCellPropSet->getPropertyValue("FillColor") >>= nFillColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x000000), nCharColor);
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, nFontWeight);
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nFillColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFreeformShapeGluePoints)
{
    createSdImpressDoc("pptx/tdf156829.pptx");
    uno::Reference<beans::XPropertySet> xFreeformShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aProps;
    xFreeformShape->getPropertyValue("CustomShapeGeometry") >>= aProps;

    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : std::as_const(aProps))
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
    for (beans::PropertyValue const& rProp : std::as_const(aPathProps))
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
        nGlueId = xConnector1->getPropertyValue("StartGluePointIndex").get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nGlueId);
        nGlueId = xConnector2->getPropertyValue("EndGluePointIndex").get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nGlueId);
    }

    saveAndReload("Impress MS PowerPoint 2007 XML");
    {
        uno::Reference<beans::XPropertySet> xConnector1(getShapeFromPage(1, 0), uno::UNO_SET_THROW);
        uno::Reference<beans::XPropertySet> xConnector2(getShapeFromPage(3, 0), uno::UNO_SET_THROW);
        nGlueId = xConnector1->getPropertyValue("StartGluePointIndex").get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nGlueId);
        nGlueId = xConnector2->getPropertyValue("EndGluePointIndex").get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nGlueId);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf153466)
{
    createSdImpressDoc("pptx/tdf153466.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPageSet(xPage, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xBackground(
        xPageSet->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());

    com::sun::star::drawing::RectanglePoint aRectanglePoint;
    xBackground->getPropertyValue("FillBitmapRectanglePoint") >>= aRectanglePoint;
    CPPUNIT_ASSERT_EQUAL(drawing::RectanglePoint::RectanglePoint_RIGHT_BOTTOM, aRectanglePoint);

    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0), uno::UNO_SET_THROW);
    xShape->getPropertyValue("FillBitmapRectanglePoint") >>= aRectanglePoint;
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

CPPUNIT_TEST_FIXTURE(SdImportTest, testStandardConnectors)
{
    createSdImpressDoc("pptx/standardConnectors.pptx");

    sal_Int32 aEdgeValue[] = { -1352, -2457, 3977, -2900, -1261, 4611, -1431, -2643, 3830, 3438 };

    sal_Int32 nCount = 0;
    sal_Int32 nEdgeLine = 0;
    for (size_t i = 0; i < 10; i++)
    {
        uno::Reference<beans::XPropertySet> xConnector(getShapeFromPage(i, 0));
        bool bConnector = xConnector->getPropertySetInfo()->hasPropertyByName("EdgeKind");
        if (bConnector)
        {
            nEdgeLine = xConnector->getPropertyValue("EdgeLine1Delta").get<sal_Int32>();
            CPPUNIT_ASSERT_EQUAL(aEdgeValue[nCount], nEdgeLine);
            nCount++;

            nEdgeLine = xConnector->getPropertyValue("EdgeLine2Delta").get<sal_Int32>();
            if (nEdgeLine != 0)
            {
                CPPUNIT_ASSERT_EQUAL(aEdgeValue[nCount], nEdgeLine);
                nCount++;
            }
        }
    }
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
        bool bConnector = xConnector->getPropertySetInfo()->hasPropertyByName("EdgeLine1Delta");
        if (bConnector)
        {
            sal_Int32 nEdgeLine = xConnector->getPropertyValue("EdgeLine1Delta").get<sal_Int32>();
            CPPUNIT_ASSERT_EQUAL(aEdgeValue[nCount], nEdgeLine);
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
    CPPUNIT_ASSERT(xConnector->getPropertySetInfo()->hasPropertyByName("EdgeLine1Delta"));

    sal_Int32 nEdgeLine = xConnector->getPropertyValue("EdgeLine1Delta").get<sal_Int32>();
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
    xPropSet1->getPropertyValue("CharUnderline") >>= nUnderline;
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
    xPropSet1->getPropertyValue("TextField") >>= xField1;
    xPropSet1.set(xField1, uno::UNO_QUERY);
    xPropSet1->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("#Slide 1"), aURL);

    uno::Reference<text::XTextRange> const xParagraph2(getParagraphFromShape(1, xShape));
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(0, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextField> xField2;
    xPropSet2->getPropertyValue("TextField") >>= xField2;
    xPropSet2.set(xField2, uno::UNO_QUERY);
    xPropSet2->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("#Slide 3"), aURL);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf149124)
{
    createSdImpressDoc("pptx/tdf149124.pptx");

    uno::Reference<container::XIndexAccess> xGroupShape(getShapeFromPage(0, 0),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xStandardConnector(xGroupShape->getByIndex(2),
                                                           uno::UNO_QUERY_THROW);

    sal_Int32 nStartGlueId
        = xStandardConnector->getPropertyValue("StartGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), nStartGlueId);
    sal_Int32 nEndGlueId
        = xStandardConnector->getPropertyValue("EndGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nEndGlueId);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf148965)
{
    // Set the system user interface to Hungarian
    SvtSysLocaleOptions aOptions;
    OUString sUIConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetUILocaleConfigString("hu-HU");
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
    xEvents1->getByName("OnClick") >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("page1"), iter1->second.get<OUString>());

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName("OnClick") >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("page3"), iter2->second.get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf89449)
{
    createSdImpressDoc("pptx/tdf89449.pptx");

    sal_Int32 nStartGlueId;
    sal_Int32 nEndGlueId;
    css::drawing::ConnectorType aConnectorType;

    uno::Reference<beans::XPropertySet> xCurvedConnector(getShapeFromPage(3, 0));
    xCurvedConnector->getPropertyValue("EdgeKind") >>= aConnectorType;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_CURVE, aConnectorType);
    nStartGlueId = xCurvedConnector->getPropertyValue("StartGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nStartGlueId);
    nEndGlueId = xCurvedConnector->getPropertyValue("EndGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nEndGlueId);

    uno::Reference<beans::XPropertySet> xStraightConnector(getShapeFromPage(4, 0));
    xStraightConnector->getPropertyValue("EdgeKind") >>= aConnectorType;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_LINE, aConnectorType);
    nStartGlueId = xStraightConnector->getPropertyValue("StartGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), nStartGlueId);
    nEndGlueId = xStraightConnector->getPropertyValue("EndGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nEndGlueId);

    uno::Reference<beans::XPropertySet> xStandardConnector(getShapeFromPage(5, 0));
    xStandardConnector->getPropertyValue("EdgeKind") >>= aConnectorType;
    CPPUNIT_ASSERT_EQUAL(drawing::ConnectorType::ConnectorType_STANDARD, aConnectorType);
    nStartGlueId = xStandardConnector->getPropertyValue("StartGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), nStartGlueId);
    nEndGlueId = xStandardConnector->getPropertyValue("EndGluePointIndex").get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), nEndGlueId);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf147459)
{
    createSdImpressDoc("pptx/tdf147459.pptx");
    uno::Reference<beans::XPropertySet> xTriangleShape(getShapeFromPage(0, 0));
    uno::Sequence<beans::PropertyValue> aProps;
    xTriangleShape->getPropertyValue("CustomShapeGeometry") >>= aProps;

    uno::Sequence<beans::PropertyValue> aPathProps;
    for (beans::PropertyValue const& rProp : std::as_const(aProps))
    {
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get<uno::Sequence<beans::PropertyValue>>();
    }

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
    for (beans::PropertyValue const& rProp : std::as_const(aPathProps))
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
    xSet->getPropertyValue("IsBackgroundObjectsVisible") >>= bBackgroundObjectsVisible;
    CPPUNIT_ASSERT_EQUAL(false, bBackgroundObjectsVisible);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf144918)
{
    createSdImpressDoc("pptx/tdf144918.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(0, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName("OnClick") >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("First slide"), iter1->second.get<OUString>());

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName("OnClick") >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("Third slide"), iter2->second.get<OUString>());
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
    xEvents->getByName("OnClick") >>= props;
    comphelper::SequenceAsHashMap map(props);
    auto iter(map.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/"), iter->second.get<OUString>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testHyperlinkOnImage)
{
    createSdImpressDoc("pptx/hyperlinkOnImage.pptx");

    uno::Reference<beans::XPropertySet> xShape1(getShapeFromPage(1, 0));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName("OnClick") >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find("ClickAction"));
    CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_LASTPAGE,
                         iter1->second.get<css::presentation::ClickAction>());

    uno::Reference<beans::XPropertySet> xShape2(getShapeFromPage(1, 1));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName("OnClick") >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find("ClickAction"));
    CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_NONE,
                         iter2->second.get<css::presentation::ClickAction>());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142645)
{
    createSdImpressDoc("pptx/tdf142645.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed1(xPage1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xNamed1->getName());
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
        xEvents->getByName("OnClick") >>= props;
        comphelper::SequenceAsHashMap map(props);
        auto iter(map.find("ClickAction"));
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
                auto iter1(map.find("Bookmark"));
                CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/"),
                                     iter1->second.get<OUString>());
            }
            break;
            case 4:
            {
                auto iter2(map.find("Bookmark"));
                CPPUNIT_ASSERT_EQUAL(OUString("End Show"), iter2->second.get<OUString>());
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

    bool bChangeManually = xPresentationProps->getPropertyValue("IsAutomatic").get<bool>();

    CPPUNIT_ASSERT_EQUAL(true, bChangeManually);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142913)
{
    createSdImpressDoc("pptx/tdf142913.pptx");

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    OUString sFirstPage = xPresentationProps->getPropertyValue("FirstPage").get<OUString>();

    CPPUNIT_ASSERT_EQUAL(OUString("Second"), sFirstPage);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf142590)
{
    createSdImpressDoc("pptx/tdf142590.pptx");

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(mxComponent,
                                                                              uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);
    const OUString sCustomShowId
        = xPresentationProps->getPropertyValue("CustomShow").get<OUString>();

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
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("The text field is missing!", xField.is());

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("#Slide2"), aURL);
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
    xPropSet1->getPropertyValue("CharColor") >>= nCharColorBlue;

    // Get run of the second paragraph
    uno::Reference<text::XTextRange> xRun2(getRunFromParagraph(1, xParagraph2));
    uno::Reference<beans::XPropertySet> xPropSet2(xRun2, uno::UNO_QUERY_THROW);
    Color nCharColorRed;
    xPropSet2->getPropertyValue("CharColor") >>= nCharColorRed;

    // Get run of the third paragraph
    uno::Reference<text::XTextRange> xRun3(getRunFromParagraph(2, xParagraph3));
    uno::Reference<beans::XPropertySet> xPropSet3(xRun3, uno::UNO_QUERY_THROW);
    Color nCharColorGreen;
    xPropSet3->getPropertyValue("CharColor") >>= nCharColorGreen;

    // Hyperlink colors should be blue, red, green.
    CPPUNIT_ASSERT_EQUAL(Color(0x4472c4), nCharColorBlue);
    CPPUNIT_ASSERT_EQUAL(Color(0xff0000), nCharColorRed);
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
                                   !(pC && pC->GetValue() == Color(0)));
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
    CPPUNIT_ASSERT_EQUAL(OUString("Linux  "), aEdit.GetText(0));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard Platform"), aEdit.GetText(1));
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

CPPUNIT_TEST_FIXTURE(SdImportTest, testGradientAngle)
{
    createSdDrawDoc("odg/gradient-angle.fodg");

    uno::Reference<lang::XMultiServiceFactory> const xDoc(mxComponent, uno::UNO_QUERY);

    awt::Gradient gradient;
    uno::Reference<container::XNameAccess> const xGradients(
        xDoc->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY);

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 38") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), gradient.Angle); // was: 3600

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 10") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(270), gradient.Angle); // 27deg

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 11") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1145), gradient.Angle); // 2rad

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 12") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 100grad

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 13") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3599), gradient.Angle); // -1

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 14") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3028), gradient.Angle); // -1rad

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 15") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(300), gradient.Angle); // 3900

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 16") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(105), gradient.Angle); // 10.5deg

    CPPUNIT_ASSERT(xGradients->getByName("Gradient 17") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // \pi rad

    uno::Reference<container::XNameAccess> const xTranspGradients(
        xDoc->createInstance("com.sun.star.drawing.TransparencyGradientTable"), uno::UNO_QUERY);

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 2") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), gradient.Angle); // 1

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 1") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 90deg

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 3") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(572), gradient.Angle); // 1.0rad

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 4") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // 1000grad
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testN778859)
{
    createSdImpressDoc("pptx/n778859.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        // Get the object
        SdrObject* pObj = pPage->GetObj(1);
        SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
        CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT(!pTxtObj->IsAutoFit());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo68594)
{
    createSdImpressDoc("ppt/fdo68594.ppt");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    SdrObject* pObj = pPage->GetObj(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const SvxColorItem* pC = &pTxtObj->GetMergedItem(EE_CHAR_COLOR);
    CPPUNIT_ASSERT_MESSAGE("no color item", pC != nullptr);
    // Color should be black
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder color mismatch", COL_BLACK, pC->GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testPlaceholderPriority)
{
    createSdImpressDoc("ppt/placeholder-priority.pptx");

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Missing placeholder", sal_uInt32(2),
                                 sal_uInt32(pPage->GetObjCount()));

    tools::Rectangle pObj1Rect(9100, 3500, 29619, 4038);
    SdrObject* pObj1 = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder position is wrong, check the placeholder priority",
                                 pObj1Rect, pObj1->GetCurrentBoundRect());

    tools::Rectangle pObj2Rect(9102, 8643, 29619, 12642);
    SdrObject* pObj2 = pPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder position is wrong, check the placeholder priority",
                                 pObj2Rect, pObj2->GetCurrentBoundRect());

    // If the placeholder positions are wrong, please check placeholder priority in Placeholders class.
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo72998)
{
    createSdImpressDoc("pptx/cshapes.pptx");

    const SdrPage* pPage = GetPage(1);
    {
        SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(2));
        CPPUNIT_ASSERT(pObj);
        const SdrCustomShapeGeometryItem& rGeometryItem
            = pObj->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
        const css::uno::Any* pViewBox = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem)
                                            .GetPropertyValueByName(OUString("ViewBox"));
        CPPUNIT_ASSERT_MESSAGE("Missing ViewBox", pViewBox);
        css::awt::Rectangle aViewBox;
        CPPUNIT_ASSERT((*pViewBox >>= aViewBox));
        CPPUNIT_ASSERT_MESSAGE("Width should be zero - for forcing scale to 1", !aViewBox.Width);
        CPPUNIT_ASSERT_MESSAGE("Height should be zero - for forcing scale to 1", !aViewBox.Height);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo77027)
{
    createSdImpressDoc("odp/fdo77027.odp");

    const SdrPage* pPage = GetPage(1);
    {
        SdrOle2Obj* const pObj = dynamic_cast<SdrOle2Obj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObj);

        // check that the fill style/color was actually imported
        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0xff6600), rColorItem.GetColorValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf97808)
{
    createSdImpressDoc("tdf97808.fodp");

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"),
                                                        uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("objectwithoutfill"),
                                               uno::UNO_QUERY);
    OUString lineend;
    CPPUNIT_ASSERT(xStyle->getPropertyValue("LineEndName") >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString("Arrow"), lineend);

    // the draw:marker-end="" did not override the style
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xLine(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    //uno::Reference<style::XStyle> xParent;
    uno::Reference<beans::XPropertySet> xParent;
    CPPUNIT_ASSERT(xLine->getPropertyValue("Style") >>= xParent);
    CPPUNIT_ASSERT_EQUAL(xStyle, xParent);
    CPPUNIT_ASSERT(xLine->getPropertyValue("LineEndName") >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString(), lineend);
}
CPPUNIT_TEST_FIXTURE(SdImportTest, testFillStyleNone)
{
    createSdDrawDoc("tdf123841.odg");

    const SdrPage* pPage = GetPage(1);
    const SdrObject* pObj = pPage->GetObj(0);
    auto& rFillStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the shape's fill was FillStyle_SOLID, making the text of the shape unreadable.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo64512)
{
    createSdImpressDoc("fdo64512.odp");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly one page", static_cast<sal_Int32>(1),
                                 xDoc->getDrawPages()->getCount());

    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not exactly three shapes", static_cast<sal_Int32>(3),
                                 xPage->getCount());

    uno::Reference<beans::XPropertySet> xConnectorShape(xPage->getByIndex(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no connector shape", xConnectorShape.is());

    uno::Reference<beans::XPropertySet> xSvgShape(xConnectorShape->getPropertyValue("StartShape"),
                                                  uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no start shape", xSvgShape.is());

    uno::Reference<beans::XPropertySet> xCustomShape(xConnectorShape->getPropertyValue("EndShape"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no end shape", xCustomShape.is());

    uno::Reference<animations::XAnimationNodeSupplier> xAnimNodeSupplier(xPage,
                                                                         uno::UNO_QUERY_THROW);
    uno::Reference<animations::XAnimationNode> xRootNode(xAnimNodeSupplier->getAnimationNode());
    std::vector<uno::Reference<animations::XAnimationNode>> aAnimVector;
    anim::create_deep_vector(xRootNode, aAnimVector);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("not 8 animation nodes", static_cast<std::size_t>(8),
                                 aAnimVector.size());

    uno::Reference<animations::XAnimate> xNode(aAnimVector[7], uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xTargetShape(xNode->getTarget(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("inner node not referencing svg shape", xTargetShape != xSvgShape);
}

// Unit test for importing charts
CPPUNIT_TEST_FIXTURE(SdImportTest, testFdo71075)
{
    double values[] = { 12.0, 13.0, 14.0 };
    css::uno::Any aAny;

    createSdImpressDoc("fdo71075.odp");

    uno::Reference<beans::XPropertySet> xPropSet(getShapeFromPage(0, 0));
    aAny = xPropSet->getPropertyValue("Model");
    CPPUNIT_ASSERT_MESSAGE("The shape doesn't have the property", aAny.hasValue());

    uno::Reference<chart::XChartDocument> xChartDoc;
    aAny >>= xChartDoc;
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChartDoc.is());
    uno::Reference<chart2::XChartDocument> xChart2Doc(xChartDoc, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load chart", xChart2Doc.is());

    uno::Reference<chart2::XCoordinateSystemContainer> xBCooSysCnt(xChart2Doc->getFirstDiagram(),
                                                                   uno::UNO_QUERY);
    uno::Sequence<uno::Reference<chart2::XCoordinateSystem>> aCooSysSeq(
        xBCooSysCnt->getCoordinateSystems());
    uno::Reference<chart2::XChartTypeContainer> xCTCnt(aCooSysSeq[0], uno::UNO_QUERY);

    uno::Reference<chart2::XDataSeriesContainer> xDSCnt(xCTCnt->getChartTypes()[0], uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("failed to load data series", xDSCnt.is());
    uno::Sequence<uno::Reference<chart2::XDataSeries>> aSeriesSeq(xDSCnt->getDataSeries());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", static_cast<sal_Int32>(1),
                                 aSeriesSeq.getLength());
    uno::Reference<chart2::data::XDataSource> xSource(aSeriesSeq[0], uno::UNO_QUERY);
    uno::Sequence<uno::Reference<chart2::data::XLabeledDataSequence>> aSeqCnt(
        xSource->getDataSequences());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", static_cast<sal_Int32>(1),
                                 aSeqCnt.getLength());
    uno::Reference<chart2::data::XDataSequence> xValueSeq(aSeqCnt[0]->getValues());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Data count",
                                 static_cast<sal_Int32>(SAL_N_ELEMENTS(values)),
                                 xValueSeq->getData().getLength());
    uno::Reference<chart2::data::XNumericalDataSequence> xNumSeq(xValueSeq, uno::UNO_QUERY);
    uno::Sequence<double> aValues(xNumSeq->getNumericalData());
    for (sal_Int32 i = 0; i < xValueSeq->getData().getLength(); i++)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Series count", values[i], aValues.getConstArray()[i]);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testStrictOOXML)
{
    createSdImpressDoc("strict_ooxml.pptx");

    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT_MESSAGE("no document", pDoc != nullptr);
    getShapeFromPage(0, 0);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc870237)
{
    createSdImpressDoc("pptx/bnc870237.pptx");
    const SdrPage* pPage = GetPage(1);

    // Simulate a:ext inside dsp:txXfrm with changing the lower distance
    const SdrObjGroup* pGroupObj = dynamic_cast<SdrObjGroup*>(pPage->GetObj(0));
    const SdrObject* pObj = pGroupObj->GetSubList()->GetObj(1);
    CPPUNIT_ASSERT_MESSAGE("no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-158), pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9760), pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-158), pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-158), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testCreationDate)
{
    createSdImpressDoc("fdo71434.pptx");
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    util::DateTime aDate = xDocumentProperties->getCreationDate();
    OUStringBuffer aBuffer;
    sax::Converter::convertDateTime(aBuffer, aDate, nullptr);
    // Metadata wasn't imported, this was 0000-00-00.
    CPPUNIT_ASSERT_EQUAL(OUString("2013-11-09T10:37:56"), aBuffer.makeStringAndClear());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testMultiColTexts)
{
    createSdImpressDoc("pptx/multicol.pptx");
    const SdrPage* pPage = GetPage(1);

    auto pTextObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTextObj);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pTextObj->GetTextColumnsSpacing());

    auto pMasterTextObj = DynCastSdrTextObj(pPage->TRG_GetMasterPage().GetObj(0));
    CPPUNIT_ASSERT(pMasterTextObj);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pMasterTextObj->GetTextColumnsNumber());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pMasterTextObj->GetTextColumnsSpacing());

    uno::Reference<text::XTextRange> xText(pMasterTextObj->getUnoShape(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("mastershape1\nmastershape2"), xText->getString());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testPredefinedTableStyle)
{
    // 073A0DAA-6AF3-43AB-8588-CEC1D06C72B9 (Medium Style 2)
    createSdImpressDoc("pptx/predefined-table-style.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xcccccc), nColor);

    xCell.set(xTable->getCellByPosition(0, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xe7e7e7), nColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc887225)
{
    createSdImpressDoc("pptx/bnc887225.pptx");
    // In the document, lastRow and lastCol table properties are used.
    // Make sure styles are set properly for individual cells.

    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(0, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(1, 1), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xd1deef), nColor);

    xCell.set(xTable->getCellByPosition(1, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xe9eff7), nColor);

    xCell.set(xTable->getCellByPosition(1, 4), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(3, 2), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);

    xCell.set(xTable->getCellByPosition(3, 4), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x5b9bd5), nColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc584721_1)
{
    // Title text shape on the master page contained wrong text.

    createSdImpressDoc("pptx/bnc584721_1_2.pptx");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    SdrObject* pObj = pPage->GetObj(0);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pObj);
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Click to edit Master title style"), aEdit.GetText(0));
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc584721_2)
{
    // Import created an extra/unneeded outliner shape on the master slide next to the imported title shape.

    createSdImpressDoc("pptx/bnc584721_1_2.pptx");

    const SdrPage* pPage = &(GetPage(1)->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc591147)
{
    createSdImpressDoc("pptx/bnc591147.pptx");

    // In the document, there are two slides with media files.
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDoc->getDrawPages()->getCount());

    // First page has video file inserted
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    //uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference<beans::XPropertySet> xPropSet(getShape(0, xPage));
    OUString sVideoURL("emptyURL");
    bool bSuccess = xPropSet->getPropertyValue("MediaURL") >>= sVideoURL;
    CPPUNIT_ASSERT_MESSAGE("MediaURL property is not set", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sVideoURL.isEmpty());

    // Second page has audio file inserted
    xPage.set(xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xPage->getCount());

    xPropSet.set(getShape(0, xPage));
    OUString sAudioURL("emptyURL");
    bSuccess = xPropSet->getPropertyValue("MediaURL") >>= sAudioURL;
    CPPUNIT_ASSERT_MESSAGE("MediaURL property is not set", bSuccess);
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sAudioURL.isEmpty());

    CPPUNIT_ASSERT_MESSAGE("sAudioURL and sVideoURL should not be equal", sAudioURL != sVideoURL);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc584721_4)
{
    // Black text was imported as white because of wrong caching mechanism

    createSdImpressDoc("pptx/bnc584721_4.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, 1));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;

    // Color should be black
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc904423)
{
    // Here the problem was that different fill properties were applied in wrong order on the shape
    // Right order: 1) master slide fill style, 2) theme, 3) direct formatting

    createSdImpressDoc("pptx/bnc904423.pptx");

    const SdrPage* pPage = GetPage(1);
    // First shape's background color is defined on master slide
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x00CC99), rColorItem.GetColorValue());
    }

    // Second shape's background color is defined by theme
    {
        SdrObject* const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x3333CC), rColorItem.GetColorValue());
    }

    // Third shape's background color is defined by direct formatting
    {
        SdrObject* const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_FILLSTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = pObj->GetMergedItem(XATTR_FILLCOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), rColorItem.GetColorValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testShapeLineStyle)
{
    // Here the problem was that different line properties were applied in wrong order on the shape
    // Right order: 1) master slide line style, 2) theme, 3) direct formatting
    createSdImpressDoc("pptx/ShapeLineProperties.pptx");

    const SdrPage* pPage = GetPage(1);
    // First shape's line style is defined on master slide
    {
        SdrObject* const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = pObj->GetMergedItem(XATTR_LINEWIDTH);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(132), rWidthItem.GetValue());
    }

    // Second shape's line style is defined by theme
    {
        SdrObject* const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x3333CC), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = pObj->GetMergedItem(XATTR_LINEWIDTH);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), rWidthItem.GetValue());
    }

    // Third shape's line style is defined by direct formatting
    {
        SdrObject* const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = pObj->GetMergedItem(XATTR_LINESTYLE);
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = pObj->GetMergedItem(XATTR_LINECOLOR);
        CPPUNIT_ASSERT_EQUAL(Color(0x7030A0), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = pObj->GetMergedItem(XATTR_LINEWIDTH);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(176), rWidthItem.GetValue());
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTableBorderLineStyle)
{
    createSdImpressDoc("pptx/tableBorderLineStyle.pptx");

    // TODO: If you are working on improvement of table border line style
    // support, then expect this unit test to fail.

    const sal_Int16 nObjBorderLineStyles[]
        = { ::table::BorderLineStyle::DASHED,   ::table::BorderLineStyle::DASH_DOT_DOT,
            ::table::BorderLineStyle::DASH_DOT, ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,   ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,   ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::DASH_DOT, ::table::BorderLineStyle::SOLID,
            ::table::BorderLineStyle::NONE };

    const SdrPage* pPage = GetPage(1);
    CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(nObjBorderLineStyles), pPage->GetObjCount());

    sdr::table::SdrTableObj* pTableObj;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    table::BorderLine2 aBorderLine;

    for (size_t i = 0; i < SAL_N_ELEMENTS(nObjBorderLineStyles); i++)
    {
        pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(i));
        CPPUNIT_ASSERT(pTableObj);
        xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
        xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
        xCell->getPropertyValue("TopBorder") >>= aBorderLine;
        if (aBorderLine.LineWidth > 0)
        {
            CPPUNIT_ASSERT_EQUAL(nObjBorderLineStyles[i], aBorderLine.LineStyle);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc862510_6)
{
    // Black text was imported instead of gray

    createSdImpressDoc("pptx/bnc862510_6.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);
    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;

    // Color should be gray
    CPPUNIT_ASSERT_EQUAL(Color(0x8B8B8B), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc862510_7)
{
    // Title shape's text was aligned to left instead of center.

    createSdImpressDoc("pptx/bnc862510_7.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<beans::XPropertySet> xPropSet(xParagraph, uno::UNO_QUERY_THROW);

    sal_Int16 nParaAdjust = 0;
    xPropSet->getPropertyValue("ParaAdjust") >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(nParaAdjust));
}

#if ENABLE_PDFIMPORT
// These tests use the old PDF-importing logic, which imports PDF elements as
// SD elements. This suffered many issues, and therefore wasn't ideal.
// The old PDF importer relied on an open-source project (xpdf) with an
// incompatible license (gpl), which has to be interfaced via an out-of-process
// library wrapper process. The resulting imported document was inaccurate
// and often very slow and with large memory footprint.
// Instead, PDFium offers state-of-the-art PDF importing logic,
// which is well-maintained and renders PDFs into images with high accuracy.
// So, the idea is to import PDFs as images using PDFium, which has a very
// high quality (and is much faster) than importing individual editable elements.
// So that's the "new" way of importing.
// The user then breaks the image to editable elements (which is not perfect,
// but very close to the old way), only if they need editing ability.
// PDFium should overall be better, and where it isn't, we just need to improve it.
// So these tests aren't really useful anymore. They should be modified to do
// import+break and then check the results. But that isn't straight-forward and
// currently await volunteering time to implement.

CPPUNIT_TEST_FIXTURE(SdImportTest, testPDFImportShared)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    comphelper::LibreOfficeKit::setActive();

    createSdDrawDoc("pdf/multipage.pdf");
    SdXImpressDocument* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();

    // This test is to verify that we share the PDF stream linked to each
    // Graphic instance in the imported document.
    // Since we import PDFs as images, we support attaching the original
    // PDF with each image to allow for advanced editing.
    // Here we iterate over all Graphic instances embedded in the pages
    // and verify that they all point to the same object in memory.
    std::vector<Graphic> aGraphics;

    for (int nPageIndex = 0; nPageIndex < pDoc->GetPageCount(); ++nPageIndex)
    {
        const SdrPage* pPage = GetPage(nPageIndex);
        if (pPage == nullptr)
            break;

        for (size_t nObjIndex = 0; nObjIndex < pPage->GetObjCount(); ++nObjIndex)
        {
            SdrObject* pObject = pPage->GetObj(nObjIndex);
            if (pObject == nullptr)
                continue;

            SdrGrafObj* pSdrGrafObj = dynamic_cast<SdrGrafObj*>(pObject);
            if (pSdrGrafObj == nullptr)
                continue;

            const GraphicObject& rGraphicObject = pSdrGrafObj->GetGraphicObject().GetGraphic();
            const Graphic& rGraphic = rGraphicObject.GetGraphic();
            CPPUNIT_ASSERT_MESSAGE(
                "After loading, the PDF shouldn't have the primitive sequence created yet",
                !rGraphic.getVectorGraphicData()->isPrimitiveSequenceCreated());
            aGraphics.push_back(rGraphic);
        }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected more than one page.", size_t(9), aGraphics.size());

    Graphic const& rFirstGraphic = aGraphics[0];

    for (size_t i = 0; i < aGraphics.size(); ++i)
    {
        Graphic const& rGraphic = aGraphics[i];
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Expected all PDF streams to be identical.",
            rFirstGraphic.getVectorGraphicData()->getBinaryDataContainer().getData(),
            rGraphic.getVectorGraphicData()->getBinaryDataContainer().getData());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected all GfxLinks to be identical.",
                                     rFirstGraphic.GetSharedGfxLink().get(),
                                     rGraphic.GetSharedGfxLink().get());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Page number doesn't match expected", sal_Int32(i),
                                     rGraphic.getPageNumber());
    }

    mxComponent->dispose();
    mxComponent.clear();

    comphelper::LibreOfficeKit::setActive(false);
}

#if defined(IMPORT_PDF_ELEMENTS)

CPPUNIT_TEST_FIXTURE(SdImportTest, testPDFImport)
{
    createSdDrawDoc("pdf/txtpic.pdf");
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("no exactly two shapes", static_cast<sal_Int32>(2),
                                 xPage->getCount());

    uno::Reference<beans::XPropertySet> xShape(getShape(0, xPage));
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE("not a text shape", xText.is());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testPDFImportSkipImages)
{
    auto pParams = std::make_shared<SfxAllItemSet>(SfxGetpApp()->GetPool());
    pParams->Put(SfxStringItem(SID_FILE_FILTEROPTIONS, "SkipImages"));

    createSdDrawDoc("pdf/txtpic.pdf"), PDF, pParams);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("no exactly one shape", static_cast<sal_Int32>(1),
                                 xPage->getCount());

    uno::Reference<drawing::XShape> xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE("not a text shape", xText.is());
}

#endif
#endif

CPPUNIT_TEST_FIXTURE(SdImportTest, testBulletSuffix)
{
    createSdImpressDoc("pptx/n83889.pptx");

    // check suffix of the char bullet
    const SdrPage* pPage = GetPage(1);
    SdrTextObj* pTxtObj = DynCastSdrTextObj(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem* pNumFmt = aEdit.GetParaAttribs(1).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bullet's suffix is wrong!",
                                 pNumFmt->GetNumRule().GetLevel(0).GetSuffix(), OUString());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testBnc910045)
{
    // Problem with table style which defines cell color with fill style

    createSdImpressDoc("pptx/bnc910045.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4f81bd), nColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testRowHeight)
{
    createSdImpressDoc("pptx/n80340.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    sal_Int32 nHeight;
    uno::Reference<css::table::XTable> xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XTableRows> xRows(xTable->getRows(), uno::UNO_SET_THROW);
    uno::Reference<beans::XPropertySet> xRefRow(xRows->getByIndex(0), uno::UNO_QUERY_THROW);
    xRefRow->getPropertyValue("Height") >>= nHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(508), nHeight);

    createSdImpressDoc("pptx/tablescale.pptx");
    const SdrPage* pPage2 = GetPage(1);

    sdr::table::SdrTableObj* pTableObj2 = dynamic_cast<sdr::table::SdrTableObj*>(pPage2->GetObj(0));
    CPPUNIT_ASSERT(pTableObj2);

    uno::Reference<css::table::XTable> xTable2(pTableObj2->getTable(), uno::UNO_SET_THROW);
    uno::Reference<css::table::XTableRows> xRows2(xTable2->getRows(), uno::UNO_SET_THROW);

    for (sal_Int32 nRow = 0; nRow < 7; ++nRow)
    {
        uno::Reference<beans::XPropertySet> xRefRow2(xRows2->getByIndex(nRow),
                                                     uno::UNO_QUERY_THROW);
        xRefRow2->getPropertyValue("Height") >>= nHeight;
        CPPUNIT_ASSERT_EQUAL(sal_Int32(800), nHeight);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf93830)
{
    // Text shape offset was ignored

    createSdImpressDoc("pptx/tdf93830.pptx");
    uno::Reference<drawing::XDrawPage> xPage(getPage(0));

    // Get the first text box from group shape
    uno::Reference<container::XIndexAccess> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xShape->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("no textbox shape", xPropSet.is());

    sal_Int32 nTextLeftDistance = 0;
    xPropSet->getPropertyValue("TextLeftDistance") >>= nTextLeftDistance;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4024), nTextLeftDistance);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf127129)
{
    createSdImpressDoc("pptx/tdf127129.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, nCharColor);

    // Without the accompanying fix in place, the highlight would be -1
    Color nCharBackColor;
    xPropSet->getPropertyValue("CharBackColor") >>= nCharBackColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xFF00), nCharBackColor);
}
CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf93097)
{
    // Throwing metadata import aborted the filter, check that metadata is now imported.

    createSdImpressDoc("pptx/tdf93097.pptx");
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("ss"), xDocumentProperties->getTitle());
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf62255)
{
    createSdImpressDoc("pptx/tdf62255.pptx");
    const SdrPage* pPage = GetPage(1);

    sdr::table::SdrTableObj* pTableObj;
    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTableObj);

    css::uno::Any aAny;
    uno::Reference<table::XCellRange> xTable;
    uno::Reference<beans::XPropertySet> xCell;
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    aAny = xCell->getPropertyValue("FillStyle");

    if (aAny.hasValue())
    {
        drawing::FillStyle aFillStyle;
        aAny >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, aFillStyle);
    }
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf89927)
{
    createSdImpressDoc("pptx/tdf89927.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf103800)
{
    createSdImpressDoc("pptx/tdf103800.pptx");
    uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(0, 0));
    uno::Reference<text::XTextRange> xParagraph(getParagraphFromShape(0, xShape));
    uno::Reference<text::XTextRange> xRun(getRunFromParagraph(0, xParagraph));
    uno::Reference<beans::XPropertySet> xPropSet(xRun, uno::UNO_QUERY_THROW);

    Color nCharColor;
    xPropSet->getPropertyValue("CharColor") >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL(Color(0xC00000), nCharColor);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf151767)
{
    createSdImpressDoc("pptx/tdf151767.pptx");

    const SdrPage* pPage = GetPage(1);
    sdr::table::SdrTableObj* pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    uno::Reference<table::XCellRange> xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByPosition(0, 0),
                                              uno::UNO_QUERY_THROW);

    table::BorderLine2 aLeft;
    xCell->getPropertyValue("LeftBorder") >>= aLeft;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The left border is missing!", true, aLeft.LineWidth > 0);

    table::BorderLine2 aRight;
    xCell->getPropertyValue("RightBorder") >>= aRight;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The right border is missing!", true, aRight.LineWidth > 0);

    table::BorderLine2 aTop;
    xCell->getPropertyValue("TopBorder") >>= aTop;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The top border is missing!", true, aTop.LineWidth > 0);

    table::BorderLine2 aBottom;
    xCell->getPropertyValue("BottomBorder") >>= aBottom;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The bottom border is missing!", true, aBottom.LineWidth > 0);
}

CPPUNIT_TEST_FIXTURE(SdImportTest, testTdf152070)
{
    createSdImpressDoc("pptx/tdf152070.pptx");

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropSet(xPage, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xBackgroundProps(
        xPropSet->getPropertyValue("Background").get<uno::Reference<beans::XPropertySet>>());

    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(50), // 50%
        xBackgroundProps->getPropertyValue("FillBitmapPositionOffsetX").get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(83), // 83%
        xBackgroundProps->getPropertyValue("FillBitmapPositionOffsetY").get<sal_Int32>());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
