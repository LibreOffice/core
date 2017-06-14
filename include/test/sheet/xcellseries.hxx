/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XCELLSERIES_HXX
#define INCLUDED_TEST_SHEET_XCELLSERIES_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <test/testdllapi.hxx>

namespace apitest {

class OOO_DLLPUBLIC_TEST XCellSeries
{
public:
    XCellSeries(sal_Int32 aStartX, sal_Int32 aStartY) : maStartX(aStartX), maStartY(aStartY){}
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;
    virtual ~XCellSeries(){}

    void testFillAuto();
    void testFillSeries();

private:
    sal_Int32 maStartX;
    sal_Int32 maStartY;
};

}

#endif // INCLUDED_TEST_SHEET_XCELLSERIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

