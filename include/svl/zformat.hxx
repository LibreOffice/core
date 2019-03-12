/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVL_ZFORMAT_HXX
#define INCLUDED_SVL_ZFORMAT_HXX

#include <svl/svldllapi.h>
#include <svl/zforlist.hxx>
#include <svl/nfkeytab.hxx>
#include <vector>

namespace utl {
    class DigitGroupingIterator;
}

namespace com { namespace sun { namespace star { namespace i18n { struct NativeNumberXmlAttributes2; } } } }

class Color;

class ImpSvNumberformatScan;            // format code string scanner
class ImpSvNumberInputScan;             // input string scanner

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

struct ImpSvNumberformatInfo            // Struct for FormatInfo
{
    std::vector<OUString> sStrArray;    // Array of symbols
    std::vector<short> nTypeArray;      // Array of infos
    sal_uInt16 nThousand;               // Count of group separator sequences
    sal_uInt16 nCntPre;                 // Count of digits before decimal point
    sal_uInt16 nCntPost;                // Count of digits after decimal point
    sal_uInt16 nCntExp;                 // Count of exponent digits, or AM/PM
    SvNumFormatType eScannedType;       // Type determined by scan
    bool bThousand;                     // Has group (AKA thousand) separator

    void Copy( const ImpSvNumberformatInfo& rNumFor, sal_uInt16 nCount );
};

// NativeNumber, represent numbers using CJK or other digits if nNum>0,
// eLang specifies the Locale to use.
class SvNumberNatNum
{
    OUString sParams;               // For [NatNum12 ordinal-number]-like syntax
    LanguageType eLang;
    sal_uInt8            nNum;
    bool            bDBNum  :1;     // DBNum, to be converted to NatNum
    bool            bDate   :1;     // Used in date? (needed for DBNum/NatNum mapping)
    bool            bSet    :1;     // If set, since NatNum0 is possible

public:

    static  sal_uInt8    MapDBNumToNatNum( sal_uInt8 nDBNum, LanguageType eLang, bool bDate );
    static  sal_uInt8    MapNatNumToDBNum( sal_uInt8 nNatNum, LanguageType eLang, bool bDate );

                    SvNumberNatNum() : eLang( LANGUAGE_DONTKNOW ), nNum(0),
                                        bDBNum(false), bDate(false), bSet(false) {}
    bool            IsComplete() const  { return bSet && eLang != LANGUAGE_DONTKNOW; }
    sal_uInt8       GetNatNum() const   { return bDBNum ? MapDBNumToNatNum( nNum, eLang, bDate ) : nNum; }
    sal_uInt8       GetDBNum() const    { return bDBNum ? nNum : MapNatNumToDBNum( nNum, eLang, bDate ); }
    LanguageType    GetLang() const     { return eLang; }
    void            SetLang( LanguageType e ) { eLang = e; }
    void            SetNum( sal_uInt8 nNumber, bool bDBNumber )
                        {
                            nNum = nNumber;
                            bDBNum = bDBNumber;
                            bSet = true;
                        }
    bool            IsSet() const       { return bSet; }
    void            SetDate( bool bDateP )   { bDate = bDateP; }
    void            SetParams(const OUString& s) { sParams = s; }
    OUString const & GetParams() const { return sParams; }
};

class CharClass;

class ImpSvNumFor                       // One of four subformats of the format code string
{
public:
    ImpSvNumFor();                      // Ctor without filling the Info
    ~ImpSvNumFor();

    void Enlarge(sal_uInt16 nCount);    // Init of arrays to the right size

    // if pSc is set, it is used to get the Color pointer
    void Copy( const ImpSvNumFor& rNumFor, ImpSvNumberformatScan* pSc );

    // Access to Info; call Enlarge before!
    ImpSvNumberformatInfo& Info() { return aI;}
    const ImpSvNumberformatInfo& Info() const { return aI; }

    // Get count of substrings (symbols)
    sal_uInt16 GetCount() const { return nStringsCnt;}

    Color* GetColor() const { return pColor; }
    void SetColor( Color* pCol, OUString const & rName )
     { pColor = pCol; sColorName = rName; }
    const OUString& GetColorName() const { return sColorName; }

    // new SYMBOLTYPE_CURRENCY in subformat?
    bool HasNewCurrency() const;
    bool GetNewCurrencySymbol( OUString& rSymbol, OUString& rExtension ) const;

    // [NatNum1], [NatNum2], ...
    void SetNatNumNum( sal_uInt8 nNum, bool bDBNum ) { aNatNum.SetNum( nNum, bDBNum ); }
    void SetNatNumLang( LanguageType eLang ) { aNatNum.SetLang( eLang ); }
    void SetNatNumDate( bool bDate ) { aNatNum.SetDate( bDate ); }
    void SetNatNumParams(const OUString& sParams) { aNatNum.SetParams(sParams); }
    const SvNumberNatNum& GetNatNum() const { return aNatNum; }

private:
    ImpSvNumberformatInfo aI;           // helper struct for remaining information
    OUString sColorName;                // color name
    Color* pColor;                      // pointer to color of subformat
    sal_uInt16 nStringsCnt;             // count of symbols
    SvNumberNatNum aNatNum;             // DoubleByteNumber

};

class SVL_DLLPUBLIC SvNumberformat
{
    struct LocaleType
    {
        enum class Substitute : sal_uInt8
        {
            NONE,
            TIME,
            LONGDATE
        };

        LanguageType meLanguage;
        LanguageType meLanguageWithoutLocaleData;
        Substitute meSubstitute;
        sal_uInt8 mnNumeralShape;
        sal_uInt8 mnCalendarType;

        OUString generateCode() const;

        LocaleType();
        LocaleType(sal_uInt32 nRawCode);

        bool isPlainLocale() const;
    };

public:
    // Normal ctor
    SvNumberformat( OUString& rString,
                   ImpSvNumberformatScan* pSc,
                   ImpSvNumberInputScan* pISc,
                   sal_Int32& nCheckPos,
                   LanguageType& eLan );

    // Copy ctor
    SvNumberformat( SvNumberformat const & rFormat );

    // Copy ctor with exchange of format code string scanner (used in merge)
    SvNumberformat( SvNumberformat const & rFormat, ImpSvNumberformatScan& rSc );

    ~SvNumberformat();

    /// Get type of format, may include css::util::NumberFormat::DEFINED bit
    SvNumFormatType GetType() const             { return eType; }

    /// Get type of format, does not include css::util::NumberFormat::DEFINED
    SvNumFormatType GetMaskedType() const       { return eType & ~SvNumFormatType::DEFINED; }

    void SetType(SvNumFormatType eSetType)      { eType = eSetType; }
    // Standard means the I18N defined standard format of this type
    void SetStandard()                          { bStandard = true; }
    bool IsStandard() const                     { return bStandard; }

    // If this format is an additional built-in format defined by i18n.
    void SetAdditionalBuiltin()                 { bAdditionalBuiltin = true; }
    bool IsAdditionalBuiltin() const            { return bAdditionalBuiltin; }

    LanguageType GetLanguage() const            { return maLocale.meLanguage;}

    /** If the format is a placeholder and needs to be substituted. */
    bool IsSubstituted() const
        {
            return maLocale.meSubstitute != LocaleType::Substitute::NONE;
        }

    /** If the format is a placeholder for the system time format and needs to
        be substituted during formatting time.
     */
    bool IsSystemTimeFormat() const
        {
            return maLocale.meSubstitute == LocaleType::Substitute::TIME && maLocale.meLanguage == LANGUAGE_SYSTEM;
        }

    /** If the format is a placeholder for the system long date format and needs
        to be substituted during formatting time.
     */
    bool IsSystemLongDateFormat() const
        {
            return maLocale.meSubstitute == LocaleType::Substitute::LONGDATE && maLocale.meLanguage == LANGUAGE_SYSTEM;
        }

    const OUString& GetFormatstring() const   { return sFormatstring; }

    // Build a format string of application defined keywords
    OUString GetMappedFormatstring( const NfKeywordTable& rKeywords,
                                    const LocaleDataWrapper& rLoc,
                                    LanguageType nOriginalLang = LANGUAGE_DONTKNOW,
                                    bool bSystemLanguage = false ) const;

    void SetStarFormatSupport( bool b )         { bStarFlag = b; }

    /**
     * Get output string from a numeric value that fits the number of
     * characters specified.
     */
    bool GetOutputString( double fNumber, sal_uInt16 nCharCount, OUString& rOutString ) const;

    bool GetOutputString( double fNumber, OUString& OutString, Color** ppColor );
    void GetOutputString( const OUString& sString, OUString& OutString, Color** ppColor );

    // True if type text
    bool IsTextFormat() const { return bool(eType & SvNumFormatType::TEXT); }
    // True if 4th subformat present
    bool HasTextFormat() const
        {
            return (NumFor[3].GetCount() > 0) ||
                (NumFor[3].Info().eScannedType == SvNumFormatType::TEXT);
        }

    void GetFormatSpecialInfo(bool& bThousand,
                              bool& IsRed,
                              sal_uInt16& nPrecision,
                              sal_uInt16& nLeadingCnt) const;

    /// Get index of subformat (0..3) according to conditions and fNumber value
    sal_uInt16 GetSubformatIndex( double fNumber ) const;

    /// Count of decimal precision
    sal_uInt16 GetFormatPrecision( sal_uInt16 nIx = 0 ) const
        { return NumFor[nIx].Info().nCntPost; }

    /// Count of integer digits
    sal_uInt16 GetFormatIntegerDigits( sal_uInt16 nIx = 0 ) const
        { return NumFor[nIx].Info().nCntPre; }

    /** Count of hidden integer digits with thousands divisor:
     * formats like "0," to show only thousands
     */
    sal_uInt16 GetThousandDivisorPrecision( sal_uInt16 nIx = 0 ) const
        { return NumFor[nIx].Info().nThousand * 3; }

    //! Read/write access on a special sal_uInt16 component, may only be used on the
    //! standard format 0, 10000, ... and only by the number formatter!
    struct FormatterPrivateAccess { friend SvNumberFormatter; private: FormatterPrivateAccess() {} };
    sal_uInt16 GetLastInsertKey( const FormatterPrivateAccess& ) const
        { return NumFor[0].Info().nThousand; }
    void SetLastInsertKey( sal_uInt16 nKey, const FormatterPrivateAccess& )
        { NumFor[0].Info().nThousand = nKey; }

    //! Only onLoad: convert from stored to current system language/country
    void ConvertLanguage( SvNumberFormatter& rConverter,
        LanguageType eConvertFrom, LanguageType eConvertTo );

    // Substring of a subformat code nNumFor (0..3)
    // nPos == 0xFFFF => last substring
    // bString==true: first/last SYMBOLTYPE_STRING or SYMBOLTYPE_CURRENCY
    const OUString* GetNumForString( sal_uInt16 nNumFor, sal_uInt16 nPos,
                                     bool bString = false ) const;

    // Subtype of a subformat code nNumFor (0..3)
    // nPos == 0xFFFF => last substring
    short GetNumForType( sal_uInt16 nNumFor, sal_uInt16 nPos ) const;

    OUString GetDenominatorString( sal_uInt16 nNumFor ) const;
    OUString GetNumeratorString( sal_uInt16 nNumFor ) const;
    OUString GetIntegerFractionDelimiterString( sal_uInt16 nNumFor ) const;
    /// Round fNumber to its fraction representation
    double GetRoundFractionValue ( double fNumber ) const;

    /** If the count of string elements (substrings, ignoring [modifiers] and
        so on) in a subformat code nNumFor (0..3) is equal to the given number.
        Used by ImpSvNumberInputScan::IsNumberFormatMain() to detect a matched
        format.  */
    bool IsNumForStringElementCountEqual( sal_uInt16 nNumFor, sal_uInt16 nAllCount,
            sal_uInt16 nNumCount ) const
        {
            if ( nNumFor < 4 )
            {
                // First try a simple approach. Note that this is called only
                // if all MidStrings did match so far, to verify that all
                // strings of the format were matched and not just the starting
                // sequence, so we don't have to check if GetCount() includes
                // [modifiers] or anything else if both counts are equal.
                sal_uInt16 nCnt = NumFor[nNumFor].GetCount();
                if ( nAllCount == nCnt )
                    return true;
                if ( nAllCount < nCnt ) // check ignoring [modifiers] and so on
                    return ImpGetNumForStringElementCount( nNumFor ) ==
                        (nAllCount - nNumCount);
            }
            return false;
        }
    /** Get the count of numbers among string elements **/
    sal_uInt16 GetNumForNumberElementCount( sal_uInt16 nNumFor ) const;

    /** Get the scanned type of the specified subformat. */
    SvNumFormatType GetNumForInfoScannedType( sal_uInt16 nNumFor ) const
    {
        return (nNumFor < 4) ? NumFor[nNumFor].Info().eScannedType : SvNumFormatType::UNDEFINED;
    }

    // Whether the second subformat code is really for negative numbers
    // or another limit set.
    bool IsSecondSubformatRealNegative() const
        {
            return fLimit1 == 0.0 && fLimit2 == 0.0 &&
            ( (eOp1 == NUMBERFORMAT_OP_GE && eOp2 == NUMBERFORMAT_OP_NO) ||
              (eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_LT) ||
              (eOp1 == NUMBERFORMAT_OP_NO && eOp2 == NUMBERFORMAT_OP_NO) );
        }

    // Whether the first subformat code is really for negative numbers
    // or another limit set.
    bool IsFirstSubformatRealNegative() const
        {
            return fLimit1 == 0.0 && fLimit2 == 0.0 &&
                ((eOp1 == NUMBERFORMAT_OP_LT &&
                  (eOp2 == NUMBERFORMAT_OP_GT || eOp2 == NUMBERFORMAT_OP_EQ ||
                   eOp2 == NUMBERFORMAT_OP_GE || eOp2 == NUMBERFORMAT_OP_NO)) ||
                 (eOp1 == NUMBERFORMAT_OP_LE &&
                  (eOp2 == NUMBERFORMAT_OP_NO || eOp2 == NUMBERFORMAT_OP_GT)));
        }

    // Whether the negative format is without a sign or not
    bool IsNegativeWithoutSign() const;

    bool IsNegativeInBracket() const;

    bool HasPositiveBracketPlaceholder() const;

    // Whether a new SYMBOLTYPE_CURRENCY is contained in the format
    bool HasNewCurrency() const;

    // strip [$-yyy] from all [$xxx-yyy] leaving only xxx's,
    static OUString StripNewCurrencyDelimiters( const OUString& rStr );

    // If a new SYMBOLTYPE_CURRENCY is contained if the format is of type
    // css::util::NumberFormat::CURRENCY, and if so the symbol xxx and the extension nnn
    // of [$xxx-nnn] are returned
    bool GetNewCurrencySymbol( OUString& rSymbol, OUString& rExtension ) const;

    static bool HasStringNegativeSign( const OUString& rStr );

    /**
        Whether a character at position nPos is somewhere between two matching
        cQuote or not.
        If nPos points to a cQuote, a true is returned on an opening cQuote,
        a false is returned on a closing cQuote.
        A cQuote between quotes may be escaped by a cEscIn, a cQuote outside of
        quotes may be escaped by a cEscOut.
        The default '\0' results in no escapement possible.
        Defaults are set right according to the "unlogic" of the Numberformatter
     */
    static bool IsInQuote( const OUString& rString, sal_Int32 nPos,
                           sal_Unicode cQuote = '"',
                           sal_Unicode cEscIn = '\0', sal_Unicode cEscOut = '\\' );

    /**
        Return the position of a matching closing cQuote if the character at
        position nPos is between two matching cQuote, otherwise return -1.
        If nPos points to an opening cQuote the position of the matching
        closing cQuote is returned.
        If nPos points to a closing cQuote nPos is returned.
        If nPos points into a part which starts with an opening cQuote but has
        no closing cQuote, rString.Len() is returned.
        Uses <method>IsInQuote</method> internally, so you don't have to call
        that prior to a call of this method.
     */
    static sal_Int32 GetQuoteEnd( const OUString& rString, sal_Int32 nPos,
                                  sal_Unicode cQuote = '"',
                                  sal_Unicode cEscIn = '\0' );

    void SetComment( const OUString& rStr )
        { sComment = rStr; }
    const OUString& GetComment() const { return sComment; }

    /** Insert the number of blanks into the string that is needed to simulate
        the width of character c for underscore formats */
    static sal_Int32 InsertBlanks( OUString& r, sal_Int32 nPos, sal_Unicode c )
    {
        sal_Int32 result;
        OUStringBuffer sBuff(r);

        result = InsertBlanks(sBuff, nPos, c);
        r = sBuff.makeStringAndClear();

        return result;
    }

    /** Insert the number of blanks into the string that is needed to simulate
        the width of character c for underscore formats */
    static sal_Int32 InsertBlanks( OUStringBuffer& r, sal_Int32 nPos, sal_Unicode c );

    /// One of YMD,DMY,MDY if date format
    DateOrder GetDateOrder() const;

    /** A coded value of the exact YMD combination used, if date format.
        For example: YYYY-MM-DD => ('Y' << 16) | ('M' << 8) | 'D'
        or: MM/YY => ('M' << 8) | 'Y'  */
    sal_uInt32 GetExactDateOrder() const;

    // used in XML export
    void GetConditions( SvNumberformatLimitOps& rOper1, double& rVal1,
                        SvNumberformatLimitOps& rOper2, double& rVal2 ) const;
    Color* GetColor( sal_uInt16 nNumFor ) const;
    void GetNumForInfo( sal_uInt16 nNumFor, SvNumFormatType& rScannedType,
                    bool& bThousand, sal_uInt16& nPrecision, sal_uInt16& nLeadingCnt ) const;

    // rAttr.Number not empty if NatNum attributes are to be stored
    void GetNatNumXml(
            css::i18n::NativeNumberXmlAttributes2& rAttr,
            sal_uInt16 nNumFor ) const;

    /** Switches to the first non-"gregorian" calendar, but only if the current
        calendar is "gregorian"; original calendar name and date/time returned,
        but only if calendar switched and rOrgCalendar was empty. */
    void SwitchToOtherCalendar( OUString& rOrgCalendar, double& fOrgDateTime ) const;

    /** Switches to the "gregorian" calendar, but only if the current calendar
        is non-"gregorian" and rOrgCalendar is not empty. Thus a preceding
        ImpSwitchToOtherCalendar() call should have been placed prior to
        calling this method. */
    void SwitchToGregorianCalendar( const OUString& rOrgCalendar, double fOrgDateTime ) const;

#ifdef THE_FUTURE
    /** Switches to the first specified calendar, if any, in subformat nNumFor
        (0..3). Original calendar name and date/time returned, but only if
        calendar switched and rOrgCalendar was empty.

        @return
            <TRUE/> if a calendar was specified and switched to,
            <FALSE/> else.
     */
    bool SwitchToSpecifiedCalendar( OUString& rOrgCalendar, double& fOrgDateTime,
            sal_uInt16 nNumFor ) const
        {
            if ( nNumFor < 4 )
                return ImpSwitchToSpecifiedCalendar( rOrgCalendar,
                        fOrgDateTime, NumFor[nNumFor] );
            return false;
        }
#endif

    /// Whether it's a (YY)YY-M(M)-D(D) format.
    bool IsIso8601( sal_uInt16 nNumFor ) const
        {
            if ( nNumFor < 4 )
                return ImpIsIso8601( NumFor[nNumFor]);
            return false;
        }

private:
    ImpSvNumFor NumFor[4];          // Array for the 4 subformats
    OUString sFormatstring;         // The format code string
    OUString sComment;                // Comment, since number formatter version 6
    double fLimit1;                 // Value for first condition
    double fLimit2;                 // Value for second condition
    ImpSvNumberformatScan& rScan;   // Format code scanner
    LocaleType maLocale;            // Language/country of the format, numeral shape and calendar type from Excel.
    SvNumberformatLimitOps eOp1;    // Operator for first condition
    SvNumberformatLimitOps eOp2;    // Operator for second condition
    SvNumFormatType eType;          // Type of format
    bool bAdditionalBuiltin;        // If this is an additional built-in format defined by i18n
    bool bStarFlag;                 // Take *n format as ESC n
    bool bStandard;                 // If this is a default standard format
    bool bIsUsed;                   // Flag as used for storing

    SVL_DLLPRIVATE sal_uInt16 ImpGetNumForStringElementCount( sal_uInt16 nNumFor ) const;

    SVL_DLLPRIVATE bool ImpIsOtherCalendar( const ImpSvNumFor& rNumFor ) const;

#ifdef THE_FUTURE
    SVL_DLLPRIVATE bool ImpSwitchToSpecifiedCalendar( OUString& rOrgCalendar,
                                                      double& fOrgDateTime,
                                                      const ImpSvNumFor& rNumFor ) const;
#endif

    /** Whether to use possessive genitive case month name, or partitive case
        month name, instead of nominative name (noun).

        @param io_nState
            0: execute check <br>
               set to 1 if nominative case is returned, <br>
               set to 2 if genitive case is returned, <br>
               set to 3 if partitive case is returned <br>
            1: don't execute check, return nominative case <br>
            2: don't execute check, return genitive case <br>
            3: don't execute check, return partitive case <br>

        @param eCodeType
            a NfKeywordIndex, must designate a month type code

        @returns one of css::i18n::CalendarDisplayCode values
            according to eCodeType and the check executed (or passed).
     */
    SVL_DLLPRIVATE static sal_Int32 ImpUseMonthCase( int & io_nState, const ImpSvNumFor& rNumFor, NfKeywordIndex eCodeType );

    /// Whether it's a (YY)YY-M(M)-D(D) format.
    SVL_DLLPRIVATE bool ImpIsIso8601( const ImpSvNumFor& rNumFor ) const;

    const CharClass& rChrCls() const;
    const LocaleDataWrapper& rLoc() const;
    CalendarWrapper& GetCal() const;
    const SvNumberFormatter& GetFormatter() const;

    // divide in substrings and color conditions
    SVL_DLLPRIVATE short ImpNextSymbol( OUStringBuffer& rString,
                                        sal_Int32& nPos,
                                        OUString& sSymbol ) const;

    // read string until ']' and strip blanks (after condition)
    SVL_DLLPRIVATE static sal_Int32 ImpGetNumber( OUStringBuffer& rString,
                                                  sal_Int32& nPos,
                                                  OUString& sSymbol );

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
     *         specifies language type. See i18nlangtag/lang.h for a complete
     *         list of language types. These numbers also correspond with the
     *         numbers used by Microsoft Office.
     */
    SVL_DLLPRIVATE static LocaleType ImpGetLocaleType( const OUString& rString, sal_Int32& nPos );

    /** Obtain calendar and numerals from a LocaleType that was parsed from a
        LCID with ImpGetLocaleType().

        Inserts a NatNum modifier to rString at nPos if needed as determined
        from the numeral code.

        @ATTENTION: may modify <member>maLocale</member> to make it follow
        aTmpLocale, in which case also nLang is adapted.

        @returns a string with the calendar if one was determined from the
        calendar code, else an empty string. The calendar string needs to be
        inserted at a proper position to rString after all bracketed prefixes.
     */
    SVL_DLLPRIVATE OUString ImpObtainCalendarAndNumerals( OUStringBuffer & rString,
                                                          sal_Int32 nPos,
                                                          LanguageType & nLang,
                                                          const LocaleType & aTmpLocale );

    // standard number output
    SVL_DLLPRIVATE void ImpGetOutputStandard( double& fNumber, OUString& OutString ) const;
    SVL_DLLPRIVATE void ImpGetOutputStandard( double& fNumber, OUStringBuffer& OutString ) const;
    SVL_DLLPRIVATE void ImpGetOutputStdToPrecision( double& rNumber, OUString& rOutString, sal_uInt16 nPrecision ) const;
    // numbers in input line
    SVL_DLLPRIVATE void ImpGetOutputInputLine( double fNumber, OUString& OutString ) const;

    // check subcondition
    // OP undefined => -1
    // else 0 or 1
    SVL_DLLPRIVATE static short ImpCheckCondition(double fNumber,
                         double fLimit,
                         SvNumberformatLimitOps eOp);

    // Helper function for number strings
    // append string symbols, insert leading 0 or ' ', or ...
    SVL_DLLPRIVATE bool ImpNumberFill( OUStringBuffer& sStr,
                    double& rNumber,
                    sal_Int32& k,
                    sal_uInt16& j,
                    sal_uInt16 nIx,
                    short eSymbolType,
                    bool bInsertRightBlank = false );

    // Helper function to fill in the integer part and the group (AKA thousand) separators
    SVL_DLLPRIVATE bool ImpNumberFillWithThousands( OUStringBuffer& sStr,
                                 double& rNumber,
                                 sal_Int32 k,
                                 sal_uInt16 j,
                                 sal_uInt16 nIx,
                                 sal_Int32 nDigCnt,
                                 bool bAddDecSep = true );

    // Helper function to fill in the group (AKA thousand) separators
    // or to skip additional digits
    SVL_DLLPRIVATE void ImpDigitFill( OUStringBuffer& sStr,
                                      sal_Int32 nStart,
                                      sal_Int32& k,
                                      sal_uInt16 nIx,
                                      sal_Int32 & nDigitCount,
                                      utl::DigitGroupingIterator & );

    SVL_DLLPRIVATE bool ImpDecimalFill( OUStringBuffer& sStr,
                                 double& rNumber,
                                 sal_Int32 nDecPos,
                                 sal_uInt16 j,
                                 sal_uInt16 nIx,
                                 bool bInteger );

    /** Calculate each element of fraction:
     * integer part, numerator part, denominator part
     * @param fNumber value to be represented as fraction. Will contain absolute fractional part
     * @param nIx subformat number 0..3
     * @param fIntPart integral part of fraction
     * @param nFrac numerator of fraction
     * @param nDic denominator of fraction
     */
    SVL_DLLPRIVATE void ImpGetFractionElements( double& fNumber,
                                                sal_uInt16 nIx,
                                                double& fIntPart,
                                                sal_uInt64& nFrac,
                                                sal_uInt64& nDiv ) const;
    SVL_DLLPRIVATE bool ImpGetFractionOutput(double fNumber,
                                             sal_uInt16 nIx,
                                             OUStringBuffer& OutString);
    SVL_DLLPRIVATE bool ImpGetScientificOutput(double fNumber,
                                               sal_uInt16 nIx,
                                               OUStringBuffer& OutString);

    SVL_DLLPRIVATE bool ImpGetDateOutput( double fNumber,
                                          sal_uInt16 nIx,
                                          OUStringBuffer& OutString );
    SVL_DLLPRIVATE bool ImpGetTimeOutput( double fNumber,
                                          sal_uInt16 nIx,
                                          OUStringBuffer& OutString );
    SVL_DLLPRIVATE bool ImpGetDateTimeOutput( double fNumber,
                                              sal_uInt16 nIx,
                                              OUStringBuffer& OutString );

    // Switches to the "gregorian" calendar if the current calendar is
    // non-"gregorian" and the era is a "Dummy" era of a calendar which doesn't
    // know a "before" era (like zh_TW ROC or ja_JP Gengou). If switched and
    // rOrgCalendar was "gregorian" the string is emptied. If rOrgCalendar was
    // empty the previous calendar name and date/time are returned.
    SVL_DLLPRIVATE bool ImpFallBackToGregorianCalendar( OUString& rOrgCalendar, double& fOrgDateTime );

    // Append a "G" short era string of the given calendar. In the case of a
    // Gengou calendar this is a one character abbreviation, for other
    // calendars the XExtendedCalendar::getDisplayString() method is called.
    SVL_DLLPRIVATE static void ImpAppendEraG( OUStringBuffer& OutStringBuffer, const CalendarWrapper& rCal,
                                              sal_Int16 nNatNum );

    SVL_DLLPRIVATE bool ImpGetNumberOutput( double fNumber,
                                            sal_uInt16 nIx,
                                            OUStringBuffer& OutString );

    SVL_DLLPRIVATE void ImpCopyNumberformat( const SvNumberformat& rFormat );

    // normal digits or other digits, depending on ImpSvNumFor.aNatNum,
    // [NatNum1], [NatNum2], ...
    SVL_DLLPRIVATE OUString ImpGetNatNumString( const SvNumberNatNum& rNum, sal_Int32 nVal,
                                              sal_uInt16 nMinDigits  ) const;

    OUString ImpIntToString( sal_uInt16 nIx, sal_Int32 nVal, sal_uInt16 nMinDigits = 0 ) const
    {
        const SvNumberNatNum& rNum = NumFor[nIx].GetNatNum();
        if ( nMinDigits || rNum.IsComplete() )
        {
            return ImpGetNatNumString( rNum, nVal, nMinDigits );
        }
        return OUString::number(nVal);
    }

    // Obtain the string of the fraction of second, without leading "0.",
    // rounded to nFractionDecimals (or nFractionDecimals+1 if
    // bAddOneRoundingDecimal==true but then truncated at nFractionDecimals,
    // for use with the result of tools::Time::GetClock()) with the length of
    // nFractionDecimals, unless nMinimumInputLineDecimals>0 is given for input
    // line string where extra trailing "0" are discarded.
    SVL_DLLPRIVATE sal_uInt16 ImpGetFractionOfSecondString( OUStringBuffer& rBuf, double fFractionOfSecond,
            int nFractionDecimals, bool bAddOneRoundingDecimal, sal_uInt16 nIx, sal_uInt16 nMinimumInputLineDecimals );

    // transliterate according to NativeNumber
    SVL_DLLPRIVATE OUString impTransliterateImpl(const OUString& rStr, const SvNumberNatNum& rNum) const;
    SVL_DLLPRIVATE void impTransliterateImpl(OUStringBuffer& rStr, const SvNumberNatNum& rNum) const;
    SVL_DLLPRIVATE OUString impTransliterateImpl(const OUString& rStr, const SvNumberNatNum& rNum, sal_uInt16 nDateKey) const;

    OUString impTransliterate(const OUString& rStr, const SvNumberNatNum& rNum) const
    {
        return rNum.IsComplete() ? impTransliterateImpl(rStr, rNum) : rStr;
    }

    SVL_DLLPRIVATE void impTransliterate(OUStringBuffer& rStr, const SvNumberNatNum& rNum) const
    {
        if(rNum.IsComplete())
        {
            impTransliterateImpl(rStr, rNum);
        }
    }

    OUString impTransliterate(const OUString& rStr, const SvNumberNatNum& rNum, sal_uInt16 nDateKey) const
    {
        return rNum.IsComplete() ? impTransliterateImpl(rStr, rNum, nDateKey) : rStr;
    }

};

#endif // INCLUDED_SVL_ZFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
