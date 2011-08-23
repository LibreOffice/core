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

#include "bf_svtools/svtdllapi.h"

#include <tools/string.hxx>
#include <i18npool/mslangid.hxx>

#include <bf_svtools/zforlist.hxx>

#include <bf_svtools/nfversi.hxx>

#include <bf_svtools/nfkeytab.hxx>

// We need ImpSvNumberformatScan for the private SvNumberformat definitions.
#ifdef _ZFORMAT_CXX
#include "zforscan.hxx"
#endif

// If comment field is also in format code string, was used for SUPD versions 371-372
#define NF_COMMENT_IN_FORMATSTRING 0

class SvStream;
class Color;
class CharClass;

namespace binfilter {

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
    USHORT nThousand;                   // Count of group separator sequences
    USHORT nCntPre;                     // Count of digits before decimal point
    USHORT nCntPost;                    // Count of digits after decimal point
    USHORT nCntExp;                     // Count of exponent digits, or AM/PM
    short eScannedType;                 // Type determined by scan
    BOOL bThousand;                     // Has group (AKA thousand) separator

    void Copy( const ImpSvNumberformatInfo& rNumFor, USHORT nAnz );
    void Load(SvStream& rStream, USHORT nAnz);
    void Save(SvStream& rStream, USHORT nAnz) const;
};

// NativeNumber, represent numbers using CJK or other digits if nNum>0,
// eLang specifies the Locale to use.
class SvNumberNatNum
{
    LanguageType    eLang;
    BYTE            nNum;
    BOOL            bDBNum  :1;     // DBNum, to be converted to NatNum
    BOOL            bDate   :1;     // Used in date? (needed for DBNum/NatNum mapping)
    BOOL            bSet    :1;     // If set, since NatNum0 is possible

public:

    static  BYTE    MapDBNumToNatNum( BYTE nDBNum, LanguageType eLang, BOOL bDate );

                    SvNumberNatNum() : eLang( LANGUAGE_DONTKNOW ), nNum(0),
                                        bDBNum(0), bDate(0), bSet(0) {}
    BOOL            IsComplete() const  { return bSet && eLang != LANGUAGE_DONTKNOW; }
    BYTE            GetRawNum() const   { return nNum; }
    BYTE            GetNatNum() const   { return bDBNum ? MapDBNumToNatNum( nNum, eLang, bDate ) : nNum; }
    LanguageType    GetLang() const     { return eLang; }
    void            SetLang( LanguageType e ) { eLang = e; }
    void            SetNum( BYTE nNumber, BOOL bDBNumber )
                        {
                            nNum = nNumber;
                            bDBNum = bDBNumber;
                            bSet = TRUE;
                        }
    BOOL            IsSet() const       { return bSet; }
    void            SetDate( BOOL bDateP )   { bDate = (bDateP != 0); }
};

class ImpSvNumFor                       // One of four subformats of the format code string
{
public:
    ImpSvNumFor();                      // Ctor without filling the Info
    ~ImpSvNumFor();

    void Enlarge(USHORT nAnz);          // Init of arrays to the right size
    void Load( SvStream& rStream, ImpSvNumberformatScan& rSc,
                String& rLoadedColorName);
    void Save( SvStream& rStream ) const;

    // if pSc is set, it is used to get the Color pointer
    void Copy( const ImpSvNumFor& rNumFor, ImpSvNumberformatScan* pSc );

    // Access to Info; call Enlarge before!
    ImpSvNumberformatInfo& Info() { return aI;}
    const ImpSvNumberformatInfo& Info() const { return aI; }

    // Get count of substrings (symbols)
    USHORT GetnAnz() const { return nAnzStrings;}

    Color* GetColor() const { return pColor; }
    void SetColor( Color* pCol, String& rName )
     { pColor = pCol; sColorName = rName; }
    const String& GetColorName() const { return sColorName; }

    // new SYMBOLTYPE_CURRENCY in subformat?
    BOOL HasNewCurrency() const;
    BOOL GetNewCurrencySymbol( String& rSymbol, String& rExtension ) const;
    void SaveNewCurrencyMap( SvStream& rStream ) const;
    void LoadNewCurrencyMap( SvStream& rStream );

    // [NatNum1], [NatNum2], ...
    void SetNatNumNum( BYTE nNum, BOOL bDBNum ) { aNatNum.SetNum( nNum, bDBNum ); }
    void SetNatNumLang( LanguageType eLang ) { aNatNum.SetLang( eLang ); }
    void SetNatNumDate( BOOL bDate ) { aNatNum.SetDate( bDate ); }
    const SvNumberNatNum& GetNatNum() const { return aNatNum; }

private:
    ImpSvNumberformatInfo aI;           // Hilfsstruct fuer die restlichen Infos
    String sColorName;                  // color name
    Color* pColor;                      // pointer to color of subformat
    USHORT nAnzStrings;                 // count of symbols
    SvNumberNatNum aNatNum;             // DoubleByteNumber

};

class  SvNumberformat
{
public:
    // Ctor for Load
    SvNumberformat( ImpSvNumberformatScan& rSc, LanguageType eLge );

    // Normal ctor
    SvNumberformat( String& rString,
                   ImpSvNumberformatScan* pSc,
                   ImpSvNumberInputScan* pISc,
                   xub_StrLen& nCheckPos,
                   LanguageType& eLan,
                   BOOL bStand = FALSE );

    // Copy ctor
    SvNumberformat( const SvNumberformat& rFormat );

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
    void SetStandard()                          { bStandard = TRUE; }
    BOOL IsStandard() const                     { return bStandard; }

    // For versions before version nVer it is UserDefined, for newer versions
    // it is builtin. nVer of SV_NUMBERFORMATTER_VERSION_...
    void SetNewStandardDefined( USHORT nVer )
        { nNewStandardDefined = nVer; eType |= NUMBERFORMAT_DEFINED; }

    USHORT GetNewStandardDefined() const        { return nNewStandardDefined; }
    BOOL IsAdditionalStandardDefined() const
        { return nNewStandardDefined == SV_NUMBERFORMATTER_VERSION_ADDITIONAL_I18N_FORMATS; }

    LanguageType GetLanguage() const            { return eLnge;}

    const String& GetFormatstring() const   { return sFormatstring; }

    void SetUsed(const BOOL b)                  { bIsUsed = b; }
    BOOL GetUsed() const                        { return bIsUsed; }
    BOOL IsStarFormatSupported() const          { return bStarFlag; }
    void SetStarFormatSupport( BOOL b )         { bStarFlag = b; }

    NfHackConversion Load( SvStream& rStream, ImpSvNumMultipleReadHeader& rHdr,
        SvNumberFormatter* pConverter, ImpSvNumberInputScan& rISc );
    void Save( SvStream& rStream, ImpSvNumMultipleWriteHeader& rHdr  ) const;

    // Load a string which might contain an Euro symbol,
    // in fact that could be any string used in number formats.
    static void LoadString( SvStream& rStream, String& rStr );

    BOOL GetOutputString( double fNumber, String& OutString, Color** ppColor );
    BOOL GetOutputString( String& sString, String& OutString, Color** ppColor );

    // True if type text
    BOOL IsTextFormat() const { return (eType & NUMBERFORMAT_TEXT) != 0; }
    // True if 4th subformat present
    BOOL HasTextFormat() const
        {
            return (NumFor[3].GetnAnz() > 0) ||
                (NumFor[3].Info().eScannedType == NUMBERFORMAT_TEXT);
        }

    void GetFormatSpecialInfo(BOOL& bThousand,
                              BOOL& IsRed,
                              USHORT& nPrecision,
                              USHORT& nAnzLeading) const;

    /// Count of decimal precision
    USHORT GetFormatPrecision() const   { return NumFor[0].Info().nCntPost; }

    //! Read/write access on a special USHORT component, may only be used on the
    //! standard format 0, 5000, ... and only by the number formatter!
    USHORT GetLastInsertKey() const
        { return NumFor[0].Info().nThousand; }
    void SetLastInsertKey(USHORT nKey)
        { NumFor[0].Info().nThousand = nKey; }

    //! Only onLoad: convert from stored to current system language/country
    void ConvertLanguage( SvNumberFormatter& rConverter,
        LanguageType eConvertFrom, LanguageType eConvertTo, BOOL bSystem = FALSE );

    // Substring of a subformat code nNumFor (0..3)
    // nPos == 0xFFFF => last substring
    // bString==TRUE: first/last SYMBOLTYPE_STRING or SYMBOLTYPE_CURRENCY
    const String* GetNumForString( USHORT nNumFor, USHORT nPos,
            BOOL bString = FALSE ) const;

    // Subtype of a subformat code nNumFor (0..3)
    // nPos == 0xFFFF => last substring
    // bString==TRUE: first/last SYMBOLTYPE_STRING or SYMBOLTYPE_CURRENCY
    short GetNumForType( USHORT nNumFor, USHORT nPos, BOOL bString = FALSE ) const;

    /** If the count of string elements (substrings, ignoring [modifiers] and
        so on) in a subformat code nNumFor (0..3) is equal to the given number.
        Used by ImpSvNumberInputScan::IsNumberFormatMain() to detect a matched
        format.  */
    BOOL IsNumForStringElementCountEqual( USHORT nNumFor, USHORT nAllCount,
            USHORT nNumCount ) const
        {
            if ( nNumFor < 4 )
            {
                // First try a simple approach. Note that this is called only
                // if all MidStrings did match so far, to verify that all
                // strings of the format were matched and not just the starting
                // sequence, so we don't have to check if GetnAnz() includes
                // [modifiers] or anything else if both counts are equal.
                USHORT nCnt = NumFor[nNumFor].GetnAnz();
                if ( nAllCount == nCnt )
                    return TRUE;
                if ( nAllCount < nCnt ) // check ignoring [modifiers] and so on
                    return ImpGetNumForStringElementCount( nNumFor ) ==
                        (nAllCount - nNumCount);
            }
            return FALSE;
        }

    // Whether the second subformat code is really for negative numbers
    // or another limit set.
    BOOL IsNegativeRealNegative() const
        {
            return fLimit1 == 0.0 && fLimit2 == 0.0 &&
            ( (eOp1 == NUMBERFORMAT_OP_GE && eOp2 == NUMBERFORMAT_OP_NO) ||
              (eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_LT) ||
              (eOp1 == NUMBERFORMAT_OP_NO && eOp2 == NUMBERFORMAT_OP_NO) );
        }

    // Whether the negative format is without a sign or not
    BOOL IsNegativeWithoutSign() const;

    // Whether a new SYMBOLTYPE_CURRENCY is contained in the format
    BOOL HasNewCurrency() const;

    // Build string from NewCurrency for saving it SO50 compatible
    void Build50Formatstring( String& rStr ) const;

    // strip [$-yyy] from all [$xxx-yyy] leaving only xxx's,
    // if bQuoteSymbol==TRUE the xxx will become "xxx"
    static String StripNewCurrencyDelimiters( const String& rStr,
        BOOL bQuoteSymbol );

    // If a new SYMBOLTYPE_CURRENCY is contained if the format is of type
    // NUMBERFORMAT_CURRENCY, and if so the symbol xxx and the extension nnn
    // of [$xxx-nnn] are returned
    BOOL GetNewCurrencySymbol( String& rSymbol, String& rExtension ) const;

    static BOOL HasStringNegativeSign( const String& rStr );

    /**
        Whether a character at position nPos is somewhere between two matching
        cQuote or not.
        If nPos points to a cQuote, a TRUE is returned on an opening cQuote,
        a FALSE is returned on a closing cQuote.
        A cQuote between quotes may be escaped by a cEscIn, a cQuote outside of
        quotes may be escaped by a cEscOut.
        The default '\0' results in no escapement possible.
        Defaults are set right according to the "unlogic" of the Numberformatter
     */
    static BOOL IsInQuote( const String& rString, xub_StrLen nPos,
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
            { sComment = rStr; }

    const String& GetComment() const { return sComment; }

    // Erase "{ "..." }" from format subcode string to get the pure comment (old version)
    static void EraseCommentBraces( String& rStr );
    // Set comment rStr in format string rFormat and in rComment (old version)

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
    Color* GetColor( USHORT nNumFor ) const;
    void GetNumForInfo( USHORT nNumFor, short& rScannedType,
                    BOOL& bThousand, USHORT& nPrecision, USHORT& nAnzLeading ) const;

    // rAttr.Number not empty if NatNum attributes are to be stored
    void GetNatNumXml(
            ::com::sun::star::i18n::NativeNumberXmlAttributes& rAttr,
            USHORT nNumFor ) const;

    /** @returns <TRUE/> if E,EE,R,RR,AAA,AAAA in format code of subformat
        nNumFor (0..3) and <b>no</b> preceding calendar was specified and the
        currently loaded calendar is "gregorian". */
    BOOL IsOtherCalendar( USHORT nNumFor ) const
        {
            if ( nNumFor < 4 )
                return ImpIsOtherCalendar( NumFor[nNumFor] );
            return FALSE;
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

private:
    ImpSvNumFor NumFor[4];          // Array for the 4 subformats
    String sFormatstring;           // The format code string
    String sComment;                // Comment, since number formatter version 6
    double fLimit1;                 // Value for first condition
    double fLimit2;                 // Value for second condition
    ImpSvNumberformatScan& rScan;   // Format code scanner
    LanguageType eLnge;             // Language/country of the format
    SvNumberformatLimitOps eOp1;    // Operator for first condition
    SvNumberformatLimitOps eOp2;    // Operator for second condition
    USHORT nNewStandardDefined;     // new builtin formats as of version 6
    short eType;                    // Type of format
    BOOL bStarFlag;                 // Take *n format as ESC n
    BOOL bStandard;                 // If this is a default standard format
    BOOL bIsUsed;                   // Flag as used for storing

    USHORT ImpGetNumForStringElementCount( USHORT nNumFor ) const;

    BOOL ImpIsOtherCalendar( const ImpSvNumFor& rNumFor ) const;

#ifdef _ZFORMAT_CXX     // ----- private implementation methods -----

    const CharClass& rChrCls() const        { return rScan.GetChrCls(); }
    const LocaleDataWrapper& rLoc() const   { return rScan.GetLoc(); }
    CalendarWrapper& GetCal() const         { return rScan.GetCal(); }
    const SvNumberFormatter& GetFormatter() const   { return *rScan.GetNumberformatter(); }

    // divide in substrings and color conditions
    short ImpNextSymbol( String& rString,
                     xub_StrLen& nPos,
                     String& sSymbol );

    // read string until ']' and strip blanks (after condition)
    static xub_StrLen ImpGetNumber( String& rString,
                   xub_StrLen& nPos,
                   String& sSymbol );

    // get xxx of "[$-xxx]" as LanguageType, starting at and advancing position nPos
    static LanguageType ImpGetLanguageType( const String& rString, xub_StrLen& nPos );

    // standard number output
    void ImpGetOutputStandard( double& fNumber, String& OutString );
    // numbers in input line
    void ImpGetOutputInputLine( double fNumber, String& OutString );

    // check subcondition
    // OP undefined => -1
    // else 0 or 1
    short ImpCheckCondition(double& fNumber,
                         double& fLimit,
                         SvNumberformatLimitOps eOp);

    ULONG ImpGGT(ULONG x, ULONG y);
    ULONG ImpGGTRound(ULONG x, ULONG y);

    // Helper function for number strings
    // append string symbols, insert leading 0 or ' ', or ...
    BOOL ImpNumberFill( String& sStr,
                    double& rNumber,
                    xub_StrLen& k,
                    USHORT& j,
                    USHORT nIx,
                    short eSymbolType );

    // Helper function to fill in the integer part and the group (AKA thousand) separators
    BOOL ImpNumberFillWithThousands( String& sStr,
                                 double& rNumber,
                                 xub_StrLen k,
                                 USHORT j,
                                 USHORT nIx,
                                 USHORT nDigCnt );
                                    // Hilfsfunktion zum Auffuellen der Vor-
                                    // kommazahl auch mit Tausenderpunkt

    // Helper function to fill in the group (AKA thousand) separators
    // or to skip additional digits
    void ImpDigitFill( String& sStr,
                    xub_StrLen nStart,
                    xub_StrLen& k,
                    USHORT nIx,
                    USHORT nThousandCnt );

    BOOL ImpGetDateOutput( double fNumber,
                       USHORT nIx,
                       String& OutString );
    BOOL ImpGetTimeOutput( double fNumber,
                       USHORT nIx,
                       String& OutString );
    BOOL ImpGetDateTimeOutput( double fNumber,
                           USHORT nIx,
                           String& OutString );

    // Switches to the "gregorian" calendar if the current calendar is
    // non-"gregorian" and the era is a "Dummy" era of a calendar which doesn't
    // know a "before" era (like zh_TW ROC or ja_JP Gengou). If switched and
    // rOrgCalendar was "gregorian" the string is emptied. If rOrgCalendar was
    // empty the previous calendar name and date/time are returned.
    BOOL ImpFallBackToGregorianCalendar( String& rOrgCalendar, double& fOrgDateTime );

    // Append a "G" short era string of the given calendar. In the case of a
    // Gengou calendar this is a one character abbreviation, for other
    // calendars the XExtendedCalendar::getDisplayString() method is called.
    static void ImpAppendEraG( String& OutString, const CalendarWrapper& rCal,
            sal_Int16 nNatNum );

    BOOL ImpGetNumberOutput( double fNumber,
                         USHORT nIx,
                         String& OutString );

    void ImpCopyNumberformat( const SvNumberformat& rFormat );

    // normal digits or other digits, depending on ImpSvNumFor.aNatNum,
    // [NatNum1], [NatNum2], ...
    String ImpGetNatNumString( const SvNumberNatNum& rNum, sal_Int32 nVal,
            USHORT nMinDigits = 0  ) const;

    String ImpIntToString( USHORT nIx, sal_Int32 nVal, USHORT nMinDigits = 0 ) const
        {
            const SvNumberNatNum& rNum = NumFor[nIx].GetNatNum();
            if ( nMinDigits || rNum.IsComplete() )
                return ImpGetNatNumString( rNum, nVal, nMinDigits );
            return String::CreateFromInt32( nVal );
        }

    // transliterate according to NativeNumber
    void ImpTransliterateImpl( String& rStr, const SvNumberNatNum& rNum ) const;

    void ImpTransliterate( String& rStr, const SvNumberNatNum& rNum ) const
        {
            if ( rNum.IsComplete() )
                ImpTransliterateImpl( rStr, rNum );
        }

#endif // _ZFORMAT_CXX

};

}

#endif  // _ZFORMAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
