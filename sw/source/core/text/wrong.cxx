/*************************************************************************
 *
 *  $RCSfile: wrong.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#include "errhdl.hxx"
#include "swtypes.hxx"
#include "txttypes.hxx"

#include "wrong.hxx"

/*************************************************************************
 * sal_Bool SwWrongList::InWrongWord() gibt den Anfang und die Laenge des Wortes
 * zurueck, wenn es als falsch markiert ist.
 *************************************************************************/
sal_Bool SwWrongList::InWrongWord( xub_StrLen &rChk, xub_StrLen &rLn ) const
{
    MSHORT nPos = GetPos( rChk );
    xub_StrLen nWrPos;
    if( nPos < Count() && ( nWrPos = WRPOS( nPos ) ) <= rChk )
    {
        rLn = WRLEN( nPos );
        if( nWrPos + rLn <= rChk )
            return sal_False;
        rChk = nWrPos;
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 * sal_Bool SwWrongList::Check() liefert den ersten falschen Bereich
 *************************************************************************/
sal_Bool SwWrongList::Check( xub_StrLen &rChk, xub_StrLen &rLn ) const
{
    MSHORT nPos = GetPos( rChk );
    rLn += rChk;
    xub_StrLen nWrPos;

    if( nPos == Count() )
        return sal_False;

    xub_StrLen nEnd = WRLEN( nPos );
    nEnd += ( nWrPos = WRPOS( nPos ) );
    if( nEnd == rChk )
    {
        ++nPos;
        if( nPos == Count() )
            return sal_False;
        else
        {
            nEnd = WRLEN( nPos );
            nEnd += ( nWrPos = WRPOS( nPos ) );
        }
    }
    if( nEnd > rChk && nWrPos < rLn )
    {
        if( nWrPos > rChk )
            rChk = nWrPos;
        if( nEnd < rLn )
            rLn = nEnd;
        rLn -= rChk;
        return 0 != rLn;
    }
    return sal_False;
}

/*************************************************************************
 * xub_StrLen SwWrongList::NextWrong() liefert die naechste Fehlerposition
 *************************************************************************/

xub_StrLen SwWrongList::NextWrong( xub_StrLen nChk ) const
{
    xub_StrLen nRet;
    xub_StrLen nPos = GetPos( nChk );
    if( nPos < Count() )
    {
        nRet = WRPOS( nPos );
        if( nRet < nChk && nRet + WRLEN( nPos ) <= nChk )
        {
            if( ++nPos < Count() )
                nRet = WRPOS( nPos );
            else
                nRet = STRING_LEN;
        }
    }
    else
        nRet = STRING_LEN;
    if( nRet > GetBeginInv() && nChk < GetEndInv() )
        nRet = nChk > GetBeginInv() ? nChk : GetBeginInv();
    return nRet;
}

/*************************************************************************
 * xub_StrLen SwWrongList::LastWrong() liefert die letzte Fehlerposition
 *************************************************************************/

xub_StrLen SwWrongList::LastWrong( xub_StrLen nChk ) const
{
    xub_StrLen nPos = GetPos( nChk );
    xub_StrLen nRet;
    if( nPos >= Count() || ( nRet = WRPOS( nPos ) ) >= nChk )
        nRet = nPos ? WRPOS( --nPos ) : STRING_LEN;
    if( nChk > GetBeginInv() && ( nRet == STRING_LEN || nRet < GetEndInv() ) )
        nRet = nChk > GetEndInv() ? GetEndInv() : nChk;
    return nRet;
}

/*************************************************************************
 *                 MSHORT SwWrongList::GetPos( xub_StrLen nValue )
 *  sucht die erste Position im Array, die groessergleich nValue ist,
 * dies kann natuerlich auch hinter dem letzten Element sein!
 *************************************************************************/

MSHORT SwWrongList::GetPos( xub_StrLen nValue ) const
{
    register MSHORT nOben = Count(), nMitte, nUnten = 0;
    if( nOben > 0 )
    {
        --nOben;
        while( nUnten <= nOben )
        {
            nMitte = nUnten + ( nOben - nUnten ) / 2;
            xub_StrLen nTmp = WRPOS( nMitte );
            if( nTmp == nValue )
                return nMitte;
            else if( nTmp < nValue )
            {
                if( nTmp + WRLEN( nMitte ) >= nValue )
                    return nMitte;
                nUnten = nMitte + 1;
            }
            else if( nMitte == 0 )
                return nUnten;
            else
                nOben = nMitte - 1;
        }
    }
    return nUnten;
}

/*************************************************************************
 *                 void SwWrongList::Invalidate()
 *************************************************************************/

void SwWrongList::_Invalidate( xub_StrLen nBegin, xub_StrLen nEnd )
{
    if ( nBegin < GetBeginInv() )
        nBeginInvalid = nBegin;
    if ( nEnd > GetEndInv() )
        nEndInvalid = nEnd;
}

/*************************************************************************
 *                      SwWrongList::Move( xub_StrLen nPos, long nDiff )
 *  veraendert alle Positionen ab nPos um den angegebenen Wert,
 *  wird nach Einfuegen oder Loeschen von Buchstaben benoetigt.
 *************************************************************************/

void SwWrongList::Move( xub_StrLen nPos, long nDiff )
{
    MSHORT i = GetPos( nPos );
    if( nDiff < 0 )
    {
        xub_StrLen nEnd = nPos + xub_StrLen( -nDiff );
        MSHORT nLst = i;
        xub_StrLen nWrPos;
        xub_StrLen nWrLen;
        sal_Bool bJump = sal_False;
        while( nLst < Count() && WRPOS( nLst ) < nEnd )
            ++nLst;
        if( nLst > i && ( nWrPos = WRPOS( nLst - 1 ) ) <= nPos &&
            nWrPos + ( nWrLen = WRLEN( nLst - 1 ) ) >= nEnd )
        {
            nWrLen += (short)nDiff;
            if( nWrLen )
            {
                GetObject( --nLst ) = nWrPos;
                aLen.GetObject( nLst ) = nWrLen;
                bJump = sal_True;
            }
        }
        Remove( i, nLst - i );
        aLen.Remove( i, nLst - i );
        if ( bJump )
            ++i;
        if( STRING_LEN == GetBeginInv() )
            SetInvalid( nPos ? nPos - 1 : nPos, nPos );
        else
        {
            ShiftLeft( nBeginInvalid, nPos, nEnd );
            ShiftLeft( nEndInvalid, nPos, nEnd );
            _Invalidate( nPos ? nPos - 1 : nPos, nPos );
        }
    }
    else
    {
        xub_StrLen nWrPos;
        xub_StrLen nEnd = nPos + xub_StrLen( nDiff );
        if( STRING_LEN != GetBeginInv() )
        {
            if( nBeginInvalid > nPos )
                nBeginInvalid += xub_StrLen( nDiff );
            if( nEndInvalid >= nPos )
                nEndInvalid += xub_StrLen( nDiff );
        }
        // Wenn wir mitten in einem falschen Wort stehen, muss vom Wortanfang
        // invalidiert werden.
        if( i < Count() && nPos >= ( nWrPos = WRPOS( i ) ) )
        {
            Invalidate( nWrPos, nEnd );
            xub_StrLen nWrLen = WRLEN( i ) + xub_StrLen( nDiff );
            aLen.GetObject( i++ ) = nWrLen;
        }
        else
            Invalidate( nPos, nEnd );
    }
    while( i < Count() )
    {
        xub_StrLen nTmp = nDiff + GetObject( i );
        GetObject( i++ ) = nTmp;
    }
}

/*************************************************************************
 *                      SwWrongList::Clear()/( xub_StrLen nBegin, xub_StrLen nEnd )
 *  loescht das Array im angegebenen Bereich
 *************************************************************************/

void SwWrongList::Clear()
{
    Remove( 0, Count() );
    aLen.Remove( 0, aLen.Count() );
}

void SwWrongList::Clear( xub_StrLen nBegin, xub_StrLen nEnd )
{
    MSHORT nFirst = 0;
    while( nFirst < Count() && WRPOS( nFirst ) < nBegin )
        ++nFirst;
    MSHORT i = nFirst;
    while( i < Count() && WRPOS( i++ ) <= nEnd )
        ;
    Remove( nFirst, i - nFirst );
    aLen.Remove( nFirst, i - nFirst );
}

sal_Bool SwWrongList::Fresh( xub_StrLen &rStart, xub_StrLen &rEnd, xub_StrLen nPos,
                         xub_StrLen nLen, MSHORT nIndex, xub_StrLen nCursorPos )
{
    sal_Bool bRet = nCursorPos > nPos + nLen || nCursorPos < nPos;
    xub_StrLen nWrPos;
    xub_StrLen nWrEnd = rEnd;
    MSHORT nCnt = nIndex;
    if( nIndex < Count() && ( nWrPos = WRPOS( nIndex ) ) < nPos )
    {
        nWrEnd = nWrPos + WRLEN( nCnt++ );
        if( rStart > nWrPos )
            rStart = nWrPos;
    }
    while( nCnt < Count() && ( nWrPos = WRPOS( nCnt ) ) < nPos )
        nWrEnd = nWrPos + WRLEN( nCnt++ );
    if( nCnt < Count() && nWrPos == nPos && WRLEN( nCnt ) == nLen )
    {
        ++nCnt;
        bRet = sal_True;
    }
    else
    {
        if( bRet )
        {
            if( rStart > nPos )
                rStart = nPos;
            nWrEnd = nPos + nLen;
        }
    }
    nPos += nLen;
    if( nCnt < Count() && ( nWrPos = WRPOS( nCnt ) ) < nPos )
    {
        nWrEnd = nWrPos + WRLEN( nCnt++ );
        if( rStart > nWrPos )
            rStart = nWrPos;
    }
    while( nCnt < Count() && ( nWrPos = WRPOS( nCnt ) ) < nPos )
        nWrEnd = nWrPos + WRLEN( nCnt++ );
    if( rEnd < nWrEnd )
        rEnd = nWrEnd;
    Remove( nIndex, nCnt - nIndex );
    aLen.Remove( nIndex, nCnt - nIndex );
    return bRet;
}

sal_Bool SwWrongList::InvalidateWrong( )
{
    if( Count() )
    {
        xub_StrLen nFirst = WRPOS( 0 );
        xub_StrLen nLast = WRPOS( Count() - 1 ) + WRLEN( Count() - 1 );
        Invalidate( nFirst, nLast );
        return sal_True;
    }
    else
        return sal_False;
}

