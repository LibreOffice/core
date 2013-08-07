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
    struct SharedFormulaEntry
    {
        ::com::sun::star::table::CellAddress maAddress;
        OUString maTokenStr;
        sal_Int32 mnSharedId;
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XFormulaTokens > mxFormulaTokens;
        SharedFormulaEntry( const ::com::sun::star::table::CellAddress& rAddress, const OUString& rTokenStr, sal_Int32 nSharedId ) : maAddress( rAddress ), maTokenStr( rTokenStr ), mnSharedId( nSharedId ) {}
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
    // shared formuala descriptions, the id and address the formula is at
    typedef std::pair< ::com::sun::star::table::CellAddress, sal_Int32 > SharedFormulaDesc;
    // sheet -> list of shared formula descriptions
    typedef ::std::map< sal_Int32, std::vector< SharedFormulaDesc > > SheetToSharedFormulaid;
    // sheet -> stuff needed to create shared formulae
    typedef ::std::map< sal_Int32, std::vector< SharedFormulaEntry > >  SheetToFormulaEntryMap;
    // sharedId -> tokedId
    typedef ::std::map< sal_Int32, sal_Int32 > SharedIdToTokenIndex;
    typedef ::std::map< sal_Int32, SharedIdToTokenIndex > SheetToSharedIdToTokenIndex;
    typedef ::std::pair< ::com::sun::star::table::CellAddress, double > ValueAddressPair;
    typedef ::std::map< sal_Int32, std::vector< ValueAddressPair > > FormulaValueMap;

    void createSharedFormula(  const ::com::sun::star::table::CellAddress& rAddress,  sal_Int32 nSharedId, const OUString& rTokens );
    ::com::sun::star::uno::Reference< com::sun::star::table::XCellRange > getRange( const ::com::sun::star::table::CellRangeAddress& rRange);
    com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheet > mxCurrSheet;
    FormulaDataMap      cellFormulas;
    ArrayFormulaDataMap cellArrayFormulas;
    SheetToFormulaEntryMap sharedFormulas;
    SheetToSharedFormulaid sharedFormulaIds;
    SheetToSharedIdToTokenIndex tokenIndexes;
    FormulaValueMap        cellFormulaValues;

    void                applyArrayFormulas(  const std::vector< TokenRangeAddressItem >& rVector );
    void                applyCellFormula( ScDocument& rDoc, const ApiTokenSequence& rTokens, const ::com::sun::star::table::CellAddress& rAddress );
    void                applyCellFormulas(  const std::vector< TokenAddressItem >& rVector );
    void                applyCellFormulaValues( const std::vector< ValueAddressPair >& rVector );

public:
    explicit            FormulaBuffer( const WorkbookHelper& rHelper );
    void                finalizeImport();
    void                setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, const OUString&  );
    void                setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nSharedId );
    void                setCellFormulaValue( const ::com::sun::star::table::CellAddress& rAddress, double fValue  );
    void                setCellArrayFormula( const ::com::sun::star::table::CellRangeAddress& rRangeAddress, const ::com::sun::star::table::CellAddress& rTokenAddress, const OUString&  );
    void                createSharedFormulaMapEntry( const ::com::sun::star::table::CellAddress& rAddress, sal_Int32 nSharedId, const OUString& rTokens );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
