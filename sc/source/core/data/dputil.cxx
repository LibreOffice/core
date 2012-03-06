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

#include "dputil.hxx"
#include "global.hxx"
#include "dpitemdata.hxx"

#include "comphelper/string.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/calendarwrapper.hxx"
#include "svl/zforlist.hxx"

#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>
#include <com/sun/star/i18n/CalendarDisplayIndex.hpp>

using namespace com::sun::star;

namespace {

const sal_uInt16 SC_DP_LEAPYEAR = 1648;     // arbitrary leap year for date calculations

rtl::OUString getTwoDigitString(sal_Int32 nValue)
{
    String aRet = String::CreateFromInt32( nValue );
    if ( aRet.Len() < 2 )
        aRet.Insert( (sal_Unicode)'0', 0 );
    return aRet;
}

void appendDateStr(rtl::OUStringBuffer& rBuffer, double fValue, SvNumberFormatter* pFormatter)
{
    sal_uLong nFormat = pFormatter->GetStandardFormat( NUMBERFORMAT_DATE, ScGlobal::eLnge );
    rtl::OUString aString;
    pFormatter->GetInputLineString(fValue, nFormat, aString);
    rBuffer.append(aString);
}

rtl::OUString getSpecialDateName(double fValue, bool bFirst, SvNumberFormatter* pFormatter)
{
    rtl::OUStringBuffer aBuffer;
    aBuffer.append(sal_Unicode(bFirst ? '<' : '>'));
    appendDateStr(aBuffer, fValue, pFormatter);
    return aBuffer.makeStringAndClear();
}

}

bool ScDPUtil::isDuplicateDimension(const rtl::OUString& rName)
{
    if (rName.isEmpty())
        return false;

    sal_Unicode cLast = rName[rName.getLength()-1];
    return cLast == sal_Unicode('*');
}

rtl::OUString ScDPUtil::getSourceDimensionName(const rtl::OUString& rName)
{
    return comphelper::string::removeTrailingChars(rName, sal_Unicode('*'));
}

rtl::OUString ScDPUtil::createDuplicateDimensionName(const rtl::OUString& rOriginal, size_t nDupCount)
{
    if (!nDupCount)
        return rOriginal;

    rtl::OUStringBuffer aBuf(rOriginal);
    for (size_t i = 0; i < nDupCount; ++i)
        aBuf.append(sal_Unicode('*'));

    return aBuf.makeStringAndClear();
}

rtl::OUString ScDPUtil::getDateGroupName(
        sal_Int32 nDatePart, sal_Int32 nValue, SvNumberFormatter* pFormatter,
        double fStart, double fEnd)
{
    if (nValue == ScDPItemData::DateFirst)
        return getSpecialDateName(fStart, true, pFormatter);
    if (nValue == ScDPItemData::DateLast)
        return getSpecialDateName(fEnd, false, pFormatter);

    switch ( nDatePart )
    {
        case sheet::DataPilotFieldGroupBy::YEARS:
            return rtl::OUString::valueOf(nValue);
        case sheet::DataPilotFieldGroupBy::QUARTERS:
            return ScGlobal::pLocaleData->getQuarterAbbreviation(sal_Int16(nValue-1));    // nValue is 1-based
        case com::sun::star::sheet::DataPilotFieldGroupBy::MONTHS:
            return ScGlobal::GetCalendar()->getDisplayName(
                        i18n::CalendarDisplayIndex::MONTH, sal_Int16(nValue-1), 0);    // 0-based, get short name
        case sheet::DataPilotFieldGroupBy::DAYS:
        {
            Date aDate(1, 1, SC_DP_LEAPYEAR);
            aDate += (nValue - 1);            // nValue is 1-based
            Date aNullDate = *pFormatter->GetNullDate();
            long nDays = aDate - aNullDate;

            sal_uLong nFormat = pFormatter->GetFormatIndex(NF_DATE_SYS_DDMMM, ScGlobal::eLnge);
            Color* pColor;
            String aStr;
            pFormatter->GetOutputString(nDays, nFormat, aStr, &pColor);
            return aStr;
        }
        case sheet::DataPilotFieldGroupBy::HOURS:
        {
            //! allow am/pm format?
            return getTwoDigitString(nValue);
        }
        break;
        case sheet::DataPilotFieldGroupBy::MINUTES:
        case sheet::DataPilotFieldGroupBy::SECONDS:
        {
            rtl::OUStringBuffer aBuf(ScGlobal::pLocaleData->getTimeSep());
            aBuf.append(getTwoDigitString(nValue));
            return aBuf.makeStringAndClear();
        }
        break;
        default:
            OSL_FAIL("invalid date part");
    }

    return rtl::OUString::createFromAscii("FIXME: unhandled value");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
