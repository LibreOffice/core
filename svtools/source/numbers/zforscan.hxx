/*************************************************************************
 *
 *  $RCSfile: zforscan.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: er $ $Date: 2000-10-16 18:24:30 $
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
#ifndef _ZFORSCAN_HXX
#define _ZFORSCAN_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif

#include "zformat.hxx"  // just for NfKeywordTable

class International;
class SvNumberFormatter;
struct ImpSvNumberformatInfo;


#define SC_MAX_ANZ_FORMAT_STRINGS   100
#define SC_MAX_ANZ_STANDARD_FARBEN  10
#define FLAG_STANDARD_IN_FORMAT     1000
                                            // Hack: nThousand=1000
                                            // => Standard kommt im
                                            // Format vor

enum Sc_SymbolType
{
    SYMBOLTYPE_STRING = -1,                     // String in Ausgabe
    SYMBOLTYPE_DEL    = -2,                     // Sonderzeichen
    SYMBOLTYPE_BLANK  = -3,                     // Blank fuer '_'
    SYMBOLTYPE_STAR   = -4,                     // *-Buchstabe
    SYMBOLTYPE_DIGIT  = -5,                     // Ziffernplatzhalter
    SYMBOLTYPE_DECSEP = -6,                     // Komma
    SYMBOLTYPE_THSEP  = -7,                     // Tausenderpunkt
    SYMBOLTYPE_EXP    = -8,                     // Exponent E
    SYMBOLTYPE_FRAC   = -9,                     // Bruch /
    SYMBOLTYPE_EMPTY  = -10,                    // geloeschte Symbole
    SYMBOLTYPE_FRACBLANK = -11,                 // Trenner Zahl - Zaehler bei Bruch
    SYMBOLTYPE_COMMENT = -12,                   // Kommentar folgt
    SYMBOLTYPE_CURRENCY = -13,                  // Waehrungssymbol
    SYMBOLTYPE_CURRDEL  = -14,                  // Waehrungssymbol Delimiter [$]
    SYMBOLTYPE_CURREXT  = -15                   // Waehrungssymbol Extension -xxx
};


class ImpSvNumberformatScan
{
public:

    ImpSvNumberformatScan( SvNumberFormatter* pFormatter );
    ~ImpSvNumberformatScan();
    void ChangeIntl();                          // tauscht Keywords aus

    void ChangeNullDate(USHORT nDay, USHORT nMonth, USHORT nYear);
                                                // tauscht Referenzdatum aus
    void ChangeStandardPrec(short nPrec);       // tauscht Standardprecision aus

    xub_StrLen ScanFormat( String& rString, String& rComment ); // Aufruf der Scan-Analyse

    void CopyInfo(ImpSvNumberformatInfo* pInfo,
                     USHORT nAnz);              // Kopiert die FormatInfo
    USHORT GetAnzResStrings() const             { return nAnzResStrings; }

    const International& GetIntl() const        { return *pFormatter->GetInternational(); }
    const CharClass& GetChrCls() const          { return *pFormatter->GetCharClass(); }

    const String& GetQuartalString() const  { return sKeyword[NF_KEY_QUARTER]; }
    const String& GetTrueString() const     { return sKeyword[NF_KEY_TRUE]; }
    const String& GetFalseString() const    { return sKeyword[NF_KEY_FALSE]; }
    const String& GetColorString() const    { return sKeyword[NF_KEY_COLOR]; }
    const String& GetRedString() const      { return sKeyword[NF_KEY_RED]; }
    const String& GetBooleanString() const  { return sKeyword[NF_KEY_BOOLEAN]; }
    const String& GetErrorString() const    { return sErrStr; }
    const String* GetKeyword() const        { return sKeyword; }

    Date* GetNullDate() const                   { return pNullDate; }
    const String& GetStandardName() const   { return sNameStandardFormat; }
    short GetStandardPrec() const               { return nStandardPrec; }
    const Color& GetRedColor() const            { return StandardColor[4]; }
    Color* GetColor(String& sStr);          // Setzt Hauptfarben oder
                                                // definierte Farben

    void SetConvertMode(LanguageType eTmpLge, LanguageType eNewLge)
    {
        bConvertMode = TRUE;
        eNewLnge = eNewLge;
        eTmpLnge = eTmpLge;
    }
    void SetConvertMode(BOOL bMode) { bConvertMode = bMode; }
                                                // Veraendert nur die Bool-Variable
                                                // (zum temporaeren Unterbrechen des
                                                // Convert-Modus)
    const BOOL GetConvertMode()     { return bConvertMode; }
    const LanguageType GetNewLnge() { return eNewLnge; }
                                                // Lesezugriff auf ConvertMode
                                                // und Konvertierungsland/Spr.
    const LanguageType GetTmpLnge() { return eTmpLnge; }
                                                // Lesezugriff auf
                                                // und Ausgangsland/Spr.
    SvNumberFormatter* GetNumberformatter() { return pFormatter; }
                                                // Zugriff auf Formatierer
                                                // (fuer zformat.cxx)


private:                            // ---- privater Teil
    NfKeywordTable sKeyword;                    // Schluesselworte der Syntax
    Color StandardColor[SC_MAX_ANZ_STANDARD_FARBEN];
                                                // Array der Standardfarben
    Date* pNullDate;                            // "1.1.1900"
    String sNameStandardFormat;             // "Standard"
    short nStandardPrec;                        // default Precision fuer Standardformat (2)
    SvNumberFormatter* pFormatter;              // Pointer auf die Formatliste

    String sStrArray[SC_MAX_ANZ_FORMAT_STRINGS];// Array der Symbole
    short nTypeArray[SC_MAX_ANZ_FORMAT_STRINGS];// Array der Infos
                                                // externe Infos:
    USHORT nAnzResStrings;                      // Anzahl der Ergebnissymbole
#if !(defined SOLARIS && defined X86)
    short eScannedType;                         // Typ gemaess Scan
#else
    int eScannedType;                           // wg. Optimierung
#endif
    BOOL bThousand;                             // Mit Tausenderpunkt
    USHORT nThousand;                           // Zaehlt ....-Folgen
    USHORT nCntPre;                             // Zaehlt Vorkommastellen
    USHORT nCntPost;                            // Zaehlt Nachkommastellen
    USHORT nCntExp;                             // Zaehlt Exp.Stellen, AM/PM
                                                // interne Infos:
    USHORT nAnzStrings;                         // Anzahl der Symbole
    USHORT nRepPos;                             // Position eines '*'
    USHORT nExpPos;                             // interne Position des E
    USHORT nBlankPos;                           // interne Position des Blank
    short nDecPos;                              // interne Pos. des ,
    BOOL bExp;                                  // wird bei Lesen des E gesetzt
    BOOL bFrac;                                 // wird bei Lesen des / gesetzt
    BOOL bBlank;                                // wird bei ' '(Fraction) ges.
    BOOL bDecSep;                               // Wird beim ersten , gesetzt
    String sCurString;                      // Das Waehrungssymbol in Upper
    String sErrStr;                         // String fuer Fehlerausgaben

    BOOL bConvertMode;                          // Wird im Convert-Mode gesetzt
                                                // Land/Sprache, in die der
    LanguageType eNewLnge;                      // gescannte String konvertiert
                                                // wird (fuer Excel Filter)
                                                // Land/Sprache, aus der der
    LanguageType eTmpLnge;                      // gescannte String konvertiert
                                                // wird (fuer Excel Filter)
    sal_Unicode cOldDecSep;                     // Dezimalsymbol der Ausgangs-
    sal_Unicode cOldThousandSep;                // spr., analog Tausenderpunkt
    sal_Unicode cOldDateSep;                    // Datums- und Zeitsymbol
    sal_Unicode cOldTimeSep;
    xub_StrLen nCurrPos;                        // Position des Waehrungssymbols

#ifdef _ZFORSCAN_CXX                // ----- private Methoden -----
    void SetDependentKeywords(LanguageType eLnge);
                                                // Setzt die Sprachabh. Keyw.
    void SkipStrings(USHORT& i,xub_StrLen& nPos);// Ueberspringt StringSymbole
    USHORT PreviousKeyword(USHORT i);           // Gibt Index des vorangeh.
                                                // Schluesselworts oder 0
    USHORT NextKeyword(USHORT i);               // Gibt Index des naechsten
                                                // Schluesselworts oder 0
    sal_Unicode PreviousChar(USHORT i);             // Gibt letzten Buchstaben
                                                // vor der Position,
                                                // skipt EMPTY, STRING, STAR, BLANK
    sal_Unicode NextChar(USHORT i);                 // Gibt ersten Buchst. danach
    short PreviousType( USHORT i );             // Gibt Typ vor Position,
                                                // skipt EMPTY
    BOOL IsLastBlankBeforeFrac(USHORT i);       // True <=> es kommt kein ' '
                                                // mehr bis zum '/'
    void Reset();                               // Reset aller Variablen
                                                // vor Analysestart
    USHORT GetKeyWord(const String& sSymbol); // Vergleicht mit den Keywords
                                                // return 0 <=> nicht gefunden
    short Next_Symbol(const String& rStr,
                        xub_StrLen& nPos,
                      String& sSymbol);       // Naechstes Symbol
    xub_StrLen Symbol_Division(const String& rString);// lexikalische Voranalyse
    xub_StrLen ScanType(const String& rString); // Analyse des Formattyps
    xub_StrLen FinalScan( String& rString, String& rComment );  // Endanalyse mit Vorgabe
                                                // des Typs

#endif //_ZFORSCAN_CXX
};



#endif  // _ZFORSCAN_HXX
