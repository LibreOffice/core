/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FORMULA_PARAMCLASS_HXX
#define INCLUDED_FORMULA_PARAMCLASS_HXX

#include <sal/config.h>
#include <sal/types.h>

namespace formula
{
    enum ParamClass : sal_uInt8
    {
        Unknown = 0,    // MUST be zero for initialization mechanism!

        /** Out of bounds, function doesn't expect that many parameters.
            However, not necessarily returned if a module specific definition
            for example returns type Value for an unlisted function. */
        Bounds,

        /** In array formula: single value to be passed. Results in JumpMatrix
            being created and multiple calls to function. Functions handling a
            formula::svDoubleRef by means of DoubleRefToPosSingleRef() or
            PopDoubleRefOrSingleRef() or GetDouble() or GetString() should have
            this. */
        Value,

        /** In array formula: area reference must stay reference. Otherwise
            don't care. Functions handling a formula::svDoubleRef by means of
            PopDoubleRefOrSingleRef() should not have this. */
        Reference,

        /** In array formula: convert area reference to array. Function will be
            called only once if no Value type is involved. Functions able to
            handle a svMatrix parameter but not a formula::svDoubleRef parameter as area
            should have this. */
        Array,

        /** Area reference must be converted to array in any case, and must
            also be propagated to subsequent operators and functions being part
            of a parameter of this function. */
        ForceArray,

        /** Area reference is not converted to array, but ForceArray must be
            propagated to subsequent operators and functions being part of a
            parameter of this function. Used with functions that treat
            references separately from arrays, but need the forced array
            calculation of parameters that are not references.*/
        ReferenceOrForceArray
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
