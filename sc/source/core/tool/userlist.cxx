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

#include <memory>
#include <unotools/charclass.hxx>

#include <global.hxx>
#include <userlist.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>

#include <algorithm>

namespace {

class FindByName
{
    const OUString& mrName;
    bool const mbUpper;
public:
    FindByName(const OUString& rName, bool bUpper) : mrName(rName), mbUpper(bUpper) {}
    bool operator() (const ScUserListData::SubStr& r) const
    {
        return mbUpper ? r.maUpper == mrName : r.maReal == mrName;
    }
};

}

ScUserListData::SubStr::SubStr(const OUString& rReal, const OUString& rUpper) :
    maReal(rReal), maUpper(rUpper) {}

void ScUserListData::InitTokens()
{
    sal_Unicode cSep = ScGlobal::cListDelimiter;
    maSubStrings.clear();
    const sal_Unicode* p = aStr.getStr();
    const sal_Unicode* p0 = p;
    sal_Int32 nLen = 0;
    bool bFirst = true;
    for (sal_Int32 i = 0, n = aStr.getLength(); i < n; ++i, ++p, ++nLen)
    {
        if (bFirst)
        {
            // very first character, or the first character after a separator.
            p0 = p;
            nLen = 0;
            bFirst = false;
        }
        if (*p == cSep)
        {
            if (nLen)
            {
                OUString aSub(p0, nLen);
                OUString aUpStr = ScGlobal::pCharClass->uppercase(aSub);
                maSubStrings.emplace_back(aSub, aUpStr);
            }
            bFirst = true;
        }
    }

    if (nLen)
    {
        OUString aSub(p0, nLen);
        OUString aUpStr = ScGlobal::pCharClass->uppercase(aSub);
        maSubStrings.emplace_back(aSub, aUpStr);
    }
}

ScUserListData::ScUserListData(const OUString& rStr) :
    aStr(rStr)
{
    InitTokens();
}

ScUserListData::ScUserListData(const ScUserListData& rData) :
    aStr(rData.aStr)
{
    InitTokens();
}

ScUserListData::~ScUserListData()
{
}

void ScUserListData::SetString( const OUString& rStr )
{
    aStr = rStr;
    InitTokens();
}

size_t ScUserListData::GetSubCount() const
{
    return maSubStrings.size();
}

bool ScUserListData::GetSubIndex(const OUString& rSubStr, sal_uInt16& rIndex, bool& bMatchCase) const
{
    // First, case sensitive search.
    SubStringsType::const_iterator itr = ::std::find_if(
        maSubStrings.begin(), maSubStrings.end(), FindByName(rSubStr, false));
    if (itr != maSubStrings.end())
    {
        rIndex = ::std::distance(maSubStrings.begin(), itr);
        bMatchCase = true;
        return true;
    }

    // When that fails, do a case insensitive search.
    OUString aUpStr = ScGlobal::pCharClass->uppercase(rSubStr);
    itr = ::std::find_if(
        maSubStrings.begin(), maSubStrings.end(), FindByName(aUpStr, true));
    if (itr != maSubStrings.end())
    {
        rIndex = ::std::distance(maSubStrings.begin(), itr);
        bMatchCase = false;
        return true;
    }
    bMatchCase = false;
    return false;
}

OUString ScUserListData::GetSubStr(sal_uInt16 nIndex) const
{
    if (nIndex < maSubStrings.size())
        return maSubStrings[nIndex].maReal;
    else
        return OUString();
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
        return ScGlobal::GetCaseTransliteration()->compareString( rSubStr1, rSubStr2 );
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
        return ScGlobal::GetpTransliteration()->compareString( rSubStr1, rSubStr2 );
}

ScUserList::ScUserList()
{
    using namespace ::com::sun::star;

    sal_Unicode cDelimiter = ScGlobal::cListDelimiter;
    uno::Sequence< i18n::CalendarItem2 > xCal;

    uno::Sequence< i18n::Calendar2 > xCalendars(
            ScGlobal::pLocaleData->getAllCalendars() );

    for ( sal_Int32 j = 0; j < xCalendars.getLength(); ++j )
    {
        xCal = xCalendars[j].Days;
        if ( xCal.getLength() )
        {
            OUStringBuffer aDayShortBuf, aDayLongBuf;
            sal_Int32 i;
            sal_Int32 nLen = xCal.getLength();
            sal_Int16 nStart = sal::static_int_cast<sal_Int16>(nLen);
            while (nStart > 0)
            {
                if (xCal[--nStart].ID == xCalendars[j].StartOfWeek)
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
                maData.push_back( std::make_unique<ScUserListData>( aDayShort ));
            if ( !HasEntry( aDayLong ) )
                maData.push_back( std::make_unique<ScUserListData>( aDayLong ));
        }

        xCal = xCalendars[j].Months;
        if ( xCal.getLength() )
        {
            OUStringBuffer aMonthShortBuf, aMonthLongBuf;
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
                maData.push_back( std::make_unique<ScUserListData>( aMonthShort ));
            if ( !HasEntry( aMonthLong ) )
                maData.push_back( std::make_unique<ScUserListData>( aMonthLong ));
        }
    }
}

ScUserList::ScUserList(const ScUserList& rOther)
{
    for (const std::unique_ptr<ScUserListData>& rData : rOther.maData)
        maData.push_back(std::make_unique<ScUserListData>(*rData));
}

const ScUserListData* ScUserList::GetData(const OUString& rSubStr) const
{
    const ScUserListData* pFirstCaseInsensitive = nullptr;
    sal_uInt16 nIndex;
    bool bMatchCase = false;

    for (const auto& rxItem : maData)
    {
        if (rxItem->GetSubIndex(rSubStr, nIndex, bMatchCase))
        {
            if (bMatchCase)
                return rxItem.get();
            if (!pFirstCaseInsensitive)
                pFirstCaseInsensitive = rxItem.get();
        }
    }

    return pFirstCaseInsensitive;
}

const ScUserListData& ScUserList::operator[](size_t nIndex) const
{
    return *maData[nIndex].get();
}

ScUserListData& ScUserList::operator[](size_t nIndex)
{
    return *maData[nIndex].get();
}

ScUserList& ScUserList::operator=( const ScUserList& rOther )
{
    maData.clear();
    for (const std::unique_ptr<ScUserListData>& rData : rOther.maData)
        maData.push_back(std::make_unique<ScUserListData>(*rData));
    return *this;
}

bool ScUserList::operator==( const ScUserList& r ) const
{
    return std::equal(maData.begin(), maData.end(), r.maData.begin(), r.maData.end(),
        [](const std::unique_ptr<ScUserListData>& lhs, const std::unique_ptr<ScUserListData>& rhs) {
            return (lhs->GetString() == rhs->GetString()) && (lhs->GetSubCount() == rhs->GetSubCount());
        });
}

bool ScUserList::operator!=( const ScUserList& r ) const
{
    return !operator==( r );
}

bool ScUserList::HasEntry( const OUString& rStr ) const
{
    return ::std::any_of(maData.begin(), maData.end(),
        [&] (std::unique_ptr<ScUserListData> const& pData)
            { return pData->GetString() == rStr; } );
}

ScUserList::iterator ScUserList::begin()
{
    return maData.begin();
}

void ScUserList::clear()
{
    maData.clear();
}

size_t ScUserList::size() const
{
    return maData.size();
}

void ScUserList::push_back(ScUserListData* p)
{
    maData.push_back(std::unique_ptr<ScUserListData>(p));
}

void ScUserList::erase(const iterator& itr)
{
    maData.erase(itr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
