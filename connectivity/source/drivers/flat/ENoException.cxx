/*************************************************************************
 *
 *  $RCSfile: ENoException.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:26:47 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _CONNECTIVITY_FLAT_TABLE_HXX_
#include "flat/ETable.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_ECONNECTION_HXX_
#include "flat/EConnection.hxx"
#endif

using namespace connectivity;
using namespace connectivity::flat;

//------------------------------------------------------------------
xub_StrLen OFlatString::GetTokenCount( sal_Unicode cTok, sal_Unicode cStrDel ) const
{
    if ( !Len() )
        return 0;

    xub_StrLen nTokCount = 1;
    BOOL bStart = TRUE;     // Stehen wir auf dem ersten Zeichen im Token?
    BOOL bInString = FALSE; // Befinden wir uns INNERHALB eines (cStrDel delimited) String?

    // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
    for( xub_StrLen i = 0; i < Len(); i++ )
    {
        if (bStart)
        {
            bStart = FALSE;
            // Erstes Zeichen ein String-Delimiter?
            if ((*this).GetChar(i) == cStrDel)
            {
                bInString = TRUE;   // dann sind wir jetzt INNERHALB des Strings!
                continue;           // dieses Zeichen ueberlesen!
            }
        }

        if (bInString) {
            // Wenn jetzt das String-Delimiter-Zeichen auftritt ...
            if ( (*this).GetChar(i) == cStrDel )
            {
                if ((i+1 < Len()) && ((*this).GetChar(i+1) == cStrDel))
                {
                    // Verdoppeltes String-Delimiter-Zeichen:
                    i++;    // kein String-Ende, naechstes Zeichen ueberlesen.
                }
                else
                {
                    // String-Ende
                    bInString = FALSE;
                }
            }
        } else {
            // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
            if ( (*this).GetChar(i) == cTok )
            {
                nTokCount++;
                bStart = TRUE;
            }
        }
    }

    return nTokCount;
}

//------------------------------------------------------------------
void OFlatString::GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel ) const
{
    _rStr.Erase();
    xub_StrLen nLen = Len();
    if ( nLen )
    {
        BOOL bInString = (nStartPos < nLen) && ((*this).GetChar(nStartPos) == cStrDel); // Befinden wir uns INNERHALB eines (cStrDel delimited) String?

        // Erstes Zeichen ein String-Delimiter?
        if (bInString )
            ++nStartPos;            // dieses Zeichen ueberlesen!
        // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
        xub_StrLen i = nStartPos;
        for( ; i < nLen; ++i )
        {
            if (bInString)
            {
                // Wenn jetzt das String-Delimiter-Zeichen auftritt ...
                if ( (*this).GetChar(i) == cStrDel )
                {
                    if ((i+1 < nLen) && ((*this).GetChar(i+1) == cStrDel))
                    {
                        // Verdoppeltes String-Delimiter-Zeichen:
                        ++i;    // kein String-Ende, naechstes Zeichen ueberlesen.

                        _rStr += (*this).GetChar(i);    // Zeichen gehoert zum Resultat-String
                    }
                    else
                    {
                        // String-Ende
                        bInString = FALSE;
                    }
                }
                else
                {
                    _rStr += (*this).GetChar(i);    // Zeichen gehoert zum Resultat-String
                }

            }
            else
            {
                // Stimmt das Tokenzeichen ueberein, dann erhoehe nTok
                if ( (*this).GetChar(i) == cTok )
                {
                    // Vorzeitiger Abbruch der Schleife moeglich, denn
                    // wir haben, was wir wollten.
                    nStartPos = i+1;
                    break;
                }
                else
                {
                    _rStr += (*this).GetChar(i);    // Zeichen gehoert zum Resultat-String
                }
            }
        }
        if ( i == nLen && nStartPos < i )
            nStartPos = nLen;
    }
}
// -----------------------------------------------------------------------------
sal_Bool OFlatTable::checkHeaderLine()
{
    if (m_nFilePos == 0 && ((OFlatConnection*)m_pConnection)->isHeaderLine())
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
sal_Bool OFlatTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    if ( !m_pFileStream )
        return sal_False;
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    // ----------------------------------------------------------
    // Positionierung vorbereiten:

    sal_uInt32 nTempPos = m_nFilePos;
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

            m_nFilePos = m_pFileStream->Tell(); // Byte-Position in der Datei merken (am ZeilenANFANG)
            m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
            if (m_pFileStream->IsEof())
                return sal_False;
            nCurPos  = m_pFileStream->Tell();
            break;
    }


    return sal_True;
}
// -----------------------------------------------------------------------------



