/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <test/xmltesttools.hxx>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class TestXPath : public CppUnit::TestFixture, public XmlTestTools
{
};

CPPUNIT_TEST_FIXTURE(TestXPath, test_getXPath)
{
    const xmlChar s_xml[] = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
                            "<xml><item attrib='val'>text</item></xml>";
    xmlDocUniquePtr pTable(xmlParseDoc(s_xml));
    CPPUNIT_ASSERT(pTable);
    // Must get existing element content without errors
    CPPUNIT_ASSERT_ASSERTION_PASS(assertXPath(pTable, "/xml/item"_ostr));
    // Must error out when getting non-existing element
    CPPUNIT_ASSERT_ASSERTION_FAIL(assertXPath(pTable, "/xml/no_item"_ostr));
    // Must get existing attribute value correctly
    CPPUNIT_ASSERT_ASSERTION_PASS(getXPath(pTable, "/xml/item"_ostr, "attrib"_ostr));
    // Must fail when requested non-empty attribute doesn't exist
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPath(pTable, "/xml/item"_ostr, "no_attrib"_ostr));
    // Must properly return attribute content
    CPPUNIT_ASSERT_EQUAL(u"val"_ustr, getXPath(pTable, "/xml/item"_ostr, "attrib"_ostr));
    // Trying to get position of missing child of a node must fail assertion
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pTable, "/xml/item"_ostr, "absent"));
    // Asserting that an attribute is absent
    CPPUNIT_ASSERT_ASSERTION_FAIL(assertXPathNoAttribute(pTable, "/xml/item"_ostr, "attrib"_ostr));
    CPPUNIT_ASSERT_ASSERTION_PASS(assertXPathNoAttribute(pTable, "/xml/item"_ostr, "foo"_ostr));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
