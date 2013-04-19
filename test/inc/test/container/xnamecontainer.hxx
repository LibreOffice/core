/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <test/testdllapi.hxx>

using namespace com::sun::star;

namespace apitest {

class OOO_DLLPUBLIC_TEST XNameContainer
{
public:
    // removes default entry
    XNameContainer();
    // removes given entry
    XNameContainer(const OUString& rName);

    virtual uno::Reference< uno::XInterface > init() = 0;

    void testRemoveByName();

    virtual ~XNameContainer() {}

private:
    OUString maNameToRemove;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
