/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>

#include <comphelper/scopeguard.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <editeng/unolingu.hxx>
#include <vcl/scheduler.hxx>

#include <scriptinfo.hxx>
#include <rootfrm.hxx>
#include <wrtsh.hxx>
#include <txtfrm.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <frameformats.hxx>
#include <unotextrange.hxx>
#include <fmtanchr.hxx>

/// Test to assert layout / rendering result of Writer.
class SwLayoutWriter5 : public SwModelTestBase
{
public:
    SwLayoutWriter5()
        : SwModelTestBase(u"/sw/qa/extras/layout/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf138194)
{
    createSwDoc("xaxis-labelbreak.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 8
    // - Actual  : 7
    // i.e. the X axis label flowed out of chart area.
    assertXPath(pXmlDoc, "//textarray", 8);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf146272)
{
    createSwDoc("tdf146272.odt");

    uno::Reference<beans::XPropertySet> xPicture(getShape(2), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDrawing(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xDrawing->getPropertyValue(u"TextBoxContent"_ustr),
                                               uno::UNO_QUERY);

    CPPUNIT_ASSERT(xPicture);
    CPPUNIT_ASSERT(xDrawing);
    CPPUNIT_ASSERT(xFrame);

    const sal_uInt64 nPitureZorder = xPicture->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>();
    const sal_uInt64 nDrawingZorder = xDrawing->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>();
    const sal_uInt64 nFrameZorder = xFrame->getPropertyValue(u"ZOrder"_ustr).get<sal_uInt64>();

    CPPUNIT_ASSERT_MESSAGE("Bad ZOrder!", nDrawingZorder < nFrameZorder);
    CPPUNIT_ASSERT_MESSAGE("Bad ZOrder!", nFrameZorder < nPitureZorder);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf138773)
{
    createSwDoc("tdf138773.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nFirstLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"2000-01\")])").toInt32();

    // This failed, if the first X axis label broke to multiple lines.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nFirstLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf124796)
{
    createSwDoc("tdf124796.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the minimum value of Y axis is not -10.
    assertXPathContent(pXmlDoc, "(//textarray)[5]/text", u"-10");

    // This failed, if the maximum value of Y axis is not 15.
    assertXPathContent(pXmlDoc, "(//textarray)[10]/text", u"15");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf72727)
{
    createSwDoc("tdf72727.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : Series1
    assertXPathContent(pXmlDoc, "(//textarray)[1]/text", u"1");
    assertXPathContent(pXmlDoc, "(//textarray)[2]/text", u"2");
    assertXPathContent(pXmlDoc, "(//textarray)[3]/text", u"3");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf130969)
{
    createSwDoc("tdf130969.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the minimum value of Y axis is not 0.35781
    assertXPathContent(pXmlDoc, "(//textarray)[5]/text", u"0.35781");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf40260)
{
    createSwDoc("tdf40260.odt");
    SwDocShell* pShell = getSwDocShell();

    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: f(x) = 1.26510397865547E-06 x − 5.95245604996327E-12
    // - Actual  : f(x) = 0 x − 0
    assertXPathContent(pXmlDoc, "(//textarray)[19]/text",
                       Concat2View("f(x) = 1.26510397865547E-06 x " + OUStringChar(u'\x2212')
                                   + " 5.95245604996327E-12"));
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf129054)
{
    createSwDoc("tdf129054.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Test the size of diameter of Pie chart.
    sal_Int32 nYTop
        = getXPath(pXmlDoc,
                   "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[4]/polyline[1]/point[1]",
                   "y")
              .toInt32();
    sal_Int32 nYBottom
        = getXPath(
              pXmlDoc,
              "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[4]/polyline[1]/point[31]",
              "y")
              .toInt32();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4615, nYTop - nYBottom, 5);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf129173)
{
    createSwDoc("testAreaChartNumberFormat.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the first data label of area chart.
    assertXPathContent(pXmlDoc, "(//textarray)[22]/text", u"56");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134866)
{
    createSwDoc("tdf134866.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the data label of pie chart.
    assertXPathContent(pXmlDoc, "(//textarray)[2]/text", u"100%");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf137116)
{
    createSwDoc("tdf137116.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nX2 = getXPath(pXmlDoc, "(//textarray)[2]", "x").toInt32(); // second data label
    sal_Int32 nX4 = getXPath(pXmlDoc, "(//textarray)[4]", "x").toInt32(); // fourth data label
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1229
    // - Actual  : -225
    // - Delta   : 100
    // i.e. the second data label appeared inside the pie slice.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1117, nX2 - nX4, 100);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf137154)
{
    createSwDoc("tdf137154.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nX1 = getXPath(pXmlDoc, "(//textarray)[1]", "x").toInt32(); // first data label
    sal_Int32 nX4 = getXPath(pXmlDoc, "(//textarray)[4]", "x").toInt32(); // fourth data label
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 10865
    // - Actual  : 10252
    // - Delta   : 50
    // i.e. the first data label appeared too close to the pie.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(nX4, nX1, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf138777)
{
    createSwDoc("outside_long_data_label.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nFirstLabelLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"really\")])").toInt32();

    // This failed, if the first data label didn't break to multiple lines.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(1), nFirstLabelLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf130031)
{
    createSwDoc("tdf130031.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc, "(//textarray)[11]", "y").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4653
    // - Actual  : 2182
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4653, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf130242)
{
    createSwDoc("tdf130242.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc, "(//textarray)[11]", "y").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 5758
    // - Actual  : 3352
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5758, nY, 50);

    nY = getXPath(pXmlDoc, "(//textarray)[13]", "y").toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2335
    // - Actual  : 2343
    // - Delta   : 50
    // i.e. the data label appeared above the data point.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2335, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134121)
{
    createSwDoc("piechart_leaderline.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // Check the custom leader line on pie chart.
    assertXPath(pXmlDoc, "//polyline", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf138018)
{
    createSwDoc("tdf138018.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 3
    // i.e. the leader line was visible.
    assertXPath(pXmlDoc, "//polyline", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf130380)
{
    createSwDoc("tdf130380.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nY = getXPath(pXmlDoc,
                            "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[1]/polypolygon/"
                            "polygon/point[1]",
                            "y")
                       .toInt32();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6727
    // - Actual  : 4411
    // - Delta   : 50
    // i.e. the area chart shrank.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6727, nY, 50);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf129095)
{
    createSwDoc("tdf129095.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // check the inner chart area (relative size) visibility with testing the X axis label
    assertXPathContent(pXmlDoc, "//textarray/text", u"Category 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf132956)
{
    createSwDoc("tdf132956.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // check the inner chart area (default size) visibility with testing the X axis label
    assertXPathContent(pXmlDoc, "//textarray/text", u"Category 1");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf137819)
{
    // Open the bugdoc and check if it went wrong
    createSwDoc("tdf137819.fodt");

    // Change the anchor if the textbox to As_char
    uno::Reference<beans::XPropertySet> xShapePropSet(getShape(1), uno::UNO_QUERY);
    xShapePropSet->setPropertyValue(
        u"AnchorType"_ustr,
        uno::Any(text::TextContentAnchorType::TextContentAnchorType_AS_CHARACTER));

    // Make the layout xml dump after the change
    auto pXml = parseLayoutDump();
    auto sTextRightSidePosition
        = getXPath(pXml, "/root/page/body/txt[6]/anchored/fly/infos/bounds", "right");
    auto sShapeRightSidePosition
        = getXPath(pXml, "/root/page/body/txt[6]/anchored/SwAnchoredDrawObject/bounds", "right");
    // Before the textframe did not follow the shape, now it supposed to
    // so the right side of the shape must be greater than the right side of
    // textframe:
    CPPUNIT_ASSERT(sTextRightSidePosition.toInt32() < sShapeRightSidePosition.toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testParagraphMarkInCell)
{
    createSwDoc("572-min.rtf");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xViewSettings(xViewSettingsSupplier->getViewSettings());
    uno::Any aOldHidden{ xViewSettings->getPropertyValue(u"ShowHiddenCharacters"_ustr) };
    uno::Any aOldNon{ xViewSettings->getPropertyValue(u"ShowNonprintingCharacters"_ustr) };
    comphelper::ScopeGuard g([&] {
        xViewSettings->setPropertyValue(u"ShowHiddenCharacters"_ustr, aOldHidden);
        xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, aOldNon);
    });

    xViewSettings->setPropertyValue(u"ShowHiddenCharacters"_ustr, uno::Any(true));
    xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, uno::Any(true));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "height", u"230");
        assertXPath(pXmlDoc, "/root/page/body/tab[1]/row[1]/infos/bounds", "height", u"690");
        assertXPath(pXmlDoc, "/root/page/body/tab[1]/row[2]/infos/bounds", "height", u"230");
        assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "height", u"230");
    }

    xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, uno::Any(false));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "height", u"230");
        // the problem was that the table rows were not hidden
        assertXPath(pXmlDoc, "/root/page/body/tab[1]/row[1]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page/body/tab[1]/row[2]/infos/bounds", "height", u"0");
        assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "height", u"230");
    }

    xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, uno::Any(true));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/body/txt[1]/infos/bounds", "height", u"230");
        assertXPath(pXmlDoc, "/root/page/body/tab[1]/row[1]/infos/bounds", "height", u"690");
        assertXPath(pXmlDoc, "/root/page/body/tab[1]/row[2]/infos/bounds", "height", u"230");
        assertXPath(pXmlDoc, "/root/page/body/txt[2]/infos/bounds", "height", u"230");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testParagraphMarkLineHeight)
{
    createSwDoc("A020-min.rtf");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> xViewSettingsSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xViewSettings(xViewSettingsSupplier->getViewSettings());
    uno::Any aOldHidden{ xViewSettings->getPropertyValue(u"ShowHiddenCharacters"_ustr) };
    uno::Any aOldNon{ xViewSettings->getPropertyValue(u"ShowNonprintingCharacters"_ustr) };
    comphelper::ScopeGuard g([&] {
        xViewSettings->setPropertyValue(u"ShowHiddenCharacters"_ustr, aOldHidden);
        xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, aOldNon);
    });

    xViewSettings->setPropertyValue(u"ShowHiddenCharacters"_ustr, uno::Any(true));
    xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, uno::Any(true));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/header/txt[1]/SwParaPortion/SwLineLayout[1]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[1]/SwParaPortion/SwLineLayout[2]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[1]/SwParaPortion/SwLineLayout[3]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[2]/SwParaPortion/SwLineLayout[1]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[2]/SwParaPortion/SwLineLayout[2]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[2]/SwParaPortion/SwLineLayout[3]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[2]/SwParaPortion/SwLineLayout[4]", "height",
                    u"184");
        assertXPath(pXmlDoc, "/root/page/header/txt[2]/SwParaPortion/SwLineLayout[5]", "height",
                    u"253");
    }

    xViewSettings->setPropertyValue(u"ShowNonprintingCharacters"_ustr, uno::Any(false));

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page/header/txt[1]/SwParaPortion/SwLineLayout[1]", "height",
                    u"253");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf122014)
{
    createSwDoc("tdf122014.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the chart title is aligned to left.
    sal_Int32 nX1 = getXPath(pXmlDoc, "(//textarray)[13]", "x").toInt32();
    sal_Int32 nX2 = getXPath(pXmlDoc, "(//textarray)[14]", "x").toInt32();
    CPPUNIT_ASSERT_GREATER(nX1 + 100, nX2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134659)
{
    createSwDoc("tdf134659.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // This failed, if the axis label is aligned to left.
    sal_Int32 nX1 = getXPath(pXmlDoc, "(//textarray)[1]", "x").toInt32();
    sal_Int32 nX2 = getXPath(pXmlDoc, "(//textarray)[2]", "x").toInt32();
    CPPUNIT_ASSERT_GREATER(nX1 + 250, nX2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134235)
{
    createSwDoc("tdf134235.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 13
    // i.e. the chart title flowed out of chart area.
    assertXPath(pXmlDoc, "//textarray", 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134676)
{
    createSwDoc("tdf134676.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 14
    // - Actual  : 13
    // i.e. the X axis title didn't break to multiple lines.
    assertXPath(pXmlDoc, "//textarray", 14);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134146)
{
    createSwDoc("tdf134146.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    const sal_Int32 nTitleLines
        = getXPathContent(pXmlDoc, "count(//text[contains(text(),\"Horizontal\")])").toInt32();

    // Without the accompanying fix in place, this test would have failed.
    // i.e. the Y axis title didn't break to multiple lines.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(1), nTitleLines);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf136061)
{
    createSwDoc("tdf136061.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // This failed, if the custom text of data label is missing.
    assertXPathContent(pXmlDoc, "(//textarray)[16]/text", u"Customlabel");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf116925)
{
    createSwDoc("tdf116925.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPathContent(pXmlDoc,
                       "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/textarray/text",
                       u"hello");
    // This failed, text color was #000000.
    assertXPath(
        pXmlDoc,
        "/metafile/push[1]/push[1]/push[1]/push[6]/push[1]/push[3]/textcolor[@color='#ffffff']", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf117028)
{
    createSwDoc("tdf117028.docx");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);

    // The only polypolygon in the rendering result was the white background we
    // want to avoid.
    assertXPath(pXmlDoc, "//polypolygon", 0);

    // Make sure the text is still rendered.
    assertXPathContent(pXmlDoc, "//textarray/text", u"Hello");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf106390)
{
    createSwDoc("tdf106390.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nBottom = getXPath(pXmlDoc, "//sectrectclipregion", "bottom").toInt32();

    // No end point of line segments shall go below the bottom of the clipping area.
    const OString sXPath = "//polyline/point[@y>" + OString::number(nBottom) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTableExtrusion1)
{
    createSwDoc("table-extrusion1.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    sal_Int32 nRight = getXPath(pXmlDoc, "//sectrectclipregion", "right").toInt32();
    sal_Int32 nLeft = static_cast<sal_Int32>(nRight * 0.95);

    // Expect table borders in right page margin.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nLeft) + " and @x<"
                           + OString::number(nRight) + "]";

    assertXPath(pXmlDoc, sXPath, 4);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTableExtrusion2)
{
    createSwDoc("table-extrusion2.odt");
    SwDocShell* pShell = getSwDocShell();

    // Dump the rendering of the first page as an XML file.
    std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
    MetafileXmlDump dumper;
    xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
    CPPUNIT_ASSERT(pXmlDoc);
    // End point position of the outer table.
    sal_Int32 nX = getXPath(pXmlDoc, "(//polyline[1]/point)[2]", "x").toInt32();

    // Do not allow inner table extrude outer table.
    const OString sXPath = "//polyline/point[@x>" + OString::number(nX) + "]";

    assertXPath(pXmlDoc, sXPath, 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf116848)
{
    createSwDoc("tdf116848.odt");
    // This resulted in a layout loop.
    calcLayout();
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf117245)
{
    createSwDoc("tdf117245.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was 2, TabOverMargin did not use a single line when there was
    // enough space for the text.
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout", 1);

    // This was 2, same problem elsewhere due to code duplication.
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf159029)
{
    createSwDoc("2024-01-19_merged-cells-on-separate-pages-vertical-alignment.odt");

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/infos/bounds", "top",
                    u"1970");
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/txt[1]/infos/bounds", "top",
                    u"1970");
    }

    // set vert orient
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable(UIName("Table1"));
    pWrtShell->Right(SwCursorSkipMode::Cells, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->SetBoxAlign(css::text::VertOrientation::BOTTOM);

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/infos/bounds", "top",
                    u"1970");
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/txt[1]/infos/bounds", "top",
                    u"15494");
    }

    // delete
    pWrtShell->SttEndDoc(true);
    pWrtShell->DelRight();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/infos/bounds", "top",
                    u"1694");
        // the problem was that this moved to the top of the cell
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/txt[1]/infos/bounds", "top",
                    u"15494");
    }

    pWrtShell->Undo();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/infos/bounds", "top",
                    u"1970");
        // the problem was that this moved to the top of the cell
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/txt[1]/infos/bounds", "top",
                    u"15494");
    }

    pWrtShell->Redo();

    {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/infos/bounds", "top",
                    u"1694");
        assertXPath(pXmlDoc, "/root/page[1]/body/tab[1]/row[1]/cell[2]/txt[1]/infos/bounds", "top",
                    u"15494");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf118672)
{
    createSwDoc("tdf118672.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check if we have hyphenation support, otherwise can't test SwHyphPortion.
    uno::Reference<linguistic2::XHyphenator> xHyphenator = LinguMgr::GetHyphenator();
    if (!xHyphenator->hasLocale(lang::Locale(u"en"_ustr, u"US"_ustr, OUString())))
        return;

    // This ended as "fol*1 2 3 4 5 6 7 8 9", i.e. "10con-" was moved to the next line.
    // Bizarrely, Word lets the text run into the right margin (loaded from .rtf)
    assertXPath(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]", "portion",
                u"He heard quiet steps behind him. That didn't bode well. Who could be fol*1 2 "
                "3 4 5 6 7 8 9 10consetetur");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf150200)
{
    createSwDoc("tdf150200.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // dash
    OUString sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"-(dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(93), sFirstLine.getLength());
    // en-dash
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"–(en-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(88), sFirstLine.getLength());
    // em-dash
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"—(em-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(77), sFirstLine.getLength());
    // figure dash
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‒(figure dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(87), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf150200_DOCX)
{
    createSwDoc("tdf150200.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // dash
    OUString sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"-(dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(93), sFirstLine.getLength());
    // en-dash
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"–(en-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(88), sFirstLine.getLength());
    // em-dash
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"—(em-dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(77), sFirstLine.getLength());
    // figure dash
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‒(figure dash)"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(87), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf150438)
{
    createSwDoc("tdf150438.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // left double quotation mark
    OUString sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"“Lorem ipsum"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(92), sFirstLine.getLength());
    // right double quotation mark
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"”Nunc viverra imperdiet enim."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), sFirstLine.getLength());
    // left single quotation mark
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‘Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
    // right single quotation mark or apostrophe
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"’Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf150438_DOCX)
{
    createSwDoc("tdf150438.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // left double quotation mark
    OUString sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[1]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"“Lorem ipsum"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(92), sFirstLine.getLength());
    // right double quotation mark
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"”Nunc viverra imperdiet enim."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(97), sFirstLine.getLength());
    // left single quotation mark
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[3]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"‘Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
    // right single quotation mark or apostrophe
    sFirstLine
        = getXPathContent(pXmlDoc, "/root/page/body/txt[4]/SwParaPortion/SwLineLayout[1]/@portion");
    CPPUNIT_ASSERT_EQUAL(true, sFirstLine.startsWith(u"’Aenean nec lorem."));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(85), sFirstLine.getLength());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf117923)
{
    createSwDoc("tdf117923.doc");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", u"GHI GHI GHI GHI");
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "type", u"PortionType::Number");
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"2.");
    // The numbering height was 960.
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
        "height", u"220");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf127606)
{
    createSwDoc("tdf117923.docx");
    // Ensure that all text portions are calculated before testing.
    SwViewShell* pViewShell = getSwDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    CPPUNIT_ASSERT(pViewShell);
    pViewShell->Reformat();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // Check that we actually test the line we need
    assertXPathContent(pXmlDoc, "/root/page/body/tab/row/cell/txt[3]", u"GHI GHI GHI GHI");
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "type", u"PortionType::Number");
    assertXPath(pXmlDoc,
                "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion",
                "expand", u"2.");
    // The numbering height was 960 in DOC format.
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
        "height", u"220");

    // tdf#127606: now it's possible to change formatting of numbering
    // increase font size (220 -> 260)
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Grow"_ustr, {});
    pViewShell->Reformat();
    pXmlDoc = parseLayoutDump();
    assertXPath(
        pXmlDoc,
        "/root/page/body/tab/row/cell/txt[3]/SwParaPortion/SwLineLayout/SwFieldPortion/SwFont",
        "height", u"260");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf127118)
{
    createSwDoc("tdf127118.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was Horizontal: merged cell split between pages didn't keep vertical writing direction
    assertXPath(pXmlDoc, "/root/page[2]/body/tab/row[1]/cell[1]/txt[1]", "WritingMode",
                u"VertBTLR");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf141220)
{
    createSwDoc("tdf141220.docx");

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    sal_Int32 nTextBoxTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    // Make sure the textbox stays inside the shape.
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, TestTextBoxChangeViaUNO)
{
    createSwDoc("TextBoxFrame.odt");
    // this file has a shape and a frame inside. Try to set up
    // the frame for the shape as textbox. Before this was not
    // implemented. This will be necessary for proper WPG import.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("There must be a shape and a frame!", 2, getShapes());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("This must be a custom shape!",
                                 u"com.sun.star.drawing.CustomShape"_ustr,
                                 getShape(1)->getShapeType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("This must be a frame shape!", u"FrameShape"_ustr,
                                 getShape(2)->getShapeType());

    CPPUNIT_ASSERT_MESSAGE("This is not supposed to be a textbox!",
                           !uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                                ->getPropertyValue(u"TextBox"_ustr)
                                .get<bool>());
    // Without the fix it will crash at this line:
    CPPUNIT_ASSERT_MESSAGE("This is not supposed to be a textbox!",
                           !uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                                ->getPropertyValue(u"TextBoxContent"_ustr)
                                .hasValue());

    // So now set the frame as textbox for the shape!
    uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
        ->setPropertyValue(u"TextBoxContent"_ustr, uno::Any(uno::Reference<text::XTextFrame>(
                                                       getShape(2), uno::UNO_QUERY_THROW)));

    CPPUNIT_ASSERT_MESSAGE("This is supposed to be a textbox!",
                           uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                               ->getPropertyValue(u"TextBox"_ustr)
                               .get<bool>());

    CPPUNIT_ASSERT_MESSAGE("This is supposed to be a textbox!",
                           uno::Reference<beans::XPropertySet>(getShape(1), uno::UNO_QUERY_THROW)
                               ->getPropertyValue(u"TextBoxContent"_ustr)
                               .hasValue());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf121509)
{
    createSwDoc("Tdf121509.odt");
    auto pDoc = getSwDoc();

    // Get all shape/frame formats
    auto vFrameFormats = pDoc->GetSpzFrameFormats();
    // Get the textbox
    auto xTextFrame = SwTextBoxHelper::getUnoTextFrame(getShape(1));
    // Get The triangle
    auto pTriangleShapeFormat = vFrameFormats->GetFormat(2);
    CPPUNIT_ASSERT(xTextFrame);
    CPPUNIT_ASSERT(pTriangleShapeFormat);

    // Get the position inside the textbox
    auto xTextContentStart = xTextFrame->getText()->getStart();
    SwUnoInternalPaM aCursor(*pDoc);
    CPPUNIT_ASSERT(sw::XTextRangeToSwPaM(aCursor, xTextContentStart));

    // Put the triangle into the textbox
    SwFormatAnchor aNewAnch(pTriangleShapeFormat->GetAnchor());
    aNewAnch.SetAnchor(aCursor.Start());
    CPPUNIT_ASSERT(pTriangleShapeFormat->SetFormatAttr(aNewAnch));

    save(u"Office Open XML Text"_ustr);

    // The second part: check if the reloaded doc has flys inside a fly
    // FIXME: if we use 'saveAndReload' or 'loadFromURL' here, it fails with
    //  Assertion `!m_pFirst && !m_pLast && "There are still indices registered"' failed.
    loadFromDesktop(maTempFile.GetURL(), u"com.sun.star.text.TextDocument"_ustr);
    auto pSecondDoc = getSwDoc();
    auto pSecondFormats = pSecondDoc->GetSpzFrameFormats();

    bool bFlyInFlyFound = false;
    for (auto secondformat : *pSecondFormats)
    {
        const SwNode* pNd = secondformat->GetAnchor().GetAnchorNode();
        if (pNd->FindFlyStartNode())
        {
            // So there is a fly inside another -> problem.
            bFlyInFlyFound = true;
            break;
        }
    }
    // Drop the tempfile
    maTempFile.CloseStream();

    // With the fix this cannot be true, if it is, that means Word unable to read the file..
    CPPUNIT_ASSERT_MESSAGE("Corrupt exported docx file!", !bFlyInFlyFound);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf134685)
{
    createSwDoc("tdf134685.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nWidth
        = getXPath(pXmlDoc, "/root/page/body/tab/row[6]/cell[1]/infos/bounds", "width").toInt32();
    // This was 2223: the content was only partially visible according to the lost cell width
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(14000), nWidth);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf109077)
{
    createSwDoc("tdf109077.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nShapeTop
        = getXPath(pXmlDoc, "//anchored/SwAnchoredDrawObject/bounds", "top").toInt32();
    sal_Int32 nTextBoxTop = getXPath(pXmlDoc, "//anchored/fly/infos/bounds", "top").toInt32();
    // This was 281: the top of the shape and its textbox should match, though
    // tolerate differences <= 1px (about 15 twips).
    CPPUNIT_ASSERT_LESS(static_cast<sal_Int32>(15), nTextBoxTop - nShapeTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testUserFieldTypeLanguage)
{
    // Set the system locale to German, the document will be English.
    SvtSysLocaleOptions aOptions;
    OUString sLocaleConfigString = aOptions.GetLanguageTag().getBcp47();
    aOptions.SetLocaleConfigString(u"de-DE"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g1([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });

    createSwDoc("user-field-type-language.fodt");
    SwDoc* pDoc = getSwDoc();
    SwViewShell* pViewShell = pDoc->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->UpdateFields();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // This was "123,456.00", via a buggy 1234.56 -> 1234,56 -> 123456 ->
    // 123,456.00 transform chain.
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion[@type='PortionType::Field']",
        "expand", u"1,234.56");

    // Now change the system locale to English (before this was failing, 1234,56 -> 0.00)
    aOptions.SetLocaleConfigString(u"en-GB"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g2([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });
    pViewShell->UpdateFields();
    pXmlDoc = parseLayoutDump();
    // We expect, that the field value is not changed. Otherwise there is a problem:
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion[@type='PortionType::Field']",
        "expand", u"1,234.56");
    // Now change the system locale to German
    aOptions.SetLocaleConfigString(u"de-DE"_ustr);
    aOptions.Commit();
    comphelper::ScopeGuard g3([&aOptions, &sLocaleConfigString] {
        aOptions.SetLocaleConfigString(sLocaleConfigString);
        aOptions.Commit();
    });
    pViewShell->UpdateFields();
    pXmlDoc = parseLayoutDump();
    // We expect, that the field value is not changed. Otherwise there is a problem:
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt/SwParaPortion/SwLineLayout/SwFieldPortion[@type='PortionType::Field']",
        "expand", u"1,234.56");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf124261)
{
#if !defined(_WIN32) && !defined(MACOSX)
    // Make sure that pressing a key in a btlr cell frame causes an immediate, correct repaint.
    createSwDoc("tdf124261.docx");
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPageFrame = pLayout->GetLower();
    CPPUNIT_ASSERT(pPageFrame->IsPageFrame());

    SwFrame* pBodyFrame = pPageFrame->GetLower();
    CPPUNIT_ASSERT(pBodyFrame->IsBodyFrame());

    SwFrame* pTabFrame = pBodyFrame->GetLower();
    CPPUNIT_ASSERT(pTabFrame->IsTabFrame());

    SwFrame* pRowFrame = pTabFrame->GetLower();
    CPPUNIT_ASSERT(pRowFrame->IsRowFrame());

    SwFrame* pCellFrame = pRowFrame->GetLower();
    CPPUNIT_ASSERT(pCellFrame->IsCellFrame());

    SwFrame* pFrame = pCellFrame->GetLower();
    CPPUNIT_ASSERT(pFrame->IsTextFrame());

    // Make sure that the text frame's area and the paint rectangle match.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 1721;
    // Actual: 1547', i.e. an area other than the text frame was invalidated for a single-line
    // paragraph.
    SwTextFrame* pTextFrame = static_cast<SwTextFrame*>(pFrame);
    SwRect aRect = pTextFrame->GetPaintSwRect();
    CPPUNIT_ASSERT_EQUAL(pTextFrame->getFrameArea().Top(), aRect.Top());
#endif
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf164932)
{
    // text frame with AutoText setting must use the left offset
    // of the list style of its text content
    createSwDoc("tdf164932.fodt");
    auto pDump = parseLayoutDump();
    // This was 4 (three lines in the text frame anchored as character in the first line
    assertXPath(pDump, "//SwLineLayout", 2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf164903)
{
    // ignore top margin of inline heading, like MSO does
    createSwDoc("tdf164903.docx");
    auto pDump = parseLayoutDump();
    auto nInlineHeadingHeight
        = getXPath(pDump, "/root/page[1]/body/txt[6]/infos/bounds", "height").toInt32();
    // This was 493 (adding top margin to the height of the Inline Heading frame)
    CPPUNIT_ASSERT_GREATER(sal_Int32(250), nInlineHeadingHeight);
    CPPUNIT_ASSERT_LESS(sal_Int32(260), nInlineHeadingHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf135991)
{
    createSwDoc("tdf135991.odt");
    auto pDump = parseLayoutDump();
    // There used to be negative values that made the column frames invisible.
    assertXPath(pDump, "//bounds[@top<0]", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf150642)
{
    createSwDoc("tdf150642.odt");
    auto pDump = parseLayoutDump();
    // There used to be negative values that made the cell frame invisible.
    assertXPath(pDump, "//bounds[@left<0]", 0);
    assertXPath(pDump, "//bounds[@right<0]", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf152085)
{
    createSwDoc("tdf152085-section-tblr.odt");
    auto pDump = parseLayoutDump();
    sal_Int32 nSectionHeight
        = getXPath(pDump, "//section/infos/bounds", "bottom").toInt32(); // was 8391
    sal_Int32 nColumnHeight
        = getXPath(pDump, "(//column/infos/bounds)[2]", "bottom").toInt32(); // was 16216
    CPPUNIT_ASSERT_MESSAGE("The column in a TBRL page should be shorter than the section.",
                           nColumnHeight <= nSectionHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf152031)
{
    createSwDoc("tdf152031-stair.odt");

    // reproduce the bug by shrinking the table width.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Shrink table width from 5" to 4"
    sal_Int32 nWidth = getProperty<sal_Int32>(xTable, u"Width"_ustr) * 4 / 5;

    uno::Reference<beans::XPropertySet> xSet(xTable, uno::UNO_QUERY);
    xSet->setPropertyValue(u"Width"_ustr, uno::Any(nWidth));

    auto pDump = parseLayoutDump();
    // There was a stair effect after change the table size.
    sal_Int32 nLeft_Row1 = getXPath(pDump, "(//row/infos/bounds)[1]", "left").toInt32();
    sal_Int32 nLeft_Row2 = getXPath(pDump, "(//row/infos/bounds)[2]", "left").toInt32();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("left values of SwRowFrames should be consistent.", nLeft_Row1,
                                 nLeft_Row2);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf153136)
{
    createSwDoc("tdf153136.docx");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    const sal_Int32 small = 300; // Small-height lines are around 276 twip
    const sal_Int32 large = 1000; // Large-height lines are 1104 twip or more

    // Page 1: standalone paragraphs

    // U+0009 CHARACTER TABULATION: height is ignored
    sal_Int32 height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[1]", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+0020 SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[2]", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+00A0 NO-BREAK SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[3]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+1680 OGHAM SPACE MARK: height is considered; not tested, because Liberation Serif lacks it

    // U+2000 EN QUAD: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[4]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2001 EM QUAD: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[5]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2002 EN SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[6]", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2003 EM SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[7]", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2004 THREE-PER-EM SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[8]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2005 FOUR-PER-EM SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[9]", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2006 SIX-PER-EM SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[10]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2007 FIGURE SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[11]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2008 PUNCTUATION SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[12]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2009 THIN SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[13]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+200A HAIR SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[14]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+202F NARROW NO-BREAK SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[1]//SwLineLayout)[15]", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+205F MEDIUM MATHEMATICAL SPACE: height is considered; not tested, because Liberation Serif lacks it
    // U+3000 IDEOGRAPHIC SPACE: height is ignored; not tested, because Liberation Serif lacks it

    // Page 2: table rows (no paragraph-level size DF)

    // U+0020 SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+00A0 NO-BREAK SPACE: height is considered (1104 or so)
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+1680 OGHAM SPACE MARK: height is considered; not tested, because Liberation Serif lacks it

    // U+2000 EN QUAD: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[3]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2001 EM QUAD: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[4]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2002 EN SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[5]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2003 EM SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[6]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2004 THREE-PER-EM SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[7]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2005 FOUR-PER-EM SPACE: height is ignored
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[8]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_LESS(small, height);

    // U+2006 SIX-PER-EM SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[9]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2007 FIGURE SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[10]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2008 PUNCTUATION SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[11]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+2009 THIN SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[12]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+200A HAIR SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[13]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+202F NARROW NO-BREAK SPACE: height is considered
    height = getXPath(pXmlDoc, "(/root/page[2]//row)[14]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(large, height);

    // U+205F MEDIUM MATHEMATICAL SPACE: height is considered; not tested, because Liberation Serif lacks it
    // U+3000 IDEOGRAPHIC SPACE: height is ignored; not tested, because Liberation Serif lacks it

    // TODO: page 3, with table having paragraphs with paragraph-level size DF;
    // all rows should have large height

    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[1]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[2]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[3]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[4]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[5]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[6]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[7]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[8]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[9]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[10]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[11]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[12]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[13]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
    // height = getXPath(pXmlDoc, "(/root/page[3]//row)[14]/infos/bounds", "height").toInt32();
    // CPPUNIT_ASSERT_GREATER(large, height);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testFoMargin)
{
    createSwDoc("fomargin.fodt");
    auto pXmlDoc = parseLayoutDump();

    auto nPage1Width
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(9600), nPage1Width);
    CPPUNIT_ASSERT_LESS(sal_Int32(9700), nPage1Width);

    auto nPage1Height
        = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(13800), nPage1Height);
    CPPUNIT_ASSERT_LESS(sal_Int32(14000), nPage1Height);

    auto nPage2Width
        = getXPath(pXmlDoc, "/root/page[2]/body/txt[1]/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(9600), nPage2Width);
    CPPUNIT_ASSERT_LESS(sal_Int32(9700), nPage2Width);

    auto nPage2Height
        = getXPath(pXmlDoc, "/root/page[2]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(13100), nPage2Height);
    CPPUNIT_ASSERT_LESS(sal_Int32(13400), nPage2Height);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf144450)
{
    createSwDoc("tdf144450.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Basic case: default, single-line spacing
    auto nCase1 = getXPath(pXmlDoc, "/root/page[1]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(3995), nCase1);
    CPPUNIT_ASSERT_LESS(sal_Int32(4005), nCase1);

    // Default spacing with snap-to-grid disabled
    auto nCase2 = getXPath(pXmlDoc, "/root/page[2]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(2175), nCase2);
    CPPUNIT_ASSERT_LESS(sal_Int32(2185), nCase2);

    // 1.15-line spacing
    auto nCase3 = getXPath(pXmlDoc, "/root/page[3]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(4295), nCase3);
    CPPUNIT_ASSERT_LESS(sal_Int32(4305), nCase3);

    // 1.5-line spacing
    auto nCase4 = getXPath(pXmlDoc, "/root/page[4]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(4995), nCase4);
    CPPUNIT_ASSERT_LESS(sal_Int32(5005), nCase4);

    // 2.0-line spacing
    auto nCase5 = getXPath(pXmlDoc, "/root/page[5]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(5995), nCase5);
    CPPUNIT_ASSERT_LESS(sal_Int32(6005), nCase5);

    // Prop spacing, 2.5x
    auto nCase6 = getXPath(pXmlDoc, "/root/page[6]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(6995), nCase6);
    CPPUNIT_ASSERT_LESS(sal_Int32(7005), nCase6);

    // At-least 0 spacing: this is a special case that disables extra grid spacing
    auto nCase7 = getXPath(pXmlDoc, "/root/page[7]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(2575), nCase7);
    CPPUNIT_ASSERT_LESS(sal_Int32(2585), nCase7);

    // At-least 0.01": should match the basic case
    auto nCase8 = getXPath(pXmlDoc, "/root/page[8]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(3995), nCase8);
    CPPUNIT_ASSERT_LESS(sal_Int32(4005), nCase8);

    // At-least 4"
    auto nCase9 = getXPath(pXmlDoc, "/root/page[9]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(11515), nCase9);
    CPPUNIT_ASSERT_LESS(sal_Int32(11525), nCase9);

    // Fixed 1"
    auto nCase10 = getXPath(pXmlDoc, "/root/page[10]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(2875), nCase10);
    CPPUNIT_ASSERT_LESS(sal_Int32(2885), nCase10);

    // Fixed 2"
    auto nCase11 = getXPath(pXmlDoc, "/root/page[11]/body/txt[1]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(5755), nCase11);
    CPPUNIT_ASSERT_LESS(sal_Int32(5765), nCase11);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf149089)
{
    createSwDoc("tdf149089.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Tests that kern portions aren't inserted for lines-and-chars grids when the
    // MS_WORD_COMP_GRID_METRICS compatibility flag is set.
    //
    // Without the fix, this would be 4
    sal_Int32 nKernPors
        = getXPathContent(pXmlDoc, "count(//SwLinePortion[@type='PortionType::Kern'])").toInt32();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nKernPors);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf165089)
{
    createSwDoc("tdf165089.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Tests that the grid is not centered when the MS_WORD_COMP_GRID_METRICS flag is set.
    //
    // Without the fix, this would be 6703
    auto nTop = getXPath(pXmlDoc, "/root/page/body/txt/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_GREATER(sal_Int32(1400), nTop);
    CPPUNIT_ASSERT_LESS(sal_Int32(1450), nTop);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf88908)
{
    createSwDoc("tdf88908.fodt");
    auto pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[1]", "portion",
                u"                                  Without  the  fix,  ");
    assertXPath(pXmlDoc, "/root/page/body/txt/SwParaPortion/SwLineLayout[2]", "portion",
                u"this  will  use  one  line");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166152)
{
    createSwDoc("tdf166152.fodt");

    auto* pWrtShell = getSwDocShell()->GetWrtShell();
    auto& rTextFrame
        = dynamic_cast<SwTextFrame&>(*pWrtShell->GetLayout()->GetLower()->GetLower()->GetLower());
    const SwScriptInfo* pSI = rTextFrame.GetScriptInfo();

    // Sanity check script changes before the edit
    CPPUNIT_ASSERT_EQUAL(size_t(8), pSI->CountScriptChg());

    std::vector<sal_Int32> aExpectedChgBefore{ 5, 10, 15, 20, 25, 30, 35, 40 };
    std::vector<sal_uInt8> aExpectedTypeBefore{ 3, 1, 3, 1, 3, 1, 3, 1 };
    for (size_t i = 0; i < 8; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(aExpectedChgBefore.at(i), pSI->GetScriptChg(i).get());
        CPPUNIT_ASSERT_EQUAL(aExpectedTypeBefore.at(i), pSI->GetScriptType(i));
    }

    // Insert text somewhere in the middle of the document.
    // This causes an incremental update of the script change vector.
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect*/ false, /*nCount*/ 17,
                     /*bBasicCall*/ false);
    pWrtShell->Insert(u"A"_ustr);

    // Check script changes after edit
    //
    // Without the fix, the count of script changes will be 11
    CPPUNIT_ASSERT_EQUAL(size_t(8), pSI->CountScriptChg());

    std::vector<sal_Int32> aExpectedChgAfter{ 5, 10, 15, 21, 26, 31, 36, 41 };
    std::vector<sal_uInt8> aExpectedTypeAfter{ 3, 1, 3, 1, 3, 1, 3, 1 };
    for (size_t i = 0; i < 8; ++i)
    {
        CPPUNIT_ASSERT_EQUAL(aExpectedChgAfter.at(i), pSI->GetScriptChg(i).get());
        CPPUNIT_ASSERT_EQUAL(aExpectedTypeAfter.at(i), pSI->GetScriptType(i));
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166210)
{
    // Given a document with a table, inside which there are two conditionally hidden sections
    createSwDoc("tdf166210.fodt");

    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection1 = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    auto xSection2 = xSections->getByName(u"Section2"_ustr).queryThrow<css::beans::XPropertySet>();

    Scheduler::ProcessEventsToIdle();
    auto pXmlDoc = parseLayoutDump();
    auto rowHeight1 = getXPath(pXmlDoc, "//body/tab/infos/bounds", "height").toInt32();

    // Hide first section
    xSection1->setPropertyValue(u"Condition"_ustr, css::uno::Any(u"1"_ustr));
    Scheduler::ProcessEventsToIdle();
    pXmlDoc = parseLayoutDump();
    auto rowHeight2 = getXPath(pXmlDoc, "//body/tab/infos/bounds", "height").toInt32();
    // Make sure that the table has shrunk its height
    CPPUNIT_ASSERT_LESS(rowHeight1, rowHeight2);

    // Hide second section
    xSection2->setPropertyValue(u"Condition"_ustr, css::uno::Any(u"1"_ustr));
    Scheduler::ProcessEventsToIdle();
    pXmlDoc = parseLayoutDump();
    auto rowHeight3 = getXPath(pXmlDoc, "//body/tab/infos/bounds", "height").toInt32();
    // Make sure that the table has shrunk its height
    CPPUNIT_ASSERT_LESS(rowHeight2, rowHeight3);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf88752)
{
    createSwDoc("tdf88752.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Without the fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 2
    assertXPath(pXmlDoc, "/root/page", 1);

    auto nHeight = getXPath(pXmlDoc, "/root/page[1]//row[3]//txt/infos/bounds", "height").toInt32();

    CPPUNIT_ASSERT_GREATER(sal_Int32(1670), nHeight);

    // Without the fix in place, this test would have failed with
    // - Expected less than: 1690
    // - Actual  : 3480
    CPPUNIT_ASSERT_LESS(sal_Int32(1690), nHeight);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166652)
{
    createSwDoc("line-spacing-80percent.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Check that all line heights are identical
    assertXPath(pXmlDoc, "//cell[1]/txt", 11);
    assertXPath(pXmlDoc, "//cell[1]/txt[1]/infos/bounds", "height", u"552");
    // Before the fix, this was 691
    assertXPath(pXmlDoc, "//cell[1]/txt[2]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[3]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[4]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[5]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[6]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[7]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[8]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[9]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[10]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[1]/txt[11]/infos/bounds", "height", u"552");

    assertXPath(pXmlDoc, "//cell[2]/txt", 11);
    assertXPath(pXmlDoc, "//cell[2]/txt[1]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[2]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[3]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[4]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[5]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[6]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[7]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[8]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[9]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[10]/infos/bounds", "height", u"552");
    assertXPath(pXmlDoc, "//cell[2]/txt[11]/infos/bounds", "height", u"552");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf165094)
{
    // Given a document that:
    // 1. Has 45 leading paragraphs (more than SwLayHelper::mnMaxParaPerPage);
    // 2. Has the next paragraph hidden;
    // 2.1. That hidden paragraph has "page break before with page style";
    // 2.2. That page style is different than page style used before;
    // 2.3. That hidden paragraph has "with page number", and the number is odd (previous page is
    //      also odd) - this is actually an alternative to 2.2;
    // 3. Then immediately goes a table.
    createSwDoc("tdf165094.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Check that the page break is created, with correct settings

    assertXPath(pXmlDoc, "//page", 3);

    assertXPath(pXmlDoc, "//page[1]", "formatName", u"Default Page Style");
    assertXPath(pXmlDoc, "//page[1]/page_info", "virtNum", u"1");
    assertXPath(pXmlDoc, "//page[1]/body/txt", 45);

    assertXPath(pXmlDoc, "//page[2]/page_info", "virtNum", u"2");
    assertXPath(pXmlDoc, "//page[2]/infos/bounds", "height", u"0"); // hidden
    assertXPath(pXmlDoc, "//page[2]/body", 0);

    assertXPath(pXmlDoc, "//page[3]", "formatName", u"Style1");
    assertXPath(pXmlDoc, "//page[3]/page_info", "virtNum", u"5");
    assertXPath(pXmlDoc, "//page[3]/body/txt[1]/infos/bounds", "height", u"0"); // hidden
    assertXPath(pXmlDoc, "//page[3]/body/tab", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166691)
{
    // Given a document with a table which disallows breaking across pages, and that has a cell
    // with an inner section with hide condition (evaluated to true), such as to have the cell
    // height enough to fit on page 1, when the section is hidden, but too tall when it's shown:
    createSwDoc("table-with-hidden-section.fodt");
    auto pXmlDoc = parseLayoutDump();

    // Check that there is only a single page
    assertXPath(pXmlDoc, "//page", 1);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166871)
{
    // Given a document with 6-column-layout pages, having hundreds of small paragraphs per page
    // (normal and headings); its metadata has the number of pages, paragraphs, etc.:
    createSwDoc("many-paragraphs-per-page-in-6-columns.odt");
    auto pXmlDoc = parseLayoutDump();

    // Check the layout - i.e., the count of paragraphs per column per page.
    // The metadata in the document must not prevent the correct layout.

    assertXPath(pXmlDoc, "//page", 12);

    // The first 11 pages have the same paragraph count in each column
    for (int page = 1; page <= 11; ++page)
    {
        OString Xpath1 = "//page[" + OString::number(page) + "]/body/column";
        assertXPath(pXmlDoc, Xpath1, 6);
        for (int column = 1; column <= 6; ++column)
        {
            OString Xpath2 = Xpath1 + "[" + OString::number(column) + "]/body/txt";
            assertXPath(pXmlDoc, Xpath2, 47);
        }
    }

    // Check the last page
    assertXPath(pXmlDoc, "//page[12]/body/column", 6);
    assertXPath(pXmlDoc, "//page[12]/body/column[1]/body/txt", 14);
    assertXPath(pXmlDoc, "//page[12]/body/column[2]/body/txt", 0);
    assertXPath(pXmlDoc, "//page[12]/body/column[3]/body/txt", 0);
    assertXPath(pXmlDoc, "//page[12]/body/column[4]/body/txt", 0);
    assertXPath(pXmlDoc, "//page[12]/body/column[5]/body/txt", 0);
    assertXPath(pXmlDoc, "//page[12]/body/column[6]/body/txt", 0);
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166978)
{
    // Given a document with a table, which cell has a section:
    createSwDoc("hideSection.fodt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Hide the section
    auto xTextSectionsSupplier = mxComponent.queryThrow<css::text::XTextSectionsSupplier>();
    auto xSections = xTextSectionsSupplier->getTextSections();
    CPPUNIT_ASSERT(xSections);
    auto xSection = xSections->getByName(u"Section1"_ustr).queryThrow<css::beans::XPropertySet>();
    xSection->setPropertyValue(u"IsVisible"_ustr, css::uno::Any(false));
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(1, getPages()); // must not split the second row of the table to page 2
    // Check the layout dump; the whole table must be still on the first page
    auto pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab", 1);
    assertXPath(pXmlDoc, "//page[1]/body/tab/row", 2);
    assertXPathContent(pXmlDoc, "//page[1]/body/tab/row[2]/cell[1]",
                       u"A2: this row suddenly jumps to the next page");
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf166181)
{
    // Given a document with a table with a merged cell, which gets split across pages:
    createSwDoc("tdf166181.fodt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // Make sure that the follow cell has the correct height
    auto pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page", 2);
    assertXPath(pXmlDoc, "//page[2]/body/tab", 1);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row", 2);
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/infos/bounds", "height", u"0");
    // Check that the first cell of the row is a follow (it has a precede)
    (void)getXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell[1]", "precede");
    // Check that it is merged
    assertXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell[1]", "rowspan", u"2");
    // Get cell height
    OUString height = getXPath(pXmlDoc, "//page[2]/body/tab/row[1]/cell[1]/infos/bounds", "height");
    // Without a fix, this was greater than 16000; it is expected around 400
    CPPUNIT_ASSERT_LESS(sal_Int32(500), height.toInt32());
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf167526)
{
    // Given a document with a floating table, immediately followed by a normal table:
    createSwDoc("tdf167526.docx");

    // check layout
    {
        // Make sure that the second node is a dummy node, and its line has height of 0
        auto pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//page['pass 1']", 1);
        assertXPath(pXmlDoc, "//body['pass 1']/txt[2]/anchored", 1);
        assertXPath(pXmlDoc, "//body['pass 1']/txt[2]/SwParaPortion/SwLineLayout", "height", u"0");
    }

    // DOCX roundtrip:
    saveAndReload(u"Office Open XML Text"_ustr);

    // check layout
    {
        // Make sure that the second node is a dummy node, and its line has height of 0
        auto pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//page['pass 2']", 1);
        assertXPath(pXmlDoc, "//body['pass 2']/txt[2]/anchored", 1);
        // Without a fix, this would fail, because the paragraph was visible after reload;
        // there were two SwLineLayout elements, each with non-0 height.
        assertXPath(pXmlDoc, "//body['pass 2']/txt[2]/SwParaPortion/SwLineLayout", "height", u"0");
    }

    // check DOCX export
    {
        auto pXmlDoc = parseExport(u"word/document.xml"_ustr);
        assertXPathNodeName(pXmlDoc, "/w:document/w:body/*[1]", "p");
        assertXPathNodeName(pXmlDoc, "/w:document/w:body/*[2]", "tbl");
        // Without a fix, this would fail, because the third element was the wrongly emitted <w:p>
        assertXPathNodeName(pXmlDoc, "/w:document/w:body/*[3]", "tbl");
    }

    // tdf#167535: check line numbering; the dummy node must not interfere with it
    {
        // Dump the rendering of the first page as an XML file.
        SwDocShell* pShell = getSwDocShell();
        std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
        MetafileXmlDump dumper;
        xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);
        // Without a fix, this was 3
        assertXPathContent(
            pXmlDoc, "/metafile/push/push/push/textarray[@index=0 and @length=1][2]/text", u"2");
    }
}

CPPUNIT_TEST_FIXTURE(SwLayoutWriter5, testTdf167540)
{
    // Given a document with a paragraph with a hard line break followed by nothing (tdf#167538);
    // and two floating tables with an empty paragraph between them (tdf#167540). The strings in
    // paragraphs all have different lengths, to allow XPath matching:
    createSwDoc("tdf167540.docx");

    // check line numbering
    auto verify_me = [this]() {
        // Dump the rendering of the first page as an XML file.
        SwDocShell* pShell = getSwDocShell();
        std::shared_ptr<GDIMetaFile> xMetaFile = pShell->GetPreviewMetaFile();
        MetafileXmlDump dumper;
        xmlDocUniquePtr pXmlDoc = dumpAndParse(dumper, *xMetaFile);

        // line 1

        assertXPathContent(pXmlDoc, "//textarray[@length=1][1]/text", u"1");
        auto ln1_y = getXPath(pXmlDoc, "//textarray[@length=1][1]", "y");

        assertXPathContent(pXmlDoc, "//textarray[@length=4]/text", u"Text\n");
        auto text1_y = getXPath(pXmlDoc, "//textarray[@length=4]", "y");

        // line numbering Y coordinate is the same as its line
        CPPUNIT_ASSERT_EQUAL(text1_y, ln1_y);

        // line 2 (empty)

        assertXPathContent(pXmlDoc, "//textarray[@length=1][2]/text", u"2");
        auto ln2_y = getXPath(pXmlDoc, "//textarray[@length=1][2]", "y");

        // line numbering for line 2 is indeed lower than for line 1
        CPPUNIT_ASSERT_GREATER(ln1_y.toInt32(), ln2_y.toInt32());

        // first floating table

        assertXPathContent(pXmlDoc, "//textarray[@length=20]/text", u"First floating table");
        auto table1_y = getXPath(pXmlDoc, "//textarray[@length=20]", "y");

        // the first floating table is indeed lower than line numbering for line 2
        CPPUNIT_ASSERT_GREATER(ln2_y.toInt32(), table1_y.toInt32());

        // line 3 (empty)

        assertXPathContent(pXmlDoc, "//textarray[@length=1][3]/text", u"3");
        auto ln3_y = getXPath(pXmlDoc, "//textarray[@length=1][3]", "y");

        // line numbering for line 3 is indeed lower than the first floating table
        CPPUNIT_ASSERT_GREATER(table1_y.toInt32(), ln3_y.toInt32());

        // second floating table

        assertXPathContent(pXmlDoc, "//textarray[@length=21]/text", u"Second floating table");
        auto table2_y = getXPath(pXmlDoc, "//textarray[@length=21]", "y");

        // the second floating table is indeed lower than line numbering for line 3
        CPPUNIT_ASSERT_GREATER(ln3_y.toInt32(), table2_y.toInt32());

        // Inline table

        assertXPathContent(pXmlDoc, "//textarray[@length=14]/text", u"A normal table");
        auto table3_y = getXPath(pXmlDoc, "//textarray[@length=14]", "y");

        // the inline table is indeed lower than second floating table
        CPPUNIT_ASSERT_GREATER(table2_y.toInt32(), table3_y.toInt32());

        // line 4

        assertXPathContent(pXmlDoc, "//textarray[@length=1][4]/text", u"4");
        auto ln4_y = getXPath(pXmlDoc, "//textarray[@length=1][4]", "y");

        assertXPathContent(pXmlDoc, "//textarray[@length=9]/text", u"More text");
        auto text4_y = getXPath(pXmlDoc, "//textarray[@length=9]", "y");

        // line numbering Y coordinate is the same as its line
        CPPUNIT_ASSERT_EQUAL(text4_y, ln4_y);
    };

    verify_me();

    // DOCX roundtrip:
    saveAndReload(u"Office Open XML Text"_ustr);

    verify_me();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
