/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/gdimtf.hxx>

#include <wrtsh.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/layout/data/";

/// Covers sw/source/core/layout/ fixes.
class SwCoreLayoutTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testTdf128195)
{
    // Load a document that has two paragraphs in the header.
    // The second paragraph should have its bottom spacing applied.
    load(DATA_DIRECTORY, "tdf128195.docx");
    sal_Int32 nTxtHeight = parseDump("//header/txt[2]/infos/bounds", "height").toInt32();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2269), nTxtHeight);
}

CPPUNIT_TEST_FIXTURE(SwCoreLayoutTest, testAnchorPositionBasedOnParagraph)
{
    // tdf#134783 check weather position of shape is good if it is achored to paragraph and
    // the "Don't add space between paragraphs of the same style" option is set
    load(DATA_DIRECTORY, "tdf134783_testAnchorPositionBasedOnParagraph.fodt");
    xmlDocPtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[1]/bounds", "top", "1671");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[1]/bounds", "bottom", "1732");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[2]/bounds", "top", "1947");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[2]/bounds", "bottom", "2008");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[3]/bounds", "top", "3783");
    assertXPath(pXmlDoc, "(//SwAnchoredDrawObject)[3]/bounds", "bottom", "3844");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
