/*************************************************************************
 *
 *  $RCSfile: userlist.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 18:10:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <unotools/charclass.hxx>
#include <string.h>

#include "global.hxx"
#include "userlist.hxx"

#ifndef _UNOTOOLS_CALENDARWRAPPER_HXX
#include <unotools/calendarwrapper.hxx>
#endif
#include <unotools/collatorwrapper.hxx>

// STATIC DATA -----------------------------------------------------------


//------------------------------------------------------------------------

void ScUserListData::InitTokens()
{
    sal_Unicode cSep = ScGlobal::cListDelimiter;
    nTokenCount = (USHORT) aStr.GetTokenCount(cSep);
    if (nTokenCount)
    {
        pSubStrings = new String[nTokenCount];
        pUpperSub   = new String[nTokenCount];
        for (USHORT i=0; i<nTokenCount; i++)
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
    aStr(rData.aStr)
{
    InitTokens();
}

__EXPORT ScUserListData::~ScUserListData()
{
    delete[] pSubStrings;
    delete[] pUpperSub;
}

ScUserListData::ScUserListData( SvStream& rStream )
{
    rStream.ReadByteString( aStr, rStream.GetStreamCharSet() );
    InitTokens();
}

BOOL ScUserListData::Store( SvStream& rStream ) const
{
    rStream.WriteByteString( aStr, rStream.GetStreamCharSet() );
    return TRUE;
}

void ScUserListData::SetString( const String& rStr )
{
    delete[] pSubStrings;
    delete[] pUpperSub;

    aStr = rStr;
    InitTokens();
}

USHORT ScUserListData::GetSubCount() const
{
    return nTokenCount;
}

BOOL ScUserListData::GetSubIndex(const String& rSubStr, USHORT& rIndex) const
{
    USHORT i;
    for (i=0; i<nTokenCount; i++)
        if (rSubStr == pSubStrings[i])
        {
            rIndex = i;
            return TRUE;
        }

    String aUpStr = rSubStr;
    ScGlobal::pCharClass->toUpper(aUpStr);
    for (i=0; i<nTokenCount; i++)
        if (aUpStr == pUpperSub[i])
        {
            rIndex = i;
            return TRUE;
        }

    return FALSE;
}

String ScUserListData::GetSubStr(USHORT nIndex) const
{
    if (nIndex < nTokenCount)
        return pSubStrings[nIndex];
    else
        return EMPTY_STRING;
}

StringCompare ScUserListData::Compare(const String& rSubStr1, const String& rSubStr2) const
{
    USHORT nIndex1;
    USHORT nIndex2;
    BOOL bFound1 = GetSubIndex(rSubStr1, nIndex1);
    BOOL bFound2 = GetSubIndex(rSubStr2, nIndex2);
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
        return (StringCompare) ScGlobal::pCaseCollator->compareString( rSubStr1, rSubStr2 );
}

StringCompare ScUserListData::ICompare(const String& rSubStr1, const String& rSubStr2) const
{
    USHORT nIndex1;
    USHORT nIndex2;
    BOOL bFound1 = GetSubIndex(rSubStr1, nIndex1);
    BOOL bFound2 = GetSubIndex(rSubStr2, nIndex2);
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
        return (StringCompare) ScGlobal::pCollator->compareString( rSubStr1, rSubStr2 );
}

ScUserList::ScUserList(USHORT nLim, USHORT nDel) :
    Collection  ( nLim, nDel )
{
    String  sMonthShort, sMonthLong, sDayShort, sDayLong;
    sal_Unicode cDelimiter = ScGlobal::cListDelimiter;
    sal_Int32 i, nCount;

    ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::CalendarItem >
        xCal = ScGlobal::pCalendar->getMonths();

    if ( xCal.getLength() )
    {
        nCount = xCal.getLength() - 1;
        for (i = 0; i < nCount; i++)
        {
            sMonthShort += String( xCal[i].AbbrevName );
            sMonthShort += cDelimiter;
            sMonthLong  += String( xCal[i].FullName );
            sMonthLong  += cDelimiter;
        }
        sMonthShort += String( xCal[i].AbbrevName );
        sMonthLong  += String( xCal[i].FullName );
    }

    xCal = ScGlobal::pCalendar->getDays();

    if ( xCal.getLength() )
    {
        nCount = xCal.getLength() - 1;
        for (i = 0; i < nCount; i++)
        {
            sDayShort += String( xCal[i].AbbrevName );
            sDayShort += cDelimiter;
            sDayLong  += String( xCal[i].FullName );
            sDayLong  += cDelimiter;
        }
        sDayShort += String( xCal[i].AbbrevName );
        sDayLong  += String( xCal[i].FullName );
    }

    Insert( new ScUserListData( sDayShort ));
    Insert( new ScUserListData( sDayLong ));
    Insert( new ScUserListData( sMonthShort ));
    Insert( new ScUserListData( sMonthLong ));
}

BOOL ScUserList::Load( SvStream& rStream )
{
    BOOL    bSuccess = TRUE;
    USHORT  nNewCount;

    while( nCount > 0 )
        AtFree(0);                  // alles loeschen

    rStream >> nNewCount;

    for ( USHORT i=0; i<nNewCount && bSuccess; i++ )
        Insert( new ScUserListData( rStream ) );

    return bSuccess;
}

BOOL ScUserList::Store( SvStream& rStream ) const
{
    BOOL bSuccess = TRUE;

    rStream << nCount;

    for ( USHORT i=0; i<nCount && bSuccess; i++ )
        bSuccess = ((const ScUserListData*)At(i))->Store( rStream );

    return bSuccess;
}

DataObject* ScUserList::Clone() const
{
    return ( new ScUserList( *this ) );
}

ScUserListData* ScUserList::GetData(const String& rSubStr) const
{
    USHORT  nIndex;
    USHORT  i = 0;
    for (i=0; i < nCount; i++)
        if (((ScUserListData*)pItems[i])->GetSubIndex(rSubStr, nIndex))
            return (ScUserListData*)pItems[i];
    return NULL;
}

BOOL ScUserList::operator==( const ScUserList& r ) const
{
    BOOL bEqual = (nCount == r.nCount);

    if ( bEqual )
    {
        ScUserListData* pMyData    = NULL;
        ScUserListData* pOtherData = NULL;

        for ( USHORT i=0; i<nCount && bEqual; i++)
        {
            pMyData    = (ScUserListData*)At(i);
            pOtherData = (ScUserListData*)r.At(i);

            bEqual =(   (pMyData->nTokenCount == pOtherData->nTokenCount)
                     && (pMyData->aStr        == pOtherData->aStr) );
        }
    }

    return bEqual;
}




