/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_INC_DPGLOBAL_HXX
#define INCLUDED_SC_INC_DPGLOBAL_HXX

#include <o3tl/typed_flags_set.hxx>

#define     PIVOT_MAXFUNC           11

enum class PivotFunc {
    NONE         = 0x0000,
    Sum          = 0x0001,
    Count        = 0x0002,
    Average      = 0x0004,
    Max          = 0x0008,
    Min          = 0x0010,
    Product      = 0x0020,
    CountNum     = 0x0040,
    StdDev       = 0x0080,
    StdDevP      = 0x0100,
    StdVar       = 0x0200,
    StdVarP      = 0x0400,
    Auto         = 0x1000
};
namespace o3tl {
    template<> struct typed_flags<PivotFunc> : is_typed_flags<PivotFunc, 0x17ff> {};
}

struct ScDPValue
{
    enum Type { Empty = 0, Value, String, Error };

    double mfValue;
    Type meType;

    ScDPValue();

    void Set( double fV, Type eT );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
