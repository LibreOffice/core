/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsubtotalfield.hxx>

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/SubTotalColumn.hpp>
#include <com/sun/star/sheet/XSubTotalField.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

CPPUNIT_NS_BEGIN

template<> struct assertion_traits<uno::Sequence< sheet::SubTotalColumn > >
{
    static bool equal(const uno::Sequence< sheet::SubTotalColumn >& x,
                      const uno::Sequence< sheet::SubTotalColumn >& y)
    {
        return x == y;
    }

    static std::string toString(const uno::Sequence< sheet::SubTotalColumn >& x)
    {
        OStringStream ost;
        ost << "Sequence: Length: " << x.getLength() << "\n";
        for (const auto& rElement : x)
            ost << "Column: " << rElement.Column << " Function:\n";
            // FIXME: Find a way to print Function
            //ost << "Column: " << element->Column << " Function: " << element->Function << "\n";
        return ost.str();
    }
};

CPPUNIT_NS_END

namespace apitest {

void XSubTotalField::testGetSetGroupColumn()
{
    uno::Reference< sheet::XSubTotalField > xSTF(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Unable to get GroupColumn", xSTF->getGroupColumn() != 0);

    xSTF->setGroupColumn(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set GroupColumn to new value",
                                 sal_Int32(2), xSTF->getGroupColumn());
}

void XSubTotalField::testGetSetTotalColumns()
{
    uno::Reference< sheet::XSubTotalField > xSTF(init(), UNO_QUERY_THROW);

    uno::Sequence< sheet::SubTotalColumn > sDefaultCols = xSTF->getSubTotalColumns();
    CPPUNIT_ASSERT_MESSAGE("Unable to get SubTotalColumns", sDefaultCols.hasElements());

    uno::Sequence< sheet::SubTotalColumn > sNewCols{ { /* Column   */ 5,
                                                       /* Function */ sheet::GeneralFunction_AVERAGE } };
    xSTF->setSubTotalColumns(sNewCols);

    CPPUNIT_ASSERT_MESSAGE("Unable to set SubTotalColumns", sDefaultCols != xSTF->getSubTotalColumns());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
