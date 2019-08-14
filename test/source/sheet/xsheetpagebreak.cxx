/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetpagebreak.hxx>

#include <com/sun/star/sheet/TablePageBreakData.hpp>
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>
#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetPageBreak::testGetColumnPageBreaks()
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak(init(), UNO_QUERY_THROW);

    uno::Sequence< sheet::TablePageBreakData > xColPageBreak = xSheetPageBreak->getColumnPageBreaks();
    CPPUNIT_ASSERT_MESSAGE("Unable to get column page breaks", xColPageBreak.hasElements());
}

void XSheetPageBreak::testGetRowPageBreaks()
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak(init(), UNO_QUERY_THROW);

    uno::Sequence< sheet::TablePageBreakData > xRowPageBreak = xSheetPageBreak->getRowPageBreaks();
    CPPUNIT_ASSERT_MESSAGE("Unable to get row page breaks", xRowPageBreak.hasElements());
}

void XSheetPageBreak::testRemoveAllManualPageBreaks()
{
    uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak(init(), UNO_QUERY_THROW);

    xSheetPageBreak->removeAllManualPageBreaks();

    const uno::Sequence< sheet::TablePageBreakData > xColPageBreak = xSheetPageBreak->getColumnPageBreaks();
    sal_Int32 manualColPageBreaks = 0;
    for ( const auto & data : xColPageBreak )
    {
        if (data.ManualBreak)
            manualColPageBreaks++;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Found manual column page break",
                                 sal_Int32(0), manualColPageBreaks);

    const uno::Sequence< sheet::TablePageBreakData > xRowPageBreak = xSheetPageBreak->getRowPageBreaks();
    sal_Int32 manualRowPageBreaks = 0;
    for ( const auto & data : xRowPageBreak )
    {
        if (data.ManualBreak)
            manualRowPageBreaks++;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Found manual row page break",
                                 sal_Int32(0), manualRowPageBreaks);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
