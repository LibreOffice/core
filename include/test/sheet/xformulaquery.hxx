/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XFORMULAQUERY_HXX
#define INCLUDED_TEST_SHEET_XFORMULAQUERY_HXX

#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XFormulaQuery
{
public:
    XFormulaQuery(css::table::CellRangeAddress aCellRangeAddressDependents,
                  css::table::CellRangeAddress aCellRangeAddressPrecedents,
                  unsigned int nIdxDependents = 1, unsigned int nIdxPrecedents = 1)
        : m_aCellRangeAddressDependents(aCellRangeAddressDependents)
        , m_aCellRangeAddressPrecedents(aCellRangeAddressPrecedents)
        , m_nIdxDependents(nIdxDependents)
        , m_nIdxPrecedents(nIdxPrecedents)
    {
    }

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual css::uno::Reference<css::uno::XInterface> getXSpreadsheet() = 0;
    void setXCell(css::uno::Reference<css::table::XCell> xCell) { m_xCell = xCell; }
    css::uno::Reference<css::table::XCell> const& getXCell() const { return m_xCell; }

    void testQueryDependents();
    void testQueryPrecedents();

protected:
    ~XFormulaQuery() {}

private:
    css::uno::Reference<css::table::XCell> m_xCell;
    css::table::CellRangeAddress const m_aCellRangeAddressDependents;
    css::table::CellRangeAddress const m_aCellRangeAddressPrecedents;
    unsigned int const m_nIdxDependents;
    unsigned int const m_nIdxPrecedents;
};
} // namespace apitest

#endif // INCLUDED_TEST_SHEET_XFORMULAQUERY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
