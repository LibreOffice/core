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
#ifndef _ZFORMAT_HXX
#define _ZFORMAT_HXX

#include "svl/svldllapi.h"
#include <tools/string.hxx>
#include <i18npool/mslangid.hxx>
#include <svl/zforlist.hxx>
#include <svl/nfversi.hxx>
#include <svl/nfkeytab.hxx>

// We need ImpSvNumberformatScan for the private SvNumberformat definitions.
#ifdef _ZFORMAT_CXX
#include "zforscan.hxx"
#endif

// If comment field is also in format code string, was used for SUPD versions 371-372
#define NF_COMMENT_IN_FORMATSTRING 0

namespace utl {
    class DigitGroupingIterator;
}

class SvStream;
class Color;

class ImpSvNumberformatScan;            // format code string scanner
class ImpSvNumberInputScan;             // input string scanner
class ImpSvNumMultipleWriteHeader;      // compatible file format
class ImpSvNumMultipleReadHeader;       // compatible file format
class SvNumberFormatter;

enum SvNumberformatLimitOps
{
    NUMBERFORMAT_OP_NO  = 0,            // Undefined, no OP
    NUMBERFORMAT_OP_EQ  = 1,            // Operator =
    NUMBERFORMAT_OP_NE  = 2,            // Operator <>
    NUMBERFORMAT_OP_LT  = 3,            // Operator <
    NUMBERFORMAT_OP_LE  = 4,            // Operator <=
    NUMBERFORMAT_OP_GT  = 5,            // Operator >
    NUMBERFORMAT_OP_GE  = 6             // Operator >=
};

// SYSTEM-german to SYSTEM-xxx and vice versa conversion hack onLoad
enum NfHackConversion
{
    NF_CONVERT_NONE,
    NF_CONVERT_GERMAN_ENGLISH,
    NF_CONVERT_ENGLISH_GERMAN
};

struct ImpSvNumberformatInfo            // Struct for FormatInfo
{
    String* sStrArray;                  // Array of symbols
    short* nTypeArray;                  // Array of infos
    sal_uInt16 nThousand;                   // Count of group separator sequences
    sal_uInt16 nCntPre;                     // Count of digits before decimal point
    sal_uInt16 nCntPost;                    // Count of digits after decimal point
    sal_uInt16 nCntExp;                     // Count of exponent digits, or AM/PM
    short eScannedType;                 // Type determined by scan
    sal_Bool bThousand;                     // Has group (AKA thousand) separator

    void Copy( const ImpSvNumberformatInfo& rNumFor, sal_uInt16 nAnz );
    void Load(SvStream& rStream, sal_uInt16 nAnz);
    void Save(SvStream& rStream, sal_uInt16 nAnz) const;
};

// NativeNumber, represent numbers using CJK or other digits if nNum>0,
// eLang specifies the Locale to use.
class SvNumberNatNum
{
    LanguageType    eLang;
    sal_uInt8            nNum;
    sal_Bool            bDBNum  :1;     // DBNum, to be converted to NatNum
    sal_Bool            bDate   :1;     // Used in date? (needed for DBNum/NatNum mapping)
    sal_Bool            bSet    :1;     // If set, since NatNum0 is possible

public:

    static  sal_uInt8    MapDBNumToNatNum( sal_uInt8 nDBNum, LanguageType eLang, sal_Bool bDate );
    static  sal_uInt8    MapNatNumToDBNum( sal_uInt8 nNatNum, LanguageType eLang, sal_Bool bDate );

                    SvNumberNatNum() : eLang( LANGUAGE_DONTKNOW ), nNum(0),
                                        bDBNum(0), bDate(0), bSet(0) {}
    sal_Bool            IsComplete() const  { return bSet && eLang != LANGUAGE_DONTKNOW; }
    sal_uInt8            GetRawNum() const   { return nNum; }
    sal_uInt8            GetNatNum() const   { return bDBNum ? MapDBNumToNatNum( nNum, eLang, bDate ) : nNum; }
    sal_uInt8            GetDBNum() const    { return bDBNum ? nNum : MapNatNumToDBNum( nNum, eLang, bDate ); }
    LanguageType    GetLang() const     { return eLang; }
    void            SetLang( LanguageType e ) { eLang = e; }
    void            SetNum( sal_uInt8 nNumber, sal_Bool bDBNumber )
                        {
                            nNum = nNumber;
                            bDBNum = bDBNumber;
                            bSet = sal_True;
                        }
    sal_Bool            IsSet() const       { return bSet; }
    void            SetDate( sal_Bool bDateP )   { bDate = (bDateP != 0); }
};

class CharClass;

class ImpSvNumFor                       // One of four subformats of the format code string
{
public:
    ImpSvNumFor();                      // Ctor without filling the Info
    ~ImpSvNumFor();

    void Enlarge(sal_uInt16 nAnz);          // Init of arrays to the right size
    void Load( SvStream& rStream, ImpSvNumberformatScan& rSc,
                String& rLoadedColorName);
    void Save( SvStream& rStream ) const;

    // if pSc is set, it is used to get the Color pointer
    void Copy( const ImpSvNumFor& rNumFor, ImpSvNumberformatScan* pSc );

    // Access to Info; call Enlarge before!
    ImpSvNumberformatInfo& Info() { return aI;}
    const ImpSvNumberformatInfo& Info() const { return aI; }

    // Get count of substrings (symbols)
    sal_uInt16 GetCount() const { return nAnzStrings;}

    Color* GetColor() const { return pColor; }
    void SetColor( Color* pCol, String& rName )
     { pColor = pCol; sColorName = rName; }
    const String& GetColorName() const { return sColorName; }

    // new SYMBOLTYPE_CURRENCY in subformat?
    sal_Bool HasNewCurrency() const;
    sal_Bool GetNewCurrencySymbol( String& rSymbol, String& rExtension ) const;
    void SaveNewCurrencyMap( SvStream& rStream ) const;
    void LoadNewCurrencyMap( SvStream& rStream );

    // [NatNum1], [NatNum2], ...
    void SetNatNumNum( sal_uInt8 nNum, sal_Bool bDBNum ) { aNatNum.SetNum( nNum, bDBNum ); }
    void SetNatNumLang( LanguageType eLang ) { aNatNum.SetLang( eLang ); }
    void SetNatNumDate( sal_Bool bDate ) { aNatNum.SetDate( bDate ); }
    const SvNumberNatNum& GetNatNum() const { return aNatNum; }

private:
    ImpSvNumberformatInfo aI;           // Hilfsstruct fuer die restlichen Infos
    String sColorName;                  // color name
    Color* pColor;                      // pointer to color of subformat
    sal_uInt16 nAnzStrings;                 // count of symbols
    SvNumberNatNum aNatNum;             // DoubleByteNumber

};

class SVL_DLLPUBLIC SvNumberformat
{
    struct LocaleType
    {
        sal_uInt8 mnNumeralShape;
        sal_uInt8 mnCalendarType;
        LanguageType meLanguage;

        ::rtl::OUString generateCode() const;

        LocaleType();
        LocaleType(sal_uInt32 nRawCode);
    };

public:
    // Ctor for Load
    SvNumberformat( ImpSvNumberformatScan& rSc, LanguageType eLge );

    // Normal ctor
    SvNumberformat( String& rString,
                   ImpSvNumberformatScan* pSc,
                   ImpSvNumberInputScan* pISc,
                   xub_StrLen& nCheckPos,
                   LanguageType& eLan,
                   sal_Bool bStand = sal_False );

    // Copy ctor
    SvNumberformat( SvNumberformat& rFormat );

    // Copy ctor with exchange of format code string scanner (used in merge)
    SvNumberformat( SvNumberformat& rFormat, ImpSvNumberformatScan& rSc );

    ~SvNumberformat();

    /// Get type of format, may include NUMBERFORMAT_DEFINED bit
    short GetType() const
        { return (nNewStandardDefined &&
            (nNewStandardDefined <= SV_NUMBERFORMATTER_VERSION)) ?
            (eType & ~NUMBERFORMAT_DEFINED) : eType; }

    void SetType(const short eSetType)          { eType = eSetType; }
    // Standard means the I18N defined standard format of this type
    void SetStandard()                          { bStandard = sal_True; }
    sal_Bool IsStandard() const                     { return bStandard; }

    // For versions before version nVer it is UserDefined, for newer versions
    // it is builtin. nVer of SV_NUMBERFORMATTER_VERSION_...
    void SetNewStandardDefined( sal_uInt16 nVer )
        { nNewStandardDefined = nVer; eType |= NUMBERFORMAT_DEFINED; }

    sal_uInt16 GetNewStandardDefined() const        { return nNewStandardDefined; }
    sal_Bool IsAdditionalStandardDefined() const
        { return nNewStandardDefined == SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS; }

    LanguageType GetLanguage() const            { return maLocale.meLanguage;}

    const String& GetFormatstring() const   { return sFormatstring; }

    // Build a format string of application defined keywords
    String GetMappedFormatstring( const NfKeywordTable& rKeywords,
                                    const LocaleDataWrapper& rLoc,
                                    sal_Bool bDontQuote = sal_False ) const;

    void SetUsed(const sal_Bool b)                  { bIsUsed = b; }
    sal_Bool GetUsed() const                        { return bIsUsed; }
    sal_Bool IsStarFormatSupported() const          { return bStarFlag; }
    void SetStarFormatSupport( sal_Bool b )         { bStarFlag = b; }

    NfHackConversion Load( SvStream& rStream, ImpSvNumMultipleReadHeader& rHdr,
        SvNumberFormatter* pConverter, ImpSvNumberInputScan& rISc );
    void Save( SvStream& rStream, ImpSvNumMultipleWriteHeader& rHdr  ) const;

    // Load a string which might contain an Euro symbol,
    // in fact that could be any string used in number formats.
    static void LoadString( SvStream& rStream, String& rStr );

    /**
     * Get output string from a numeric value that fits the number of
     * characters specified.
     */
    bool GetOutputString( double fNumber, sal_uInt16 nCharCount, String& rOutString ) const;

    sal_Bool GetOutputString( double fNumber, String& OutString, Color** ppColor );
    sal_Bool GetOutputString( String& sString, String& OutString, Color** ppColor );

    // True if type text
    sal_Bool IsTextFormat() const { return (eType & NUMBERFORMAT_TEXT) != 0; }
    // True if 4th subformat present
    sal_Bool HasTextFormat() const
        {
            return (NumFor[3].GetCount() > 0) ||
                (NumFor[3].Info().eScannedType == NUMBERFORMAT_TEXT);
        }

    void GetFormatSpecialInfo(sal_Bool& bThousand,
                              sal_Bool& IsRed,
                              sal_uInt16& nPrecision,
                              sal_uInt16& nAnzLeading) const;

    /// Count of decimal precision
    sal_uInt16 GetFormatPrecision() const   { return NumFor[0].Info().nCntPost; }

    //! Read/write access on a special sal_uInt16 component, may only be used on the
    //! standard format 0, 5000, ... and only by the number formatter!
    sal_uInt16 GetLastInsertKey() const
        { return NumFor[0].Info().nThousand; }
    void SetLastInsertKey(sal_uInt16 nKey)
        { NumFor[0].Info().nThousand = nKey; }

    //! Only onLoad: convert from stored to current system language/country
    void ConvertLanguage( SvNumberFormatter& rConverter,
        LanguageType eConvertFrom, LanguageType eConvertTo, sal_Bool bSystem = sal_False );

    // Substring of a subformat code nNumFor (0..3)
    // nPos == 0xFFFF => last substring
    // bString==sal_True: first/last SYMBOLTYPE_STRING or SYMBOLTYPE_CURRENCY
    const String* GetNumForString( sal_uInt16 nNumFor, sal_uInt16 nPos,
            sal_Bool bString = sal_False ) const;

    // Subtype of a subformat code nNumFor (0..3)
    // nPos == 0xFFFF => last substring
    // bString==sal_True: first/last SYMBOLTYPE_STRING or SYMBOLTYPE_CURRENCY
    short GetNumForType( sal_uInt16 nNumFor, sal_uInt16 nPos, sal_Bool bString = sal_False ) const;

    /** If the count of string elements (substrings, ignoring [modifiers] and
        so on) in a subformat code nNumFor (0..3) is equal to the given number.
        Used by ImpSvNumberInputScan::IsNumberFormatMain() to detect a matched
        format.  */
    sal_Bool IsNumForStringElementCountEqual( sal_uInt16 nNumFor, sal_uInt16 nAllCount,
            sal_uInt16 nNumCount ) const
        {
            if ( nNumFor < 4 )
            {
                // First try a simple approach. Note that this is called only
                // if all MidStrings did match so far, to verify that all
                // strings of the format were matched and not just the starting
                // sequence, so we don't have to check if GetnAnz() includes
                // [modifiers] or anything else if both counts are equal.
                sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
                if ( nAllCount == nCnt )
                    return sal_True;
                if ( nAllCount < nCnt ) // check ignoring [modifiers] and so on
                    return ImpGetNumForStringElementCount( nNumFor ) ==
                        (nAllCount - nNumCount);
            }
            return sal_False;
        }

    // Whether the second subformat code is really for negative numbers
    // or another limit set.
    sal_Bool IsNegativeRealNegative() const
        {
            return fLimit1 == 0.0 && fLimit2 == 0.0 &&
            ( (eOp1 == NUMBERFORMAT_OP_GE && eOp2 == NUMBERFORMAT_OP_NO) ||
              (eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_LT) ||
              (eOp1 == NUMBERFORMAT_OP_NO && eOp2 == NUMBERFORMAT_OP_NO) );
        }

    // Whether the negative format is without a sign or not
    sal_Bool IsNegativeWithoutSign() const;

    sal_Bool IsNegativeInBracket() const;

    sal_Bool HasPositiveBracketPlaceholder() const;

    // Whether a new SYMBOLTYPE_CURRENCY is contained in the format
    sal_Bool HasNewCurrency() const;

    // Build string from NewCurrency for saving it SO50 compatible
    void Build50Formatstring( String& rStr ) const;

    // strip [$-yyy] from all [$xxx-yyy] leaving only xxx's,
    // if bQuoteSymbol==sal_True the xxx will become "xxx"
    static String StripNewCurrencyDelimiters( const String& rStr,
        sal_Bool bQuoteSymbol );

    // If a new SYMBOLTYPE_CURRENCY is contained if the format is of type
    // NUMBERFORMAT_CURRENCY, and if so the symbol xxx and the extension nnn
    // of [$xxx-nnn] are returned
    sal_Bool GetNewCurrencySymbol( String& rSymbol, String& rExtension ) const;

    static sal_Bool HasStringNegativeSign( const String& rStr );

    /**
        Whether a character at position nPos is somewhere between two matching
        cQuote or not.
        If nPos points to a cQuote, a sal_True is returned on an opening cQuote,
        a sal_False is returned on a closing cQuote.
        A cQuote between quotes may be escaped by a cEscIn, a cQuote outside of
        quotes may be escaped by a cEscOut.
        The default '\0' results in no escapement possible.
        Defaults are set right according to the "unlogic" of the Numberformatter
     */
    static sal_Bool IsInQuote( const String& rString, xub_StrLen nPos,
            sal_Unicode cQuote = '"',
            sal_Unicode cEscIn = '\0', sal_Unicode cEscOut = '\\' );

    /**
        Return the position of a matching closing cQuote if the character at
        position nPos is between two matching cQuote, otherwise return
        STRING_NOTFOUND.
        If nPos points to an opening cQuote the position of the matching
        closing cQuote is returned.
        If nPos points to a closing cQuote nPos is returned.
        If nPos points into a part which starts with an opening cQuote but has
        no closing cQuote, rString.Len() is returned.
        Uses <method>IsInQuote</method> internally, so you don't have to call
        that prior to a call of this method.
     */
    static xub_StrLen GetQuoteEnd( const String& rString, xub_StrLen nPos,
                sal_Unicode cQuote = '"',
                sal_Unicode cEscIn = '\0', sal_Unicode cEscOut = '\\' );

    void SetComment( const String& rStr )
#if NF_COMMENT_IN_FORMATSTRING
        { SetComment( rStr, sFormatstring, sComment ); }
#else
        { sComment = rStr; }
#endif
    const String& GetComment() const { return sComment; }

    // Erase "{ "..." }" from format subcode string to get the pure comment (old version)
    static void EraseCommentBraces( String& rStr );
    // Set comment rStr in format string rFormat and in rComment (old version)
    static void SetComment( const String& rStr, String& rFormat, String& rComment );
    // Erase comment at end of rStr to get pure format code string (old version)
    static void EraseComment( String& rStr );

    /** Insert the number of blanks into the string that is needed to simulate
        the width of character c for underscore formats */
    static xub_StrLen InsertBlanks( String& r, xub_StrLen nPos, sal_Unicode c );

    /// One of YMD,DMY,MDY if date format
    DateFormat GetDateOrder() const;

    /** A coded value of the exact YMD combination used, if date format.
        For example: YYYY-MM-DD => ('Y' << 16) | ('M' << 8) | 'D'
        or: MM/YY => ('M' << 8) | 'Y'  */
    sal_uInt32 GetExactDateOrder() const;

    ImpSvNumberformatScan& ImpGetScan() const { return rScan; }

    // used in XML export
    void GetConditions( SvNumberformatLimitOps& rOper1, double& rVal1,
                        SvNumberformatLimitOps& rOper2, double& rVal2 ) const;
    Color* GetColor( sal_uInt16 nNumFor ) const;
    void GetNumForInfo( sal_uInt16 nNumFor, short& rScannedType,
                    sal_Bool& bThousand, sal_uInt16& nPrecision, sal_uInt16& nAnzLeading ) const;

    // rAttr.Number not empty if NatNum attributes are to be stored
    void GetNatNumXml(
            ::com::sun::star::i18n::NativeNumberXmlAttributes& rAttr,
            sal_uInt16 nNumFor ) const;

    /** @returns <TRUE/> if E,EE,R,RR,AAA,AAAA in format code of subformat
        nNumFor (0..3) and <b>no</b> preceding calendar was specified and the
        currently loaded calendar is "gregorian". */
    sal_Bool IsOtherCalendar( sal_uInt16 nNumFor ) const
        {
            if ( nNumFor < 4 )
                return ImpIsOtherCalendar( NumFor[nNumFor] );
            return sal_False;
        }

    /** Switches to the first non-"gregorian" calendar, but only if the current
        calendar is "gregorian"; original calendar name and date/time returned,
        but only if calendar switched and rOrgCalendar was empty. */
    void SwitchToOtherCalendar( String& rOrgCalendar, double& fOrgDateTime ) const;

    /** Switches to the "gregorian" calendar, but only if the current calendar
        is non-"gregorian" and rOrgCalendar is not empty. Thus a preceding
        ImpSwitchToOtherCalendar() call should have been placed prior to
        calling this method. */
    void SwitchToGregorianCalendar( const String& rOrgCalendar, double fOrgDateTime ) const;

    /** Switches to the first specified calendar, if any, in subformat nNumFor
        (0..3). Original calendar name and date/time returned, but only if
        calendar switched and rOrgCalendar was empty.

        @return
            <TRUE/> if a calendar was specified and switched to,
            <FALSE/> else.
     */
    sal_Bool SwitchToSpecifiedCalendar( String& rOrgCalendar, double& fOrgDateTime,
            sal_uInt16 nNumFor ) const
        {
            if ( nNumFor < 4 )
                return ImpSwitchToSpecifiedCalendar( rOrgCalendar,
                        fOrgDateTime, NumFor[nNumFor] );
            return sal_False;
        }

private:
    ImpSvNumFor NumFor[4];          // Array for the 4 subformats
    String sFormatstring;           // The format code string
    String sComment;                // Comment, since number formatter version 6
    double fLimit1;                 // Value for first condition
    double fLimit2;                 // Value for second condition
    ImpSvNumberformatScan& rScan;   // Format code scanner
    LocaleType maLocale;            // Language/country of the format, numeral shape and calendar type from Excel.
    SvNumberformatLimitOps eOp1;    // Operator for first condition
    SvNumberformatLimitOps eOp2;    // Operator for second condition
    sal_uInt16 nNewStandardDefined;     // new builtin formats as of version 6
    short eType;                    // Type of format
    sal_Bool bStarFlag;                 // Take *n format as ESC n
    sal_Bool bStandard;                 // If this is a default standard format
    sal_Bool bIsUsed;                   // Flag as used for storing

    SVL_DLLPRIVATE sal_uInt16 ImpGetNumForStringElementCount( sal_uInt16 nNumFor ) const;

    SVL_DLLPRIVATE sal_Bool ImpIsOtherCalendar( const ImpSvNumFor& rNumFor ) const;

    SVL_DLLPRIVATE sal_Bool ImpSwitchToSpecifiedCalendar( String& rOrgCalendar,
            double& fOrgDateTime, const ImpSvNumFor& rNumFor ) const;

#ifdef _ZFORMAT_CXX     // ----- private implementation methods -----

    const CharClass& rChrCls() const        { return rScan.GetChrCls(); }
    const LocaleDataWrapper& rLoc() const   { return rScan.GetLoc(); }
    CalendarWrapper& GetCal() const         { return rScan.GetCal(); }
    const SvNumberFormatter& GetFormatter() const   { return *rScan.GetNumberformatter(); }

    // divide in substrings and color conditions
    SVL_DLLPRIVATE short ImpNextSymbol( String& rString,
                     xub_StrLen& nPos,
                     String& sSymbol );

    // read string until ']' and strip blanks (after condition)
    SVL_DLLPRIVATE static xub_StrLen ImpGetNumber( String& rString,
                   xub_StrLen& nPos,
                   String& sSymbol );

    /**
     * Parse the content of '[$-xxx] or '[$-xxxxxxxx]' and extract the locale
     * type from it.  Given the string, start parsing at position specified by
     * nPos, and store the end position with nPos when the parsing is
     * complete.  The nPos should point to the '$' before the parsing, and to
     * the closing bracket after the parsing.  When the content is [$-xxx],
     * the xxx part represents the language type (aka LCID) in hex numerals.
     * When the content is [$-xxxxxxxx] the last 4 digits represent the LCID
     * (again in hex), the next 2 digits represent the calendar type, and the
     * 2 highest digits (if exists) is the numeral shape.
     *
     * @reference
     * http://office.microsoft.com/en-us/excel-help/creating-international-number-formats-HA001034635.aspx
     *
     * @param rString input string
     * @param nPos position (see above).
     *
     * @return struct containing numeral shape, calendar type, and LCID that
     *         specifies language type. See i18npool/lang.h for a complete
     *         list of language types. These numbers also correspond with the
     *         numbers used by Microsoft Office.
     */
    SVL_DLLPRIVATE static LocaleType ImpGetLocaleType( const String& rString, xub_StrLen& nPos );

    // standard number output
    SVL_DLLPRIVATE void ImpGetOutputStandard( double& fNumber, String& OutString );
    SVL_DLLPRIVATE void ImpGetOutputStdToPrecision( double& rNumber, String& rOutString, sal_uInt16 nPrecision ) const;
    // numbers in input line
    SVL_DLLPRIVATE void ImpGetOutputInputLine( double fNumber, String& OutString );

    // check subcondition
    // OP undefined => -1
    // else 0 or 1
    SVL_DLLPRIVATE short ImpCheckCondition(double& fNumber,
                         double& fLimit,
                         SvNumberformatLimitOps eOp);

    SVL_DLLPRIVATE sal_uLong ImpGGT(sal_uLong x, sal_uLong y);
    SVL_DLLPRIVATE sal_uLong ImpGGTRound(sal_uLong x, sal_uLong y);

    // Helper function for number strings
    // append string symbols, insert leading 0 or ' ', or ...
    SVL_DLLPRIVATE sal_Bool ImpNumberFill( String& sStr,
                    double& rNumber,
                    xub_StrLen& k,
                    sal_uInt16& j,
                    sal_uInt16 nIx,
                    short eSymbolType );

    // Helper function to fill in the integer part and the group (AKA thousand) separators
    SVL_DLLPRIVATE sal_Bool ImpNumberFillWithThousands( String& sStr,
                                 double& rNumber,
                                 xub_StrLen k,
                                 sal_uInt16 j,
                                 sal_uInt16 nIx,
                                 sal_uInt16 nDigCnt );
                                    // Hilfsfunktion zum Auffuellen der Vor-
                                    // kommazahl auch mit Tausenderpunkt

    // Helper function to fill in the group (AKA thousand) separators
    // or to skip additional digits
    SVL_DLLPRIVATE void ImpDigitFill( String& sStr,
                    xub_StrLen nStart,
                    xub_StrLen& k,
                    sal_uInt16 nIx,
                    xub_StrLen & nDigitCount,
                    utl::DigitGroupingIterator & );

    SVL_DLLPRIVATE sal_Bool ImpGetDateOutput( double fNumber,
                       sal_uInt16 nIx,
                       String& OutString );
    SVL_DLLPRIVATE sal_Bool ImpGetTimeOutput( double fNumber,
                       sal_uInt16 nIx,
                       String& OutString );
    SVL_DLLPRIVATE sal_Bool ImpGetDateTimeOutput( double fNumber,
                           sal_uInt16 nIx,
                           String& OutString );

    // Switches to the "gregorian" calendar if the current calendar is
    // non-"gregorian" and the era is a "Dummy" era of a calendar which doesn't
    // know a "before" era (like zh_TW ROC or ja_JP Gengou). If switched and
    // rOrgCalendar was "gregorian" the string is emptied. If rOrgCalendar was
    // empty the previous calendar name and date/time are returned.
    SVL_DLLPRIVATE sal_Bool ImpFallBackToGregorianCalendar( String& rOrgCalendar, double& fOrgDateTime );

    // Append a "G" short era string of the given calendar. In the case of a
    // Gengou calendar this is a one character abbreviation, for other
    // calendars the XExtendedCalendar::getDisplayString() method is called.
    SVL_DLLPRIVATE static void ImpAppendEraG( String& OutString, const CalendarWrapper& rCal,
            sal_Int16 nNatNum );

    SVL_DLLPRIVATE sal_Bool ImpGetNumberOutput( double fNumber,
                         sal_uInt16 nIx,
                         String& OutString );

    SVL_DLLPRIVATE void ImpCopyNumberformat( const SvNumberformat& rFormat );

    // normal digits or other digits, depending on ImpSvNumFor.aNatNum,
    // [NatNum1], [NatNum2], ...
    SVL_DLLPRIVATE String ImpGetNatNumString( const SvNumberNatNum& rNum, sal_Int32 nVal,
            sal_uInt16 nMinDigits = 0  ) const;

    String ImpIntToString( sal_uInt16 nIx, sal_Int32 nVal, sal_uInt16 nMinDigits = 0 ) const
        {
            const SvNumberNatNum& rNum = NumFor[nIx].GetNatNum();
            if ( nMinDigits || rNum.IsComplete() )
                return ImpGetNatNumString( rNum, nVal, nMinDigits );
            return String::CreateFromInt32( nVal );
        }

    // transliterate according to NativeNumber
    SVL_DLLPRIVATE void ImpTransliterateImpl( String& rStr, const SvNumberNatNum& rNum ) const;

    void ImpTransliterate( String& rStr, const SvNumberNatNum& rNum ) const
        {
            if ( rNum.IsComplete() )
                ImpTransliterateImpl( rStr, rNum );
        }

#endif // _ZFORMAT_CXX

};

#endif  // _ZFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
