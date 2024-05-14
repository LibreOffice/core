/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/cellproperties.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppunit/TestAssert.h>
#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest
{
void CellProperties::testVertJustify()
{
    uno::Reference<beans::XPropertySet> xCellRangeBase(init(), UNO_QUERY_THROW);
    OUString aVertJustify(u"VertJustify"_ustr);
    uno::Any aOldVertJustify = xCellRangeBase->getPropertyValue(aVertJustify);
    sal_Int32 aValue = 0;
    CPPUNIT_ASSERT(aOldVertJustify >>= aValue);
    std::cout << "Old VertJustify value: " << aValue << std::endl;

    uno::Any aNewVertJustify;
    aNewVertJustify <<= static_cast<sal_Int32>(3);
    xCellRangeBase->setPropertyValue(aVertJustify, aNewVertJustify);
    uno::Any aVertJustifyControllValue = xCellRangeBase->getPropertyValue(aVertJustify);
    CPPUNIT_ASSERT(aVertJustifyControllValue >>= aValue);
    std::cout << "New VertJustify value: " << aValue << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("value has not been changed", sal_Int32(3), aValue);
}

void CellProperties::testRotateReference()
{
    uno::Reference<beans::XPropertySet> xCellRangeBase(init(), UNO_QUERY_THROW);
    OUString aRotateReference(u"RotateReference"_ustr);
    uno::Any aOldRotateReference = xCellRangeBase->getPropertyValue(aRotateReference);
    sal_Int32 aValue = 0;
    CPPUNIT_ASSERT(aOldRotateReference >>= aValue);
    std::cout << "Old RotateReference Value: " << aValue << std::endl;

    uno::Any aNewRotateReference;
    aNewRotateReference <<= static_cast<sal_Int32>(3);
    xCellRangeBase->setPropertyValue(aRotateReference, aNewRotateReference);
    uno::Any aRotateReferenceControllValue = xCellRangeBase->getPropertyValue(aRotateReference);
    CPPUNIT_ASSERT(aRotateReferenceControllValue >>= aValue);
    std::cout << "New RotateReference value: " << aValue << std::endl;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("value has not been changed", sal_Int32(3), aValue);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
