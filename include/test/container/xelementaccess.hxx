/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace com::sun::star;

namespace apitest {

class XElementAccess
{
public:
    XElementAccess(uno::Type& rType): maType(rType) {}

    void testGetElementType();
    void testHasElements();

    virtual uno::Reference< uno::XInterface > init() = 0;

    virtual ~XElementAccess() {}

private:
    uno::Type maType;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
