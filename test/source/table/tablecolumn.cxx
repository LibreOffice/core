/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/tablecolumn.hxx>
#include <test/unoapi_property_testers.hxx>
#include <comphelper/types.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void TableColumn::testTableColumnProperties()
{
    uno::Reference<beans::XPropertySet> xPS(init(), uno::UNO_QUERY_THROW);

    OUString aPropName = u"Width"_ustr;
    testLongProperty(xPS, aPropName);

    // currently changing the value to false has no effect
    aPropName = "OptimalWidth";
    uno::Reference<table::XCellRange> xCR(m_xSheet, uno::UNO_QUERY_THROW);
    xCR->getCellByPosition(0, 0)->setFormula(u"That's a pretty long text."_ustr);
    const sal_Int64 nWidthBefore = ::comphelper::getINT64(xPS->getPropertyValue(u"Width"_ustr));
    xPS->setPropertyValue(aPropName, uno::Any(true));
    CPPUNIT_ASSERT(::comphelper::getBOOL(xPS->getPropertyValue(aPropName)));
    const sal_Int64 nWidthAfter = ::comphelper::getINT64(xPS->getPropertyValue(u"Width"_ustr));
    CPPUNIT_ASSERT(nWidthBefore != nWidthAfter);

    aPropName = "IsVisible";
    testBooleanProperty(xPS, aPropName);

    aPropName = "IsStartOfNewPage";
    testBooleanProperty(xPS, aPropName);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
