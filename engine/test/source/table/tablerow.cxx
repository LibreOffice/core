/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/table/tablerow.hxx>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

using namespace css;

namespace apitest
{
void TableRow::testTableRowProperties()
{
    uno::Reference<beans::XPropertySet> xPS(init(), uno::UNO_QUERY_THROW);

    OUString aPropName;

    aPropName = u"Height"_ustr;
    testLongProperty(xPS, aPropName);

    aPropName = u"OptimalHeight"_ustr;
    testBooleanProperty(xPS, aPropName);

    aPropName = u"IsVisible"_ustr;
    testBooleanProperty(xPS, aPropName);

    aPropName = u"IsStartOfNewPage"_ustr;
    testBooleanProperty(xPS, aPropName);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
