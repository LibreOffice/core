/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formulabuffer.hxx>
#include <formulaparser.hxx>
#include <externallinkbuffer.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <documentimport.hxx>

#include <rangelst.hxx>
#include <autonamecache.hxx>
#include <tokenuno.hxx>
#include <tokenarray.hxx>
#include <sharedformulagroups.hxx>
#include <externalrefmgr.hxx>
#include <tokenstringcontext.hxx>
#include <oox/token/tokens.hxx>
#include <svl/sharedstringpool.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sheet;

#include <memory>

namespace oox { namespace xls {

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

    explicit CachedTokenArray( ScDocument& rDoc ) :
        maCxt(&rDoc, formula::FormulaGrammar::GRAM_OOXML) {}

    ~CachedTokenArray()
    {
        for (const std::pair<SCCOL, Item*>& rCacheItem : maCache)
            delete rCacheItem.second;
    }

    Item* get( const ScAddress& rPos, const OUString& rFormula )
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
                maCache.emplace(rPos.Col(), new Item);
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
    typedef std::unordered_map<SCCOL, Item*> ColCacheType;
    ColCacheType maCache;
    sc::TokenStringContext maCxt;
};

void applySharedFormulas(
    ScDocumentImport& rDoc,
    SvNumberFormatter& rFormatter,
    std::vector<FormulaBuffer::SharedFormulaEntry>& rSharedFormulas,
    std::vector<FormulaBuffer::SharedFormulaDesc>& rCells,
    bool bGeneratorKnownGood)
{
    sc::SharedFormulaGroups aGroups;
    {
        // Process shared formulas first.
        for (const FormulaBuffer::SharedFormulaEntry& rEntry : rSharedFormulas)
        {
            const ScAddress& aPos = rEntry.maAddress;
            sal_Int32 nId = rEntry.mnSharedId;
            const OUString& rTokenStr = rEntry.maTokenStr;

            ScCompiler aComp(&rDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_OOXML);
            aComp.SetNumberFormatter(&rFormatter);
            ScTokenArray* pArray = aComp.CompileString(rTokenStr);
            if (pArray)
            {
                aComp.CompileTokenArray(); // Generate RPN tokens.
                aGroups.set(nId, pArray);
            }
        }
    }

    {
        svl::SharedStringPool& rStrPool = rDoc.getDoc().GetSharedStringPool();
        // Process formulas that use shared formulas.
        for (const FormulaBuffer::SharedFormulaDesc& rDesc : rCells)
        {
            const ScAddress& aPos = rDesc.maAddress;
            const ScTokenArray* pArray = aGroups.get(rDesc.mnSharedId);
            if (!pArray)
                continue;

            ScFormulaCell* pCell = new ScFormulaCell(&rDoc.getDoc(), aPos, *pArray);
            rDoc.setFormulaCell(aPos, pCell);
            if (rDesc.maCellValue.isEmpty())
            {
                // No cached cell value. Mark it for re-calculation.
                pCell->SetDirty();
                continue;
            }

            // Set cached formula results. For now, we only use numeric and string-formula
            // results. Find out how to utilize cached results of other types.
            switch (rDesc.mnValueType)
            {
                case XML_n:
                    // numeric value.
                    pCell->SetResultDouble(rDesc.maCellValue.toDouble());
                    /* TODO: is it on purpose that we never reset dirty here
                     * and thus recalculate anyway if cell was dirty? Or is it
                     * never dirty and therefor set dirty below otherwise? This
                     * is different from the non-shared case in
                     * applyCellFormulaValues(). */
                break;
                case XML_str:
                    if (bGeneratorKnownGood)
                    {
                        // See applyCellFormulaValues
                        svl::SharedString aSS = rStrPool.intern(rDesc.maCellValue);
                        pCell->SetResultToken(new formula::FormulaStringToken(aSS));
                        // If we don't reset dirty, then e.g. disabling macros makes all cells
                        // that use macro functions to show #VALUE!
                        pCell->ResetDirty();
                        pCell->SetChanged(false);
                        break;
                    }
                    SAL_FALLTHROUGH;
                default:
                    // Mark it for re-calculation.
                    pCell->SetDirty();
            }
        }
    }
}

void applyCellFormulas(
    ScDocumentImport& rDoc, CachedTokenArray& rCache, SvNumberFormatter& rFormatter,
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

                pCell = new ScFormulaCell(&rDoc.getDoc(), aPos, xGroup);
            }
            else
                pCell = new ScFormulaCell(&rDoc.getDoc(), aPos, p->mpCell->GetCode()->Clone());

            rDoc.setFormulaCell(aPos, pCell);

            // Update the cache.
            p->mnRow = aPos.Row();
            p->mpCell = pCell;
            continue;
        }

        ScCompiler aCompiler(&rDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_OOXML);
        aCompiler.SetNumberFormatter(&rFormatter);
        aCompiler.SetExternalLinks(rExternalLinks);
        ScTokenArray* pCode = aCompiler.CompileString(rItem.maTokenStr);
        if (!pCode)
            continue;

        aCompiler.CompileTokenArray(); // Generate RPN tokens.

        // Check if ocDde/ocWebservice is in any formula for external links warning.
        rDoc.getDoc().CheckLinkFormulaNeedingCheck(*pCode);

        ScFormulaCell* pCell = new ScFormulaCell(&rDoc.getDoc(), aPos, pCode);
        rDoc.setFormulaCell(aPos, pCell);
        rCache.store(aPos, pCell);
    }
}

void applyArrayFormulas(
    ScDocumentImport& rDoc, SvNumberFormatter& rFormatter,
    const std::vector<FormulaBuffer::TokenRangeAddressItem>& rArrays )
{
    for (const FormulaBuffer::TokenRangeAddressItem& rAddressItem : rArrays)
    {
        const ScAddress& aPos = rAddressItem.maTokenAndAddress.maAddress;

        ScCompiler aComp(&rDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_OOXML);
        aComp.SetNumberFormatter(&rFormatter);
        std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(rAddressItem.maTokenAndAddress.maTokenStr));
        if (pArray)
            rDoc.setMatrixCells(rAddressItem.maRange, *pArray, formula::FormulaGrammar::GRAM_OOXML);
    }
}

void applyCellFormulaValues(
    ScDocumentImport& rDoc, const std::vector<FormulaBuffer::FormulaValue>& rVector, bool bGeneratorKnownGood )
{
    svl::SharedStringPool& rStrPool = rDoc.getDoc().GetSharedStringPool();

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
                pCell->SetResultDouble(rValueStr.toDouble());
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
                    pCell->SetResultToken(new formula::FormulaStringToken(aSS));
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
    ScDocumentImport& rDoc, FormulaBuffer::SheetItem& rItem, SvNumberFormatter& rFormatter,
    const Sequence<ExternalLinkInfo>& rExternalLinks, bool bGeneratorKnownGood )
{
    if (rItem.mpSharedFormulaEntries && rItem.mpSharedFormulaIDs)
        applySharedFormulas(rDoc, rFormatter, *rItem.mpSharedFormulaEntries,
                            *rItem.mpSharedFormulaIDs, bGeneratorKnownGood);

    if (rItem.mpCellFormulas)
    {
        CachedTokenArray aCache(rDoc.getDoc());
        applyCellFormulas(rDoc, aCache, rFormatter, rExternalLinks, *rItem.mpCellFormulas);
    }

    if (rItem.mpArrayFormulas)
        applyArrayFormulas(rDoc, rFormatter, *rItem.mpArrayFormulas);

    if (rItem.mpCellFormulaValues)
        applyCellFormulaValues(rDoc, *rItem.mpCellFormulaValues, bGeneratorKnownGood);
}

}

FormulaBuffer::SharedFormulaEntry::SharedFormulaEntry(
    const ScAddress& rAddr,
    const OUString& rTokenStr, sal_Int32 nSharedId ) :
    maAddress(rAddr), maTokenStr(rTokenStr), mnSharedId(nSharedId) {}

FormulaBuffer::SharedFormulaDesc::SharedFormulaDesc(
    const ScAddress& rAddr, sal_Int32 nSharedId,
    const OUString& rCellValue, sal_Int32 nValueType ) :
    maAddress(rAddr), mnSharedId(nSharedId), maCellValue(rCellValue), mnValueType(nValueType) {}

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
    rDoc.getDoc().SetAutoNameCache(new ScAutoNameCache(&rDoc.getDoc()));
    ScExternalRefManager::ApiGuard aExtRefGuard(&rDoc.getDoc());

    SCTAB nTabCount = rDoc.getDoc().GetTableCount();

    // Fetch all the formulas to process first.
    std::vector<SheetItem> aSheetItems;
    aSheetItems.reserve(nTabCount);
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
        aSheetItems.push_back(getSheetItem(nTab));

    for (SheetItem& rItem : aSheetItems)
        processSheetFormulaCells(rDoc, rItem, *rDoc.getDoc().GetFormatTable(), getExternalLinks().getLinkInfos(),
                isGeneratorKnownGood());

    // With formula results being set and not recalculated we need to
    // force-trigger adding all linked external files to the LinkManager.
    rDoc.getDoc().GetExternalRefManager()->addFilesToLinkManager();

    rDoc.getDoc().SetAutoNameCache(nullptr);

    xFormulaBar->setPosition( 1.0 );
}

FormulaBuffer::SheetItem FormulaBuffer::getSheetItem( SCTAB nTab )
{
    osl::MutexGuard aGuard(&maMtxData);

    SheetItem aItem;

    if( static_cast<size_t>(nTab) >= maCellFormulas.size() )
    {
        SAL_WARN( "sc", "Tab " << nTab << " out of bounds " << maCellFormulas.size() );
        return aItem;
    }

    if( maCellFormulas[ nTab ].size() > 0 )
        aItem.mpCellFormulas = &maCellFormulas[ nTab ];
    if( maCellArrayFormulas[ nTab ].size() > 0 )
        aItem.mpArrayFormulas = &maCellArrayFormulas[ nTab ];
    if( maCellFormulaValues[ nTab ].size() > 0 )
        aItem.mpCellFormulaValues = &maCellFormulaValues[ nTab ];
    if( maSharedFormulas[ nTab ].size() > 0 )
        aItem.mpSharedFormulaEntries = &maSharedFormulas[ nTab ];
    if( maSharedFormulaIds[ nTab ].size() > 0 )
        aItem.mpSharedFormulaIDs = &maSharedFormulaIds[ nTab ];

    return aItem;
}

void FormulaBuffer::createSharedFormulaMapEntry(
    const ScAddress& rAddress,
    sal_Int32 nSharedId, const OUString& rTokens )
{
    assert( rAddress.Tab() >= 0 && static_cast<size_t>(rAddress.Tab()) < maSharedFormulas.size() );
    std::vector<SharedFormulaEntry>& rSharedFormulas = maSharedFormulas[ rAddress.Tab() ];
    SharedFormulaEntry aEntry(rAddress, rTokens, nSharedId);
    rSharedFormulas.push_back( aEntry );
}

void FormulaBuffer::setCellFormula( const ScAddress& rAddress, const OUString& rTokenStr )
{
    assert( rAddress.Tab() >= 0 && static_cast<size_t>(rAddress.Tab()) < maCellFormulas.size() );
    maCellFormulas[ rAddress.Tab() ].emplace_back( rTokenStr, rAddress );
}

void FormulaBuffer::setCellFormula(
    const ScAddress& rAddress, sal_Int32 nSharedId, const OUString& rCellValue, sal_Int32 nValueType )
{
    assert( rAddress.Tab() >= 0 && static_cast<size_t>(rAddress.Tab()) < maSharedFormulaIds.size() );
    maSharedFormulaIds[rAddress.Tab()].emplace_back(rAddress, nSharedId, rCellValue, nValueType);
}

void FormulaBuffer::setCellArrayFormula( const ScRange& rRangeAddress, const ScAddress& rTokenAddress, const OUString& rTokenStr )
{

    TokenAddressItem tokenPair( rTokenStr, rTokenAddress );
    assert( rRangeAddress.aStart.Tab() >= 0 && static_cast<size_t>(rRangeAddress.aStart.Tab()) < maCellArrayFormulas.size() );
    maCellArrayFormulas[ rRangeAddress.aStart.Tab() ].emplace_back( tokenPair, rRangeAddress );
}

void FormulaBuffer::setCellFormulaValue(
        const ScAddress& rAddress, const OUString& rValueStr, sal_Int32 nCellType )
{
    assert( rAddress.Tab() >= 0 && static_cast<size_t>(rAddress.Tab()) < maCellFormulaValues.size() );
    FormulaValue aVal;
    aVal.maAddress = rAddress;
    aVal.maValueStr = rValueStr;
    aVal.mnCellType = nCellType;
    maCellFormulaValues[rAddress.Tab()].push_back(aVal);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
