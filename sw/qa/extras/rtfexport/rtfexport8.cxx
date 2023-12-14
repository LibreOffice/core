/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
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

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testTdf158586_0, "tdf158586_pageBreak0.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    // const auto& pLayout = parseLayoutDump();
    // assertXPathContent(pLayout, "//page[1]/body/txt"_ostr, "First page");}
}

DECLARE_RTFEXPORT_TEST(testTdf158586_0B, "tdf158586_pageBreak0B.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf158586_1, "tdf158586_pageBreak1.rtf")
{
    // None of the specified text frame settings initiates a real text frame - page break not lost
    // CPPUNIT_ASSERT_EQUAL(2, getPages());
    // CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    // const auto& pLayout = parseLayoutDump();
    // assertXPathContent(pLayout, "//page[2]/body/txt"_ostr, "Second page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_lostFrame, "tdf158586_lostFrame.rtf")
{
    // The anchor and align properties are sufficient to define a frame
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored"_ostr, 1);
    assertXPathContent(pLayout, "//page[1]/body//txt"_ostr, "1st page");
    // assertXPathContent(pLayout, "//page[2]/body//txt"_ostr, "2nd page");

    // CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
