/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

namespace net_uno
{
using IntPtr = void*;
using String = const sal_Unicode*;

union Value {
    // bool
    sal_Bool boolData;

    // byte
    sal_Int8 byteData;

    // char
    sal_Unicode charData;

    // short
    sal_Int16 shortData;
    sal_uInt16 unsigShortData;

    // long
    sal_Int32 longData;
    sal_uInt32 unsigLongData;

    // hyper
    sal_Int64 hyperData;
    sal_uInt64 unsigHyperData;

    // float/double
    float floatData;
    double doubleData;

    // string
    IntPtr stringData;

    // type
    IntPtr typeData;

    // any
    struct Any
    {
        IntPtr data;
        IntPtr type;
    } anyData;

    // enum
    sal_Int32 enumData;

    // struct
    IntPtr structData;

    // exception
    IntPtr exceptionData;

    // sequence
    struct Sequence
    {
        IntPtr data;
        sal_Int32 length;
    } sequenceData;

    // interface
    IntPtr interfaceData;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
