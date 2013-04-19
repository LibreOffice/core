/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <test/testdllapi.hxx>

using namespace com::sun::star;

namespace apitest {

class OOO_DLLPUBLIC_TEST XSearchable
{
public:
    XSearchable(const OUString& rString, sal_Int32 nCount) : maSearchString(rString), mnCount(nCount) {}
    XSearchable() : maSearchString(RTL_CONSTASCII_USTRINGPARAM("SearchString")), mnCount(1) {}
    virtual ~XSearchable();

    void testFindFirst();
    void testFindNext();
    void testFindAll();

    virtual uno::Reference< uno::XInterface > init() = 0;

private:
    OUString maSearchString;
    sal_Int32 mnCount;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
