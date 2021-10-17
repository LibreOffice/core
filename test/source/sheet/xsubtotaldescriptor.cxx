/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsubtotaldescriptor.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSubTotalDescriptor::testAddNew()
{
    uno::Reference<sheet::XSubTotalDescriptor> xSTD(init(), uno::UNO_QUERY_THROW);

    uno::Sequence<sheet::SubTotalColumn> xCols{ { /* Column   */ 5,
                                                  /* Function */ sheet::GeneralFunction_SUM } };

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to add column", xSTD->addNew(xCols, 1));
}

void XSubTotalDescriptor::testClear()
{
    uno::Reference<sheet::XSubTotalDescriptor> xSTD(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to clear XSubTotalDescriptor", xSTD->clear());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
