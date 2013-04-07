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
        sal_Bool bStart = sal_True;     // Are we on the first character in the Token?
        sal_Bool bInString = sal_False; // Are we WITHIN a (cStrDel delimited) String?

        // Search for String-end after the first not matching character
        for( xub_StrLen i = 0; i < nLen; ++i )
        {
            const sal_Unicode cChar = m_sString.GetChar(i);
            if (bStart)
            {
                bStart = sal_False;
                // First character a String-Delimiter?
                if ( cChar == cStrDel )
                {
                    bInString = sal_True;   // then we are now WITHIN the string!
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
                        bInString = sal_False;
                    }
                }
            } // if (bInString)
            else
            {
                // does the Token-character match, then raise TokCount
                if ( cChar == cTok )
                {
                    ++nTokCount;
                    bStart = sal_True;
                }
            }
        }
        //OSL_TRACE("QuotedTokenizedString::nTokCount = %d\n", ((OUtoCStr(OUString(nTokCount))) ? (OUtoCStr(OUString(nTokCount))):("NULL")) );

        return nTokCount;
    }

    //------------------------------------------------------------------
    String QuotedTokenizedString::GetTokenSpecial(xub_StrLen& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel) const
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "QuotedTokenizedString::GetTokenCount" );
        String aStr;
        const xub_StrLen nLen = m_sString.Len();
        if ( nLen )
        {
            sal_Bool bInString = (nStartPos < nLen) && (m_sString.GetChar(nStartPos) == cStrDel);   // are we WITHIN a (cStrDel delimited) String?

            // First character a String-Delimiter?
            if (bInString )
                ++nStartPos;            // skip this character!
            if ( nStartPos >= nLen )
                return aStr;

            sal_Unicode* pData = aStr.AllocBuffer( nLen - nStartPos + 1 );
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
                            bInString = sal_False;
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
            aStr.ReleaseBufferAccess(xub_StrLen(pData - pStart));
        }
        return aStr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
