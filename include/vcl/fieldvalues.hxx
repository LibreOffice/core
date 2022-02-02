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

#ifndef INCLUDED_VCL_FIELDVALUES_HXX
#define INCLUDED_VCL_FIELDVALUES_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <tools/fldunit.hxx>
#include <tools/mapunit.hxx>

class LocaleDataWrapper;

namespace vcl
{
VCL_DLLPUBLIC FieldUnit EnglishStringToMetric(const OUString& rEnglishMetricString);

VCL_DLLPUBLIC bool TextToValue(const OUString& rStr, double& rValue, sal_Int64 nBaseValue,
                               sal_uInt16 nDecDigits, const LocaleDataWrapper& rLocaleDataWrapper,
                               FieldUnit eUnit);

VCL_DLLPUBLIC sal_Int64 ConvertValue(sal_Int64 nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                     FieldUnit eInUnit, FieldUnit eOutUnit);
VCL_DLLPUBLIC sal_Int64 ConvertValue(sal_Int64 nValue, sal_uInt16 nDecDigits, MapUnit eInUnit,
                                     FieldUnit eOutUnit);

// for backwards compatibility
// caution: conversion to double loses precision
VCL_DLLPUBLIC double ConvertDoubleValue(double nValue, sal_Int64 mnBaseValue, sal_uInt16 nDecDigits,
                                        FieldUnit eInUnit, FieldUnit eOutUnit);
VCL_DLLPUBLIC double ConvertDoubleValue(double nValue, sal_uInt16 nDecDigits, FieldUnit eInUnit,
                                        MapUnit eOutUnit);
VCL_DLLPUBLIC double ConvertDoubleValue(double nValue, sal_uInt16 nDecDigits, MapUnit eInUnit,
                                        FieldUnit eOutUnit);

// for backwards compatibility
// caution: conversion to double loses precision
inline double ConvertDoubleValue(sal_Int64 nValue, sal_Int64 nBaseValue, sal_uInt16 nDecDigits,
                                 FieldUnit eInUnit, FieldUnit eOutUnit)
{
    return ConvertDoubleValue(static_cast<double>(nValue), nBaseValue, nDecDigits, eInUnit,
                              eOutUnit);
}

inline double ConvertDoubleValue(sal_Int64 nValue, sal_uInt16 nDecDigits, FieldUnit eInUnit,
                                 MapUnit eOutUnit)
{
    return ConvertDoubleValue(static_cast<double>(nValue), nDecDigits, eInUnit, eOutUnit);
}

inline double ConvertDoubleValue(sal_Int64 nValue, sal_uInt16 nDecDigits, MapUnit eInUnit,
                                 FieldUnit eOutUnit)
{
    return ConvertDoubleValue(static_cast<double>(nValue), nDecDigits, eInUnit, eOutUnit);
}
}

#endif // INCLUDED_VCL_FIELDVALUES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
