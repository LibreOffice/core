/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetannotationshapesupplier.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/sheet/XSheetAnnotationShapeSupplier.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace apitest {

void XSheetAnnotationShapeSupplier::testGetAnnotationShape()
{
    uno::Reference< sheet::XSheetAnnotationShapeSupplier > xSheetAnnotationShapeSupplier(init(), UNO_QUERY_THROW);
    uno::Reference< drawing::XShape > xShape = xSheetAnnotationShapeSupplier->getAnnotationShape();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong X position", 7373L, xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong Y position", 426L, xShape->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong width", 11275L, xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong height", 1386L, xShape->getSize().Height);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
