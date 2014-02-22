/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <stdio.h>
#include <rtl/tencinfo.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <svtools/rtftoken.h>
#include <svtools/rtfkeywd.hxx>
#include <svtools/parrtf.hxx>
#include <comphelper/string.hxx>

const int MAX_STRING_LEN = 1024;
const int MAX_TOKEN_LEN = 128;

#define RTF_ISDIGIT( c ) comphelper::string::isdigitAscii(c)
#define RTF_ISALPHA( c ) comphelper::string::isalphaAscii(c)

SvRTFParser::SvRTFParser( SvStream& rIn, sal_uInt8 nStackSize )
    : SvParser( rIn, nStackSize ),
    eUNICodeSet( RTL_TEXTENCODING_MS_1252 ),    
    nUCharOverread( 1 )
{
    
    SetSrcEncoding( RTL_TEXTENCODING_MS_1252 );
    bRTF_InTextRead = false;
}

SvRTFParser::~SvRTFParser()
{
}




int SvRTFParser::_GetNextToken()
{
    int nRet = 0;
    do {
        bool bNextCh = true;
        switch( nNextCh )
        {
        case '\\':
            {
                
                switch( nNextCh = GetNextChar() )
                {
                case '{':
                case '}':
                case '\\':
                case '+':       
                case '~':       
                case '-':       
                case '_':       
                case '\'':      
                    nNextCh = '\\';
                    rInput.SeekRel( -1 );
                    ScanText();
                    nRet = RTF_TEXTTOKEN;
                    bNextCh = 0 == nNextCh;
                    break;

                case '*':       
                    nRet = RTF_IGNOREFLAG;
                    break;
                case ':':       
                    nRet = RTF_SUBENTRYINDEX;
                    break;
                case '|':       
                    nRet = RTF_FORMULA;
                    break;

                case 0x0a:
                case 0x0d:
                    nRet = RTF_PAR;
                    break;

                default:
                    if( RTF_ISALPHA( nNextCh ) )
                    {
                        aToken = "\\";
                        {
                            OUStringBuffer aStrBuffer;
                            aStrBuffer.setLength( MAX_TOKEN_LEN );
                            sal_Int32 nStrLen = 0;
                            do {
                                aStrBuffer[nStrLen++] = nNextCh;
                                if( MAX_TOKEN_LEN == nStrLen )
                                {
                                    aToken += aStrBuffer.toString();
                                    nStrLen = 0;
                                }
                                nNextCh = GetNextChar();
                            } while( RTF_ISALPHA( nNextCh ) );
                            if( nStrLen )
                            {
                                aToken += aStrBuffer.makeStringAndClear();
                            }
                        }

                        
                        bool bNegValue = false;
                        if( '-' == nNextCh )
                        {
                            bNegValue = true;
                            nNextCh = GetNextChar();
                        }

                        
                        if( RTF_ISDIGIT( nNextCh ) )
                        {
                            nTokenValue = 0;
                            do {
                                nTokenValue *= 10;
                                nTokenValue += nNextCh - '0';
                                nNextCh = GetNextChar();
                            } while( RTF_ISDIGIT( nNextCh ) );
                            if( bNegValue )
                                nTokenValue = -nTokenValue;
                            bTokenHasValue=true;
                        }
                        else if( bNegValue )        
                        {
                            nNextCh = '-';
                            rInput.SeekRel( -1 );
                        }
                        if( ' ' == nNextCh )        
                            nNextCh = GetNextChar();

                        
                        if( 0 == (nRet = GetRTFToken( aToken )) )
                            
                            nRet = RTF_UNKNOWNCONTROL;

                        
                        bNextCh = false;
                        switch( nRet )
                        {
                        case RTF_UC:
                            if( 0 <= nTokenValue )
                            {
                                nUCharOverread = (sal_uInt8)nTokenValue;
                                
                                aParserStates.top().
                                    nUCharOverread = nUCharOverread;
                            }
                            aToken = ""; 
                            
                            nRet = 0;
                            break;

                        case RTF_UPR:
                            if (!_inSkipGroup) {
                            
                            
                            while( '{' != _GetNextToken() )
                                ;
                            SkipGroup();
                            _GetNextToken();  
                            nRet = 0;
                            }
                            break;

                        case RTF_U:
                            if( !bRTF_InTextRead )
                            {
                                nRet = RTF_TEXTTOKEN;
                                aToken = OUString( (sal_Unicode)nTokenValue );

                                
                                
                                for( sal_uInt8 m = 0; m < nUCharOverread; ++m )
                                {
                                    sal_Unicode cAnsi = nNextCh;
                                    while( 0xD == cAnsi )
                                        cAnsi = GetNextChar();
                                    while( 0xA == cAnsi )
                                        cAnsi = GetNextChar();

                                    if( '\\' == cAnsi &&
                                        '\'' == ( cAnsi = GetNextChar() ))
                                        
                                        cAnsi = GetHexValue();
                                    nNextCh = GetNextChar();
                                }
                                ScanText();
                                bNextCh = 0 == nNextCh;
                            }
                            break;
                        }
                    }
                    else if( SVPAR_PENDING != eState )
                    {
                        
                        
                        bNextCh = false;
                    }
                    break;
                }
            }
            break;

        case sal_Unicode(EOF):
            eState = SVPAR_ACCEPTED;
            nRet = nNextCh;
            break;

        case '{':
            {
                if( 0 <= nOpenBrakets )
                {
                    RtfParserState_Impl aState( nUCharOverread, GetSrcEncoding() );
                    aParserStates.push( aState );
                }
                ++nOpenBrakets;
                DBG_ASSERT(
                    static_cast<size_t>(nOpenBrakets) == aParserStates.size(),
                    "ParserStateStack unequal to bracket count" );
                nRet = nNextCh;
            }
            break;

        case '}':
            --nOpenBrakets;
            if( 0 <= nOpenBrakets )
            {
                aParserStates.pop();
                if( !aParserStates.empty() )
                {
                    const RtfParserState_Impl& rRPS =
                            aParserStates.top();
                    nUCharOverread = rRPS.nUCharOverread;
                    SetSrcEncoding( rRPS.eCodeSet );
                }
                else
                {
                    nUCharOverread = 1;
                    SetSrcEncoding( GetCodeSet() );
                }
            }
            DBG_ASSERT(
                static_cast<size_t>(nOpenBrakets) == aParserStates.size(),
                "ParserStateStack unequal to bracket count" );
            nRet = nNextCh;
            break;

        case 0x0d:
        case 0x0a:
            break;

        default:
            
            ScanText();
            nRet = RTF_TEXTTOKEN;
            bNextCh = 0 == nNextCh;
            break;
        }

        if( bNextCh )
            nNextCh = GetNextChar();

    } while( !nRet && SVPAR_WORKING == eState );
    return nRet;
}


sal_Unicode SvRTFParser::GetHexValue()
{
    
    int n;
    sal_Unicode nHexVal = 0;

    for( n = 0; n < 2; ++n )
    {
        nHexVal *= 16;
        nNextCh = GetNextChar();
        if( nNextCh >= '0' && nNextCh <= '9' )
            nHexVal += (nNextCh - 48);
        else if( nNextCh >= 'a' && nNextCh <= 'f' )
            nHexVal += (nNextCh - 87);
        else if( nNextCh >= 'A' && nNextCh <= 'F' )
            nHexVal += (nNextCh - 55);
    }
    return nHexVal;
}

void SvRTFParser::ScanText( const sal_Unicode cBreak )
{
    OUStringBuffer aStrBuffer;
    bool bContinue = true;
    while( bContinue && IsParserWorking() && aStrBuffer.getLength() < MAX_STRING_LEN)
    {
        bool bNextCh = true;
        switch( nNextCh )
        {
        case '\\':
            {
                switch (nNextCh = GetNextChar())
                {
                case '\'':
                    {

                        OStringBuffer aByteString;
                        while (true)
                        {
                            char c = (char)GetHexValue();
                            /*
                             * Note: \'00 is a valid internal character in  a
                             * string in RTF. OStringBuffer supports
                             * appending nulls fine
                             */
                            aByteString.append(c);

                            bool bBreak = false;
                            sal_Char nSlash = '\\';
                            while (!bBreak)
                            {
                                wchar_t __next=GetNextChar();
                                if (__next>0xFF) 
                                {
                                    if (!aByteString.isEmpty())
                                        aStrBuffer.append( OStringToOUString(aByteString.makeStringAndClear(), GetSrcEncoding()) );
                                    aStrBuffer.append((sal_Unicode)__next);

                                    continue;
                                }
                                nSlash = (sal_Char)__next;
                                while (nSlash == 0xD || nSlash == 0xA)
                                    nSlash = (sal_Char)GetNextChar();

                                switch (nSlash)
                                {
                                    case '{':
                                    case '}':
                                    case '\\':
                                        bBreak = true;
                                        break;
                                    default:
                                        aByteString.append(nSlash);
                                        break;
                                }
                            }

                            nNextCh = GetNextChar();

                            if (nSlash != '\\' || nNextCh != '\'')
                            {
                                rInput.SeekRel(-1);
                                nNextCh = nSlash;
                                break;
                            }
                        }

                        bNextCh = false;

                        if (!aByteString.isEmpty())
                            aStrBuffer.append( OStringToOUString(aByteString.makeStringAndClear(), GetSrcEncoding()) );
                    }
                    break;
                case '\\':
                case '}':
                case '{':
                case '+':       
                    aStrBuffer.append(nNextCh);
                    break;
                case '~':       
                    aStrBuffer.append(static_cast< sal_Unicode >(0xA0));
                    break;
                case '-':       
                    aStrBuffer.append(static_cast< sal_Unicode >(0xAD));
                    break;
                case '_':       
                    aStrBuffer.append(static_cast< sal_Unicode >(0x2011));
                    break;

                case 'u':
                    
                    {
                        nNextCh = GetNextChar();
                        rInput.SeekRel( -2 );

                        if( '-' == nNextCh || RTF_ISDIGIT( nNextCh ) )
                        {
                            bRTF_InTextRead = true;

                            OUString sSave( aToken );
                            nNextCh = '\\';
                            #ifdef DBG_UTIL
                            int nToken =
                            #endif
                                _GetNextToken();
                            DBG_ASSERT( RTF_U == nToken, "doch kein UNI-Code Zeichen" );
                            
                            aStrBuffer.append(static_cast< sal_Unicode >(nTokenValue));

                            
                            
                            for( sal_uInt8 m = 0; m < nUCharOverread; ++m )
                            {
                                sal_Unicode cAnsi = nNextCh;
                                while( 0xD == cAnsi )
                                    cAnsi = GetNextChar();
                                while( 0xA == cAnsi )
                                    cAnsi = GetNextChar();

                                if( '\\' == cAnsi &&
                                    '\'' == ( cAnsi = GetNextChar() ))
                                    
                                    cAnsi = GetHexValue();
                                nNextCh = GetNextChar();
                            }
                            bNextCh = false;
                            aToken = sSave;
                            bRTF_InTextRead = false;
                        }
                        else
                        {
                            nNextCh = '\\';
                            bContinue = false;        
                        }
                    }
                    break;

                default:
                    rInput.SeekRel( -1 );
                    nNextCh = '\\';
                    bContinue = false;        
                    break;
                }
            }
            break;

        case sal_Unicode(EOF):
                eState = SVPAR_ERROR;
                
        case '{':
        case '}':
            bContinue = false;
            break;

        case 0x0a:
        case 0x0d:
            break;

        default:
            if( nNextCh == cBreak || aStrBuffer.getLength() >= MAX_STRING_LEN)
                bContinue = false;
            else
            {
                do {
                    
                    aStrBuffer.append(nNextCh);

                    if (sal_Unicode(EOF) == (nNextCh = GetNextChar()))
                    {
                        if (!aStrBuffer.isEmpty())
                            aToken += aStrBuffer.toString();
                        return;
                    }
                } while
                (
                    (RTF_ISALPHA(nNextCh) || RTF_ISDIGIT(nNextCh)) &&
                    (aStrBuffer.getLength() < MAX_STRING_LEN)
                );
                bNextCh = false;
            }
        }

        if( bContinue && bNextCh )
            nNextCh = GetNextChar();
    }

    if (!aStrBuffer.isEmpty())
        aToken += aStrBuffer.makeStringAndClear();
}


short SvRTFParser::_inSkipGroup=0;

void SvRTFParser::SkipGroup()
{
short nBrackets=1;
if (_inSkipGroup>0)
    return;
_inSkipGroup++;

    do
    {
        switch (nNextCh)
        {
            case '{':
                ++nBrackets;
                break;
            case '}':
                if (!--nBrackets) {
                    _inSkipGroup--;
                    return;
                }
                break;
        }
        int nToken = _GetNextToken();
        if (nToken == RTF_BIN)
        {
            rInput.SeekRel(-1);
            rInput.SeekRel(nTokenValue);
            nNextCh = GetNextChar();
        }
        while (nNextCh==0xa || nNextCh==0xd)
        {
            nNextCh = GetNextChar();
        }
    } while (sal_Unicode(EOF) != nNextCh && IsParserWorking());

    if( SVPAR_PENDING != eState && '}' != nNextCh )
        eState = SVPAR_ERROR;
    _inSkipGroup--;
}

void SvRTFParser::ReadUnknownData() { SkipGroup(); }
void SvRTFParser::ReadBitmapData()  { SkipGroup(); }
void SvRTFParser::ReadOLEData()     { SkipGroup(); }


SvParserState SvRTFParser::CallParser()
{
    sal_Char cFirstCh;
    nNextChPos = rInput.Tell();
    rInput.ReadChar( cFirstCh ); nNextCh = cFirstCh;
    eState = SVPAR_WORKING;
    nOpenBrakets = 0;
    SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1252 );
    eUNICodeSet = RTL_TEXTENCODING_MS_1252;     

    
    if( '{' == GetNextToken() && RTF_RTF == GetNextToken() )
    {
        AddRef();
        Continue( 0 );
        if( SVPAR_PENDING != eState )
            ReleaseRef();       
    }
    else
        eState = SVPAR_ERROR;

    return eState;
}

void SvRTFParser::Continue( int nToken )
{



    if( !nToken )
        nToken = GetNextToken();

    while( IsParserWorking() )
    {
        SaveState( nToken );
        switch( nToken )
        {
        case '}':
            if( nOpenBrakets )
                goto NEXTTOKEN;
            eState = SVPAR_ACCEPTED;
            break;

        case '{':
            
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;      
                }
            }
            goto NEXTTOKEN;

        case RTF_UNKNOWNCONTROL:
            break;      
        case RTF_NEXTTYPE:
        case RTF_ANSITYPE:
            SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1252 );
            break;
        case RTF_MACTYPE:
            SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_APPLE_ROMAN );
            break;
        case RTF_PCTYPE:
            SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_437 );
            break;
        case RTF_PCATYPE:
            SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_850 );
            break;
        case RTF_ANSICPG:
            eCodeSet = rtl_getTextEncodingFromWindowsCodePage(nTokenValue);
            SetSrcEncoding(eCodeSet);
            break;
        default:
NEXTTOKEN:
            NextToken( nToken );
            break;
        }
        if( IsParserWorking() )
            SaveState( 0 );         
                                    
        nToken = GetNextToken();
    }
    if( SVPAR_ACCEPTED == eState && 0 < nOpenBrakets )
        eState = SVPAR_ERROR;
}

void SvRTFParser::SetEncoding( rtl_TextEncoding eEnc )
{
    if (eEnc == RTL_TEXTENCODING_DONTKNOW)
        eEnc = GetCodeSet();

    if (!aParserStates.empty())
        aParserStates.top().eCodeSet = eEnc;
    SetSrcEncoding(eEnc);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
