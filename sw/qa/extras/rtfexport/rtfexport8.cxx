/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/TabStop.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentSettingAccess.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testTdf155663, "piccrop.rtf")
{
    auto const xShape(getShape(1));
    if (!isExported())
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2004), xShape->getSize().Height);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2004), xShape->getSize().Width);
    }
    else // bit of rounding loss?
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2013), xShape->getSize().Height);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2013), xShape->getSize().Width);
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                         getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Bottom);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                         getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                         getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Right);
}

DECLARE_RTFEXPORT_TEST(testTdf158586_0, "tdf158586_pageBreak0.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // There should be no empty paragraph at the start
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored", 1);
    assertXPathContent(pLayout, "/root/page[1]/body//txt", "First page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_0B, "tdf158586_pageBreak0B.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // There should be no empty paragraph at the start
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored", 1);
    assertXPathContent(pLayout, "/root/page[1]/body//txt", "First page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_1, "tdf158586_pageBreak1.rtf")
{
    // None of the specified text frame settings initiates a real text frame - page break not lost
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    const auto& pLayout = parseLayoutDump();
    // on import there is a section on page 2; on reimport there is no section
    // (probably not an important difference?)
    assertXPathContent(pLayout, "/root/page[2]/body//txt", "Second page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_1header, "tdf158586_pageBreak1_header.rtf")
{
    // None of the specified text frame settings initiates a real text frame - page break not lost
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    const auto& pLayout = parseLayoutDump();
    // on import there is a section on page 2; on reimport there is no section
    // (probably not an important difference?)
    assertXPathContent(pLayout, "/root/page[2]/body//txt", "Second page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_lostFrame, "tdf158586_lostFrame.rtf")
{
    // The anchor and align properties are sufficient to define a frame
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored", 1);
    assertXPathContent(pLayout, "//page[1]/body//txt", "1st page");
    assertXPathContent(pLayout, "//page[2]/body//txt", "2nd page");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf158983, "fdo55504-1-min.rtf")
{
    // the problem was that the page break was missing and the shapes were
    // all anchored to the same node

    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/body/section/txt", 1);
    assertXPath(pLayout, "/root/page[1]/body/section/txt/anchored/fly", 1);
    // Word shows these shapes anchored in the fly, not body, but at least they are not lost
    assertXPath(pLayout, "/root/page[1]/body/section/txt/anchored/SwAnchoredDrawObject", 2);
    // page break, paragraph break, section break.
    assertXPath(pLayout, "/root/page[2]/body/section[1]/txt", 1);
    assertXPath(pLayout, "/root/page[2]/body/section[1]/txt/anchored", 0);
    assertXPath(pLayout, "/root/page[2]/body/section[2]/txt", 1);
    assertXPath(pLayout, "/root/page[2]/body/section[2]/txt/anchored/fly", 1);
    // Word shows these shapes anchored in the fly, not body, but at least they are not lost
    assertXPath(pLayout, "/root/page[2]/body/section[2]/txt/anchored/SwAnchoredDrawObject", 2);

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testAnnotationPar, "tdf136445-1-min.rtf")
{
    // the problem was that the paragraph break following annotation was missing
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(
        OUString("Annotation"),
        getProperty<OUString>(
            getRun(getParagraph(1, "I ax xoixx xuxixx xxe xixxx. (Xaxxexx 1989 x.x. xaxax a)"), 2),
            "TextPortionType"));
    CPPUNIT_ASSERT(
        !getProperty<OUString>(getParagraph(2, "Xix\txaxa\tx-a\t\t\txix\tx xi = xa."), "ListId")
             .isEmpty());
}

DECLARE_RTFEXPORT_TEST(testTdf158826_extraCR, "tdf158826_extraCR.rtf")
{
    // Note: this is a hand-minimized sample, and very likely doesn't follow RTF { } rules...

    // The page break defined before the document content should not cause a page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // There is a two-column floating table
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
