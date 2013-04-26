/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>

#include <test/testdllapi.hxx>

#include <vector>

namespace apitest {

class OOO_DLLPUBLIC_TEST XDataPilotDescriptor
{
public:
    void testSourceRange();
    void testTag();
    void testGetFilterDescriptor();
    void testGetDataPilotFields();
    void testGetColumnFields();
    void testGetRowFields();
    void testGetPageFields();
    void testGetDataFields();
    void testGetHiddenFields();

    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

protected:
    ~XDataPilotDescriptor() {}

private:
    void testGetDataPilotFields_Impl( css::uno::Reference< css::sheet::XDataPilotDescriptor > xDescr );

    void checkName( css::uno::Reference< css::container::XIndexAccess > xIndex, sal_Int32 nIndex );
    static std::vector<OUString> maFieldNames;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
