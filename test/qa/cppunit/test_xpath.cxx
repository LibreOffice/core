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
#include <unotest/bootstrapfixturebase.hxx>

class TestXPath : public CppUnit::TestFixture, public XmlTestTools
{
};

CPPUNIT_TEST_FIXTURE(TestXPath, test_getXPath)
{
    const xmlChar s_xml[] = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
                            "<xml><item attrib='val'>text</item></xml>";
    xmlDocPtr pTable = xmlParseDoc(s_xml);
    CPPUNIT_ASSERT(pTable);
    // Must get existing element content without errors
    CPPUNIT_ASSERT_ASSERTION_PASS(getXPath(pTable, "/xml/item", ""));
    // Must error out when getting non-existing element
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPath(pTable, "/xml/no_item", ""));
    // Must get existing attribute value correctly
    CPPUNIT_ASSERT_ASSERTION_PASS(getXPath(pTable, "/xml/item", "attrib"));
    // Must fail when requested non-empty attribute doesn't exist
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPath(pTable, "/xml/item", "no_attrib"));
    // Must return empty string if not asking an attribute, regardless what is its content
    CPPUNIT_ASSERT_EQUAL(OUString(), getXPath(pTable, "/xml/item", ""));
    // Must properly return attribute content
    CPPUNIT_ASSERT_EQUAL(OUString("val"), getXPath(pTable, "/xml/item", "attrib"));
    // Trying to get position of missing child of a node must fail assertion
    CPPUNIT_ASSERT_ASSERTION_FAIL(getXPathPosition(pTable, "/xml/item", "absent"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
