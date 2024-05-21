/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/sheet/spreadsheetdocumentsettings.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScSpreadsheetSettingsObj : public UnoApiTest,
                                 public apitest::SpreadsheetDocumentSettings,
                                 public apitest::XPropertySet
{
public:
    ScSpreadsheetSettingsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScSpreadsheetSettingsObj);

    // SpreadsheetDocumentSettings
    CPPUNIT_TEST(testSpreadsheetDocumentSettingsProperties);

    // XPropertySet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    CPPUNIT_TEST_SUITE_END();
};

ScSpreadsheetSettingsObj::ScSpreadsheetSettingsObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XPropertySet({ u"AreaLinks"_ustr, u"CharLocale"_ustr, u"CharLocaleAsian"_ustr,
                     u"CharLocaleComplex"_ustr, u"ColumnLabelRanges"_ustr, u"DDELinks"_ustr,
                     u"DatabaseRanges"_ustr, u"ExternalDocLinks"_ustr, u"InteropGrabBag"_ustr,
                     u"NamedRanges"_ustr, u"NullDate"_ustr, u"RowLabelRanges"_ustr,
                     u"SheetLinks"_ustr, u"Theme"_ustr })
{
}

uno::Reference<uno::XInterface> ScSpreadsheetSettingsObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    return xDoc;
}

void ScSpreadsheetSettingsObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSpreadsheetSettingsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
