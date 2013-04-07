/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef __SC_DPUTIL_HXX__
#define __SC_DPUTIL_HXX__

#include "rtl/ustring.hxx"
#include "scdllapi.h"

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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
