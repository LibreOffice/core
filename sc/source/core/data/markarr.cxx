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

#include "markarr.hxx"
#include "global.hxx"
#include "address.hxx"

#include <osl/diagnose.h>

ScMarkArray::ScMarkArray() :
    m_data( 1 )
{
    // set top "no marks"
    clear();
}

// Move constructor
ScMarkArray::ScMarkArray( ScMarkArray&& rArray ) :
    m_data( std::move( rArray.m_data ) )
{
}

ScMarkArray::~ScMarkArray()
{
}

void ScMarkArray::clear()
{
    m_data.resize( 1 );
    m_data[0].nRow = MAXROW;
    m_data[0].bMarked = false;
}

bool ScMarkArray::Search( SCROW nRow, SCSIZE& nIndex ) const
{
    long    nHi         = static_cast<long>(m_data.size()) - 1;
    long    i           = 0;
    bool    bFound      = (m_data.size() == 1);
    long    nLo         = 0;
    long    nStartRow   = 0;
    while ( !bFound && nLo <= nHi )
    {
        i = (nLo + nHi) / 2;
        if (i > 0)
            nStartRow = (long) m_data[i - 1].nRow;
        else
            nStartRow = -1;
        long nEndRow = (long) m_data[i].nRow;
        if (nEndRow < (long) nRow)
            nLo = ++i;
        else
            if (nStartRow >= (long) nRow)
                nHi = --i;
            else
                bFound = true;
    }

    if (bFound)
        nIndex=(SCSIZE)i;
    else
        nIndex=0;
    return bFound;
}

bool ScMarkArray::GetMark( SCROW nRow ) const
{
    SCSIZE i;
    if (Search( nRow, i ))
        return m_data[i].bMarked;
    else
        return false;

}

void ScMarkArray::SetMarkArea( SCROW nStartRow, SCROW nEndRow, bool bMarked )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow))
    {
        if ((nStartRow == 0) && (nEndRow == MAXROW))
        {
            clear();
            m_data[0].bMarked = bMarked;
        }
        else
        {
            SCSIZE ni = 0;          // number of entries in beginning
            SCSIZE nInsert = 0;     // insert position (MAXROW+1 := no insert)
            bool bCombined = false;
            bool bSplit = false;
            if ( nStartRow > 0 )
            {
                // skip beginning
                SCSIZE nIndex;
                Search( nStartRow, nIndex );
                ni = nIndex;

                nInsert = MAXROWCOUNT;
                if ( m_data[ni].bMarked != bMarked )
                {
                    if ( ni == 0 || (m_data[ni-1].nRow < nStartRow - 1) )
                    {   // may be a split or a simple insert or just a shrink,
                        // row adjustment is done further down
                        if ( m_data[ni].nRow > nEndRow )
                            bSplit = true;
                        ni++;
                        nInsert = ni;
                    }
                    else if ( ni > 0 && m_data[ni-1].nRow == nStartRow - 1 )
                        nInsert = ni;
                }
                if ( ni > 0 && m_data[ni-1].bMarked == bMarked )
                {   // combine
                    m_data[ni-1].nRow = nEndRow;
                    nInsert = MAXROWCOUNT;
                    bCombined = true;
                }
            }

            SCSIZE nj = ni;     // stop position of range to replace
            while ( nj < m_data.size() && m_data[nj].nRow <= nEndRow )
                nj++;
            if ( !bSplit )
            {
                if ( nj < m_data.size() && m_data[nj].bMarked == bMarked )
                {   // combine
                    if ( ni > 0 )
                    {
                        if ( m_data[ni-1].bMarked == bMarked )
                        {   // adjacent entries
                            m_data[ni-1].nRow = m_data[nj].nRow;
                            nj++;
                        }
                        else if ( ni == nInsert )
                            m_data[ni-1].nRow = nStartRow - 1;   // shrink
                    }
                    nInsert = MAXROWCOUNT;
                    bCombined = true;
                }
                else if ( ni > 0 && ni == nInsert )
                    m_data[ni-1].nRow = nStartRow - 1;   // shrink
            }
            if ( ni < nj )
            {   // remove middle entries
                if ( !bCombined )
                {   // replace one entry
                    m_data[ni].nRow = nEndRow;
                    m_data[ni].bMarked = bMarked;
                    ni++;
                    nInsert = MAXROWCOUNT;
                }
                m_data.erase( m_data.begin()+ni, m_data.begin()+nj );
            }

            if ( nInsert < sal::static_int_cast<SCSIZE>(MAXROWCOUNT) )
            {   // insert or append new entry
                if ( nInsert <= m_data.size() && bSplit )
                {
                    m_data.insert( m_data.begin()+nInsert, m_data[nInsert-1] );
                }
                if ( nInsert )
                    m_data[nInsert-1].nRow = nStartRow - 1;
                m_data.insert( m_data.begin()+nInsert, { nEndRow, bMarked } );
            }
        }
    }
}

bool ScMarkArray::IsAllMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    SCSIZE nStartIndex;
    SCSIZE nEndIndex;

    if ( Search( nStartRow, nStartIndex ) )
        if ( m_data[nStartIndex].bMarked )
            if ( Search( nEndRow, nEndIndex ) )
                if ( nEndIndex==nStartIndex )
                    return true;

    return false;
}

bool ScMarkArray::HasOneMark( SCROW& rStartRow, SCROW& rEndRow ) const
{
    bool bRet = false;
    if ( m_data.size() == 1 )
    {
        if ( m_data[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = MAXROW;
            bRet = true;
        }
    }
    else if ( m_data.size() == 2 )
    {
        if ( m_data[0].bMarked )
        {
            rStartRow = 0;
            rEndRow = m_data[0].nRow;
        }
        else
        {
            rStartRow = m_data[0].nRow + 1;
            rEndRow = MAXROW;
        }
        bRet = true;
    }
    else if ( m_data.size() == 3 )
    {
        if ( m_data[1].bMarked )
        {
            rStartRow = m_data[0].nRow + 1;
            rEndRow = m_data[1].nRow;
            bRet = true;
        }
    }
    return bRet;
}

SCsROW ScMarkArray::GetNextMarked( SCsROW nRow, bool bUp ) const
{
    SCsROW nRet = nRow;
    if (ValidRow(nRow))
    {
        SCSIZE nIndex;
        Search(nRow, nIndex);
        if (!m_data[nIndex].bMarked)
        {
            if (bUp)
            {
                if (nIndex>0)
                    nRet = m_data[nIndex-1].nRow;
                else
                    nRet = -1;
            }
            else
                nRet = m_data[nIndex].nRow + 1;
        }
    }
    return nRet;
}

SCROW ScMarkArray::GetMarkEnd( SCROW nRow, bool bUp ) const
{
    SCROW nRet;
    SCSIZE nIndex;
    Search(nRow, nIndex);
    OSL_ENSURE( m_data[nIndex].bMarked, "GetMarkEnd without bMarked" );
    if (bUp)
    {
        if (nIndex>0)
            nRet = m_data[nIndex-1].nRow + 1;
        else
            nRet = 0;
    }
    else
        nRet = m_data[nIndex].nRow;

    return nRet;
}

//  -------------- Iterator ----------------------------------------------

ScMarkArrayIter::ScMarkArrayIter( const ScMarkArray* pNewArray ) :
    pArray( pNewArray ),
    nPos( 0 )
{
}

ScMarkArrayIter::~ScMarkArrayIter()
{
}

bool ScMarkArrayIter::Next( SCROW& rTop, SCROW& rBottom )
{
    if ( nPos >= pArray->m_data.size() )
        return false;
    while (!pArray->m_data[nPos].bMarked)
    {
        ++nPos;
        if ( nPos >= pArray->m_data.size() )
            return false;
    }
    rBottom = pArray->m_data[nPos].nRow;
    if (nPos==0)
        rTop = 0;
    else
        rTop = pArray->m_data[nPos-1].nRow + 1;
    ++nPos;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
