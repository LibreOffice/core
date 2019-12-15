/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/container/xchild.hxx>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XChild::testGetSetParent()
{
    uno::Reference<container::XChild> xChild(init(), UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xInterface(xChild, UNO_QUERY_THROW);
    xInterface = xChild->getParent();

    CPPUNIT_ASSERT_MESSAGE("Successfully able to Get Parent", xInterface);

    xChild->setParent(xInterface);

    CPPUNIT_ASSERT_MESSAGE("Successfully able to Set Parent", xChild);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
