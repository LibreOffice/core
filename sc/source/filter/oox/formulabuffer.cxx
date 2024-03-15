/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formulabuffer.hxx>
#include <externallinkbuffer.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <documentimport.hxx>

#include <autonamecache.hxx>
#include <tokenarray.hxx>
#include <sharedformulagroups.hxx>
#include <externalrefmgr.hxx>
#include <tokenstringcontext.hxx>
#include <o3tl/safeint.hxx>
#include <oox/token/tokens.hxx>
#include <oox/helper/progressbar.hxx>
#include <svl/sharedstringpool.hxx>
#include <svl/numformat.hxx>
#include <sal/log.hxx>
#include <memory>
#include <utility>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sheet;

namespace oox::xls {

namespace {

/**
 * Cache the token array for the last cell position in each column. We use
 * one cache per sheet.
 */
class CachedTokenArray
{
public:
    CachedTokenArray(const CachedTokenArray&) = delete;
    const CachedTokenArray& operator=(const CachedTokenArray&) = delete;

    struct Item
    {
        SCROW mnRow;
        ScFormulaCell* mpCell;

        Item(const Item&) = delete;
        const Item& operator=(const Item&) = delete;

        Item() : mnRow(-1), mpCell(nullptr) {}
    };

    explicit CachedTokenArray( const ScDocument& rDoc ) :
        maCxt(rDoc, formula::FormulaGrammar::GRAM_OOXML) {}

    Item* get( const ScAddress& rPos, std::u16string_view rFormula )
    {
        // Check if a token array is cached for this column.
        ColCacheType::iterator it = maCache.find(rPos.Col());
        if (it == maCache.end())
            return nullptr;

        Item& rCached = *it->second;
        const ScTokenArray& rCode = *rCached.mpCell->GetCode();
        OUString aPredicted = rCode.CreateString(maCxt, rPos);
        if (rFormula == aPredicted)
            return &rCached;

        return nullptr;
    }

    void store( const ScAddress& rPos, ScFormulaCell* pCell )
    {
        ColCacheType::iterator it = maCache.find(rPos.Col());
        if (it == maCache.end())
        {
            // Create an entry for this column.
            std::pair<ColCacheType::iterator,bool> r =
                maCache.emplace(rPos.Col(), std::make_unique<Item>());
            if (!r.second)
                // Insertion failed.
                return;

            it = r.first;
        }

        Item& rItem = *it->second;
        rItem.mnRow = rPos.Row();
        rItem.mpCell = pCell;
    }

private:
    typedef std::unordered_map<SCCOL, std::unique_ptr<Item>> ColCacheType;
    ColCacheType maCache;
    sc::TokenStringContext maCxt;
};

void applySharedFormulas(
    ScDocumentImport& rDoc,
    std::vector<FormulaBuffer::SharedFormulaEntry>& rSharedFormulas,
    std::vector<FormulaBuffer::SharedFormulaDesc>& rCells,
    WorkbookHelper& rWorkbookHelper)
{
    sc::SharedFormulaGroups aGroups;
    {
        // Process shared formulas first.
        for (const FormulaBuffer::SharedFormulaEntry& rEntry : rSharedFormulas)
        {
            const ScAddress& aPos = rEntry.maAddress;
            sal_Int32 nId = rEntry.mnSharedId;
            const OUString& rTokenStr = rEntry.maTokenStr;

            ScCompiler aComp(rDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_OOXML, true, false);
            std::unique_ptr<ScTokenArray> pArray = aComp.CompileString(rTokenStr);
            if (pArray)
            {
                aComp.CompileTokenArray(); // Generate RPN tokens.
                aGroups.set(nId, std::move(pArray), aPos);
            }
        }
    }

    {
        svl::SharedStringPool& rStrPool = rDoc.getDoc().GetSharedStringPool();
        const bool bGeneratorKnownGood = rWorkbookHelper.isGeneratorKnownGood();
        bool bHasCalculatedFormulaCells = rWorkbookHelper.hasCalculatedFormulaCells();
        // Process formulas that use shared formulas.
        for (const FormulaBuffer::SharedFormulaDesc& rDesc : rCells)
        {
            const ScAddress& aPos = rDesc.maAddress;
            const sc::SharedFormulaGroupEntry* pEntry = aGroups.getEntry(rDesc.mnSharedId);
            if (!pEntry)
                continue;

            const ScTokenArray* pArray = pEntry->getTokenArray();
            assert(pArray);
            const ScAddress& rOrigin = pEntry->getOrigin();
            assert(rOrigin.IsValid());

            ScFormulaCell* pCell;
            // In case of shared-formula along a row, do not let
            // these cells share the same token objects.
            // If we do, any reference-updates on these cells
            // (while editing) will mess things up. Pass the cloned array as a
            // pointer and not as reference to avoid any further allocation.
            if (rOrigin.Col() != aPos.Col())
                pCell = new ScFormulaCell(rDoc.getDoc(), aPos, pArray->Clone());
            else
                pCell = new ScFormulaCell(rDoc.getDoc(), aPos, *pArray);

            rDoc.setFormulaCell(aPos, pCell);
            const bool bNeedNumberFormat = ((rDoc.getDoc().GetNumberFormat(
                            aPos.Col(), aPos.Row(), aPos.Tab()) % SV_COUNTRY_LANGUAGE_OFFSET) == 0);
            if (bNeedNumberFormat)
                pCell->SetNeedNumberFormat(true);

            if (rDesc.maCellValue.isEmpty())
            {
                // No cached cell value. Mark it for re-calculation.
                pCell->SetDirty();
                // Recalc even if AutoCalc is disabled. Must be after
                // SetDirty() as it also calls SetDirtyVar().
                pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE);
                continue;
            }

            // Set cached formula results. For now, we only use boolean,
            // numeric and string-formula results. Find out how to utilize
            // cached results of other types.
            switch (rDesc.mnValueType)
            {
                case XML_b:
                    // boolean value.
                    if (bNeedNumberFormat)
                    {
                        rDoc.getDoc().SetNumberFormat( aPos,
                                rDoc.getDoc().GetFormatTable()->GetStandardFormat( SvNumFormatType::LOGICAL));
                    }
                    if (rDesc.maCellValue == "1" || rDesc.maCellValue == "0")
                        pCell->SetResultDouble(rDesc.maCellValue == "1" ? 1.0 : 0.0);
                    else
                    {
                        // Recalc even if AutoCalc is disabled.
                        pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE);
                    }
                break;
                case XML_n:
                    // numeric value.
                    {
                        const double fVal = rDesc.maCellValue.toDouble();
                        if (!bHasCalculatedFormulaCells && fVal != 0.0)
                        {
                            rWorkbookHelper.setCalculatedFormulaCells();
                            bHasCalculatedFormulaCells = true;
                        }
                        pCell->SetResultDouble(fVal);
                        /* TODO: is it on purpose that we never reset dirty here
                         * and thus recalculate anyway if cell was dirty? Or is it
                         * never dirty and therefore set dirty below otherwise? This
                         * is different from the non-shared case in
                         * applyCellFormulaValues(). */
                    }
                break;
                case XML_str:
                    if (bGeneratorKnownGood)
                    {
                        // See applyCellFormulaValues
                        svl::SharedString aSS = rStrPool.intern(rDesc.maCellValue);
                        pCell->SetResultToken(new formula::FormulaStringToken(std::move(aSS)));
                        // If we don't reset dirty, then e.g. disabling macros makes all cells
                        // that use macro functions to show #VALUE!
                        pCell->ResetDirty();
                        pCell->SetChanged(false);
                        break;
                    }
                    [[fallthrough]];
                default:
                    // Mark it for re-calculation.
                    pCell->SetDirty();
                    // Recalc even if AutoCalc is disabled. Must be after
                    // SetDirty() as it also calls SetDirtyVar().
                    pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE);
            }
        }
    }
}

void applyCellFormulas(
    ScDocumentImport& rDoc, CachedTokenArray& rCache,
    const Sequence<ExternalLinkInfo>& rExternalLinks,
    const std::vector<FormulaBuffer::TokenAddressItem>& rCells )
{
    for (const FormulaBuffer::TokenAddressItem& rItem : rCells)
    {
        const ScAddress& aPos = rItem.maAddress;
        CachedTokenArray::Item* p = rCache.get(aPos, rItem.maTokenStr);
        if (p)
        {
            // Use the cached version to avoid re-compilation.

            ScFormulaCell* pCell = nullptr;
            if (p->mnRow + 1 == aPos.Row())
            {
                // Put them in the same formula group.
                ScFormulaCell& rPrev = *p->mpCell;
                ScFormulaCellGroupRef xGroup = rPrev.GetCellGroup();
                if (!xGroup)
                {
                    // Last cell is not grouped yet. Start a new group.
                    assert(rPrev.aPos.Row() == p->mnRow);
                    xGroup = rPrev.CreateCellGroup(1, false);
                }
                ++xGroup->mnLength;

                pCell = new ScFormulaCell(rDoc.getDoc(), aPos, xGroup);
            }
            else
                pCell = new ScFormulaCell(rDoc.getDoc(), aPos, p->mpCell->GetCode()->Clone());

            rDoc.setFormulaCell(aPos, pCell);
            if (rDoc.getDoc().GetNumberFormat(aPos.Col(), aPos.Row(), aPos.Tab()) % SV_COUNTRY_LANGUAGE_OFFSET == 0)
                pCell->SetNeedNumberFormat(true);

            // Update the cache.
            p->mnRow = aPos.Row();
            p->mpCell = pCell;
            continue;
        }

        ScCompiler aCompiler(rDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_OOXML, true, false);
        aCompiler.SetExternalLinks(rExternalLinks);
        std::unique_ptr<ScTokenArray> pCode = aCompiler.CompileString(rItem.maTokenStr);
        if (!pCode)
            continue;

        aCompiler.CompileTokenArray(); // Generate RPN tokens.

        ScFormulaCell* pCell = new ScFormulaCell(rDoc.getDoc(), aPos, std::move(pCode));
        rDoc.setFormulaCell(aPos, pCell);
        if (rDoc.getDoc().GetNumberFormat(aPos.Col(), aPos.Row(), aPos.Tab()) % SV_COUNTRY_LANGUAGE_OFFSET == 0)
            pCell->SetNeedNumberFormat(true);
        rCache.store(aPos, pCell);
    }
}

void applyArrayFormulas(
    ScDocumentImport& rDoc,
    const Sequence<ExternalLinkInfo>& rExternalLinks,
    const std::vector<FormulaBuffer::TokenRangeAddressItem>& rArrays )
{
    for (const FormulaBuffer::TokenRangeAddressItem& rAddressItem : rArrays)
    {
        const ScAddress& aPos = rAddressItem.maTokenAndAddress.maAddress;

        ScCompiler aComp(rDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_OOXML);
        aComp.SetExternalLinks(rExternalLinks);
        std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(rAddressItem.maTokenAndAddress.maTokenStr));
        if (pArray)
            rDoc.setMatrixCells(rAddressItem.maRange, *pArray, formula::FormulaGrammar::GRAM_OOXML);
    }
}

void applyCellFormulaValues(
    ScDocumentImport& rDoc, const std::vector<FormulaBuffer::FormulaValue>& rVector, WorkbookHelper& rWorkbookHelper )
{
    svl::SharedStringPool& rStrPool = rDoc.getDoc().GetSharedStringPool();
    const bool bGeneratorKnownGood = rWorkbookHelper.isGeneratorKnownGood();
    bool bHasCalculatedFormulaCells = rWorkbookHelper.hasCalculatedFormulaCells();

    for (const FormulaBuffer::FormulaValue& rValue : rVector)
    {
        const ScAddress& aCellPos = rValue.maAddress;
        ScFormulaCell* pCell = rDoc.getDoc().GetFormulaCell(aCellPos);
        const OUString& rValueStr = rValue.maValueStr;
        if (!pCell)
            continue;

        switch (rValue.mnCellType)
        {
            case XML_n:
            {
                const double fVal = rValueStr.toDouble();
                if (!bHasCalculatedFormulaCells && fVal != 0.0)
                {
                    rWorkbookHelper.setCalculatedFormulaCells();
                    bHasCalculatedFormulaCells = true;
                }
                pCell->SetResultDouble(fVal);
                pCell->ResetDirty();
                pCell->SetChanged(false);
            }
            break;
            case XML_str:
                // Excel uses t="str" for string results (per definition
                // ECMA-376 18.18.11 ST_CellType (Cell Type) "Cell containing a
                // formula string.", but that 't' Cell Data Type attribute, "an
                // enumeration representing the cell's data type", is meant for
                // the content of the <v> element). We follow that. Other
                // applications might not and instead use t="str" for the cell
                // content if formula. Setting an otherwise numeric result as
                // string result fouls things up, set result strings only for
                // documents claiming to be generated by a known good
                // generator. See tdf#98481
                if (bGeneratorKnownGood)
                {
                    svl::SharedString aSS = rStrPool.intern(rValueStr);
                    pCell->SetResultToken(new formula::FormulaStringToken(std::move(aSS)));
                    pCell->ResetDirty();
                    pCell->SetChanged(false);
                }
            break;
            default:
                ;
        }
    }
}

void processSheetFormulaCells(
    ScDocumentImport& rDoc, FormulaBuffer::SheetItem& rItem,
    const Sequence<ExternalLinkInfo>& rExternalLinks, WorkbookHelper& rWorkbookHelper )
{
    if (rItem.mpSharedFormulaEntries && rItem.mpSharedFormulaIDs)
        applySharedFormulas(rDoc, *rItem.mpSharedFormulaEntries,
                            *rItem.mpSharedFormulaIDs, rWorkbookHelper);

    if (rItem.mpCellFormulas)
    {
        CachedTokenArray aCache(rDoc.getDoc());
        applyCellFormulas(rDoc, aCache, rExternalLinks, *rItem.mpCellFormulas);
    }

    if (rItem.mpArrayFormulas)
        applyArrayFormulas(rDoc, rExternalLinks, *rItem.mpArrayFormulas);

    if (rItem.mpCellFormulaValues)
        applyCellFormulaValues(rDoc, *rItem.mpCellFormulaValues, rWorkbookHelper);
}

}

FormulaBuffer::SharedFormulaEntry::SharedFormulaEntry(
    const ScAddress& rAddr,
    OUString aTokenStr, sal_Int32 nSharedId ) :
    maAddress(rAddr), maTokenStr(std::move(aTokenStr)), mnSharedId(nSharedId) {}

FormulaBuffer::SharedFormulaDesc::SharedFormulaDesc(
    const ScAddress& rAddr, sal_Int32 nSharedId,
    OUString aCellValue, sal_Int32 nValueType ) :
    maAddress(rAddr), maCellValue(std::move(aCellValue)), mnSharedId(nSharedId), mnValueType(nValueType) {}

FormulaBuffer::SheetItem::SheetItem() :
    mpCellFormulas(nullptr),
    mpArrayFormulas(nullptr),
    mpCellFormulaValues(nullptr),
    mpSharedFormulaEntries(nullptr),
    mpSharedFormulaIDs(nullptr) {}

FormulaBuffer::FormulaBuffer( const WorkbookHelper& rHelper ) : WorkbookHelper( rHelper )
{
}

void FormulaBuffer::SetSheetCount( SCTAB nSheets )
{
    maCellFormulas.resize( nSheets );
    maCellArrayFormulas.resize( nSheets );
    maSharedFormulas.resize( nSheets );
    maSharedFormulaIds.resize( nSheets );
    maCellFormulaValues.resize( nSheets );
}

void FormulaBuffer::finalizeImport()
{
    ISegmentProgressBarRef xFormulaBar = getProgressBar().createSegment( getProgressBar().getFreeLength() );

    ScDocumentImport& rDoc = getDocImport();
    rDoc.getDoc().SetAutoNameCache(std::make_unique<ScAutoNameCache>(rDoc.getDoc()));
    ScExternalRefManager::ApiGuard aExtRefGuard(rDoc.getDoc());

    SCTAB nTabCount = rDoc.getDoc().GetTableCount();

    // Fetch all the formulas to process first.
    std::vector<SheetItem> aSheetItems;
    aSheetItems.reserve(nTabCount);
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
        aSheetItems.push_back(getSheetItem(nTab));

    for (SheetItem& rItem : aSheetItems)
        processSheetFormulaCells(rDoc, rItem, getExternalLinks().getLinkInfos(),
                *this);

    // With formula results being set and not recalculated we need to
    // force-trigger adding all linked external files to the LinkManager.
    rDoc.getDoc().GetExternalRefManager()->addFilesToLinkManager();

    rDoc.getDoc().SetAutoNameCache(nullptr);

    xFormulaBar->setPosition( 1.0 );
}

FormulaBuffer::SheetItem FormulaBuffer::getSheetItem( SCTAB nTab )
{
    std::scoped_lock aGuard(maMtxData);

    SheetItem aItem;

    if( o3tl::make_unsigned(nTab) >= maCellFormulas.size() )
    {
        SAL_WARN( "sc", "Tab " << nTab << " out of bounds " << maCellFormulas.size() );
        return aItem;
    }

    if( !maCellFormulas[ nTab ].empty() )
        aItem.mpCellFormulas = &maCellFormulas[ nTab ];
    if( !maCellArrayFormulas[ nTab ].empty() )
        aItem.mpArrayFormulas = &maCellArrayFormulas[ nTab ];
    if( !maCellFormulaValues[ nTab ].empty() )
        aItem.mpCellFormulaValues = &maCellFormulaValues[ nTab ];
    if( !maSharedFormulas[ nTab ].empty() )
        aItem.mpSharedFormulaEntries = &maSharedFormulas[ nTab ];
    if( !maSharedFormulaIds[ nTab ].empty() )
        aItem.mpSharedFormulaIDs = &maSharedFormulaIds[ nTab ];

    return aItem;
}

void FormulaBuffer::createSharedFormulaMapEntry(
    const ScAddress& rAddress,
    sal_Int32 nSharedId, const OUString& rTokens )
{
    assert( rAddress.Tab() >= 0 && o3tl::make_unsigned(rAddress.Tab()) < maSharedFormulas.size() );
    std::vector<SharedFormulaEntry>& rSharedFormulas = maSharedFormulas[ rAddress.Tab() ];
    SharedFormulaEntry aEntry(rAddress, rTokens, nSharedId);
    rSharedFormulas.push_back( aEntry );
}

void FormulaBuffer::setCellFormula( const ScAddress& rAddress, const OUString& rTokenStr )
{
    assert( rAddress.Tab() >= 0 && o3tl::make_unsigned(rAddress.Tab()) < maCellFormulas.size() );
    maCellFormulas[ rAddress.Tab() ].emplace_back( rTokenStr, rAddress );
}

void FormulaBuffer::setCellFormula(
    const ScAddress& rAddress, sal_Int32 nSharedId, const OUString& rCellValue, sal_Int32 nValueType )
{
    assert( rAddress.Tab() >= 0 && o3tl::make_unsigned(rAddress.Tab()) < maSharedFormulaIds.size() );
    maSharedFormulaIds[rAddress.Tab()].emplace_back(rAddress, nSharedId, rCellValue, nValueType);
}

void FormulaBuffer::setCellArrayFormula( const ScRange& rRangeAddress, const ScAddress& rTokenAddress, const OUString& rTokenStr )
{

    TokenAddressItem tokenPair( rTokenStr, rTokenAddress );
    assert( rRangeAddress.aStart.Tab() >= 0 && o3tl::make_unsigned(rRangeAddress.aStart.Tab()) < maCellArrayFormulas.size() );
    maCellArrayFormulas[ rRangeAddress.aStart.Tab() ].emplace_back( tokenPair, rRangeAddress );
}

void FormulaBuffer::setCellFormulaValue(
        const ScAddress& rAddress, const OUString& rValueStr, sal_Int32 nCellType )
{
    assert( rAddress.Tab() >= 0 && o3tl::make_unsigned(rAddress.Tab()) < maCellFormulaValues.size() );
    FormulaValue aVal;
    aVal.maAddress = rAddress;
    aVal.maValueStr = rValueStr;
    aVal.mnCellType = nCellType;
    maCellFormulaValues[rAddress.Tab()].push_back(aVal);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
