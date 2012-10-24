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

#include "swtypes.hxx"
#include "txttypes.hxx"

#include "SwGrammarMarkUp.hxx"


/*************************************************************************
 * SwWrongList::SwWrongList()
 *************************************************************************/
SwWrongList::SwWrongList( WrongListType eType ) :
    meType       (eType),
    nBeginInvalid(STRING_LEN),  // everything correct... (the invalid area starts beyond the string)
    nEndInvalid  (STRING_LEN)
{
    maList.reserve( 5 );
}

SwWrongList::~SwWrongList()
{
    ClearList();
}

/*************************************************************************
 * SwWrongList* SwWrongList::Clone()
 *************************************************************************/

SwWrongList* SwWrongList::Clone()
{
    SwWrongList* pClone = new SwWrongList( meType );
    pClone->CopyFrom( *this );
    return pClone;
}

/*************************************************************************
 * void SwWrongList::CopyFrom( const SwWrongList& rCopy )
 *************************************************************************/

void SwWrongList::CopyFrom( const SwWrongList& rCopy )
{
    maList = rCopy.maList;
    meType = rCopy.meType;
    nBeginInvalid = rCopy.nBeginInvalid;
    nEndInvalid = rCopy.nEndInvalid;
    for( size_t i = 0; i < maList.size(); ++i )
    {
        if( maList[i].mpSubList )
            maList[i].mpSubList = maList[i].mpSubList->Clone();
    }
}

/*************************************************************************
 * SwWrongList::ClearList()
 *************************************************************************/
void SwWrongList::ClearList()
{
    for ( size_t i = 0; i < maList.size(); ++i)
    {
        if (maList[i].mpSubList)
            delete maList[i].mpSubList;
        maList[i].mpSubList = NULL;
    }
    maList.clear();
}

/*************************************************************************
 * sal_Bool SwWrongList::InWrongWord() gibt den Anfang und die Laenge des
 * Wortes zurueck, wenn es als falsch markiert ist.
 *************************************************************************/
sal_Bool SwWrongList::InWrongWord( xub_StrLen &rChk, xub_StrLen &rLn ) const
{
    MSHORT nPos = GetWrongPos( rChk );
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

/*************************************************************************
 * sal_Bool SwWrongList::Check() liefert den ersten falschen Bereich
 *************************************************************************/
sal_Bool SwWrongList::Check( xub_StrLen &rChk, xub_StrLen &rLn ) const
{
    MSHORT nPos = GetWrongPos( rChk );
    rLn = rLn + rChk;
    xub_StrLen nWrPos;

    if( nPos == Count() )
        return sal_False;

    xub_StrLen nEnd = Len( nPos );
    nEnd = nEnd + ( nWrPos = Pos( nPos ) );
    if( nEnd == rChk )
    {
        ++nPos;
        if( nPos == Count() )
            return sal_False;
        else
        {
            nEnd = Len( nPos );
            nEnd = nEnd + ( nWrPos = Pos( nPos ) );
        }
    }
    if( nEnd > rChk && nWrPos < rLn )
    {
        if( nWrPos > rChk )
            rChk = nWrPos;
        if( nEnd < rLn )
            rLn = nEnd;
        rLn = rLn - rChk;
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
    xub_StrLen nPos = GetWrongPos( nChk );
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

/*************************************************************************
 *                 MSHORT SwWrongList::GetWrongPos( xub_StrLen nValue )
 *  sucht die erste Position im Array, die groessergleich nValue ist,
 * dies kann natuerlich auch hinter dem letzten Element sein!
 *************************************************************************/

MSHORT SwWrongList::GetWrongPos( xub_StrLen nValue ) const
{
    MSHORT nOben = Count(), nUnten = 0;

    if( nOben > 0 )
    {
        // For smart tag lists, we may not use a binary search. We return the
        // position of the first smart tag which coveres nValue
        if ( !maList[0].maType.isEmpty() || maList[0].mpSubList )
        {
            std::vector<SwWrongArea>::const_iterator aIter = maList.begin();
            while ( aIter != maList.end() )
            {
                const xub_StrLen nSTPos = (*aIter).mnPos;
                const xub_StrLen nSTLen = (*aIter).mnLen;
                if ( nSTPos <= nValue && nValue < nSTPos + nSTLen )
                    break;
                else if ( nSTPos > nValue )
                    break;

                ++aIter;
                ++nUnten;
            }
            return nUnten;
        }

        --nOben;
        MSHORT nMitte = 0;
        while( nUnten <= nOben )
        {
            nMitte = nUnten + ( nOben - nUnten ) / 2;
            xub_StrLen nTmp = Pos( nMitte );
            if( nTmp == nValue )
            {
                nUnten = nMitte;
                break;
            }
            else if( nTmp < nValue )
            {
                if( nTmp + Len( nMitte ) >= nValue )
                {
                    nUnten = nMitte;
                    break;
                }
                nUnten = nMitte + 1;
            }
            else if( nMitte == 0 )
            {
                break;
            }
            else
                nOben = nMitte - 1;
        }
    }

    // nUnten now points to an index i into the wrong list which
    // 1. nValue is inside [ Area[i].pos, Area[i].pos + Area[i].len ] (inkl!!!)
    // 2. nValue < Area[i].pos

    return nUnten;
}

/*************************************************************************
 *                 void SwWrongList::_Invalidate()
 *************************************************************************/

void SwWrongList::_Invalidate( xub_StrLen nBegin, xub_StrLen nEnd )
{
    if ( nBegin < GetBeginInv() )
        nBeginInvalid = nBegin;
    if ( nEnd > GetEndInv() )
        nEndInvalid = nEnd;
}

void SwWrongList::SetInvalid( xub_StrLen nBegin, xub_StrLen nEnd )
{
    nBeginInvalid = nBegin;
    nEndInvalid = nEnd;
}


/*************************************************************************
 *                      SwWrongList::Move( xub_StrLen nPos, long nDiff )
 *  veraendert alle Positionen ab nPos um den angegebenen Wert,
 *  wird nach Einfuegen oder Loeschen von Buchstaben benoetigt.
 *************************************************************************/

void SwWrongList::Move( xub_StrLen nPos, long nDiff )
{
    MSHORT i = GetWrongPos( nPos );
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
                nBeginInvalid = nBeginInvalid + xub_StrLen( nDiff );
            if( nEndInvalid >= nPos )
                nEndInvalid = nEndInvalid + xub_StrLen( nDiff );
        }
        // Wenn wir mitten in einem falschen Wort stehen, muss vom Wortanfang
        // invalidiert werden.
        if( i < Count() && nPos >= ( nWrPos = Pos( i ) ) )
        {
            Invalidate( nWrPos, nEnd );
            xub_StrLen nWrLen = Len( i ) + xub_StrLen( nDiff );
            maList[i++].mnLen = nWrLen;
            nWrLen = nWrLen + nWrPos;
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
 * For a given range [nPos, nPos + nLen[ and an index nIndex, this function
 * basically counts the number of SwWrongArea entries starting with nIndex
 * up to nPos + nLen. All these entries are removed.
 *************************************************************************/
sal_Bool SwWrongList::Fresh( xub_StrLen &rStart, xub_StrLen &rEnd, xub_StrLen nPos,
                             xub_StrLen nLen, MSHORT nIndex, xub_StrLen nCursorPos )
{
    // length of word must be greater than 0 and cursor position must be outside the word
    sal_Bool bRet = nLen && ( nCursorPos > nPos + nLen || nCursorPos < nPos );

    xub_StrLen nWrPos = 0;
    xub_StrLen nWrEnd = rEnd;
    MSHORT nCnt = nIndex;
    if( nCnt < Count() && ( nWrPos = Pos( nIndex ) ) < nPos )
    {
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

    nPos = nPos + nLen;

    if( nCnt < Count() && ( nWrPos = Pos( nCnt ) ) < nPos )
    {
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

void SwWrongList::Invalidate( xub_StrLen nBegin, xub_StrLen nEnd )
{
    if (STRING_LEN == GetBeginInv())
        SetInvalid( nBegin, nEnd );
    else
        _Invalidate( nBegin, nEnd );
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
        if( WRONGLIST_GRAMMAR == GetWrongListType() )
            pRet = new SwGrammarMarkUp();
        else
            pRet = new SwWrongList( GetWrongListType() );
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
    if (pNext)
    {
        OSL_ENSURE( GetWrongListType() == pNext->GetWrongListType(), "type mismatch with next list" );
    }
    if( pNext )
    {
        sal_uInt16 nCnt = Count();
        pNext->Move( 0, nInsertPos );
        Insert(nCnt, pNext->maList.begin(), pNext->maList.end());

        Invalidate( pNext->GetBeginInv(), pNext->GetEndInv() );
        if( nCnt && Count() > nCnt )
        {
            xub_StrLen nWrPos = Pos( nCnt );
            xub_StrLen nWrLen = Len( nCnt );
            if( !nWrPos )
            {
                nWrPos = nWrPos + nInsertPos;
                nWrLen = nWrLen - nInsertPos;
                maList[nCnt].mnPos = nWrPos;
                maList[nCnt].mnLen = nWrLen;
            }
            if( nWrPos == Pos( nCnt - 1 ) + Len( nCnt - 1 ) )
            {
                nWrLen = nWrLen + Len( nCnt - 1 );
                maList[nCnt - 1].mnLen = nWrLen;
                Remove( nCnt, 1 );
            }
        }
    }
    Invalidate( nInsertPos ? nInsertPos - 1 : nInsertPos, nInsertPos + 1 );
}


void SwWrongList::InsertSubList( xub_StrLen nNewPos, xub_StrLen nNewLen, sal_uInt16 nWhere, SwWrongList* pSubList )
{
    if (pSubList)
    {
        OSL_ENSURE( GetWrongListType() == pSubList->GetWrongListType(), "type mismatch with sub list" );
    }
    std::vector<SwWrongArea>::iterator i = maList.begin();
    if ( nWhere >= maList.size() )
        i = maList.end(); // robust
    else
        i += nWhere;
    maList.insert(i, SwWrongArea( OUString(), 0, nNewPos, nNewLen, pSubList ) );
}


// New functions: Necessary because SwWrongList has been changed to use std::vector
void SwWrongList::Insert(sal_uInt16 nWhere, std::vector<SwWrongArea>::iterator startPos, std::vector<SwWrongArea>::iterator endPos)
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

void SwWrongList::Remove(sal_uInt16 nIdx, sal_uInt16 nLen )
{
    if ( nIdx >= maList.size() ) return;
    std::vector<SwWrongArea>::iterator i1 = maList.begin();
    i1 += nIdx;

    std::vector<SwWrongArea>::iterator i2 = i1;
    if ( nIdx + nLen >= static_cast<sal_uInt16>(maList.size()) )
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

#if OSL_DEBUG_LEVEL > 0
    const int nOldSize = Count();
    (void) nOldSize;
#endif

    maList.erase(i1, i2);

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( Count() + nLen == nOldSize, "SwWrongList::Remove() trouble" );
#endif
}

void SwWrongList::RemoveEntry( xub_StrLen nBegin, xub_StrLen nEnd ) {
    sal_uInt16 nDelPos = 0;
    sal_uInt16 nDel = 0;
    std::vector<SwWrongArea>::iterator aIter = maList.begin();
    while( aIter != maList.end() && (*aIter).mnPos < nBegin )
    {
        ++aIter;
        ++nDelPos;
    }
    if( WRONGLIST_GRAMMAR == GetWrongListType() )
    {
        while( aIter != maList.end() && nBegin < nEnd && nEnd > (*aIter).mnPos )
        {
            ++aIter;
            ++nDel;
        }
    }
    else
    {
        while( aIter != maList.end() && nBegin == (*aIter).mnPos && nEnd == (*aIter).mnPos +(*aIter).mnLen )
        {
            ++aIter;
            ++nDel;
        }
    }
    if( nDel )
        Remove( nDelPos, nDel );
}

bool SwWrongList::LookForEntry( xub_StrLen nBegin, xub_StrLen nEnd ) {
    std::vector<SwWrongArea>::iterator aIter = maList.begin();
    while( aIter != maList.end() && (*aIter).mnPos < nBegin )
        ++aIter;
    if( aIter != maList.end() && nBegin == (*aIter).mnPos && nEnd == (*aIter).mnPos +(*aIter).mnLen )
        return true;
    return false;
}

void SwWrongList::Insert( const OUString& rType,
                          com::sun::star::uno::Reference< com::sun::star::container::XStringKeyMap > xPropertyBag,
                          xub_StrLen nNewPos, xub_StrLen nNewLen )
{
    std::vector<SwWrongArea>::iterator aIter = maList.begin();

    while ( aIter != maList.end() )
    {
        const xub_StrLen nSTPos = (*aIter).mnPos;

        if ( nNewPos < nSTPos )
        {
            // insert at current position
            break;
        }
        else if ( nNewPos == nSTPos )
        {
            while ( aIter != maList.end() && (*aIter).mnPos == nSTPos )
            {
                const xub_StrLen nSTLen = (*aIter).mnLen;

                if ( nNewLen < nSTLen )
                {
                    // insert at current position
                    break;
                }

                ++aIter;
            }

            break;
        }

        ++aIter;
    }

    maList.insert(aIter, SwWrongArea( rType, xPropertyBag, nNewPos, nNewLen, 0 ) );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
