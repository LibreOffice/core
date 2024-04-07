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
#include <string_view>

#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

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
#include <vcl/graphicfilter.hxx>
#include <vcl/dibtools.hxx>
#include <o3tl/string_view.hxx>
#include <editeng/brushitem.hxx>

#include <swmodule.hxx>
#include <swdll.hxx>
#include <usrpref.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>
#include <formatlinebreak.hxx>
#include <itabenum.hxx>

namespace
{
/// Test RTF parser that just extracts a single OLE2 object from a file.
class TestReqIfRtfReader : public SvRTFParser
{
public:
    TestReqIfRtfReader(SvStream& rStream);
    void NextToken(int nToken) override;
    bool WriteObjectData(SvStream& rOLE);
    tools::Long GetObjw() const { return m_nObjw; }
    tools::Long GetObjh() const { return m_nObjh; }
    int getWmetafile() const { return m_nWmetafile; }

private:
    bool m_bInObjData = false;
    OStringBuffer m_aHex;
    tools::Long m_nObjw = 0;
    tools::Long m_nObjh = 0;
    int m_nWmetafile = 0;
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
        case RTF_OBJW:
            m_nObjw = nTokenValue;
            break;
        case RTF_OBJH:
            m_nObjh = nTokenValue;
            break;
        case RTF_WMETAFILE:
            m_nWmetafile = nTokenValue;
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
    std::vector<char> m_aNativeData;
    sal_uInt32 m_nPresentationDataSize;

    OLE1Reader(SvStream& rStream);
};

OLE1Reader::OLE1Reader(SvStream& rStream)
{
    // Skip ObjectHeader, see [MS-OLEDS] 2.2.4.
    rStream.Seek(0);
    CPPUNIT_ASSERT(rStream.remainingSize());
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
    m_aNativeData.resize(m_nNativeDataSize);
    rStream.ReadBytes(m_aNativeData.data(), m_aNativeData.size());

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

/// Covers sw/source/filter/html/wrthtml.cxx and related fixes.
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
        return filename != std::string_view("fdo62336.docx");
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
            if (o3tl::ends_with(filename, ".xhtml"))
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

        if (filename == std::string_view("charborder.odt"))
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
    SwHtmlDomExportTest()
        : SwModelTestBase("/sw/qa/extras/htmlexport/data/")
    {
    }

    OUString GetObjectPath(const OUString& ext);
    /// Get the .ole path, assuming maTempFile is an XHTML export result.
    OUString GetOlePath() { return GetObjectPath(".ole"); }
    OUString GetPngPath() { return GetObjectPath(".png"); }
    /// Parse the ole1 data out of an RTF fragment URL.
    void ParseOle1FromRtfUrl(const OUString& rRtfUrl, SvMemoryStream& rOle1);
    /// Export using the C++ HTML export filter, with xhtmlns=reqif-xhtml.
    void ExportToReqif();
    /// Import using the C++ HTML import filter, with xhtmlns=reqif-xhtml.
    void ImportFromReqif(const OUString& rUrl);
};

OUString SwHtmlDomExportTest::GetObjectPath(const OUString& ext)
{
    assert(ext.startsWith("."));
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);
    OUString aOlePath = getXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "data");
    CPPUNIT_ASSERT(aOlePath.endsWith(ext));
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setBase(aOlePath.subView(0, aOlePath.getLength() - ext.getLength()));
    aUrl.setExtension(ext.subView(1));
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

void SwHtmlDomExportTest::ExportToReqif()
{
    setFilterOptions("xhtmlns=reqif-xhtml");
    save("HTML (StarWriter)");
}

void SwHtmlDomExportTest::ImportFromReqif(const OUString& rUrl)
{
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
    };
    mxComponent = loadFromDesktop(rUrl, "com.sun.star.text.TextDocument", aLoadProperties);
}

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

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testFdo62336)
{
    // The problem was essentially a crash during table export as docx/rtf/html
    // If calc-layout is enabled, the crash does not occur
    loadAndReload("fdo62336.docx");
}

DECLARE_HTMLEXPORT_ROUNDTRIP_TEST(testFdo86857, "fdo86857.html")
{
    // problem was that background color on page style was not exported
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xStyle(xPageStyles->getByName("HTML"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xStyle, "BackColor"));
    // check that table background color works, which still uses RES_BACKGROUND
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x66ffff), getProperty<Color>(xCell, "BackColor"));
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

    // This was '<reqif-xhtml:a id="...">': non-unique bookmark name in reqif fragment.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aStream.indexOf("<reqif-xhtml:a id="));
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

    uno::Reference<beans::XPropertySet> xObjectProps(xObject, uno::UNO_QUERY);
    uno::Reference<io::XActiveDataStreamer> xStreamProvider(
        xObjectProps->getPropertyValue("EmbeddedObject"), uno::UNO_QUERY);
    uno::Reference<io::XSeekable> xStream(xStreamProvider->getStream(), uno::UNO_QUERY);
    // This was empty when either import or export handling was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int64>(37888), xStream->getLength());

    // Check alternate text (it was empty, for export the 'alt' attribute was used).
    CPPUNIT_ASSERT_EQUAL(OUString("OLE Object"), getProperty<OUString>(xObject, "Title").trim());

    if (!isExported())
        return;

    // "type" attribute was missing for the inner <object> element.
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("type=\"image/png\"") != -1);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIfPngImg)
{
    auto verify = [this](bool bExported) {
        uno::Reference<container::XNamed> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShape.is());

        if (!bExported)
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
    };

    ImportFromReqif(createFileURL(u"reqif-png-img.xhtml"));
    verify(/*bExported=*/false);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportImagesAsOLE", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    mxComponent->dispose();
    ImportFromReqif(maTempFile.GetURL());
    verify(/*bExported=*/true);
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
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // <div> was missing, so the XHTML fragment wasn't a valid
    // xhtml.BlkStruct.class type anymore.
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr/reqif-xhtml:th",
                1);
    // Make sure that the cell background is not written using CSS.
    assertXPathNoAttribute(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr/reqif-xhtml:th",
        "style");
    // The attribute was present, which is not valid in reqif-xhtml.
    assertXPathNoAttribute(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr/reqif-xhtml:th",
        "bgcolor");
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

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIfTableHeight)
{
    // Given a document with a table in it, with an explicit row height:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aTableProperties = {
        comphelper::makePropertyValue("Rows", static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue("Columns", static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, ".uno:InsertTable", aTableProperties);
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xRow(xTable->getRows()->getByIndex(0), uno::UNO_QUERY);
    xRow->setPropertyValue("Height", uno::Any(static_cast<sal_Int32>(1000)));

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that the explicit cell height is omitted from the output:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed, explicit height was
    // written, which is not valid reqif-xhtml.
    assertXPathNoAttribute(pDoc, "//reqif-xhtml:td", "height");
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

    if (isExported())
    {
        // Check that the replacement graphic is exported at RTF level.
        SvMemoryStream aStream;
        WrapReqifFromTempFile(aStream);
        xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
        CPPUNIT_ASSERT(pDoc);
        // Get the path of the RTF data.
        OUString aOlePath = getXPath(
            pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object", "data");
        OUString aOleSuffix(".ole");
        CPPUNIT_ASSERT(aOlePath.endsWith(aOleSuffix));
        INetURLObject aUrl(maTempFile.GetURL());
        aUrl.setBase(aOlePath.subView(0, aOlePath.getLength() - aOleSuffix.getLength()));
        aUrl.setExtension(u"ole");
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

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTransparentImageReqIf)
{
    createSwDoc("transparent-image.odt");
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportImagesAsOLE", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
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
    WrapReqifFromTempFile(aStream);
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
    WrapReqifFromTempFile(aStream);
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

DECLARE_HTMLEXPORT_TEST(testTdf132739, "tdf132739.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    // Without the fix in place, this test would have failed with
    // - Expected: background: #5983b0; border: 1px solid #333333; padding: 0.04in
    // - Actual  : background: #5983b0
    assertXPath(pDoc, "/html/body/table/tr[1]/td", "style",
                "background: #5983b0; border: 1px solid #333333; padding: 0.04in");
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

DECLARE_HTMLEXPORT_TEST(testTdf126879, "tdf126879.odt")
{
    const OString aExpected("<!DOCTYPE html>");
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    const OString aActual(read_uInt8s_ToOString(*pStream, aExpected.getLength()));
    // Without the fix in place, this test would have failed with
    // - Expected: <!DOCTYPE html>
    // - Actual  : <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testBlockQuoteReqIf)
{
    // Build a document model that uses the Quotations paragraph style.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("ParaStyleName", uno::Any(OUString("Quotations")));

    // Export it.
    setFilterOptions("xhtmlns=reqif-xhtml");
    save("HTML (StarWriter)");
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
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
    ImportFromReqif(createFileURL(u"reqif-ole-data.xhtml"));

    // Export it.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("RTFOLEMimeType", aType),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
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
    createSwDoc("reqif-chinese.odt");

    // Export it.
    ExportToReqif();
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);

    // Without the accompanying fix in place, this test would have failed as the output was not
    // well-formed.
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifComment)
{
    // Create a document with a comment in it.
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Text", uno::Any(OUString("some text")) },
        { "Author", uno::Any(OUString("me")) },
    });
    dispatchCommand(mxComponent, ".uno:InsertAnnotation", aPropertyValues);

    // Export it.
    ExportToReqif();
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);

    // Without the accompanying fix in place, this test would have failed as the output was not
    // well-formed.
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifFontNameSize)
{
    // Create a document with a custom font name and size in it.
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("CharFontName", uno::Any(OUString("Liberation Serif")));
    float fCharHeight = 14.0;
    xParagraph->setPropertyValue("CharHeight", uno::Any(fCharHeight));
    sal_Int32 nCharColor = 0xff0000;
    xParagraph->setPropertyValue("CharColor", uno::Any(nCharColor));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    xTextRange->setString("x");

    // Export it.
    ExportToReqif();
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
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
    createSwDoc();
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("ParaAdjust",
                                 uno::Any(static_cast<sal_Int16>(style::ParagraphAdjust_RIGHT)));

    // Export it.
    ExportToReqif();
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
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
    createSwDoc("pdf-ole.odt");

    ExportToReqif();
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
    ImportFromReqif(maTempFile.GetURL());
    save("writer8");
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("Object 2"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    rtl::Reference<SotStorage> pStorage = new SotStorage(*pStream);
    rtl::Reference<SotStorageStream> pOleNative = pStorage->OpenSotStream("\1Ole10Native");
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
    ImportFromReqif(createFileURL(u"paint-ole.xhtml"));

    // Save it as ODT to inspect the result of the OLE1 -> OLE2 conversion.
    save("writer8");
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName("Object 2"),
                                                  uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    rtl::Reference<SotStorage> pStorage = new SotStorage(*pStream);
    // Check the clsid of the root stream of the OLE2 storage.
    SvGlobalName aActual = pStorage->GetClassName();
    SvGlobalName aExpected(0x0003000A, 0, 0, 0xc0, 0, 0, 0, 0, 0, 0, 0x46);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0003000A-0000-0000-c000-000000000046
    // - Actual  : 0003000C-0000-0000-c000-000000000046
    // i.e. the "Package" clsid was used on the OLE2 storage unconditionally, even for an mspaint
    // case, which has its own clsid.
    CPPUNIT_ASSERT_EQUAL(aExpected.GetHexName(), aActual.GetHexName());

    ExportToReqif();
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

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PaintBitmapFormat)
{
    // Given a document with a 8bpp bitmap:
    createSwDoc("paint-ole-bitmap-format.odt");

    // When exporting to reqif-xhtml with ExportImagesAsOLE enabled:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportImagesAsOLE", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure the resulting bitmap is 24bpp:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);
    OLE1Reader aOle1Reader(aOle1);
    Bitmap aBitmap;
    SvMemoryStream aMemory;
    aMemory.WriteBytes(aOle1Reader.m_aNativeData.data(), aOle1Reader.m_aNativeData.size());
    aMemory.Seek(0);
    CPPUNIT_ASSERT(ReadDIB(aBitmap, aMemory, true));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 24
    // - Actual  : 8
    // i.e. it was not a pixel format ms paint could handle in OLE mode.
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N24_BPP, aBitmap.getPixelFormat());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testMultiParaListItem)
{
    // Create a document with 3 list items: A, B&C and D.
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("A");
    SwDoc* pDoc = pWrtShell->GetDoc();
    {
        // Enable numbering.
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert("B");
    pWrtShell->SplitNode();
    pWrtShell->Insert("C");
    {
        // C is in the same list item as B.
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetCountedInList(false);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert("D");

    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
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
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertString(xText->getEnd(), "x", /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("CharUnderline", uno::Any(sal_Int16(awt::FontUnderline::NONE)));

    // Export to reqif-xhtml.
    ExportToReqif();

    // Make sure that the paragraph has no explicit style, because "text-decoration: none" is
    // filtered out.
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:div/reqif-xhtml:p", "style");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOle1PresDataNoOle2)
{
    // Save to reqif-xhtml.
    createSwDoc("no-ole2-pres-data.odt");
    ExportToReqif();
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
    createSwDoc("ole1-pres-data-wmf.odt");
    ExportToReqif();
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

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifAscharObjsize)
{
    // Given a document with an as-char anchored embedded object:
    createSwDoc("reqif-aschar-objsize.odt");

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that the RTF snippet has the correct aspect ratio:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aRtfUrl, aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 7344
    // - Actual  : 2836
    // i.e. the aspect ratio was 1:1, while the PNG aspect ratio was correctly not 1:1.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(7344), xReader->GetObjw());
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(4116), xReader->GetObjh());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifObjdataPresentationDataSize)
{
    // Given a document with an OLE2 embedded object, containing a preview:
    createSwDoc("reqif-objdata-presentationdatasize.odt");

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure that the PresentationDataSize in the RTF's objdata blob is correct:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aOle1;
    ParseOle1FromRtfUrl(aRtfUrl, aOle1);
    OLE1Reader aOle1Reader(aOle1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 565994
    // - Actual  : 330240 (Linux)
    // - Actual  : 566034 (Windows, when Word is installed)
    // because PresentationData was taken from the OLE2 stream but its size was taken from RTF.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(565994), aOle1Reader.m_nPresentationDataSize);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testListHeading)
{
    // Given a document with a list heading:
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("list header");
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
    SwTextNode& rTextNode = *rNode.GetTextNode();
    rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    rTextNode.SetCountedInList(false);

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is valid xhtml:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Without the accompanying fix in place, this test would have failed:
    // - expected: <div><ol><li style="display: block"><p>...</p></li></ol></div>
    // - actual  : <div><ol><p>...</p></li></ol></div>
    // because a </li> but no <li> is not well-formed and <ol> with a non-li children is invalid.
    OUString aContent
        = getXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/"
                                   "reqif-xhtml:li[@style='display: block']/reqif-xhtml:p");
    CPPUNIT_ASSERT_EQUAL(OUString("list header"), aContent.trim());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testPartiallyNumberedList)
{
    // Given a document with a list, first para is numbered, second is not:
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("list header");
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }
    pWrtShell->Insert2("numbered");
    pWrtShell->SplitNode();
    pWrtShell->Insert2("not numbered");
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetCountedInList(false);
    }

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is well-formed xhtml:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed:
    // - expected: <li><p>...</p><p>...</p></li>
    // - actual  : <li><p>...</p><p>...</p>
    // because a <li> without a matching </li> is not well-formed, and the </li> was omitted because
    // the second para was not numbered.
    assertXPath(pXmlDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testListHeaderAndItem)
{
    // Given a document with a list, first para is not numbered, but the second is:
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("not numbered");
    SwDoc* pDoc = pWrtShell->GetDoc();
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetCountedInList(false);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert2("numbered");
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
    }

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is well-formed xhtml:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed:
    // Entity: line 3: parser error : Opening and ending tag mismatch: ol line 3 and li
    // <reqif-xhtml:ol><reqif-xhtml:p>not numbered</reqif-xhtml:p></reqif-xhtml:li>
    CPPUNIT_ASSERT(pXmlDoc);
    // Make sure that in case the list has a header and an item, then both are wrapped in an <li>
    // element.
    assertXPath(pXmlDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testBlockQuoteNoMargin)
{
    // Given a document with some text, para style set to Quotations, no bottom margin:
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    xText->insertString(xText->getEnd(), "string", /*bAbsorb=*/false);
    uno::Reference<beans::XPropertySet> xQuotations(
        getStyles("ParagraphStyles")->getByName("Quotations"), uno::UNO_QUERY);
    xQuotations->setPropertyValue("ParaBottomMargin", uno::Any(static_cast<sal_Int32>(0)));
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    xParagraph->setPropertyValue("ParaStyleName", uno::Any(OUString("Quotations")));

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the output is valid xhtml:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed:
    // - expected: <blockquote><p>...</p></blockquote>
    // - actual  : <blockquote>...</blockquote>
    // i.e. <blockquote> is can't have character children, but it had.
    assertXPathContent(pXmlDoc,
                       "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:blockquote/reqif-xhtml:p",
                       "string");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifImageToOle)
{
    // Given a document with an image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"ole2.png")),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // When exporting to XHTML:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportImagesAsOLE", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure we export that PNG as WMF in ReqIF mode:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aRtfUrl, aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    // Without the accompanying fix in place, this test would have failed:
    // - Expected: 8
    // - Actual  : 0
    // i.e. the image was exported as PNG, not as WMF (with a version).
    CPPUNIT_ASSERT_EQUAL(8, xReader->getWmetafile());

    // Make sure that the native data byte array is not empty.
    SvMemoryStream aOle1;
    CPPUNIT_ASSERT(xReader->WriteObjectData(aOle1));
    // Without the accompanying fix in place, this test would have failed, as aOle1 was empty.
    OLE1Reader aOle1Reader(aOle1);
    CPPUNIT_ASSERT(aOle1Reader.m_nNativeDataSize);

    // Make sure that the presentation data byte array is not empty.
    // Without the accompanying fix in place, this test would have failed, as aOle1 only contained
    // the native data.
    CPPUNIT_ASSERT(aOle1Reader.m_nPresentationDataSize);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedPNGDirectly)
{
    // Given a document with an image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"ole2.png")),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the PNG is embedded directly, without an RTF wrapper:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/png
    // - Actual  : text/rtf
    // i.e. even PNG was wrapped in an RTF.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedJPGDirectly)
{
    // Given a document with an image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"reqif-ole-img.jpg")),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the JPG is embedded directly, without an RTF wrapper:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "image/jpeg");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/jpeg
    // - Actual  : image/png
    // i.e. first the original JPG data was lost, then the inner PNG fallback was missing.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object", "type",
                "image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedPNGShapeDirectly)
{
    // Given a document with an image shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("GraphicURL", uno::Any(createFileURL(u"ole2.png")));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the PNG is embedded directly, without an RTF wrapper:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - no attribute 'type' exist
    // i.e. the PNG was exported as GIF, without an explicit type.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedJPGShapeDirectly)
{
    // Given a document with an image:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("GraphicURL", uno::Any(createFileURL(u"reqif-ole-img.jpg")));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the JPG is embedded directly, without an RTF wrapper:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/jpeg
    // - Actual  : image/png
    // i.e. first the original JPG data was lost, then the inner PNG fallback was missing.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "image/jpeg");
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:object", "type",
                "image/png");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedPNGShapeAsOLE)
{
    // Given a document with an image shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue("GraphicURL", uno::Any(createFileURL(u"ole2.png")));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportImagesAsOLE", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure the PNG is embedded with an RTF wrapper:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: text/rtf
    // - Actual  : image/png
    // i.e. the OLE wrapper around the PNG was missing.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "text/rtf");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedShapeAsPNG)
{
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to XHTML:
    ExportToReqif();

    // Then make sure the shape is embedded as a PNG:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: image/png
    // - Actual  : image/x-vclgraphic
    // i.e. the result was invalid ReqIF.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "image/png");

    // Then check the pixel size of the shape:
    Size aPixelSize(Application::GetDefaultDevice()->LogicToPixel(Size(10000, 10000),
                                                                  MapMode(MapUnit::Map100thMM)));
    // Without the accompanying fix in place, this test would have failed with:
    // - no attribute 'width' exist
    // i.e. shapes had no width.
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "width",
                OUString::number(aPixelSize.getWidth()));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testShapeAsImageHtml)
{
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(5080, 2540));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to plain HTML:
    reload("HTML (StarWriter)", "");

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected:
    // - Actual  :  />
    // i.e. the output was not well-formed.
    CPPUNIT_ASSERT_EQUAL(OUString(" "), getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testJson)
{
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(2540, 2540));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When exporting to HTML, and specifying options as JSON:
    setFilterOptions("{\"XhtmlNs\":{\"type\":\"string\", \"value\":\"reqif-xhtml\"},"
                     "\"ShapeDPI\":{\"type\":\"long\",\"value\":\"192\"}}");
    save("HTML (StarWriter)");

    // Then make sure those options are not ignored:
    // Without the accompanying fix in place, this test would have failed, as GetPngPath() expects
    // XML output, but xhtmlns=reqif-xhtml was ignored.
    OUString aPngUrl = GetPngPath();
    SvFileStream aFileStream(aPngUrl, StreamMode::READ);
    GraphicDescriptor aDescriptor(aFileStream, nullptr);
    aDescriptor.Detect(/*bExtendedInfo=*/true);
    // Make sure that the increased DPI is taken into account:
    tools::Long nExpected = 192;
    CPPUNIT_ASSERT_EQUAL(nExpected, aDescriptor.GetSizePixel().getWidth());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifEmbedShapeAsPNGCustomDPI)
{
    // Given a document with a shape:
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
    xShape->setSize(awt::Size(5080, 2540));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);
    sal_Int32 nDPI = 600;

    // When exporting to XHTML:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ShapeDPI", nDPI),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure the shape is embedded as a PNG:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "type", "image/png");

    // Then check the pixel size of the shape:
    Size aPixelSize(Application::GetDefaultDevice()->LogicToPixel(Size(5080, 2540),
                                                                  MapMode(MapUnit::Map100thMM)));
    tools::Long nPNGWidth = 1200;
    OUString aPngUrl = GetPngPath();
    SvFileStream aFileStream(aPngUrl, StreamMode::READ);
    GraphicDescriptor aDescriptor(aFileStream, nullptr);
    aDescriptor.Detect(/*bExtendedInfo=*/true);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1200
    // - Actual  : 1000
    // i.e. first setting a double DPI didn't result in larger pixel width of the PNG, then it was
    // limited to 1000 pixels (because the pixel limit was 500k).
    CPPUNIT_ASSERT_EQUAL(nPNGWidth, aDescriptor.GetSizePixel().getWidth());

    // Then make sure the shape's logic size (in CSS pixels) don't change:
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object", "width",
                OUString::number(aPixelSize.getWidth()));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqifOleBmpTransparent)
{
    // Given a document with a transparent image:
    createSwDoc();
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", createFileURL(u"transparent.png")),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    // When exporting to reqif with ExportImagesAsOLE=true:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportImagesAsOLE", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure the transparent pixel turns into white:
    OUString aRtfUrl = GetOlePath();
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aRtfUrl, aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    SvMemoryStream aOle1;
    CPPUNIT_ASSERT(xReader->WriteObjectData(aOle1));
    OLE1Reader aOle1Reader(aOle1);
    SvMemoryStream aBitmapStream(aOle1Reader.m_aNativeData.data(), aOle1Reader.m_aNativeData.size(),
                                 StreamMode::READ);
    Bitmap aBitmap;
    ReadDIB(aBitmap, aBitmapStream, /*bFileHeader=*/true);
    Size aBitmapSize = aBitmap.GetSizePixel();
    BitmapEx aBitmapEx(aBitmap);
    Color nActualColor
        = aBitmapEx.GetPixelColor(aBitmapSize.getWidth() - 1, aBitmapSize.getHeight() - 1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Color: R:255 G:255 B:255 A:0
    // - Actual  : Color: R:0 G:0 B:0 A:0
    // i.e. the bitmap without an alpha channel was black, not white.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, nActualColor);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testListsHeading)
{
    // Given a document with lh, lh, li, li, lh and lh nodes:
    createSwDoc();
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("list 1, header 1");
    pWrtShell->SplitNode();
    pWrtShell->Insert("list 1, header 2");
    pWrtShell->SplitNode();
    pWrtShell->Insert("list 2, item 1");
    pWrtShell->SplitNode();
    pWrtShell->Insert("list 2, item 2");
    pWrtShell->SplitNode();
    pWrtShell->Insert("list 3, header 1");
    pWrtShell->SplitNode();
    pWrtShell->Insert("list 3, header 2");
    SwDoc* pDoc = pWrtShell->GetDoc();
    pWrtShell->Up(false, 5);
    {
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
        pWrtShell->Down(false, 1);
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
    }
    pWrtShell->Down(false, 1);
    {
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        }
        pWrtShell->Down(false, 1);
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        }
    }
    pWrtShell->Down(false, 1);
    {
        sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
        SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
        pWrtShell->Down(false, 1);
        {
            SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
            SwTextNode& rTextNode = *rNode.GetTextNode();
            rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
            rTextNode.SetCountedInList(false);
        }
    }

    // When exporting to ReqIF:
    ExportToReqif();

    // Then make sure the output is valid xhtml:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    OUString aContent
        = getXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol/"
                                   "reqif-xhtml:li[@style='display: block']/reqif-xhtml:p");
    CPPUNIT_ASSERT_EQUAL(OUString("list 1, header 1"), aContent.trim());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testOleEmfPreviewToHtml)
{
    // Given a document containing an embedded object, with EMF preview:
    createSwDoc("ole2.odt");

    // When exporting to HTML:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure the <img> tag has matching file extension and data:
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    OUString aPath = getXPath(pDoc, "/html/body/p/img", "src");
    // Without the accompanying fix in place, this test would have failed, as aPath was
    // ole_html_3978e5f373402b43.JPG, with EMF data.
    CPPUNIT_ASSERT(aPath.endsWith("gif"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testNestedBullets)
{
    // Given a documented with nested lists:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("first");
    sal_uInt16 nPos = pDoc->MakeNumRule(pDoc->GetUniqueNumRuleName());
    SwNumRule* pNumRule = pDoc->GetNumRuleTable()[nPos];
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetAttrListLevel(0);
    }
    pWrtShell->SplitNode();
    pWrtShell->Insert("second");
    {
        SwNode& rNode = pWrtShell->GetCursor()->GetPoint()->GetNode();
        SwTextNode& rTextNode = *rNode.GetTextNode();
        rTextNode.SetAttr(SwNumRuleItem(pNumRule->GetName()));
        rTextNode.SetAttrListLevel(1);
    }

    // When exporting to xhtml:
    ExportToReqif();

    // Then make sure that there is a <li> between the outer and the inner <ol>:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p' not found
    // i.e. the <li> inside the outer <ol> was missing.
    assertXPathContent(
        pXmlDoc, "//reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:ol/reqif-xhtml:li/reqif-xhtml:p",
        "second");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTrailingLineBreak)
{
    // Given a document with a trailing line-break:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("test\n");

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that we still have a single line-break:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 2
    // - XPath '//reqif-xhtml:br' number of nodes is incorrect
    assertXPath(pXmlDoc, "//reqif-xhtml:br", 1);

    // Then test the import side:

    // Given an empty document:
    mxComponent->dispose();

    // When importing a <br> from reqif-xhtml:
    ImportFromReqif(maTempFile.GetURL());

    // Then make sure that line-break is not lost:
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    pDoc = pTextDoc->GetDocShell()->GetDoc();
    pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    OUString aActual = pWrtShell->GetCursor()->GetPointNode().GetTextNode()->GetText();
    // Without the accompanying fix in place, this test would have failed, as the trailing
    // line-break was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("test\n"), aActual);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testLeadingTab)
{
    // Given a document with leading tabs:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("\t first");
    pWrtShell->SplitNode();
    pWrtShell->Insert("\t\t second");
    pWrtShell->SplitNode();
    pWrtShell->Insert("thi \t rd");

    // When exporting to HTML, using LeadingTabWidth=2:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("LeadingTabWidth", static_cast<sal_Int32>(2)),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure that leading tabs are replaced with 2 nbsps:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: <nbsp><nbsp><space>first
    // - Actual  : <tab><space>first
    // i.e. the leading tab was not replaced by 2 nbsps.
    assertXPathContent(pXmlDoc, "//reqif-xhtml:p[1]", u"\xa0\xa0 first");
    // Test a leading tab that is not at the start of the paragraph:
    assertXPathContent(pXmlDoc, "//reqif-xhtml:p[2]", u"\xa0\xa0\xa0\xa0 second");
    // Test a tab which is not leading:
    assertXPathContent(pXmlDoc, "//reqif-xhtml:p[3]", u"thi \t rd");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testLeadingTabHTML)
{
    // Given a document with leading tabs:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("\t test");

    // When exporting to plain HTML, using LeadingTabWidth=2:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("LeadingTabWidth", static_cast<sal_Int32>(2)),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure that leading tabs are replaced with 2 nbsps:
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: <newline><nbsp><nbsp><space>test
    // - Actual  : <newline><tab><space>test
    // i.e. the leading tab was not replaced by 2 nbsps.
    assertXPathContent(pHtmlDoc, "/html/body/p", SAL_NEWLINE_STRING u"\xa0\xa0 test");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testClearingBreak)
{
    auto verify = [this]() {
        uno::Reference<container::XEnumerationAccess> xParagraph(getParagraph(1), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        uno::Reference<beans::XPropertySet> xPortion;
        OUString aPortionType;
        while (true)
        {
            // Ignore leading comments.
            xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
            xPortion->getPropertyValue("TextPortionType") >>= aPortionType;
            if (aPortionType != "Annotation")
            {
                break;
            }
        }
        // Skip "foo".
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.container.NoSuchElementException
        // i.e. the first para was just comments + text portion, the clearing break was lost.
        xPortion.set(xPortions->nextElement(), uno::UNO_QUERY);
        xPortion->getPropertyValue("TextPortionType") >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(OUString("LineBreak"), aPortionType);
        uno::Reference<text::XTextContent> xLineBreak;
        xPortion->getPropertyValue("LineBreak") >>= xLineBreak;
        sal_Int16 eClear{};
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->getPropertyValue("Clear") >>= eClear;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
    };

    // Given a document with an at-para anchored image + a clearing break:
    // When loading that file:
    createSwWebDoc("clearing-break.html");
    // Then make sure that the clear property of the break is not ignored:
    verify();
    reload(mpFilter, "clearing-break.html");
    // Make sure that the clear property of the break is not ignored during export:
    verify();
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTableBackground)
{
    // Given a document with two tables: first stable has a background, second table has a
    // background in its first row:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwInsertTableOptions aInsertTableOptions(SwInsertTableFlags::DefaultBorder,
                                             /*nRowsToRepeat=*/0);
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/1, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    SvxBrushItem aBrush(Color(0xff0000), RES_BACKGROUND);
    pWrtShell->SetTabBackground(aBrush);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->SplitNode();
    pWrtShell->InsertTable(aInsertTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    aBrush.SetColor(0x00ff00);
    pWrtShell->SetRowBackground(aBrush);
    pWrtShell->Down(/*bSelect=*/false);
    // Second row has an explicit transparent background.
    aBrush.SetColor(COL_TRANSPARENT);
    pWrtShell->SetRowBackground(aBrush);

    // When exporting to reqif-xhtml:
    ExportToReqif();

    // Then make sure that CSS markup is used, not HTML one:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:table[1]' no attribute 'style' exist
    // i.e. HTML markup was used for the table background color.
    assertXPath(pXmlDoc, "//reqif-xhtml:table[1]", "style", "background: #ff0000");
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:table[1]", "bgcolor");
    assertXPath(pXmlDoc, "//reqif-xhtml:table[2]/reqif-xhtml:tr[1]", "style",
                "background: #00ff00");
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:table[2]/reqif-xhtml:tr[1]", "bgcolor");
    // Second row has no explicit style, the default is not written.
    assertXPathNoAttribute(pXmlDoc, "//reqif-xhtml:table[2]/reqif-xhtml:tr[2]", "style");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testImageKeepRatio)
{
    // Given a document with an image: width is relative, height is "keep ratio":
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xFactory->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    xTextGraphic->setPropertyValue("RelativeWidth", uno::Any(static_cast<sal_Int16>(42)));
    xTextGraphic->setPropertyValue("IsSyncHeightToWidth", uno::Any(true));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor(xBodyText->createTextCursor());
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xBodyText->insertTextContent(xCursor, xTextContent, false);

    // When exporting to HTML:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure that the width is not a fixed size, that would break on resizing the browser
    // window:
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: auto
    // - Actual  : 2
    // i.e. a static (CSS pixel) height was written.
    assertXPath(pDoc, "/html/body/p/img", "height", "auto");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testSectionDir)
{
    // Given a document with a section:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("test");
    pWrtShell->SelAll();
    SwSectionData aSectionData(SectionType::Content, "mysect");
    pWrtShell->InsertSection(aSectionData);

    // When exporting to (reqif-)xhtml:
    ExportToReqif();

    // Then make sure CSS is used to export the text direction of the section:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//reqif-xhtml:div[@id='mysect']' no attribute 'style' exist
    // i.e. the dir="ltr" HTML attribute was used instead.
    assertXPath(pXmlDoc, "//reqif-xhtml:div[@id='mysect']", "style", "dir: ltr");
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf114769)
{
    // Create document from scratch since relative urls to filesystem can be replaced
    // by absolute during save/load
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert("Hyperlink1");
    pWrtShell->SplitNode();
    pWrtShell->Insert("Hyperlink2");
    pWrtShell->SplitNode();
    pWrtShell->Insert("Hyperlink3");
    pWrtShell->SplitNode();
    pWrtShell->Insert("Hyperlink4");
    pWrtShell->SplitNode();
    pWrtShell->Insert("Hyperlink5");
    pWrtShell->SplitNode();

    // Normal external URL
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    xRun->setPropertyValue("HyperLinkURL", uno::Any(OUString("http://libreoffice.org/")));

    // Bookmark reference
    xRun.set(getRun(getParagraph(2), 1), uno::UNO_QUERY);
    xRun->setPropertyValue("HyperLinkURL", uno::Any(OUString("#some_bookmark")));

    // Filesystem absolute link
    xRun.set(getRun(getParagraph(3), 1), uno::UNO_QUERY);
    xRun->setPropertyValue("HyperLinkURL", uno::Any(OUString("C:\\test.txt")));

    // Filesystem relative link
    xRun.set(getRun(getParagraph(4), 1), uno::UNO_QUERY);
    xRun->setPropertyValue("HyperLinkURL", uno::Any(OUString("..\\..\\test.odt")));

    // Filesystem relative link
    xRun.set(getRun(getParagraph(5), 1), uno::UNO_QUERY);
    xRun->setPropertyValue("HyperLinkURL", uno::Any(OUString(".\\another.odt")));

    // Export
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProperties
        = { comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")) };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);

    CPPUNIT_ASSERT_EQUAL(OUString("http://libreoffice.org/"),
                         getXPath(pHtmlDoc, "/html/body/p[1]/a", "href"));
    CPPUNIT_ASSERT_EQUAL(OUString("#some_bookmark"),
                         getXPath(pHtmlDoc, "/html/body/p[2]/a", "href"));
    CPPUNIT_ASSERT_EQUAL(OUString("C:\\test.txt"), getXPath(pHtmlDoc, "/html/body/p[3]/a", "href"));
    CPPUNIT_ASSERT_EQUAL(OUString("..\\..\\test.odt"),
                         getXPath(pHtmlDoc, "/html/body/p[4]/a", "href"));
    CPPUNIT_ASSERT_EQUAL(OUString(".\\another.odt"),
                         getXPath(pHtmlDoc, "/html/body/p[5]/a", "href"));
}

CPPUNIT_TEST_FIXTURE(HtmlExportTest, testTdf153923)
{
    createSwDoc("TableWithIndent.fodt");
    save("HTML (StarWriter)");

    // Parse it as XML (strict!)
    xmlDocUniquePtr pDoc = parseXml(maTempFile);
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body//dl", 3);
    // The 'dd' tag was not closed
    assertXPath(pDoc, "/html/body//dd", 3);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf153923_ReqIF)
{
    createSwDoc("TableWithIndent.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//reqif-xhtml:table");
    // There should be no 'dd' or 'dl' tags, used as a hack for table indentation
    assertXPath(pDoc, "//reqif-xhtml:dl", 0);
    assertXPath(pDoc, "//reqif-xhtml:dd", 0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIfTransparentTifImg)
{
    // reqIf export must keep the TIF encoding of the image
    createSwDoc("reqif-transparent-tif-img.odt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    assertXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object[1]", "type", "image/tiff");
    OUString imageName = getXPath(pXmlDoc, "//reqif-xhtml:p/reqif-xhtml:object[1]", "data");
    // Without the accompanying fix in place, this test would have failed,
    // ending with .gif, because XOutFlags::UseGifIfSensible flag combined
    // with the transparent image would result in GIF export
    CPPUNIT_ASSERT(imageName.endsWith(".tif"));

    INetURLObject aURL(maTempFile.GetURL());
    aURL.setName(imageName);
    GraphicDescriptor aDescriptor(aURL);
    aDescriptor.Detect();
    CPPUNIT_ASSERT_EQUAL(GraphicFileFormat::TIF, aDescriptor.GetFileFormat());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf155387)
{
    createSwDoc("sub_li_and_ctd.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(pDoc);

    // Single top-level list
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul");
    // Single top-level item
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li");
    // 4 top-level paragraphs in the item
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:p", 4);
    // 2 sublists in the item
    assertXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul", 2);
    // 2 items in the first sublist
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul[1]/"
                "reqif-xhtml:li",
                2);
    // Check the last (most nested) subitem's text
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:ul[2]/"
        "reqif-xhtml:li/reqif-xhtml:ul/reqif-xhtml:li/reqif-xhtml:p",
        "l3");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf155496)
{
    createSwDoc("listItemSubheader.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(pDoc);

    // Two top-level lists
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul", 2);
    // Single top-level item
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li");
    // One top-level paragraph in the item
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:p");
    // One sublist in the item
    assertXPath(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:ul");
    // One item in the sublist
    assertXPath(pDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:ul/"
                "reqif-xhtml:li");
    // Check its text
    OUString aContent = getXPathContent(
        pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ul[1]/reqif-xhtml:li/reqif-xhtml:ul/"
              "reqif-xhtml:li/reqif-xhtml:p");
    CPPUNIT_ASSERT_EQUAL(OUString("list 1 item 1\n\t\tsub-header"), aContent.trim());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_RightAlignedTable)
{
    createSwDoc("tableRight.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // No 'align' attribute must be present in 'div'
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:div", "align");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_ListsWithNumFormat)
{
    createSwDoc("listsWithNumFormat.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // No 'type' attribute must be present in 'ol'
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[1]", "type");
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[2]", "type");
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[3]", "type");
    assertXPathNoAttribute(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:ol[4]", "type");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf155871)
{
    createSwDoc("tdf155871.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    // Without the fix in place, this would fail
    CPPUNIT_ASSERT(pDoc);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_ListsNoStartAttribute)
{
    createSwDoc("twoListsWithSameStyle.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // No 'start' attribute must be present in 'ol'
    assertXPath(pDoc, "//reqif-xhtml:ol[@start]", 0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_FrameTextAsObjectAltText)
{
    createSwDoc("frameWithText.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // Without the fix, this would fail with
    // - Expected: Some text in frame & <foo>
    // - Actual  : Frame1
    // i.e., frame name was used as the object element content, not frame text
    assertXPathContent(pDoc,
                       "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[2]/reqif-xhtml:object",
                       "Some text in frame & <foo>");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testSingleOleExport)
{
    // Given a document containing an embedded OLE object:
    createSwDoc("ole2.odt");

    // Create a selection for that object:
    uno::Reference<css::drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
    auto xDrawPage(xDrawPageSupplier->getDrawPage());
    uno::Reference<css::frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<css::view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                              uno::UNO_QUERY_THROW);
    xController->select(xDrawPage->getByIndex(0));

    // Store only the selection
    uno::Reference<css::frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    css::uno::Sequence<css::beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("RTFOLEMimeType", OUString("text/rtf")),
        comphelper::makePropertyValue("SelectionOnly", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);

    // The root element must be reqif-xhtml:object
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", "type", "text/rtf");
    // It has no children
    assertXPathChildren(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", 0);
    // And the content is empty
    assertXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", "");

    OUString aRtfData = getXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:object", "data");
    INetURLObject aUrl(maTempFile.GetURL());
    aUrl.setName(aRtfData);
    SvMemoryStream aRtf;
    HtmlExportTest::wrapRtfFragment(aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE), aRtf);
    tools::SvRef<TestReqIfRtfReader> xReader(new TestReqIfRtfReader(aRtf));
    // The RTF OLE exports correctly
    CPPUNIT_ASSERT(xReader->CallParser() != SvParserState::Error);
    CPPUNIT_ASSERT_EQUAL(tools::Long(9358), xReader->GetObjw());
    CPPUNIT_ASSERT_EQUAL(tools::Long(450), xReader->GetObjh());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_Tdf156602)
{
    createSwDoc("NestingInA1.fodt");
    ExportToReqif();

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pDoc);

    // The outer table must be kept in the document where the outer table is the first element,
    // and its A1 starts with a nested table

    // Only two sub-elements must be inside the div: an outer table and a trailing paragraph
    assertXPathChildren(pDoc, "/reqif-xhtml:html/reqif-xhtml:div", 2);
    // The outer table must have exactly two rows
    assertXPath(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr", 2);
    // First outer table cell must have two sub-elements: an inner table and a trailing paragraph
    assertXPathChildren(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]",
        2);
    // The inner table must have exactly two rows
    assertXPath(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr",
        2);
    // Check all the elements' content
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/reqif-xhtml:p",
        "Inner.A1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[2]/reqif-xhtml:p",
        "Inner.B1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[1]/reqif-xhtml:p",
        "Inner.A2");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[2]/reqif-xhtml:p",
        "Inner.B2");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[1]/"
        "reqif-xhtml:p",
        "Outer.A1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[1]/reqif-xhtml:td[2]/"
        "reqif-xhtml:p",
        "Outer.B1");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[1]/"
        "reqif-xhtml:p",
        "Outer.A2");
    assertXPathContent(
        pDoc,
        "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr[2]/reqif-xhtml:td[2]/"
        "reqif-xhtml:p",
        "Outer.B2");
    assertXPathContent(pDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p", "Following text");
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf156647_CellPaddingRoundtrip)
{
    // Given a document with a table with cell padding:
    createSwDoc("table_cell_padding.fodt");
    {
        auto xTable = getParagraphOrTable(1);
        auto aTableBorder = getProperty<css::table::TableBorder2>(xTable, "TableBorder2");
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1270), aTableBorder.Distance);
        CPPUNIT_ASSERT(aTableBorder.IsDistanceValid);
    }
    // When exporting to reqif-xhtml:
    ExportToReqif();
    // Make sure that we export it:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//reqif-xhtml:table", "cellpadding", "48"); // px
    // Now import it
    mxComponent->dispose();
    ImportFromReqif(maTempFile.GetURL());
    // Then make sure that padding is not lost:
    {
        auto xTable = getParagraphOrTable(1);
        auto aTableBorder = getProperty<css::table::TableBorder2>(xTable, "TableBorder2");
        // Without the accompanying fix in place, this test would have failed:
        // - Expected: 1270
        // - Actual  : 97
        // as the padding was lost, and the default 55 twip padding was used.
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1270), aTableBorder.Distance);
        CPPUNIT_ASSERT(aTableBorder.IsDistanceValid);
    }
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testTdf157643_WideHBorder)
{
    // Given a document with a table with a wide border between its two rows:
    createSwDoc("table_with_wide_horizontal_border.fodt");
    // When exporting to reqif-xhtml:
    ExportToReqif();
    // Make sure that there's no extra tr's:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:table/reqif-xhtml:tr", 2);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_PreserveSpaces)
{
    // Given a document with leading, trailing, and repeating intermediate spaces:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const OUString paraText = u"\t test  \t more  text \t";
    pWrtShell->Insert(paraText);

    // When exporting to plain HTML, using PreserveSpaces:
    uno::Reference<css::frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    css::uno::Sequence<css::beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("PreserveSpaces", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure that "white-space: pre-wrap" is written into the paragraph's style:
    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);
    const OUString style = getXPath(pHtmlDoc, "/html/body/p", "style");
    CPPUNIT_ASSERT(style.indexOf("white-space: pre-wrap") >= 0);
    // Also check that the paragraph text is correct, without modifications in whitespace
    assertXPathContent(pHtmlDoc, "/html/body/p", paraText);

    // Test import

    setImportFilterName("HTML (StarWriter)");
    UnoApiTest::load(maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(paraText, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_PreserveSpaces)
{
    // Given a document with leading, trailing, and repeating intermediate spaces:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const OUString paraText = u"\t test  \t more  text \t";
    pWrtShell->Insert(paraText);

    // When exporting to ReqIF, using PreserveSpaces:
    uno::Reference<css::frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    css::uno::Sequence<css::beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("PreserveSpaces", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Then make sure that xml:space="preserve" attribute exists in the paragraph element:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p", "space", u"preserve");
    // Also check that the paragraph text is correct, without modifications in whitespace
    assertXPathContent(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p", paraText);

    // Test import

    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    UnoApiTest::load(maTempFile.GetURL());
    CPPUNIT_ASSERT_EQUAL(paraText, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_NoPreserveSpaces)
{
    // Test cases where "PreserveSpaces" should not introduce respective markup

    const auto assertXPath_NoWhiteSpaceInStyle
        = [this](const xmlDocUniquePtr& pDoc, const OString& rXPath) {
              xmlXPathObjectPtr pXmlObj = getXPathNode(pDoc, rXPath);
              xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
              CPPUNIT_ASSERT_EQUAL_MESSAGE(rXPath.getStr(), 1, xmlXPathNodeSetGetLength(pXmlNodes));
              xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
              if (xmlChar* prop = xmlGetProp(pXmlNode, BAD_CAST("style")))
              {
                  OUString style = OUString::fromUtf8(reinterpret_cast<const char*>(prop));
                  CPPUNIT_ASSERT_MESSAGE(rXPath.getStr(), style.indexOf("white-space:") < 0);
              }
              xmlXPathFreeObject(pXmlObj);
          };
    const auto assertXPath_HasWhiteSpaceInStyle
        = [this](const xmlDocUniquePtr& pDoc, const OString& rXPath) {
              const OUString style = getXPath(pDoc, rXPath, "style");
              CPPUNIT_ASSERT_MESSAGE(rXPath.getStr(), style.indexOf("white-space: pre-wrap") >= 0);
          };

    createSwDoc("test_no_space_preserve.fodt");

    // Export to plain HTML, using PreserveSpaces:
    uno::Reference<css::frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    css::uno::Sequence<css::beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("PreserveSpaces", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    htmlDocUniquePtr pHtmlDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pHtmlDoc);

    // No whitespace preservation, where no leading / trailing / double whitespace
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[1]");
    // Whitespace preserved for a leading space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[2]");
    // Whitespace preserved for a trailing space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[3]");
    // Whitespace preserved for a double space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[4]");
    // No whitespace preservation for leading / trailing breaks
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[5]");
    // Whitespace preserved for a leading break + space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[6]");
    // Whitespace preserved for a trailing space + break
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[7]");
    // No whitespace preservation for a middle break
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[8]");
    // Whitespace preserved for a middle space + break
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[9]");
    // Whitespace preserved for a middle break + space
    assertXPath_HasWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[10]");
    // No whitespace preservation for a trailing space and SVG
    assertXPath_NoWhiteSpaceInStyle(pHtmlDoc, "/html/body/p[11]");

    // Test import

    setImportFilterName("HTML (StarWriter)");
    UnoApiTest::load(maTempFile.GetURL());

    CPPUNIT_ASSERT_EQUAL(OUString("No special spaces"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" Leading space"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Trailing space "), getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Double  space"), getParagraph(4)->getString());
    // Trailing break is removed in SwHTMLParser::AppendTextNode, and replaced with para spacing
    CPPUNIT_ASSERT_EQUAL(OUString("\nLeading/trailing breaks"), getParagraph(5)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("\n Leading break + space"), getParagraph(6)->getString());
    // Trailing break is removed in SwHTMLParser::AppendTextNode, and replaced with para spacing
    CPPUNIT_ASSERT_EQUAL(OUString("Trailing space + break "), getParagraph(7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Middle\nbreak"), getParagraph(8)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Middle space \n+ break"), getParagraph(9)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Middle break\n + space"), getParagraph(10)->getString());
    // The SVG is replaced by a space in SwXParagraph::getString()
    CPPUNIT_ASSERT_EQUAL(OUString("Trailing space and SVG  "), getParagraph(11)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_NoPreserveSpaces)
{
    // Test cases where "PreserveSpaces" should not introduce respective markup

    createSwDoc("test_no_space_preserve.fodt");

    // Export to ReqIF, using PreserveSpaces:
    uno::Reference<css::frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    css::uno::Sequence<css::beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("PreserveSpaces", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);

    // No whitespace preservation, where no leading / trailing / double whitespace
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[1]", "space");
    // Whitespace preserved for a leading space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[2]", "space",
                u"preserve");
    // Whitespace preserved for a trailing space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[3]", "space",
                u"preserve");
    // Whitespace preserved for a double space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[4]", "space",
                u"preserve");
    // No whitespace preservation for leading / trailing breaks
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[5]", "space");
    // Whitespace preserved for a leading break + space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[6]", "space",
                u"preserve");
    // No whitespace preservation for a trailing space + break
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[7]", "space");
    // No whitespace preservation for a middle break
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[8]", "space");
    // No whitespace preservation for a middle space + break
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[9]", "space");
    // Whitespace preserved for a middle break + space
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[10]", "space",
                u"preserve");
    // No whitespace preservation for a trailing space and SVG
    assertXPathNoAttribute(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[11]", "space");

    // Test import

    setImportFilterOptions("xhtmlns=reqif-xhtml");
    setImportFilterName("HTML (StarWriter)");
    UnoApiTest::load(maTempFile.GetURL());

    CPPUNIT_ASSERT_EQUAL(OUString("No special spaces"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(" Leading space"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Trailing space "), getParagraph(3)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Double  space"), getParagraph(4)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("\nLeading/trailing breaks\n"), getParagraph(5)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("\n Leading break + space"), getParagraph(6)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Trailing space + break \n"), getParagraph(7)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Middle\nbreak"), getParagraph(8)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Middle space \n+ break"), getParagraph(9)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Middle break\n + space"), getParagraph(10)->getString());
    // The SVG is replaced by a space in SwXParagraph::getString()
    CPPUNIT_ASSERT_EQUAL(OUString("Trailing space and SVG  "), getParagraph(11)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_ExportFormulasAsPDF)
{
    // Given a document with a formula:
    createSwDoc("embedded_formula.fodt");

    // When exporting to reqif with ExportFormulasAsPDF=true:
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue("FilterName", OUString("HTML (StarWriter)")),
        comphelper::makePropertyValue("FilterOptions", OUString("xhtmlns=reqif-xhtml")),
        comphelper::makePropertyValue("ExportFormulasAsPDF", true),
    };
    xStorable->storeToURL(maTempFile.GetURL(), aStoreProperties);

    // Make sure that the formula is exported as PDF:
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p[2]/reqif-xhtml:object",
                "type", "application/pdf");

    css::uno::Sequence<css::beans::PropertyValue> descr{
        comphelper::makePropertyValue("URL", GetObjectPath(".pdf")),
    };

    uno::Reference<lang::XMultiServiceFactory> xFactory(
        comphelper::getProcessComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XTypeDetection> xTypeDetection(
        xFactory->createInstance("com.sun.star.document.TypeDetection"), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(OUString("pdf_Portable_Document_Format"),
                         xTypeDetection->queryTypeByDescriptor(descr, true));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_NoBrClearForImageWrap)
{
    // Given a document with a paragraph-anchored image with "none" wrap:
    createSwDoc("image_anchored_to_paragraph_no_wrap.fodt");
    // When exporting to reqif:
    ExportToReqif();
    // Make sure that there's no 'br' elements in the 'object' (used to represent the wrapping
    // in HTML export, using 'clear' attribute):
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object");
    assertXPath(pXmlDoc,
                "/reqif-xhtml:html/reqif-xhtml:div/reqif-xhtml:p/reqif-xhtml:object/reqif-xhtml:br",
                0);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testReqIF_Tdf160017_spanClosingOrder)
{
    // Given a document with a paragraph having explicit font color and character border properties:
    createSwDoc("char_border_and_font_color.fodt");
    // When exporting to reqif:
    ExportToReqif();
    SvMemoryStream aStream;
    WrapReqifFromTempFile(aStream);
    xmlDocUniquePtr pXmlDoc = parseXmlStream(&aStream);
    // Without the fix, this would fail, because there was an extra closing </reqif-xhtml:span>
    CPPUNIT_ASSERT(pXmlDoc);
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_Tdf160017_spanClosingOrder)
{
    // Given a document with a paragraph having explicit font color and character border properties:
    createSwDoc("char_border_and_font_color.fodt");
    // When exporting to HTML:
    save("HTML (StarWriter)");
    // Parse it as XML (strict!)
    // Without the fix, this would fail, because span and font elements closed in wrong order
    CPPUNIT_ASSERT(parseXml(maTempFile));
}

CPPUNIT_TEST_FIXTURE(SwHtmlDomExportTest, testHTML_Tdf160390)
{
    // This document must not hang infinitely on HTML export
    createSwDoc("tdf160390.fodt");
    save("HTML (StarWriter)");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
