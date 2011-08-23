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
#ifndef _ZFORSCAN_HXX
#define _ZFORSCAN_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_NFKEYTAB_HXX
#include <bf_svtools/nfkeytab.hxx>
#endif

namespace binfilter
{

class SvNumberFormatter;
struct ImpSvNumberformatInfo;


const size_t NF_MAX_FORMAT_SYMBOLS   = 100;
const size_t NF_MAX_DEFAULT_COLORS   = 10;

// Hack: nThousand==1000 => "Default" occurs in format string
const USHORT FLAG_STANDARD_IN_FORMAT = 1000;

class ImpSvNumberformatScan
{
public:

    ImpSvNumberformatScan( SvNumberFormatter* pFormatter );
    ~ImpSvNumberformatScan();
    void ChangeIntl();							// tauscht Keywords aus

    void ChangeNullDate(USHORT nDay, USHORT nMonth, USHORT nYear);
                                                // tauscht Referenzdatum aus
    void ChangeStandardPrec(short nPrec);		// tauscht Standardprecision aus

    xub_StrLen ScanFormat( String& rString, String& rComment );	// Aufruf der Scan-Analyse

    void CopyInfo(ImpSvNumberformatInfo* pInfo,
                     USHORT nAnz);				// Kopiert die FormatInfo
    USHORT GetAnzResStrings() const				{ return nAnzResStrings; }

    const CharClass& GetChrCls() const			{ return *pFormatter->GetCharClass(); }
    const LocaleDataWrapper& GetLoc() const		{ return *pFormatter->GetLocaleData(); }
    CalendarWrapper& GetCal() const				{ return *pFormatter->GetCalendar(); }

    const String* GetKeywords() const
        {
            if ( bKeywordsNeedInit )
                InitKeywords();
            return sKeyword;
        }
    // Keywords used in output like TRUE and FALSE
    const String& GetSpecialKeyword( NfKeywordIndex eIdx ) const
        {
            if ( !sKeyword[eIdx].Len() )
                InitSpecialKeyword( eIdx );
            return sKeyword[eIdx];
        }
    const String& GetTrueString() const     { return GetSpecialKeyword( NF_KEY_TRUE ); }
    const String& GetFalseString() const    { return GetSpecialKeyword( NF_KEY_FALSE ); }
    const String& GetColorString() const    { return GetKeywords()[NF_KEY_COLOR]; }
    const String& GetRedString() const      { return GetKeywords()[NF_KEY_RED]; }
    const String& GetBooleanString() const  { return GetKeywords()[NF_KEY_BOOLEAN]; }
    const String& GetErrorString() const  	{ return sErrStr; }

    Date* GetNullDate() const					{ return pNullDate; }
    const String& GetStandardName() const
        {
            if ( bKeywordsNeedInit )
                InitKeywords();
            return sNameStandardFormat;
        }
    short GetStandardPrec() const				{ return nStandardPrec; }
    const Color& GetRedColor() const			{ return StandardColor[4]; }
    Color* GetColor(String& sStr);			// Setzt Hauptfarben oder
                                                // definierte Farben

    // the compatibility currency symbol for old automatic currency formats
    const String& GetCurSymbol() const
        {
            if ( bCompatCurNeedInit )
                InitCompatCur();
            return sCurSymbol;
        }

    // the compatibility currency abbreviation for CCC format code
    const String& GetCurAbbrev() const
        {
            if ( bCompatCurNeedInit )
                InitCompatCur();
            return sCurAbbrev;
        }

    // the compatibility currency symbol upper case for old automatic currency formats
    const String& GetCurString() const
        {
            if ( bCompatCurNeedInit )
                InitCompatCur();
            return sCurString;
        }

    void SetConvertMode(LanguageType eTmpLge, LanguageType eNewLge,
            BOOL bSystemToSystem = FALSE )
    {
        bConvertMode = TRUE;
        eNewLnge = eNewLge;
        eTmpLnge = eTmpLge;
        bConvertSystemToSystem = bSystemToSystem;
    }
    void SetConvertMode(BOOL bMode) { bConvertMode = bMode; }
                                                // Veraendert nur die Bool-Variable
                                                // (zum temporaeren Unterbrechen des
                                                // Convert-Modus)
    BOOL GetConvertMode() const     { return bConvertMode; }
    LanguageType GetNewLnge() const { return eNewLnge; }
                                                // Lesezugriff auf ConvertMode
                                                // und Konvertierungsland/Spr.
    LanguageType GetTmpLnge() const { return eTmpLnge; }
                                                // Lesezugriff auf
                                                // und Ausgangsland/Spr.

                                                /// get Thai T speciality
    BYTE GetNatNumModifier() const      { return nNatNumModifier; }
                                                /// set Thai T speciality
    void SetNatNumModifier( BYTE n )    { nNatNumModifier = n; }

    SvNumberFormatter* GetNumberformatter() { return pFormatter; }
                                                // Zugriff auf Formatierer
                                                // (fuer zformat.cxx)


private:							// ---- privater Teil
    NfKeywordTable sKeyword; 					// Schluesselworte der Syntax
    Color StandardColor[NF_MAX_DEFAULT_COLORS];
                                                // Array der Standardfarben
    Date* pNullDate;							// 30Dec1899
    String sNameStandardFormat;				// "Standard"
    short nStandardPrec;						// default Precision fuer Standardformat (2)
    SvNumberFormatter* pFormatter;				// Pointer auf die Formatliste

    String sStrArray[NF_MAX_FORMAT_SYMBOLS];    // Array der Symbole
    short nTypeArray[NF_MAX_FORMAT_SYMBOLS];    // Array der Infos
                                                // externe Infos:
    USHORT nAnzResStrings;						// Anzahl der Ergebnissymbole
#if !(defined SOLARIS && defined X86)
    short eScannedType;							// Typ gemaess Scan
#else
    int eScannedType;							// wg. Optimierung
#endif
    BOOL bThousand;								// Mit Tausenderpunkt
    USHORT nThousand;							// Zaehlt ....-Folgen
    USHORT nCntPre;								// Zaehlt Vorkommastellen
    USHORT nCntPost;							// Zaehlt Nachkommastellen
    USHORT nCntExp;								// Zaehlt Exp.Stellen, AM/PM
                                                // interne Infos:
    USHORT nAnzStrings;							// Anzahl der Symbole
    USHORT nRepPos;								// Position eines '*'
    USHORT nExpPos;								// interne Position des E
    USHORT nBlankPos;							// interne Position des Blank
    short nDecPos;								// interne Pos. des ,
    BOOL bExp;									// wird bei Lesen des E gesetzt
    BOOL bFrac;									// wird bei Lesen des / gesetzt
    BOOL bBlank;								// wird bei ' '(Fraction) ges.
    BOOL bDecSep;								// Wird beim ersten , gesetzt
    mutable BOOL bKeywordsNeedInit;             // Locale dependent keywords need to be initialized
    mutable BOOL bCompatCurNeedInit;            // Locale dependent compatibility currency need to be initialized
    String sCurSymbol;                          // Currency symbol for compatibility format codes
    String sCurString;                          // Currency symbol in upper case
    String sCurAbbrev;                          // Currency abbreviation
    String sErrStr;                             // String fuer Fehlerausgaben

    BOOL bConvertMode;							// Wird im Convert-Mode gesetzt
                                                // Land/Sprache, in die der
    LanguageType eNewLnge;						// gescannte String konvertiert
                                                // wird (fuer Excel Filter)
                                                // Land/Sprache, aus der der
    LanguageType eTmpLnge;						// gescannte String konvertiert
                                                // wird (fuer Excel Filter)
    BOOL bConvertSystemToSystem;				// Whether the conversion is
                                                // from one system locale to
                                                // another system locale (in
                                                // this case the automatic
                                                // currency symbol is converted
                                                // too).

    xub_StrLen nCurrPos;						// Position des Waehrungssymbols

    BYTE nNatNumModifier;                       // Thai T speciality

    void InitKeywords() const;
    void InitSpecialKeyword( NfKeywordIndex eIdx ) const;
    void InitCompatCur() const;

#ifdef _ZFORSCAN_CXX				// ----- private Methoden -----
    void SetDependentKeywords();
                                                // Setzt die Sprachabh. Keyw.
    void SkipStrings(USHORT& i,xub_StrLen& nPos);// Ueberspringt StringSymbole
    USHORT PreviousKeyword(USHORT i);			// Gibt Index des vorangeh.
                                                // Schluesselworts oder 0
    USHORT NextKeyword(USHORT i);				// Gibt Index des naechsten
                                                // Schluesselworts oder 0
    sal_Unicode PreviousChar(USHORT i);				// Gibt letzten Buchstaben
                                                // vor der Position,
                                                // skipt EMPTY, STRING, STAR, BLANK
    sal_Unicode NextChar(USHORT i);					// Gibt ersten Buchst. danach
    short PreviousType( USHORT i );				// Gibt Typ vor Position,
                                                // skipt EMPTY
    BOOL IsLastBlankBeforeFrac(USHORT i);		// True <=> es kommt kein ' '
                                                // mehr bis zum '/'
    void Reset();								// Reset aller Variablen
                                                // vor Analysestart
    short GetKeyWord( const String& sSymbol,	// determine keyword at nPos
        xub_StrLen nPos );                      // return 0 <=> not found

    inline BOOL IsAmbiguousE( short nKey )		// whether nKey is ambiguous E of NF_KEY_E/NF_KEY_EC
        {
            return (nKey == NF_KEY_EC || nKey == NF_KEY_E) &&
                (GetKeywords()[NF_KEY_EC] == GetKeywords()[NF_KEY_E]);
        }

    // if 0 at strArray[i] is of S,00 or SS,00 or SS"any"00 in ScanType() or FinalScan()
    BOOL Is100SecZero( USHORT i, BOOL bHadDecSep );

    short Next_Symbol(const String& rStr,
                        xub_StrLen& nPos,
                      String& sSymbol);       // Naechstes Symbol
    xub_StrLen Symbol_Division(const String& rString);// lexikalische Voranalyse
    xub_StrLen ScanType(const String& rString);	// Analyse des Formattyps
    xub_StrLen FinalScan( String& rString, String& rComment );	// Endanalyse mit Vorgabe
                                                // des Typs
    // -1:= error, return nPos in FinalScan; 0:= no calendar, 1:= calendar found
    int FinalScanGetCalendar( xub_StrLen& nPos, USHORT& i, USHORT& nAnzResStrings );

    static inline BOOL StringEqualsChar( const String& rStr, sal_Unicode ch )
        { return rStr.GetChar(0) == ch && rStr.Len() == 1; }
        // Yes, for efficiency get the character first and then compare length
        // because in most places where this is used the string is one char.

    // remove "..." and \... quotes from rStr, return how many chars removed
    static xub_StrLen RemoveQuotes( String& rStr );

#endif //_ZFORSCAN_CXX
};

}

#endif	// _ZFORSCAN_HXX
