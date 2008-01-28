/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxform.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 13:58:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SBXFORM_HXX
#define _SBXFORM_HXX

//====================================================================
// Klasse zur Implementation des Basic-Befehls: Format$( d,formatStr )
//====================================================================
/*
  Die Grammatik des Format-Strings (ein Versuch):
  -----------------------------------------------

  format_string     := {\special_char} general_format | scientific_format {\special_char} {;format_string}
  general_format    := {#[,]}{0[,]}[.{0}{#}]
  scientific_format := {0}[.{0}{#}](e | E)(+ | -){#}{0}

  percent_char      := '%'
  special_char      := \char | + | - | ( | ) | $ | space_char
  char              := all_ascii_chars
  space_char        := ' '

  {}    mehrfach wiederholt, auch null-mal
  []    genau einmal oder null-mal
  ()    Klammer, z.B. (e | E) heisst e oder E, genau einmal

  Weitere vordefinierte Formate f"ur den Format-String im Format$()-Befehl:
    "General Number"
    "Currency"
    "Fixed"
    "Standard"
    "Percent"
    "Scientific"
    "Yes/No"
    "True/False"
    "On/Off"

 Bemerkung: fehlerhafte Format-Strings werden wie bei Visual-Basic ignoriert,
            die Ausgabe ist dann ggf. 'undefiniert'.
            Ascii-Buchstaben werden direkt ausgegeben.

 Einschr"ankungen in Visual-Basic:
    - der Exponent (wiss. Schreibweise) kann maximal 3 Stellen haben !

 Einschr"ankungen der neuen Implementation:
    - das '+' Zeichen als Platzhalter bei der Mantisse ist nicht erlaubt

 TODO:
    - Datums-Formatierung
        Platzhalter sind: 'h', 'm', 's', 'y'
        vordefinierte String-Konstanten/Befehle:
            "AMPM", "Long Date", "Long Time"
*/

/*
    es gibt zwei M"oglichkeiten eine Zahl auf einzelne
    Ziffern zu untersuchen:

        a) verwende sprintf()
        b) verwende log10() und pow() um Ziffer
           'selbst' zu pr"aparieren (hab Probleme mit Rundungsfehlern!)
*/
#define _with_sprintf   // verwende M"oglichkeit a)

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

class SbxBasicFormater {
  public:
    // der Konstruktor, nimmt die Zeichen f"ur den Dezimal-Punkt,
    // das Tausender-Trenn-Zeichen sowie die notwendigen Resource
    // Strings als Parameter.
    SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      String _sOnStrg,
                      String _sOffStrg,
                      String _sYesStrg,
                      String _sNoStrg,
                      String _sTrueStrg,
                      String _sFalseStrg,
                      String _sCurrencyStrg,
                      String _sCurrencyFormatStrg );

    /* der Basic-Befehl: Format$( number,format-string )

       Parameter:
        dNumber     : die Zahl, die formatiert ausgegeben werden soll
        sFormatStrg : der Format-String, z.B. ###0.0###

       Return-Wert:
        String mit der gew"unschten, formatierten Ausgabe
    */
    String  BasicFormat( double dNumber, String sFormatStrg );
    String  BasicFormatNull( String sFormatStrg );

    static  BOOL isBasicFormat( String sFormatStrg );

  private:
    //*** einige Hilfsmethoden ***
    //void  ShowError( char *sErrMsg );
    inline void ShiftString( String& sStrg, USHORT nStartPos );
    inline void StrAppendChar( String& sStrg, sal_Unicode ch );
    void    AppendDigit( String& sStrg, short nDigit );
    void    LeftShiftDecimalPoint( String& sStrg );
    void    StrRoundDigit( String& sStrg, short nPos, BOOL& bOverflow );
    void    StrRoundDigit( String& sStrg, short nPos );
    void    ParseBack( String& sStrg, const String& sFormatStrg,
                short nFormatPos );
#ifdef _with_sprintf
    // Methoden "uber String-Konvertierung mit sprintf():
    void    InitScan( double _dNum );
    void    InitExp( double _dNewExp );
    short   GetDigitAtPosScan( short nPos, BOOL& bFoundFirstDigit );
    short   GetDigitAtPosExpScan( double dNewExponent, short nPos,
                BOOL& bFoundFirstDigit );
    short   GetDigitAtPosExpScan( short nPos, BOOL& bFoundFirstDigit );
#else
    // Methoden "uber direktes 'ausrechen' mit log10() und pow():
    short   GetDigitAtPos( double dNumber, short nPos, double& dNextNumber,
                BOOL& bFoundFirstDigit );
    short   RoundDigit( double dNumber );
#endif
    String  GetPosFormatString( const String& sFormatStrg, BOOL & bFound );
    String  GetNegFormatString( const String& sFormatStrg, BOOL & bFound );
    String  Get0FormatString( const String& sFormatStrg, BOOL & bFound );
    String  GetNullFormatString( const String& sFormatStrg, BOOL & bFound );
    short   AnalyseFormatString( const String& sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits,
                short& nNoOfOptionalExponentDigits,
                BOOL& bPercent, BOOL& bCurrency, BOOL& bScientific,
                BOOL& bGenerateThousandSeparator,
                short& nMultipleThousandSeparators );
    void    ScanFormatString( double dNumber, const String& sFormatStrg,
                String& sReturnStrg, BOOL bCreateSign );

    //*** Daten ***
    sal_Unicode cDecPoint;      // das Zeichen f"ur den Dezimal-Punkt
    sal_Unicode cThousandSep;   // das Zeichen f"ur das Tausender-Trennzeichen
    // Texte zur Ausgabe:
    String  sOnStrg;
    String  sOffStrg;
    String  sYesStrg;
    String  sNoStrg;
    String  sTrueStrg;
    String  sFalseStrg;
    String  sCurrencyStrg;
    String  sCurrencyFormatStrg;

    //*** tempor"are Daten f"ur die Scan-Schleife ***
    //-----------------------------------------------
    // Zeichenkette, in dem die Zahl als Scientific-Format abgelegt wird
    String  sSciNumStrg;
    // Zeichenkette, in der der Exponent der Zahl abgelegt wird
    String  sNumExpStrg;
    double  dNum;           // die zu scannede Zahl
    short   nNumExp;        // der Exponent der zu scannenden Zahl
    short   nExpExp;        // die Anzahl der Stellen im Exponenten
};

#endif

