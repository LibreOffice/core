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
#ifndef _ZFORLIST_HXX
#define _ZFORLIST_HXX

#include "bf_svtools/svtdllapi.h"

#include <tools/string.hxx>
#include <tools/table.hxx>
#include <i18npool/lang.h>
#include <bf_svtools/svarray.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/NumberFormatCode.hpp>
#include <unotools/localedatawrapper.hxx>
#include <bf_svtools/ondemand.hxx>
#include <tools/link.hxx>
#include <bf_svtools/nfkeytab.hxx>

class Date;
class SvStream;
class Color;
class CharClass;
class CalendarWrapper;

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

namespace binfilter
{

class SvUShorts;
class ImpSvNumberformatScan;
class ImpSvNumberInputScan;
class SvNumberformat;


#define SV_COUNTRY_LANGUAGE_OFFSET  5000	// Max count of formats per country/language
#define SV_MAX_ANZ_STANDARD_FORMATE  100	// Max count of builtin default formats per CL

// Format types
#ifndef NUMBERFORMAT_ALL
//	also defined in com/sun/star/util/NumberFormat.hpp
//!	=> put in single .idl file and include here
#define NUMBERFORMAT_ALL			 0x000	/// Just for Output of total list, not a real format type
#define NUMBERFORMAT_DEFINED		 0x001	/// Format defined by user
#define NUMBERFORMAT_DATE			 0x002	/// Number as date
#define NUMBERFORMAT_TIME			 0x004	/// Number as time
#define NUMBERFORMAT_CURRENCY		 0x008	/// Number as currency
#define NUMBERFORMAT_NUMBER			 0x010	/// Any "normal" number format
#define NUMBERFORMAT_SCIENTIFIC		 0x020	/// Number as scientific
#define NUMBERFORMAT_FRACTION		 0x040	/// Number as fraction
#define NUMBERFORMAT_PERCENT		 0x080	/// Number as percent
#define NUMBERFORMAT_TEXT			 0x100	/// Text format
#define NUMBERFORMAT_DATETIME		 0x006	/// Number as date and time
#define NUMBERFORMAT_LOGICAL		 0x400	/// Number as boolean value
#define NUMBERFORMAT_UNDEFINED		 0x800	/// Format undefined yet in analyzing
#endif
#define NUMBERFORMAT_ENTRY_NOT_FOUND (sal_uInt32)(0xffffffff)	/// MAX_ULONG

/** enum values for <method>SvNumberFormatter::GetFormatIndex</method>

    <p>
     Builtin standard formats, order should be also the arrangement in the
    dialog list box representation.</p>

    <p>
     Date specials:<ul>
    <li>SYSTEM: As set in System Regional Settings.
    <li>SYS: short/long defined, order and separators from System Regional Settings.
    <li>DEF: short/long and order defined, separators from System Regional Settings.
    <li>DIN: all settings hard coded as DIN (Deutsche Industrie Norm) and EN (European Norm) require.
    <li>all other: hard coded
    </ul>
 */
enum NfIndexTableOffset
{
    NF_NUMERIC_START = 0,

    NF_NUMBER_START = NF_NUMERIC_START,
    NF_NUMBER_STANDARD = NF_NUMBER_START,	// Standard/General
    NF_NUMBER_INT,							// 0
    NF_NUMBER_DEC2,							// 0.00
    NF_NUMBER_1000INT,						// #,##0
    NF_NUMBER_1000DEC2,						// #,##0.00
    NF_NUMBER_SYSTEM,						// #,##0.00 or whatever is set in System Regional Settings
    NF_NUMBER_END = NF_NUMBER_SYSTEM,

    NF_SCIENTIFIC_START,
    NF_SCIENTIFIC_000E000 = NF_SCIENTIFIC_START,	// 0.00E+000
    NF_SCIENTIFIC_000E00,							// 0.00E+00
    NF_SCIENTIFIC_END = NF_SCIENTIFIC_000E00,

    NF_PERCENT_START,
    NF_PERCENT_INT = NF_PERCENT_START,		// 0%
    NF_PERCENT_DEC2,						// 0.00%
    NF_PERCENT_END = NF_PERCENT_DEC2,

    NF_FRACTION_START,
    NF_FRACTION_1 = NF_FRACTION_START,		// # ?/?
    NF_FRACTION_2,							// # ??/??
    NF_FRACTION_END = NF_FRACTION_2,

    NF_NUMERIC_END = NF_FRACTION_END,

    NF_CURRENCY_START,
    NF_CURRENCY_1000INT = NF_CURRENCY_START,// #,##0 DM
    NF_CURRENCY_1000DEC2,					// #,##0.00 DM
    NF_CURRENCY_1000INT_RED,                // #,##0 DM			negative in red
    NF_CURRENCY_1000DEC2_RED,				// #,##0.00 DM		negative in red
    NF_CURRENCY_1000DEC2_CCC,				// #,##0.00 DEM		currency abbreviation
    NF_CURRENCY_1000DEC2_DASHED,			// #,##0.-- DM
    NF_CURRENCY_END = NF_CURRENCY_1000DEC2_DASHED,

    NF_DATE_START,
    NF_DATE_SYSTEM_SHORT = NF_DATE_START,	// 08.10.97
    NF_DATE_SYSTEM_LONG,					// Wednesday, 8. October 1997
    NF_DATE_SYS_DDMMYY,		   				// 08.10.97
    NF_DATE_SYS_DDMMYYYY,	   				// 08.10.1997
    NF_DATE_SYS_DMMMYY,	   					// 8. Oct 97
    NF_DATE_SYS_DMMMYYYY,	   				// 8. Oct 1997
    NF_DATE_DIN_DMMMYYYY,					// 8. Oct. 1997					DIN
    NF_DATE_SYS_DMMMMYYYY,					// 8. October 1997
    NF_DATE_DIN_DMMMMYYYY,					// 8. October 1997				DIN
    NF_DATE_SYS_NNDMMMYY,		 			// Wed, 8. Okt 97
    NF_DATE_DEF_NNDDMMMYY,		 			// Wed 08.Okt 97
    NF_DATE_SYS_NNDMMMMYYYY,	   			// Wed, 8. Oktober 1997
    NF_DATE_SYS_NNNNDMMMMYYYY,	   			// Wednesday, 8. Oktober 1997
    NF_DATE_DIN_MMDD,		 				// 10-08						DIN
    NF_DATE_DIN_YYMMDD,		 				// 97-10-08						DIN
    NF_DATE_DIN_YYYYMMDD,	 				// 1997-10-08					DIN
    NF_DATE_SYS_MMYY,		   				// 10.97
    NF_DATE_SYS_DDMMM,						// 08.Oct
    NF_DATE_MMMM,							// October
    NF_DATE_QQJJ,							// 4. Quarter 97
    NF_DATE_WW,								// week of year
    NF_DATE_END = NF_DATE_WW,

    NF_TIME_START,
    NF_TIME_HHMM = NF_TIME_START,			// HH:MM
    NF_TIME_HHMMSS,							// HH:MM:SS
    NF_TIME_HHMMAMPM,						// HH:MM AM/PM
    NF_TIME_HHMMSSAMPM,						// HH:MM:SS AM/PM
    NF_TIME_HH_MMSS,						// [HH]:MM:SS
    NF_TIME_MMSS00,							// MM:SS,00
    NF_TIME_HH_MMSS00,	  					// [HH]:MM:SS,00
    NF_TIME_END = NF_TIME_HH_MMSS00,

    NF_DATETIME_START,
    NF_DATETIME_SYSTEM_SHORT_HHMM = NF_DATETIME_START,	// 08.10.97 01:23
    NF_DATETIME_SYS_DDMMYYYY_HHMMSS,		// 08.10.1997 01:23:45
    NF_DATETIME_END = NF_DATETIME_SYS_DDMMYYYY_HHMMSS,

    NF_BOOLEAN,								// BOOLEAN
    NF_TEXT,								// @
    NF_INDEX_TABLE_ENTRIES
};


// #45717# IsNumberFormat( "98-10-24", 30, x ), YMD Format set with DMY
// International settings doesn't recognize the string as a date.
/** enum values for <method>SvNumberFormatter::SetEvalDateFormat</method>

    <p>How <method>ImpSvNumberInputScan::GetDateRef</method> shall take the
     DateFormat order (YMD,DMY,MDY) into account, if called from IsNumberFormat
    with a date format to match against.
 */
enum NfEvalDateFormat
{
    /** DateFormat only from International, default. */
    NF_EVALDATEFORMAT_INTL,

    /** DateFormat only from date format passed to function (if any).
        If no date format is passed then the DateFormat is taken from International. */
    NF_EVALDATEFORMAT_FORMAT,

    /** First try the DateFormat from International. If it doesn't match a
         valid date try the DateFormat from the date format passed. */
    NF_EVALDATEFORMAT_INTL_FORMAT,

    /** First try the DateFormat from the date format passed. If it doesn't
        match a valid date try the DateFormat from International. */
    NF_EVALDATEFORMAT_FORMAT_INTL
};


//#if 0 // _SOLAR__PRIVATE
#define _ZFORLIST_DECLARE_TABLE
//#endif
#ifdef _ZFORLIST_DECLARE_TABLE
DECLARE_TABLE (SvNumberFormatTable, SvNumberformat*)
DECLARE_TABLE (SvNumberFormatterIndexTable, sal_uInt32*)
#else
typedef Table SvNumberFormatTable;
typedef Table SvNumberFormatterIndexTable;
#endif


/** Language/country dependent currency entries
 */
class  NfCurrencyEntry
{
    String   		aSymbol;			/// currency symbol
    String			aBankSymbol;		/// currency abbreviation
    LanguageType	eLanguage;			/// language/country value
    USHORT			nPositiveFormat;	/// position of symbol
    USHORT			nNegativeFormat;	/// position of symbol and type and position of negative sign
    USHORT			nDigits;			/// count of decimal digits
    sal_Unicode		cZeroChar;			/// which character is used for zeros as last decimal digits

                        /// not implemented, prevent usage
                        NfCurrencyEntry( const NfCurrencyEntry& );
                        /// not implemented, prevent usage
    NfCurrencyEntry&	operator=( const NfCurrencyEntry& );

private:

//#if 0 // _SOLAR__PRIVATE
                        // nDecimalFormat := 0, 1, 2
                        // #,##0 or #,##0.00 or #,##0.-- are assigned
    void				Impl_BuildFormatStringNumChars( String&,
                            const LocaleDataWrapper&, USHORT nDecimalFormat ) const;
//#endif	// __PRIVATE

public:

                        NfCurrencyEntry( const LocaleDataWrapper& rLocaleData,
                            LanguageType eLang );
                        NfCurrencyEntry(
                            const ::com::sun::star::i18n::Currency & rCurr,
                            const LocaleDataWrapper& rLocaleData,
                            LanguageType eLang );
                        ~NfCurrencyEntry() {}

                        /// Symbols and language identical
    BOOL				operator==( const NfCurrencyEntry& r ) const;

    const String&		GetSymbol() const			{ return aSymbol; }
    const String&		GetBankSymbol() const		{ return aBankSymbol; }
    LanguageType		GetLanguage() const			{ return eLanguage; }
    USHORT				GetPositiveFormat() const	{ return nPositiveFormat; }
    USHORT				GetNegativeFormat() const	{ return nNegativeFormat; }
    USHORT				GetDigits() const			{ return nDigits; }
    sal_Unicode			GetZeroChar() const			{ return cZeroChar; }

                        /** [$DM-407] (bBank==FALSE) or [$DEM] (bBank==TRUE)
                            is assigned to rStr, if bBank==FALSE and
                            bWithoutExtension==TRUE only [$DM] */
    void				BuildSymbolString( String& rStr, BOOL bBank,
                            BOOL bWithoutExtension = FALSE ) const;

                        /** #,##0.00 [$DM-407] is assigned to rStr, separators
                              from rLoc,	incl. minus sign but without [RED] */
    void				BuildPositiveFormatString( String& rStr, BOOL bBank,
                            const LocaleDataWrapper&, USHORT nDecimalFormat = 1 ) const;
    void				BuildNegativeFormatString( String& rStr, BOOL bBank,
                            const LocaleDataWrapper&, USHORT nDecimalFormat = 1 ) const;

                        /** [$DM-407] (or [$DEM] if bBank==TRUE)
                            is appended/prepended to rStr, incl. minus sign */
    void				CompletePositiveFormatString( String& rStr, BOOL bBank,
                            USHORT nPosiFormat ) const;
    void				CompleteNegativeFormatString( String& rStr, BOOL bBank,
                            USHORT nNegaFormat ) const;

                        /// rSymStr is appended/prepended to rStr, incl. minus sign
    static	void		CompletePositiveFormatString( String& rStr,
                            const String& rSymStr, USHORT nPosiFormat );
    static	void		CompleteNegativeFormatString( String& rStr,
                            const String& rSymStr, USHORT nNegaFormat );

                        /** Representation of a currency (symbol position and
                             negative sign) in other language settings */
    static	USHORT		GetEffectivePositiveFormat( USHORT nIntlFormat,
                            USHORT nCurrFormat, BOOL bBank );
    static	USHORT		GetEffectiveNegativeFormat( USHORT nIntlFormat,
                            USHORT nCurrFormat, BOOL bBank );

                        /// General Unicode Euro symbol
    static inline sal_Unicode	GetEuroSymbol() { return sal_Unicode(0x20AC); }
                        /** Platform and CharSet dependent Euro symbol,
                             needed for import/export */
    static	sal_Char	GetEuroSymbol( rtl_TextEncoding eTextEncoding );
};

typedef NfCurrencyEntry* NfCurrencyEntryPtr;
SV_DECL_PTRARR_DEL( NfCurrencyTable, NfCurrencyEntryPtr, 128, 1 )
typedef String* WSStringPtr;
SV_DECL_PTRARR_DEL_VISIBILITY( NfWSStringsDtor, WSStringPtr, 8, 1,  )


class SvNumberFormatterRegistry_Impl;

class  SvNumberFormatter
{
public:

    /// Preferred ctor with service manager and language/country enum
    SvNumberFormatter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr,
        LanguageType eLang
        );

    ~SvNumberFormatter();

    /// Set CallBack to ColorTable
    void SetColorLink( const Link& rColorTableCallBack )    { aColorLink = rColorTableCallBack; }
    /// Do the CallBack to ColorTable
    Color* GetUserDefColor(USHORT nIndex);

    /// Change language/country, also input and format scanner
    void ChangeIntl( LanguageType eLnge );
    /// Change the reference null date
    void ChangeNullDate(USHORT nDay, USHORT nMonth, USHORT nYear);
    /// Change standard precision
    void ChangeStandardPrec(short nPrec);
    /// Set zero value suppression
    void SetNoZero(BOOL bNZ) { bNoZero = bNZ; }

    /** The language with which the formatter was initialized (system setting),
        NOT the current language after a ChangeIntl() */
    LanguageType GetLanguage() const { return IniLnge; }

    // Determine whether two format types are input compatible or not
    BOOL IsCompatible(short eOldType, short eNewType);

    /** Get table of formats of a specific type of a locale. A format FIndex is
        tested whether it has the type and locale requested, if it doesn't
        match FIndex returns the default format for the type/locale. If no
        specific format is to be selected FIndex may be initialized to 0. */
    SvNumberFormatTable& GetEntryTable(short eType,
                                       sal_uInt32& FIndex,
                                       LanguageType eLnge);

    /** Get table of formats of a specific type of a language/country.
        FIndex returns the default format of that type.
        If the language/country was never touched before new entries are generated */
    SvNumberFormatTable& ChangeCL(short eType,
                                  sal_uInt32& FIndex,
                                  LanguageType eLnge);

    /// Delete an entry including the format it is refering to
    void DeleteEntry(sal_uInt32 nKey);

    /** Create new entry of a format code string for language/country.
        @return
            <TRUE/> if string new and ok and inserted.
             <FALSE/> if string already exists or an unresolvable parse error
             occured, in which case nCheckPos is the error position within rString.
            If the error occurs at position 0 or rString is empty nCheckPos
            will be 1, so an error in the string is always indicated by
             nCheckPos not being zero.
            The content of the rString variable can be changed and corrected
             by the method.
            nType contains the type of the format.
            nKey contains the index key of the format.
     */
    BOOL PutEntry( String& rString, xub_StrLen& nCheckPos, short& nType, sal_uInt32& nKey,
                  LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Same as <method>PutEntry</method> but the format code string is
         considered to be of language/country eLnge and is converted to
        language/country eNewLnge */
    BOOL PutandConvertEntry( String& rString, xub_StrLen& nCheckPos,
                             short& nType, sal_uInt32& nKey,
                             LanguageType eLnge, LanguageType eNewLnge );

    /** Same as <method>PutandConvertEntry</method> but the format code string
         is considered to be of the System language/country eLnge and is
        converted to another System language/country eNewLnge. In this case
         the automatic currency is converted too. */
    BOOL PutandConvertEntrySystem( String& rString, xub_StrLen& nCheckPos,
                             short& nType, sal_uInt32& nKey,
                             LanguageType eLnge, LanguageType eNewLnge );

    /** Create a format code string using format nIndex as a template and
        applying other settings (passed from the dialog) */
    void GenerateFormat( String& sString, sal_uInt32 nIndex,
                        LanguageType eLnge = LANGUAGE_DONTKNOW,
                        BOOL bThousand = FALSE, BOOL IsRed = FALSE,
                        USHORT nPrecision = 0, USHORT nAnzLeading = 1 );

    /** Analyze an input string
        @return
            <TRUE/> if input is a number or is matching a format F_Index
                F_Index is set to a matching format if number, the value is
                returned in fOutNumber
            <FALSE/> if input is not a number
     */
    BOOL IsNumberFormat( const String& sString, sal_uInt32& F_Index, double& fOutNumber );

    /// Format a number according to a format index, return string and color
    void GetOutputString( const double& fOutNumber, sal_uInt32 nFIndex,
                          String& sOutString, Color** ppColor );

    /** Format a string according to a format index, return string and color.
        Formats only if the format code is of type text or the 4th subcode
        of a format code is specified, otherwise sOutString will be == "" */
    void GetOutputString( String& sString, sal_uInt32 nFIndex,
                          String& sOutString, Color** ppColor );

    /** Format a number according to the standard default format matching
        the given format index */
    void GetInputLineString( const double& fOutNumber,
                            sal_uInt32 nFIndex, String& sOutString );

    /** Format a number according to a format code string to be scanned.
        @return
            <FALSE/> if format code contains an error
            <TRUE/> else, in which case the string and color are returned.
     */
    BOOL GetPreviewString( const String& sFormatString, double fPreviewNumber,
                          String& sOutString, Color** ppColor,
                          LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Same as <method>GetPreviewString</method> but the format code string
        may be either language/country eLnge or en_US english US */
    BOOL GetPreviewStringGuess( const String& sFormatString, double fPreviewNumber,
                          String& sOutString, Color** ppColor,
                          LanguageType eLnge = LANGUAGE_DONTKNOW );

    /// Whether format index nFIndex is of type text or not
    BOOL IsTextFormat(sal_uInt32 nFIndex) const;

    /// Load all formats from a stream
    BOOL Load( SvStream& rStream );
    /// Save all formats to a stream
    BOOL Save( SvStream& rStream ) const;

    /// Get additional info of a format index, e.g. for dialog box
    void GetFormatSpecialInfo(sal_uInt32 nFormat, BOOL& bThousand, BOOL& IsRed,
                              USHORT& nPrecision, USHORT& nAnzLeading);

    /// Count of decimals
    USHORT GetFormatPrecision( sal_uInt32 nFormat ) const;

    /** Return the format index of the format code string for language/country,
        or NUMBERFORMAT_ENTRY_NOT_FOUND */
    sal_uInt32 GetEntryKey( const String& sStr, LanguageType eLnge = LANGUAGE_DONTKNOW );

    /// Return the format for a format index
    const SvNumberformat* GetEntry(sal_uInt32 nKey) const
        { return (SvNumberformat*) aFTable.Get(nKey); }

    /// Return the format index of the standard default number format for language/country
    sal_uInt32 GetStandardIndex(LanguageType eLnge = LANGUAGE_DONTKNOW);

    /// Return the format index of the default format of a type for language/country
    sal_uInt32 GetStandardFormat(short eType, LanguageType eLnge = LANGUAGE_DONTKNOW);

    /** Return the format index of the default format of a type for language/country.
        Maybe not the default format but a special builtin format, e.g. for
        NF_TIME_HH_MMSS00, if that format is passed in nFIndex. */
    sal_uInt32 GetStandardFormat( sal_uInt32 nFIndex, short eType, LanguageType eLnge );

    /** Return the format index of the default format of a type for language/country.
        Maybe not the default format but a special builtin format, e.g. for
        NF_TIME_HH_MMSS00, or NF_TIME_HH_MMSS if fNumber >= 1.0  */
    sal_uInt32 GetStandardFormat( double fNumber, sal_uInt32 nFIndex, short eType,
                            LanguageType eLnge );

    /// Whether nFIndex is a special builtin format
    BOOL IsSpecialStandardFormat( sal_uInt32 nFIndex, LanguageType eLnge );

    /// Return the reference date
    Date* GetNullDate();
    /// Return the standard decimal precision
    short GetStandardPrec();
    /// Return whether zero suppression is switched on
    BOOL GetNoZero() { return bNoZero; }
    /** Get the type of a format (or NUMBERFORMAT_UNDEFINED if no entry),
         but with NUMBERFORMAT_DEFINED masked out */
    short GetType(sal_uInt32 nFIndex);

    /// As the name says
    void ClearMergeTable();
    /// Merge in all new entries from rNewTable and return a table of resulting new format indices
    SvNumberFormatterIndexTable* MergeFormatter(SvNumberFormatter& rNewTable);

    /// Whether a merge table is present or not
    inline BOOL HasMergeFmtTbl() const;
    /// Return the new format index for an old format index, if a merge table exists
    inline sal_uInt32 GetMergeFmtIndex( sal_uInt32 nOldFmt ) const;

    /// Return the last used position ever of a language/country combination
    USHORT GetLastInsertKey(sal_uInt32 CLOffset);

    /** Return the format index of a builtin format for a specific language/country.
        If nFormat is not a builtin format nFormat is returned. */
    sal_uInt32 GetFormatForLanguageIfBuiltIn( sal_uInt32 nFormat,
                                    LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Return the format index for a builtin format of a specific language
        @see NfIndexTableOffset
     */
    sal_uInt32 GetFormatIndex( NfIndexTableOffset, LanguageType eLnge = LANGUAGE_DONTKNOW );

    /** Return enum index of a format index of a builtin format,
        NF_INDEX_TABLE_ENTRIES if it's not a builtin format.
        @see NfIndexTableOffset
     */
    NfIndexTableOffset GetIndexTableOffset( sal_uInt32 nFormat ) const;

    /** Set evaluation type and order of input date strings
        @see NfEvalDateFormat
     */
    void SetEvalDateFormat( NfEvalDateFormat eEDF ) { eEvalDateFormat = eEDF; }
    NfEvalDateFormat GetEvalDateFormat() const { return eEvalDateFormat; }

    /** Set TwoDigitYearStart, how the input string scanner handles a two digit year.
        Default from VCL: 1930, 30-99 19xx, 00-29 20xx

        <p> Historically (prior to src513e) it was a two digit number determing
        until which number the string scanner recognizes a year to be 20xx,
        default <= 29 is used by SFX/OfaMiscCfg.
        The name Year2000 is kept although the actual functionality is now a
        TwoDigitYearStart which might be in any century.
     */
    void	SetYear2000( USHORT nVal );
    USHORT	GetYear2000() const;
    static	USHORT	GetYear2000Default();

    USHORT	ExpandTwoDigitYear( USHORT nYear ) const;
    inline	static	USHORT	ExpandTwoDigitYear( USHORT nYear, USHORT nTwoDigitYearStart );

    /// DEPRICATED: Return first character of the decimal separator of the current language/country
    sal_Unicode GetDecSep() const { return GetNumDecimalSep().GetChar(0); }
    /// Return the decimal separator of the current language/country
    String GetDecimalSep() const { return GetNumDecimalSep(); }

    /// Return a <type>SvPtrArr</type> with pointers to <type>NfCurrencyEntry</type> entries
    static const NfCurrencyTable& GetTheCurrencyTable();

    /** Searches, according to the default locale currency, an entry of the
        CurrencyTable which is <bold>not</bold> the first (LANGUAGE_SYSTEM) entry.
        @return
            <NULL/> if not found
            else pointer to <type>NfCurrencyEntry</type>
     */
    static const NfCurrencyEntry*  MatchSystemCurrency();

    /** Return a <type>NfCurrencyEntry</type> matching a language/country.
        If language/country is LANGUAGE_SYSTEM a <method>MatchSystemCurrency</method>
        call is tried to get an entry. If that fails or the corresponding
        language/country is not present the entry for LANGUAGE_SYSTEM is returned.
     */
    static const NfCurrencyEntry&  GetCurrencyEntry( LanguageType );

    /** Set the default system currency. The combination of abbreviation and
        language must match an existent element of theCurrencyTable. If not,
        the SYSTEM (current locale) entry becomes the default.
        This method is meant for the configuration of the default currency.
     */
    static void SetDefaultSystemCurrency( const String& rAbbrev, LanguageType eLang );

    /** Get all standard formats for a specific currency, formats are
        appended to the <type>NfWSStringsDtor</type> list.
        @param bBank
            <TRUE/>: generate only format strings with currency abbreviation
            <FALSE/>: mixed format strings
        @return
            position of default format
     */
    USHORT	GetCurrencyFormatStrings( NfWSStringsDtor&, const NfCurrencyEntry&,
                BOOL bBank ) const;

    /** Whether nFormat is of type NUMBERFORMAT_CURRENCY and the format code
        contains a new SYMBOLTYPE_CURRENCY and if so which one [$xxx-nnn].
        If ppEntry is not NULL and exactly one entry is found, a [$xxx-nnn] is
        returned, even if the format code only contains [$xxx] !
     */
    BOOL	GetNewCurrencySymbolString( sal_uInt32 nFormat, String& rSymbol,
                const NfCurrencyEntry** ppEntry = NULL, BOOL* pBank = NULL ) const;

    /** Look up the corresponding <type>NfCurrencyEntry</type> matching
        rSymbol (may be CurrencySymbol or CurrencyAbbreviation) and possibly
        a rExtension (being yyy of [$xxx-yyy]) or a given language/country
        value. Tries to match a rSymbol with rExtension first, then with
         eFormatLanguage, then rSymbol only. This is because a currency entry
        might have been constructed using I18N locale data where a used locale
        of a currrency format code must not necessarily match the locale of
        the locale data itself, e.g. [$HK$-40C] (being "zh_HK" locale) in
        zh_CN locale data. Here the rExtension would have the value 0x40c but
         eFormatLanguage of the number format would have the value of zh_CN
         locale, the value with which the corresponding CurrencyEntry is
        constructed.

        @param bFoundBank
            Only used for output.
             If the return value is not <NULL/> this value is set to <TRUE/> if
             the matching entry was found by comparing rSymbol against the
             CurrencyAbbreviation (AKA BankSymbol).
             If the return value is <NULL/> the value of bFoundBank is undefined.
        @param rSymbol
            Currency symbol, preferably obtained of a format by a call to
            <method>SvNumberformat::GetNewCurrencySymbol()</method>
        @param rExtension
            Currency extension, preferably obtained of a format by a call to
            <method>SvNumberformat::GetNewCurrencySymbol()</method>
        @param eFormatLanguage
            The language/country value of the format of which rSymbol and
            rExtension are obtained (<method>SvNumberformat::GetLanguage()</method>).
        @param bOnlyStringLanguage
            If <TRUE/> only entries with language/country of rExtension are
            checked, no match on eFormatLanguage. If rExtension is empty all
             entries are checked.
        @return
            The matching entry if unique (in which case bFoundBank is set),
             else <NULL/>.
     */
    static const NfCurrencyEntry* GetCurrencyEntry( BOOL & bFoundBank,
                const String& rSymbol, const String& rExtension,
                LanguageType eFormatLanguage, BOOL bOnlyStringLanguage = FALSE );

    /// Get compatibility ("automatic" old style) currency from I18N locale data
    void GetCompatibilityCurrency( String& rSymbol, String& rAbbrev ) const;

    /// Fill rList with the language/country codes that have been allocated
    void	GetUsedLanguages( SvUShorts& rList );

    /** Return a keyword for a language/country and <type>NfKeywordIndex</type>
        for XML import, to generate number format strings. */
    String GetKeyword( LanguageType eLnge, USHORT nIndex );

    /** Return the GENERAL keyword in proper case ("General") for a
        language/country, used in XML import */
    String GetStandardName( LanguageType eLnge );

    /// Skip a NumberFormatter in stream, Chart needs this
    static void SkipNumberFormatterInStream( SvStream& );


private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;
    ::com::sun::star::lang::Locale aLocale;
    SvNumberFormatTable aFTable;			// Table of format keys to format entries
    Table aDefaultFormatKeys;				// Table of default standard to format keys
    SvNumberFormatTable* pFormatTable;		// For the UI dialog
    SvNumberFormatterIndexTable* pMergeTable;				// List of indices for merging two formatters
    CharClass* pCharClass;					// CharacterClassification
    OnDemandLocaleDataWrapper xLocaleData;  // LocaleData switched between SYSTEM, ENGLISH and other
    OnDemandTransliterationWrapper xTransliteration;    // Transliteration loaded on demand
    OnDemandCalendarWrapper xCalendar;      // Calendar loaded on demand
    OnDemandNativeNumberWrapper xNatNum;    // Native number service loaded on demand
    ImpSvNumberInputScan* pStringScanner;	// Input string scanner
    ImpSvNumberformatScan* pFormatScanner;	// Format code string scanner
    Link aColorLink;                        // User defined color table CallBack
    sal_uInt32 MaxCLOffset;						// Max language/country offset used
    sal_uInt32 nDefaultSystemCurrencyFormat;		// NewCurrency matching SYSTEM locale
    LanguageType IniLnge;                   // Initialized setting language/country
    LanguageType ActLnge;                   // Current setting language/country
    NfEvalDateFormat eEvalDateFormat;		// DateFormat evaluation
    BOOL bNoZero;							// Zero value suppression

    // cached locale data items needed almost any time
    String aDecimalSep;
    String aThousandSep;
    String aDateSep;

#ifdef _ZFORLIST_CXX				// ----- private Methoden -----

    static BOOL 			bCurrencyTableInitialized;
    static USHORT			nSystemCurrencyPosition;
    static SvNumberFormatterRegistry_Impl* pFormatterRegistry;

    // get the registry, create one if none exists
    static SvNumberFormatterRegistry_Impl& GetFormatterRegistry();

    // called by ctors
    void ImpConstruct( LanguageType eLang );

    // Changes initialized language/country, clears the entries and generates
    // new ones, may ONLY be called by the binary file format load
    void ImpChangeSysCL( LanguageType eLnge, BOOL bLoadingSO5 );

    // Generate builtin formats provided by i18n behind CLOffset,
    // if bLoadingSO5==FALSE also generate additional i18n formats.
    void ImpGenerateFormats( sal_uInt32 CLOffset, BOOL bLoadingSO5 );

    // Generate additional formats provided by i18n
    void ImpGenerateAdditionalFormats(
                sal_uInt32 CLOffset,
                NumberFormatCodeWrapper& rNumberFormatCode,
                BOOL bAfterLoadingSO5 );

    SvNumberformat* ImpInsertFormat(
                const ::com::sun::star::i18n::NumberFormatCode& rCode,
                sal_uInt32 nPos,
                BOOL bAfterLoadingSO5 = FALSE,
                sal_Int16 nOrgIndex = 0 );
    // ImpInsertNewStandardFormat for new (since version ...) builtin formats
    SvNumberformat* ImpInsertNewStandardFormat(
                const ::com::sun::star::i18n::NumberFormatCode& rCode,
                sal_uInt32 nPos,
                USHORT nVersion,
                BOOL bAfterLoadingSO5 = FALSE,
                sal_Int16 nOrgIndex = 0 );

    // Return CLOffset or (MaxCLOffset + SV_COUNTRY_LANGUAGE_OFFSET) if new language/country
    sal_uInt32 ImpGetCLOffset(LanguageType eLnge) const;

    // Test whether format code already exists, then return index key,
    // otherwise NUMBERFORMAT_ENTRY_NOT_FOUND
    sal_uInt32 ImpIsEntry( const String& rString,
                        sal_uInt32 CLOffset,
                        LanguageType eLnge );

    // Create builtin formats for language/country if necessary, return CLOffset
    sal_uInt32 ImpGenerateCL( LanguageType eLnge, BOOL bLoadingSO5 = FALSE );

    // Build negative currency format, old compatibility style
    void ImpGetNegCurrFormat( String& sNegStr, const String& rCurrSymbol );
    // Build positive currency format, old compatibility style
    void ImpGetPosCurrFormat( String& sPosStr, const String& rCurrSymbol );

    // Create <type>theCurrencyTable</type> with all <type>NfCurrencyEntry</type>
    static void ImpInitCurrencyTable();

    // Return the format index of the currency format of the system locale.
    // Format is created if not already present.
    sal_uInt32	ImpGetDefaultSystemCurrencyFormat();

    // Return the format index of the currency format of the current locale.
    // Format is created if not already present.
    sal_uInt32	ImpGetDefaultCurrencyFormat();

    // Return the default format for a given type and current locale.
    // May ONLY be called from within GetStandardFormat().
    sal_uInt32	ImpGetDefaultFormat( short nType );

    // Return the index in a sequence of format codes matching an enum of
    // NfIndexTableOffset. If not found 0 is returned. If the sequence doesn't
    // contain any format code elements a default element is created and inserted.
    sal_Int32 ImpGetFormatCodeIndex(
        ::com::sun::star::uno::Sequence< ::com::sun::star::i18n::NumberFormatCode >& rSeq,
        const NfIndexTableOffset nTabOff );

    // Adjust a sequence of format codes to contain only one (THE) default
    // instead of multiple defaults for short/medium/long types.
    // If there is no medium but a short and a long default the long is taken.
    // Return the default index in the sequence.
    // Non-PRODUCT version may check locale data for matching defaults in one
    // FormatElement group.
    sal_Int32 ImpAdjustFormatCodeDefault(
        ::com::sun::star::i18n::NumberFormatCode * pFormatArr,
        sal_Int32 nCount, BOOL bCheckCorrectness = TRUE
        );

    // used as a loop body inside of GetNewCurrencySymbolString() and GetCurrencyEntry()
#ifndef DBG_UTIL
    inline
#endif
        static BOOL ImpLookupCurrencyEntryLoopBody(
            const NfCurrencyEntry*& pFoundEntry, BOOL& bFoundBank,
            const NfCurrencyEntry* pData, USHORT nPos, const String& rSymbol );

    // link to be set at <method>SvtSysLocaleOptions::SetCurrencyChangeLink()</method>
    DECL_STATIC_LINK( SvNumberFormatter, CurrencyChangeLink, void* );

#endif // _ZFORLIST_CXX

public:

    // own static mutex, may also be used by internal class SvNumberFormatterRegistry_Impl
    static ::osl::Mutex&        GetMutex();

    // called by SvNumberFormatterRegistry_Impl::Notify if the default system currency changes
    void ResetDefaultSystemCurrency();

    // Replace the SYSTEM language/country format codes. Called upon change of
    // the user configurable locale.
    // Old compatibility codes are replaced, user defined are converted, and
    // new format codes are appended.
    void ReplaceSystemCL( LanguageType eOldLanguage );

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory >
        GetServiceManager() const { return xServiceManager; }



    //! The following method is not to be used from outside but must be
    //! public for the InputScanner.
    // return the current FormatScanner
    inline const ImpSvNumberformatScan* GetFormatScanner() const { return pFormatScanner; }



    //! The following methods are not to be used from outside but must be
    //! public for the InputScanner and FormatScanner.

    // return current (!) Locale
    inline const ::com::sun::star::lang::Locale& GetLocale() const { return aLocale; }

    // return corresponding Transliteration wrapper
    inline const ::utl::TransliterationWrapper* GetTransliteration() const
                { return xTransliteration.get(); }

    // return corresponding Transliteration wrapper with loadModuleByImplName()
    inline const ::utl::TransliterationWrapper* GetTransliterationForModule(
            const String& rModule, LanguageType eLang ) const
                { return xTransliteration.getForModule( rModule, eLang ); }

    // return the corresponding CharacterClassification wrapper
    inline const CharClass* GetCharClass() const { return pCharClass; }

    // return the corresponding LocaleData wrapper
    inline const LocaleDataWrapper* GetLocaleData() const { return xLocaleData.get(); }

    // return the corresponding Calendar wrapper
    inline CalendarWrapper* GetCalendar() const { return xCalendar.get(); }

    // return the corresponding NativeNumberSupplier wrapper
    inline const NativeNumberWrapper* GetNatNum() const { return xNatNum.get(); }

    // cached locale data items

    // return the corresponding decimal separator
    inline const String& GetNumDecimalSep() const { return aDecimalSep; }

    // return the corresponding group (AKA thousand) separator
    inline const String& GetNumThousandSep() const { return aThousandSep; }

    // return the corresponding date separator
    inline const String& GetDateSep() const { return aDateSep; }

};


// --------------------------- inline --------------------------------------

inline sal_uInt32 SvNumberFormatter::GetMergeFmtIndex( sal_uInt32 nOldFmt ) const
{
    sal_uInt32* pU = (pMergeTable && pMergeTable->Count()) ? (sal_uInt32*)pMergeTable->Get( nOldFmt ) : 0;
    return pU ? *pU : nOldFmt;
}

inline BOOL SvNumberFormatter::HasMergeFmtTbl() const
{
    return pMergeTable && (0 != pMergeTable->Count());
}


// static
inline USHORT SvNumberFormatter::ExpandTwoDigitYear(
            USHORT nYear, USHORT nTwoDigitYearStart )
{
    if ( nYear < 100 )
    {
        if ( nYear < (nTwoDigitYearStart % 100) )
            return nYear + (((nTwoDigitYearStart / 100) + 1) * 100);
        else
            return nYear + ((nTwoDigitYearStart / 100) * 100);
    }
    return nYear;
}

}

#endif	// _ZFORLIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
