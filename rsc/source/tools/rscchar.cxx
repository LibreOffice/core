/*************************************************************************
 *
 *  $RCSfile: rscchar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/tools/rscchar.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.14  2000/09/17 12:51:12  willem.vandorp
    OpenOffice header added.

    Revision 1.13  2000/07/26 17:13:23  willem.vandorp
    Headers/footers replaced

    Revision 1.12  1999/09/20 16:12:22  mm
    \X is possible in the character converter

    Revision 1.11  1999/09/09 13:10:53  mm
    UTF8 corrected

    Revision 1.10  1999/09/07 13:30:06  mm
    UniCode

    Revision 1.9  1998/01/08 13:20:56  MM
    includes angepasst


      Rev 1.8   08 Jan 1998 13:20:56   MM
   includes angepasst

      Rev 1.7   07 Aug 1997 15:47:20   TJ
   include

      Rev 1.6   21 Nov 1995 19:50:52   TLX
   Neuer Link

      Rev 1.5   25 Aug 1993 15:31:38   mm
   Zeichensaetze aus den tools

      Rev 1.4   06 Aug 1993 13:14:56   TH
   Equal-Tablle zwischen 200-209 jetzt richtig

      Rev 1.3   19 Jul 1993 19:21:42   mm
   \ verbessert

      Rev 1.2   19 Jul 1993 16:29:46   mm

      Rev 1.1   20 Aug 1992 18:18:20   mm

**************************************************************************/
/****************** I N C L U D E S **************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

// Solar Definitionen
#include <tools/solar.h>
#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

#include <rtl/textcvt.h>
#include <rtl/textenc.h>

/*************************************************************************
|*
|*    RscChar::MakeChar()
|*
|*    Beschreibung      Der String wird nach C-Konvention umgesetzt
|*    Ersterstellung    MM 20.03.91
|*    Letzte Aenderung  MM 20.03.91
|*
*************************************************************************/
char * RscChar::MakeUTF8( char * pStr, UINT16 nTextEncoding )
{
    sal_Size        nMaxUniCodeBuf = strlen( pStr ) + 1;
    char *          pOrgStr = new char[ nMaxUniCodeBuf ];
    USHORT          nOrgLen = 0;

    if( nMaxUniCodeBuf * 6 > 0xFFFF )
        RscExit( 10 );

    char cOld = '1';
    while( cOld != 0 )
    {
        char c;

        if( *pStr == '\\' )
        {
            ++pStr;
            switch( *pStr )
            {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '?':
                    c = '\?';
                    break;
                case '\'':
                    c = '\'';
                    break;
                case '\"':
                    c = '\"';
                    break;
                default:
                {
                    if( '0' <= *pStr && '7' >= *pStr )
                    {
                        USHORT  nChar = 0;
                        USHORT  i = 0;
                        while( '0' <= *pStr && '7' >= *pStr && i != 3 )
                        {
                            nChar = nChar * 8 + (BYTE)*pStr - (BYTE)'0';
                            ++pStr;
                            i++;
                        }
                        if( nChar > 255 )
                        {
                            RscMem::Free( pOrgStr );

                            // Wert zu gross, oder kein 3 Ziffern
                            return( NULL );
                        }
                        c = (char)nChar;
                        pStr--;
                    }
                    else if( 'x' == *pStr )
                    {
                        USHORT  nChar = 0;
                        USHORT  i = 0;
                        ++pStr;
                        while( isxdigit( *pStr ) && i != 2 )
                        {
                            if( isdigit( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'0';
                            else if( isupper( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'A' +10;
                            else
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'a' +10;
                            ++pStr;
                            i++;
                        }
                        c = (char)nChar;
                        pStr--;
                    }
                    else
                        c = *pStr;
                };
            }
        }
        else
            c = *pStr;
        pOrgStr[ nOrgLen++ ] = c;
        cOld = *pStr;
        pStr++;
    }

    sal_Unicode *   pUniCode = new sal_Unicode[ nMaxUniCodeBuf ];
    rtl_TextToUnicodeConverter hConv = rtl_createTextToUnicodeConverter( nTextEncoding );

    sal_uInt32 nInfo;
    sal_Size   nSrcCvtBytes;
    sal_Size nUniSize = rtl_convertTextToUnicode( hConv, 0,
                                                pOrgStr, nOrgLen,
                                                pUniCode, nMaxUniCodeBuf,
                                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT
                                                | RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                                &nInfo,
                                                &nSrcCvtBytes );

    rtl_destroyTextToUnicodeConverter( hConv );

    hConv = rtl_createUnicodeToTextConverter( RTL_TEXTENCODING_UTF8 );
    // factor fo 6 is the maximum size of an UNICODE character as utf8
    char * pUtf8 = (char *)RscMem::Malloc( (USHORT)(nMaxUniCodeBuf * 6) );
    UINT16  nUtf8Len = 0;

    rtl_convertUnicodeToText( hConv, 0,
                            pUniCode, nUniSize,
                            pUtf8, nMaxUniCodeBuf * 6,
                            RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT
                            | RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT
                            | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                            &nInfo,
                            &nSrcCvtBytes );

    rtl_destroyTextToUnicodeConverter( hConv );

    delete pUniCode;
    delete pOrgStr;
    return pUtf8;
};

/*************************************************************************
|*
|*    RscChar::MakeChar()
|*
|*    Beschreibung      Der String wird nach C-Konvention umgesetzt
|*    Ersterstellung    MM 20.03.91
|*    Letzte Aenderung  MM 20.03.91
|*
*************************************************************************/
char * RscChar::MakeUTF8FromL( char * pStr )
{
    sal_Size        nUniPos = 0;
    sal_Unicode *   pUniCode = new sal_Unicode[ strlen( pStr ) + 1 ];

    char cOld = '1';
    while( cOld != 0 )
    {
        sal_Unicode  c;
        if( *pStr == '\\' )
        {
            ++pStr;
            switch( *pStr )
            {
                case 'a':
                    c = '\a';
                    break;
                case 'b':
                    c = '\b';
                    break;
                case 'f':
                    c = '\f';
                    break;
                case 'n':
                    c = '\n';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'v':
                    c = '\v';
                    break;
                case '\\':
                    c = '\\';
                    break;
                case '?':
                    c = '\?';
                    break;
                case '\'':
                    c = '\'';
                    break;
                case '\"':
                    c = '\"';
                    break;
                default:
                {
                    if( '0' <= *pStr && '7' >= *pStr )
                    {
                        UINT32  nChar = 0;
                        USHORT  i = 0;
                        while( '0' <= *pStr && '7' >= *pStr && i != 6 )
                        {
                            nChar = nChar * 8 + (BYTE)*pStr - (BYTE)'0';
                            ++pStr;
                            i++;
                        }
                        if( nChar > 0xFFFF )
                            // Wert zu gross, oder kein 3 Ziffern
                            return( FALSE );
                        c = (UINT16)nChar;
                        pStr--;
                    }
                    else if( 'x' == *pStr || 'X' == *pStr )
                    {
                        UINT32  nChar = 0;
                        USHORT  i = 0;
                        ++pStr;
                        while( isxdigit( *pStr ) && i != 4 )
                        {
                            if( isdigit( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'0';
                            else if( isupper( *pStr ) )
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'A' +10;
                            else
                                nChar = nChar * 16 + (BYTE)*pStr - (BYTE)'a' +10;
                            ++pStr;
                            i++;
                        }
                        c = (UINT16)nChar;
                        pStr--;
                    }
                    else
                        c = *pStr;
                };
            }
        }
        else
            c = *pStr;

        pUniCode[ nUniPos++ ] = c;
        cOld = *pStr;
        pStr++;
    }

    // factor fo 6 is the maximum size of an UNICODE character as utf8
    sal_Size nMaxUtf8Len = nUniPos * 6;
    if( nUniPos * 6 > 0xFFFF )
        RscExit( 10 );
    char * pUtf8 = (char *)RscMem::Malloc( (USHORT)nMaxUtf8Len );
    rtl_TextToUnicodeConverter hConv = rtl_createUnicodeToTextConverter( RTL_TEXTENCODING_UTF8 );

    sal_uInt32 nInfo;
    sal_Size   nSrcCvtBytes;
    sal_Size nSize = rtl_convertUnicodeToText( hConv, 0,
                                                pUniCode, nUniPos,
                                                pUtf8, nMaxUtf8Len,
                                                RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT
                                                | RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT
                                                | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                                                &nInfo,
                                                &nSrcCvtBytes );

    rtl_destroyUnicodeToTextConverter( hConv );

    delete pUniCode;
    return pUtf8;
};

