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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
