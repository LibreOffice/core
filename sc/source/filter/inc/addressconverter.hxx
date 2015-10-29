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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_ADDRESSCONVERTER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_ADDRESSCONVERTER_HXX

#include <vector>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include "workbookhelper.hxx"

namespace oox {
namespace xls {

class BiffInputStream;

/** A vector of com.sun.star.table.CellRangeAddress elements and additional
    functionality. */
class ApiCellRangeList
{
public:
    inline explicit     ApiCellRangeList() : mvAddresses() {}

    size_t size() const { return mvAddresses.size(); }

    bool empty() const { return mvAddresses.empty(); }

    const css::table::CellRangeAddress& front() const
    { return mvAddresses.front(); }

    css::table::CellRangeAddress& operator[]( size_t i )
    { return mvAddresses[ i ]; }

    ::std::vector< css::table::CellRangeAddress >::const_iterator begin() const
    { return mvAddresses.begin(); }
    ::std::vector< css::table::CellRangeAddress >::iterator begin()
    { return mvAddresses.begin(); }

    ::std::vector< css::table::CellRangeAddress >::const_iterator end() const
    { return mvAddresses.end(); }

    ::std::vector< css::table::CellRangeAddress >::reverse_iterator rbegin()
    { return mvAddresses.rbegin(); }

    ::std::vector< css::table::CellRangeAddress >::reverse_iterator rend()
    { return mvAddresses.rend(); }

    void clear() { mvAddresses.clear(); }

    void erase( ::std::vector< css::table::CellRangeAddress >::iterator it )
    { mvAddresses.erase( it ); }

    void pop_back() { mvAddresses.pop_back(); }

    void push_back( const css::table::CellRangeAddress& rAddress )
    { mvAddresses.push_back( rAddress ); }

    /** Returns the base address of this range list (top-left cell of first range). */
    css::table::CellAddress
                        getBaseAddress() const;

    /** Converts to a sequence. */
    css::uno::Sequence< css::table::CellRangeAddress >
    toSequence() const;

private:
    ::std::vector< css::table::CellRangeAddress > mvAddresses;
};

/** A 2D cell address struct for binary filters. */
struct BinAddress
{
    sal_Int32           mnCol;
    sal_Int32           mnRow;

    inline explicit     BinAddress() : mnCol( 0 ), mnRow( 0 ) {}
    inline explicit     BinAddress( sal_Int32 nCol, sal_Int32 nRow ) : mnCol( nCol ), mnRow( nRow ) {}
    inline explicit     BinAddress( const css::table::CellAddress& rAddr ) : mnCol( rAddr.Column ), mnRow( rAddr.Row ) {}

    void                read( SequenceInputStream& rStrm );
    void                read( BiffInputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
};

inline bool operator==( const BinAddress& rL, const BinAddress& rR )
{
    return (rL.mnCol == rR.mnCol) && (rL.mnRow == rR.mnRow);
}

inline bool operator<( const BinAddress& rL, const BinAddress& rR )
{
    return (rL.mnCol < rR.mnCol) || ((rL.mnCol == rR.mnCol) && (rL.mnRow < rR.mnRow));
}

inline SequenceInputStream& operator>>( SequenceInputStream& rStrm, BinAddress& orPos )
{
    orPos.read( rStrm );
    return rStrm;
}

inline BiffInputStream& operator>>( BiffInputStream& rStrm, BinAddress& orPos )
{
    orPos.read( rStrm );
    return rStrm;
}

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
    inline explicit     BinRange( const css::table::CellAddress& rAddr ) : maFirst( rAddr ), maLast( rAddr ) {}
    inline explicit     BinRange( const css::table::CellAddress& rFirst, const css::table::CellAddress& rLast ) : maFirst( rFirst ), maLast( rLast ) {}
    inline explicit     BinRange( const css::table::CellRangeAddress& rRange ) : maFirst( rRange.StartColumn, rRange.StartRow ), maLast( rRange.EndColumn, rRange.EndRow ) {}

    void                read( SequenceInputStream& rStrm );
    void                read( BiffInputStream& rStrm, bool bCol16Bit = true, bool bRow32Bit = false );
};

inline SequenceInputStream& operator>>( SequenceInputStream& rStrm, BinRange& orRange )
{
    orRange.read( rStrm );
    return rStrm;
}

inline BiffInputStream& operator>>( BiffInputStream& rStrm, BinRange& orRange )
{
    orRange.read( rStrm );
    return rStrm;
}

/** A 2D cell range address list for binary filters. */
class BinRangeList
{
public:
    inline explicit     BinRangeList() : mvRanges() {}

    ::std::vector< BinRange >::const_iterator begin() const { return mvRanges.begin(); }
    ::std::vector< BinRange >::const_iterator end() const { return mvRanges.end(); }

    void                read( SequenceInputStream& rStrm );

private:
    ::std::vector< BinRange > mvRanges;
};

inline SequenceInputStream& operator>>( SequenceInputStream& rStrm, BinRangeList& orRanges )
{
    orRanges.read( rStrm );
    return rStrm;
}

/** Different target types that can be encoded in a BIFF URL. */
enum BiffTargetType
{
    BIFF_TARGETTYPE_URL,            /// URL, URL with sheet name, or sheet name.
    BIFF_TARGETTYPE_SAMESHEET,      /// Target for special '!A1' syntax to refer to current sheet.
    BIFF_TARGETTYPE_LIBRARY,        /// Library directory in application installation.
    BIFF_TARGETTYPE_DDE_OLE,        /// DDE server/topic or OLE class/target.
    BIFF_TARGETTYPE_UNKNOWN         /// Unknown/unsupported target type.
};

/** Converter for cell addresses and cell ranges for OOXML and BIFF filters.
 */
class AddressConverter : public WorkbookHelper
{
public:
    explicit            AddressConverter( const WorkbookHelper& rHelper );

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
                            const OUString& rString,
                            sal_Int32 nStart = 0,
                            sal_Int32 nLength = SAL_MAX_INT32 );

    static bool parseOoxAddress2d(
        sal_Int32& ornColumn, sal_Int32& ornRow, const char* pStr );

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
                            const OUString& rString,
                            sal_Int32 nStart = 0,
                            sal_Int32 nLength = SAL_MAX_INT32 );

    /** Returns the biggest valid cell address in the own Calc document. */
    inline const css::table::CellAddress&
                        getMaxApiAddress() const { return maMaxApiPos; }

    /** Returns the biggest valid cell address in the imported/exported
        Excel document. */
    inline const css::table::CellAddress&
                        getMaxXlsAddress() const { return maMaxXlsPos; }

    /** Returns the biggest valid cell address in both Calc and the
        imported/exported Excel document. */
    inline const css::table::CellAddress&
                        getMaxAddress() const { return maMaxPos; }

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

    /** Checks the passed cell address if it fits into the spreadsheet limits.

        @param rAddress  The cell address to be checked.
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  true = Passed address is valid (no index overflow).
     */
    bool                checkCellAddress(
                            const css::table::CellAddress& rAddress,
                            bool bTrackOverflow );

    /** Converts the passed string to a single cell address, without checking
        any sheet limits.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rString  Cell address string in A1 notation.
        @param nSheet  Sheet index to be inserted into orAddress.
        @return  true = Cell address could be parsed from the passed string.
     */
    static bool        convertToCellAddressUnchecked(
                            css::table::CellAddress& orAddress,
                            const OUString& rString,
                            sal_Int16 nSheet );

    static bool convertToCellAddressUnchecked(
        css::table::CellAddress& orAddress, const char* pStr, sal_Int16 nSheet );

    /** Tries to convert the passed string to a single cell address.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rString  Cell address string in A1 notation.
        @param nSheet  Sheet index to be inserted into orAddress (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  true = Converted address is valid (no index overflow).
     */
    bool                convertToCellAddress(
                            css::table::CellAddress& orAddress,
                            const OUString& rString,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    bool convertToCellAddress(
        css::table::CellAddress& rAddress,
        const char* pStr, sal_Int16 nSheet, bool bTrackOverflow );

    /** Returns a valid cell address by moving it into allowed dimensions.

        @param rString  Cell address string in A1 notation.
        @param nSheet  Sheet index for the returned address (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  A valid API cell address struct. */
    css::table::CellAddress
                        createValidCellAddress(
                            const OUString& rString,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    /** Converts the passed address to a single cell address, without checking
        any sheet limits.

        @param orAddress  (out-parameter) Returns the converted cell address.
        @param rBinAddress  Binary cell address struct.
        @param nSheet  Sheet index to be inserted into orAddress.
     */
    static void        convertToCellAddressUnchecked(
                            css::table::CellAddress& orAddress,
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
                            css::table::CellAddress& orAddress,
                            const BinAddress& rBinAddress,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

    /** Returns a valid cell address by moving it into allowed dimensions.

        @param rBinAddress  Binary cell address struct.
        @param nSheet  Sheet index for the returned address (will be checked).
        @param bTrackOverflow  true = Update the internal overflow flags, if
            the address is outside of the supported sheet limits.
        @return  A valid API cell address struct. */
    css::table::CellAddress
                        createValidCellAddress(
                            const BinAddress& rBinAddress,
                            sal_Int16 nSheet,
                            bool bTrackOverflow );

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
                            const css::table::CellRangeAddress& rRange,
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
                            css::table::CellRangeAddress& orRange,
                            bool bAllowOverflow, bool bTrackOverflow );

    /** Converts the passed string to a cell range address, without checking
        any sheet limits.

        @param orRange  (out-parameter) Returns the converted range address.
        @param rString  Cell range string in A1 notation.
        @param nSheet  Sheet index to be inserted into orRange.
        @return  true = Range address could be parsed from the passed string.
     */
    static bool         convertToCellRangeUnchecked(
                            css::table::CellRangeAddress& orRange,
                            const OUString& rString,
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
                            css::table::CellRangeAddress& orRange,
                            const OUString& rString,
                            sal_Int16 nSheet,
                            bool bAllowOverflow, bool bTrackOverflow );

    /** Converts the passed range to a cell range address, without checking any
        sheet limits.

        @param orRange  (out-parameter) Returns the converted range address.
        @param rBinRange  Binary cell range struct.
        @param nSheet  Sheet index to be inserted into orRange.
     */
    static void         convertToCellRangeUnchecked(
                            css::table::CellRangeAddress& orRange,
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
                            css::table::CellRangeAddress& orRange,
                            const BinRange& rBinRange,
                            sal_Int16 nSheet,
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
                            const OUString& rString,
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

    css::table::CellAddress maMaxApiPos;     /// Maximum valid cell address in Calc.
    css::table::CellAddress maMaxXlsPos;     /// Maximum valid cell address in Excel.
    css::table::CellAddress maMaxPos;        /// Maximum valid cell address in Calc/Excel.
    ControlCharacters       maLinkChars;     /// Control characters for external link import (BIFF).
    ControlCharacters       maDConChars;     /// Control characters for DCON* record import (BIFF).
    bool                    mbColOverflow;   /// Flag for "columns overflow".
    bool                    mbRowOverflow;   /// Flag for "rows overflow".
    bool                    mbTabOverflow;   /// Flag for "tables overflow".
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
