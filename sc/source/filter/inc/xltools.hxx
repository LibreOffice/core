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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLTOOLS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLTOOLS_HXX

#include "address.hxx"
#include "ftools.hxx"

class SfxObjectShell;

// BIFF versions ==============================================================

#define DBG_ERROR_BIFF()        OSL_FAIL( "Unknown BIFF type!" )
#define OSL_ENSURE_BIFF( c )    OSL_ENSURE( c, "Unknown BIFF type!" )

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

XclImpStream& operator>>( XclImpStream& rStrm, XclGuid& rGuid );
XclExpStream& operator<<( XclExpStream& rStrm, const XclGuid& rGuid );

// Excel Tools ================================================================

/** This class contains static helper methods for the Excel import and export filters. */
class XclTools
{
public:
    // noncopyable nonconstructable -------------------------------------------

    XclTools(const XclTools&) = delete;
    const XclTools& operator=(const XclTools&) = delete;
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static items. */
    XclTools() = delete;


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
    static XclBoolError ErrorToEnum( double& rfDblValue, bool bErrOrBool, sal_uInt8 nValue );

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
     *  Excel Column width is stored as 1/256th of a character.
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
    static OUString GetXclFontName( const OUString& rFontName );

    // built-in defined names -------------------------------------------------

    /** Returns the raw English UI representation of a built-in defined name used in NAME records.
        @param cBuiltIn  Excel index of the built-in name. */
    static OUString GetXclBuiltInDefName( sal_Unicode cBuiltIn );
    /** Returns the Calc UI representation of a built-in defined name used in NAME records.
        @descr  Adds a prefix to the representation returned by GetXclBuiltInDefName().
        @param cBuiltIn  Excel index of the built-in name. */
    static OUString GetBuiltInDefName( sal_Unicode cBuiltIn );
    /** Returns the Excel built-in name with OOXML prefix
        @descr  Adds the "_xlnm." prefix to the representation returned by GetXclBuiltInDefName()
        @param cBuiltIn  Excel index of the built in name.*/
    static OUString GetBuiltInDefNameXml( sal_Unicode cBuiltIn );
    /** Returns the Excel built-in name index of the passed defined name from Calc.
        @descr  Ignores any characters following a valid representation of a built-in name.
        @param pcBuiltIn  (out-param) If not 0, the index of the built-in name will be returned here.
        @return  true = passed string is a built-in name; false = user-defined name. */
    static sal_Unicode  GetBuiltInDefNameIndex( const OUString& rDefName );

    // built-in style names ---------------------------------------------------

    /** Returns the specified built-in cell style name.
        @param nStyleId  The identifier of the built-in style.
        @param rName  Default name for unknown styles.
        @param nLevel  The zero-based outline level for RowLevel and ColLevel styles.
        @return  The style name or an empty string, if the parameters are not valid. */
    static OUString GetBuiltInStyleName( sal_uInt8 nStyleId, const OUString& rName, sal_uInt8 nLevel );

    /** Returns true, if the passed string is a name of an Excel built-in style.
        @param pnStyleId  If not 0, the found style identifier will be returned here.
        @param pnNextChar  If not 0, the index of the char after the evaluated substring will be returned here. */
    static bool         IsBuiltInStyleName( const OUString& rStyleName, sal_uInt8* pnStyleId = nullptr, sal_Int32* pnNextChar = nullptr );
    /** Returns the Excel built-in style identifier of a passed style name.
        @param rnStyleId  The style identifier is returned here.
        @param rnLevel  The zero-based outline level for RowLevel and ColLevel styles is returned here.
        @param rStyleName  The style name to examine.
        @return  true = passed string is a built-in style name, false = user style. */
    static bool         GetBuiltInStyleId(
                            sal_uInt8& rnStyleId, sal_uInt8& rnLevel,
                            const OUString& rStyleName );

    // conditional formatting style names -------------------------------------

    /** Returns the style name for a single condition of a conditional formatting.
        @param nScTab  The current Calc sheet index.
        @param nFormat  The zero-based index of the conditional formatting.
        @param nCondition  The zero-based index of the condition.
        @return  A style sheet name in the form "Excel_CondFormat_<sheet>_<format>_<condition>". */
    static OUString GetCondFormatStyleName( SCTAB nScTab, sal_Int32 nFormat, sal_uInt16 nCondition );
    /** Returns true, if the passed string is a name of a conditional format style created by Excel import.
        @param pnNextChar  If not 0, the index of the char after the evaluated substring will be returned here. */
    static bool         IsCondFormatStyleName( const OUString& rStyleName );

    // stream handling --------------------------------------------------------

    /** Skips a substream (BOF/EOF record block). Includes all embedded substreams. */
    static void         SkipSubStream( XclImpStream& rStrm );

    // Basic macro names ------------------------------------------------------

    /** Returns the full StarBasic macro URL from an Excel macro name. */
    static OUString GetSbMacroUrl( const OUString& rMacroName, SfxObjectShell* pDocShell = nullptr );
    /** Returns the Excel macro name from a full StarBasic macro URL. */
    static OUString GetXclMacroName( const OUString& rSbMacroUrl );

private:
    static const OUString maDefNamePrefix;      /// Prefix for built-in defined names.
    static const OUString maDefNamePrefixXml;   /// Prefix for built-in defined names for OOX
    static const OUString maStyleNamePrefix1;   /// Prefix for built-in cell style names.
    static const OUString maStyleNamePrefix2;   /// Prefix for built-in cell style names from OOX filter.
    static const OUString maCFStyleNamePrefix1; /// Prefix for cond. formatting style names.
    static const OUString maCFStyleNamePrefix2; /// Prefix for cond. formatting style names from OOX filter.
    static const OUString maSbMacroPrefix;   /// Prefix for StarBasic macros.
    static const OUString maSbMacroSuffix;   /// Suffix for StarBasic macros.
};

// read/write colors ----------------------------------------------------------

/** Reads a color from the passed stream.
    @descr  The color has the format (all values 8-bit): Red, Green, Blue, 0. */
XclImpStream& operator>>( XclImpStream& rStrm, Color& rColor );

/** Reads a color to the passed stream.
    @descr  The color has the format (all values 8-bit): Red, Green, Blue, 0. */
XclExpStream& operator<<( XclExpStream& rStrm, const Color& rColor );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
