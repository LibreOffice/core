/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/util/xindent.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XIndent.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XIndent::testIncrementIndent()
{
    uno::Reference<util::XIndent> xIndent(init(), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndent, UNO_QUERY_THROW);
    uno::Any aAny = xPropertySet->getPropertyValue(u"ParaIndent"_ustr);
    sal_Int32 nOldValue = aAny.get<sal_Int32>();

    xIndent->incrementIndent();

    uno::Any aAny2 = xPropertySet->getPropertyValue(u"ParaIndent"_ustr);
    sal_Int32 nNewValue = aAny2.get<sal_Int32>();
    CPPUNIT_ASSERT_MESSAGE("Successfully able to Increment Indent", nOldValue < nNewValue);
}
void XIndent::testDecrementIndent()
{
    uno::Reference<util::XIndent> xIndent(init(), UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndent, UNO_QUERY_THROW);
    xIndent->incrementIndent();
    uno::Any aAny = xPropertySet->getPropertyValue(u"ParaIndent"_ustr);
    sal_Int32 nOldValue = aAny.get<sal_Int32>();

    xIndent->decrementIndent();

    uno::Any aAny2 = xPropertySet->getPropertyValue(u"ParaIndent"_ustr);
    sal_Int32 nNewValue = aAny2.get<sal_Int32>();
    CPPUNIT_ASSERT_MESSAGE("Successfully able to Decrement Indent", nOldValue > nNewValue);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
