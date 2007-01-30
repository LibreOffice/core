/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrong.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-30 08:02:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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
    if( nPos < Count() && ( nWrPos = Pos( nPos ) ) <= rChk )
    {
        rLn = Len( nPos );
        if( nWrPos + rLn <= rChk )
            return sal_False;
        rChk = nWrPos;
        return sal_True;
    }
    return sal_False;
}

sal_Bool SwWrongList::InWrongWordSub( xub_StrLen &rChk, xub_StrLen &rLn, xub_StrLen &sub_rChk, xub_StrLen &sub_rLn ) const
{
   MSHORT nPos = GetPos( rChk );
   xub_StrLen nWrPos;
   if( nPos < Count() && ( nWrPos = Pos( nPos ) ) <= rChk )
   {
       rLn = Len( nPos );
       if( nWrPos + rLn <= rChk )
           return sal_False;
       rChk = nWrPos;

       // check subposition
       SwWrongList* pWrongList = SubList(nPos);
       if (pWrongList)
       {
         return pWrongList->InWrongWord(sub_rChk, sub_rLn);
       }

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

    xub_StrLen nEnd = Len( nPos );
    nEnd += ( nWrPos = Pos( nPos ) );
    if( nEnd == rChk )
    {
        ++nPos;
        if( nPos == Count() )
            return sal_False;
        else
        {
            nEnd = Len( nPos );
            nEnd += ( nWrPos = Pos( nPos ) );
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

sal_Bool SwWrongList::CheckSub( xub_StrLen &rChk, xub_StrLen &rLn, xub_StrLen &sub_rChk, xub_StrLen &sub_rLn  ) const
{
    MSHORT nPos = GetPos( rChk );
    rLn += rChk;
    xub_StrLen nWrPos;
    sal_Bool subListResult = sal_False;

    if( nPos == Count() )
        return sal_False;

    xub_StrLen nEnd = Len( nPos );
    nEnd += ( nWrPos = Pos( nPos ) );

    // Check sublist at current position
    SwWrongList *pWrongList = SubList(nPos);
    if (pWrongList)
    {
        subListResult = pWrongList->Check(sub_rChk, sub_rLn);
    }

    if( nEnd == rChk )
    {
        ++nPos;
        if( nPos == Count() )
            return sal_False;
        else
        {
            nEnd = Len( nPos );
            nEnd += ( nWrPos = Pos( nPos ) );

            pWrongList = SubList(nPos);
            if (pWrongList)
            {
                subListResult = pWrongList->Check(sub_rChk, sub_rLn);
            }
        }
    }

    if( nEnd > rChk && nWrPos < rLn )
    {
        if( nWrPos > rChk )
            rChk = nWrPos;
        if( nEnd < rLn )
            rLn = nEnd;
        rLn -= rChk;
        if (pWrongList)
            return subListResult;
        else
            return (0 != rLn);
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
        nRet = Pos( nPos );
        if( nRet < nChk && nRet + Len( nPos ) <= nChk )
        {
            if( ++nPos < Count() )
                nRet = Pos( nPos );
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

void SwWrongList::NextWrongSub( xub_StrLen &nChk, xub_StrLen &nSubChk ) const
{
    xub_StrLen nRet;
    xub_StrLen nSubRet = STRING_LEN;
    xub_StrLen nPos = GetPos( nChk );
    if( nPos < Count() )
    {
        nRet = Pos( nPos );
        if( nRet < nChk && nRet + Len( nPos ) <= nChk )
        {
            if( ++nPos < Count() )
                nRet = Pos( nPos );
            else
                nRet = STRING_LEN;
        }
    }
    else
        nRet = STRING_LEN;

    SwWrongList* pWrongList = SubList(nPos);
    if (pWrongList)
    {
        nSubRet = pWrongList->NextWrong(nSubChk);
    }

    if( nRet > GetBeginInv() && nChk < GetEndInv() )
        nRet = nChk > GetBeginInv() ? nChk : GetBeginInv();

    nChk = nRet;
    nSubChk = nSubRet;
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
            xub_StrLen nTmp = Pos( nMitte );
            if( nTmp == nValue )
                return nMitte;
            else if( nTmp < nValue )
            {
                if( nTmp + Len( nMitte ) >= nValue )
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
        while( nLst < Count() && Pos( nLst ) < nEnd )
            ++nLst;
        if( nLst > i && ( nWrPos = Pos( nLst - 1 ) ) <= nPos )
        {
            nWrLen = Len( nLst - 1 );
            // calculate new length of word
            nWrLen = ( nEnd > nWrPos + nWrLen ) ?
                       nPos - nWrPos :
                       static_cast<xub_StrLen>(nWrLen + nDiff);
            if( nWrLen )
            {
                maList[--nLst].mnLen = nWrLen;
                bJump = sal_True;
            }
        }
        Remove( i, nLst - i );

        if ( bJump )
            ++i;
        if( STRING_LEN == GetBeginInv() )
            SetInvalid( nPos ? nPos - 1 : nPos, nPos + 1 );
        else
        {
            ShiftLeft( nBeginInvalid, nPos, nEnd );
            ShiftLeft( nEndInvalid, nPos, nEnd );
            _Invalidate( nPos ? nPos - 1 : nPos, nPos + 1 );
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
        if( i < Count() && nPos >= ( nWrPos = Pos( i ) ) )
        {
            Invalidate( nWrPos, nEnd );
            xub_StrLen nWrLen = Len( i ) + xub_StrLen( nDiff );
            maList[i++].mnLen = nWrLen;
            nWrLen += nWrPos;
            Invalidate( nWrPos, nWrLen );
        }
        else
            Invalidate( nPos, nEnd );
    }
    while( i < Count() )
    {
        const xub_StrLen nTmp = static_cast<xub_StrLen>(nDiff + maList[i].mnPos);
        maList[i++].mnPos = nTmp;
    }
}

/*************************************************************************
 *                      SwWrongList::Fresh
 *
 * In this method the wrong list is updated, new wrong words are inserted,
 * and by evaluating the postiztion of wrong words, we also know, which
 * words are not wrong any longer and have to be removed.
 * Note: Fresh has to be called at the end of the check of the invalid region,
 * in order to find words, which are behind the last wrong word but not wrong
 * any longer
 *************************************************************************/
sal_Bool SwWrongList::Fresh( xub_StrLen &rStart, xub_StrLen &rEnd, xub_StrLen nPos,
                             xub_StrLen nLen, MSHORT nIndex, xub_StrLen nCursorPos )
{
  // length of word must be greater than 0 and cursor position must be outside the word
    sal_Bool bRet = nLen && ( nCursorPos > nPos + nLen || nCursorPos < nPos );
    xub_StrLen nWrPos;
    xub_StrLen nWrEnd = rEnd;
    MSHORT nCnt = nIndex;
    if( nIndex < Count() && ( nWrPos = Pos( nIndex ) ) < nPos )
    {
        nWrEnd = nWrPos + Len( nCnt++ );
        if( rStart > nWrPos )
            rStart = nWrPos;
    }
    while( nCnt < Count() && ( nWrPos = Pos( nCnt ) ) < nPos )
        nWrEnd = nWrPos + Len( nCnt++ );
    if( nCnt < Count() && nWrPos == nPos && Len( nCnt ) == nLen )
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
    if( nCnt < Count() && ( nWrPos = Pos( nCnt ) ) < nPos )
    {
        nWrEnd = nWrPos + Len( nCnt++ );
        if( rStart > nWrPos )
            rStart = nWrPos;
    }
    while( nCnt < Count() && ( nWrPos = Pos( nCnt ) ) < nPos )
        nWrEnd = nWrPos + Len( nCnt++ );
    if( rEnd < nWrEnd )
        rEnd = nWrEnd;
    Remove( nIndex, nCnt - nIndex );

    return bRet;
}

sal_Bool SwWrongList::InvalidateWrong( )
{
    if( Count() )
    {
        xub_StrLen nFirst = Pos( 0 );
        xub_StrLen nLast = Pos( Count() - 1 ) + Len( Count() - 1 );
        Invalidate( nFirst, nLast );
        return sal_True;
    }
    else
        return sal_False;
}

SwWrongList* SwWrongList::SplitList( xub_StrLen nSplitPos )
{
    SwWrongList *pRet = NULL;
    MSHORT nLst = 0;
    xub_StrLen nWrPos;
    xub_StrLen nWrLen;
    while( nLst < Count() && Pos( nLst ) < nSplitPos )
        ++nLst;
    if( nLst && ( nWrPos = Pos( nLst - 1 ) )
        + ( nWrLen = Len( nLst - 1 ) ) > nSplitPos )
    {
        nWrLen += nWrPos - nSplitPos;
        maList[--nLst].mnPos = nSplitPos;
        maList[nLst].mnLen = nWrLen;
    }
    if( nLst )
    {
        pRet = new SwWrongList;
        pRet->Insert(0, maList.begin(), ( nLst >= maList.size() ? maList.end() : maList.begin() + nLst ) );

        pRet->SetInvalid( GetBeginInv(), GetEndInv() );
        pRet->_Invalidate( nSplitPos ? nSplitPos - 1 : nSplitPos, nSplitPos );
        Remove( 0, nLst );

    }
    if( STRING_LEN == GetBeginInv() )
        SetInvalid( 0, 1 );
    else
    {
        ShiftLeft( nBeginInvalid, 0, nSplitPos );
        ShiftLeft( nEndInvalid, 0, nSplitPos );
        _Invalidate( 0, 1 );
    }
    nLst = 0;
    while( nLst < Count() )
    {
        nWrPos = maList[nLst].mnPos - nSplitPos;
        maList[nLst++].mnPos = nWrPos;
    }
    return pRet;
}

void SwWrongList::JoinList( SwWrongList* pNext, xub_StrLen nInsertPos )
{
    if( pNext )
    {
        USHORT nCnt = Count();
        pNext->Move( 0, nInsertPos );
        Insert(nCnt, pNext->maList.begin(), pNext->maList.end());

        Invalidate( pNext->GetBeginInv(), pNext->GetEndInv() );
        if( nCnt && Count() > nCnt )
        {
            xub_StrLen nWrPos = Pos( nCnt );
            xub_StrLen nWrLen = Len( nCnt );
            if( !nWrPos )
            {
                nWrPos += nInsertPos;
                nWrLen -= nInsertPos;
                maList[nCnt].mnPos = nWrPos;
                maList[nCnt].mnLen = nWrLen;
            }
            if( nWrPos == Pos( nCnt - 1 ) + Len( nCnt - 1 ) )
            {
                nWrLen += Len( nCnt - 1 );
                maList[nCnt - 1].mnLen = nWrLen;
                Remove( nCnt, 1 );

            }
        }
    }
    Invalidate( nInsertPos ? nInsertPos - 1 : nInsertPos, nInsertPos + 1 );
}

// New functions: Necessary because SwWrongList has been changed to use std::vector
void SwWrongList::Insert(USHORT nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator endPos)
{
    std::vector<SwWrongArea>::iterator i = maList.begin();
    if ( nWhere >= maList.size() )
        i = maList.end(); // robust
    else
        i += nWhere;
    maList.insert(i, startPos, endPos); // insert [startPos, endPos[ before i

    // ownership of the sublist is passed to maList, therefore we have to set the
    // pSubList-Pointers to 0
    while ( startPos != endPos )
    {
        (*startPos).mpSubList = 0;
        ++startPos;
    }
}

void SwWrongList::Remove(USHORT nIdx, USHORT nLen )
{
    if ( nIdx >= maList.size() ) return;
    std::vector<SwWrongArea>::iterator i1 = maList.begin();
    i1 += nIdx;

    std::vector<SwWrongArea>::iterator i2 = i1;
    if ( nIdx + nLen >= maList.size() )
        i2 = maList.end(); // robust
    else
        i2 += nLen;

    std::vector<SwWrongArea>::iterator iLoop = i1;
    while ( iLoop != i2 )
    {
        if ( (*iLoop).mpSubList )
            delete (*iLoop).mpSubList;
        ++iLoop;
    }

#if OSL_DEBUG_LEVEL > 1
    const nOldSize = Count();
#endif

    maList.erase(i1, i2);

#if OSL_DEBUG_LEVEL > 1
    ASSERT( Count() + nLen == nOldSize, "SwWrongList::Remove() trouble" )
#endif
}
