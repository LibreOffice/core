/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include "scdllapi.h"
#include "global.hxx"

class SvNumberFormatter;
struct ScDPNumGroupInfo;
enum class ScGeneralFunction;

class ScDPUtil
{
public:
    static bool isDuplicateDimension(const OUString& rName);

    SC_DLLPUBLIC static OUString getSourceDimensionName(std::u16string_view rName);

    /**
     * Get a duplicate index in case the dimension is a duplicate.  It returns
     * 0 in case it's an original dimension.
     */
    sal_uInt8 static getDuplicateIndex(const OUString& rName);

    SC_DLLPUBLIC static OUString createDuplicateDimensionName(const OUString& rOriginal, size_t nDupCount);

    SC_DLLPUBLIC static OUString getDateGroupName(
        sal_Int32 nDatePart, sal_Int32 nValue, SvNumberFormatter* pFormatter,
        double fStart, double fEnd);

    static double getNumGroupStartValue(double fValue, const ScDPNumGroupInfo& rInfo);

    static OUString getNumGroupName(
        double fValue, const ScDPNumGroupInfo& rInfo, sal_Unicode cDecSep,
        SvNumberFormatter* pFormatter);

    static sal_Int32 getDatePartValue(
        double fValue, const ScDPNumGroupInfo* pInfo, sal_Int32 nDatePart,
        const SvNumberFormatter* pFormatter);

    SC_DLLPUBLIC static OUString getDisplayedMeasureName(std::u16string_view rName, ScSubTotalFunc eFunc);

    SC_DLLPUBLIC static ScSubTotalFunc toSubTotalFunc(ScGeneralFunction eGenFunc);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
