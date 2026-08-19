/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include <comphelper/propertyvalue.hxx>
#include <cppuhelper/implbase.hxx>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>

using namespace css;
using namespace css::uno;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }
};

class ProgressRecorder : public cppu::WeakImplHelper<css::task::XStatusIndicator>
{
public:
    sal_Int32 mnStartCount = 0;
    sal_Int32 mnEndCount = 0;
    sal_Int32 mnLastValue = -1;
    OUString maText;

    void SAL_CALL start(const OUString& rText, sal_Int32 /*nRange*/) override
    {
        ++mnStartCount;
        maText = rText;
    }
    void SAL_CALL end() override { ++mnEndCount; }
    void SAL_CALL setText(const OUString& rText) override { maText = rText; }
    void SAL_CALL setValue(sal_Int32 nValue) override { mnLastValue = nValue; }
    void SAL_CALL reset() override {}
};

DECLARE_OOXMLEXPORT_TEST(testTdf38575_fullWidthLine, "tdf38575_fullWidthLine.docx")
{
    // given a document where the header contains a graphic that spans the entire left side,
    // reducing the available body text area for every page in the document,
    // and an AS_CHAR horizontal line that wants the entire width (and thus could never fit)

    // for compatibilityMode 15, in MS Word 2024, this is 4 pages long (not infinitely long)
    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124398_groupshapeChart)
{
    // given a document with grouped chart and textbox
    // TODO: seeing the chart is broken since 24.8.4

    createSwDoc("tdf124398_groupshapeChart.docx");

    // MS Word considered this document to be corrupt for several reasons.
    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // DOCX/wps: doesn't use nvGraphicFramePr - only PPTX/p: and XLSX/xdr: do.
    assertXPath(pXmlDoc, "//wpg:graphicFrame/wpg:cNvPr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171527_flyInFramePr)
{
    // given a with a framePr'd image anchoring a drawing shape
    createSwDoc("tdf171527_flyInFramePr.doc");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, MS Word complained the file was corrupt.
    // Round-trip the framePr'd image as a framePr instead of as a drawingML
    assertXPath(pXmlDoc, "//w:body/w:p/w:pPr/w:framePr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171433_equation)
{
    // given a document with formula
    createSwDoc("tdf171433_equation.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, the formula was completely broken - missing SubSup entirely
    assertXPath(pXmlDoc, "//m:sSubSup/m:sup", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtPictureDataBinding)
{
    // Given a DOCX with a picture content control (w:picture + w:dataBinding)
    // where the data binding resolves to base64 image data from custom XML.
    // The sdtContent has a w:drawing with the actual rendered image.
    // Without the fix, the drawing was replaced by the raw base64 text.
    createSwDoc("sdt-picture-databinding.docx");

    // The first paragraph should contain a graphic, not text
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testCool15788_symbolContentControl)
{
    createSwDoc("Cool15788_symbolContentControl.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // this must not exist, otherwise Word complains about the file
    // <w:sym w:font="Wingdings" w:char="f04b"/>
    CPPUNIT_ASSERT_EQUAL(0, countXPathNodes(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:sym"));
    // simply a <w:t> element with the symbol must exist instead
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t", u"\xf04b");
}

CPPUNIT_TEST_FIXTURE(Test, testStarBatsBulletKeepsAVisibleGlyph)
{
    createSwDoc("starbats-bullet.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    // The bullet comes out in a font that is widely available, at the character that font holds
    // the bullet at. A reader without the old StarBats font still gets a bullet rather than an
    // empty rectangle.
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:rPr/w:rFonts", "ascii",
                u"Symbol");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:lvlText", "val", u"\xF0B7");
}

CPPUNIT_TEST_FIXTURE(Test, testDocxSaveReportsProgress)
{
    createSwDoc();

    rtl::Reference<ProgressRecorder> xRecorder(new ProgressRecorder);
    cpo::uno::Sequence<beans::PropertyValue> aStoreArguments{
        comphelper::makePropertyValue(u"FilterName"_ustr, u"MS Word 2007 XML"_ustr),
        comphelper::makePropertyValue(
            u"StatusIndicator"_ustr, uno::Reference<task::XStatusIndicator>(xRecorder))
    };

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY_THROW);
    xStorable->storeToURL(maTempFile.GetURL(), aStoreArguments);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xRecorder->mnStartCount);
    CPPUNIT_ASSERT(xRecorder->mnLastValue >= 0);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xRecorder->mnEndCount);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
