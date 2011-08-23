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

#ifndef OOX_XLS_ADDRESSCONVERTER_HXX
#define OOX_XLS_ADDRESSCONVERTER_HXX

#include <vector>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

class BiffInputStream;
class BiffOutputStream;

// ============================================================================
// ============================================================================

/** A vector of com.sun.star.table.CellRangeAddress elements and additional
    functionality. */
class ApiCellRangeList : public ::std::vector< ::com::sun::star::table::CellRangeAddress >
{
public:
    inline explicit     ApiCellRangeList() {}

    /** Returns the base address of this range list (top-left cell of first range). */
    ::com::sun::star::table::CellAddress
                        getBaseAddress() const;
};

// ============================================================================

/** A 2D cell address struct for binary filters. */
struct BinAddress
{
    sal_Int32           mnCol;
    sal_Int32           mnRow;

    inline explicit     BinAddress() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit     BinAddress( sal_Int32 nCol, sal_Int32 nRow ) : mnCol( nCol ), mnRow( nRow ) {}
    inline explicit     BinAddress( const ::com::sun::star::table::CellAddress& rAddr ) : mnCol( rAddr.Column ), mnRow( rAddr.Row ) {}

    inline void         set( sal_Int32 nCol, sal_Int32 nRow ) { mnCol = nCol; mnRow = nRow; }
    inline void         set( const ::com::sun::star::table::CellAddress& rAddr ) { mnCol = rAddr.Column; mnRow = rAddr.Row; }

    void                read( RecordInputStream& rStrm );
    void                read( BiffInputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
    void                write( BiffOutputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false ) const;
};

// ----------------------------------------------------------------------------

inline bool operator==( const BinAddress& rL, const BinAddress& rR )
{
    return (rL.mnCol == rR.mnCol) && (rL.mnRow == rR.mnRow);
}

inline bool operator<( const BinAddress& rL, const BinAddress& rR )
{
    return (rL.mnCol < rR.mnCol) || ((rL.mnCol == rR.mnCol) && (rL.mnRow < rR.mnRow));
}

inline RecordInputStream& operator>>( RecordInputStream& rStrm, BinAddress& orPos )
{
    orPos.read( rStrm );
    return rStrm;
}

inline BiffInputStream& operator>>( BiffInputStream& rStrm, BinAddress& orPos )
{
    orPos.read( rStrm );
    return rStrm;
}

inline BiffOutputStream& operator<<( BiffOutputStream& rStrm, const BinAddress& rPos )
{
    rPos.write( rStrm );
    return rStrm;
}

// ============================================================================

/** A 2D cell range address struct for binary filters. */
struct BinRange
{
    BinAddress          maFirst;
    BinAddress          maLast;

    inline explicit     BinRange() {}
    inline explicit     BinRange( const BinAddress& rAddr ) : maFirst( rAddr ), maLast( rAddr ) {}
    inline explicit     BinRange( const BinAddress& rFirst, const BinAddress& rLast ) : maFirst( rFirst ), maLast( rLast ) {}
    inline explicit     BinRange( sal_Int32 nCol1, sal_Int32 nRow1, sal_Int32 nCol2, sal_Int32 nRow2 ) :
                            maFirst( nCol1, nRow1 ), maLast( nCol2, nRow2 ) {}
    inline explicit     BinRange( const ::com::sun::star::table::CellAddress& rAddr ) : maFirst( rAddr ), maLast( rAddr ) {}
    inline explicit     BinRange( const ::com::sun::star::table::CellAddress& rFirst, const ::com::sun::star::table::CellAddress& rLast ) : maFirst( rFirst ), maLast( rLast ) {}
    inline explicit     BinRange( const ::com::sun::star::table::CellRangeAddress& rRange ) : maFirst( rRange.StartColumn, rRange.StartRow ), maLast( rRange.EndColumn, rRange.EndRow ) {}

    inline void         set( const BinAddress& rFirst, const BinAddress& rLast )
                            { maFirst = rFirst; maLast = rLast; }
    inline void         set( sal_Int32 nCol1, sal_Int32 nRow1, sal_Int32 nCol2, sal_Int32 nRow2 )
                            { maFirst.set( nCol1, nRow1 ); maLast.set( nCol2, nRow2 ); }
    inline void         set( const ::com::sun::star::table::CellAddress& rFirst, const ::com::sun::star::table::CellAddress& rLast )
                            { maFirst.set( rFirst ); maLast.set( rLast ); }
    inline void         set( const ::com::sun::star::table::CellRangeAddress& rRange )
                            { maFirst.set( rRange.StartColumn, rRange.StartRow ); maLast.set( rRange.EndColumn, rRange.EndRow ); }

    inline sal_Int32    getColCount() const { return maLast.mnCol - maFirst.mnCol + 1; }
    inline sal_Int32    getRowCount() const { return maLast.mnRow - maFirst.mnRow + 1; }
    bool                contains( const BinAddress& rAddr ) const;

    void                read( RecordInputStream& rStrm );
    void                read( BiffInputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
    void                write( BiffOutputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false ) const;
};

// ----------------------------------------------------------------------------

inline bool operator==( const BinRange& rL, const BinRange& rR )
{
    return (rL.maFirst == rR.maFirst) && (rL.maLast == rR.maLast);
}

inline bool operator<( const BinRange& rL, const BinRange& rR )
{
    return (rL.maFirst < rR.maFirst) || ((rL.maFirst == rR.maFirst) && (rL.maLast < rR.maLast));
}

inline RecordInputStream& operator>>( RecordInputStream& rStrm, BinRange& orRange )
{
    orRange.read( rStrm );
    return rStrm;
}

inline BiffInputStream& operator>>( BiffInputStream& rStrm, BinRange& orRange )
{
    orRange.read( rStrm );
    return rStrm;
}

inline BiffOutputStream& operator<<( BiffOutputStream& rStrm, const BinRange& rRange )
{
    rRange.write( rStrm );
    return rStrm;
}

// ============================================================================

/** A 2D cell range address list for binary filters. */
class BinRangeList : public ::std::vector< BinRange >
{
public:
    inline explicit     BinRangeList() {}

    BinRange            getEnclosingRange() const;

    void                read( RecordInputStream& rStrm );
    void                read( BiffInputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
    void                write( BiffOutputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false ) const;
    void                writeSubList( BiffOutputStream& rStrm,
                            size_t nBegin, size_t nCount, bool bCol16Bit = true, bool bRow32Bit = false ) const;
};

// ----------------------------------------------------------------------------

inline RecordInputStream& operator>>( RecordInputStream& rStrm, BinRangeList& orRanges )
{
    orRanges.read( rStrm );
    return rStrm;
}

inline BiffInputStream& operator>>( BiffInputStream& rStrm, BinRangeList& orRanges )
{
    orRanges.read( rStrm );
    return rStrm;
}

inline BiffOutputStream& operator<<( BiffOutputStream& rStrm, const BinRangeList& rRanges )
{
    rRanges.write( rStrm );
    return rStrm;
}

// ============================================================================

/** Different target types that can be encoded in a BIFF URL. */
enum BiffTargetType
{
    BIFF_TARGETTYPE_URL,            /// URL, URL with sheet name, or sheet name.
    BIFF_TARGETTYPE_SAMESHEET,      /// Target for special '!A1' syntax to refer to current sheet.
    BIFF_TARGETTYPE_LIBRARY,        /// Library directory in application installation.
    BIFF_TARGETTYPE_DDE_OLE,        /// DDE server/topic or OLE class/target.
    BIFF_TARGETTYPE_UNKNOWN         /// Unknown/unsupported target type.
};

// ============================================================================
// ============================================================================

/** Converter for cell addresses and cell ranges for OOX and BIFF filters.
 */
class AddressConverter : public WorkbookHelper
{
public:
    explicit            AddressConverter( const WorkbookHelper& rHelper );

    // ------------------------------------------------------------------------

    /** Tries to parse the passed string for a 2d cell address in A1 notation.

        This function accepts all strings that match the regular expression
        "[a-zA-Z]{1,6}0*[1-9][0-9]{0,8}" (without quotes), i.e. 1 to 6 letters
        for the column index (translated to 0-based column indexes from 0 to
        321,272,405), and 1 to 9 digits for the 1-based row index (translated
        to 0-based row indexes from 0 to 999,999,998). The row number part may
        contain leading zeros, they will be ignored. It is up to the caller to
        handle cell addresses outside of a specific valid range (e.g. the
        entire spreadsheet).

        @param ornColumn  (out-parameter) Returns the converted column index.
        @param ornRow  (out-parameter) returns the converted row index.
        @param rString  The string containing the cell address.
        @param nStart  Start index of string part in rString to be parsed.
        @param nLength  Length of string part in rString to be parsed.

        @return  true = Parsed string was valid, returned values can be used.
     */
    static bool         parseOoxAddress2d(
                            sal_Int32& ornColumn, sal_Int32& ornRow,
                            const ::rtl::OUString& rString,
                            sal_Int32 nStart = 0,
                            sal_Int32 nLength = SAL_MAX_INT32 );

    /** Tries to parse the passed string for a 2d cell range in A1 notation.

        This function accepts all strings that match the regular expression
        "ADDR(:ADDR)?" (without quotes), where ADDR is a cell address accepted
        by the parseOoxAddress2d() function of this class. It is up to the
        caller to handle cell ranges outside of a specific valid range (e.g.
        the entire spreadsheet).

        @param ornStartColumn  (out-parameter) Returns the converted start column index.
        @param ornStartRow  (out-parameter) returns the converted start row index.
        @param ornEndColumn  (out-parameter) Returns the converted end column index.
        @param ornEndRow  (out-parameter) returns the converted end row index.
        @param rString  The string containing the cell address.
        @param nStart  Start index of string part in rString to be parsed.
        @param nLength  Length of string part in rString to be parsed.

        @return  true = Parsed string was valid, returned values can be used.
     */
    static bool         parseOoxRange2d(
                            sal_Int32& ornStartColumn, sal_Int32& ornStartRow,
                            sal_Int32& ornEndColumn, sal_Int32& ornEndRow,
                            const ::rtl::OUString& rString,
                            sal_Int32 nStart = 0,
                            sal_Int32 nLength = SAL_MAX_INT32 );

    /** Tries to parse an encoded name of an external link target in BIFF
        documents, e.g. from EXTERNSHEET or SUPBOOK records.

        @param orClassName  (out-parameter) DDE server name or OLE class name.
        @param orTargetUrl  (out-parameter) Target URL, DDE topic or OLE object name.
        @param orSheetName  (out-parameter) Sheet name in target document.
        @param rBiffEncoded  Encoded name of the external link target.
        @param bFromDConRec  True = path from DCONREF/DCONNAME/DCONBINAME records, false = other records.

        @return  Type of the decoded target.
      */
    BiffTargetType      parseBiffTargetUrl(
                            ::rtl::OUString& orClassName,
                            ::rtl::OUString& orTargetUrl,
                            ::rtl::OUString& orSheetName,
                            const ::rtl::OUString& rBiffTargetUrl,
                            bool bFromDConRec = false );

    // ------------------------------------------------------------------------

    /** Returns the biggest valid cell address in the own Calc document. */
    inline const ::com::sun::star::table::CellAddress&
                        getMaxApiAddress() const { return maMaxApiPos; }

    /** Returns the biggest valid cell address in the imported/exported
        Excel document. */
    inline const ::com::sun::star::table::CellAddress&
                        getMaxXlsAddress() const { return maMaxXlsPos; }

    /** Returns the biggest valid cell address in both Calc and the
        imported/exported Excel document. */
    inline const ::com::sun::star::table::CellAddress&
                        getMaxAddress() const { return maMaxPos; }

    /** Returns the column overflow status. */
    inline bool         isColOverflow() const { return mbColOverflow; }
    /** Returns the row overflow status. */
    inline bool         isRowOverflow() const { return mbRowOverflow; }
    /** Returns the sheet overflow status. */
    inline bool         isTabOverflow() const { return mbTabOverflow; }

    // ------------------------------------------------------------------------

    /** Checks if the passed column index is valid.

        @param nCol  The column index to check.
        @param bTrackOverflow  true = Update the internal overflow flag, if the
            column index is outside of the supported limits.
        @return  true = Passed column index is valid (no index overflow).
     */
    bool                checkCol( sal_Int32 nCol, bool bTrackOverflow );

    /** Checks if the passed row index is valid.

        @param nRow  The row index to check.
        @param bTrackOverflow  true = Update the internal overflow flag, if the
            row index is outside of the supported limits.
        @return  true = Passed row index is valid (no index overflow).
     */
    bool                checkRow( sal_Int32 nRow, bool bTrackOverflow );

    /** Checks if the passed sheet index is valid.

        @param nSheet  The sheet index to check.
        @param bTrackOverflow  true = Update the internal overflow flag, if the
            sheet index is outside of the supported limits.
        @return  true = Passed sheet index is valid (no index overflow).
     */
    bool                checkTab( sal_Int16 nSheet, bool bTrackOverflow );

    // ------------------------------------------------------------------------

    /** Checks the passed cell address if it fits into the spreadsheet limits.

        @param rAddress  The cell address to be checked.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  true = Passed address is valid (no index overflow).
     */
    bool                checkCellAddress(
                            const ::com::sun::star::table::CellAddress& rAddress,
                            bool bTrackOverflow );

    /** Converts the passed string to a single cell address, without checking
        any sheet limits.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rString  Cell address string in A1 notation.
        @param nSheet  Sheet index to be inserted into orAddress.
        @return  true = Cell address could be parsed from the passed string.
     */
    bool                convertToCellAddressUnchecked(
                            ::com::sun::star::table::CellAddress& orAddress,
                            const ::rtl::OUString& rString,
                            sal_Int16 nSheet );

    /** Tries to convert the passed string to a single cell address.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rString  Cell address string in A1 notation.
        @param nSheet  Sheet index to be inserted into orAddress (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  true = Converted address is valid (no index overflow).
     */
    bool                convertToCellAddress(
                            ::com::sun::star::table::CellAddress& orAddress,
                            const ::rtl::OUString& rString,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    /** Returns a valid cell address by moving it into allowed dimensions.

        @param rString  Cell address string in A1 notation.
        @param nSheet  Sheet index for the returned address (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  A valid API cell address struct. */
    ::com::sun::star::table::CellAddress
                        createValidCellAddress(
                            const ::rtl::OUString& rString,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    /** Converts the passed address to a single cell address, without checking
        any sheet limits.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rBinAddress  Binary cell address struct.
        @param nSheet  Sheet index to be inserted into orAddress.
     */
    void                convertToCellAddressUnchecked(
                            ::com::sun::star::table::CellAddress& orAddress,
                            const BinAddress& rBinAddress,
                            sal_Int16 nSheet );

    /** Tries to convert the passed address to a single cell address.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rBinAddress  Binary cell address struct.
        @param nSheet  Sheet index to be inserted into orAddress (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  true = Converted address is valid (no index overflow).
     */
    bool                convertToCellAddress(
                            ::com::sun::star::table::CellAddress& orAddress,
                            const BinAddress& rBinAddress,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    /** Returns a valid cell address by moving it into allowed dimensions.

        @param rBinAddress  Binary cell address struct.
        @param nSheet  Sheet index for the returned address (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  A valid API cell address struct. */
    ::com::sun::star::table::CellAddress
                        createValidCellAddress(
                            const BinAddress& rBinAddress,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    // ------------------------------------------------------------------------

    /** Checks the passed cell range if it fits into the spreadsheet limits.

        @param rRange  The cell range address to be checked.
        @param bAllowOverflow  true = Allow ranges that start inside the
            supported sheet limits but may end outside of these limits.
            false = Do not allow ranges that overflow the supported limits.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the passed range contains cells outside of the supported sheet
            limits.
        @return  true = Cell range is valid. This function returns also true,
            if only parts of the range are outside the current sheet limits and
            such an overflow is allowed via parameter bAllowOverflow. Returns
            false, if the entire range is outside the sheet limits, or if
            overflow is not allowed via parameter bAllowOverflow.
     */
    bool                checkCellRange(
                            const ::com::sun::star::table::CellRangeAddress& rRange,
                            bool bAllowOverflow, bool bTrackOverflow );

    /** Checks the passed cell range, may try to fit it to current sheet limits.

        First, this function reorders the column and row indexes so that the
        starting indexes are less than or equal to the end indexes. Then,
        depending on the parameter bAllowOverflow, the range is just checked or
        cropped to the current sheet limits.

        @param orRange  (in-out-parameter) Converts the passed cell range
            into a valid cell range address. If the passed range contains cells
            outside the currently supported spreadsheet limits, it will be
            cropped to these limits.
        @param bAllowOverflow  true = Allow ranges that start inside the
            supported sheet limits but may end outside of these limits. The
            cell range returned in orRange will be cropped to these limits.
            false = Do not allow ranges that overflow the supported limits. The
            function will return false when the range overflows the sheet limits.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the original range contains cells outside of the supported sheet
            limits.
        @return  true = Converted range address is valid. This function
            returns also true, if overflowing ranges are allowed via parameter
            bAllowOverflow and the range has been cropped, but still contains
            cells inside the current sheet limits. Returns false, if the entire
            range is outside the sheet limits or overflowing ranges are not
            allowed via parameter bAllowOverflow.
     */
    bool                validateCellRange(
                            ::com::sun::star::table::CellRangeAddress& orRange,
                            bool bAllowOverflow, bool bTrackOverflow );

    /** Converts the passed string to a cell range address, without checking
        any sheet limits.

        @param orRange  (out-parameter) Returns the converted range address.
        @param rString  Cell range string in A1 notation.
        @param nSheet  Sheet index to be inserted into orRange.
        @return  true = Range address could be parsed from the passed string.
     */
    bool                convertToCellRangeUnchecked(
                            ::com::sun::star::table::CellRangeAddress& orRange,
                            const ::rtl::OUString& rString,
                            sal_Int16 nSheet );

    /** Tries to convert the passed string to a cell range address.

        @param orRange  (out-parameter) Returns the converted cell range
            address. If the original range in the passed string contains cells
            outside the currently supported spreadsheet limits, and parameter
            bAllowOverflow is set to true, the range will be cropped to these
            limits. Example: the range string "A1:ZZ100000" may be converted to
            the range A1:IV65536.
        @param rString  Cell range string in A1 notation.
        @param nSheet  Sheet index to be inserted into orRange (will be checked).
        @param bAllowOverflow  true = Allow ranges that start inside the
            supported sheet limits but may end outside of these limits. The
            cell range returned in orRange will be cropped to these limits.
            false = Do not allow ranges that overflow the supported limits.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the original range contains cells outside of the supported sheet
            limits.
        @return  true = Converted and returned range is valid. This function
            returns also true, if overflowing ranges are allowed via parameter
            bAllowOverflow and the range has been cropped, but still contains
            cells inside the current sheet limits. Returns false, if the entire
            range is outside the sheet limits or overflowing ranges are not
            allowed via parameter bAllowOverflow.
     */
    bool                convertToCellRange(
                            ::com::sun::star::table::CellRangeAddress& orRange,
                            const ::rtl::OUString& rString,
                            sal_Int16 nSheet,
                            bool bAllowOverflow, bool bTrackOverflow );

    /** Converts the passed range to a cell range address, without checking any
        sheet limits.

        @param orRange  (out-parameter) Returns the converted range address.
        @param rBinRange  Binary cell range struct.
        @param nSheet  Sheet index to be inserted into orRange.
     */
    void                convertToCellRangeUnchecked(
                            ::com::sun::star::table::CellRangeAddress& orRange,
                            const BinRange& rBinRange,
                            sal_Int16 nSheet );

    /** Tries to convert the passed range to a cell range address.

        @param orRange  (out-parameter) Returns the converted cell range
            address. If the passed original range contains cells outside the
            currently supported spreadsheet limits, and parameter bAllowOverflow
            is set to true, the range will be cropped to these limits.
        @param rBinRange  Binary cell range struct.
        @param nSheet  Sheet index to be inserted into orRange (will be checked).
        @param bAllowOverflow  true = Allow ranges that start inside the
            supported sheet limits but may end outside of these limits. The
            cell range returned in orRange will be cropped to these limits.
            false = Do not allow ranges that overflow the supported limits.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the original range contains cells outside of the supported sheet
            limits.
        @return  true = Converted and returned range is valid. This function
            returns also true, if overflowing ranges are allowed via parameter
            bAllowOverflow and the range has been cropped, but still contains
            cells inside the current sheet limits. Returns false, if the entire
            range is outside the sheet limits or if overflowing ranges are not
            allowed via parameter bAllowOverflow.
     */
    bool                convertToCellRange(
                            ::com::sun::star::table::CellRangeAddress& orRange,
                            const BinRange& rBinRange,
                            sal_Int16 nSheet,
                            bool bAllowOverflow, bool bTrackOverflow );

    // ------------------------------------------------------------------------

    /** Checks the passed cell range list if it fits into the spreadsheet limits.

        @param rRanges  The cell range list to be checked.
        @param bAllowOverflow  true = Allow ranges that start inside the
            supported sheet limits but may end outside of these limits.
            false = Do not allow ranges that overflow the supported limits.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the passed range list contains cells outside of the supported sheet
            limits.
        @return  true = All cell ranges are valid. This function returns also
            true, if overflowing ranges are allowed via parameter bAllowOverflow
            and only parts of the ranges are outside the current sheet limits.
            Returns false, if one of the ranges is completely outside the sheet
            limits or if overflowing ranges are not allowed via parameter
            bAllowOverflow.
     */
    bool                checkCellRangeList(
                            const ApiCellRangeList& rRanges,
                            bool bAllowOverflow, bool bTrackOverflow );

    /** Tries to restrict the passed cell range list to current sheet limits.

        @param orRanges  (in-out-parameter) Restricts the cell range addresses
            in the passed list to the current sheet limits and removes invalid
            ranges from the list.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the original ranges contain cells outside of the supported sheet
            limits.
     */
    void                validateCellRangeList(
                            ApiCellRangeList& orRanges,
                            bool bTrackOverflow );

    /** Tries to convert the passed string to a cell range list.

        @param orRanges  (out-parameter) Returns the converted cell range
            addresses. If a range in the passed string contains cells outside
            the currently supported spreadsheet limits, it will be cropped to
            these limits. Example: the range string "A1:ZZ100000" may be
            converted to the range A1:IV65536. If a range is completely outside
            the limits, it will be omitted.
        @param rString  Cell range list string in A1 notation, space separated.
        @param nSheet  Sheet index to be inserted into orRanges (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the original ranges contain cells outside of the supported sheet
            limits.
     */
    void                convertToCellRangeList(
                            ApiCellRangeList& orRanges,
                            const ::rtl::OUString& rString,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    /** Tries to convert the passed range list to a cell range list.

        @param orRanges  (out-parameter) Returns the converted cell range
            addresses. If a range in the passed string contains cells outside
            the currently supported spreadsheet limits, it will be cropped to
            these limits. Example: the range string "A1:ZZ100000" may be
            converted to the range A1:IV65536. If a range is completely outside
            the limits, it will be omitted.
        @param rBinRanges  List of binary cell range objects.
        @param nSheet  Sheet index to be inserted into orRanges (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the original ranges contain cells outside of the supported sheet
            limits.
     */
    void                convertToCellRangeList(
                            ApiCellRangeList& orRanges,
                            const BinRangeList& rBinRanges,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    // ------------------------------------------------------------------------
private:
    void                initializeMaxPos(
                            sal_Int16 nMaxXlsTab, sal_Int32 nMaxXlsCol, sal_Int32 nMaxXlsRow );

private:
    struct ControlCharacters
    {
        sal_Unicode         mcThisWorkbook;             /// Control character: Link to current workbook.
        sal_Unicode         mcExternal;                 /// Control character: Link to external workbook/sheet.
        sal_Unicode         mcThisSheet;                /// Control character: Link to current sheet.
        sal_Unicode         mcInternal;                 /// Control character: Link to internal sheet.
        sal_Unicode         mcSameSheet;                /// Control character: Link to same sheet (special '!A1' syntax).

        void                set(
                                sal_Unicode cThisWorkbook, sal_Unicode cExternal,
                                sal_Unicode cThisSheet, sal_Unicode cInternal,
                                sal_Unicode cSameSheet );
    };

    ::com::sun::star::table::CellAddress maMaxApiPos;   /// Maximum valid cell address in Calc.
    ::com::sun::star::table::CellAddress maMaxXlsPos;   /// Maximum valid cell address in Excel.
    ::com::sun::star::table::CellAddress maMaxPos;      /// Maximum valid cell address in Calc/Excel.
    ControlCharacters   maLinkChars;                    /// Control characters for external link import (BIFF).
    ControlCharacters   maDConChars;                    /// Control characters for DCON* record import (BIFF).
    bool                mbColOverflow;                  /// Flag for "columns overflow".
    bool                mbRowOverflow;                  /// Flag for "rows overflow".
    bool                mbTabOverflow;                  /// Flag for "tables overflow".
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

