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

#ifndef SC_XLTOOLS_HXX
#define SC_XLTOOLS_HXX

#include "address.hxx"
#include "ftools.hxx"

class SfxObjectShell;

// BIFF versions ==============================================================

#define DBG_ERROR_BIFF()        DBG_ERRORFILE( "Unknown BIFF type!" )
#define DBG_ASSERT_BIFF( c )    DBG_ASSERT( c, "Unknown BIFF type!" )

// Enumerations ===============================================================

/** An enumeration for all Excel error codes and the values true and false. */
enum XclBoolError
{
    xlErrNull,                  /// The error code #NULL!
    xlErrDiv0,                  /// The error code #DIV/0!
    xlErrValue,                 /// The error code #VALUE!
    xlErrRef,                   /// The error code #REF!
    xlErrName,                  /// The error code #NAME?
    xlErrNum,                   /// The error code #NUM!
    xlErrNA,                    /// The error code #N/A!
    xlErrTrue,                  /// The Boolean value true.
    xlErrFalse,                 /// The Boolean value false.
    xlErrUnknown                /// For unknown codes and values.
};

// GUID import/export =========================================================

class XclImpStream;
class XclExpStream;

/** This struct stores a GUID (class ID) and supports reading, writing and comparison. */
struct XclGuid
{
    sal_uInt8           mpnData[ 16 ];  /// Stores GUID always in little endian.

    explicit            XclGuid();
    explicit            XclGuid(
                            sal_uInt32 nData1,
                            sal_uInt16 nData2, sal_uInt16 nData3,
                            sal_uInt8 nData41, sal_uInt8 nData42,
                            sal_uInt8 nData43, sal_uInt8 nData44,
                            sal_uInt8 nData45, sal_uInt8 nData46,
                            sal_uInt8 nData47, sal_uInt8 nData48 );
};

bool operator==( const XclGuid& rCmp1, const XclGuid& rCmp2 );
inline bool operator!=( const XclGuid& rCmp1, const XclGuid& rCmp2 ) { return !(rCmp1 == rCmp2); }
bool operator<( const XclGuid& rCmp1, const XclGuid& rCmp2 );

XclImpStream& operator>>( XclImpStream& rStrm, XclGuid& rGuid );
XclExpStream& operator<<( XclExpStream& rStrm, const XclGuid& rGuid );

// Excel Tools ================================================================

class SvStream;
class ScDocument;

/** This class contains static helper methods for the Excel import and export filters. */
class XclTools : ScfNoInstance
{
public:
    // GUID's -----------------------------------------------------------------

    static const XclGuid maGuidStdLink;     /// GUID of StdLink (HLINK record).
    static const XclGuid maGuidUrlMoniker;  /// GUID of URL moniker (HLINK record).
    static const XclGuid maGuidFileMoniker; /// GUID of file moniker (HLINK record).

    // numeric conversion -----------------------------------------------------

    /** Calculates the double value from an RK value (encoded integer or double). */
    static double       GetDoubleFromRK( sal_Int32 nRKValue );
    /** Calculates an RK value (encoded integer or double) from a double value.
        @param rnRKValue  Returns the calculated RK value.
        @param fValue  The double value.
        @return  true = An RK value could be created. */
    static bool         GetRKFromDouble( sal_Int32& rnRKValue, double fValue );

    /** Calculates an angle (in 1/100 of degrees) from an Excel angle value.
        @param nRotForStacked  This value will be returned, if nXclRot contains 'stacked'. */
    static sal_Int32    GetScRotation( sal_uInt16 nXclRot, sal_Int32 nRotForStacked );
    /** Calculates the Excel angle value from an angle in 1/100 of degrees. */
    static sal_uInt8    GetXclRotation( sal_Int32 nScRot );

    /** Calculates BIFF8 rotation angle from BIFF2-BIFF5 text orientation. */
    static sal_uInt8    GetXclRotFromOrient( sal_uInt8 nXclOrient );
    /** Calculates BIFF2-BIFF5 text orientation from BIFF8 rotation angle. */
    static sal_uInt8    GetXclOrientFromRot( sal_uInt16 nXclRot );

    /** Converts a Calc error code to an Excel error code. */
    static sal_uInt8    GetXclErrorCode( sal_uInt16 nScError );
    /** Converts an Excel error code to a Calc error code. */
    static sal_uInt16       GetScErrorCode( sal_uInt8 nXclError );

    /** Converts the passed BIFF error to a double containing the respective Calc error code. */
    static double       ErrorToDouble( sal_uInt8 nXclError );
    /** Gets a translated error code or Boolean value from Excel error codes.
        @param rfDblValue  Returns 0.0 for error codes or the value of a Boolean (0.0 or 1.0).
        @param bErrorOrBool  false = nError is a Boolean value; true = is an error value.
        @param nValue  The error code or Boolean value. */
    static XclBoolError ErrorToEnum( double& rfDblValue, sal_uInt8 bErrOrBool, sal_uInt8 nValue );

    /** Returns the length in twips calculated from a length in inches. */
    static sal_uInt16   GetTwipsFromInch( double fInches );
    /** Returns the length in twips calculated from a length in 1/100 mm. */
    static sal_uInt16   GetTwipsFromHmm( sal_Int32 nHmm );

    /** Returns the length in inches calculated from a length in twips. */
    static double       GetInchFromTwips( sal_Int32 nTwips );
    /** Returns the length in inches calculated from a length in 1/100 mm. */
    static double       GetInchFromHmm( sal_Int32 nHmm );

    /** Returns the length in 1/100 mm calculated from a length in inches. */
    static sal_Int32    GetHmmFromInch( double fInches );
    /** Returns the length in 1/100 mm calculated from a length in twips. */
    static sal_Int32    GetHmmFromTwips( sal_Int32 nTwips );

    /** Returns the Calc column width (twips) for the passed Excel width.
        @param nScCharWidth  Width of the '0' character in Calc (twips). */
    static sal_uInt16       GetScColumnWidth( sal_uInt16 nXclWidth, long nScCharWidth );
    /** Returns the Excel column width for the passed Calc width (twips).
        @param nScCharWidth  Width of the '0' character in Calc (twips). */
    static sal_uInt16   GetXclColumnWidth( sal_uInt16 nScWidth, long nScCharWidth );

    /** Returns a correction value to convert column widths from/to default column widths.
        @param nXclDefFontHeight  Excel height of application default font. */
    static double       GetXclDefColWidthCorrection( long nXclDefFontHeight );

    // formatting -------------------------------------------------------------

    /** Returns the best fitting color for an Excel pattern area format. */
    static Color        GetPatternColor( const Color& rPattColor, const Color& rBackColor, sal_uInt16 nXclPattern );

    // text encoding ----------------------------------------------------------

    /** Returns a text encoding from an Excel code page.
        @return  The corresponding text encoding or RTL_TEXTENCODING_DONTKNOW. */
    static rtl_TextEncoding GetTextEncoding( sal_uInt16 nCodePage );

    /** Returns an Excel code page from a text encoding. */
    static sal_uInt16   GetXclCodePage( rtl_TextEncoding eTextEnc );

    // font names -------------------------------------------------------------

    /** Returns the matching Excel font name for a passed Calc font name. */
    static String       GetXclFontName( const String& rFontName );

    // built-in defined names -------------------------------------------------

    /** Returns the raw English UI representation of a built-in defined name used in NAME records.
        @param cBuiltIn  Excel index of the built-in name. */
    static String       GetXclBuiltInDefName( sal_Unicode cBuiltIn );
    /** Returns the Calc UI representation of a built-in defined name used in NAME records.
        @descr  Adds a prefix to the representation returned by GetXclBuiltInDefName().
        @param cBuiltIn  Excel index of the built-in name. */
    static String       GetBuiltInDefName( sal_Unicode cBuiltIn );
    /** Returns the Excel built-in name index of the passed defined name from Calc.
        @descr  Ignores any characters following a valid representation of a built-in name.
        @param pcBuiltIn  (out-param) If not 0, the index of the built-in name will be returned here.
        @return  true = passed string is a built-in name; false = user-defined name. */
    static sal_Unicode  GetBuiltInDefNameIndex( const String& rDefName );

    // built-in style names ---------------------------------------------------

    /** Returns the specified built-in cell style name.
        @param nStyleId  The identifier of the built-in style.
        @param rName  Default name for unknown styles.
        @param nLevel  The zero-based outline level for RowLevel and ColLevel styles.
        @return  The style name or an empty string, if the parameters are not valid. */
    static String       GetBuiltInStyleName( sal_uInt8 nStyleId, const String& rName, sal_uInt8 nLevel );
    /** Returns the passed style name with a special built-in prefix. */
    static String       GetBuiltInStyleName( const String& rStyleName );
    /** Returns true, if the passed string is a name of an Excel built-in style.
        @param pnStyleId  If not 0, the found style identifier will be returned here.
        @param pnNextChar  If not 0, the index of the char after the evaluated substring will be returned here. */
    static bool         IsBuiltInStyleName( const String& rStyleName, sal_uInt8* pnStyleId = 0, xub_StrLen* pnNextChar = 0 );
    /** Returns the Excel built-in style identifier of a passed style name.
        @param rnStyleId  The style identifier is returned here.
        @param rnLevel  The zero-based outline level for RowLevel and ColLevel styles is returned here.
        @param rStyleName  The style name to examine.
        @return  true = passed string is a built-in style name, false = user style. */
    static bool         GetBuiltInStyleId(
                            sal_uInt8& rnStyleId, sal_uInt8& rnLevel,
                            const String& rStyleName );

    // conditional formatting style names -------------------------------------

    /** Returns the style name for a single condition of a conditional formatting.
        @param nScTab  The current Calc sheet index.
        @param nFormat  The zero-based index of the conditional formatting.
        @param nCondition  The zero-based index of the condition.
        @return  A style sheet name in the form "Excel_CondFormat_<sheet>_<format>_<condition>". */
    static String       GetCondFormatStyleName( SCTAB nScTab, sal_Int32 nFormat, sal_uInt16 nCondition );
    /** Returns true, if the passed string is a name of a conditional format style created by Excel import.
        @param pnNextChar  If not 0, the index of the char after the evaluated substring will be returned here. */
    static bool         IsCondFormatStyleName( const String& rStyleName, xub_StrLen* pnNextChar = 0 );

    // stream handling --------------------------------------------------------

    /** Skips a substream (BOF/EOF record block). Includes all embedded substreams. */
    static void         SkipSubStream( XclImpStream& rStrm );

    // Basic macro names ------------------------------------------------------

    /** Returns the full StarBasic macro URL from an Excel macro name. */
    static ::rtl::OUString GetSbMacroUrl( const String& rMacroName, SfxObjectShell* pDocShell = 0 );
    /** Returns the full StarBasic macro URL from an Excel module and macro name. */
    static ::rtl::OUString GetSbMacroUrl( const String& rModuleName, const String& rMacroName, SfxObjectShell* pDocShell = 0 );
    /** Returns the Excel macro name from a full StarBasic macro URL. */
    static String       GetXclMacroName( const ::rtl::OUString& rSbMacroUrl );

// ------------------------------------------------------------------------
private:
    static const String maDefNamePrefix;            /// Prefix for built-in defined names.
    static const String maStyleNamePrefix1;         /// Prefix for built-in cell style names.
    static const String maStyleNamePrefix2;         /// Prefix for built-in cell style names from OOX filter.
    static const String maCFStyleNamePrefix1;       /// Prefix for cond. formatting style names.
    static const String maCFStyleNamePrefix2;       /// Prefix for cond. formatting style names from OOX filter.
    static const ::rtl::OUString maSbMacroPrefix;   /// Prefix for StarBasic macros.
    static const ::rtl::OUString maSbMacroSuffix;   /// Suffix for StarBasic macros.
};

// read/write colors ----------------------------------------------------------

/** Reads a color from the passed stream.
    @descr  The color has the format (all values 8-bit): Red, Green, Blue, 0. */
XclImpStream& operator>>( XclImpStream& rStrm, Color& rColor );

/** Reads a color to the passed stream.
    @descr  The color has the format (all values 8-bit): Red, Green, Blue, 0. */
XclExpStream& operator<<( XclExpStream& rStrm, const Color& rColor );

// ============================================================================

#endif

