/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initTestial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/sheet/tableautoformatfield.hxx>

#include <iostream>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

uno::Reference< beans::XPropertySet > TableAutoFormatField::initTest()
{
    uno::Reference< container::XIndexAccess > xAutoFormatFields( getServiceFactory()->createInstance(rtl::OUString("com.sun.star.sheet.TableAutoFormats")), UNO_QUERY_THROW );
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
    rtl::OUString aRotateReference("RotateReference");
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
    rtl::OUString aVertJustify("VertJustify");
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
