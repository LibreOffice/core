/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include <unotools/charclass.hxx>
#include <string.h>

#include "global.hxx"
#include "userlist.hxx"
#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>

// STATIC DATA -----------------------------------------------------------


//------------------------------------------------------------------------

void ScUserListData::InitTokens()
{
    sal_Unicode cSep = ScGlobal::cListDelimiter;
    nTokenCount = (sal_uInt16) aStr.GetTokenCount(cSep);
    if (nTokenCount)
    {
        pSubStrings = new String[nTokenCount];
        pUpperSub   = new String[nTokenCount];
        for (sal_uInt16 i=0; i<nTokenCount; i++)
        {
            pUpperSub[i] = pSubStrings[i] = aStr.GetToken((xub_StrLen)i,cSep);
            ScGlobal::pCharClass->toUpper(pUpperSub[i]);
        }
    }
    else
        pSubStrings = pUpperSub = NULL;
}

ScUserListData::ScUserListData(const String& rStr) :
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

__EXPORT ScUserListData::~ScUserListData()
{
    delete[] pSubStrings;
    delete[] pUpperSub;
}

void ScUserListData::SetString( const String& rStr )
{
    delete[] pSubStrings;
    delete[] pUpperSub;

    aStr = rStr;
    InitTokens();
}

sal_uInt16 ScUserListData::GetSubCount() const
{
    return nTokenCount;
}

sal_Bool ScUserListData::GetSubIndex(const String& rSubStr, sal_uInt16& rIndex) const
{
    sal_uInt16 i;
    for (i=0; i<nTokenCount; i++)
        if (rSubStr == pSubStrings[i])
        {
            rIndex = i;
            return sal_True;
        }

    String aUpStr = rSubStr;
    ScGlobal::pCharClass->toUpper(aUpStr);
    for (i=0; i<nTokenCount; i++)
        if (aUpStr == pUpperSub[i])
        {
            rIndex = i;
            return sal_True;
        }

    return sal_False;
}

String ScUserListData::GetSubStr(sal_uInt16 nIndex) const
{
    if (nIndex < nTokenCount)
        return pSubStrings[nIndex];
    else
        return EMPTY_STRING;
}

StringCompare ScUserListData::Compare(const String& rSubStr1, const String& rSubStr2) const
{
    sal_uInt16 nIndex1;
    sal_uInt16 nIndex2;
    sal_Bool bFound1 = GetSubIndex(rSubStr1, nIndex1);
    sal_Bool bFound2 = GetSubIndex(rSubStr2, nIndex2);
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

StringCompare ScUserListData::ICompare(const String& rSubStr1, const String& rSubStr2) const
{
    sal_uInt16 nIndex1;
    sal_uInt16 nIndex2;
    sal_Bool bFound1 = GetSubIndex(rSubStr1, nIndex1);
    sal_Bool bFound2 = GetSubIndex(rSubStr2, nIndex2);
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

ScUserListData* ScUserList::GetData(const String& rSubStr) const
{
    sal_uInt16  nIndex;
    sal_uInt16  i = 0;
    for (i=0; i < nCount; i++)
        if (((ScUserListData*)pItems[i])->GetSubIndex(rSubStr, nIndex))
            return (ScUserListData*)pItems[i];
    return NULL;
}

sal_Bool ScUserList::operator==( const ScUserList& r ) const
{
    sal_Bool bEqual = (nCount == r.nCount);

    if ( bEqual )
    {
        ScUserListData* pMyData    = NULL;
        ScUserListData* pOtherData = NULL;

        for ( sal_uInt16 i=0; i<nCount && bEqual; i++)
        {
            pMyData    = (ScUserListData*)At(i);
            pOtherData = (ScUserListData*)r.At(i);

            bEqual =(   (pMyData->nTokenCount == pOtherData->nTokenCount)
                     && (pMyData->aStr        == pOtherData->aStr) );
        }
    }

    return bEqual;
}


sal_Bool ScUserList::HasEntry( const String& rStr ) const
{
    for ( sal_uInt16 i=0; i<nCount; i++)
    {
        const ScUserListData* pMyData = (ScUserListData*) At(i);
        if ( pMyData->aStr == rStr )
            return sal_True;
    }
    return sal_False;
}

