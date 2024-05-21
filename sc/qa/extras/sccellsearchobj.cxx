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
#include <test/lang/xserviceinfo.hxx>
#include <test/util/searchdescriptor.hxx>
#include <test/util/xreplacedescriptor.hxx>
#include <test/util/xsearchdescriptor.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/XSearchable.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScCellSearchObj : public UnoApiTest,
                        public apitest::SearchDescriptor,
                        public apitest::XPropertySet,
                        public apitest::XReplaceDescriptor,
                        public apitest::XSearchDescriptor,
                        public apitest::XServiceInfo
{
public:
    ScCellSearchObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScCellSearchObj);

    // SearchDescriptor
    CPPUNIT_TEST(testSearchDescriptorProperties);

    // XPropertSet
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);

    // XReplaceDescriptor
    CPPUNIT_TEST(testGetSetReplaceString);

    // XSearchDescriptor
    CPPUNIT_TEST(testGetSetSearchString);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();
};

ScCellSearchObj::ScCellSearchObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    , XServiceInfo(u"ScCellSearchObj"_ustr, { u"com.sun.star.util.ReplaceDescriptor"_ustr,
                                              u"com.sun.star.util.SearchDescriptor"_ustr })
{
}

uno::Reference<uno::XInterface> ScCellSearchObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<util::XSearchable> xSearchable(xSheet0, uno::UNO_QUERY_THROW);
    return xSearchable->createSearchDescriptor();
}

void ScCellSearchObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCellSearchObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
