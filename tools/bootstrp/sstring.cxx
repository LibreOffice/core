/*************************************************************************
 *
 *  $RCSfile: sstring.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:00 $
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

#ifdef WTC
#ifdef _TOOLS_STRINGLIST
#undef _TOOLS_STRINGLIST
#endif
#endif

#define _TOOLS_STRINGLIST
#include "sstring.hxx"

SByteStringList::SByteStringList()
{
}

SByteStringList::~SByteStringList()
{
}

ULONG SByteStringList::IsString( ByteString* pStr )
{
    ULONG nRet = NOT_THERE;
    if ( (nRet = GetPrevString( pStr )) != 0 )
    {
        ByteString* pString = GetObject( nRet );
        if ( *pString == *pStr )
            return nRet;
        else
            return NOT_THERE;
    }
    else
    {
        ByteString* pString = GetObject( 0 );
        if ( pString && (*pString == *pStr) )
            return 0;
        else
            return NOT_THERE;
    }
    return nRet;
}

ULONG SByteStringList::GetPrevString( ByteString* pStr )
{
    ULONG nRet = 0;
    BOOL bFound = FALSE;
    ULONG nCount = Count();
    ULONG nUpper = nCount;
    ULONG nLower = 0;
    ULONG nCurrent = nUpper / 2;
    ULONG nRem = 0;
    ByteString* pString;

    do
    {
        if ( (nCurrent == nLower) || (nCurrent == nUpper) )
            return nLower;
        pString = GetObject( nCurrent );
        ULONG nResult =  pStr->CompareTo( *pString );
        if ( nResult == COMPARE_LESS )
        {
            nUpper = nCurrent;
            nCurrent = (nCurrent + nLower) /2;
        }
        else if ( nResult == COMPARE_GREATER )
        {
            nLower = nCurrent;
            nCurrent = (nUpper + nCurrent) /2;
        }
        else if ( nResult == COMPARE_EQUAL )
            return nCurrent;
        if ( nRem == nCurrent )
            return nCurrent;
        nRem = nCurrent;
    }
    while ( !bFound );
    return nRet;
}

/**************************************************************************
*
*   Sortiert einen ByteString in die Liste ein und gibt die Position,
*   an der einsortiert wurde, zurueck
*
**************************************************************************/

ULONG SByteStringList::PutString( ByteString* pStr )
{
    ULONG nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            ByteString* pString = GetObject( 0 );
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                Insert( pStr, (ULONG)0 );
                return (ULONG)0;
            }
        }
        ByteString* pString = GetObject( nPos );
        if ( *pStr != *pString )
        {
            Insert( pStr, nPos+1 );
            return ( nPos +1 );
        }
    }
    else
    {
        Insert( pStr );
        return (ULONG)0;
    }

    return NOT_THERE;
}

ByteString* SByteStringList::RemoveString( const ByteString& rName )
{
    ULONG i;
    ByteString* pReturn;

    for( i = 0 ; i < Count(); i++ )
    {
        if ( rName == *GetObject( i ) )
        {
            pReturn = GetObject(i);
            Remove(i);
            return pReturn;
        }
    }

    return NULL;
}









SUniStringList::SUniStringList()
{
}

SUniStringList::~SUniStringList()
{
}

ULONG SUniStringList::IsString( UniString* pStr )
{
    ULONG nRet = NOT_THERE;
    if ( (nRet = GetPrevString( pStr )) != 0 )
    {
        UniString* pString = GetObject( nRet );
        if ( *pString == *pStr )
            return nRet;
        else
            return NOT_THERE;
    }
    else
    {
        UniString* pString = GetObject( 0 );
        if ( pString && (*pString == *pStr) )
            return 0;
        else
            return NOT_THERE;
    }
    return nRet;
}

ULONG SUniStringList::GetPrevString( UniString* pStr )
{
    ULONG nRet = 0;
    BOOL bFound = FALSE;
    ULONG nCount = Count();
    ULONG nUpper = nCount;
    ULONG nLower = 0;
    ULONG nCurrent = nUpper / 2;
    ULONG nRem = 0;
    UniString* pString;

    do
    {
        if ( (nCurrent == nLower) || (nCurrent == nUpper) )
            return nLower;
        pString = GetObject( nCurrent );
        ULONG nResult =  pStr->CompareTo( *pString );
        if ( nResult == COMPARE_LESS )
        {
            nUpper = nCurrent;
            nCurrent = (nCurrent + nLower) /2;
        }
        else if ( nResult == COMPARE_GREATER )
        {
            nLower = nCurrent;
            nCurrent = (nUpper + nCurrent) /2;
        }
        else if ( nResult == COMPARE_EQUAL )
            return nCurrent;
        if ( nRem == nCurrent )
            return nCurrent;
        nRem = nCurrent;
    }
    while ( !bFound );
    return nRet;
}

/**************************************************************************
*
*   Sortiert einen UniString in die Liste ein und gibt die Position,
*   an der einsortiert wurde, zurueck
*
**************************************************************************/

ULONG SUniStringList::PutString( UniString* pStr )
{
    ULONG nPos = GetPrevString ( pStr );
    if ( Count() )
    {
        {
            UniString* pString = GetObject( 0 );
            if ( pString->CompareTo( *pStr ) == COMPARE_GREATER )
            {
                Insert( pStr, (ULONG)0);
                return (ULONG)0;
            }
        }
        UniString* pString = GetObject( nPos );
        if ( *pStr != *pString )
        {
            Insert( pStr, nPos+1 );
            return ( nPos +1 );
        }
    }
    else
    {
        Insert( pStr );
        return (ULONG)0;
    }

    return NOT_THERE;
}

UniString* SUniStringList::RemoveString( const UniString& rName )
{
    ULONG i;
    UniString* pReturn;

    for( i = 0 ; i < Count(); i++ )
    {
        if ( rName == *GetObject( i ) )
        {
            pReturn = GetObject(i);
            Remove(i);
            return pReturn;
        }
    }

    return NULL;
}
