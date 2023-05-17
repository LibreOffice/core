/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <swmodeltestbase.hxx>

#include <test/htmltesttools.hxx>

class XHtmlExportTest : public SwModelTestBase, public HtmlTestTools
{
public:
    XHtmlExportTest()
        : SwModelTestBase("/sw/qa/extras/odfexport/data/", "XHTML Writer File")
    {
    }

private:
    virtual std::unique_ptr<Resetter> preTest(const char*) override
    {
        setFilterOptions("UTF8");
        return nullptr;
    }
};

#define DECLARE_HTMLEXPORT_TEST(TestName, filename)                                                \
    DECLARE_SW_EXPORT_TEST(TestName, filename, nullptr, XHtmlExportTest)

DECLARE_HTMLEXPORT_TEST(testImageEmbedding, "image-mimetype.odt")
{
    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/div[1]/div[1]/img", 1);
    OUString aValue = getXPath(pDoc, "/html/body/div[1]/div[1]/img", "src");
    CPPUNIT_ASSERT(aValue.startsWith("data:image/svg+xml;base64"));
}

DECLARE_HTMLEXPORT_TEST(testTdf131812, "tdf131812.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(
        aStream.indexOf(".paragraph-P1{ font-size:12pt; font-family:\'Liberation Serif\'; "
                        "writing-mode:horizontal-tb; direction:rtl; text-align:right ! important;}")
        != -1);
}

DECLARE_HTMLEXPORT_TEST(testTdf146264, "tdf146264.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    sal_Int32 nFirstHello = aStream.indexOf("Hello");
    CPPUNIT_ASSERT(nFirstHello > 0);
    if (nFirstHello > 0)
    {
        sal_Int32 nSecondHello = aStream.indexOf("Hello", nFirstHello + 1);
        constexpr sal_Int32 nMinusOne = -1;
        CPPUNIT_ASSERT_EQUAL(nMinusOne, nSecondHello);
    }
}

DECLARE_HTMLEXPORT_TEST(testTdf118637, "tdf118637.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("The formula  ") != -1);
    CPPUNIT_ASSERT(aStream.indexOf("should be inline.</div>") != -1);
}

DECLARE_HTMLEXPORT_TEST(testTdf145361, "tdf145361.odt")
{
    // Without the fix in place, this test would have failed with
    // - SfxBaseModel::impl_store <file:///tmp/lu66091ameq.tmp> failed: 0xc10(Error Area:Io Class:Write Code:16)
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(
        aStream.indexOf("List entry has<br/><span style=\"margin-left:0cm\"/>a line break") != -1);
}

DECLARE_HTMLEXPORT_TEST(testTdf142483, "tdf142483.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(
        aStream.indexOf("padding:0.0382in; border-left-width:thin; border-left-style:solid; "
                        "border-left-color:#000000; border-right-style:none; "
                        "border-top-width:thin; border-top-style:solid; border-top-color:#000000; "
                        "border-bottom-width:thin; border-bottom-style:solid; "
                        "border-bottom-color:#000000;")
        != -1);
    CPPUNIT_ASSERT(
        aStream.indexOf("padding:0.0382in; border-left-width:thin; border-left-style:solid; "
                        "border-left-color:#000000; border-right-style:none; "
                        "border-top-style:none; border-bottom-width:thin; "
                        "border-bottom-style:solid; border-bottom-color:#000000;")
        != -1);
    CPPUNIT_ASSERT(
        aStream.indexOf("border-top-width:thin; border-top-style:solid; border-top-color:#000000; "
                        "border-left-width:thin; border-left-style:solid; "
                        "border-left-color:#000000; border-bottom-width:thin; "
                        "border-bottom-style:solid; border-bottom-color:#000000; "
                        "border-right-width:thin; border-right-style:solid; "
                        "border-right-color:#000000; padding:0.0382in;")
        != -1);
    CPPUNIT_ASSERT(
        aStream.indexOf("padding:0.0382in; border-left-width:thin; border-left-style:solid; "
                        "border-left-color:#000000; border-right-width:thin; "
                        "border-right-style:solid; border-right-color:#000000; "
                        "border-top-style:none; border-bottom-width:thin; "
                        "border-bottom-style:solid; border-bottom-color:#000000;")
        != -1);
}

DECLARE_HTMLEXPORT_TEST(testTdf107696, "tdf107696.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    //tdf#111492
    CPPUNIT_ASSERT(aStream.indexOf("span.heading_numbering { margin-right: 0.8rem; }* { margin:0;}")
                   != -1);
    CPPUNIT_ASSERT(
        aStream.indexOf("<h2 class=\"paragraph-Heading_20_2\"><a "
                        "id=\"a__Level_2_without_number\"><span/></a>Level 2 without number</h2>")
        != -1);
    CPPUNIT_ASSERT(
        aStream.indexOf(
            "<h2 class=\"paragraph-Heading_20_2\"><a id=\"a_3_1_Level_2__number_3_1\"><span "
            "class=\"heading_numbering\">3.1</span></a>Level <span "
            "class=\"text-T2\">2</span>, <span class=\"text-T1\">number 3.1</span></h2>")
        != -1);
    CPPUNIT_ASSERT(
        aStream.indexOf(
            "<h2 class=\"paragraph-Heading_20_2\"><a id=\"a_3_2_Level_2__number_3_2\"><span "
            "class=\"heading_numbering\">3.2</span></a>Level 2, <span "
            "class=\"text-T1\">number 3.2</span></h2>")
        != -1);
}

DECLARE_HTMLEXPORT_TEST(testTdf66305, "tdf66305.odt")
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(
        aStream.indexOf("<p class=\"paragraph-P6\"><a href=\"#__RefHeading__82004_486970805\" "
                        "class=\"text-Internet_20_link\">Introduction</a></p><p "
                        "class=\"paragraph-P7\"> </p>")
        != -1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
