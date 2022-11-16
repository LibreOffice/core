/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>

#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <unotxdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf148360)
{
    loadAndReload("tdf148360.doc");
    const auto& pLayout = parseLayoutDump();

    // Ensure first element is a tab
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[1]", "type", "PortionType::TabLeft");
    // and only then goes content
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/SwParaPortion/SwLineLayout/child::*[2]", "type", "PortionType::Text");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77964)
{
    loadAndReload("tdf77964.doc");
    // both images were loading as AT_PARA instead of AS_CHAR. Image2 visually had text wrapping.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShapeByName(u"Image2"), "AnchorType"));
}

DECLARE_WW8EXPORT_TEST(testTdf150197_anlv2ListFormat, "tdf150197_anlv2ListFormat.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix me? I should be 2.1", OUString("4.1"),
                                 getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_WW8EXPORT_TEST(testTdf117994_CRnumformatting, "tdf117994_CRnumformatting.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "expand"));
    //Without this fix in place, it would become 200 (and non-bold).
    CPPUNIT_ASSERT_EQUAL(OUString("160"), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "font-height"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
