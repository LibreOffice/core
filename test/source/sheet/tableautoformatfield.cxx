/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/tableautoformatfield.hxx>

#include <iostream>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

uno::Reference< beans::XPropertySet > TableAutoFormatField::initTest()
{
    uno::Reference< container::XIndexAccess > xAutoFormatFields( getServiceFactory()->createInstance("com.sun.star.sheet.TableAutoFormats"), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xAutoFormatFields.is());
    uno::Reference< container::XIndexAccess > xIndex( xAutoFormatFields->getByIndex(0), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xIndex.is());
    uno::Reference< beans::XPropertySet > xReturn( xIndex->getByIndex(0), UNO_QUERY_THROW );
    CPPUNIT_ASSERT_MESSAGE("could not create table auto formats", xReturn.is());
    return xReturn;
}

void TableAutoFormatField::testRotateReference()
{
    uno::Reference< beans::XPropertySet > xTableAutoFormatField = initTest();
    OUString aRotateReference("RotateReference");
    uno::Any aOldRotateReference = xTableAutoFormatField->getPropertyValue(aRotateReference);
    sal_Int32 aValue = 0;
    CPPUNIT_ASSERT(aOldRotateReference >>= aValue);
    std::cout << "Old RotateReferene Value: " << aValue << std::endl;

    uno::Any aNewRotateReference;
    aNewRotateReference <<= static_cast<sal_Int32>(3);
    xTableAutoFormatField->setPropertyValue(aRotateReference, aNewRotateReference);
    uno::Any aRotateReferenceControllValue = xTableAutoFormatField->getPropertyValue(aRotateReference);
    CPPUNIT_ASSERT(aRotateReferenceControllValue >>= aValue);
    std::cout << "New VertJustify value: " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("value has not been changed", aValue == 3);
}

void TableAutoFormatField::testVertJustify()
{
    uno::Reference< beans::XPropertySet > xTableAutoFormatField = initTest();
    OUString aVertJustify("VertJustify");
    uno::Any aOldVertJustify = xTableAutoFormatField->getPropertyValue(aVertJustify);
    sal_Int32 aValue = 0;
    CPPUNIT_ASSERT(aOldVertJustify >>= aValue);
    std::cout << "Old VertJustify value: " << aValue << std::endl;

    uno::Any aNewVertJustify;
    aNewVertJustify <<= static_cast<sal_Int32>(3);
    xTableAutoFormatField->setPropertyValue(aVertJustify, aNewVertJustify);
    uno::Any aVertJustifyControllValue = xTableAutoFormatField->getPropertyValue(aVertJustify);
    CPPUNIT_ASSERT(aVertJustifyControllValue >>= aValue);
    std::cout << "New VertJustify value: " << aValue << std::endl;
    CPPUNIT_ASSERT_MESSAGE("value has not been changed", aValue == 3);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
