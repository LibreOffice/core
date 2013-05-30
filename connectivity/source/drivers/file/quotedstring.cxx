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
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

namespace connectivity
{
    //==================================================================
    //= QuotedTokenizedString
    //==================================================================
    //------------------------------------------------------------------
    sal_Int32 QuotedTokenizedString::GetTokenCount( sal_Unicode cTok, sal_Unicode cStrDel ) const
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "QuotedTokenizedString::GetTokenCount" );
        const sal_Int32 nLen = m_sString.getLength();
        if ( !nLen )
            return 0;

        sal_Int32 nTokCount = 1;
        sal_Bool bStart = sal_True;     // Are we on the first character in the Token?
        sal_Bool bInString = sal_False; // Are we WITHIN a (cStrDel delimited) String?

        // Search for String-end after the first not matching character
        for( sal_Int32 i = 0; i < nLen; ++i )
        {
            const sal_Unicode cChar = m_sString[i];
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
                    if ((i+1 < nLen) && (m_sString[i+1] == cStrDel))
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
    OUString QuotedTokenizedString::GetTokenSpecial(sal_Int32& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel) const
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "QuotedTokenizedString::GetTokenCount" );
        const sal_Int32 nLen = m_sString.getLength();
        if ( nLen )
        {
            sal_Bool bInString = (nStartPos < nLen) && (m_sString[nStartPos] == cStrDel);   // are we WITHIN a (cStrDel delimited) String?

            // First character a String-Delimiter?
            if (bInString )
                ++nStartPos;            // skip this character!
            if ( nStartPos >= nLen )
                return OUString();

            OUStringBuffer aStr(nLen - nStartPos + 1);

            // Search until end of string for the first not matching character
            for( sal_Int32 i = nStartPos; i < nLen; ++i )
            {
                const sal_Unicode cChar = m_sString[i];
                if (bInString)
                {
                    // when now the String-Delimiter-character occurs ...
                    if ( cChar == cStrDel )
                    {
                        if ((i+1 < nLen) && (m_sString[i+1] == cStrDel))
                        {
                            // double String Delimiter-character
                            // no end of string, skip next character.
                            ++i;
                            aStr.append(m_sString[i]);    // character belongs to Result-String
                        }
                        else
                        {
                            //end of String
                            bInString = sal_False;
                        }
                    }
                    else
                    {
                        aStr.append(cChar);   // character belongs to Result-String
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
                        aStr.append(cChar);   // character belongs to Result-String
                    }
                }
            } // for( xub_StrLen i = nStartPos; i < nLen; ++i )
            return aStr.makeStringAndClear();
        }
        else
        {
            return OUString();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
