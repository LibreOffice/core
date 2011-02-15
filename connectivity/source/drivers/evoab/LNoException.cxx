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
#include "precompiled_connectivity.hxx"
#include "LTable.hxx"
#include "LConnection.hxx"

using namespace connectivity;
using namespace connectivity::evoab;

//------------------------------------------------------------------
xub_StrLen OEvoabString::GetTokenCount( sal_Unicode cTok, sal_Unicode cStrDel ) const
{
    if ( !Len() )
        return 0;

    xub_StrLen nTokCount = 1;
    BOOL bStart = TRUE;     // Are we on the fist character of the token?
    BOOL bInString = FALSE; // Are we within a (cStrDel delimited) string?

    // Search for the first not-matching character (search ends at the end of string)
    for( xub_StrLen i = 0; i < Len(); i++ )
    {
        if (bStart)
        {
            bStart = FALSE;
            // First character a string delimiter?
            if ((*this).GetChar(i) == cStrDel)
            {
                bInString = TRUE;   // then we are within a string!
                continue;       // read next character!
            }
        }

        if (bInString) {
            // If we see the string delimiter ...
            if ( (*this).GetChar(i) == cStrDel )
            {
                if ((i+1 < Len()) && ((*this).GetChar(i+1) == cStrDel))
                {
                    // doubled string-delimiter:
                    i++;    // no end of string, skip next character.
                }
                else
                {
                    // end of String
                    bInString = FALSE;
                }
            }
        } else {
            // if the token character matches, then raise TokCount
            if ( (*this).GetChar(i) == cTok )
            {
                nTokCount++;
                bStart = TRUE;
            }
        }
    }
    //OSL_TRACE("OEvoabString::nTokCount = %d\n", ((OUtoCStr(::rtl::OUString(nTokCount))) ? (OUtoCStr(::rtl::OUString(nTokCount))):("NULL")) );

    return nTokCount;
}

//------------------------------------------------------------------
void OEvoabString::GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel ) const
{
    _rStr.Erase();
    xub_StrLen nLen = Len();
    if ( nLen )
    {
        BOOL bInString = (nStartPos < nLen) && ((*this).GetChar(nStartPos) == cStrDel); // are we within a (cStrDel delimited) String?

        // Is the first character a String-Delimiter?
        if (bInString )
            ++nStartPos;            // skip the character!
        // Search until end of string for the first not-matching character
        for( xub_StrLen i = nStartPos; i < nLen; ++i )
        {
            if (bInString)
            {
                // If we see the String-Delimiter ...
                if ( (*this).GetChar(i) == cStrDel )
                {
                    if ((i+1 < nLen) && ((*this).GetChar(i+1) == cStrDel))
                    {
                        // doubled String-Delimiter:
                        ++i;    // no end of String, skip next character
                        _rStr += (*this).GetChar(i);    // Character belongs to Result-String
                    }
                    else
                    {
                        // end of String
                        bInString = FALSE;
                    }
                }
                else
                {
                    _rStr += (*this).GetChar(i);    // Character belongs to Result-String
                }

            }
            else
            {
                // Does the Token-character match, then raise nTok
                if ( (*this).GetChar(i) == cTok )
                {
                    // Early termination of loop possible, because we found what we were looking for.
                    nStartPos = i+1;
                    break;
                }
                else
                {
                    _rStr += (*this).GetChar(i);    // Character belongs to Result-String
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OEvoabTable::refreshIndexes()
{
}
// -----------------------------------------------------------------------------
sal_Bool OEvoabTable::checkHeaderLine()
{
    if (m_nFilePos == 0 && ((OEvoabConnection*)m_pConnection)->isHeaderLine())
    {
        BOOL bRead2;
        do
        {
            bRead2 = m_pFileStream->ReadByteStringLine(m_aCurrentLine,m_pConnection->getTextEncoding());
        }
        while(bRead2 && !m_aCurrentLine.Len());

        m_nFilePos = m_pFileStream->Tell();
        if (m_pFileStream->IsEof())
            return sal_False;
    }
    return sal_True;
}
//------------------------------------------------------------------
sal_Bool OEvoabTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    //OSL_TRACE("OEvoabTable::(before SeekRow)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    if ( !m_pFileStream )
        return sal_False;
    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;
    // ----------------------------------------------------------
    // prepare positioning:
    //OSL_TRACE("OEvoabTable::(before SeekRow,m_pFileStriam Exist)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case IResultSetHelper::FIRST:
            m_nFilePos = 0;
            m_nRowPos = 1;
            // run through
        case IResultSetHelper::NEXT:
            if(eCursorPosition != IResultSetHelper::FIRST)
                ++m_nRowPos;
            m_pFileStream->Seek(m_nFilePos);
            if (m_pFileStream->IsEof() || !checkHeaderLine())
            {
                m_nMaxRowCount = m_nRowPos;
                return sal_False;
            }

            m_aRowToFilePos.insert(::std::map<sal_Int32,sal_Int32>::value_type(m_nRowPos,m_nFilePos));

            m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
            if (m_pFileStream->IsEof())
            {
                m_nMaxRowCount = m_nRowPos;
                return sal_False;
            }
            nCurPos = m_pFileStream->Tell();
            break;
        case IResultSetHelper::PRIOR:
            --m_nRowPos;
            if(m_nRowPos > 0)
            {
                m_nFilePos = m_aRowToFilePos.find(m_nRowPos)->second;
                m_pFileStream->Seek(m_nFilePos);
                if (m_pFileStream->IsEof() || !checkHeaderLine())
                    return sal_False;
                m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                if (m_pFileStream->IsEof())
                    return sal_False;
                nCurPos = m_pFileStream->Tell();
            }
            else
                m_nRowPos = 0;

            break;

            break;
        case IResultSetHelper::LAST:
            if(m_nMaxRowCount)
            {
                m_nFilePos = m_aRowToFilePos.rbegin()->second;
                m_nRowPos  = m_aRowToFilePos.rbegin()->first;
                m_pFileStream->Seek(m_nFilePos);
                if (m_pFileStream->IsEof() || !checkHeaderLine())
                    return sal_False;
                m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                if (m_pFileStream->IsEof())
                    return sal_False;
                nCurPos = m_pFileStream->Tell();
            }
            else
            {
                while(seekRow(IResultSetHelper::NEXT,1,nCurPos)) ; // run through after last row
                // now I know all
                seekRow(IResultSetHelper::PRIOR,1,nCurPos);
            }
            break;
        case IResultSetHelper::RELATIVE:
            if(nOffset > 0)
            {
                for(sal_Int32 i = 0;i<nOffset;++i)
                    seekRow(IResultSetHelper::NEXT,1,nCurPos);
            }
            else if(nOffset < 0)
            {
                for(sal_Int32 i = nOffset;i;++i)
                    seekRow(IResultSetHelper::PRIOR,1,nCurPos);
            }
            break;
        case IResultSetHelper::ABSOLUTE:
            {
                if(nOffset < 0)
                    nOffset = m_nRowPos + nOffset;
                ::std::map<sal_Int32,sal_Int32>::const_iterator aIter = m_aRowToFilePos.find(nOffset);
                if(aIter != m_aRowToFilePos.end())
                {
                    m_nFilePos = aIter->second;
                    m_pFileStream->Seek(m_nFilePos);
                    if (m_pFileStream->IsEof() || !checkHeaderLine())
                        return sal_False;
                    m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                    if (m_pFileStream->IsEof())
                        return sal_False;
                    nCurPos = m_pFileStream->Tell();
                }
                else if(m_nMaxRowCount && nOffset > m_nMaxRowCount) // offset is outside the table
                {
                    m_nRowPos = m_nMaxRowCount;
                    return sal_False;
                }
                else
                {
                    aIter = m_aRowToFilePos.upper_bound(nOffset);
                    if(aIter == m_aRowToFilePos.end())
                    {
                        m_nRowPos   = m_aRowToFilePos.rbegin()->first;
                        nCurPos = m_nFilePos = m_aRowToFilePos.rbegin()->second;
                        while(m_nRowPos != nOffset)
                            seekRow(IResultSetHelper::NEXT,1,nCurPos);
                    }
                    else
                    {
                        --aIter;
                        m_nRowPos   = aIter->first;
                        m_nFilePos  = aIter->second;
                        m_pFileStream->Seek(m_nFilePos);
                        if (m_pFileStream->IsEof() || !checkHeaderLine())
                            return sal_False;
                        m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                        if (m_pFileStream->IsEof())
                            return sal_False;
                        nCurPos = m_pFileStream->Tell();
                    }
                }
            }

            break;
        case IResultSetHelper::BOOKMARK:
            m_pFileStream->Seek(nOffset);
            if (m_pFileStream->IsEof())
                return sal_False;

            m_nFilePos = m_pFileStream->Tell(); // save Byte-Position in the file (at start of line)
            m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
            if (m_pFileStream->IsEof())
                return sal_False;
            nCurPos  = m_pFileStream->Tell();
            break;
    }

    //OSL_TRACE("OEvoabTable::(after SeekRow)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    return sal_True;
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
