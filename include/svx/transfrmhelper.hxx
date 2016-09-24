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

#ifndef INCLUDED_SVX_TRANSFRMHELPER_HXX
#define INCLUDED_SVX_TRANSFRMHELPER_HXX

#include <basegfx/range/b2drange.hxx>
#include <tools/fldunit.hxx>
#include <tools/mapunit.hxx>

class SVX_DLLPUBLIC TransfrmHelper
{
public:
    static void ConvertRect(basegfx::B2DRange& rRange, const sal_uInt16 nDigits, const MapUnit ePoolUnit, const FieldUnit eDlgUnit)
    {
        const basegfx::B2DPoint aTopLeft(
            (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinX()), nDigits, ePoolUnit, eDlgUnit),
            (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinY()), nDigits, ePoolUnit, eDlgUnit));
        const basegfx::B2DPoint aBottomRight(
            (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxX()), nDigits, ePoolUnit, eDlgUnit),
            (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxY()), nDigits, ePoolUnit, eDlgUnit));

        rRange = basegfx::B2DRange(aTopLeft, aBottomRight);
    }

    static void ScaleRect(basegfx::B2DRange& rRange, const Fraction& rUIScale)
    {
        const double fFactor(1.0 / double(rUIScale));
        rRange = basegfx::B2DRange(rRange.getMinimum() * fFactor, rRange.getMaximum() * fFactor);
    }
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
