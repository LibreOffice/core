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

#include <algorithm>
#include <deque>

#include <boost/bind.hpp>
#include <sal/macros.h>
#include <vcl/mapmod.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include "editeng/fieldupdater.hxx"

#include "cell.hxx"
#include "compiler.hxx"
#include "formula/errorcodes.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "rechead.hxx"
#include "refupdat.hxx"
#include "scmatrix.hxx"
#include "editutil.hxx"
#include "chgtrack.hxx"
#include "externalrefmgr.hxx"
#include "scitems.hxx"
#include "patattr.hxx"
#include <rtl/strbuf.hxx>

#include <cstdio>

using namespace formula;

// STATIC DATA -----------------------------------------------------------

#ifdef USE_MEMPOOL
IMPL_FIXEDMEMPOOL_NEWDEL( ScEditCell )
#endif

// ============================================================================

ScEditCell::ScEditCell(EditTextObject* pObject, ScDocument* pDoc) :
    ScBaseCell(CELLTYPE_EDIT),
    mpData(pObject), mpString(NULL), mpDoc(pDoc) {}

ScEditCell::ScEditCell(
    const EditTextObject& rObject, ScDocument* pDoc, const SfxItemPool* pFromPool) :
    ScBaseCell(CELLTYPE_EDIT),
    mpString(NULL),
    mpDoc(pDoc)
{
    SetTextObject(&rObject, pFromPool);
}

ScEditCell::ScEditCell(const ScEditCell& rCell, ScDocument& rDoc, const ScAddress& rDestPos) :
    ScBaseCell(rCell), mpString(NULL), mpDoc(&rDoc)
{
    SetTextObject( rCell.mpData, rCell.mpDoc->GetEditPool() );
    UpdateFields(rDestPos.Tab());
}

ScEditCell::ScEditCell(const OUString& rString, ScDocument* pDoc)  :
    ScBaseCell(CELLTYPE_EDIT),
    mpData(NULL),
    mpString(NULL),
    mpDoc(pDoc)
{
    OSL_ENSURE( rString.indexOf('\n') != -1 ||
                rString.indexOf(CHAR_CR) != -1,
                "EditCell mit einfachem Text !?!?" );

    EditEngine& rEngine = mpDoc->GetEditEngine();
    rEngine.SetText( rString );
    mpData = rEngine.CreateTextObject();
}

ScEditCell::~ScEditCell()
{
    delete mpData;
    delete mpString;

#if OSL_DEBUG_LEVEL > 0
    eCellType = CELLTYPE_DESTROYED;
#endif
}

void ScEditCell::ClearData()
{
    delete mpString;
    mpString = NULL;
    delete mpData;
    mpData = NULL;
}

void ScEditCell::SetData(const EditTextObject& rObject, const SfxItemPool* pFromPool)
{
    ClearData();
    SetTextObject(&rObject, pFromPool);
}

void ScEditCell::SetData(EditTextObject* pObject)
{
    ClearData();
    mpData = pObject;
}

OUString ScEditCell::GetString() const
{
    if (mpString)
        return *mpString;

    if (mpData)
    {
        // Also Text from URL fields, Doc-Engine is a ScFieldEditEngine
        EditEngine& rEngine = mpDoc->GetEditEngine();
        rEngine.SetText(*mpData);
        rtl::OUString sRet = ScEditUtil::GetMultilineString(rEngine); // string with line separators between paragraphs
        // cache short strings for formulas
        if ( sRet.getLength() < 256 )
            mpString = new rtl::OUString(sRet);   //! non-const
        return sRet;
    }

    return OUString();
}

const EditTextObject* ScEditCell::GetData() const
{
    return mpData;
}

void ScEditCell::RemoveCharAttribs( const ScPatternAttr& rAttr )
{
    ScEditUtil::RemoveCharAttribs(*mpData, rAttr);
}

void ScEditCell::UpdateFields(SCTAB nTab)
{
    editeng::FieldUpdater aUpdater = mpData->GetFieldUpdater();
    aUpdater.updateTableFields(nTab);
}

void ScEditCell::SetTextObject( const EditTextObject* pObject,
            const SfxItemPool* pFromPool )
{
    if ( pObject )
    {
        if ( pFromPool && mpDoc->GetEditPool() == pFromPool )
            mpData = pObject->Clone();
        else
        {   //! another "spool"
            // Sadly there is no other way to change the Pool than to
            // "spool" the Object through a corresponding Engine
            EditEngine& rEngine = mpDoc->GetEditEngine();
            if ( pObject->HasOnlineSpellErrors() )
            {
                sal_uLong nControl = rEngine.GetControlWord();
                const sal_uLong nSpellControl = EE_CNTRL_ONLINESPELLING | EE_CNTRL_ALLOWBIGOBJS;
                bool bNewControl = ( (nControl & nSpellControl) != nSpellControl );
                if ( bNewControl )
                    rEngine.SetControlWord( nControl | nSpellControl );
                rEngine.SetText( *pObject );
                mpData = rEngine.CreateTextObject();
                if ( bNewControl )
                    rEngine.SetControlWord( nControl );
            }
            else
            {
                rEngine.SetText( *pObject );
                mpData = rEngine.CreateTextObject();
            }
        }
    }
    else
        mpData = NULL;
}

ScEditDataArray::ScEditDataArray()
{
}

ScEditDataArray::~ScEditDataArray()
{
}

void ScEditDataArray::AddItem(SCTAB nTab, SCCOL nCol, SCROW nRow,
                              EditTextObject* pOldData, EditTextObject* pNewData)
{
    maArray.push_back(Item(nTab, nCol, nRow, pOldData, pNewData));
}

const ScEditDataArray::Item* ScEditDataArray::First()
{
    maIter = maArray.begin();
    if (maIter == maArray.end())
        return NULL;
    return &(*maIter++);
}

const ScEditDataArray::Item* ScEditDataArray::Next()
{
    if (maIter == maArray.end())
        return NULL;
    return &(*maIter++);
}

// ============================================================================

ScEditDataArray::Item::Item(SCTAB nTab, SCCOL nCol, SCROW nRow,
                            EditTextObject* pOldData, EditTextObject* pNewData) :
    mnTab(nTab),
    mnCol(nCol),
    mnRow(nRow)
{
    mpOldData.reset(pOldData);
    mpNewData.reset(pNewData);
}

ScEditDataArray::Item::~Item()
{
}

const EditTextObject* ScEditDataArray::Item::GetOldData() const
{
    return mpOldData.get();
}

const EditTextObject* ScEditDataArray::Item::GetNewData() const
{
    return mpNewData.get();
}

SCTAB ScEditDataArray::Item::GetTab() const
{
    return mnTab;
}

SCCOL ScEditDataArray::Item::GetCol() const
{
    return mnCol;
}

SCROW ScEditDataArray::Item::GetRow() const
{
    return mnRow;
}

// ============================================================================

namespace
{

using std::deque;

typedef SCCOLROW(*DimensionSelector)(const ScSingleRefData&);


static SCCOLROW lcl_GetCol(const ScSingleRefData& rData)
{
    return rData.nCol;
}


static SCCOLROW lcl_GetRow(const ScSingleRefData& rData)
{
    return rData.nRow;
}


static SCCOLROW lcl_GetTab(const ScSingleRefData& rData)
{
    return rData.nTab;
}


/** Check if both references span the same range in selected dimension.
 */
static bool
lcl_checkRangeDimension(
        const SingleDoubleRefProvider& rRef1,
        const SingleDoubleRefProvider& rRef2,
        const DimensionSelector aWhich)
{
    return
        aWhich(rRef1.Ref1) == aWhich(rRef2.Ref1)
        && aWhich(rRef1.Ref2) == aWhich(rRef2.Ref2);
}


static bool
lcl_checkRangeDimensions(
        const SingleDoubleRefProvider& rRef1,
        const SingleDoubleRefProvider& rRef2,
        bool& bCol, bool& bRow, bool& bTab)
{
    const bool bSameCols(lcl_checkRangeDimension(rRef1, rRef2, lcl_GetCol));
    const bool bSameRows(lcl_checkRangeDimension(rRef1, rRef2, lcl_GetRow));
    const bool bSameTabs(lcl_checkRangeDimension(rRef1, rRef2, lcl_GetTab));

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
        bOk = lcl_checkRangeDimensions(aRef, aRefCur, bCol, bRow, bTab);
    }
    while (bOk && aCur != aEnd)
    {
        const SingleDoubleRefProvider aRefCur(**aCur);
        bool bColTmp(false);
        bool bRowTmp(false);
        bool bTabTmp(false);
        bOk = lcl_checkRangeDimensions(aRef, aRefCur, bColTmp, bRowTmp, bTabTmp);
        bOk = bOk && (bCol == bColTmp && bRow == bRowTmp && bTab == bTabTmp);
        ++aCur;
    }

    if (bOk && aCur == aEnd)
    {
        return true;
    }
    return false;
}


bool
lcl_lessReferenceBy(
        const ScToken* const pRef1, const ScToken* const pRef2,
        const DimensionSelector aWhich)
{
    const SingleDoubleRefProvider rRef1(*pRef1);
    const SingleDoubleRefProvider rRef2(*pRef2);
    return aWhich(rRef1.Ref1) < aWhich(rRef2.Ref1);
}


/** Returns true if range denoted by token pRef2 starts immediately after
    range denoted by token pRef1. Dimension, in which the comparison takes
    place, is given by aWhich.
 */
bool
lcl_isImmediatelyFollowing(
        const ScToken* const pRef1, const ScToken* const pRef2,
        const DimensionSelector aWhich)
{
    const SingleDoubleRefProvider rRef1(*pRef1);
    const SingleDoubleRefProvider rRef2(*pRef2);
    return aWhich(rRef2.Ref1) - aWhich(rRef1.Ref2) == 1;
}


static bool
lcl_checkIfAdjacent(
        const deque<ScToken*>& rReferences,
        const DimensionSelector aWhich)
{
    typedef deque<ScToken*>::const_iterator Iter;
    Iter aBegin(rReferences.begin());
    Iter aEnd(rReferences.end());
    Iter aBegin1(aBegin);
    ++aBegin1, --aEnd;
    return std::equal(
            aBegin, aEnd, aBegin1,
            boost::bind(lcl_isImmediatelyFollowing, _1, _2, aWhich));
}


static void
lcl_fillRangeFromRefList(
        const deque<ScToken*>& rReferences, ScRange& rRange)
{
    const ScSingleRefData aStart(
            SingleDoubleRefProvider(*rReferences.front()).Ref1);
    rRange.aStart.Set(aStart.nCol, aStart.nRow, aStart.nTab);
    const ScSingleRefData aEnd(
            SingleDoubleRefProvider(*rReferences.back()).Ref2);
    rRange.aEnd.Set(aEnd.nCol, aEnd.nRow, aEnd.nTab);
}


static bool
lcl_refListFormsOneRange(
        const ScAddress& aPos, deque<ScToken*>& rReferences,
        ScRange& rRange)
{
    std::for_each(
            rReferences.begin(), rReferences.end(),
            bind(&ScToken::CalcAbsIfRel, _1, aPos))
        ;
    if (rReferences.size() == 1) {
        lcl_fillRangeFromRefList(rReferences, rRange);
        return true;
    }

    bool bCell(false);
    bool bRow(false);
    bool bTab(false);
    if (lcl_checkRangeDimensions(rReferences.begin(), rReferences.end(),
            bCell, bRow, bTab))
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
        std::sort(rReferences.begin(), rReferences.end(),
                boost::bind(lcl_lessReferenceBy, _1, _2, aWhich));
        if (lcl_checkIfAdjacent(rReferences, aWhich))
        {
            lcl_fillRangeFromRefList(rReferences, rRange);
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

rtl::OUString ScFormulaCell::GetString()
{
    MaybeInterpret();
    if ((!pCode->GetCodeError() || pCode->GetCodeError() == errDoubleRef) &&
            !aResult.GetResultError())
        return aResult.GetString();
    return rtl::OUString();
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
                rRef.CalcAbsIfRel( aPos );
                if ( rRef.Valid() )
                {
                    rPos.Set( rRef.nCol, rRef.nRow, rRef.nTab );
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

sal_uInt16 ScFormulaCell::GetMatrixEdge( ScAddress& rOrgPos )
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
                ScFormulaCell* pFCell;
                if ( cMatrixFlag == MM_REFERENCE )
                    pFCell = pDocument->GetFormulaCell(aOrg);
                else
                    pFCell = this;      // this MM_FORMULA
                // There's only one this, don't compare pFCell==this.
                if ( pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA
                  && pFCell->cMatrixFlag == MM_FORMULA )
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
                        pFCell->SetMatColsRows( nC, nR );
                    }
                }
                else
                {
#if OSL_DEBUG_LEVEL > 0
                    rtl::OUString aTmp;
                    rtl::OStringBuffer aMsg(RTL_CONSTASCII_STRINGPARAM(
                        "broken Matrix, no MatFormula at origin, Pos: "));
                    aPos.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                    aMsg.append(rtl::OUStringToOString(aTmp, RTL_TEXTENCODING_ASCII_US));
                    aMsg.append(RTL_CONSTASCII_STRINGPARAM(", MatOrg: "));
                    aOrg.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                    aMsg.append(rtl::OUStringToOString(aTmp, RTL_TEXTENCODING_ASCII_US));
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
                    nEdges |= 4;            // left edge
                if ( dC+1 == nC )
                    nEdges |= 16;           // right edge
                if ( dR == 0 )
                    nEdges |= 8;            // top edge
                if ( dR+1 == nR )
                    nEdges |= 2;            // bottom edge
                if ( !nEdges )
                    nEdges = 1;             // inside
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                rtl::OUString aTmp;
                rtl::OStringBuffer aMsg( "broken Matrix, Pos: " );
                aPos.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                aMsg.append(rtl::OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8 ));
                aMsg.append(RTL_CONSTASCII_STRINGPARAM(", MatOrg: "));
                aOrg.Format( aTmp, SCA_VALID_COL | SCA_VALID_ROW, pDocument );
                aMsg.append(rtl::OUStringToOString(aTmp, RTL_TEXTENCODING_UTF8 ));
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

bool ScFormulaCell::HasOneReference( ScRange& r ) const
{
    pCode->Reset();
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    if( p && !pCode->GetNextReferenceRPN() )        // only one!
    {
        p->CalcAbsIfRel( aPos );
        SingleDoubleRefProvider aProv( *p );
        r.aStart.Set( aProv.Ref1.nCol,
                      aProv.Ref1.nRow,
                      aProv.Ref1.nTab );
        r.aEnd.Set( aProv.Ref2.nCol,
                    aProv.Ref2.nRow,
                    aProv.Ref2.nTab );
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

bool ScFormulaCell::UpdateReference(UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScDocument* pUndoDoc, const ScAddress* pUndoCellPos )
{
    bool bCellStateChanged = false;

    SCCOL nCol1 = r.aStart.Col();
    SCROW nRow1 = r.aStart.Row();
    SCCOL nCol = aPos.Col();
    SCROW nRow = aPos.Row();
    SCTAB nTab = aPos.Tab();
    ScAddress aUndoPos( aPos );         // position for undo cell in pUndoDoc
    if ( pUndoCellPos )
        aUndoPos = *pUndoCellPos;
    ScAddress aOldPos( aPos );
//  bool bPosChanged = false;           // if this cell was moved
    bool bIsInsert = (eUpdateRefMode == URM_INSDEL &&
                nDx >= 0 && nDy >= 0 && nDz >= 0);
    if (eUpdateRefMode == URM_INSDEL && r.In( aPos ))
    {
        aPos.Move(nDx, nDy, nDz);
        bCellStateChanged = aPos != aOldPos;
    }
    else if ( r.In( aPos ) )
    {
        aOldPos.Set( nCol - nDx, nRow - nDy, nTab - nDz );
    }

    bool bHasRefs = false;
    bool bHasColRowNames = false;
    bool bOnRefMove = false;
    if ( !pDocument->IsClipOrUndo() )
    {
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
    if( bHasRefs || bOnRefMove )
    {
        ScTokenArray* pOld = pUndoDoc ? pCode->Clone() : NULL;
        ScRangeData* pRangeData;
        bool bValChanged = false;
        bool bRangeModified = false;    // any range, not only shared formula
        bool bRefSizeChanged = false;
        if ( bHasRefs )
        {
            ScCompiler aComp(pDocument, aPos, *pCode);
            aComp.SetGrammar(pDocument->GetGrammar());
            pRangeData = aComp.UpdateReference(eUpdateRefMode, aOldPos, r,
                                             nDx, nDy, nDz,
                                             bValChanged, bRefSizeChanged);
            bRangeModified = aComp.HasModifiedRange();
        }
        else
        {
            bValChanged = false;
            pRangeData = NULL;
            bRangeModified = false;
            bRefSizeChanged = false;
        }

        bCellStateChanged |= bValChanged;

        if ( bOnRefMove )
            bOnRefMove = (bValChanged || (aPos != aOldPos));
            // Cell may reference itself, e.g. ocColumn, ocRow without parameter

        bool bColRowNameCompile, bHasRelName, bNewListening, bInDeleteUndo;
        if ( bHasRefs )
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
                        rRef.CalcAbsIfRel( aPos );
                        ScAddress aAdr( rRef.nCol, rRef.nRow, rRef.nTab );
                        ScRangePair* pR = pColList->Find( aAdr );
                        if ( pR )
                        {   // defined
                            if ( pR->GetRange(1).aStart.Row() == nRow1 )
                                bColRowNameCompile = true;
                        }
                        else
                        {   // on the fly
                            if ( rRef.nRow + 1 == nRow1 )
                                bColRowNameCompile = true;
                        }
                    }
                    if ( nDx > 0 && rRef.IsRowRel() )
                    {   // RowName
                        rRef.CalcAbsIfRel( aPos );
                        ScAddress aAdr( rRef.nCol, rRef.nRow, rRef.nTab );
                        ScRangePair* pR = pRowList->Find( aAdr );
                        if ( pR )
                        {   // defined
                            if ( pR->GetRange(1).aStart.Col() == nCol1 )
                                bColRowNameCompile = true;
                        }
                        else
                        {   // on the fly
                            if ( rRef.nCol + 1 == nCol1 )
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
                        rRef.CalcAbsIfRel( aPos );
                        if ( rRef.Valid() )
                        {
                            ScAddress aAdr( rRef.nCol, rRef.nRow, rRef.nTab );
                            if ( r.In( aAdr ) )
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
            if ( pChangeTrack && pChangeTrack->IsInDeleteUndo() )
                bInDeleteUndo = true;
            else
                bInDeleteUndo = false;
            // RelNameRefs are always moved
            bHasRelName = HasRelNameReference();
            // Reference changed and new listening needed?
            // Except in Insert/Delete without specialties.
            bNewListening = (bRangeModified || pRangeData || bColRowNameCompile
                    || (bValChanged && (eUpdateRefMode != URM_INSDEL ||
                            bInDeleteUndo || bRefSizeChanged)) ||
                    (bHasRelName && eUpdateRefMode != URM_COPY))
                // #i36299# Don't duplicate action during cut&paste / drag&drop
                // on a cell in the range moved, start/end listeners is done
                // via ScDocument::DeleteArea() and ScDocument::CopyFromClip().
                && !(eUpdateRefMode == URM_MOVE &&
                        pDocument->IsInsertingFromOtherDoc() && r.In(aPos));
            if ( bNewListening )
                EndListeningTo( pDocument, pOld, aOldPos );
        }
        else
        {
            bColRowNameCompile = bHasRelName = bNewListening = bInDeleteUndo =
                false;
        }

        bool bNeedDirty = false;
        // NeedDirty for changes except for Copy and Move/Insert without RelNames
        if ( bRangeModified || pRangeData || bColRowNameCompile ||
                (bValChanged && eUpdateRefMode != URM_COPY &&
                 (eUpdateRefMode != URM_MOVE || bHasRelName) &&
                 (!bIsInsert || bHasRelName || bInDeleteUndo ||
                  bRefSizeChanged)) || bOnRefMove)
            bNeedDirty = true;
        else
            bNeedDirty = false;
        if (pUndoDoc && (bValChanged || pRangeData || bOnRefMove))
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
                        pOld, eTempGrammar, cMatrixFlag );
                pFCell->aResult.SetToken( NULL);  // to recognize it as changed later (Cut/Paste!)
                pUndoDoc->PutCell( aUndoPos, pFCell );
            }
        }
        bValChanged = false;
        if ( pRangeData )
        {   // Replace shared formula with own formula
            pDocument->RemoveFromFormulaTree( this );   // update formula count
            delete pCode;
            pCode = pRangeData->GetCode()->Clone();
            // #i18937# #i110008# call MoveRelWrap, but with the old position
            ScCompiler::MoveRelWrap(*pCode, pDocument, aOldPos, pRangeData->GetMaxCol(), pRangeData->GetMaxRow());
            ScCompiler aComp2(pDocument, aPos, *pCode);
            aComp2.SetGrammar(pDocument->GetGrammar());
            aComp2.UpdateSharedFormulaReference( eUpdateRefMode, aOldPos, r,
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
        if ( bNeedDirty && (!(eUpdateRefMode == URM_INSDEL && bHasRelName) || pRangeData) )
        {   // Cut off references, invalid or similar?
            bool bOldAutoCalc = pDocument->GetAutoCalc();
            // No Interpret in SubMinimalRecalc because of eventual wrong reference
            pDocument->SetAutoCalc( false );
            SetDirty();
            pDocument->SetAutoCalc( bOldAutoCalc );
        }

        delete pOld;
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
            t->CalcAbsIfRel( aOldPos );
            bool bMod;
            {   // Own scope for SingleDoubleRefModifier dtor if SingleRef
                SingleDoubleRefModifier aMod( *t );
                ScComplexRefData& rRef = aMod.Ref();
                bMod = (ScRefUpdate::UpdateTranspose( pDocument, rSource,
                    rDest, rRef ) != UR_NOTHING || bPosChanged);
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
        pDocument->RemoveFromFormulaTree( this ); // update formula count
        delete pCode;
        pCode = new ScTokenArray( *pShared->GetCode() );
        bRefChanged = true;
        pCode->Reset();
        while( (t = static_cast<ScToken*>(pCode->GetNextReference())) != NULL )
        {
            if( t->GetType() != svIndex )
            {
                t->CalcAbsIfRel( aOldPos );
                bool bMod;
                {   // Own scope for SingleDoubleRefModifier dtor if SingleRef
                    SingleDoubleRefModifier aMod( *t );
                    ScComplexRefData& rRef = aMod.Ref();
                    bMod = (ScRefUpdate::UpdateTranspose( pDocument, rSource,
                        rDest, rRef ) != UR_NOTHING || bPosChanged);
                }
                if ( bMod )
                    t->CalcRelFromAbs( aPos );
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

bool ScFormulaCell::IsChanged() const
{
    return bChanged;
}

void ScFormulaCell::ResetChanged()
{
    bChanged = false;
}

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
            rtl::OUString aFormula;
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
            rtl::OUString aFormula;
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

// we really want to be a lot more descriptive than this
struct ScSimilarFormulaDelta : std::vector< size_t >
{
    bool IsCompatible( ScSimilarFormulaDelta *pDelta )
    {
        if ( size() != pDelta->size() )
            return false;
        for ( size_t i = 0; i < size(); i++ )
        {
            if ( (*this)[ i ] != (*pDelta)[ i ] )
                return false;
        }
        return true;
    }

    void push_delta( const ScSingleRefData& a, const ScSingleRefData& b )
    {
        push_back( b.nCol - a.nCol );
        push_back( b.nRow - a.nRow );
        push_back( b.nTab - a.nTab );
    }

    /// if the vector is zero then nothing changes down the column.
    bool IsInvariant() const
    {
        for ( size_t i = 0; i < size(); i++ )
        {
            if ( (*this)[ i ] != 0 )
                return false;
        }
        return true;
    }
};

bool ScFormulaCellGroup::IsCompatible( ScSimilarFormulaDelta *pDelta )
{
    return pDelta && mpDelta && mpDelta->IsCompatible( pDelta );
}

/// compare formulae tokens and build a series of deltas describing
/// the difference - ie. the result, when added to this
/// formulae should produce pOther
ScSimilarFormulaDelta *ScFormulaCell::BuildDeltaTo( ScFormulaCell *pOtherCell )
{
    // no Matrix formulae yet.
    if ( GetMatrixFlag() != MM_NONE )
        return NULL;

    // are these formule at all similar ?
    if ( GetHash() != pOtherCell->GetHash() )
        return NULL;

    FormulaToken **pThis = pCode->GetCode();
    sal_uInt16     pThisLen = pCode->GetCodeLen();
    FormulaToken **pOther = pOtherCell->pCode->GetCode();
    sal_uInt16     pOtherLen = pOtherCell->pCode->GetCodeLen();

    if ( !pThis || !pOther )
    {
//        fprintf( stderr, "Error: no compiled code for cells !" );
        return NULL;
    }

    if ( pThisLen != pOtherLen )
        return NULL;

    // check we are basically the same function
    for ( sal_uInt16 i = 0; i < pThisLen; i++ )
    {
        if ( pThis[ i ]->GetType() != pOther[ i ]->GetType() ||
             pThis[ i ]->GetOpCode() != pOther[ i ]->GetOpCode() ||
             pThis[ i ]->GetParamCount() != pOther[ i ]->GetParamCount() )
        {
//            fprintf( stderr, "Incompatible type, op-code or param counts\n" );
            return NULL;
        }
        switch( pThis[ i ]->GetType() )
        {
        case formula::svMatrix:
        case formula::svExternalSingleRef:
        case formula::svExternalDoubleRef:
//            fprintf( stderr, "Ignoring matrix and external references for now\n" );
            return NULL;
        default:
            break;
        }
    }

    ScSimilarFormulaDelta *pDelta = new ScSimilarFormulaDelta();

    for ( sal_uInt16 i = 0; i < pThisLen; i++ )
    {
        ScToken *pThisTok = static_cast< ScToken * >( pThis[ i ] );
        ScToken *pOtherTok = static_cast< ScToken * >( pOther[ i ] );

        if ( pThis[i]->GetType() == formula::svSingleRef ||
             pThis[i]->GetType() == formula::svDoubleRef )
        {
            const ScSingleRefData& aThisRef = pThisTok->GetSingleRef();
            const ScSingleRefData& aOtherRef = pOtherTok->GetSingleRef();
            pDelta->push_delta( aThisRef, aOtherRef );
        }
        if ( pThis[i]->GetType() == formula::svDoubleRef )
        {
            const ScSingleRefData& aThisRef2 = pThisTok->GetSingleRef2();
            const ScSingleRefData& aOtherRef2 = pOtherTok->GetSingleRef2();
            pDelta->push_delta( aThisRef2, aOtherRef2 );
        }
    }

    return pDelta;
}

/// To avoid exposing impl. details of ScSimilarFormulaDelta publicly
void ScFormulaCell::ReleaseDelta( ScSimilarFormulaDelta *pDelta )
{
    delete pDelta;
}

bool ScFormulaCell::InterpretFormulaGroup()
{
    // Re-build formulae groups if necessary - ideally this is done at
    // import / insert / delete etc. and is integral to the data structures
    pDocument->RebuildFormulaGroups();

    if( !xGroup.get() )
        return false;

//    fprintf( stderr, "Interpret cell %d, %d\n", (int)aPos.Col(), (int)aPos.Row() );

    if ( xGroup->mpDelta->IsInvariant() )
    {
//        fprintf( stderr, "struck gold - completely invariant for %d items !\n",
//                 (int)xGroup->mnLength );

        // calculate ourselves:
        InterpretTail( SCITP_NORMAL );
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

            // FIXME: there is a view / refresh missing here it appears.
        }
        return true;
    }
    else
    {
        // scan the formula ...
        // have a document method: "Get2DRangeAsDoublesArray" that does the
        // column-based heavy lifting call it for each absolute range from the
        // first cell pos in the formula group.
        //
        // Project single references to ranges by adding their vector * xGroup->mnLength
        //
        // TODO:
        //    elide multiple dimensional movement in vectors eg. =SUM(A1<1,1>)
        //    produces a diagonal 'column' that serves no useful purpose for us.
        //    these should be very rare. Should elide in GetDeltas anyway and
        //    assert here.
        //
        // Having built our input data ...
        // Throw it, and the formula over to some 'OpenCLCalculage' hook
        //
        // on return - release references on these double buffers
        //
        // transfer the result to the formula cells (as above)
        // store the doubles in the columns' maDoubles array for
        // dependent formulae
        //
        // TODO:
        //    need to abort/fail when we get errors returned and fallback to
        //    stock interpreting [ I guess ], unless we can use NaN etc. to
        //    signal errors.

        return false;
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
        StackVar eType = t->GetType();
        ScSingleRefData& rRef1 = t->GetSingleRef();
        ScSingleRefData& rRef2 = (eType == svDoubleRef ?
            t->GetDoubleRef().Ref2 : rRef1);
        switch( eType )
        {
            case svSingleRef:
                rRef1.CalcAbsIfRel(aPos);
                if ( rRef1.Valid() )
                {
                    pDoc->StartListeningCell(
                        ScAddress( rRef1.nCol,
                                   rRef1.nRow,
                                   rRef1.nTab ), this );
                }
            break;
            case svDoubleRef:
                t->CalcAbsIfRel(aPos);
                if ( rRef1.Valid() && rRef2.Valid() )
                {
                    if ( t->GetOpCode() == ocColRowNameAuto )
                    {   // automagically
                        if ( rRef1.IsColRel() )
                        {   // ColName
                            pDoc->StartListeningArea( ScRange (
                                rRef1.nCol,
                                rRef1.nRow,
                                rRef1.nTab,
                                rRef2.nCol,
                                MAXROW,
                                rRef2.nTab ), this );
                        }
                        else
                        {   // RowName
                            pDoc->StartListeningArea( ScRange (
                                rRef1.nCol,
                                rRef1.nRow,
                                rRef1.nTab,
                                MAXCOL,
                                rRef2.nRow,
                                rRef2.nTab ), this );
                        }
                    }
                    else
                    {
                        pDoc->StartListeningArea( ScRange (
                            rRef1.nCol,
                            rRef1.nRow,
                            rRef1.nTab,
                            rRef2.nCol,
                            rRef2.nRow,
                            rRef2.nTab ), this );
                    }
                }
            break;
            default:
                ;   // nothing
        }
    }
    SetNeedsListening( false);
}

//  pArr gesetzt -> Referenzen von anderer Zelle nehmen
// Then aPos must also be commited

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
        StackVar eType = t->GetType();
        ScSingleRefData& rRef1 = t->GetSingleRef();
        ScSingleRefData& rRef2 = (eType == svDoubleRef ?
            t->GetDoubleRef().Ref2 : rRef1);
        switch( eType )
        {
            case svSingleRef:
                rRef1.CalcAbsIfRel( aCellPos );
                if ( rRef1.Valid() )
                {
                    pDoc->EndListeningCell(
                        ScAddress( rRef1.nCol,
                                   rRef1.nRow,
                                   rRef1.nTab ), this );
                }
            break;
            case svDoubleRef:
                t->CalcAbsIfRel( aCellPos );
                if ( rRef1.Valid() && rRef2.Valid() )
                {
                    if ( t->GetOpCode() == ocColRowNameAuto )
                    {   // automagically
                        if ( rRef1.IsColRel() )
                        {   // ColName
                            pDoc->EndListeningArea( ScRange (
                                rRef1.nCol,
                                rRef1.nRow,
                                rRef1.nTab,
                                rRef2.nCol,
                                MAXROW,
                                rRef2.nTab ), this );
                        }
                        else
                        {   // RowName
                            pDoc->EndListeningArea( ScRange (
                                rRef1.nCol,
                                rRef1.nRow,
                                rRef1.nTab,
                                MAXCOL,
                                rRef2.nRow,
                                rRef2.nTab ), this );
                        }
                    }
                    else
                    {
                        pDoc->EndListeningArea( ScRange (
                            rRef1.nCol,
                            rRef1.nRow,
                            rRef1.nTab,
                            rRef2.nCol,
                            rRef2.nRow,
                            rRef2.nTab ), this );
                    }
                }
            break;
            default:
                ;   // nothing
        }
    }
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
