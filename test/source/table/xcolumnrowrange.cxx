/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <test/table/xcolumnrowrange.hxx>

#include <com/sun/star/table/XColumnRowRange.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XColumnRowRange::testGetColumns()
{
    uno::Reference<table::XColumnRowRange> xColumnRowRange(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xColumnRowRange->getColumns());
}

void XColumnRowRange::testGetRows()
{
    uno::Reference<table::XColumnRowRange> xColumnRowRange(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xColumnRowRange->getRows());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
