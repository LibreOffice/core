/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/drawing/xshapes.hxx>
#include <sal/types.h>

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XShapes::testAddRemove()
{
    uno::Reference<drawing::XShapes> xShapes(init(), uno::UNO_QUERY_THROW);

    const sal_Int32 nCountBeforeAdd = xShapes->getCount();
    xShapes->add(m_xShape);
    const sal_Int32 nCountAfterAdd = xShapes->getCount();
    CPPUNIT_ASSERT_EQUAL(nCountBeforeAdd + 1, nCountAfterAdd);

    xShapes->remove(m_xShape);
    const sal_Int32 nCountAfterRemove = xShapes->getCount();
    CPPUNIT_ASSERT_EQUAL(nCountBeforeAdd, nCountAfterRemove);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
