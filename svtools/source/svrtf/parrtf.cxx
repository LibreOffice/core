/*************************************************************************
 *
 *  $RCSfile: parrtf.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:39:40 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#include <stdio.h>                      // for EOF

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include "rtftoken.h"
#include "rtfkeywd.hxx"
#include "parrtf.hxx"

const int MAX_STRING_LEN = 1024;
const int MAX_TOKEN_LEN = 128;

#define RTF_ISDIGIT( c ) (c >= '0' && c <= '9')
#define RTF_ISALPHA( c ) ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )

SV_IMPL_VARARR( RtfParserStates_Impl, RtfParserState_Impl )

SvRTFParser::SvRTFParser( SvStream& rIn, BYTE nStackSize )
    : SvParser( rIn, nStackSize ),
    eUNICodeSet( RTL_TEXTENCODING_MS_1252 ),    // default ist ANSI-CodeSet
    nUCharOverread( 1 )
{
    // default ist ANSI-CodeSet
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
        int bNextCh = true;
        switch( nNextCh )
        {
        case '\\':
            {
                // Steuerzeichen
                switch( nNextCh = GetNextChar() )
                {
                case '{':
                case '}':
                case '\\':
                case '+':       // habe ich in einem RTF-File gefunden
                case '~':       // nonbreaking space
                case '-':       // optional hyphen
                case '_':       // nonbreaking hyphen
                case '\'':      // HexValue
                    nNextCh = '\\';
                    rInput.SeekRel( -1 );
                    ScanText();
                    nRet = RTF_TEXTTOKEN;
                    bNextCh = 0 == nNextCh;
                    break;

                case '*':       // ignoreflag
                    nRet = RTF_IGNOREFLAG;
                    break;
                case ':':       // subentry in an index entry
                    nRet = RTF_SUBENTRYINDEX;
                    break;
                case '|':       // formula-charakter
                    nRet = RTF_FORMULA;
                    break;

                case 0x0a:
                case 0x0d:
                    nRet = RTF_PAR;
                    break;

                default:
                    if( RTF_ISALPHA( nNextCh ) )
                    {
                        aToken = '\\';
                        {
                            String aStrBuffer;
                            sal_Unicode* pStr = aStrBuffer.AllocBuffer(
                                                            MAX_TOKEN_LEN );
                            int nStrLen = 0;
                            do {
                                *(pStr + nStrLen++) = nNextCh;
                                if( MAX_TOKEN_LEN == nStrLen )
                                {
                                    aToken += aStrBuffer;
                                    aToken.GetBufferAccess();  // make unique string!
                                    nStrLen = 0;
                                }
                                nNextCh = GetNextChar();
                            } while( RTF_ISALPHA( nNextCh ) );
                            if( nStrLen )
                            {
                                aStrBuffer.ReleaseBufferAccess( nStrLen );
                                aToken += aStrBuffer;
                            }
                        }

                        // Minus fuer numerischen Parameter
                        int bNegValue = false;
                        if( '-' == nNextCh )
                        {
                            bNegValue = true;
                            nNextCh = GetNextChar();
                        }

                        // evt. Numerischer Parameter
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
                        }
                        else if( bNegValue )        // das Minus wieder zurueck
                        {
                            nNextCh = '-';
                            rInput.SeekRel( -1 );
                        }
                        if( ' ' == nNextCh )        // Blank gehoert zum Token!
                            nNextCh = GetNextChar();

                        // suche das Token in der Tabelle:
                        if( 0 == (nRet = GetRTFToken( aToken )) )
                            // Unknown Control
                            nRet = RTF_UNKNOWNCONTROL;

                        // bug 76812 - unicode token handled as normal text
                        bNextCh = false;
                        switch( nRet )
                        {
                        case RTF_UC:
                            if( 0 <= nTokenValue )
                            {
                                nUCharOverread = (BYTE)nTokenValue;
#if 1
                                //cmc: other ifdef breaks #i3584
                                aParserStates[ aParserStates.Count()-1].
                                    nUCharOverread = nUCharOverread;
#else
                                if( !nUCharOverread )
                                    nUCharOverread = aParserStates[
                                        aParserStates.Count()-1].nUCharOverread;
                                else
                                    aParserStates[ aParserStates.Count()-1].
                                        nUCharOverread = nUCharOverread;
#endif
                            }
                            // read next token
                            nRet = 0;
                            break;

                        case RTF_UPR:
                            // UPR - overread the group with the ansi
                            //       informations
                            while( '{' != _GetNextToken() )
                                ;
                            SkipGroup();
                            _GetNextToken();  // overread the last bracket
                            nRet = 0;
                            break;

                        case RTF_U:
                            if( !bRTF_InTextRead )
                            {
                                nRet = RTF_TEXTTOKEN;
                                aToken = (sal_Unicode)nTokenValue;

                                // overread the next n "RTF" characters. This
                                // can be also \{, \}, \'88
                                for( BYTE m = 0; m < nUCharOverread; ++m )
                                {
                                    sal_Unicode cAnsi = nNextCh;
                                    while( 0xD == cAnsi )
                                        cAnsi = GetNextChar();
                                    while( 0xA == cAnsi )
                                        cAnsi = GetNextChar();

                                    if( '\\' == cAnsi &&
                                        '\'' == ( cAnsi = GetNextChar() ))
                                        // HexValue ueberlesen
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
                        // Bug 34631 - "\ " ueberlesen - Blank als Zeichen
                        // eState = SVPAR_ERROR;
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
                    aParserStates.Insert( aState, nOpenBrakets );
                }
                ++nOpenBrakets;
                DBG_ASSERT( nOpenBrakets == aParserStates.Count(),
                            "ParserStateStack unequal to bracket count" );
                nRet = nNextCh;
            }
            break;

        case '}':
            --nOpenBrakets;
            if( 0 <= nOpenBrakets )
            {
                aParserStates.Remove( nOpenBrakets );
                if( aParserStates.Count() )
                {
                    const RtfParserState_Impl& rRPS =
                            aParserStates[ aParserStates.Count() - 1 ];
                    nUCharOverread = rRPS.nUCharOverread;
                    SetSrcEncoding( rRPS.eCodeSet );
                }
                else
                {
                    nUCharOverread = 1;
                    SetSrcEncoding( GetCodeSet() );
                }
            }
            DBG_ASSERT( nOpenBrakets == aParserStates.Count(),
                        "ParserStateStack unequal to bracket count" );
            nRet = nNextCh;
            break;

        case 0x0d:
        case 0x0a:
            break;

        default:
            // es folgt normaler Text
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
    // Hex-Wert sammeln
    register int n;
    register sal_Unicode nHexVal = 0;

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
    String aStrBuffer;
    int bWeiter = true;
    while( bWeiter && IsParserWorking() )
    {
        int bNextCh = true;
        switch( nNextCh )
        {
        case '\\':
            {
                switch (nNextCh = GetNextChar())
                {
                case '\'':
                    {
                        ByteString aByteString;
                        while (1)
                        {
                            aByteString.Append((char)GetHexValue());

                            bool bBreak = false;
                            sal_Char nSlash;
                            while (!bBreak)
                            {
                                nSlash = (sal_Char)GetNextChar();
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
                                        aByteString.Append(nSlash);
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

                        if (aByteString.Len())
                            aStrBuffer.Append(String(aByteString, GetSrcEncoding()));
                    }
                    break;
                case '\\':
                case '}':
                case '{':
                case '+':       // habe ich in einem RTF-File gefunden
                    aStrBuffer.Append(nNextCh);
                    break;
                case '~':       // nonbreaking space
                    aStrBuffer.Append(0xA0);
                    break;
                case '-':       // optional hyphen
                    aStrBuffer.Append(0xAD);
                    break;
                case '_':       // nonbreaking hyphen
                    aStrBuffer.Append(0x2011);
                    break;

                case 'u':
                    // UNI-Code Zeichen lesen
                    {
                        nNextCh = GetNextChar();
                        rInput.SeekRel( -2 );

                        if( '-' == nNextCh || RTF_ISDIGIT( nNextCh ) )
                        {
                            bRTF_InTextRead = true;

                            String sSave( aToken );
                            nNextCh = '\\';
                            int nToken = _GetNextToken();
                            DBG_ASSERT( RTF_U == nToken, "doch kein UNI-Code Zeichen" );
                            // dont convert symbol chars
                            aStrBuffer.Append(nTokenValue);

                            // overread the next n "RTF" characters. This
                            // can be also \{, \}, \'88
                            for( BYTE m = 0; m < nUCharOverread; ++m )
                            {
                                sal_Unicode cAnsi = nNextCh;
                                while( 0xD == cAnsi )
                                    cAnsi = GetNextChar();
                                while( 0xA == cAnsi )
                                    cAnsi = GetNextChar();

                                if( '\\' == cAnsi &&
                                    '\'' == ( cAnsi = GetNextChar() ))
                                    // HexValue ueberlesen
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
                            bWeiter = false;        // Abbrechen, String zusammen
                        }
                    }
                    break;

                default:
                    rInput.SeekRel( -1 );
                    nNextCh = '\\';
                    bWeiter = false;        // Abbrechen, String zusammen
                    break;
                }
            }
            break;

        case sal_Unicode(EOF):
                eState = SVPAR_ERROR;
                // weiter
        case '{':
        case '}':
            bWeiter = false;
            break;

        case 0x0a:
        case 0x0d:
            break;

        default:
            if( nNextCh == cBreak || aToken.Len() >= MAX_STRING_LEN)
                bWeiter = false;
            else
            {
                do {
                    // alle anderen Zeichen kommen in den Text
                    aStrBuffer.Append(nNextCh);

                    if (sal_Unicode(EOF) == (nNextCh = GetNextChar()))
                    {
                        if (aStrBuffer.Len())
                            aToken += aStrBuffer;
                        return;
                    }
                } while( RTF_ISALPHA( nNextCh ) || RTF_ISDIGIT( nNextCh ) );
                bNextCh = false;
            }
        }

        if( bWeiter && bNextCh )
            nNextCh = GetNextChar();
    }

    if (aStrBuffer.Len())
        aToken += aStrBuffer;
}


void SvRTFParser::SkipGroup()
{
    short nBrackets = 1;
    sal_Unicode cPrev = 0;
    do {
        switch( nNextCh )
        {
        case '{':
            if( '\\' != cPrev )
                ++nBrackets;
            break;

        case '}':
            if( '\\' != cPrev && !--nBrackets )
                return;
            break;

        case '\\':
            if( '\\' == cPrev )
                nNextCh = 0;
            break;
        }
        cPrev = nNextCh;
        nNextCh = GetNextChar();
    } while( sal_Unicode(EOF) != nNextCh && IsParserWorking() );

    if( SVPAR_PENDING != eState && '}' != nNextCh )
        eState = SVPAR_ERROR;
}

void SvRTFParser::ReadUnknownData() { SkipGroup(); }
void SvRTFParser::ReadBitmapData()  { SkipGroup(); }
void SvRTFParser::ReadOLEData()     { SkipGroup(); }


SvParserState SvRTFParser::CallParser()
{
    sal_Char cFirstCh;
    nNextChPos = rInput.Tell();
    rInput >> cFirstCh; nNextCh = cFirstCh;
    eState = SVPAR_WORKING;
    nOpenBrakets = 0;
    SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1252 );
    eUNICodeSet = RTL_TEXTENCODING_MS_1252;     // default ist ANSI-CodeSet

    // die 1. beiden Token muessen '{' und \\rtf sein !!
    if( '{' == GetNextToken() && RTF_RTF == GetNextToken() )
    {
        AddRef();
        Continue( 0 );
        if( SVPAR_PENDING != eState )
            ReleaseRef();       // dann brauchen wir den Parser nicht mehr!
    }
    else
        eState = SVPAR_ERROR;

    return eState;
}

void SvRTFParser::Continue( int nToken )
{
//  DBG_ASSERT( SVPAR_CS_DONTKNOW == GetCharSet(),
//              "Zeichensatz wurde geaendert." );

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
            // eine unbekannte Gruppe ?
            {
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nToken = SkipToken( -1 );
                else if( RTF_UNKNOWNCONTROL != GetNextToken() )
                    nToken = SkipToken( -2 );
                else
                {
                    // gleich herausfiltern
                    ReadUnknownData();
                    nToken = GetNextToken();
                    if( '}' != nToken )
                        eState = SVPAR_ERROR;
                    break;      // auf zum naechsten Token!!
                }
            }
            goto NEXTTOKEN;

        case RTF_UNKNOWNCONTROL:
            break;      // unbekannte Token ueberspringen

        case RTF_NEXTTYPE:
        case RTF_ANSITYPE:      SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1252 );      break;
        case RTF_MACTYPE:       SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_APPLE_ROMAN );      break;
        case RTF_PCTYPE:        SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_437 );  break;
        case RTF_PCATYPE:       SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_850 );  break;

        case RTF_ANSICPG:
                switch( nTokenValue )
                {
                case 1250:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1250 );  break;
                case 1251:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1251 );  break;
                case 1253:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1253 );  break;
                case 1254:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1254 );  break;
                case 1255:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1255 );  break;
                case 1256:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1256 );  break;
                case 1257:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1257 );  break;
                case 1258:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_1258 );  break;
                case  950:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_950 );   break;
                case  932:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_932 );   break;
                case  936:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_MS_936 );   break;
                case  437:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_437 );  break;
                case  850:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_850 );  break;
                case  852:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_852 );  break;
                case  862:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_862 );  break;
                case  863:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_863 );  break;
                case  864:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_864 );  break;
                case  865:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_865 );  break;
                case  866:  SetSrcEncoding( eCodeSet = RTL_TEXTENCODING_IBM_866 );  break;
                }
                break;


        default:
NEXTTOKEN:
            NextToken( nToken );
            break;
        }
        if( IsParserWorking() )
            SaveState( 0 );         // bis hierhin abgearbeitet,
                                    // weiter mit neuem Token!
        nToken = GetNextToken();
    }
    if( SVPAR_ACCEPTED == eState && 0 < nOpenBrakets )
        eState = SVPAR_ERROR;
}

void SvRTFParser::SetEncoding( rtl_TextEncoding eEnc )
{
    switch( eEnc )
    {
    case RTL_TEXTENCODING_DONTKNOW:
    case RTL_TEXTENCODING_SYMBOL:
        eEnc = GetCodeSet();
        break;
    }
    if( aParserStates.Count() )
        aParserStates[ aParserStates.Count() - 1 ].eCodeSet = eEnc;
    SetSrcEncoding( eEnc );
}

#ifdef USED
void SvRTFParser::SaveState( int nToken )
{
    SvParser::SaveState( nToken );
}

void SvRTFParser::RestoreState()
{
    SvParser::RestoreState();
}
#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
