/*************************************************************************
 *
 *  $RCSfile: parhtml.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:05 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#include <ctype.h>
#include <stdio.h>

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svstdarr.hxx>
#endif

#include "parhtml.hxx"
#include "htmltokn.h"
#include "htmlkywd.hxx"

const sal_Int32 MAX_LEN = 1024L;
//static sal_Unicode sTmpBuffer[ MAX_LEN+1 ];
const sal_Int32 MAX_MACRO_LEN = 1024;

/*  */

// Tabellen zum Umwandeln von Options-Werten in Strings

// <INPUT TYPE=xxx>
static HTMLOptionEnum __READONLY_DATA aInputTypeOptEnums[] =
{
    { sHTML_IT_text,        HTML_IT_TEXT        },
    { sHTML_IT_password,    HTML_IT_PASSWORD    },
    { sHTML_IT_checkbox,    HTML_IT_CHECKBOX    },
    { sHTML_IT_radio,       HTML_IT_RADIO       },
    { sHTML_IT_range,       HTML_IT_RANGE       },
    { sHTML_IT_scribble,    HTML_IT_SCRIBBLE    },
    { sHTML_IT_file,        HTML_IT_FILE        },
    { sHTML_IT_hidden,      HTML_IT_HIDDEN      },
    { sHTML_IT_submit,      HTML_IT_SUBMIT      },
    { sHTML_IT_image,       HTML_IT_IMAGE       },
    { sHTML_IT_reset,       HTML_IT_RESET       },
    { sHTML_IT_button,      HTML_IT_BUTTON      },
    { 0,                    0                   }
};

// <TABLE FRAME=xxx>
static HTMLOptionEnum __READONLY_DATA aTableFrameOptEnums[] =
{
    { sHTML_TF_void,    HTML_TF_VOID    },
    { sHTML_TF_above,   HTML_TF_ABOVE   },
    { sHTML_TF_below,   HTML_TF_BELOW   },
    { sHTML_TF_hsides,  HTML_TF_HSIDES  },
    { sHTML_TF_lhs,     HTML_TF_LHS     },
    { sHTML_TF_rhs,     HTML_TF_RHS     },
    { sHTML_TF_vsides,  HTML_TF_VSIDES  },
    { sHTML_TF_box,     HTML_TF_BOX     },
    { sHTML_TF_border,  HTML_TF_BOX     },
    { 0,                0               }
};

// <TABLE RULES=xxx>
static HTMLOptionEnum __READONLY_DATA aTableRulesOptEnums[] =
{
    { sHTML_TR_none,    HTML_TR_NONE    },
    { sHTML_TR_groups,  HTML_TR_GROUPS  },
    { sHTML_TR_rows,    HTML_TR_ROWS    },
    { sHTML_TR_cols,    HTML_TR_COLS    },
    { sHTML_TR_all,     HTML_TR_ALL     },
    { 0,                0               }
};


SV_IMPL_PTRARR(HTMLOptions,HTMLOptionPtr)

/*  */

USHORT HTMLOption::GetEnum( const HTMLOptionEnum *pOptEnums, USHORT nDflt ) const
{
    USHORT nValue = nDflt;

    while( pOptEnums->pName )
        if( aValue.EqualsIgnoreCaseAscii( pOptEnums->pName ) )
            break;
        else
            pOptEnums++;

    if( pOptEnums->pName )
        nValue = pOptEnums->nValue;

    return nValue;
}

BOOL HTMLOption::GetEnum( USHORT &rEnum, const HTMLOptionEnum *pOptEnums ) const
{
    while( pOptEnums->pName )
    {
        if( aValue.EqualsIgnoreCaseAscii( pOptEnums->pName ) )
            break;
        else
            pOptEnums++;
    }

    const sal_Char *pName = pOptEnums->pName;
    if( pName )
        rEnum = pOptEnums->nValue;

    return (pName != 0);
}

HTMLOption::HTMLOption( USHORT nTok, const String& rToken,
                        const String& rValue )
    : nToken( nTok ), aToken(rToken), aValue(rValue)
{
    DBG_ASSERT( nToken>=HTML_OPTION_START && nToken<HTML_OPTION_END,
        "HTMLOption: unbekanntes Token" );
}

ULONG HTMLOption::GetNumber() const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_NUMBER_START &&
                 nToken<HTML_OPTION_NUMBER_END) ||
                (nToken>=HTML_OPTION_CONTEXT_START &&
                 nToken<HTML_OPTION_CONTEXT_END) ||
                nToken==HTML_O_VALUE,
        "GetNumber: Option ist nicht numerisch" );
    String aTmp( aValue );
    aTmp.EraseLeadingChars();
    sal_Int32 nTmp = aTmp.ToInt32();
    return nTmp >= 0L ? (sal_uInt32)nTmp : 0UL;
}

INT32 HTMLOption::GetSNumber() const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_NUMBER_START && nToken<HTML_OPTION_NUMBER_END) ||
                (nToken>=HTML_OPTION_CONTEXT_START && nToken<HTML_OPTION_CONTEXT_END),
        "GetSNumber: Option ist nicht numerisch" );
    String aTmp( aValue );
    aTmp.EraseLeadingChars();
    return aTmp.ToInt32();
}

void HTMLOption::GetNumbers( SvULongs &rLongs, BOOL bSpaceDelim ) const
{
    if( rLongs.Count() )
        rLongs.Remove( 0, rLongs.Count() );

    if( bSpaceDelim )
    {
        // das ist ein sehr stark vereinfachter Scanner. Er sucht einfach
        // alle Tiffern aus dem String
        BOOL bInNum = FALSE;
        ULONG nNum = 0UL;
        for( sal_uInt32 i=0UL; i<aValue.Len(); i++ )
        {
            register sal_Unicode c = aValue.GetChar( i );
            if( c>='0' && c<='9' )
            {
                nNum *= 10UL;
                nNum += (c - '0');
                bInNum = TRUE;
            }
            else if( bInNum )
            {
                rLongs.Insert( nNum, rLongs.Count() );
                bInNum = FALSE;
                nNum = 0UL;
            }
        }
        if( bInNum )
        {
            rLongs.Insert( nNum, rLongs.Count() );
        }
    }
    else
    {
        // hier wird auf die korrekte Trennung der Zahlen durch ',' geachtet
        // und auch mal eine 0 eingefuegt
        sal_uInt32 nPos = 0UL;
        while( nPos < aValue.Len() )
        {
            register sal_Unicode c;
            while( nPos < aValue.Len() &&
                   ((c=aValue.GetChar(nPos)) == ' ' || c == '\t' ||
                   c == '\n' || c== '\r' ) )
                nPos++;

            if( nPos==aValue.Len() )
                rLongs.Insert( 0UL, rLongs.Count() );
            else
            {
                sal_uInt32 nEnd = aValue.Search( (sal_Unicode)',', nPos );
                if( STRING_NOTFOUND==nEnd )
                {
                    sal_Int32 nTmp = aValue.Copy(nPos).ToInt32();
                    rLongs.Insert( nTmp >= 0L ? (sal_uInt32)nTmp : 0UL,
                                   rLongs.Count() );
                    nPos = aValue.Len();
                }
                else
                {
                    sal_Int32 nTmp =
                        aValue.Copy(nPos,nEnd-nPos).ToInt32();
                    rLongs.Insert( nTmp >= 0L ? (sal_uInt32)nTmp : 0UL,
                                   rLongs.Count() );
                    nPos = nEnd+1UL;
                }
            }
        }
    }
}

void HTMLOption::GetColor( Color& rColor ) const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_COLOR_START && nToken<HTML_OPTION_COLOR_END) || nToken==HTML_O_SIZE,
        "GetColor: Option spezifiziert keine Farbe" );

    String aTmp( aValue );
    aTmp.ToUpperAscii();
    ULONG nColor = ULONG_MAX;
    if( '#'!=aTmp.GetChar( 0UL ) )
        nColor = GetHTMLColor( aTmp );

    if( ULONG_MAX == nColor )
    {
        nColor = 0UL;
        sal_uInt32 nPos = 0UL;
        for( sal_uInt32 i=0UL; i<6UL; i++ )
        {
            // MIB 26.06.97: Wie auch immer Netscape Farbwerte ermittelt,
            // maximal drei Zeichen, die kleiner als '0' sind werden
            // ignoriert. Bug #40901# stimmt damit. Mal schauen, was sich
            // irgendwelche HTML-Autoren noch so einfallen lassen...
            register sal_Unicode c = nPos<aTmp.Len() ? aTmp.GetChar( nPos++ )
                                                     : '0';
            if( c < '0' )
            {
                c = nPos<aTmp.Len() ? aTmp.GetChar(nPos++) : '0';
                if( c < '0' )
                    c = nPos<aTmp.Len() ? aTmp.GetChar(nPos++) : '0';
            }
            nColor *= 16UL;
            if( c >= '0' && c <= '9' )
                nColor += (c - 48);
            else if( c >= 'A' && c <= 'F' )
                nColor += (c - 55);
        }
    }

    rColor.SetRed(   (BYTE)((nColor & 0x00ff0000UL) >> 16) );
    rColor.SetGreen( (BYTE)((nColor & 0x0000ff00UL) >> 8));
    rColor.SetBlue(  (BYTE)(nColor & 0x000000ffUL) );
}

HTMLInputType HTMLOption::GetInputType() const
{
    DBG_ASSERT( nToken==HTML_O_TYPE, "GetInputType: Option nicht TYPE" );
    return (HTMLInputType)GetEnum( aInputTypeOptEnums, HTML_IT_TEXT );
}

HTMLTableFrame HTMLOption::GetTableFrame() const
{
    DBG_ASSERT( nToken==HTML_O_FRAME, "GetTableFrame: Option nicht FRAME" );
    return (HTMLTableFrame)GetEnum( aTableFrameOptEnums, HTML_TF_VOID );
}

HTMLTableRules HTMLOption::GetTableRules() const
{
    DBG_ASSERT( nToken==HTML_O_RULES, "GetTableRules: Option nicht RULES" );
    return (HTMLTableRules)GetEnum( aTableRulesOptEnums, HTML_TR_NONE );
}

/*  */

HTMLParser::HTMLParser( SvStream& rIn, int bReadNewDoc )
    : SvParser( rIn )
{
    bNewDoc = bReadNewDoc;
    bReadListing = bReadXMP = bReadPRE = bReadTextArea =
        bReadScript = bReadStyle =
        bEndTokenFound = bIsInBody = bReadNextChar =
        bReadComment = FALSE;
    bIsInHeader = TRUE;
    pOptions = new HTMLOptions;
}

HTMLParser::~HTMLParser()
{
    if( pOptions && pOptions->Count() )
        pOptions->DeleteAndDestroy( 0, pOptions->Count() );
    delete pOptions;
}

SvParserState __EXPORT HTMLParser::CallParser()
{
    eState = SVPAR_WORKING;
    nNextCh = GetNextChar();
    SaveState( 0 );

    nPre_LinePos = 0UL;
    bPre_IgnoreNewPara = FALSE;

    AddRef();
    Continue( 0 );
    if( SVPAR_PENDING != eState )
        ReleaseRef();       // dann brauchen wir den Parser nicht mehr!

    return eState;
}

void HTMLParser::Continue( int nToken )
{
    if( !nToken )
        nToken = GetNextToken();

    while( IsParserWorking() )
    {
        SaveState( nToken );
        nToken = FilterToken( nToken );

        if( nToken )
            NextToken( nToken );

        if( IsParserWorking() )
            SaveState( 0 );         // bis hierhin abgearbeitet,
                                    // weiter mit neuem Token!
        nToken = GetNextToken();
    }
}

int HTMLParser::FilterToken( int nToken )
{
    switch( nToken )
    {
    case sal_Unicode(EOF):
        nToken = 0;
        break;          // nicht verschicken

    case HTML_HEAD_OFF:
        bIsInBody = TRUE;
    case HTML_HEAD_ON:
        bIsInHeader = HTML_HEAD_ON == nToken;
        break;

    case HTML_BODY_ON:
    case HTML_FRAMESET_ON:
        bIsInHeader = FALSE;
        bIsInBody = HTML_BODY_ON == nToken;
        break;

    case HTML_BODY_OFF:
        bIsInBody = bReadPRE = bReadListing = bReadXMP = FALSE;
        break;

    case HTML_HTML_OFF:
        nToken = 0;
        bReadPRE = bReadListing = bReadXMP = FALSE;
        break;      // HTML_ON wurde auch nicht verschickt !

    case HTML_PREFORMTXT_ON:
        StartPRE();
        break;

    case HTML_PREFORMTXT_OFF:
        FinishPRE();
        break;

    case HTML_LISTING_ON:
        StartListing();
        break;

    case HTML_LISTING_OFF:
        FinishListing();
        break;

    case HTML_XMP_ON:
        StartXMP();
        break;

    case HTML_XMP_OFF:
        FinishXMP();
        break;

    default:
        if( bReadPRE )
            nToken = FilterPRE( nToken );
        else if( bReadListing )
            nToken = FilterListing( nToken );
        else if( bReadXMP )
            nToken = FilterXMP( nToken );

        break;
    }

    return nToken;
}

#define HTML_ISDIGIT( c ) (c >= '0' && c <= '9')
#define HTML_ISALPHA( c ) ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )
#define HTML_ISALNUM( c ) ( HTML_ISALPHA(c) || HTML_ISDIGIT(c) )
#define HTML_ISSPACE( c ) ( ' ' == c || (c >= 0x09 && c <= 0x0d) )
#define HTML_ISPRINTABLE( c ) ( c >= 32 && c != 127)

int HTMLParser::ScanText( const sal_Unicode cBreak )
{
    ::rtl::OUStringBuffer sTmpBuffer( MAX_LEN );
    int bWeiter = TRUE;
    int bEqSignFound = FALSE;
    sal_Unicode cQuote = 0U;

    while( bWeiter && IsParserWorking() )
    {
        int bNextCh = TRUE;
        switch( nNextCh )
        {
        case '&':
            bEqSignFound = FALSE;
            if( bReadXMP )
                sTmpBuffer.append( (sal_Unicode)'&' );
            else
            {
                ULONG nStreamPos = rInput.Tell();
                ULONG nLinePos = GetLinePos();

                sal_Unicode cChar = 0U;
                if( '#' == (nNextCh = GetNextChar()) )
                {
                    nNextCh = GetNextChar();
                    if( HTML_ISDIGIT(nNextCh) )
                    {
                        do
                        {
                            cChar = cChar * 10U + sal_Unicode( nNextCh - '0');
                            nNextCh = GetNextChar();
                        }
                        while( HTML_ISDIGIT(nNextCh) );

                        if( RTL_TEXTENCODING_DONTKNOW != eSrcEnc &&
                            RTL_TEXTENCODING_UCS2 != eSrcEnc &&
                            RTL_TEXTENCODING_UTF8 != eSrcEnc )
                        {
                             sal_Unicode cOrig = cChar;
                            cChar = ByteString::ConvertToUnicode(
                                            (sal_Char)cChar, eSrcEnc );
                            if( 0U == cChar )
                            {
                                // #73398#: If the character could not be
                                // converted, because a conversion is not
                                // available, do no conversion at all.
                                cChar = cOrig;
                            }
                        }
                    }
                    else
                        nNextCh = 0U;
                }
                else if( HTML_ISALPHA( nNextCh ) )
                {
                    ::rtl::OUStringBuffer sEntityBuffer( 6L );
                    sal_Int32 nPos = 0L;
                    do
                    {
                        sEntityBuffer.append( nNextCh );
                        nPos++;
                        nNextCh = GetNextChar();
                    }
                    while( nPos < 6L && HTML_ISALNUM( nNextCh ) &&
                           !rInput.IsEof() );

                    if( IsParserWorking() && !rInput.IsEof() )
                    {
                        String sEntity( sEntityBuffer.getStr(),
                                        (sal_uInt32)nPos );
                        cChar = GetHTMLCharName( sEntity );

                        // nicht gefunden ( == 0 ), dann Klartext
                        // oder ein Zeichen das als Attribut eingefuegt
                        // wird
                        if( 0U == cChar && ';' != nNextCh )
                        {
                            DBG_ASSERT( rInput.Tell() - nStreamPos ==
                                        (ULONG)(nPos+1L)*GetCharSize(),
                                        "UTF-8 geht hier schief" );
                            for( sal_Int32 i=nPos-1L; i>1L; i-- )
                            {
                                nNextCh = sEntityBuffer[i];
                                sEntityBuffer.setLength( i );
                                sEntity.Assign( sEntityBuffer.getStr(),
                                                (sal_uInt32)i );
                                 cChar = GetHTMLCharName( sEntity );
                                if( cChar )
                                {
                                    rInput.SeekRel( -(long)
                                            ((nPos-i)*GetCharSize()) );
                                    nlLinePos -= sal_uInt32(nPos-i);
                                    nPos = i;
                                    break;
                                }
                            }
                        }

                        if( !cChar )        // unbekanntes Zeichen?
                        {
                            // dann im Stream zurueck, das '&' als Zeichen
                            // einfuegen und mit dem nachfolgenden Zeichen
                            // wieder aufsetzen
                            sTmpBuffer.append( (sal_Unicode)'&' );

//                          rInput.SeekRel( -(long)(++nPos*GetCharSize()) );
//                          nlLinePos -= nPos;
                            DBG_ASSERT( rInput.Tell()-nStreamPos ==
                                        (ULONG)(nPos+1)*GetCharSize(),
                                        "Falsche Stream-Position" );
                            DBG_ASSERT( nlLinePos-nLinePos ==
                                        (ULONG)(nPos+1),
                                        "Falsche Zeilen-Position" );
                            rInput.Seek( nStreamPos );
                            nlLinePos = nLinePos;
                            break;
                        }

                        // 1 == Non Breaking Space
                        // 2 == SoftHyphen

                        if( cChar < 3U )
                        {
                            if( '>' == cBreak )
                            {
                                // Wenn der Inhalt eines Tags gelesen wird,
                                // muessen wir ein Space bzw. - daraus machen
                                switch( cChar )
                                {
                                case 1U: cChar = ' '; break;
                                case 2U: cChar = '-'; break;
                                default:
                                    DBG_ASSERT( cChar==1U,
                            "\0x00 sollte doch schon laengt abgefangen sein!" );
                                    break;
                                }
                            }
                            else
                            {
                                // Wenn kein Tag gescannt wird, enstprechendes
                                // Token zurueckgeben
                                aToken +=
                                    String( sTmpBuffer.makeStringAndClear() );
                                if( cChar )
                                {
                                    if( aToken.Len() )
                                    {
                                        // mit dem Zeichen wieder aufsetzen
                                        nNextCh = '&';
//                                      rInput.SeekRel( -(long)(++nPos*GetCharSize()) );
//                                      nlLinePos -= nPos;
                                        DBG_ASSERT( rInput.Tell()-nStreamPos ==
                                                    (ULONG)(nPos+1)*GetCharSize(),
                                                    "Falsche Stream-Position" );
                                        DBG_ASSERT( nlLinePos-nLinePos ==
                                                    (ULONG)(nPos+1),
                                                    "Falsche Zeilen-Position" );
                                        rInput.Seek( nStreamPos );
                                        nlLinePos = nLinePos;
                                        return HTML_TEXTTOKEN;
                                    }

                                    // Hack: _GetNextChar soll nicht das
                                    // naechste Zeichen lesen
                                    if( ';' != nNextCh )
                                        aToken += ' ';
                                    if( 1U == cChar )
                                        return HTML_NONBREAKSPACE;
                                    if( 2U == cChar )
                                        return HTML_SOFTHYPH;
                                }
                                aToken += (sal_Unicode)'&';
                                aToken +=
                                    String(sEntityBuffer.makeStringAndClear());
                                break;
                            }
                        }
                    }
                    else
                        nNextCh = 0U;
                }
                // MIB 03/02/2000: &{...};-JavaScript-Macros are not
                // supported any longer.
                else if( IsParserWorking() )
                {
                    sTmpBuffer.append( (sal_Unicode)'&' );
                    bNextCh = FALSE;
                    break;
                }

                bNextCh = (';' == nNextCh);
                if( cBreak=='>' && (cChar=='\\' || cChar=='\'' ||
                                    cChar=='\"' || cChar==' ') )
                {
                    // ' und " mussen innerhalb von Tags mit einem \
                    // gekennzeichnet werden, um sie von ' und " als Klammern
                    // um Optionen zu unterscheiden. Logischerweise muss
                    // deshalb auch ein \ gekeenzeichnet werden. Ausserdem
                    // schuetzen wir ein Space, weil es kein Trennzeichen
                    // zwischen Optionen ist.
                    sTmpBuffer.append( (sal_Unicode)'\\' );
                    if( MAX_LEN == sTmpBuffer.getLength() )
                        aToken += String(sTmpBuffer.makeStringAndClear());
                }
                if( IsParserWorking() )
                {
                    if( cChar )
                        sTmpBuffer.append( cChar );
                }
                else if( SVPAR_PENDING==eState && '>'!=cBreak )
                {
                    // Mit dem '&' Zeichen wieder aufsetzen, der Rest
                    // wird als Texttoken zurueckgegeben.
                    if( aToken.Len() || sTmpBuffer.getLength() )
                    {
                        // Der bisherige Text wird von _GetNextChar()
                        // zurueckgegeben und beim naechsten Aufruf wird
                        // ein neues Zeichen gelesen. Also muessen wir uns
                        // noch vor das & stellen.
                        nNextCh = 0U;
                        rInput.Seek( nStreamPos-(sal_uInt32)GetCharSize() );
                        nlLinePos = nLinePos-1UL;
                        bReadNextChar = TRUE;
                    }
                    bNextCh = FALSE;
                }
            }
            break;
        case '=':
            if( '>'==cBreak && !cQuote )
                bEqSignFound = TRUE;
            sTmpBuffer.append( nNextCh );
            break;

        case '\\':
            if( '>'==cBreak )
            {
                // Innerhalb von Tags kennzeichnen
                sTmpBuffer.append( (sal_Unicode)'\\' );
                if( MAX_LEN == sTmpBuffer.getLength() )
                    aToken += String(sTmpBuffer.makeStringAndClear());
            }
            sTmpBuffer.append( (sal_Unicode)'\\' );
            break;

        case '\"':
        case '\'':
            if( '>'==cBreak )
            {
                if( bEqSignFound )
                    cQuote = nNextCh;
                else if( cQuote && (cQuote==nNextCh ) )
                    cQuote = 0U;
            }
            sTmpBuffer.append( nNextCh );
            bEqSignFound = FALSE;
            break;

        case sal_Unicode(EOF):
            if( rInput.IsEof() )
            {
// MIB 20.11.98: Das macht hier keinen Sinn, oder doch: Zumindest wird
// abc&auml;<EOF> nicht angezeigt, also lassen wir das in Zukunft.
//              if( '>' != cBreak )
//                  eState = SVPAR_ACCEPTED;
                bWeiter = FALSE;
            }
            else
            {
                sTmpBuffer.append( nNextCh );
            }
            break;

        case '<':
            bEqSignFound = FALSE;
            if( '>'==cBreak )
                sTmpBuffer.append( nNextCh );
            else
                bWeiter = FALSE;        // Abbrechen, String zusammen
            break;

        case '\f':
            if( '>' == cBreak )
            {
                // Beim Scannen von Optionen wie ein Space behandeln
                sTmpBuffer.append( (sal_Unicode)' ' );
            }
            else
            {
                // sonst wird es ein eigenes Token
                bWeiter = FALSE;
            }
            break;

        case '\r':
        case '\n':
            if( '>'==cBreak )
            {
                // #26979# cr/lf in Tag wird in _GetNextToken() behandeln
                sTmpBuffer.append( nNextCh );
                break;
            }
            else if( bReadListing || bReadXMP || bReadPRE || bReadTextArea )
            {
                bWeiter = FALSE;
                break;
            }
            // Bug 18984: CR-LF -> Blank
            //      Folge von CR/LF/BLANK/TAB nur in ein Blank wandeln
            // kein break!!
        case '\t':
            if( '\t'==nNextCh && bReadPRE && '>'!=cBreak )
            {
                // In <PRE>: Tabs nach oben durchreichen
                bWeiter = FALSE;
                break;
            }
            // kein break
        case '\x0b':
            if( '\x0b'==nNextCh && (bReadPRE || bReadXMP ||bReadListing) &&
                '>'!=cBreak )
            {
                break;
            }
            nNextCh = ' ';
            // kein break;
        case ' ':
            sTmpBuffer.append( nNextCh );
            if( '>'!=cBreak && (!bReadListing && !bReadXMP &&
                                !bReadPRE && !bReadTextArea) )
            {
                // alle Folgen von Blanks/Tabs/CR/LF zu einem Blank umwandeln
                do {
                    if( sal_Unicode(EOF) == (nNextCh = GetNextChar()) &&
                        rInput.IsEof() )
                    {
                        if( aToken.Len() || sTmpBuffer.getLength() > 1L )
                        {
                            // ausser den Blanks wurde noch etwas geselen
                            aToken += String(sTmpBuffer.makeStringAndClear());
                            return HTML_TEXTTOKEN;
                        }
                        else
                            // nur Blanks gelesen: dann darf kein Text
                            // mehr zurueckgegeben werden und _GetNextToken
                            // muss auf EOF laufen
                            return 0;
                    }
                } while ( ' ' == nNextCh || '\t' == nNextCh ||
                          '\r' == nNextCh || '\n' == nNextCh ||
                          '\x0b' == nNextCh );
                bNextCh = FALSE;
            }
            break;

        default:
            bEqSignFound = FALSE;
            if( (nNextCh==cBreak && !cQuote) ||
                (ULONG(aToken.Len()) + MAX_LEN) > ULONG(STRING_MAXLEN & ~1 ))
                bWeiter = FALSE;
            else
            {
                do {
                    // alle anderen Zeichen kommen in den Text
                    sTmpBuffer.append( nNextCh );
                    if( MAX_LEN == sTmpBuffer.getLength() )
                    {
                        aToken += String(sTmpBuffer.makeStringAndClear());
                        if( (ULONG(aToken.Len()) + MAX_LEN) >
                                ULONG(STRING_MAXLEN & ~1 ) )
                        {
                            nNextCh = GetNextChar();
                            return HTML_TEXTTOKEN;
                        }
                    }
                    if( ( sal_Unicode(EOF) == (nNextCh = GetNextChar()) &&
                          rInput.IsEof() ) ||
                        !IsParserWorking() )
                    {
                        if( sTmpBuffer.getLength() )
                            aToken += String(sTmpBuffer.makeStringAndClear());
                        return HTML_TEXTTOKEN;
                    }
                } while( HTML_ISALPHA( nNextCh ) || HTML_ISDIGIT( nNextCh ) );
                bNextCh = FALSE;
            }
        }

        if( MAX_LEN == sTmpBuffer.getLength() )
            aToken += String(sTmpBuffer.makeStringAndClear());

        if( bWeiter && bNextCh )
            nNextCh = GetNextChar();
    }

    if( sTmpBuffer.getLength() )
        aToken += String(sTmpBuffer.makeStringAndClear());

    return HTML_TEXTTOKEN;
}

int HTMLParser::_GetNextRawToken()
{
    ::rtl::OUStringBuffer sTmpBuffer( MAX_LEN );

    if( bEndTokenFound )
    {
        // beim letzten Aufruf haben wir das End-Token bereits gefunden,
        // deshalb muessen wir es nicht noch einmal suchen
        bReadScript = FALSE;
        bReadStyle = FALSE;
        aEndToken.Erase();
        bEndTokenFound = FALSE;

        return 0;
    }

    // per default geben wir HTML_RAWDATA zurueck
    int bWeiter = TRUE;
    int nToken = HTML_RAWDATA;
    SaveState( 0 );
    while( bWeiter && IsParserWorking() )
    {
        int bNextCh = TRUE;
        switch( nNextCh )
        {
        case '<':
            {
                // Vielleicht haben wir das Ende erreicht

                // das bisher gelesene erstmal retten
                aToken += String(sTmpBuffer.makeStringAndClear());

                // und die Position im Stream merken
                ULONG nStreamPos = rInput.Tell();
                ULONG nLineNr = GetLineNr();
                ULONG nLinePos = GetLinePos();

                // Start eines End-Token?
                int bOffState = FALSE;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = TRUE;
                    nNextCh = GetNextChar();
                }
                else if( '!' == nNextCh )
                {
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                // jetzt die Buchstaben danach lesen
                while( (HTML_ISALPHA(nNextCh) || '-'==nNextCh) &&
                       IsParserWorking() && sTmpBuffer.getLength() < MAX_LEN )
                {
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                String aTok( sTmpBuffer.getStr(),
                             (sal_uInt32)sTmpBuffer.getLength() );
                aTok.ToUpperAscii();
                BOOL bDone = FALSE;
                if( bReadScript || aEndToken.Len() )
                {
                    if( !bReadComment )
                    {
                        if( aTok.CompareToAscii( sHTML_comment, 3UL )
                                == COMPARE_EQUAL )
                        {
                            bReadComment = TRUE;
                        }
                        else
                        {
                            // ein Script muss mit "</SCRIPT>" aufhoehren, wobei
                            // wir es mit dem ">" aus sicherheitsgruenden
                            // erstmal nicht so genau nehmen
                            bDone = bOffState && // '>'==nNextCh &&
                            COMPARE_EQUAL == ( bReadScript
                                ? aTok.CompareToAscii(sHTML_script)
                                : aTok.CompareTo(aEndToken) );
                        }
                    }
                    if( bReadComment && '>'==nNextCh && aTok.Len() >= 2UL &&
                        aTok.Copy( aTok.Len()-2UL ).EqualsAscii( "--" ) )
                    {
                        // hier ist ein Kommentar der Art <!-----> zuende
                        bReadComment = FALSE;
                    }
                }
                else
                {
                    // ein Style-Sheet kann mit </STYLE>, </HEAD> oder
                    // <BODY> aughoehren
                    if( bOffState )
                        bDone = aTok.CompareToAscii(sHTML_style)
                                    == COMPARE_EQUAL ||
                                aTok.CompareToAscii(sHTML_head)
                                    == COMPARE_EQUAL;
                    else
                        bDone =
                            aTok.CompareToAscii(sHTML_body) == COMPARE_EQUAL;
                }

                if( bDone )
                {
                    // das war's, jetzt muessen wir gegebenenfalls den
                    // bisher gelesenen String zurueckgeben und dnach normal
                    // weitermachen

                    bWeiter = FALSE;

                    // nToken==0 heisst, dass _GetNextToken gleich weiterliest
                    if( !aToken.Len() && (bReadStyle || bReadScript) )
                    {
                        // wir koennen sofort die Umgebung beeden und
                        // das End-Token parsen
                        bReadScript = FALSE;
                        bReadStyle = FALSE;
                        aEndToken.Erase();
                        nToken = 0;
                    }
                    else
                    {
                        // wir muessen bReadScript/bReadStyle noch am
                        // Leben lassen und koennen erst beim naechsten
                        // mal das End-Token Parsen
                        bEndTokenFound = TRUE;
                    }

                    // jetzt fahren wir im Stream auf das '<' zurueck
                    rInput.Seek( nStreamPos );
                    SetLineNr( nLineNr );
                    SetLinePos( nLinePos );
                    nNextCh = '<';

                    // den String wollen wir nicht an das Token haengen
                    sTmpBuffer.setLength( 0L );
                }
                else
                {
                    // "</" merken, alles andere steht noch im buffer
                    aToken += (sal_Unicode)'<';
                    if( bOffState )
                        aToken += (sal_Unicode)'/';

                    bNextCh = FALSE;
                }
            }
            break;
        case '-':
            sTmpBuffer.append( nNextCh );
            if( bReadComment )
            {
                BOOL bTwoMinus = FALSE;
                nNextCh = GetNextChar();
                while( '-' == nNextCh && IsParserWorking() )
                {
                    bTwoMinus = TRUE;

                    if( MAX_LEN == sTmpBuffer.getLength() )
                        aToken += String(sTmpBuffer.makeStringAndClear());
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                if( '>' == nNextCh && IsParserWorking() && bTwoMinus )
                    bReadComment = FALSE;

                bNextCh = FALSE;
            }
            break;

        case '\r':
            // \r\n? beendet das aktuelle Text-Token (auch wenn es leer ist)
            nNextCh = GetNextChar();
            if( nNextCh=='\n' )
                nNextCh = GetNextChar();
            bWeiter = FALSE;
            break;
        case '\n':
            // \n beendet das aktuelle Text-Token (auch wenn es leer ist)
            nNextCh = GetNextChar();
            bWeiter = FALSE;
            break;
        case sal_Unicode(EOF):
            // eof beendet das aktuelle Text-Token und tut so, als ob
            // ein End-Token gelesen wurde
            if( rInput.IsEof() )
            {
                bWeiter = FALSE;
                if( aToken.Len() || sTmpBuffer.getLength() )
                {
                    bEndTokenFound = TRUE;
                }
                else
                {
                    bReadScript = FALSE;
                    bReadStyle = FALSE;
                    aEndToken.Erase();
                    nToken = 0;
                }
                break;
            }
            // kein break
        default:
            // alle anderen Zeichen landen im Buffer
            sTmpBuffer.append( nNextCh );
            break;
        }

        if( (!bWeiter && sTmpBuffer.getLength() > 0L) ||
            MAX_LEN == sTmpBuffer.getLength() )
            aToken += String(sTmpBuffer.makeStringAndClear());

        if( bWeiter && bNextCh )
            nNextCh = GetNextChar();
    }

    if( IsParserWorking() )
        SaveState( 0 );
    else
        nToken = 0;

    return nToken;
}

// scanne das naechste Token,
int __EXPORT HTMLParser::_GetNextToken()
{
    int nRet = 0;
    sSaveToken.Erase();

    // die Optionen loeschen
    if( pOptions->Count() )
        pOptions->DeleteAndDestroy( 0, pOptions->Count() );

    if( !IsParserWorking() )        // wenn schon Fehler, dann nicht weiter!
        return 0;

    BOOL bReadNextCharSave = bReadNextChar;
    if( bReadNextChar )
    {
        DBG_ASSERT( !bEndTokenFound,
                    "</SCRIPT> gelesen und trotzdem noch ein Zeichen lesen?" );
        nNextCh = GetNextChar();
        if( !IsParserWorking() )        // wenn schon Fehler, dann nicht weiter!
            return 0;
        bReadNextChar = FALSE;
    }

    if( bReadScript || bReadStyle || aEndToken.Len() )
    {
        nRet = _GetNextRawToken();
        if( nRet || !IsParserWorking() )
            return nRet;
    }

    do {
        int bNextCh = TRUE;
        switch( nNextCh )
        {
        case '<':
            {
                ULONG nStreamPos = rInput.Tell();
                ULONG nLineNr = GetLineNr();
                ULONG nLinePos = GetLinePos();

                int bOffState = FALSE;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = TRUE;
                    nNextCh = GetNextChar();
                }
                if( HTML_ISALPHA( nNextCh ) || '!'==nNextCh ) // fix #26984#
                {
                    ::rtl::OUStringBuffer sTmpBuffer;
                    do {
                        sTmpBuffer.append( nNextCh );
                        if( MAX_LEN == sTmpBuffer.getLength() )
                            aToken += String(sTmpBuffer.makeStringAndClear());
                        nNextCh = GetNextChar();
                    } while( '>' != nNextCh && !HTML_ISSPACE( nNextCh ) &&
                             IsParserWorking() && !rInput.IsEof() );

                    if( sTmpBuffer.getLength() )
                        aToken += String(sTmpBuffer.makeStringAndClear());

                    // Blanks ueberlesen
                    while( HTML_ISSPACE( nNextCh ) && IsParserWorking() )
                        nNextCh = GetNextChar();

                    if( !IsParserWorking() )
                    {
                        if( SVPAR_PENDING == eState )
                            bReadNextChar = bReadNextCharSave;
                        break;
                    }

                    // suche das Token in der Tabelle:
                    sSaveToken = aToken;
                    aToken.ToUpperAscii();
                    if( 0 == (nRet = GetHTMLToken( aToken )) )
                        // Unknown Control
                        nRet = HTML_UNKNOWNCONTROL_ON;

                    // Wenn es ein Token zum ausschalten ist ...
                    if( bOffState )
                    {
                         if( HTML_TOKEN_ONOFF & nRet )
                         {
                            // und es ein Off-Token gibt, das daraus machen
                            ++nRet;
                         }
                         else if( HTML_LINEBREAK!=nRet )
                         {
                            // und es kein Off-Token gibt, ein unbekanntes
                            // Token daraus machen (ausser </BR>, das wird
                            // wie <BR> behandelt
                            nRet = HTML_UNKNOWNCONTROL_OFF;
                         }
                    }

                    if( nRet == HTML_COMMENT )
                    {
                        // fix: sSaveToken wegen Gross-/Kleinschreibung
                        // als Anfang des Kommentars benutzen und ein
                        // Space anhaengen.
                        aToken = sSaveToken;
                        if( '>'!=nNextCh )
                            aToken += (sal_Unicode)' ';
                        ULONG nCStreamPos = 0UL;
                        ULONG nCLineNr, nCLinePos;
                        sal_uInt32 nCStrLen;

                        BOOL bDone = FALSE;
                        // bis zum schliessenden --> lesen. wenn keins gefunden
                        // wurde beim der ersten > wieder aufsetzen
                        while( !bDone && !rInput.IsEof() && IsParserWorking() )
                        {
                            if( '>'==nNextCh )
                            {
                                if( !nCStreamPos )
                                {
                                    nCStreamPos = rInput.Tell();
                                    nCStrLen = aToken.Len();
                                    nCLineNr = GetLineNr();
                                    nCLinePos = GetLinePos();
                                }
                                bDone = aToken.Len() >= 2UL &&
                                        aToken.Copy(aToken.Len()-2UL,2UL).
                                                        EqualsAscii( "--" );
                                if( !bDone )
                                aToken += nNextCh;
                            }
                            else
                                aToken += nNextCh;
                            if( !bDone )
                                nNextCh = GetNextChar();
                        }
                        if( !bDone && IsParserWorking() && nCStreamPos )
                        {
                            rInput.Seek( nCStreamPos );
                            SetLineNr( nCLineNr );
                            SetLinePos( nCLinePos );
                            aToken.Erase( nCStrLen );
                            nNextCh = '>';
                        }
                    }
                    else
                    {
                        // den TokenString koennen wir jetzt verwerfen
                        aToken.Erase();
                    }

                    // dann lesen wir mal alles bis zur schliessenden '>'
                    if( '>' != nNextCh && IsParserWorking() )
                    {
                        ScanText( '>' );
                        if( sal_Unicode(EOF) == nNextCh && rInput.IsEof() )
                        {
                            // zurueck hinter die < gehen  und dort neu
                            // aufsetzen, das < als Text zurueckgeben
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );

                            aToken = '<';
                            nRet = HTML_TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = FALSE;
                            break;
                        }
                    }
                    if( SVPAR_PENDING == eState )
                        bReadNextChar = bReadNextCharSave;
                }
                else
                {
                    if( bOffState )
                    {
                        // einfach alles wegschmeissen
                        ScanText( '>' );
                        if( sal_Unicode(EOF) == nNextCh && rInput.IsEof() )
                        {
                            // zurueck hinter die < gehen  und dort neu
                            // aufsetzen, das < als Text zurueckgeben
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );

                            aToken = '<';
                            nRet = HTML_TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = FALSE;
                            break;
                        }
                        if( SVPAR_PENDING == eState )
                            bReadNextChar = bReadNextCharSave;
                        aToken.Erase();
                    }
                    else if( '%' == nNextCh )
                    {
                        nRet = HTML_UNKNOWNCONTROL_ON;

                        ULONG nCStreamPos = rInput.Tell();
                        ULONG nCLineNr = GetLineNr(), nCLinePos = GetLinePos();

                        BOOL bDone = FALSE;
                        // bis zum schliessenden %> lesen. wenn keins gefunden
                        // wurde beim der ersten > wieder aufsetzen
                        while( !bDone && !rInput.IsEof() && IsParserWorking() )
                        {
                            bDone = '>'==nNextCh && aToken.Len() >= 1UL &&
                                    '%' == aToken.GetChar( aToken.Len()-1UL );
                            if( !bDone )
                            {
                                aToken += nNextCh;
                                nNextCh = GetNextChar();
                            }
                        }
                        if( !bDone && IsParserWorking() )
                        {
                            rInput.Seek( nCStreamPos );
                            SetLineNr( nCLineNr );
                            SetLinePos( nCLinePos );
                            aToken.AssignAscii( "<%", 2UL );
                            nRet = HTML_TEXTTOKEN;
                            break;
                        }
                        if( IsParserWorking() )
                        {
                            sSaveToken = aToken;
                            aToken.Erase();
                        }
                    }
                    else
                    {
                        aToken = '<';
                        nRet = HTML_TEXTTOKEN;
                        bNextCh = FALSE;
                        break;
                    }
                }

                if( IsParserWorking() )
                {
                    bNextCh = '>' == nNextCh;
                    switch( nRet )
                    {
                    case HTML_TEXTAREA_ON:
                        bReadTextArea = TRUE;
                        break;
                    case HTML_TEXTAREA_OFF:
                        bReadTextArea = FALSE;
                        break;
                    case HTML_SCRIPT_ON:
                        if( !bReadTextArea )
                            bReadScript = TRUE;
                        break;
                    case HTML_SCRIPT_OFF:
                        if( !bReadTextArea )
                        {
                            bReadScript = FALSE;
                            // JavaScript kann den Stream veraendern
                            // also muss das letzte Zeichen nochmals
                            // gelesen werden
                            bReadNextChar = TRUE;
                            bNextCh = FALSE;
                        }
                        break;

                    case HTML_STYLE_ON:
                        bReadStyle = TRUE;
                        break;
                    case HTML_STYLE_OFF:
                        bReadStyle = FALSE;
                        break;
                    }

                }
            }
            break;

        case sal_Unicode(EOF):
            if( rInput.IsEof() )
            {
                eState = SVPAR_ACCEPTED;
                nRet = nNextCh;
            }
            else
            {
                // normalen Text lesen
                goto scan_text;
            }
            break;

        case '\f':
            // Form-Feeds werden jetzt extra nach oben gereicht
            nRet = HTML_LINEFEEDCHAR; // !!! eigentlich FORMFEEDCHAR
            break;

        case '\n':
        case '\r':
            if( bReadListing || bReadXMP || bReadPRE || bReadTextArea )
            {
                sal_Unicode c = GetNextChar();
                if( ( '\n' != nNextCh || '\r' != c ) &&
                    ( '\r' != nNextCh || '\n' != c ) )
                {
                    bNextCh = FALSE;
                    nNextCh = c;
                }
                nRet = HTML_NEWPARA;
                break;
            }
            // kein break !
        case '\t':
            if( bReadPRE )
            {
                nRet = HTML_TABCHAR;
                break;
            }
            // kein break !
        case ' ':
            // kein break !
        default:

scan_text:
            // es folgt "normaler" Text
            nRet = ScanText();
            bNextCh = 0 == aToken.Len();

            // der Text sollte noch verarbeitet werden
            if( !bNextCh && eState == SVPAR_PENDING )
            {
                eState = SVPAR_WORKING;
                bReadNextChar = TRUE;
            }

            break;
        }

        if( bNextCh && SVPAR_WORKING == eState )
        {
            nNextCh = GetNextChar();
            if( SVPAR_PENDING == eState && nRet && HTML_TEXTTOKEN != nRet )
            {
                bReadNextChar = TRUE;
                eState = SVPAR_WORKING;
            }
        }

    } while( !nRet && SVPAR_WORKING == eState );

    if( SVPAR_PENDING == eState )
        nRet = -1;      // irgendwas ungueltiges

    return nRet;
}

void HTMLParser::UnescapeToken()
{
    sal_uInt32 nPos=0UL;

    BOOL bEscape = FALSE;
    while( nPos < aToken.Len() )
    {
        BOOL bOldEscape = bEscape;
        bEscape = FALSE;
        if( '\\'==aToken.GetChar(nPos) && !bOldEscape )
        {
            aToken.Erase( nPos, 1UL );
            bEscape = TRUE;
        }
        else
        {
            nPos++;
        }
    }
}

// hole die Optionen
const HTMLOptions *HTMLParser::GetOptions( USHORT *pNoConvertToken ) const
{
    // wenn die Option fuer das aktuelle Token schon einmal
    // geholt wurden, geben wir sie noch einmal zurueck
    if( pOptions->Count() )
        return pOptions;

    sal_uInt32 nPos = 0UL;
    while( nPos < aToken.Len() )
    {
        // ein Zeichen ? Dann faengt hier eine Option an
        if( HTML_ISALPHA( aToken.GetChar(nPos) ) )
        {
            USHORT nToken;
            String aValue;
            sal_uInt32 nStt = nPos;
            register sal_Unicode c;

            // Eigentlich sind hier nur ganz bestimmte Zeichen erlaubt.
            // Netscape achtet aber nur auf "=" und Leerzeichen (siehe
            // Mozilla: PA_FetchRequestedNameValues in
            // lipparse/pa_mdl.c
//          while( nPos < aToken.Len() &&
//                  ( '-'==(c=aToken[nPos]) || isalnum(c) || '.'==c || '_'==c) )
            while( nPos < aToken.Len() && '=' != (c=aToken.GetChar(nPos)) &&
                   HTML_ISPRINTABLE(c) && !HTML_ISSPACE(c) )
                nPos++;

            String sName( aToken.Copy( nStt, nPos-nStt ) );

//JP 23.03.97: die PlugIns wollen die TokenName im "Original" haben
//              also nur fuers Suchen in UpperCase wandeln
            String sNameUpperCase( sName );
            sNameUpperCase.ToUpperAscii();

            nToken = GetHTMLOption( sNameUpperCase ); // der Name ist fertig
            DBG_ASSERTWARNING( nToken!=HTML_O_UNKNOWN,
                        "GetOption: unbekannte HTML-Option" );
            BOOL bStripCRLF = (nToken < HTML_OPTION_SCRIPT_START ||
                               nToken >= HTML_OPTION_SCRIPT_END) &&
                              (!pNoConvertToken || nToken != *pNoConvertToken);

            while( nPos < aToken.Len() &&
                   ( !HTML_ISPRINTABLE( (c=aToken.GetChar(nPos)) ) ||
                     HTML_ISSPACE(c) ) )
                nPos++;

            // hat die Option auch einen Wert?
            if( nPos!=aToken.Len() && '='==c )
            {
                nPos++;

                while( nPos < aToken.Len() &&
                        ( !HTML_ISPRINTABLE( (c=aToken.GetChar(nPos)) ) ||
                          ' '==c || '\t'==c || '\r'==c || '\n'==c ) )
                    nPos++;

                if( nPos != aToken.Len() )
                {
                    sal_uInt32 nLen = 0UL;
                    nStt = nPos;
                    if( ('"'==c) || ('\'')==c )
                    {
                        sal_Unicode cEnd = c;
                        nPos++; nStt++;
                        BOOL bDone = FALSE;
                        BOOL bEscape = FALSE;
                        while( nPos < aToken.Len() && !bDone )
                        {
                            BOOL bOldEscape = bEscape;
                            bEscape = FALSE;
                            c = aToken.GetChar(nPos);
                            switch( c )
                            {
                            case '\r':
                            case '\n':
                                if( bStripCRLF )
                                    ((String &)aToken).Erase( nPos, 1UL );
                                else
                                    nPos++, nLen++;
                                break;
                            case '\\':
                                if( bOldEscape )
                                {
                                    nPos++, nLen++;
                                }
                                else
                                {
                                    ((String &)aToken).Erase( nPos, 1UL );
                                    bEscape = TRUE;
                                }
                                break;
                            case '"':
                            case '\'':
                                bDone = !bOldEscape && c==cEnd;
                                if( !bDone )
                                    nPos++, nLen++;
                                break;
                            default:
                                nPos++, nLen++;
                                break;
                            }
                        }
                        if( nPos!=aToken.Len() )
                            nPos++;
                    }
                    else
                    {
                        // hier sind wir etwas laxer als der
                        // Standard und erlauben alles druckbare
                        BOOL bEscape = FALSE;
                        BOOL bDone = FALSE;
                        while( nPos < aToken.Len() && !bDone )
                        {
                            BOOL bOldEscape = bEscape;
                            bEscape = FALSE;
                            sal_Unicode c = aToken.GetChar(nPos);
                            switch( c )
                            {
                            case ' ':
                                bDone = !bOldEscape;
                                if( !bDone )
                                    nPos++, nLen++;
                                break;

                            case '\t':
                            case '\r':
                            case '\n':
                                bDone = TRUE;
                                break;

                            case '\\':
                                if( bOldEscape )
                                {
                                    nPos++, nLen++;
                                }
                                else
                                {
                                    ((String &)aToken).Erase( nPos, 1UL );
                                    bEscape = TRUE;
                                }
                                break;

                            default:
                                if( HTML_ISPRINTABLE( c ) )
                                    nPos++, nLen++;
                                else
                                    bDone = TRUE;
                                break;
                            }
                        }
                    }

                    if( nLen )
                        aValue = aToken.Copy( nStt, nLen );
                }
            }

            // Wir kennen das Token und koennen es Speichern
            HTMLOption *pOption =
                new HTMLOption( nToken, sName, aValue );

            pOptions->Insert( pOption, pOptions->Count() );

        }
        else
            // white space un unerwartete Zeichen ignorieren wie
            nPos++;
    }

    return pOptions;
}

int HTMLParser::FilterPRE( int nToken )
{
    switch( nToken )
    {
#ifdef HTML_BEHAVIOUR
    // diese werden laut Definition zu LFs
    case HTML_PARABREAK_ON:
    case HTML_LINEBREAK:
        nToken = HTML_NEWPARA;
#else
    // in Netscape zeigen sie aber nur in nicht-leeren Absaetzen Wirkung
    case HTML_PARABREAK_ON:
        nToken = HTML_LINEBREAK;
    case HTML_LINEBREAK:
#endif
    case HTML_NEWPARA:
        nPre_LinePos = 0UL;
        if( bPre_IgnoreNewPara )
            nToken = 0;
        break;

    case HTML_TABCHAR:
        {
            sal_uInt32 nSpaces = 8UL - (nPre_LinePos % 8UL);
            DBG_ASSERT( !aToken.Len(), "Wieso ist das Token nicht leer?" );
            aToken.Expand( nSpaces, ' ' );
            nPre_LinePos += nSpaces;
            nToken = HTML_TEXTTOKEN;
        }
        break;
    // diese bleiben erhalten
    case HTML_TEXTTOKEN:
        nPre_LinePos += aToken.Len();
        break;

    case HTML_SELECT_ON:
    case HTML_SELECT_OFF:
    case HTML_BODY_ON:
    case HTML_FORM_ON:
    case HTML_FORM_OFF:
    case HTML_INPUT:
    case HTML_OPTION:
    case HTML_TEXTAREA_ON:
    case HTML_TEXTAREA_OFF:

    case HTML_IMAGE:
    case HTML_APPLET_ON:
    case HTML_APPLET_OFF:
    case HTML_PARAM:
    case HTML_EMBED:

    case HTML_HEAD1_ON:
    case HTML_HEAD1_OFF:
    case HTML_HEAD2_ON:
    case HTML_HEAD2_OFF:
    case HTML_HEAD3_ON:
    case HTML_HEAD3_OFF:
    case HTML_HEAD4_ON:
    case HTML_HEAD4_OFF:
    case HTML_HEAD5_ON:
    case HTML_HEAD5_OFF:
    case HTML_HEAD6_ON:
    case HTML_HEAD6_OFF:
    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE_OFF:
    case HTML_ADDRESS_ON:
    case HTML_ADDRESS_OFF:
    case HTML_HORZRULE:

    case HTML_CENTER_ON:
    case HTML_CENTER_OFF:
    case HTML_DIVISION_ON:
    case HTML_DIVISION_OFF:

    case HTML_SCRIPT_ON:
    case HTML_SCRIPT_OFF:
    case HTML_RAWDATA:

    case HTML_TABLE_ON:
    case HTML_TABLE_OFF:
    case HTML_CAPTION_ON:
    case HTML_CAPTION_OFF:
    case HTML_COLGROUP_ON:
    case HTML_COLGROUP_OFF:
    case HTML_COL_ON:
    case HTML_COL_OFF:
    case HTML_THEAD_ON:
    case HTML_THEAD_OFF:
    case HTML_TFOOT_ON:
    case HTML_TFOOT_OFF:
    case HTML_TBODY_ON:
    case HTML_TBODY_OFF:
    case HTML_TABLEROW_ON:
    case HTML_TABLEROW_OFF:
    case HTML_TABLEDATA_ON:
    case HTML_TABLEDATA_OFF:
    case HTML_TABLEHEADER_ON:
    case HTML_TABLEHEADER_OFF:

    case HTML_ANCHOR_ON:
    case HTML_ANCHOR_OFF:
    case HTML_BOLD_ON:
    case HTML_BOLD_OFF:
    case HTML_ITALIC_ON:
    case HTML_ITALIC_OFF:
    case HTML_UNDERLINE_ON:
    case HTML_UNDERLINE_OFF:
    case HTML_BASEFONT_ON:
    case HTML_BASEFONT_OFF:
    case HTML_FONT_ON:
    case HTML_FONT_OFF:
    case HTML_BLINK_ON:
    case HTML_BLINK_OFF:
    case HTML_SPAN_ON:
    case HTML_SPAN_OFF:
        break;

    // der Rest wird als unbekanntes Token behandelt
    default:
        if( nToken )
        {
            nToken =
                ( ((HTML_TOKEN_ONOFF & nToken) && (1 & nToken))
                    ? HTML_UNKNOWNCONTROL_OFF
                    : HTML_UNKNOWNCONTROL_ON );
        }
        break;
    }

    bPre_IgnoreNewPara = FALSE;

    return nToken;
}

int HTMLParser::FilterXMP( int nToken )
{
    switch( nToken )
    {
    case HTML_NEWPARA:
        if( bPre_IgnoreNewPara )
            nToken = 0;
    case HTML_TEXTTOKEN:
    case HTML_NONBREAKSPACE:
    case HTML_SOFTHYPH:
        break;              // bleiben erhalten

    default:
        if( nToken )
        {
            if( (HTML_TOKEN_ONOFF & nToken) && (1 & nToken) )
            {
                sSaveToken.Insert( '<', 0UL );
                sSaveToken.Insert( '/', 1UL );
            }
            else
                sSaveToken.Insert( '<', 0UL );
            if( aToken.Len() )
            {
                UnescapeToken();
                sSaveToken += (sal_Unicode)' ';
                aToken.Insert( sSaveToken, 0UL );
            }
            else
                aToken = sSaveToken;
            aToken += (sal_Unicode)'>';
            nToken = HTML_TEXTTOKEN;
        }
        break;
    }

    bPre_IgnoreNewPara = FALSE;

    return nToken;
}

int HTMLParser::FilterListing( int nToken )
{
    switch( nToken )
    {
    case HTML_NEWPARA:
        if( bPre_IgnoreNewPara )
            nToken = 0;
    case HTML_TEXTTOKEN:
    case HTML_NONBREAKSPACE:
    case HTML_SOFTHYPH:
        break;      // bleiben erhalten

    default:
        if( nToken )
        {
            nToken =
                ( ((HTML_TOKEN_ONOFF & nToken) && (1 & nToken))
                    ? HTML_UNKNOWNCONTROL_OFF
                    : HTML_UNKNOWNCONTROL_ON );
        }
        break;
    }

    bPre_IgnoreNewPara = FALSE;

    return nToken;
}

FASTBOOL HTMLParser::IsHTMLFormat( const sal_Char* pHeader,
                                   BOOL bSwitchToUCS2,
                                   rtl_TextEncoding eEnc )
{
    // Einer der folgenden regulaeren Ausdrucke muss sich auf den String
    // anwenden lassen, damit das Dok ein HTML-Dokument ist.
    //
    // ^[^<]*<[^ \t]*[> \t]
    //        -------
    // ^<!
    //
    // wobei der unterstrichene Teilausdruck einem HTML-Token
    // ensprechen muss

    ByteString sCmp;
    BOOL bUCS2B = FALSE;
    if( bSwitchToUCS2 )
    {
        if( 0xfeU == (sal_Char)pHeader[0] &&
            0xffU == (sal_Char)pHeader[1] )
        {
            eEnc = RTL_TEXTENCODING_UCS2;
            bUCS2B = TRUE;
        }
        else if( 0xffU == (sal_Char)pHeader[0] &&
                 0xfeU == (sal_Char)pHeader[1] )
        {
            eEnc = RTL_TEXTENCODING_UCS2;
        }
    }
    if( RTL_TEXTENCODING_UCS2 == eEnc &&
        (0xfe == (sal_Char)pHeader[0] && 0xff == (sal_Char)pHeader[1]) ||
        (0xff == (sal_Char)pHeader[0] && 0xfe == (sal_Char)pHeader[1]) )
    {
        if( 0xfe == (sal_Char)pHeader[0] )
            bUCS2B = TRUE;

        sal_uInt32 nLen;
        for( nLen = 2UL;
             pHeader[nLen] != 0 || pHeader[nLen+1UL] != 0;
             nLen+=2 )
            ;

        ::rtl::OStringBuffer sTmp( (nLen - 2UL)/2UL );
        for( sal_uInt32 nPos = 2UL; nPos < nLen; nPos += 2UL )
        {
            sal_Unicode cUC;
            if( bUCS2B )
                cUC = (sal_Unicode(pHeader[nPos]) << 8) | pHeader[nPos+1UL];
            else
                cUC = (sal_Unicode(pHeader[nPos+1UL]) << 8) | pHeader[nPos];
            if( 0U == cUC )
                break;

            sTmp.append( cUC < 256U ? (sal_Char)cUC : '.' );
        }
        sCmp = ByteString( sTmp.makeStringAndClear() );
    }
    else
    {
        sCmp = (sal_Char *)pHeader;
    }

    sCmp.ToUpperAscii();

    // Ein HTML-Dokument muss in der ersten Zeile ein '<' besitzen
    sal_uInt32 nStart = sCmp.Search( '<' );
    if( STRING_NOTFOUND  == nStart )
        return FALSE;
    nStart++;

    // danach duerfen beliebige andere Zeichen bis zu einem blank oder
    // '>' kommen
    sal_Char c;
    sal_uInt32 nPos;
    for( nPos = nStart; nPos<sCmp.Len(); nPos++ )
    {
        if( '>'==(c=sCmp.GetChar(nPos)) || HTML_ISSPACE(c) )
            break;
    }

    // wenn das Dokeument hinter dem < aufhoert ist es wohl kein HTML
    if( nPos==nStart )
        return FALSE;

    // die Zeichenkette nach dem '<' muss ausserdem ein bekanntes
    // HTML Token sein. Damit die Ausgabe eines DOS-dir-Befehls nicht
    // als HTML interpretiert wird, wird ein <DIR> jedoch nicht als HTML
    // interpretiert.
    String sTest( sCmp.Copy( nStart, nPos-nStart ), RTL_TEXTENCODING_ASCII_US );
    int nTok = GetHTMLToken( sTest );
    if( 0 != nTok && HTML_DIRLIST_ON != nTok )
        return TRUE;

    // oder es handelt sich um ein "<!" ganz am Anfang der Datei (fix #27092#)
    if( nStart == 1UL && '!' == sCmp.GetChar( 1UL ) )
        return TRUE;

    // oder wir finden irgendwo ein <HTML> in den ersten 80 Zeichen
    nStart = sCmp.Search( sHTML_html );
    if( nStart!=STRING_NOTFOUND &&
        nStart>0UL && '<'==sCmp.GetChar(nStart-1UL) &&
        nStart+4UL < sCmp.Len() && '>'==sCmp.GetChar(nStart+4UL) )
        return TRUE;

    // sonst ist es wohl doch eher kein HTML-Dokument
    return FALSE;
}

BOOL HTMLParser::InternalImgToPrivateURL( String& rURL )
{
    if( rURL.Len() < 19UL || 'i' != rURL.GetChar(0UL) ||
        rURL.CompareToAscii( sHTML_internal_gopher, 9UL ) != COMPARE_EQUAL )
        return FALSE;

    BOOL bFound = FALSE;

    if( rURL.CompareToAscii( sHTML_internal_gopher,16UL) == COMPARE_EQUAL )
    {
        String aName( rURL.Copy(16UL) );
        switch( aName.GetChar(0UL) )
        {
        case 'b':
            bFound = aName.EqualsAscii( sHTML_INT_GOPHER_binary );
            break;
        case 'i':
            bFound = aName.EqualsAscii( sHTML_INT_GOPHER_image ) ||
                     aName.EqualsAscii( sHTML_INT_GOPHER_index );
            break;
        case 'm':
            bFound = aName.EqualsAscii( sHTML_INT_GOPHER_menu ) ||
                     aName.EqualsAscii( sHTML_INT_GOPHER_movie );
            break;
        case 's':
            bFound = aName.EqualsAscii( sHTML_INT_GOPHER_sound );
            break;
        case 't':
            bFound = aName.EqualsAscii( sHTML_INT_GOPHER_telnet ) ||
                     aName.EqualsAscii( sHTML_INT_GOPHER_text );
            break;
        case 'u':
            bFound = aName.EqualsAscii( sHTML_INT_GOPHER_unknown );
            break;
        }
    }
    else if( rURL.CompareToAscii( sHTML_internal_icon,14UL) == COMPARE_EQUAL )
    {
        String aName( rURL.Copy(14UL) );
        switch( aName.GetChar(0UL) )
        {
        case 'b':
            bFound = aName.EqualsAscii( sHTML_INT_ICON_baddata );
            break;
        case 'd':
            bFound = aName.EqualsAscii( sHTML_INT_ICON_delayed );
            break;
        case 'e':
            bFound = aName.EqualsAscii( sHTML_INT_ICON_embed );
            break;
        case 'i':
            bFound = aName.EqualsAscii( sHTML_INT_ICON_insecure );
            break;
        case 'n':
            bFound = aName.EqualsAscii( sHTML_INT_ICON_notfound );
            break;
        }
    }
    if( bFound )
    {
        String sTmp ( rURL );
        rURL.AssignAscii( sHTML_private_image );
        rURL.Append( sTmp );
    }

    return bFound;
}


#ifdef USED
void HTMLParser::SaveState( int nToken )
{
    SvParser::SaveState( nToken );
}

void HTMLParser::RestoreState()
{
    SvParser::RestoreState();
}
#endif

