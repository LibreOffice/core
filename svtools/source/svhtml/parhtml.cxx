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


#include <ctype.h>
#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/color.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <tools/tenccvt.hxx>
#include <tools/datetime.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <svtools/parhtml.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>

#include <memory>
#include <utility>

using namespace ::com::sun::star;


const sal_Int32 MAX_LEN( 1024L );

const sal_Int32 MAX_ENTITY_LEN( 8L );


// Tables to convert option values into strings

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
    { nullptr,                    0                   }
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
    { nullptr,                0               }
};

// <TABLE RULES=xxx>
static HTMLOptionEnum const aTableRulesOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_TR_none,  HTML_TR_NONE    },
    { OOO_STRING_SVTOOLS_HTML_TR_groups,    HTML_TR_GROUPS  },
    { OOO_STRING_SVTOOLS_HTML_TR_rows,  HTML_TR_ROWS    },
    { OOO_STRING_SVTOOLS_HTML_TR_cols,  HTML_TR_COLS    },
    { OOO_STRING_SVTOOLS_HTML_TR_all,       HTML_TR_ALL     },
    { nullptr,                0               }
};

sal_uInt16 HTMLOption::GetEnum( const HTMLOptionEnum *pOptEnums, sal_uInt16 nDflt ) const
{
    sal_uInt16 nValue = nDflt;

    while( pOptEnums->pName )
        if( aValue.equalsIgnoreAsciiCaseAscii( pOptEnums->pName ) )
            break;
        else
            pOptEnums++;

    if( pOptEnums->pName )
        nValue = pOptEnums->nValue;

    return nValue;
}

bool HTMLOption::GetEnum( sal_uInt16 &rEnum, const HTMLOptionEnum *pOptEnums ) const
{
    while( pOptEnums->pName )
    {
        if( aValue.equalsIgnoreAsciiCaseAscii( pOptEnums->pName ) )
            break;
        else
            pOptEnums++;
    }

    const sal_Char *pName = pOptEnums->pName;
    if( pName )
        rEnum = pOptEnums->nValue;

    return (pName != nullptr);
}

HTMLOption::HTMLOption( sal_uInt16 nTok, const OUString& rToken,
                        const OUString& rValue )
    : aValue(rValue)
    , aToken(rToken)
    , nToken( nTok )
{
    DBG_ASSERT( nToken>=HTML_OPTION_START && nToken<HTML_OPTION_END,
        "HTMLOption: unknown token" );
}

sal_uInt32 HTMLOption::GetNumber() const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_NUMBER_START &&
                 nToken<HTML_OPTION_NUMBER_END) ||
                (nToken>=HTML_OPTION_CONTEXT_START &&
                 nToken<HTML_OPTION_CONTEXT_END) ||
                nToken==HTML_O_VALUE,
        "GetNumber: Option not numerical" );
    OUString aTmp(comphelper::string::stripStart(aValue, ' '));
    sal_Int32 nTmp = aTmp.toInt32();
    return nTmp >= 0 ? (sal_uInt32)nTmp : 0;
}

sal_Int32 HTMLOption::GetSNumber() const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_NUMBER_START && nToken<HTML_OPTION_NUMBER_END) ||
                (nToken>=HTML_OPTION_CONTEXT_START && nToken<HTML_OPTION_CONTEXT_END),
        "GetSNumber: Option not numerical" );
    OUString aTmp(comphelper::string::stripStart(aValue, ' '));
    return aTmp.toInt32();
}

void HTMLOption::GetNumbers( std::vector<sal_uInt32> &rNumbers, bool bSpaceDelim ) const
{
    rNumbers.clear();

    if( bSpaceDelim )
    {
        // This is a very simplified scanner: it only searches all
        // numerals in the string.
        bool bInNum = false;
        sal_uLong nNum = 0;
        for( sal_Int32 i=0; i<aValue.getLength(); i++ )
        {
            sal_Unicode c = aValue[ i ];
            if( c>='0' && c<='9' )
            {
                nNum *= 10;
                nNum += (c - '0');
                bInNum = true;
            }
            else if( bInNum )
            {
                rNumbers.push_back( nNum );
                bInNum = false;
                nNum = 0;
            }
        }
        if( bInNum )
        {
            rNumbers.push_back( nNum );
        }
    }
    else
    {
        // Check whether numbers are separated by ',' and
        // insert 0 if necessary
        sal_Int32 nPos = 0;
        while( nPos < aValue.getLength() )
        {
            sal_Unicode c;
            while( nPos < aValue.getLength() &&
                   ((c=aValue[nPos]) == ' ' || c == '\t' ||
                   c == '\n' || c== '\r' ) )
                nPos++;

            if( nPos==aValue.getLength() )
                rNumbers.push_back(0);
            else
            {
                sal_Int32 nEnd = aValue.indexOf( (sal_Unicode)',', nPos );
                if( -1 == nEnd )
                {
                    sal_Int32 nTmp = aValue.copy(nPos).toInt32();
                    rNumbers.push_back( nTmp >= 0 ? (sal_uInt32)nTmp : 0 );
                    nPos = aValue.getLength();
                }
                else
                {
                    sal_Int32 nTmp = aValue.copy(nPos,nEnd-nPos).toInt32();
                    rNumbers.push_back( nTmp >= 0 ? (sal_uInt32)nTmp : 0 );
                    nPos = nEnd+1;
                }
            }
        }
    }
}

void HTMLOption::GetColor( Color& rColor ) const
{
    DBG_ASSERT( (nToken>=HTML_OPTION_COLOR_START && nToken<HTML_OPTION_COLOR_END) || nToken==HTML_O_SIZE,
        "GetColor: Option is not a color." );

    OUString aTmp(aValue.toAsciiLowerCase());
    sal_uInt32 nColor = SAL_MAX_UINT32;
    if (!aTmp.isEmpty() && aTmp[0] != '#')
        nColor = GetHTMLColor(aTmp);

    if( SAL_MAX_UINT32 == nColor )
    {
        nColor = 0;
        sal_Int32 nPos = 0;
        for (sal_uInt32 i=0; i<6; ++i)
        {
            // Whatever Netscape does to get color values,
            // at maximum three characters < '0' are ignored.
            sal_Unicode c = nPos<aTmp.getLength() ? aTmp[ nPos++ ] : '0';
            if( c < '0' )
            {
                c = nPos<aTmp.getLength() ? aTmp[nPos++] : '0';
                if( c < '0' )
                    c = nPos<aTmp.getLength() ? aTmp[nPos++] : '0';
            }
            nColor *= 16;
            if( c >= '0' && c <= '9' )
                nColor += (c - '0');
            else if( c >= 'a' && c <= 'f' )
                nColor += (c + 0xa - 'a');
        }
    }

    rColor.SetRed(   (sal_uInt8)((nColor & 0x00ff0000) >> 16) );
    rColor.SetGreen( (sal_uInt8)((nColor & 0x0000ff00) >> 8));
    rColor.SetBlue(  (sal_uInt8)(nColor & 0x000000ff) );
}

HTMLInputType HTMLOption::GetInputType() const
{
    DBG_ASSERT( nToken==HTML_O_TYPE, "GetInputType: Option not TYPE" );
    return (HTMLInputType)GetEnum( aInputTypeOptEnums, HTML_IT_TEXT );
}

HTMLTableFrame HTMLOption::GetTableFrame() const
{
    DBG_ASSERT( nToken==HTML_O_FRAME, "GetTableFrame: Option not FRAME" );
    return (HTMLTableFrame)GetEnum( aTableFrameOptEnums );
}

HTMLTableRules HTMLOption::GetTableRules() const
{
    DBG_ASSERT( nToken==HTML_O_RULES, "GetTableRules: Option not RULES" );
    return (HTMLTableRules)GetEnum( aTableRulesOptEnums );
}

HTMLParser::HTMLParser( SvStream& rIn, bool bReadNewDoc ) :
    SvParser( rIn ),
    bNewDoc(bReadNewDoc),
    bIsInHeader(true),
    bIsInBody(false),
    bReadListing(false),
    bReadXMP(false),
    bReadPRE(false),
    bReadTextArea(false),
    bReadScript(false),
    bReadStyle(false),
    bEndTokenFound(false),
    bPre_IgnoreNewPara(false),
    bReadNextChar(false),
    bReadComment(false),
    nPre_LinePos(0),
    mnPendingOffToken(0)
{
    //#i76649, default to UTF-8 for HTML unless we know differently
    SetSrcEncoding(RTL_TEXTENCODING_UTF8);
}

HTMLParser::~HTMLParser()
{
}

SvParserState HTMLParser::CallParser()
{
    eState = SVPAR_WORKING;
    nNextCh = GetNextChar();
    SaveState( 0 );

    nPre_LinePos = 0;
    bPre_IgnoreNewPara = false;

    AddFirstRef();
    Continue( 0 );
    if( SVPAR_PENDING != eState )
        ReleaseRef();       // Parser not needed anymore

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
            SaveState( 0 );         // continue with new token

        nToken = GetNextToken();
    }
}

int HTMLParser::FilterToken( int nToken )
{
    switch( nToken )
    {
    case sal_Unicode(EOF):
        nToken = 0;
        break;          // don't pass

    case HTML_HEAD_OFF:
        bIsInBody = true;
        bIsInHeader = false;
        break;

    case HTML_HEAD_ON:
        bIsInHeader = true;
        break;

    case HTML_BODY_ON:
        bIsInHeader = false;
        bIsInBody = true;
        break;

    case HTML_FRAMESET_ON:
        bIsInHeader = false;
        bIsInBody = false;
        break;

    case HTML_BODY_OFF:
        bIsInBody = bReadPRE = bReadListing = bReadXMP = false;
        break;

    case HTML_HTML_OFF:
        nToken = 0;
        bReadPRE = bReadListing = bReadXMP = false;
        break;      // HTML_ON hasn't been passed either !

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

#define HTML_ISDIGIT( c ) rtl::isAsciiDigit(c)
#define HTML_ISALPHA( c ) rtl::isAsciiAlpha(c)
#define HTML_ISALNUM( c ) rtl::isAsciiAlphanumeric(c)
#define HTML_ISSPACE( c ) ( ' ' == c || (c >= 0x09 && c <= 0x0d) )
#define HTML_ISPRINTABLE( c ) ( c >= 32 && c != 127)
#define HTML_ISHEXDIGIT( c ) rtl::isAsciiHexDigit(c)

int HTMLParser::ScanText( const sal_Unicode cBreak )
{
    OUStringBuffer sTmpBuffer( MAX_LEN );
    bool bContinue = true;
    bool bEqSignFound = false;
    sal_Unicode cQuote = 0U;

    while( bContinue && IsParserWorking() )
    {
        bool bNextCh = true;
        switch( nNextCh )
        {
        case '&':
            bEqSignFound = false;
            if( bReadXMP )
                sTmpBuffer.append( '&' );
            else
            {
                sal_uLong nStreamPos = rInput.Tell();
                sal_uLong nLinePos = GetLinePos();

                sal_Unicode cChar = 0U;
                if( '#' == (nNextCh = GetNextChar()) )
                {
                    nNextCh = GetNextChar();
                    const bool bIsHex( 'x' == nNextCh );
                    const bool bIsDecOrHex( bIsHex || HTML_ISDIGIT(nNextCh) );
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
                            const sal_uInt32 convertFlags =
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT;

                            sal_Char cEncodedChar = static_cast<sal_Char>(cChar);
                            cChar = OUString(&cEncodedChar, 1, eSrcEnc, convertFlags).toChar();
                            if( 0U == cChar )
                            {
                                // If the character could not be
                                // converted, because a conversion is not
                                // available, do no conversion at all.
                                cChar = cEncodedChar;
                            }
                        }
                    }
                    else
                        nNextCh = 0U;
                }
                else if( HTML_ISALPHA( nNextCh ) )
                {
                    OUStringBuffer sEntityBuffer( MAX_ENTITY_LEN );
                    sal_Int32 nPos = 0L;
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
                        OUString sEntity(sEntityBuffer.getStr(), nPos);
                        cChar = GetHTMLCharName( sEntity );

                        // not found ( == 0 ): plain text
                        // or a character which is inserted as attribute
                        if( 0U == cChar && ';' != nNextCh )
                        {
                            DBG_ASSERT( rInput.Tell() - nStreamPos ==
                                        (sal_uLong)(nPos+1L)*GetCharSize(),
                                        "UTF-8 is failing here" );
                            for( sal_Int32 i = nPos-1; i>1; i-- )
                            {
                                nNextCh = sEntityBuffer[i];
                                sEntityBuffer.setLength( i );
                                sEntity = OUString(sEntityBuffer.getStr(), i);
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

                        if( !cChar )        // unknown character?
                        {
                            // back in stream, insert '&'
                            // and restart with next character
                            sTmpBuffer.append( '&' );

                            DBG_ASSERT( rInput.Tell()-nStreamPos ==
                                        (sal_uLong)(nPos+1)*GetCharSize(),
                                        "Wrong stream position" );
                            DBG_ASSERT( nlLinePos-nLinePos ==
                                        (sal_uLong)(nPos+1),
                                        "Wrong line position" );
                            rInput.Seek( nStreamPos );
                            nlLinePos = nLinePos;
                            ClearTxtConvContext();
                            break;
                        }

                        assert(cChar != 0);

                        // 1 == Non Breaking Space
                        // 2 == SoftHyphen

                        if (cChar == 1 || cChar == 2)
                        {
                            if( '>' == cBreak )
                            {
                                // When reading the content of a tag we have
                                // to change it to ' ' or '-'
                                if( 1U == cChar )
                                    cChar = ' ';
                                else //2U
                                    cChar = '-';
                            }
                            else
                            {
                                // If not scanning a tag return token
                                aToken += sTmpBuffer.makeStringAndClear();

                                if( !aToken.isEmpty() )
                                {
                                    // restart with character
                                    nNextCh = '&';
                                    DBG_ASSERT( rInput.Tell()-nStreamPos ==
                                                (sal_uLong)(nPos+1)*GetCharSize(),
                                                "Wrong stream position" );
                                    DBG_ASSERT( nlLinePos-nLinePos ==
                                                (sal_uLong)(nPos+1),
                                                "Wrong line position" );
                                    rInput.Seek( nStreamPos );
                                    nlLinePos = nLinePos;
                                    ClearTxtConvContext();
                                    return HTML_TEXTTOKEN;
                                }

                                // Hack: _GetNextChar shall not read the
                                // next character
                                if( ';' != nNextCh )
                                    aToken += " ";
                                if( 1U == cChar )
                                    return HTML_NONBREAKSPACE;
                                else //2U
                                    return HTML_SOFTHYPH;
                            }
                        }
                    }
                    else
                        nNextCh = 0U;
                }
                // &{...};-JavaScript-Macros are not supported any longer.
                else if( IsParserWorking() )
                {
                    sTmpBuffer.append( '&' );
                    bNextCh = false;
                    break;
                }

                bNextCh = (';' == nNextCh);
                if( cBreak=='>' && (cChar=='\\' || cChar=='\'' ||
                                    cChar=='\"' || cChar==' ') )
                {
                    // ' and " have to be escaped within tags to separate
                    // them from ' and " enclosing options.
                    // \ has to be escaped as well.
                    // Space is protected because it's not a delimiter between
                    // options.
                    sTmpBuffer.append( '\\' );
                    if( MAX_LEN == sTmpBuffer.getLength() )
                        aToken += sTmpBuffer.makeStringAndClear();
                }
                if( IsParserWorking() )
                {
                    if( cChar )
                        sTmpBuffer.append( cChar );
                }
                else if( SVPAR_PENDING==eState && '>'!=cBreak )
                {
                    // Restart with '&', the remainder is returned as
                    // text token.
                    if( !aToken.isEmpty() || !sTmpBuffer.isEmpty() )
                    {
                        // _GetNextChar() returns the previous text and
                        // during the next execution a new character is read.
                        // Thus we have to position in front of the '&'.
                        nNextCh = 0U;
                        rInput.Seek( nStreamPos-(sal_uInt32)GetCharSize() );
                        nlLinePos = nLinePos-1;
                        ClearTxtConvContext();
                        bReadNextChar = true;
                    }
                    bNextCh = false;
                }
            }
            break;
        case '=':
            if( '>'==cBreak && !cQuote )
                bEqSignFound = true;
            sTmpBuffer.append( nNextCh );
            break;

        case '\\':
            if( '>'==cBreak )
            {
                // Innerhalb von Tags kennzeichnen
                sTmpBuffer.append( '\\' );
                if( MAX_LEN == sTmpBuffer.getLength() )
                    aToken += sTmpBuffer.makeStringAndClear();
            }
            sTmpBuffer.append( '\\' );
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
            bEqSignFound = false;
            break;

        case sal_Unicode(EOF):
            if( rInput.IsEof() )
            {
                bContinue = false;
            }
            else
            {
                sTmpBuffer.append( nNextCh );
            }
            break;

        case '<':
            bEqSignFound = false;
            if( '>'==cBreak )
                sTmpBuffer.append( nNextCh );
            else
                bContinue = false;      // break, String zusammen
            break;

        case '\f':
            if( '>' == cBreak )
            {
                // If scanning options treat it like a space, ...
                sTmpBuffer.append( ' ' );
            }
            else
            {
                // otherwise it's a separate token.
                bContinue = false;
            }
            break;

        case '\r':
        case '\n':
            if( '>'==cBreak )
            {
                // cr/lf in tag is handled in _GetNextToken()
                sTmpBuffer.append( nNextCh );
                break;
            }
            else if( bReadListing || bReadXMP || bReadPRE || bReadTextArea )
            {
                bContinue = false;
                break;
            }
            // Reduce sequence of CR/LF/BLANK/TAB to a single blank
            // no break!!
        case '\t':
            if( '\t'==nNextCh && bReadPRE && '>'!=cBreak )
            {
                // Pass Tabs up in <PRE>
                bContinue = false;
                break;
            }
            // no break
        case '\x0b':
            if( '\x0b'==nNextCh && (bReadPRE || bReadXMP ||bReadListing) &&
                '>'!=cBreak )
            {
                break;
            }
            nNextCh = ' ';
            // no break;
        case ' ':
            sTmpBuffer.append( nNextCh );
            if( '>'!=cBreak && (!bReadListing && !bReadXMP &&
                                !bReadPRE && !bReadTextArea) )
            {
                // Reduce sequences of Blanks/Tabs/CR/LF to a single blank
                do {
                    if( sal_Unicode(EOF) == (nNextCh = GetNextChar()) &&
                        rInput.IsEof() )
                    {
                        if( !aToken.isEmpty() || sTmpBuffer.getLength() > 1L )
                        {
                            // Have seen s.th. aside from blanks?
                            aToken += sTmpBuffer.makeStringAndClear();
                            return HTML_TEXTTOKEN;
                        }
                        else
                            // Only read blanks: no text must be returned
                            // and _GetNextToken has to read until EOF
                            return 0;
                    }
                } while ( ' ' == nNextCh || '\t' == nNextCh ||
                          '\r' == nNextCh || '\n' == nNextCh ||
                          '\x0b' == nNextCh );
                bNextCh = false;
            }
            break;

        default:
            bEqSignFound = false;
            if (nNextCh == cBreak && !cQuote)
                bContinue = false;
            else
            {
                do {
                    // All remaining characters make their way into the text.
                    sTmpBuffer.append( nNextCh );
                    if( MAX_LEN == sTmpBuffer.getLength() )
                    {
                        aToken += sTmpBuffer.makeStringAndClear();
                    }
                    if( ( sal_Unicode(EOF) == (nNextCh = GetNextChar()) &&
                          rInput.IsEof() ) ||
                        !IsParserWorking() )
                    {
                        if( !sTmpBuffer.isEmpty() )
                            aToken += sTmpBuffer.makeStringAndClear();
                        return HTML_TEXTTOKEN;
                    }
                } while( HTML_ISALPHA( nNextCh ) || HTML_ISDIGIT( nNextCh ) );
                bNextCh = false;
            }
        }

        if( MAX_LEN == sTmpBuffer.getLength() )
            aToken += sTmpBuffer.makeStringAndClear();

        if( bContinue && bNextCh )
            nNextCh = GetNextChar();
    }

    if( !sTmpBuffer.isEmpty() )
        aToken += sTmpBuffer.makeStringAndClear();

    return HTML_TEXTTOKEN;
}

int HTMLParser::_GetNextRawToken()
{
    OUStringBuffer sTmpBuffer( MAX_LEN );

    if( bEndTokenFound )
    {
        // During the last execution we already found the end token,
        // thus we don't have to search it again.
        bReadScript = false;
        bReadStyle = false;
        aEndToken.clear();
        bEndTokenFound = false;

        return 0;
    }

    // Default return value: HTML_RAWDATA
    bool bContinue = true;
    int nToken = HTML_RAWDATA;
    SaveState( 0 );
    while( bContinue && IsParserWorking() )
    {
        bool bNextCh = true;
        switch( nNextCh )
        {
        case '<':
            {
                // Maybe we've reached the end.

                // Save what we have read previously...
                aToken += sTmpBuffer.makeStringAndClear();

                // and remember position in stream.
                sal_uLong nStreamPos = rInput.Tell();
                sal_uLong nLineNr = GetLineNr();
                sal_uLong nLinePos = GetLinePos();

                // Start of an end token?
                bool bOffState = false;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = true;
                    nNextCh = GetNextChar();
                }
                else if( '!' == nNextCh )
                {
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                // Read following letters
                while( (HTML_ISALPHA(nNextCh) || '-'==nNextCh) &&
                       IsParserWorking() && sTmpBuffer.getLength() < MAX_LEN )
                {
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                OUString aTok( sTmpBuffer.toString() );
                aTok = aTok.toAsciiLowerCase();
                bool bDone = false;
                if( bReadScript || !aEndToken.isEmpty() )
                {
                    if( !bReadComment )
                    {
                        if( aTok.startsWith( OOO_STRING_SVTOOLS_HTML_comment ) )
                        {
                            bReadComment = true;
                        }
                        else
                        {
                            // A script has to end with "</SCRIPT>". But
                            // ">" is optional for security reasons
                            bDone = bOffState &&
                            ( bReadScript
                                ? aTok == OOO_STRING_SVTOOLS_HTML_script
                                : aTok.equals(aEndToken) );
                        }
                    }
                    if( bReadComment && '>'==nNextCh && aTok.endsWith( "--" ) )
                    {
                        // End of comment of style <!----->
                        bReadComment = false;
                    }
                }
                else
                {
                    // Style sheets can be closed by </STYLE>, </HEAD> or <BODY>
                    if( bOffState )
                        bDone = aTok == OOO_STRING_SVTOOLS_HTML_style ||
                                aTok == OOO_STRING_SVTOOLS_HTML_head;
                    else
                        bDone = aTok == OOO_STRING_SVTOOLS_HTML_body;
                }

                if( bDone )
                {
                    // Done! Return the previously read string (if requested)
                    // and continue.

                    bContinue = false;

                    // nToken==0 means, _GetNextToken continues to read
                    if( aToken.isEmpty() && (bReadStyle || bReadScript) )
                    {
                        // Immediately close environment (or context?)
                        // and parse the end token
                        bReadScript = false;
                        bReadStyle = false;
                        aEndToken.clear();
                        nToken = 0;
                    }
                    else
                    {
                        // Keep bReadScript/bReadStyle alive
                        // and parse end token during next execution
                        bEndTokenFound = true;
                    }

                    // Move backwards in stream to '<'
                    rInput.Seek( nStreamPos );
                    SetLineNr( nLineNr );
                    SetLinePos( nLinePos );
                    ClearTxtConvContext();
                    nNextCh = '<';

                    // Don't append string to token.
                    sTmpBuffer.setLength( 0L );
                }
                else
                {
                    // remember "</" , everything else we find in the buffer
                    aToken += "<";
                    if( bOffState )
                        aToken += "/";

                    bNextCh = false;
                }
            }
            break;
        case '-':
            sTmpBuffer.append( nNextCh );
            if( bReadComment )
            {
                bool bTwoMinus = false;
                nNextCh = GetNextChar();
                while( '-' == nNextCh && IsParserWorking() )
                {
                    bTwoMinus = true;

                    if( MAX_LEN == sTmpBuffer.getLength() )
                        aToken += sTmpBuffer.makeStringAndClear();
                    sTmpBuffer.append( nNextCh );
                    nNextCh = GetNextChar();
                }

                if( '>' == nNextCh && IsParserWorking() && bTwoMinus )
                    bReadComment = false;

                bNextCh = false;
            }
            break;

        case '\r':
            // \r\n? closes the current text token (even if it's empty)
            nNextCh = GetNextChar();
            if( nNextCh=='\n' )
                nNextCh = GetNextChar();
            bContinue = false;
            break;
        case '\n':
            // \n closes the current text token (even if it's empty)
            nNextCh = GetNextChar();
            bContinue = false;
            break;
        case sal_Unicode(EOF):
            // eof closes the current text token and behaves like having read
            // an end token
            if( rInput.IsEof() )
            {
                bContinue = false;
                if( !aToken.isEmpty() || !sTmpBuffer.isEmpty() )
                {
                    bEndTokenFound = true;
                }
                else
                {
                    bReadScript = false;
                    bReadStyle = false;
                    aEndToken.clear();
                    nToken = 0;
                }
                break;
            }
            // no break
        default:
            // all remaining characters are appended to the buffer
            sTmpBuffer.append( nNextCh );
            break;
        }

        if( (!bContinue && !sTmpBuffer.isEmpty()) ||
            MAX_LEN == sTmpBuffer.getLength() )
            aToken += sTmpBuffer.makeStringAndClear();

        if( bContinue && bNextCh )
            nNextCh = GetNextChar();
    }

    if( IsParserWorking() )
        SaveState( 0 );
    else
        nToken = 0;

    return nToken;
}

// Scan next token
int HTMLParser::_GetNextToken()
{
    int nRet = 0;
    sSaveToken.clear();

    if (mnPendingOffToken)
    {
        // HTML_<TOKEN>_OFF generated for HTML_<TOKEN>_ON
        nRet = mnPendingOffToken;
        mnPendingOffToken = 0;
        aToken.clear();
        return nRet;
    }

    // Delete options
    if (!maOptions.empty())
        maOptions.clear();

    if( !IsParserWorking() )        // Don't continue if already an error occurred
        return 0;

    bool bReadNextCharSave = bReadNextChar;
    if( bReadNextChar )
    {
        DBG_ASSERT( !bEndTokenFound,
                    "Read a character despite </SCRIPT> was read?" );
        nNextCh = GetNextChar();
        if( !IsParserWorking() )        // Don't continue if already an error occurred
            return 0;
        bReadNextChar = false;
    }

    if( bReadScript || bReadStyle || !aEndToken.isEmpty() )
    {
        nRet = _GetNextRawToken();
        if( nRet || !IsParserWorking() )
            return nRet;
    }

    do {
        bool bNextCh = true;
        switch( nNextCh )
        {
        case '<':
            {
                sal_uLong nStreamPos = rInput.Tell();
                sal_uLong nLineNr = GetLineNr();
                sal_uLong nLinePos = GetLinePos();

                bool bOffState = false;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = true;
                    nNextCh = GetNextChar();
                }
                if( HTML_ISALPHA( nNextCh ) || '!'==nNextCh )
                {
                    OUStringBuffer sTmpBuffer;
                    do {
                        sTmpBuffer.append( nNextCh );
                        if( MAX_LEN == sTmpBuffer.getLength() )
                            aToken += sTmpBuffer.makeStringAndClear();
                        nNextCh = GetNextChar();
                    } while( '>' != nNextCh && '/' != nNextCh && !HTML_ISSPACE( nNextCh ) &&
                             IsParserWorking() && !rInput.IsEof() );

                    if( !sTmpBuffer.isEmpty() )
                        aToken += sTmpBuffer.makeStringAndClear();

                    // Skip blanks
                    while( HTML_ISSPACE( nNextCh ) && IsParserWorking() )
                        nNextCh = GetNextChar();

                    if( !IsParserWorking() )
                    {
                        if( SVPAR_PENDING == eState )
                            bReadNextChar = bReadNextCharSave;
                        break;
                    }

                    // Search token in table:
                    sSaveToken = aToken;
                    aToken = aToken.toAsciiLowerCase();
                    if( 0 == (nRet = GetHTMLToken( aToken )) )
                        // Unknown control
                        nRet = HTML_UNKNOWNCONTROL_ON;

                    // If it's a token which can be switched off...
                    if( bOffState )
                    {
                         if( HTML_TOKEN_ONOFF & nRet )
                         {
                            // and there is an off token, return off token instead
                            ++nRet;
                         }
                         else if( HTML_LINEBREAK!=nRet )
                         {
                            // and there is no off token, return unknown token.
                            // (except for </BR>, that is treated like <BR>)
                            nRet = HTML_UNKNOWNCONTROL_OFF;
                         }
                    }

                    if( nRet == HTML_COMMENT )
                    {
                        // fix: due to being case sensitive use sSaveToken as start of comment
                        //      and append a blank.
                        aToken = sSaveToken;
                        if( '>'!=nNextCh )
                            aToken += " ";
                        sal_uLong nCStreamPos = 0;
                        sal_uLong nCLineNr = 0;
                        sal_uLong nCLinePos = 0;
                        sal_Int32 nCStrLen = 0;

                        bool bDone = false;
                        // Read until closing -->. If not found restart at first >
                        while( !bDone && !rInput.IsEof() && IsParserWorking() )
                        {
                            if( '>'==nNextCh )
                            {
                                if( !nCStreamPos )
                                {
                                    nCStreamPos = rInput.Tell();
                                    nCStrLen = aToken.getLength();
                                    nCLineNr = GetLineNr();
                                    nCLinePos = GetLinePos();
                                }
                                bDone = aToken.endsWith( "--" );
                                if( !bDone )
                                aToken += OUString(nNextCh);
                            }
                            else
                                aToken += OUString(nNextCh);
                            if( !bDone )
                                nNextCh = GetNextChar();
                        }
                        if( !bDone && IsParserWorking() && nCStreamPos )
                        {
                            rInput.Seek( nCStreamPos );
                            SetLineNr( nCLineNr );
                            SetLinePos( nCLinePos );
                            ClearTxtConvContext();
                            aToken = aToken.copy(0, nCStrLen);
                            nNextCh = '>';
                        }
                    }
                    else
                    {
                        // TokenString not needed anymore
                        aToken.clear();
                    }

                    // Read until closing '>'
                    if( '>' != nNextCh && IsParserWorking() )
                    {
                        ScanText( '>' );

                        // fdo#34666 fdo#36080 fdo#36390: closing "/>"?:
                        // generate pending HTML_<TOKEN>_OFF for HTML_<TOKEN>_ON
                        // Do not convert this to a single HTML_<TOKEN>_OFF
                        // which lead to fdo#56772.
                        if ((HTML_TOKEN_ONOFF & nRet) && aToken.endsWith("/"))
                        {
                            mnPendingOffToken = nRet + 1;       // HTML_<TOKEN>_ON -> HTML_<TOKEN>_OFF
                            aToken = aToken.replaceAt( aToken.getLength()-1, 1, "");   // remove trailing '/'
                        }
                        if( sal_Unicode(EOF) == nNextCh && rInput.IsEof() )
                        {
                            // Move back in front of < and restart there.
                            // Return < as text.
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );
                            ClearTxtConvContext();

                            aToken = "<";
                            nRet = HTML_TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = false;
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
                            // Move back in front of < and restart there.
                            // Return < as text.
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );
                            ClearTxtConvContext();

                            aToken = "<";
                            nRet = HTML_TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = false;
                            break;
                        }
                        if( SVPAR_PENDING == eState )
                            bReadNextChar = bReadNextCharSave;
                        aToken.clear();
                    }
                    else if( '%' == nNextCh )
                    {
                        nRet = HTML_UNKNOWNCONTROL_ON;

                        sal_uLong nCStreamPos = rInput.Tell();
                        sal_uLong nCLineNr = GetLineNr(), nCLinePos = GetLinePos();

                        bool bDone = false;
                        // Read until closing %>. If not found restart at first >.
                        while( !bDone && !rInput.IsEof() && IsParserWorking() )
                        {
                            bDone = '>'==nNextCh && aToken.endsWith("%");
                            if( !bDone )
                            {
                                aToken += OUString(nNextCh);
                                nNextCh = GetNextChar();
                            }
                        }
                        if( !bDone && IsParserWorking() )
                        {
                            rInput.Seek( nCStreamPos );
                            SetLineNr( nCLineNr );
                            SetLinePos( nCLinePos );
                            ClearTxtConvContext();
                            aToken = "<%";
                            nRet = HTML_TEXTTOKEN;
                            break;
                        }
                        if( IsParserWorking() )
                        {
                            sSaveToken = aToken;
                            aToken.clear();
                        }
                    }
                    else
                    {
                        aToken = "<";
                        nRet = HTML_TEXTTOKEN;
                        bNextCh = false;
                        break;
                    }
                }

                if( IsParserWorking() )
                {
                    bNextCh = '>' == nNextCh;
                    switch( nRet )
                    {
                    case HTML_TEXTAREA_ON:
                        bReadTextArea = true;
                        break;
                    case HTML_TEXTAREA_OFF:
                        bReadTextArea = false;
                        break;
                    case HTML_SCRIPT_ON:
                        if( !bReadTextArea )
                            bReadScript = true;
                        break;
                    case HTML_SCRIPT_OFF:
                        if( !bReadTextArea )
                        {
                            bReadScript = false;
                            // JavaScript might modify the stream,
                            // thus the last character has to be read again.
                            bReadNextChar = true;
                            bNextCh = false;
                        }
                        break;

                    case HTML_STYLE_ON:
                        bReadStyle = true;
                        break;
                    case HTML_STYLE_OFF:
                        bReadStyle = false;
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
                // Read normal text.
                goto scan_text;
            }
            break;

        case '\f':
            // form feeds are passed upwards separately
            nRet = HTML_LINEFEEDCHAR; // !!! should be FORMFEEDCHAR
            break;

        case '\n':
        case '\r':
            if( bReadListing || bReadXMP || bReadPRE || bReadTextArea )
            {
                sal_Unicode c = GetNextChar();
                if( ( '\n' != nNextCh || '\r' != c ) &&
                    ( '\r' != nNextCh || '\n' != c ) )
                {
                    bNextCh = false;
                    nNextCh = c;
                }
                nRet = HTML_NEWPARA;
                break;
            }
            // no break !
        case '\t':
            if( bReadPRE )
            {
                nRet = HTML_TABCHAR;
                break;
            }
            // no break !
        case ' ':
            // no break !
        default:

scan_text:
            // "normal" text to come
            nRet = ScanText();
            bNextCh = 0 == aToken.getLength();

            // the text should be processed
            if( !bNextCh && eState == SVPAR_PENDING )
            {
                eState = SVPAR_WORKING;
                bReadNextChar = true;
            }

            break;
        }

        if( bNextCh && SVPAR_WORKING == eState )
        {
            nNextCh = GetNextChar();
            if( SVPAR_PENDING == eState && nRet && HTML_TEXTTOKEN != nRet )
            {
                bReadNextChar = true;
                eState = SVPAR_WORKING;
            }
        }

    } while( !nRet && SVPAR_WORKING == eState );

    if( SVPAR_PENDING == eState )
        nRet = -1;      // s.th. invalid

    return nRet;
}

void HTMLParser::UnescapeToken()
{
    sal_Int32 nPos=0;

    bool bEscape = false;
    while( nPos < aToken.getLength() )
    {
        bool bOldEscape = bEscape;
        bEscape = false;
        if( '\\'==aToken[nPos] && !bOldEscape )
        {
            aToken = aToken.replaceAt( nPos, 1, "" );
            bEscape = true;
        }
        else
        {
            nPos++;
        }
    }
}

const HTMLOptions& HTMLParser::GetOptions( sal_uInt16 *pNoConvertToken )
{
    // If the options for the current token have already been returned,
    // return them once again.
    if (!maOptions.empty())
        return maOptions;

    sal_Int32 nPos = 0;
    while( nPos < aToken.getLength() )
    {
        // A letter? Option beginning here.
        if( HTML_ISALPHA( aToken[nPos] ) )
        {
            int nToken;
            OUString aValue;
            sal_Int32 nStt = nPos;
            sal_Unicode cChar = 0;

            // Actually only certain characters allowed.
            // Netscape only looks for "=" and white space (c.f.
            // Mozilla: PA_FetchRequestedNameValues in libparse/pa_mdl.c)
            while( nPos < aToken.getLength() && '=' != (cChar=aToken[nPos]) &&
                   HTML_ISPRINTABLE(cChar) && !HTML_ISSPACE(cChar) )
                nPos++;

            OUString sName( aToken.copy( nStt, nPos-nStt ) );

            // PlugIns require original token name. Convert to lower case only for searching.
            nToken = GetHTMLOption( sName.toAsciiLowerCase() ); // Name is ready
            SAL_WARN_IF( nToken==HTML_O_UNKNOWN, "svtools",
                        "GetOption: unknown HTML option" );
            bool bStripCRLF = (nToken < HTML_OPTION_SCRIPT_START ||
                               nToken >= HTML_OPTION_SCRIPT_END) &&
                              (!pNoConvertToken || nToken != *pNoConvertToken);

            while( nPos < aToken.getLength() &&
                   ( !HTML_ISPRINTABLE( (cChar=aToken[nPos]) ) ||
                     HTML_ISSPACE(cChar) ) )
                nPos++;

            // Option with value?
            if( nPos!=aToken.getLength() && '='==cChar )
            {
                nPos++;

                while( nPos < aToken.getLength() &&
                        ( !HTML_ISPRINTABLE( (cChar=aToken[nPos]) ) ||
                          ' '==cChar || '\t'==cChar || '\r'==cChar || '\n'==cChar ) )
                    nPos++;

                if( nPos != aToken.getLength() )
                {
                    sal_Int32 nLen = 0;
                    nStt = nPos;
                    if( ('"'==cChar) || ('\'')==cChar )
                    {
                        sal_Unicode cEnd = cChar;
                        nPos++; nStt++;
                        bool bDone = false;
                        bool bEscape = false;
                        while( nPos < aToken.getLength() && !bDone )
                        {
                            bool bOldEscape = bEscape;
                            bEscape = false;
                            cChar = aToken[nPos];
                            switch( cChar )
                            {
                            case '\r':
                            case '\n':
                                if( bStripCRLF )
                                    aToken = aToken.replaceAt( nPos, 1, "" );
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
                                    aToken = aToken.replaceAt( nPos, 1, "" );
                                    bEscape = true;
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
                        if( nPos!=aToken.getLength() )
                            nPos++;
                    }
                    else
                    {
                        // More liberal than the standard: allow all printable characters
                        bool bEscape = false;
                        bool bDone = false;
                        while( nPos < aToken.getLength() && !bDone )
                        {
                            bool bOldEscape = bEscape;
                            bEscape = false;
                            sal_Unicode c = aToken[nPos];
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
                                bDone = true;
                                break;

                            case '\\':
                                if( bOldEscape )
                                {
                                    nPos++, nLen++;
                                }
                                else
                                {
                                    aToken = aToken.replaceAt( nPos, 1, "" );
                                    bEscape = true;
                                }
                                break;

                            default:
                                if( HTML_ISPRINTABLE( c ) )
                                    nPos++, nLen++;
                                else
                                    bDone = true;
                                break;
                            }
                        }
                    }

                    if( nLen )
                        aValue = aToken.copy( nStt, nLen );
                }
            }

            // Token is known and can be saved
            maOptions.push_back(
                HTMLOption(sal::static_int_cast<sal_uInt16>(nToken), sName, aValue));

        }
        else
            // Ignore white space and unexpected characters
            nPos++;
    }

    return maOptions;
}

int HTMLParser::FilterPRE( int nToken )
{
    switch( nToken )
    {
    // in Netscape they only have impact in not empty paragraphs
    case HTML_PARABREAK_ON:
        nToken = HTML_LINEBREAK;
        //fall-through
    case HTML_LINEBREAK:
    case HTML_NEWPARA:
        nPre_LinePos = 0;
        if( bPre_IgnoreNewPara )
            nToken = 0;
        break;

    case HTML_TABCHAR:
        {
            sal_Int32 nSpaces = (8 - (nPre_LinePos % 8));
            DBG_ASSERT( aToken.isEmpty(), "Why is the token not empty?" );
            if (aToken.getLength() < nSpaces)
            {
                using comphelper::string::padToLength;
                OUStringBuffer aBuf(aToken);
                aToken = padToLength(aBuf, nSpaces, ' ').makeStringAndClear();
            }
            nPre_LinePos += nSpaces;
            nToken = HTML_TEXTTOKEN;
        }
        break;
    // Keep those
    case HTML_TEXTTOKEN:
        nPre_LinePos += aToken.getLength();
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

    // The remainder is treated as an unknown token.
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

    bPre_IgnoreNewPara = false;

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
        break;              // kept

    default:
        if( nToken )
        {
            if( (HTML_TOKEN_ONOFF & nToken) && (1 & nToken) )
            {
                sSaveToken = "</" + sSaveToken;
            }
            else
                sSaveToken = "<" + sSaveToken;
            if( !aToken.isEmpty() )
            {
                UnescapeToken();
                sSaveToken += " ";
                aToken = sSaveToken + aToken;
            }
            else
                aToken = sSaveToken;
            aToken += ">";
            nToken = HTML_TEXTTOKEN;
        }
        break;
    }

    bPre_IgnoreNewPara = false;

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
        break;      // kept

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

    bPre_IgnoreNewPara = false;

    return nToken;
}

bool HTMLParser::InternalImgToPrivateURL( OUString& rURL )
{
    bool bFound = false;

    if( rURL.startsWith( OOO_STRING_SVTOOLS_HTML_internal_icon ) )
    {
        OUString aName( rURL.copy(14) );
        switch( aName[0] )
        {
        case 'b':
            bFound = aName == OOO_STRING_SVTOOLS_HTML_INT_ICON_baddata;
            break;
        case 'd':
            bFound = aName == OOO_STRING_SVTOOLS_HTML_INT_ICON_delayed;
            break;
        case 'e':
            bFound = aName == OOO_STRING_SVTOOLS_HTML_INT_ICON_embed;
            break;
        case 'i':
            bFound = aName == OOO_STRING_SVTOOLS_HTML_INT_ICON_insecure;
            break;
        case 'n':
            bFound = aName == OOO_STRING_SVTOOLS_HTML_INT_ICON_notfound;
            break;
        }
    }
    if( bFound )
    {
        OUString sTmp ( rURL );
        rURL =  OOO_STRING_SVTOOLS_HTML_private_image;
        rURL += sTmp;
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
    { nullptr,                                          0                       }
};


void HTMLParser::AddMetaUserDefined( OUString const & )
{
}

bool HTMLParser::ParseMetaOptionsImpl(
        const uno::Reference<document::XDocumentProperties> & i_xDocProps,
        SvKeyValueIterator *i_pHTTPHeader,
        const HTMLOptions& aOptions,
        rtl_TextEncoding& o_rEnc )
{
    OUString aName, aContent;
    sal_uInt16 nAction = HTML_META_NONE;
    bool bHTTPEquiv = false, bChanged = false;

    for ( size_t i = aOptions.size(); i; )
    {
        const HTMLOption& aOption = aOptions[--i];
        switch ( aOption.GetToken() )
        {
            case HTML_O_NAME:
                aName = aOption.GetString();
                if ( HTML_META_NONE==nAction )
                {
                    aOption.GetEnum( nAction, aHTMLMetaNameTable );
                }
                break;
            case HTML_O_HTTPEQUIV:
                aName = aOption.GetString();
                aOption.GetEnum( nAction, aHTMLMetaNameTable );
                bHTTPEquiv = true;
                break;
            case HTML_O_CONTENT:
                aContent = aOption.GetString();
                break;
        }
    }

    if ( bHTTPEquiv || HTML_META_DESCRIPTION != nAction )
    {
        // if it is not a Description, remove CRs and LFs from CONTENT
        aContent = comphelper::string::remove(aContent, '\r');
        aContent = comphelper::string::remove(aContent, '\n');
    }
    else
    {
        // convert line endings for Description
        aContent = convertLineEnd(aContent, GetSystemLineEnd());
    }


    if ( bHTTPEquiv && i_pHTTPHeader )
    {
        // Netscape seems to just ignore a closing ", so we do too
        if ( aContent.endsWith("\"") )
        {
            aContent = aContent.copy( 0, aContent.getLength() - 1 );
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
            if ( i_xDocProps.is() && !aContent.isEmpty() &&
                 comphelper::string::getTokenCount(aContent, ';') == 2 )
            {
                Date aDate( (sal_uLong)aContent.getToken(0, ';').toInt32() );
                tools::Time aTime( (sal_uLong)aContent.getToken(1, ';').toInt32() );
                DateTime aDateTime( aDate, aTime );
                ::util::DateTime uDT = aDateTime.GetUNODateTime();
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
            if ( !aContent.isEmpty() )
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
                            beans::PropertyAttribute::REMOVABLE,
                            uno::makeAny(OUString(aContent)));
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
        eEnc = GetExtendedCompatibilityTextEncoding( eEnc );
        SetSrcEncoding( eEnc );
    }

    return bRet;
}

rtl_TextEncoding HTMLParser::GetEncodingByMIME( const OUString& rMime )
{
    OUString sType;
    OUString sSubType;
    INetContentTypeParameterList aParameters;
    if (INetContentTypes::parse(rMime, sType, sSubType, &aParameters))
    {
        auto const iter = aParameters.find("charset");
        if (iter != aParameters.end())
        {
            const INetContentTypeParameter * pCharset = &iter->second;
            OString sValue(OUStringToOString(pCharset->m_sValue, RTL_TEXTENCODING_ASCII_US));
            return GetExtendedCompatibilityTextEncoding( rtl_getTextEncodingFromMimeCharset( sValue.getStr() ) );
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
        for( bool bCont = pHTTPHeader->GetFirst( aKV ); bCont;
             bCont = pHTTPHeader->GetNext( aKV ) )
        {
            if( aKV.GetKey().equalsIgnoreAsciiCase( OOO_STRING_SVTOOLS_HTML_META_content_type ) )
            {
                if( !aKV.GetValue().isEmpty() )
                {
                    eRet = HTMLParser::GetEncodingByMIME( aKV.GetValue() );
                }
            }
        }
    }
    return eRet;
}

bool HTMLParser::SetEncodingByHTTPHeader( SvKeyValueIterator *pHTTPHeader )
{
    bool bRet = false;
    rtl_TextEncoding eEnc = HTMLParser::GetEncodingByHttpHeader( pHTTPHeader );
    if(RTL_TEXTENCODING_DONTKNOW != eEnc)
    {
        SetSrcEncoding( eEnc );
        bRet = true;
    }
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
