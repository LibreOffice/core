/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <test/htmltesttools.hxx>

#include <comphelper/propertyvalue.hxx>
#include <filter/msfilter/rtfutil.hxx>
#include <o3tl/numeric.hxx>
#include <svtools/parrtf.hxx>
#include <svtools/rtftoken.h>
#include <tools/urlobj.hxx>

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
            sal_Int8 parsed = o3tl::convertToHex<sal_Int8>(ch);
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

/// Covers sw/source/filter/html/wrthtml.cxx and related fixes.
class HtmlExportTest : public SwModelTestBase, public HtmlTestTools
{
public:
    HtmlExportTest()
        : SwModelTestBase(u"/sw/qa/extras/htmlexport/data/"_ustr, u"HTML (StarWriter)"_ustr)
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
};

/// HTML export of the sw doc model tests.
class SwHtmlDomExportTest : public SwModelTestBase, public HtmlTestTools
{
public:
    SwHtmlDomExportTest()
        : SwModelTestBase(u"/sw/qa/extras/htmlexport/data/"_ustr)
    {
    }

    OUString GetObjectPath(const OUString& ext);
    /// Get the .ole path, assuming maTempFile is an XHTML export result.
    OUString GetOlePath() { return GetObjectPath(u".ole"_ustr); }
    OUString GetPngPath() { return GetObjectPath(u".png"_ustr); }
    /// Parse the ole1 data out of an RTF fragment URL.
    void ParseOle1FromRtfUrl(const OUString& rRtfUrl, SvMemoryStream& rOle1);
    /// Export using the C++ HTML export filter, with xhtmlns=reqif-xhtml.
    void ExportToReqif();
    /// Import using the C++ HTML import filter, with xhtmlns=reqif-xhtml.
    void ImportFromReqif(const OUString& rUrl);
    /// Export using the C++ HTML export filter
    void ExportToHTML();
};

OUString SwHtmlDomExportTest::GetObjectPath(const OUString& ext)
{
    assert(ext.startsWith("."));
    xmlDocUniquePtr pDoc = WrapReqifFromTempFile();
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
    setFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    save(u"HTML (StarWriter)"_ustr);
}

void SwHtmlDomExportTest::ExportToHTML()
{
    uno::Sequence<beans::PropertyValue> aStoreProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
    };
    saveWithParams(aStoreProperties);
}

void SwHtmlDomExportTest::ImportFromReqif(const OUString& rUrl)
{
    uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
    };
    loadWithParams(rUrl, aLoadProperties);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
