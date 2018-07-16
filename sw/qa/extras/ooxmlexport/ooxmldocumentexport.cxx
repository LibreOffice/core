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

//#include <ftninfo.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

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
};

class DocmTest : public SwModelTestBase
{
public:
    DocmTest()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "MS Word 2007 XML VBA")
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testSdtAlias, "sdt-alias.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // <w:alias> was completely missing.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:alias", "val", "Subtitle");
}

DECLARE_OOXMLEXPORT_TEST(testSdtDateCharformat, "sdt-date-charformat.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
    {
        // character formatting (bold) was missing, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:rPr/w:b", 1);
        // alias was also missing.
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:alias", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testFooterBodyDistance, "footer-body-distance.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // Page break was exported as section break, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testPlausableBorder, "plausable-border.docx")
{
    // sw::util::IsPlausableSingleWordSection() did not merge two page styles due to borders.
    if (xmlDocPtr pXmlDoc = parseExport())
        // Page break was exported as section break, this was 0
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", 1);
}

DECLARE_OOXMLEXPORT_TEST(testUnwantedSectionBreak, "unwanted-section-break.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // This was 2: an additional sectPr was added to the document.
        assertXPath(pXmlDoc, "//w:sectPr", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo80897, "fdo80897.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:bodyPr/a:prstTxWarp",
                "prst", "textTriangle");
}

DECLARE_OOXMLEXPORT_TEST(testFdo80902, "fdo80902.docx")
{
    // The problem was that the docGrid type was set as default so fix it for other grid type
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:docGrid", "type", "lines");
}

DECLARE_OOXMLEXPORT_TEST(testParaShading, "para-shading.docx")
{
    // Make sure the themeColor attribute is not written when it would be empty.
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
        assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:shd", "themeColor");
}

DECLARE_OOXMLEXPORT_TEST(testFirstHeaderFooter, "first-header-footer.docx")
{
    // Test import and export of a section's headerf/footerf properties.
    // (copied from a ww8export test, with doc converted to docx using Word)

    // The document has 6 pages. Note that we don't test if 4 or just 2 page
    // styles are created, the point is that layout should be correct.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header"),
                         parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer"),
                         parseDump("/root/page[1]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header"),
                         parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer"),
                         parseDump("/root/page[2]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header"), parseDump("/root/page[3]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer"), parseDump("/root/page[3]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page header2"),
                         parseDump("/root/page[4]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer 2"),
                         parseDump("/root/page[4]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page header 2"),
                         parseDump("/root/page[5]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Odd page footer 2"),
                         parseDump("/root/page[5]/footer/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page header 2"),
                         parseDump("/root/page[6]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Even page footer 2"),
                         parseDump("/root/page[6]/footer/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testFDO83044, "fdo83044.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:text", 1);
}

DECLARE_OOXMLEXPORT_TEST(testfdo83428, "fdo83428.docx")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps(
        xDocumentPropertiesSupplier->getDocumentProperties());
    uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Document"), getProperty<OUString>(xUDProps, "Testing"));
}

DECLARE_OOXMLEXPORT_TEST(testSdtIgnoredFooter, "sdt-ignored-footer.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This was 1, make sure no w:sdt sneaks into the main document from the footer.
        assertXPath(pXmlDoc, "//w:sdt", 0);
    }
}

DECLARE_OOXMLEXPORT_TEST(testSdtRunPicture, "sdt-run-picture.docx")
{
    // SDT around run was exported as SDT around paragraph
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        // This was 1: there was an SDT around w:p.
        assertXPath(pXmlDoc, "//w:body/w:sdt", 0);
        // This was 0: there were no SDT around w:r.
        assertXPath(pXmlDoc, "//w:body/w:p/w:sdt", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf89791, "tdf89791.docx")
{
    if (mbExported)
    {
        uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
            = packages::zip::ZipFileAccess::createWithURL(
                comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
        CPPUNIT_ASSERT_EQUAL(false, bool(xNameAccess->hasByName("docProps/custom.xml")));
    }
}

DECLARE_OOXMLEXPORT_TEST(testFDO63053, "fdo63053.docx")
{
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    CPPUNIT_ASSERT_EQUAL(OUString("test1&test2"), xDocumentProperties->getTitle());
    CPPUNIT_ASSERT_EQUAL(OUString("test1&test2"), xDocumentProperties->getSubject());
}

DECLARE_OOXMLEXPORT_TEST(testAuthorPropertySdt, "author-property.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "xpath",
                "/ns1:coreProperties[1]/ns0:creator[1]");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "storeItemID",
                "{6C3C8BC8-F283-45AE-878A-BAB7291924A1}");
    // FIXME: the next property doesn't match, though it's correct in theory. A bug in assertXPath?
    // assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w:dataBinding", "prefixMappings",
    //            "xmlns:ns0='http://purl.org/dc/elements/1.1/' xmlns:ns1='http://schemas.openxmlformats.org/package/2006/metadata/core-properties'");
}

DECLARE_OOXMLEXPORT_TEST(testfdo79817, "fdo79817.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "storeItemID",
                    "{9222E47B-A68B-4AEB-9855-21C912B9D3D2}");
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dataBinding", "xpath",
                    "/ns0:properties[1]/documentManagement[1]/"
                    "ns2:Responsible_x0020_Officer_x0020_Title[1]");
    }
}

DECLARE_OOXMLEXPORT_TEST(testSdt2Run, "sdt-2-run.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;

    // The problem was that <w:sdt> was closed after "first", not after "second", so the second assert failed.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[1]/w:t", "first");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t", "second");
    // Make sure the third portion is still outside <w:sdt>.
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/w:t", "third");
}

DECLARE_OOXMLEXPORT_TEST(testFD083057, "fdo83057.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/header1.xml");
    if (!pXmlDoc)
        return;

    // A fly frame was attached to a para which started with a hint (run) containing an SDT.
    // This SDT was handled while exporting the FLYFRAME and also the text of the run.
    // So, even though the original file had only one sdt in the header, the RT file had two;
    // one for a separate run as expected, and one incorrectly exported in the alternateContent (FLYFRAME)

    // Assert that the file has only one sdt, in a separate run
    assertXPath(pXmlDoc, "//w:sdt", 1);
    assertXPath(pXmlDoc, "//mc:AlternateContent//w:sdt", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf89774, "tdf89774.fodt")
{
    if (xmlDocPtr pXmlDoc = parseExport("docProps/app.xml"))
        // This was 65, as unit was seconds instead of minutes.
        assertXPathContent(pXmlDoc, "/extended-properties:Properties/extended-properties:TotalTime",
                           "1");
}

DECLARE_OOXMLEXPORT_TEST(testDocm, "hello.docm")
{
    // Make sure that we check the name of the export filter.
    // This was application/vnd.ms-word.document.macroEnabled.main+xml when the
    // name of the import filter was checked.
    if (xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml"))
        assertXPath(
            pXmlDoc, "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
            "ContentType",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
}

DECLARE_SW_ROUNDTRIP_TEST(testDocmSave, "hello.docm", nullptr, DocmTest)
{
    // This was
    // application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml,
    // we used the wrong content type for .docm files.
    if (xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml"))
        assertXPath(pXmlDoc,
                    "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
                    "ContentType", "application/vnd.ms-word.document.macroEnabled.main+xml");
}

DECLARE_SW_ROUNDTRIP_TEST(testBadDocm, "bad.docm", nullptr, DocmTest)
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // This was 'MS Word 2007 XML', broken docm files were not recognized.
    CPPUNIT_ASSERT_EQUAL(OUString("MS Word 2007 XML VBA"),
                         pTextDoc->GetDocShell()->GetMedium()->GetFilter()->GetName());
}

DECLARE_SW_ROUNDTRIP_TEST(testTdf108269, "tdf108269.docm", nullptr, DocmTest)
{
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // This failed: VBA streams were not roundtripped via the doc-level
    // grab-bag.
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/vbaProject.bin"));
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/vbaData.xml"));
}

DECLARE_OOXMLEXPORT_TEST(testGlossary, "testGlossary.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/glossary/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:glossaryDocument", "Ignorable", "w14 wp14");
}

DECLARE_OOXMLEXPORT_TEST(testOpenDocumentAsReadOnly, "open-as-read-only.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT(pTextDoc->GetDocShell()->IsSecurityOptOpenReadOnly());
}

DECLARE_OOXMLEXPORT_TEST(testTdf117805, "tdf117805.odt")
{
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // This failed, the header was lost. It's still referenced at an incorrect
    // location in document.xml, though.
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/header1.xml"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
