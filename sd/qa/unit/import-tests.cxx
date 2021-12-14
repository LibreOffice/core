/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_poppler.h>
#include <memory>
#include <ostream>
#include <sdpage.hxx>

#include "sdmodeltestbase.hxx"

#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/unoprnms.hxx>
#include <svl/style.hxx>

#include <svx/svdotext.hxx>
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
#include <svx/sdooitm.hxx>
#include <animations/animationnodehelper.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XNumericalDataSequence.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>

#include <stlpool.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/graphicmimetype.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/dibtools.hxx>
#include <svx/svdograf.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace com::sun::star::uno {

template<class T>
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
    virtual void setUp() override;

    void testDocumentLayout();
    void testTdf146223();
    void testTdf144918();
    void testTdf144917();
    void testHyperlinkOnImage();
    void testTdf142645();
    void testTdf141704();
    void testTdf142915();
    void testTdf142913();
    void testTdf142590();
    void testCustomSlideShow();
    void testInternalHyperlink();
    void testHyperlinkColor();
    void testSmoketest();
    void testTdf131269();
    void testN759180();
    void testN778859();
    void testMasterPageStyleParent();
    void testGradientAngle();
    void testTdf97808();
    void testFillStyleNone();
    void testFdo64512();
    void testFdo71075();
    void testN828390_2();
    void testN828390_3();
    void testFdo68594();
    void testPlaceholderPriority();
    void testFdo72998();
    void testFdo77027();
    void testStrictOOXML();
    void testN862510_1();
    void testN862510_2();
    void testN862510_4();
    void testBnc870237();
    void testBnc887225();
    void testPredefinedTableStyle();
    void testBnc591147();
    void testCreationDate();
    void testMultiColTexts();
    void testBnc584721_1();
    void testBnc584721_2();
    void testBnc584721_4();
    void testBnc904423();
    void testShapeLineStyle();
    void testTableBorderLineStyle();
    void testBnc862510_6();
    void testBnc862510_7();
#if ENABLE_PDFIMPORT
    void testPDFImportShared();
#if defined(IMPORT_PDF_ELEMENTS)
    void testPDFImport();
    void testPDFImportSkipImages();
#endif
#endif
    void testBulletSuffix();
    void testBnc910045();
    void testRowHeight();
    void testTdf93830();
    void testTdf127129();
    void testTdf93097();
    void testTdf62255();
    void testTdf113163();
    void testTdf93124();
    void testTdf99729();
    void testTdf89927();

    CPPUNIT_TEST_SUITE(SdImportTest);

    CPPUNIT_TEST(testDocumentLayout);
    CPPUNIT_TEST(testTdf146223);
    CPPUNIT_TEST(testTdf144918);
    CPPUNIT_TEST(testTdf144917);
    CPPUNIT_TEST(testHyperlinkOnImage);
    CPPUNIT_TEST(testTdf142645);
    CPPUNIT_TEST(testTdf141704);
    CPPUNIT_TEST(testTdf142915);
    CPPUNIT_TEST(testTdf142913);
    CPPUNIT_TEST(testTdf142590);
    CPPUNIT_TEST(testCustomSlideShow);
    CPPUNIT_TEST(testInternalHyperlink);
    CPPUNIT_TEST(testHyperlinkColor);
    CPPUNIT_TEST(testSmoketest);
    CPPUNIT_TEST(testTdf131269);
    CPPUNIT_TEST(testN759180);
    CPPUNIT_TEST(testN778859);
    CPPUNIT_TEST(testMasterPageStyleParent);
    CPPUNIT_TEST(testGradientAngle);
    CPPUNIT_TEST(testTdf97808);
    CPPUNIT_TEST(testFillStyleNone);
    CPPUNIT_TEST(testFdo64512);
    CPPUNIT_TEST(testFdo71075);
    CPPUNIT_TEST(testN828390_2);
    CPPUNIT_TEST(testN828390_3);
    CPPUNIT_TEST(testFdo68594);
    CPPUNIT_TEST(testPlaceholderPriority);
    CPPUNIT_TEST(testFdo72998);
    CPPUNIT_TEST(testFdo77027);
    CPPUNIT_TEST(testStrictOOXML);
    CPPUNIT_TEST(testN862510_1);
    CPPUNIT_TEST(testN862510_2);
    CPPUNIT_TEST(testN862510_4);
    CPPUNIT_TEST(testBnc870237);
    CPPUNIT_TEST(testBnc887225);
    CPPUNIT_TEST(testPredefinedTableStyle);
    CPPUNIT_TEST(testBnc591147);
    CPPUNIT_TEST(testCreationDate);
    CPPUNIT_TEST(testMultiColTexts);
    CPPUNIT_TEST(testBnc584721_1);
    CPPUNIT_TEST(testBnc584721_2);
    CPPUNIT_TEST(testBnc584721_4);
    CPPUNIT_TEST(testBnc904423);
    CPPUNIT_TEST(testShapeLineStyle);
    CPPUNIT_TEST(testTableBorderLineStyle);
    CPPUNIT_TEST(testBnc862510_6);
    CPPUNIT_TEST(testBnc862510_7);
#if ENABLE_PDFIMPORT
    CPPUNIT_TEST(testPDFImportShared);
#if defined(IMPORT_PDF_ELEMENTS)
    CPPUNIT_TEST(testPDFImport);
    CPPUNIT_TEST(testPDFImportSkipImages);
#endif
#endif
    CPPUNIT_TEST(testBulletSuffix);
    CPPUNIT_TEST(testBnc910045);
    CPPUNIT_TEST(testRowHeight);
    CPPUNIT_TEST(testTdf93830);
    CPPUNIT_TEST(testTdf127129);
    CPPUNIT_TEST(testTdf93097);
    CPPUNIT_TEST(testTdf62255);
    CPPUNIT_TEST(testTdf113163);
    CPPUNIT_TEST(testTdf93124);
    CPPUNIT_TEST(testTdf99729);
    CPPUNIT_TEST(testTdf89927);

    CPPUNIT_TEST_SUITE_END();
};

void SdImportTest::setUp()
{
    SdModelTestBase::setUp();
    mxDesktop.set(frame::Desktop::create(getComponentContext()));
}

/** Test document against a reference XML dump of shapes.

If you want to update one of these tests, or add a new one, set the nUpdateMe
to the index of the test, and the dump XML's will be created (or rewritten)
instead of checking. Use with care - when the test is failing, first find out
why, instead of just updating .xml's blindly.

Example: Let's say you are adding a test called fdoABCD.pptx.  You'll place it
to the data/ subdirectory, and will add an entry to aFilesToCompare below,
the 3rd parameter is for export test - can be -1 (don't export), ODP, PPT or PPTX
like:

        { "fdoABCD.pptx", "xml/fdoABCD_", PPTX },

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
void SdImportTest::testDocumentLayout()
{
    static const struct { std::u16string_view sInput, sDump; sal_Int32 nFormat; sal_Int32 nExportType; } aFilesToCompare[] =
    {
        { u"odp/shapes-test.odp", u"xml/shapes-test_page", ODP, -1 },
        { u"fdo47434.pptx", u"xml/fdo47434_", PPTX, -1 },
        { u"n758621.ppt", u"xml/n758621_", PPT, -1 },
        { u"fdo64586.ppt", u"xml/fdo64586_", PPT, -1 },
        { u"n819614.pptx", u"xml/n819614_", PPTX, -1 },
        { u"n820786.pptx", u"xml/n820786_", PPTX, -1 },
        { u"n762695.pptx", u"xml/n762695_", PPTX, -1 },
        { u"n593612.pptx", u"xml/n593612_", PPTX, -1 },
        { u"fdo71434.pptx", u"xml/fdo71434_", PPTX, -1 },
        { u"n902652.pptx", u"xml/n902652_", PPTX, -1 },
        { u"tdf90403.pptx", u"xml/tdf90403_", PPTX, -1 },
        { u"tdf90338.odp", u"xml/tdf90338_", ODP, PPTX },
        { u"tdf92001.odp", u"xml/tdf92001_", ODP, PPTX },
// GCC -mfpmath=387 rounding issues in lclPushMarkerProperties
// (oox/source/drawingml/lineproperties.cxx); see mail sub-thread starting at
// <https://lists.freedesktop.org/archives/libreoffice/2016-September/
// 075211.html> "Re: Test File: sc/qa/unit/data/functions/fods/chiinv.fods:
// fails with Assertion" for how "-mfpmath=sse -msse2" would fix that:
#if !(defined LINUX && defined X86)
        { u"tdf100491.pptx", u"xml/tdf100491_", PPTX, -1 },
#endif
        { u"tdf109317.pptx", u"xml/tdf109317_", PPTX, ODP},
        // { u"pptx/n828390.pptx", u"pptx/xml/n828390_", PPTX, PPTX }, // Example
    };

    for ( int i = 0; i < static_cast< int >( SAL_N_ELEMENTS( aFilesToCompare ) ); ++i )
    {
        int const nUpdateMe = -1; // index of test we want to update; supposedly only when the test is created

        sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc( u"/sd/qa/unit/data/" ) + aFilesToCompare[i].sInput, aFilesToCompare[i].nFormat );
        if( aFilesToCompare[i].nExportType >= 0 )
            xDocShRef = saveAndReload( xDocShRef.get(), aFilesToCompare[i].nExportType );
        compareWithShapesDump( xDocShRef,
                OUStringConcatenation(m_directories.getPathFromSrc( u"/sd/qa/unit/data/" ) + aFilesToCompare[i].sDump),
                i == nUpdateMe );
    }
}

void SdImportTest::testTdf146223()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf146223.pptx"), PPTX);

    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSet(xPage1, uno::UNO_QUERY_THROW);

    bool bBackgroundObjectsVisible;
    xSet->getPropertyValue("IsBackgroundObjectsVisible") >>= bBackgroundObjectsVisible;
    CPPUNIT_ASSERT_EQUAL(false, bBackgroundObjectsVisible);

    xDocShRef->DoClose();
}

void SdImportTest::testTdf144918()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf144918.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape1(getShapeFromPage(0, 1, xDocShRef));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName("OnClick") >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("First slide"), iter1->second.get<OUString>());

    uno::Reference< beans::XPropertySet > xShape2(getShapeFromPage(1, 1, xDocShRef));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName("OnClick") >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("Third slide"), iter2->second.get<OUString>());

    xDocShRef->DoClose();
}

void SdImportTest::testTdf144917()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf144917.pptx"), PPTX);

    uno::Reference<container::XIndexAccess> xGroupShape(getShapeFromPage(0, 0, xDocShRef),
                                                        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xShape(xGroupShape->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<document::XEventsSupplier> xEventsSupplier(xShape, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents(xEventsSupplier->getEvents());
    uno::Sequence<beans::PropertyValue> props;
    xEvents->getByName("OnClick") >>= props;
    comphelper::SequenceAsHashMap map(props);
    auto iter(map.find("Bookmark"));
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/"), iter->second.get<OUString>());

    xDocShRef->DoClose();
}

void SdImportTest::testHyperlinkOnImage()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/hyperlinkOnImage.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape1(getShapeFromPage(1, 0, xDocShRef));
    uno::Reference<document::XEventsSupplier> xEventsSupplier1(xShape1, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents1(xEventsSupplier1->getEvents());
    uno::Sequence<beans::PropertyValue> props1;
    xEvents1->getByName("OnClick") >>= props1;
    comphelper::SequenceAsHashMap map1(props1);
    auto iter1(map1.find("ClickAction"));
    CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_LASTPAGE,
        iter1->second.get<css::presentation::ClickAction>());

    uno::Reference< beans::XPropertySet > xShape2(getShapeFromPage(1, 1, xDocShRef));
    uno::Reference<document::XEventsSupplier> xEventsSupplier2(xShape2, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xEvents2(xEventsSupplier2->getEvents());
    uno::Sequence<beans::PropertyValue> props2;
    xEvents2->getByName("OnClick") >>= props2;
    comphelper::SequenceAsHashMap map2(props2);
    auto iter2(map2.find("ClickAction"));
    CPPUNIT_ASSERT_EQUAL(css::presentation::ClickAction_NONE,
                         iter2->second.get<css::presentation::ClickAction>());

    xDocShRef->DoClose();
}

void SdImportTest::testTdf142645()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf142645.pptx"), PPTX);
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(xDocShRef->GetDoc()->getUnoModel(),
                                                     uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPage> xPage1(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xNamed1(xPage1, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xNamed1->getName());

    xDocShRef->DoClose();
}

void SdImportTest::testTdf141704()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/tdf141704.pptx"), PPTX);

    for (sal_Int32 i = 0; i < 7; i++)
    {
        uno::Reference<beans::XPropertySet> xShape(getShapeFromPage(1, i, xDocShRef));
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
                CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/"), iter1->second.get<OUString>());
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

    xDocShRef->DoClose();
}

void SdImportTest::testTdf142915()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf142915.pptx"), PPTX);

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);

    bool bChangeManually = xPresentationProps->getPropertyValue("IsAutomatic").get<bool>();

    CPPUNIT_ASSERT_EQUAL(true, bChangeManually);

    xDocShRef->DoClose();
}

void SdImportTest::testTdf142913()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf142913.pptx"), PPTX);

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
        uno::UNO_QUERY_THROW);

    OUString sFirstPage = xPresentationProps->getPropertyValue("FirstPage").get<OUString>();

    CPPUNIT_ASSERT_EQUAL(OUString("Second"), sFirstPage);

    xDocShRef->DoClose();
}

void SdImportTest::testTdf142590()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf142590.pptx"), PPTX);

    uno::Reference<presentation::XPresentationSupplier> xPresentationSupplier(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPresentationProps(xPresentationSupplier->getPresentation(),
                                                           uno::UNO_QUERY_THROW);
    const OUString sCustomShowId
        = xPresentationProps->getPropertyValue("CustomShow").get<OUString>();

    CPPUNIT_ASSERT(!sCustomShowId.isEmpty());

    xDocShRef->DoClose();
}

void SdImportTest::testCustomSlideShow()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf131390.pptx"), PPTX);

    css::uno::Reference<css::presentation::XCustomPresentationSupplier> aXCPSup(
        xDocShRef->GetModel(), css::uno::UNO_QUERY);
    css::uno::Reference<css::container::XNameContainer> aXCont(aXCPSup->getCustomPresentations());
    const css::uno::Sequence< OUString> aNameSeq( aXCont->getElementNames() );

    // In the document, there are two custom presentations.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), aNameSeq.size());

    xDocShRef->DoClose();
}

void SdImportTest::testInternalHyperlink()
{
    ::sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf65724.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 1, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // first chunk of text
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    uno::Reference<text::XTextField> xField;
    xPropSet->getPropertyValue("TextField") >>= xField;
    CPPUNIT_ASSERT_MESSAGE("The text field is missing!", xField.is() );

    xPropSet.set(xField, uno::UNO_QUERY);
    OUString aURL;
    xPropSet->getPropertyValue("URL") >>= aURL;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URLs don't match", OUString("#Slide2"), aURL);

    xDocShRef->DoClose();
}

void SdImportTest::testHyperlinkColor()
{
    ::sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf137367.pptx"), PPTX);

    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph1( getParagraphFromShape( 0, xShape ) );
    // Get second paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph2( getParagraphFromShape( 1, xShape ) );
    // Get third paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph3( getParagraphFromShape( 2, xShape ) );

    // Get run of the first paragraph
    uno::Reference<text::XTextRange> xRun1( getRunFromParagraph (0, xParagraph1 ) );
    uno::Reference< beans::XPropertySet > xPropSet1( xRun1, uno::UNO_QUERY_THROW );
    Color nCharColorBlue;
    xPropSet1->getPropertyValue( "CharColor" ) >>= nCharColorBlue;

    // Get run of the second paragraph
    uno::Reference<text::XTextRange> xRun2( getRunFromParagraph (1, xParagraph2 ) );
    uno::Reference< beans::XPropertySet > xPropSet2( xRun2, uno::UNO_QUERY_THROW );
    Color nCharColorRed;
    xPropSet2->getPropertyValue( "CharColor" ) >>= nCharColorRed;

    // Get run of the third paragraph
    uno::Reference<text::XTextRange> xRun3( getRunFromParagraph (2, xParagraph3 ) );
    uno::Reference< beans::XPropertySet > xPropSet3( xRun3, uno::UNO_QUERY_THROW );
    Color nCharColorGreen;
    xPropSet3->getPropertyValue( "CharColor" ) >>= nCharColorGreen;

    // Hyperlink colors should be blue, red, green.
    CPPUNIT_ASSERT_EQUAL( Color(0x4472c4), nCharColorBlue );
    CPPUNIT_ASSERT_EQUAL( Color(0xff0000), nCharColorRed );
    CPPUNIT_ASSERT_EQUAL( Color(0x548235), nCharColorGreen );

    xDocShRef->DoClose();
}

void SdImportTest::testSmoketest()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/smoketest.pptx"), PPTX);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    // cf. SdrModel svx/svdmodel.hxx ...

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "wrong page count", static_cast<sal_uInt16>(3), pDoc->GetPageCount());

    const SdrPage *pPage = pDoc->GetPage (1);
    CPPUNIT_ASSERT_MESSAGE( "no page", pPage != nullptr );

    CPPUNIT_ASSERT_MESSAGE( "changed", !pDoc->IsChanged() );

    xDocShRef->DoClose();
}

void SdImportTest::testTdf131269()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/tdf131269.ppt"), PPT);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    // Without the fix in place, this test would have failed with
    // - Expected: 115
    // - Actual  : 3
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(115), pDoc->GetPageCount());

    xDocShRef->DoClose();
}

void SdImportTest::testN759180()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/n759180.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Get the object
    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT(pTxtObj);
    std::vector<EECharAttrib> rLst;
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxULSpaceItem *pULSpace = aEdit.GetParaAttribs(0).GetItem(EE_PARA_ULSPACE);
    CPPUNIT_ASSERT(pULSpace);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Para bottom spacing is wrong!", static_cast<sal_uInt16>(0), pULSpace->GetLower());
    aEdit.GetCharAttribs(1, rLst);
    auto it = std::find_if(rLst.rbegin(), rLst.rend(),
        [](const EECharAttrib& rCharAttr) { return dynamic_cast<const SvxFontHeightItem *>(rCharAttr.pAttr) != nullptr; });
    if (it != rLst.rend())
    {
        const SvxFontHeightItem * pFontHeight = dynamic_cast<const SvxFontHeightItem *>((*it).pAttr);
        // nStart == 9
        // font height = 5 => 5*2540/72
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Font height is wrong", static_cast<sal_uInt32>(176), pFontHeight->GetHeight() );
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_1()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n862510_1.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj( 0 );
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs( 0, rLst );
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it!=rLst.rend(); ++it)
        {
            const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>( (*it).pAttr );
            CPPUNIT_ASSERT_MESSAGE( "Baseline attribute not handled properly", !(pFontEscapement && pFontEscapement->GetProportionalHeight() != 100) );
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_2()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n862510_2.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());
        SdrObjGroup *pGrpObj = dynamic_cast<SdrObjGroup *>( pPage->GetObj( 0 ) );
        CPPUNIT_ASSERT( pGrpObj );
        SdrObjCustomShape *pObj = dynamic_cast<SdrObjCustomShape *>( pGrpObj->GetSubList()->GetObj( 1 ) );
        CPPUNIT_ASSERT( pObj );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong Text Rotation!", 90.0, pObj->GetExtraTextRotation( true ) );
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN862510_4()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n862510_4.pptx"), PPTX );

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        std::vector<EECharAttrib> rLst;
        SdrObject *pObj = pPage->GetObj( 0 );
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT( pTxtObj );
        const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
        aEdit.GetCharAttribs( 0, rLst );
        for( std::vector<EECharAttrib>::reverse_iterator it = rLst.rbegin(); it != rLst.rend(); ++it )
        {
            const SvxColorItem *pC = dynamic_cast<const SvxColorItem *>( (*it).pAttr );
            CPPUNIT_ASSERT_MESSAGE( "gradfill for text color not handled!", !( pC && pC->GetValue() == Color(0) ) );
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testN828390_2()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n828390_2.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT( pTxtObj );
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Linux  "), aEdit.GetText(0));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard Platform"), aEdit.GetText(1));

    xDocShRef->DoClose();
}

void SdImportTest::testN828390_3()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n828390_3.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT( pTxtObj );
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    std::vector<EECharAttrib> rLst;
    aEdit.GetCharAttribs(1, rLst);
    bool bPassed = std::none_of(rLst.rbegin(), rLst.rend(),
        [](const EECharAttrib& rCharAttr) {
            const SvxEscapementItem *pFontEscapement = dynamic_cast<const SvxEscapementItem *>(rCharAttr.pAttr);
            return pFontEscapement && (pFontEscapement->GetEsc() != 0);
        });
    CPPUNIT_ASSERT_MESSAGE("CharEscapment not imported properly", bPassed);

    xDocShRef->DoClose();
}

void SdImportTest::testMasterPageStyleParent()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/masterpage_style_parent.odp"), ODP );
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    SdStyleSheetPool *const pPool(pDoc->GetSdStyleSheetPool());

    int parents(0);
    SfxStyleSheetIterator iter(pPool, SfxStyleFamily::Page);
    for (SfxStyleSheetBase * pStyle = iter.First(); pStyle; pStyle = iter.Next())
    {
        OUString const name(pStyle->GetName());
        OUString const parent(pStyle->GetParent());
        if (!parent.isEmpty())
        {
            ++parents;
            // check that parent exists
            SfxStyleSheetBase *const pParentStyle(
                    pPool->Find(parent, SfxStyleFamily::Page));
            CPPUNIT_ASSERT(pParentStyle);
            CPPUNIT_ASSERT_EQUAL(pParentStyle->GetName(), parent);
            // check that parent has the same master page as pStyle
            CPPUNIT_ASSERT(parent.indexOf(SD_LT_SEPARATOR) != -1);
            CPPUNIT_ASSERT(name.indexOf(SD_LT_SEPARATOR) != -1);
            CPPUNIT_ASSERT_EQUAL(
                    parent.copy(0, parent.indexOf(SD_LT_SEPARATOR)),
                    name.copy(0, name.indexOf(SD_LT_SEPARATOR)));
        }
    }
    // check that there are actually parents...
    CPPUNIT_ASSERT_EQUAL(16, parents);

    xDocShRef->DoClose();
}

void SdImportTest::testGradientAngle()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/gradient-angle.fodg"), FODG);

    uno::Reference<lang::XMultiServiceFactory> const xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY);

    awt::Gradient gradient;
    uno::Reference<container::XNameAccess> const xGradients(
        xDoc->createInstance("com.sun.star.drawing.GradientTable"),
        uno::UNO_QUERY);

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
        xDoc->createInstance("com.sun.star.drawing.TransparencyGradientTable"),
        uno::UNO_QUERY);

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 2") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), gradient.Angle); // 1

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 1") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(900), gradient.Angle); // 90deg

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 3") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(572), gradient.Angle); // 1.0rad

    CPPUNIT_ASSERT(xTranspGradients->getByName("Transparency 4") >>= gradient);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1800), gradient.Angle); // 1000grad

    xDocShRef->DoClose();
}

void SdImportTest::testN778859()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n778859.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        // Get the object
        SdrObject *pObj = pPage->GetObj(1);
        SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
        CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
        CPPUNIT_ASSERT(!pTxtObj->IsAutoFit());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testFdo68594()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/fdo68594.ppt"), PPT);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    SdrObject *pObj = pPage->GetObj(1);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const SvxColorItem *pC = &pTxtObj->GetMergedItem(EE_CHAR_COLOR);
    CPPUNIT_ASSERT_MESSAGE( "no color item", pC != nullptr);
    // Color should be black
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Placeholder color mismatch", COL_BLACK, pC->GetValue() );

    xDocShRef->DoClose();
}

void SdImportTest::testPlaceholderPriority()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/placeholder-priority.pptx"), PPTX);

    const SdrPage* pPage = GetPage( 1, xDocShRef );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Missing placeholder", sal_uInt32(2), sal_uInt32(pPage->GetObjCount()));

    tools::Rectangle pObj1Rect(9100, 3500, 29619, 4038);
    SdrObject *pObj1 = pPage->GetObj(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder position is wrong, check the placeholder priority", pObj1Rect, pObj1->GetCurrentBoundRect());

    tools::Rectangle pObj2Rect(9102, 8643, 29619, 12642);
    SdrObject *pObj2 = pPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Placeholder position is wrong, check the placeholder priority", pObj2Rect, pObj2->GetCurrentBoundRect());

    // If the placeholder positions are wrong, please check placeholder priority in Placeholders class.
}

void SdImportTest::testFdo72998()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/cshapes.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrObjCustomShape *pObj = dynamic_cast<SdrObjCustomShape *>(pPage->GetObj(2));
        CPPUNIT_ASSERT( pObj );
        const SdrCustomShapeGeometryItem& rGeometryItem = pObj->GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
        const css::uno::Any* pViewBox = const_cast<SdrCustomShapeGeometryItem&>(rGeometryItem).GetPropertyValueByName( OUString( "ViewBox" ) );
        CPPUNIT_ASSERT_MESSAGE( "Missing ViewBox", pViewBox );
        css::awt::Rectangle aViewBox;
        CPPUNIT_ASSERT( (*pViewBox >>= aViewBox ) );
        CPPUNIT_ASSERT_MESSAGE( "Width should be zero - for forcing scale to 1", !aViewBox.Width );
        CPPUNIT_ASSERT_MESSAGE( "Height should be zero - for forcing scale to 1", !aViewBox.Height );
    }

    xDocShRef->DoClose();
}

void SdImportTest::testFdo77027()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/fdo77027.odp"), ODP);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    {
        SdrOle2Obj *const pObj = dynamic_cast<SdrOle2Obj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT(pObj);

        // check that the fill style/color was actually imported
        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xff6600), rColorItem.GetColorValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTdf97808()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/tdf97808.fodp"), FODP);

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(
        xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("graphics"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName("objectwithoutfill"), uno::UNO_QUERY);
    OUString lineend;
    CPPUNIT_ASSERT(xStyle->getPropertyValue("LineEndName") >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString("Arrow"), lineend);

    // the draw:marker-end="" did not override the style
    uno::Reference<drawing::XDrawPagesSupplier> xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xLine(
        xPage->getByIndex(0), uno::UNO_QUERY_THROW);
    //uno::Reference<style::XStyle> xParent;
    uno::Reference<beans::XPropertySet> xParent;
    CPPUNIT_ASSERT(xLine->getPropertyValue("Style") >>= xParent);
    CPPUNIT_ASSERT_EQUAL(xStyle, xParent);
    CPPUNIT_ASSERT(xLine->getPropertyValue("LineEndName") >>= lineend);
    CPPUNIT_ASSERT_EQUAL(OUString(), lineend);

    xDocShRef->DoClose();
}
void SdImportTest::testFillStyleNone()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/tdf123841.odg"), ODG);

    const SdrPage* pPage = GetPage(1, xDocShRef);
    const SdrObject* pObj = pPage->GetObj(0);
    auto& rFillStyleItem
        = dynamic_cast<const XFillStyleItem&>(pObj->GetMergedItem(XATTR_FILLSTYLE));
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the shape's fill was FillStyle_SOLID, making the text of the shape unreadable.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, rFillStyleItem.GetValue());
    xDocShRef->DoClose();
}

void SdImportTest::testFdo64512()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo64512.odp"), ODP);

    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "not exactly one page", static_cast<sal_Int32>(1), xDoc->getDrawPages()->getCount() );

    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "not exactly three shapes", static_cast<sal_Int32>(3), xPage->getCount() );

    uno::Reference< beans::XPropertySet > xConnectorShape(
        xPage->getByIndex(2), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no connector shape", xConnectorShape.is() );

    uno::Reference< beans::XPropertySet > xSvgShape(
        xConnectorShape->getPropertyValue("StartShape"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no start shape", xSvgShape.is() );

    uno::Reference< beans::XPropertySet > xCustomShape(
        xConnectorShape->getPropertyValue("EndShape"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no end shape", xCustomShape.is() );

    uno::Reference< animations::XAnimationNodeSupplier > xAnimNodeSupplier(
        xPage, uno::UNO_QUERY_THROW );
    uno::Reference< animations::XAnimationNode > xRootNode(
        xAnimNodeSupplier->getAnimationNode() );
    std::vector< uno::Reference< animations::XAnimationNode > > aAnimVector;
    anim::create_deep_vector(xRootNode, aAnimVector);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "not 8 animation nodes", static_cast<std::size_t>(8), aAnimVector.size() );

    uno::Reference< animations::XAnimate > xNode(
        aAnimVector[7], uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xTargetShape(
        xNode->getTarget(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE( "inner node not referencing svg shape",
                            xTargetShape != xSvgShape );

    xDocShRef->DoClose();
}

// Unit test for importing charts
void SdImportTest::testFdo71075()
{
    double values[] = { 12.0, 13.0, 14.0 };
    css::uno::Any aAny;
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo71075.odp"), ODP);

    uno::Reference< beans::XPropertySet > xPropSet( getShapeFromPage( 0, 0, xDocShRef ) );
    aAny = xPropSet->getPropertyValue( "Model" );
    CPPUNIT_ASSERT_MESSAGE( "The shape doesn't have the property", aAny.hasValue() );

    uno::Reference< chart::XChartDocument > xChartDoc;
    aAny >>= xChartDoc;
    CPPUNIT_ASSERT_MESSAGE( "failed to load chart", xChartDoc.is() );
    uno::Reference< chart2::XChartDocument > xChart2Doc( xChartDoc, uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "failed to load chart", xChart2Doc.is() );

    uno::Reference< chart2::XCoordinateSystemContainer > xBCooSysCnt( xChart2Doc->getFirstDiagram(), uno::UNO_QUERY );
    uno::Sequence< uno::Reference< chart2::XCoordinateSystem > > aCooSysSeq( xBCooSysCnt->getCoordinateSystems());
    uno::Reference< chart2::XChartTypeContainer > xCTCnt( aCooSysSeq[0], uno::UNO_QUERY );

    uno::Reference< chart2::XDataSeriesContainer > xDSCnt( xCTCnt->getChartTypes()[0], uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "failed to load data series", xDSCnt.is() );
    uno::Sequence< uno::Reference< chart2::XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Series count", static_cast<sal_Int32>(1), aSeriesSeq.getLength() );
    uno::Reference< chart2::data::XDataSource > xSource( aSeriesSeq[0], uno::UNO_QUERY );
    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSeqCnt(xSource->getDataSequences());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Series count", static_cast<sal_Int32>(1), aSeqCnt.getLength());
    uno::Reference< chart2::data::XDataSequence > xValueSeq( aSeqCnt[0]->getValues() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Data count", static_cast<sal_Int32>(SAL_N_ELEMENTS(values)), xValueSeq->getData().getLength());
    uno::Reference< chart2::data::XNumericalDataSequence > xNumSeq( xValueSeq, uno::UNO_QUERY );
    uno::Sequence< double > aValues( xNumSeq->getNumericalData());
    for(sal_Int32 i=0;i<xValueSeq->getData().getLength();i++)
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Invalid Series count", values[i], aValues.getConstArray()[i]);

    xDocShRef->DoClose();
}

void SdImportTest::testStrictOOXML()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/strict_ooxml.pptx"), PPTX);

    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    getShapeFromPage( 0, 0, xDocShRef );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc870237()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc870237.pptx"), PPTX);
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    // Simulate a:ext inside dsp:txXfrm with changing the lower distance
    const SdrObjGroup* pGroupObj = dynamic_cast<SdrObjGroup*>( pPage->GetObj( 0 ) );
    const SdrObject* pObj = pGroupObj->GetSubList()->GetObj( 1 );
    CPPUNIT_ASSERT_MESSAGE( "no object", pObj != nullptr);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_UPPERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(9919), pObj->GetMergedItem(SDRATTR_TEXT_LOWERDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), pObj->GetMergedItem(SDRATTR_TEXT_LEFTDIST).GetValue());

    xDocShRef->DoClose();
}

void SdImportTest::testCreationDate()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/fdo71434.pptx"), PPTX);
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    util::DateTime aDate = xDocumentProperties->getCreationDate();
    OUStringBuffer aBuffer;
    sax::Converter::convertDateTime(aBuffer, aDate, nullptr);
    // Metadata wasn't imported, this was 0000-00-00.
    CPPUNIT_ASSERT_EQUAL(OUString("2013-11-09T10:37:56"), aBuffer.makeStringAndClear());
    xDocShRef->DoClose();
}

void SdImportTest::testMultiColTexts()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/multicol.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    auto pTextObj = dynamic_cast<SdrTextObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pTextObj);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pTextObj->GetTextColumnsNumber());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pTextObj->GetTextColumnsSpacing());

    auto pMasterTextObj = dynamic_cast<SdrTextObj*>(pPage->TRG_GetMasterPage().GetObj(0));
    CPPUNIT_ASSERT(pMasterTextObj);

    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), pMasterTextObj->GetTextColumnsNumber());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), pMasterTextObj->GetTextColumnsSpacing());

    uno::Reference<text::XTextRange> xText(pMasterTextObj->getUnoShape(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("mastershape1\nmastershape2"), xText->getString());
}

void SdImportTest::testPredefinedTableStyle()
{
    // 073A0DAA-6AF3-43AB-8588-CEC1D06C72B9 (Medium Style 2)
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/predefined-table-style.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
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

    xDocShRef->DoClose();
}

void SdImportTest::testBnc887225()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc887225.pptx"), PPTX );
    // In the document, lastRow and lastCol table properties are used.
    // Make sure styles are set properly for individual cells.

    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
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

    xDocShRef->DoClose();
}

void SdImportTest::testBnc584721_1()
{
    // Title text shape on the master page contained wrong text.

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc584721_1_2.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    SdrObject *pObj = pPage->GetObj(0);
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pObj );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    CPPUNIT_ASSERT_EQUAL(OUString("Click to edit Master title style"), aEdit.GetText(0));
    xDocShRef->DoClose();
}

void SdImportTest::testBnc584721_2()
{
    // Import created an extra/unneeded outliner shape on the master slide next to the imported title shape.

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc584721_1_2.pptx"), PPTX);

    const SdrPage *pPage = &(GetPage( 1, xDocShRef )->TRG_GetMasterPage());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pPage->GetObjCount());

    xDocShRef->DoClose();
}

void SdImportTest::testBnc591147()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc591147.pptx"), PPTX);

    // In the document, there are two slides with media files.
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(
        xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(2), xDoc->getDrawPages()->getCount() );

    // First page has video file inserted
    uno::Reference< drawing::XDrawPage > xPage(
        xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), xPage->getCount() );

    //uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropSet( getShape( 0, xPage ) );
    OUString sVideoURL("emptyURL");
    bool bSuccess = xPropSet->getPropertyValue("MediaURL") >>= sVideoURL;
    CPPUNIT_ASSERT_MESSAGE( "MediaURL property is not set", bSuccess );
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sVideoURL.isEmpty());

    // Second page has audio file inserted
    xPage.set( xDoc->getDrawPages()->getByIndex(1), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(1), xPage->getCount() );

    xPropSet.set( getShape( 0, xPage ) );
    OUString sAudioURL("emptyURL");
    bSuccess = xPropSet->getPropertyValue("MediaURL") >>= sAudioURL;
    CPPUNIT_ASSERT_MESSAGE( "MediaURL property is not set", bSuccess );
    CPPUNIT_ASSERT_MESSAGE("MediaURL is empty", !sAudioURL.isEmpty());

    CPPUNIT_ASSERT_MESSAGE( "sAudioURL and sVideoURL should not be equal", sAudioURL != sVideoURL );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc584721_4()
{
    // Black text was imported as white because of wrong caching mechanism

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc584721_4.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 1, 1, xDocShRef ) );

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph (0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );
    Color nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;

    // Color should be black
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, nCharColor );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc904423()
{
    // Here the problem was that different fill properties were applied in wrong order on the shape
    // Right order: 1) master slide fill style, 2) theme, 3) direct formatting
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/bnc904423.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    // First shape's background color is defined on master slide
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x00CC99), rColorItem.GetColorValue());
    }

    // Second shape's background color is defined by theme
    {
        SdrObject *const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x3333CC), rColorItem.GetColorValue());
    }

    // Third shape's background color is defined by direct formatting
    {
        SdrObject *const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XFillStyleItem& rStyleItem = dynamic_cast<const XFillStyleItem&>(
                pObj->GetMergedItem(XATTR_FILLSTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, rStyleItem.GetValue());
        const XFillColorItem& rColorItem = dynamic_cast<const XFillColorItem&>(
                pObj->GetMergedItem(XATTR_FILLCOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), rColorItem.GetColorValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testShapeLineStyle()
{
    // Here the problem was that different line properties were applied in wrong order on the shape
    // Right order: 1) master slide line style, 2) theme, 3) direct formatting
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"sd/qa/unit/data/pptx/ShapeLineProperties.pptx"), PPTX);

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    // First shape's line style is defined on master slide
    {
        SdrObject *const pObj = pPage->GetObj(0);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_DASH, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0xFF0000), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(132), rWidthItem.GetValue());
    }

    // Second shape's line style is defined by theme
    {
        SdrObject *const pObj = pPage->GetObj(1);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x3333CC), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(35), rWidthItem.GetValue());
    }

    // Third shape's line style is defined by direct formatting
    {
        SdrObject *const pObj = pPage->GetObj(2);
        CPPUNIT_ASSERT(pObj);

        const XLineStyleItem& rStyleItem = dynamic_cast<const XLineStyleItem&>(
                pObj->GetMergedItem(XATTR_LINESTYLE));
        CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_SOLID, rStyleItem.GetValue());

        const XLineColorItem& rColorItem = dynamic_cast<const XLineColorItem&>(
                pObj->GetMergedItem(XATTR_LINECOLOR));
        CPPUNIT_ASSERT_EQUAL(Color(0x7030A0), rColorItem.GetColorValue());

        const XLineWidthItem& rWidthItem = dynamic_cast<const XLineWidthItem&>(
                pObj->GetMergedItem(XATTR_LINEWIDTH));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(176), rWidthItem.GetValue());
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTableBorderLineStyle()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tableBorderLineStyle.pptx"), PPTX );

    // TODO: If you are working on improvement of table border line style
    // support, then expect this unit test to fail.

    const sal_Int16 nObjBorderLineStyles[] =
        {
            ::table::BorderLineStyle::DASHED,
            ::table::BorderLineStyle::DASH_DOT_DOT,
            ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,
            ::table::BorderLineStyle::DOTTED,
            ::table::BorderLineStyle::DASHED,
            ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::DASH_DOT,
            ::table::BorderLineStyle::SOLID,
            ::table::BorderLineStyle::NONE
        };

    const SdrPage *pPage = GetPage( 1, xDocShRef );
    CPPUNIT_ASSERT_EQUAL(SAL_N_ELEMENTS(nObjBorderLineStyles), pPage->GetObjCount());

    sdr::table::SdrTableObj *pTableObj;
    uno::Reference< table::XCellRange > xTable;
    uno::Reference< beans::XPropertySet > xCell;
    table::BorderLine2 aBorderLine;

    for (size_t i = 0; i < SAL_N_ELEMENTS(nObjBorderLineStyles); i++)
    {
        pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(i));
        CPPUNIT_ASSERT( pTableObj );
        xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
        xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
        xCell->getPropertyValue("TopBorder") >>= aBorderLine;
        if (aBorderLine.LineWidth > 0) {
            CPPUNIT_ASSERT_EQUAL(nObjBorderLineStyles[i], aBorderLine.LineStyle);
        }
    }

    xDocShRef->DoClose();
}

void SdImportTest::testBnc862510_6()
{
    // Black text was imported instead of gray
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc862510_6.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph of the text
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );

    // Get first run of the paragraph
    uno::Reference<text::XTextRange> xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );
    Color nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;

    // Color should be gray
    CPPUNIT_ASSERT_EQUAL( Color(0x8B8B8B), nCharColor );

    xDocShRef->DoClose();
}

void SdImportTest::testBnc862510_7()
{
    // Title shape's text was aligned to left instead of center.
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc862510_7.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );

    // Get first paragraph
    uno::Reference<text::XTextRange> const xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< beans::XPropertySet > xPropSet( xParagraph, uno::UNO_QUERY_THROW );

    sal_Int16 nParaAdjust = 0;
    xPropSet->getPropertyValue( "ParaAdjust" ) >>= nParaAdjust;
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(nParaAdjust));

    xDocShRef->DoClose();
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

void SdImportTest::testPDFImportShared()
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    comphelper::LibreOfficeKit::setActive();
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pdf/multipage.pdf"), PDF);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    // This test is to verify that we share the PDF stream linked to each
    // Graphic instance in the imported document.
    // Since we import PDFs as images, we support attaching the original
    // PDF with each image to allow for advanced editing.
    // Here we iterate over all Graphic instances embedded in the pages
    // and verify that they all point to the same object in memory.
    std::vector<Graphic> aGraphics;

    for (int nPageIndex = 0; nPageIndex < pDoc->GetPageCount(); ++nPageIndex)
    {
        const SdrPage* pPage = GetPage(nPageIndex, xDocShRef);
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
            CPPUNIT_ASSERT_MESSAGE("After loading, the PDF shouldn't have the primitive sequence created yet",
                                   !rGraphic.getVectorGraphicData()->isPrimitiveSequenceCreated());
            aGraphics.push_back(rGraphic);
        }
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected more than one page.", size_t(9), aGraphics.size());

    Graphic const & rFirstGraphic = aGraphics[0];

    for (size_t i = 0; i < aGraphics.size(); ++i)
    {
        Graphic const & rGraphic = aGraphics[i];
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected all PDF streams to be identical.",
                                     rFirstGraphic.getVectorGraphicData()->getBinaryDataContainer().getData(),
                                     rGraphic.getVectorGraphicData()->getBinaryDataContainer().getData());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Expected all GfxLinks to be identical.",
                                     rFirstGraphic.GetSharedGfxLink().get(),
                                     rGraphic.GetSharedGfxLink().get());

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Page number doesn't match expected", sal_Int32(i), rGraphic.getPageNumber());
    }

    xDocShRef->DoClose();
    comphelper::LibreOfficeKit::setActive(false);
}

#if defined(IMPORT_PDF_ELEMENTS)

void SdImportTest::testPDFImport()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pdf/txtpic.pdf"), PDF);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "no exactly two shapes", static_cast<sal_Int32>(2), xPage->getCount() );

    uno::Reference< beans::XPropertySet > xShape( getShape( 0, xPage ) );
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );

    xDocShRef->DoClose();
}

void SdImportTest::testPDFImportSkipImages()
{
    auto pParams = std::make_shared<SfxAllItemSet>( SfxGetpApp()->GetPool() );
    pParams->Put( SfxStringItem ( SID_FILE_FILTEROPTIONS, "SkipImages" ) );

    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc("/sd/qa/unit/data/pdf/txtpic.pdf"), PDF, pParams);
    SdDrawDocument *pDoc = xDocShRef->GetDoc();
    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );
    uno::Reference< drawing::XDrawPagesSupplier > xDoc(xDocShRef->GetDoc()->getUnoModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XDrawPage > xPage(xDoc->getDrawPages()->getByIndex(0), uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "no exactly one shape", static_cast<sal_Int32>(1), xPage->getCount() );

    uno::Reference< drawing::XShape > xShape(xPage->getByIndex(0), uno::UNO_QUERY_THROW );
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_MESSAGE( "not a text shape", xText.is() );

    xDocShRef->DoClose();
}

#endif
#endif

void SdImportTest::testBulletSuffix()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n83889.pptx"), PPTX );

    // check suffix of the char bullet
    const SdrPage *pPage = GetPage( 1, xDocShRef );
    SdrTextObj *pTxtObj = dynamic_cast<SdrTextObj *>( pPage->GetObj(0) );
    CPPUNIT_ASSERT_MESSAGE( "no text object", pTxtObj != nullptr);
    const EditTextObject& aEdit = pTxtObj->GetOutlinerParaObject()->GetTextObject();
    const SvxNumBulletItem *pNumFmt = aEdit.GetParaAttribs(1).GetItem(EE_PARA_NUMBULLET);
    CPPUNIT_ASSERT(pNumFmt);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Bullet's suffix is wrong!", pNumFmt->GetNumRule().GetLevel(0).GetSuffix(), OUString() );
    xDocShRef->DoClose();
}

void SdImportTest::testBnc910045()
{
    // Problem with table style which defines cell color with fill style
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/bnc910045.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );
    uno::Reference< table::XCellRange > xTable(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xCell;
    Color nColor;

    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    xCell->getPropertyValue("FillColor") >>= nColor;
    CPPUNIT_ASSERT_EQUAL(Color(0x4f81bd), nColor);

    xDocShRef->DoClose();
}

void SdImportTest::testRowHeight()
{
    sd::DrawDocShellRef xDocShRef = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/n80340.pptx"), PPTX );
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    sal_Int32 nHeight;
    uno::Reference< css::table::XTable > xTable(pTableObj->getTable(), uno::UNO_SET_THROW);
    uno::Reference< css::table::XTableRows > xRows( xTable->getRows(), uno::UNO_SET_THROW);
    uno::Reference< beans::XPropertySet > xRefRow( xRows->getByIndex(0), uno::UNO_QUERY_THROW );
    xRefRow->getPropertyValue( "Height" ) >>= nHeight;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(508), nHeight);

    xDocShRef->DoClose();

    sd::DrawDocShellRef xDocShRef2 = loadURL( m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tablescale.pptx"), PPTX );
    const SdrPage *pPage2 = GetPage( 1, xDocShRef2 );

    sdr::table::SdrTableObj *pTableObj2 = dynamic_cast<sdr::table::SdrTableObj*>(pPage2->GetObj(0));
    CPPUNIT_ASSERT( pTableObj2 );

    uno::Reference< css::table::XTable > xTable2(pTableObj2->getTable(), uno::UNO_SET_THROW);
    uno::Reference< css::table::XTableRows > xRows2( xTable2->getRows(), uno::UNO_SET_THROW);

    for(sal_Int32 nRow = 0; nRow < 7; ++nRow)
    {
        uno::Reference< beans::XPropertySet > xRefRow2( xRows2->getByIndex(nRow), uno::UNO_QUERY_THROW );
        xRefRow2->getPropertyValue( "Height" ) >>= nHeight;
        CPPUNIT_ASSERT_EQUAL( sal_Int32(800), nHeight);
    }

    xDocShRef2->DoClose();
}

void SdImportTest::testTdf93830()
{
    // Text shape offset was ignored
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf93830.pptx"), PPTX);
    uno::Reference< drawing::XDrawPage > xPage( getPage( 0, xDocShRef ) );

    // Get the first text box from group shape
    uno::Reference< container::XIndexAccess > xShape( xPage->getByIndex(0), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPropSet( xShape->getByIndex(3), uno::UNO_QUERY );
    CPPUNIT_ASSERT_MESSAGE( "no textbox shape", xPropSet.is() );

    sal_Int32 nTextLeftDistance = 0;
    xPropSet->getPropertyValue( "TextLeftDistance" ) >>= nTextLeftDistance;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4152), nTextLeftDistance);

    xDocShRef->DoClose();
}

void SdImportTest::testTdf127129()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf127129.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< text::XTextRange > xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< text::XTextRange > xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    Color nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, nCharColor );

    // Without the accompanying fix in place, the highlight would be -1
    Color nCharBackColor;
    xPropSet->getPropertyValue( "CharBackColor" ) >>= nCharBackColor;
    CPPUNIT_ASSERT_EQUAL( Color(0xFF00), nCharBackColor );

    xDocShRef->DoClose();
}
void SdImportTest::testTdf93097()
{
    // Throwing metadata import aborted the filter, check that metadata is now imported.
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf93097.pptx"), PPTX);
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(xDocShRef->GetModel(), uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("ss"), xDocumentProperties->getTitle());
    xDocShRef->DoClose();
}

void SdImportTest::testTdf62255()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf62255.pptx"), PPTX);
    const SdrPage *pPage = GetPage( 1, xDocShRef );

    sdr::table::SdrTableObj *pTableObj;
    pTableObj = dynamic_cast<sdr::table::SdrTableObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT( pTableObj );

    css::uno::Any aAny;
    uno::Reference< table::XCellRange > xTable;
    uno::Reference< beans::XPropertySet > xCell;
    xTable.set(pTableObj->getTable(), uno::UNO_QUERY_THROW);
    xCell.set(xTable->getCellByPosition(0, 0), uno::UNO_QUERY_THROW);
    aAny = xCell->getPropertyValue("FillStyle");

    if (aAny.hasValue())
    {
        drawing::FillStyle aFillStyle;
        aAny >>= aFillStyle;
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, aFillStyle);
    }

    xDocShRef->DoClose();
}

void SdImportTest::testTdf113163()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf113163.pptx"), PPTX);
    uno::Reference < uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence< beans::PropertyValue > aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence< beans::PropertyValue > aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference< lang::XComponent > xPage(getPage(0, xDocShRef), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
            {
                // Check all pixels in the image are black
                // Without the fix in place, this test would have failed with
                // - Expected: 0
                // - Actual  : 16777215
                const Color aColor = pReadAccess->GetColor(nX, nY);
                CPPUNIT_ASSERT_EQUAL(COL_BLACK, aColor);
            }
        }
    }
    xDocShRef->DoClose();
}

void SdImportTest::testTdf93124()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf93124.ppt"), PPT);
    uno::Reference < uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence< beans::PropertyValue > aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(320)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(180))
    };

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence< beans::PropertyValue > aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference< lang::XComponent > xPage(getPage(0, xDocShRef), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    CPPUNIT_ASSERT_EQUAL(Size(320, 180), aBMPEx.GetSizePixel());
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
        int nNonWhiteCount = 0;
        // The word "Top" should be in rectangle 34,4 - 76,30. If text alignment is wrong, the rectangle will be white.
        for (tools::Long nY = 4; nY < (4 + 26); ++nY)
        {
            for (tools::Long nX = 34; nX < (34 + 43); ++nX)
            {
                const Color aColor = pReadAccess->GetColor(nY, nX);
                if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff) || (aColor.GetBlue() != 0xff))
                    ++nNonWhiteCount;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Tdf93124: vertical alignment of text is incorrect!", nNonWhiteCount>50);
    }
    xDocShRef->DoClose();
}

void SdImportTest::testTdf99729()
{
    const OUString filenames[] = { "/sd/qa/unit/data/odp/tdf99729-new.odp", "/sd/qa/unit/data/odp/tdf99729-legacy.odp" };
    int nonwhitecounts[] = { 0, 0 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(filenames); ++i)
    {
        // 1st check for new behaviour - having AnchoredTextOverflowLegacy compatibility flag set to false in settings.xml
        uno::Reference<lang::XComponent> xComponent
            = loadFromDesktop(m_directories.getURLFromSrc(filenames[i]),
                              "com.sun.star.presentation.PresentationDocument");

        uno::Reference<uno::XComponentContext> xContext = getComponentContext();
        CPPUNIT_ASSERT(xContext.is());
        uno::Reference< drawing::XGraphicExportFilter > xGraphicExporter = drawing::GraphicExportFilter::create(xContext);
        CPPUNIT_ASSERT(xGraphicExporter.is());

        uno::Sequence< beans::PropertyValue > aFilterData{
            comphelper::makePropertyValue("PixelWidth", sal_Int32(320)),
            comphelper::makePropertyValue("PixelHeight", sal_Int32(240))
        };

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();

        uno::Sequence< beans::PropertyValue > aDescriptor{
            comphelper::makePropertyValue("URL", aTempFile.GetURL()),
            comphelper::makePropertyValue("FilterName", OUString("PNG")),
            comphelper::makePropertyValue("FilterData", aFilterData)
        };

        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
        uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                               uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPage.is());
        xGraphicExporter->setSourceDocument(xPage);
        xGraphicExporter->filter(aDescriptor);

        SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
        vcl::PngImageReader aPNGReader(aFileStream);
        BitmapEx aBMPEx = aPNGReader.read();
        Bitmap aBMP = aBMPEx.GetBitmap();
        Bitmap::ScopedReadAccess pRead(aBMP);
        for (tools::Long nX = 154; nX < (154 + 12); ++nX)
        {
            for (tools::Long nY = 16; nY < (16 + 96); ++nY)
            {
                const Color aColor = pRead->GetColor(nY, nX);
                if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff) || (aColor.GetBlue() != 0xff))
                    ++nonwhitecounts[i];
            }
        }
        xComponent->dispose();
    }
    // The numbers 1-9 should be above the Text Box in rectangle 154,16 - 170,112.
    // If text alignment is wrong, the rectangle will be white.
    CPPUNIT_ASSERT_MESSAGE("Tdf99729: vertical alignment of text is incorrect!", nonwhitecounts[0]>100); // it is 134 with cleartype disabled
    // The numbers 1-9 should be below the Text Box -> rectangle 154,16 - 170,112 should be white.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tdf99729: legacy vertical alignment of text is incorrect!", 0, nonwhitecounts[1]);
}

void SdImportTest::testTdf89927()
{
    sd::DrawDocShellRef xDocShRef = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf89927.pptx"), PPTX);
    uno::Reference< beans::XPropertySet > xShape( getShapeFromPage( 0, 0, xDocShRef ) );
    uno::Reference< text::XTextRange > xParagraph( getParagraphFromShape( 0, xShape ) );
    uno::Reference< text::XTextRange > xRun( getRunFromParagraph( 0, xParagraph ) );
    uno::Reference< beans::XPropertySet > xPropSet( xRun, uno::UNO_QUERY_THROW );

    Color nCharColor;
    xPropSet->getPropertyValue( "CharColor" ) >>= nCharColor;
    CPPUNIT_ASSERT_EQUAL( COL_WHITE, nCharColor );

    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdImportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
