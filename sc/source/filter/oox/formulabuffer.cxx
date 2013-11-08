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
#include "oox/token/tokens.hxx"

using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::container;

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace oox { namespace xls {

namespace {

/**
 * Cache the token array for the last cell position in each column. We use
 * one cache per sheet.
 */
class CachedTokenArray : boost::noncopyable
{
    struct Item : boost::noncopyable
    {
        SCROW mnRow;
        boost::scoped_ptr<ScTokenArray> mpCode;

        Item() : mnRow(-1), mpCode(NULL) {}
        Item( SCROW nRow, ScTokenArray* p ) : mnRow(nRow), mpCode(p) {}
    };

    typedef boost::unordered_map<SCCOL, Item*> ColCacheType;
    ColCacheType maCache;
    ScDocument& mrDoc;

public:
    CachedTokenArray( ScDocument& rDoc ) : mrDoc(rDoc) {}

    ~CachedTokenArray()
    {
        ColCacheType::const_iterator it = maCache.begin(), itEnd = maCache.end();
        for (; it != itEnd; ++it)
            delete it->second;
    }

    const ScTokenArray* get( const ScAddress& rPos, const OUString& rFormula ) const
    {
        // Check if a token array is cached for this column.
        ColCacheType::const_iterator it = maCache.find(rPos.Col());
        if (it == maCache.end())
            return NULL;

        const Item& rCached = *it->second;
        ScCompiler aComp(&mrDoc, rPos, *rCached.mpCode);
        aComp.SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);
        OUStringBuffer aBuf;
        aComp.CreateStringFromTokenArray(aBuf);
        OUString aPredicted = aBuf.makeStringAndClear();
        if (rFormula == aPredicted)
            return rCached.mpCode.get();

        return NULL;
    }

    void store( const ScAddress& rPos, const ScTokenArray& rArray )
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

        it->second->mnRow = rPos.Row();
        it->second->mpCode.reset(rArray.Clone());
    }
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
            aComp.SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);
            ScTokenArray* pArray = aComp.CompileString(rTokenStr);
            if (pArray)
                aGroups.set(nId, pArray);
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
                pCell->SetDirty(true);
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
                    pCell->SetDirty(true);
            }
        }
    }
}

void applyCellFormulas(
    ScDocumentImport& rDoc, CachedTokenArray& rCache, SvNumberFormatter& rFormatter,
    const std::vector<FormulaBuffer::TokenAddressItem>& rCells )
{
    std::vector<FormulaBuffer::TokenAddressItem>::const_iterator it = rCells.begin(), itEnd = rCells.end();
    for (; it != itEnd; ++it)
    {
        ScAddress aPos;
        ScUnoConversion::FillScAddress(aPos, it->maCellAddress);
        const ScTokenArray* p = rCache.get(aPos, it->maTokenStr);
        if (p)
        {
            // Use the cached version to avoid re-compilation.
            rDoc.setFormulaCell(aPos, p->Clone());
            continue;
        }

        ScCompiler aCompiler(&rDoc.getDoc(), aPos);
        aCompiler.SetNumberFormatter(&rFormatter);
        aCompiler.SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);
        ScTokenArray* pCode = aCompiler.CompileString(it->maTokenStr);
        if (!pCode)
            continue;

        rDoc.setFormulaCell(aPos, pCode);
        rCache.store(aPos, *pCode);
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
        aComp.SetGrammar(formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);
        boost::scoped_ptr<ScTokenArray> pArray(aComp.CompileString(it->maTokenAndAddress.maTokenStr));
        if (pArray)
            rDoc.setMatrixCells(aRange, *pArray, formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX);
    }
}

void applyCellFormulaValues(
    ScDocumentImport& rDoc, const std::vector<FormulaBuffer::ValueAddressPair>& rVector )
{
    std::vector<FormulaBuffer::ValueAddressPair>::const_iterator it = rVector.begin(), itEnd = rVector.end();
    for (; it != itEnd; ++it)
    {
        ScAddress aCellPos;
        ScUnoConversion::FillScAddress(aCellPos, it->first);
        ScFormulaCell* pCell = rDoc.getDoc().GetFormulaCell(aCellPos);
        if (pCell)
        {
            pCell->SetHybridDouble(it->second);
            pCell->ResetDirty();
            pCell->SetChanged(false);
        }
    }
}

class WorkerThread : public salhelper::Thread
{
    ScDocumentImport& mrDoc;
    FormulaBuffer::SheetItem& mrItem;
    boost::scoped_ptr<SvNumberFormatter> mpFormatter;

    WorkerThread( const WorkerThread& );
    WorkerThread& operator= ( const WorkerThread& );

public:
    WorkerThread( ScDocumentImport& rDoc, FormulaBuffer::SheetItem& rItem, SvNumberFormatter* pFormatter ) :
        salhelper::Thread("xlsx-import-formula-buffer-worker-thread"),
        mrDoc(rDoc), mrItem(rItem), mpFormatter(pFormatter) {}

    virtual ~WorkerThread() {}

protected:
    virtual void execute()
    {
        if (mrItem.mpSharedFormulaEntries && mrItem.mpSharedFormulaIDs)
            applySharedFormulas(mrDoc, *mpFormatter, *mrItem.mpSharedFormulaEntries, *mrItem.mpSharedFormulaIDs);

        if (mrItem.mpCellFormulas)
        {
            CachedTokenArray aCache(mrDoc.getDoc());
            applyCellFormulas(mrDoc, aCache, *mpFormatter, *mrItem.mpCellFormulas);
        }

        if (mrItem.mpArrayFormulas)
            applyArrayFormulas(mrDoc, *mpFormatter, *mrItem.mpArrayFormulas);

        if (mrItem.mpCellFormulaValues)
            applyCellFormulaValues(mrDoc, *mrItem.mpCellFormulaValues);
    }
};

}

FormulaBuffer::SharedFormulaEntry::SharedFormulaEntry(
    const table::CellAddress& rAddr, const table::CellRangeAddress& rRange,
    const OUString& rTokenStr, sal_Int32 nSharedId ) :
    maAddress(rAddr), maRange(rRange), maTokenStr(rTokenStr), mnSharedId(nSharedId) {}

FormulaBuffer::SharedFormulaDesc::SharedFormulaDesc(
    const com::sun::star::table::CellAddress& rAddr, sal_Int32 nSharedId,
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

    typedef rtl::Reference<WorkerThread> WorkerThreadRef;
    std::vector<WorkerThreadRef> aThreads;
    aThreads.reserve(nThreadCount);

    std::vector<SheetItem>::iterator it = aSheetItems.begin(), itEnd = aSheetItems.end();

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

            WorkerThreadRef xThread(new WorkerThread(rDoc, *it, rDoc.getDoc().CreateFormatTable()));
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

    rDoc.getDoc().SetAutoNameCache(NULL);

    xFormulaBar->setPosition( 1.0 );
}

FormulaBuffer::SheetItem FormulaBuffer::getSheetItem( SCTAB nTab )
{
    osl::MutexGuard aGuard(&maMtxData);

    SheetItem aItem;
    {
        FormulaDataMap::iterator it = maCellFormulas.find(nTab);
        if (it != maCellFormulas.end())
            aItem.mpCellFormulas = &it->second;
    }

    {
        ArrayFormulaDataMap::iterator it = maCellArrayFormulas.find(nTab);
        if (it != maCellArrayFormulas.end())
            aItem.mpArrayFormulas = &it->second;
    }

    {
        FormulaValueMap::iterator it = maCellFormulaValues.find(nTab);
        if (it != maCellFormulaValues.end())
            aItem.mpCellFormulaValues = &it->second;
    }

    {
        SheetToFormulaEntryMap::iterator it = maSharedFormulas.find(nTab);
        if (it != maSharedFormulas.end())
            aItem.mpSharedFormulaEntries = &it->second;
    }

    {
        SheetToSharedFormulaid::iterator it = maSharedFormulaIds.find(nTab);
        if (it != maSharedFormulaIds.end())
            aItem.mpSharedFormulaIDs = &it->second;
    }
    return aItem;
}

void FormulaBuffer::createSharedFormulaMapEntry(
    const table::CellAddress& rAddress, const table::CellRangeAddress& rRange,
    sal_Int32 nSharedId, const OUString& rTokens )
{
    std::vector<SharedFormulaEntry>& rSharedFormulas = maSharedFormulas[ rAddress.Sheet ];
    SharedFormulaEntry aEntry(rAddress, rRange, rTokens, nSharedId);
    rSharedFormulas.push_back( aEntry );
}

void FormulaBuffer::setCellFormula( const ::com::sun::star::table::CellAddress& rAddress, const OUString& rTokenStr )
{
    maCellFormulas[ rAddress.Sheet ].push_back( TokenAddressItem( rTokenStr, rAddress ) );
}

void FormulaBuffer::setCellFormula(
    const table::CellAddress& rAddress, sal_Int32 nSharedId, const OUString& rCellValue, sal_Int32 nValueType )
{
    maSharedFormulaIds[rAddress.Sheet].push_back(
        SharedFormulaDesc(rAddress, nSharedId, rCellValue, nValueType));
}

void FormulaBuffer::setCellArrayFormula( const ::com::sun::star::table::CellRangeAddress& rRangeAddress, const ::com::sun::star::table::CellAddress& rTokenAddress, const OUString& rTokenStr )
{

    TokenAddressItem tokenPair( rTokenStr, rTokenAddress );
    maCellArrayFormulas[ rRangeAddress.Sheet ].push_back( TokenRangeAddressItem( tokenPair, rRangeAddress ) );
}

void FormulaBuffer::setCellFormulaValue( const ::com::sun::star::table::CellAddress& rAddress, double fValue )
{
    maCellFormulaValues[ rAddress.Sheet ].push_back( ValueAddressPair( rAddress, fValue ) );
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
