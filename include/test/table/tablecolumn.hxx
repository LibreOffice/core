/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST TableColumn
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    void setSpreadsheet(const css::uno::Reference<css::sheet::XSpreadsheet>& r_xSheet)
    {
        m_xSheet = r_xSheet;
    }

    void testTableColumnProperties();

protected:
    ~TableColumn() {}

private:
    css::uno::Reference<css::sheet::XSpreadsheet> m_xSheet;
};

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
