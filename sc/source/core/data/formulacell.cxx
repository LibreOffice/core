/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <config_features.h>

#include <sal/config.h>

#include <cassert>
#include <cstdlib>

#include <formulacell.hxx>
#include <grouptokenconverter.hxx>

#include <compiler.hxx>
#include <document.hxx>
#include <globalnames.hxx>
#include <cellvalue.hxx>
#include <interpre.hxx>
#include <macromgr.hxx>
#include <refupdat.hxx>
#include <recursionhelper.hxx>
#include <docoptio.hxx>
#include <rangenam.hxx>
#include <rangelst.hxx>
#include <dbdata.hxx>
#include <progress.hxx>
#include <scmatrix.hxx>
#include <rechead.hxx>
#include <scitems.hxx>
#include <validat.hxx>
#include <editutil.hxx>
#include <chgtrack.hxx>
#include <tokenarray.hxx>

#include <comphelper/threadpool.hxx>
#include <formula/errorcodes.hxx>
#include <formula/vectortoken.hxx>
#include <svl/intitem.hxx>
#include <o3tl/make_unique.hxx>
#include <rtl/strbuf.hxx>
#include <formulagroup.hxx>
#include <listenercontext.hxx>
#include <types.hxx>
#include <scopetools.hxx>
#include <refupdatecontext.hxx>
#include <tokenstringcontext.hxx>
#include <refhint.hxx>
#include <listenerquery.hxx>
#include <listenerqueryids.hxx>
#include <grouparealistener.hxx>
#include <formulalogger.hxx>

#if HAVE_FEATURE_OPENCL
#include <opencl/openclwrapper.hxx>
#endif

#include <memory>
#include <map>

using namespace formula;

IMPL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )

#define DEBUG_CALCULATION 0
#if DEBUG_CALCULATION
static bool bDebugCalculationActive = false;                // Set to true for global active init,
static ScAddress aDebugCalculationTriggerAddress(1,2,0);    // or on cell Sheet1.B3, whatever you like

struct DebugCalculationEntry
{
          ScAddress     maPos;
          OUString      maResult;
    const ScDocument*   mpDoc;
          sal_uInt32    mnGroup;
          sal_uInt16    mnRecursion;

    DebugCalculationEntry( const ScAddress& rPos, ScDocument* pDoc, sal_uInt32 nGroup ) :
        maPos(rPos),
        mpDoc(pDoc),
        mnGroup(nGroup),
        mnRecursion(pDoc->GetRecursionHelper().GetRecursionCount())
    {
    }
};

/** Debug/dump formula cell calculation chain.
    Either, somewhere set aDC.mbActive=true, or
    aDC.maTrigger=ScAddress(col,row,tab) of interest from where to start.
    This does not work for deep recursion > MAXRECURSION, the results are
    somewhat.. funny.. ;)
 */
static struct DebugCalculation
{
    std::vector< DebugCalculationEntry >    mvPos;
    std::vector< DebugCalculationEntry >    mvResults;
    ScAddress                               maTrigger;
    sal_uInt32                              mnGroup;
    bool                                    mbActive;
    bool                                    mbSwitchOff;
    bool                                    mbPrint;
    bool                                    mbPrintResults;

    DebugCalculation() : mnGroup(0), mbActive(bDebugCalculationActive), mbSwitchOff(false),
    mbPrint(true), mbPrintResults(false) {}

    /** Print chain in encountered dependency order. */
    void print() const
    {
        for (auto const& it : mvPos)
        {
            OUString aStr( it.maPos.Format( ScRefFlags::VALID | ScRefFlags::TAB_3D, it.mpDoc) +
                    " [" + OUString::number( it.mnRecursion) + "," + OUString::number( it.mnGroup) + "]");
            fprintf( stderr, "%s -> ", aStr.toUtf8().getStr());
        }
        fprintf( stderr, "%s", "END\n");
    }

    /** Print chain results. */
    void printResults() const
    {
        for (auto const& it : mvResults)
        {
            OUString aStr( it.maPos.Format( ScRefFlags::VALID | ScRefFlags::TAB_3D, it.mpDoc));
            aStr += " (" + it.maResult + ")";
            fprintf( stderr, "%s, ", aStr.toUtf8().getStr());
        }
        fprintf( stderr, "%s", "END\n");
    }

    void storeResult( const svl::SharedString& rStr )
    {
        if (mbActive && !mvPos.empty())
            mvPos.back().maResult = "\"" + rStr.getString() + "\"";
    }

    void storeResult( const double& fVal )
    {
        if (mbActive && !mvPos.empty())
            mvPos.back().maResult = rtl::math::doubleToUString( fVal, rtl_math_StringFormat_G, 2, '.', true);
    }

    void storeResultError( const sal_uInt16& nErr )
    {
        if (mbActive && !mvPos.empty())
            mvPos.back().maResult = "Err:" + OUString::number( nErr);
    }

    void enterGroup()
    {
        ++mnGroup;
    }

    void leaveGroup()
    {
        --mnGroup;
    }

} aDC;

struct DebugCalculationStacker
{
    DebugCalculationStacker( const ScAddress& rPos, ScDocument* pDoc )
    {
        if (!aDC.mbActive && rPos == aDC.maTrigger)
            aDC.mbActive = aDC.mbSwitchOff = true;
        if (aDC.mbActive)
        {
            aDC.mvPos.push_back( DebugCalculationEntry( rPos, pDoc, aDC.mnGroup));
            aDC.mbPrint = true;
        }
    }

    ~DebugCalculationStacker()
    {
        if (aDC.mbActive)
        {
            if (!aDC.mvPos.empty())
            {
                if (aDC.mbPrint)
                {
                    aDC.print();
                    aDC.mbPrint = false;
                }
                if (aDC.mbPrintResults)
                {
                    // Store results until final result is available, reversing order.
                    aDC.mvResults.push_back( aDC.mvPos.back());
                }
                aDC.mvPos.pop_back();
                if (aDC.mbPrintResults && aDC.mvPos.empty())
                {
                    aDC.printResults();
                    std::vector< DebugCalculationEntry >().swap( aDC.mvResults);
                }
                if (aDC.mbSwitchOff && aDC.mvPos.empty())
                    aDC.mbActive = false;
            }
        }
    }
};
#endif

namespace {

// More or less arbitrary, of course all recursions must fit into available
// stack space (which is what on all systems we don't know yet?). Choosing a
// lower value may be better than trying a much higher value that also isn't
// sufficient but temporarily leads to high memory consumption. On the other
// hand, if the value fits all recursions, execution is quicker as no resumes
// are necessary. Could be made a configurable option.
// Allow for a year's calendar (366).
const sal_uInt16 MAXRECURSION = 400;

using std::deque;

typedef SCCOLROW(*DimensionSelector)(const ScAddress&, const ScSingleRefData&);

SCCOLROW lcl_GetCol(const ScAddress& rPos, const ScSingleRefData& rData)
{
    return rData.toAbs(rPos).Col();
}

SCCOLROW lcl_GetRow(const ScAddress& rPos, const ScSingleRefData& rData)
{
    return rData.toAbs(rPos).Row();
}

SCCOLROW lcl_GetTab(const ScAddress& rPos, const ScSingleRefData& rData)
{
    return rData.toAbs(rPos).Tab();
}

/** Check if both references span the same range in selected dimension.
 */
bool
lcl_checkRangeDimension(
    const ScAddress& rPos, const SingleDoubleRefProvider& rRef1, const SingleDoubleRefProvider& rRef2,
    const DimensionSelector aWhich)
{
    return aWhich(rPos, rRef1.Ref1) == aWhich(rPos, rRef2.Ref1) &&
        aWhich(rPos, rRef1.Ref2) == aWhich(rPos, rRef2.Ref2);
}

bool
lcl_checkRangeDimensions(
    const ScAddress& rPos, const SingleDoubleRefProvider& rRef1, const SingleDoubleRefProvider& rRef2,
    bool& bCol, bool& bRow, bool& bTab)
{
    const bool bSameCols(lcl_checkRangeDimension(rPos, rRef1, rRef2, lcl_GetCol));
    const bool bSameRows(lcl_checkRangeDimension(rPos, rRef1, rRef2, lcl_GetRow));
    const bool bSameTabs(lcl_checkRangeDimension(rPos, rRef1, rRef2, lcl_GetTab));

    // Test if exactly two dimensions are equal
    if (int(bSameCols) + int(bSameRows) + int(bSameTabs) == 2)
    {
        bCol = !bSameCols;
        bRow = !bSameRows;
        bTab = !bSameTabs;
        return true;
    }
    return false;
}

/** Check if references in given reference list can possibly
    form a range. To do that, two of their dimensions must be the same.
 */
bool
lcl_checkRangeDimensions(
    const ScAddress& rPos,
    const deque<formula::FormulaToken*>::const_iterator& rBegin,
    const deque<formula::FormulaToken*>::const_iterator& rEnd,
    bool& bCol, bool& bRow, bool& bTab)
{
    deque<formula::FormulaToken*>::const_iterator aCur(rBegin);
    ++aCur;
    const SingleDoubleRefProvider aRef(**rBegin);
    bool bOk(false);
    {
        const SingleDoubleRefProvider aRefCur(**aCur);
        bOk = lcl_checkRangeDimensions(rPos, aRef, aRefCur, bCol, bRow, bTab);
    }
    while (bOk && aCur != rEnd)
    {
        const SingleDoubleRefProvider aRefCur(**aCur);
        bool bColTmp(false);
        bool bRowTmp(false);
        bool bTabTmp(false);
        bOk = lcl_checkRangeDimensions(rPos, aRef, aRefCur, bColTmp, bRowTmp, bTabTmp);
        bOk = bOk && (bCol == bColTmp && bRow == bRowTmp && bTab == bTabTmp);
        ++aCur;
    }

    return bOk && aCur == rEnd;
}

class LessByReference
{
    ScAddress maPos;
    DimensionSelector maFunc;
public:
    LessByReference(const ScAddress& rPos, const DimensionSelector& rFunc) :
        maPos(rPos), maFunc(rFunc) {}

    bool operator() (const formula::FormulaToken* pRef1, const formula::FormulaToken* pRef2)
    {
        const SingleDoubleRefProvider aRef1(*pRef1);
        const SingleDoubleRefProvider aRef2(*pRef2);
        return maFunc(maPos, aRef1.Ref1) < maFunc(maPos, aRef2.Ref1);
    }
};

/**
 * Returns true if range denoted by token p2 starts immediately after range
 * denoted by token p1. Dimension, in which the comparison takes place, is
 * given by maFunc.
 */
class AdjacentByReference
{
    ScAddress maPos;
    DimensionSelector maFunc;
public:
    AdjacentByReference(const ScAddress& rPos, DimensionSelector aFunc) :
        maPos(rPos), maFunc(aFunc) {}

    bool operator() (const formula::FormulaToken* p1, const formula::FormulaToken* p2)
    {
        const SingleDoubleRefProvider aRef1(*p1);
        const SingleDoubleRefProvider aRef2(*p2);
        return maFunc(maPos, aRef2.Ref1) - maFunc(maPos, aRef1.Ref2) == 1;
    }
};

bool
lcl_checkIfAdjacent(
    const ScAddress& rPos, const deque<formula::FormulaToken*>& rReferences, const DimensionSelector aWhich)
{
    typedef deque<formula::FormulaToken*>::const_iterator Iter;
    Iter aBegin(rReferences.begin());
    Iter aEnd(rReferences.end());
    Iter aBegin1(aBegin);
    ++aBegin1;
    --aEnd;
    return std::equal(aBegin, aEnd, aBegin1, AdjacentByReference(rPos, aWhich));
}

void
lcl_fillRangeFromRefList(
    const ScAddress& aPos, const deque<formula::FormulaToken*>& rReferences, ScRange& rRange)
{
    const ScSingleRefData aStart(
            SingleDoubleRefProvider(*rReferences.front()).Ref1);
    rRange.aStart = aStart.toAbs(aPos);
    const ScSingleRefData aEnd(
            SingleDoubleRefProvider(*rReferences.back()).Ref2);
    rRange.aEnd = aEnd.toAbs(aPos);
}

bool
lcl_refListFormsOneRange(
        const ScAddress& rPos, deque<formula::FormulaToken*>& rReferences,
        ScRange& rRange)
{
    if (rReferences.size() == 1)
    {
        lcl_fillRangeFromRefList(rPos, rReferences, rRange);
        return true;
    }

    bool bCell(false);
    bool bRow(false);
    bool bTab(false);
    if (lcl_checkRangeDimensions(rPos, rReferences.begin(), rReferences.end(), bCell, bRow, bTab))
    {
        DimensionSelector aWhich;
        if (bCell)
        {
            aWhich = lcl_GetCol;
        }
        else if (bRow)
        {
            aWhich = lcl_GetRow;
        }
        else if (bTab)
        {
            aWhich = lcl_GetTab;
        }
        else
        {
            OSL_FAIL( "lcl_checkRangeDimensions shouldn't allow that!");
            aWhich = lcl_GetRow;    // initialize to avoid warning
        }

        // Sort the references by start of range
        std::sort(rReferences.begin(), rReferences.end(), LessByReference(rPos, aWhich));
        if (lcl_checkIfAdjacent(rPos, rReferences, aWhich))
        {
            lcl_fillRangeFromRefList(rPos, rReferences, rRange);
            return true;
        }
    }
    return false;
}

bool lcl_isReference(const FormulaToken& rToken)
{
    return
        rToken.GetType() == svSingleRef ||
        rToken.GetType() == svDoubleRef;
}

void adjustRangeName(formula::FormulaToken* pToken, ScDocument& rNewDoc, const ScDocument* pOldDoc,
        const ScAddress& rNewPos, const ScAddress& rOldPos, bool bGlobalNamesToLocal)
{
    ScRangeData* pRangeData = nullptr;
    SCTAB nSheet = pToken->GetSheet();
    sal_uInt16 nIndex = pToken->GetIndex();
    if (!pOldDoc->CopyAdjustRangeName( nSheet, nIndex, pRangeData, rNewDoc, rNewPos, rOldPos, bGlobalNamesToLocal, true))
        return; // nothing to do

    if (!pRangeData)
    {
        // If this happened we have a real problem.
        pToken->SetIndex(0);
        OSL_FAIL("inserting the range name should not fail");
        return;
    }

    pToken->SetIndex(nIndex);
    pToken->SetSheet(nSheet);
}

void adjustDBRange(formula::FormulaToken* pToken, ScDocument& rNewDoc, const ScDocument* pOldDoc)
{
    ScDBCollection* pOldDBCollection = pOldDoc->GetDBCollection();
    if (!pOldDBCollection)
        return;//strange error case, don't do anything
    ScDBCollection::NamedDBs& aOldNamedDBs = pOldDBCollection->getNamedDBs();
    ScDBData* pDBData = aOldNamedDBs.findByIndex(pToken->GetIndex());
    if (!pDBData)
        return; //invalid index
    OUString aDBName = pDBData->GetUpperName();

    //search in new document
    ScDBCollection* pNewDBCollection = rNewDoc.GetDBCollection();
    if (!pNewDBCollection)
    {
        pNewDBCollection = new ScDBCollection(&rNewDoc);
        rNewDoc.SetDBCollection(pNewDBCollection);
    }
    ScDBCollection::NamedDBs& aNewNamedDBs = pNewDBCollection->getNamedDBs();
    ScDBData* pNewDBData = aNewNamedDBs.findByUpperName(aDBName);
    if (!pNewDBData)
    {
        pNewDBData = new ScDBData(*pDBData);
        bool ins = aNewNamedDBs.insert(pNewDBData);
        assert(ins); (void)ins;
    }
    pToken->SetIndex(pNewDBData->GetIndex());
}

struct AreaListenerKey
{
    ScRange maRange;
    bool mbStartFixed;
    bool mbEndFixed;

    AreaListenerKey( const ScRange& rRange, bool bStartFixed, bool bEndFixed ) :
        maRange(rRange), mbStartFixed(bStartFixed), mbEndFixed(bEndFixed) {}

    bool operator < ( const AreaListenerKey& r ) const
    {
        if (maRange.aStart.Tab() != r.maRange.aStart.Tab())
            return maRange.aStart.Tab() < r.maRange.aStart.Tab();
        if (maRange.aStart.Col() != r.maRange.aStart.Col())
            return maRange.aStart.Col() < r.maRange.aStart.Col();
        if (maRange.aStart.Row() != r.maRange.aStart.Row())
            return maRange.aStart.Row() < r.maRange.aStart.Row();
        if (maRange.aEnd.Tab() != r.maRange.aEnd.Tab())
            return maRange.aEnd.Tab() < r.maRange.aEnd.Tab();
        if (maRange.aEnd.Col() != r.maRange.aEnd.Col())
            return maRange.aEnd.Col() < r.maRange.aEnd.Col();
        if (maRange.aEnd.Row() != r.maRange.aEnd.Row())
            return maRange.aEnd.Row() < r.maRange.aEnd.Row();
        if (mbStartFixed != r.mbStartFixed)
            return r.mbStartFixed;
        if (mbEndFixed != r.mbEndFixed)
            return r.mbEndFixed;

        return false;
    }
};

typedef std::map<AreaListenerKey, std::unique_ptr<sc::FormulaGroupAreaListener>> AreaListenersType;

}

struct ScFormulaCellGroup::Impl
{
    AreaListenersType m_AreaListeners;
};

ScFormulaCellGroup::ScFormulaCellGroup() :
    mpImpl(new Impl),
    mnRefCount(0),
    mpCode(nullptr),
    mpTopCell(nullptr),
    mnLength(0),
    mnFormatType(css::util::NumberFormat::NUMBER),
    mbInvariant(false),
    mbSubTotal(false),
    meCalcState(sc::GroupCalcEnabled)
{
}

ScFormulaCellGroup::~ScFormulaCellGroup()
{
    delete mpCode;
}

void ScFormulaCellGroup::setCode( const ScTokenArray& rCode )
{
    delete mpCode;
    mpCode = rCode.Clone();
    mbInvariant = mpCode->IsInvariant();
    mpCode->GenHash();
}

void ScFormulaCellGroup::setCode( ScTokenArray* pCode )
{
    delete mpCode;
    mpCode = pCode; // takes ownership of the token array.
    mbInvariant = mpCode->IsInvariant();
    mpCode->GenHash();
}

void ScFormulaCellGroup::compileCode(
    ScDocument& rDoc, const ScAddress& rPos, FormulaGrammar::Grammar eGram )
{
    if (!mpCode)
        return;

    if (mpCode->GetLen() && mpCode->GetCodeError() == FormulaError::NONE && !mpCode->GetCodeLen())
    {
        ScCompiler aComp(&rDoc, rPos, *mpCode, eGram);
        mbSubTotal = aComp.CompileTokenArray();
        mnFormatType = aComp.GetNumFormatType();
    }
    else
    {
        mbSubTotal = mpCode->HasOpCodeRPN( ocSubTotal ) || mpCode->HasOpCodeRPN( ocAggregate );
    }
}

sc::FormulaGroupAreaListener* ScFormulaCellGroup::getAreaListener(
    ScFormulaCell** ppTopCell, const ScRange& rRange, bool bStartFixed, bool bEndFixed )
{
    AreaListenerKey aKey(rRange, bStartFixed, bEndFixed);

    AreaListenersType::iterator it = mpImpl->m_AreaListeners.lower_bound(aKey);
    if (it == mpImpl->m_AreaListeners.end() || mpImpl->m_AreaListeners.key_comp()(aKey, it->first))
    {
        // Insert a new one.
        it = mpImpl->m_AreaListeners.insert(
            it, std::make_pair(aKey, o3tl::make_unique<sc::FormulaGroupAreaListener>(
                rRange, *(*ppTopCell)->GetDocument(), (*ppTopCell)->aPos, mnLength, bStartFixed, bEndFixed)));
    }

    return it->second.get();
}

void ScFormulaCellGroup::endAllGroupListening( ScDocument& rDoc )
{
    AreaListenersType::iterator it = mpImpl->m_AreaListeners.begin(), itEnd = mpImpl->m_AreaListeners.end();
    for (; it != itEnd; ++it)
    {
        sc::FormulaGroupAreaListener *const pListener = it->second.get();
        ScRange aListenRange = pListener->getListeningRange();
        // This "always listen" special range is never grouped.
        bool bGroupListening = (aListenRange != BCA_LISTEN_ALWAYS);
        rDoc.EndListeningArea(aListenRange, bGroupListening, pListener);
    }

    mpImpl->m_AreaListeners.clear();
}

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos ) :
    eTempGrammar(formula::FormulaGrammar::GRAM_DEFAULT),
    pCode(new ScTokenArray),
    pDocument(pDoc),
    pPrevious(nullptr),
    pNext(nullptr),
    pPreviousTrack(nullptr),
    pNextTrack(nullptr),
    nSeenInIteration(0),
    nFormatType(css::util::NumberFormat::NUMBER),
    cMatrixFlag(ScMatrixMode::NONE),
    bDirty(false),
    bChanged(false),
    bRunning(false),
    bCompile(false),
    bSubTotal(false),
    bIsIterCell(false),
    bInChangeTrack(false),
    bTableOpDirty(false),
    bNeedListening(false),
    mbNeedsNumberFormat(false),
    mbAllowNumberFormatChange(false),
    mbPostponedDirty(false),
    mbIsExtRef(false),
    aPos(rPos)
{
}

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const OUString& rFormula,
                              const FormulaGrammar::Grammar eGrammar,
                              ScMatrixMode cMatInd ) :
    eTempGrammar( eGrammar),
    pCode( nullptr ),
    pDocument( pDoc ),
    pPrevious(nullptr),
    pNext(nullptr),
    pPreviousTrack(nullptr),
    pNextTrack(nullptr),
    nSeenInIteration(0),
    nFormatType ( css::util::NumberFormat::NUMBER ),
    cMatrixFlag ( cMatInd ),
    bDirty( true ), // -> Because of the use of the Auto Pilot Function was: cMatInd != 0
    bChanged( false ),
    bRunning( false ),
    bCompile( false ),
    bSubTotal( false ),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    mbAllowNumberFormatChange(false),
    mbPostponedDirty(false),
    mbIsExtRef(false),
    aPos(rPos)
{
    Compile( rFormula, true, eGrammar );    // bNoListening, Insert does that
    if (!pCode)
        // We need to have a non-NULL token array instance at all times.
        pCode = new ScTokenArray;
}

ScFormulaCell::ScFormulaCell(
    ScDocument* pDoc, const ScAddress& rPos, ScTokenArray* pArray,
    const FormulaGrammar::Grammar eGrammar, ScMatrixMode cMatInd ) :
    eTempGrammar( eGrammar),
    pCode(pArray),
    pDocument( pDoc ),
    pPrevious(nullptr),
    pNext(nullptr),
    pPreviousTrack(nullptr),
    pNextTrack(nullptr),
    nSeenInIteration(0),
    nFormatType ( css::util::NumberFormat::NUMBER ),
    cMatrixFlag ( cMatInd ),
    bDirty( true ),
    bChanged( false ),
    bRunning( false ),
    bCompile( false ),
    bSubTotal( false ),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    mbAllowNumberFormatChange(false),
    mbPostponedDirty(false),
    mbIsExtRef(false),
    aPos(rPos)
{
    assert(pArray); // Never pass a NULL pointer here.

    // Generate RPN token array.
    if (pCode->GetLen() && pCode->GetCodeError() == FormulaError::NONE && !pCode->GetCodeLen())
    {
        ScCompiler aComp( pDocument, aPos, *pCode, eTempGrammar);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
    }
    else
    {
        if ( pCode->HasOpCodeRPN( ocSubTotal ) || pCode->HasOpCodeRPN( ocAggregate ) )
            bSubTotal = true;
    }

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);

    pCode->GenHash();
}

ScFormulaCell::ScFormulaCell(
    ScDocument* pDoc, const ScAddress& rPos, const ScTokenArray& rArray,
    const FormulaGrammar::Grammar eGrammar, ScMatrixMode cMatInd ) :
    eTempGrammar( eGrammar),
    pCode(new ScTokenArray(rArray)),
    pDocument( pDoc ),
    pPrevious(nullptr),
    pNext(nullptr),
    pPreviousTrack(nullptr),
    pNextTrack(nullptr),
    nSeenInIteration(0),
    nFormatType ( css::util::NumberFormat::NUMBER ),
    cMatrixFlag ( cMatInd ),
    bDirty( true ),
    bChanged( false ),
    bRunning( false ),
    bCompile( false ),
    bSubTotal( false ),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    mbAllowNumberFormatChange(false),
    mbPostponedDirty(false),
    mbIsExtRef(false),
    aPos(rPos)
{
    // RPN array generation
    if( pCode->GetLen() && pCode->GetCodeError() == FormulaError::NONE && !pCode->GetCodeLen() )
    {
        ScCompiler aComp( pDocument, aPos, *pCode, eTempGrammar);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
    }
    else
    {
        if ( pCode->HasOpCodeRPN( ocSubTotal ) || pCode->HasOpCodeRPN( ocAggregate ) )
            bSubTotal = true;
    }

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);

    pCode->GenHash();
}

ScFormulaCell::ScFormulaCell(
    ScDocument* pDoc, const ScAddress& rPos, const ScFormulaCellGroupRef& xGroup,
    const FormulaGrammar::Grammar eGrammar, ScMatrixMode cInd ) :
    mxGroup(xGroup),
    eTempGrammar( eGrammar),
    pCode(xGroup->mpCode ? xGroup->mpCode : new ScTokenArray),
    pDocument( pDoc ),
    pPrevious(nullptr),
    pNext(nullptr),
    pPreviousTrack(nullptr),
    pNextTrack(nullptr),
    nSeenInIteration(0),
    nFormatType(xGroup->mnFormatType),
    cMatrixFlag ( cInd ),
    bDirty(true),
    bChanged( false ),
    bRunning( false ),
    bCompile( false ),
    bSubTotal(xGroup->mbSubTotal),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    mbAllowNumberFormatChange(false),
    mbPostponedDirty(false),
    mbIsExtRef(false),
    aPos(rPos)
{
    if (bSubTotal)
        pDocument->AddSubTotalCell(this);
}

ScFormulaCell::ScFormulaCell(const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, ScCloneFlags nCloneFlags) :
    SvtListener(),
    aResult( rCell.aResult ),
    eTempGrammar( rCell.eTempGrammar),
    pDocument( &rDoc ),
    pPrevious(nullptr),
    pNext(nullptr),
    pPreviousTrack(nullptr),
    pNextTrack(nullptr),
    nSeenInIteration(0),
    nFormatType( rCell.nFormatType ),
    cMatrixFlag ( rCell.cMatrixFlag ),
    bDirty( rCell.bDirty ),
    bChanged( rCell.bChanged ),
    bRunning( false ),
    bCompile( rCell.bCompile ),
    bSubTotal( rCell.bSubTotal ),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    mbAllowNumberFormatChange(false),
    mbPostponedDirty(false),
    mbIsExtRef(false),
    aPos(rPos)
{
    pCode = rCell.pCode->Clone();

    //  set back any errors and recompile
    //  not in the Clipboard - it must keep the received error flag
    //  Special Length=0: as bad cells are generated, then they are also retained
    if ( pCode->GetCodeError() != FormulaError::NONE && !pDocument->IsClipboard() && pCode->GetLen() )
    {
        pCode->SetCodeError( FormulaError::NONE );
        bCompile = true;
    }
    // Compile ColRowNames on URM_MOVE/URM_COPY _after_ UpdateReference !
    bool bCompileLater = false;
    bool bClipMode = rCell.pDocument->IsClipboard();

    //update ScNameTokens
    if (!pDocument->IsClipOrUndo() || rDoc.IsUndo())
    {
        if (!pDocument->IsClipboardSource() || aPos.Tab() != rCell.aPos.Tab())
        {
            bool bGlobalNamesToLocal = ((nCloneFlags & ScCloneFlags::NamesToLocal) != ScCloneFlags::Default);
            formula::FormulaToken* pToken = nullptr;
            formula::FormulaTokenArrayPlainIterator aIter(*pCode);
            while((pToken = aIter.GetNextName())!= nullptr)
            {
                OpCode eOpCode = pToken->GetOpCode();
                if (eOpCode == ocName)
                    adjustRangeName(pToken, rDoc, rCell.pDocument, aPos, rCell.aPos, bGlobalNamesToLocal);
                else if (eOpCode == ocDBArea || eOpCode == ocTableRef)
                    adjustDBRange(pToken, rDoc, rCell.pDocument);
            }
        }

        bool bCopyBetweenDocs = pDocument->GetPool() != rCell.pDocument->GetPool();
        if (bCopyBetweenDocs && !(nCloneFlags & ScCloneFlags::NoMakeAbsExternal))
        {
            pCode->ReadjustAbsolute3DReferences( rCell.pDocument, &rDoc, rCell.aPos);
        }

        pCode->AdjustAbsoluteRefs( rCell.pDocument, rCell.aPos, aPos, bCopyBetweenDocs );
    }

    if (!pDocument->IsClipOrUndo())
    {
        if (&pDocument->GetSharedStringPool() != &rCell.pDocument->GetSharedStringPool())
            pCode->ReinternStrings( pDocument->GetSharedStringPool());
        pCode->AdjustReferenceOnCopy( aPos);
    }

    if( !bCompile )
    {   // Name references with references and ColRowNames
        formula::FormulaTokenArrayPlainIterator aIter(*pCode);
        formula::FormulaToken* t;
        while ( ( t = aIter.GetNextReferenceOrName() ) != nullptr && !bCompile )
        {
            if ( t->IsExternalRef() )
            {
                // External name, cell, and area references.
                bCompile = true;
            }
            else if ( t->GetType() == svIndex )
            {
                const ScRangeData* pRangeData = rDoc.FindRangeNameBySheetAndIndex( t->GetSheet(), t->GetIndex());
                if( pRangeData )
                {
                    if( pRangeData->HasReferences() )
                        bCompile = true;
                }
                else
                    bCompile = true;    // invalid reference!
            }
            else if ( t->GetOpCode() == ocColRowName )
            {
                bCompile = true;        // new lookup needed
                bCompileLater = bClipMode;
            }
        }
    }
    if( bCompile )
    {
        if ( !bCompileLater && bClipMode )
        {
            // Merging ranges needs the actual positions after UpdateReference.
            // ColRowNames and TableRefs need new lookup after positions are
            // adjusted.
            bCompileLater = pCode->HasOpCode( ocRange) || pCode->HasOpCode( ocColRowName) ||
                pCode->HasOpCode( ocTableRef);
        }
        if ( !bCompileLater )
        {
            // bNoListening, not at all if in Clipboard/Undo,
            // and not from Clipboard either, instead after Insert(Clone) and UpdateReference.
            CompileTokenArray( true );
        }
    }

    if( nCloneFlags & ScCloneFlags::StartListening )
        StartListeningTo( &rDoc );

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);
}

ScFormulaCell::~ScFormulaCell()
{
    pDocument->RemoveFromFormulaTrack( this );
    pDocument->RemoveFromFormulaTree( this );
    pDocument->RemoveSubTotalCell(this);
    if (pCode->HasOpCode(ocMacro))
        pDocument->GetMacroManager()->RemoveDependentCell(this);

    if (pDocument->HasExternalRefManager())
        pDocument->GetExternalRefManager()->removeRefCell(this);

    if (!mxGroup || !mxGroup->mpCode)
        // Formula token is not shared.
        delete pCode;
}

ScFormulaCell* ScFormulaCell::Clone() const
{
    return new ScFormulaCell(*this, *pDocument, aPos);
}

ScFormulaCell* ScFormulaCell::Clone( const ScAddress& rPos ) const
{
    return new ScFormulaCell(*this, *pDocument, rPos, ScCloneFlags::Default);
}

size_t ScFormulaCell::GetHash() const
{
    return pCode->GetHash();
}

ScFormulaVectorState ScFormulaCell::GetVectorState() const
{
    return pCode->GetVectorState();
}

void ScFormulaCell::GetFormula( OUStringBuffer& rBuffer,
                                const FormulaGrammar::Grammar eGrammar ) const
{
    if( pCode->GetCodeError() != FormulaError::NONE && !pCode->GetLen() )
    {
        rBuffer = ScGlobal::GetErrorString(pCode->GetCodeError());
        return;
    }
    else if( cMatrixFlag == ScMatrixMode::Reference )
    {
        // Reference to another cell that contains a matrix formula.
        formula::FormulaTokenArrayPlainIterator aIter(*pCode);
        formula::FormulaToken* p = aIter.GetNextReferenceRPN();
        if( p )
        {
            /* FIXME: original GetFormula() code obtained
             * pCell only if (!IsInChangeTrack()),
             * GetEnglishFormula() omitted that test.
             * Can we live without in all cases? */
            ScFormulaCell* pCell = nullptr;
            ScSingleRefData& rRef = *p->GetSingleRef();
            ScAddress aAbs = rRef.toAbs(aPos);
            if (ValidAddress(aAbs))
                pCell = pDocument->GetFormulaCell(aAbs);

            if (pCell)
            {
                pCell->GetFormula( rBuffer, eGrammar);
                return;
            }
            else
            {
                ScCompiler aComp( pDocument, aPos, *pCode, eGrammar);
                aComp.CreateStringFromTokenArray( rBuffer );
            }
        }
        else
        {
            OSL_FAIL("ScFormulaCell::GetFormula: not a matrix");
        }
    }
    else
    {
        ScCompiler aComp( pDocument, aPos, *pCode, eGrammar);
        aComp.CreateStringFromTokenArray( rBuffer );
    }

    rBuffer.insert( 0, '=');
    if( cMatrixFlag != ScMatrixMode::NONE )
    {
        rBuffer.insert( 0, '{');
        rBuffer.append( '}');
    }
}

void ScFormulaCell::GetFormula( OUString& rFormula, const FormulaGrammar::Grammar eGrammar ) const
{
    OUStringBuffer rBuffer( rFormula );
    GetFormula( rBuffer, eGrammar );
    rFormula = rBuffer.makeStringAndClear();
}

OUString ScFormulaCell::GetFormula( sc::CompileFormulaContext& rCxt ) const
{
    OUStringBuffer aBuf;
    if (pCode->GetCodeError() != FormulaError::NONE && !pCode->GetLen())
    {
        ScTokenArray aCode;
        aCode.AddToken( FormulaErrorToken( pCode->GetCodeError()));
        ScCompiler aComp(rCxt, aPos, aCode);
        aComp.CreateStringFromTokenArray(aBuf);
        return aBuf.makeStringAndClear();
    }
    else if( cMatrixFlag == ScMatrixMode::Reference )
    {
        // Reference to another cell that contains a matrix formula.
        formula::FormulaTokenArrayPlainIterator aIter(*pCode);
        formula::FormulaToken* p = aIter.GetNextReferenceRPN();
        if( p )
        {
            /* FIXME: original GetFormula() code obtained
             * pCell only if (!IsInChangeTrack()),
             * GetEnglishFormula() omitted that test.
             * Can we live without in all cases? */
            ScFormulaCell* pCell = nullptr;
            ScSingleRefData& rRef = *p->GetSingleRef();
            ScAddress aAbs = rRef.toAbs(aPos);
            if (ValidAddress(aAbs))
                pCell = pDocument->GetFormulaCell(aAbs);

            if (pCell)
            {
                return pCell->GetFormula(rCxt);
            }
            else
            {
                ScCompiler aComp(rCxt, aPos, *pCode);
                aComp.CreateStringFromTokenArray(aBuf);
            }
        }
        else
        {
            OSL_FAIL("ScFormulaCell::GetFormula: not a matrix");
        }
    }
    else
    {
        ScCompiler aComp(rCxt, aPos, *pCode);
        aComp.CreateStringFromTokenArray(aBuf);
    }

    aBuf.insert( 0, '=');
    if( cMatrixFlag != ScMatrixMode::NONE )
    {
        aBuf.insert( 0, '{');
        aBuf.append( '}');
    }

    return aBuf.makeStringAndClear();
}

void ScFormulaCell::GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows )
{
    MaybeInterpret();

    const ScMatrix* pMat = nullptr;
    if (pCode->GetCodeError() == FormulaError::NONE && aResult.GetType() == svMatrixCell &&
            ((pMat = aResult.GetToken().get()->GetMatrix()) != nullptr))
        pMat->GetDimensions( rCols, rRows );
    else
    {
        rCols = 0;
        rRows = 0;
    }
}

void ScFormulaCell::ResetDirty() { bDirty = bTableOpDirty = mbPostponedDirty = false; }
void ScFormulaCell::SetNeedsListening( bool bVar ) { bNeedListening = bVar; }

void ScFormulaCell::SetNeedsDirty( bool bVar )
{
    mbPostponedDirty = bVar;
}

void ScFormulaCell::SetNeedNumberFormat( bool bVal )
{
    mbNeedsNumberFormat = mbAllowNumberFormatChange = bVal;
}

void ScFormulaCell::Compile( const OUString& rFormula, bool bNoListening,
                            const FormulaGrammar::Grammar eGrammar )
{
    if ( pDocument->IsClipOrUndo() )
        return;
    bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
    if ( bWasInFormulaTree )
        pDocument->RemoveFromFormulaTree( this );
    // pCode may not deleted for queries, but must be empty
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    ScCompiler aComp( pDocument, aPos, eGrammar);
    pCode = aComp.CompileString( rFormula );
    delete pCodeOld;
    if( pCode->GetCodeError() == FormulaError::NONE )
    {
        if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() && rFormula == aResult.GetHybridFormula() )
        {   // not recursive CompileTokenArray/Compile/CompileTokenArray
            if ( rFormula[0] == '=' )
                pCode->AddBad( rFormula.copy(1) );
            else
                pCode->AddBad( rFormula );
        }
        bCompile = true;
        CompileTokenArray( bNoListening );
    }
    else
        bChanged = true;

    if ( bWasInFormulaTree )
        pDocument->PutInFormulaTree( this );
}

void ScFormulaCell::Compile(
    sc::CompileFormulaContext& rCxt, const OUString& rFormula, bool bNoListening )
{
    if ( pDocument->IsClipOrUndo() )
        return;
    bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
    if ( bWasInFormulaTree )
        pDocument->RemoveFromFormulaTree( this );
    // pCode may not deleted for queries, but must be empty
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    ScCompiler aComp(rCxt, aPos);
    pCode = aComp.CompileString( rFormula );
    delete pCodeOld;
    if( pCode->GetCodeError() == FormulaError::NONE )
    {
        if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() && rFormula == aResult.GetHybridFormula() )
        {   // not recursive CompileTokenArray/Compile/CompileTokenArray
            if ( rFormula[0] == '=' )
                pCode->AddBad( rFormula.copy(1) );
            else
                pCode->AddBad( rFormula );
        }
        bCompile = true;
        CompileTokenArray(rCxt, bNoListening);
    }
    else
        bChanged = true;

    if ( bWasInFormulaTree )
        pDocument->PutInFormulaTree( this );
}

void ScFormulaCell::CompileTokenArray( bool bNoListening )
{
    // Not already compiled?
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        Compile( aResult.GetHybridFormula(), bNoListening, eTempGrammar);
    }
    else if( bCompile && !pDocument->IsClipOrUndo() && pCode->GetCodeError() == FormulaError::NONE )
    {
        // RPN length may get changed
        bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
        if ( bWasInFormulaTree )
            pDocument->RemoveFromFormulaTree( this );

        // Loading from within filter? No listening yet!
        if( pDocument->IsInsertingFromOtherDoc() )
            bNoListening = true;

        if( !bNoListening && pCode->GetCodeLen() )
            EndListeningTo( pDocument );
        ScCompiler aComp(pDocument, aPos, *pCode, pDocument->GetGrammar());
        bSubTotal = aComp.CompileTokenArray();
        if( pCode->GetCodeError() == FormulaError::NONE )
        {
            nFormatType = aComp.GetNumFormatType();
            bChanged = true;
            aResult.SetToken( nullptr);
            bCompile = false;
            if ( !bNoListening )
                StartListeningTo( pDocument );
        }
        if ( bWasInFormulaTree )
            pDocument->PutInFormulaTree( this );

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }
}

void ScFormulaCell::CompileTokenArray( sc::CompileFormulaContext& rCxt, bool bNoListening )
{
    // Not already compiled?
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        rCxt.setGrammar(eTempGrammar);
        Compile(rCxt, aResult.GetHybridFormula(), bNoListening);
    }
    else if( bCompile && !pDocument->IsClipOrUndo() && pCode->GetCodeError() == FormulaError::NONE)
    {
        // RPN length may get changed
        bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
        if ( bWasInFormulaTree )
            pDocument->RemoveFromFormulaTree( this );

        // Loading from within filter? No listening yet!
        if( pDocument->IsInsertingFromOtherDoc() )
            bNoListening = true;

        if( !bNoListening && pCode->GetCodeLen() )
            EndListeningTo( pDocument );
        ScCompiler aComp(rCxt, aPos, *pCode);
        bSubTotal = aComp.CompileTokenArray();
        if( pCode->GetCodeError() == FormulaError::NONE )
        {
            nFormatType = aComp.GetNumFormatType();
            bChanged = true;
            aResult.SetToken( nullptr);
            bCompile = false;
            if ( !bNoListening )
                StartListeningTo( pDocument );
        }
        if ( bWasInFormulaTree )
            pDocument->PutInFormulaTree( this );

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }
}

void ScFormulaCell::CompileXML( sc::CompileFormulaContext& rCxt, ScProgress& rProgress )
{
    if ( cMatrixFlag == ScMatrixMode::Reference )
    {   // is already token code via ScDocFunc::EnterMatrix, ScDocument::InsertMatrixFormula
        // just establish listeners
        StartListeningTo( pDocument );
        return ;
    }

    // Error constant formula cell stays as is.
    if (!pCode->GetLen() && pCode->GetCodeError() != FormulaError::NONE)
        return;

    // Compilation changes RPN count, remove and reinsert to FormulaTree if it
    // was in to update its count.
    bool bWasInFormulaTree = pDocument->IsInFormulaTree( this);
    if (bWasInFormulaTree)
        pDocument->RemoveFromFormulaTree( this);
    rCxt.setGrammar(eTempGrammar);
    ScCompiler aComp(rCxt, aPos, *pCode);
    OUString aFormula, aFormulaNmsp;
    aComp.CreateStringFromXMLTokenArray( aFormula, aFormulaNmsp );
    pDocument->DecXMLImportedFormulaCount( aFormula.getLength() );
    rProgress.SetStateCountDownOnPercent( pDocument->GetXMLImportedFormulaCount() );
    // pCode may not deleted for queries, but must be empty
    pCode->Clear();

    bool bDoCompile = true;

    if ( !mxGroup && aFormulaNmsp.isEmpty() ) // optimization
    {
        ScAddress aPreviousCell( aPos );
        aPreviousCell.IncRow( -1 );
        ScFormulaCell *pPreviousCell = pDocument->GetFormulaCell( aPreviousCell );
        if (pPreviousCell && pPreviousCell->GetCode()->IsShareable())
        {
            // Build formula string using the tokens from the previous cell,
            // but use the current cell position.
            ScCompiler aBackComp( rCxt, aPos, *(pPreviousCell->pCode) );
            OUStringBuffer aShouldBeBuf;
            aBackComp.CreateStringFromTokenArray( aShouldBeBuf );

            // The initial '=' is optional in ODFF.
            const sal_Int32 nLeadingEqual = (aFormula.getLength() > 0 && aFormula[0] == '=') ? 1 : 0;
            OUString aShouldBe = aShouldBeBuf.makeStringAndClear();
            if (aFormula.getLength() == aShouldBe.getLength() + nLeadingEqual &&
                    aFormula.match( aShouldBe, nLeadingEqual))
            {
                // Put them in the same formula group.
                ScFormulaCellGroupRef xGroup = pPreviousCell->GetCellGroup();
                if (!xGroup) // Last cell is not grouped yet. Start a new group.
                    xGroup = pPreviousCell->CreateCellGroup(1, false);
                ++xGroup->mnLength;
                SetCellGroup( xGroup );

                // Do setup here based on previous cell.

                nFormatType = pPreviousCell->nFormatType;
                bSubTotal = pPreviousCell->bSubTotal;
                bChanged = true;
                bCompile = false;

                if (bSubTotal)
                    pDocument->AddSubTotalCell(this);

                bDoCompile = false;
                pCode = pPreviousCell->pCode;
                if (pPreviousCell->mbIsExtRef)
                    pDocument->GetExternalRefManager()->insertRefCellFromTemplate( pPreviousCell, this );
            }
        }
    }

    if (bDoCompile)
    {
        ScTokenArray* pCodeOld = pCode;
        pCode = aComp.CompileString( aFormula, aFormulaNmsp );
        delete pCodeOld;

        if( pCode->GetCodeError() == FormulaError::NONE )
        {
            if ( !pCode->GetLen() )
            {
                if ( !aFormula.isEmpty() && aFormula[0] == '=' )
                    pCode->AddBad( aFormula.copy( 1 ) );
                else
                    pCode->AddBad( aFormula );
            }
            bSubTotal = aComp.CompileTokenArray();
            if( pCode->GetCodeError() == FormulaError::NONE )
            {
                nFormatType = aComp.GetNumFormatType();
                bChanged = true;
                bCompile = false;
            }

            if (bSubTotal)
                pDocument->AddSubTotalCell(this);
        }
        else
            bChanged = true;
    }

    //  Same as in Load: after loading, it must be known if ocMacro is in any formula
    //  (for macro warning, CompileXML is called at the end of loading XML file)
    if ( !pDocument->GetHasMacroFunc() && pCode->HasOpCodeRPN( ocMacro ) )
        pDocument->SetHasMacroFunc( true );

    //volatile cells must be added here for import
    if( pCode->IsRecalcModeAlways() || pCode->IsRecalcModeForced() ||
        pCode->IsRecalcModeOnLoad() || pCode->IsRecalcModeOnLoadOnce() )
    {
        // During load, only those cells that are marked explicitly dirty get
        // recalculated.  So we need to set it dirty here.
        SetDirtyVar();
        pDocument->AppendToFormulaTrack(this);
        // Do not call TrackFormulas() here, not all listeners may have been
        // established, postponed until ScDocument::CompileXML() finishes.
    }
    else if (bWasInFormulaTree)
        pDocument->PutInFormulaTree(this);
}

void ScFormulaCell::CalcAfterLoad( sc::CompileFormulaContext& rCxt, bool bStartListening )
{
    bool bNewCompiled = false;
    // If a Calc 1.0-doc is read, we have a result, but no token array
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        rCxt.setGrammar(eTempGrammar);
        Compile(rCxt, aResult.GetHybridFormula(), true);
        aResult.SetToken( nullptr);
        bDirty = true;
        bNewCompiled = true;
    }
    // The RPN array is not created when a Calc 3.0-Doc has been read as the Range Names exist until now.
    if( pCode->GetLen() && !pCode->GetCodeLen() && pCode->GetCodeError() == FormulaError::NONE )
    {
        ScCompiler aComp(rCxt, aPos, *pCode);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
        bDirty = true;
        bCompile = false;
        bNewCompiled = true;

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }

    // On OS/2 with broken FPU exception, we can somehow store /0 without Err503. Later on in
    // the BLC Lib NumberFormatter crashes when doing a fabs (NAN) (# 32739 #).
    // We iron this out here for all systems, such that we also have an Err503 here.
    if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
    {
        OSL_FAIL("Formula cell INFINITY!!! Where does this document come from?");
        aResult.SetResultError( FormulaError::IllegalFPOperation );
        bDirty = true;
    }

    // DoubleRefs for binary operators were always a Matrix before version v5.0.
    // Now this is only the case when in an array formula, otherwise it's an implicit intersection
    if ( pDocument->GetSrcVersion() < SC_MATRIX_DOUBLEREF &&
            GetMatrixFlag() == ScMatrixMode::NONE && pCode->HasMatrixDoubleRefOps() )
    {
        cMatrixFlag = ScMatrixMode::Formula;
        SetMatColsRows( 1, 1);
    }

    // Do the cells need to be calculated? After Load cells can contain an error code, and then start
    // the listener and Recalculate (if needed) if not ScRecalcMode::NORMAL
    if( !bNewCompiled || pCode->GetCodeError() == FormulaError::NONE )
    {
        if (bStartListening)
            StartListeningTo(pDocument);

        if( !pCode->IsRecalcModeNormal() )
            bDirty = true;
    }
    if ( pCode->IsRecalcModeAlways() )
    {   // random(), today(), now() always stay in the FormulaTree, so that they are calculated
        // for each F9
        bDirty = true;
    }
    // No SetDirty yet, as no all Listeners are known yet (only in SetDirtyAfterLoad)
}

bool ScFormulaCell::MarkUsedExternalReferences()
{
    return pCode && pDocument->MarkUsedExternalReferences(*pCode, aPos);
}

namespace {
class RecursionCounter
{
    ScRecursionHelper&  rRec;
    bool                bStackedInIteration;
public:
    RecursionCounter( ScRecursionHelper& r, ScFormulaCell* p ) : rRec(r)
    {
        bStackedInIteration = rRec.IsDoingIteration();
        if (bStackedInIteration)
            rRec.GetRecursionInIterationStack().push( p);
        rRec.IncRecursionCount();
    }
    ~RecursionCounter()
    {
        rRec.DecRecursionCount();
        if (bStackedInIteration)
            rRec.GetRecursionInIterationStack().pop();
    }
};
}

void ScFormulaCell::Interpret()
{
    ScRecursionHelper& rRecursionHelper = pDocument->GetRecursionHelper();

#if DEBUG_CALCULATION
    static bool bDebugCalculationInit = true;
    if (bDebugCalculationInit)
    {
        aDC.maTrigger = aDebugCalculationTriggerAddress;
        aDC.mbPrintResults = true;
        bDebugCalculationInit = false;
    }
    DebugCalculationStacker aDebugEntry( aPos, pDocument);
#endif

    if (!IsDirtyOrInTableOpDirty() || rRecursionHelper.IsInReturn())
        return;     // no double/triple processing

    //FIXME:
    //  If the call originates from a Reschedule in DdeLink update, leave dirty
    //  Better: Do a Dde Link Update without Reschedule or do it completely asynchronously!
    if ( pDocument->IsInDdeLinkUpdate() )
        return;

    if (bRunning)
    {
        if (!pDocument->GetDocOptions().IsIter())
        {
            aResult.SetResultError( FormulaError::CircularReference );
            return;
        }

        if (aResult.GetResultError() == FormulaError::CircularReference)
            aResult.SetResultError( FormulaError::NONE );

        // Start or add to iteration list.
        if (!rRecursionHelper.IsDoingIteration() ||
                !rRecursionHelper.GetRecursionInIterationStack().top()->bIsIterCell)
            rRecursionHelper.SetInIterationReturn( true);

        return;
    }
    // no multiple interprets for GetErrCode, IsValue, GetValue and
    // different entry point recursions. Would also lead to premature
    // convergence in iterations.
    if (rRecursionHelper.GetIteration() && nSeenInIteration ==
            rRecursionHelper.GetIteration())
        return ;

    bool bOldRunning = bRunning;
    if (rRecursionHelper.GetRecursionCount() > MAXRECURSION)
    {
        bRunning = true;
        rRecursionHelper.SetInRecursionReturn( true);
    }
    else
    {
        pDocument->IncInterpretLevel();
#if DEBUG_CALCULATION
        aDC.enterGroup();
        bool bGroupInterpreted = InterpretFormulaGroup();
        aDC.leaveGroup();
        if (!bGroupInterpreted)
            InterpretTail( pDocument->GetNonThreadedContext(), SCITP_NORMAL);
#else
        if (!InterpretFormulaGroup())
            InterpretTail( pDocument->GetNonThreadedContext(), SCITP_NORMAL);
#endif
        pDocument->DecInterpretLevel();
    }

    // While leaving a recursion or iteration stack, insert its cells to the
    // recursion list in reverse order.
    if (rRecursionHelper.IsInReturn())
    {
        if (rRecursionHelper.GetRecursionCount() > 0 ||
                !rRecursionHelper.IsDoingRecursion())
            rRecursionHelper.Insert( this, bOldRunning, aResult);
        bool bIterationFromRecursion = false;
        bool bResumeIteration = false;
        do
        {
            if ((rRecursionHelper.IsInIterationReturn() &&
                        rRecursionHelper.GetRecursionCount() == 0 &&
                        !rRecursionHelper.IsDoingIteration()) ||
                    bIterationFromRecursion || bResumeIteration)
            {
                bool & rDone = rRecursionHelper.GetConvergingReference();
                rDone = false;
                if (!bIterationFromRecursion && bResumeIteration)
                {
                    bResumeIteration = false;
                    // Resuming iteration expands the range.
                    ScFormulaRecursionList::const_iterator aOldStart(
                            rRecursionHelper.GetLastIterationStart());
                    rRecursionHelper.ResumeIteration();
                    // Mark new cells being in iteration.
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            aOldStart; ++aIter)
                    {
                        ScFormulaCell* pIterCell = (*aIter).pCell;
                        pIterCell->bIsIterCell = true;
                    }
                    // Mark older cells dirty again, in case they converted
                    // without accounting for all remaining cells in the circle
                    // that weren't touched so far, e.g. conditional. Restore
                    // backupped result.
                    sal_uInt16 nIteration = rRecursionHelper.GetIteration();
                    for (ScFormulaRecursionList::const_iterator aIter(
                                aOldStart); aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        ScFormulaCell* pIterCell = (*aIter).pCell;
                        if (pIterCell->nSeenInIteration == nIteration)
                        {
                            if (!pIterCell->bDirty || aIter == aOldStart)
                            {
                                pIterCell->aResult = (*aIter).aPreviousResult;
                            }
                            --pIterCell->nSeenInIteration;
                        }
                        pIterCell->bDirty = true;
                    }
                }
                else
                {
                    bResumeIteration = false;
                    // Close circle once.
                    pDocument->IncInterpretLevel();
                    rRecursionHelper.GetList().back().pCell->InterpretTail( pDocument->GetNonThreadedContext(),
                                                                            SCITP_CLOSE_ITERATION_CIRCLE);
                    pDocument->DecInterpretLevel();
                    // Start at 1, init things.
                    rRecursionHelper.StartIteration();
                    // Mark all cells being in iteration.
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        ScFormulaCell* pIterCell = (*aIter).pCell;
                        pIterCell->bIsIterCell = true;
                    }
                }
                bIterationFromRecursion = false;
                sal_uInt16 nIterMax = pDocument->GetDocOptions().GetIterCount();
                for ( ; rRecursionHelper.GetIteration() <= nIterMax && !rDone;
                        rRecursionHelper.IncIteration())
                {
                    rDone = true;
                    for ( ScFormulaRecursionList::iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            rRecursionHelper.GetIterationEnd() &&
                            !rRecursionHelper.IsInReturn(); ++aIter)
                    {
                        ScFormulaCell* pIterCell = (*aIter).pCell;
                        if (pIterCell->IsDirtyOrInTableOpDirty() &&
                                rRecursionHelper.GetIteration() !=
                                pIterCell->GetSeenInIteration())
                        {
                            (*aIter).aPreviousResult = pIterCell->aResult;
                            pDocument->IncInterpretLevel();
                            pIterCell->InterpretTail( pDocument->GetNonThreadedContext(), SCITP_FROM_ITERATION);
                            pDocument->DecInterpretLevel();
                        }
                        rDone = rDone && !pIterCell->IsDirtyOrInTableOpDirty();
                    }
                    if (rRecursionHelper.IsInReturn())
                    {
                        bResumeIteration = true;
                        break;  // for
                        // Don't increment iteration.
                    }
                }
                if (!bResumeIteration)
                {
                    if (rDone)
                    {
                        for (ScFormulaRecursionList::const_iterator aIter(
                                    rRecursionHelper.GetIterationStart());
                                aIter != rRecursionHelper.GetIterationEnd();
                                ++aIter)
                        {
                            ScFormulaCell* pIterCell = (*aIter).pCell;
                            pIterCell->bIsIterCell = false;
                            pIterCell->nSeenInIteration = 0;
                            pIterCell->bRunning = (*aIter).bOldRunning;
                        }
                    }
                    else
                    {
                        for (ScFormulaRecursionList::const_iterator aIter(
                                    rRecursionHelper.GetIterationStart());
                                aIter != rRecursionHelper.GetIterationEnd();
                                ++aIter)
                        {
                            ScFormulaCell* pIterCell = (*aIter).pCell;
                            pIterCell->bIsIterCell = false;
                            pIterCell->nSeenInIteration = 0;
                            pIterCell->bRunning = (*aIter).bOldRunning;
                            // If one cell didn't converge, all cells of this
                            // circular dependency don't, no matter whether
                            // single cells did.
                            pIterCell->ResetDirty();
                            pIterCell->aResult.SetResultError( FormulaError::NoConvergence);
                            pIterCell->bChanged = true;
                        }
                    }
                    // End this iteration and remove entries.
                    rRecursionHelper.EndIteration();
                    bResumeIteration = rRecursionHelper.IsDoingIteration();
                }
            }
            if (rRecursionHelper.IsInRecursionReturn() &&
                    rRecursionHelper.GetRecursionCount() == 0 &&
                    !rRecursionHelper.IsDoingRecursion())
            {
                bIterationFromRecursion = false;
                // Iterate over cells known so far, start with the last cell
                // encountered, inserting new cells if another recursion limit
                // is reached. Repeat until solved.
                rRecursionHelper.SetDoingRecursion( true);
                do
                {
                    rRecursionHelper.SetInRecursionReturn( false);
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart());
                            !rRecursionHelper.IsInReturn() && aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        ScFormulaCell* pCell = (*aIter).pCell;
                        if (pCell->IsDirtyOrInTableOpDirty())
                        {
                            pDocument->IncInterpretLevel();
                            pCell->InterpretTail( pDocument->GetNonThreadedContext(), SCITP_NORMAL);
                            pDocument->DecInterpretLevel();
                            if (!pCell->IsDirtyOrInTableOpDirty() && !pCell->IsIterCell())
                                pCell->bRunning = (*aIter).bOldRunning;
                        }
                    }
                } while (rRecursionHelper.IsInRecursionReturn());
                rRecursionHelper.SetDoingRecursion( false);
                if (rRecursionHelper.IsInIterationReturn())
                {
                    if (!bResumeIteration)
                        bIterationFromRecursion = true;
                }
                else if (bResumeIteration ||
                        rRecursionHelper.IsDoingIteration())
                    rRecursionHelper.GetList().erase(
                            rRecursionHelper.GetIterationStart(),
                            rRecursionHelper.GetLastIterationStart());
                else
                    rRecursionHelper.Clear();
            }
        } while (bIterationFromRecursion || bResumeIteration);
    }

#if DEBUG_CALCULATION
    if (sal_uInt16 nErr = aResult.GetResultError())
        aDC.storeResultError( nErr);
    else if (aResult.IsValue())
        aDC.storeResult( aResult.GetDouble());
    else
        aDC.storeResult( aResult.GetString());
#endif
}

namespace {
class StackCleaner
{
    ScInterpreter*  pInt;
    public:
    StackCleaner( ScInterpreter* pI )
        : pInt(pI)
        {}
    ~StackCleaner()
    {
        delete pInt;
    }
};
}

void ScFormulaCell::InterpretTail( const ScInterpreterContext& rContext, ScInterpretTailParameter eTailParam )
{
    RecursionCounter aRecursionCounter( pDocument->GetRecursionHelper(), this);
    nSeenInIteration = pDocument->GetRecursionHelper().GetIteration();
    if( !pCode->GetCodeLen() && pCode->GetCodeError() == FormulaError::NONE )
    {
        // #i11719# no RPN and no error and no token code but result string present
        // => interpretation of this cell during name-compilation and unknown names
        // => can't exchange underlying code array in CompileTokenArray() /
        // Compile() because interpreter's token iterator would crash or pCode
        // would be deleted twice if this cell was interpreted during
        // compilation.
        // This should only be a temporary condition and, since we set an
        // error, if ran into it again we'd bump into the dirty-clearing
        // condition further down.
        if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
        {
            pCode->SetCodeError( FormulaError::NoCode );
            // This is worth an assertion; if encountered in daily work
            // documents we might need another solution. Or just confirm correctness.
            return;
        }
        CompileTokenArray();
    }

    if( pCode->GetCodeLen() && pDocument )
    {
        ScInterpreter* pInterpreter = new ScInterpreter( this, pDocument, rContext, aPos, *pCode );
        StackCleaner aStackCleaner(pInterpreter);
        FormulaError nOldErrCode = aResult.GetResultError();
        if ( nSeenInIteration == 0 )
        {   // Only the first time
            // With bChanged=false, if a newly compiled cell has a result of
            // 0.0, no change is detected and the cell will not be repainted.
            // bChanged = false;
            aResult.SetResultError( FormulaError::NONE );
        }

        switch ( aResult.GetResultError() )
        {
            case FormulaError::CircularReference :     // will be determined again if so
                aResult.SetResultError( FormulaError::NONE );
            break;
            default: break;
        }

        bool bOldRunning = bRunning;
        bRunning = true;
        pInterpreter->Interpret();
        if (pDocument->GetRecursionHelper().IsInReturn() && eTailParam != SCITP_CLOSE_ITERATION_CIRCLE)
        {
            if (nSeenInIteration > 0)
                --nSeenInIteration;     // retry when iteration is resumed

            if ( aResult.GetType() == formula::svUnknown )
                aResult.SetToken( pInterpreter->GetResultToken().get() );

            return;
        }
        bRunning = bOldRunning;

        // #i102616# For single-sheet saving consider only content changes, not format type,
        // because format type isn't set on loading (might be changed later)
        bool bContentChanged = false;

        // Do not create a HyperLink() cell if the formula results in an error.
        if( pInterpreter->GetError() != FormulaError::NONE && pCode->IsHyperLink())
            pCode->SetHyperLink(false);

        if( pInterpreter->GetError() != FormulaError::NONE && pInterpreter->GetError() != FormulaError::CircularReference)
        {
            bChanged = true;

            if (pInterpreter->GetError() == FormulaError::RetryCircular)
            {
                // Array formula matrix calculation corner case. Keep dirty
                // state, do not remove from formula tree or anything else, but
                // store FormulaError::CircularReference in case this cell does not get
                // recalculated.
                aResult.SetResultError( FormulaError::CircularReference);
                return;
            }

            ResetDirty();
        }

        if (eTailParam == SCITP_FROM_ITERATION && IsDirtyOrInTableOpDirty())
        {
            bool bIsValue = aResult.IsValue();  // the previous type
            // Did it converge?
            if ((bIsValue && pInterpreter->GetResultType() == svDouble && fabs(
                            pInterpreter->GetNumResult() - aResult.GetDouble()) <=
                        pDocument->GetDocOptions().GetIterEps()) ||
                    (!bIsValue && pInterpreter->GetResultType() == svString &&
                     pInterpreter->GetStringResult() == aResult.GetString()))
            {
                // A convergence in the first iteration doesn't necessarily
                // mean that it's done, it may be as not all related cells
                // of a circle changed their values yet. If the set really
                // converges it will do so also during the next iteration. This
                // fixes situations like of #i44115#. If this wasn't wanted an
                // initial "uncalculated" value would be needed for all cells
                // of a circular dependency => graph needed before calculation.
                if (nSeenInIteration > 1 ||
                        pDocument->GetDocOptions().GetIterCount() == 1)
                {
                    ResetDirty();
                }
            }
        }

        // New error code?
        if( pInterpreter->GetError() != nOldErrCode )
        {
            bChanged = true;
            // bContentChanged only has to be set if the file content would be changed
            if ( aResult.GetCellResultType() != svUnknown )
                bContentChanged = true;
        }

        ScFormulaResult aNewResult( pInterpreter->GetResultToken().get());

        // For IF() and other jumps or changed formatted source data the result
        // format may change for different runs, e.g. =IF(B1,B1) with first
        // B1:0 boolean FALSE next B1:23 numeric 23, we don't want the 23
        // displayed as TRUE. Do not force a general format though if
        // mbNeedsNumberFormat is set (because there was a general format..).
        // Note that nFormatType may be out of sync here if a format was
        // applied or cleared after the last run, but obtaining the current
        // format always just to check would be expensive. There may be
        // cases where the format should be changed but is not. If that turns
        // out to be a real problem then obtain the current format type after
        // the initial check when needed.
        bool bForceNumberFormat = (mbAllowNumberFormatChange && !mbNeedsNumberFormat &&
                !SvNumberFormatter::IsCompatible( nFormatType, pInterpreter->GetRetFormatType()));

        // We have some requirements additionally to IsCompatible().
        // * Do not apply a NumberFormat::LOGICAL if the result value is not
        //   1.0 or 0.0
        // * Do not override an already set numeric number format if the result
        //   is of type NumberFormat::LOGICAL, it could be user applied.
        //   On the other hand, for an empty jump path instead of FALSE an
        //   unexpected for example 0% could be displayed. YMMV.
        // * Never override a non-standard number format that indicates user
        //   applied.
        // * NumberFormat::TEXT does not force a change.
        if (bForceNumberFormat)
        {
            sal_uInt32 nOldFormatIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
            const short nRetType = pInterpreter->GetRetFormatType();
            if (nRetType == css::util::NumberFormat::LOGICAL)
            {
                double fVal;
                if ((fVal = aNewResult.GetDouble()) != 1.0 && fVal != 0.0)
                    bForceNumberFormat = false;
                else
                {
                    nOldFormatIndex = pDocument->GetNumberFormat( aPos);
                    nFormatType = pDocument->GetFormatTable()->GetType( nOldFormatIndex);
                    switch (nFormatType)
                    {
                        case css::util::NumberFormat::PERCENT:
                        case css::util::NumberFormat::CURRENCY:
                        case css::util::NumberFormat::SCIENTIFIC:
                        case css::util::NumberFormat::FRACTION:
                            bForceNumberFormat = false;
                        break;
                        case css::util::NumberFormat::NUMBER:
                            if ((nOldFormatIndex % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
                                bForceNumberFormat = false;
                        break;
                    }
                }
            }
            else if (nRetType == css::util::NumberFormat::TEXT)
            {
                bForceNumberFormat = false;
            }
            if (bForceNumberFormat)
            {
                if (nOldFormatIndex == NUMBERFORMAT_ENTRY_NOT_FOUND)
                {
                    nOldFormatIndex = pDocument->GetNumberFormat( aPos);
                    nFormatType = pDocument->GetFormatTable()->GetType( nOldFormatIndex);
                }
                if (nOldFormatIndex !=
                        ScGlobal::GetStandardFormat( *pDocument->GetFormatTable(), nOldFormatIndex, nFormatType))
                    bForceNumberFormat = false;
            }
        }

        if( mbNeedsNumberFormat || bForceNumberFormat )
        {
            bool bSetFormat = true;
            const short nOldFormatType = nFormatType;
            nFormatType = pInterpreter->GetRetFormatType();
            sal_uInt32 nFormatIndex = pInterpreter->GetRetFormatIndex();

            if (nFormatType == css::util::NumberFormat::TEXT)
            {
                // Don't set text format as hard format.
                bSetFormat = false;
            }
            else if (nFormatType == css::util::NumberFormat::LOGICAL && cMatrixFlag != ScMatrixMode::NONE)
            {
                // In a matrix range do not set an (inherited) logical format
                // as hard format if the value does not represent a strict TRUE
                // or FALSE value. But do set for a top left error value so
                // following matrix cells can inherit for non-error values.
                // This solves a problem with IF() expressions in array context
                // where incidentally the top left element results in logical
                // type but some others don't. It still doesn't solve the
                // reverse case though, where top left is not logical type but
                // some other elements should be. We'd need to transport type
                // or format information on arrays.
                StackVar eNewCellResultType = aNewResult.GetCellResultType();
                if (eNewCellResultType != svError || cMatrixFlag == ScMatrixMode::Reference)
                {
                    double fVal;
                    if (eNewCellResultType != svDouble)
                    {
                        bSetFormat = false;
                        nFormatType = nOldFormatType;   // that? or number?
                    }
                    else if ((fVal = aNewResult.GetDouble()) != 1.0 && fVal != 0.0)
                    {
                        bSetFormat = false;
                        nFormatType = css::util::NumberFormat::NUMBER;
                    }
                }
            }

            if (bSetFormat && (bForceNumberFormat || ((nFormatIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0)))
                nFormatIndex = ScGlobal::GetStandardFormat(*rContext.GetFormatTable(),
                        nFormatIndex, nFormatType);

            // Do not replace a General format (which was the reason why
            // mbNeedsNumberFormat was set) with a General format.
            // 1. setting a format has quite some overhead in the
            // ScPatternAttr/ScAttrArray handling, even if identical.
            // 2. the General formats may be of different locales.
            // XXX if mbNeedsNumberFormat was set even if the current format
            // was not General then we'd have to obtain the current format here
            // and check at least the types.
            if (bSetFormat && (bForceNumberFormat || ((nFormatIndex % SV_COUNTRY_LANGUAGE_OFFSET) != 0)))
            {
                // set number format explicitly
                pDocument->SetNumberFormat( aPos, nFormatIndex );
                bChanged = true;
            }

            mbNeedsNumberFormat = false;
        }

        // In case of changes just obtain the result, no temporary and
        // comparison needed anymore.
        if (bChanged)
        {
            // #i102616# Compare anyway if the sheet is still marked unchanged for single-sheet saving
            // Also handle special cases of initial results after loading.
            if ( !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                StackVar eOld = aResult.GetCellResultType();
                StackVar eNew = aNewResult.GetCellResultType();
                if ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) )
                {
                    // ScXMLTableRowCellContext::EndElement doesn't call SetFormulaResultDouble for 0
                    // -> no change
                }
                else
                {
                    if ( eOld == svHybridCell )     // string result from SetFormulaResultString?
                        eOld = svString;            // ScHybridCellToken has a valid GetString method

                    // #i106045# use approxEqual to compare with stored value
                    bContentChanged = (eOld != eNew ||
                            (eNew == svDouble && !rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble() )) ||
                            (eNew == svString && aResult.GetString() != aNewResult.GetString()));
                }
            }

            aResult.SetToken( pInterpreter->GetResultToken().get() );
        }
        else
        {
            StackVar eOld = aResult.GetCellResultType();
            StackVar eNew = aNewResult.GetCellResultType();
            bChanged = (eOld != eNew ||
                    (eNew == svDouble && aResult.GetDouble() != aNewResult.GetDouble()) ||
                    (eNew == svString && aResult.GetString() != aNewResult.GetString()));

            // #i102616# handle special cases of initial results after loading
            // (only if the sheet is still marked unchanged)
            if ( bChanged && !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                if ((eOld == svUnknown && (eNew == svError || (eNew == svDouble && aNewResult.GetDouble() == 0.0))) ||
                        ((eOld == svHybridCell) &&
                         eNew == svString && aResult.GetString() == aNewResult.GetString()) ||
                        (eOld == svDouble && eNew == svDouble &&
                         rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble())))
                {
                    // no change, see above
                }
                else
                    bContentChanged = true;
            }

            aResult.Assign( aNewResult);
        }

        // Precision as shown?
        if ( aResult.IsValue() && pInterpreter->GetError() == FormulaError::NONE
          && pDocument->GetDocOptions().IsCalcAsShown()
          && nFormatType != css::util::NumberFormat::DATE
          && nFormatType != css::util::NumberFormat::TIME
          && nFormatType != css::util::NumberFormat::DATETIME )
        {
            sal_uLong nFormat = pDocument->GetNumberFormat( aPos );
            aResult.SetDouble( pDocument->RoundValueAsShown(
                        aResult.GetDouble(), nFormat));
        }
        if (eTailParam == SCITP_NORMAL)
        {
            ResetDirty();
        }
        if( aResult.GetMatrix() )
        {
            // If the formula wasn't entered as a matrix formula, live on with
            // the upper left corner and let reference counting delete the matrix.
            if( cMatrixFlag != ScMatrixMode::Formula && !pCode->IsHyperLink() )
                aResult.SetToken( aResult.GetCellResultToken().get());
        }
        if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
        {
            // Coded double error may occur via filter import.
            FormulaError nErr = GetDoubleErrorValue( aResult.GetDouble());
            aResult.SetResultError( nErr);
            bChanged = bContentChanged = true;
        }

        if (bContentChanged && pDocument->IsStreamValid(aPos.Tab()))
        {
            // pass bIgnoreLock=true, because even if called from pending row height update,
            // a changed result must still reset the stream flag
            pDocument->SetStreamValid(aPos.Tab(), false, true);
        }
        if ( !pDocument->mbThreadedGroupCalcInProgress && !pCode->IsRecalcModeAlways() )
            pDocument->RemoveFromFormulaTree( this );

        //  FORCED cells also immediately tested for validity (start macro possibly)

        if ( pCode->IsRecalcModeForced() )
        {
            sal_uLong nValidation = static_cast<const SfxUInt32Item*>(pDocument->GetAttr(
                    aPos.Col(), aPos.Row(), aPos.Tab(), ATTR_VALIDDATA ))->GetValue();
            if ( nValidation )
            {
                const ScValidationData* pData = pDocument->GetValidationEntry( nValidation );
                ScRefCellValue aTmpCell(this);
                if ( pData && !pData->IsDataValid(aTmpCell, aPos))
                    pData->DoCalcError( this );
            }
        }

        // Reschedule slows the whole thing down considerably, thus only execute on percent change
        if (!pDocument->mbThreadedGroupCalcInProgress)
        {
            ScProgress *pProgress = ScProgress::GetInterpretProgress();
            if (pProgress && pProgress->Enabled())
            {
                pProgress->SetStateCountDownOnPercent(
                    pDocument->GetFormulaCodeInTree()/MIN_NO_CODES_PER_PROGRESS_UPDATE );
            }

            switch (pInterpreter->GetVolatileType())
            {
                case ScInterpreter::VOLATILE:
                    // Volatile via built-in volatile functions.  No actions needed.
                break;
                case ScInterpreter::VOLATILE_MACRO:
                    // The formula contains a volatile macro.
                    pCode->SetExclusiveRecalcModeAlways();
                    pDocument->PutInFormulaTree(this);
                    StartListeningTo(pDocument);
                break;
                case ScInterpreter::NOT_VOLATILE:
                    if (pCode->IsRecalcModeAlways())
                    {
                        // The formula was previously volatile, but no more.
                        EndListeningTo(pDocument);
                        pCode->SetExclusiveRecalcModeNormal();
                    }
                    else
                    {
                        // non-volatile formula.  End listening to the area in case
                        // it's listening due to macro module change.
                        pDocument->EndListeningArea(BCA_LISTEN_ALWAYS, false, this);
                    }
                    pDocument->RemoveFromFormulaTree(this);
                break;
                default:
                    ;
            }
        }
    }
    else
    {
        // Cells with compiler errors should not be marked dirty forever
        OSL_ENSURE( pCode->GetCodeError() != FormulaError::NONE, "no RPN code and no errors ?!?!" );
        ResetDirty();
    }
}

void ScFormulaCell::HandleStuffAfterParallelCalculation()
{
    if( pCode->GetCodeLen() && pDocument )
    {
        if ( !pCode->IsRecalcModeAlways() )
            pDocument->RemoveFromFormulaTree( this );

        ScInterpreter* pInterpreter = new ScInterpreter( this, pDocument, pDocument->GetNonThreadedContext(), aPos, *pCode );
        StackCleaner aStackCleaner(pInterpreter);

        switch (pInterpreter->GetVolatileType())
        {
            case ScInterpreter::VOLATILE_MACRO:
                // The formula contains a volatile macro.
                pCode->SetExclusiveRecalcModeAlways();
                pDocument->PutInFormulaTree(this);
                StartListeningTo(pDocument);
            break;
            case ScInterpreter::NOT_VOLATILE:
                if (pCode->IsRecalcModeAlways())
                {
                    // The formula was previously volatile, but no more.
                    EndListeningTo(pDocument);
                    pCode->SetExclusiveRecalcModeNormal();
                }
                else
                {
                    // non-volatile formula.  End listening to the area in case
                    // it's listening due to macro module change.
                    pDocument->EndListeningArea(BCA_LISTEN_ALWAYS, false, this);
                }
                pDocument->RemoveFromFormulaTree(this);
            break;
            default:
                ;
        }
    }
}

void ScFormulaCell::SetCompile( bool bVal )
{
    bCompile = bVal;
}

void ScFormulaCell::SetMatColsRows( SCCOL nCols, SCROW nRows )
{
    ScMatrixFormulaCellToken* pMat = aResult.GetMatrixFormulaCellTokenNonConst();
    if (pMat)
        pMat->SetMatColsRows( nCols, nRows );
    else if (nCols || nRows)
    {
        aResult.SetToken( new ScMatrixFormulaCellToken( nCols, nRows));
        // Setting the new token actually forces an empty result at this top
        // left cell, so have that recalculated.
        SetDirty();
    }
}

void ScFormulaCell::GetMatColsRows( SCCOL & nCols, SCROW & nRows ) const
{
    const ScMatrixFormulaCellToken* pMat = aResult.GetMatrixFormulaCellToken();
    if (pMat)
        pMat->GetMatColsRows( nCols, nRows);
    else
    {
        nCols = 0;
        nRows = 0;
    }
}

void ScFormulaCell::SetInChangeTrack( bool bVal )
{
    bInChangeTrack = bVal;
}

void ScFormulaCell::Notify( const SfxHint& rHint )
{
    if (pDocument->IsInDtorClear())
        return;

    const SfxHintId nHint = rHint.GetId();
    if (nHint == SfxHintId::ScReference)
    {
        const sc::RefHint& rRefHint = static_cast<const sc::RefHint&>(rHint);

        switch (rRefHint.getType())
        {
            case sc::RefHint::Moved:
            {
                // One of the references has moved.

                const sc::RefMovedHint& rRefMoved = static_cast<const sc::RefMovedHint&>(rRefHint);
                if (!IsShared() || IsSharedTop())
                {
                    sc::RefUpdateResult aRes = pCode->MoveReference(aPos, rRefMoved.getContext());
                    if (aRes.mbNameModified)
                    {
                        // RPN token needs to be re-generated.
                        bCompile = true;
                        CompileTokenArray();
                        SetDirtyVar();
                    }
                }
            }
            break;
            case sc::RefHint::ColumnReordered:
            {
                const sc::RefColReorderHint& rRefColReorder =
                    static_cast<const sc::RefColReorderHint&>(rRefHint);
                if (!IsShared() || IsSharedTop())
                    pCode->MoveReferenceColReorder(
                        aPos, rRefColReorder.getTab(),
                        rRefColReorder.getStartRow(),
                        rRefColReorder.getEndRow(),
                        rRefColReorder.getColMap());
            }
            break;
            case sc::RefHint::RowReordered:
            {
                const sc::RefRowReorderHint& rRefRowReorder =
                    static_cast<const sc::RefRowReorderHint&>(rRefHint);
                if (!IsShared() || IsSharedTop())
                    pCode->MoveReferenceRowReorder(
                        aPos, rRefRowReorder.getTab(),
                        rRefRowReorder.getStartColumn(),
                        rRefRowReorder.getEndColumn(),
                        rRefRowReorder.getRowMap());
            }
            break;
            case sc::RefHint::StartListening:
            {
                StartListeningTo( pDocument);
            }
            break;
            case sc::RefHint::StopListening:
            {
                EndListeningTo( pDocument);
            }
            break;
            default:
                ;
        }

        return;
    }

    if ( pDocument->GetHardRecalcState() == ScDocument::HardRecalcState::OFF )
    {
        if (nHint == SfxHintId::ScDataChanged || nHint == SfxHintId::ScTableOpDirty || (bSubTotal && nHint == SfxHintId::ScHiddenRowsChanged))
        {
            bool bForceTrack = false;
            if ( nHint == SfxHintId::ScTableOpDirty )
            {
                bForceTrack = !bTableOpDirty;
                if ( !bTableOpDirty )
                {
                    pDocument->AddTableOpFormulaCell( this );
                    bTableOpDirty = true;
                }
            }
            else
            {
                bForceTrack = !bDirty;
                SetDirtyVar();
            }
            // Don't remove from FormulaTree to put in FormulaTrack to
            // put in FormulaTree again and again, only if necessary.
            // Any other means except ScRecalcMode::ALWAYS by which a cell could
            // be in FormulaTree if it would notify other cells through
            // FormulaTrack which weren't in FormulaTrack/FormulaTree before?!?
            // Yes. The new TableOpDirty made it necessary to have a
            // forced mode where formulas may still be in FormulaTree from
            // TableOpDirty but have to notify dependents for normal dirty.
            if ( (bForceTrack || !pDocument->IsInFormulaTree( this )
                    || pCode->IsRecalcModeAlways())
                    && !pDocument->IsInFormulaTrack( this ) )
                pDocument->AppendToFormulaTrack( this );
        }
    }
}

void ScFormulaCell::Query( SvtListener::QueryBase& rQuery ) const
{
    switch (rQuery.getId())
    {
        case SC_LISTENER_QUERY_FORMULA_GROUP_POS:
        {
            sc::RefQueryFormulaGroup& rRefQuery =
                static_cast<sc::RefQueryFormulaGroup&>(rQuery);
            if (IsShared())
                rRefQuery.add(aPos);
        }
        break;
        default:
            ;
    }
}

void ScFormulaCell::SetDirty( bool bDirtyFlag )
{
    if (IsInChangeTrack())
        return;

    if ( pDocument->GetHardRecalcState() != ScDocument::HardRecalcState::OFF )
    {
        SetDirtyVar();
        pDocument->SetStreamValid(aPos.Tab(), false);
        return;
    }

    // Avoid multiple formula tracking in Load() and in CompileAll()
    // after CopyScenario() and CopyBlockFromClip().
    // If unconditional formula tracking is needed, set bDirty=false
    // before calling SetDirty(), for example in CompileTokenArray().
    if ( !bDirty || mbPostponedDirty || !pDocument->IsInFormulaTree( this ) )
    {
        if( bDirtyFlag )
            SetDirtyVar();
        pDocument->AppendToFormulaTrack( this );

        // While loading a document listeners have not been established yet.
        // Tracking would remove this cell from the FormulaTrack and add it to
        // the FormulaTree, once in there it would be assumed that its
        // dependents already had been tracked and it would be skipped on a
        // subsequent notify. Postpone tracking until all listeners are set.
        if (!pDocument->IsImportingXML())
            pDocument->TrackFormulas();
    }

    pDocument->SetStreamValid(aPos.Tab(), false);
}

void ScFormulaCell::SetDirtyVar()
{
    bDirty = true;
    mbPostponedDirty = false;
    if (mxGroup && mxGroup->meCalcState == sc::GroupCalcRunning)
        mxGroup->meCalcState = sc::GroupCalcEnabled;

    // mark the sheet of this cell to be calculated
    //#FIXME do we need to revert this remnant of old fake vba events? pDocument->AddCalculateTable( aPos.Tab() );
}

void ScFormulaCell::SetDirtyAfterLoad()
{
    bDirty = true;
    if ( pDocument->GetHardRecalcState() == ScDocument::HardRecalcState::OFF )
        pDocument->PutInFormulaTree( this );
}

void ScFormulaCell::ResetTableOpDirtyVar()
{
    bTableOpDirty = false;
}

void ScFormulaCell::SetTableOpDirty()
{
    if ( !IsInChangeTrack() )
    {
        if ( pDocument->GetHardRecalcState() != ScDocument::HardRecalcState::OFF )
            bTableOpDirty = true;
        else
        {
            if ( !bTableOpDirty || !pDocument->IsInFormulaTree( this ) )
            {
                if ( !bTableOpDirty )
                {
                    pDocument->AddTableOpFormulaCell( this );
                    bTableOpDirty = true;
                }
                pDocument->AppendToFormulaTrack( this );
                pDocument->TrackFormulas( SfxHintId::ScTableOpDirty );
            }
        }
    }
}

bool ScFormulaCell::IsDirtyOrInTableOpDirty() const
{
    return bDirty || (bTableOpDirty && pDocument->IsInInterpreterTableOp());
}

void ScFormulaCell::SetResultDouble( double n )
{
    aResult.SetDouble(n);
}

void ScFormulaCell::SetResultToken( const formula::FormulaToken* pToken )
{
    aResult.SetToken(pToken);
}

svl::SharedString ScFormulaCell::GetResultString() const
{
    return aResult.GetString();
}

void ScFormulaCell::SetResultMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, const formula::FormulaToken* pUL )
{
    aResult.SetMatrix(nCols, nRows, pMat, pUL);
}

void ScFormulaCell::SetErrCode( FormulaError n )
{
    /* FIXME: check the numerous places where ScTokenArray::GetCodeError() is
     * used whether it is solely for transport of a simple result error and get
     * rid of that abuse. */
    pCode->SetCodeError( n );
    // Hard set errors are transported as result type value per convention,
    // e.g. via clipboard. ScFormulaResult::IsValue() and
    // ScFormulaResult::GetDouble() handle that.
    aResult.SetResultError( n );
}

void ScFormulaCell::SetResultError( FormulaError n )
{
    aResult.SetResultError( n );
}

void ScFormulaCell::AddRecalcMode( ScRecalcMode nBits )
{
    if ( (nBits & ScRecalcMode::EMask) != ScRecalcMode::NORMAL )
        SetDirtyVar();
    if ( nBits & ScRecalcMode::ONLOAD_ONCE )
    {   // OnLoadOnce is used only to set Dirty after filter import.
        nBits = (nBits & ~ScRecalcMode::EMask) | ScRecalcMode::NORMAL;
    }
    pCode->AddRecalcMode( nBits );
}

void ScFormulaCell::SetHybridDouble( double n )
{
    aResult.SetHybridDouble( n);
}

void ScFormulaCell::SetHybridString( const svl::SharedString& r )
{
    aResult.SetHybridString( r);
}

void ScFormulaCell::SetHybridEmptyDisplayedAsString()
{
    aResult.SetHybridEmptyDisplayedAsString();
}

void ScFormulaCell::SetHybridFormula( const OUString& r,
                                    const formula::FormulaGrammar::Grammar eGrammar )
{
    aResult.SetHybridFormula( r); eTempGrammar = eGrammar;
}

OUString ScFormulaCell::GetHybridFormula() const
{
    return aResult.GetHybridFormula();
}

// Dynamically create the URLField on a mouse-over action on a hyperlink() cell.
void ScFormulaCell::GetURLResult( OUString& rURL, OUString& rCellText )
{
    OUString aCellString;

    Color* pColor;

    // Cell Text uses the Cell format while the URL uses
    // the default format for the type.
    const sal_uInt32 nCellFormat = pDocument->GetNumberFormat( aPos );
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();

    const sal_uInt32 nURLFormat = ScGlobal::GetStandardFormat( *pFormatter, nCellFormat, css::util::NumberFormat::NUMBER);

    if ( IsValue() )
    {
        double fValue = GetValue();
        pFormatter->GetOutputString( fValue, nCellFormat, rCellText, &pColor );
    }
    else
    {
        aCellString = GetString().getString();
        pFormatter->GetOutputString( aCellString, nCellFormat, rCellText, &pColor );
    }
    ScConstMatrixRef xMat( aResult.GetMatrix());
    if (xMat)
    {
        // determine if the matrix result is a string or value.
        if (!xMat->IsValue(0, 1))
            rURL = xMat->GetString(0, 1).getString();
        else
            pFormatter->GetOutputString(
                xMat->GetDouble(0, 1), nURLFormat, rURL, &pColor);
    }

    if(rURL.isEmpty())
    {
        if(IsValue())
            pFormatter->GetOutputString( GetValue(), nURLFormat, rURL, &pColor );
        else
            pFormatter->GetOutputString( aCellString, nURLFormat, rURL, &pColor );
    }
}

bool ScFormulaCell::IsMultilineResult()
{
    if (!IsValue())
        return aResult.IsMultiline();
    return false;
}

bool ScFormulaCell::NeedsInterpret() const
{
    if (!IsDirtyOrInTableOpDirty())
        return false;

    return (pDocument->GetAutoCalc() || (cMatrixFlag != ScMatrixMode::NONE));
}

void ScFormulaCell::MaybeInterpret()
{
    if (NeedsInterpret())
    {
        assert(!pDocument->mbThreadedGroupCalcInProgress);
        Interpret();
    }
}

bool ScFormulaCell::IsHyperLinkCell() const
{
    return pCode && pCode->IsHyperLink();
}

EditTextObject* ScFormulaCell::CreateURLObject()
{
    OUString aCellText;
    OUString aURL;
    GetURLResult( aURL, aCellText );

    return ScEditUtil::CreateURLObjectFromURL( *pDocument, aURL, aCellText );
}

bool ScFormulaCell::IsEmpty()
{
    MaybeInterpret();
    return aResult.GetCellResultType() == formula::svEmptyCell;
}

bool ScFormulaCell::IsEmptyDisplayedAsString()
{
    MaybeInterpret();
    return aResult.IsEmptyDisplayedAsString();
}

bool ScFormulaCell::IsValue()
{
    MaybeInterpret();
    return aResult.IsValue();
}

bool ScFormulaCell::IsValueNoError()
{
    MaybeInterpret();
    if (pCode->GetCodeError() != FormulaError::NONE)
        return false;

    return aResult.IsValueNoError();
}

bool ScFormulaCell::IsValueNoError() const
{
    if (NeedsInterpret())
        // false if the cell is dirty & needs to be interpreted.
        return false;

    if (pCode->GetCodeError() != FormulaError::NONE)
        return false;

    return aResult.IsValueNoError();
}

double ScFormulaCell::GetValue()
{
    MaybeInterpret();
    return GetRawValue();
}

svl::SharedString ScFormulaCell::GetString()
{
    MaybeInterpret();
    return GetRawString();
}

double ScFormulaCell::GetRawValue() const
{
    if ((pCode->GetCodeError() == FormulaError::NONE) &&
            aResult.GetResultError() == FormulaError::NONE)
        return aResult.GetDouble();
    return 0.0;
}

svl::SharedString ScFormulaCell::GetRawString() const
{
    if ((pCode->GetCodeError() == FormulaError::NONE) &&
            aResult.GetResultError() == FormulaError::NONE)
        return aResult.GetString();

    return svl::SharedString::getEmptyString();
}

const ScMatrix* ScFormulaCell::GetMatrix()
{
    if ( pDocument->GetAutoCalc() )
    {
        if( IsDirtyOrInTableOpDirty()
        // Was stored !bDirty but an accompanying matrix cell was bDirty?
        || (!bDirty && cMatrixFlag == ScMatrixMode::Formula && !aResult.GetMatrix()))
            Interpret();
    }
    return aResult.GetMatrix().get();
}

bool ScFormulaCell::GetMatrixOrigin( ScAddress& rPos ) const
{
    switch ( cMatrixFlag )
    {
        case ScMatrixMode::Formula :
            rPos = aPos;
            return true;
        case ScMatrixMode::Reference :
        {
            formula::FormulaTokenArrayPlainIterator aIter(*pCode);
            formula::FormulaToken* t = aIter.GetNextReferenceRPN();
            if( t )
            {
                ScSingleRefData& rRef = *t->GetSingleRef();
                ScAddress aAbs = rRef.toAbs(aPos);
                if (ValidAddress(aAbs))
                {
                    rPos = aAbs;
                    return true;
                }
            }
        }
        break;
        default: break;
    }
    return false;
}

sc::MatrixEdge ScFormulaCell::GetMatrixEdge( ScAddress& rOrgPos ) const
{
    switch ( cMatrixFlag )
    {
        case ScMatrixMode::Formula :
        case ScMatrixMode::Reference :
        {
            static thread_local SCCOL nC;
            static thread_local SCROW nR;
            ScAddress aOrg;
            if ( !GetMatrixOrigin( aOrg ) )
                return sc::MatrixEdge::Nothing;
            if ( aOrg != rOrgPos )
            {   // First time or a different matrix than last time.
                rOrgPos = aOrg;
                const ScFormulaCell* pFCell;
                if ( cMatrixFlag == ScMatrixMode::Reference )
                    pFCell = pDocument->GetFormulaCell(aOrg);
                else
                    pFCell = this;      // this ScMatrixMode::Formula
                // There's only one this, don't compare pFCell==this.
                if (pFCell && pFCell->cMatrixFlag == ScMatrixMode::Formula)
                {
                    pFCell->GetMatColsRows( nC, nR );
                    if ( nC == 0 || nR == 0 )
                    {
                        // No ScMatrixFormulaCellToken available yet, calculate new.
                        nC = 1;
                        nR = 1;
                        ScAddress aTmpOrg;
                        ScFormulaCell* pCell;
                        ScAddress aAdr( aOrg );
                        aAdr.IncCol();
                        bool bCont = true;
                        do
                        {
                            pCell = pDocument->GetFormulaCell(aAdr);
                            if (pCell && pCell->cMatrixFlag == ScMatrixMode::Reference &&
                                pCell->GetMatrixOrigin(aTmpOrg) && aTmpOrg == aOrg)
                            {
                                nC++;
                                aAdr.IncCol();
                            }
                            else
                                bCont = false;
                        } while ( bCont );
                        aAdr = aOrg;
                        aAdr.IncRow();
                        bCont = true;
                        do
                        {
                            pCell = pDocument->GetFormulaCell(aAdr);
                            if (pCell && pCell->cMatrixFlag == ScMatrixMode::Reference &&
                                pCell->GetMatrixOrigin(aTmpOrg) && aTmpOrg == aOrg)
                            {
                                nR++;
                                aAdr.IncRow();
                            }
                            else
                                bCont = false;
                        } while ( bCont );

                        const_cast<ScFormulaCell*>(pFCell)->SetMatColsRows(nC, nR);
                    }
                }
                else
                {
#if OSL_DEBUG_LEVEL > 0
                    SAL_WARN( "sc", "broken Matrix, no MatFormula at origin, Pos: "
                                << aPos.Format(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID, pDocument)
                                << ", MatOrg: "
                                << aOrg.Format(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID, pDocument) );
#endif
                    return sc::MatrixEdge::Nothing;
                }
            }
            // here we are, healthy and clean, somewhere in between
            SCCOL dC = aPos.Col() - aOrg.Col();
            SCROW dR = aPos.Row() - aOrg.Row();
            sc::MatrixEdge nEdges = sc::MatrixEdge::Nothing;
            if ( dC >= 0 && dR >= 0 && dC < nC && dR < nR )
            {
                if ( dC == 0 )
                    nEdges |= sc::MatrixEdge::Left;
                if ( dC+1 == nC )
                    nEdges |= sc::MatrixEdge::Right;
                if ( dR == 0 )
                    nEdges |= sc::MatrixEdge::Top;
                if ( dR+1 == nR )
                    nEdges |= sc::MatrixEdge::Bottom;
                if ( nEdges == sc::MatrixEdge::Nothing )
                    nEdges = sc::MatrixEdge::Inside;
            }
            else
            {
                SAL_WARN( "sc", "broken Matrix, Pos: "
                    << aPos.Format(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID, pDocument)
                    << ", MatOrg: "
                    << aOrg.Format(ScRefFlags::COL_VALID | ScRefFlags::ROW_VALID, pDocument)
                    << ", MatCols: " << static_cast<sal_Int32>( nC )
                    << ", MatRows: " << static_cast<sal_Int32>( nR )
                    << ", DiffCols: " << static_cast<sal_Int32>( dC )
                    << ", DiffRows: " << static_cast<sal_Int32>( dR ));
            }
            return nEdges;
        }
        default:
            return sc::MatrixEdge::Nothing;
    }
}

FormulaError ScFormulaCell::GetErrCode()
{
    MaybeInterpret();

    /* FIXME: If ScTokenArray::SetCodeError() was really only for code errors
     * and not also abused for signaling other error conditions we could bail
     * out even before attempting to interpret broken code. */
    FormulaError nErr =  pCode->GetCodeError();
    if (nErr != FormulaError::NONE)
        return nErr;
    return aResult.GetResultError();
}

FormulaError ScFormulaCell::GetRawError()
{
    FormulaError nErr =  pCode->GetCodeError();
    if (nErr != FormulaError::NONE)
        return nErr;
    return aResult.GetResultError();
}

bool ScFormulaCell::GetErrorOrValue( FormulaError& rErr, double& rVal )
{
    MaybeInterpret();

    rErr = pCode->GetCodeError();
    if (rErr != FormulaError::NONE)
        return true;

    return aResult.GetErrorOrDouble(rErr, rVal);
}

sc::FormulaResultValue ScFormulaCell::GetResult()
{
    MaybeInterpret();

    FormulaError nErr = pCode->GetCodeError();
    if (nErr != FormulaError::NONE)
        return sc::FormulaResultValue(nErr);

    return aResult.GetResult();
}

sc::FormulaResultValue ScFormulaCell::GetResult() const
{
    FormulaError nErr = pCode->GetCodeError();
    if (nErr != FormulaError::NONE)
        return sc::FormulaResultValue(nErr);

    return aResult.GetResult();
}

bool ScFormulaCell::HasOneReference( ScRange& r ) const
{
    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* p = aIter.GetNextReferenceRPN();
    if( p && !aIter.GetNextReferenceRPN() )        // only one!
    {
        SingleDoubleRefProvider aProv( *p );
        r.aStart = aProv.Ref1.toAbs(aPos);
        r.aEnd = aProv.Ref2.toAbs(aPos);
        return true;
    }
    else
        return false;
}

bool
ScFormulaCell::HasRefListExpressibleAsOneReference(ScRange& rRange) const
{
    /* If there appears just one reference in the formula, it's the same
       as HasOneReference(). If there are more of them, they can denote
       one range if they are (sole) arguments of one function.
       Union of these references must form one range and their
       intersection must be empty set.
    */

    // Detect the simple case of exactly one reference in advance without all
    // overhead.
    // #i107741# Doing so actually makes outlines using SUBTOTAL(x;reference)
    // work again, where the function does not have only references.
    if (HasOneReference( rRange))
        return true;

    // Get first reference, if any
    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* const pFirstReference(aIter.GetNextReferenceRPN());
    if (pFirstReference)
    {
        // Collect all consecutive references, starting by the one
        // already found
        std::deque<formula::FormulaToken*> aReferences;
        aReferences.push_back(pFirstReference);
        FormulaToken* pToken(aIter.NextRPN());
        FormulaToken* pFunction(nullptr);
        while (pToken)
        {
            if (lcl_isReference(*pToken))
            {
                aReferences.push_back(pToken);
                pToken = aIter.NextRPN();
            }
            else
            {
                if (pToken->IsFunction())
                {
                    pFunction = pToken;
                }
                break;
            }
        }
        if (pFunction && !aIter.GetNextReferenceRPN()
                && (pFunction->GetParamCount() == aReferences.size()))
        {
            return lcl_refListFormsOneRange(aPos, aReferences, rRange);
        }
    }
    return false;
}

ScFormulaCell::RelNameRef ScFormulaCell::HasRelNameReference() const
{
    RelNameRef eRelNameRef = RelNameRef::NONE;
    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* t;
    while ( ( t = aIter.GetNextReferenceRPN() ) != nullptr )
    {
        switch (t->GetType())
        {
            case formula::svSingleRef:
                if (t->GetSingleRef()->IsRelName() && eRelNameRef == RelNameRef::NONE)
                    eRelNameRef = RelNameRef::SINGLE;
            break;
            case formula::svDoubleRef:
                if (t->GetDoubleRef()->Ref1.IsRelName() || t->GetDoubleRef()->Ref2.IsRelName())
                    // May originate from individual cell names, in which case
                    // it needs recompilation.
                    return RelNameRef::DOUBLE;
                /* TODO: have an extra flag at ScComplexRefData if range was
                 * extended? or too cumbersome? might narrow recompilation to
                 * only needed cases.
                 * */
            break;
            default:
                ;   // nothing
        }
    }
    return eRelNameRef;
}

bool ScFormulaCell::UpdatePosOnShift( const sc::RefUpdateContext& rCxt )
{
    if (rCxt.meMode != URM_INSDEL)
        // Just in case...
        return false;

    if (!rCxt.mnColDelta && !rCxt.mnRowDelta && !rCxt.mnTabDelta)
        // No movement.
        return false;

    if (!rCxt.maRange.In(aPos))
        return false;

    // This formula cell itself is being shifted during cell range
    // insertion or deletion. Update its position.
    ScAddress aErrorPos( ScAddress::UNINITIALIZED );
    if (!aPos.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta, aErrorPos))
    {
        assert(!"can't move ScFormulaCell");
    }

    return true;
}

namespace {

/**
 * Check if we need to re-compile column or row names.
 */
bool checkCompileColRowName(
    const sc::RefUpdateContext& rCxt, ScDocument& rDoc, const ScTokenArray& rCode,
    const ScAddress& aOldPos, const ScAddress& aPos, bool bValChanged)
{
    switch (rCxt.meMode)
    {
        case URM_INSDEL:
        {
            if (rCxt.mnColDelta <= 0 && rCxt.mnRowDelta <= 0)
                return false;

            formula::FormulaTokenArrayPlainIterator aIter(rCode);
            formula::FormulaToken* t;
            ScRangePairList* pColList = rDoc.GetColNameRanges();
            ScRangePairList* pRowList = rDoc.GetRowNameRanges();
            while ((t = aIter.GetNextColRowName()) != nullptr)
            {
                ScSingleRefData& rRef = *t->GetSingleRef();
                if (rCxt.mnRowDelta > 0 && rRef.IsColRel())
                {   // ColName
                    ScAddress aAdr = rRef.toAbs(aPos);
                    ScRangePair* pR = pColList->Find( aAdr );
                    if ( pR )
                    {   // defined
                        if (pR->GetRange(1).aStart.Row() == rCxt.maRange.aStart.Row())
                            return true;
                    }
                    else
                    {   // on the fly
                        if (aAdr.Row() + 1 == rCxt.maRange.aStart.Row())
                            return true;
                    }
                }
                if (rCxt.mnColDelta > 0 && rRef.IsRowRel())
                {   // RowName
                    ScAddress aAdr = rRef.toAbs(aPos);
                    ScRangePair* pR = pRowList->Find( aAdr );
                    if ( pR )
                    {   // defined
                        if ( pR->GetRange(1).aStart.Col() == rCxt.maRange.aStart.Col())
                            return true;
                    }
                    else
                    {   // on the fly
                        if (aAdr.Col() + 1 == rCxt.maRange.aStart.Col())
                            return true;
                    }
                }
            }
        }
        break;
        case URM_MOVE:
        {   // Recomplie for Move/D&D when ColRowName was moved or this Cell
            // points to one and was moved.
            bool bMoved = (aPos != aOldPos);
            if (bMoved)
                return true;

            formula::FormulaTokenArrayPlainIterator aIter(rCode);
            const formula::FormulaToken* t = aIter.GetNextColRowName();
            for (; t; t = aIter.GetNextColRowName())
            {
                const ScSingleRefData& rRef = *t->GetSingleRef();
                ScAddress aAbs = rRef.toAbs(aPos);
                if (ValidAddress(aAbs))
                {
                    if (rCxt.maRange.In(aAbs))
                        return true;
                }
            }
        }
        break;
        case URM_COPY:
            return bValChanged;
        default:
            ;
    }

    return false;
}

void setOldCodeToUndo(
    ScDocument* pUndoDoc, const ScAddress& aUndoPos, const ScTokenArray* pOldCode, FormulaGrammar::Grammar eTempGrammar, ScMatrixMode cMatrixFlag)
{
    // Copy the cell to aUndoPos, which is its current position in the document,
    // so this works when UpdateReference is called before moving the cells
    // (InsertCells/DeleteCells - aPos is changed above) as well as when UpdateReference
    // is called after moving the cells (MoveBlock/PasteFromClip - aOldPos is changed).

    // If there is already a formula cell in the undo document, don't overwrite it,
    // the first (oldest) is the important cell.
    if (pUndoDoc->GetCellType(aUndoPos) == CELLTYPE_FORMULA)
        return;

    ScFormulaCell* pFCell =
        new ScFormulaCell(
            pUndoDoc, aUndoPos, pOldCode ? *pOldCode : ScTokenArray(), eTempGrammar, cMatrixFlag);

    pFCell->SetResultToken(nullptr);  // to recognize it as changed later (Cut/Paste!)
    pUndoDoc->SetFormulaCell(aUndoPos, pFCell);
}

}

bool ScFormulaCell::UpdateReferenceOnShift(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (rCxt.meMode != URM_INSDEL)
        // Just in case...
        return false;

    bool bCellStateChanged = false;
    ScAddress aUndoPos( aPos );         // position for undo cell in pUndoDoc
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );
    bCellStateChanged = UpdatePosOnShift(rCxt);

    // Check presence of any references or column row names.
    bool bHasRefs = pCode->HasReferences();
    bool bHasColRowNames = false;
    if (!bHasRefs)
    {
        bHasColRowNames = (formula::FormulaTokenArrayPlainIterator(*pCode).GetNextColRowName() != nullptr);
        bHasRefs = bHasRefs || bHasColRowNames;
    }
    bool bOnRefMove = pCode->IsRecalcModeOnRefMove();

    if (!bHasRefs && !bOnRefMove)
        // This formula cell contains no references, nor needs recalculating
        // on reference update. Bail out.
        return bCellStateChanged;

    std::unique_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    bool bValChanged = false;
    bool bRefModified = false;
    bool bRecompile = bCompile;

    if (bHasRefs)
    {
        // Update cell or range references.
        sc::RefUpdateResult aRes = pCode->AdjustReferenceOnShift(rCxt, aOldPos);
        bRefModified = aRes.mbReferenceModified;
        bValChanged = aRes.mbValueChanged;
        if (aRes.mbNameModified)
            bRecompile = true;
    }

    if (bValChanged || bRefModified)
        bCellStateChanged = true;

    if (bOnRefMove)
        // Cell may reference itself, e.g. ocColumn, ocRow without parameter
        bOnRefMove = (bValChanged || (aPos != aOldPos) || bRefModified);

    bool bNewListening = false;
    bool bInDeleteUndo = false;

    if (bHasRefs)
    {
        // Upon Insert ColRowNames have to be recompiled in case the
        // insertion occurs right in front of the range.
        if (bHasColRowNames && !bRecompile)
            bRecompile = checkCompileColRowName(rCxt, *pDocument, *pCode, aOldPos, aPos, bValChanged);

        ScChangeTrack* pChangeTrack = pDocument->GetChangeTrack();
        bInDeleteUndo = (pChangeTrack && pChangeTrack->IsInDeleteUndo());

        // RelNameRefs are always moved
        bool bHasRelName = false;
        if (!bRecompile)
        {
            RelNameRef eRelNameRef = HasRelNameReference();
            bHasRelName = (eRelNameRef != RelNameRef::NONE);
            bRecompile = (eRelNameRef == RelNameRef::DOUBLE);
        }
        // Reference changed and new listening needed?
        // Except in Insert/Delete without specialties.
        bNewListening = (bRefModified || bRecompile
                || (bValChanged && bInDeleteUndo) || bHasRelName);

        if ( bNewListening )
            EndListeningTo(pDocument, pOldCode.get(), aOldPos);
    }

    // NeedDirty for changes except for Copy and Move/Insert without RelNames
    bool bNeedDirty = (bValChanged || bRecompile || bOnRefMove);

    if (pUndoDoc && (bValChanged || bOnRefMove))
        setOldCodeToUndo(pUndoDoc, aUndoPos, pOldCode.get(), eTempGrammar, cMatrixFlag);

    bCompile |= bRecompile;
    if (bCompile)
    {
        CompileTokenArray( bNewListening ); // no Listening
        bNeedDirty = true;
    }

    if ( !bInDeleteUndo )
    {   // In ChangeTrack Delete-Reject listeners are established in
        // InsertCol/InsertRow
        if ( bNewListening )
        {
            // Inserts/Deletes re-establish listeners after all
            // UpdateReference calls.
            // All replaced shared formula listeners have to be
            // established after an Insert or Delete. Do nothing here.
            SetNeedsListening( true);
        }
    }

    if (bNeedDirty)
    {   // Cut off references, invalid or similar?
        // Postpone SetDirty() until all listeners have been re-established in
        // Inserts/Deletes.
        mbPostponedDirty = true;
    }

    return bCellStateChanged;
}

bool ScFormulaCell::UpdateReferenceOnMove(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (rCxt.meMode != URM_MOVE)
        return false;

    ScAddress aUndoPos( aPos );         // position for undo cell in pUndoDoc
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );

    bool bCellInMoveTarget = rCxt.maRange.In(aPos);

    if ( bCellInMoveTarget )
    {
        // The cell is being moved or copied to a new position. I guess the
        // position has been updated prior to this call?  Determine
        // its original position before the move which will be used to adjust
        // relative references later.
        aOldPos.Set(aPos.Col() - rCxt.mnColDelta, aPos.Row() - rCxt.mnRowDelta, aPos.Tab() - rCxt.mnTabDelta);
    }

    // Check presence of any references or column row names.
    bool bHasRefs = pCode->HasReferences();
    bool bHasColRowNames = false;
    if (!bHasRefs)
    {
        bHasColRowNames = (formula::FormulaTokenArrayPlainIterator(*pCode).GetNextColRowName() != nullptr);
        bHasRefs = bHasRefs || bHasColRowNames;
    }
    bool bOnRefMove = pCode->IsRecalcModeOnRefMove();

    if (!bHasRefs && !bOnRefMove)
        // This formula cell contains no references, nor needs recalculating
        // on reference update. Bail out.
        return false;

    bool bCellStateChanged = false;
    std::unique_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    bool bValChanged = false;
    bool bRefModified = false;

    if (bHasRefs)
    {
        // Update cell or range references.
        sc::RefUpdateResult aRes = pCode->AdjustReferenceOnMove(rCxt, aOldPos, aPos);
        bRefModified = aRes.mbReferenceModified || aRes.mbNameModified;
        bValChanged = aRes.mbValueChanged;
        if (aRes.mbNameModified)
            // Re-compile to get the RPN token regenerated to reflect updated names.
            bCompile = true;
    }

    if (bValChanged || bRefModified)
        bCellStateChanged = true;

    if (bOnRefMove)
        // Cell may reference itself, e.g. ocColumn, ocRow without parameter
        bOnRefMove = (bValChanged || (aPos != aOldPos));

    bool bColRowNameCompile = false;
    bool bHasRelName = false;
    bool bNewListening = false;
    bool bInDeleteUndo = false;

    if (bHasRefs)
    {
        // Upon Insert ColRowNames have to be recompiled in case the
        // insertion occurs right in front of the range.
        if (bHasColRowNames)
            bColRowNameCompile = checkCompileColRowName(rCxt, *pDocument, *pCode, aOldPos, aPos, bValChanged);

        ScChangeTrack* pChangeTrack = pDocument->GetChangeTrack();
        bInDeleteUndo = (pChangeTrack && pChangeTrack->IsInDeleteUndo());

        // RelNameRefs are always moved
        RelNameRef eRelNameRef = HasRelNameReference();
        bHasRelName = (eRelNameRef != RelNameRef::NONE);
        bCompile |= (eRelNameRef == RelNameRef::DOUBLE);
        // Reference changed and new listening needed?
        // Except in Insert/Delete without specialties.
        bNewListening = (bRefModified || bColRowNameCompile
                || bValChanged || bHasRelName)
            // #i36299# Don't duplicate action during cut&paste / drag&drop
            // on a cell in the range moved, start/end listeners is done
            // via ScDocument::DeleteArea() and ScDocument::CopyFromClip().
            && !(pDocument->IsInsertingFromOtherDoc() && rCxt.maRange.In(aPos));

        if ( bNewListening )
            EndListeningTo(pDocument, pOldCode.get(), aOldPos);
    }

    bool bNeedDirty = false;
    // NeedDirty for changes except for Copy and Move/Insert without RelNames
    if ( bRefModified || bColRowNameCompile ||
         (bValChanged && bHasRelName ) || bOnRefMove)
        bNeedDirty = true;

    if (pUndoDoc && !bCellInMoveTarget && (bValChanged || bRefModified || bOnRefMove))
        setOldCodeToUndo(pUndoDoc, aUndoPos, pOldCode.get(), eTempGrammar, cMatrixFlag);

    bValChanged = false;

    bCompile = (bCompile || bValChanged || bColRowNameCompile);
    if ( bCompile )
    {
        CompileTokenArray( bNewListening ); // no Listening
        bNeedDirty = true;
    }

    if ( !bInDeleteUndo )
    {   // In ChangeTrack Delete-Reject listeners are established in
        // InsertCol/InsertRow
        if ( bNewListening )
        {
            StartListeningTo( pDocument );
        }
    }

    if (bNeedDirty)
    {   // Cut off references, invalid or similar?
        sc::AutoCalcSwitch aACSwitch(*pDocument, false);
        SetDirty();
    }

    return bCellStateChanged;
}

bool ScFormulaCell::UpdateReferenceOnCopy(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (rCxt.meMode != URM_COPY)
        return false;

    ScAddress aUndoPos( aPos );         // position for undo cell in pUndoDoc
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );

    if (rCxt.maRange.In(aPos))
    {
        // The cell is being moved or copied to a new position. I guess the
        // position has been updated prior to this call?  Determine
        // its original position before the move which will be used to adjust
        // relative references later.
        aOldPos.Set(aPos.Col() - rCxt.mnColDelta, aPos.Row() - rCxt.mnRowDelta, aPos.Tab() - rCxt.mnTabDelta);
    }

    // Check presence of any references or column row names.
    bool bHasRefs = pCode->HasReferences();
    bool bHasColRowNames = (formula::FormulaTokenArrayPlainIterator(*pCode).GetNextColRowName() != nullptr);
    bHasRefs = bHasRefs || bHasColRowNames;
    bool bOnRefMove = pCode->IsRecalcModeOnRefMove();

    if (!bHasRefs && !bOnRefMove)
        // This formula cell contains no references, nor needs recalculating
        // on reference update. Bail out.
        return false;

    std::unique_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    if (bOnRefMove)
        // Cell may reference itself, e.g. ocColumn, ocRow without parameter
        bOnRefMove = (aPos != aOldPos);

    bool bNeedDirty = bOnRefMove;

    if (pUndoDoc && bOnRefMove)
        setOldCodeToUndo(pUndoDoc, aUndoPos, pOldCode.get(), eTempGrammar, cMatrixFlag);

    if (bCompile)
    {
        CompileTokenArray(); // no Listening
        bNeedDirty = true;
    }

    if (bNeedDirty)
    {   // Cut off references, invalid or similar?
        sc::AutoCalcSwitch aACSwitch(*pDocument, false);
        SetDirty();
    }

    return false;
}

bool ScFormulaCell::UpdateReference(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (pDocument->IsClipOrUndo())
        return false;

    if (mxGroup && mxGroup->mpTopCell != this)
    {
        // This is not a top cell of a formula group. Don't update references.

        switch (rCxt.meMode)
        {
            case URM_INSDEL:
                return UpdatePosOnShift(rCxt);
            break;
            default:
                ;
        }
        return false;
    }

    switch (rCxt.meMode)
    {
        case URM_INSDEL:
            return UpdateReferenceOnShift(rCxt, pUndoDoc, pUndoCellPos);
        case URM_MOVE:
            return UpdateReferenceOnMove(rCxt, pUndoDoc, pUndoCellPos);
        case URM_COPY:
            return UpdateReferenceOnCopy(rCxt, pUndoDoc, pUndoCellPos);
        default:
            ;
    }

    return false;
}

void ScFormulaCell::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    // Adjust tokens only when it's not grouped or grouped top cell.
    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    bool bPosChanged = (rCxt.mnInsertPos <= aPos.Tab());
    if (pDocument->IsClipOrUndo() || !pCode->HasReferences())
    {
        if (bPosChanged)
            aPos.IncTab(rCxt.mnSheets);

        return;
    }

    EndListeningTo( pDocument );
    ScAddress aOldPos = aPos;
    // IncTab _after_ EndListeningTo and _before_ Compiler UpdateInsertTab!
    if (bPosChanged)
        aPos.IncTab(rCxt.mnSheets);

    if (!bAdjustCode)
        return;

    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnInsertedTab(rCxt, aOldPos);
    if (aRes.mbNameModified)
        // Re-compile after new sheet(s) have been inserted.
        bCompile = true;

    // no StartListeningTo because the new sheets have not been inserted yet.
}

void ScFormulaCell::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    // Adjust tokens only when it's not grouped or grouped top cell.
    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    bool bPosChanged = (aPos.Tab() >= rCxt.mnDeletePos + rCxt.mnSheets);
    if (pDocument->IsClipOrUndo() || !pCode->HasReferences())
    {
        if (bPosChanged)
            aPos.IncTab(-1*rCxt.mnSheets);
        return;
    }

    EndListeningTo( pDocument );
    // IncTab _after_ EndListeningTo and _before_ Compiler UpdateDeleteTab!
    ScAddress aOldPos = aPos;
    if (bPosChanged)
        aPos.IncTab(-1*rCxt.mnSheets);

    if (!bAdjustCode)
        return;

    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnDeletedTab(rCxt, aOldPos);
    if (aRes.mbNameModified)
        // Re-compile after sheet(s) have been deleted.
        bCompile = true;
}

void ScFormulaCell::UpdateMoveTab( const sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo )
{
    // Adjust tokens only when it's not grouped or grouped top cell.
    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;

    if (!pCode->HasReferences() || pDocument->IsClipOrUndo())
    {
        aPos.SetTab(nTabNo);
        return;
    }

    EndListeningTo(pDocument);
    ScAddress aOldPos = aPos;
    // SetTab _after_ EndListeningTo and _before_ Compiler UpdateMoveTab !
    aPos.SetTab(nTabNo);

    // no StartListeningTo because pTab[nTab] not yet correct!

    if (!bAdjustCode)
        return;

    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnMovedTab(rCxt, aOldPos);
    if (aRes.mbNameModified)
        // Re-compile after sheet(s) have been deleted.
        bCompile = true;
}

void ScFormulaCell::UpdateInsertTabAbs(SCTAB nTable)
{
    if (pDocument->IsClipOrUndo())
        return;

    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    if (!bAdjustCode)
        return;

    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* p = aIter.GetNextReferenceRPN();
    while (p)
    {
        ScSingleRefData& rRef1 = *p->GetSingleRef();
        if (!rRef1.IsTabRel() && nTable <= rRef1.Tab())
            rRef1.IncTab(1);
        if (p->GetType() == formula::svDoubleRef)
        {
            ScSingleRefData& rRef2 = p->GetDoubleRef()->Ref2;
            if (!rRef2.IsTabRel() && nTable <= rRef2.Tab())
                rRef2.IncTab(1);
        }
        p = aIter.GetNextReferenceRPN();
    }
}

bool ScFormulaCell::TestTabRefAbs(SCTAB nTable)
{
    if (pDocument->IsClipOrUndo())
        return false;

    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    if (!bAdjustCode)
        return false;

    bool bRet = false;
    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* p = aIter.GetNextReferenceRPN();
    while (p)
    {
        ScSingleRefData& rRef1 = *p->GetSingleRef();
        if (!rRef1.IsTabRel())
        {
            if (nTable != rRef1.Tab())
                bRet = true;
            else if (nTable != aPos.Tab())
                rRef1.SetAbsTab(aPos.Tab());
        }
        if (p->GetType() == formula::svDoubleRef)
        {
            ScSingleRefData& rRef2 = p->GetDoubleRef()->Ref2;
            if (!rRef2.IsTabRel())
            {
                if(nTable != rRef2.Tab())
                    bRet = true;
                else if (nTable != aPos.Tab())
                    rRef2.SetAbsTab(aPos.Tab());
            }
        }
        p = aIter.GetNextReferenceRPN();
    }
    return bRet;
}

void ScFormulaCell::UpdateCompile( bool bForceIfNameInUse )
{
    if ( bForceIfNameInUse && !bCompile )
        bCompile = pCode->HasNameOrColRowName();
    if ( bCompile )
        pCode->SetCodeError( FormulaError::NONE );   // make sure it will really be compiled
    CompileTokenArray();
}

// Reference transposition is only called in Clipboard Document
void ScFormulaCell::TransposeReference()
{
    bool bFound = false;
    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* t;
    while ( ( t = aIter.GetNextReference() ) != nullptr )
    {
        ScSingleRefData& rRef1 = *t->GetSingleRef();
        if ( rRef1.IsColRel() && rRef1.IsRowRel() )
        {
            bool bDouble = (t->GetType() == formula::svDoubleRef);
            ScSingleRefData& rRef2 = (bDouble ? t->GetDoubleRef()->Ref2 : rRef1);
            if ( !bDouble || (rRef2.IsColRel() && rRef2.IsRowRel()) )
            {
                SCCOLROW nTemp;

                nTemp = rRef1.Col();
                rRef1.SetRelCol(rRef1.Row());
                rRef1.SetRelRow(nTemp);

                if ( bDouble )
                {
                    nTemp = rRef2.Col();
                    rRef2.SetRelCol(rRef2.Row());
                    rRef2.SetRelRow(nTemp);
                }

                bFound = true;
            }
        }
    }

    if (bFound)
        bCompile = true;
}

void ScFormulaCell::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                        ScDocument* pUndoDoc )
{
    EndListeningTo( pDocument );

    ScAddress aOldPos = aPos;
    bool bPosChanged = false; // Whether this cell has been moved

    ScRange aDestRange( rDest, ScAddress(
                static_cast<SCCOL>(rDest.Col() + rSource.aEnd.Row() - rSource.aStart.Row()),
                static_cast<SCROW>(rDest.Row() + rSource.aEnd.Col() - rSource.aStart.Col()),
                rDest.Tab() + rSource.aEnd.Tab() - rSource.aStart.Tab() ) );
    if ( aDestRange.In( aOldPos ) )
    {
        // Count back Positions
        SCCOL nRelPosX = aOldPos.Col();
        SCROW nRelPosY = aOldPos.Row();
        SCTAB nRelPosZ = aOldPos.Tab();
        ScRefUpdate::DoTranspose( nRelPosX, nRelPosY, nRelPosZ, pDocument, aDestRange, rSource.aStart );
        aOldPos.Set( nRelPosX, nRelPosY, nRelPosZ );
        bPosChanged = true;
    }

    ScTokenArray* pOld = pUndoDoc ? pCode->Clone() : nullptr;
    bool bRefChanged = false;

    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* t;
    while( (t = aIter.GetNextReferenceOrName()) != nullptr )
    {
        if( t->GetOpCode() == ocName )
        {
            const ScRangeData* pName = pDocument->FindRangeNameBySheetAndIndex( t->GetSheet(), t->GetIndex());
            if (pName)
            {
                if (pName->IsModified())
                    bRefChanged = true;
            }
        }
        else if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier aMod(*t);
            ScComplexRefData& rRef = aMod.Ref();
            ScRange aAbs = rRef.toAbs(aOldPos);
            bool bMod = (ScRefUpdate::UpdateTranspose(pDocument, rSource, rDest, aAbs) != UR_NOTHING || bPosChanged);
            if (bMod)
            {
                rRef.SetRange(aAbs, aPos); // based on the new anchor position.
                bRefChanged = true;
            }
        }
    }

    if (bRefChanged)
    {
        if (pUndoDoc)
        {
            ScFormulaCell* pFCell = new ScFormulaCell(
                    pUndoDoc, aPos, pOld ? *pOld : ScTokenArray(), eTempGrammar, cMatrixFlag);

            pFCell->aResult.SetToken( nullptr);  // to recognize it as changed later (Cut/Paste!)
            pUndoDoc->SetFormulaCell(aPos, pFCell);
        }

        bCompile = true;
        CompileTokenArray(); // also call StartListeningTo
        SetDirty();
    }
    else
        StartListeningTo( pDocument ); // Listener as previous

    delete pOld;
}

void ScFormulaCell::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    EndListeningTo( pDocument );

    bool bRefChanged = false;

    formula::FormulaTokenArrayPlainIterator aIter(*pCode);
    formula::FormulaToken* t;

    while( (t = aIter.GetNextReferenceOrName()) != nullptr )
    {
        if( t->GetOpCode() == ocName )
        {
            const ScRangeData* pName = pDocument->FindRangeNameBySheetAndIndex( t->GetSheet(), t->GetIndex());
            if (pName)
            {
                if (pName->IsModified())
                    bRefChanged = true;
            }
        }
        else if( t->GetType() != svIndex )
        {
            SingleDoubleRefModifier aMod(*t);
            ScComplexRefData& rRef = aMod.Ref();
            ScRange aAbs = rRef.toAbs(aPos);
            bool bMod = (ScRefUpdate::UpdateGrow(rArea, nGrowX, nGrowY, aAbs) != UR_NOTHING);
            if (bMod)
            {
                rRef.SetRange(aAbs, aPos);
                bRefChanged = true;
            }
        }
    }

    if (bRefChanged)
    {
        bCompile = true;
        CompileTokenArray(); // Also call StartListeningTo
        SetDirty();
    }
    else
        StartListeningTo( pDocument ); // Listener as previous
}

// See also ScDocument::FindRangeNamesReferencingSheet()
static void lcl_FindRangeNamesInUse(sc::UpdatedRangeNames& rIndexes, const ScTokenArray* pCode, const ScDocument* pDoc,
        int nRecursion)
{
    FormulaTokenArrayPlainIterator aIter(*pCode);
    for (FormulaToken* p = aIter.First(); p; p = aIter.Next())
    {
        if (p->GetOpCode() == ocName)
        {
            sal_uInt16 nTokenIndex = p->GetIndex();
            SCTAB nTab = p->GetSheet();
            rIndexes.setUpdatedName( nTab, nTokenIndex);

            if (nRecursion < 126)   // whatever.. 42*3
            {
                ScRangeData* pSubName = pDoc->FindRangeNameBySheetAndIndex( nTab, nTokenIndex);
                if (pSubName)
                    lcl_FindRangeNamesInUse(rIndexes, pSubName->GetCode(), pDoc, nRecursion+1);
            }
        }
    }
}

void ScFormulaCell::FindRangeNamesInUse(sc::UpdatedRangeNames& rIndexes) const
{
    lcl_FindRangeNamesInUse( rIndexes, pCode, pDocument, 0);
}

void ScFormulaCell::SetChanged(bool b)
{
    bChanged = b;
}

void ScFormulaCell::SetCode( ScTokenArray* pNew )
{
    assert(!mxGroup); // Don't call this if it's shared.
    delete pCode;
    pCode = pNew; // takes ownership.
}

void ScFormulaCell::SetRunning( bool bVal )
{
    bRunning = bVal;
}

void ScFormulaCell::CompileDBFormula( sc::CompileFormulaContext& rCxt )
{
    FormulaTokenArrayPlainIterator aIter(*pCode);
    for( FormulaToken* p = aIter.First(); p; p = aIter.Next() )
    {
        OpCode eOp = p->GetOpCode();
        if ( eOp == ocDBArea || eOp == ocTableRef )
        {
            bCompile = true;
            CompileTokenArray(rCxt);
            SetDirty();
            break;
        }
    }
}

void ScFormulaCell::CompileColRowNameFormula( sc::CompileFormulaContext& rCxt )
{
    FormulaTokenArrayPlainIterator aIter(*pCode);
    for ( FormulaToken* p = aIter.First(); p; p = aIter.Next() )
    {
        if ( p->GetOpCode() == ocColRowName )
        {
            bCompile = true;
            CompileTokenArray(rCxt);
            SetDirty();
            break;
        }
    }
}

void            ScFormulaCell::SetPrevious( ScFormulaCell* pF )    { pPrevious = pF; }
void            ScFormulaCell::SetNext( ScFormulaCell* pF )        { pNext = pF; }
void            ScFormulaCell::SetPreviousTrack( ScFormulaCell* pF )   { pPreviousTrack = pF; }
void            ScFormulaCell::SetNextTrack( ScFormulaCell* pF )       { pNextTrack = pF; }

ScFormulaCellGroupRef ScFormulaCell::CreateCellGroup( SCROW nLen, bool bInvariant )
{
    if (mxGroup)
    {
        // You can't create a new group if the cell is already a part of a group.
        // Is this a sign of some inconsistent or incorrect data structures? Or normal?
        SAL_INFO("sc.opencl", "You can't create a new group if the cell is already a part of a group");
        return ScFormulaCellGroupRef();
    }

    mxGroup.reset(new ScFormulaCellGroup);
    mxGroup->mpTopCell = this;
    mxGroup->mbInvariant = bInvariant;
    mxGroup->mnLength = nLen;
    mxGroup->mpCode = pCode; // Move this to the shared location.
    return mxGroup;
}

void ScFormulaCell::SetCellGroup( const ScFormulaCellGroupRef &xRef )
{
    if (!xRef)
    {
        // Make this cell a non-grouped cell.
        if (mxGroup)
            pCode = mxGroup->mpCode->Clone();

        mxGroup = xRef;
        return;
    }

    // Group object has shared token array.
    if (!mxGroup)
        // Currently not shared. Delete the existing token array first.
        delete pCode;

    mxGroup = xRef;
    pCode = mxGroup->mpCode;
}

ScFormulaCell::CompareState ScFormulaCell::CompareByTokenArray( const ScFormulaCell& rOther ) const
{
    // no Matrix formulae yet.
    if ( GetMatrixFlag() != ScMatrixMode::NONE )
        return NotEqual;

    // are these formulas at all similar ?
    if ( GetHash() != rOther.GetHash() )
        return NotEqual;

    if (!pCode->IsShareable() || !rOther.pCode->IsShareable())
        return NotEqual;

    FormulaToken **pThis = pCode->GetCode();
    sal_uInt16     nThisLen = pCode->GetCodeLen();
    FormulaToken **pOther = rOther.pCode->GetCode();
    sal_uInt16     nOtherLen = rOther.pCode->GetCodeLen();

    if ( !pThis || !pOther )
    {
        // Error: no compiled code for cells !"
        return NotEqual;
    }

    if ( nThisLen != nOtherLen )
        return NotEqual;

    // No tokens can be an error cell so check error code, otherwise we could
    // end up with a series of equal error values instead of individual error
    // values. Also if for any reason different errors are set even if all
    // tokens are equal, the cells are not equal.
    if (pCode->GetCodeError() != rOther.pCode->GetCodeError())
        return NotEqual;

    bool bInvariant = true;

    // check we are basically the same function
    for ( sal_uInt16 i = 0; i < nThisLen; i++ )
    {
        formula::FormulaToken *pThisTok = pThis[i];
        formula::FormulaToken *pOtherTok = pOther[i];

        if ( pThisTok->GetType() != pOtherTok->GetType() ||
             pThisTok->GetOpCode() != pOtherTok->GetOpCode() ||
             pThisTok->GetParamCount() != pOtherTok->GetParamCount() )
        {
            // Incompatible type, op-code or param counts.
            return NotEqual;
        }

        switch (pThisTok->GetType())
        {
            case formula::svMatrix:
            case formula::svExternalSingleRef:
            case formula::svExternalDoubleRef:
                // Ignoring matrix and external references for now.
                return NotEqual;

            case formula::svSingleRef:
            {
                // Single cell reference.
                const ScSingleRefData& rRef = *pThisTok->GetSingleRef();
                if (rRef != *pOtherTok->GetSingleRef())
                    return NotEqual;

                if (rRef.IsRowRel())
                    bInvariant = false;
            }
            break;
            case formula::svDoubleRef:
            {
                // Range reference.
                const ScSingleRefData& rRef1 = *pThisTok->GetSingleRef();
                const ScSingleRefData& rRef2 = *pThisTok->GetSingleRef2();
                if (rRef1 != *pOtherTok->GetSingleRef())
                    return NotEqual;

                if (rRef2 != *pOtherTok->GetSingleRef2())
                    return NotEqual;

                if (rRef1.IsRowRel())
                    bInvariant = false;

                if (rRef2.IsRowRel())
                    bInvariant = false;
            }
            break;
            case formula::svDouble:
            {
                if(!rtl::math::approxEqual(pThisTok->GetDouble(), pOtherTok->GetDouble()))
                    return NotEqual;
            }
            break;
            case formula::svString:
            {
                if(pThisTok->GetString() != pOtherTok->GetString())
                    return NotEqual;
            }
            break;
            case formula::svIndex:
            {
                if(pThisTok->GetIndex() != pOtherTok->GetIndex() || pThisTok->GetSheet() != pOtherTok->GetSheet())
                    return NotEqual;
            }
            break;
            case formula::svByte:
            {
                if(pThisTok->GetByte() != pOtherTok->GetByte())
                    return NotEqual;
            }
            break;
            case formula::svExternal:
            {
                if (pThisTok->GetExternal() != pOtherTok->GetExternal())
                    return NotEqual;

                if (pThisTok->GetByte() != pOtherTok->GetByte())
                    return NotEqual;
            }
            break;
            case formula::svError:
            {
                if (pThisTok->GetError() != pOtherTok->GetError())
                    return NotEqual;
            }
            break;
            default:
                ;
        }
    }

    // If still the same, check lexical names as different names may result in
    // identical RPN code.

    pThis = pCode->GetArray();
    nThisLen = pCode->GetLen();
    pOther = rOther.pCode->GetArray();
    nOtherLen = rOther.pCode->GetLen();

    if ( !pThis || !pOther )
    {
        // Error: no code for cells !"
        return NotEqual;
    }

    if ( nThisLen != nOtherLen )
        return NotEqual;

    for ( sal_uInt16 i = 0; i < nThisLen; i++ )
    {
        formula::FormulaToken *pThisTok = pThis[i];
        formula::FormulaToken *pOtherTok = pOther[i];

        if ( pThisTok->GetType() != pOtherTok->GetType() ||
             pThisTok->GetOpCode() != pOtherTok->GetOpCode() ||
             pThisTok->GetParamCount() != pOtherTok->GetParamCount() )
        {
            // Incompatible type, op-code or param counts.
            return NotEqual;
        }

        switch (pThisTok->GetType())
        {
            // All index tokens are names. Different categories already had
            // different OpCode values.
            case formula::svIndex:
                {
                    if (pThisTok->GetIndex() != pOtherTok->GetIndex())
                        return NotEqual;
                    switch (pThisTok->GetOpCode())
                    {
                        case ocTableRef:
                            // nothing, sheet value assumed as -1, silence
                            // ScTableRefToken::GetSheet() SAL_WARN about
                            // unhandled
                            ;
                            break;
                        default:    // ocName, ocDBArea
                            if (pThisTok->GetSheet() != pOtherTok->GetSheet())
                                return NotEqual;
                    }
                }
                break;
            default:
                ;
        }
    }

    return bInvariant ? EqualInvariant : EqualRelativeRef;
}

namespace {

// Split N into optimally equal-sized pieces, each not larger than K.
// Return value P is number of pieces. A returns the number of pieces
// one larger than N/P, 0..P-1.

int splitup(int N, int K, int& A)
{
    assert(N > 0);
    assert(K > 0);

    A = 0;

    if (N <= K)
        return 1;

    const int ideal_num_parts = N / K;
    if (ideal_num_parts * K == N)
        return ideal_num_parts;

    const int num_parts = ideal_num_parts + 1;
    const int nominal_part_size = N / num_parts;

    A = N - num_parts * nominal_part_size;

    return num_parts;
}

} // anonymous namespace

struct ScDependantsCalculator
{
    ScDocument& mrDoc;
    const ScTokenArray& mrCode;
    const SCROW mnLen;
    const ScAddress& mrPos;

    ScDependantsCalculator(ScDocument& rDoc, const ScTokenArray& rCode, const ScFormulaCell& rCell, const ScAddress& rPos) :
        mrDoc(rDoc),
        mrCode(rCode),
        mnLen(rCell.GetCellGroup()->mnLength),
        mrPos(rPos)
    {
    }

    // FIXME: copy-pasted from ScGroupTokenConverter. factor out somewhere else

    // I think what this function does is to check whether the relative row reference nRelRow points
    // to a row that is inside the range of rows covered by the formula group.

    bool isSelfReferenceRelative(const ScAddress& rRefPos, SCROW nRelRow)
    {
        if (rRefPos.Col() != mrPos.Col())
            return false;

        SCROW nEndRow = mrPos.Row() + mnLen - 1;

        if (nRelRow < 0)
        {
            SCROW nTest = nEndRow;
            nTest += nRelRow;
            if (nTest >= mrPos.Row())
                return true;
        }
        else if (nRelRow > 0)
        {
            SCROW nTest = mrPos.Row(); // top row.
            nTest += nRelRow;
            if (nTest <= nEndRow)
                return true;
        }

        return false;
    }

    // FIXME: another copy-paste

    // And this correspondingly checks whether an absolute row is inside the range of rows covered
    // by the formula group.

    bool isSelfReferenceAbsolute(const ScAddress& rRefPos)
    {
        if (rRefPos.Col() != mrPos.Col())
            return false;

        SCROW nEndRow = mrPos.Row() + mnLen - 1;

        if (rRefPos.Row() < mrPos.Row())
            return false;

        if (rRefPos.Row() > nEndRow)
            return false;

        return true;
    }

    // FIXME: another copy-paste
    SCROW trimLength(SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCROW nRow, SCROW nRowLen)
    {
        SCROW nLastRow = nRow + nRowLen - 1; // current last row.
        nLastRow = mrDoc.GetLastDataRow(nTab, nCol1, nCol2, nLastRow);
        if (nLastRow < (nRow + nRowLen - 1))
        {
            // This can end up negative! Was that the original intent, or
            // is it accidental? Was it not like that originally but the
            // surrounding conditions changed?
            nRowLen = nLastRow - nRow + 1;
            // Anyway, let's assume it doesn't make sense to return a
            // negative or zero value here.
            if (nRowLen <= 0)
                nRowLen = 1;
        }
        else if (nLastRow == 0)
            // Column is empty.
            nRowLen = 1;

        return nRowLen;
    }

    bool DoIt()
    {
        // Partially from ScGroupTokenConverter::convert in sc/source/core/data/grouptokenconverter.cxx

        ScRangeList aRangeList;
        for (auto p: mrCode.Tokens())
        {
            switch (p->GetType())
            {
            case svSingleRef:
                {
                    ScSingleRefData aRef = *p->GetSingleRef(); // =Sheet1!A1
                    ScAddress aRefPos = aRef.toAbs(mrPos);

                    if (!mrDoc.TableExists(aRefPos.Tab()))
                        return false; // or true?

                    if (aRef.IsRowRel())
                    {
                        if (isSelfReferenceRelative(aRefPos, aRef.Row()))
                            return false;

                        // Trim data array length to actual data range.
                        SCROW nTrimLen = trimLength(aRefPos.Tab(), aRefPos.Col(), aRefPos.Col(), aRefPos.Row(), mnLen);
                        // Fetch double array guarantees that the length of the
                        // returned array equals or greater than the requested
                        // length.

                        formula::VectorRefArray aArray;
                        if (nTrimLen)
                            aArray = mrDoc.FetchVectorRefArray(aRefPos, nTrimLen);

                        if (!aArray.isValid())
                            return false;

                        aRangeList.Join(ScRange(aRefPos.Col(), aRefPos.Row(), aRefPos.Tab(),
                                                aRefPos.Col(), aRefPos.Row() + nTrimLen - 1, aRefPos.Tab()));
                    }
                    else
                    {
                        if (isSelfReferenceAbsolute(aRefPos))
                            return false;

                        aRangeList.Join(ScRange(aRefPos.Col(), aRefPos.Row(), aRefPos.Tab()));
                    }
                }
                break;
            case svDoubleRef:
                {
                    ScComplexRefData aRef = *p->GetDoubleRef();
                    ScRange aAbs = aRef.toAbs(mrPos);

                    // Multiple sheet
                    if (aRef.Ref1.Tab() != aRef.Ref2.Tab())
                        return false;

                    // Check for self reference.
                    if (aRef.Ref1.IsRowRel())
                    {
                        if (isSelfReferenceRelative(aAbs.aStart, aRef.Ref1.Row()))
                            return false;
                    }
                    else if (isSelfReferenceAbsolute(aAbs.aStart))
                        return false;

                    if (aRef.Ref2.IsRowRel())
                    {
                        if (isSelfReferenceRelative(aAbs.aEnd, aRef.Ref2.Row()))
                            return false;
                    }
                    else if (isSelfReferenceAbsolute(aAbs.aEnd))
                        return false;

                    // Row reference is relative.
                    bool bAbsLast = !aRef.Ref2.IsRowRel();
                    ScAddress aRefPos = aAbs.aStart;
                    SCROW nRefRowSize = aAbs.aEnd.Row() - aAbs.aStart.Row() + 1;
                    SCROW nArrayLength = nRefRowSize;
                    if (!bAbsLast)
                    {
                        // range end position is relative. Extend the array length.
                        SCROW nLastRefRowOffset = aAbs.aEnd.Row() - mrPos.Row();
                        SCROW nLastRefRow = mrPos.Row() + mnLen - 1 + nLastRefRowOffset;
                        SCROW nNewLength = nLastRefRow - aAbs.aStart.Row() + 1;
                        if (nNewLength > nArrayLength)
                            nArrayLength = nNewLength;
                    }

                    // Trim trailing empty rows.
                    nArrayLength = trimLength(aRefPos.Tab(), aAbs.aStart.Col(), aAbs.aEnd.Col(), aRefPos.Row(), nArrayLength);

                    aRangeList.Join(ScRange(aAbs.aStart.Col(), aRefPos.Row(), aRefPos.Tab(),
                               aAbs.aEnd.Col(), aRefPos.Row() + nArrayLength - 1, aRefPos.Tab()));
                }
                break;
            default:
                break;
            }
        }

        for (auto i = 0u; i < aRangeList.size(); ++i)
        {
            const ScRange* pRange = aRangeList[i];
            assert(pRange->aStart.Tab() == pRange->aEnd.Tab());
            for (auto nCol = pRange->aStart.Col(); nCol <= pRange->aEnd.Col(); nCol++)
            {
                if (!mrDoc.HandleRefArrayForParallelism(ScAddress(nCol, pRange->aStart.Row(), pRange->aStart.Tab()),
                                                        pRange->aEnd.Row() - pRange->aStart.Row() + 1))
                    return false;
            }
        }
        return true;
    }
};

bool ScFormulaCell::InterpretFormulaGroup()
{
    if (!mxGroup || !pCode)
        return false;

    auto aScope = sc::FormulaLogger::get().enterGroup(*pDocument, *this);

    if (pDocument->GetRecursionHelper().GetRecursionCount())
    {
        // Do not attempt to interpret a group when calculations are already
        // running, otherwise we may run into a circular reference hell. See
        // tdf#95748
        aScope.addMessage("group calc disabled during recursive calculation.");
        return false;
    }

    if (mxGroup->meCalcState == sc::GroupCalcDisabled)
    {
        aScope.addMessage("group calc disabled");
        return false;
    }

    static const bool bThreadingRequested = std::getenv("CPU_THREADED_CALCULATION");

    // To temporarilu use threading for sc unit tests regardless of the size of the formula group,
    // add the condition !std::getenv("LO_TESTNAME") below (with &&), and run with
    // CPU_THREADED_CALCULATION=yes
    if (GetWeight() < ScInterpreter::GetGlobalConfig().mnOpenCLMinimumFormulaGroupSize)
    {
        mxGroup->meCalcState = sc::GroupCalcDisabled;
        aScope.addGroupSizeThresholdMessage(*this);
        return false;
    }

    if (cMatrixFlag != ScMatrixMode::NONE)
    {
        mxGroup->meCalcState = sc::GroupCalcDisabled;
        aScope.addMessage("matrix skipped");
        return false;
    }

    if (!ScCalcConfig::isOpenCLEnabled() && bThreadingRequested)
    {
        // iterate over code in the formula ...
        // ensure all input is pre-calculated -
        // to avoid writing during the calculation
        ScDependantsCalculator aCalculator(*pDocument, *pCode, *this, mxGroup->mpTopCell->aPos);

        // Disable or hugely enlarge subset for S/W group
        // threading interpreter

        if (!aCalculator.DoIt())
        {
            mxGroup->meCalcState = sc::GroupCalcDisabled;
            aScope.addMessage("could not do new dependencies calculation thing");
            return false;
        }

        // Then do the threaded calculation

        class Executor : public comphelper::ThreadTask
        {
        private:
            const unsigned mnThisThread;
            const unsigned mnThreadsTotal;
            ScDocument* mpDocument;
            SvNumberFormatter* mpFormatter;
            const ScAddress& mrTopPos;
            SCROW mnLength;

        public:
            Executor(std::shared_ptr<comphelper::ThreadTaskTag>& rTag,
                     unsigned nThisThread,
                     unsigned nThreadsTotal,
                     ScDocument* pDocument,
                     SvNumberFormatter* pFormatter,
                     const ScAddress& rTopPos,
                     SCROW nLength) :
                comphelper::ThreadTask(rTag),
                mnThisThread(nThisThread),
                mnThreadsTotal(nThreadsTotal),
                mpDocument(pDocument),
                mpFormatter(pFormatter),
                mrTopPos(rTopPos),
                mnLength(nLength)
            {
            }

            virtual void doWork() override
            {
                std::unique_ptr<SvNumberFormatter> pFormatterForThisThread
                    (new SvNumberFormatter(mpFormatter->GetComponentContext(),
                                           mpFormatter->GetLanguage()));
                ScInterpreterContext aContext(*mpDocument, pFormatterForThisThread.get());

                mpDocument->CalculateInColumnInThread(aContext, mrTopPos, mnLength, mnThisThread, mnThreadsTotal).MergeBackIntoNonThreadedData(mpDocument->maNonThreaded);
            }

        };

        SvNumberFormatter* pNonThreadedFormatter = pDocument->GetNonThreadedContext().GetFormatTable();

        comphelper::ThreadPool& rThreadPool(comphelper::ThreadPool::getSharedOptimalPool());
        sal_Int32 nThreadCount = rThreadPool.getWorkerCount();

        SAL_INFO("sc.threaded", "Running " << nThreadCount << " threads");

        {
            assert(!pDocument->mbThreadedGroupCalcInProgress);
            pDocument->mbThreadedGroupCalcInProgress = true;

            ScMutationGuard aGuard(pDocument, ScMutationGuardFlags::CORE);

            // Start nThreadCount new threads
            std::shared_ptr<comphelper::ThreadTaskTag> aTag = comphelper::ThreadPool::createThreadTaskTag();
            for (int i = 0; i < nThreadCount; ++i)
            {
                rThreadPool.pushTask(new Executor(aTag, i, nThreadCount, pDocument, pNonThreadedFormatter, mxGroup->mpTopCell->aPos, mxGroup->mnLength));
            }

            SAL_INFO("sc.threaded", "Joining threads");
            rThreadPool.waitUntilDone(aTag);

            pDocument->mbThreadedGroupCalcInProgress = false;

            SAL_INFO("sc.threaded", "Done");
        }

        pDocument->HandleStuffAfterParallelCalculation(mxGroup->mpTopCell->aPos, mxGroup->mnLength);

        return true;
    }

    switch (pCode->GetVectorState())
    {
        case FormulaVectorEnabled:
        case FormulaVectorCheckReference:
            // Good.
        break;

        // Not good.
        case FormulaVectorDisabledByOpCode:
            aScope.addMessage("group calc disabled due to vector state (non-vector-supporting opcode)");
            return false;
        case FormulaVectorDisabledNotInSoftwareSubset:
            aScope.addMessage("group calc disabled due to vector state (opcode not in software subset)");
            return false;
        case FormulaVectorDisabledByStackVariable:
            aScope.addMessage("group calc disabled due to vector state (non-vector-supporting stack variable)");
            return false;
        case FormulaVectorDisabledNotInSubSet:
            aScope.addMessage("group calc disabled due to vector state (opcode not in subset)");
            return false;
        case FormulaVectorDisabled:
        case FormulaVectorUnknown:
        default:
            aScope.addMessage("group calc disabled due to vector state (unknown)");
            return false;
    }

    if (!ScCalcConfig::isOpenCLEnabled() && !ScCalcConfig::isSwInterpreterEnabled())
    {
        aScope.addMessage("opencl not enabled and sw interpreter not enabled");
        return false;
    }

    // Guard against endless recursion of Interpret() calls, for this to work
    // ScFormulaCell::InterpretFormulaGroup() must never be called through
    // anything else than ScFormulaCell::Interpret(), same as
    // ScFormulaCell::InterpretTail()
    RecursionCounter aRecursionCounter( pDocument->GetRecursionHelper(), this);

    // TODO : Disable invariant formula group interpretation for now in order
    // to get implicit intersection to work.
    if (mxGroup->mbInvariant && false)
        return InterpretInvariantFormulaGroup();

    int nMaxGroupLength = INT_MAX;

#ifdef _WIN32
    // Heuristic: Certain old low-end OpenCL implementations don't
    // work for us with too large group lengths. 1000 was determined
    // empirically to be a good compromise.
    if (openclwrapper::gpuEnv.mbNeedsTDRAvoidance)
        nMaxGroupLength = 1000;
#endif

    if (std::getenv("SC_MAX_GROUP_LENGTH"))
        nMaxGroupLength = std::atoi(std::getenv("SC_MAX_GROUP_LENGTH"));

    int nNumOnePlus;
    const int nNumParts = splitup(GetSharedLength(), nMaxGroupLength, nNumOnePlus);

    int nOffset = 0;
    int nCurChunkSize;
    ScAddress aOrigPos = mxGroup->mpTopCell->aPos;
    for (int i = 0; i < nNumParts; i++, nOffset += nCurChunkSize)
    {
        nCurChunkSize = GetSharedLength()/nNumParts + (i < nNumOnePlus ? 1 : 0);

        ScFormulaCellGroupRef xGroup;

        if (nNumParts == 1)
            xGroup = mxGroup;
        else
        {
            // Ugly hack
            xGroup = new ScFormulaCellGroup();
            xGroup->mpTopCell = mxGroup->mpTopCell;
            xGroup->mpTopCell->aPos = aOrigPos;
            xGroup->mpTopCell->aPos.IncRow(nOffset);
            xGroup->mbInvariant = mxGroup->mbInvariant;
            xGroup->mnLength = nCurChunkSize;
            xGroup->mpCode = mxGroup->mpCode;
        }

        ScTokenArray aCode;
        ScGroupTokenConverter aConverter(aCode, *pDocument, *this, xGroup->mpTopCell->aPos);
        if (!aConverter.convert(*pCode, aScope))
        {
            SAL_INFO("sc.opencl", "conversion of group " << this << " failed, disabling");
            mxGroup->meCalcState = sc::GroupCalcDisabled;

            // Undo the hack above
            if (nNumParts > 1)
            {
                mxGroup->mpTopCell->aPos = aOrigPos;
                xGroup->mpTopCell = nullptr;
                xGroup->mpCode = nullptr;
            }

            aScope.addMessage("group token conversion failed");
            return false;
        }

        // The converted code does not have RPN tokens yet.  The interpreter will
        // generate them.
        xGroup->meCalcState = mxGroup->meCalcState = sc::GroupCalcRunning;
        sc::FormulaGroupInterpreter *pInterpreter = sc::FormulaGroupInterpreter::getStatic();

        if (pInterpreter == nullptr ||
            !pInterpreter->interpret(*pDocument, xGroup->mpTopCell->aPos, xGroup, aCode))
        {
            SAL_INFO("sc.opencl", "interpreting group " << mxGroup << " (state " << (int) mxGroup->meCalcState << ") failed, disabling");
            mxGroup->meCalcState = sc::GroupCalcDisabled;

            // Undo the hack above
            if (nNumParts > 1)
            {
                mxGroup->mpTopCell->aPos = aOrigPos;
                xGroup->mpTopCell = nullptr;
                xGroup->mpCode = nullptr;
            }

            aScope.addMessage("group interpretation unsuccessful");
            return false;
        }

        aScope.setCalcComplete();

        if (nNumParts > 1)
        {
            xGroup->mpTopCell = nullptr;
            xGroup->mpCode = nullptr;
        }
    }

    if (nNumParts > 1)
        mxGroup->mpTopCell->aPos = aOrigPos;
    mxGroup->meCalcState = sc::GroupCalcEnabled;
    return true;
}

bool ScFormulaCell::InterpretInvariantFormulaGroup()
{
    if (pCode->GetVectorState() == FormulaVectorCheckReference)
    {
        // An invariant group should only have absolute row references, and no
        // external references are allowed.

        ScTokenArray aCode;
        FormulaTokenArrayPlainIterator aIter(*pCode);
        for (const formula::FormulaToken* p = aIter.First(); p; p = aIter.Next())
        {
            switch (p->GetType())
            {
                case svSingleRef:
                {
                    ScSingleRefData aRef = *p->GetSingleRef();
                    ScAddress aRefPos = aRef.toAbs(aPos);
                    formula::FormulaTokenRef pNewToken = pDocument->ResolveStaticReference(aRefPos);
                    if (!pNewToken)
                        return false;

                    aCode.AddToken(*pNewToken);
                }
                break;
                case svDoubleRef:
                {
                    ScComplexRefData aRef = *p->GetDoubleRef();
                    ScRange aRefRange = aRef.toAbs(aPos);
                    formula::FormulaTokenRef pNewToken = pDocument->ResolveStaticReference(aRefRange);
                    if (!pNewToken)
                        return false;

                    aCode.AddToken(*pNewToken);
                }
                break;
                default:
                    aCode.AddToken(*p);
            }
        }

        ScCompiler aComp(pDocument, aPos, aCode, pDocument->GetGrammar());
        aComp.CompileTokenArray(); // Create RPN token array.
        ScInterpreter aInterpreter(this, pDocument, pDocument->GetNonThreadedContext(), aPos, aCode);
        aInterpreter.Interpret();
        aResult.SetToken(aInterpreter.GetResultToken().get());
    }
    else
    {
        // Formula contains no references.
        ScInterpreter aInterpreter(this, pDocument, pDocument->GetNonThreadedContext(), aPos, *pCode);
        aInterpreter.Interpret();
        aResult.SetToken(aInterpreter.GetResultToken().get());
    }

    for ( sal_Int32 i = 0; i < mxGroup->mnLength; i++ )
    {
        ScAddress aTmpPos = aPos;
        aTmpPos.SetRow(mxGroup->mpTopCell->aPos.Row() + i);
        ScFormulaCell* pCell = pDocument->GetFormulaCell(aTmpPos);
        if (!pCell)
        {
            SAL_WARN("sc.core.formulacell", "GetFormulaCell not found");
            continue;
        }

        // FIXME: this set of horrors is unclear to me ... certainly
        // the above GetCell is profoundly nasty & slow ...
        // Ensure the cell truly has a result:
        pCell->aResult = aResult;
        pCell->ResetDirty();
        pCell->SetChanged(true);
    }

    return true;
}

namespace {

void startListeningArea(
    ScFormulaCell* pCell, ScDocument& rDoc, const ScAddress& rPos, const formula::FormulaToken& rToken)
{
    const ScSingleRefData& rRef1 = *rToken.GetSingleRef();
    const ScSingleRefData& rRef2 = *rToken.GetSingleRef2();
    ScAddress aCell1 = rRef1.toAbs(rPos);
    ScAddress aCell2 = rRef2.toAbs(rPos);
    if (aCell1.IsValid() && aCell2.IsValid())
    {
        if (rToken.GetOpCode() == ocColRowNameAuto)
        {   // automagically
            if ( rRef1.IsColRel() )
            {   // ColName
                aCell2.SetRow(MAXROW);
            }
            else
            {   // RowName
                aCell2.SetCol(MAXCOL);
            }
        }
        rDoc.StartListeningArea(ScRange(aCell1, aCell2), false, pCell);
    }
}

}

void ScFormulaCell::StartListeningTo( ScDocument* pDoc )
{
    if (mxGroup)
        mxGroup->endAllGroupListening(*pDoc);

    if (pDoc->IsClipOrUndo() || pDoc->GetNoListening() || IsInChangeTrack())
        return;

    pDoc->SetDetectiveDirty(true);  // It has changed something

    ScTokenArray* pArr = GetCode();
    if( pArr->IsRecalcModeAlways() )
    {
        pDoc->StartListeningArea(BCA_LISTEN_ALWAYS, false, this);
        SetNeedsListening( false);
        return;
    }

    formula::FormulaTokenArrayPlainIterator aIter(*pArr);
    formula::FormulaToken* t;
    while ( ( t = aIter.GetNextReferenceRPN() ) != nullptr )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell =  t->GetSingleRef()->toAbs(aPos);
                if (aCell.IsValid())
                    pDoc->StartListeningCell(aCell, this);
            }
            break;
            case svDoubleRef:
                startListeningArea(this, *pDoc, aPos, *t);
            break;
            default:
                ;   // nothing
        }
    }
    SetNeedsListening( false);
}

void ScFormulaCell::StartListeningTo( sc::StartListeningContext& rCxt )
{
    ScDocument& rDoc = rCxt.getDoc();

    if (mxGroup)
        mxGroup->endAllGroupListening(rDoc);

    if (rDoc.IsClipOrUndo() || rDoc.GetNoListening() || IsInChangeTrack())
        return;

    rDoc.SetDetectiveDirty(true);  // It has changed something

    ScTokenArray* pArr = GetCode();
    if( pArr->IsRecalcModeAlways() )
    {
        rDoc.StartListeningArea(BCA_LISTEN_ALWAYS, false, this);
        SetNeedsListening( false);
        return;
    }

    formula::FormulaTokenArrayPlainIterator aIter(*pArr);
    formula::FormulaToken* t;
    while ( ( t = aIter.GetNextReferenceRPN() ) != nullptr )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell = t->GetSingleRef()->toAbs(aPos);
                if (aCell.IsValid())
                    rDoc.StartListeningCell(rCxt, aCell, *this);
            }
            break;
            case svDoubleRef:
                startListeningArea(this, rDoc, aPos, *t);
            break;
            default:
                ;   // nothing
        }
    }
    SetNeedsListening( false);
}

namespace {

void endListeningArea(
    ScFormulaCell* pCell, ScDocument& rDoc, const ScAddress& rPos, const formula::FormulaToken& rToken)
{
    const ScSingleRefData& rRef1 = *rToken.GetSingleRef();
    const ScSingleRefData& rRef2 = *rToken.GetSingleRef2();
    ScAddress aCell1 = rRef1.toAbs(rPos);
    ScAddress aCell2 = rRef2.toAbs(rPos);
    if (aCell1.IsValid() && aCell2.IsValid())
    {
        if (rToken.GetOpCode() == ocColRowNameAuto)
        {   // automagically
            if ( rRef1.IsColRel() )
            {   // ColName
                aCell2.SetRow(MAXROW);
            }
            else
            {   // RowName
                aCell2.SetCol(MAXCOL);
            }
        }

        rDoc.EndListeningArea(ScRange(aCell1, aCell2), false, pCell);
    }
}

}

void ScFormulaCell::EndListeningTo( ScDocument* pDoc, ScTokenArray* pArr,
        ScAddress aCellPos )
{
    if (mxGroup)
        mxGroup->endAllGroupListening(*pDoc);

    if (pDoc->IsClipOrUndo() || IsInChangeTrack())
        return;

    if (!HasBroadcaster())
        return;

    pDoc->SetDetectiveDirty(true);  // It has changed something

    if ( GetCode()->IsRecalcModeAlways() )
    {
        pDoc->EndListeningArea(BCA_LISTEN_ALWAYS, false, this);
        return;
    }

    if (!pArr)
    {
        pArr = GetCode();
        aCellPos = aPos;
    }
    formula::FormulaTokenArrayPlainIterator aIter(*pArr);
    formula::FormulaToken* t;
    while ( ( t = aIter.GetNextReferenceRPN() ) != nullptr )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell = t->GetSingleRef()->toAbs(aCellPos);
                if (aCell.IsValid())
                    pDoc->EndListeningCell(aCell, this);
            }
            break;
            case svDoubleRef:
                endListeningArea(this, *pDoc, aCellPos, *t);
            break;
            default:
                ;   // nothing
        }
    }
}

void ScFormulaCell::EndListeningTo( sc::EndListeningContext& rCxt )
{
    if (mxGroup)
        mxGroup->endAllGroupListening(rCxt.getDoc());

    if (rCxt.getDoc().IsClipOrUndo() || IsInChangeTrack())
        return;

    if (!HasBroadcaster())
        return;

    ScDocument& rDoc = rCxt.getDoc();
    rDoc.SetDetectiveDirty(true);  // It has changed something

    ScTokenArray* pArr = rCxt.getOldCode();
    ScAddress aCellPos = rCxt.getOldPosition(aPos);
    if (!pArr)
        pArr = pCode;

    if (pArr->IsRecalcModeAlways())
    {
        rDoc.EndListeningArea(BCA_LISTEN_ALWAYS, false, this);
        return;
    }

    formula::FormulaTokenArrayPlainIterator aIter(*pArr);
    formula::FormulaToken* t;
    while ( ( t = aIter.GetNextReferenceRPN() ) != nullptr )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell = t->GetSingleRef()->toAbs(aCellPos);
                if (aCell.IsValid())
                    rDoc.EndListeningCell(rCxt, aCell, *this);
            }
            break;
            case svDoubleRef:
                endListeningArea(this, rDoc, aCellPos, *t);
            break;
            default:
                ;   // nothing
        }
    }
}

bool ScFormulaCell::IsShared() const
{
    return mxGroup.get() != nullptr;
}

bool ScFormulaCell::IsSharedTop() const
{
    if (!mxGroup)
        return false;

    return mxGroup->mpTopCell == this;
}

SCROW ScFormulaCell::GetSharedTopRow() const
{
    return mxGroup ? mxGroup->mpTopCell->aPos.Row() : -1;
}

SCROW ScFormulaCell::GetSharedLength() const
{
    return mxGroup ? mxGroup->mnLength : 0;
}

sal_Int32 ScFormulaCell::GetWeight() const
{
    if (!mxGroup)
        return 1;
    double nSharedCodeWeight = GetSharedCode()->GetWeight();
    double nResult = nSharedCodeWeight * GetSharedLength();
    if (nResult < SAL_MAX_INT32)
        return nResult;
    else
        return SAL_MAX_INT32;
}

ScTokenArray* ScFormulaCell::GetSharedCode()
{
    return mxGroup ? mxGroup->mpCode : nullptr;
}

const ScTokenArray* ScFormulaCell::GetSharedCode() const
{
    return mxGroup ? mxGroup->mpCode : nullptr;
}

void ScFormulaCell::SyncSharedCode()
{
    if (!mxGroup)
        // Not a shared formula cell.
        return;

    pCode = mxGroup->mpCode;
}

#if DUMP_COLUMN_STORAGE

void ScFormulaCell::Dump() const
{
    cout << "-- formula cell (" << aPos.Format(ScRefFlags::VALID | ScRefFlags::TAB_3D, pDocument) << ")" << endl;
    cout << "  * shared: " << (mxGroup ? "true" : "false") << endl;
    if (mxGroup)
    {
        cout << "    * shared length: " << mxGroup->mnLength << endl;
        cout << "    * shared calc state: " << mxGroup->meCalcState << endl;
    }

    sc::TokenStringContext aCxt(pDocument, pDocument->GetGrammar());
    cout << "  * code: " << pCode->CreateString(aCxt, aPos) << endl;

    FormulaError nErrCode = pCode->GetCodeError();
    cout << "  * code error: ";
    if (nErrCode == FormulaError::NONE)
        cout << "(none)";
    else
    {
        OUString aStr = ScGlobal::GetErrorString(nErrCode);
        cout << "  * code error: " << aStr << " (" << int(nErrCode) << ")";
    }
    cout << endl;

    cout << "  * result: ";
    sc::FormulaResultValue aRV = aResult.GetResult();
    switch (aRV.meType)
    {
        case sc::FormulaResultValue::Value:
            cout << aRV.mfValue << " (value)";
            break;
        case sc::FormulaResultValue::String:
            cout << aRV.maString.getString() << " (string)";
            break;
        case sc::FormulaResultValue::Error:
            cout << ScGlobal::GetErrorString(aRV.mnError) << " (error: " << int(aRV.mnError) << ")";
            break;
        case sc::FormulaResultValue::Invalid:
            cout << "(invalid)";
            break;
        default:
            ;
    }
    cout << endl;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
