/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <memory>

#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <test/htmltesttools.hxx>
#include <tools/urlobj.hxx>
#include <svtools/rtfkeywd.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <svtools/parrtf.hxx>
#include <rtl/strbuf.hxx>
#include <svtools/rtftoken.h>
#include <filter/msfilter/rtfutil.hxx>
#include <sot/storage.hxx>
#include <vcl/svapp.hxx>
#include <unotools/mediadescriptor.hxx>
#include <svtools/htmlcfg.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <swmodule.hxx>
#include <swdll.hxx>
#include <usrpref.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

namespace
{
/// Test RTF parser that just extracts a single OLE2 object from a file.
class TestReqIfRtfReader : public SvRTFParser
{
public:
    TestReqIfRtfReader(SvStream& rStream);
    void NextToken(int nToken) override;
    bool WriteObjectData(SvStream& rOLE);

private:
    bool m_bInObjData = false;
    OStringBuffer m_aHex;
};

TestReqIfRtfReader::TestReqIfRtfReader(SvStream& rStream)
    : SvRTFParser(rStream)
{
}

void TestReqIfRtfReader::NextToken(int nToken)
{
    switch (nToken)
    {
        case '}':
            m_bInObjData = false;
            break;
        case RTF_TEXTTOKEN:
            if (m_bInObjData)
                m_aHex.append(OUStringToOString(aToken, RTL_TEXTENCODING_ASCII_US));
            break;
        case RTF_OBJDATA:
            m_bInObjData = true;
            break;
    }
}

bool TestReqIfRtfReader::WriteObjectData(SvStream& rOLE)
{
    OString aObjdata = m_aHex.makeStringAndClear();

    SvMemoryStream aStream;
    int b = 0;
    int count = 2;

    // Feed the destination text to a stream.
    for (int i = 0; i < aObjdata.getLength(); ++i)
    {
        char ch = aObjdata[i];
        if (ch != 0x0d && ch != 0x0a)
        {
            b = b << 4;
            sal_Int8 parsed = msfilter::rtfutil::AsHex(ch);
            if (parsed == -1)
                return false;
            b += parsed;
            count--;
            if (!count)
            {
                aStream.WriteChar(b);
                count = 2;
                b = 0;
            }
        }
    }

    aStream.Seek(0);
    rOLE.WriteStream(aStream);
    return true;
}

/// Parser for [MS-OLEDS] 2.2.5 EmbeddedObject, aka OLE1.
struct OLE1Reader
{
    sal_uInt32 m_nNativeDataSize;
    sal_uInt32 m_nPresentationDataSize;

    OLE1Reader(SvStream& rStream);
};

OLE1Reader::OLE1Reader(SvStream& rStream)
{
    // Skip ObjectHeader, see [MS-OLEDS] 2.2.4.
    rStream.Seek(0);
    sal_uInt32 nData;
    rStream.ReadUInt32(nData); // OLEVersion
    rStream.ReadUInt32(nData); // FormatID
    rStream.ReadUInt32(nData); // ClassName
    rStream.SeekRel(nData);
    rStream.ReadUInt32(nData); // TopicName
    rStream.SeekRel(nData);
    rStream.ReadUInt32(nData); // ItemName
    rStream.SeekRel(nData);

    rStream.ReadUInt32(m_nNativeDataSize);
    rStream.SeekRel(m_nNativeDataSize);

    rStream.ReadUInt32(nData); // OLEVersion for presentation data
    CPPUNIT_ASSERT(rStream.good());
    rStream.ReadUInt32(nData); // FormatID
    rStream.ReadUInt32(nData); // ClassName
    rStream.SeekRel(nData);
    rStream.ReadUInt32(nData); // Width
    rStream.ReadUInt32(nData); // Height
    rStream.ReadUInt32(nData); // PresentationDataSize
    m_nPresentationDataSize = nData;
}
}

class HtmlExportTest : public SwModelTestBase, public HtmlTestTools
{
private:
    FieldUnit m_eUnit;

public:
    HtmlExportTest()
        : SwModelTestBase("/sw/qa/extras/htmlexport/data/", "HTML (StarWriter)")
        , m_eUnit(FieldUnit::NONE)
    {
    }

    /**
     * Wraps a reqif-xhtml fragment into an XHTML file, so an XML parser can
     * parse it.
     */
    static void wrapFragment(const utl::TempFile& rTempFile, SvMemoryStream& rStream)
    {
        rStream.WriteCharPtr(
            "<reqif-xhtml:html xmlns:reqif-xhtml=\"http://www.w3.org/1999/xhtml\">\n");
        SvFileStream aFileStream(rTempFile.GetURL(), StreamMode::READ);
        rStream.WriteStream(aFileStream);
        rStream.WriteCharPtr("</reqif-xhtml:html>\n");
        rStream.Seek(0);
    }

    /// Wraps an RTF fragment into a complete RTF file, so an RTF parser can handle it.
    static void wrapRtfFragment(const OUString& rURL, SvMemoryStream& rStream)
    {
        SvFileStream aRtfStream(rURL, StreamMode::READ);
        rStream.WriteOString("{\\rtf1");
        rStream.WriteStream(aRtfStream);
        rStream.WriteOString("}");
        rStream.Seek(0);
    }

private:
    bool mustCalcLayoutOf(const char* filename) override
    {
        return OString(filename) != "fdo62336.docx";
    }

    bool mustTestImportOf(const char* filename) const override
    {
        return OString(filename) != "fdo62336.docx";
    }

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (getTestName().indexOf("SkipImages") != -1)
            setFilterOptions("SkipImages");
        else if (getTestName().indexOf("EmbedImages") != -1)
            setFilterOptions("EmbedImages");
        else if (getTestName().indexOf("XHTML") != -1)
            setFilterOptions("XHTML");
        else if (getTestName().indexOf("ReqIf") != -1)
        {
            if (OString(filename).endsWith(".xhtml"))
            {
                setImportFilterOptions("xhtmlns=reqif-xhtml");
                // Bypass filter detect.
                setImportFilterName("HTML (StarWriter)");
            }
            // Export options (implies XHTML).
            setFilterOptions("xhtmlns=reqif-xhtml");
        }
        else
            setFilterOptions("");

        if (OString(filename) == "charborder.odt")
        {
            // FIXME if padding-top gets exported as inches, not cms, we get rounding errors.
            SwGlobals::ensure(); // make sure that SW_MOD() is not 0
            std::unique_ptr<Resetter> pResetter(new Resetter([this]() {
                SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
                pPref->SetMetric(this->m_eUnit);
            }));
            SwMasterUsrPref* pPref = const_cast<SwMasterUsrPref*>(SW_MOD()->GetUsrPref(false));
            m_eUnit = pPref->GetMetric();
            pPref->SetMetric(FieldUnit::CM);
            return pResetter;
        }
        return nullptr;
    }
};

#define DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(TestName, filename)                                      \
    DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, HtmlExportTest)

/// HTML export of the sw doc model tests.
class SwHtmlDomExportTest : public SwModelTestBase, public HtmlTestTools
{
public:
    /// Get the .ole path, assuming maTempFile is an XHTML export result.
    OUString GetOlePath();
    /// Parse the ole1 data out of an RTF fragment URL.
    void ParseOle1FromRtfUrl(const OUString& rRtfUrl, SvMemoryStream& rOle1);
};

OUString SwHtmlDomExportTest::GetOlePath()
{
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);
    OUString aOlePath = getXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "data");
    OUString aOleSuffix(".ole");
    CPPUNIT_ASSERT(aOlePath.endsWith(aOleSuffix));
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setBase(aOlePath.copy(0, aOlePath.getLength() - aOleSuffix.getLength()));
    aUrl.setExtension("ole");
    return aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE);
}

void SwHtmlDomExportTest::ParseOle1FromRtfUrl(const OUString& rRtfUrl, SvMemoryStream& rOle1)
{
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(rRtfUrl, aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    CPPUNIT_ASSERT(xReader->WriteObjectData(rOle1));
    CPPUNIT_ASSERT(rOle1.Tell());
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/htmlexport/data/";

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo81276, "fdo81276.html")
{
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("HTML"), uno::UNO_QUERY);
    // some rounding going on here?
    CPPUNIT_ASSERT(abs(sal_Int32(29700) - getProperty<sal_Int32>(xStyle, "Width")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(21006) - getProperty<sal_Int32>(xStyle, "Height")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, "LeftMargin")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, "RightMargin")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(2000) - getProperty<sal_Int32>(xStyle, "TopMargin")) < 10);
    CPPUNIT_ASSERT(abs(sal_Int32(500) - getProperty<sal_Int32>(xStyle, "BottomMargin")) < 10);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo62336, "fdo62336.docx")
{
    // The problem was essentially a crash during table export as docx/rtf/html
    // If either of no-calc-layout or no-test-import is enabled, the crash does not occur
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo86857, "fdo86857.html")
{
    // problem was that background color on page style was not exported
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("HTML"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(xStyle, "BackColor"));
    // check that table background color works, which still uses RES_BACKGROUND
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x66ffff), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testCharacterBorder, "charborder.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    // Different Border
    {
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x6666FF, 12, 12, 12, 3, 37),
                                    getProperty<table::BorderLine2>(xRun, "CharTopBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF9900, 0, 99, 0, 2, 99),
                                    getProperty<table::BorderLine2>(xRun, "CharLeftBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0xFF0000, 0, 169, 0, 1, 169),
                                    getProperty<table::BorderLine2>(xRun, "CharBottomBorder"));
        CPPUNIT_ASSERT_BORDER_EQUAL(table::BorderLine2(0x0000FF, 0, 169, 0, 0, 169),
                                    getProperty<table::BorderLine2>(xRun, "CharRightBorder"));
    }

    // Different Padding
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(450), getProperty<sal_Int32>(xRun, "CharTopBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(550),
                             getProperty<sal_Int32>(xRun, "CharLeftBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(150),
                             getProperty<sal_Int32>(xRun, "CharBottomBorderDistance"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(250),
                             getProperty<sal_Int32>(xRun, "CharRightBorderDistance"));
    }

    // No shadow
}

#define DECLARE_HTMLEXPORT_TEST(TestName, filename)                                                \
    DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, HtmlExportTest)

DECLARE_HTMLEXPORT_TEST(testExportOfImages, "textAndImage.docx")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 1);
}

DECLARE_HTMLEXPORT_TEST(testExportOfImagesWithSkipImagesEnabled, "textAndImage.docx")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p/img", 0);
}

DECLARE_HTMLEXPORT_TEST(testSkipImagesEmbedded, "skipimage-embedded.doc")
{
    // Embedded spreadsheet was exported as image, so content was lost. Make
    // sure it's exported as HTML instead.
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // This was 0.
    assertXPath(pDoc, "//table", 1);
    // This was 2, the HTML header was in the document two times.
    assertXPath(pDoc, "//meta[@name='generator']", 1);
    // This was 0, <table> was directly under <p>, which caused errors in the parser.
    assertXPath(pDoc, "//span/table", 1);
}

DECLARE_HTMLEXPORT_TEST(testSkipImagesEmbeddedDocument, "skipimage-embedded-document.docx")
{
    // Similar to testSkipImagesEmbedded, but with an embedded Writer object,
    // not a Calc one, and this time OOXML, not WW8.
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // This was 2, the HTML header was in the document two times.
    assertXPath(pDoc, "//meta[@name='generator']", 1);
    // Text of embedded document was missing.
    assertXPathContent(pDoc, "/html/body/p/span/p/span", "Inner.");
}

DECLARE_HTMLEXPORT_TEST(testExportImageProperties, "HTMLImage.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    assertXPath(pDoc, "/html/body/p/map/area", "shape", "poly");
    assertXPath(pDoc, "/html/body/p/map/area", "href", "http://www.microsoft.com/");
    assertXPath(pDoc, "/html/body/p/map/area", "target", "_self");
    assertXPath(pDoc, "/html/body/p/map/area", "alt", "microsoft");

    assertXPath(pDoc, "/html/body/p/a", 1);
    assertXPath(pDoc, "/html/body/p/a", "href", "http://www.google.com/");

    assertXPath(pDoc, "/html/body/p/a/font", 1);
    assertXPath(pDoc, "/html/body/p/a/font", "color", "#ff0000");

    assertXPath(pDoc, "/html/body/p/a/font/img", 1);
    assertXPath(pDoc, "/html/body/p/a/font/img", "name", "Text");
    assertXPath(pDoc, "/html/body/p/a/font/img", "alt", "Four colors");
    assertXPath(pDoc, "/html/body/p/a/font/img", "align", "middle");

    // Probably the DPI in OSX is different and Twip -> Pixel conversion produces
    // different results - so disable OSX for now.
    //
    // It would make sense to switch to use CSS and use "real world" units instead
    // i.e. (style="margin: 0cm 1.5cm; width: 1cm; height: 1cm")

#if 0 // disabled as it depends that the system DPI is set to 96
    assertXPath(pDoc, "/html/body/p/a/font/img", "hspace", "38");
    assertXPath(pDoc, "/html/body/p/a/font/img", "vspace", "19");
    assertXPath(pDoc, "/html/body/p/a/font/img", "width", "222");
    assertXPath(pDoc, "/html/body/p/a/font/img", "height", "222");
    assertXPath(pDoc, "/html/body/p/a/font/img", "border", "3");
#endif

    assertXPath(pDoc, "/html/body/p/a/font/img", "usemap", "#map1");
}

DECLARE_HTMLEXPORT_TEST(testExportCheckboxRadioButtonState, "checkbox-radiobutton.doc")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/p[1]/input", "type", "checkbox");
    assertXPath(pDoc, "/html/body/p[1]/input", "checked", "checked");
    assertXPath(pDoc, "/html/body/p[2]/input", "type", "checkbox");
    assertXPathNoAttribute(pDoc, "/html/body/p[2]/input", "checked");
    assertXPath(pDoc, "/html/body/form/p[1]/input", "type", "checkbox");
    assertXPath(pDoc, "/html/body/form/p[1]/input", "checked", "checked");
    assertXPath(pDoc, "/html/body/form/p[2]/input", "type", "checkbox");
    assertXPathNoAttribute(pDoc, "/html/body/form/p[2]/input", "checked");
    assertXPath(pDoc, "/html/body/form/p[3]/input", "type", "radio");
    assertXPath(pDoc, "/html/body/form/p[3]/input", "checked", "checked");
    assertXPath(pDoc, "/html/body/form/p[4]/input", "type", "radio");
    assertXPathNoAttribute(pDoc, "/html/body/form/p[4]/input", "checked");
}

DECLARE_HTMLEXPORT_TEST(testExportUrlEncoding, "tdf76291.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Test URI encoded hyperlink with Chinese characters
    assertXPath(pDoc, "/html/body/p/a", "href",
                "http://www.youtube.com/results?search_query=%E7%B2%B5%E8%AA%9Emv&sm=12");
}

DECLARE_HTMLEXPORT_TEST(testExportInternalUrl, "tdf90905.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Internal url should be valid
    assertXPath(pDoc, "/html/body/p[1]/a", "href", "#0.0.1.Text|outline");
    assertXPath(pDoc, "/html/body/p[2]/a", "href", "#bookmark");
}

DECLARE_HTMLEXPORT_TEST(testExportImageBulletList, "tdf66822.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Encoded base64 SVG bullet should match and render on browser
    assertXPath(pDoc, "/html/body/ul", 1);
    assertXPath(
        pDoc, "/html/body/ul", "style",
        "list-style-image: url(data:image/svg+xml;base64,"
        "PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4NCjwhLS0gR2VuZXJhdG9yOiBBZG9iZSBJbGx1c3"
        "RyYXRvciAxMi4wLjEsIFNWRyBFeHBvcnQgUGx1Zy1JbiAuIFNWRyBWZXJzaW9uOiA2LjAwIEJ1aWxkIDUxNDQ4KSAg"
        "LS0+DQo8IURPQ1RZUEUgc3ZnIFBVQkxJQyAiLS8vVzNDLy9EVEQgU1ZHIDEuMS8vRU4iICJodHRwOi8vd3d3LnczLm"
        "9yZy9HcmFwaGljcy9TVkcvMS4xL0RURC9zdmcxMS5kdGQiIFsNCgk8IUVOVElUWSBuc19zdmcgImh0dHA6Ly93d3cu"
        "dzMub3JnLzIwMDAvc3ZnIj4NCgk8IUVOVElUWSBuc194bGluayAiaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluay"
        "I+DQpdPg0KPHN2ZyAgdmVyc2lvbj0iMS4xIiBpZD0iTGF5ZXJfMSIgeG1sbnM9IiZuc19zdmc7IiB4bWxuczp4bGlu"
        "az0iJm5zX3hsaW5rOyIgd2lkdGg9IjE0LjAwOCIgaGVpZ2h0PSIxNC4wMSINCgkgdmlld0JveD0iMCAwIDE0LjAwOC"
        "AxNC4wMSIgb3ZlcmZsb3c9InZpc2libGUiIGVuYWJsZS1iYWNrZ3JvdW5kPSJuZXcgMCAwIDE0LjAwOCAxNC4wMSIg"
        "eG1sOnNwYWNlPSJwcmVzZXJ2ZSI+DQo8Zz4NCgk8cmFkaWFsR3JhZGllbnQgaWQ9IlhNTElEXzRfIiBjeD0iNy4wMD"
        "Q0IiBjeT0iNy4wMDQ5IiByPSI3LjAwNDQiIGdyYWRpZW50VW5pdHM9InVzZXJTcGFjZU9uVXNlIj4NCgkJPHN0b3Ag"
        "IG9mZnNldD0iMCIgc3R5bGU9InN0b3AtY29sb3I6IzM1REIzNSIvPg0KCQk8c3RvcCAgb2Zmc2V0PSIxIiBzdHlsZT"
        "0ic3RvcC1jb2xvcjojMDBBMDAwIi8+DQoJPC9yYWRpYWxHcmFkaWVudD4NCgk8Y2lyY2xlIGZpbGw9InVybCgjWE1M"
        "SURfNF8pIiBjeD0iNy4wMDQiIGN5PSI3LjAwNSIgcj0iNy4wMDQiLz4NCgk8ZGVmcz4NCgkJPGZpbHRlciBpZD0iQW"
        "RvYmVfT3BhY2l0eU1hc2tGaWx0ZXIiIGZpbHRlclVuaXRzPSJ1c2VyU3BhY2VPblVzZSIgeD0iMy40ODEiIHk9IjAu"
        "NjkzIiB3aWR0aD0iNi45ODgiIGhlaWdodD0iMy44OTMiPg0KCQkJPGZlQ29sb3JNYXRyaXggIHR5cGU9Im1hdHJpeC"
        "IgdmFsdWVzPSIxIDAgMCAwIDAgIDAgMSAwIDAgMCAgMCAwIDEgMCAwICAwIDAgMCAxIDAiLz4NCgkJPC9maWx0ZXI+"
        "DQoJPC9kZWZzPg0KCTxtYXNrIG1hc2tVbml0cz0idXNlclNwYWNlT25Vc2UiIHg9IjMuNDgxIiB5PSIwLjY5MyIgd2"
        "lkdGg9IjYuOTg4IiBoZWlnaHQ9IjMuODkzIiBpZD0iWE1MSURfNV8iPg0KCQk8ZyBmaWx0ZXI9InVybCgjQWRvYmVf"
        "T3BhY2l0eU1hc2tGaWx0ZXIpIj4NCgkJCTxsaW5lYXJHcmFkaWVudCBpZD0iWE1MSURfNl8iIGdyYWRpZW50VW5pdH"
        "M9InVzZXJTcGFjZU9uVXNlIiB4MT0iNy4xMjIxIiB5MT0iMC4xMDMiIHgyPSI3LjEyMjEiIHkyPSI1LjIzNDQiPg0K"
        "CQkJCTxzdG9wICBvZmZzZXQ9IjAiIHN0eWxlPSJzdG9wLWNvbG9yOiNGRkZGRkYiLz4NCgkJCQk8c3RvcCAgb2Zmc2"
        "V0PSIxIiBzdHlsZT0ic3RvcC1jb2xvcjojMDAwMDAwIi8+DQoJCQk8L2xpbmVhckdyYWRpZW50Pg0KCQkJPHJlY3Qg"
        "eD0iMy4xOTkiIHk9IjAuMzM5IiBvcGFjaXR5PSIwLjciIGZpbGw9InVybCgjWE1MSURfNl8pIiB3aWR0aD0iNy44ND"
        "YiIGhlaWdodD0iNC42MDEiLz4NCgkJPC9nPg0KCTwvbWFzaz4NCgk8ZWxsaXBzZSBtYXNrPSJ1cmwoI1hNTElEXzVf"
        "KSIgZmlsbD0iI0ZGRkZGRiIgY3g9IjYuOTc1IiBjeT0iMi42NCIgcng9IjMuNDk0IiByeT0iMS45NDYiLz4NCjwvZz"
        "4NCjwvc3ZnPg0K);");
}

DECLARE_HTMLEXPORT_TEST(testTdf83890, "tdf83890.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body/ol[2]/ol", "start", "2");
}

DECLARE_HTMLEXPORT_TEST(testExtbChars, "extb.html")
{
    OUString aExpected(u"\U00024b62");
    // Assert that UTF8 encoded non-BMP Unicode character is correct
    uno::Reference<text::XTextRange> xTextRange1 = getRun(getParagraph(1), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange1->getString());

    // Assert that non-BMP Unicode in character entity format is correct
    uno::Reference<text::XTextRange> xTextRange2 = getRun(getParagraph(2), 1);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange2->getString());
}

DECLARE_HTMLEXPORT_TEST(testNormalImageExport, "textAndImage.docx")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    // the HTML export normally does not embed the images
    OUString imgSrc = getXPath(pDoc, "/html/body/p/img", "src");
    CPPUNIT_ASSERT(imgSrc.endsWith(".png"));
}

DECLARE_HTMLEXPORT_TEST(testEmbedImagesEnabled, "textAndImage.docx")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);

    // the HTML export normally does not embed the images, but here the test
    // name triggers setting of the "EmbedImages" filter option, meaning the
    // image will not be a separate PNG, but an embedded base64 encoded
    // version of that
    OUString imgSrc = getXPath(pDoc, "/html/body/p/img", "src");
    CPPUNIT_ASSERT(imgSrc.startsWith("data:image/png;base64,"));
}

DECLARE_HTMLEXPORT_TEST(testXHTML, "hello.html")
{
    OString aExpected("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML");
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // This was HTML, not XHTML.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);
    // This was lang, not xml:lang.
    assertXPath(pDoc, "/html/body", "xml:lang", "en-US");
}

DECLARE_HTMLEXPORT_TEST(testReqIfParagraph, "reqif-p.xhtml")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();

    OString aExpected = "<reqif-xhtml:div><reqif-xhtml:p>aaa<reqif-xhtml:br/>\nbbb"
                        "</reqif-xhtml:p>" SAL_NEWLINE_STRING
                        // This was '<table' instead.
                        "<reqif-xhtml:table";

    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    pStream->Seek(0);
    OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // This was a HTML header, like '<!DOCTYPE html ...'.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    // This was "<a", was not found.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:a") != -1);

    // This was "<u>" instead of CSS.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"text-decoration: underline\"") != -1);

    // This was <strong>, namespace prefix was missing.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:strong>") != -1);

    // This was "<strike>" instead of CSS.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"text-decoration: line-through\"")
                   != -1);

    // This was "<font>" instead of CSS + namespace prefix was missing.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:span style=\"color: #ce181e\"") != -1);

    // This was '<a name="Bookmark 1"': missing namespace prefix, wrong
    // attribute name, wrong attribute value.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:a id=\"Bookmark_1\"></reqif-xhtml:a>") != -1);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOleData, "reqif-ole-data.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    // This was 0, <object> without URL was ignored.
    // Then this was 0 on export, as data of OLE nodes was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOleImg, "reqif-ole-img.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                               uno::UNO_QUERY);
    // This failed, OLE object had no replacement image.
    // And then it also failed when the export lost the replacement image.
    uno::Reference<graphic::XGraphic> xGraphic = xObject->getReplacementGraphic();
    // This failed when query and fragment of file:// URLs were not ignored.
    CPPUNIT_ASSERT(xGraphic.is());

    uno::Reference<drawing::XShape> xShape(xObject, uno::UNO_QUERY);
    OutputDevice* pDevice = Application::GetDefaultDevice();
    Size aPixel(64, 64);
    // Expected to be 1693.
    Size aLogic(pDevice->PixelToLogic(aPixel, MapMode(MapUnit::Map100thMM)));
    awt::Size aSize = xShape->getSize();
    // This was only 1247, size was not set explicitly.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aLogic.getWidth()), aSize.Width);

    // Check mime/media types.
    CPPUNIT_ASSERT_EQUAL(OUString("image/png"), getProperty<OUString>(xGraphic, "MimeType"));

    uno::Reference<document::XStorageBasedDocument> xStorageProvider(mxComponent, uno::UNO_QUERY);
    uno::Reference<embed::XStorage> xStorage = xStorageProvider->getDocumentStorage();
    auto aStreamName = getProperty<OUString>(xObject, "StreamName");
    uno::Reference<io::XStream> xStream
        = xStorage->openStreamElement(aStreamName, embed::ElementModes::READ);
    // This was empty when either import or export handling was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("text/rtf"), getProperty<OUString>(xStream, "MediaType"));

    // Check alternate text (it was empty, for export the 'alt' attribute was used).
    CPPUNIT_ASSERT_EQUAL(OUString("OLE Object"), getProperty<OUString>(xObject, "Title").trim());

    if (!mbExported)
        return;

    // "type" attribute was missing for the inner <object> element.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfPngImg, "reqif-png-img.xhtml")
{
    uno::Reference<container::XNamed> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());

    if (!mbExported)
    {
        // Imported PNG image is not an object.
        CPPUNIT_ASSERT_EQUAL(OUString("Image1"), xShape->getName());
        return;
    }

    // All images are exported as objects in ReqIF mode.
    CPPUNIT_ASSERT_EQUAL(OUString("Object1"), xShape->getName());

    // This was <img>, not <object>, which is not valid in the reqif-xhtml
    // subset.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:object") != -1);

    // Make sure that both RTF and PNG versions are written.
    CPPUNIT_ASSERT(aStream.indexOf("text/rtf") != -1);
    // This failed when images with a query in their file:// URL failed to
    // import.
    CPPUNIT_ASSERT(aStream.indexOf("image/png") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfJpgImg, "reqif-jpg-img.xhtml")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This was image/jpeg, JPG was not converted to PNG in ReqIF mode.
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfTable, "reqif-table.xhtml")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // <div> was missing, so the XHTML fragment wasn't a valid
    // xhtml.BlkStruct.class type anymore.
    assertXPath(pDoc, "/html/body/div/table/tr/th", 1);
    // The attribute was present to contain "background" and "border", which is
    // ignored in reqif-xhtml.
    assertXPathNoAttribute(pDoc, "/html/body/div/table/tr/th", "style");
    // The attribute was present, which is not valid in reqif-xhtml.
    assertXPathNoAttribute(pDoc, "/html/body/div/table/tr/th", "bgcolor");
}

DECLARE_HTMLEXPORT_TEST(testReqIfTable2, "reqif-table2.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <reqif-xhtml:td width="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:td>") != -1);
}

DECLARE_HTMLEXPORT_TEST(testXHTMLUseCSS, "xhtml-css.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <font face="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<span style=\"font-family:") != -1);
    // This failed, <font size="..."> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<span style=\"font-size:") != -1);
}

DECLARE_HTMLEXPORT_TEST(testReqIfList, "reqif-list.xhtml")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, <ul> was written.
    CPPUNIT_ASSERT(aStream.indexOf("<reqif-xhtml:ul>") != -1);

    // This failed, the 'style' attribute was written, even if the input did
    // not had one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aStream.indexOf(" style=\""));

    // This failed <li> was only opened, not closed.
    CPPUNIT_ASSERT(aStream.indexOf("</reqif-xhtml:li>") != -1);
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOle2, "reqif-ole2.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                               uno::UNO_QUERY);
    uno::Reference<io::XActiveDataStreamer> xEmbeddedObject(
        xObject->getExtendedControlOverEmbeddedObject(), uno::UNO_QUERY);
    // This failed, the "RTF fragment" native data was loaded as-is, we had no
    // filter to handle it, so nothing happened on double-click.
    CPPUNIT_ASSERT(xEmbeddedObject.is());
    uno::Reference<io::XSeekable> xStream(xEmbeddedObject->getStream(), uno::UNO_QUERY);
    // This was 38375, msfilter::rtfutil::ExtractOLE2FromObjdata() wrote
    // everything after the OLE1 header into the OLE2 stream, while the
    // Presentation field after the OLE2 data doesn't belong there.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(37888), xStream->getLength());
    // Finally the export also failed as it tried to open the stream from the
    // document storage, but the embedded object already opened it, so an
    // exception of type com.sun.star.io.IOException was thrown.

    if (mbExported)
    {
        // Check that the replacement graphic is exported at RTF level.
        SvMemoryStream aStream;
        wrapFragment(maTempFile, aStream);
        xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
        CPPUNIT_ASSERT(pDoc);
        // Get the path of the RTF data.
        OUString aOlePath = getXPath(
            pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "data");
        OUString aOleSuffix(".ole");
        CPPUNIT_ASSERT(aOlePath.endsWith(aOleSuffix));
        INetURLObject aUrl(maTempFile.GetURL());
        aUrl.setBase(aOlePath.copy(0, aOlePath.getLength() - aOleSuffix.getLength()));
        aUrl.setExtension("ole");
        OUString aOleUrl = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE);

        // Search for \result in the RTF data.
        SvFileStream aOleStream(aOleUrl, StreamMode::READ);
        CPPUNIT_ASSERT(aOleStream.IsOpen());
        OString aOleString(read_uInt8s_ToOString(aOleStream, aOleStream.TellEnd()));
        // Without the accompanying fix in place, this test would have failed,
        // replacement graphic was missing at RTF level.
        CPPUNIT_ASSERT(aOleString.indexOf(OOO_STRING_SVTOOLS_RTF_RESULT) != -1);
    }
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testReqIfOle2Odg, "reqif-ole-odg.xhtml")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier> xTextEmbeddedObject(xObjects->getByIndex(0),
                                                                          uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xObject(xTextEmbeddedObject->getEmbeddedObject(),
                                               uno::UNO_QUERY);
    // This failed, both import and export failed to handle OLE2 that contains
    // just ODF.
    CPPUNIT_ASSERT(xObject.is());
    CPPUNIT_ASSERT(xObject->supportsService("com.sun.star.drawing.DrawingDocument"));
}

DECLARE_HTMLEXPORT_TEST(testList, "list.html")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    // This failed, it was <li/>, i.e. list item was closed before content
    // started.
    CPPUNIT_ASSERT(aStream.indexOf("<li>") != -1);
}

DECLARE_HTMLEXPORT_TEST(testTransparentImage, "transparent-image.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    OUString aSource = getXPath(pDoc, "/html/body/p/img", "src");
    OUString aMessage = "src attribute is: " + aSource;
    // This was a jpeg, transparency was lost.
    CPPUNIT_ASSERT_MESSAGE(aMessage.toUtf8().getStr(), aSource.endsWith(".gif"));
}

DECLARE_HTMLEXPORT_TEST(testTransparentImageReqIf, "transparent-image.odt")
{
    SvMemoryStream aStream;
    wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    OUString aSource = getXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object",
        "data");
    OUString aMessage = "src attribute is: " + aSource;
    // This was GIF, when the intention was to force PNG.
    CPPUNIT_ASSERT_MESSAGE(aMessage.toUtf8().getStr(), aSource.endsWith(".png"));
}

DECLARE_HTMLEXPORT_TEST(testOleNodataReqIf, "reqif-ole-nodata.odt")
{
    // This failed, io::IOException was thrown during the filter() call.
    SvMemoryStream aStream;
    wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Make sure the native <object> element has the required data attribute.
    OUString aSource = getXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object",
        "data");
    CPPUNIT_ASSERT(!aSource.isEmpty());
}

DECLARE_HTMLEXPORT_TEST(testNoLangReqIf, "reqif-no-lang.odt")
{
    SvMemoryStream aStream;
    wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Make sure that xml:lang is not written in ReqIF mode.
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:h1", "lang");
}

DECLARE_HTMLEXPORT_TEST(testFieldShade, "field-shade.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with 'Expected: 1; Actual:
    // 0', i.e. shading for the field was lost.
    assertXPath(pDoc, "/html/body/p[1]/span", "style", "background: #c0c0c0");

    // Check that field shading is written only in case there is no user-defined span background.
    assertXPath(pDoc, "/html/body/p[2]/span", "style", "background: #ff0000");
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e there was an inner span hiding the wanted background color.
    assertXPath(pDoc, "/html/body/p[2]/span/span", 0);
}

DECLARE_HTMLEXPORT_TEST(testFieldShadeReqIf, "field-shade-reqif.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the ReqIF subset of xhtml had a background color and a page number field, resulting in
    // an invalid ReqIF-XHTML.
    assertXPath(pDoc, "/html/body/div/p[1]/span", 0);
    assertXPath(pDoc, "/html/body/div/p[1]/sdfield", 0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testBlockQuoteReqIf)
{
    // Build a document model that uses the Quotations paragraph style.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("ParaStyleName", uno::makeAny(OUString("Quotations")));

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("HTML (StarWriter)");
    aMediaDescriptor["FilterOptions"] <<= OUString("xhtmlns=reqif-xhtml");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <blackquote> had character (direct) children, which is invalid xhtml.
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:blockquote/reqif-xhtml:p", 1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testRTFOLEMimeType)
{
    // Import a document with an embedded object.
    OUString aType("test/rtf");
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "reqif-ole-data.xhtml";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties);

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("RTFOLEMimeType", aType),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: test/rtf
    // - Actual  : text/rtf
    // i.e. the MIME type was always text/rtf, not taking the store parameter into account.
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "type",
                aType);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testChinese)
{
    // Load a document with Chinese text in it.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "reqif-chinese.odt";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", {});

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };

    // Prevent parseXmlStream guess incompatible encoding and complaint.
    SvxHtmlOptions& rOptions = SvxHtmlOptions::Get();
    rtl_TextEncoding eOldEncoding = rOptions.GetTextEncoding();
    rOptions.SetTextEncoding(RTL_TEXTENCODING_UTF8);

    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);

    // Without the accompanying fix in place, this test would have failed as the output was not
    // well-formed.
    CPPUNIT_ASSERT(pDoc);
    rOptions.SetTextEncoding(eOldEncoding);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifComment)
{
    // Create a document with a comment in it.
    loadURL("private:factory/swriter", nullptr);
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Text", uno::makeAny(OUString("some text")) },
        { "Author", uno::makeAny(OUString("me")) },
    });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);

    // Without the accompanying fix in place, this test would have failed as the output was not
    // well-formed.
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifFontNameSize)
{
    // Create a document with a custom font name and size in it.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("CharFontName", uno::makeAny(OUString("Liberation Serif")));
    float fCharHeight = 14.0;
    xParagraph->setPropertyValue("CharHeight", uno::makeAny(fCharHeight));
    sal_Int32 nCharColor = 0xff0000;
    xParagraph->setPropertyValue("CharColor", uno::makeAny(nCharColor));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    xTextRange->setString("x");

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);

    // Make sure the output is well-formed.
    CPPUNIT_ASSERT(pDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. font name and size was written, even if that's not relevant for ReqIF.
    assertXPath(pDoc, "//reqif-xhtml:span", 1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifParagraphAlignment)
{
    // Create a document with an explicitly aligned paragraph.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue(
        "ParaAdjust", uno::makeAny(static_cast<sal_Int16>(style::ParagraphAdjust_RIGHT)));

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);

    // Make sure the output is well-formed.
    CPPUNIT_ASSERT(pDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected:
    // - Actual  : right
    // i.e. the <reqif-xhtml:p align="..."> markup was used, which is invalid.
    assertXPathNoAttribute(pDoc, "//reqif-xhtml:p", "align");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PDF)
{
    // Save to reqif-xhtml.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "pdf-ole.odt";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", {});

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    // Check the content of the ole1 data.
    OLE1Reader aOle1Reader(aOle1);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 39405
    // - Actual  : 43008
    // i.e. we did not work with the Ole10Native stream, rather created an OLE1 wrapper around the
    // OLE1-in-OLE2 data, resulting in additional size.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0x99ed), aOle1Reader.m_nNativeDataSize);

    // Now import this back and check the ODT result.
    mxComponent->dispose();
    mxComponent.clear();
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    mxComponent
        = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument", aLoadProperties);
    xStorable.set(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("writer8")),
    };
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProperties);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      aTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("Object 2"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    tools::SvRef<SotStorage> pStorage = new SotStorage(*pStream);
    tools::SvRef<SotStorageStream> pOleNative = pStorage->OpenSotStream("\1Ole10Native");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 39409
    // - Actual  : 0
    // i.e. we didn't handle the case when the ole1 payload was not an ole2 container. Note how the
    // expected value is the same as nData above + 4 bytes, since this data is length-prefixed.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(39409), pOleNative->GetSize());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1Paint)
{
    // Load the bug document, which has OLE1 data in it, which is not a wrapper around OLE2 data.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "paint-ole.xhtml";
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", aLoadProperties);

    // Save it as ODT to inspect the result of the OLE1 -> OLE2 conversion.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable.set(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("writer8")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("Object 2"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    tools::SvRef<SotStorage> pStorage = new SotStorage(*pStream);
    // Check the clsid of the root stream of the OLE2 storage.
    SvGlobalName aActual = pStorage->GetClassName();
    SvGlobalName aExpected(0x0003000A, 0, 0, 0xc0, 0, 0, 0, 0, 0, 0, 0x46);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0003000A-0000-0000-c000-000000000046
    // - Actual  : 0003000C-0000-0000-c000-000000000046
    // i.e. the "Package" clsid was used on the OLE2 storage unconditionally, even for an mspaint
    // case, which has its own clsid.
    CPPUNIT_ASSERT_EQUAL(aExpected.GetHexName(), aActual.GetHexName());

    aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    // Check the content of the ole1 data.
    // Skip ObjectHeader, see [MS-OLEDS] 2.2.4.
    aOle1.Seek(0);
    sal_uInt32 nData;
    aOle1.ReadUInt32(nData); // OLEVersion
    aOle1.ReadUInt32(nData); // FormatID
    aOle1.ReadUInt32(nData); // ClassName
    CPPUNIT_ASSERT(nData);
    OString aClassName = read_uInt8s_ToOString(aOle1, nData - 1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: PBrush
    // - Actual  : Package
    // i.e. a hardcoded class name was written.
    CPPUNIT_ASSERT_EQUAL(OString("PBrush"), aClassName);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testMultiParaListItem)
{
    // Create a document with 3 list items: A, B&C and D.
    loadURL("private:factory/swriter", nullptr);
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("A");
    SwDoc* pDoc = pWrtShell->GetDoc();
    {
        // Enable numbering.
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->nNode.GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert("B");
    pWrtShell->SplitNode();
    pWrtShell->Insert("C");
    {
        // C is in the same list item as B.
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->nNode.GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetCountedInList(false);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert("D");

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[1]/reqif-xhtml:p", "A");
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[2]/reqif-xhtml:p[1]", "B");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '//reqif-xhtml:ol/reqif-xhtml:li[2]/reqif-xhtml:p[2]' not found
    // i.e. </li> was written before "C", not after "C", so "C" was not in the 2nd list item.
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[2]/reqif-xhtml:p[2]", "C");
    assertXPathContent(pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li[3]/reqif-xhtml:p", "D");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testUnderlineNone)
{
    // Create a document with a single paragraph: its underlying is set to an explicit 'none' value.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertString(xText->getEnd(), "x", /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("CharUnderline",
                                 uno::makeAny(sal_Int16(awt::FontUnderline::NONE)));

    // Export to reqif-xhtml.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Make sure that the paragraph has no explicit style, because "text-decoration: none" is
    // filtered out.
    SvMemoryStream aStream;
    HtmlExportTest::wrapFragment(maTempFile, aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:div/reqif-xhtml:p", "style");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PresDataNoOle2)
{
    // Save to reqif-xhtml.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "no-ole2-pres-data.odt";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", {});
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    // Check the content of the ole1 data.
    // Without the accompanying fix in place, this test would have failed as there was no
    // presentation data after the native data in the OLE1 container. The result was not editable in
    // Word.
    OLE1Reader aOle1Reader(aOle1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PresDataWmfOnly)
{
    // Save to reqif-xhtml.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ole1-pres-data-wmf.odt";
    mxComponent = loadFromDesktop(aURL, "com.sun.star.text.TextDocument", {});
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);

    OLE1Reader aOle1Reader(aOle1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 135660
    // - Actual  : 272376
    // i.e. we wrote some additional EMF data into the WMF output, which broke Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(135660), aOle1Reader.m_nPresentationDataSize);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
