/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "formulacell.hxx"

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

#include "formula/errorcodes.hxx"
#include "formula/vectortoken.hxx"
#include "svl/intitem.hxx"
#include "rtl/strbuf.hxx"
#include "formulagroup.hxx"
#include "listenercontext.hxx"
#include "types.hxx"
#include "scopetools.hxx"

#include <boost/scoped_ptr.hpp>

using namespace formula;

#ifdef USE_MEMPOOL
IMPL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
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

    // Test if exactly two dimensions are equal
    if (!(bSameCols ^ bSameRows ^ bSameTabs)
            && (bSameCols || bSameRows || bSameTabs))
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

void adjustRangeName(ScToken* pToken, ScDocument& rNewDoc, const ScDocument* pOldDoc, const ScAddress& aNewPos, const ScAddress& aOldPos)
{
    bool bOldGlobal = pToken->IsGlobal();
    SCTAB aOldTab = aOldPos.Tab();
    OUString aRangeName;
    int nOldIndex = pToken->GetIndex();
    ScRangeData* pOldRangeData = NULL;

    //search the name of the RangeName
    if (!bOldGlobal)
    {
        pOldRangeData = pOldDoc->GetRangeName(aOldTab)->findByIndex(nOldIndex);
        if (!pOldRangeData)
            return;     //might be an error in the formula array
        aRangeName = pOldRangeData->GetUpperName();
    }
    else
    {
        pOldRangeData = pOldDoc->GetRangeName()->findByIndex(nOldIndex);
        if (!pOldRangeData)
            return;     //might be an error in the formula array
        aRangeName = pOldRangeData->GetUpperName();
    }

    //find corresponding range name in new document
    //first search for local range name then global range names
    SCTAB aNewTab = aNewPos.Tab();
    ScRangeName* pRangeName = rNewDoc.GetRangeName(aNewTab);
    ScRangeData* pRangeData = NULL;
    bool bNewGlobal = false;
    //search local range names
    if (pRangeName)
    {
        pRangeData = pRangeName->findByUpperName(aRangeName);
    }
    //search global range names
    if (!pRangeData)
    {
        bNewGlobal = true;
        pRangeName = rNewDoc.GetRangeName();
        if (pRangeName)
            pRangeData = pRangeName->findByUpperName(aRangeName);
    }
    //if no range name was found copy it
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
            //if this happened we have a real problem
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
        aNewNamedDBs.insert(pNewDBData);
    }
    pToken->SetIndex(pNewDBData->GetIndex());
}

}

ScFormulaCellGroup::ScFormulaCellGroup() :
    mnRefCount(0),
    mnStart(0),
    mnLength(0),
    mbInvariant(false),
    meCalcState(sc::GroupCalcEnabled)
{
}

ScFormulaCellGroup::~ScFormulaCellGroup()
{
}

// ============================================================================

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
    aPos( rPos )
{
    Compile( rFormula, true, eGrammar );    // bNoListening, Insert does that
    if (!pCode)
        // We need to have a non-NULL token array instance at all times.
        pCode = new ScTokenArray;
}

// Used by import filters

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const ScTokenArray* pArr,
                              const FormulaGrammar::Grammar eGrammar, sal_uInt8 cInd ) :
    eTempGrammar( eGrammar),
    pCode( pArr ? new ScTokenArray( *pArr ) : new ScTokenArray ),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nSeenInIteration(0),
    cMatrixFlag ( cInd ),
    nFormatType ( NUMBERFORMAT_NUMBER ),
    bDirty( NULL != pArr ), // -> Because of the use of the Auto Pilot Function was: cInd != 0
    bChanged( false ),
    bRunning( false ),
    bCompile( false ),
    bSubTotal( false ),
    bIsIterCell( false ),
    bInChangeTrack( false ),
    bTableOpDirty( false ),
    bNeedListening( false ),
    mbNeedsNumberFormat( false ),
    aPos( rPos )
{
    // UPN-Array generation
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
    aPos( rPos )
{
    pCode = rCell.pCode->Clone();

    //  set back any errors and recompile
    //  not in the Clipboard - it must keep the received error flag
    //  Special Length=0: as bad cells are generated, then they are also retained
    if ( pCode->GetCodeError() && !pDocument->IsClipboard() && pCode->GetLen() )
    {
        pCode->SetCodeError( 0 );
        bCompile = true;
    }
    //! Compile ColRowNames on URM_MOVE/URM_COPY _after_ UpdateReference
    bool bCompileLater = false;
    bool bClipMode = rCell.pDocument->IsClipboard();

    //update ScNameTokens
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
    {   // Name references with references and ColRowNames
        pCode->Reset();
        ScToken* t;
        while ( ( t = static_cast<ScToken*>(pCode->GetNextReferenceOrName()) ) != NULL && !bCompile )
        {
            if ( t->IsExternalRef() )
            {
                // External name, cell, and area references.
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
            // ColRowNames need new lookup after positions are adjusted.
            bCompileLater = pCode->HasOpCode( ocRange) || pCode->HasOpCode( ocColRowName);
        }
        if ( !bCompileLater )
        {
            // bNoListening, not at all if in Clipboard/Undo,
            // and not from Clipboard either, instead after Insert(Clone) and UpdateReference.
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
        // Reference to another cell that contains a matrix formula.
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

    sal_Unicode ch('=');
    rBuffer.insert( 0, &ch, 1 );
    if( cMatrixFlag )
    {
        sal_Unicode ch2('{');
        rBuffer.insert( 0, &ch2, 1);
        rBuffer.append( sal_Unicode('}'));
    }
}

void ScFormulaCell::GetFormula( OUString& rFormula, const FormulaGrammar::Grammar eGrammar ) const
{
    OUStringBuffer rBuffer( rFormula );
    GetFormula( rBuffer, eGrammar );
    rFormula = rBuffer.makeStringAndClear();
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
void ScFormulaCell::ResetDirty() { bDirty = false; }
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
    // pCode may not deleted for queries, but must be empty
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


void ScFormulaCell::CompileTokenArray( bool bNoListening )
{
    // Not already compiled?
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
        Compile( aResult.GetHybridFormula(), bNoListening, eTempGrammar);
    else if( bCompile && !pDocument->IsClipOrUndo() && !pCode->GetCodeError() )
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


void ScFormulaCell::CompileXML( ScProgress& rProgress )
{
    if ( cMatrixFlag == MM_REFERENCE )
    {   // is already token code via ScDocFunc::EnterMatrix, ScDocument::InsertMatrixFormula
        // just establish listeners
        StartListeningTo( pDocument );
        return ;
    }

    ScCompiler aComp( pDocument, aPos, *pCode);
    aComp.SetGrammar(eTempGrammar);
    OUString aFormula, aFormulaNmsp;
    aComp.CreateStringFromXMLTokenArray( aFormula, aFormulaNmsp );
    pDocument->DecXMLImportedFormulaCount( aFormula.getLength() );
    rProgress.SetStateCountDownOnPercent( pDocument->GetXMLImportedFormulaCount() );
    // pCode may not deleted for queries, but must be empty
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
        pDocument->PutInFormulaTree(this);
    }
}


void ScFormulaCell::CalcAfterLoad()
{
    bool bNewCompiled = false;
    // If a Calc 1.0-doc is read, we have a result, but no token array
    if( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        Compile( aResult.GetHybridFormula(), true, eTempGrammar);
        aResult.SetToken( NULL);
        bDirty = true;
        bNewCompiled = true;
    }
    // The UPN array is not created when a Calc 3.0-Doc has been read as the Range Names exist until now.
    if( pCode->GetLen() && !pCode->GetCodeLen() && !pCode->GetCodeError() )
    {
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
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
        aResult.SetResultError( errIllegalFPOperation );
        bDirty = true;
    }

    // DoubleRefs for binary operators were always a Matrix before version v5.0.
    // Now this is only the case when when in an array formula, otherwise it's an implicit intersection
    if ( pDocument->GetSrcVersion() < SC_MATRIX_DOUBLEREF &&
            GetMatrixFlag() == MM_NONE && pCode->HasMatrixDoubleRefOps() )
    {
        cMatrixFlag = MM_FORMULA;
        SetMatColsRows( 1, 1);
    }

    // Do the cells need to be calculated? After Load cells can contain an error code, and then start
    // the listener and Recalculate (if needed) if not RECALCMODE_NORMAL
    if( !bNewCompiled || !pCode->GetCodeError() )
    {
        StartListeningTo( pDocument );
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
    return pCode && pDocument->MarkUsedExternalReferences( *pCode);
}


void ScFormulaCell::Interpret()
{
    if (!IsDirtyOrInTableOpDirty() || pDocument->GetRecursionHelper().IsInReturn())
        return;     // no double/triple processing

    //! HACK:
    //  If the call originates from a Reschedule in DdeLink update, leave dirty
    //  Better: Do a Dde Link Update without Reschedule or do it completely asynchronously!
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

        // Start or add to iteration list.
        if (!pDocument->GetRecursionHelper().IsDoingIteration() ||
                !pDocument->GetRecursionHelper().GetRecursionInIterationStack().top()->bIsIterCell)
            pDocument->GetRecursionHelper().SetInIterationReturn( true);

        return;
    }
    // no multiple interprets for GetErrCode, IsValue, GetValue and
    // different entry point recursions. Would also lead to premature
    // convergence in iterations.
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
                ScFormulaCell* pIterCell = this; // scope for debug convenience
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
                        pIterCell = (*aIter).pCell;
                        pIterCell->bIsIterCell = true;
                    }
                    // Mark older cells dirty again, in case they converted
                    // without accounting for all remaining cells in the circle
                    // that weren't touched so far, e.g. conditional. Restore
                    // backuped result.
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
                    // Close circle once.
                    rRecursionHelper.GetList().back().pCell->InterpretTail(
                            SCITP_CLOSE_ITERATION_CIRCLE);
                    // Start at 1, init things.
                    rRecursionHelper.StartIteration();
                    // Mark all cells being in iteration.
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
                            // If one cell didn't converge, all cells of this
                            // circular dependency don't, no matter whether
                            // single cells did.
                            pIterCell->bDirty = false;
                            pIterCell->bTableOpDirty = false;
                            pIterCell->aResult.SetResultError( errNoConvergence);
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
        // #i11719# no UPN and no error and no token code but result string present
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
            pCode->SetCodeError( errNoCode );
            // This is worth an assertion; if encountered in daily work
            // documents we might need another solution. Or just confirm correctness.
            OSL_FAIL( "ScFormulaCell::Interpret: no UPN, no error, no token, but hybrid formula string" );
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
        {   // Only the first time
            // With bChanged=false, if a newly compiled cell has a result of
            // 0.0, no change is detected and the cell will not be repainted.
            // bChanged = false;
            aResult.SetResultError( 0 );
        }

        switch ( aResult.GetResultError() )
        {
            case errCircularReference :     // will be determined again if so
                aResult.SetResultError( 0 );
            break;
        }

        bool bOldRunning = bRunning;
        bRunning = true;
        p->Interpret();
        if (pDocument->GetRecursionHelper().IsInReturn() && eTailParam != SCITP_CLOSE_ITERATION_CIRCLE)
        {
            if (nSeenInIteration > 0)
                --nSeenInIteration;     // retry when iteration is resumed
            return;
        }
        bRunning = bOldRunning;

        // #i102616# For single-sheet saving consider only content changes, not format type,
        // because format type isn't set on loading (might be changed later)
        bool bContentChanged = false;

        // Do not create a HyperLink() cell if the formula results in an error.
        if( p->GetError() && pCode->IsHyperLink())
            pCode->SetHyperLink(false);

        if( p->GetError() && p->GetError() != errCircularReference)
        {
            bDirty = false;
            bTableOpDirty = false;
            bChanged = true;
        }
        if (eTailParam == SCITP_FROM_ITERATION && IsDirtyOrInTableOpDirty())
        {
            bool bIsValue = aResult.IsValue();  // the previous type
            // Did it converge?
            if ((bIsValue && p->GetResultType() == svDouble && fabs(
                            p->GetNumResult() - aResult.GetDouble()) <=
                        pDocument->GetDocOptions().GetIterEps()) ||
                    (!bIsValue && p->GetResultType() == svString &&
                     p->GetStringResult() == aResult.GetString()))
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
                    bDirty = false;
                    bTableOpDirty = false;
                }
            }
        }

        // New error code?
        if( p->GetError() != nOldErrCode )
        {
            bChanged = true;
            // bContentChanged only has to be set if the file content would be changed
            if ( aResult.GetCellResultType() != svUnknown )
                bContentChanged = true;
        }

        if( mbNeedsNumberFormat )
        {
            nFormatType = p->GetRetFormatType();
            sal_Int32 nFormatIndex = p->GetRetFormatIndex();

            // don't set text format as hard format
            if(nFormatType == NUMBERFORMAT_TEXT)
                nFormatIndex = 0;
            else if((nFormatIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
                nFormatIndex = ScGlobal::GetStandardFormat(*pDocument->GetFormatTable(),
                        nFormatIndex, nFormatType);

            // set number format explicitly
            pDocument->SetNumberFormat( aPos, nFormatIndex );

            bChanged = true;
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
                ScFormulaResult aNewResult( p->GetResultToken().get());
                StackVar eOld = aResult.GetCellResultType();
                StackVar eNew = aNewResult.GetCellResultType();
                if ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) )
                {
                    // ScXMLTableRowCellContext::EndElement doesn't call SetFormulaResultDouble for 0
                    // -> no change
                }
                else
                {
                    if ( eOld == svHybridCell || eOld == svHybridValueCell )     // string result from SetFormulaResultString?
                        eOld = svString;            // ScHybridCellToken has a valid GetString method

                    // #i106045# use approxEqual to compare with stored value
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

            // #i102616# handle special cases of initial results after loading (only if the sheet is still marked unchanged)
            if ( bChanged && !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                if ( ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) ) ||
                     ( (eOld == svHybridCell || eOld == svHybridValueCell) && eNew == svString && aResult.GetString() == aNewResult.GetString() ) ||
                     ( eOld == svDouble && eNew == svDouble && rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble() ) ) )
                {
                    // no change, see above
                }
                else
                    bContentChanged = true;
            }

            aResult.Assign( aNewResult);
        }

        // Precision as shown?
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
            bDirty = false;
            bTableOpDirty = false;
        }
        if( aResult.GetMatrix() )
        {
            // If the formula wasn't entered as a matrix formula, live on with
            // the upper left corner and let reference counting delete the matrix.
            if( cMatrixFlag != MM_FORMULA && !pCode->IsHyperLink() )
                aResult.SetToken( aResult.GetCellResultToken().get());
        }
        if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
        {
            // Coded double error may occur via filter import.
            sal_uInt16 nErr = GetDoubleErrorValue( aResult.GetDouble());
            aResult.SetResultError( nErr);
            bChanged = bContentChanged = true;
        }

        if (bContentChanged && pDocument->IsStreamValid(aPos.Tab()))
        {
            // pass bIgnoreLock=true, because even if called from pending row height update,
            // a changed result must still reset the stream flag
            pDocument->SetStreamValid(aPos.Tab(), false, true);
        }
        if ( !pCode->IsRecalcModeAlways() )
            pDocument->RemoveFromFormulaTree( this );

        //  FORCED cells also immediately tested for validity (start macro possibly)

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

        // Reschedule slows the whole thing down considerably, thus only execute on percent change
        ScProgress::GetInterpretProgress()->SetStateCountDownOnPercent(
            pDocument->GetFormulaCodeInTree()/MIN_NO_CODES_PER_PROGRESS_UPDATE );

        switch (p->GetVolatileType())
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
        // Cells with compiler errors should not be marked dirty forever
        OSL_ENSURE( pCode->GetCodeError(), "no UPN-Code und no errors ?!?!" );
        bDirty = false;
        bTableOpDirty = false;
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
        // Setting the new token actually forces an empty result at this top
        // left cell, so have that recalculated.
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
            // Don't remove from FormulaTree to put in FormulaTrack to
            // put in FormulaTree again and again, only if necessary.
            // Any other means except RECALCMODE_ALWAYS by which a cell could
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

void ScFormulaCell::SetDirty( bool bDirtyFlag )
{
    if ( !IsInChangeTrack() )
    {
        if ( pDocument->GetHardRecalcState() )
            SetDirtyVar();
        else
        {
            // Multiple Formulas avoid tracking in Load and Copy compileAll
            // by Scenario and Copy Block From Clip.
            // If unconditional required Formula tracking is set before SetDirty
            // bDirty = false, eg in CompileTokenArray
            if ( !bDirty || !pDocument->IsInFormulaTree( this ) )
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
    if (xGroup)
        xGroup->meCalcState = sc::GroupCalcEnabled;

    // mark the sheet of this cell to be calculated
    //#FIXME do we need to revert this remnant of old fake vba events? pDocument->AddCalculateTable( aPos.Tab() );
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

double ScFormulaCell::GetResultDouble() const
{
    return aResult.GetDouble();
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
    // Hard set errors are transported as result type value per convention,
    // e.g. via clipboard. ScFormulaResult::IsValue() and
    // ScFormulaResult::GetDouble() handle that.
    aResult.SetResultError( n );
}

void ScFormulaCell::AddRecalcMode( ScRecalcMode nBits )
{
    if ( (nBits & RECALCMODE_EMASK) != RECALCMODE_NORMAL )
        SetDirtyVar();
    if ( nBits & RECALCMODE_ONLOAD_ONCE )
    {   // OnLoadOnce nur zum Dirty setzen nach Filter-Import
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

// Dynamically create the URLField on a mouse-over action on a hyperlink() cell.
void ScFormulaCell::GetURLResult( OUString& rURL, OUString& rCellText )
{
    OUString aCellString;

    Color* pColor;

    // Cell Text uses the Cell format while the URL uses
    // the default format for the type.
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
        aCellString = GetString();
        pFormatter->GetOutputString( aCellString, nCellFormat, rCellText, &pColor );
    }
    ScConstMatrixRef xMat( aResult.GetMatrix());
    if (xMat)
    {
        // determine if the matrix result is a string or value.
        if (!xMat->IsValue(0, 1))
            rURL = xMat->GetString(0, 1);
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

double ScFormulaCell::GetValueAlways()
{
    // for goal seek: return result value even if error code is set
    MaybeInterpret();
    return aResult.GetDouble();
}

OUString ScFormulaCell::GetString()
{
    MaybeInterpret();
    if ((!pCode->GetCodeError() || pCode->GetCodeError() == errDoubleRef) &&
            !aResult.GetResultError())
        return aResult.GetString();
    return OUString();
}

const ScMatrix* ScFormulaCell::GetMatrix()
{
    if ( pDocument->GetAutoCalc() )
    {
        if( IsDirtyOrInTableOpDirty()
        // Was stored !bDirty but an accompanying matrix cell was bDirty?
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
                return 0;               // bad luck..
            if ( aOrg != rOrgPos )
            {   // First time or a different matrix than last time.
                rOrgPos = aOrg;
                const ScFormulaCell* pFCell;
                if ( cMatrixFlag == MM_REFERENCE )
                    pFCell = pDocument->GetFormulaCell(aOrg);
                else
                    pFCell = this;      // this MM_FORMULA
                // There's only one this, don't compare pFCell==this.
                if (pFCell && pFCell->cMatrixFlag == MM_FORMULA)
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
                    OUString aTmp;
                    OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM(
                        "broken Matrix, no MatFormula at origin, Pos: "));
                    aPos.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                    aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_ASCII_US));
                    aMsg.append(RTL_CONSTASCII_STRINGPARAM(", MatOrg: "));
                    aOrg.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                    aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_ASCII_US));
                    OSL_FAIL(aMsg.getStr());
#endif
                    return 0;           // bad luck ...
                }
            }
            // here we are, healthy and clean, somewhere in between
            SCsCOL dC = aPos.Col() - aOrg.Col();
            SCsROW dR = aPos.Row() - aOrg.Row();
            sal_uInt16 nEdges = 0;
            if ( dC >= 0 && dR >= 0 && dC < nC && dR < nR )
            {
                if ( dC == 0 )
                    nEdges |= sc::MatrixEdgeLeft;            // left edge
                if ( dC+1 == nC )
                    nEdges |= sc::MatrixEdgeRight;           // right edge
                if ( dR == 0 )
                    nEdges |= sc::MatrixEdgeTop;            // top edge
                if ( dR+1 == nR )
                    nEdges |= sc::MatrixEdgeBottom;            // bottom edge
                if ( !nEdges )
                    nEdges = sc::MatrixEdgeInside;             // inside
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OUString aTmp;
                OStringBuffer aMsg( "broken Matrix, Pos: " );
                aPos.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8 ));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM(", MatOrg: "));
                aOrg.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                aMsg.append(OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8 ));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM(", MatCols: "));
                aMsg.append(static_cast<sal_Int32>( nC ));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM(", MatRows: "));
                aMsg.append(static_cast<sal_Int32>( nR ));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM(", DiffCols: "));
                aMsg.append(static_cast<sal_Int32>( dC ));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM(", DiffRows: "));
                aMsg.append(static_cast<sal_Int32>( dR ));
                OSL_FAIL( aMsg.makeStringAndClear().getStr());
            }
#endif
            return nEdges;
//            break;
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

bool ScFormulaCell::HasOneReference( ScRange& r ) const
{
    pCode->Reset();
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    if( p && !pCode->GetNextReferenceRPN() )        // only one!
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

    pCode->Reset();
    // Get first reference, if any
    ScToken* const pFirstReference(
            dynamic_cast<ScToken*>(pCode->GetNextReferenceRPN()));
    if (pFirstReference)
    {
        // Collect all consecutive references, starting by the one
        // already found
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

bool ScFormulaCell::UpdateReference(
    UpdateRefMode eUpdateRefMode, const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
    ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    bool bCellStateChanged = false;

    SCCOL nCol1 = rRange.aStart.Col();
    SCROW nRow1 = rRange.aStart.Row();
    SCCOL nCol = aPos.Col();
    SCROW nRow = aPos.Row();
    SCTAB nTab = aPos.Tab();
    ScAddress aUndoPos( aPos );         // position for undo cell in pUndoDoc
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );
    bool bIsInsert = (eUpdateRefMode == URM_INSDEL && nDx >= 0 && nDy >= 0 && nDz >= 0);

    if (eUpdateRefMode == URM_INSDEL && rRange.In(aPos))
    {
        // This formula cell itself is being shifted during cell range
        // insertion or deletion. Update its position.
        aPos.Move(nDx, nDy, nDz);
        if (xGroup && xGroup->mnStart == aOldPos.Row())
            xGroup->mnStart += nDy;

        bCellStateChanged = aPos != aOldPos;
    }
    else if (rRange.In(aPos))
    {
        // The cell is being moved or copied to a new position. I guess the
        // position has been updated prior to this call?  Determine
        // its original position before the move which will be used to adjust
        // relative references later.
        aOldPos.Set( nCol - nDx, nRow - nDy, nTab - nDz );
    }

    bool bHasRefs = false;
    bool bHasColRowNames = false;
    bool bOnRefMove = false;
    if ( !pDocument->IsClipOrUndo() )
    {
        // Check presence of any references or column row names.
        pCode->Reset();
        bHasRefs = (pCode->GetNextReferenceRPN() != NULL);
        if ( !bHasRefs || eUpdateRefMode == URM_COPY )
        {
            pCode->Reset();
            bHasColRowNames = (pCode->GetNextColRowName() != NULL);
            bHasRefs = bHasRefs || bHasColRowNames;
        }
        bOnRefMove = pCode->IsRecalcModeOnRefMove();
    }

    if (!bHasRefs && !bOnRefMove)
        // This formula cell contains no references, nor needs recalculating
        // on reference update. Bail out.
        return bCellStateChanged;

    boost::scoped_ptr<ScTokenArray> pOldCode;
    if (pUndoDoc)
        pOldCode.reset(pCode->Clone());

    ScRangeData* pSharedCode = NULL;
    bool bValChanged = false;
    bool bRangeModified = false;    // any range, not only shared formula
    bool bRefSizeChanged = false;

    if (bHasRefs)
    {
        // Update cell or range references.
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        pSharedCode = aComp.UpdateReference(eUpdateRefMode, aOldPos, rRange,
                                         nDx, nDy, nDz,
                                         bValChanged, bRefSizeChanged);
        bRangeModified = aComp.HasModifiedRange();
    }

    bCellStateChanged |= bValChanged;

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
        bColRowNameCompile =
            (eUpdateRefMode == URM_INSDEL && (nDx > 0 || nDy > 0));

        if ( bColRowNameCompile )
        {
            bColRowNameCompile = false;
            ScToken* t;
            ScRangePairList* pColList = pDocument->GetColNameRanges();
            ScRangePairList* pRowList = pDocument->GetRowNameRanges();
            pCode->Reset();
            while ( !bColRowNameCompile && (t = static_cast<ScToken*>(pCode->GetNextColRowName())) != NULL )
            {
                ScSingleRefData& rRef = t->GetSingleRef();
                if ( nDy > 0 && rRef.IsColRel() )
                {   // ColName
                    ScAddress aAdr = rRef.toAbs(aPos);
                    ScRangePair* pR = pColList->Find( aAdr );
                    if ( pR )
                    {   // defined
                        if ( pR->GetRange(1).aStart.Row() == nRow1 )
                            bColRowNameCompile = true;
                    }
                    else
                    {   // on the fly
                        if (aAdr.Row() + 1 == nRow1)
                            bColRowNameCompile = true;
                    }
                }
                if ( nDx > 0 && rRef.IsRowRel() )
                {   // RowName
                    ScAddress aAdr = rRef.toAbs(aPos);
                    ScRangePair* pR = pRowList->Find( aAdr );
                    if ( pR )
                    {   // defined
                        if ( pR->GetRange(1).aStart.Col() == nCol1 )
                            bColRowNameCompile = true;
                    }
                    else
                    {   // on the fly
                        if (aAdr.Col() + 1 == nCol1)
                            bColRowNameCompile = true;
                    }
                }
            }
        }
        else if ( eUpdateRefMode == URM_MOVE )
        {   // Recomplie for Move/D&D when ColRowName was moved or this Cell
            // points to one and was moved.
            bColRowNameCompile = bCompile;      // Possibly from Copy ctor
            if ( !bColRowNameCompile )
            {
                bool bMoved = (aPos != aOldPos);
                pCode->Reset();
                ScToken* t = static_cast<ScToken*>(pCode->GetNextColRowName());
                if ( t && bMoved )
                    bColRowNameCompile = true;
                while ( t && !bColRowNameCompile )
                {
                    ScSingleRefData& rRef = t->GetSingleRef();
                    ScAddress aAbs = rRef.toAbs(aPos);
                    if (ValidAddress(aAbs))
                    {
                        if (rRange.In(aAbs))
                            bColRowNameCompile = true;
                    }
                    t = static_cast<ScToken*>(pCode->GetNextColRowName());
                }
            }
        }
        else if ( eUpdateRefMode == URM_COPY && bHasColRowNames && bValChanged )
        {
            bColRowNameCompile = true;
        }

        ScChangeTrack* pChangeTrack = pDocument->GetChangeTrack();
        bInDeleteUndo = (pChangeTrack && pChangeTrack->IsInDeleteUndo());

        // RelNameRefs are always moved
        bHasRelName = HasRelNameReference();
        // Reference changed and new listening needed?
        // Except in Insert/Delete without specialties.
        bNewListening = (bRangeModified || pSharedCode || bColRowNameCompile
                || (bValChanged && (eUpdateRefMode != URM_INSDEL ||
                        bInDeleteUndo || bRefSizeChanged)) ||
                (bHasRelName && eUpdateRefMode != URM_COPY))
            // #i36299# Don't duplicate action during cut&paste / drag&drop
            // on a cell in the range moved, start/end listeners is done
            // via ScDocument::DeleteArea() and ScDocument::CopyFromClip().
            && !(eUpdateRefMode == URM_MOVE &&
                    pDocument->IsInsertingFromOtherDoc() && rRange.In(aPos));

        if ( bNewListening )
            EndListeningTo(pDocument, pOldCode.get(), aOldPos);
    }

    bool bNeedDirty = false;
    // NeedDirty for changes except for Copy and Move/Insert without RelNames
    if ( bRangeModified || pSharedCode || bColRowNameCompile ||
            (bValChanged && eUpdateRefMode != URM_COPY &&
             (eUpdateRefMode != URM_MOVE || bHasRelName) &&
             (!bIsInsert || bHasRelName || bInDeleteUndo ||
              bRefSizeChanged)) || bOnRefMove)
        bNeedDirty = true;

    if (pUndoDoc && (bValChanged || pSharedCode || bOnRefMove))
    {
        // Copy the cell to aUndoPos, which is its current position in the document,
        // so this works when UpdateReference is called before moving the cells
        // (InsertCells/DeleteCells - aPos is changed above) as well as when UpdateReference
        // is called after moving the cells (MoveBlock/PasteFromClip - aOldPos is changed).

        // If there is already a formula cell in the undo document, don't overwrite it,
        // the first (oldest) is the important cell.
        if ( pUndoDoc->GetCellType( aUndoPos ) != CELLTYPE_FORMULA )
        {
            ScFormulaCell* pFCell = new ScFormulaCell( pUndoDoc, aUndoPos,
                    pOldCode.get(), eTempGrammar, cMatrixFlag );
            pFCell->aResult.SetToken( NULL);  // to recognize it as changed later (Cut/Paste!)
            pUndoDoc->SetFormulaCell(aUndoPos, pFCell);
        }
    }

    bValChanged = false;

    if ( pSharedCode )
    {   // Replace shared formula with own formula
        pDocument->RemoveFromFormulaTree( this );   // update formula count
        delete pCode;
        pCode = pSharedCode->GetCode()->Clone();
        // #i18937# #i110008# call MoveRelWrap, but with the old position
        ScCompiler::MoveRelWrap(*pCode, pDocument, aOldPos, pSharedCode->GetMaxCol(), pSharedCode->GetMaxRow());
        ScCompiler aComp2(pDocument, aPos, *pCode);
        aComp2.SetGrammar(pDocument->GetGrammar());
        aComp2.UpdateSharedFormulaReference( eUpdateRefMode, aOldPos, rRange,
            nDx, nDy, nDz );
        bValChanged = true;
        bNeedDirty = true;
    }

    if ( ( bCompile = (bCompile || bValChanged || bRangeModified || bColRowNameCompile) ) != 0 )
    {
        CompileTokenArray( bNewListening ); // no Listening
        bNeedDirty = true;
    }

    if ( !bInDeleteUndo )
    {   // In ChangeTrack Delete-Reject listeners are established in
        // InsertCol/InsertRow
        if ( bNewListening )
        {
            if ( eUpdateRefMode == URM_INSDEL )
            {
                // Inserts/Deletes re-establish listeners after all
                // UpdateReference calls.
                // All replaced shared formula listeners have to be
                // established after an Insert or Delete. Do nothing here.
                SetNeedsListening( true);
            }
            else
                StartListeningTo( pDocument );
        }
    }

    if ( bNeedDirty && (!(eUpdateRefMode == URM_INSDEL && bHasRelName) || pSharedCode) )
    {   // Cut off references, invalid or similar?
        sc::AutoCalcSwitch(*pDocument, false);
        SetDirty();
    }

    return bCellStateChanged;
}

void ScFormulaCell::UpdateInsertTab(SCTAB nTable, SCTAB nNewSheets)
{
    bool bPosChanged = ( aPos.Tab() >= nTable ? true : false );
    pCode->Reset();
    if( pCode->GetNextReferenceRPN() && !pDocument->IsClipOrUndo() )
    {
        EndListeningTo( pDocument );
        // IncTab _after_ EndListeningTo and _before_ Compiler UpdateInsertTab!
        if ( bPosChanged )
            aPos.IncTab(nNewSheets);
        ScRangeData* pRangeData;
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        pRangeData = aComp.UpdateInsertTab( nTable, false, nNewSheets );
        if (pRangeData) // Exchange Shared Formula with real Formula
        {
            bool bRefChanged;
            pDocument->RemoveFromFormulaTree( this );   // update formula count
            delete pCode;
            pCode = new ScTokenArray( *pRangeData->GetCode() );
            ScCompiler aComp2(pDocument, aPos, *pCode);
            aComp2.SetGrammar(pDocument->GetGrammar());
            aComp2.MoveRelWrap(pRangeData->GetMaxCol(), pRangeData->GetMaxRow());
            aComp2.UpdateInsertTab( nTable, false, nNewSheets );
            // If the shared formula contained a named range/formula containing
            // an absolute reference to a sheet, those have to be readjusted.
            aComp2.UpdateDeleteTab( nTable, false, true, bRefChanged, nNewSheets );
            bCompile = true;
        }
        // no StartListeningTo because pTab[nTab] does not exsist!
    }
    else if ( bPosChanged )
        aPos.IncTab();
}

bool ScFormulaCell::UpdateDeleteTab(SCTAB nTable, bool bIsMove, SCTAB nSheets)
{
    bool bRefChanged = false;
    bool bPosChanged = ( aPos.Tab() >= nTable + nSheets ? true : false );
    pCode->Reset();
    if( pCode->GetNextReferenceRPN() && !pDocument->IsClipOrUndo() )
    {
        EndListeningTo( pDocument );
        // IncTab _after_ EndListeningTo und _before_ Compiler UpdateDeleteTab!
        if ( bPosChanged )
            aPos.IncTab(-1*nSheets);
        ScRangeData* pRangeData;
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        pRangeData = aComp.UpdateDeleteTab(nTable, bIsMove, false, bRefChanged, nSheets);
        if (pRangeData) // Exchange Shared Formula with real Formula
        {
            pDocument->RemoveFromFormulaTree( this );   // update formula count
            delete pCode;
            pCode = pRangeData->GetCode()->Clone();
            ScCompiler aComp2(pDocument, aPos, *pCode);
            aComp2.SetGrammar(pDocument->GetGrammar());
            aComp2.CompileTokenArray();
            aComp2.MoveRelWrap(pRangeData->GetMaxCol(), pRangeData->GetMaxRow());
            aComp2.UpdateDeleteTab( nTable, false, false, bRefChanged, nSheets );
            // If the shared formula contained a named range/formula containing
            // an absolute reference to a sheet, those have to be readjusted.
            aComp2.UpdateInsertTab( nTable,true, nSheets );
            // bRefChanged could have been reset at the last UpdateDeleteTab
            bRefChanged = true;
            bCompile = true;
        }
        // no StartListeningTo because pTab[nTab] not yet correct!
    }
    else if ( bPosChanged )
        aPos.IncTab(-1*nSheets);

    return bRefChanged;
}

void ScFormulaCell::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo )
{
    pCode->Reset();
    if( pCode->GetNextReferenceRPN() && !pDocument->IsClipOrUndo() )
    {
        EndListeningTo( pDocument );
        // SetTab _after_ EndListeningTo und _before_ Compiler UpdateMoveTab !
        aPos.SetTab( nTabNo );
        ScRangeData* pRangeData;
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        pRangeData = aComp.UpdateMoveTab( nOldPos, nNewPos, false );
        if (pRangeData) // Exchange Shared Formula with real Formula
        {
            pDocument->RemoveFromFormulaTree( this );   // update formula count
            delete pCode;
            pCode = pRangeData->GetCode()->Clone();
            ScCompiler aComp2(pDocument, aPos, *pCode);
            aComp2.SetGrammar(pDocument->GetGrammar());
            aComp2.CompileTokenArray();
            aComp2.MoveRelWrap(pRangeData->GetMaxCol(), pRangeData->GetMaxRow());
            aComp2.UpdateMoveTab( nOldPos, nNewPos, true );
            bCompile = true;
        }
        // no StartListeningTo because pTab[nTab] not yet correct!
    }
    else
        aPos.SetTab( nTabNo );
}

void ScFormulaCell::UpdateInsertTabAbs(SCTAB nTable)
{
    if( !pDocument->IsClipOrUndo() )
    {
        pCode->Reset();
        ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        while( p )
        {
            ScSingleRefData& rRef1 = p->GetSingleRef();
            if( !rRef1.IsTabRel() && (SCsTAB) nTable <= rRef1.nTab )
                rRef1.nTab++;
            if( p->GetType() == formula::svDoubleRef )
            {
                ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
                if( !rRef2.IsTabRel() && (SCsTAB) nTable <= rRef2.nTab )
                    rRef2.nTab++;
            }
            p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        }
    }
}

bool ScFormulaCell::TestTabRefAbs(SCTAB nTable)
{
    bool bRet = false;
    if( !pDocument->IsClipOrUndo() )
    {
        pCode->Reset();
        ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        while( p )
        {
            ScSingleRefData& rRef1 = p->GetSingleRef();
            if( !rRef1.IsTabRel() )
            {
                if( (SCsTAB) nTable != rRef1.nTab )
                    bRet = true;
                else if (nTable != aPos.Tab())
                    rRef1.nTab = aPos.Tab();
            }
            if( p->GetType() == formula::svDoubleRef )
            {
                ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
                if( !rRef2.IsTabRel() )
                {
                    if( (SCsTAB) nTable != rRef2.nTab )
                        bRet = true;
                    else if (nTable != aPos.Tab())
                        rRef2.nTab = aPos.Tab();
                }
            }
            p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        }
    }
    return bRet;
}

void ScFormulaCell::UpdateCompile( bool bForceIfNameInUse )
{
    if ( bForceIfNameInUse && !bCompile )
        bCompile = pCode->HasNameOrColRowName();
    if ( bCompile )
        pCode->SetCodeError( 0 );   // make sure it will really be compiled
    CompileTokenArray();
}

// Reference transposition is only called in Clipboard Document
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
                sal_Int16 nTemp;

                nTemp = rRef1.nRelCol;
                rRef1.nRelCol = static_cast<SCCOL>(rRef1.nRelRow);
                rRef1.nRelRow = static_cast<SCROW>(nTemp);

                if ( bDouble )
                {
                    nTemp = rRef2.nRelCol;
                    rRef2.nRelCol = static_cast<SCCOL>(rRef2.nRelRow);
                    rRef2.nRelRow = static_cast<SCROW>(nTemp);
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
                if (pName->HasType(RT_SHAREDMOD))
                    pShared = pName;
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

    if (pShared) // Exchange Shared Formula with real Formula
    {
        pDocument->RemoveFromFormulaTree( this ); // update formula count
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
                    rRef.SetRange(aAbs, aPos); // based on the new anchor position.
            }
        }
    }

    if (bRefChanged)
    {
        if (pUndoDoc)
        {
            ScFormulaCell* pFCell = new ScFormulaCell( pUndoDoc, aPos, pOld,
                    eTempGrammar, cMatrixFlag);
            pFCell->aResult.SetToken( NULL);  // to recognize it as changed later (Cut/Paste!)
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
                if (pName->HasType(RT_SHAREDMOD))
                    pShared = pName;
            }
        }
        else if( t->GetType() != svIndex )
        {
            t->CalcAbsIfRel( aPos );
            bool bMod;
            {   // Own scope for SingleDoubleRefModifier dtor if SingleRef
                SingleDoubleRefModifier aMod( *t );
                ScComplexRefData& rRef = aMod.Ref();
                bMod = (ScRefUpdate::UpdateGrow( rArea,nGrowX,nGrowY,
                    rRef ) != UR_NOTHING);
            }
            if ( bMod )
            {
                t->CalcRelFromAbs( aPos );
                bRefChanged = true;
            }
        }
    }

    if (pShared) // Exchange Shared Formula with real Formula
    {
        pDocument->RemoveFromFormulaTree( this ); // Update formula count
        delete pCode;
        pCode = new ScTokenArray( *pShared->GetCode() );
        bRefChanged = true;
        pCode->Reset();
        while( (t = static_cast<ScToken*>(pCode->GetNextReference())) != NULL )
        {
            if( t->GetType() != svIndex )
            {
                t->CalcAbsIfRel( aPos );
                bool bMod;
                {   // Own scope for SingleDoubleRefModifier dtor if SingleRef
                    SingleDoubleRefModifier aMod( *t );
                    ScComplexRefData& rRef = aMod.Ref();
                    bMod = (ScRefUpdate::UpdateGrow( rArea,nGrowX,nGrowY,
                        rRef ) != UR_NOTHING);
                }
                if ( bMod )
                    t->CalcRelFromAbs( aPos );
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

sal_uInt8       ScFormulaCell::GetMatrixFlag() const                   { return cMatrixFlag; }
ScTokenArray*   ScFormulaCell::GetCode() const                         { return pCode; }
bool            ScFormulaCell::IsRunning() const                       { return bRunning; }
void            ScFormulaCell::SetRunning( bool bVal )                 { bRunning = bVal; }

void ScFormulaCell::CompileDBFormula()
{
    for( FormulaToken* p = pCode->First(); p; p = pCode->Next() )
    {
        if ( p->GetOpCode() == ocDBArea
            || (p->GetOpCode() == ocName && p->GetIndex() >= SC_START_INDEX_DB_COLL) )
        {
            bCompile = true;
            CompileTokenArray();
            SetDirty();
            break;
        }
    }
}

void ScFormulaCell::CompileDBFormula( bool bCreateFormulaString )
{
    // Two phases must be called after each other
    // 1. Formula String with old generated names
    // 2. Formula String with new generated names
    if ( bCreateFormulaString )
    {
        bool bRecompile = false;
        pCode->Reset();
        for ( FormulaToken* p = pCode->First(); p && !bRecompile; p = pCode->Next() )
        {
            switch ( p->GetOpCode() )
            {
                case ocBad:             // DB Area eventually goes bad
                case ocColRowName:      // in case of the same names
                case ocDBArea:          // DB Area
                    bRecompile = true;
                break;
                case ocName:
                    if ( p->GetIndex() >= SC_START_INDEX_DB_COLL )
                        bRecompile = true;  // DB Area
                break;
                default:
                    ; // nothing
            }
        }
        if ( bRecompile )
        {
            OUString aFormula;
            GetFormula( aFormula, formula::FormulaGrammar::GRAM_NATIVE);
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
            SetHybridFormula( aFormula, formula::FormulaGrammar::GRAM_NATIVE);
        }
    }
    else if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        Compile( aResult.GetHybridFormula(), false, eTempGrammar );
        aResult.SetToken( NULL);
        SetDirty();
    }
}

void ScFormulaCell::CompileNameFormula( bool bCreateFormulaString )
{
    // Two phases must be called after each other
    // 1. Formula String with old generated names
    // 2. Formula String with new generated names
    if ( bCreateFormulaString )
    {
        bool bRecompile = false;
        pCode->Reset();
        for ( FormulaToken* p = pCode->First(); p && !bRecompile; p = pCode->Next() )
        {
            switch ( p->GetOpCode() )
            {
                case ocBad:             // in case RangeName goes bad
                case ocColRowName:      // in case the names are the same
                    bRecompile = true;
                break;
                default:
                    if ( p->GetType() == svIndex )
                        bRecompile = true;  // RangeName
            }
        }
        if ( bRecompile )
        {
            OUString aFormula;
            GetFormula( aFormula, formula::FormulaGrammar::GRAM_NATIVE);
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
            SetHybridFormula( aFormula, formula::FormulaGrammar::GRAM_NATIVE);
        }
    }
    else if ( !pCode->GetLen() && !aResult.GetHybridFormula().isEmpty() )
    {
        Compile( aResult.GetHybridFormula(), false, eTempGrammar );
        aResult.SetToken( NULL);
        SetDirty();
    }
}

void ScFormulaCell::CompileColRowNameFormula()
{
    pCode->Reset();
    for ( FormulaToken* p = pCode->First(); p; p = pCode->Next() )
    {
        if ( p->GetOpCode() == ocColRowName )
        {
            bCompile = true;
            CompileTokenArray();
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

ScFormulaCellGroupRef ScFormulaCell::GetCellGroup()
{
    return xGroup;
}

void ScFormulaCell::SetCellGroup( const ScFormulaCellGroupRef &xRef )
{
    xGroup = xRef;
}

ScFormulaCell::CompareState ScFormulaCell::CompareByTokenArray( ScFormulaCell& rOther ) const
{
    // no Matrix formulae yet.
    if ( GetMatrixFlag() != MM_NONE )
        return NotEqual;

    // are these formule at all similar ?
    if ( GetHash() != rOther.GetHash() )
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

    bool bInvariant = true;

    // check we are basically the same function
    for ( sal_uInt16 i = 0; i < nThisLen; i++ )
    {
        ScToken *pThisTok = static_cast<ScToken*>( pThis[i] );
        ScToken *pOtherTok = static_cast<ScToken*>( pOther[i] );

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
                const ScSingleRefData& rRef = pThisTok->GetSingleRef();
                if (rRef != pOtherTok->GetSingleRef())
                    return NotEqual;

                if (rRef.IsRowRel())
                    bInvariant = false;
            }
            break;
            case formula::svDoubleRef:
            {
                // Range reference.
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
            default:
                ;
        }
    }

    return bInvariant ? EqualInvariant : EqualRelativeRef;
}

namespace {

class GroupTokenConverter
{
    sc::FormulaGroupContext& mrCxt;
    ScTokenArray& mrGroupTokens;
    ScDocument& mrDoc;
    ScFormulaCell& mrCell;
    const ScAddress& mrPos;
public:
    GroupTokenConverter(sc::FormulaGroupContext& rCxt, ScTokenArray& rGroupTokens, ScDocument& rDoc, ScFormulaCell& rCell, const ScAddress& rPos) :
        mrCxt(rCxt), mrGroupTokens(rGroupTokens), mrDoc(rDoc), mrCell(rCell), mrPos(rPos) {}

    bool convert(ScTokenArray& rCode)
    {
        { // debug to start with:
            ScCompiler aComp( &mrDoc, mrPos, rCode);
            aComp.SetGrammar(formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1);
            OUStringBuffer aAsString;
            aComp.CreateStringFromTokenArray(aAsString);
            SAL_DEBUG("interpret formula: " << aAsString.makeStringAndClear());
        }

        rCode.Reset();
        for (const formula::FormulaToken* p = rCode.First(); p; p = rCode.Next())
        {
            // A reference can be either absolute or relative.  If it's absolute,
            // convert it to a static value token.  If relative, convert it to a
            // vector reference token.  Note: we only care about relative vs
            // absolute reference state for row directions.

            const ScToken* pToken = static_cast<const ScToken*>(p);
            switch (pToken->GetType())
            {
                case svSingleRef:
                {
                    ScSingleRefData aRef = pToken->GetSingleRef();
                    ScAddress aRefPos = aRef.toAbs(mrPos);
                    if (aRef.IsRowRel())
                    {
                        // Fetch double array guarantees that the length of the
                        // returned array equals or greater than the requested
                        // length.

                        const double* pArray = mrDoc.FetchDoubleArray(mrCxt, aRefPos, mrCell.GetCellGroup()->mnLength);
                        if (!pArray)
                            return false;

                        formula::SingleVectorRefToken aTok(pArray, mrCell.GetCellGroup()->mnLength);
                        mrGroupTokens.AddToken(aTok);
                    }
                    else
                    {
                        // Absolute row reference.
                        formula::FormulaTokenRef pNewToken = mrDoc.ResolveStaticReference(aRefPos);
                        if (!pNewToken)
                            return false;

                        mrGroupTokens.AddToken(*pNewToken);
                    }
                }
                break;
                case svDoubleRef:
                {
                    ScComplexRefData aRef = pToken->GetDoubleRef();
                    aRef.CalcAbsIfRel(mrCell.aPos);

                    // Row reference is relative.
                    bool bAbsFirst = !aRef.Ref1.IsRowRel();
                    bool bAbsLast = !aRef.Ref2.IsRowRel();
                    ScAddress aRefPos(aRef.Ref1.nCol, aRef.Ref1.nRow, aRef.Ref1.nTab);
                    size_t nCols = aRef.Ref2.nCol - aRef.Ref1.nCol + 1;
                    std::vector<const double*> aArrays;
                    aArrays.reserve(nCols);
                    SCROW nArrayLength = mrCell.GetCellGroup()->mnLength;
                    SCROW nRefRowSize = aRef.Ref2.nRow - aRef.Ref1.nRow + 1;
                    if (!bAbsLast)
                    {
                        // range end position is relative. Extend the array length.
                        nArrayLength += nRefRowSize - 1;
                    }

                    for (SCCOL i = aRef.Ref1.nCol; i <= aRef.Ref2.nCol; ++i)
                    {
                        aRefPos.SetCol(i);
                        const double* pArray = mrDoc.FetchDoubleArray(mrCxt, aRefPos, nArrayLength);
                        if (!pArray)
                            return false;

                        aArrays.push_back(pArray);
                    }

                    formula::DoubleVectorRefToken aTok(aArrays, nArrayLength, nRefRowSize, bAbsFirst, bAbsLast);
                    mrGroupTokens.AddToken(aTok);
                }
                break;
                case svIndex:
                {
                    // Named range.
                    ScRangeName* pNames = mrDoc.GetRangeName();
                    if (!pNames)
                        // This should never fail.
                        return false;

                    ScRangeData* pRange = pNames->findByIndex(p->GetIndex());
                    if (!pRange)
                        // No named range exists by that index.
                        return false;

                    ScTokenArray* pNamedTokens = pRange->GetCode();
                    if (!pNamedTokens)
                        // This named range is empty.
                        return false;

                    mrGroupTokens.AddOpCode(ocOpen);

                    if (!convert(*pNamedTokens))
                        return false;

                    mrGroupTokens.AddOpCode(ocClose);
                }
                break;
                default:
                    mrGroupTokens.AddToken(*pToken);
            }
        }

        return true;
    }
};

}

bool ScFormulaCell::InterpretFormulaGroup()
{
    if (!ScInterpreter::GetGlobalConfig().mbOpenCLEnabled)
        return false;

    // Re-build formulae groups if necessary - ideally this is done at
    // import / insert / delete etc. and is integral to the data structures
    pDocument->RebuildFormulaGroups();

    if (!xGroup || !pCode)
        return false;

    if (xGroup->meCalcState == sc::GroupCalcDisabled)
        return false;

    switch (pCode->GetVectorState())
    {
        case FormulaVectorEnabled:
        case FormulaVectorCheckReference:
            // Good.
        break;
        case FormulaVectorDisabled:
        case FormulaVectorUnknown:
        default:
            // Not good.
            return false;
    }

    if (xGroup->mbInvariant)
        return InterpretInvariantFormulaGroup();

    sc::FormulaGroupContext aCxt;
    ScTokenArray aCode;
    ScAddress aTopPos = aPos;
    aTopPos.SetRow(xGroup->mnStart);
    GroupTokenConverter aConverter(aCxt, aCode, *pDocument, *this, aTopPos);
    if (!aConverter.convert(*pCode))
    {
        xGroup->meCalcState = sc::GroupCalcDisabled;
        return false;
    }

    xGroup->meCalcState = sc::GroupCalcRunning;
    if (!sc::FormulaGroupInterpreter::getStatic()->interpret(*pDocument, aTopPos, xGroup, aCode))
    {
        xGroup->meCalcState = sc::GroupCalcDisabled;
        return false;
    }

    xGroup->meCalcState = sc::GroupCalcEnabled;
    return true;
}

bool ScFormulaCell::InterpretInvariantFormulaGroup()
{
    if (pCode->GetVectorState() == FormulaVectorCheckReference)
    {
        // An invariant group should only have absolute row references, and no
        // external references are allowed.

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
                    aRef.CalcAbsIfRel(aPos); // column may be relative.
                    ScAddress aRefPos(aRef.nCol, aRef.nRow, aRef.nTab);
                    formula::FormulaTokenRef pNewToken = pDocument->ResolveStaticReference(aRefPos);
                    if (!pNewToken)
                        return false;

                    aCode.AddToken(*pNewToken);
                }
                break;
                case svDoubleRef:
                {
                    ScComplexRefData aRef = pToken->GetDoubleRef();
                    aRef.CalcAbsIfRel(aPos); // column may be relative.
                    ScRange aRefRange(
                        aRef.Ref1.nCol, aRef.Ref1.nRow, aRef.Ref1.nTab,
                        aRef.Ref2.nCol, aRef.Ref2.nRow, aRef.Ref2.nTab);

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
        aComp.CompileTokenArray(); // Create RPN token array.
        ScInterpreter aInterpreter(this, pDocument, aPos, aCode);
        aInterpreter.Interpret();
        aResult.SetToken(aInterpreter.GetResultToken().get());
    }
    else
    {
        // Formula contains no references.
        ScInterpreter aInterpreter(this, pDocument, aPos, *pCode);
        aInterpreter.Interpret();
        aResult.SetToken(aInterpreter.GetResultToken().get());
    }

    for ( sal_Int32 i = 0; i < xGroup->mnLength; i++ )
    {
        ScAddress aTmpPos = aPos;
        aTmpPos.SetRow(xGroup->mnStart + i);
        ScFormulaCell* pCell = pDocument->GetFormulaCell(aTmpPos);
        assert( pCell != NULL );

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
    ScFormulaCell* pCell, ScDocument& rDoc, const ScAddress& rPos, const ScToken& rToken)
{
    const ScSingleRefData& rRef1 = rToken.GetSingleRef();
    const ScSingleRefData& rRef2 = rToken.GetSingleRef2();
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
        rDoc.StartListeningArea(ScRange(aCell1, aCell2), pCell);
    }
}

}

void ScFormulaCell::StartListeningTo( ScDocument* pDoc )
{
    if (pDoc->IsClipOrUndo() || pDoc->GetNoListening() || IsInChangeTrack())
        return;

    pDoc->SetDetectiveDirty(true);  // It has changed something

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
                ;   // nothing
        }
    }
    SetNeedsListening( false);
}

void ScFormulaCell::StartListeningTo( sc::StartListeningContext& rCxt )
{
    ScDocument& rDoc = rCxt.getDoc();

    if (rDoc.IsClipOrUndo() || rDoc.GetNoListening() || IsInChangeTrack())
        return;

    rDoc.SetDetectiveDirty(true);  // It has changed something

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
                ;   // nothing
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

        rDoc.EndListeningArea(ScRange(aCell1, aCell2), pCell);
    }
}

}

void ScFormulaCell::EndListeningTo( ScDocument* pDoc, ScTokenArray* pArr,
        ScAddress aCellPos )
{
    if (pDoc->IsClipOrUndo() || IsInChangeTrack())
        return;

    pDoc->SetDetectiveDirty(true);  // It has changed something

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
                ;   // nothing
        }
    }
}

void ScFormulaCell::EndListeningTo( sc::EndListeningContext& rCxt )
{
    if (rCxt.getDoc().IsClipOrUndo() || IsInChangeTrack())
        return;

    ScDocument& rDoc = rCxt.getDoc();
    rDoc.SetDetectiveDirty(true);  // It has changed something

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
                ;   // nothing
        }
    }
}

bool ScFormulaCell::IsShared() const
{
    return xGroup.get() != NULL;
}

bool ScFormulaCell::IsSharedInvariant() const
{
    return xGroup ? xGroup->mbInvariant : false;
}

SCROW ScFormulaCell::GetSharedTopRow() const
{
    return xGroup ? xGroup->mnStart : -1;
}
SCROW ScFormulaCell::GetSharedLength() const
{
    return xGroup ? xGroup->mnLength : 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
