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
#include "precompiled_svtools.hxx"

#include <ctype.h>
#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/color.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#endif

#include <tools/tenccvt.hxx>
#include <tools/datetime.hxx>
#include <svl/inettype.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <svtools/parhtml.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>


using namespace ::com::sun::star;


const sal_Int32 MAX_LEN( 1024L );
//static sal_Unicode sTmpBuffer[ MAX_LEN+1 ];
const sal_Int32 MAX_MACRO_LEN( 1024 );

const sal_Int32 MAX_ENTITY_LEN( 8L );

/*  */

// Tabellen zum Umwandeln von Options-Werten in Strings

// <INPUT TYPE=xxx>
static HTMLOptionEnum const aInputTypeOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_IT_text,      HTML_IT_TEXT        },
    { OOO_STRING_SVTOOLS_HTML_IT_password,  HTML_IT_PASSWORD    },
    { OOO_STRING_SVTOOLS_HTML_IT_checkbox,  HTML_IT_CHECKBOX    },
    { OOO_STRING_SVTOOLS_HTML_IT_radio,     HTML_IT_RADIO       },
    { OOO_STRING_SVTOOLS_HTML_IT_range,     HTML_IT_RANGE       },
    { OOO_STRING_SVTOOLS_HTML_IT_scribble,  HTML_IT_SCRIBBLE    },
    { OOO_STRING_SVTOOLS_HTML_IT_file,      HTML_IT_FILE        },
    { OOO_STRING_SVTOOLS_HTML_IT_hidden,    HTML_IT_HIDDEN      },
    { OOO_STRING_SVTOOLS_HTML_IT_submit,    HTML_IT_SUBMIT      },
    { OOO_STRING_SVTOOLS_HTML_IT_image,     HTML_IT_IMAGE       },
    { OOO_STRING_SVTOOLS_HTML_IT_reset,     HTML_IT_RESET       },
    { OOO_STRING_SVTOOLS_HTML_IT_button,    HTML_IT_BUTTON      },
    { 0,                    0                   }
};

// <TABLE FRAME=xxx>
static HTMLOptionEnum const aTableFrameOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_TF_void,  HTML_TF_VOID    },
    { OOO_STRING_SVTOOLS_HTML_TF_above, HTML_TF_ABOVE   },
    { OOO_STRING_SVTOOLS_HTML_TF_below, HTML_TF_BELOW   },
    { OOO_STRING_SVTOOLS_HTML_TF_hsides,    HTML_TF_HSIDES  },
    { OOO_STRING_SVTOOLS_HTML_TF_lhs,       HTML_TF_LHS     },
    { OOO_STRING_SVTOOLS_HTML_TF_rhs,       HTML_TF_RHS     },
    { OOO_STRING_SVTOOLS_HTML_TF_vsides,    HTML_TF_VSIDES  },
    { OOO_STRING_SVTOOLS_HTML_TF_box,       HTML_TF_BOX     },
    { OOO_STRING_SVTOOLS_HTML_TF_border,    HTML_TF_BOX     },
    { 0,                0               }
};

// <TABLE RULES=xxx>
static HTMLOptionEnum const aTableRulesOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_TR_none,  HTML_TR_NONE    },
    { OOO_STRING_SVTOOLS_HTML_TR_groups,    HTML_TR_GROUPS  },
    { OOO_STRING_SVTOOLS_HTML_TR_rows,  HTML_TR_ROWS    },
    { OOO_STRING_SVTOOLS_HTML_TR_cols,  HTML_TR_COLS    },
    { OOO_STRING_SVTOOLS_HTML_TR_all,       HTML_TR_ALL     },
    { 0,                0               }
};


SV_IMPL_PTRARR(HTMLOptions,HTMLOptionPtr)

/*  */

sal_uInt16 HTMLOption::GetEnum( const HTMLOptionEnum *pOptEnums, sal_uInt16 nDflt ) const
{
    sal_uInt16 nValue = nDflt;

    while( pOptEnums->pName )
        if( aValue.EqualsIgnoreCaseAscii( pOptEnums->pName ) )
            break;
        else
            pOptEnums++;

    if( pOptEnums->pName )
        nValue = pOptEnums->nValue;

    return nValue;
}

sal_Bool HTMLOption::GetEnum( sal_uInt16 &rEnum, const HTMLOptionEnum *pOptEnums ) const
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

HTMLOption::HTMLOption( sal_uInt16 nTok, const String& rToken,
                        const String& rValue )
    : aValue(rValue)
    , aToken(rToken)
    , nToken( nTok )
{
    DBG_ASSERT( nToken>=HTML_OPTION_START && nToken<HTML_OPTION_END,
        "HTMLOption: unbekanntes Token" );
}

sal_uInt32 HTMLOption::GetNumber() const
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
    return nTmp >= 0 ? (sal_uInt32)nTmp : 0;
}

sal_Int32 HTMLOption::GetSNumber() const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_NUMBER_START && nToken<HTML_OPTION_NUMBER_END) ||
                (nToken>=HTML_OPTION_CONTEXT_START && nToken<HTML_OPTION_CONTEXT_END),
        "GetSNumber: Option ist nicht numerisch" );
    String aTmp( aValue );
    aTmp.EraseLeadingChars();
    return aTmp.ToInt32();
}

void HTMLOption::GetNumbers( SvULongs &rLongs, sal_Bool bSpaceDelim ) const
{
    if( rLongs.Count() )
        rLongs.Remove( 0, rLongs.Count() );

    if( bSpaceDelim )
    {
        // das ist ein sehr stark vereinfachter Scanner. Er sucht einfach
        // alle Tiffern aus dem String
        sal_Bool bInNum = sal_False;
        sal_uLong nNum = 0;
        for( xub_StrLen i=0; i<aValue.Len(); i++ )
        {
            register sal_Unicode c = aValue.GetChar( i );
            if( c>='0' && c<='9' )
            {
                nNum *= 10;
                nNum += (c - '0');
                bInNum = sal_True;
            }
            else if( bInNum )
            {
                rLongs.Insert( nNum, rLongs.Count() );
                bInNum = sal_False;
                nNum = 0;
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
        xub_StrLen nPos = 0;
        while( nPos < aValue.Len() )
        {
            register sal_Unicode c;
            while( nPos < aValue.Len() &&
                   ((c=aValue.GetChar(nPos)) == ' ' || c == '\t' ||
                   c == '\n' || c== '\r' ) )
                nPos++;

            if( nPos==aValue.Len() )
                rLongs.Insert( sal_uLong(0), rLongs.Count() );
            else
            {
                xub_StrLen nEnd = aValue.Search( (sal_Unicode)',', nPos );
                if( STRING_NOTFOUND==nEnd )
                {
                    sal_Int32 nTmp = aValue.Copy(nPos).ToInt32();
                    rLongs.Insert( nTmp >= 0 ? (sal_uInt32)nTmp : 0,
                                   rLongs.Count() );
                    nPos = aValue.Len();
                }
                else
                {
                    sal_Int32 nTmp =
                        aValue.Copy(nPos,nEnd-nPos).ToInt32();
                    rLongs.Insert( nTmp >= 0 ? (sal_uInt32)nTmp : 0,
                                   rLongs.Count() );
                    nPos = nEnd+1;
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
    sal_uLong nColor = ULONG_MAX;
    if( '#'!=aTmp.GetChar( 0 ) )
        nColor = GetHTMLColor( aTmp );

    if( ULONG_MAX == nColor )
    {
        nColor = 0;
        xub_StrLen nPos = 0;
        for( sal_uInt32 i=0; i<6; i++ )
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
            nColor *= 16;
            if( c >= '0' && c <= '9' )
                nColor += (c - 48);
            else if( c >= 'A' && c <= 'F' )
                nColor += (c - 55);
        }
    }

    rColor.SetRed(   (sal_uInt8)((nColor & 0x00ff0000) >> 16) );
    rColor.SetGreen( (sal_uInt8)((nColor & 0x0000ff00) >> 8));
    rColor.SetBlue(  (sal_uInt8)(nColor & 0x000000ff) );
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
        bReadComment = sal_False;
    bIsInHeader = sal_True;
    pOptions = new HTMLOptions;

    //#i76649, default to UTF-8 for HTML unless we know differently
    SetSrcEncoding(RTL_TEXTENCODING_UTF8);
}

HTMLParser::~HTMLParser()
{
    if( pOptions && pOptions->Count() )
        pOptions->DeleteAndDestroy( 0, pOptions->Count() );
    delete pOptions;
}

SvParserState HTMLParser::CallParser()
{
    eState = SVPAR_WORKING;
    nNextCh = GetNextChar();
    SaveState( 0 );

    nPre_LinePos = 0;
    bPre_IgnoreNewPara = sal_False;

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
        bIsInBody = sal_True;
    case HTML_HEAD_ON:
        bIsInHeader = HTML_HEAD_ON == nToken;
        break;

    case HTML_BODY_ON:
    case HTML_FRAMESET_ON:
        bIsInHeader = sal_False;
        bIsInBody = HTML_BODY_ON == nToken;
        break;

    case HTML_BODY_OFF:
        bIsInBody = bReadPRE = bReadListing = bReadXMP = sal_False;
        break;

    case HTML_HTML_OFF:
        nToken = 0;
        bReadPRE = bReadListing = bReadXMP = sal_False;
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
#define HTML_ISHEXDIGIT( c ) ( HTML_ISDIGIT(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') )

int HTMLParser::ScanText( const sal_Unicode cBreak )
{
    ::rtl::OUStringBuffer sTmpBuffer( MAX_LEN );
    int bContinue = sal_True;
    int bEqSignFound = sal_False;
    sal_Unicode cQuote = 0U;

    while( bContinue && IsParserWorking() )
    {
        int bNextCh = sal_True;
        switch( nNextCh )
        {
        case '&':
            bEqSignFound = sal_False;
            if( bReadXMP )
                sTmpBuffer.append( (sal_Unicode)'&' );
            else
            {
                sal_uLong nStreamPos = rInput.Tell();
                sal_uLong nLinePos = GetLinePos();

                sal_Unicode cChar = 0U;
                if( '#' == (nNextCh = GetNextChar()) )
                {
                    nNextCh = GetNextChar();
                    const sal_Bool bIsHex( 'x' == nNextCh );
                    const sal_Bool bIsDecOrHex( bIsHex || HTML_ISDIGIT(nNextCh) );
                    if ( bIsDecOrHex )
                    {
                        if ( bIsHex )
                        {
                            nNextCh = GetNextChar();
                            while ( HTML_ISHEXDIGIT(nNextCh) )
                            {
                                cChar = cChar * 16U +
                                        ( nNextCh <= '9'
                                          ? sal_Unicode( nNextCh - '0' )
                                          : ( nNextCh <= 'F'
                                              ? sal_Unicode( nNextCh - 'A' + 10 )
                                              : sal_Unicode( nNextCh - 'a' + 10 ) ) );
                                nNextCh = GetNextChar();
                            }
                        }
                        else
                        {
                            do
                            {
                                cChar = cChar * 10U + sal_Unicode( nNextCh - '0');
                                nNextCh = GetNextChar();
                            }
                            while( HTML_ISDIGIT(nNextCh) );
                        }

                        if( RTL_TEXTENCODING_DONTKNOW != eSrcEnc &&
                            RTL_TEXTENCODING_UCS2 != eSrcEnc &&
                            RTL_TEXTENCODING_UTF8 != eSrcEnc &&
                             cChar < 256 )
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
                    ::rtl::OUStringBuffer sEntityBuffer( MAX_ENTITY_LEN );
                    xub_StrLen nPos = 0L;
                    do
                    {
                        sEntityBuffer.append( nNextCh );
                        nPos++;
                        nNextCh = GetNextChar();
                    }
                    while( nPos < MAX_ENTITY_LEN && HTML_ISALNUM( nNextCh ) &&
                           !rInput.IsEof() );

                    if( IsParserWorking() && !rInput.IsEof() )
                    {
                        String sEntity( sEntityBuffer.getStr(), nPos );
                        cChar = GetHTMLCharName( sEntity );

                        // nicht gefunden ( == 0 ), dann Klartext
                        // oder ein Zeichen das als Attribut eingefuegt
                        // wird
                        if( 0U == cChar && ';' != nNextCh )
                        {
                            DBG_ASSERT( rInput.Tell() - nStreamPos ==
                                        (sal_uLong)(nPos+1L)*GetCharSize(),
                                        "UTF-8 geht hier schief" );
                            for( xub_StrLen i=nPos-1L; i>1L; i-- )
                            {
                                nNextCh = sEntityBuffer[i];
                                sEntityBuffer.setLength( i );
                                sEntity.Assign( sEntityBuffer.getStr(), i );
                                 cChar = GetHTMLCharName( sEntity );
                                if( cChar )
                                {
                                    rInput.SeekRel( -(long)
                                            ((nPos-i)*GetCharSize()) );
                                    nlLinePos -= sal_uInt32(nPos-i);
                                    nPos = i;
                                    ClearTxtConvContext();
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
                                        (sal_uLong)(nPos+1)*GetCharSize(),
                                        "Falsche Stream-Position" );
                            DBG_ASSERT( nlLinePos-nLinePos ==
                                        (sal_uLong)(nPos+1),
                                        "Falsche Zeilen-Position" );
                            rInput.Seek( nStreamPos );
                            nlLinePos = nLinePos;
                            ClearTxtConvContext();
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
                                                    (sal_uLong)(nPos+1)*GetCharSize(),
                                                    "Falsche Stream-Position" );
                                        DBG_ASSERT( nlLinePos-nLinePos ==
                                                    (sal_uLong)(nPos+1),
                                                    "Falsche Zeilen-Position" );
                                        rInput.Seek( nStreamPos );
                                        nlLinePos = nLinePos;
                                        ClearTxtConvContext();
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
                    bNextCh = sal_False;
                    break;
                }

                bNextCh = (';' == nNextCh);
                if( cBreak=='>' && (cChar=='\\' || cChar=='\'' ||
                                    cChar=='\"' || cChar==' ') )
                {
                    // ' und " mussen innerhalb von Tags mit einem
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
                        nlLinePos = nLinePos-1;
                        ClearTxtConvContext();
                        bReadNextChar = sal_True;
                    }
                    bNextCh = sal_False;
                }
            }
            break;
        case '=':
            if( '>'==cBreak && !cQuote )
                bEqSignFound = sal_True;
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
            bEqSignFound = sal_False;
            break;

        case sal_Unicode(EOF):
            if( rInput.IsEof() )
            {
// MIB 20.11.98: Das macht hier keinen Sinn, oder doch: Zumindest wird
// abc&auml;<EOF> nicht angezeigt, also lassen wir das in Zukunft.
//              if( '>' != cBreak )
//                  eState = SVPAR_ACCEPTED;
                bContinue = sal_False;
            }
            else
            {
                sTmpBuffer.append( nNextCh );
            }
            break;

        case '<':
            bEqSignFound = sal_False;
            if( '>'==cBreak )
                sTmpBuffer.append( nNextCh );
            else
                bContinue = sal_False;      // Abbrechen, String zusammen
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
                bContinue = sal_False;
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
                bContinue = sal_False;
                break;
            }
            // Bug 18984: CR-LF -> Blank
            //      Folge von CR/LF/BLANK/TAB nur in ein Blank wandeln
            // kein break!!
        case '\t':
            if( '\t'==nNextCh && bReadPRE && '>'!=cBreak )
            {
                // In <PRE>: Tabs nach oben durchreichen
                bContinue = sal_False;
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
                bNextCh = sal_False;
            }
            break;

        default:
            bEqSignFound = sal_False;
            if( (nNextCh==cBreak && !cQuote) ||
                (sal_uLong(aToken.Len()) + MAX_LEN) > sal_uLong(STRING_MAXLEN & ~1 ))
                bContinue = sal_False;
            else
            {
                do {
                    // alle anderen Zeichen kommen in den Text
                    sTmpBuffer.append( nNextCh );
                    if( MAX_LEN == sTmpBuffer.getLength() )
                    {
                        aToken += String(sTmpBuffer.makeStringAndClear());
                        if( (sal_uLong(aToken.Len()) + MAX_LEN) >
                                sal_uLong(STRING_MAXLEN & ~1 ) )
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
                bNextCh = sal_False;
            }
        }

        if( MAX_LEN == sTmpBuffer.getLength() )
            aToken += String(sTmpBuffer.makeStringAndClear());

        if( bContinue && bNextCh )
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
        bReadScript = sal_False;
        bReadStyle = sal_False;
        aEndToken.Erase();
        bEndTokenFound = sal_False;

        return 0;
    }

    // per default geben wir HTML_RAWDATA zurueck
    int bContinue = sal_True;
    int nToken = HTML_RAWDATA;
    SaveState( 0 );
    while( bContinue && IsParserWorking() )
    {
        int bNextCh = sal_True;
        switch( nNextCh )
        {
        case '<':
            {
                // Vielleicht haben wir das Ende erreicht

                // das bisher gelesene erstmal retten
                aToken += String(sTmpBuffer.makeStringAndClear());

                // und die Position im Stream merken
                sal_uLong nStreamPos = rInput.Tell();
                sal_uLong nLineNr = GetLineNr();
                sal_uLong nLinePos = GetLinePos();

                // Start eines End-Token?
                int bOffState = sal_False;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = sal_True;
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
                             sal::static_int_cast< xub_StrLen >(
                                 sTmpBuffer.getLength()) );
                aTok.ToUpperAscii();
                sal_Bool bDone = sal_False;
                if( bReadScript || aEndToken.Len() )
                {
                    if( !bReadComment )
                    {
                        if( aTok.CompareToAscii( OOO_STRING_SVTOOLS_HTML_comment, 3 )
                                == COMPARE_EQUAL )
                        {
                            bReadComment = sal_True;
                        }
                        else
                        {
                            // ein Script muss mit "</SCRIPT>" aufhoehren, wobei
                            // wir es mit dem ">" aus sicherheitsgruenden
                            // erstmal nicht so genau nehmen
                            bDone = bOffState && // '>'==nNextCh &&
                            COMPARE_EQUAL == ( bReadScript
                                ? aTok.CompareToAscii(OOO_STRING_SVTOOLS_HTML_script)
                                : aTok.CompareTo(aEndToken) );
                        }
                    }
                    if( bReadComment && '>'==nNextCh && aTok.Len() >= 2 &&
                        aTok.Copy( aTok.Len()-2 ).EqualsAscii( "--" ) )
                    {
                        // hier ist ein Kommentar der Art <!-----> zuende
                        bReadComment = sal_False;
                    }
                }
                else
                {
                    // ein Style-Sheet kann mit </STYLE>, </HEAD> oder
                    // <BODY> aughoehren
                    if( bOffState )
                        bDone = aTok.CompareToAscii(OOO_STRING_SVTOOLS_HTML_style)
                                    == COMPARE_EQUAL ||
                                aTok.CompareToAscii(OOO_STRING_SVTOOLS_HTML_head)
                                    == COMPARE_EQUAL;
                    else
                        bDone =
                            aTok.CompareToAscii(OOO_STRING_SVTOOLS_HTML_body) == COMPARE_EQUAL;
                }

                if( bDone )
                {
                    // das war's, jetzt muessen wir gegebenenfalls den
                    // bisher gelesenen String zurueckgeben und dnach normal
                    // weitermachen

                    bContinue = sal_False;

                    // nToken==0 heisst, dass _GetNextToken gleich weiterliest
                    if( !aToken.Len() && (bReadStyle || bReadScript) )
                    {
                        // wir koennen sofort die Umgebung beeden und
                        // das End-Token parsen
                        bReadScript = sal_False;
                        bReadStyle = sal_False;
                        aEndToken.Erase();
                        nToken = 0;
                    }
                    else
                    {
                        // wir muessen bReadScript/bReadStyle noch am
                        // Leben lassen und koennen erst beim naechsten
                        // mal das End-Token Parsen
                        bEndTokenFound = sal_True;
                    }

                    // jetzt fahren wir im Stream auf das '<' zurueck
                    rInput.Seek( nStreamPos );
                    SetLineNr( nLineNr );
                    SetLinePos( nLinePos );
                    ClearTxtConvContext();
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

                    bNextCh = sal_False;
                }
            }
            break;
        case '-':
            sTmpBuffer.append( nNextCh );
            if( bReadComment )
            {
                sal_Bool bTwoMinus = sal_False;
                nNextCh = GetNextChar();
                while( '-' == nNextCh && IsParserWorking() )
                {
                    bTwoMinus = sal_True;

                    if( MAX_LEN == sTmpBuffer.getLength() )
                        aToken += String(sTmpBuffer.makeStringAndClear());
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                if( '>' == nNextCh && IsParserWorking() && bTwoMinus )
                    bReadComment = sal_False;

                bNextCh = sal_False;
            }
            break;

        case '\r':
            // \r\n? beendet das aktuelle Text-Token (auch wenn es leer ist)
            nNextCh = GetNextChar();
            if( nNextCh=='\n' )
                nNextCh = GetNextChar();
            bContinue = sal_False;
            break;
        case '\n':
            // \n beendet das aktuelle Text-Token (auch wenn es leer ist)
            nNextCh = GetNextChar();
            bContinue = sal_False;
            break;
        case sal_Unicode(EOF):
            // eof beendet das aktuelle Text-Token und tut so, als ob
            // ein End-Token gelesen wurde
            if( rInput.IsEof() )
            {
                bContinue = sal_False;
                if( aToken.Len() || sTmpBuffer.getLength() )
                {
                    bEndTokenFound = sal_True;
                }
                else
                {
                    bReadScript = sal_False;
                    bReadStyle = sal_False;
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

        if( (!bContinue && sTmpBuffer.getLength() > 0L) ||
            MAX_LEN == sTmpBuffer.getLength() )
            aToken += String(sTmpBuffer.makeStringAndClear());

        if( bContinue && bNextCh )
            nNextCh = GetNextChar();
    }

    if( IsParserWorking() )
        SaveState( 0 );
    else
        nToken = 0;

    return nToken;
}

// scanne das naechste Token,
int HTMLParser::_GetNextToken()
{
    int nRet = 0;
    sSaveToken.Erase();

    // die Optionen loeschen
    if( pOptions->Count() )
        pOptions->DeleteAndDestroy( 0, pOptions->Count() );

    if( !IsParserWorking() )        // wenn schon Fehler, dann nicht weiter!
        return 0;

    sal_Bool bReadNextCharSave = bReadNextChar;
    if( bReadNextChar )
    {
        DBG_ASSERT( !bEndTokenFound,
                    "</SCRIPT> gelesen und trotzdem noch ein Zeichen lesen?" );
        nNextCh = GetNextChar();
        if( !IsParserWorking() )        // wenn schon Fehler, dann nicht weiter!
            return 0;
        bReadNextChar = sal_False;
    }

    if( bReadScript || bReadStyle || aEndToken.Len() )
    {
        nRet = _GetNextRawToken();
        if( nRet || !IsParserWorking() )
            return nRet;
    }

    do {
        int bNextCh = sal_True;
        switch( nNextCh )
        {
        case '<':
            {
                sal_uLong nStreamPos = rInput.Tell();
                sal_uLong nLineNr = GetLineNr();
                sal_uLong nLinePos = GetLinePos();

                int bOffState = sal_False;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = sal_True;
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
                        sal_uLong nCStreamPos = 0;
                        sal_uLong nCLineNr = 0;
                        sal_uLong nCLinePos = 0;
                        xub_StrLen nCStrLen = 0;

                        sal_Bool bDone = sal_False;
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
                                bDone = aToken.Len() >= 2 &&
                                        aToken.Copy(aToken.Len()-2,2).
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
                            ClearTxtConvContext();
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

                        // fdo#34666: closing "/>"?:
                        // return HTML_UNKNOWNCONTROL_OFF instead of
                        // HTML_UNKNOWNCONTROL_ON
                        if (aToken.Len() >= 1 && '/' == aToken.GetChar(aToken.Len()-1)) {
                            if (HTML_UNKNOWNCONTROL_ON == nRet)
                                nRet = HTML_UNKNOWNCONTROL_OFF;
                        }
                        if( sal_Unicode(EOF) == nNextCh && rInput.IsEof() )
                        {
                            // zurueck hinter die < gehen  und dort neu
                            // aufsetzen, das < als Text zurueckgeben
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );
                            ClearTxtConvContext();

                            aToken = '<';
                            nRet = HTML_TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = sal_False;
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
                            ClearTxtConvContext();

                            aToken = '<';
                            nRet = HTML_TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = sal_False;
                            break;
                        }
                        if( SVPAR_PENDING == eState )
                            bReadNextChar = bReadNextCharSave;
                        aToken.Erase();
                    }
                    else if( '%' == nNextCh )
                    {
                        nRet = HTML_UNKNOWNCONTROL_ON;

                        sal_uLong nCStreamPos = rInput.Tell();
                        sal_uLong nCLineNr = GetLineNr(), nCLinePos = GetLinePos();

                        sal_Bool bDone = sal_False;
                        // bis zum schliessenden %> lesen. wenn keins gefunden
                        // wurde beim der ersten > wieder aufsetzen
                        while( !bDone && !rInput.IsEof() && IsParserWorking() )
                        {
                            bDone = '>'==nNextCh && aToken.Len() >= 1 &&
                                    '%' == aToken.GetChar( aToken.Len()-1 );
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
                            ClearTxtConvContext();
                            aToken.AssignAscii( "<%", 2 );
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
                        bNextCh = sal_False;
                        break;
                    }
                }

                if( IsParserWorking() )
                {
                    bNextCh = '>' == nNextCh;
                    switch( nRet )
                    {
                    case HTML_TEXTAREA_ON:
                        bReadTextArea = sal_True;
                        break;
                    case HTML_TEXTAREA_OFF:
                        bReadTextArea = sal_False;
                        break;
                    case HTML_SCRIPT_ON:
                        if( !bReadTextArea )
                            bReadScript = sal_True;
                        break;
                    case HTML_SCRIPT_OFF:
                        if( !bReadTextArea )
                        {
                            bReadScript = sal_False;
                            // JavaScript kann den Stream veraendern
                            // also muss das letzte Zeichen nochmals
                            // gelesen werden
                            bReadNextChar = sal_True;
                            bNextCh = sal_False;
                        }
                        break;

                    case HTML_STYLE_ON:
                        bReadStyle = sal_True;
                        break;
                    case HTML_STYLE_OFF:
                        bReadStyle = sal_False;
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
                    bNextCh = sal_False;
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
                bReadNextChar = sal_True;
            }

            break;
        }

        if( bNextCh && SVPAR_WORKING == eState )
        {
            nNextCh = GetNextChar();
            if( SVPAR_PENDING == eState && nRet && HTML_TEXTTOKEN != nRet )
            {
                bReadNextChar = sal_True;
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
    xub_StrLen nPos=0;

    sal_Bool bEscape = sal_False;
    while( nPos < aToken.Len() )
    {
        sal_Bool bOldEscape = bEscape;
        bEscape = sal_False;
        if( '\\'==aToken.GetChar(nPos) && !bOldEscape )
        {
            aToken.Erase( nPos, 1 );
            bEscape = sal_True;
        }
        else
        {
            nPos++;
        }
    }
}

// hole die Optionen
const HTMLOptions *HTMLParser::GetOptions( sal_uInt16 *pNoConvertToken ) const
{
    // wenn die Option fuer das aktuelle Token schon einmal
    // geholt wurden, geben wir sie noch einmal zurueck
    if( pOptions->Count() )
        return pOptions;

    xub_StrLen nPos = 0;
    while( nPos < aToken.Len() )
    {
        // ein Zeichen ? Dann faengt hier eine Option an
        if( HTML_ISALPHA( aToken.GetChar(nPos) ) )
        {
            int nToken;
            String aValue;
            xub_StrLen nStt = nPos;
            sal_Unicode cChar = 0;

            // Eigentlich sind hier nur ganz bestimmte Zeichen erlaubt.
            // Netscape achtet aber nur auf "=" und Leerzeichen (siehe
            // Mozilla: PA_FetchRequestedNameValues in
            // lipparse/pa_mdl.c
//          while( nPos < aToken.Len() &&
//                  ( '-'==(c=aToken[nPos]) || isalnum(c) || '.'==c || '_'==c) )
            while( nPos < aToken.Len() && '=' != (cChar=aToken.GetChar(nPos)) &&
                   HTML_ISPRINTABLE(cChar) && !HTML_ISSPACE(cChar) )
                nPos++;

            String sName( aToken.Copy( nStt, nPos-nStt ) );

//JP 23.03.97: die PlugIns wollen die TokenName im "Original" haben
//              also nur fuers Suchen in UpperCase wandeln
            String sNameUpperCase( sName );
            sNameUpperCase.ToUpperAscii();

            nToken = GetHTMLOption( sNameUpperCase ); // der Name ist fertig
            DBG_ASSERTWARNING( nToken!=HTML_O_UNKNOWN,
                        "GetOption: unbekannte HTML-Option" );
            sal_Bool bStripCRLF = (nToken < HTML_OPTION_SCRIPT_START ||
                               nToken >= HTML_OPTION_SCRIPT_END) &&
                              (!pNoConvertToken || nToken != *pNoConvertToken);

            while( nPos < aToken.Len() &&
                   ( !HTML_ISPRINTABLE( (cChar=aToken.GetChar(nPos)) ) ||
                     HTML_ISSPACE(cChar) ) )
                nPos++;

            // hat die Option auch einen Wert?
            if( nPos!=aToken.Len() && '='==cChar )
            {
                nPos++;

                while( nPos < aToken.Len() &&
                        ( !HTML_ISPRINTABLE( (cChar=aToken.GetChar(nPos)) ) ||
                          ' '==cChar || '\t'==cChar || '\r'==cChar || '\n'==cChar ) )
                    nPos++;

                if( nPos != aToken.Len() )
                {
                    xub_StrLen nLen = 0;
                    nStt = nPos;
                    if( ('"'==cChar) || ('\'')==cChar )
                    {
                        sal_Unicode cEnd = cChar;
                        nPos++; nStt++;
                        sal_Bool bDone = sal_False;
                        sal_Bool bEscape = sal_False;
                        while( nPos < aToken.Len() && !bDone )
                        {
                            sal_Bool bOldEscape = bEscape;
                            bEscape = sal_False;
                            cChar = aToken.GetChar(nPos);
                            switch( cChar )
                            {
                            case '\r':
                            case '\n':
                                if( bStripCRLF )
                                    ((String &)aToken).Erase( nPos, 1 );
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
                                    ((String &)aToken).Erase( nPos, 1 );
                                    bEscape = sal_True;
                                }
                                break;
                            case '"':
                            case '\'':
                                bDone = !bOldEscape && cChar==cEnd;
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
                        sal_Bool bEscape = sal_False;
                        sal_Bool bDone = sal_False;
                        while( nPos < aToken.Len() && !bDone )
                        {
                            sal_Bool bOldEscape = bEscape;
                            bEscape = sal_False;
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
                                bDone = sal_True;
                                break;

                            case '\\':
                                if( bOldEscape )
                                {
                                    nPos++, nLen++;
                                }
                                else
                                {
                                    ((String &)aToken).Erase( nPos, 1 );
                                    bEscape = sal_True;
                                }
                                break;

                            default:
                                if( HTML_ISPRINTABLE( c ) )
                                    nPos++, nLen++;
                                else
                                    bDone = sal_True;
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
                new HTMLOption(
                    sal::static_int_cast< sal_uInt16 >(nToken), sName, aValue );

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
        nPre_LinePos = 0;
        if( bPre_IgnoreNewPara )
            nToken = 0;
        break;

    case HTML_TABCHAR:
        {
            xub_StrLen nSpaces = sal::static_int_cast< xub_StrLen >(
                8 - (nPre_LinePos % 8));
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
    case HTML_STRIKE_ON:
    case HTML_STRIKE_OFF:
    case HTML_STRIKETHROUGH_ON:
    case HTML_STRIKETHROUGH_OFF:
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
    case HTML_SUBSCRIPT_ON:
    case HTML_SUBSCRIPT_OFF:
    case HTML_SUPERSCRIPT_ON:
    case HTML_SUPERSCRIPT_OFF:
    case HTML_BIGPRINT_ON:
    case HTML_BIGPRINT_OFF:
    case HTML_SMALLPRINT_OFF:
    case HTML_SMALLPRINT_ON:

    case HTML_EMPHASIS_ON:
    case HTML_EMPHASIS_OFF:
    case HTML_CITIATION_ON:
    case HTML_CITIATION_OFF:
    case HTML_STRONG_ON:
    case HTML_STRONG_OFF:
    case HTML_CODE_ON:
    case HTML_CODE_OFF:
    case HTML_SAMPLE_ON:
    case HTML_SAMPLE_OFF:
    case HTML_KEYBOARD_ON:
    case HTML_KEYBOARD_OFF:
    case HTML_VARIABLE_ON:
    case HTML_VARIABLE_OFF:
    case HTML_DEFINSTANCE_ON:
    case HTML_DEFINSTANCE_OFF:
    case HTML_SHORTQUOTE_ON:
    case HTML_SHORTQUOTE_OFF:
    case HTML_LANGUAGE_ON:
    case HTML_LANGUAGE_OFF:
    case HTML_AUTHOR_ON:
    case HTML_AUTHOR_OFF:
    case HTML_PERSON_ON:
    case HTML_PERSON_OFF:
    case HTML_ACRONYM_ON:
    case HTML_ACRONYM_OFF:
    case HTML_ABBREVIATION_ON:
    case HTML_ABBREVIATION_OFF:
    case HTML_INSERTEDTEXT_ON:
    case HTML_INSERTEDTEXT_OFF:
    case HTML_DELETEDTEXT_ON:
    case HTML_DELETEDTEXT_OFF:
    case HTML_TELETYPE_ON:
    case HTML_TELETYPE_OFF:

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

    bPre_IgnoreNewPara = sal_False;

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
                sSaveToken.Insert( '<', 0 );
                sSaveToken.Insert( '/', 1 );
            }
            else
                sSaveToken.Insert( '<', 0 );
            if( aToken.Len() )
            {
                UnescapeToken();
                sSaveToken += (sal_Unicode)' ';
                aToken.Insert( sSaveToken, 0 );
            }
            else
                aToken = sSaveToken;
            aToken += (sal_Unicode)'>';
            nToken = HTML_TEXTTOKEN;
        }
        break;
    }

    bPre_IgnoreNewPara = sal_False;

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

    bPre_IgnoreNewPara = sal_False;

    return nToken;
}

bool HTMLParser::IsHTMLFormat( const sal_Char* pHeader,
                               sal_Bool bSwitchToUCS2,
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
    sal_Bool bUCS2B = sal_False;
    if( bSwitchToUCS2 )
    {
        if( 0xfeU == (sal_uChar)pHeader[0] &&
            0xffU == (sal_uChar)pHeader[1] )
        {
            eEnc = RTL_TEXTENCODING_UCS2;
            bUCS2B = sal_True;
        }
        else if( 0xffU == (sal_uChar)pHeader[0] &&
                 0xfeU == (sal_uChar)pHeader[1] )
        {
            eEnc = RTL_TEXTENCODING_UCS2;
        }
    }
    if
       (
        RTL_TEXTENCODING_UCS2 == eEnc &&
        (
         (0xfe == (sal_uChar)pHeader[0] && 0xff == (sal_uChar)pHeader[1]) ||
         (0xff == (sal_uChar)pHeader[0] && 0xfe == (sal_uChar)pHeader[1])
        )
       )
    {
        if( 0xfe == (sal_uChar)pHeader[0] )
            bUCS2B = sal_True;

        xub_StrLen nLen;
        for( nLen = 2;
             pHeader[nLen] != 0 || pHeader[nLen+1] != 0;
             nLen+=2 )
            ;

        ::rtl::OStringBuffer sTmp( (nLen - 2)/2 );
        for( xub_StrLen nPos = 2; nPos < nLen; nPos += 2 )
        {
            sal_Unicode cUC;
            if( bUCS2B )
                cUC = (sal_Unicode(pHeader[nPos]) << 8) | pHeader[nPos+1];
            else
                cUC = (sal_Unicode(pHeader[nPos+1]) << 8) | pHeader[nPos];
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
    xub_StrLen nStart = sCmp.Search( '<' );
    if( STRING_NOTFOUND  == nStart )
        return sal_False;
    nStart++;

    // danach duerfen beliebige andere Zeichen bis zu einem blank oder
    // '>' kommen
    sal_Char c;
    xub_StrLen nPos;
    for( nPos = nStart; nPos<sCmp.Len(); nPos++ )
    {
        if( '>'==(c=sCmp.GetChar(nPos)) || HTML_ISSPACE(c) )
            break;
    }

    // wenn das Dokeument hinter dem < aufhoert ist es wohl kein HTML
    if( nPos==nStart )
        return sal_False;

    // die Zeichenkette nach dem '<' muss ausserdem ein bekanntes
    // HTML Token sein. Damit die Ausgabe eines DOS-dir-Befehls nicht
    // als HTML interpretiert wird, wird ein <DIR> jedoch nicht als HTML
    // interpretiert.
    String sTest( sCmp.Copy( nStart, nPos-nStart ), RTL_TEXTENCODING_ASCII_US );
    int nTok = GetHTMLToken( sTest );
    if( 0 != nTok && HTML_DIRLIST_ON != nTok )
        return sal_True;

    // oder es handelt sich um ein "<!" ganz am Anfang der Datei (fix #27092#)
    if( nStart == 1 && '!' == sCmp.GetChar( 1 ) )
        return sal_True;

    // oder wir finden irgendwo ein <HTML> in den ersten 80 Zeichen
    nStart = sCmp.Search( OOO_STRING_SVTOOLS_HTML_html );
    if( nStart!=STRING_NOTFOUND &&
        nStart>0 && '<'==sCmp.GetChar(nStart-1) &&
        nStart+4 < sCmp.Len() && '>'==sCmp.GetChar(nStart+4) )
        return sal_True;

    // sonst ist es wohl doch eher kein HTML-Dokument
    return sal_False;
}

sal_Bool HTMLParser::InternalImgToPrivateURL( String& rURL )
{
    if( rURL.Len() < 19 || 'i' != rURL.GetChar(0) ||
        rURL.CompareToAscii( OOO_STRING_SVTOOLS_HTML_internal_gopher, 9 ) != COMPARE_EQUAL )
        return sal_False;

    sal_Bool bFound = sal_False;

    if( rURL.CompareToAscii( OOO_STRING_SVTOOLS_HTML_internal_gopher,16) == COMPARE_EQUAL )
    {
        String aName( rURL.Copy(16) );
        switch( aName.GetChar(0) )
        {
        case 'b':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_binary );
            break;
        case 'i':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_image ) ||
                     aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_index );
            break;
        case 'm':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_menu ) ||
                     aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_movie );
            break;
        case 's':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_sound );
            break;
        case 't':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_telnet ) ||
                     aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_text );
            break;
        case 'u':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_GOPHER_unknown );
            break;
        }
    }
    else if( rURL.CompareToAscii( OOO_STRING_SVTOOLS_HTML_internal_icon,14) == COMPARE_EQUAL )
    {
        String aName( rURL.Copy(14) );
        switch( aName.GetChar(0) )
        {
        case 'b':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_ICON_baddata );
            break;
        case 'd':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_ICON_delayed );
            break;
        case 'e':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_ICON_embed );
            break;
        case 'i':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_ICON_insecure );
            break;
        case 'n':
            bFound = aName.EqualsAscii( OOO_STRING_SVTOOLS_HTML_INT_ICON_notfound );
            break;
        }
    }
    if( bFound )
    {
        String sTmp ( rURL );
        rURL.AssignAscii( OOO_STRING_SVTOOLS_HTML_private_image );
        rURL.Append( sTmp );
    }

    return bFound;
}

enum eHtmlMetas {
    HTML_META_NONE = 0,
    HTML_META_AUTHOR,
    HTML_META_DESCRIPTION,
    HTML_META_KEYWORDS,
    HTML_META_REFRESH,
    HTML_META_CLASSIFICATION,
    HTML_META_CREATED,
    HTML_META_CHANGEDBY,
    HTML_META_CHANGED,
    HTML_META_GENERATOR,
    HTML_META_SDFOOTNOTE,
    HTML_META_SDENDNOTE,
    HTML_META_CONTENT_TYPE
};

// <META NAME=xxx>
static HTMLOptionEnum const aHTMLMetaNameTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_META_author,        HTML_META_AUTHOR        },
    { OOO_STRING_SVTOOLS_HTML_META_changed,       HTML_META_CHANGED       },
    { OOO_STRING_SVTOOLS_HTML_META_changedby,     HTML_META_CHANGEDBY     },
    { OOO_STRING_SVTOOLS_HTML_META_classification,HTML_META_CLASSIFICATION},
    { OOO_STRING_SVTOOLS_HTML_META_content_type,  HTML_META_CONTENT_TYPE  },
    { OOO_STRING_SVTOOLS_HTML_META_created,       HTML_META_CREATED       },
    { OOO_STRING_SVTOOLS_HTML_META_description,   HTML_META_DESCRIPTION   },
    { OOO_STRING_SVTOOLS_HTML_META_keywords,      HTML_META_KEYWORDS      },
    { OOO_STRING_SVTOOLS_HTML_META_generator,     HTML_META_GENERATOR     },
    { OOO_STRING_SVTOOLS_HTML_META_refresh,       HTML_META_REFRESH       },
    { OOO_STRING_SVTOOLS_HTML_META_sdendnote,     HTML_META_SDENDNOTE     },
    { OOO_STRING_SVTOOLS_HTML_META_sdfootnote,    HTML_META_SDFOOTNOTE    },
    { 0,                                          0                       }
};


void HTMLParser::AddMetaUserDefined( ::rtl::OUString const & )
{
}

bool HTMLParser::ParseMetaOptionsImpl(
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        SvKeyValueIterator *i_pHTTPHeader,
        const HTMLOptions *i_pOptions,
        rtl_TextEncoding& o_rEnc )
{
    String aName, aContent;
    sal_uInt16 nAction = HTML_META_NONE;
    bool bHTTPEquiv = false, bChanged = false;

    for ( sal_uInt16 i = i_pOptions->Count(); i; )
    {
        const HTMLOption *pOption = (*i_pOptions)[ --i ];
        switch ( pOption->GetToken() )
        {
            case HTML_O_NAME:
                aName = pOption->GetString();
                if ( HTML_META_NONE==nAction )
                {
                    pOption->GetEnum( nAction, aHTMLMetaNameTable );
                }
                break;
            case HTML_O_HTTPEQUIV:
                aName = pOption->GetString();
                pOption->GetEnum( nAction, aHTMLMetaNameTable );
                bHTTPEquiv = true;
                break;
            case HTML_O_CONTENT:
                aContent = pOption->GetString();
                break;
        }
    }

    if ( bHTTPEquiv || HTML_META_DESCRIPTION != nAction )
    {
        // if it is not a Description, remove CRs and LFs from CONTENT
        aContent.EraseAllChars( _CR );
        aContent.EraseAllChars( _LF );
    }
    else
    {
        // convert line endings for Description
        aContent.ConvertLineEnd();
    }


    if ( bHTTPEquiv && i_pHTTPHeader )
    {
        // #57232#: Netscape seems to just ignore a closing ", so we do too
        if ( aContent.Len() && '"' == aContent.GetChar( aContent.Len()-1 ) )
        {
            aContent.Erase( aContent.Len() - 1 );
        }
        SvKeyValue aKeyValue( aName, aContent );
        i_pHTTPHeader->Append( aKeyValue );
    }

    switch ( nAction )
    {
        case HTML_META_AUTHOR:
            if (i_xDocProps.is()) {
                i_xDocProps->setAuthor( aContent );
                bChanged = true;
            }
            break;
        case HTML_META_DESCRIPTION:
            if (i_xDocProps.is()) {
                i_xDocProps->setDescription( aContent );
                bChanged = true;
            }
            break;
        case HTML_META_KEYWORDS:
            if (i_xDocProps.is()) {
                i_xDocProps->setKeywords(
                    ::comphelper::string::convertCommaSeparated(aContent));
                bChanged = true;
            }
            break;
        case HTML_META_CLASSIFICATION:
            if (i_xDocProps.is()) {
                i_xDocProps->setSubject( aContent );
                bChanged = true;
            }
            break;

        case HTML_META_CHANGEDBY:
            if (i_xDocProps.is()) {
                i_xDocProps->setModifiedBy( aContent );
            }
            break;

        case HTML_META_CREATED:
        case HTML_META_CHANGED:
            if ( i_xDocProps.is() && aContent.Len() &&
                 aContent.GetTokenCount() == 2 )
            {
                Date aDate( (sal_uLong)aContent.GetToken(0).ToInt32() );
                Time aTime( (sal_uLong)aContent.GetToken(1).ToInt32() );
                DateTime aDateTime( aDate, aTime );
                ::util::DateTime uDT(aDateTime.Get100Sec(),
                    aDateTime.GetSec(), aDateTime.GetMin(),
                    aDateTime.GetHour(), aDateTime.GetDay(),
                    aDateTime.GetMonth(), aDateTime.GetYear());
                if ( HTML_META_CREATED==nAction )
                    i_xDocProps->setCreationDate( uDT );
                else
                    i_xDocProps->setModificationDate( uDT );
                bChanged = true;
            }
            break;

        case HTML_META_REFRESH:
            DBG_ASSERT( !bHTTPEquiv || i_pHTTPHeader,
        "Reload-URL aufgrund unterlassener MUSS-Aenderung verlorengegangen" );
            break;

        case HTML_META_CONTENT_TYPE:
            if ( aContent.Len() )
            {
                o_rEnc = GetEncodingByMIME( aContent );
            }
            break;

        case HTML_META_NONE:
            if ( !bHTTPEquiv )
            {
                if (i_xDocProps.is())
                {
                    uno::Reference<beans::XPropertyContainer> xUDProps
                        = i_xDocProps->getUserDefinedProperties();
                    try {
                        xUDProps->addProperty(aName,
                            beans::PropertyAttribute::REMOVEABLE,
                            uno::makeAny(::rtl::OUString(aContent)));
                        AddMetaUserDefined(aName);
                        bChanged = true;
                    } catch (uno::Exception &) {
                        // ignore
                    }
                }
            }
            break;
        default:
            break;
    }

    return bChanged;
}

bool HTMLParser::ParseMetaOptions(
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        SvKeyValueIterator *i_pHeader )
{
    sal_uInt16 nContentOption = HTML_O_CONTENT;
    rtl_TextEncoding eEnc = RTL_TEXTENCODING_DONTKNOW;

    bool bRet = ParseMetaOptionsImpl( i_xDocProps, i_pHeader,
                      GetOptions(&nContentOption),
                      eEnc );

    // If the encoding is set by a META tag, it may only overwrite the
    // current encoding if both, the current and the new encoding, are 1-sal_uInt8
    // encodings. Everything else cannot lead to reasonable results.
    if (RTL_TEXTENCODING_DONTKNOW != eEnc &&
        rtl_isOctetTextEncoding( eEnc ) &&
        rtl_isOctetTextEncoding( GetSrcEncoding() ) )
    {
        eEnc = GetExtendedCompatibilityTextEncoding( eEnc ); // #89973#
        SetSrcEncoding( eEnc );
    }

    return bRet;
}

rtl_TextEncoding HTMLParser::GetEncodingByMIME( const String& rMime )
{
    ByteString sType;
    ByteString sSubType;
    INetContentTypeParameterList aParameters;
    ByteString sMime( rMime, RTL_TEXTENCODING_ASCII_US );
    if (INetContentTypes::parse(sMime, sType, sSubType, &aParameters))
    {
        const INetContentTypeParameter * pCharset
            = aParameters.find("charset");
        if (pCharset != 0)
        {
            ByteString sValue( pCharset->m_sValue, RTL_TEXTENCODING_ASCII_US );
            return GetExtendedCompatibilityTextEncoding(
                    rtl_getTextEncodingFromMimeCharset( sValue.GetBuffer() ) );
        }
    }
    return RTL_TEXTENCODING_DONTKNOW;
}

rtl_TextEncoding HTMLParser::GetEncodingByHttpHeader( SvKeyValueIterator *pHTTPHeader )
{
    rtl_TextEncoding eRet = RTL_TEXTENCODING_DONTKNOW;
    if( pHTTPHeader )
    {
        SvKeyValue aKV;
        for( sal_Bool bCont = pHTTPHeader->GetFirst( aKV ); bCont;
             bCont = pHTTPHeader->GetNext( aKV ) )
        {
            if( aKV.GetKey().EqualsIgnoreCaseAscii( OOO_STRING_SVTOOLS_HTML_META_content_type ) )
            {
                if( aKV.GetValue().Len() )
                {
                    eRet = HTMLParser::GetEncodingByMIME( aKV.GetValue() );
                }
            }
        }
    }
    return eRet;
}

sal_Bool HTMLParser::SetEncodingByHTTPHeader(
                                SvKeyValueIterator *pHTTPHeader )
{
    sal_Bool bRet = sal_False;
    rtl_TextEncoding eEnc = HTMLParser::GetEncodingByHttpHeader( pHTTPHeader );
    if(RTL_TEXTENCODING_DONTKNOW != eEnc)
    {
        SetSrcEncoding( eEnc );
        bRet = sal_True;
    }
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
