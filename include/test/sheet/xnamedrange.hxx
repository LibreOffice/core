/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_XNAMEDRANGE_HXX
#define INCLUDED_TEST_SHEET_XNAMEDRANGE_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/sheet/XNamedRange.hpp>
#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST XNamedRange
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;

    // XNamedRange
    void testGetContent();
    void testSetContent();
    void testGetType();
    void testSetType();
    void testGetReferencePosition();
    void testSetReferencePosition();

protected:
    ~XNamedRange() {}

    virtual css::uno::Reference<css::sheet::XNamedRange> getNamedRange(const OUString&) = 0;
};
}

#endif // INCLUDED_TEST_SHEET_XNAMEDRANGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
