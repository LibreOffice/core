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
#include "precompiled_basic.hxx"

#include <stdlib.h>

#include <basic/sbxform.hxx>

/*
TODO: gibt es noch irgend welche Star-Basic Besonderheiten ?

        was bedeutet: * als Platzhalter

BEMERKUNG: Visual-Basic behandelt folgende (ung"ultige) Format-Strings
      wie angezeigt:

        ##0##.##0##     --> ##000.000##

      (diese Klasse verh"alt sich genau so).
*/

#include <stdio.h>          // f"ur: sprintf()
#include <float.h>          // f"ur: DBL_DIG, DBL_EPSILON
#include <math.h>           // f"ur: floor(), fabs(), log10(), pow()

//=================================================================
//=========================== DEFINES =============================
//=================================================================

#define _NO_DIGIT                   -1

#define MAX_NO_OF_EXP_DIGITS        5
                    // +4 wegen dem Wertebereich: zwischen -308 und +308
                    // +1 f"ur abschliessende 0
#define MAX_NO_OF_DIGITS            DBL_DIG
#define MAX_DOUBLE_BUFFER_LENGTH    MAX_NO_OF_DIGITS + 9
                    // +1 f"ur Vorzeichen
                    // +1 f"ur Ziffer vor dem Dezimal-Punkt
                    // +1 f"ur Dezimal-Punkt
                    // +2 f"ur Exponent E und Exp. Vorzeichen
                    // +3 f"ur den Wert des Exponenten
                    // +1 f"ur abschliessende 0

// Defines f"ur die Ziffern:
#define ASCII_0                     '0' // 48
#define ASCII_9                     '9' // 57

#define CREATE_1000SEP_CHAR         '@'

#define FORMAT_SEPARATOR            ';'

// vordefinierte Formate f"ur den Format$()-Befehl:
#define BASICFORMAT_GENERALNUMBER   "General Number"
#define BASICFORMAT_CURRENCY        "Currency"
#define BASICFORMAT_FIXED           "Fixed"
#define BASICFORMAT_STANDARD        "Standard"
#define BASICFORMAT_PERCENT         "Percent"
#define BASICFORMAT_SCIENTIFIC      "Scientific"
#define BASICFORMAT_YESNO           "Yes/No"
#define BASICFORMAT_TRUEFALSE       "True/False"
#define BASICFORMAT_ONOFF           "On/Off"

#define EMPTYFORMATSTRING           ""

// Bem.: Visual-Basic hat bei Floating-Point-Zahlen maximal 12 Stellen
//       nach dem Dezimal-Punkt.
// Alle Format-Strings sind kompatibel zu Visual-Basic:
#define GENERALNUMBER_FORMAT        "0.############"
            // max. 12 Stellen in Visual-Basic !
#define CURRENCY_FORMAT             "@$0.00;@($0.00)"
#define FIXED_FORMAT                "0.00"
#define STANDARD_FORMAT             "@0.00"
#define PERCENT_FORMAT              "0.00%"
#define SCIENTIFIC_FORMAT           "#.00E+00"
// BEMERKUNG: das Zeichen @ bedeutet, das Tausender-Separatoren erzeugt
//            weden sollen. Dies ist eine StarBasic 'Erweiterung'.

//=================================================================

// zur Bestimmung der Anzahl Stellen in dNumber
double get_number_of_digits( double dNumber )
//double floor_log10_fabs( double dNumber )
{
    if( dNumber==0.0 )
        // 0 hat zumindest auch eine Stelle !
        return 0.0; //ehemals 1.0, jetzt 0.0 wegen #40025;
    else
        return floor( log10( fabs( dNumber ) ) );
}

//=================================================================
//======================= IMPLEMENTATION ==========================
//=================================================================

SbxBasicFormater::SbxBasicFormater( sal_Unicode _cDecPoint, sal_Unicode _cThousandSep,
                      String _sOnStrg,
                      String _sOffStrg,
                      String _sYesStrg,
                      String _sNoStrg,
                      String _sTrueStrg,
                      String _sFalseStrg,
                      String _sCurrencyStrg,
                      String _sCurrencyFormatStrg )
{
    cDecPoint = _cDecPoint;
    cThousandSep = _cThousandSep;
    sOnStrg = _sOnStrg;
    sOffStrg = _sOffStrg;
    sYesStrg = _sYesStrg;
    sNoStrg = _sNoStrg;
    sTrueStrg = _sTrueStrg;
    sFalseStrg = _sFalseStrg;
    sCurrencyStrg = _sCurrencyStrg;
    sCurrencyFormatStrg = _sCurrencyFormatStrg;
}

// Funktion zur Ausgabe eines Fehler-Textes (zum Debuggen)
// verschiebt alle Zeichen des Strings, angefangen von der nStartPos,
// um eine Position zu gr"osseren Indizes, d.h. es wird Platz f"ur
// ein neues (einzuf"ugendes) Zeichen geschafft.
// ACHTUNG: der String MUSS gross genug sein !
inline void SbxBasicFormater::ShiftString( String& sStrg, sal_uInt16 nStartPos )
{
    sStrg.Erase( nStartPos,1 );
}

// Funktion um ein Zeichen an einen String anzuh"angen
inline void SbxBasicFormater::StrAppendChar( String& sStrg, sal_Unicode ch )
{
    sStrg.Insert( ch );
}

// h"angt die "ubergebene Ziffer nDigit an den "ubergebenen String sStrg
// an, dabei wird "uberpr"uft ob nDigit eine g"ultige Ziffer ist,
// falls dies nicht der Fall ist, wird nichts gemacht.
void SbxBasicFormater::AppendDigit( String& sStrg, short nDigit )
{
    if( nDigit>=0 && nDigit<=9 )
        StrAppendChar( sStrg, (sal_Unicode)(nDigit+ASCII_0) );
}

// verschiebt den Dezimal-Punkt um eine Stelle nach links
void SbxBasicFormater::LeftShiftDecimalPoint( String& sStrg )
{
    sal_uInt16 nPos = sStrg.Search( cDecPoint );

    if( nPos!=STRING_NOTFOUND )
    {
        // vertausche Dezimal-Punkt
        sStrg.SetChar( nPos, sStrg.GetChar( nPos - 1 ) );
        sStrg.SetChar( nPos-1, cDecPoint );
    }
}

// rundet in einem String die Ziffer an der angegebenen Stelle,
// es wird ein Flag zur"uckgeliefert, falls ein Overflow auftrat,
// d.h. 99.99 --> 100.00, d.h. ein Gr"ossenordung ge"andert wurde
// (geschieht beim Runden einer 9).
void SbxBasicFormater::StrRoundDigit( String& sStrg, short nPos, sal_Bool& bOverflow )
{
    // wurde ggf ein falscher Index uebergeben --> Aufruf ignorieren
    if( nPos<0 )
        return;

    bOverflow = sal_False;
    // "uberspringe den Dezimalpunkt und Tausender-Trennzeichen
    sal_Unicode c = sStrg.GetChar( nPos );
    if( nPos>0 && (c == cDecPoint || c == cThousandSep) )
    {
        StrRoundDigit( sStrg,nPos-1,bOverflow );
        // AENDERUNG ab 9.3.1997: nach rekursivem Call die Methode SOFORT beenden !
        return;
    }
    // "uberspringe alle nicht-Ziffern:
    // BEMERKUNG:
    // in einem g"ultigen Format-String sollte die Ausgabe
    // der Zahl an einem St"uck geschen, d.h. Sonderzeichen sollten
    // NUR vor ODER nach der Zahl stehen und nicht mitten in der
    // Format-Angabe f"ur die Zahl
    while( nPos>=0 && (sStrg.GetChar( nPos )<ASCII_0 || sStrg.GetChar( nPos )>ASCII_9) )
        nPos--;
    // muss ggf. noch Platz f"ur eine weitere (f"uhrende) Ziffer
    // geschaffen werden ?
    if( nPos==-1 )
    {
        ShiftString( sStrg,0 );
        // f"uhrende 1 einf"ugen: z.B. 99.99 f"ur 0.0
        sStrg.SetChar( 0, '1' );
        bOverflow = sal_True;
    }
    else
    {
        // ist die zu rundende Position eine Ziffer ?
        sal_Unicode c2 = sStrg.GetChar( nPos );
        if( c2 >= ASCII_0 && c2 <= ASCII_9 )
        {
            // muss eine 9 gerundet werden? Falls: Ja --> rekursiver Aufruf
            if( c2 == ASCII_9 )
            {
                sStrg.SetChar( nPos, '0' );
                StrRoundDigit( sStrg,nPos-1,bOverflow );
            }
            else
                sStrg.SetChar( nPos, c2+1 );
        }
        else
        {
        // --> Nein, d.h. Platz f"ur Ziffer schaffen: z.B. -99.99 f"ur #0.0
            // da gerundet wird MUSS es immer eine g"ultige Position
            // nPos+1 geben !
            ShiftString( sStrg,nPos+1 );
            // f"uhrende 1 einf"ugen
            sStrg.SetChar( nPos+1, '1' );
            bOverflow = sal_True;
        }
    }
}

// rundet in einem String die Ziffer an der angegebenen Stelle
void SbxBasicFormater::StrRoundDigit( String& sStrg, short nPos )
{
    sal_Bool bOverflow;

    StrRoundDigit( sStrg,nPos,bOverflow );
}

// parse den Formatstring von der "ubergebenen Position zur"uck
// und l"osche ggf. "uberf"ussige 0en, z.B. 4.50 in 0.0#
void SbxBasicFormater::ParseBack( String& sStrg, const String& sFormatStrg,
                                  short nFormatPos )
{
    // WICHTIG: nFormatPos kann auch negativ sein, in diesem Fall Aufruf ignorieren
    for( short i=nFormatPos;
         i>0 && sFormatStrg.GetChar( i ) == '#' && sStrg.GetChar( (sStrg.Len()-1) ) == '0';
         i-- )
         { sStrg.Erase( sStrg.Len()-1 ); }
}

#ifdef _with_sprintf

/*
    Bemerkung:
    Zahl wird mit maximaler (sinnvollen) Genauigkeit in einen String
    umgewandelt (mit sprintf()), dieser String wird dann im Schleifen-
    Durchlauf nach der entsprechenden Ziffer durchsucht.
*/
// initialisiert die Daten der Klasse um einen Scan-Durchlauf durchzuf"uhren
void SbxBasicFormater::InitScan( double _dNum )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];

    dNum = _dNum;
    InitExp( get_number_of_digits( dNum ) );
    // maximal 15 Nachkomma-Stellen, Format-Beispiel: -1.234000000000000E-001
    /*int nCount =*/ sprintf( sBuffer,"%+22.15lE",dNum );
    sSciNumStrg.AssignAscii( sBuffer );
}

void SbxBasicFormater::InitExp( double _dNewExp )
{
    char sBuffer[ MAX_DOUBLE_BUFFER_LENGTH ];
    // bestimme den Exponenten (kann immer GENAU durch int dargestellt werden)
    nNumExp = (short)_dNewExp;
    // und dessen String
    /*int nCount =*/ sprintf( sBuffer,"%+i",nNumExp );
    sNumExpStrg.AssignAscii( sBuffer );
    // bestimme die Anzahl der Stellen im Exponenten
    nExpExp = (short)get_number_of_digits( (double)nNumExp );
}

// bestimmt die Ziffer an der angegebenen Stelle (gedacht zur Anwendung im
// Scan-Durchlauf)
short SbxBasicFormater::GetDigitAtPosScan( short nPos, sal_Bool& bFoundFirstDigit )
{
    // Versuch eine gr"ossere Ziffer zu lesen,
    // z.B. Stelle 4 in 1.234,
    // oder eine Ziffer ausserhalb der Aufl"osung der
    // Zahl (double) zu lesen (z.B. max. 15 Stellen).
    if( nPos>nNumExp || abs(nNumExp-nPos)>MAX_NO_OF_DIGITS )
        return _NO_DIGIT;
    // bestimme den Index der Stelle in dem Number-String:
    // "uberlese das Vorzeichen
    sal_uInt16 no = 1;
    // falls notwendig den Dezimal-Punkt "uberlesen:
    if( nPos<nNumExp )
        no++;
    no += nNumExp-nPos;
    // Abfrage der ersten (g"ultigen) Ziffer der Zahl --> Flag setzen
    if( nPos==nNumExp )
        bFoundFirstDigit = sal_True;
    return (short)(sSciNumStrg.GetChar( no ) - ASCII_0);
}

short SbxBasicFormater::GetDigitAtPosExpScan( short nPos, sal_Bool& bFoundFirstDigit )
{
    // ist die abgefragte Stelle zu gross f"ur den Exponenten ?
    if( nPos>nExpExp )
        return -1;

    // bestimme den Index der Stelle in dem Number-String:
    // "uberlese das Vorzeichen
    sal_uInt16 no = 1;
    no += nExpExp-nPos;
    // Abfrage der ersten (g"ultigen) Ziffer der Zahl --> Flag setzen
    if( nPos==nExpExp )
        bFoundFirstDigit = sal_True;
    return (short)(sNumExpStrg.GetChar( no ) - ASCII_0);
}

// es kann ein Wert f"ur den Exponent angegeben werden, da ggf. die
// Zahl ggf. NICHT normiert (z.B. 1.2345e-03) dargestellt werden soll,
// sondern eventuell 123.345e-3 !
short SbxBasicFormater::GetDigitAtPosExpScan( double dNewExponent, short nPos,
                                              sal_Bool& bFoundFirstDigit )
{
    // neuer Exponent wurde "ubergeben, aktualisiere
    // die tempor"aren Klassen-Variablen
    InitExp( dNewExponent );
    // und jetzt die Stelle bestimmen
    return GetDigitAtPosExpScan( nPos,bFoundFirstDigit );
}

#else

/* Probleme mit der folgenden Methode:

TODO: ggf einen 'intelligenten' Peek-Parser um Rundungsfehler bei
      double-Zahlen herauszufinden ? z.B. f"ur  0.00115 #.#e-000

  Problem mit: format( 0.3345 ,  "0.000" )
  Problem mit: format( 0.00115 , "0.0000" )

*/
// liefert die Ziffer an der angegebenen '10er System'-Position,
// d.h. positive nPos f"ur Stellen vor dem Komma und negative
// f"ur Stellen nach dem Komma.
// nPos==0 bedeutet erste Stelle vor dem Komma, also 10^0.
// liefert 0..9 f"ur g"ultige Ziffern und -1 f"ur nicht vorhanden,
// d.h. falls die "ubergebene Zahl zu klein ist
// (z.B. Stelle 5 bei dNumber=123).
// Weiter wird in dNextNumber die um die f"uhrenden Stellen
// (bis nPos) gek"urzte Zahl zur"uckgeliefert, z.B.
//   GetDigitAtPos( 3434.565 , 2 , dNewNumber ) --> dNewNumber = 434.565
// dies kann f"ur Schleifenabarbeitung g"unstiger sein, d.h.
// die Zahlen immer von der gr"ossten Stelle abarbeiten/scanen.
// In bFoundFirstDigit wird ggf. ein Flag gesetzt wenn eine Ziffer
// gefunden wurde, dies wird dazu verwendet um 'Fehler' beim Parsen 202
// zu vermeiden, die
// ACHTUNG: anscheinend gibt es manchmal noch Probleme mit Rundungs-Fehlern!
short SbxBasicFormater::GetDigitAtPos( double dNumber, short nPos,
                                double& dNextNumber, sal_Bool& bFoundFirstDigit )
// ACHTUNG: nPos kann auch negativ werden, f"ur Stellen nach dem Dezimal-Punkt
{
    double dDigit;
    short  nMaxDigit;

    // erst mal aus der Zahl eine positive Zahl machen:
    dNumber = fabs( dNumber );

    // "uberpr"ufe ob Zahl zu klein f"ur angegebene Stelle ist
    nMaxDigit = (short)get_number_of_digits( dNumber );
    // f"uhrende Ziffern 'l"oschen'
    // Bem.: Fehler nur bei Zahlen gr"osser 0, d.h. bei Ziffern vor dem
    //       Dezimal-Punkt
    if( nMaxDigit<nPos && !bFoundFirstDigit && nPos>=0 )
        return _NO_DIGIT;
    // Ziffer gefunden, setze Flag:
    bFoundFirstDigit = sal_True;
    for( short i=nMaxDigit; i>=nPos; i-- )
    {
        double dI = (double)i;
        double dTemp1 = pow( 10.0,dI );
        // pr"apariere nun die gesuchte Ziffer:
        dDigit = floor( pow( 10.0,log10( fabs( dNumber ) )-dI ) );
        dNumber -= dTemp1 * dDigit;
    }
        // Zuweisung f"ur optimierte Schleifen-Durchl"aufe
    dNextNumber = dNumber;
    // und zum Schluss noch die float-Rundungsungenauigkeiten heraus filtern
    return RoundDigit( dDigit );
}

// rundet eine double-Zahl zwischen 0 und 9 auf die genaue
// Integer-Zahl, z.B. 2.8 -> 3 und 2.2 -> 2
short SbxBasicFormater::RoundDigit( double dNumber )
{
    // ist der Wertebereich g"ultig ?
    if( dNumber<0.0 || dNumber>10.0 )
        return -1;
    short nTempHigh = (short)(dNumber+0.5); // ggf. floor( )
    return nTempHigh;
}

#endif

// kopiert den entsprechenden Teil des Format-Strings, falls vorhanden,
// und liefert diesen zur"uck.
// Somit wird ein neuer String erzeugt, der vom Aufrufer wieder freigegeben
// werden muss
String SbxBasicFormater::GetPosFormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        bFound = sal_True;
        // der Format-String f"ur die positiven Zahlen ist alles
        // vor dem ersten ';'
        return sFormatStrg.Copy( 0,nPos );
    }
    // kein ; gefunden, liefere Leerstring
    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// siehe auch GetPosFormatString()
String SbxBasicFormater::GetNegFormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        // der Format-String f"ur die negative Zahlen ist alles
        // zwischen dem ersten und dem zweiten ';'.
        // Daher: hole erst mal alles nach dem ersten ';'
        String sTempStrg = sFormatStrg.Copy( nPos+1 );
        // und suche darin ggf. ein weiteres ';'
        nPos = sTempStrg.Search( FORMAT_SEPARATOR );
        bFound = sal_True;
        if( nPos==STRING_NOTFOUND )
            // keins gefunden, liefere alles...
            return sTempStrg;
        else
            // ansonsten den String zwischen den beiden ';' liefern
            return sTempStrg.Copy( 0,nPos );
    }
    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// siehe auch GetPosFormatString()
String SbxBasicFormater::Get0FormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        // der Format-String f"ur die Null ist alles
        // was nach dem zweiten ';' kommt.
        // Daher: hole erst mal alles nach dem ersten ';'
        String sTempStrg = sFormatStrg.Copy( nPos+1 );
        // und suche darin ggf. ein weiteres ';'
        nPos = sTempStrg.Search( FORMAT_SEPARATOR );
        if( nPos!=STRING_NOTFOUND )
        {
            bFound = sal_True;
            sTempStrg = sTempStrg.Copy( nPos+1 );
            nPos = sTempStrg.Search( FORMAT_SEPARATOR );
            if( nPos==STRING_NOTFOUND )
                // keins gefunden, liefere alles...
                return sTempStrg;
            else
                return sTempStrg.Copy( 0,nPos );
        }
    }
    // kein ; gefunden, liefere Leerstring
    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// siehe auch GetPosFormatString()
String SbxBasicFormater::GetNullFormatString( const String& sFormatStrg, sal_Bool & bFound )
{
    bFound = sal_False;     // default...
    sal_uInt16 nPos = sFormatStrg.Search( FORMAT_SEPARATOR );

    if( nPos!=STRING_NOTFOUND )
    {
        // der Format-String f"ur die Null ist alles
        // was nach dem dritten ';' kommt.
        // Daher: hole erst mal alles nach dem ersten ';'
        String sTempStrg = sFormatStrg.Copy( nPos+1 );
        // und suche darin ggf. ein weiteres ';'
        nPos = sTempStrg.Search( FORMAT_SEPARATOR );
        if( nPos!=STRING_NOTFOUND )
        {
            // und suche nun nach dem dritten ';'
            sTempStrg = sTempStrg.Copy( nPos+1 );
            nPos = sTempStrg.Search( FORMAT_SEPARATOR );
            if( nPos!=STRING_NOTFOUND )
            {
                bFound = sal_True;
                return sTempStrg.Copy( nPos+1 );
            }
        }
    }
    // kein ; gefunden, liefere Leerstring
    String aRetStr;
    aRetStr.AssignAscii( EMPTYFORMATSTRING );
    return aRetStr;
}

// analysiert den Format-String, liefert Wert <> 0 falls ein Fehler
// aufgetreten ist
short SbxBasicFormater::AnalyseFormatString( const String& sFormatStrg,
                short& nNoOfDigitsLeft, short& nNoOfDigitsRight,
                short& nNoOfOptionalDigitsLeft,
                short& nNoOfExponentDigits, short& nNoOfOptionalExponentDigits,
                sal_Bool& bPercent, sal_Bool& bCurrency, sal_Bool& bScientific,
                sal_Bool& bGenerateThousandSeparator,
                short& nMultipleThousandSeparators )
{
    sal_uInt16 nLen;
    short nState = 0;

    nLen = sFormatStrg.Len();
    // initialisiere alle Z"ahler und Flags
    nNoOfDigitsLeft = 0;
    nNoOfDigitsRight = 0;
    nNoOfOptionalDigitsLeft = 0;
    nNoOfExponentDigits = 0;
    nNoOfOptionalExponentDigits = 0;
    bPercent = sal_False;
    bCurrency = sal_False;
    bScientific = sal_False;
    // ab 11.7.97: sobald ein Komma in dem Format String gefunden wird,
    // werden alle 3 Zehnerpotenzen markiert (d.h. tausender, milionen, ...)
    // bisher wurde nur an den gesetzten Position ein Tausender-Separator
    // ausgegeben oder wenn ein @ im Format-String stand.
    // Dies war ein Missverstaendnis der VB Kompatiblitaet.
    bGenerateThousandSeparator = sFormatStrg.Search( ',' ) != STRING_NOTFOUND;
    nMultipleThousandSeparators = 0;
    // und untersuche den Format-String nach den gew"unschten Informationen
    for( sal_uInt16 i=0; i<nLen; i++ )
    {
        sal_Unicode c = sFormatStrg.GetChar( i );
        switch( c ) {
            case '#':
            case '0':
                if( nState==0 )
                {
                    nNoOfDigitsLeft++;
// TODO  hier ggf. bessere Fehler-"Uberpr"ufung der Mantisse auf g"ultige Syntax (siehe Grammatik)
                    // ACHTUNG: 'undefiniertes' Verhalten falls # und 0
                    //   gemischt werden !!!
                    // BEMERKUNG: eigentlich sind #-Platzhalter bei Scientific
                    //   Darstellung vor dem Dezimal-Punkt sinnlos !
                    if( c=='#' )
                        nNoOfOptionalDigitsLeft++;
                }
                else if( nState==1 )
                    nNoOfDigitsRight++;
                else if( nState==-1 )   // suche 0 im Exponent
                {
                    if( c=='#' )    // # schaltet den Zustand weiter
                    {
                        nNoOfOptionalExponentDigits++;
                        nState = -2;
                    }
                    nNoOfExponentDigits++;
                }
                else if( nState==-2 )   // suche # im Exponent
                {
                    if( c=='0' )
                        // ERROR: 0 nach # im Exponent ist NICHT erlaubt !!
                        return -4;
                    nNoOfOptionalExponentDigits++;
                    nNoOfExponentDigits++;
                }
                break;
            case '.':
                nState++;
                if( nState>1 )
                    return -1;  // ERROR: zu viele Dezimal-Punkte
                break;
            case '%':
                bPercent = sal_True;
                break;
            case '(':
                bCurrency = sal_True;
                break;
            case ',':
            {
                sal_Unicode ch = sFormatStrg.GetChar( i+1 );
                // vorl"aufig wird NUR auf zwei aufeinanderfolgede
                // Zeichen gepr"uft
                if( ch!=0 && (ch==',' || ch=='.') )
                    nMultipleThousandSeparators++;
            }   break;
            case 'e':
            case 'E':
                // #i13821 not when no digits before
                if( nNoOfDigitsLeft > 0 || nNoOfDigitsRight > 0 )
                {
                     nState = -1;   // breche jetzt das Z"ahlen der Stellen ab
                    bScientific = sal_True;
                }
                break;
            // EIGENES Kommando-Zeichen, das die Erzeugung der
            // Tausender-Trennzeichen einschaltet
            case '\\':
                // Ignore next char
                i++;
                break;
            case CREATE_1000SEP_CHAR:
                bGenerateThousandSeparator = sal_True;
                break;
        }
    }
    return 0;
}

// das Flag bCreateSign zeigt an, dass bei der Mantisse ein Vorzeichen
// erzeugt werden soll
void SbxBasicFormater::ScanFormatString( double dNumber,
                                const String& sFormatStrg, String& sReturnStrg,
                                sal_Bool bCreateSign )
{
    short   /*nErr,*/nNoOfDigitsLeft,nNoOfDigitsRight,nNoOfOptionalDigitsLeft,
            nNoOfExponentDigits,nNoOfOptionalExponentDigits,
            nMultipleThousandSeparators;
    sal_Bool    bPercent,bCurrency,bScientific,bGenerateThousandSeparator;

    // Initialisiere den Return-String
    sReturnStrg = String();

    // analysiere den Format-String, d.h. bestimme folgende Werte:
    /*
            - Anzahl der Ziffern vor dem Komma
            - Anzahl der Ziffern nach dem Komma
            - optionale Ziffern vor dem Komma
            - Anzahl der Ziffern im Exponent
            - optionale Ziffern im Exponent
            - Prozent-Zeichen gefunden ?
            - () f"ur negatives Vorzeichen ?
            - Exponetial-Schreibweise ?
            - sollen Tausender-Separatoren erzeugt werden ?
            - wird ein Prozent-Zeichen gefunden ? --> dNumber *= 100.0;
            - gibt es aufeinanderfolgende Tausender-Trennzeichen ?
                ,, oder ,. --> dNumber /= 1000.0;
            - sonstige Fehler ? mehrfache Dezimalpunkte, E's, etc.
        --> Fehler werden zur Zeit einfach ignoriert
    */
    AnalyseFormatString( sFormatStrg,nNoOfDigitsLeft,nNoOfDigitsRight,
                    nNoOfOptionalDigitsLeft,nNoOfExponentDigits,
                    nNoOfOptionalExponentDigits,
                    bPercent,bCurrency,bScientific,bGenerateThousandSeparator,
                    nMultipleThousandSeparators );
        // Spezialbehandlung f"ur Spezialzeichen
        if( bPercent )
            dNumber *= 100.0;
// TODO: diese Vorgabe (,, oder ,.) ist NICHT Visual-Basic kompatibel !
        // Frage: soll das hier stehen bleiben (Anforderungen) ?
        if( nMultipleThousandSeparators )
            dNumber /= 1000.0;

        // einige Arbeits-Variablen
        double dExponent;
        short i,nLen;
        short nState,nDigitPos,nExponentPos,nMaxDigit,nMaxExponentDigit;
        sal_Bool bFirstDigit,bFirstExponentDigit,bFoundFirstDigit,
             bIsNegative,bZeroSpaceOn, bSignHappend,bDigitPosNegative;

        // Initialisierung der Arbeits-Variablen
        bSignHappend = sal_False;
        bFoundFirstDigit = sal_False;
        bIsNegative = dNumber<0.0;
        nLen = sFormatStrg.Len();
        dExponent = get_number_of_digits( dNumber );
        nExponentPos = 0;
        nMaxExponentDigit = 0;
        nMaxDigit = (short)dExponent;
        bDigitPosNegative = false;
        if( bScientific )
        {
            // beim Exponent ggf. "uberz"ahlige Stellen vor dem Komma abziehen
            dExponent = dExponent - (double)(nNoOfDigitsLeft-1);
            nDigitPos = nMaxDigit;
            nMaxExponentDigit = (short)get_number_of_digits( dExponent );
            nExponentPos = nNoOfExponentDigits-1 - nNoOfOptionalExponentDigits;
        }
        else
        {
            nDigitPos = nNoOfDigitsLeft-1; // Z"ahlweise f"angt bei 0 an, 10^0
            // hier ben"otigt man keine Exponent-Daten !
            bDigitPosNegative = (nDigitPos < 0);
        }
        bFirstDigit = sal_True;
        bFirstExponentDigit = sal_True;
        nState = 0; // 0 --> Mantisse; 1 --> Exponent
        bZeroSpaceOn = 0;


#ifdef _with_sprintf
        InitScan( dNumber );
#endif
        // scanne jetzt den Format-String:
        sal_Unicode cForce = 0;
        for( i=0; i<nLen; i++ )
        {
            sal_Unicode c;
            if( cForce )
            {
                c = cForce;
                cForce = 0;
            }
            else
            {
                c = sFormatStrg.GetChar( i );
            }
            switch( c ) {
                case '0':
                case '#':
                    if( nState==0 )
                    {
                    // Behandlung der Mantisse
                        if( bFirstDigit )
                        {
                            // ggf. Vorzeichen erzeugen
                            // Bem.: bei bCurrency soll das negative
                            //       Vorzeichen durch () angezeigt werden
                            if( bIsNegative && !bCreateSign && !bSignHappend )
                            {
                                // nur einmal ein Vorzeichen ausgeben
                                bSignHappend = sal_True;
                                StrAppendChar( sReturnStrg,'-' );
                            }
                            // hier jetzt "uberz"ahlige Stellen ausgeben,
                            // d.h. vom Format-String nicht erfasste Stellen
                            if( nMaxDigit>nDigitPos )
                            {
                                for( short j=nMaxDigit; j>nDigitPos; j-- )
                                {
                                    short nTempDigit;
#ifdef _with_sprintf
                                    AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPosScan( j,bFoundFirstDigit ) );
#else
                                    AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPos( dNumber,j,dNumber,bFoundFirstDigit ) );
#endif
                                    // wurde wirklich eine Ziffer eingefuegt ?
                                    if( nTempDigit!=_NO_DIGIT )
                                        // jetzt wurde wirklich eine Ziffer ausgegeben, Flag setzen
                                        bFirstDigit = sal_False;
                                    // muss ggf. ein Tausender-Trennzeichen erzeugt werden?
                                    if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && j>0 && (j % 3 == 0) )
                                        StrAppendChar( sReturnStrg,cThousandSep );
                                }
                            }
                        }
                        // muss f"ur eine leere Stelle eventuell eine 0 ausgegeben werden ?
                        if( nMaxDigit<nDigitPos && ( c=='0' || bZeroSpaceOn ) )
                        {
                            AppendDigit( sReturnStrg,0 );       // Ja
                            // jetzt wurde wirklich eine Ziffer ausgegeben, Flag setzen
                            bFirstDigit = sal_False;
                            bZeroSpaceOn = 1;
                            // BEM.: bei Visual-Basic schaltet die erste 0 f"ur alle
                            //       nachfolgenden # (bis zum Dezimal-Punkt) die 0 ein,
                            //       dieses Verhalten wird hier mit dem Flag simmuliert.
                            // muss ggf. ein Tausender-Trennzeichen erzeugt werden?
                            if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && nDigitPos>0 && (nDigitPos % 3 == 0) )
                                StrAppendChar( sReturnStrg,cThousandSep );
                        }
                        else
                        {
                            short nTempDigit;
#ifdef _with_sprintf
                            AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPosScan( nDigitPos,bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg,nTempDigit = GetDigitAtPos( dNumber,nDigitPos,dNumber,bFoundFirstDigit ) );
#endif
                            // wurde wirklich eine Ziffer eingefuegt ?
                            if( nTempDigit!=_NO_DIGIT )
                                // jetzt wurde wirklich eine Ziffer ausgegeben, Flag setzen
                                bFirstDigit = sal_False;
                            // muss ggf. ein Tausender-Trennzeichen erzeugt werden?
                            if( bGenerateThousandSeparator && ( c=='0' || nMaxDigit>=nDigitPos ) && nDigitPos>0 && (nDigitPos % 3 == 0) )
                                StrAppendChar( sReturnStrg,cThousandSep );
                        }
                        // und Position aktualisieren
                        nDigitPos--;
                    }
                    else
                    {
                    // Behandlung des Exponenten
                        if( bFirstExponentDigit )
                        {
                            // Vorzeichen wurde schon bei e/E ausgegeben
                            bFirstExponentDigit = sal_False;
                            if( nMaxExponentDigit>nExponentPos )
                            // hier jetzt "uberz"ahlige Stellen ausgeben,
                            // d.h. vom Format-String nicht erfasste Stellen
                            {
                                for( short j=nMaxExponentDigit; j>nExponentPos; j-- )
                                {
#ifdef _with_sprintf
                                    AppendDigit( sReturnStrg,GetDigitAtPosExpScan( dExponent,j,bFoundFirstDigit ) );
#else
                                    AppendDigit( sReturnStrg,GetDigitAtPos( dExponent,j,dExponent,bFoundFirstDigit ) );
#endif
                                }
                            }
                        }
                        // muss f"ur eine leere Stelle eventuell eine 0 ausgegeben werden ?
                        if( nMaxExponentDigit<nExponentPos && c=='0' )
                            AppendDigit( sReturnStrg,0 );       // Ja
                        else
#ifdef _with_sprintf
                            AppendDigit( sReturnStrg,GetDigitAtPosExpScan( dExponent,nExponentPos,bFoundFirstDigit ) );
#else
                            AppendDigit( sReturnStrg,GetDigitAtPos( dExponent,nExponentPos,dExponent,bFoundFirstDigit ) );
#endif
                        nExponentPos--;
                    }
                    break;
                case '.':
                    if( bDigitPosNegative ) // #i13821: If no digits before .
                    {
                        bDigitPosNegative = false;
                        nDigitPos = 0;
                        cForce = '#';
                        i-=2;
                        break;
                    }
                    // gebe Komma aus
                    StrAppendChar( sReturnStrg,cDecPoint );
                    break;
                case '%':
                    // ggf. "uberf"ussige 0en l"oschen, z.B. 4.500e4 in 0.0##e-00
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    // gebe Prozent-Zeichen aus
                    sReturnStrg.Insert('%');
                    break;
                case 'e':
                case 'E':
                    // muss Mantisse noch gerundet werden, bevor der Exponent angezeigt wird ?
                    {
                        // gibt es ueberhaupt eine Mantisse ?
                        if( bFirstDigit )
                        {
                            // anscheinend nicht, d.h. ungueltiger Format String, z.B. E000.00
                            // d.h. ignoriere diese e bzw. E Zeichen
                            // ggf. einen Fehler (wie Visual Basic) ausgeben ?

                            // #i13821: VB 6 behaviour
                            StrAppendChar( sReturnStrg,c );
                            break;
                        }

                        sal_Bool bOverflow = sal_False;
#ifdef _with_sprintf
                        short nNextDigit = GetDigitAtPosScan( nDigitPos,bFoundFirstDigit );
#else
                        short nNextDigit = GetDigitAtPos( dNumber,nDigitPos,dNumber,bFoundFirstDigit );
#endif
                        if( nNextDigit>=5 )
                            StrRoundDigit( sReturnStrg,sReturnStrg.Len()-1,bOverflow );
                        if( bOverflow )
                        {
                            // es wurde eine f"uhrende 9 gerundet, d.h.
                            // verschiebe den Dezimal-Punkt um eine Stelle nach links
                            LeftShiftDecimalPoint( sReturnStrg );
                            // und l"osche die letzte Ziffer, diese wird
                            // duch die f"uhrende 1 ersetzt:
                            sReturnStrg.SetChar( sReturnStrg.Len()-1 , 0 );
                            // der Exponent muss um 1 erh"oht werden,
                            // da der Dezimalpunkt verschoben wurde
                            dExponent += 1.0;
                        }
                        // ggf. "uberf"ussige 0en l"oschen, z.B. 4.500e4 in 0.0##e-00
                        ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    }
                    // "andere Zustand des Scanners
                    nState++;
                    // gebe Exponent-Zeichen aus
                    StrAppendChar( sReturnStrg,c );
                    // i++; // MANIPULATION der Schleifen-Variable !
                    c = sFormatStrg.GetChar( ++i );
                    // und gebe Vorzeichen / Exponent aus
                    if( c!=0 )
                    {
                        if( c=='-' )
                        {
                            // falls Exponent < 0 gebe - aus
                            if( dExponent<0.0 )
                                StrAppendChar( sReturnStrg,'-' );
                        }
                        else if( c=='+' )
                        {
                            // gebe auf jeden Fall das Vorzeichen des Exponenten aus !
                            if( dExponent<0.0 )
                                StrAppendChar( sReturnStrg,'-' );
                            else
                                StrAppendChar( sReturnStrg,'+' );
                        }
                    }
                    break;
                case ',':
                    break;
                case ';':
                    break;
                case '(':
                case ')':
                    // ggf. "uberf"ussige 0en l"oschen, z.B. 4.500e4 in 0.0##e-00
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    if( bIsNegative )
                        StrAppendChar( sReturnStrg,c );
                    break;
                case '$':
                    // den String fuer die Waehrung dranhengen:
                    sReturnStrg += sCurrencyStrg;
                    break;
                case ' ':
                case '-':
                case '+':
                    // ggf. "uberf"ussige 0en l"oschen, z.B. 4.500e4 in 0.0##e-00
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    // gebe das jeweilige Zeichen direkt aus
                    StrAppendChar( sReturnStrg,c );
                    break;
                case '\\':
                    // ggf. "uberf"ussige 0en l"oschen, z.B. 4.500e4 in 0.0##e-00
                    // falls Sonderzeichen am Ende oder mitten in
                    // Format-String vorkommen
                    ParseBack( sReturnStrg,sFormatStrg,i-1 );
                    // Sonderzeichen gefunden, gebe N"ACHSTES
                    // Zeichen direkt aus (falls es existiert)
                    c = sFormatStrg.GetChar( ++i );
                    if( c!=0 )
                        StrAppendChar( sReturnStrg,c );
                    break;
                case CREATE_1000SEP_CHAR:
                    // hier ignorieren, Aktion wurde schon in
                    // AnalyseFormatString durchgef"uhrt
                    break;
                default:
                    // auch die Zeichen und Ziffern ausgeben (wie in Visual-Basic)
                    if( ( c>='a' && c<='z' ) ||
                        ( c>='A' && c<='Z' ) ||
                        ( c>='1' && c<='9' ) )
                        StrAppendChar( sReturnStrg,c );
            }
        }
        // Format-String wurde vollst"andig gescanned,
        // muss die letzte Stelle nun gerundet werden ?
        // Dies hier ist jedoch NUR notwendig, falls das
        // Zahlenformat NICHT Scientific-Format ist !
        if( !bScientific )
        {
#ifdef _with_sprintf
            short nNextDigit = GetDigitAtPosScan( nDigitPos,bFoundFirstDigit );
#else
            short nNextDigit = GetDigitAtPos( dNumber,nDigitPos,dNumber,bFoundFirstDigit );
#endif
            if( nNextDigit>=5 )
                StrRoundDigit( sReturnStrg,sReturnStrg.Len()-1 );
        }
        // und ganz zum Schluss:
        // ggf. "uberf"ussige 0en l"oschen, z.B. 4.500e4 in 0.0##e-00#,
        // ABER nur Stellen nach dem Dezimal-Punkt k"onnen gel"oscht werden
        if( nNoOfDigitsRight>0 )
            ParseBack( sReturnStrg,sFormatStrg,sFormatStrg.Len()-1 );
}

String SbxBasicFormater::BasicFormatNull( String sFormatStrg )
{
    sal_Bool bNullFormatFound;
    String sNullFormatStrg = GetNullFormatString( sFormatStrg,bNullFormatFound );

    if( bNullFormatFound )
        return sNullFormatStrg;
    String aRetStr;
    aRetStr.AssignAscii( "null" );
    return aRetStr;
}

String SbxBasicFormater::BasicFormat( double dNumber, String sFormatStrg )
{
    sal_Bool bPosFormatFound,bNegFormatFound,b0FormatFound;

    // analysiere Format-String auf vordefinierte Formate:
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_GENERALNUMBER ) )
        sFormatStrg.AssignAscii( GENERALNUMBER_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_CURRENCY ) )
        sFormatStrg = sCurrencyFormatStrg; // old: CURRENCY_FORMAT;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_FIXED ) )
        sFormatStrg.AssignAscii( FIXED_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_STANDARD ) )
        sFormatStrg.AssignAscii( STANDARD_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_PERCENT ) )
        sFormatStrg.AssignAscii( PERCENT_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_SCIENTIFIC ) )
        sFormatStrg.AssignAscii( SCIENTIFIC_FORMAT );
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_YESNO ) )
        return ( dNumber==0.0 ) ? sNoStrg : sYesStrg ;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_TRUEFALSE ) )
        return ( dNumber==0.0 ) ? sFalseStrg : sTrueStrg ;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_ONOFF ) )
        return ( dNumber==0.0 ) ? sOffStrg : sOnStrg ;

    // analysiere Format-String auf ';', d.h. Format-Strings f"ur
    // positive-, negative- und 0-Werte
    String sPosFormatStrg = GetPosFormatString( sFormatStrg, bPosFormatFound );
    String sNegFormatStrg = GetNegFormatString( sFormatStrg, bNegFormatFound );
    String s0FormatStrg = Get0FormatString( sFormatStrg, b0FormatFound );

    String sReturnStrg;
    String sTempStrg;

    if( dNumber==0.0 )
    {
        sTempStrg = sFormatStrg;
        if( b0FormatFound )
        {
            // wurde ggf. Leer-String uebergeben ?
            if( s0FormatStrg.Len() == 0 && bPosFormatFound )
                // --> Ja, dann verwende String fuer positive Werte
                sTempStrg = sPosFormatStrg;
            else
                sTempStrg = s0FormatStrg;
        }
        else if( bPosFormatFound )
        {
            // verwende String fuer positive Werte
            sTempStrg = sPosFormatStrg;
        }
        ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/sal_False );
    }
    else
    {
        if( dNumber<0.0 )
        {
            if( bNegFormatFound )
            {
                // wurde ggf. Leer-String uebergeben ?
                if( sNegFormatStrg.Len() == 0 && bPosFormatFound )
                {
                    // --> Ja, dann verwende String fuer positive Werte
                    // und setzte Minus-Zeichen davor !
                    sTempStrg = String::CreateFromAscii("-");
                    sTempStrg += sPosFormatStrg;
                }
                else
                    sTempStrg = sNegFormatStrg;
           }
            else
                sTempStrg = sFormatStrg;
            // falls KEIN Format-String speziell f"ur negative Werte angegeben
            // wurde, so soll das Vorzeichen ausgegeben werden
            ScanFormatString( dNumber, sTempStrg, sReturnStrg,/*bCreateSign=*/bNegFormatFound/*sNegFormatStrg!=EMPTYFORMATSTRING*/ );
        }
        else // if( dNumber>0.0 )
        {
            ScanFormatString( dNumber,
                    (/*sPosFormatStrg!=EMPTYFORMATSTRING*/bPosFormatFound ? sPosFormatStrg : sFormatStrg),
                    sReturnStrg,/*bCreateSign=*/sal_False );
        }
    }
    return sReturnStrg;
}

sal_Bool SbxBasicFormater::isBasicFormat( String sFormatStrg )
{
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_GENERALNUMBER ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_CURRENCY ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_FIXED ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_STANDARD ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_PERCENT ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_SCIENTIFIC ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_YESNO ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_TRUEFALSE ) )
        return sal_True;
    if( sFormatStrg.EqualsIgnoreCaseAscii( BASICFORMAT_ONOFF ) )
        return sal_True;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
