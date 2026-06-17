/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>

using namespace com::sun::star;

namespace
{
class FormsTest : public UnoApiTest
{
public:
    FormsTest()
        : UnoApiTest(u""_ustr)
    {
    }

    uno::Reference<beans::XPropertySet> createFormComponent(const OUString& rServiceName)
    {
        return uno::Reference<beans::XPropertySet>(
            comphelper::getProcessServiceFactory()->createInstance(rServiceName),
            uno::UNO_QUERY_THROW);
    }
};

CPPUNIT_TEST_FIXTURE(FormsTest, testTdf51727_GridControlTabStopDefault)
{
    auto xModel = createFormComponent(u"com.sun.star.form.component.GridControl"_ustr);

    // Without the fix in place, this test would have failed with
    // - Expected: false
    // - Actual  : void
    // i.e. the tab stop property of the grid control was not initialized correctly
    uno::Any aTabStop = xModel->getPropertyValue(u"Tabstop"_ustr);
    CPPUNIT_ASSERT(aTabStop.hasValue());
    bool bTabStop = true;
    CPPUNIT_ASSERT(aTabStop >>= bTabStop);
    CPPUNIT_ASSERT_EQUAL(false, bTabStop);
}

} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
