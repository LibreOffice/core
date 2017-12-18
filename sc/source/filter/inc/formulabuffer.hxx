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
#include <map>
#include <vector>
#include "worksheethelper.hxx"
#include "sheetdatabuffer.hxx"

namespace oox { namespace xls {

class FormulaBuffer : public WorkbookHelper
{
public:
    /**
     * Represents a shared formula definition.
     */
    struct SharedFormulaEntry
    {
        ScAddress maAddress;
        OUString maTokenStr;
        sal_Int32 mnSharedId;

        SharedFormulaEntry(
            const ScAddress& rAddress,
            const OUString& rTokenStr, sal_Int32 nSharedId );
    };

    /**
     * Represents a formula cell that uses shared formula.
     */
    struct SharedFormulaDesc
    {
        ScAddress maAddress;
        sal_Int32 mnSharedId;
        OUString maCellValue;
        sal_Int32 mnValueType;

        SharedFormulaDesc(
            const ScAddress& rAddr, sal_Int32 nSharedId,
            const OUString& rCellValue, sal_Int32 nValueType );
    };

    struct TokenAddressItem
    {
        OUString maTokenStr;
        ScAddress maAddress;
        TokenAddressItem( const OUString& rTokenStr, const ScAddress& rAddress ) : maTokenStr( rTokenStr ), maAddress( rAddress ) {}
    };

    struct TokenRangeAddressItem
    {
        TokenAddressItem maTokenAndAddress;
        ScRange maRange;
        TokenRangeAddressItem( const TokenAddressItem& rTokenAndAddress, const ScRange& rRange ) : maTokenAndAddress( rTokenAndAddress ), maRange( rRange ) {}
    };

    struct FormulaValue
    {
        ScAddress maAddress;
        OUString maValueStr;
        sal_Int32 mnCellType;
    };

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
    void                setCellFormula( const ScAddress& rAddress, const OUString&  );

    void setCellFormula(
        const ScAddress& rAddress, sal_Int32 nSharedId,
        const OUString& rCellValue, sal_Int32 nValueType );

    void setCellFormulaValue(
        const ScAddress& rAddress, const OUString& rValueStr, sal_Int32 nCellType );

    void                setCellArrayFormula( const ScRange& rRangeAddress,
                                             const ScAddress& rTokenAddress,
                                             const OUString& );

    void                createSharedFormulaMapEntry( const ScAddress& rAddress,
                                                     sal_Int32 nSharedId, const OUString& rTokens );

    /// ensure sizes of vectors matches the number of sheets
    void SetSheetCount( SCTAB nSheets );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
