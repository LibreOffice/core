/*************************************************************************
 *
 *  $RCSfile: textconv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#pragma hdrstop

#ifndef ENABLEUNICODE

#define _TEXTCONV_CXX
#include <textconv.hxx>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif

#include <rtl/tencinfo.h>
#include <rtl/textenc.h>

SV_IMPL_VARARR( NonConvChars, AttribRange );

TextConverter::TextConverter()
{
    eSourceCharSet = CHARSET_DONTKNOW;
    eDestCharSet = CHARSET_DONTKNOW;
}

TextConverter::TextConverter( CharSet eSource, CharSet eDest )
{
    eSourceCharSet = eSource;
    eDestCharSet = eDest;
}

void TextConverter::Convert( String& rText, BOOL bSmartReplace )
{
    aNotConvertedChars.Remove( 0, aNotConvertedChars.Count() );

    if( rText.Len() )
    {
        rtl_TextEncodingInfo aTextEncInfo1;
        aTextEncInfo1.StructSize = sizeof( aTextEncInfo1 );
        rtl_getTextEncodingInfo( eSourceCharSet, &aTextEncInfo1 );

        rtl_TextEncodingInfo aTextEncInfo2;
        aTextEncInfo2.StructSize = sizeof( aTextEncInfo2 );
        rtl_getTextEncodingInfo( eDestCharSet, &aTextEncInfo2 );

        if ( (aTextEncInfo1.MaximumCharSize == 1) &&
             (aTextEncInfo2.MaximumCharSize == 1) )
        {
            // Erstmal den gesamten String konvertieren und dann ggf. die nicht
            // konvertierbaren Zeichen zurueckkopieren.
            // Das kann auch nicht teurer sein als staendig neue kleine
            // Strings zu erzeugen und aneinander zu haengen.
            String aNewText( rText );
            aNewText.Convert( eSourceCharSet, eDestCharSet, FALSE );

            // bReplace = FALSE => nicht konvertiebare Zeichen sind jetzt ZERO

            // Bereich fuer nicht konvertierbare Zeichen
            USHORT nNotConvertedFrom = INVPOS;
            USHORT nNotConvertedTo = 0;

            USHORT nCurNonConvblRange = 0;
            USHORT nNonConvblStartOff = INVPOS;
            USHORT nNonConvblEndOff = 0;

            // 1. nicht konvertierbarer Bereich:
            GetNonConvblRange( nCurNonConvblRange++, nNonConvblStartOff, nNonConvblEndOff );

            USHORT nLen = rText.Len();
            char* pNewText = aNewText.GetCharStr();
            for( USHORT nChar = 0; nChar < nLen; nChar++ )
            {
                // ggf. naechster nicht konvertierbarer Bereich:
                if( ( nNonConvblStartOff != 0xFFFF ) && ( nNonConvblEndOff <= nChar ) )
                    GetNonConvblRange( nCurNonConvblRange++, nNonConvblStartOff, nNonConvblEndOff );

                // Testen, ob man sich im verbotenen Bereich befindet:
                if( ( nNonConvblStartOff == INVPOS ) || ( nChar < nNonConvblStartOff ) || ( nChar >= nNonConvblEndOff ) )
                {
                    if( !pNewText[nChar] && bSmartReplace )
                    {
                        // Gottseidank haben alle OS/2-Zeichensaetze bis auf 865
                        // die Zeichen ® und ¯ am selben Fleck. Da hier nur
                        // die nichtkonvertierbaren ZS durchschlagen, ist die
                        // Abfrage sehr einfach zu halten. ANSI und Mac werden
                        // von String::Convert() bereits korrekt konvertiert.
                        // CHARSET_UNIXANSI gibt es (noch) nicht
                        switch( eSourceCharSet )
                        {
                            case CHARSET_ANSI:
                            {
                                switch( (unsigned char)rText.GetChar( nChar ) )
                                {
                                    case 132: pNewText[nChar] = (char)(unsigned char)175; break;
                                    case 147: pNewText[nChar] = (char)(unsigned char)174; break;
                                }
                            }
                            break;
                            case CHARSET_MAC:
                            {
                                switch( (unsigned char)rText.GetChar( nChar ) )
                                {
                                    case 210:
                                    case 227: pNewText[nChar] = (char)(unsigned char)175; break;
                                    case 211: pNewText[nChar] = (char)(unsigned char)174; break;
                                }
                            }
                            break;
                        }

                        // Der Zeichensatz 865 hat keine Matchpaare fuer Anf.Zeichen
                        if( ( eDestCharSet == CHARSET_IBMPC_865 ) && ( ( (unsigned char)pNewText[nChar] == 174 ) || ( (unsigned char)pNewText[nChar] == 175 ) ) )
                            pNewText[nChar] = '"';
                    }

                    if( pNewText[nChar] )
                    {
                        // Zeichen konnte (wieder) konvertiert werden.

                        // Range der Zeichen, die nicht konvertiert werden konnten:
                        if( nNotConvertedFrom != INVPOS )
                        {
                            aNotConvertedChars.Insert( AttribRange( nNotConvertedFrom, nNotConvertedTo ), aNotConvertedChars.Count() );
                            nNotConvertedFrom = INVPOS;
                        }
                    }
                    else
                    {
                        // Zeichen ist nicht konvertierbar: merken und restaurieren
                        pNewText[nChar] = rText.GetChar( nChar );
                        if( nNotConvertedFrom == INVPOS )
                            nNotConvertedFrom = nChar;
                        nNotConvertedTo = nChar;
                    }
                }
                else
                {
                    pNewText[nChar] = rText.GetChar( nChar );
                    // Zeichen die nicht konvertiert werden duerfen, sollen nicht
                    // als NotConverted auftauchen.
                    if( nNotConvertedFrom != INVPOS )
                    {
                        aNotConvertedChars.Insert( AttribRange( nNotConvertedFrom, nNotConvertedTo ), aNotConvertedChars.Count() );
                        nNotConvertedFrom = INVPOS;
                    }
                }
            }
            // Eventuellen uebriggelassenen nicht konvertierbaren Bereich:
            if( nNotConvertedFrom != INVPOS )
                aNotConvertedChars.Insert( AttribRange( nNotConvertedFrom, nNotConvertedTo ), aNotConvertedChars.Count() );

        #ifdef DBG_UTIL
            for ( USHORT n = aNewText.Len(); n; )
                DBG_ASSERT( aNewText.GetChar( --n ), "TextConverter: Char not converted - error in String" );
        #endif

            rText = aNewText;
        }
        else
        {
            // NonConvertableChars not supported here!
            rText.Convert( eSourceCharSet, eDestCharSet, TRUE );
        }
    }
}

char TextConverter::GetReplaceChar( OutputDevice* pDev, sal_Unicode cUnicode, Font& rFont )
{
    char cReplace = 0;

    if ( pDev )
    {
        pDev->Push( PUSH_FONT );
        pDev->SetFont( rFont );
        FontMetric aNewFont = pDev->GetFontMetric();
        cReplace = String::ConvertFromUnicode( cUnicode, aNewFont.GetCharSet(), FALSE );
        pDev->Pop();
    }

    if ( !cReplace )
    {
        // Try replacement character in 'StarBats'
        switch ( cUnicode )
        {
            // Currency
            case 0x00A2:    cReplace = (char)224;
                            break;
            case 0x00A4:    cReplace = (char)225;
                            break;
            case 0x00A5:    cReplace = (char)226;
                            break;
            case 0x20A1:    cReplace = (char)228;
                            break;
            case 0x20A2:    cReplace = (char)229;
                            break;
            case 0x20A3:    cReplace = (char)230;
                            break;
            case 0x20A4:    cReplace = (char)231;
                            break;
            case 0x20A7:    cReplace = (char)227;
                            break;
            case 0x20A8:    cReplace = (char)234;
                            break;
            case 0x20A9:    cReplace = (char)232;
                            break;
            case 0x20AB:    cReplace = (char)233;
                            break;
            case 0x20AC:    cReplace = (char)128;
                            break;

            // Punctuation and other
            case 0x201A:    cReplace = (char)130;   // SINGLE LOW-9 QUOTATION MARK
                            break;
            case 0x0192:    cReplace = (char)131;   // LATIN SMALL LETTER F WITH HOOK
                            break;
            case 0x201E:                            // DOUBLE LOW-9 QUOTATION MARK
            case 0x301F:                            // LOW DOUBLE PRIME QUOTATION MARK
                            cReplace = (char)132;
                            break;
            case 0x2026:    cReplace = (char)133;   // HORIZONTAL ELLIPSES
                            break;
            case 0x2020:    cReplace = (char)134;   // DAGGER
                            break;
            case 0x2021:    cReplace = (char)135;   // DOUBLE DAGGER
                            break;
            case 0x02C6:    cReplace = (char)136;   // MODIFIER LETTER CIRCUMFLEX ACCENT
                            break;
            case 0x2030:    cReplace = (char)137;   // PER MILLE SIGN
                            break;
            case 0x0160:    cReplace = (char)138;   // LATIN CAPITAL LETTER S WITH CARON
                            break;
            case 0x2039:    cReplace = (char)139;   // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
                            break;
            case 0x0152:    cReplace = (char)140;   // LATIN CAPITAL LIGATURE OE
                            break;
            case 0x017D:    cReplace = (char)142;   // LATIN CAPITAL LETTER Z WITH CARON
                            break;
            case 0x2018:                            // LEFT SINGLE QUOTATION MARK
            case 0x02BB:                            // MODIFIER LETTER TURNED COMMA
                            cReplace = (char)145;
                            break;
            case 0x2019:                            // RIGHT SINGLE QUOTATION MARK
            case 0x02BC:                            // MODIFIER LETTER APOSTROPHE
                            cReplace = (char)146;
                            break;
            case 0x201C:                            // LEFT DOUBLE QUOTATION MARK
            case 0x301D:                            // REVERSED DOUBLE PRIME QUOTATION MARK
                            cReplace = (char)147;
                            break;
            case 0x201D:                            // RIGHT DOUBLE QUOTATION MARK
            case 0x301E:                            // REVERSED DOUBLE PRIME QUOTATION MARK
                            cReplace = (char)148;
                            break;
            case 0x2022:    cReplace = (char)149;   // BULLET
                            break;
            case 0x2013:    cReplace = (char)150;   // EN DASH
                            break;
            case 0x2014:    cReplace = (char)151;   // EM DASH
                            break;
            case 0x02DC:    cReplace = (char)152;   // SMALL TILDE
                            break;
            case 0x2122:    cReplace = (char)153;   // TRADE MARK SIGN
                            break;
            case 0x0161:    cReplace = (char)154;   // LATIN SMALL LETTER S WITH CARON
                            break;
            case 0x203A:    cReplace = (char)155;   // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
                            break;
            case 0x0153:    cReplace = (char)156;   // LATIN SMALL LIGATURE OE
                            break;
            case 0x017E:    cReplace = (char)158;   // LATIN SMALL LETTER Z WITH CARON
                            break;
            case 0x0178:    cReplace = (char)159;   // LATIN CAPITAL LETTER Y WITH DIAERESIS
                            break;
            case 0x00B6:    cReplace = (char)222;   // PILCROW SIGN / PARAGRAPH SIGN
                            break;
        }

        if ( cReplace )
        {
            // return replacement character from 'StarBats'
            rFont.SetName( "StarBats" );
            rFont.SetCharSet( CHARSET_SYMBOL );
        }
        else
        {
            // return replacement character from original font
            cReplace = String::ConvertFromUnicode( cUnicode, rFont.GetCharSet(), TRUE );
        }
    }

    return cReplace;
}

#endif  // ENABLEUNICODE
