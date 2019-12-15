/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/drawing/xshapegrouper.hxx>
#include <sal/types.h>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapeGroup.hpp>
#include <com/sun/star/drawing/XShapeGrouper.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XShapeGrouper::testGroup()
{
    uno::Reference<drawing::XShapeGrouper> xSG(init(), uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XShapes> xShapes(m_xDrawPage, uno::UNO_QUERY_THROW);
    const sal_Int32 nCountBeforeGroup = xShapes->getCount();

    uno::Reference<drawing::XShape> xShape(xSG->group(xShapes), uno::UNO_QUERY_THROW);
    const sal_Int32 nCountAfterGroup = xShapes->getCount();

    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT(nCountAfterGroup < nCountBeforeGroup);
}

void XShapeGrouper::testUngroup()
{
    uno::Reference<drawing::XShapeGrouper> xSG(init(), uno::UNO_QUERY_THROW);

    uno::Reference<drawing::XShapes> xShapes(m_xDrawPage, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xShape(xSG->group(xShapes), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapeGroup> xShapeGroup(xShape, uno::UNO_QUERY_THROW);

    const sal_Int32 nCountAfterGroup = xShapes->getCount();

    xSG->ungroup(xShapeGroup);
    const sal_Int32 nCountAfterUngroup = xShapes->getCount();

    CPPUNIT_ASSERT(nCountAfterUngroup != nCountAfterGroup);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
