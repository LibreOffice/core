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

#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/color.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/character.hxx>
#include <rtl/tencinfo.h>
#include <tools/tenccvt.hxx>
#include <tools/datetime.hxx>
#include <unotools/datetime.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <svtools/parhtml.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>

#include <memory>
#include <utility>

using namespace ::com::sun::star;


const sal_Int32 MAX_LEN( 1024 );

const sal_Int32 MAX_ENTITY_LEN( 8 );


// Tables to convert option values into strings

// <INPUT TYPE=xxx>
static HTMLOptionEnum<HTMLInputType> const aInputTypeOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_IT_text,      HTMLInputType::Text        },
    { OOO_STRING_SVTOOLS_HTML_IT_password,  HTMLInputType::Password    },
    { OOO_STRING_SVTOOLS_HTML_IT_checkbox,  HTMLInputType::Checkbox    },
    { OOO_STRING_SVTOOLS_HTML_IT_radio,     HTMLInputType::Radio       },
    { OOO_STRING_SVTOOLS_HTML_IT_range,     HTMLInputType::Range       },
    { OOO_STRING_SVTOOLS_HTML_IT_scribble,  HTMLInputType::Scribble    },
    { OOO_STRING_SVTOOLS_HTML_IT_file,      HTMLInputType::File        },
    { OOO_STRING_SVTOOLS_HTML_IT_hidden,    HTMLInputType::Hidden      },
    { OOO_STRING_SVTOOLS_HTML_IT_submit,    HTMLInputType::Submit      },
    { OOO_STRING_SVTOOLS_HTML_IT_image,     HTMLInputType::Image       },
    { OOO_STRING_SVTOOLS_HTML_IT_reset,     HTMLInputType::Reset       },
    { OOO_STRING_SVTOOLS_HTML_IT_button,    HTMLInputType::Button      },
    { nullptr,                              HTMLInputType(0)    }
};

// <TABLE FRAME=xxx>
static HTMLOptionEnum<HTMLTableFrame> const aTableFrameOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_TF_void,    HTMLTableFrame::Void    },
    { OOO_STRING_SVTOOLS_HTML_TF_above,   HTMLTableFrame::Above   },
    { OOO_STRING_SVTOOLS_HTML_TF_below,   HTMLTableFrame::Below   },
    { OOO_STRING_SVTOOLS_HTML_TF_hsides,  HTMLTableFrame::HSides  },
    { OOO_STRING_SVTOOLS_HTML_TF_lhs,     HTMLTableFrame::LHS     },
    { OOO_STRING_SVTOOLS_HTML_TF_rhs,     HTMLTableFrame::RHS     },
    { OOO_STRING_SVTOOLS_HTML_TF_vsides,  HTMLTableFrame::VSides  },
    { OOO_STRING_SVTOOLS_HTML_TF_box,     HTMLTableFrame::Box     },
    { OOO_STRING_SVTOOLS_HTML_TF_border,  HTMLTableFrame::Box     },
    { nullptr,                            HTMLTableFrame(0) }
};

// <TABLE RULES=xxx>
static HTMLOptionEnum<HTMLTableRules> const aTableRulesOptEnums[] =
{
    { OOO_STRING_SVTOOLS_HTML_TR_none,   HTMLTableRules::NONE      },
    { OOO_STRING_SVTOOLS_HTML_TR_groups, HTMLTableRules::Groups    },
    { OOO_STRING_SVTOOLS_HTML_TR_rows,   HTMLTableRules::Rows      },
    { OOO_STRING_SVTOOLS_HTML_TR_cols,   HTMLTableRules::Cols      },
    { OOO_STRING_SVTOOLS_HTML_TR_all,    HTMLTableRules::All       },
    { nullptr,                           HTMLTableRules(0) }
};


HTMLOption::HTMLOption( HtmlOptionId nTok, const OUString& rToken,
                        const OUString& rValue )
    : aValue(rValue)
    , aToken(rToken)
    , nToken( nTok )
{
    DBG_ASSERT( nToken>=HtmlOptionId::BOOL_START && nToken<HtmlOptionId::END,
        "HTMLOption: unknown token" );
}

sal_uInt32 HTMLOption::GetNumber() const
{
    DBG_ASSERT( (nToken>=HtmlOptionId::NUMBER_START &&
                 nToken<HtmlOptionId::NUMBER_END) ||
                (nToken>=HtmlOptionId::CONTEXT_START &&
                 nToken<HtmlOptionId::CONTEXT_END) ||
                nToken==HtmlOptionId::VALUE,
        "GetNumber: Option not numerical" );
    OUString aTmp(comphelper::string::stripStart(aValue, ' '));
    sal_Int32 nTmp = aTmp.toInt32();
    return nTmp >= 0 ? static_cast<sal_uInt32>(nTmp) : 0;
}

sal_Int32 HTMLOption::GetSNumber() const
{
    DBG_ASSERT( (nToken>=HtmlOptionId::NUMBER_START && nToken<HtmlOptionId::NUMBER_END) ||
                (nToken>=HtmlOptionId::CONTEXT_START && nToken<HtmlOptionId::CONTEXT_END),
        "GetSNumber: Option not numerical" );
    OUString aTmp(comphelper::string::stripStart(aValue, ' '));
    return aTmp.toInt32();
}

void HTMLOption::GetNumbers( std::vector<sal_uInt32> &rNumbers ) const
{
    rNumbers.clear();

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

void HTMLOption::GetColor( Color& rColor ) const
{
    DBG_ASSERT( (nToken>=HtmlOptionId::COLOR_START && nToken<HtmlOptionId::COLOR_END) || nToken==HtmlOptionId::SIZE,
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

    rColor.SetRed(   static_cast<sal_uInt8>((nColor & 0x00ff0000) >> 16) );
    rColor.SetGreen( static_cast<sal_uInt8>((nColor & 0x0000ff00) >> 8));
    rColor.SetBlue(  static_cast<sal_uInt8>(nColor & 0x000000ff) );
}

HTMLInputType HTMLOption::GetInputType() const
{
    DBG_ASSERT( nToken==HtmlOptionId::TYPE, "GetInputType: Option not TYPE" );
    return GetEnum( aInputTypeOptEnums, HTMLInputType::Text );
}

HTMLTableFrame HTMLOption::GetTableFrame() const
{
    DBG_ASSERT( nToken==HtmlOptionId::FRAME, "GetTableFrame: Option not FRAME" );
    return GetEnum( aTableFrameOptEnums );
}

HTMLTableRules HTMLOption::GetTableRules() const
{
    DBG_ASSERT( nToken==HtmlOptionId::RULES, "GetTableRules: Option not RULES" );
    return GetEnum( aTableRulesOptEnums );
}

HTMLParser::HTMLParser( SvStream& rIn, bool bReadNewDoc ) :
    SvParser<HtmlTokenId>( rIn ),
    bNewDoc(bReadNewDoc),
    bIsInHeader(true),
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
    mnPendingOffToken(HtmlTokenId::NONE)
{
    //#i76649, default to UTF-8 for HTML unless we know differently
    SetSrcEncoding(RTL_TEXTENCODING_UTF8);
}

HTMLParser::~HTMLParser()
{
}

void HTMLParser::SetNamespace(const OUString& rNamespace)
{
    // Convert namespace alias to a prefix.
    maNamespace = rNamespace + ":";
}

namespace
{
    class RefGuard
    {
    private:
        HTMLParser& m_rParser;
    public:
        RefGuard(HTMLParser& rParser)
            : m_rParser(rParser)
        {
            m_rParser.AddFirstRef();
        }

        ~RefGuard()
        {
            if (m_rParser.GetStatus() != SvParserState::Pending)
                m_rParser.ReleaseRef(); // Parser not needed anymore
        }
    };
}

SvParserState HTMLParser::CallParser()
{
    eState = SvParserState::Working;
    nNextCh = GetNextChar();
    SaveState( HtmlTokenId::NONE );

    nPre_LinePos = 0;
    bPre_IgnoreNewPara = false;

    RefGuard aRefGuard(*this);

    Continue( HtmlTokenId::NONE );

    return eState;
}

void HTMLParser::Continue( HtmlTokenId nToken )
{
    if( nToken == HtmlTokenId::NONE )
        nToken = GetNextToken();

    while( IsParserWorking() )
    {
        SaveState( nToken );
        nToken = FilterToken( nToken );

        if( nToken != HtmlTokenId::NONE )
            NextToken( nToken );

        if( IsParserWorking() )
            SaveState( HtmlTokenId::NONE );         // continue with new token

        nToken = GetNextToken();
    }
}

HtmlTokenId HTMLParser::FilterToken( HtmlTokenId nToken )
{
    switch( nToken )
    {
    case HtmlTokenId(EOF):
        nToken = HtmlTokenId::NONE;
        break;          // don't pass

    case HtmlTokenId::HEAD_OFF:
        bIsInHeader = false;
        break;

    case HtmlTokenId::HEAD_ON:
        bIsInHeader = true;
        break;

    case HtmlTokenId::BODY_ON:
        bIsInHeader = false;
        break;

    case HtmlTokenId::FRAMESET_ON:
        bIsInHeader = false;
        break;

    case HtmlTokenId::BODY_OFF:
        bReadPRE = bReadListing = bReadXMP = false;
        break;

    case HtmlTokenId::HTML_OFF:
        nToken = HtmlTokenId::NONE;
        bReadPRE = bReadListing = bReadXMP = false;
        break;      // HtmlTokenId::ON hasn't been passed either !

    case HtmlTokenId::PREFORMTXT_ON:
        StartPRE();
        break;

    case HtmlTokenId::PREFORMTXT_OFF:
        FinishPRE();
        break;

    case HtmlTokenId::LISTING_ON:
        StartListing();
        break;

    case HtmlTokenId::LISTING_OFF:
        FinishListing();
        break;

    case HtmlTokenId::XMP_ON:
        StartXMP();
        break;

    case HtmlTokenId::XMP_OFF:
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

namespace {

constexpr bool HTML_ISPRINTABLE(sal_Unicode c) { return c >= 32 && c != 127; }

}

HtmlTokenId HTMLParser::ScanText( const sal_Unicode cBreak )
{
    OUStringBuffer sTmpBuffer( MAX_LEN );
    bool bContinue = true;
    bool bEqSignFound = false;
    sal_uInt32  cQuote = 0U;

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
                sal_uInt64 nStreamPos = rInput.Tell();
                sal_uLong nLinePos = GetLinePos();

                sal_uInt32 cChar = 0U;
                if( '#' == (nNextCh = GetNextChar()) )
                {
                    nNextCh = GetNextChar();
                    const bool bIsHex( 'x' == nNextCh );
                    const bool bIsDecOrHex( bIsHex || rtl::isAsciiDigit(nNextCh) );
                    if ( bIsDecOrHex )
                    {
                        if ( bIsHex )
                        {
                            nNextCh = GetNextChar();
                            while ( rtl::isAsciiHexDigit(nNextCh) )
                            {
                                cChar = cChar * 16U +
                                        ( nNextCh <= '9'
                                          ? sal_uInt32( nNextCh - '0' )
                                          : ( nNextCh <= 'F'
                                              ? sal_uInt32( nNextCh - 'A' + 10 )
                                              : sal_uInt32( nNextCh - 'a' + 10 ) ) );
                                nNextCh = GetNextChar();
                            }
                        }
                        else
                        {
                            do
                            {
                                cChar = cChar * 10U + sal_uInt32( nNextCh - '0');
                                nNextCh = GetNextChar();
                            }
                            while( rtl::isAsciiDigit(nNextCh) );
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

                    if ( ! rtl::isUnicodeCodePoint( cChar ) )
                        cChar = '?';
                }
                else if( rtl::isAsciiAlpha( nNextCh ) )
                {
                    OUStringBuffer sEntityBuffer( MAX_ENTITY_LEN );
                    sal_Int32 nPos = 0;
                    do
                    {
                        sEntityBuffer.appendUtf32( nNextCh );
                        nPos++;
                        nNextCh = GetNextChar();
                    }
                    while( nPos < MAX_ENTITY_LEN && rtl::isAsciiAlphanumeric( nNextCh ) &&
                           !rInput.eof() );

                    if( IsParserWorking() && !rInput.eof() )
                    {
                        OUString sEntity(sEntityBuffer.getStr(), nPos);
                        cChar = GetHTMLCharName( sEntity );

                        // not found ( == 0 ): plain text
                        // or a character which is inserted as attribute
                        if( 0U == cChar && ';' != nNextCh )
                        {
                            DBG_ASSERT( rInput.Tell() - nStreamPos ==
                                        static_cast<sal_uInt64>(nPos+1)*GetCharSize(),
                                        "UTF-8 is failing here" );
                            for( sal_Int32 i = nPos-1; i>1; i-- )
                            {
                                nNextCh = sEntityBuffer[i];
                                sEntityBuffer.setLength( i );
                                sEntity = OUString(sEntityBuffer.getStr(), i);
                                cChar = GetHTMLCharName( sEntity );
                                if( cChar )
                                {
                                    rInput.SeekRel( -static_cast<sal_Int64>
                                            (nPos-i)*GetCharSize() );
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
                                        static_cast<sal_uInt64>(nPos+1)*GetCharSize(),
                                        "Wrong stream position" );
                            DBG_ASSERT( nlLinePos-nLinePos ==
                                        static_cast<sal_uLong>(nPos+1),
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
                                                static_cast<sal_uInt64>(nPos+1)*GetCharSize(),
                                                "Wrong stream position" );
                                    DBG_ASSERT( nlLinePos-nLinePos ==
                                                static_cast<sal_uLong>(nPos+1),
                                                "Wrong line position" );
                                    rInput.Seek( nStreamPos );
                                    nlLinePos = nLinePos;
                                    ClearTxtConvContext();
                                    return HtmlTokenId::TEXTTOKEN;
                                }

                                // Hack: _GetNextChar shall not read the
                                // next character
                                if( ';' != nNextCh )
                                    aToken += " ";
                                if( 1U == cChar )
                                    return HtmlTokenId::NONBREAKSPACE;
                                else //2U
                                    return HtmlTokenId::SOFTHYPH;
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
                        sTmpBuffer.appendUtf32( cChar );
                }
                else if( SvParserState::Pending==eState && '>'!=cBreak )
                {
                    // Restart with '&', the remainder is returned as
                    // text token.
                    if( !aToken.isEmpty() || !sTmpBuffer.isEmpty() )
                    {
                        // _GetNextChar() returns the previous text and
                        // during the next execution a new character is read.
                        // Thus we have to position in front of the '&'.
                        nNextCh = 0U;
                        rInput.Seek( nStreamPos - GetCharSize() );
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
            sTmpBuffer.appendUtf32( nNextCh );
            break;

        case '\\':
            if( '>'==cBreak )
            {
                // mark within tags
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
            sTmpBuffer.appendUtf32( nNextCh );
            bEqSignFound = false;
            break;

        case sal_Unicode(EOF):
            if( rInput.eof() )
            {
                bContinue = false;
            }
            else
            {
                sTmpBuffer.appendUtf32( nNextCh );
            }

            break;

        case '<':
            bEqSignFound = false;
            if( '>'==cBreak )
                sTmpBuffer.appendUtf32( nNextCh );
            else
                bContinue = false;      // break, string is together
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
                // cr/lf in tag is handled in GetNextToken_()
                sTmpBuffer.appendUtf32( nNextCh );
                break;
            }
            else if( bReadListing || bReadXMP || bReadPRE || bReadTextArea )
            {
                bContinue = false;
                break;
            }
            // Reduce sequence of CR/LF/BLANK/TAB to a single blank
            SAL_FALLTHROUGH;
        case '\t':
            if( '\t'==nNextCh && bReadPRE && '>'!=cBreak )
            {
                // Pass Tabs up in <PRE>
                bContinue = false;
                break;
            }
            SAL_FALLTHROUGH;
        case '\x0b':
            if( '\x0b'==nNextCh && (bReadPRE || bReadXMP ||bReadListing) &&
                '>'!=cBreak )
            {
                break;
            }
            nNextCh = ' ';
            SAL_FALLTHROUGH;
        case ' ':
            sTmpBuffer.appendUtf32( nNextCh );
            if( '>'!=cBreak && (!bReadListing && !bReadXMP &&
                                !bReadPRE && !bReadTextArea) )
            {
                // Reduce sequences of Blanks/Tabs/CR/LF to a single blank
                do {
                    if( sal_Unicode(EOF) == (nNextCh = GetNextChar()) &&
                        rInput.eof() )
                    {
                        if( !aToken.isEmpty() || sTmpBuffer.getLength() > 1 )
                        {
                            // Have seen s.th. aside from blanks?
                            aToken += sTmpBuffer.makeStringAndClear();
                            return HtmlTokenId::TEXTTOKEN;
                        }
                        else
                            // Only read blanks: no text must be returned
                            // and GetNextToken_ has to read until EOF
                            return HtmlTokenId::NONE;
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
                    sTmpBuffer.appendUtf32( nNextCh );
                    if( MAX_LEN == sTmpBuffer.getLength() )
                    {
                        aToken += sTmpBuffer.makeStringAndClear();
                    }
                    if( ( sal_Unicode(EOF) == (nNextCh = GetNextChar()) &&
                          rInput.eof() ) ||
                        !IsParserWorking() )
                    {
                        if( !sTmpBuffer.isEmpty() )
                            aToken += sTmpBuffer.makeStringAndClear();
                        return HtmlTokenId::TEXTTOKEN;
                    }
                } while( rtl::isAsciiAlpha( nNextCh ) || rtl::isAsciiDigit( nNextCh ) );
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

    return HtmlTokenId::TEXTTOKEN;
}

HtmlTokenId HTMLParser::GetNextRawToken()
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

        return HtmlTokenId::NONE;
    }

    // Default return value: HtmlTokenId::RAWDATA
    bool bContinue = true;
    HtmlTokenId nToken = HtmlTokenId::RAWDATA;
    SaveState( HtmlTokenId::NONE );
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
                sal_uInt64 nStreamPos = rInput.Tell();
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
                    sTmpBuffer.appendUtf32( nNextCh );
                    nNextCh = GetNextChar();
                }

                // Read following letters
                while( (rtl::isAsciiAlpha(nNextCh) || '-'==nNextCh) &&
                       IsParserWorking() && sTmpBuffer.getLength() < MAX_LEN )
                {
                    sTmpBuffer.appendUtf32( nNextCh );
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
                                : aTok == aEndToken );
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

                    // nToken==0 means, GetNextToken_ continues to read
                    if( aToken.isEmpty() && (bReadStyle || bReadScript) )
                    {
                        // Immediately close environment (or context?)
                        // and parse the end token
                        bReadScript = false;
                        bReadStyle = false;
                        aEndToken.clear();
                        nToken = HtmlTokenId::NONE;
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
                    sTmpBuffer.setLength( 0 );
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
            sTmpBuffer.appendUtf32( nNextCh );
            if( bReadComment )
            {
                bool bTwoMinus = false;
                nNextCh = GetNextChar();
                while( '-' == nNextCh && IsParserWorking() )
                {
                    bTwoMinus = true;

                    if( MAX_LEN == sTmpBuffer.getLength() )
                        aToken += sTmpBuffer.makeStringAndClear();
                    sTmpBuffer.appendUtf32( nNextCh );
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
            if( rInput.eof() )
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
                    nToken = HtmlTokenId::NONE;
                }
                break;
            }
            SAL_FALLTHROUGH;
        default:
            // all remaining characters are appended to the buffer
            sTmpBuffer.appendUtf32( nNextCh );
            break;
        }

        if( (!bContinue && !sTmpBuffer.isEmpty()) ||
            MAX_LEN == sTmpBuffer.getLength() )
            aToken += sTmpBuffer.makeStringAndClear();

        if( bContinue && bNextCh )
            nNextCh = GetNextChar();
    }

    if( IsParserWorking() )
        SaveState( HtmlTokenId::NONE );
    else
        nToken = HtmlTokenId::NONE;

    return nToken;
}

// Scan next token
HtmlTokenId HTMLParser::GetNextToken_()
{
    HtmlTokenId nRet = HtmlTokenId::NONE;
    sSaveToken.clear();

    if (mnPendingOffToken != HtmlTokenId::NONE)
    {
        // HtmlTokenId::<TOKEN>_OFF generated for HtmlTokenId::<TOKEN>_ON
        nRet = mnPendingOffToken;
        mnPendingOffToken = HtmlTokenId::NONE;
        aToken.clear();
        return nRet;
    }

    // Delete options
    if (!maOptions.empty())
        maOptions.clear();

    if( !IsParserWorking() )        // Don't continue if already an error occurred
        return HtmlTokenId::NONE;

    bool bReadNextCharSave = bReadNextChar;
    if( bReadNextChar )
    {
        DBG_ASSERT( !bEndTokenFound,
                    "Read a character despite </SCRIPT> was read?" );
        nNextCh = GetNextChar();
        if( !IsParserWorking() )        // Don't continue if already an error occurred
            return HtmlTokenId::NONE;
        bReadNextChar = false;
    }

    if( bReadScript || bReadStyle || !aEndToken.isEmpty() )
    {
        nRet = GetNextRawToken();
        if( nRet != HtmlTokenId::NONE || !IsParserWorking() )
            return nRet;
    }

    do {
        bool bNextCh = true;
        switch( nNextCh )
        {
        case '<':
            {
                sal_uInt64 nStreamPos = rInput.Tell();
                sal_uLong nLineNr = GetLineNr();
                sal_uLong nLinePos = GetLinePos();

                bool bOffState = false;
                if( '/' == (nNextCh = GetNextChar()) )
                {
                    bOffState = true;
                    nNextCh = GetNextChar();
                }
                // Assume '<?' is a start of an XML declaration, ignore it.
                if (rtl::isAsciiAlpha(nNextCh) || nNextCh == '!' || nNextCh == '?')
                {
                    OUStringBuffer sTmpBuffer;
                    do {
                        sTmpBuffer.appendUtf32( nNextCh );
                        if( MAX_LEN == sTmpBuffer.getLength() )
                            aToken += sTmpBuffer.makeStringAndClear();
                        nNextCh = GetNextChar();
                    } while( '>' != nNextCh && '/' != nNextCh && !rtl::isAsciiWhiteSpace( nNextCh ) &&
                             IsParserWorking() && !rInput.eof() );

                    if( !sTmpBuffer.isEmpty() )
                        aToken += sTmpBuffer.makeStringAndClear();

                    // Skip blanks
                    while( rtl::isAsciiWhiteSpace( nNextCh ) && IsParserWorking() )
                        nNextCh = GetNextChar();

                    if( !IsParserWorking() )
                    {
                        if( SvParserState::Pending == eState )
                            bReadNextChar = bReadNextCharSave;
                        break;
                    }

                    // Search token in table:
                    sSaveToken = aToken;
                    aToken = aToken.toAsciiLowerCase();

                    if (!maNamespace.isEmpty() && aToken.startsWith(maNamespace))
                        aToken = aToken.copy(maNamespace.getLength());

                    if( HtmlTokenId::NONE == (nRet = GetHTMLToken( aToken )) )
                        // Unknown control
                        nRet = HtmlTokenId::UNKNOWNCONTROL_ON;

                    // If it's a token which can be switched off...
                    if( bOffState )
                    {
                         if( nRet >= HtmlTokenId::ONOFF_START )
                         {
                            // and there is an off token, return off token instead
                            nRet = static_cast<HtmlTokenId>(static_cast<int>(nRet) + 1);
                         }
                         else if( HtmlTokenId::LINEBREAK!=nRet )
                         {
                            // and there is no off token, return unknown token.
                            // (except for </BR>, that is treated like <BR>)
                            nRet = HtmlTokenId::UNKNOWNCONTROL_OFF;
                         }
                    }

                    if( nRet == HtmlTokenId::COMMENT )
                    {
                        // fix: due to being case sensitive use sSaveToken as start of comment
                        //      and append a blank.
                        aToken = sSaveToken;
                        if( '>'!=nNextCh )
                            aToken += " ";
                        sal_uInt64 nCStreamPos = 0;
                        sal_uLong nCLineNr = 0;
                        sal_uLong nCLinePos = 0;
                        sal_Int32 nCStrLen = 0;

                        bool bDone = false;
                        // Read until closing -->. If not found restart at first >
                        while( !bDone && !rInput.eof() && IsParserWorking() )
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
                                aToken += OUString(&nNextCh,1);
                            }
                            else
                                aToken += OUString(&nNextCh,1);
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
                        // generate pending HtmlTokenId::<TOKEN>_OFF for HtmlTokenId::<TOKEN>_ON
                        // Do not convert this to a single HtmlTokenId::<TOKEN>_OFF
                        // which lead to fdo#56772.
                        if ((nRet >= HtmlTokenId::ONOFF_START) && aToken.endsWith("/"))
                        {
                            mnPendingOffToken = static_cast<HtmlTokenId>(static_cast<int>(nRet) + 1);       // HtmlTokenId::<TOKEN>_ON -> HtmlTokenId::<TOKEN>_OFF
                            aToken = aToken.replaceAt( aToken.getLength()-1, 1, "");   // remove trailing '/'
                        }
                        if( sal_Unicode(EOF) == nNextCh && rInput.eof() )
                        {
                            // Move back in front of < and restart there.
                            // Return < as text.
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );
                            ClearTxtConvContext();

                            aToken = "<";
                            nRet = HtmlTokenId::TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = false;
                            break;
                        }
                    }
                    if( SvParserState::Pending == eState )
                        bReadNextChar = bReadNextCharSave;
                }
                else
                {
                    if( bOffState )
                    {
                        // simply throw away everything
                        ScanText( '>' );
                        if( sal_Unicode(EOF) == nNextCh && rInput.eof() )
                        {
                            // Move back in front of < and restart there.
                            // Return < as text.
                            rInput.Seek( nStreamPos );
                            SetLineNr( nLineNr );
                            SetLinePos( nLinePos );
                            ClearTxtConvContext();

                            aToken = "<";
                            nRet = HtmlTokenId::TEXTTOKEN;
                            nNextCh = GetNextChar();
                            bNextCh = false;
                            break;
                        }
                        if( SvParserState::Pending == eState )
                            bReadNextChar = bReadNextCharSave;
                        aToken.clear();
                    }
                    else if( '%' == nNextCh )
                    {
                        nRet = HtmlTokenId::UNKNOWNCONTROL_ON;

                        sal_uInt64 nCStreamPos = rInput.Tell();
                        sal_uLong nCLineNr = GetLineNr(), nCLinePos = GetLinePos();

                        bool bDone = false;
                        // Read until closing %>. If not found restart at first >.
                        sal_Unicode nLastTokenChar = !aToken.isEmpty() ? aToken[aToken.getLength() - 1] : 0;
                        OUStringBuffer aTmpBuffer(aToken);
                        while( !bDone && !rInput.eof() && IsParserWorking() )
                        {
                            bDone = '>'==nNextCh && nLastTokenChar == '%';
                            if( !bDone )
                            {
                                aTmpBuffer.appendUtf32(nNextCh);
                                nLastTokenChar = nNextCh;
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
                            nRet = HtmlTokenId::TEXTTOKEN;
                            break;
                        }
                        aToken = aTmpBuffer.makeStringAndClear();
                        if( IsParserWorking() )
                        {
                            sSaveToken = aToken;
                            aToken.clear();
                        }
                    }
                    else
                    {
                        aToken = "<";
                        nRet = HtmlTokenId::TEXTTOKEN;
                        bNextCh = false;
                        break;
                    }
                }

                if( IsParserWorking() )
                {
                    bNextCh = '>' == nNextCh;
                    switch( nRet )
                    {
                    case HtmlTokenId::TEXTAREA_ON:
                        bReadTextArea = true;
                        break;
                    case HtmlTokenId::TEXTAREA_OFF:
                        bReadTextArea = false;
                        break;
                    case HtmlTokenId::SCRIPT_ON:
                        if( !bReadTextArea )
                            bReadScript = true;
                        break;
                    case HtmlTokenId::SCRIPT_OFF:
                        if( !bReadTextArea )
                        {
                            bReadScript = false;
                            // JavaScript might modify the stream,
                            // thus the last character has to be read again.
                            bReadNextChar = true;
                            bNextCh = false;
                        }
                        break;

                    case HtmlTokenId::STYLE_ON:
                        bReadStyle = true;
                        break;
                    case HtmlTokenId::STYLE_OFF:
                        bReadStyle = false;
                        break;
                    default: break;
                    }
                }
            }
            break;

        case sal_Unicode(EOF):
            if( rInput.eof() )
            {
                eState = SvParserState::Accepted;
                nRet = HtmlTokenId(nNextCh);
            }
            else
            {
                // Read normal text.
                goto scan_text;
            }
            break;

        case '\f':
            // form feeds are passed upwards separately
            nRet = HtmlTokenId::LINEFEEDCHAR; // !!! should be FORMFEEDCHAR
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
                nRet = HtmlTokenId::NEWPARA;
                break;
            }
            SAL_FALLTHROUGH;
        case '\t':
            if( bReadPRE )
            {
                nRet = HtmlTokenId::TABCHAR;
                break;
            }
            SAL_FALLTHROUGH;
        case ' ':
            SAL_FALLTHROUGH;
        default:

scan_text:
            // "normal" text to come
            nRet = ScanText();
            bNextCh = 0 == aToken.getLength();

            // the text should be processed
            if( !bNextCh && eState == SvParserState::Pending )
            {
                eState = SvParserState::Working;
                bReadNextChar = true;
            }

            break;
        }

        if( bNextCh && SvParserState::Working == eState )
        {
            nNextCh = GetNextChar();
            if( SvParserState::Pending == eState && nRet != HtmlTokenId::NONE && HtmlTokenId::TEXTTOKEN != nRet )
            {
                bReadNextChar = true;
                eState = SvParserState::Working;
            }
        }

    } while( nRet == HtmlTokenId::NONE && SvParserState::Working == eState );

    if( SvParserState::Pending == eState )
        nRet = HtmlTokenId::INVALID;      // s.th. invalid

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

const HTMLOptions& HTMLParser::GetOptions( HtmlOptionId const *pNoConvertToken )
{
    // If the options for the current token have already been returned,
    // return them once again.
    if (!maOptions.empty())
        return maOptions;

    sal_Int32 nPos = 0;
    while( nPos < aToken.getLength() )
    {
        // A letter? Option beginning here.
        if( rtl::isAsciiAlpha( aToken[nPos] ) )
        {
            HtmlOptionId nToken;
            OUString aValue;
            sal_Int32 nStt = nPos;
            sal_Unicode cChar = 0;

            // Actually only certain characters allowed.
            // Netscape only looks for "=" and white space (c.f.
            // Mozilla: PA_FetchRequestedNameValues in libparse/pa_mdl.c)
            while( nPos < aToken.getLength() && '=' != (cChar=aToken[nPos]) &&
                   HTML_ISPRINTABLE(cChar) && !rtl::isAsciiWhiteSpace(cChar) )
                nPos++;

            OUString sName( aToken.copy( nStt, nPos-nStt ) );

            // PlugIns require original token name. Convert to lower case only for searching.
            nToken = GetHTMLOption( sName.toAsciiLowerCase() ); // Name is ready
            SAL_WARN_IF( nToken==HtmlOptionId::UNKNOWN, "svtools",
                        "GetOption: unknown HTML option '" << sName << "'" );
            bool bStripCRLF = (nToken < HtmlOptionId::SCRIPT_START ||
                               nToken >= HtmlOptionId::SCRIPT_END) &&
                              (!pNoConvertToken || nToken != *pNoConvertToken);

            while( nPos < aToken.getLength() &&
                   ( !HTML_ISPRINTABLE( (cChar=aToken[nPos]) ) ||
                     rtl::isAsciiWhiteSpace(cChar) ) )
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
                    if( ('"'==cChar) || '\''==cChar )
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
                                {
                                    nPos++;
                                    nLen++;
                                }
                                break;
                            case '\\':
                                if( bOldEscape )
                                {
                                    nPos++;
                                    nLen++;
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
                                {
                                    nPos++;
                                    nLen++;
                                }
                                break;
                            default:
                                nPos++;
                                nLen++;
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
                                {
                                    nPos++;
                                    nLen++;
                                }
                                break;

                            case '\t':
                            case '\r':
                            case '\n':
                                bDone = true;
                                break;

                            case '\\':
                                if( bOldEscape )
                                {
                                    nPos++;
                                    nLen++;
                                }
                                else
                                {
                                    aToken = aToken.replaceAt( nPos, 1, "" );
                                    bEscape = true;
                                }
                                break;

                            default:
                                if( HTML_ISPRINTABLE( c ) )
                                {
                                    nPos++;
                                    nLen++;
                                }
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
            maOptions.emplace_back(nToken, sName, aValue);

        }
        else
            // Ignore white space and unexpected characters
            nPos++;
    }

    return maOptions;
}

HtmlTokenId HTMLParser::FilterPRE( HtmlTokenId nToken )
{
    switch( nToken )
    {
    // in Netscape they only have impact in not empty paragraphs
    case HtmlTokenId::PARABREAK_ON:
        nToken = HtmlTokenId::LINEBREAK;
        SAL_FALLTHROUGH;
    case HtmlTokenId::LINEBREAK:
    case HtmlTokenId::NEWPARA:
        nPre_LinePos = 0;
        if( bPre_IgnoreNewPara )
            nToken = HtmlTokenId::NONE;
        break;

    case HtmlTokenId::TABCHAR:
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
            nToken = HtmlTokenId::TEXTTOKEN;
        }
        break;
    // Keep those
    case HtmlTokenId::TEXTTOKEN:
        nPre_LinePos += aToken.getLength();
        break;

    case HtmlTokenId::SELECT_ON:
    case HtmlTokenId::SELECT_OFF:
    case HtmlTokenId::BODY_ON:
    case HtmlTokenId::FORM_ON:
    case HtmlTokenId::FORM_OFF:
    case HtmlTokenId::INPUT:
    case HtmlTokenId::OPTION:
    case HtmlTokenId::TEXTAREA_ON:
    case HtmlTokenId::TEXTAREA_OFF:

    case HtmlTokenId::IMAGE:
    case HtmlTokenId::APPLET_ON:
    case HtmlTokenId::APPLET_OFF:
    case HtmlTokenId::PARAM:
    case HtmlTokenId::EMBED:

    case HtmlTokenId::HEAD1_ON:
    case HtmlTokenId::HEAD1_OFF:
    case HtmlTokenId::HEAD2_ON:
    case HtmlTokenId::HEAD2_OFF:
    case HtmlTokenId::HEAD3_ON:
    case HtmlTokenId::HEAD3_OFF:
    case HtmlTokenId::HEAD4_ON:
    case HtmlTokenId::HEAD4_OFF:
    case HtmlTokenId::HEAD5_ON:
    case HtmlTokenId::HEAD5_OFF:
    case HtmlTokenId::HEAD6_ON:
    case HtmlTokenId::HEAD6_OFF:
    case HtmlTokenId::BLOCKQUOTE_ON:
    case HtmlTokenId::BLOCKQUOTE_OFF:
    case HtmlTokenId::ADDRESS_ON:
    case HtmlTokenId::ADDRESS_OFF:
    case HtmlTokenId::HORZRULE:

    case HtmlTokenId::CENTER_ON:
    case HtmlTokenId::CENTER_OFF:
    case HtmlTokenId::DIVISION_ON:
    case HtmlTokenId::DIVISION_OFF:

    case HtmlTokenId::SCRIPT_ON:
    case HtmlTokenId::SCRIPT_OFF:
    case HtmlTokenId::RAWDATA:

    case HtmlTokenId::TABLE_ON:
    case HtmlTokenId::TABLE_OFF:
    case HtmlTokenId::CAPTION_ON:
    case HtmlTokenId::CAPTION_OFF:
    case HtmlTokenId::COLGROUP_ON:
    case HtmlTokenId::COLGROUP_OFF:
    case HtmlTokenId::COL_ON:
    case HtmlTokenId::COL_OFF:
    case HtmlTokenId::THEAD_ON:
    case HtmlTokenId::THEAD_OFF:
    case HtmlTokenId::TFOOT_ON:
    case HtmlTokenId::TFOOT_OFF:
    case HtmlTokenId::TBODY_ON:
    case HtmlTokenId::TBODY_OFF:
    case HtmlTokenId::TABLEROW_ON:
    case HtmlTokenId::TABLEROW_OFF:
    case HtmlTokenId::TABLEDATA_ON:
    case HtmlTokenId::TABLEDATA_OFF:
    case HtmlTokenId::TABLEHEADER_ON:
    case HtmlTokenId::TABLEHEADER_OFF:

    case HtmlTokenId::ANCHOR_ON:
    case HtmlTokenId::ANCHOR_OFF:
    case HtmlTokenId::BOLD_ON:
    case HtmlTokenId::BOLD_OFF:
    case HtmlTokenId::ITALIC_ON:
    case HtmlTokenId::ITALIC_OFF:
    case HtmlTokenId::STRIKE_ON:
    case HtmlTokenId::STRIKE_OFF:
    case HtmlTokenId::STRIKETHROUGH_ON:
    case HtmlTokenId::STRIKETHROUGH_OFF:
    case HtmlTokenId::UNDERLINE_ON:
    case HtmlTokenId::UNDERLINE_OFF:
    case HtmlTokenId::BASEFONT_ON:
    case HtmlTokenId::BASEFONT_OFF:
    case HtmlTokenId::FONT_ON:
    case HtmlTokenId::FONT_OFF:
    case HtmlTokenId::BLINK_ON:
    case HtmlTokenId::BLINK_OFF:
    case HtmlTokenId::SPAN_ON:
    case HtmlTokenId::SPAN_OFF:
    case HtmlTokenId::SUBSCRIPT_ON:
    case HtmlTokenId::SUBSCRIPT_OFF:
    case HtmlTokenId::SUPERSCRIPT_ON:
    case HtmlTokenId::SUPERSCRIPT_OFF:
    case HtmlTokenId::BIGPRINT_ON:
    case HtmlTokenId::BIGPRINT_OFF:
    case HtmlTokenId::SMALLPRINT_OFF:
    case HtmlTokenId::SMALLPRINT_ON:

    case HtmlTokenId::EMPHASIS_ON:
    case HtmlTokenId::EMPHASIS_OFF:
    case HtmlTokenId::CITIATION_ON:
    case HtmlTokenId::CITIATION_OFF:
    case HtmlTokenId::STRONG_ON:
    case HtmlTokenId::STRONG_OFF:
    case HtmlTokenId::CODE_ON:
    case HtmlTokenId::CODE_OFF:
    case HtmlTokenId::SAMPLE_ON:
    case HtmlTokenId::SAMPLE_OFF:
    case HtmlTokenId::KEYBOARD_ON:
    case HtmlTokenId::KEYBOARD_OFF:
    case HtmlTokenId::VARIABLE_ON:
    case HtmlTokenId::VARIABLE_OFF:
    case HtmlTokenId::DEFINSTANCE_ON:
    case HtmlTokenId::DEFINSTANCE_OFF:
    case HtmlTokenId::SHORTQUOTE_ON:
    case HtmlTokenId::SHORTQUOTE_OFF:
    case HtmlTokenId::LANGUAGE_ON:
    case HtmlTokenId::LANGUAGE_OFF:
    case HtmlTokenId::AUTHOR_ON:
    case HtmlTokenId::AUTHOR_OFF:
    case HtmlTokenId::PERSON_ON:
    case HtmlTokenId::PERSON_OFF:
    case HtmlTokenId::ACRONYM_ON:
    case HtmlTokenId::ACRONYM_OFF:
    case HtmlTokenId::ABBREVIATION_ON:
    case HtmlTokenId::ABBREVIATION_OFF:
    case HtmlTokenId::INSERTEDTEXT_ON:
    case HtmlTokenId::INSERTEDTEXT_OFF:
    case HtmlTokenId::DELETEDTEXT_ON:
    case HtmlTokenId::DELETEDTEXT_OFF:
    case HtmlTokenId::TELETYPE_ON:
    case HtmlTokenId::TELETYPE_OFF:

        break;

    // The remainder is treated as an unknown token.
    default:
        if( nToken != HtmlTokenId::NONE )
        {
            nToken =
                ( ((nToken >= HtmlTokenId::ONOFF_START) && isOffToken(nToken))
                    ? HtmlTokenId::UNKNOWNCONTROL_OFF
                    : HtmlTokenId::UNKNOWNCONTROL_ON );
        }
        break;
    }

    bPre_IgnoreNewPara = false;

    return nToken;
}

HtmlTokenId HTMLParser::FilterXMP( HtmlTokenId nToken )
{
    switch( nToken )
    {
    case HtmlTokenId::NEWPARA:
        if( bPre_IgnoreNewPara )
            nToken = HtmlTokenId::NONE;
        SAL_FALLTHROUGH;
    case HtmlTokenId::TEXTTOKEN:
    case HtmlTokenId::NONBREAKSPACE:
    case HtmlTokenId::SOFTHYPH:
        break;              // kept

    default:
        if( nToken != HtmlTokenId::NONE )
        {
            if( (nToken >= HtmlTokenId::ONOFF_START) && isOffToken(nToken) )
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
            nToken = HtmlTokenId::TEXTTOKEN;
        }
        break;
    }

    bPre_IgnoreNewPara = false;

    return nToken;
}

HtmlTokenId HTMLParser::FilterListing( HtmlTokenId nToken )
{
    switch( nToken )
    {
    case HtmlTokenId::NEWPARA:
        if( bPre_IgnoreNewPara )
            nToken = HtmlTokenId::NONE;
        SAL_FALLTHROUGH;
    case HtmlTokenId::TEXTTOKEN:
    case HtmlTokenId::NONBREAKSPACE:
    case HtmlTokenId::SOFTHYPH:
        break;      // kept

    default:
        if( nToken != HtmlTokenId::NONE )
        {
            nToken =
                ( ((nToken >= HtmlTokenId::ONOFF_START) && isOffToken(nToken))
                    ? HtmlTokenId::UNKNOWNCONTROL_OFF
                    : HtmlTokenId::UNKNOWNCONTROL_ON );
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

enum class HtmlMeta {
    NONE = 0,
    Author,
    Description,
    Keywords,
    Refresh,
    Classification,
    Created,
    ChangedBy,
    Changed,
    Generator,
    SDFootnote,
    SDEndnote,
    ContentType
};

// <META NAME=xxx>
static HTMLOptionEnum<HtmlMeta> const aHTMLMetaNameTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_META_author,        HtmlMeta::Author        },
    { OOO_STRING_SVTOOLS_HTML_META_changed,       HtmlMeta::Changed       },
    { OOO_STRING_SVTOOLS_HTML_META_changedby,     HtmlMeta::ChangedBy     },
    { OOO_STRING_SVTOOLS_HTML_META_classification,HtmlMeta::Classification},
    { OOO_STRING_SVTOOLS_HTML_META_content_type,  HtmlMeta::ContentType   },
    { OOO_STRING_SVTOOLS_HTML_META_created,       HtmlMeta::Created       },
    { OOO_STRING_SVTOOLS_HTML_META_description,   HtmlMeta::Description   },
    { OOO_STRING_SVTOOLS_HTML_META_keywords,      HtmlMeta::Keywords      },
    { OOO_STRING_SVTOOLS_HTML_META_generator,     HtmlMeta::Generator     },
    { OOO_STRING_SVTOOLS_HTML_META_refresh,       HtmlMeta::Refresh       },
    { OOO_STRING_SVTOOLS_HTML_META_sdendnote,     HtmlMeta::SDEndnote     },
    { OOO_STRING_SVTOOLS_HTML_META_sdfootnote,    HtmlMeta::SDFootnote    },
    { nullptr,                                    HtmlMeta(0)             }
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
    HtmlMeta nAction = HtmlMeta::NONE;
    bool bHTTPEquiv = false, bChanged = false;

    for ( size_t i = aOptions.size(); i; )
    {
        const HTMLOption& aOption = aOptions[--i];
        switch ( aOption.GetToken() )
        {
            case HtmlOptionId::NAME:
                aName = aOption.GetString();
                if ( HtmlMeta::NONE==nAction )
                {
                    aOption.GetEnum( nAction, aHTMLMetaNameTable );
                }
                break;
            case HtmlOptionId::HTTPEQUIV:
                aName = aOption.GetString();
                aOption.GetEnum( nAction, aHTMLMetaNameTable );
                bHTTPEquiv = true;
                break;
            case HtmlOptionId::CONTENT:
                aContent = aOption.GetString();
                break;
            case HtmlOptionId::CHARSET:
            {
                OString sValue(OUStringToOString(aOption.GetString(), RTL_TEXTENCODING_ASCII_US));
                o_rEnc = GetExtendedCompatibilityTextEncoding(rtl_getTextEncodingFromMimeCharset(sValue.getStr()));
                break;
            }
            default: break;
        }
    }

    if ( bHTTPEquiv || HtmlMeta::Description != nAction )
    {
        // if it is not a Description, remove CRs and LFs from CONTENT
        aContent = aContent.replaceAll("\r", "").replaceAll("\n", "");
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
        case HtmlMeta::Author:
            if (i_xDocProps.is()) {
                i_xDocProps->setAuthor( aContent );
                bChanged = true;
            }
            break;
        case HtmlMeta::Description:
            if (i_xDocProps.is()) {
                i_xDocProps->setDescription( aContent );
                bChanged = true;
            }
            break;
        case HtmlMeta::Keywords:
            if (i_xDocProps.is()) {
                i_xDocProps->setKeywords(
                    ::comphelper::string::convertCommaSeparated(aContent));
                bChanged = true;
            }
            break;
        case HtmlMeta::Classification:
            if (i_xDocProps.is()) {
                i_xDocProps->setSubject( aContent );
                bChanged = true;
            }
            break;

        case HtmlMeta::ChangedBy:
            if (i_xDocProps.is()) {
                i_xDocProps->setModifiedBy( aContent );
                bChanged = true;
            }
            break;

        case HtmlMeta::Created:
        case HtmlMeta::Changed:
            if (i_xDocProps.is() && !aContent.isEmpty())
            {
                ::util::DateTime uDT;
                bool valid = false;
                if (comphelper::string::getTokenCount(aContent, ';') == 2)
                {
                    Date aDate(aContent.getToken(0, ';').toInt32());
                    tools::Time aTime(aContent.getToken(1, ';').toInt64());
                    DateTime aDateTime(aDate, aTime);
                    uDT = aDateTime.GetUNODateTime();
                    valid = true;
                }
                else if (utl::ISO8601parseDateTime(aContent, uDT))
                    valid = true;

                if (valid)
                {
                    bChanged = true;
                    if (HtmlMeta::Created == nAction)
                        i_xDocProps->setCreationDate(uDT);
                    else
                        i_xDocProps->setModificationDate(uDT);
                }
            }
            break;

        case HtmlMeta::Refresh:
            DBG_ASSERT( !bHTTPEquiv || i_pHTTPHeader, "Lost Reload-URL because of omitted MUST change." );
            break;

        case HtmlMeta::ContentType:
            if ( !aContent.isEmpty() )
            {
                o_rEnc = GetEncodingByMIME( aContent );
            }
            break;

        case HtmlMeta::NONE:
            if ( !bHTTPEquiv )
            {
                if (i_xDocProps.is())
                {
                    uno::Reference<beans::XPropertyContainer> xUDProps
                        = i_xDocProps->getUserDefinedProperties();
                    try {
                        xUDProps->addProperty(aName,
                            beans::PropertyAttribute::REMOVABLE,
                            uno::makeAny(aContent));
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
    HtmlOptionId nContentOption = HtmlOptionId::CONTENT;
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
