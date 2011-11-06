/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        sal_Bool bStart = sal_True;     // Stehen wir auf dem ersten Zeichen im Token?
        sal_Bool bInString = sal_False; // Befinden wir uns INNERHALB eines (cStrDel delimited) String?

        // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
        for( xub_StrLen i = 0; i < nLen; ++i )
        {
            const sal_Unicode cChar = m_sString.GetChar(i);
            if (bStart)
            {
                bStart = sal_False;
                // Erstes Zeichen ein String-Delimiter?
                if ( cChar == cStrDel )
                {
                    bInString = sal_True;   // dann sind wir jetzt INNERHALB des Strings!
                    continue;           // dieses Zeichen ueberlesen!
                }
            }

            if (bInString)
            {
                // Wenn jetzt das String-Delimiter-Zeichen auftritt ...
                if ( cChar == cStrDel )
                {
                    if ((i+1 < nLen) && (m_sString.GetChar(i+1) == cStrDel))
                    {
                        // Verdoppeltes String-Delimiter-Zeichen:
                        ++i;    // kein String-Ende, naechstes Zeichen ueberlesen.
                    }
                    else
                    {
                        // String-Ende
                        bInString = sal_False;
                    }
                }
            } // if (bInString)
            else
            {
                // Stimmt das Tokenzeichen ueberein, dann erhoehe TokCount
                if ( cChar == cTok )
                {
                    ++nTokCount;
                    bStart = sal_True;
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
            sal_Bool bInString = (nStartPos < nLen) && (m_sString.GetChar(nStartPos) == cStrDel);   // Befinden wir uns INNERHALB eines (cStrDel delimited) String?

            // Erstes Zeichen ein String-Delimiter?
            if (bInString )
                ++nStartPos;            // dieses Zeichen ueberlesen!
            if ( nStartPos >= nLen )
                return;

            sal_Unicode* pData = _rStr.AllocBuffer( nLen - nStartPos + 1 );
            const sal_Unicode* pStart = pData;
            // Suche bis Stringende nach dem ersten nicht uebereinstimmenden Zeichen
            for( xub_StrLen i = nStartPos; i < nLen; ++i )
            {
                const sal_Unicode cChar = m_sString.GetChar(i);
                if (bInString)
                {
                    // Wenn jetzt das String-Delimiter-Zeichen auftritt ...
                    if ( cChar == cStrDel )
                    {
                        if ((i+1 < nLen) && (m_sString.GetChar(i+1) == cStrDel))
                        {
                            // Verdoppeltes String-Delimiter-Zeichen:
                            // kein String-Ende, naechstes Zeichen ueberlesen.
                            ++i;
                            *pData++ = m_sString.GetChar(i);    // Zeichen gehoert zum Resultat-String
                        }
                        else
                        {
                            // String-Ende
                            bInString = sal_False;
                            *pData = 0;
                        }
                    }
                    else
                    {
                        *pData++ = cChar;   // Zeichen gehoert zum Resultat-String
                    }

                }
                else
                {
                    // Stimmt das Tokenzeichen ueberein, dann erhoehe nTok
                    if ( cChar == cTok )
                    {
                        // Vorzeitiger Abbruch der Schleife moeglich, denn
                        // wir haben, was wir wollten.
                        nStartPos = i+1;
                        break;
                    }
                    else
                    {
                        *pData++ = cChar;   // Zeichen gehoert zum Resultat-String
                    }
                }
            } // for( xub_StrLen i = nStartPos; i < nLen; ++i )
            *pData = 0;
            _rStr.ReleaseBufferAccess(xub_StrLen(pData - pStart));
        }
    }
}
