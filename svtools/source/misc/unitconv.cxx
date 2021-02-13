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

#include <o3tl/temporary.hxx>
#include <svtools/unitconv.hxx>
#include <tools/debug.hxx>
#include <tools/UnitConversion.hxx>
#include <vcl/outdev.hxx>
#include <vcl/weld.hxx>

void SetFieldUnit(weld::MetricSpinButton& rField, FieldUnit eUnit, bool bAll)
{
    int nMin, nMax;
    rField.get_range(nMin, nMax, FieldUnit::TWIP);
    int nValue = rField.get_value(FieldUnit::TWIP);
    nMin = rField.denormalize(nMin);
    nMax = rField.denormalize(nMax);
    nValue = rField.denormalize(nValue);

    if (!bAll)
    {
        switch (eUnit)
        {
            case FieldUnit::M:
            case FieldUnit::KM:
                eUnit = FieldUnit::CM;
                break;
            case FieldUnit::FOOT:
            case FieldUnit::MILE:
                eUnit = FieldUnit::INCH;
                break;
            default: //prevent warning
                break;
        }
    }

    rField.set_unit(eUnit);

    if (FieldUnit::POINT == eUnit)
    {
        if (rField.get_digits() > 1)
            rField.set_digits(1);
    }
    else
        rField.set_digits(2);

    switch (eUnit)
    {
        // _CHAR and _LINE sets the step of "char" and "line" unit, they are same as FieldUnit::MM
        case FieldUnit::CHAR:
        case FieldUnit::LINE:
        case FieldUnit::MM:
            rField.set_increments(50, 500, eUnit);
            break;
        case FieldUnit::INCH:
            rField.set_increments(2, 20, eUnit);
            break;
        default:
            rField.set_increments(10, 100, eUnit);
            break;
    }

    if (!bAll)
    {
        nMin = rField.normalize(nMin);
        nMax = rField.normalize(nMax);
        rField.set_range(nMin, nMax, FieldUnit::TWIP);
    }

    rField.set_value(rField.normalize(nValue), FieldUnit::TWIP);
}

void SetMetricValue(weld::MetricSpinButton& rField, int nCoreValue, MapUnit eUnit)
{
    auto nVal = OutputDevice::LogicToLogic(nCoreValue, eUnit, MapUnit::Map100thMM);
    nVal = rField.normalize(nVal);
    rField.set_value(nVal, FieldUnit::MM_100TH);
}

int GetCoreValue(const weld::MetricSpinButton& rField, MapUnit eUnit)
{
    int nVal = rField.get_value(FieldUnit::MM_100TH);
    // avoid rounding issues
    const int nSizeMask = 0xff000000;
    bool bRoundBefore = true;
    if( nVal >= 0 )
    {
        if( (nVal & nSizeMask) == 0 )
            bRoundBefore = false;
    }
    else
    {
        if( ((-nVal) & nSizeMask ) == 0 )
            bRoundBefore = false;
    }
    if( bRoundBefore )
        nVal = rField.denormalize( nVal );
    auto nUnitVal = OutputDevice::LogicToLogic(nVal, MapUnit::Map100thMM, eUnit);
    if (!bRoundBefore)
        nUnitVal = rField.denormalize(nUnitVal);
    return nUnitVal;
}

tools::Long CalcToUnit( float nIn, MapUnit eUnit )
{
    // nIn is in Points

    DBG_ASSERT( eUnit == MapUnit::MapTwip       ||
                eUnit == MapUnit::Map100thMM   ||
                eUnit == MapUnit::Map10thMM    ||
                eUnit == MapUnit::MapMM         ||
                eUnit == MapUnit::MapCM, "this unit is not implemented" );

    if (const auto eTo = MapToO3tlLength(eUnit); eTo != o3tl::Length::invalid)
        return o3tl::convert(nIn, o3tl::Length::pt, eTo);

    return 0;
}


tools::Long ItemToControl( tools::Long nIn, MapUnit eItem, FieldUnit eCtrl )
{
    const auto eFrom = MapToO3tlLength(eItem), eTo = FieldToO3tlLength(eCtrl);
    if (eFrom != o3tl::Length::invalid && eTo != o3tl::Length::invalid)
        return o3tl::convert(nIn, eFrom, eTo);
    return 0;
}


tools::Long ControlToItem( tools::Long nIn, FieldUnit eCtrl, MapUnit eItem )
{
    return ItemToControl( nIn, eItem, eCtrl );
}


FieldUnit MapToFieldUnit( const MapUnit eUnit )
{
    switch ( eUnit )
    {
        case MapUnit::Map100thMM:
        case MapUnit::Map10thMM:
        case MapUnit::MapMM:
            return FieldUnit::MM;

        case MapUnit::MapCM:
            return FieldUnit::CM;

        case MapUnit::Map1000thInch:
        case MapUnit::Map100thInch:
        case MapUnit::Map10thInch:
        case MapUnit::MapInch:
            return FieldUnit::INCH;

        case MapUnit::MapPoint:
            return FieldUnit::POINT;

        case MapUnit::MapTwip:
            return FieldUnit::TWIP;
        default: ;//prevent warning
    }
    return FieldUnit::NONE;
}


tools::Long CalcToPoint( tools::Long nIn, MapUnit eUnit, sal_uInt16 nFactor )
{
    DBG_ASSERT( eUnit == MapUnit::MapTwip       ||
                eUnit == MapUnit::Map100thMM   ||
                eUnit == MapUnit::Map10thMM    ||
                eUnit == MapUnit::MapMM         ||
                eUnit == MapUnit::MapCM, "this unit is not implemented" );

    if (const auto eTo = MapToO3tlLength(eUnit); eTo != o3tl::Length::invalid)
        return o3tl::convert(nIn, eTo, o3tl::Length::pt) * nFactor;
    return 0;
}


tools::Long TransformMetric( tools::Long nVal, FieldUnit aOld, FieldUnit aNew )
{
    const auto eFrom = FieldToO3tlLength(aOld), eTo = FieldToO3tlLength(aNew);
    if (eFrom == o3tl::Length::invalid || eTo == o3tl::Length::invalid)
        return nVal;
    return o3tl::convert(nVal, eFrom, eTo, o3tl::temporary(bool())); // Just 0 when overflown
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
