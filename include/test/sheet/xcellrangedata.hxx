/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XCELLRANGEDATA_HXX
#define INCLUDED_TEST_SHEET_XCELLRANGEDATA_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XCellRangeData
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual css::uno::Reference<css::uno::XInterface> getXCellRangeData() = 0;

    void testGetDataArray();
    void testSetDataArray();
    void testGetDataArrayOnTableSheet();
    void testSetDataArrayOnTableSheet();

protected:
    ~XCellRangeData() {}
};
}

#endif // INCLUDED_TEST_SHEET_XCELLRANGEDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
