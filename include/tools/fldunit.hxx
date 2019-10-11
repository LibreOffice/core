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
#ifndef INCLUDED_TOOLS_FLDUNIT_HXX
#define INCLUDED_TOOLS_FLDUNIT_HXX

#include <sal/types.h>
#include <string>

// Corresponds to offapi/com/sun/star/awt/FieldUnit.idl
enum class FieldUnit : sal_uInt16
{
    NONE,
    MM,
    CM,
    M,
    KM,
    TWIP,
    POINT,
    PICA,
    INCH,
    FOOT,
    MILE,
    CHAR,
    LINE,
    CUSTOM,
    PERCENT,
    MM_100TH,
    PIXEL,
    DEGREE,
    SECOND,
    MILLISECOND,
};

static std::string FieldUnitToString(FieldUnit unit)
{
    switch(unit)
    {
        case FieldUnit::NONE:
            return "";

        case FieldUnit::MM:
            return "mm";

        case FieldUnit::CM:
            return "cm";

        case FieldUnit::M:
            return "m";

        case FieldUnit::KM:
            return "km";

        case FieldUnit::TWIP:
            return "twip";

        case FieldUnit::POINT:
            return "point";

        case FieldUnit::PICA:
            return "pica";

        case FieldUnit::INCH:
            return "inch";

        case FieldUnit::FOOT:
            return "foot";

        case FieldUnit::MILE:
            return "mile";

        case FieldUnit::CHAR:
            return "char";

        case FieldUnit::LINE:
            return "line";

        case FieldUnit::CUSTOM:
            return "custom";

        case FieldUnit::PERCENT:
            return "percent";

        case FieldUnit::MM_100TH:
            return "mm100th";

        case FieldUnit::PIXEL:
            return "pixel";

        case FieldUnit::DEGREE:
            return "degree";

        case FieldUnit::SECOND:
            return "second";

        case FieldUnit::MILLISECOND:
            return "millisecond";
    }

    return "";
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
