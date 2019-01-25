/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XNAMEDRANGES_HXX
#define INCLUDED_TEST_SHEET_XNAMEDRANGES_HXX

#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XNamedRanges
{
public:
    // remove default entry
    XNamedRanges()
        : maNameToRemove("initial1")
    {
    }

    // removes given entry
    XNamedRanges(const OUString& rNameToRemove)
        : maNameToRemove(rNameToRemove)
    {
    }

    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual css::uno::Reference<css::uno::XInterface> getXNamedRanges(sal_Int32 nSheet = 0) = 0;

    // XNamedRanges
    void testAddNewByName();
    void testAddNewFromTitles();
    void testRemoveByName();
    void testOutputList();

protected:
    ~XNamedRanges(){};
    css::uno::Reference<css::sheet::XSpreadsheet> xSheet;

private:
    OUString const maNameToRemove;
};

} // namespace apitest

#endif // INCLUDED_TEST_SHEET_XNAMEDRANGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
