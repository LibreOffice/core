/*************************************************************************
 *
 *  $RCSfile: rtfout.cxx,v $
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
 /*************************************************************************
#*
#*    $Workfile:   rtfout.cxx  $
#*
#*    class HTMLOutFuncs
#*
#*    Beschreibung      DOKUNAME.DOC
#*
#*    Ersterstellung    JP 20.02.96
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-18 16:59:05 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/svtools/source/svrtf/rtfout.cxv  $
#*
#*    Copyright (c) 1989 - 2000, STAR DIVISION
#*
#*************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _RTFKEYWD_HXX
#include <rtfkeywd.hxx>
#endif
#ifndef _RTFOUT_HXX
#include <rtfout.hxx>
#endif


#if defined(MAC)
const sal_Char RTFOutFuncs::sNewLine = '\015';
#elif defined(UNX)
const sal_Char RTFOutFuncs::sNewLine = '\012';
#else
const sal_Char __FAR_DATA RTFOutFuncs::sNewLine[] = "\015\012";
#endif


// ER 01.04.99 diese Methode wurde aus sw/source/filter/rtf/rtfatr.cxx
// Writer& OutRTF_SwChar( Writer& rWrt, BYTE c ) kopiert und angepasst
SvStream& RTFOutFuncs::Out_Char( SvStream& rStream, sal_Unicode c,
                            rtl_TextEncoding eDestEnc, BOOL bWriteHelpFile )
{
    const sal_Char* pStr = 0;
    switch( c )
    {
    case 0x1:   // this are control character of our textattributes and
    case 0x2:   // will never be written
                break;

    case 0xA0:          rStream << "\\~";   break;
    case 0xAD:          rStream << "\\-";   break;
    case 0x2011:        rStream << "\\_";   break;
    case '\n':          pStr = sRTF_LINE; break;
    case '\t':          pStr = sRTF_TAB; break;

    default:
        {
            sal_Unicode cCh = c;
            c = (BYTE)ByteString::ConvertFromUnicode( c, eDestEnc, FALSE );
            if( !c )
            {
                // then write as unicode - character
        //      rWrt.Strm() << "\\u";
        //      rWrt.OutULong( eCh ) << " ?";
                rStream << "\\'";
                Out_Hex( rStream, cCh, 2 );
                break;
            }

            if( !bWriteHelpFile )
                switch( c )
                {
#ifdef MAC
                case 0xa5:          pStr = sRTF_BULLET; break;
                case 0xd0:          pStr = sRTF_ENDASH; break;
                case 0xd1:          pStr = sRTF_EMDASH; break;
                case 0xd4:          pStr = sRTF_LQUOTE; break;
                case 0xd5:          pStr = sRTF_RQUOTE; break;
                case 0xd2:          pStr = sRTF_LDBLQUOTE; break;
                case 0xd3:          pStr = sRTF_RDBLQUOTE; break;
#else
                case 149:           pStr = sRTF_BULLET; break;
                case 150:           pStr = sRTF_ENDASH; break;
                case 151:           pStr = sRTF_EMDASH; break;
                case 145:           pStr = sRTF_LQUOTE; break;
                case 146:           pStr = sRTF_RQUOTE; break;
                case 147:           pStr = sRTF_LDBLQUOTE; break;
                case 148:           pStr = sRTF_RDBLQUOTE; break;
#endif
            // MAC
                }
            if( !pStr )
                switch ( c )
                {
                case '\\':
                case '}':
                case '{':           rStream << '\\' << (sal_Char)c; break;

                default:
                    if( c >= ' ' && c <= '~' )
                        rStream << (sal_Char)c;
                    else if( 0xFF < c )
                    {
                        // write as unicode-char
                        ByteString aNumStr( ByteString::CreateFromInt32( c ));
                        rStream << "\\u" << aNumStr.GetBuffer() << " ?";
                    }
                    else
                    {
                        rStream << "\\'";
                        Out_Hex( rStream, c, 2 );
                    }
                    break;
                }
        }
        break;
    }

    if( pStr )
        rStream << pStr << ' ';

    return rStream;
}


SvStream& RTFOutFuncs::Out_String( SvStream& rStream, const String& rStr,
                            rtl_TextEncoding eDestEnc, BOOL bWriteHelpFile )
{
    for( xub_StrLen n = 0; n < rStr.Len(); n++ )
        Out_Char( rStream, rStr.GetChar( n ), eDestEnc, bWriteHelpFile );
    return rStream;
}


SvStream& RTFOutFuncs::Out_Hex( SvStream& rStream, ULONG nHex, BYTE nLen )
{
    sal_Char aNToABuf[] = "0000000000000000";

    DBG_ASSERT( nLen < sizeof(aNToABuf), "zu viele Stellen" );
    if( nLen >= sizeof(aNToABuf) )
        nLen = (sizeof(aNToABuf)-1);

    // Pointer an das Bufferende setzen
    sal_Char* pStr = aNToABuf + (sizeof(aNToABuf)-1);
    for( BYTE n = 0; n < nLen; ++n )
    {
        *(--pStr) = (sal_Char)(nHex & 0xf ) + 48;
        if( *pStr > '9' )
            *pStr += 39;
        nHex >>= 4;
    }
    return rStream << pStr;
}



