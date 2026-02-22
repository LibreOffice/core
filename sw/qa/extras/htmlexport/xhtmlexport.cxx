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
#include <comphelper/propertyvalue.hxx>

namespace
{
class XHtmlExportTest : public SwModelTestBase, public HtmlTestTools
{
public:
    XHtmlExportTest()
        : SwModelTestBase(u"/sw/qa/extras/odfexport/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testImageEmbedding)
{
    createSwDoc("image-mimetype.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });

    htmlDocUniquePtr pDoc = parseHtml(maTempFile);
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "/html/body", 1);
    assertXPath(pDoc, "/html/body/div[1]/div[1]/img", 1);
    OUString aValue = getXPath(pDoc, "/html/body/div[1]/div[1]/img", "src");
    CPPUNIT_ASSERT(aValue.startsWith("data:image/svg+xml;base64"));
}

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf131812)
{
    createSwDoc("tdf131812.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf(
                       ".paragraph-P1{ font-size:12pt; font-family:\'Liberation Serif\'; "
                       "writing-mode:horizontal-tb; direction:rtl; text-align:right ! important; }")
                   != -1);
}

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf146264)
{
    createSwDoc("tdf146264.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
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

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf167910)
{
    createSwDoc("tdf167910.fodt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    auto para1{ aStream.indexOf("text-decoration:underline;") };
    CPPUNIT_ASSERT(para1 != -1);
    auto para2{ aStream.indexOf("text-decoration:overline;", para1) };
    CPPUNIT_ASSERT(para2 != -1);
    auto para3{ aStream.indexOf("text-decoration:underline overline;", para2) };
    CPPUNIT_ASSERT(para3 != -1);
    // the paragraph 4 is using common style not automatic style and these
    // are exported after all automatic ones...
    auto para5{ aStream.indexOf("text-decoration:underline;", para3) };
    CPPUNIT_ASSERT(para5 != -1);
    auto para6{ aStream.indexOf("text-decoration: overline;", para5) };
    CPPUNIT_ASSERT(para6 != -1);
    // this one is style overridden by autostyle
    auto para7{ aStream.indexOf("text-decoration:none ! important;", para6) };
    CPPUNIT_ASSERT(para7 != -1);
    auto para4style{ aStream.indexOf(".paragraph-underover", para7) };
    CPPUNIT_ASSERT(para4style != -1);
    auto para4{ aStream.indexOf("text-decoration:underline overline;", para4style) };
    CPPUNIT_ASSERT(para4 != -1);
}

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf118637)
{
    createSwDoc("tdf118637.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(aStream.indexOf("The formula  ") != -1);
    CPPUNIT_ASSERT(aStream.indexOf("should be inline.</div>") != -1);
}

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf145361)
{
    createSwDoc("tdf145361.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
    // Without the fix in place, this test would have failed with
    // - SfxBaseModel::impl_store <file:///tmp/lu66091ameq.tmp> failed: 0xc10(Error Area:Io Class:Write Code:16)
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(
        aStream.indexOf("List entry has<br/><span style=\"margin-left:0cm\"/>a line break") != -1);
}

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf142483)
{
    createSwDoc("tdf142483.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
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

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf107696)
{
    createSwDoc("tdf107696.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
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

CPPUNIT_TEST_FIXTURE(XHtmlExportTest, testTdf66305)
{
    createSwDoc("tdf66305.odt");
    save(TestFilter::XHTML_WRITER,
         {
             comphelper::makePropertyValue(u"FilterOptions"_ustr, u"UTF8"_ustr),
         });
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    sal_uInt64 nLength = pStream->TellEnd();
    OString aStream(read_uInt8s_ToOString(*pStream, nLength));
    CPPUNIT_ASSERT(
        aStream.indexOf("<p class=\"paragraph-P5\"><a href=\"#__RefHeading__82004_486970805\" "
                        "class=\"text-Internet_20_link\">Introduction</a></p><p "
                        "class=\"paragraph-P6\"> </p>")
        != -1);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
