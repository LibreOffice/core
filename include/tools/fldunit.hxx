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

#include <sal/config.h>

#include <o3tl/unit_conversion.hxx>
#include <sal/types.h>

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
    CUSTOM,
    PERCENT,
    MM_100TH,
    CHAR,
    LINE,
    PIXEL,
    DEGREE,
    SECOND,
    MILLISECOND,
};

constexpr o3tl::Length FieldToO3tlLength(FieldUnit eU, o3tl::Length ePixelValue = o3tl::Length::px)
{
    switch (eU)
    {
        case FieldUnit::MM:
            return o3tl::Length::mm;
        case FieldUnit::CM:
            return o3tl::Length::cm;
        case FieldUnit::M:
            return o3tl::Length::m;
        case FieldUnit::KM:
            return o3tl::Length::km;
        case FieldUnit::TWIP:
            return o3tl::Length::twip;
        case FieldUnit::POINT:
            return o3tl::Length::pt;
        case FieldUnit::PICA:
            return o3tl::Length::pc;
        case FieldUnit::INCH:
            return o3tl::Length::in;
        case FieldUnit::FOOT:
            return o3tl::Length::ft;
        case FieldUnit::MILE:
            return o3tl::Length::mi;
        case FieldUnit::CHAR:
            return o3tl::Length::ch;
        case FieldUnit::LINE:
            return o3tl::Length::line;
        case FieldUnit::MM_100TH:
            return o3tl::Length::mm100;
        case FieldUnit::PIXEL:
            return ePixelValue;
        default:
            return o3tl::Length::invalid;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
