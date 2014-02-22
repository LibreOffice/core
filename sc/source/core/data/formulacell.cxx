/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "formulacell.hxx"
#include "grouptokenconverter.hxx"

#include "compiler.hxx"
#include "document.hxx"
#include "globalnames.hxx"
#include "cellvalue.hxx"
#include "interpre.hxx"
#include "macromgr.hxx"
#include "refupdat.hxx"
#include "recursionhelper.hxx"
#include "docoptio.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "progress.hxx"
#include "scmatrix.hxx"
#include "rechead.hxx"
#include "scitems.hxx"
#include "validat.hxx"
#include "editutil.hxx"
#include "chgtrack.hxx"
#include "tokenarray.hxx"
#include "clkernelthread.hxx"

#include "formula/errorcodes.hxx"
#include "formula/vectortoken.hxx"
#include "svl/intitem.hxx"
#include "rtl/strbuf.hxx"
#include "formulagroup.hxx"
#include "listenercontext.hxx"
#include "types.hxx"
#include "scopetools.hxx"
#include "refupdatecontext.hxx"
#include <tokenstringcontext.hxx>

#include <boost/scoped_ptr.hpp>

using namespace formula;

#ifdef USE_MEMPOOL
IMPL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

namespace {








const sal_uInt16 MAXRECURSION = 400;

using std::deque;

typedef SCCOLROW(*DimensionSelector)(const ScAddress&, const ScSingleRefData&);


static SCCOLROW lcl_GetCol(const ScAddress& rPos, const ScSingleRefData& rData)
{
    return rData.toAbs(rPos).Col();
}


static SCCOLROW lcl_GetRow(const ScAddress& rPos, const ScSingleRefData& rData)
{
    return rData.toAbs(rPos).Row();
}


static SCCOLROW lcl_GetTab(const ScAddress& rPos, const ScSingleRefData& rData)
{
    return rData.toAbs(rPos).Tab();
}


/** Check if both references span the same range in selected dimension.
 */
static bool
lcl_checkRangeDimension(
    const ScAddress& rPos, const SingleDoubleRefProvider& rRef1, const SingleDoubleRefProvider& rRef2,
    const DimensionSelector aWhich)
{
    return aWhich(rPos, rRef1.Ref1) == aWhich(rPos, rRef2.Ref1) &&
        aWhich(rPos, rRef1.Ref2) == aWhich(rPos, rRef2.Ref2);
}


static bool
lcl_checkRangeDimensions(
    const ScAddress& rPos, const SingleDoubleRefProvider& rRef1, const SingleDoubleRefProvider& rRef2,
    bool& bCol, bool& bRow, bool& bTab)
{
    const bool bSameCols(lcl_checkRangeDimension(rPos, rRef1, rRef2, lcl_GetCol));
    const bool bSameRows(lcl_checkRangeDimension(rPos, rRef1, rRef2, lcl_GetRow));
    const bool bSameTabs(lcl_checkRangeDimension(rPos, rRef1, rRef2, lcl_GetTab));

    
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
static bool
lcl_checkRangeDimensions(
    const ScAddress& rPos,
    const deque<ScToken*>::const_iterator aBegin,
    const deque<ScToken*>::const_iterator aEnd,
    bool& bCol, bool& bRow, bool& bTab)
{
    deque<ScToken*>::const_iterator aCur(aBegin);
    ++aCur;
    const SingleDoubleRefProvider aRef(**aBegin);
    bool bOk(false);
    {
        const SingleDoubleRefProvider aRefCur(**aCur);
        bOk = lcl_checkRangeDimensions(rPos, aRef, aRefCur, bCol, bRow, bTab);
    }
    while (bOk && aCur != aEnd)
    {
        const SingleDoubleRefProvider aRefCur(**aCur);
        bool bColTmp(false);
        bool bRowTmp(false);
        bool bTabTmp(false);
        bOk = lcl_checkRangeDimensions(rPos, aRef, aRefCur, bColTmp, bRowTmp, bTabTmp);
        bOk = bOk && (bCol == bColTmp && bRow == bRowTmp && bTab == bTabTmp);
        ++aCur;
    }

    if (bOk && aCur == aEnd)
    {
        return true;
    }
    return false;
}

class LessByReference : std::binary_function<const ScToken*, const ScToken*, bool>
{
    ScAddress maPos;
    DimensionSelector maFunc;
public:
    LessByReference(const ScAddress& rPos, const DimensionSelector& rFunc) :
        maPos(rPos), maFunc(rFunc) {}

    bool operator() (const ScToken* pRef1, const ScToken* pRef2)
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
class AdjacentByReference : std::binary_function<const ScToken*, const ScToken*, bool>
{
    ScAddress maPos;
    DimensionSelector maFunc;
public:
    AdjacentByReference(const ScAddress& rPos, DimensionSelector aFunc) :
        maPos(rPos), maFunc(aFunc) {}

    bool operator() (const ScToken* p1, const ScToken* p2)
    {
        const SingleDoubleRefProvider aRef1(*p1);
        const SingleDoubleRefProvider aRef2(*p2);
        return maFunc(maPos, aRef2.Ref1) - maFunc(maPos, aRef1.Ref2) == 1;
    }
};

static bool
lcl_checkIfAdjacent(
    const ScAddress& rPos, const deque<ScToken*>& rReferences, const DimensionSelector aWhich)
{
    typedef deque<ScToken*>::const_iterator Iter;
    Iter aBegin(rReferences.begin());
    Iter aEnd(rReferences.end());
    Iter aBegin1(aBegin);
    ++aBegin1, --aEnd;
    return std::equal(aBegin, aEnd, aBegin1, AdjacentByReference(rPos, aWhich));
}


static void
lcl_fillRangeFromRefList(
    const ScAddress& aPos, const deque<ScToken*>& rReferences, ScRange& rRange)
{
    const ScSingleRefData aStart(
            SingleDoubleRefProvider(*rReferences.front()).Ref1);
    rRange.aStart = aStart.toAbs(aPos);
    const ScSingleRefData aEnd(
            SingleDoubleRefProvider(*rReferences.back()).Ref2);
    rRange.aEnd = aEnd.toAbs(aPos);
}


static bool
lcl_refListFormsOneRange(
        const ScAddress& rPos, deque<ScToken*>& rReferences,
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
            aWhich = lcl_GetRow;    
        }

        
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

void adjustRangeName(ScToken* pToken, ScDocument& rNewDoc, const ScDocument* pOldDoc, const ScAddress& aNewPos, const ScAddress& aOldPos)
{
    bool bOldGlobal = pToken->IsGlobal();
    SCTAB aOldTab = aOldPos.Tab();
    OUString aRangeName;
    int nOldIndex = pToken->GetIndex();
    ScRangeData* pOldRangeData = NULL;

    
    if (!bOldGlobal)
    {
        pOldRangeData = pOldDoc->GetRangeName(aOldTab)->findByIndex(nOldIndex);
        if (!pOldRangeData)
            return;     
        aRangeName = pOldRangeData->GetUpperName();
    }
    else
    {
        pOldRangeData = pOldDoc->GetRangeName()->findByIndex(nOldIndex);
        if (!pOldRangeData)
            return;     
        aRangeName = pOldRangeData->GetUpperName();
    }

    
    
    SCTAB aNewTab = aNewPos.Tab();
    ScRangeName* pRangeName = rNewDoc.GetRangeName(aNewTab);
    ScRangeData* pRangeData = NULL;
    bool bNewGlobal = false;
    
    if (pRangeName)
    {
        pRangeData = pRangeName->findByUpperName(aRangeName);
    }
    
    if (!pRangeData)
    {
        bNewGlobal = true;
        pRangeName = rNewDoc.GetRangeName();
        if (pRangeName)
            pRangeData = pRangeName->findByUpperName(aRangeName);
    }
    
    if (!pRangeData)
    {
        bNewGlobal = bOldGlobal;
        pRangeData = new ScRangeData(*pOldRangeData, &rNewDoc);
        ScTokenArray* pRangeNameToken = pRangeData->GetCode();
        if (rNewDoc.GetPool() != const_cast<ScDocument*>(pOldDoc)->GetPool())
        {
            pRangeNameToken->ReadjustAbsolute3DReferences(pOldDoc, &rNewDoc, pRangeData->GetPos(), true);
            pRangeNameToken->AdjustAbsoluteRefs(pOldDoc, aOldPos, aNewPos, false, true);
        }

        bool bInserted;
        if (bNewGlobal)
            bInserted = rNewDoc.GetRangeName()->insert(pRangeData);
        else
            bInserted = rNewDoc.GetRangeName(aNewTab)->insert(pRangeData);
        if (!bInserted)
        {
            
            pRangeData = NULL;
            pToken->SetIndex(0);
            OSL_FAIL("inserting the range name should not fail");
            return;
        }
    }
    sal_Int32 nIndex = pRangeData->GetIndex();
    pToken->SetIndex(nIndex);
    pToken->SetGlobal(bNewGlobal);
}

void adjustDBRange(ScToken* pToken, ScDocument& rNewDoc, const ScDocument* pOldDoc)
{
    ScDBCollection* pOldDBCollection = pOldDoc->GetDBCollection();
    if (!pOldDBCollection)
        return;
    ScDBCollection::NamedDBs& aOldNamedDBs = pOldDBCollection->getNamedDBs();
    ScDBData* pDBData = aOldNamedDBs.findByIndex(pToken->GetIndex());
    if (!pDBData)
        return; 
    OUString aDBName = pDBData->GetUpperName();

    
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
        aNewNamedDBs.insert(pNewDBData);
    }
    pToken->SetIndex(pNewDBData->GetIndex());
}

}


static osl::Mutex& getOpenCLCompilationThreadMutex()
{
    static osl::Mutex* pMutex = NULL;
    if( !pMutex )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            static osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }

    return *pMutex;
}

int ScFormulaCellGroup::snCount = 0;
rtl::Reference<sc::CLBuildKernelThread> ScFormulaCellGroup::sxCompilationThread;

ScFormulaCellGroup::ScFormulaCellGroup() :
    mnRefCount(0),
    mpCode(NULL),
    mpCompiledFormula(NULL),
    mpTopCell(NULL),
    mnLength(0),
    mnFormatType(NUMBERFORMAT_NUMBER),
    mbInvariant(false),
    mbSubTotal(false),
    meCalcState(sc::GroupCalcEnabled)
{
    if (ScInterpreter::GetGlobalConfig().mbOpenCLEnabled)
    {
        osl::MutexGuard aGuard(getOpenCLCompilationThreadMutex());
        if (snCount++ == 0)
        {
            assert(!sxCompilationThread.is());
            sxCompilationThread.set(new sc::CLBuildKernelThread);
            sxCompilationThread->launch();
        }
    }
}

ScFormulaCellGroup::~ScFormulaCellGroup()
{
    if (ScInterpreter::GetGlobalConfig().mbOpenCLEnabled)
    {
        osl::MutexGuard aGuard(getOpenCLCompilationThreadMutex());
        if (--snCount == 0 && sxCompilationThread.is())
            {
                assert(sxCompilationThread.is());
                sxCompilationThread->finish();
                sxCompilationThread->join();
                SAL_INFO("sc.opencl", "OpenCL kernel compilation thread has finished");
                sxCompilationThread.clear();
            }
    }
    delete mpCode;
}

void ScFormulaCellGroup::scheduleCompilation()
{
    meCalcState = sc::GroupCalcOpenCLKernelCompilationScheduled;
    sc::CLBuildKernelWorkItem aWorkItem;
    aWorkItem.meWhatToDo = sc::CLBuildKernelWorkItem::COMPILE;
    aWorkItem.mxGroup = this;
    sxCompilationThread->push(aWorkItem);
}

void ScFormulaCellGroup::setCode( const ScTokenArray& rCode )
{
    delete mpCode;
    mpCode = rCode.Clone();
    mbInvariant = mpCode->IsInvariant();
    mpCode->GenHash();
}

void ScFormulaCellGroup::compileCode(
    ScDocument& rDoc, const ScAddress& rPos, FormulaGrammar::Grammar eGram )
{
    if (!mpCode)
        return;

    if (mpCode->GetLen() && !mpCode->GetCodeError() && !mpCode->GetCodeLen())
    {
        ScCompiler aComp(&rDoc, rPos, *mpCode);
        aComp.SetGrammar(eGram);
        mbSubTotal = aComp.CompileTokenArray();
        mnFormatType = aComp.GetNumFormatType();
    }
    else
    {
        mpCode->Reset();
        mbSubTotal = mpCode->GetNextOpCodeRPN(ocSubTotal) != NULL;
    }
}



ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos ) :
    eTempGrammar(formula::FormulaGrammar::GRAM_DEFAULT),
    pCode(new ScTokenArray),
    pDocument(pDoc),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag(MM_NONE),
    nFormatType(NUMBERFORMAT_NUMBER),
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
    mbPostponedDirty(false),
    aPos(rPos)
{
}

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const OUString& rFormula,
                              const FormulaGrammar::Grammar eGrammar,
                              sal_uInt8 cMatInd ) :
    eTempGrammar( eGrammar),
    pCode( NULL ),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag ( cMatInd ),
    nFormatType ( NUMBERFORMAT_NUMBER ),
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
    mbPostponedDirty(false),
    aPos( rPos )
{
    Compile( rFormula, true, eGrammar );    
    if (!pCode)
        
        pCode = new ScTokenArray;
}

ScFormulaCell::ScFormulaCell(
    ScDocument* pDoc, const ScAddress& rPos, ScTokenArray* pArray,
    const FormulaGrammar::Grammar eGrammar, sal_uInt8 cMatInd ) :
    eTempGrammar( eGrammar),
    pCode(pArray),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag ( cMatInd ),
    nFormatType ( NUMBERFORMAT_NUMBER ),
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
    mbPostponedDirty(false),
    aPos( rPos )
{
    assert(pArray); 

    
    if (pCode->GetLen() && !pCode->GetCodeError() && !pCode->GetCodeLen())
    {
        ScCompiler aComp( pDocument, aPos, *pCode);
        aComp.SetGrammar(eTempGrammar);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
    }
    else
    {
        pCode->Reset();
        if (pCode->GetNextOpCodeRPN(ocSubTotal))
            bSubTotal = true;
    }

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);

    pCode->GenHash();
}

ScFormulaCell::ScFormulaCell(
    ScDocument* pDoc, const ScAddress& rPos, const ScTokenArray& rArray,
    const FormulaGrammar::Grammar eGrammar, sal_uInt8 cMatInd ) :
    eTempGrammar( eGrammar),
    pCode(new ScTokenArray(rArray)),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag ( cMatInd ),
    nFormatType ( NUMBERFORMAT_NUMBER ),
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
    mbPostponedDirty(false),
    aPos( rPos )
{
    
    if( pCode->GetLen() && !pCode->GetCodeError() && !pCode->GetCodeLen() )
    {
        ScCompiler aComp( pDocument, aPos, *pCode);
        aComp.SetGrammar(eTempGrammar);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
    }
    else
    {
        pCode->Reset();
        if ( pCode->GetNextOpCodeRPN( ocSubTotal ) )
            bSubTotal = true;
    }

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);

    pCode->GenHash();
}

ScFormulaCell::ScFormulaCell(
    ScDocument* pDoc, const ScAddress& rPos, const ScFormulaCellGroupRef& xGroup,
    const FormulaGrammar::Grammar eGrammar, sal_uInt8 cInd ) :
    mxGroup(xGroup),
    eTempGrammar( eGrammar),
    pCode(xGroup->mpCode ? xGroup->mpCode : new ScTokenArray),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag ( cInd ),
    nFormatType(xGroup->mnFormatType),
    bDirty(false),
    bChanged( false ),
    bRunning( false ),
    bCompile( false ),
    bSubTotal(xGroup->mbSubTotal),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    mbPostponedDirty(false),
    aPos( rPos )
{
    if (bSubTotal)
        pDocument->AddSubTotalCell(this);
}

ScFormulaCell::ScFormulaCell( const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, int nCloneFlags ) :
    SvtListener(),
    aResult( rCell.aResult ),
    eTempGrammar( rCell.eTempGrammar),
    pDocument( &rDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag ( rCell.cMatrixFlag ),
    nFormatType( rCell.nFormatType ),
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
    mbPostponedDirty(false),
    aPos( rPos )
{
    pCode = rCell.pCode->Clone();

    
    
    
    if ( pCode->GetCodeError() && !pDocument->IsClipboard() && pCode->GetLen() )
    {
        pCode->SetCodeError( 0 );
        bCompile = true;
    }
    
    bool bCompileLater = false;
    bool bClipMode = rCell.pDocument->IsClipboard();

    
    if (!pDocument->IsClipOrUndo() || rDoc.IsUndo())
    {
        if (!pDocument->IsClipboardSource() || aPos.Tab() != rCell.aPos.Tab())
        {
            ScToken* pToken = NULL;
            while((pToken = static_cast<ScToken*>(pCode->GetNextName()))!= NULL)
            {
                OpCode eOpCode = pToken->GetOpCode();
                if (eOpCode == ocName)
                    adjustRangeName(pToken, rDoc, rCell.pDocument, aPos, rCell.aPos);
                else if (eOpCode == ocDBArea)
                    adjustDBRange(pToken, rDoc, rCell.pDocument);
            }
        }

        bool bCopyBetweenDocs = pDocument->GetPool() != rCell.pDocument->GetPool();
        if (bCopyBetweenDocs && !(nCloneFlags & SC_CLONECELL_NOMAKEABS_EXTERNAL))
        {
            pCode->ReadjustAbsolute3DReferences( rCell.pDocument, &rDoc, rCell.aPos);
        }

        pCode->AdjustAbsoluteRefs( rCell.pDocument, rCell.aPos, aPos, false, bCopyBetweenDocs );
    }

    if ( nCloneFlags & SC_CLONECELL_ADJUST3DREL )
        pCode->ReadjustRelative3DReferences( rCell.aPos, aPos );

    if( !bCompile )
    {   
        pCode->Reset();
        ScToken* t;
        while ( ( t = static_cast<ScToken*>(pCode->GetNextReferenceOrName()) ) != NULL && !bCompile )
        {
            if ( t->IsExternalRef() )
            {
                
                bCompile = true;
            }
            else if ( t->GetType() == svIndex )
            {
                ScRangeData* pRangeData = rDoc.GetRangeName()->findByIndex( t->GetIndex() );
                if( pRangeData )
                {
                    if( pRangeData->HasReferences() )
                        bCompile = true;
                }
                else
                    bCompile = true;    
            }
            else if ( t->GetOpCode() == ocColRowName )
            {
                bCompile = true;        
                bCompileLater = bClipMode;
            }
        }
    }
    if( bCompile )
    {
        if ( !bCompileLater && bClipMode )
        {
            
            
            bCompileLater = pCode->HasOpCode( ocRange) || pCode->HasOpCode( ocColRowName);
        }
        if ( !bCompileLater )
        {
            
            
            CompileTokenArray( true );
        }
    }

    if( nCloneFlags & SC_CLONECELL_STARTLISTENING )
        StartListeningTo( &rDoc );

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);
}

ScFormulaCell::~ScFormulaCell()
{
    pDocument->RemoveFromFormulaTree( this );
    pDocument->RemoveSubTotalCell(this);
    if (pCode->HasOpCode(ocMacro))
        pDocument->GetMacroManager()->RemoveDependentCell(this);

    if (pDocument->HasExternalRefManager())
        pDocument->GetExternalRefManager()->removeRefCell(this);

    if (!mxGroup || !mxGroup->mpCode)
        
        delete pCode;
}

ScFormulaCell* ScFormulaCell::Clone() const
{
    return new ScFormulaCell(*this, *pDocument, aPos);
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
    if( pCode->GetCodeError() && !pCode->GetLen() )
    {
        rBuffer = OUStringBuffer( ScGlobal::GetErrorString( pCode->GetCodeError()));
        return;
    }
    else if( cMatrixFlag == MM_REFERENCE )
    {
        
        pCode->Reset();
        ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        if( p )
        {
            /* FIXME: original GetFormula() code obtained
             * pCell only if (!this->IsInChangeTrack()),
             * GetEnglishFormula() omitted that test.
             * Can we live without in all cases? */
            ScFormulaCell* pCell = NULL;
            ScSingleRefData& rRef = p->GetSingleRef();
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
                ScCompiler aComp( pDocument, aPos, *pCode);
                aComp.SetGrammar(eGrammar);
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
        ScCompiler aComp( pDocument, aPos, *pCode);
        aComp.SetGrammar(eGrammar);
        aComp.CreateStringFromTokenArray( rBuffer );
    }

    rBuffer.insert( 0, '=');
    if( cMatrixFlag )
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
    if (pCode->GetCodeError() && !pCode->GetLen())
    {
        aBuf = OUStringBuffer( ScGlobal::GetErrorString( pCode->GetCodeError()));
        return aBuf.makeStringAndClear();
    }
    else if( cMatrixFlag == MM_REFERENCE )
    {
        
        pCode->Reset();
        ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        if( p )
        {
            /* FIXME: original GetFormula() code obtained
             * pCell only if (!this->IsInChangeTrack()),
             * GetEnglishFormula() omitted that test.
             * Can we live without in all cases? */
            ScFormulaCell* pCell = NULL;
            ScSingleRefData& rRef = p->GetSingleRef();
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
    if( cMatrixFlag )
    {
        aBuf.insert( 0, '{');
        aBuf.append( '}');
    }

    return aBuf.makeStringAndClear();
}

void ScFormulaCell::GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows )
{
    MaybeInterpret();

    const ScMatrix* pMat = NULL;
    if (!pCode->GetCodeError() && aResult.GetType() == svMatrixCell &&
            ((pMat = static_cast<const ScToken*>(aResult.GetToken().get())->GetMatrix()) != 0))
        pMat->GetDimensions( rCols, rRows );
    else
    {
        rCols = 0;
        rRows = 0;
    }
}

bool ScFormulaCell::GetDirty() const { return bDirty; }
void ScFormulaCell::ResetDirty() { bDirty = bTableOpDirty = mbPostponedDirty = false; }
bool ScFormulaCell::NeedsListening() const { return bNeedListening; }
void ScFormulaCell::SetNeedsListening( bool bVar ) { bNeedListening = bVar; }
void ScFormulaCell::SetNeedNumberFormat( bool bVal ) { mbNeedsNumberFormat = bVal; }
short ScFormulaCell::GetFormatType() const { return nFormatType; }

void ScFormulaCell::Compile( const OUString& rFormula, bool bNoListening,
                            const FormulaGrammar::Grammar eGrammar )
{
    if ( pDocument->IsClipOrUndo() )
        return;
    bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
    if ( bWasInFormulaTree )
        pDocument->RemoveFromFormulaTree( this );
    
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    ScCompiler aComp( pDocument, aPos);
    aComp.SetGrammar(eGrammar);
    pCode = aComp.CompileString( rFormula );
    if ( pCodeOld )
        delete pCodeOld;
    if( !pCode->GetCodeError() )
    {
        if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() && rFormula == aResult.GetHybridFormula() )
        {   
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
    
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    ScCompiler aComp(rCxt, aPos);
    pCode = aComp.CompileString( rFormula );
    if ( pCodeOld )
        delete pCodeOld;
    if( !pCode->GetCodeError() )
    {
        if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() && rFormula == aResult.GetHybridFormula() )
        {   
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
    
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        Compile( aResult.GetHybridFormula(), bNoListening, eTempGrammar);
    }
    else if( bCompile && !pDocument->IsClipOrUndo() && !pCode->GetCodeError() )
    {
        
        bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
        if ( bWasInFormulaTree )
            pDocument->RemoveFromFormulaTree( this );

        
        if( pDocument->IsInsertingFromOtherDoc() )
            bNoListening = true;

        if( !bNoListening && pCode->GetCodeLen() )
            EndListeningTo( pDocument );
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetCodeError() )
        {
            nFormatType = aComp.GetNumFormatType();
            bChanged = true;
            aResult.SetToken( NULL);
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
    
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        rCxt.setGrammar(eTempGrammar);
        Compile(rCxt, aResult.GetHybridFormula(), bNoListening);
    }
    else if( bCompile && !pDocument->IsClipOrUndo() && !pCode->GetCodeError() )
    {
        
        bool bWasInFormulaTree = pDocument->IsInFormulaTree( this );
        if ( bWasInFormulaTree )
            pDocument->RemoveFromFormulaTree( this );

        
        if( pDocument->IsInsertingFromOtherDoc() )
            bNoListening = true;

        if( !bNoListening && pCode->GetCodeLen() )
            EndListeningTo( pDocument );
        ScCompiler aComp(rCxt, aPos, *pCode);
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetCodeError() )
        {
            nFormatType = aComp.GetNumFormatType();
            bChanged = true;
            aResult.SetToken( NULL);
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
    if ( cMatrixFlag == MM_REFERENCE )
    {   
        
        StartListeningTo( pDocument );
        return ;
    }

    
    
    bool bWasInFormulaTree = pDocument->IsInFormulaTree( this);
    if (bWasInFormulaTree)
        pDocument->RemoveFromFormulaTree( this);
    rCxt.setGrammar(eTempGrammar);
    ScCompiler aComp(rCxt, aPos, *pCode);
    OUString aFormula, aFormulaNmsp;
    aComp.CreateStringFromXMLTokenArray( aFormula, aFormulaNmsp );
    pDocument->DecXMLImportedFormulaCount( aFormula.getLength() );
    rProgress.SetStateCountDownOnPercent( pDocument->GetXMLImportedFormulaCount() );
    
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    pCode = aComp.CompileString( aFormula, aFormulaNmsp );
    delete pCodeOld;
    if( !pCode->GetCodeError() )
    {
        if ( !pCode->GetLen() )
        {
            if ( aFormula[0] == '=' )
                pCode->AddBad( aFormula.copy( 1 ) );
            else
                pCode->AddBad( aFormula );
        }
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetCodeError() )
        {
            nFormatType = aComp.GetNumFormatType();
            bChanged = true;
            bCompile = false;
            StartListeningTo( pDocument );
        }

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }
    else
        bChanged = true;

    
    
    if ( !pDocument->GetHasMacroFunc() && pCode->HasOpCodeRPN( ocMacro ) )
        pDocument->SetHasMacroFunc( true );

    
    if( pCode->IsRecalcModeAlways() || pCode->IsRecalcModeForced() ||
        pCode->IsRecalcModeOnLoad() || pCode->IsRecalcModeOnLoadOnce() )
    {
        
        
        SetDirtyVar();
        pDocument->PutInFormulaTree(this);
    }
    else if (bWasInFormulaTree)
        pDocument->PutInFormulaTree(this);
}


void ScFormulaCell::CalcAfterLoad( sc::CompileFormulaContext& rCxt )
{
    bool bNewCompiled = false;
    
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        rCxt.setGrammar(eTempGrammar);
        Compile(rCxt, aResult.GetHybridFormula(), true);
        aResult.SetToken( NULL);
        bDirty = true;
        bNewCompiled = true;
    }
    
    if( pCode->GetLen() && !pCode->GetCodeLen() && !pCode->GetCodeError() )
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

    
    
    
    if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
    {
        OSL_FAIL("Formula cell INFINITY!!! Where does this document come from?");
        aResult.SetResultError( errIllegalFPOperation );
        bDirty = true;
    }

    
    
    if ( pDocument->GetSrcVersion() < SC_MATRIX_DOUBLEREF &&
            GetMatrixFlag() == MM_NONE && pCode->HasMatrixDoubleRefOps() )
    {
        cMatrixFlag = MM_FORMULA;
        SetMatColsRows( 1, 1);
    }

    
    
    if( !bNewCompiled || !pCode->GetCodeError() )
    {
        StartListeningTo( pDocument );
        if( !pCode->IsRecalcModeNormal() )
            bDirty = true;
    }
    if ( pCode->IsRecalcModeAlways() )
    {   
        
        bDirty = true;
    }
    
}


bool ScFormulaCell::MarkUsedExternalReferences()
{
    return pCode && pDocument->MarkUsedExternalReferences(*pCode, aPos);
}


void ScFormulaCell::Interpret()
{
    if (!IsDirtyOrInTableOpDirty() || pDocument->GetRecursionHelper().IsInReturn())
        return;     

    
    
    
    if ( pDocument->IsInDdeLinkUpdate() )
        return;

    if (bRunning)
    {
        if (!pDocument->GetDocOptions().IsIter())
        {
            aResult.SetResultError( errCircularReference );
            return;
        }

        if (aResult.GetResultError() == errCircularReference)
            aResult.SetResultError( 0 );

        
        if (!pDocument->GetRecursionHelper().IsDoingIteration() ||
                !pDocument->GetRecursionHelper().GetRecursionInIterationStack().top()->bIsIterCell)
            pDocument->GetRecursionHelper().SetInIterationReturn( true);

        return;
    }
    
    
    
    if (pDocument->GetRecursionHelper().GetIteration() && nSeenInIteration ==
            pDocument->GetRecursionHelper().GetIteration())
        return ;

    ScRecursionHelper& rRecursionHelper = pDocument->GetRecursionHelper();
    bool bOldRunning = bRunning;
    if (rRecursionHelper.GetRecursionCount() > MAXRECURSION)
    {
        bRunning = true;
        rRecursionHelper.SetInRecursionReturn( true);
    }
    else
    {
        if ( ! InterpretFormulaGroup() )
            InterpretTail( SCITP_NORMAL);
    }

    
    
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
                ScFormulaCell* pIterCell = this; 
                bool & rDone = rRecursionHelper.GetConvergingReference();
                rDone = false;
                if (!bIterationFromRecursion && bResumeIteration)
                {
                    bResumeIteration = false;
                    
                    ScFormulaRecursionList::const_iterator aOldStart(
                            rRecursionHelper.GetLastIterationStart());
                    rRecursionHelper.ResumeIteration();
                    
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            aOldStart; ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
                        pIterCell->bIsIterCell = true;
                    }
                    
                    
                    
                    
                    sal_uInt16 nIteration = rRecursionHelper.GetIteration();
                    for (ScFormulaRecursionList::const_iterator aIter(
                                aOldStart); aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
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
                    
                    rRecursionHelper.GetList().back().pCell->InterpretTail(
                            SCITP_CLOSE_ITERATION_CIRCLE);
                    
                    rRecursionHelper.StartIteration();
                    
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
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
                        pIterCell = (*aIter).pCell;
                        if (pIterCell->IsDirtyOrInTableOpDirty() &&
                                rRecursionHelper.GetIteration() !=
                                pIterCell->GetSeenInIteration())
                        {
                            (*aIter).aPreviousResult = pIterCell->aResult;
                            pIterCell->InterpretTail( SCITP_FROM_ITERATION);
                        }
                        rDone = rDone && !pIterCell->IsDirtyOrInTableOpDirty();
                    }
                    if (rRecursionHelper.IsInReturn())
                    {
                        bResumeIteration = true;
                        break;  
                        
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
                            pIterCell = (*aIter).pCell;
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
                            pIterCell = (*aIter).pCell;
                            pIterCell->bIsIterCell = false;
                            pIterCell->nSeenInIteration = 0;
                            pIterCell->bRunning = (*aIter).bOldRunning;
                            
                            
                            
                            pIterCell->ResetDirty();
                            pIterCell->aResult.SetResultError( errNoConvergence);
                            pIterCell->bChanged = true;
                        }
                    }
                    
                    rRecursionHelper.EndIteration();
                    bResumeIteration = rRecursionHelper.IsDoingIteration();
                }
            }
            if (rRecursionHelper.IsInRecursionReturn() &&
                    rRecursionHelper.GetRecursionCount() == 0 &&
                    !rRecursionHelper.IsDoingRecursion())
            {
                bIterationFromRecursion = false;
                
                
                
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
                            pCell->InterpretTail( SCITP_NORMAL);
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
}

bool ScFormulaCell::IsIterCell() const { return bIsIterCell; }
sal_uInt16 ScFormulaCell::GetSeenInIteration() const { return nSeenInIteration; }

void ScFormulaCell::InterpretTail( ScInterpretTailParameter eTailParam )
{
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
    } aRecursionCounter( pDocument->GetRecursionHelper(), this);
    nSeenInIteration = pDocument->GetRecursionHelper().GetIteration();
    if( !pCode->GetCodeLen() && !pCode->GetCodeError() )
    {
        
        
        
        
        
        
        
        
        
        if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
        {
            pCode->SetCodeError( errNoCode );
            
            
            OSL_FAIL( "ScFormulaCell::Interpret: no RPN, no error, no token, but hybrid formula string" );
            return;
        }
        CompileTokenArray();
    }

    if( pCode->GetCodeLen() && pDocument )
    {
        class StackCleaner
        {
            ScDocument*     pDoc;
            ScInterpreter*  pInt;
            public:
            StackCleaner( ScDocument* pD, ScInterpreter* pI )
                : pDoc(pD), pInt(pI)
                {}
            ~StackCleaner()
            {
                delete pInt;
                pDoc->DecInterpretLevel();
            }
        };
        pDocument->IncInterpretLevel();
        ScInterpreter* p = new ScInterpreter( this, pDocument, aPos, *pCode );
        StackCleaner aStackCleaner( pDocument, p);
        sal_uInt16 nOldErrCode = aResult.GetResultError();
        if ( nSeenInIteration == 0 )
        {   
            
            
            
            aResult.SetResultError( 0 );
        }

        switch ( aResult.GetResultError() )
        {
            case errCircularReference :     
                aResult.SetResultError( 0 );
            break;
        }

        bool bOldRunning = bRunning;
        bRunning = true;
        p->Interpret();
        if (pDocument->GetRecursionHelper().IsInReturn() && eTailParam != SCITP_CLOSE_ITERATION_CIRCLE)
        {
            if (nSeenInIteration > 0)
                --nSeenInIteration;     
            return;
        }
        bRunning = bOldRunning;

        
        
        bool bContentChanged = false;

        
        if( p->GetError() && pCode->IsHyperLink())
            pCode->SetHyperLink(false);

        if( p->GetError() && p->GetError() != errCircularReference)
        {
            ResetDirty();
            bChanged = true;
        }
        if (eTailParam == SCITP_FROM_ITERATION && IsDirtyOrInTableOpDirty())
        {
            bool bIsValue = aResult.IsValue();  
            
            if ((bIsValue && p->GetResultType() == svDouble && fabs(
                            p->GetNumResult() - aResult.GetDouble()) <=
                        pDocument->GetDocOptions().GetIterEps()) ||
                    (!bIsValue && p->GetResultType() == svString &&
                     p->GetStringResult() == aResult.GetString()))
            {
                
                
                
                
                
                
                
                if (nSeenInIteration > 1 ||
                        pDocument->GetDocOptions().GetIterCount() == 1)
                {
                    ResetDirty();
                }
            }
        }

        
        if( p->GetError() != nOldErrCode )
        {
            bChanged = true;
            
            if ( aResult.GetCellResultType() != svUnknown )
                bContentChanged = true;
        }

        if( mbNeedsNumberFormat )
        {
            nFormatType = p->GetRetFormatType();
            sal_Int32 nFormatIndex = p->GetRetFormatIndex();

            
            if(nFormatType == NUMBERFORMAT_TEXT)
                nFormatIndex = 0;
            else if((nFormatIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
                nFormatIndex = ScGlobal::GetStandardFormat(*pDocument->GetFormatTable(),
                        nFormatIndex, nFormatType);

            
            pDocument->SetNumberFormat( aPos, nFormatIndex );

            bChanged = true;
            mbNeedsNumberFormat = false;
        }

        
        
        if (bChanged)
        {
            
            
            if ( !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                ScFormulaResult aNewResult( p->GetResultToken().get());
                StackVar eOld = aResult.GetCellResultType();
                StackVar eNew = aNewResult.GetCellResultType();
                if ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) )
                {
                    
                    
                }
                else
                {
                    if ( eOld == svHybridCell || eOld == svHybridValueCell )     
                        eOld = svString;            

                    
                    bContentChanged = (eOld != eNew ||
                            (eNew == svDouble && !rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble() )) ||
                            (eNew == svString && aResult.GetString() != aNewResult.GetString()));
                }
            }

            aResult.SetToken( p->GetResultToken().get() );
        }
        else
        {
            ScFormulaResult aNewResult( p->GetResultToken().get());
            StackVar eOld = aResult.GetCellResultType();
            StackVar eNew = aNewResult.GetCellResultType();
            bChanged = (eOld != eNew ||
                    (eNew == svDouble && aResult.GetDouble() != aNewResult.GetDouble()) ||
                    (eNew == svString && aResult.GetString() != aNewResult.GetString()));

            
            if ( bChanged && !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                if ( ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) ) ||
                     ( (eOld == svHybridCell || eOld == svHybridValueCell) && eNew == svString && aResult.GetString() == aNewResult.GetString() ) ||
                     ( eOld == svDouble && eNew == svDouble && rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble() ) ) )
                {
                    
                }
                else
                    bContentChanged = true;
            }

            aResult.Assign( aNewResult);
        }

        
        if ( aResult.IsValue() && !p->GetError()
          && pDocument->GetDocOptions().IsCalcAsShown()
          && nFormatType != NUMBERFORMAT_DATE
          && nFormatType != NUMBERFORMAT_TIME
          && nFormatType != NUMBERFORMAT_DATETIME )
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
            
            
            if( cMatrixFlag != MM_FORMULA && !pCode->IsHyperLink() )
                aResult.SetToken( aResult.GetCellResultToken().get());
        }
        if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
        {
            
            sal_uInt16 nErr = GetDoubleErrorValue( aResult.GetDouble());
            aResult.SetResultError( nErr);
            bChanged = bContentChanged = true;
        }

        if (bContentChanged && pDocument->IsStreamValid(aPos.Tab()))
        {
            
            
            pDocument->SetStreamValid(aPos.Tab(), false, true);
        }
        if ( !pCode->IsRecalcModeAlways() )
            pDocument->RemoveFromFormulaTree( this );

        

        if ( pCode->IsRecalcModeForced() )
        {
            sal_uLong nValidation = ((const SfxUInt32Item*) pDocument->GetAttr(
                    aPos.Col(), aPos.Row(), aPos.Tab(), ATTR_VALIDDATA ))->GetValue();
            if ( nValidation )
            {
                const ScValidationData* pData = pDocument->GetValidationEntry( nValidation );
                ScRefCellValue aTmpCell(this);
                if ( pData && !pData->IsDataValid(aTmpCell, aPos))
                    pData->DoCalcError( this );
            }
        }

        
        ScProgress::GetInterpretProgress()->SetStateCountDownOnPercent(
            pDocument->GetFormulaCodeInTree()/MIN_NO_CODES_PER_PROGRESS_UPDATE );

        switch (p->GetVolatileType())
        {
            case ScInterpreter::VOLATILE:
                
            break;
            case ScInterpreter::VOLATILE_MACRO:
                
                pCode->SetExclusiveRecalcModeAlways();
                pDocument->PutInFormulaTree(this);
                StartListeningTo(pDocument);
            break;
            case ScInterpreter::NOT_VOLATILE:
                if (pCode->IsRecalcModeAlways())
                {
                    
                    EndListeningTo(pDocument);
                    pCode->SetExclusiveRecalcModeNormal();
                }
                else
                {
                    
                    
                    pDocument->EndListeningArea(BCA_LISTEN_ALWAYS, this);
                }
                pDocument->RemoveFromFormulaTree(this);
            break;
            default:
                ;
        }
    }
    else
    {
        
        OSL_ENSURE( pCode->GetCodeError(), "no RPN code und no errors ?!?!" );
        ResetDirty();
    }
}

void ScFormulaCell::SetCompile( bool bVal )
{
    bCompile = bVal;
}

ScDocument* ScFormulaCell::GetDocument() const
{
    return pDocument;
}

void ScFormulaCell::SetMatColsRows( SCCOL nCols, SCROW nRows, bool bDirtyFlag )
{
    ScMatrixFormulaCellToken* pMat = aResult.GetMatrixFormulaCellTokenNonConst();
    if (pMat)
        pMat->SetMatColsRows( nCols, nRows );
    else if (nCols || nRows)
    {
        aResult.SetToken( new ScMatrixFormulaCellToken( nCols, nRows));
        
        
        SetDirty( bDirtyFlag );
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

bool ScFormulaCell::IsInChangeTrack() const
{
    return bInChangeTrack;
}

void ScFormulaCell::Notify( SvtBroadcaster&, const SfxHint& rHint)
{
    if ( !pDocument->IsInDtorClear() && !pDocument->GetHardRecalcState() )
    {
        const ScHint* p = PTR_CAST( ScHint, &rHint );
        sal_uLong nHint = (p ? p->GetId() : 0);
        if (nHint & (SC_HINT_DATACHANGED | SC_HINT_TABLEOPDIRTY))
        {
            bool bForceTrack = false;
            if ( nHint & SC_HINT_TABLEOPDIRTY )
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
            
            
            
            
            
            
            
            
            if ( (bForceTrack || !pDocument->IsInFormulaTree( this )
                    || pCode->IsRecalcModeAlways())
                    && !pDocument->IsInFormulaTrack( this ) )
                pDocument->AppendToFormulaTrack( this );
        }
    }
}

void ScFormulaCell::SetDirty( bool bDirtyFlag )
{
    if ( !IsInChangeTrack() )
    {
        if ( pDocument->GetHardRecalcState() )
            SetDirtyVar();
        else
        {
            
            
            
            
            if ( !bDirty || mbPostponedDirty || !pDocument->IsInFormulaTree( this ) )
            {
                if( bDirtyFlag )
                    SetDirtyVar();
                pDocument->AppendToFormulaTrack( this );
                pDocument->TrackFormulas();
            }
        }

        if (pDocument->IsStreamValid(aPos.Tab()))
            pDocument->SetStreamValid(aPos.Tab(), false);
    }
}

void ScFormulaCell::SetDirtyVar()
{
    bDirty = true;
    mbPostponedDirty = false;
    if (mxGroup && mxGroup->meCalcState == sc::GroupCalcRunning)
        mxGroup->meCalcState = sc::GroupCalcEnabled;

    
    
}

void ScFormulaCell::SetDirtyAfterLoad()
{
    bDirty = true;
    if ( !pDocument->GetHardRecalcState() )
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
        if ( pDocument->GetHardRecalcState() )
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
                pDocument->TrackFormulas( SC_HINT_TABLEOPDIRTY );
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
    aResult.SetDouble( n);
}

void ScFormulaCell::SetResultToken( const formula::FormulaToken* pToken )
{
    aResult.SetToken(pToken);
}

svl::SharedString ScFormulaCell::GetResultString() const
{
    return aResult.GetString();
}

void ScFormulaCell::SetResultMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL )
{
    aResult.SetMatrix(nCols, nRows, pMat, pUL);
}

void ScFormulaCell::SetErrCode( sal_uInt16 n )
{
    /* FIXME: check the numerous places where ScTokenArray::GetCodeError() is
     * used whether it is solely for transport of a simple result error and get
     * rid of that abuse. */
    pCode->SetCodeError( n );
    
    
    
    aResult.SetResultError( n );
}

void ScFormulaCell::AddRecalcMode( ScRecalcMode nBits )
{
    if ( (nBits & RECALCMODE_EMASK) != RECALCMODE_NORMAL )
        SetDirtyVar();
    if ( nBits & RECALCMODE_ONLOAD_ONCE )
    {   
        nBits = (nBits & ~RECALCMODE_EMASK) | RECALCMODE_NORMAL;
    }
    pCode->AddRecalcMode( nBits );
}

void ScFormulaCell::SetHybridDouble( double n )
{
    aResult.SetHybridDouble( n);
}

void ScFormulaCell::SetHybridString( const OUString& r )
{
    aResult.SetHybridString( r);
}

void ScFormulaCell::SetHybridFormula( const OUString& r,
                                    const formula::FormulaGrammar::Grammar eGrammar )
{
    aResult.SetHybridFormula( r); eTempGrammar = eGrammar;
}


void ScFormulaCell::GetURLResult( OUString& rURL, OUString& rCellText )
{
    OUString aCellString;

    Color* pColor;

    
    
    sal_uLong nCellFormat = pDocument->GetNumberFormat( aPos );
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();

    sal_uLong nURLFormat = ScGlobal::GetStandardFormat( *pFormatter, nCellFormat, NUMBERFORMAT_NUMBER);

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

void ScFormulaCell::MaybeInterpret()
{
    if (mxGroup && mxGroup->meCalcState == sc::GroupCalcOpenCLKernelCompilationScheduled)
        return;

    if (!IsDirtyOrInTableOpDirty())
        return;

    if (pDocument->GetAutoCalc() || (cMatrixFlag != MM_NONE))
        Interpret();
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
    if (pCode->GetCodeError())
        return false;

    return aResult.IsValueNoError();
}

bool ScFormulaCell::IsHybridValueCell()
{
    return aResult.GetType() == formula::svHybridValueCell;
}

double ScFormulaCell::GetValue()
{
    MaybeInterpret();
    if ((!pCode->GetCodeError() || pCode->GetCodeError() == errDoubleRef) &&
            !aResult.GetResultError())
        return aResult.GetDouble();
    return 0.0;
}

svl::SharedString ScFormulaCell::GetString()
{
    MaybeInterpret();
    if ((!pCode->GetCodeError() || pCode->GetCodeError() == errDoubleRef) &&
            !aResult.GetResultError())
        return aResult.GetString();

    return svl::SharedString::getEmptyString();
}

const ScMatrix* ScFormulaCell::GetMatrix()
{
    if ( pDocument->GetAutoCalc() )
    {
        if( IsDirtyOrInTableOpDirty()
        
        || (!bDirty && cMatrixFlag == MM_FORMULA && !aResult.GetMatrix()))
            Interpret();
    }
    return aResult.GetMatrix().get();
}

bool ScFormulaCell::GetMatrixOrigin( ScAddress& rPos ) const
{
    switch ( cMatrixFlag )
    {
        case MM_FORMULA :
            rPos = aPos;
            return true;
        case MM_REFERENCE :
        {
            pCode->Reset();
            ScToken* t = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
            if( t )
            {
                ScSingleRefData& rRef = t->GetSingleRef();
                ScAddress aAbs = rRef.toAbs(aPos);
                if (ValidAddress(aAbs))
                {
                    rPos = aAbs;
                    return true;
                }
            }
        }
        break;
    }
    return false;
}


/*
 Edge-Values:

   8
 4   16
   2

 inside: 1
 outside: 0
 (reserved: open: 32)
 */

sal_uInt16 ScFormulaCell::GetMatrixEdge( ScAddress& rOrgPos ) const
{
    switch ( cMatrixFlag )
    {
        case MM_FORMULA :
        case MM_REFERENCE :
        {
            static SCCOL nC;
            static SCROW nR;
            ScAddress aOrg;
            if ( !GetMatrixOrigin( aOrg ) )
                return 0;               
            if ( aOrg != rOrgPos )
            {   
                rOrgPos = aOrg;
                const ScFormulaCell* pFCell;
                if ( cMatrixFlag == MM_REFERENCE )
                    pFCell = pDocument->GetFormulaCell(aOrg);
                else
                    pFCell = this;      
                
                if (pFCell && pFCell->cMatrixFlag == MM_FORMULA)
                {
                    pFCell->GetMatColsRows( nC, nR );
                    if ( nC == 0 || nR == 0 )
                    {
                        
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
                            if (pCell && pCell->cMatrixFlag == MM_REFERENCE &&
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
                            if (pCell && pCell->cMatrixFlag == MM_REFERENCE &&
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
                    OStringBuffer aMsg("broken Matrix, no MatFormula at origin, Pos: ");
                    OUString aTmp(aPos.Format(SCA_VALID_COL | SCA_VALID_ROW, pDocument));
                    aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_ASCII_US));
                    aMsg.append(", MatOrg: ");
                    aTmp = aOrg.Format(SCA_VALID_COL | SCA_VALID_ROW, pDocument);
                    aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_ASCII_US));
                    OSL_FAIL(aMsg.getStr());
#endif
                    return 0;           
                }
            }
            
            SCsCOL dC = aPos.Col() - aOrg.Col();
            SCsROW dR = aPos.Row() - aOrg.Row();
            sal_uInt16 nEdges = 0;
            if ( dC >= 0 && dR >= 0 && dC < nC && dR < nR )
            {
                if ( dC == 0 )
                    nEdges |= sc::MatrixEdgeLeft;            
                if ( dC+1 == nC )
                    nEdges |= sc::MatrixEdgeRight;           
                if ( dR == 0 )
                    nEdges |= sc::MatrixEdgeTop;            
                if ( dR+1 == nR )
                    nEdges |= sc::MatrixEdgeBottom;            
                if ( !nEdges )
                    nEdges = sc::MatrixEdgeInside;             
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OStringBuffer aMsg( "broken Matrix, Pos: " );
                OUString aTmp(aPos.Format(SCA_VALID_COL | SCA_VALID_ROW, pDocument));
                aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8 ));
                aMsg.append(", MatOrg: ");
                aTmp = aOrg.Format(SCA_VALID_COL | SCA_VALID_ROW, pDocument);
                aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8 ));
                aMsg.append(", MatCols: ");
                aMsg.append(static_cast<sal_Int32>( nC ));
                aMsg.append(", MatRows: ");
                aMsg.append(static_cast<sal_Int32>( nR ));
                aMsg.append(", DiffCols: ");
                aMsg.append(static_cast<sal_Int32>( dC ));
                aMsg.append(", DiffRows: ");
                aMsg.append(static_cast<sal_Int32>( dR ));
                OSL_FAIL( aMsg.makeStringAndClear().getStr());
            }
#endif
            return nEdges;

        }
        default:
            return 0;
    }
}

sal_uInt16 ScFormulaCell::GetErrCode()
{
    MaybeInterpret();

    /* FIXME: If ScTokenArray::SetCodeError() was really only for code errors
     * and not also abused for signaling other error conditions we could bail
     * out even before attempting to interpret broken code. */
    sal_uInt16 nErr =  pCode->GetCodeError();
    if (nErr)
        return nErr;
    return aResult.GetResultError();
}

sal_uInt16 ScFormulaCell::GetRawError()
{
    sal_uInt16 nErr =  pCode->GetCodeError();
    if (nErr)
        return nErr;
    return aResult.GetResultError();
}

bool ScFormulaCell::GetErrorOrValue( sal_uInt16& rErr, double& rVal )
{
    MaybeInterpret();

    rErr = pCode->GetCodeError();
    if (rErr)
        return true;

    return aResult.GetErrorOrDouble(rErr, rVal);
}

bool ScFormulaCell::GetErrorOrString( sal_uInt16& rErr, svl::SharedString& rStr )
{
    MaybeInterpret();

    rErr = pCode->GetCodeError();
    if (rErr)
        return true;

    return aResult.GetErrorOrString(rErr, rStr);
}

sc::FormulaResultValue ScFormulaCell::GetResult()
{
    MaybeInterpret();

    sal_uInt16 nErr = pCode->GetCodeError();
    if (nErr)
        return sc::FormulaResultValue(nErr);

    return aResult.GetResult();
}

bool ScFormulaCell::HasOneReference( ScRange& r ) const
{
    pCode->Reset();
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    if( p && !pCode->GetNextReferenceRPN() )        
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

    
    
    
    
    if (HasOneReference( rRange))
        return true;

    pCode->Reset();
    
    ScToken* const pFirstReference(
            dynamic_cast<ScToken*>(pCode->GetNextReferenceRPN()));
    if (pFirstReference)
    {
        
        
        std::deque<ScToken*> aReferences;
        aReferences.push_back(pFirstReference);
        FormulaToken* pToken(pCode->NextRPN());
        FormulaToken* pFunction(0);
        while (pToken)
        {
            if (lcl_isReference(*pToken))
            {
                aReferences.push_back(dynamic_cast<ScToken*>(pToken));
                pToken = pCode->NextRPN();
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
        if (pFunction && !pCode->GetNextReferenceRPN()
                && (pFunction->GetParamCount() == aReferences.size()))
        {
            return lcl_refListFormsOneRange(aPos, aReferences, rRange);
        }
    }
    return false;
}

bool ScFormulaCell::HasRelNameReference() const
{
    pCode->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pCode->GetNextReferenceRPN()) ) != NULL )
    {
        if ( t->GetSingleRef().IsRelName() ||
                (t->GetType() == formula::svDoubleRef &&
                t->GetDoubleRef().Ref2.IsRelName()) )
            return true;
    }
    return false;
}

bool ScFormulaCell::HasColRowName() const
{
    pCode->Reset();
    return (pCode->GetNextColRowName() != NULL);
}

bool ScFormulaCell::UpdatePosOnShift( const sc::RefUpdateContext& rCxt )
{
    if (rCxt.meMode != URM_INSDEL)
        
        return false;

    if (!rCxt.mnColDelta && !rCxt.mnRowDelta && !rCxt.mnTabDelta)
        
        return false;

    if (!rCxt.maRange.In(aPos))
        return false;

    
    
    aPos.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);

    return true;
}

namespace {

/**
 * Check if we need to re-compile column or row names.
 */
bool checkCompileColRowName(
    const sc::RefUpdateContext& rCxt, ScDocument& rDoc, ScTokenArray& rCode,
    const ScAddress& aOldPos, const ScAddress& aPos, bool bValChanged)
{
    switch (rCxt.meMode)
    {
        case URM_INSDEL:
        {
            if (rCxt.mnColDelta <= 0 && rCxt.mnRowDelta <= 0)
                return false;

            ScToken* t;
            ScRangePairList* pColList = rDoc.GetColNameRanges();
            ScRangePairList* pRowList = rDoc.GetRowNameRanges();
            rCode.Reset();
            while ((t = static_cast<ScToken*>(rCode.GetNextColRowName())) != NULL)
            {
                ScSingleRefData& rRef = t->GetSingleRef();
                if (rCxt.mnRowDelta > 0 && rRef.IsColRel())
                {   
                    ScAddress aAdr = rRef.toAbs(aPos);
                    ScRangePair* pR = pColList->Find( aAdr );
                    if ( pR )
                    {   
                        if (pR->GetRange(1).aStart.Row() == rCxt.maRange.aStart.Row())
                            return true;
                    }
                    else
                    {   
                        if (aAdr.Row() + 1 == rCxt.maRange.aStart.Row())
                            return true;
                    }
                }
                if (rCxt.mnColDelta > 0 && rRef.IsRowRel())
                {   
                    ScAddress aAdr = rRef.toAbs(aPos);
                    ScRangePair* pR = pRowList->Find( aAdr );
                    if ( pR )
                    {   
                        if ( pR->GetRange(1).aStart.Col() == rCxt.maRange.aStart.Col())
                            return true;
                    }
                    else
                    {   
                        if (aAdr.Col() + 1 == rCxt.maRange.aStart.Col())
                            return true;
                    }
                }
            }
        }
        break;
        case URM_MOVE:
        {   
            
            bool bMoved = (aPos != aOldPos);
            if (bMoved)
                return true;

            rCode.Reset();
            const ScToken* t = static_cast<const ScToken*>(rCode.GetNextColRowName());
            for (; t; t = static_cast<const ScToken*>(rCode.GetNextColRowName()))
            {
                const ScSingleRefData& rRef = t->GetSingleRef();
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
    ScDocument* pUndoDoc, const ScAddress& aUndoPos, ScTokenArray* pOldCode, FormulaGrammar::Grammar eTempGrammar, sal_uInt8 cMatrixFlag)
{
    
    
    
    

    
    
    if (pUndoDoc->GetCellType(aUndoPos) == CELLTYPE_FORMULA)
        return;

    ScFormulaCell* pFCell =
        new ScFormulaCell(
            pUndoDoc, aUndoPos, pOldCode ? *pOldCode : ScTokenArray(), eTempGrammar, cMatrixFlag);

    pFCell->SetResultToken(NULL);  
    pUndoDoc->SetFormulaCell(aUndoPos, pFCell);
}

}

bool ScFormulaCell::UpdateReferenceOnShift(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (rCxt.meMode != URM_INSDEL)
        
        return false;

    bool bCellStateChanged = false;
    ScAddress aUndoPos( aPos );         
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );
    bCellStateChanged = UpdatePosOnShift(rCxt);

    
    pCode->Reset();
    bool bHasRefs = (pCode->GetNextReferenceRPN() != NULL);
    bool bHasColRowNames = false;
    if (!bHasRefs)
    {
        pCode->Reset();
        bHasColRowNames = (pCode->GetNextColRowName() != NULL);
        bHasRefs = bHasRefs || bHasColRowNames;
    }
    bool bOnRefMove = pCode->IsRecalcModeOnRefMove();

    if (!bHasRefs && !bOnRefMove)
        
        
        return bCellStateChanged;

    boost::scoped_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    bool bValChanged = false;
    bool bRefModified = false;
    bool bRefSizeChanged = false;
    bool bRecompile = bCompile;

    if (bHasRefs)
    {
        
        sc::RefUpdateResult aRes = pCode->AdjustReferenceOnShift(rCxt, aOldPos);
        bRefModified = aRes.mbReferenceModified;
        bValChanged = aRes.mbValueChanged;
        if (aRes.mbNameModified)
            bRecompile = true;
    }

    if (bValChanged || bRefModified)
        bCellStateChanged = true;

    if (bOnRefMove)
        
        bOnRefMove = (bValChanged || (aPos != aOldPos));

    bool bNewListening = false;
    bool bInDeleteUndo = false;

    if (bHasRefs)
    {
        
        
        if (bHasColRowNames && !bRecompile)
            bRecompile = checkCompileColRowName(rCxt, *pDocument, *pCode, aOldPos, aPos, bValChanged);

        ScChangeTrack* pChangeTrack = pDocument->GetChangeTrack();
        bInDeleteUndo = (pChangeTrack && pChangeTrack->IsInDeleteUndo());

        
        bool bHasRelName = HasRelNameReference();
        
        
        bNewListening = (bRefModified || bRecompile
                || (bValChanged && (bInDeleteUndo || bRefSizeChanged)) || bHasRelName);

        if ( bNewListening )
            EndListeningTo(pDocument, pOldCode.get(), aOldPos);
    }

    
    bool bNeedDirty = (bValChanged || bRecompile || bOnRefMove);

    if (pUndoDoc && (bValChanged || bOnRefMove))
        setOldCodeToUndo(pUndoDoc, aUndoPos, pOldCode.get(), eTempGrammar, cMatrixFlag);

    bCompile |= bRecompile;
    if (bCompile)
    {
        CompileTokenArray( bNewListening ); 
        bNeedDirty = true;
    }

    if ( !bInDeleteUndo )
    {   
        
        if ( bNewListening )
        {
            
            
            
            
            SetNeedsListening( true);
        }
    }

    if (bNeedDirty)
    {   
        
        
        mbPostponedDirty = true;
    }

    return bCellStateChanged;
}

bool ScFormulaCell::UpdateReferenceOnMove(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (rCxt.meMode != URM_MOVE)
        return false;

    ScAddress aUndoPos( aPos );         
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );

    if (rCxt.maRange.In(aPos))
    {
        
        
        
        
        aOldPos.Set(aPos.Col() - rCxt.mnColDelta, aPos.Row() - rCxt.mnRowDelta, aPos.Tab() - rCxt.mnTabDelta);
    }

    
    pCode->Reset();
    bool bHasRefs = (pCode->GetNextReferenceRPN() != NULL);
    bool bHasColRowNames = false;
    if (!bHasRefs)
    {
        pCode->Reset();
        bHasColRowNames = (pCode->GetNextColRowName() != NULL);
        bHasRefs = bHasRefs || bHasColRowNames;
    }
    bool bOnRefMove = pCode->IsRecalcModeOnRefMove();

    if (!bHasRefs && !bOnRefMove)
        
        
        return false;

    bool bCellStateChanged = false;
    boost::scoped_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    bool bValChanged = false;
    bool bRefModified = false;
    bool bRefSizeChanged = false;

    if (bHasRefs)
    {
        
        sc::RefUpdateResult aRes = pCode->AdjustReferenceOnMove(rCxt, aOldPos, aPos);
        bRefModified = aRes.mbReferenceModified;
        bValChanged = aRes.mbValueChanged;
    }

    if (bValChanged || bRefModified)
        bCellStateChanged = true;

    if (bOnRefMove)
        
        bOnRefMove = (bValChanged || (aPos != aOldPos));

    bool bColRowNameCompile = false;
    bool bHasRelName = false;
    bool bNewListening = false;
    bool bInDeleteUndo = false;

    if (bHasRefs)
    {
        
        
        if (bHasColRowNames)
            bColRowNameCompile = checkCompileColRowName(rCxt, *pDocument, *pCode, aOldPos, aPos, bValChanged);

        ScChangeTrack* pChangeTrack = pDocument->GetChangeTrack();
        bInDeleteUndo = (pChangeTrack && pChangeTrack->IsInDeleteUndo());

        
        bHasRelName = HasRelNameReference();
        
        
        bNewListening = (bRefModified || bColRowNameCompile
                || bValChanged || bHasRelName)
            
            
            
            && !(pDocument->IsInsertingFromOtherDoc() && rCxt.maRange.In(aPos));

        if ( bNewListening )
            EndListeningTo(pDocument, pOldCode.get(), aOldPos);
    }

    bool bNeedDirty = false;
    
    if ( bRefModified || bColRowNameCompile ||
         (bValChanged && bHasRelName && (bHasRelName || bInDeleteUndo || bRefSizeChanged)) || bOnRefMove)
        bNeedDirty = true;

    if (pUndoDoc && (bValChanged || bOnRefMove))
        setOldCodeToUndo(pUndoDoc, aUndoPos, pOldCode.get(), eTempGrammar, cMatrixFlag);

    bValChanged = false;

    bCompile = (bCompile || bValChanged || bRefModified || bColRowNameCompile);
    if ( bCompile )
    {
        CompileTokenArray( bNewListening ); 
        bNeedDirty = true;
    }

    if ( !bInDeleteUndo )
    {   
        
        if ( bNewListening )
        {
            StartListeningTo( pDocument );
        }
    }

    if (bNeedDirty)
    {   
        sc::AutoCalcSwitch(*pDocument, false);
        SetDirty();
    }

    return bCellStateChanged;
}

bool ScFormulaCell::UpdateReferenceOnCopy(
    const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    if (rCxt.meMode != URM_COPY)
        return false;

    ScAddress aUndoPos( aPos );         
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );

    if (rCxt.maRange.In(aPos))
    {
        
        
        
        
        aOldPos.Set(aPos.Col() - rCxt.mnColDelta, aPos.Row() - rCxt.mnRowDelta, aPos.Tab() - rCxt.mnTabDelta);
    }

    
    pCode->Reset();
    bool bHasRefs = (pCode->GetNextReferenceRPN() != NULL);
    pCode->Reset();
    bool bHasColRowNames = (pCode->GetNextColRowName() != NULL);
    bHasRefs = bHasRefs || bHasColRowNames;
    bool bOnRefMove = pCode->IsRecalcModeOnRefMove();

    if (!bHasRefs && !bOnRefMove)
        
        
        return false;

    boost::scoped_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    if (bOnRefMove)
        
        bOnRefMove = (aPos != aOldPos);

    bool bNeedDirty = bOnRefMove;

    if (pUndoDoc && bOnRefMove)
        setOldCodeToUndo(pUndoDoc, aUndoPos, pOldCode.get(), eTempGrammar, cMatrixFlag);

    if (bCompile)
    {
        CompileTokenArray(false); 
        bNeedDirty = true;
    }

    if (bNeedDirty)
    {   
        sc::AutoCalcSwitch(*pDocument, false);
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
    
    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    bool bPosChanged = (rCxt.mnInsertPos <= aPos.Tab());
    pCode->Reset();
    if (pDocument->IsClipOrUndo() || !pCode->GetNextReferenceRPN())
    {
        if (bPosChanged)
            aPos.IncTab(rCxt.mnSheets);

        return;
    }

    EndListeningTo( pDocument );
    ScAddress aOldPos = aPos;
    
    if (bPosChanged)
        aPos.IncTab(rCxt.mnSheets);

    if (!bAdjustCode)
        return;

    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnInsertedTab(rCxt, aOldPos);
    if (aRes.mbNameModified)
        
        bCompile = true;

    
}

bool ScFormulaCell::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    
    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    bool bPosChanged = (aPos.Tab() >= rCxt.mnDeletePos + rCxt.mnSheets);
    pCode->Reset();
    if (pDocument->IsClipOrUndo() || !pCode->GetNextReferenceRPN())
    {
        if (bPosChanged)
            aPos.IncTab(-1*rCxt.mnSheets);
        return false;
    }

    EndListeningTo( pDocument );
    
    ScAddress aOldPos = aPos;
    if (bPosChanged)
        aPos.IncTab(-1*rCxt.mnSheets);

    if (!bAdjustCode)
        return false;

    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnDeletedTab(rCxt, aOldPos);
    if (aRes.mbNameModified)
        
        bCompile = true;

    return aRes.mbReferenceModified;
}

void ScFormulaCell::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo )
{
    
    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;

    pCode->Reset();
    if (!pCode->GetNextReferenceRPN() || pDocument->IsClipOrUndo())
    {
        aPos.SetTab(nTabNo);
        return;
    }

    EndListeningTo(pDocument);
    ScAddress aOldPos = aPos;
    
    aPos.SetTab(nTabNo);

    

    if (!bAdjustCode)
        return;

    sc::RefUpdateResult aRes = pCode->AdjustReferenceOnMovedTab(rCxt, aOldPos);
    if (aRes.mbNameModified)
        
        bCompile = true;
}

void ScFormulaCell::UpdateInsertTabAbs(SCTAB nTable)
{
    if (pDocument->IsClipOrUndo())
        return;

    bool bAdjustCode = !mxGroup || mxGroup->mpTopCell == this;
    if (!bAdjustCode)
        return;

    pCode->Reset();
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    while (p)
    {
        ScSingleRefData& rRef1 = p->GetSingleRef();
        if (!rRef1.IsTabRel() && nTable <= rRef1.Tab())
            rRef1.IncTab(1);
        if (p->GetType() == formula::svDoubleRef)
        {
            ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
            if (!rRef2.IsTabRel() && nTable <= rRef2.Tab())
                rRef2.IncTab(1);
        }
        p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
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
    pCode->Reset();
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    while (p)
    {
        ScSingleRefData& rRef1 = p->GetSingleRef();
        if (!rRef1.IsTabRel())
        {
            if (nTable != rRef1.Tab())
                bRet = true;
            else if (nTable != aPos.Tab())
                rRef1.SetAbsTab(aPos.Tab());
        }
        if (p->GetType() == formula::svDoubleRef)
        {
            ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
            if (!rRef2.IsTabRel())
            {
                if(nTable != rRef2.Tab())
                    bRet = true;
                else if (nTable != aPos.Tab())
                    rRef2.SetAbsTab(aPos.Tab());
            }
        }
        p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    }
    return bRet;
}

void ScFormulaCell::UpdateCompile( bool bForceIfNameInUse )
{
    if ( bForceIfNameInUse && !bCompile )
        bCompile = pCode->HasNameOrColRowName();
    if ( bCompile )
        pCode->SetCodeError( 0 );   
    CompileTokenArray();
}


void ScFormulaCell::TransposeReference()
{
    bool bFound = false;
    pCode->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pCode->GetNextReference()) ) != NULL )
    {
        ScSingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsColRel() && rRef1.IsRowRel() )
        {
            bool bDouble = (t->GetType() == formula::svDoubleRef);
            ScSingleRefData& rRef2 = (bDouble ? t->GetDoubleRef().Ref2 : rRef1);
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
    bool bPosChanged = false; 

    ScRange aDestRange( rDest, ScAddress(
                static_cast<SCCOL>(rDest.Col() + rSource.aEnd.Row() - rSource.aStart.Row()),
                static_cast<SCROW>(rDest.Row() + rSource.aEnd.Col() - rSource.aStart.Col()),
                rDest.Tab() + rSource.aEnd.Tab() - rSource.aStart.Tab() ) );
    if ( aDestRange.In( aOldPos ) )
    {
        
        SCsCOL nRelPosX = aOldPos.Col();
        SCsROW nRelPosY = aOldPos.Row();
        SCsTAB nRelPosZ = aOldPos.Tab();
        ScRefUpdate::DoTranspose( nRelPosX, nRelPosY, nRelPosZ, pDocument, aDestRange, rSource.aStart );
        aOldPos.Set( nRelPosX, nRelPosY, nRelPosZ );
        bPosChanged = true;
    }

    ScTokenArray* pOld = pUndoDoc ? pCode->Clone() : NULL;
    bool bRefChanged = false;
    ScToken* t;

    ScRangeData* pShared = NULL;
    pCode->Reset();
    while( (t = static_cast<ScToken*>(pCode->GetNextReferenceOrName())) != NULL )
    {
        if( t->GetOpCode() == ocName )
        {
            ScRangeData* pName = pDocument->GetRangeName()->findByIndex( t->GetIndex() );
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
                rRef.SetRange(aAbs, aPos); 
                bRefChanged = true;
            }
        }
    }

    if (pShared) 
    {
        pDocument->RemoveFromFormulaTree( this ); 
        delete pCode;
        pCode = new ScTokenArray( *pShared->GetCode() );
        bRefChanged = true;
        pCode->Reset();
        while( (t = static_cast<ScToken*>(pCode->GetNextReference())) != NULL )
        {
            if( t->GetType() != svIndex )
            {
                SingleDoubleRefModifier aMod(*t);
                ScComplexRefData& rRef = aMod.Ref();
                ScRange aAbs = rRef.toAbs(aOldPos);
                bool bMod = (ScRefUpdate::UpdateTranspose(pDocument, rSource, rDest, aAbs) != UR_NOTHING || bPosChanged);
                if (bMod)
                    rRef.SetRange(aAbs, aPos); 
            }
        }
    }

    if (bRefChanged)
    {
        if (pUndoDoc)
        {
            ScFormulaCell* pFCell = new ScFormulaCell(
                    pUndoDoc, aPos, pOld ? *pOld : ScTokenArray(), eTempGrammar, cMatrixFlag);

            pFCell->aResult.SetToken( NULL);  
            pUndoDoc->SetFormulaCell(aPos, pFCell);
        }

        bCompile = true;
        CompileTokenArray(); 
        SetDirty();
    }
    else
        StartListeningTo( pDocument ); 

    delete pOld;
}

void ScFormulaCell::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    EndListeningTo( pDocument );

    bool bRefChanged = false;
    ScToken* t;
    ScRangeData* pShared = NULL;

    pCode->Reset();
    while( (t = static_cast<ScToken*>(pCode->GetNextReferenceOrName())) != NULL )
    {
        if( t->GetOpCode() == ocName )
        {
            ScRangeData* pName = pDocument->GetRangeName()->findByIndex( t->GetIndex() );
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

    if (pShared) 
    {
        pDocument->RemoveFromFormulaTree( this ); 
        delete pCode;
        pCode = new ScTokenArray( *pShared->GetCode() );
        bRefChanged = true;
        pCode->Reset();
        while( (t = static_cast<ScToken*>(pCode->GetNextReference())) != NULL )
        {
            if( t->GetType() != svIndex )
            {
                SingleDoubleRefModifier aMod(*t);
                ScComplexRefData& rRef = aMod.Ref();
                ScRange aAbs = rRef.toAbs(aPos);
                bool bMod = (ScRefUpdate::UpdateGrow(rArea, nGrowX, nGrowY, aAbs) != UR_NOTHING);
                if (bMod)
                    rRef.SetRange(aAbs, aPos);
            }
        }
    }

    if (bRefChanged)
    {
        bCompile = true;
        CompileTokenArray(); 
        SetDirty();
    }
    else
        StartListeningTo( pDocument ); 
}

static void lcl_FindRangeNamesInUse(std::set<sal_uInt16>& rIndexes, ScTokenArray* pCode, ScRangeName* pNames)
{
    for (FormulaToken* p = pCode->First(); p; p = pCode->Next())
    {
        if (p->GetOpCode() == ocName)
        {
            sal_uInt16 nTokenIndex = p->GetIndex();
            rIndexes.insert( nTokenIndex );

            ScRangeData* pSubName = pNames->findByIndex(p->GetIndex());
            if (pSubName)
                lcl_FindRangeNamesInUse(rIndexes, pSubName->GetCode(), pNames);
        }
    }
}

void ScFormulaCell::FindRangeNamesInUse(std::set<sal_uInt16>& rIndexes) const
{
    lcl_FindRangeNamesInUse( rIndexes, pCode, pDocument->GetRangeName() );
}

bool ScFormulaCell::IsSubTotal() const
{
    return bSubTotal;
}

bool ScFormulaCell::IsChanged() const
{
    return bChanged;
}

void ScFormulaCell::SetChanged(bool b)
{
    bChanged = b;
}

sal_uInt8 ScFormulaCell::GetMatrixFlag() const
{
    return cMatrixFlag;
}

ScTokenArray* ScFormulaCell::GetCode()
{
    return pCode;
}

const ScTokenArray* ScFormulaCell::GetCode() const
{
    return pCode;
}

bool ScFormulaCell::IsRunning() const
{
    return bRunning;
}

void ScFormulaCell::SetRunning( bool bVal )
{
    bRunning = bVal;
}

void ScFormulaCell::CompileDBFormula( sc::CompileFormulaContext& rCxt )
{
    for( FormulaToken* p = pCode->First(); p; p = pCode->Next() )
    {
        if ( p->GetOpCode() == ocDBArea
            || (p->GetOpCode() == ocName && p->GetIndex() >= SC_START_INDEX_DB_COLL) )
        {
            bCompile = true;
            CompileTokenArray(rCxt);
            SetDirty();
            break;
        }
    }
}

void ScFormulaCell::CompileDBFormula( sc::CompileFormulaContext& rCxt, bool bCreateFormulaString )
{
    
    
    
    if ( bCreateFormulaString )
    {
        bool bRecompile = false;
        pCode->Reset();
        for ( FormulaToken* p = pCode->First(); p && !bRecompile; p = pCode->Next() )
        {
            switch ( p->GetOpCode() )
            {
                case ocBad:             
                case ocColRowName:      
                case ocDBArea:          
                    bRecompile = true;
                break;
                case ocName:
                    if ( p->GetIndex() >= SC_START_INDEX_DB_COLL )
                        bRecompile = true;  
                break;
                default:
                    ; 
            }
        }
        if ( bRecompile )
        {
            OUString aFormula = GetFormula(rCxt);
            if ( GetMatrixFlag() != MM_NONE && !aFormula.isEmpty() )
            {
                if ( aFormula[ aFormula.getLength()-1 ] == '}' )
                    aFormula = aFormula.copy( 0, aFormula.getLength()-1 );
                if ( aFormula[0] == '{' )
                    aFormula = aFormula.copy( 1 );
            }
            EndListeningTo( pDocument );
            pDocument->RemoveFromFormulaTree( this );
            pCode->Clear();
            SetHybridFormula(aFormula, rCxt.getGrammar());
        }
    }
    else if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        rCxt.setGrammar(eTempGrammar);
        Compile(rCxt, aResult.GetHybridFormula(), false);
        aResult.SetToken( NULL);
        SetDirty();
    }
}

void ScFormulaCell::CompileNameFormula( sc::CompileFormulaContext& rCxt, bool bCreateFormulaString )
{
    
    
    
    if ( bCreateFormulaString )
    {
        bool bRecompile = false;
        pCode->Reset();
        for ( FormulaToken* p = pCode->First(); p && !bRecompile; p = pCode->Next() )
        {
            switch ( p->GetOpCode() )
            {
                case ocBad:             
                case ocColRowName:      
                    bRecompile = true;
                break;
                default:
                    if ( p->GetType() == svIndex )
                        bRecompile = true;  
            }
        }
        if ( bRecompile )
        {
            OUString aFormula = GetFormula(rCxt);
            if ( GetMatrixFlag() != MM_NONE && !aFormula.isEmpty() )
            {
                if ( aFormula[ aFormula.getLength()-1 ] == '}' )
                    aFormula = aFormula.copy( 0, aFormula.getLength()-1 );
                if ( aFormula[0] == '{' )
                    aFormula = aFormula.copy( 1 );
            }
            EndListeningTo( pDocument );
            pDocument->RemoveFromFormulaTree( this );
            pCode->Clear();
            SetHybridFormula(aFormula, rCxt.getGrammar());
        }
    }
    else if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        rCxt.setGrammar(eTempGrammar);
        Compile(rCxt, aResult.GetHybridFormula(), false);
        aResult.SetToken( NULL);
        SetDirty();
    }
}

void ScFormulaCell::CompileColRowNameFormula( sc::CompileFormulaContext& rCxt )
{
    pCode->Reset();
    for ( FormulaToken* p = pCode->First(); p; p = pCode->Next() )
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

ScFormulaCell*  ScFormulaCell::GetPrevious() const                 { return pPrevious; }
ScFormulaCell*  ScFormulaCell::GetNext() const                     { return pNext; }
void            ScFormulaCell::SetPrevious( ScFormulaCell* pF )    { pPrevious = pF; }
void            ScFormulaCell::SetNext( ScFormulaCell* pF )        { pNext = pF; }
ScFormulaCell*  ScFormulaCell::GetPreviousTrack() const                { return pPreviousTrack; }
ScFormulaCell*  ScFormulaCell::GetNextTrack() const                    { return pNextTrack; }
void            ScFormulaCell::SetPreviousTrack( ScFormulaCell* pF )   { pPreviousTrack = pF; }
void            ScFormulaCell::SetNextTrack( ScFormulaCell* pF )       { pNextTrack = pF; }

ScFormulaCellGroupRef ScFormulaCell::CreateCellGroup( SCROW nLen, bool bInvariant )
{
    if (mxGroup)
    {
        
        
        SAL_INFO("sc.opencl", "You can't create a new group if the cell is already a part of a group");
        return ScFormulaCellGroupRef();
    }

    mxGroup.reset(new ScFormulaCellGroup);
    mxGroup->mpTopCell = this;
    mxGroup->mbInvariant = bInvariant;
    mxGroup->mnLength = nLen;
    mxGroup->mpCode = pCode; 
    if (mxGroup->sxCompilationThread.is())
        mxGroup->scheduleCompilation();
    return mxGroup;
}

ScFormulaCellGroupRef ScFormulaCell::GetCellGroup()
{
    return mxGroup;
}

void ScFormulaCell::SetCellGroup( const ScFormulaCellGroupRef &xRef )
{
    if (!xRef)
    {
        
        if (mxGroup)
            pCode = mxGroup->mpCode->Clone();

        mxGroup = xRef;
        return;
    }

    
    if (!mxGroup)
        
        delete pCode;

    mxGroup = xRef;
    pCode = mxGroup->mpCode;
}

ScFormulaCell::CompareState ScFormulaCell::CompareByTokenArray( ScFormulaCell& rOther ) const
{
    
    if ( GetMatrixFlag() != MM_NONE )
        return NotEqual;

    
    if ( GetHash() != rOther.GetHash() )
        return NotEqual;

    FormulaToken **pThis = pCode->GetCode();
    sal_uInt16     nThisLen = pCode->GetCodeLen();
    FormulaToken **pOther = rOther.pCode->GetCode();
    sal_uInt16     nOtherLen = rOther.pCode->GetCodeLen();

    if ( !pThis || !pOther )
    {
        
        return NotEqual;
    }

    if ( nThisLen != nOtherLen )
        return NotEqual;

    bool bInvariant = true;

    
    for ( sal_uInt16 i = 0; i < nThisLen; i++ )
    {
        ScToken *pThisTok = static_cast<ScToken*>( pThis[i] );
        ScToken *pOtherTok = static_cast<ScToken*>( pOther[i] );

        if ( pThisTok->GetType() != pOtherTok->GetType() ||
             pThisTok->GetOpCode() != pOtherTok->GetOpCode() ||
             pThisTok->GetParamCount() != pOtherTok->GetParamCount() )
        {
            
            return NotEqual;
        }

        switch (pThisTok->GetType())
        {
            case formula::svMatrix:
            case formula::svExternalSingleRef:
            case formula::svExternalDoubleRef:
                
                return NotEqual;

            case formula::svSingleRef:
            {
                
                const ScSingleRefData& rRef = pThisTok->GetSingleRef();
                if (rRef != pOtherTok->GetSingleRef())
                    return NotEqual;

                if (rRef.IsRowRel())
                    bInvariant = false;
            }
            break;
            case formula::svDoubleRef:
            {
                
                const ScSingleRefData& rRef1 = pThisTok->GetSingleRef();
                const ScSingleRefData& rRef2 = pThisTok->GetSingleRef2();
                if (rRef1 != pOtherTok->GetSingleRef())
                    return NotEqual;

                if (rRef2 != pOtherTok->GetSingleRef2())
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
                if(pThisTok->GetIndex() != pOtherTok->GetIndex())
                    return NotEqual;
            }
            break;
            case formula::svByte:
            {
                if(pThisTok->GetByte() != pOtherTok->GetByte())
                    return NotEqual;
            }
            break;
            default:
                ;
        }
    }

    return bInvariant ? EqualInvariant : EqualRelativeRef;
}

bool ScFormulaCell::InterpretFormulaGroup()
{
    if (!ScInterpreter::GetGlobalConfig().mbOpenCLEnabled)
        return false;

    if (!mxGroup || !pCode)
        return false;

    if (mxGroup->meCalcState == sc::GroupCalcDisabled)
        return false;

    switch (pCode->GetVectorState())
    {
        case FormulaVectorEnabled:
        case FormulaVectorCheckReference:
            
        break;
        case FormulaVectorDisabled:
        case FormulaVectorUnknown:
        default:
            
            return false;
    }

    
    
    if (mxGroup->mbInvariant && false)
        return InterpretInvariantFormulaGroup();

    if (mxGroup->meCalcState == sc::GroupCalcEnabled)
    {
        ScTokenArray aCode;
        ScAddress aTopPos = aPos;
        aTopPos.SetRow(mxGroup->mpTopCell->aPos.Row());
        ScGroupTokenConverter aConverter(aCode, *pDocument, *this, mxGroup->mpTopCell->aPos);
        if (!aConverter.convert(*pCode))
        {
            SAL_INFO("sc.opencl", "conversion of group " << this << " failed, disabling");
            mxGroup->meCalcState = sc::GroupCalcDisabled;
            return false;
        }
        mxGroup->meCalcState = sc::GroupCalcRunning;
        if (!sc::FormulaGroupInterpreter::getStatic()->interpret(*pDocument, mxGroup->mpTopCell->aPos, mxGroup, aCode))
        {
            SAL_INFO("sc.opencl", "interpreting group " << mxGroup << " (state " << mxGroup->meCalcState << ") failed, disabling");
            mxGroup->meCalcState = sc::GroupCalcDisabled;
            return false;
        }
        mxGroup->meCalcState = sc::GroupCalcEnabled;
    }
    else
    {
        ScTokenArray aDummy;
        if (!sc::FormulaGroupInterpreter::getStatic()->interpret(*pDocument, mxGroup->mpTopCell->aPos, mxGroup, aDummy))
        {
            SAL_INFO("sc.opencl", "interpreting group " << mxGroup << " (state " << mxGroup->meCalcState << ") failed, disabling");
            mxGroup->meCalcState = sc::GroupCalcDisabled;
            return false;
        }
    }

    return true;
}

bool ScFormulaCell::InterpretInvariantFormulaGroup()
{
    if (pCode->GetVectorState() == FormulaVectorCheckReference)
    {
        
        

        ScTokenArray aCode;
        pCode->Reset();
        for (const formula::FormulaToken* p = pCode->First(); p; p = pCode->Next())
        {
            const ScToken* pToken = static_cast<const ScToken*>(p);
            switch (pToken->GetType())
            {
                case svSingleRef:
                {
                    ScSingleRefData aRef = pToken->GetSingleRef();
                    ScAddress aRefPos = aRef.toAbs(aPos);
                    formula::FormulaTokenRef pNewToken = pDocument->ResolveStaticReference(aRefPos);
                    if (!pNewToken)
                        return false;

                    aCode.AddToken(*pNewToken);
                }
                break;
                case svDoubleRef:
                {
                    ScComplexRefData aRef = pToken->GetDoubleRef();
                    ScRange aRefRange = aRef.toAbs(aPos);
                    formula::FormulaTokenRef pNewToken = pDocument->ResolveStaticReference(aRefRange);
                    if (!pNewToken)
                        return false;

                    aCode.AddToken(*pNewToken);
                }
                break;
                default:
                    aCode.AddToken(*pToken);
            }
        }

        ScCompiler aComp(pDocument, aPos, aCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        aComp.CompileTokenArray(); 
        ScInterpreter aInterpreter(this, pDocument, aPos, aCode);
        aInterpreter.Interpret();
        aResult.SetToken(aInterpreter.GetResultToken().get());
    }
    else
    {
        
        ScInterpreter aInterpreter(this, pDocument, aPos, *pCode);
        aInterpreter.Interpret();
        aResult.SetToken(aInterpreter.GetResultToken().get());
    }

    for ( sal_Int32 i = 0; i < mxGroup->mnLength; i++ )
    {
        ScAddress aTmpPos = aPos;
        aTmpPos.SetRow(mxGroup->mpTopCell->aPos.Row() + i);
        ScFormulaCell* pCell = pDocument->GetFormulaCell(aTmpPos);
        assert( pCell != NULL );

        
        

        
        pCell->aResult = aResult;
        pCell->ResetDirty();
        pCell->SetChanged(true);
    }

    return true;
}

namespace {

void startListeningArea(
    ScFormulaCell* pCell, ScDocument& rDoc, const ScAddress& rPos, const ScToken& rToken)
{
    const ScSingleRefData& rRef1 = rToken.GetSingleRef();
    const ScSingleRefData& rRef2 = rToken.GetSingleRef2();
    ScAddress aCell1 = rRef1.toAbs(rPos);
    ScAddress aCell2 = rRef2.toAbs(rPos);
    if (aCell1.IsValid() && aCell2.IsValid())
    {
        if (rToken.GetOpCode() == ocColRowNameAuto)
        {   
            if ( rRef1.IsColRel() )
            {   
                aCell2.SetRow(MAXROW);
            }
            else
            {   
                aCell2.SetCol(MAXCOL);
            }
        }
        rDoc.StartListeningArea(ScRange(aCell1, aCell2), pCell);
    }
}

}

void ScFormulaCell::StartListeningTo( ScDocument* pDoc )
{
    if (pDoc->IsClipOrUndo() || pDoc->GetNoListening() || IsInChangeTrack())
        return;

    pDoc->SetDetectiveDirty(true);  

    ScTokenArray* pArr = GetCode();
    if( pArr->IsRecalcModeAlways() )
    {
        pDoc->StartListeningArea(BCA_LISTEN_ALWAYS, this);
        SetNeedsListening( false);
        return;
    }

    pArr->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pArr->GetNextReferenceRPN()) ) != NULL )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell =  t->GetSingleRef().toAbs(aPos);
                if (aCell.IsValid())
                    pDoc->StartListeningCell(aCell, this);
            }
            break;
            case svDoubleRef:
                startListeningArea(this, *pDoc, aPos, *t);
            break;
            default:
                ;   
        }
    }
    SetNeedsListening( false);
}

void ScFormulaCell::StartListeningTo( sc::StartListeningContext& rCxt )
{
    ScDocument& rDoc = rCxt.getDoc();

    if (rDoc.IsClipOrUndo() || rDoc.GetNoListening() || IsInChangeTrack())
        return;

    rDoc.SetDetectiveDirty(true);  

    ScTokenArray* pArr = GetCode();
    if( pArr->IsRecalcModeAlways() )
    {
        rDoc.StartListeningArea(BCA_LISTEN_ALWAYS, this);
        SetNeedsListening( false);
        return;
    }

    pArr->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pArr->GetNextReferenceRPN()) ) != NULL )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell = t->GetSingleRef().toAbs(aPos);
                if (aCell.IsValid())
                    rDoc.StartListeningCell(rCxt, aCell, *this);
            }
            break;
            case svDoubleRef:
                startListeningArea(this, rDoc, aPos, *t);
            break;
            default:
                ;   
        }
    }
    SetNeedsListening( false);
}

namespace {

void endListeningArea(
    ScFormulaCell* pCell, ScDocument& rDoc, const ScAddress& rPos, const ScToken& rToken)
{
    const ScSingleRefData& rRef1 = rToken.GetSingleRef();
    const ScSingleRefData& rRef2 = rToken.GetSingleRef2();
    ScAddress aCell1 = rRef1.toAbs(rPos);
    ScAddress aCell2 = rRef2.toAbs(rPos);
    if (aCell1.IsValid() && aCell2.IsValid())
    {
        if (rToken.GetOpCode() == ocColRowNameAuto)
        {   
            if ( rRef1.IsColRel() )
            {   
                aCell2.SetRow(MAXROW);
            }
            else
            {   
                aCell2.SetCol(MAXCOL);
            }
        }

        rDoc.EndListeningArea(ScRange(aCell1, aCell2), pCell);
    }
}

}

void ScFormulaCell::EndListeningTo( ScDocument* pDoc, ScTokenArray* pArr,
        ScAddress aCellPos )
{
    if (pDoc->IsClipOrUndo() || IsInChangeTrack())
        return;

    pDoc->SetDetectiveDirty(true);  

    if ( GetCode()->IsRecalcModeAlways() )
    {
        pDoc->EndListeningArea( BCA_LISTEN_ALWAYS, this );
        return;
    }

    if (!pArr)
    {
        pArr = GetCode();
        aCellPos = aPos;
    }
    pArr->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pArr->GetNextReferenceRPN()) ) != NULL )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell = t->GetSingleRef().toAbs(aPos);
                if (aCell.IsValid())
                    pDoc->EndListeningCell(aCell, this);
            }
            break;
            case svDoubleRef:
                endListeningArea(this, *pDoc, aCellPos, *t);
            break;
            default:
                ;   
        }
    }
}

void ScFormulaCell::EndListeningTo( sc::EndListeningContext& rCxt )
{
    if (rCxt.getDoc().IsClipOrUndo() || IsInChangeTrack())
        return;

    ScDocument& rDoc = rCxt.getDoc();
    rDoc.SetDetectiveDirty(true);  

    if (pCode->IsRecalcModeAlways())
    {
        rDoc.EndListeningArea(BCA_LISTEN_ALWAYS, this);
        return;
    }

    pCode->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pCode->GetNextReferenceRPN()) ) != NULL )
    {
        switch (t->GetType())
        {
            case svSingleRef:
            {
                ScAddress aCell = t->GetSingleRef().toAbs(aPos);
                if (aCell.IsValid())
                    rDoc.EndListeningCell(rCxt, aCell, *this);
            }
            break;
            case svDoubleRef:
                endListeningArea(this, rDoc, aPos, *t);
            break;
            default:
                ;   
        }
    }
}

bool ScFormulaCell::IsShared() const
{
    return mxGroup.get() != NULL;
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

ScTokenArray* ScFormulaCell::GetSharedCode()
{
    return mxGroup ? mxGroup->mpCode : NULL;
}

const ScTokenArray* ScFormulaCell::GetSharedCode() const
{
    return mxGroup ? mxGroup->mpCode : NULL;
}

bool ScFormulaCell::IsPostponedDirty() const
{
    return mbPostponedDirty;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
