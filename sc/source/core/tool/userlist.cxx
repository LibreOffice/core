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

#include <sal/config.h>

#include <unotools/charclass.hxx>

#include <global.hxx>
#include <userlist.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <com/sun/star/i18n/Calendar2.hpp>

#include <algorithm>
#include <utility>

ScUserListData::SubStr::SubStr(OUString&& aReal) :
    maReal(std::move(aReal)), maUpper(ScGlobal::getCharClass().uppercase(maReal)) {}

void ScUserListData::InitTokens()
{
    maSubStrings.clear();
    sal_Int32 nIndex = 0;
    do
    {
        OUString aSub = aStr.getToken(0, ScGlobal::cListDelimiter, nIndex);
        if (!aSub.isEmpty())
            maSubStrings.emplace_back(std::move(aSub));
    } while (nIndex >= 0);
}

ScUserListData::ScUserListData(OUString _aStr) :
    aStr(std::move(_aStr))
{
    InitTokens();
}

void ScUserListData::SetString( const OUString& rStr )
{
    aStr = rStr;
    InitTokens();
}

bool ScUserListData::GetSubIndex(const OUString& rSubStr, sal_uInt16& rIndex, bool& bMatchCase) const
{
    // First, case sensitive search.
    auto itr = ::std::find_if(maSubStrings.begin(), maSubStrings.end(),
                              [&rSubStr](const SubStr& item) { return item.maReal == rSubStr; });
    if (itr != maSubStrings.end())
    {
        rIndex = ::std::distance(maSubStrings.begin(), itr);
        bMatchCase = true;
        return true;
    }

    // When that fails, do a case insensitive search.
    bMatchCase = false;
    OUString aUpStr = ScGlobal::getCharClass().uppercase(rSubStr);
    itr = ::std::find_if(maSubStrings.begin(), maSubStrings.end(),
                         [&aUpStr](const SubStr& item) { return item.maUpper == aUpStr; });
    if (itr != maSubStrings.end())
    {
        rIndex = ::std::distance(maSubStrings.begin(), itr);
        return true;
    }
    return false;
}

const OUString & ScUserListData::GetSubStr(sal_uInt16 nIndex) const
{
    if (nIndex < maSubStrings.size())
        return maSubStrings[nIndex].maReal;
    else
        return EMPTY_OUSTRING;
}

sal_Int32 ScUserListData::Compare(const OUString& rSubStr1, const OUString& rSubStr2) const
{
    sal_uInt16 nIndex1, nIndex2;
    bool bMatchCase;
    bool bFound1 = GetSubIndex(rSubStr1, nIndex1, bMatchCase);
    bool bFound2 = GetSubIndex(rSubStr2, nIndex2, bMatchCase);
    if (bFound1)
    {
        if (bFound2)
        {
            if (nIndex1 < nIndex2)
                return -1;
            else if (nIndex1 > nIndex2)
                return 1;
            else
                return 0;
        }
        else
            return -1;
    }
    else if (bFound2)
        return 1;
    else
        return ScGlobal::GetCaseTransliteration().compareString( rSubStr1, rSubStr2 );
}

sal_Int32 ScUserListData::ICompare(const OUString& rSubStr1, const OUString& rSubStr2) const
{
    sal_uInt16 nIndex1, nIndex2;
    bool bMatchCase;
    bool bFound1 = GetSubIndex(rSubStr1, nIndex1, bMatchCase);
    bool bFound2 = GetSubIndex(rSubStr2, nIndex2, bMatchCase);
    if (bFound1)
    {
        if (bFound2)
        {
            if (nIndex1 < nIndex2)
                return -1;
            else if (nIndex1 > nIndex2)
                return 1;
            else
                return 0;
        }
        else
            return -1;
    }
    else if (bFound2)
        return 1;
    else
        return ScGlobal::GetTransliteration().compareString( rSubStr1, rSubStr2 );
}

ScUserList::ScUserList(bool initDefault)
{
    if (initDefault)
        AddDefaults();
}

void ScUserList::AddDefaults()
{
    sal_Unicode cDelimiter = ScGlobal::cListDelimiter;
    for (const auto& rCalendar : ScGlobal::getLocaleData().getAllCalendars())
    {
        if (const auto& xCal = rCalendar.Days; xCal.hasElements())
        {
            OUStringBuffer aDayShortBuf(32), aDayLongBuf(64);
            sal_Int32 i;
            sal_Int32 nLen = xCal.getLength();
            sal_Int16 nStart = sal::static_int_cast<sal_Int16>(nLen);
            while (nStart > 0)
            {
                if (xCal[--nStart].ID == rCalendar.StartOfWeek)
                    break;
            }
            sal_Int16 nLast = sal::static_int_cast<sal_Int16>( (nStart + nLen - 1) % nLen );
            for (i = nStart; i != nLast; i = (i+1) % nLen)
            {
                aDayShortBuf.append(xCal[i].AbbrevName);
                aDayShortBuf.append(cDelimiter);
                aDayLongBuf.append(xCal[i].FullName);
                aDayLongBuf.append(cDelimiter);
            }
            aDayShortBuf.append(xCal[i].AbbrevName);
            aDayLongBuf.append(xCal[i].FullName);

            OUString aDayShort = aDayShortBuf.makeStringAndClear();
            OUString aDayLong = aDayLongBuf.makeStringAndClear();

            if ( !HasEntry( aDayShort ) )
                emplace_back(aDayShort);
            if ( !HasEntry( aDayLong ) )
                emplace_back(aDayLong);
        }

        if (const auto& xCal = rCalendar.Months; xCal.hasElements())
        {
            OUStringBuffer aMonthShortBuf(128), aMonthLongBuf(128);
            sal_Int32 i;
            sal_Int32 nLen = xCal.getLength() - 1;
            for (i = 0; i < nLen; i++)
            {
                aMonthShortBuf.append(xCal[i].AbbrevName);
                aMonthShortBuf.append(cDelimiter);
                aMonthLongBuf.append(xCal[i].FullName);
                aMonthLongBuf.append(cDelimiter);
            }
            aMonthShortBuf.append(xCal[i].AbbrevName);
            aMonthLongBuf.append(xCal[i].FullName);

            OUString aMonthShort = aMonthShortBuf.makeStringAndClear();
            OUString aMonthLong = aMonthLongBuf.makeStringAndClear();

            if ( !HasEntry( aMonthShort ) )
                emplace_back(aMonthShort);
            if ( !HasEntry( aMonthLong ) )
                emplace_back(aMonthLong);
        }
    }
}

const ScUserListData* ScUserList::GetData(const OUString& rSubStr) const
{
    const ScUserListData* pFirstCaseInsensitive = nullptr;
    sal_uInt16 nIndex;
    bool bMatchCase = false;

    for (const auto& rxItem : maData)
    {
        if (rxItem.GetSubIndex(rSubStr, nIndex, bMatchCase))
        {
            if (bMatchCase)
                return &rxItem;
            if (!pFirstCaseInsensitive)
                pFirstCaseInsensitive = &rxItem;
        }
    }

    return pFirstCaseInsensitive;
}

bool ScUserList::operator==( const ScUserList& r ) const
{
    return std::equal(maData.begin(), maData.end(), r.maData.begin(), r.maData.end(),
        [](const ScUserListData& lhs, const ScUserListData& rhs) {
            return lhs.GetString() == rhs.GetString();
        });
}

bool ScUserList::HasEntry( std::u16string_view rStr ) const
{
    return ::std::any_of(maData.begin(), maData.end(),
        [&] (ScUserListData const& pData)
            { return pData.GetString() == rStr; } );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
