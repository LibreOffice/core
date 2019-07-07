/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SHEET_FUNCTIONDESCRIPTION_HXX
#define INCLUDED_TEST_SHEET_FUNCTIONDESCRIPTION_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST FunctionDescription
{
public:
    virtual css::uno::Sequence<css::beans::PropertyValue> init() = 0;

    void testFunctionDescriptionProperties();

protected:
    ~FunctionDescription() {}
};
}

#endif // INCLUDED_TEST_SHEET_FUNCTIONDESCRIPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
