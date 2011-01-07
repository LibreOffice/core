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
