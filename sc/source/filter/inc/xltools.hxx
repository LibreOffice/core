/*************************************************************************
 *
 *  $RCSfile: xltools.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:25:01 $
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

// ============================================================================

#ifndef SC_XLTOOLS_HXX
#define SC_XLTOOLS_HXX

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif
#ifndef SC_XLCONST_HXX
#include "xlconst.hxx"
#endif


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
    sal_uInt8                   mpData[ 16 ];   /// Stores GUID always in little endian.

    explicit                    XclGuid();
    explicit                    XclGuid(
                                    sal_uInt32 nData1,
                                    sal_uInt16 nData2, sal_uInt16 nData3,
                                    sal_uInt8 nData41, sal_uInt8 nData42,
                                    sal_uInt8 nData43, sal_uInt8 nData44,
                                    sal_uInt8 nData45, sal_uInt8 nData46,
                                    sal_uInt8 nData47, sal_uInt8 nData48 );
};

bool operator==( const XclGuid& rCmp1, const XclGuid& rCmp2 );
inline bool operator!=( const XclGuid& rCmp1, const XclGuid& rCmp2 ) { return !(rCmp1 == rCmp2); }

XclImpStream& operator>>( XclImpStream& rStrm, XclGuid& rGuid );
XclExpStream& operator<<( XclExpStream& rStrm, const XclGuid& rGuid );


// Excel Tools ================================================================

class SvStream;
class ScDocument;
class ScAddress;
class ScRange;
class ScRangeList;

/** This class contains static helper methods for the Excel import and export filters. */
class XclTools : ScfNoInstance
{
public:
// GUID's ---------------------------------------------------------------------

    static const XclGuid        maGuidStdLink;      /// GUID of StdLink (HLINK record).
    static const XclGuid        maGuidUrlMoniker;   /// GUID of URL moniker (HLINK record).
    static const XclGuid        maGuidFileMoniker;  /// GUID of file moniker (HLINK record).

// numeric conversion ---------------------------------------------------------

    /** Calculates the double value from an RK value (encoded integer or double). */
    static double               GetDoubleFromRK( sal_Int32 nRKValue );
    /** Calculates an RK value (encoded integer or double) from a double value.
        @param rnRKValue  Returns the calculated RK value.
        @param fValue  The double value.
        @return  true = An RK value could be created. */
    static bool                 GetRKFromDouble( sal_Int32& rnRKValue, double fValue );

    /** Calculates an angle (in 1/100 of degrees) from an Excel angle value. */
    static sal_Int32            GetScRotation( sal_uInt16 nXclRot );
    /** Calculates the Excel angle value from an angle in 1/100 of degrees. */
    static sal_uInt8            GetXclRotation( sal_Int32 nScRot );

    /** Converts a Calc error code to an Excel error code. */
    static sal_uInt8            GetXclErrorCode( USHORT nScError );
    /** Converts an Excel error code to a Calc error code. */
    static USHORT               GetScErrorCode( sal_uInt8 nXclError );

    /** Gets a translated error code or Boolean value from Excel error codes.
        @param rfDblValue  Returns 0.0 for error codes or the value of a Boolean (0.0 or 1.0).
        @param bErrorOrBool  false = nError is a Boolean value; true = is an error value.
        @param nValue  The error code or Boolean value. */
    static XclBoolError         ErrorToEnum( double& rfDblValue, sal_uInt8 bErrOrBool, sal_uInt8 nValue );

    /** Returns the length in twips calculated from a length in inches. */
    static sal_uInt16           GetTwipsFromInch( double fInches );
    /** Returns the length in twips calculated from a length in 1/100 mm. */
    static sal_uInt16           GetTwipsFromHmm( sal_Int32 nHmm );

    /** Returns the length in inches calculated from a length in twips. */
    static double               GetInchFromTwips( sal_Int32 nTwips );
    /** Returns the length in inches calculated from a length in 1/100 mm. */
    static double               GetInchFromHmm( sal_Int32 nHmm );

    /** Returns the length in 1/100 mm calculated from a length in inches. */
    static sal_Int32            GetHmmFromInch( double fInches );
    /** Returns the length in 1/100 mm calculated from a length in twips. */
    static sal_Int32            GetHmmFromTwips( sal_Int32 nTwips );

    /** Returns the Calc column width (twips) for the passed Excel width.
        @param nScCharWidth  Width of the '0' character in Calc (twips). */
    static sal_uInt16           GetScColumnWidth( sal_uInt16 nXclWidth, long nScCharWidth );
    /** Returns the Excel column width for the passed Calc width (twips).
        @param nScCharWidth  Width of the '0' character in Calc (twips). */
    static sal_uInt16           GetXclColumnWidth( sal_uInt16 nScWidth, long nScCharWidth );

// text encoding --------------------------------------------------------------

    /** Returns a text encoding from an Excel code page.
        @return  The corresponding text encoding or RTL_TEXTENCODING_DONTKNOW. */
    static rtl_TextEncoding     GetTextEncoding( sal_uInt16 nCodePage );
    /** Returns an Excel code page from a text encoding. */
    static sal_uInt16           GetXclCodePage( rtl_TextEncoding eTextEnc );

// font names -----------------------------------------------------------------

    /** Returns the matching Excel font name for a passed Calc font name. */
    static String               GetXclFontName( const String& rFontName );

// built-in defined names -----------------------------------------------------

    /** Returns the raw English UI representation of a built-in defined name used in NAME records.
        @param nBuiltInIndex  Excel index of the built-in name. */
    static String               GetXclBuiltInDefName( sal_Unicode nBuiltInIndex );
    /** Returns the Calc UI representation of a built-in defined name used in NAME records.
        @descr  Adds a prefix to the representation returned by GetXclBuiltInDefName().
        @param nBuiltInIndex  Excel index of the built-in name. */
    static String               GetBuiltInDefName( sal_Unicode nBuiltInIndex );
    /** Tests on valid built-in name with sheet index.
        @param rnSheet  Here the parsed sheet index is returned.
        @param rString  The string to be determined.
        @param nIndex  Index to built-in name to be compared with the string.
        @return  true = The string is valid. */
    static bool                 IsBuiltInDefName( sal_uInt16& rnSheet, const String& rName, sal_Unicode nIndex );
    /** Returns the Excel built-in name index of the passed defined name from Calc.
        @descr  Ignores any characters following a valid representation of a built-in name.
        @param pnBuiltInIndex  0, the index of the built-in name will be returned here.
        @return  true = passed string is a built-in name, false = user-defined name. */
    static bool                 IsBuiltInDefName( const String& rDefName, sal_Unicode* pnBuiltInIndex = NULL );

// built-in style names -------------------------------------------------------

    /** Returns the specified built-in cell style name.
        @param nStyleId  The identifier of the built-in style.
        @param nLevel  The zero-based outline level for RowLevel and ColLevel styles.
        @return  The style name or an empty string, if the parameters are not valid. */
    static String               GetBuiltInStyleName( sal_uInt8 nStyleId, sal_uInt8 nLevel );
    /** Returns true, if the passed string is a name of an Excel built-in style.
        @param pnStyleId  If not 0, the found style identifier will be returned here.
        @param pnNextChar  If not 0, the index of the char after the evaluated substring will be returned here. */
    static bool                 IsBuiltInStyleName( const String& rStyleName, sal_uInt8* pnStyleId = NULL, xub_StrLen* pnNextChar = NULL );
    /** Returns the Excel built-in style identifier of a passed style name.
        @param rnStyleId  The style identifier is returned here.
        @param rnLevel  The zero-based outline level for RowLevel and ColLevel styles is returned here.
        @param rStyleName  The style name to examine.
        @return  true = passed string is a built-in style name, false = user style. */
    static bool                 GetBuiltInStyleId(
                                    sal_uInt8& rnStyleId, sal_uInt8& rnLevel,
                                    const String& rStyleName );
    /** Returns the XF index of a built-in style. */
    static sal_uInt16           GetBuiltInXFIndex( sal_uInt8 nStyleId, sal_uInt8 nLevel );

// conditional formatting style names -----------------------------------------

    /** Returns the style name for a single condition of a conditional formatting.
        @param nScTab  The current Calc sheet index.
        @param nFormat  The zero-based index of the conditional formatting.
        @param nCondition  The zero-based index of the condition.
        @return  A style sheet name in the form "Excel_CondFormat_<sheet>_<format>_<condition>". */
    static String               GetCondFormatStyleName( SCTAB nScTab, sal_Int32 nFormat, sal_uInt16 nCondition );
    /** Returns true, if the passed string is a name of a conditional format style created by Excel import.
        @param pnNextChar  If not 0, the index of the char after the evaluated substring will be returned here. */
    static bool                 IsCondFormatStyleName( const String& rStyleName, xub_StrLen* pnNextChar = NULL );

// ----------------------------------------------------------------------------

private:
    static const String         maDefNamePrefix;        /// Prefix for built-in defined names.
    static const String         maStyleNamePrefix;      /// Prefix for built-in cell style names.
    static const String         maCFStyleNamePrefix;    /// Prefix for cond. formatting style names.
};


// read/write range lists -----------------------------------------------------

/** Reads a cell range list.
    @descr  The list has the following format (all values 16 bit):
    (range count); n * (first row; last row; first column; last column).
    The new ranges are appended to the cell range list. */
XclImpStream& operator>>( XclImpStream& rStrm, ScRangeList& rRanges );

/** Writes a cell range list.
    @descr  The list has the following format (all values 16 bit):
    (range count); n * (first row; last row; first column; last column). */
XclExpStream& operator<<( XclExpStream& rStrm, const ScRangeList& rRanges );


// Rich-string formatting runs ================================================

/** Represents a formatting run for rich-strings.
    @descr  An Excel formatting run stores the first formatted character in a
    rich-string and the index of a font used to format this and the following
    characters. */
struct XclFormatRun
{
    sal_uInt16                  mnChar;
    sal_uInt16                  mnFontIx;

    explicit inline             XclFormatRun() : mnChar( 0 ), mnFontIx( 0 ) {}
    explicit inline             XclFormatRun( sal_uInt16 nChar, sal_uInt16 nFontIx ) :
                                    mnChar( nChar ), mnFontIx( nFontIx ) {}
};

inline bool operator==( const XclFormatRun& rLeft, const XclFormatRun& rRight )
{
    return (rLeft.mnChar == rRight.mnChar) && (rLeft.mnFontIx == rRight.mnFontIx);
}

inline bool operator<( const XclFormatRun& rLeft, const XclFormatRun& rRight )
{
    return (rLeft.mnChar < rRight.mnChar) || ((rLeft.mnChar == rRight.mnChar) && (rLeft.mnFontIx < rRight.mnFontIx));
}


// ----------------------------------------------------------------------------

/** A vector with all formatting runs for a rich-string. */
typedef ::std::vector< XclFormatRun > XclFormatRunVec;


// ============================================================================

#endif

