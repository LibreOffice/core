/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OOX_XLS_FORMULABUFFER_HXX
#define OOX_XLS_FORMULABUFFER_HXX

#include <utility>
#include "oox/helper/refmap.hxx"
#include "oox/helper/refvector.hxx"
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
    /**
     * Represents a shared formula definition.
     */
    struct SharedFormulaEntry
    {
        com::sun::star::table::CellAddress maAddress;
        com::sun::star::table::CellRangeAddress maRange;
        OUString maTokenStr;
        sal_Int32 mnSharedId;

        SharedFormulaEntry(
            const com::sun::star::table::CellAddress& rAddress,
            const com::sun::star::table::CellRangeAddress& rRange,
            const OUString& rTokenStr, sal_Int32 nSharedId );
    };

    /**
     * Represents a formula cell that uses shared formula.
     */
    struct SharedFormulaDesc
    {
        com::sun::star::table::CellAddress maAddress;
        sal_Int32 mnSharedId;
        OUString maCellValue;
        sal_Int32 mnValueType;

        SharedFormulaDesc(
            const com::sun::star::table::CellAddress& rAddr, sal_Int32 nSharedId,
            const OUString& rCellValue, sal_Int32 nValueType );
    };

    struct TokenAddressItem
    {
        OUString maTokenStr;
        ::com::sun::star::table::CellAddress maCellAddress;
        TokenAddressItem( const OUString& rTokenStr, const ::com::sun::star::table::CellAddress& rCellAddress ) : maTokenStr( rTokenStr ), maCellAddress( rCellAddress ) {}
    };

    struct TokenRangeAddressItem
    {
        TokenAddressItem maTokenAndAddress;
        ::com::sun::star::table::CellRangeAddress maCellRangeAddress;
        TokenRangeAddressItem( const TokenAddressItem& rTokenAndAddress, const ::com::sun::star::table::CellRangeAddress& rCellRangeAddress ) : maTokenAndAddress( rTokenAndAddress ), maCellRangeAddress( rCellRangeAddress ) {}
    };

    typedef ::std::map< sal_Int32, std::vector< TokenAddressItem > > FormulaDataMap;
    typedef ::std::map< sal_Int32, std::vector< TokenRangeAddressItem > > ArrayFormulaDataMap;
    // sheet -> list of shared formula descriptions
    typedef ::std::map< sal_Int32, std::vector< SharedFormulaDesc > > SheetToSharedFormulaid;
    // sheet -> stuff needed to create shared formulae
    typedef ::std::map< sal_Int32, std::vector< SharedFormulaEntry > >  SheetToFormulaEntryMap;
    // sharedId -> tokedId
    typedef ::std::map< sal_Int32, sal_Int32 > SharedIdToTokenIndex;
    typedef ::std::map< sal_Int32, SharedIdToTokenIndex > SheetToSharedIdToTokenIndex;
    typedef ::std::pair< ::com::sun::star::table::CellAddress, double > ValueAddressPair;
    typedef ::std::map< sal_Int32, std::vector< ValueAddressPair > > FormulaValueMap;

    com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheet > mxCurrSheet;
    FormulaDataMap maCellFormulas;
    ArrayFormulaDataMap maCellArrayFormulas;
    SheetToFormulaEntryMap maSharedFormulas;
    SheetToSharedFormulaid maSharedFormulaIds;
    SheetToSharedIdToTokenIndex maTokenIndexes;
    FormulaValueMap maCellFormulaValues;

    com::sun::star::uno::Reference<com::sun::star::table::XCellRange>
        getRange( const com::sun::star::table::CellRangeAddress& rRange );

    void                applyArrayFormulas(  const std::vector< TokenRangeAddressItem >& rVector );
    void                applyCellFormula( ScDocument& rDoc, const ApiTokenSequence& rTokens, const ::com::sun::star::table::CellAddress& rAddress );
    void                applyCellFormulas(  const std::vector< TokenAddressItem >& rVector );
    void                applyCellFormulaValues( const std::vector< ValueAddressPair >& rVector );
    void applySharedFormulas( sal_Int32 nTab );

public:
    explicit            FormulaBuffer( const WorkbookHelper& rHelper );
    void                finalizeImport();
    void                setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, const OUString&  );

    void setCellFormula(
        const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nSharedId,
        const OUString& rCellValue, sal_Int32 nValueType );

    void                setCellFormulaValue( const ::com::sun::star::table::CellAddress& rAddress, double fValue  );
    void                setCellArrayFormula( const ::com::sun::star::table::CellRangeAddress& rRangeAddress, const ::com::sun::star::table::CellAddress& rTokenAddress, const OUString&  );
    void createSharedFormulaMapEntry(
        const com::sun::star::table::CellAddress& rAddress,
        const com::sun::star::table::CellRangeAddress& rRange,
        sal_Int32 nSharedId, const OUString& rTokens );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
