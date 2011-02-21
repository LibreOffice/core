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
#include "file/quotedstring.hxx"
#include <rtl/logfile.hxx>

namespace connectivity
{
    //==================================================================
    //= QuotedTokenizedString
    //==================================================================
    //------------------------------------------------------------------
    xub_StrLen QuotedTokenizedString::GetTokenCount( sal_Unicode cTok, sal_Unicode cStrDel ) const
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "QuotedTokenizedString::GetTokenCount" );
        const xub_StrLen nLen = m_sString.Len();
        if ( !nLen )
            return 0;

        xub_StrLen nTokCount = 1;
        BOOL bStart = TRUE;     // Are we on the first character in the Token?
        BOOL bInString = FALSE; // Are we WITHIN a (cStrDel delimited) String?

        // Search for String-end after the first not matching character
        for( xub_StrLen i = 0; i < nLen; ++i )
        {
            const sal_Unicode cChar = m_sString.GetChar(i);
            if (bStart)
            {
                bStart = FALSE;
                // First character a String-Delimiter?
                if ( cChar == cStrDel )
                {
                    bInString = TRUE;   // then we are now WITHIN the string!
                    continue;           // skip this character!
                }
            }

            if (bInString)
            {
                // when now the String-Delimiter-character occurs ...
                if ( cChar == cStrDel )
                {
                    if ((i+1 < nLen) && (m_sString.GetChar(i+1) == cStrDel))
                    {
                        // double String-Delimter-character:
                        ++i;    // no string-end, skip next character.
                    }
                    else
                    {
                        // String-End
                        bInString = FALSE;
                    }
                }
            } // if (bInString)
            else
            {
                // does the Token-character match, then raise TokCount
                if ( cChar == cTok )
                {
                    ++nTokCount;
                    bStart = TRUE;
                }
            }
        }
        //OSL_TRACE("QuotedTokenizedString::nTokCount = %d\n", ((OUtoCStr(::rtl::OUString(nTokCount))) ? (OUtoCStr(::rtl::OUString(nTokCount))):("NULL")) );

        return nTokCount;
    }

    //------------------------------------------------------------------
    void QuotedTokenizedString::GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel ) const
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "QuotedTokenizedString::GetTokenCount" );
        _rStr.Erase();
        const xub_StrLen nLen = m_sString.Len();
        if ( nLen )
        {
            BOOL bInString = (nStartPos < nLen) && (m_sString.GetChar(nStartPos) == cStrDel);   // are we WITHIN a (cStrDel delimited) String?

            // First character a String-Delimiter?
            if (bInString )
                ++nStartPos;            // skip this character!
            if ( nStartPos >= nLen )
                return;

            sal_Unicode* pData = _rStr.AllocBuffer( nLen - nStartPos + 1 );
            const sal_Unicode* pStart = pData;
            // Search until end of string for the first not matching character
            for( xub_StrLen i = nStartPos; i < nLen; ++i )
            {
                const sal_Unicode cChar = m_sString.GetChar(i);
                if (bInString)
                {
                    // when now the String-Delimiter-character occurs ...
                    if ( cChar == cStrDel )
                    {
                        if ((i+1 < nLen) && (m_sString.GetChar(i+1) == cStrDel))
                        {
                            // double String Delimiter-character
                            // no end of string, skip next character.
                            ++i;
                            *pData++ = m_sString.GetChar(i);    // character belongs to Result-String
                        }
                        else
                        {
                            //end of String
                            bInString = FALSE;
                            *pData = 0;
                        }
                    }
                    else
                    {
                        *pData++ = cChar;   // character belongs to Result-String
                    }

                }
                else
                {
                    // does the Token-sign match, then raise nTok
                    if ( cChar == cTok )
                    {
                        // premature break of loop possible, because we found what we were looking for
                        nStartPos = i+1;
                        break;
                    }
                    else
                    {
                        *pData++ = cChar;   // character belongs to Result-String
                    }
                }
            } // for( xub_StrLen i = nStartPos; i < nLen; ++i )
            *pData = 0;
            _rStr.ReleaseBufferAccess(xub_StrLen(pData - pStart));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
