/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: quotedstring.cxx,v $
 * $Revision: 1.4 $
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

namespace connectivity
{
    //==================================================================
    //= QuotedTokenizedString
    //==================================================================
    //------------------------------------------------------------------
    xub_StrLen QuotedTokenizedString::GetTokenCount( sal_Unicode cTok, sal_Unicode cStrDel ) const
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
        //OSL_TRACE("QuotedTokenizedString::nTokCount = %d\n", ((OUtoCStr(::rtl::OUString(nTokCount))) ? (OUtoCStr(::rtl::OUString(nTokCount))):("NULL")) );

        return nTokCount;
    }

    //------------------------------------------------------------------
    void QuotedTokenizedString::GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel ) const
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
            for( xub_StrLen i = nStartPos; i < nLen; ++i )
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
        }
    }
}
