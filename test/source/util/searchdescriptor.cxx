/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/util/searchdescriptor.hxx>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void SearchDescriptor::testSearchDescriptorProperties()
{
    uno::Reference<beans::XPropertySet> xPS(init(), uno::UNO_QUERY_THROW);

    OUString aPropName;

    aPropName = "SearchBackwards";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchCaseSensitive";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchWords";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchRegularExpression";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchStyles";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchSimilarity";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchSimilarityRelax";
    testBooleanProperty(xPS, aPropName);

    aPropName = "SearchSimilarityRemove";
    testShortProperty(xPS, aPropName);

    aPropName = "SearchSimilarityAdd";
    testShortProperty(xPS, aPropName);

    aPropName = "SearchSimilarityExchange";
    testShortProperty(xPS, aPropName);

    aPropName = "SearchWildcard";
    testBooleanOptionalProperty(xPS, aPropName);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
