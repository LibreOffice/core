/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xconsolidatable.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XConsolidatable.hpp>
#include <com/sun/star/sheet/XConsolidationDescriptor.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XConsolidatable::testCreateConsolidationDescriptor()
{
    uno::Reference<sheet::XConsolidatable> xConsolidatable(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(
        xConsolidatable->createConsolidationDescriptor(true), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to create ConsolidationDescriptor",
                           xConsolidationDescriptor.is());
}

void XConsolidatable::testConsolidate()
{
    uno::Reference<sheet::XConsolidatable> xConsolidatable(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XConsolidationDescriptor> xConsolidationDescriptor(
        xConsolidatable->createConsolidationDescriptor(true), UNO_QUERY_THROW);

    xConsolidatable->consolidate(xConsolidationDescriptor);
    CPPUNIT_ASSERT_MESSAGE("consolidate()", true);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
