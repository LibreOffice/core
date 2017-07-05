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

    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong X position",
                                 sal_Int32(7373), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong Y position",
                                 sal_Int32(426), xShape->getPosition().Y);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong width",
                                 sal_Int32(11275), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("getAnnotationShape() wrong height",
                                 sal_Int32(1386), xShape->getSize().Height);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
