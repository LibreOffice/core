/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/datapilotitem.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void DataPilotItem::testProperties()
{
    uno::Reference< beans::XPropertySet > xItem(init(), UNO_QUERY_THROW);


    const OUString propNameIS("IsHidden");

    bool bIsHidden = true;
    CPPUNIT_ASSERT(xItem->getPropertyValue(propNameIS) >>= bIsHidden);
    CPPUNIT_ASSERT_MESSAGE("Default IsHidden already changed", !bIsHidden);

    uno::Any aNewIsHidden;
    aNewIsHidden <<= false;
    xItem->setPropertyValue(propNameIS, aNewIsHidden);
    CPPUNIT_ASSERT(xItem->getPropertyValue(propNameIS) >>= bIsHidden);
    CPPUNIT_ASSERT_MESSAGE("Value of IsHidden wasn't changed", !bIsHidden);


    const OUString propNameSD("ShowDetail");

    bool bShowDetail = false;
    CPPUNIT_ASSERT(xItem->getPropertyValue(propNameSD) >>= bShowDetail);
    CPPUNIT_ASSERT_MESSAGE("Default ShowDetail already changed", bShowDetail);

    uno::Any aNewShowDetail;
    aNewShowDetail <<= true;
    xItem->setPropertyValue(propNameSD, aNewShowDetail);
    CPPUNIT_ASSERT(xItem->getPropertyValue(propNameSD) >>= bShowDetail);
    CPPUNIT_ASSERT_MESSAGE("Value of ShowDetail wasn't changed", bShowDetail);


    const OUString propNameP("Position");

    sal_Int32 nPosition = 42;
    CPPUNIT_ASSERT(xItem->getPropertyValue(propNameP) >>= nPosition);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default Position already changed", sal_Int32(0), nPosition);

    // FIXME: This throws somehow a com.sun.star.lang.IllegalArgumentException
    //uno::Any aNewPosition;
    //aNewPosition <<= static_cast<sal_Int32>(42);
    //xItem->setPropertyValue(propNameP, aNewPosition);
    //CPPUNIT_ASSERT(xItem->getPropertyValue(propNameP) >>= nPosition);
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Value of Position wasn't changed", sal_Int32(42), nPosition);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
