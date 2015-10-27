/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formulabuffer.hxx"
#include "formulaparser.hxx"
#include <externallinkbuffer.hxx>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCell2.hpp>
#include "formulacell.hxx"
#include "document.hxx"
#include "documentimport.hxx"
#include "convuno.hxx"

#include "rangelst.hxx"
#include "autonamecache.hxx"
#include "tokenuno.hxx"
#include "tokenarray.hxx"
#include "sharedformulagroups.hxx"
#include "externalrefmgr.hxx"
#include "tokenstringcontext.hxx"
#include <oox/token/tokens.hxx>
#include <svl/sharedstringpool.hxx>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::container;

#include <boost/noncopyable.hpp>
#include <memory>

namespace oox { namespace xls {

namespace {

/**
 * Cache the token array for the last cell position in each column. We use
 * one cache per sheet.
 */
class CachedTokenArray : boost::noncopyable
{
public:

    struct Item : boost::noncopyable
    {
        SCROW mnRow;
        ScFormulaCell* mpCell;

        Item() : mnRow(-1), mpCell(NULL) {}
    };

    CachedTokenArray( ScDocument& rDoc ) :
        maCxt(&rDoc, formula::FormulaGrammar::GRAM_OOXML) {}

    ~CachedTokenArray()
    {
        ColCacheType::const_iterator it = maCache.begin(), itEnd = maCache.end();
        for (; it != itEnd; ++it)
            delete it->second;
    }

    Item* get( const ScAddress& rPos, const OUString& rFormula )
    {
        // Check if a token array is cached for this column.
        ColCacheType::iterator it = maCache.find(rPos.Col());
        if (it == maCache.end())
            return NULL;

        Item& rCached = *it->second;
        const ScTokenArray& rCode = *rCached.mpCell->GetCode();
        OUString aPredicted = rCode.CreateString(maCxt, rPos);
        if (rFormula == aPredicted)
            return &rCached;

        return NULL;
    }

    void store( const ScAddress& rPos, ScFormulaCell* pCell )
    {
        ColCacheType::iterator it = maCache.find(rPos.Col());
        if (it == maCache.end())
        {
            // Create an entry for this column.
            std::pair<ColCacheType::iterator,bool> r =
                maCache.insert(ColCacheType::value_type(rPos.Col(), new Item));
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
    std::vector<FormulaBuffer::SharedFormulaDesc>& rCells )
{
    sc::SharedFormulaGroups aGroups;
    {
        // Process shared formulas first.
        std::vector<FormulaBuffer::SharedFormulaEntry>::const_iterator it = rSharedFormulas.begin(), itEnd = rSharedFormulas.end();
        for (; it != itEnd; ++it)
        {
            const table::CellAddress& rAddr = it->maAddress;
            sal_Int32 nId = it->mnSharedId;
            const OUString& rTokenStr = it->maTokenStr;

            ScAddress aPos;
            ScUnoConversion::FillScAddress(aPos, rAddr);
            ScCompiler aComp(&rDoc.getDoc(), aPos);
            aComp.SetNumberFormatter(&rFormatter);
            aComp.SetGrammar(formula::FormulaGrammar::GRAM_OOXML);
            ScTokenArray* pArray = aComp.CompileString(rTokenStr);
            if (pArray)
            {
                aComp.CompileTokenArray(); // Generate RPN tokens.
                aGroups.set(nId, pArray);
            }
        }
    }

    {
        // Process formulas that use shared formulas.
        std::vector<FormulaBuffer::SharedFormulaDesc>::const_iterator it = rCells.begin(), itEnd = rCells.end();
        for (; it != itEnd; ++it)
        {
            const table::CellAddress& rAddr = it->maAddress;
            const ScTokenArray* pArray = aGroups.get(it->mnSharedId);
            if (!pArray)
                continue;

            ScAddress aPos;
            ScUnoConversion::FillScAddress(aPos, rAddr);
            ScFormulaCell* pCell = new ScFormulaCell(&rDoc.getDoc(), aPos, *pArray);
            rDoc.setFormulaCell(aPos, pCell);
            if (it->maCellValue.isEmpty())
            {
                // No cached cell value. Mark it for re-calculation.
                pCell->SetDirty();
                continue;
            }

            // Set cached formula results. For now, we only use numeric
            // results. Find out how to utilize cached results of other types.
            switch (it->mnValueType)
            {
                case XML_n:
                    // numeric value.
                    pCell->SetResultDouble(it->maCellValue.toDouble());
                break;
                default:
                    // Mark it for re-calculation.
                    pCell->SetDirty();
            }
        }
    }
}

void applyCellFormulas(
    ScDocumentImport& rDoc, CachedTokenArray& rCache, SvNumberFormatter& rFormatter,
    const uno::Sequence<sheet::ExternalLinkInfo>& rExternalLinks,
    const std::vector<FormulaBuffer::TokenAddressItem>& rCells )
{
    std::vector<FormulaBuffer::TokenAddressItem>::const_iterator it = rCells.begin(), itEnd = rCells.end();
    for (; it != itEnd; ++it)
    {
        ScAddress aPos;
        ScUnoConversion::FillScAddress(aPos, it->maCellAddress);
        CachedTokenArray::Item* p = rCache.get(aPos, it->maTokenStr);
        if (p)
        {
            // Use the cached version to avoid re-compilation.

            ScFormulaCell* pCell = NULL;
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

        ScCompiler aCompiler(&rDoc.getDoc(), aPos);
        aCompiler.SetNumberFormatter(&rFormatter);
        aCompiler.SetGrammar(formula::FormulaGrammar::GRAM_OOXML);
        aCompiler.SetExternalLinks(rExternalLinks);
        ScTokenArray* pCode = aCompiler.CompileString(it->maTokenStr);
        if (!pCode)
            continue;

        aCompiler.CompileTokenArray(); // Generate RPN tokens.
        ScFormulaCell* pCell = new ScFormulaCell(&rDoc.getDoc(), aPos, pCode);
        rDoc.setFormulaCell(aPos, pCell);
        rCache.store(aPos, pCell);
    }
}

void applyArrayFormulas(
    ScDocumentImport& rDoc, SvNumberFormatter& rFormatter,
    const std::vector<FormulaBuffer::TokenRangeAddressItem>& rArrays )
{
    std::vector<FormulaBuffer::TokenRangeAddressItem>::const_iterator it = rArrays.begin(), itEnd = rArrays.end();
    for (; it != itEnd; ++it)
    {
        ScAddress aPos;
        ScUnoConversion::FillScAddress(aPos, it->maTokenAndAddress.maCellAddress);
        ScRange aRange;
        ScUnoConversion::FillScRange(aRange, it->maCellRangeAddress);

        ScCompiler aComp(&rDoc.getDoc(), aPos);
        aComp.SetNumberFormatter(&rFormatter);
        aComp.SetGrammar(formula::FormulaGrammar::GRAM_OOXML);
        std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(it->maTokenAndAddress.maTokenStr));
        if (pArray)
            rDoc.setMatrixCells(aRange, *pArray, formula::FormulaGrammar::GRAM_OOXML);
    }
}

void applyCellFormulaValues(
    ScDocumentImport& rDoc, const std::vector<FormulaBuffer::FormulaValue>& rVector )
{
    svl::SharedStringPool& rStrPool = rDoc.getDoc().GetSharedStringPool();

    std::vector<FormulaBuffer::FormulaValue>::const_iterator it = rVector.begin(), itEnd = rVector.end();
    for (; it != itEnd; ++it)
    {
        ScAddress aCellPos;
        ScUnoConversion::FillScAddress(aCellPos, it->maCellAddress);
        ScFormulaCell* pCell = rDoc.getDoc().GetFormulaCell(aCellPos);
        const OUString& rValueStr = it->maValueStr;
        if (!pCell)
            continue;

        switch (it->mnCellType)
        {
            case XML_n:
            {
                pCell->SetResultDouble(rValueStr.toDouble());
                pCell->ResetDirty();
                pCell->SetChanged(false);
            }
            break;
            case XML_str:
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
    const uno::Sequence<sheet::ExternalLinkInfo>& rExternalLinks )
{
    if (rItem.mpSharedFormulaEntries && rItem.mpSharedFormulaIDs)
        applySharedFormulas(rDoc, rFormatter, *rItem.mpSharedFormulaEntries, *rItem.mpSharedFormulaIDs);

    if (rItem.mpCellFormulas)
    {
        CachedTokenArray aCache(rDoc.getDoc());
        applyCellFormulas(rDoc, aCache, rFormatter, rExternalLinks, *rItem.mpCellFormulas);
    }

    if (rItem.mpArrayFormulas)
        applyArrayFormulas(rDoc, rFormatter, *rItem.mpArrayFormulas);

    if (rItem.mpCellFormulaValues)
        applyCellFormulaValues(rDoc, *rItem.mpCellFormulaValues);
}

class WorkerThread: public salhelper::Thread, private boost::noncopyable
{
    ScDocumentImport& mrDoc;
    FormulaBuffer::SheetItem& mrItem;
    std::unique_ptr<SvNumberFormatter> mpFormatter;
    const uno::Sequence<sheet::ExternalLinkInfo>& mrExternalLinks;

public:
    WorkerThread(
        ScDocumentImport& rDoc, FormulaBuffer::SheetItem& rItem, SvNumberFormatter* pFormatter,
        const uno::Sequence<sheet::ExternalLinkInfo>& rExternalLinks ) :
        salhelper::Thread("xlsx-import-formula-buffer-worker-thread"),
        mrDoc(rDoc), mrItem(rItem), mpFormatter(pFormatter), mrExternalLinks(rExternalLinks) {}

    virtual ~WorkerThread() {}

protected:
    virtual void execute() override
    {
        processSheetFormulaCells(mrDoc, mrItem, *mpFormatter, mrExternalLinks);
    }
};

}

FormulaBuffer::SharedFormulaEntry::SharedFormulaEntry(
    const table::CellAddress& rAddr, const table::CellRangeAddress& rRange,
    const OUString& rTokenStr, sal_Int32 nSharedId ) :
    maAddress(rAddr), maRange(rRange), maTokenStr(rTokenStr), mnSharedId(nSharedId) {}

FormulaBuffer::SharedFormulaDesc::SharedFormulaDesc(
    const css::table::CellAddress& rAddr, sal_Int32 nSharedId,
    const OUString& rCellValue, sal_Int32 nValueType ) :
    maAddress(rAddr), mnSharedId(nSharedId), maCellValue(rCellValue), mnValueType(nValueType) {}

FormulaBuffer::SheetItem::SheetItem() :
    mpCellFormulas(NULL),
    mpArrayFormulas(NULL),
    mpCellFormulaValues(NULL),
    mpSharedFormulaEntries(NULL),
    mpSharedFormulaIDs(NULL) {}

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

    const size_t nThreadCount = 1;
    ScDocumentImport& rDoc = getDocImport();
    rDoc.getDoc().SetAutoNameCache(new ScAutoNameCache(&rDoc.getDoc()));
    ScExternalRefManager::ApiGuard aExtRefGuard(&rDoc.getDoc());

    SCTAB nTabCount = rDoc.getDoc().GetTableCount();

    // Fetch all the formulas to process first.
    std::vector<SheetItem> aSheetItems;
    aSheetItems.reserve(nTabCount);
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
        aSheetItems.push_back(getSheetItem(nTab));

    std::vector<SheetItem>::iterator it = aSheetItems.begin(), itEnd = aSheetItems.end();

    if (nThreadCount == 1)
    {
        for (; it != itEnd; ++it)
            processSheetFormulaCells(rDoc, *it, *rDoc.getDoc().GetFormatTable(), getExternalLinks().getLinkInfos());
    }
    else
    {
        typedef rtl::Reference<WorkerThread> WorkerThreadRef;
        std::vector<WorkerThreadRef> aThreads;
        aThreads.reserve(nThreadCount);
        // TODO: Right now we are spawning multiple threads all at once and block
        // on them all at once.  Any more clever thread management would require
        // use of condition variables which our own osl thread framework seems to
        // lack.
        while (it != itEnd)
        {
            for (size_t i = 0; i < nThreadCount; ++i)
            {
                if (it == itEnd)
                    break;

                WorkerThreadRef xThread(new WorkerThread(rDoc, *it, rDoc.getDoc().CreateFormatTable(), getExternalLinks().getLinkInfos()));
                ++it;
                aThreads.push_back(xThread);
                xThread->launch();
            }

            for (size_t i = 0, n = aThreads.size(); i < n; ++i)
            {
                if (aThreads[i].is())
                    aThreads[i]->join();
            }

            aThreads.clear();
        }
    }

    rDoc.getDoc().SetAutoNameCache(NULL);

    xFormulaBar->setPosition( 1.0 );
}

FormulaBuffer::SheetItem FormulaBuffer::getSheetItem( SCTAB nTab )
{
    osl::MutexGuard aGuard(&maMtxData);

    SheetItem aItem;

    if( (size_t) nTab >= maCellFormulas.size() )
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
    const table::CellAddress& rAddress, const table::CellRangeAddress& rRange,
    sal_Int32 nSharedId, const OUString& rTokens )
{
    assert( rAddress.Sheet >= 0 && (size_t)rAddress.Sheet < maSharedFormulas.size() );
    std::vector<SharedFormulaEntry>& rSharedFormulas = maSharedFormulas[ rAddress.Sheet ];
    SharedFormulaEntry aEntry(rAddress, rRange, rTokens, nSharedId);
    rSharedFormulas.push_back( aEntry );
}

void FormulaBuffer::setCellFormula( const css::table::CellAddress& rAddress, const OUString& rTokenStr )
{
    assert( rAddress.Sheet >= 0 && (size_t)rAddress.Sheet < maCellFormulas.size() );
    maCellFormulas[ rAddress.Sheet ].push_back( TokenAddressItem( rTokenStr, rAddress ) );
}

void FormulaBuffer::setCellFormula(
    const table::CellAddress& rAddress, sal_Int32 nSharedId, const OUString& rCellValue, sal_Int32 nValueType )
{
    assert( rAddress.Sheet >= 0 && (size_t)rAddress.Sheet < maSharedFormulaIds.size() );
    maSharedFormulaIds[rAddress.Sheet].push_back(
        SharedFormulaDesc(rAddress, nSharedId, rCellValue, nValueType));
}

void FormulaBuffer::setCellArrayFormula( const css::table::CellRangeAddress& rRangeAddress, const css::table::CellAddress& rTokenAddress, const OUString& rTokenStr )
{

    TokenAddressItem tokenPair( rTokenStr, rTokenAddress );
    assert( rRangeAddress.Sheet >= 0 && (size_t)rRangeAddress.Sheet < maCellArrayFormulas.size() );
    maCellArrayFormulas[ rRangeAddress.Sheet ].push_back( TokenRangeAddressItem( tokenPair, rRangeAddress ) );
}

void FormulaBuffer::setCellFormulaValue(
        const css::table::CellAddress& rAddress, const OUString& rValueStr, sal_Int32 nCellType )
{
    assert( rAddress.Sheet >= 0 && (size_t)rAddress.Sheet < maCellFormulaValues.size() );
    FormulaValue aVal;
    aVal.maCellAddress = rAddress;
    aVal.maValueStr = rValueStr;
    aVal.mnCellType = nCellType;
    maCellFormulaValues[rAddress.Sheet].push_back(aVal);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
