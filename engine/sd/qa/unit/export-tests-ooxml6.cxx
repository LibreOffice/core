/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <com/sun/star/drawing/XControlShape.hpp>
#include <test/unoapi_test.hxx>
#include <tools/color.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <officecfg/Office/Common.hxx>
#include <test/commontesttools.hxx>

#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <svx/unoapi.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/settings.hxx>
#include <vcl/themecolors.hxx>
#include <xmloff/autolayout.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <sdpage.hxx>
#include <SlideSectionManager.hxx>
#include <unomodel.hxx>

using namespace css;
using namespace ::cpo;
using namespace ::cpo::uno;

class SdOOXMLExportTest6 : public SdModelTestBase
{
public:
    SdOOXMLExportTest6()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// A presentation written under a dark appearance keeps automatic text readable: the saved colour
// is decided by the page background, not by the colour the application paints behind the page.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testAutomaticTextColorFollowsPageBackground)
{
    // The view takes the document background colour when it is created, so the dark appearance
    // has to be in place before the document is loaded.
    const AppearanceMode eOldMode = MiscSettings::GetAppColorMode();
    MiscSettings::SetAppColorMode(AppearanceMode::DARK);
    comphelper::ScopeGuard aResetMode([eOldMode] { MiscSettings::SetAppColorMode(eOldMode); });

    createSdImpressDoc("odp/automatic-text-color.fodp");
    save(TestFilter::PPTX);

    // The page carries no fill of its own, so its background is light and the text stays black.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr",
                "val", u"000000");

    // Text that takes its colour from the master stays readable as well: the default run
    // properties the master carries hold no white text colour.
    xmlDocUniquePtr pXmlMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlMaster, "//a:defRPr/a:solidFill/a:srgbClr[@val='FFFFFF']", 0);
}

// The placeholders of a master page are still there after the file has been saved twice. They are
// written to the layout, which is where reading the file back looks for them, so a master that
// went out once comes back whole and goes out whole again.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testMasterPlaceholdersSurviveRepeatedSaves)
{
    createSdImpressDoc("pptx/ShapeLineProperties.pptx");

    auto* pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);
    const size_t nShapes
        = pDocument->GetDoc()->GetMasterSdPage(0, PageKind::Standard)->GetObjCount();
    CPPUNIT_ASSERT_GREATER(size_t(1), nShapes);

    saveAndReload(TestFilter::PPTX);

    xmlDocUniquePtr pLayout = parseExport(u"ppt/slideLayouts/slideLayout1.xml"_ustr);
    assertXPath(pLayout, "/p:sldLayout/p:cSld/p:spTree/p:sp/p:nvSpPr/p:nvPr/p:ph", 5);

    pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);
    CPPUNIT_ASSERT_EQUAL(
        nShapes, pDocument->GetDoc()->GetMasterSdPage(0, PageKind::Standard)->GetObjCount());

    saveAndReload(TestFilter::PPTX);

    pDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pDocument);
    CPPUNIT_ASSERT_EQUAL(
        nShapes, pDocument->GetDoc()->GetMasterSdPage(0, PageKind::Standard)->GetObjCount());
}

// A paragraph that hangs its punctuation still does after the document has been saved. The
// attribute that carries it was never written, so every paragraph of a saved file came back
// with punctuation set inside the margin instead.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testHangingPunctuationSurvivesASave)
{
    createSdImpressDoc("pptx/3columns.pptx");

    uno::Reference<beans::XPropertySet> xParagraph(
        getParagraphFromShape(0, getShapeFromPage(0, 0)), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(true,
                         xParagraph->getPropertyValue(u"ParaIsHangingPunctuation"_ustr).get<bool>());

    saveAndReload(TestFilter::PPTX);

    xParagraph.set(getParagraphFromShape(0, getShapeFromPage(0, 0)), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(true,
                         xParagraph->getPropertyValue(u"ParaIsHangingPunctuation"_ustr).get<bool>());
}

// A diagram keeps the name the document gave it. Every diagram was written under a name made from
// the number it was counted with, so the name a reader shows for it changed on every save.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testDiagramKeepsItsName)
{
    createSdImpressDoc("pptx/Bar_List.pptx");

    uno::Reference<container::XNamed> xDiagram(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    const OUString aName = xDiagram->getName();
    CPPUNIT_ASSERT(!aName.isEmpty());

    saveAndReload(TestFilter::PPTX);

    xDiagram.set(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(aName, xDiagram->getName());
}

// A placeholder keeps the name the document gave it. Every placeholder was written under a name
// made from the number it was counted with, so the name a reader shows for the title of a slide
// changed to a generic one on every save.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testPlaceholderKeepsItsName)
{
    createSdImpressDoc("pptx/3columns.pptx");

    uno::Reference<container::XNamed> xPlaceholder(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    const OUString aName = xPlaceholder->getName();
    CPPUNIT_ASSERT(!aName.isEmpty());
    CPPUNIT_ASSERT(!aName.startsWith("PlaceHolder"));

    saveAndReload(TestFilter::PPTX);

    xPlaceholder.set(getShapeFromPage(0, 0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(aName, xPlaceholder->getName());
}

// The list style of a shape says nothing about the language, so the level it stands for does not
// hand the language of one run to every paragraph written at that level.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testListStyleStatesNoLanguage)
{
    createSdImpressDoc("pptx/tdf145162.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    // The run keeps the language it is written in, and the level beside it states none, which
    // is what lets the text styles of the master name one.
    assertXPathNoAttribute(
        pMaster, "/p:sldMaster/p:cSld/p:spTree/p:sp[1]/p:txBody/a:lstStyle/a:lvl1pPr/a:defRPr",
        "lang");
    assertXPath(pMaster, "/p:sldMaster/p:cSld/p:spTree/p:sp[1]/p:txBody/a:p/a:r/a:rPr", "lang",
                u"en-US");
}

// A list style states no colour for a level whose first run asks for the automatic one. The
// automatic colour is the colour the shape holding the text asks for, and every shape that takes
// this level asks for its own, so freezing one run's answer into the level hands it to them all.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testAutomaticColorStaysOutOfAListStyle)
{
    createSdImpressDoc("pptx/tdf157740.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pMaster, "//p:sp/p:txBody/a:lstStyle//a:defRPr/a:solidFill", 0);

    // The text itself still names a colour, because OOXML has no automatic one for a reader to
    // resolve.
    CPPUNIT_ASSERT(countXPathNodes(pMaster, "//p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill") > 0);
}

// The top and bottom insets of a shape stay where they are. A shape that states no height of its
// own was taken to have a text area of no height, and the insets were then written as what they
// would have to be to fit inside it, which is a negative distance.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testTextInsetsOfAShapeWithNoHeight)
{
    createSdImpressDoc("pptx/subtitle-animation-save.pptx");

    auto aTopInset = [this]() -> sal_Int32 {
        uno::Reference<drawing::XMasterPagesSupplier> xSupplier(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes(xSupplier->getMasterPages()->getByIndex(0),
                                                 uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xShape(xShapes->getByIndex(0), uno::UNO_QUERY_THROW);
        sal_Int32 nTop = 0;
        xShape->getPropertyValue(u"TextUpperDistance"_ustr) >>= nTop;
        return nTop;
    };

    const sal_Int32 nBefore = aTopInset();
    CPPUNIT_ASSERT_GREATER(sal_Int32(0), nBefore);

    saveAndReload(TestFilter::PPTX);

    CPPUNIT_ASSERT_EQUAL(nBefore, aTopInset());
}

// The pages of a Draw document keep their identity across sessions the same way the slides of
// a presentation do.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testPageGuidODG)
{
    createSdDrawDoc();
    auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    const OUString sPageGuid = pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString();
    const OUString sMasterGuid
        = pDoc->GetMasterSdPage(0, PageKind::Standard)->GetGuid().getOUString();

    saveAndReload(TestFilter::ODG);

    xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
    assertXPath(pContentXml, "/office:document-content/office:body/office:drawing/draw:page[1]",
                "guid", sPageGuid);
    xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
    assertXPath(pStylesXml, "/office:document-styles/office:master-styles/style:master-page[1]",
                "guid", sMasterGuid);

    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL(sPageGuid,
                         pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString());
    CPPUNIT_ASSERT_EQUAL(sMasterGuid,
                         pDoc->GetMasterSdPage(0, PageKind::Standard)->GetGuid().getOUString());
}

// The globally unique identifier of every page is written to ODF and read back, so a page keeps
// its identity across sessions.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testPageGuidODP)
{
    createSdImpressDoc();
    auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    const OUString sSlideGuid = pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString();
    const OUString sNotesGuid = pDoc->GetSdPage(0, PageKind::Notes)->GetGuid().getOUString();
    const OUString sMasterGuid
        = pDoc->GetMasterSdPage(0, PageKind::Standard)->GetGuid().getOUString();
    const OUString sNotesMasterGuid
        = pDoc->GetMasterSdPage(0, PageKind::Notes)->GetGuid().getOUString();
    const OUString sHandoutMasterGuid
        = pDoc->GetMasterSdPage(0, PageKind::Handout)->GetGuid().getOUString();

    saveAndReload(TestFilter::ODP);

    xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
    static constexpr OString sPagePath
        = "/office:document-content/office:body/office:presentation/draw:page[1]"_ostr;
    assertXPath(pContentXml, sPagePath, "guid", sSlideGuid);
    assertXPath(pContentXml, sPagePath + "/presentation:notes", "guid", sNotesGuid);

    xmlDocUniquePtr pStylesXml = parseExport(u"styles.xml"_ustr);
    static constexpr OString sMasterStylesPath
        = "/office:document-styles/office:master-styles"_ostr;
    assertXPath(pStylesXml, sMasterStylesPath + "/style:master-page[1]", "guid", sMasterGuid);
    assertXPath(pStylesXml, sMasterStylesPath + "/style:master-page[1]/presentation:notes", "guid",
                sNotesMasterGuid);
    assertXPath(pStylesXml, sMasterStylesPath + "/style:handout-master", "guid",
                sHandoutMasterGuid);

    // The reloaded document holds the same identifiers.
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL(sSlideGuid,
                         pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString());
    CPPUNIT_ASSERT_EQUAL(sNotesGuid,
                         pDoc->GetSdPage(0, PageKind::Notes)->GetGuid().getOUString());
    CPPUNIT_ASSERT_EQUAL(sMasterGuid,
                         pDoc->GetMasterSdPage(0, PageKind::Standard)->GetGuid().getOUString());
    CPPUNIT_ASSERT_EQUAL(sNotesMasterGuid,
                         pDoc->GetMasterSdPage(0, PageKind::Notes)->GetGuid().getOUString());
    CPPUNIT_ASSERT_EQUAL(sHandoutMasterGuid,
                         pDoc->GetMasterSdPage(0, PageKind::Handout)->GetGuid().getOUString());
}

// The identifier is also written to OOXML, in an extension list entry of the slide and of the
// slide master.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testPageGuidPPTX)
{
    createSdImpressDoc();
    auto* pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    SdDrawDocument* pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);

    const OUString sSlideGuid = pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString();
    const OUString sMasterGuid
        = pDoc->GetMasterSdPage(0, PageKind::Standard)->GetGuid().getOUString();

    saveAndReload(TestFilter::PPTX);

    xmlDocUniquePtr pSlideXml = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pSlideXml, "/p:sld/p:extLst/p:ext/coextml:pageGuid", "val", sSlideGuid);

    xmlDocUniquePtr pMasterXml = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pMasterXml, "/p:sldMaster/p:extLst/p:ext/coextml:pageGuid", "val", sMasterGuid);

    // The reloaded slide and master hold the identifiers they were saved with.
    pXImpressDocument = dynamic_cast<SdXImpressDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pXImpressDocument);
    pDoc = pXImpressDocument->GetDoc();
    CPPUNIT_ASSERT(pDoc);
    CPPUNIT_ASSERT_EQUAL(sSlideGuid,
                         pDoc->GetSdPage(0, PageKind::Standard)->GetGuid().getOUString());
    CPPUNIT_ASSERT_EQUAL(sMasterGuid,
                         pDoc->GetMasterSdPage(0, PageKind::Standard)->GetGuid().getOUString());
}

// A shape that takes its fill and its line from the theme keeps naming the theme, so that
// changing the theme still recolours it.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testShapeStyleThemeColor)
{
    createSdImpressDoc("pptx/connectors.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:solidFill/a:schemeClr", "val",
                u"lt1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:ln/a:solidFill/a:schemeClr",
                "val", u"dk1");

    // The font reference of the style names the theme color the text of the shape takes.
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:style/a:fontRef", "idx", u"minor");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:style/a:fontRef/a:schemeClr", "val",
                u"dk1");

    // A connector carries the style the same way.
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[1]/p:style/a:lnRef", "idx", u"1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:cxnSp[1]/p:style/a:lnRef/a:schemeClr", "val",
                u"accent1");
}

// The border of a table cell whose color comes from the theme keeps naming the theme.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testTableCellBorderThemeColor)
{
    createSdImpressDoc("pptx/tablescale.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    static constexpr OString aCell
        = "/p:sld/p:cSld/p:spTree/p:graphicFrame/a:graphic/a:graphicData/a:tbl/a:tr[1]/a:tc[1]/"
          "a:tcPr"_ostr;
    assertXPath(pXmlDoc, aCell + "/a:lnL/a:solidFill/a:schemeClr", "val", u"dk1");
    assertXPath(pXmlDoc, aCell + "/a:lnT/a:solidFill/a:schemeClr", "val", u"dk1");
}

// The master holds the text style of its title and of all nine outline levels.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testMasterTextStyles)
{
    createSdImpressDoc("pptx/tdf112209.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sldMaster/p:txStyles/p:titleStyle/a:lvl1pPr/a:defRPr", "sz", u"2600");
    assertXPath(pXmlDoc, "/p:sldMaster/p:txStyles/p:bodyStyle/a:lvl1pPr/a:defRPr", "sz", u"2000");
    assertXPath(pXmlDoc,
                "/p:sldMaster/p:txStyles/p:bodyStyle/a:lvl1pPr/a:defRPr/a:solidFill/a:schemeClr",
                "val", u"accent2");

    // The level a placeholder has no paragraph for is the one that used to go missing.
    assertXPath(pXmlDoc, "/p:sldMaster/p:txStyles/p:bodyStyle/a:lvl9pPr/a:defRPr", 1);
}

// A form control on a slide is written with the data that makes it a control, and is read back
// as one.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testFormControlExport)
{
    createSdImpressDoc("pptx/activex_checkbox.pptx");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:controls/p:control", 3);

    saveAndReload(TestFilter::PPTX);

    uno::Reference<drawing::XDrawPage> xPage(getPage(0));
    std::vector<OUString> aNames;
    for (sal_Int32 nShape = 0; nShape < xPage->getCount(); ++nShape)
    {
        uno::Reference<drawing::XControlShape> xControlShape(xPage->getByIndex(nShape),
                                                             uno::UNO_QUERY);
        if (!xControlShape.is())
            continue;
        uno::Reference<beans::XPropertySet> xModel(xControlShape->getControl(), uno::UNO_QUERY);
        OUString aName;
        xModel->getPropertyValue(u"Name"_ustr) >>= aName;
        aNames.push_back(aName);
    }
    CPPUNIT_ASSERT_EQUAL(size_t(3), aNames.size());
    CPPUNIT_ASSERT_EQUAL(u"CheckBox1"_ustr, aNames[0]);
}

// A fill whose theme color went out of date keeps the color it shows.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testFillWithStaleThemeColor)
{
    // Given a document with two shapes filled with #c2d2e1: the first still carries the theme
    // color dark1 (#062033), the second carries light2 at 90% brightness, which gives #c2d2e1.
    createSdImpressDoc("odp/fill-stale-theme-color.fodp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Without the fix in place, this test would have failed with:
    // - In <>, XPath '/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:solidFill/a:srgbClr' number of
    //   nodes is incorrect
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[1]/p:spPr/a:solidFill/a:srgbClr", "val",
                u"C2D2E1");
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp[2]/p:spPr/a:solidFill/a:schemeClr", "val",
                u"lt2");
}

// A run given a plain color keeps that color, even when its style names a theme color.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testCharColorOverThemedStyle)
{
    // Given a document with a text box whose graphic style colors its text with the theme color
    // accent1, and whose first run is colored #ff0000 with no theme color:
    createSdImpressDoc("odp/char-color-over-theme-style.fodp");
    save(TestFilter::PPTX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    // Without the fix in place, this test would have failed with:
    // - In <>, XPath '//p:txBody/a:p/a:r[1]/a:rPr/a:solidFill/a:srgbClr' number of nodes is
    //   incorrect
    // i.e. the run was written as the theme color accent1.
    assertXPath(pXmlDoc, "//p:txBody/a:p/a:r[1]/a:rPr/a:solidFill/a:srgbClr", "val", u"FF0000");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
