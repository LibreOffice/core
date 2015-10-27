/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_FORMULABUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_FORMULABUFFER_HXX

#include <utility>
#include <oox/helper/refmap.hxx>
#include <oox/helper/refvector.hxx>
#include <salhelper/thread.hxx>
#include <osl/mutex.hxx>
#include "workbookhelper.hxx"
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <map>
#include <vector>
#include "worksheethelper.hxx"
#include "sheetdatabuffer.hxx"
#include <com/sun/star/sheet/XFormulaTokens.hpp>

namespace oox { namespace xls {

class FormulaBuffer : public WorkbookHelper
{
public:
    /**
     * Represents a shared formula definition.
     */
    struct SharedFormulaEntry
    {
        css::table::CellAddress maAddress;
        css::table::CellRangeAddress maRange;
        OUString maTokenStr;
        sal_Int32 mnSharedId;

        SharedFormulaEntry(
            const css::table::CellAddress& rAddress,
            const css::table::CellRangeAddress& rRange,
            const OUString& rTokenStr, sal_Int32 nSharedId );
    };

    /**
     * Represents a formula cell that uses shared formula.
     */
    struct SharedFormulaDesc
    {
        css::table::CellAddress maAddress;
        sal_Int32 mnSharedId;
        OUString maCellValue;
        sal_Int32 mnValueType;

        SharedFormulaDesc(
            const css::table::CellAddress& rAddr, sal_Int32 nSharedId,
            const OUString& rCellValue, sal_Int32 nValueType );
    };

    struct TokenAddressItem
    {
        OUString maTokenStr;
        css::table::CellAddress maCellAddress;
        TokenAddressItem( const OUString& rTokenStr, const css::table::CellAddress& rCellAddress ) : maTokenStr( rTokenStr ), maCellAddress( rCellAddress ) {}
    };

    struct TokenRangeAddressItem
    {
        TokenAddressItem maTokenAndAddress;
        css::table::CellRangeAddress maCellRangeAddress;
        TokenRangeAddressItem( const TokenAddressItem& rTokenAndAddress, const css::table::CellRangeAddress& rCellRangeAddress ) : maTokenAndAddress( rTokenAndAddress ), maCellRangeAddress( rCellRangeAddress ) {}
    };

    struct FormulaValue
    {
        css::table::CellAddress maCellAddress;
        OUString maValueStr;
        sal_Int32 mnCellType;
    };
    typedef std::pair<css::table::CellAddress, double> ValueAddressPair;

    struct SheetItem
    {
        std::vector<TokenAddressItem>* mpCellFormulas;
        std::vector<TokenRangeAddressItem>* mpArrayFormulas;
        std::vector<FormulaValue>* mpCellFormulaValues;
        std::vector<SharedFormulaEntry>* mpSharedFormulaEntries;
        std::vector<SharedFormulaDesc>* mpSharedFormulaIDs;

        SheetItem();
    };

private:
    // Vectors indexed by SCTAB - cf. SetSheetCount
    typedef ::std::vector< std::vector<TokenAddressItem> > FormulaDataArray;
    typedef ::std::vector< std::vector<TokenRangeAddressItem> > ArrayFormulaDataArray;
    // sheet -> list of shared formula descriptions
    typedef ::std::vector< std::vector<SharedFormulaDesc> > SheetToSharedFormulaid;
    // sheet -> stuff needed to create shared formulae
    typedef ::std::vector< std::vector<SharedFormulaEntry> >  SheetToFormulaEntryArray;
    typedef ::std::vector< std::vector<FormulaValue> > FormulaValueArray;

    osl::Mutex maMtxData;
    FormulaDataArray         maCellFormulas;
    ArrayFormulaDataArray    maCellArrayFormulas;
    SheetToFormulaEntryArray maSharedFormulas;
    SheetToSharedFormulaid   maSharedFormulaIds;
    FormulaValueArray        maCellFormulaValues;

    SheetItem getSheetItem( SCTAB nTab );

public:
    explicit            FormulaBuffer( const WorkbookHelper& rHelper );
    void                finalizeImport();
    void                setCellFormula( const css::table::CellAddress& rAddress, const OUString&  );

    void setCellFormula(
        const css::table::CellAddress& rAddress, sal_Int32 nSharedId,
        const OUString& rCellValue, sal_Int32 nValueType );

    void setCellFormulaValue(
        const css::table::CellAddress& rAddress, const OUString& rValueStr, sal_Int32 nCellType );

    void                setCellArrayFormula( const css::table::CellRangeAddress& rRangeAddress,
                                             const css::table::CellAddress& rTokenAddress,
                                             const OUString& );
    void                createSharedFormulaMapEntry( const css::table::CellAddress& rAddress,
                                                     const css::table::CellRangeAddress& rRange,
                                                     sal_Int32 nSharedId, const OUString& rTokens );

    /// ensure sizes of vectors matches the number of sheets
    void SetSheetCount( SCTAB nSheets );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
