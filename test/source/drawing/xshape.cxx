/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/drawing/xshape.hxx>
#include <test/cppunitasserthelper.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XShape::testGetSetPosition()
{
    uno::Reference<drawing::XShape> xShape(init(), uno::UNO_QUERY_THROW);
    if (m_bObjSupportsPosition)
    {
        awt::Point aOldPoint = xShape->getPosition();
        CPPUNIT_ASSERT_NO_THROW(
            xShape->setPosition(awt::Point(aOldPoint.X + 100, aOldPoint.Y + 100)));

        awt::Point aNewPoint = xShape->getPosition();
        CPPUNIT_ASSERT_EQUAL(awt::Point(aOldPoint.X + 100, aOldPoint.Y + 100), aNewPoint);
    }
    CPPUNIT_ASSERT(true);
}

void XShape::testGetSetSize()
{
    uno::Reference<drawing::XShape> xShape(init(), uno::UNO_QUERY_THROW);

    awt::Size aOldSize = xShape->getSize();

    CPPUNIT_ASSERT_NO_THROW(xShape->setSize(awt::Size(aOldSize.Width + 10, aOldSize.Height + 10)));

    awt::Size aNewSize = xShape->getSize();
    const sal_Int16 aAcceptableLimit = 2;
    CPPUNIT_ASSERT(aOldSize.Height - aNewSize.Height <= aAcceptableLimit);
    CPPUNIT_ASSERT(aOldSize.Height - aNewSize.Height <= aAcceptableLimit);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
