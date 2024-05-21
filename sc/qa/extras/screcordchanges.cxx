/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/ustring.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of calc Record Changes test */

class ScRecordChangesTest : public UnoApiTest
{
public:
    ScRecordChangesTest();

    void testSetRecordChanges();
    void testCheckRecordChangesProtection();

    CPPUNIT_TEST_SUITE(ScRecordChangesTest);
    CPPUNIT_TEST(testSetRecordChanges);
    CPPUNIT_TEST(testCheckRecordChangesProtection);
    CPPUNIT_TEST_SUITE_END();
};

void ScRecordChangesTest::testSetRecordChanges()
{
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xDocSettingsPropSet(xDoc, UNO_QUERY_THROW);

    bool recordChangesValue = true;
    bool protectionValue = true;

    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"RecordChanges"_ustr)
                   >>= recordChangesValue);
    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"IsRecordChangesProtected"_ustr)
                   >>= protectionValue);

    CPPUNIT_ASSERT_MESSAGE("a new document does not record changes", !recordChangesValue);
    CPPUNIT_ASSERT_MESSAGE("a new document does not protect record changes", !protectionValue);

    // now activate recording
    uno::Any aValue;
    aValue <<= true;
    xDocSettingsPropSet->setPropertyValue(u"RecordChanges"_ustr, aValue);

    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"RecordChanges"_ustr)
                   >>= recordChangesValue);
    CPPUNIT_ASSERT_MESSAGE("the document should record changes", recordChangesValue);
}

void ScRecordChangesTest::testCheckRecordChangesProtection()
{
    // test with protected changes
    loadFromFile(u"RecordChangesProtected.ods");

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xDocSettingsPropSet(xDoc, UNO_QUERY_THROW);

    bool recordChangesValue = false;
    bool protectionValue = false;

    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"RecordChanges"_ustr)
                   >>= recordChangesValue);
    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"IsRecordChangesProtected"_ustr)
                   >>= protectionValue);

    CPPUNIT_ASSERT_MESSAGE("the document should be recording changes", recordChangesValue);
    CPPUNIT_ASSERT_MESSAGE("the protection should be active", protectionValue);

    // try to de-activate recording
    uno::Any aValue;
    aValue <<= false;
    xDocSettingsPropSet->setPropertyValue(u"RecordChanges"_ustr, aValue);

    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"RecordChanges"_ustr)
                   >>= recordChangesValue);
    CPPUNIT_ASSERT(xDocSettingsPropSet->getPropertyValue(u"IsRecordChangesProtected"_ustr)
                   >>= protectionValue);

    // this document should still record changes as protection is set
    CPPUNIT_ASSERT_MESSAGE("the document should still be recording changes", recordChangesValue);
    CPPUNIT_ASSERT_MESSAGE("the protection should still be active", protectionValue);
}

ScRecordChangesTest::ScRecordChangesTest()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScRecordChangesTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
