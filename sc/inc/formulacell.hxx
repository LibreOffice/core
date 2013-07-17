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

#ifndef SC_FORMULACELL_HXX
#define SC_FORMULACELL_HXX

#include "formularesult.hxx"

#include "formula/tokenarray.hxx"
#include "svl/listener.hxx"
#include "types.hxx"

#include <set>

namespace sc {

class StartListeningContext;
class EndListeningContext;
struct RefUpdateContext;

}

class ScProgress;
class ScTokenArray;
struct ScSimilarFormulaDelta;

struct SC_DLLPUBLIC ScFormulaCellGroup
{
    mutable size_t mnRefCount;

    SCROW mnStart;  // Start offset of that cell
    SCROW mnLength; // How many of these do we have ?
    bool mbInvariant;
    sc::GroupCalcState meCalcState;

    ScFormulaCellGroup();
    ~ScFormulaCellGroup();
};

inline void intrusive_ptr_add_ref(const ScFormulaCellGroup *p)
{
    p->mnRefCount++;
}

inline void intrusive_ptr_release(const ScFormulaCellGroup *p)
{
    if( --p->mnRefCount == 0 )
        delete p;
}

enum ScMatrixMode {
    MM_NONE      = 0,                   // No matrix formula
    MM_FORMULA   = 1,                   // Upper left matrix formula cell
    MM_REFERENCE = 2,                   // Remaining cells, via ocMatRef reference token
    MM_FAKE      = 3                    // Interpret "as-if" matrix formula (legacy)
};

class SC_DLLPUBLIC ScFormulaCell : public SvtListener
{
private:
    ScFormulaResult aResult;
    formula::FormulaGrammar::Grammar  eTempGrammar;   // used between string (creation) and (re)compilation
    ScTokenArray*   pCode;              // The (new) token array
    ScDocument*     pDocument;
    ScFormulaCell*  pPrevious;
    ScFormulaCell*  pNext;
    ScFormulaCell*  pPreviousTrack;
    ScFormulaCell*  pNextTrack;
    ScFormulaCellGroupRef xGroup;       // re-factoring hack - group of formulae we're part of.
    sal_uInt16      nSeenInIteration;   // Iteration cycle in which the cell was last encountered
    sal_uInt8       cMatrixFlag;        // One of ScMatrixMode
    short           nFormatType;
    bool            bDirty         : 1; // Must be (re)calculated
    bool            bChanged       : 1; // Whether something changed regarding display/representation
    bool            bRunning       : 1; // Already interpreting right now
    bool            bCompile       : 1; // Must be (re)compiled
    bool            bSubTotal      : 1; // Cell is part of or contains a SubTotal
    bool            bIsIterCell    : 1; // Cell is part of a circular reference
    bool            bInChangeTrack : 1; // Cell is in ChangeTrack
    bool            bTableOpDirty  : 1; // Dirty flag for TableOp
    bool            bNeedListening : 1; // Listeners need to be re-established after UpdateReference
    bool            mbNeedsNumberFormat : 1; // set the calculated number format as hard number format

                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
    void            InterpretTail( ScInterpretTailParameter );

    ScFormulaCell( const ScFormulaCell& );
public:

    enum CompareState { NotEqual = 0, EqualInvariant, EqualRelativeRef };

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

    ScAddress       aPos;

                    ~ScFormulaCell();

    ScFormulaCell* Clone() const;

    /** Empty formula cell, or with a preconstructed token array. */
    ScFormulaCell( ScDocument*, const ScAddress&, const ScTokenArray* = NULL,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    sal_uInt8 = MM_NONE );

    /** With formula string and grammar to compile with.
       formula::FormulaGrammar::GRAM_DEFAULT effectively isformula::FormulaGrammar::GRAM_NATIVE_UI that
        also includes formula::FormulaGrammar::CONV_UNSPECIFIED, therefor uses the address
        convention associated with rPos::nTab by default. */
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                    const OUString& rFormula,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    sal_uInt8 cMatInd = MM_NONE );

    ScFormulaCell( const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, int nCloneFlags = SC_CLONECELL_DEFAULT );

    size_t GetHash() const;

    ScFormulaVectorState GetVectorState() const;

    void            GetFormula( OUString& rFormula,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    void            GetFormula( OUStringBuffer& rBuffer,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    void            SetDirty( bool bDirtyFlag=true );
    void            SetDirtyVar();
    // If setting entire document dirty after load, no broadcasts but still append to FormulaTree.
    void            SetDirtyAfterLoad();
    void ResetTableOpDirtyVar();
    void            SetTableOpDirty();
    bool            IsDirtyOrInTableOpDirty() const;
    bool GetDirty() const;
    void ResetDirty();
    bool NeedsListening() const;
    void SetNeedsListening( bool bVar );
    void SetNeedNumberFormat( bool bVal );
    short GetFormatType() const;
    void            Compile(const OUString& rFormula,
                            bool bNoListening = false,
                            const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    void            CompileTokenArray( bool bNoListening = false );
    void            CompileXML( ScProgress& rProgress );        // compile temporary string tokens
    void            CalcAfterLoad();
    bool            MarkUsedExternalReferences();
    void            Interpret();
    bool IsIterCell() const;
    sal_uInt16 GetSeenInIteration() const;

    bool            HasOneReference( ScRange& r ) const;
    /* Checks if the formula contains reference list that can be
       expressed by one reference (like A1;A2;A3:A5 -> A1:A5). The
       reference list is not required to be sorted (i.e. A3;A1;A2 is
       still recognized as A1:A3), but no overlapping is allowed.
       If one reference is recognized, the rRange is filled.

       It is similar to HasOneReference(), but more general.
     */
    bool HasRefListExpressibleAsOneReference(ScRange& rRange) const;
    bool            HasRelNameReference() const;
    bool            HasColRowName() const;

    bool UpdateReference(
        const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = NULL, const ScAddress* pUndoCellPos = NULL );

    void            TransposeReference();
    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                        ScDocument* pUndoDoc );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void            UpdateInsertTab(SCTAB nTable, SCTAB nNewSheets = 1);
    void            UpdateInsertTabAbs(SCTAB nTable);
    bool            UpdateDeleteTab(SCTAB nTable, bool bIsMove = false, SCTAB nSheets = 1);
    void            UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
    void            UpdateRenameTab(SCTAB nTable, const OUString& rName);
    bool            TestTabRefAbs(SCTAB nTable);
    void            UpdateCompile( bool bForceIfNameInUse = false );
    void            FindRangeNamesInUse(std::set<sal_uInt16>& rIndexes) const;
    bool IsSubTotal() const;
    bool            IsChanged() const;
    void SetChanged(bool b);
    bool            IsEmpty();      // formula::svEmptyCell result
                    // display as empty string if formula::svEmptyCell result
    bool            IsEmptyDisplayedAsString();
    bool            IsValue();      // also true if formula::svEmptyCell
    bool IsValueNoError();
    bool            IsHybridValueCell(); // for cells after import to deal with inherited number formats
    double          GetValue();
    double          GetValueAlways();   // ignore errors
    OUString   GetString();
    const ScMatrix* GetMatrix();
    bool            GetMatrixOrigin( ScAddress& rPos ) const;
    void            GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
    sal_uInt16 GetMatrixEdge( ScAddress& rOrgPos ) const;
    sal_uInt16      GetErrCode();   // interpret first if necessary
    sal_uInt16      GetRawError();  // don't interpret, just return code or result error
    bool GetErrorOrValue( sal_uInt16& rErr, double& rVal );
    sal_uInt8       GetMatrixFlag() const;
    ScTokenArray*   GetCode() const;

    bool            IsRunning() const;
    void            SetRunning( bool bVal );
    void            CompileDBFormula();
    void            CompileDBFormula( bool bCreateFormulaString );
    void            CompileNameFormula( bool bCreateFormulaString );
    void            CompileColRowNameFormula();
    ScFormulaCell*  GetPrevious() const;
    ScFormulaCell*  GetNext() const;
    void            SetPrevious( ScFormulaCell* pF );
    void            SetNext( ScFormulaCell* pF );
    ScFormulaCell*  GetPreviousTrack() const;
    ScFormulaCell*  GetNextTrack() const;
    void            SetPreviousTrack( ScFormulaCell* pF );
    void            SetNextTrack( ScFormulaCell* pF );

    virtual void    Notify( SvtBroadcaster& rBC, const SfxHint& rHint);
    void SetCompile( bool bVal );
    ScDocument* GetDocument() const;
    void            SetMatColsRows( SCCOL nCols, SCROW nRows, bool bDirtyFlag=true );
    void            GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const;

                    // cell belongs to ChangeTrack and not to the real document
    void SetInChangeTrack( bool bVal );
    bool IsInChangeTrack() const;

    // For import filters!
    void            AddRecalcMode( formula::ScRecalcMode );
    /** For import only: set a double result. */
    void SetHybridDouble( double n );
    /** For import only: set a string result.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void SetHybridString( const OUString& r );
    /** For import only: set a temporary formula string to be compiled later.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void SetHybridFormula(
        const OUString& r, const formula::FormulaGrammar::Grammar eGrammar );

    void SetResultMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL );

    /** For import only: set a double result.
        Use this instead of SetHybridDouble() if there is no (temporary)
        formula string because the formula is present as a token array, as it
        is the case for binary Excel import.
     */
    void SetResultDouble( double n );

    void SetResultToken( const formula::FormulaToken* pToken );

    double GetResultDouble() const;

    void            SetErrCode( sal_uInt16 n );
    bool IsHyperLinkCell() const;
    EditTextObject* CreateURLObject();
    void GetURLResult( OUString& rURL, OUString& rCellText );

    /** Determines whether or not the result string contains more than one paragraph */
    bool            IsMultilineResult();

    void            MaybeInterpret();

    ScFormulaCellGroupRef GetCellGroup();
    void SetCellGroup( const ScFormulaCellGroupRef &xRef );

    CompareState CompareByTokenArray( ScFormulaCell& rOther ) const;

    bool InterpretFormulaGroup();
    bool InterpretInvariantFormulaGroup();

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void StartListeningTo( ScDocument* pDoc );
    void StartListeningTo( sc::StartListeningContext& rCxt );
    void EndListeningTo(
        ScDocument* pDoc, ScTokenArray* pArr = NULL, ScAddress aPos = ScAddress() );
    void EndListeningTo( sc::EndListeningContext& rCxt );

    bool IsShared() const;
    bool IsSharedInvariant() const;
    SCROW GetSharedTopRow() const;
    SCROW GetSharedLength() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
