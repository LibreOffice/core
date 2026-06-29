/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "workbookhelper.hxx"
#include <formula/opcode.hxx>
#include <sal/types.h>
#include <initializer_list>
#include <mutex>
#include <utility>
#include <vector>

class ScTokenArray;

namespace oox::xls {

// When one of the trigger opcodes is followed by ((...)) that
// spans the whole argument, drop the inner parenthesis pair:
//   OPCODE((expr)) -> OPCODE(expr)
// An inner pair that does not span the whole argument, or
// parentheses that belong to a function call inside the
// argument, stay:
//   OPCODE((A)+B)     -> OPCODE((A)+B)
//   OPCODE(FUNC(arg)) -> OPCODE(FUNC(arg))
SAL_DLLPUBLIC_EXPORT void stripRedundantParentheses(
    ScTokenArray& rArray, std::initializer_list<OpCode> aTriggerOpCodes);

// _xlfn.ANCHORARRAY(<ref>) is the OOXML spelling of the native
// postfix <ref>#. Both give the same RPN, but the parse array
// keeps the source order and would read back as #(<ref>), so
// rewrite each such span to <ref>#.
SAL_DLLPUBLIC_EXPORT void liftAnchorArrayToPostfix(ScTokenArray& rArray);

class FormulaBuffer final : public WorkbookHelper
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
            OUString aTokenStr, sal_Int32 nSharedId );
    };

    /**
     * Represents a formula cell that uses shared formula.
     */
    struct SharedFormulaDesc
    {
        ScAddress maAddress;
        OUString maCellValue;
        sal_Int32 mnSharedId;
        sal_Int32 mnValueType;
        /// True when the XLSX cell carried cm="1", marking it as a dynamic-array
        /// master.
        bool mbDynamicArrayMaster = false;

        SharedFormulaDesc(
            const ScAddress& rAddr, sal_Int32 nSharedId,
            OUString aCellValue, sal_Int32 nValueType,
            bool bDynamicArrayMaster = false );
    };

    struct TokenAddressItem
    {
        OUString maTokenStr;
        ScAddress maAddress;
        /// True when the XLSX cell carried cm="1", marking it as a dynamic-array
        /// master.
        bool mbDynamicArrayMaster = false;
        TokenAddressItem( OUString aTokenStr, const ScAddress& rAddress,
                          bool bDynamicArrayMaster = false )
            : maTokenStr(std::move( aTokenStr ))
            , maAddress( rAddress )
            , mbDynamicArrayMaster(bDynamicArrayMaster) {}
    };

    struct TokenRangeAddressItem
    {
        TokenAddressItem maTokenAndAddress;
        ScRange maRange;

        /// File was saved with a #SPILL! error on the master cell. Triggers
        /// blocker preservation so reference cells aren't materialised over
        /// real user data sitting in the matrix range.
        bool mbCachedSpill;
        /// True when the XLSX cell carried cm="1", marking it as a dynamic-array
        /// master.
        bool mbDynamicArrayMaster = false;

        TokenRangeAddressItem(TokenAddressItem aTokenAndAddress, const ScRange& rRange,
                              bool bCachedSpill = false,
                              bool bDynamicArrayMaster = false)
            : maTokenAndAddress(std::move(aTokenAndAddress))
            , maRange(rRange)
            , mbCachedSpill(bCachedSpill)
            , mbDynamicArrayMaster(bDynamicArrayMaster)
        {}
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

    std::mutex maMtxData;
    // Vectors indexed by SCTAB - cf. SetSheetCount
    std::vector< std::vector<TokenAddressItem> >         maCellFormulas;
    std::vector< std::vector<TokenRangeAddressItem> >    maCellArrayFormulas;
    std::vector< std::vector<SharedFormulaEntry> >  maSharedFormulas; // sheet -> stuff needed to create shared formulae
    std::vector< std::vector<SharedFormulaDesc> >   maSharedFormulaIds; // sheet -> list of shared formula descriptions
    std::vector< std::vector<FormulaValue> >        maCellFormulaValues; // sheet -> stuff needed to create shared formulae

    SheetItem getSheetItem( SCTAB nTab );

    std::vector<ScRange> maDataTables;

public:
    explicit            FormulaBuffer( const WorkbookHelper& rHelper );
    void                finalizeImport();
    void                setCellFormula( const ScAddress& rAddress, const OUString&,
                                        bool bDynamicArrayMaster = false );

    void setCellFormula(
        const ScAddress& rAddress, sal_Int32 nSharedId,
        const OUString& rCellValue, sal_Int32 nValueType,
        bool bDynamicArrayMaster = false );

    void setCellFormulaValue(
        const ScAddress& rAddress, const OUString& rValueStr, sal_Int32 nCellType );

    void setCellArrayFormula(const ScRange& rRangeAddress,
                             const ScAddress& rTokenAddress,
                             const OUString&,
                             bool bCachedSpill = false,
                             bool bDynamicArrayMaster = false);

    void                createSharedFormulaMapEntry( const ScAddress& rAddress,
                                                     sal_Int32 nSharedId, const OUString& rTokens );

    /// ensure sizes of vectors matches the number of sheets
    void SetSheetCount( SCTAB nSheets );

    void addDataTable(const ScRange& rRange) { maDataTables.push_back(rRange); }
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
