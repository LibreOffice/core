/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetannotationanchor.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XSheetAnnotationAnchor::testGetAnnotation()
{
    uno::Reference<sheet::XSheetAnnotationAnchor> xAnchor(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetAnnotation> xAnnotation(xAnchor->getAnnotation(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Unable to get XSheetAnnotation", xAnnotation.is());

    CPPUNIT_ASSERT_MESSAGE("Unable to check: getAuthor()", xAnnotation->getAuthor().isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Unable to check: getDate()", !xAnnotation->getDate().isEmpty());
    CPPUNIT_ASSERT_MESSAGE("Unable to check: getIsVisible()", !xAnnotation->getIsVisible());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to check: getPosition()", table::CellAddress(0, 2, 3),
                                 xAnnotation->getPosition());

    xAnnotation->setIsVisible(false);
    CPPUNIT_ASSERT_MESSAGE("Unable to setIsVisible() to false", !xAnnotation->getIsVisible());
    xAnnotation->setIsVisible(true);
    CPPUNIT_ASSERT_MESSAGE("Unable to setIsVisible() to true", xAnnotation->getIsVisible());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
