/*************************************************************************
 *
 *  $RCSfile: zforfind.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
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

#ifndef _ZFORFIND_HXX
#define _ZFORFIND_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class International;
class Date;
class SvNumberformat;
class SvNumberFormatter;

//  passiert in "string.hxx":
//
//  #ifdef ENABLEUNICODE            /* bei Unicode */
//  #define XubString UniString     /* Wide-Strings */
//  #define xub_Unicode sal_Unicode         /* Wide-Character */
//  #else                           /* sonst */
//  #define XubString String            /* normale Strings */
//  #define xub_Unicode char                /* normaler char */
//  #endif

#define SV_MAX_ANZ_INPUT_STRINGS  20            // max. Anzahl der Sub-Strings

class ImpSvNumberInputScan
{
public:                             // ---- oeffentlicher Teil
    ImpSvNumberInputScan( SvNumberFormatter* pFormatter );
    ~ImpSvNumberInputScan();

/*!*/   void ChangeIntl();                      // MUST be called if language changes

    void ChangeNullDate(                        // Referenzdatum setzen
            const USHORT nDay,
            const USHORT nMonth,
            const USHORT nYear );

    BOOL IsNumberFormat(                        // Eingabe in Zahl umwandeln
            const XubString& rString,           // Eingabestring
            short& F_Type,                      // Typus, Format (in + out)
            double& fOutNumber,                 // ermittelter Wert
            const SvNumberformat* pFormat = NULL ); // evtl. gesetztes Zahlenformat

                                                // nach IsNumberFormat:
    short   GetDecPos() const { return nDecPos; }
    USHORT  GetAnzNums() const { return nAnzNums; }
    void    SetYear2000( USHORT nVal ) { nYear2000 = nVal; }
    USHORT  GetYear2000() const { return nYear2000; }

private:                            // ---- privater Teil
    SvNumberFormatter*  pFormatter;
    XubString aUpperMonthText[12];              // Die 12 Monate
    XubString aUpperAbbrevMonthText[12];            // Die 12 Monate, abgekuerzt
    XubString aUpperDayText[7];                 // Die 7 Wochentage
    XubString aUpperAbbrevDayText[7];           // Die 7 Wochentage, abgekuerzt
    XubString aUpperCurrSymbol;                 // Das Waehrungssymbol
    BOOL    bTextInitialized;                   // Sind Monate und Wochentage initialisert?
    Date* pNullDate;                            // "1.1.1900"
                                                // Variablen für Zwischenergebnisse:
    XubString sStrArray[SV_MAX_ANZ_INPUT_STRINGS];  // Array der Zahl- oder Str.-Symbole
    BOOL   IsNum[SV_MAX_ANZ_INPUT_STRINGS];     // Markiert die Zahl-Symb.
    USHORT nNums[SV_MAX_ANZ_INPUT_STRINGS];     // Die Zahlen in Reihenfolge
    USHORT nAnzStrings;                         // Gesamtzahl der Symbole
    USHORT nAnzNums;                            // Anzahl der Zahlensymbole
    BOOL   bDecSepInDateSeps;                   // True <=> DecSep in {.,-,/,DateSep}

    short  nSign;                               // Vorzeichen der Zahl
    short  nMonth;                              // Monat(1..12), falls Datum
                                                // negativ => Kurzformat
    short  nMonthPos;                           // 1 = vorn, 2 = Mitte
                                                // 3 = hinten
    USHORT nTimePos;                            // Index of first time separator (+1)
    short  nDecPos;                             // Index des Strings mit ", " (+1)
    short  nNegCheck;                           // '( )' fuer negativ
    short  nESign;                              // Vorzeichen Exp
    short  nAmPm;                               // +1 AM, -1 PM, sonst 0
    short  nLogical;                            // -1 => False, 1 => True
    USHORT nThousand;                           // Anz. der Tausenderpkte.
    USHORT nPosThousandString;                  // Position des zusammenge-
                                                // fassten 000.000.000-Strings
    short  eScannedType;                        // Typ gemaess Scan
    short  eSetType;                            // Typ gemaess Zelle

    USHORT nStringScanNumFor;                   // fixe Strings erkannt in
                                                // pFormat->NumFor[nNumForStringScan]
    short  nStringScanSign;                     // Vorzeichen durch FixString
    USHORT nYear2000;                           // Bis zu welcher Zahl zweistellige
                                                // Jahresangaben als 20xx erkannt
                                                // werden, default 18
                                                // Zahl <= nYear2000 => 20xx
                                                // Zahl >  nYear2000 => 19xx

#ifdef _ZFORFIND_CXX                // ----- private Methoden -----
    void Reset();                               // Reset aller Variablen vor Analysestart

    void InitText();                            // Monate und Wochentage initialisieren

    double StringToDouble(                      // String in double umwandlen
            const XubString& rStr );

    BOOL NextNumberStringSymbol(                // Naechstes Zahl/String Symbol
            const xub_Unicode*& pStr,
            XubString& rSymbol );

    BOOL SkipThousands(                         // Fasst .000.123 Bloecke in
            const xub_Unicode*& pStr,                   // der Eingabe zusammen 000123
            XubString& rSymbol );
    void NumberStringDivision(                  // Zerlegen in Zahlen/Strings in obige Arrays und Var.
            const XubString& rString );         // Leerzeichen am Anfang und hinter Zahlen fallen weg!

    static BOOL StringContains(                 // if rString contains rWhat at nPos
            const XubString& rWhat,
            const XubString& rString,
            xub_StrLen nPos );
    static inline BOOL SkipChar(                // spezielles Zeichen ueberspringen
            xub_Unicode c,
            const XubString& rString,
            xub_StrLen& nPos );
    static inline void SkipBlanks(              // ueberspringe Leerzeichen
            const XubString& rString,
            xub_StrLen& nPos );
    static inline BOOL SkipString(              // jump over rWhat in rString at nPos
            const XubString& rWhat,
            const XubString& rString,
            xub_StrLen& nPos );

    inline BOOL GetThousandSep(                 // erkennt genau .111 als Tausenderpunkt
            const XubString& rString,
            xub_StrLen& nPos,
            USHORT nStringPos );
    short GetLogical(                           // logischen Wert holen
            const XubString& rString );
    short GetMonth(                             // Monat holen
            const XubString& rString,
            xub_StrLen& nPos );
    short GetDayOfWeek(                         // Wochentag holen
            const XubString& rString,
            xub_StrLen& nPos );
    BOOL GetCurrency(                           // Waehrungssysmbol holen
            const XubString& rString,
            xub_StrLen& nPos,
            const SvNumberformat* pFormat = NULL ); // evtl. gesetztes Zahlenformat
    BOOL GetTimeAmPm(                           // Symbol Am od. Pm holen
            const XubString& rString,
            xub_StrLen& nPos );
    inline BOOL GetDecSep(                      // Dezimaltrenner holen
            const XubString& rString,
            xub_StrLen& nPos );
    short GetSign(                              // Vorzeichen holen
            const XubString& rString,           // mit Sonderfall '('
            xub_StrLen& nPos );
    short GetESign(                             // Vorzeichen holen
            const XubString& rString,           // f. Exponent
            xub_StrLen& nPos );

    inline BOOL GetNextNumber(                  // Naechste Zahl holen
            USHORT& i,
            USHORT& j );
    void GetTimeRef(                            // Umwandlung Zeit -> double (nur Nachkomma)
            double& fOutNumber,                 // Ergebniss als double
            USHORT nIndex,                      // Index der Stunde in der Eingabe,
            USHORT nAnz );                      // Anz der Zeitstrings in der Eingabe
    USHORT ImplGetDay  ( USHORT nIndex );       // Tag: Eingabe od. aktuell
    USHORT ImplGetMonth( USHORT nIndex );       // Monat: Eingabe od. aktuell
    USHORT ImplGetYear ( USHORT nIndex );       // Jahr: nix: aktuell, xx=19xx
    BOOL GetDateRef(                            // Umwandlung Datum -> Date
            Date& aDt,
            USHORT& nCounter,                   // zaehlt Datumsstrings
            const SvNumberformat* pFormat = NULL ); // evtl. gesetztes Zahlenformat

    BOOL ScanStartString(                       // Analyse des Anfangs
            const XubString& rString,
            const SvNumberformat* pFormat = NULL );
    BOOL ScanMidString(                         // Analyse der Mitte
            const XubString& rString,
            USHORT nStringPos );
    BOOL ScanEndString(                         // Analyse des Endes
            const XubString& rString,
            const SvNumberformat* pFormat = NULL );

    BOOL ScanStringNumFor(
            const XubString& rString,
            xub_StrLen nPos,
            const SvNumberformat* pFormat,
            USHORT nString );
        // String mit TeilString nString aus Format vergleichen,
        // nString == 0xFFFF => letzter

    BOOL IsNumberFormatMain(                    // Verteilfunktion zur Analyse
            const XubString& rString,
            double& fOutNumber,                 // Rueckgabewert, falls als Zahl darstellbar
            const SvNumberformat* pFormat = NULL ); // evtl. gesetztes Zahlenformat


#endif  // _ZFORFIND_CXX
};



#endif  // _ZFORFIND_HXX
