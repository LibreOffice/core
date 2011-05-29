/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include <unotools/charclass.hxx>

#include "global.hxx"
#include "userlist.hxx"
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>

using ::rtl::OUString;

namespace {

class FindByName : public ::std::unary_function<ScUserListData::SubStr, bool>
{
    const OUString& mrName;
    bool mbUpper;
public:
    FindByName(const OUString& rName, bool bUpper) : mrName(rName), mbUpper(bUpper) {}
    bool operator() (const ScUserListData::SubStr& r) const
    {
        return mbUpper ? r.maUpper.equals(mrName) : r.maReal.equals(mrName);
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
                String aUpStr = aSub;
                ScGlobal::pCharClass->toUpper(aUpStr);
                maSubStrings.push_back(new SubStr(aSub, aUpStr));
            }
            bFirst = true;
        }
    }

    if (nLen)
    {
        OUString aSub(p0, nLen);
        String aUpStr = aSub;
        ScGlobal::pCharClass->toUpper(aUpStr);
        maSubStrings.push_back(new SubStr(aSub, aUpStr));
    }
}

ScUserListData::ScUserListData(const OUString& rStr) :
    aStr(rStr)
{
    InitTokens();
}

ScUserListData::ScUserListData(const ScUserListData& rData) :
    ScDataObject(),
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

bool ScUserListData::GetSubIndex(const OUString& rSubStr, sal_uInt16& rIndex) const
{
    // First, case sensitive search.
    SubStringsType::const_iterator itr = ::std::find_if(
        maSubStrings.begin(), maSubStrings.end(), FindByName(rSubStr, false));
    if (itr != maSubStrings.end())
    {
        rIndex = ::std::distance(maSubStrings.begin(), itr);
        return true;
    }

    // When that fails, do a case insensitive search.
    String aTmp = rSubStr;
    ScGlobal::pCharClass->toUpper(aTmp);
    OUString aUpStr = aTmp;
    itr = ::std::find_if(
        maSubStrings.begin(), maSubStrings.end(), FindByName(aUpStr, true));
    if (itr != maSubStrings.end())
    {
        rIndex = ::std::distance(maSubStrings.begin(), itr);
        return true;
    }
    return false;
}

OUString ScUserListData::GetSubStr(sal_uInt16 nIndex) const
{
    if (nIndex < maSubStrings.size())
        return maSubStrings[nIndex].maReal;
    else
        return OUString();
}

StringCompare ScUserListData::Compare(const OUString& rSubStr1, const OUString& rSubStr2) const
{
    sal_uInt16 nIndex1, nIndex2;
    bool bFound1 = GetSubIndex(rSubStr1, nIndex1);
    bool bFound2 = GetSubIndex(rSubStr2, nIndex2);
    if (bFound1)
    {
        if (bFound2)
        {
            if (nIndex1 < nIndex2)
                return COMPARE_LESS;
            else if (nIndex1 > nIndex2)
                return COMPARE_GREATER;
            else
                return COMPARE_EQUAL;
        }
        else
            return COMPARE_LESS;
    }
    else if (bFound2)
        return COMPARE_GREATER;
    else
        return (StringCompare) ScGlobal::GetCaseTransliteration()->compareString( rSubStr1, rSubStr2 );
}

StringCompare ScUserListData::ICompare(const OUString& rSubStr1, const OUString& rSubStr2) const
{
    sal_uInt16 nIndex1, nIndex2;
    bool bFound1 = GetSubIndex(rSubStr1, nIndex1);
    bool bFound2 = GetSubIndex(rSubStr2, nIndex2);
    if (bFound1)
    {
        if (bFound2)
        {
            if (nIndex1 < nIndex2)
                return COMPARE_LESS;
            else if (nIndex1 > nIndex2)
                return COMPARE_GREATER;
            else
                return COMPARE_EQUAL;
        }
        else
            return COMPARE_LESS;
    }
    else if (bFound2)
        return COMPARE_GREATER;
    else
        return (StringCompare) ScGlobal::GetpTransliteration()->compareString( rSubStr1, rSubStr2 );
}

ScUserList::ScUserList(sal_uInt16 nLim, sal_uInt16 nDel) :
    ScCollection    ( nLim, nDel )
{
    using namespace ::com::sun::star;

    sal_Unicode cDelimiter = ScGlobal::cListDelimiter;
    uno::Sequence< i18n::CalendarItem > xCal;

    uno::Sequence< i18n::Calendar > xCalendars(
            ScGlobal::pLocaleData->getAllCalendars() );

    for ( sal_Int32 j = 0; j < xCalendars.getLength(); ++j )
    {
        xCal = xCalendars[j].Days;
        if ( xCal.getLength() )
        {
            String sDayShort, sDayLong;
            sal_Int32 i;
            sal_Int32 nLen = xCal.getLength();
            rtl::OUString sStart = xCalendars[j].StartOfWeek;
            sal_Int16 nStart = sal::static_int_cast<sal_Int16>(nLen);
            while (nStart > 0)
            {
                if (xCal[--nStart].ID == sStart)
                    break;
            }
            sal_Int16 nLast = sal::static_int_cast<sal_Int16>( (nStart + nLen - 1) % nLen );
            for (i = nStart; i != nLast; i = (i+1) % nLen)
            {
                sDayShort += String( xCal[i].AbbrevName );
                sDayShort += cDelimiter;
                sDayLong  += String( xCal[i].FullName );
                sDayLong  += cDelimiter;
            }
            sDayShort += String( xCal[i].AbbrevName );
            sDayLong  += String( xCal[i].FullName );

            if ( !HasEntry( sDayShort ) )
                Insert( new ScUserListData( sDayShort ));
            if ( !HasEntry( sDayLong ) )
                Insert( new ScUserListData( sDayLong ));
        }

        xCal = xCalendars[j].Months;
        if ( xCal.getLength() )
        {
            String sMonthShort, sMonthLong;
            sal_Int32 i;
            sal_Int32 nLen = xCal.getLength() - 1;
            for (i = 0; i < nLen; i++)
            {
                sMonthShort += String( xCal[i].AbbrevName );
                sMonthShort += cDelimiter;
                sMonthLong  += String( xCal[i].FullName );
                sMonthLong  += cDelimiter;
            }
            sMonthShort += String( xCal[i].AbbrevName );
            sMonthLong  += String( xCal[i].FullName );

            if ( !HasEntry( sMonthShort ) )
                Insert( new ScUserListData( sMonthShort ));
            if ( !HasEntry( sMonthLong ) )
                Insert( new ScUserListData( sMonthLong ));
        }
    }
}

ScDataObject* ScUserList::Clone() const
{
    return ( new ScUserList( *this ) );
}

ScUserListData* ScUserList::GetData(const OUString& rSubStr) const
{
    sal_uInt16  nIndex;
    sal_uInt16  i = 0;
    for (i=0; i < nCount; i++)
        if (((ScUserListData*)pItems[i])->GetSubIndex(rSubStr, nIndex))
            return (ScUserListData*)pItems[i];
    return NULL;
}

ScUserListData* ScUserList::operator[]( const sal_uInt16 nIndex) const
{
    return (ScUserListData*)At(nIndex);
}

ScUserList& ScUserList::operator=( const ScUserList& r )
{
    return (ScUserList&)ScCollection::operator=( r );
}

bool ScUserList::operator==( const ScUserList& r ) const
{
    bool bEqual = (nCount == r.nCount);

    if ( bEqual )
    {
        ScUserListData* pMyData    = NULL;
        ScUserListData* pOtherData = NULL;

        for ( sal_uInt16 i=0; i<nCount && bEqual; i++)
        {
            pMyData    = (ScUserListData*)At(i);
            pOtherData = (ScUserListData*)r.At(i);

            bEqual = ((pMyData->GetSubCount() == pOtherData->GetSubCount())
                     && (pMyData->GetString() == pOtherData->GetString()) );
        }
    }

    return bEqual;
}

bool ScUserList::operator!=( const ScUserList& r ) const
{
    return !operator==( r );
}


bool ScUserList::HasEntry( const OUString& rStr ) const
{
    for ( sal_uInt16 i=0; i<nCount; i++)
    {
        const ScUserListData* pMyData = (ScUserListData*) At(i);
        if ( pMyData->GetString() == rStr )
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
