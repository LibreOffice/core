/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_TABLE_XCELLRANGE_HXX
#define INCLUDED_TEST_TABLE_XCELLRANGE_HXX

#include <test/testdllapi.hxx>

#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <utility>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XCellRange
{
public:
    XCellRange()
        : m_aRangeName(u"A1:A1"_ustr)
    {
    }
    XCellRange(OUString aRangeName)
        : m_aRangeName(std::move(aRangeName))
    {
    }

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;

    void testGetCellByPosition();
    void testGetCellRangeByName();
    void testGetCellRangeByPosition();

protected:
    ~XCellRange() {}

private:
    OUString const m_aRangeName;
};
} // namespace apitest

#endif // INCLUDED_TEST_TABLE_XCELLRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
