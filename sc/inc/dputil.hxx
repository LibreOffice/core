/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_DPUTIL_HXX
#define SC_DPUTIL_HXX

#include "rtl/ustring.hxx"
#include "scdllapi.h"
#include "global.hxx"

#include <com/sun/star/sheet/GeneralFunction.hpp>

class SvNumberFormatter;
struct ScDPNumGroupInfo;

class ScDPUtil
{
public:
    static bool isDuplicateDimension(const OUString& rName);

    SC_DLLPUBLIC static OUString getSourceDimensionName(const OUString& rName);

    static OUString createDuplicateDimensionName(const OUString& rOriginal, size_t nDupCount);

    static OUString getDateGroupName(
        sal_Int32 nDatePart, sal_Int32 nValue, SvNumberFormatter* pFormatter,
        double fStart, double fEnd);

    static double getNumGroupStartValue(double fValue, const ScDPNumGroupInfo& rInfo);

    static OUString getNumGroupName(
        double fValue, const ScDPNumGroupInfo& rInfo, sal_Unicode cDecSep,
        SvNumberFormatter* pFormatter);

    static sal_Int32 getDatePartValue(
        double fValue, const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart,
        SvNumberFormatter* pFormatter);

    static OUString getDisplayedMeasureName(const OUString& rName, ScSubTotalFunc eFunc);

    static ScSubTotalFunc toSubTotalFunc(com::sun::star::sheet::GeneralFunction eGenFunc);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
