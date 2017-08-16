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

#ifndef INCLUDED_SC_INC_FORMULACELL_HXX
#define INCLUDED_SC_INC_FORMULACELL_HXX

#include <set>
#include <memory>

#include <formula/tokenarray.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <svl/listener.hxx>

#include "types.hxx"

#include "formularesult.hxx"

namespace sc {

class CompiledFormula;
class StartListeningContext;
class EndListeningContext;
struct RefUpdateContext;
struct RefUpdateInsertTabContext;
struct RefUpdateDeleteTabContext;
struct RefUpdateMoveTabContext;
class CompileFormulaContext;
class FormulaGroupAreaListener;
class UpdatedRangeNames;

}

class ScFormulaCell;
class ScProgress;
class ScTokenArray;

struct SC_DLLPUBLIC ScFormulaCellGroup
{
private:
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:

    mutable size_t mnRefCount;

    ScTokenArray* mpCode;
    ScFormulaCell *mpTopCell;
    SCROW mnLength; // How many of these do we have ?
    short mnFormatType;
    bool mbInvariant:1;
    bool mbSubTotal:1;

    sal_uInt8 meCalcState;

    ScFormulaCellGroup();
    ScFormulaCellGroup(const ScFormulaCellGroup&) = delete;
    const ScFormulaCellGroup& operator=(const ScFormulaCellGroup&) = delete;
    ~ScFormulaCellGroup();

    void setCode( const ScTokenArray& rCode );
    void setCode( ScTokenArray* pCode );
    void compileCode(
        ScDocument& rDoc, const ScAddress& rPos, formula::FormulaGrammar::Grammar eGram );

    sc::FormulaGroupAreaListener* getAreaListener(
        ScFormulaCell** ppTopCell, const ScRange& rRange, bool bStartFixed, bool bEndFixed );

    void endAllGroupListening( ScDocument& rDoc );
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

enum class ScMatrixMode : sal_uInt8 {
    NONE      = 0,                   // No matrix formula
    Formula   = 1,                   // Upper left matrix formula cell
    Reference = 2                    // Remaining cells, via ocMatRef reference token
};

class SC_DLLPUBLIC ScFormulaCell : public SvtListener
{
private:
    ScFormulaCellGroupRef mxGroup;       // Group of formulae we're part of
    ScFormulaResult aResult;
    formula::FormulaGrammar::Grammar  eTempGrammar;   // used between string (creation) and (re)compilation
    ScTokenArray*   pCode;              // The (new) token array
    ScDocument*     pDocument;
    ScFormulaCell*  pPrevious;
    ScFormulaCell*  pNext;
    ScFormulaCell*  pPreviousTrack;
    ScFormulaCell*  pNextTrack;
    sal_uInt16      nSeenInIteration;   // Iteration cycle in which the cell was last encountered
    short           nFormatType;
    ScMatrixMode    cMatrixFlag;
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
    bool            mbAllowNumberFormatChange : 1; /* allow setting further calculated
                                                      number formats as hard number format */
    bool            mbPostponedDirty : 1;   // if cell needs to be set dirty later
    bool            mbIsExtRef       : 1; // has references in ScExternalRefManager; never cleared after set

    /**
     * Update reference in response to cell copy-n-paste.
     */
    bool UpdateReferenceOnCopy(
        const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos );

    ScFormulaCell( const ScFormulaCell& ) = delete;
public:


                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
    void            InterpretTail( ScInterpretTailParameter, bool bUpdateProgress );

    enum CompareState { NotEqual = 0, EqualInvariant, EqualRelativeRef };

    DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )

    ScAddress       aPos;

                    virtual ~ScFormulaCell() override;

    ScFormulaCell* Clone() const;
    ScFormulaCell* Clone( const ScAddress& rPos ) const;

    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos );

    /**
     * Transfer the ownership of the passed token array instance to the
     * formula cell being constructed.  The caller <i>must not</i> pass a NULL
     * token array pointer.
     */
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos, ScTokenArray* pArray,
                   const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT,
                   ScMatrixMode cMatInd = ScMatrixMode::NONE );

    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos, const ScTokenArray& rArray,
                   const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT,
                   ScMatrixMode cMatInd = ScMatrixMode::NONE );

    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos, const ScFormulaCellGroupRef& xGroup,
                   const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                   ScMatrixMode = ScMatrixMode::NONE );

    /** With formula string and grammar to compile with.
       formula::FormulaGrammar::GRAM_DEFAULT effectively isformula::FormulaGrammar::GRAM_NATIVE_UI that
        also includes formula::FormulaGrammar::CONV_UNSPECIFIED, therefore uses the address
        convention associated with rPos::nTab by default. */
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                    const OUString& rFormula,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    ScMatrixMode cMatInd = ScMatrixMode::NONE );

    ScFormulaCell(const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, ScCloneFlags nCloneFlags = ScCloneFlags::Default);

    size_t GetHash() const;

    ScFormulaVectorState GetVectorState() const;

    void            GetFormula( OUString& rFormula,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    void            GetFormula( OUStringBuffer& rBuffer,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    OUString GetFormula( sc::CompileFormulaContext& rCxt ) const;

    void            SetDirty( bool bDirtyFlag=true );
    void            SetDirtyVar();
    // If setting entire document dirty after load, no broadcasts but still append to FormulaTree.
    void            SetDirtyAfterLoad();
    void ResetTableOpDirtyVar();
    void            SetTableOpDirty();
    bool            IsDirtyOrInTableOpDirty() const;
    bool GetDirty() const { return bDirty; }
    void ResetDirty();
    bool NeedsListening() const { return bNeedListening; }
    void SetNeedsListening( bool bVar );
    void SetNeedsDirty( bool bVar );
    void SetNeedNumberFormat( bool bVal );
    bool NeedsNumberFormat() const { return mbNeedsNumberFormat;}
    short GetFormatType() const { return nFormatType; }
    void            Compile(const OUString& rFormula,
                            bool bNoListening,
                            const formula::FormulaGrammar::Grammar );
    void Compile(
        sc::CompileFormulaContext& rCxt, const OUString& rFormula, bool bNoListening = false );

    void CompileTokenArray( bool bNoListening = false );
    void CompileTokenArray( sc::CompileFormulaContext& rCxt, bool bNoListening = false );
    void CompileXML( sc::CompileFormulaContext& rCxt, ScProgress& rProgress );        // compile temporary string tokens
    void CalcAfterLoad( sc::CompileFormulaContext& rCxt, bool bStartListening );
    bool            MarkUsedExternalReferences();
    void            Interpret();
    bool IsIterCell() const { return bIsIterCell; }
    sal_uInt16 GetSeenInIteration() const { return nSeenInIteration; }

    bool            HasOneReference( ScRange& r ) const;
    /* Checks if the formula contains reference list that can be
       expressed by one reference (like A1;A2;A3:A5 -> A1:A5). The
       reference list is not required to be sorted (i.e. A3;A1;A2 is
       still recognized as A1:A3), but no overlapping is allowed.
       If one reference is recognized, the rRange is filled.

       It is similar to HasOneReference(), but more general.
     */
    bool HasRefListExpressibleAsOneReference(ScRange& rRange) const;

    enum class RelNameRef
    {
        NONE,   ///< no relative reference from named expression
        SINGLE, ///< only single cell relative reference
        DOUBLE  ///< at least one range relative reference from named expression
    };
    RelNameRef      HasRelNameReference() const;

    bool UpdateReference(
        const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc = nullptr, const ScAddress* pUndoCellPos = nullptr );

    /**
     * Shift the position of formula cell as part of reference update.
     *
     * @return true if the position has shifted, false otherwise.
     */
    bool UpdatePosOnShift( const sc::RefUpdateContext& rCxt );

    /**
     * Update reference in response to cell insertion or deletion.
     */
    bool UpdateReferenceOnShift(
        const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos );

    /**
     * Update reference in response to cell move.
     */
    bool UpdateReferenceOnMove(
        const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos );

    void            TransposeReference();
    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                        ScDocument* pUndoDoc );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void            UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt );
    void            UpdateInsertTabAbs(SCTAB nTable);
    void            UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt );
    void            UpdateMoveTab( const sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo );
    bool            TestTabRefAbs(SCTAB nTable);
    void            UpdateCompile( bool bForceIfNameInUse );
    void            FindRangeNamesInUse(sc::UpdatedRangeNames& rIndexes) const;
    bool            IsSubTotal() const { return bSubTotal;}
    bool            IsChanged() const { return bChanged;}
    void            SetChanged(bool b);
    bool            IsEmpty();      // formula::svEmptyCell result
                    // display as empty string if formula::svEmptyCell result
    bool            IsEmptyDisplayedAsString();
    bool            IsValue();      // also true if formula::svEmptyCell
    bool            IsValueNoError();
    bool            IsValueNoError() const;
    double          GetValue();
    svl::SharedString GetString();

    /**
     * Get a numeric value without potentially triggering re-calculation.
     */
    double GetRawValue() const;

    /**
     * Get a string value without potentially triggering re-calculation.
     */
    svl::SharedString GetRawString() const;
    const ScMatrix* GetMatrix();
    bool            GetMatrixOrigin( ScAddress& rPos ) const;
    void            GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
    sc::MatrixEdge  GetMatrixEdge( ScAddress& rOrgPos ) const;
    FormulaError    GetErrCode();   // interpret first if necessary
    FormulaError    GetRawError();  // don't interpret, just return code or result error
    bool            GetErrorOrValue( FormulaError& rErr, double& rVal );
    sc::FormulaResultValue GetResult();
    sc::FormulaResultValue GetResult() const;
    ScMatrixMode    GetMatrixFlag() const { return cMatrixFlag;}
    ScTokenArray*   GetCode() { return pCode;}
    const ScTokenArray* GetCode() const { return pCode;}

    void SetCode( ScTokenArray* pNew );

    bool            IsRunning() const { return bRunning;}
    void            SetRunning( bool bVal );
    void CompileDBFormula( sc::CompileFormulaContext& rCxt );
    void CompileColRowNameFormula( sc::CompileFormulaContext& rCxt );
    ScFormulaCell*  GetPrevious() const { return pPrevious; }
    ScFormulaCell*  GetNext() const { return pNext; }
    void            SetPrevious( ScFormulaCell* pF );
    void            SetNext( ScFormulaCell* pF );
    ScFormulaCell*  GetPreviousTrack() const { return pPreviousTrack; }
    ScFormulaCell*  GetNextTrack() const { return pNextTrack; }
    void            SetPreviousTrack( ScFormulaCell* pF );
    void            SetNextTrack( ScFormulaCell* pF );

    virtual void Notify( const SfxHint& rHint ) override;
    virtual void Query( SvtListener::QueryBase& rQuery ) const override;

    void SetCompile( bool bVal );
    ScDocument* GetDocument() const { return pDocument;}
    void            SetMatColsRows( SCCOL nCols, SCROW nRows );
    void            GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const;

                    // cell belongs to ChangeTrack and not to the real document
    void SetInChangeTrack( bool bVal );
    bool IsInChangeTrack() const { return bInChangeTrack;}

    // For import filters!
    void            AddRecalcMode( ScRecalcMode );
    /** For import only: set a double result. */
    void SetHybridDouble( double n );
    /** For import only: set a string result.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void SetHybridString( const svl::SharedString& r );
    /** For import only: set an empty cell result to be displayed as empty string.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridEmptyDisplayedAsString() or SetHybridFormula(), use
        SetHybridDouble() first for performance reasons and use
        SetHybridEmptyDisplayedAsString() last because SetHybridDouble() and
        SetHybridString() will override it.*/
    void SetHybridEmptyDisplayedAsString();
    /** For import only: set a temporary formula string to be compiled later.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void SetHybridFormula(
        const OUString& r, const formula::FormulaGrammar::Grammar eGrammar );

    OUString GetHybridFormula() const;

    void SetResultMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, const formula::FormulaToken* pUL );

    /** For import only: set a double result.
        Use this instead of SetHybridDouble() if there is no (temporary)
        formula string because the formula is present as a token array, as it
        is the case for binary Excel import.
     */
    void SetResultDouble( double n );

    void SetResultToken( const formula::FormulaToken* pToken );

    svl::SharedString GetResultString() const;

    /* Sets the shared code array to error state in addition to the cell result */
    void SetErrCode( FormulaError n );

    /* Sets just the result to error */
    void SetResultError( FormulaError n );

    bool IsHyperLinkCell() const;
    EditTextObject* CreateURLObject();
    void GetURLResult( OUString& rURL, OUString& rCellText );

    /** Determines whether or not the result string contains more than one paragraph */
    bool            IsMultilineResult();

    bool NeedsInterpret() const;

    void            MaybeInterpret();

    /**
     * Turn a non-grouped cell into the top of a grouped cell.
     */
    ScFormulaCellGroupRef CreateCellGroup( SCROW nLen, bool bInvariant );
    const ScFormulaCellGroupRef& GetCellGroup() const { return mxGroup;}
    void SetCellGroup( const ScFormulaCellGroupRef &xRef );

    CompareState CompareByTokenArray( const ScFormulaCell& rOther ) const;

    bool InterpretFormulaGroup();
    bool InterpretInvariantFormulaGroup();

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void StartListeningTo( ScDocument* pDoc );
    void StartListeningTo( sc::StartListeningContext& rCxt );
    void EndListeningTo(
        ScDocument* pDoc, ScTokenArray* pArr = nullptr, ScAddress aPos = ScAddress() );
    void EndListeningTo( sc::EndListeningContext& rCxt );

    bool IsShared() const;
    bool IsSharedTop() const;
    SCROW GetSharedTopRow() const;
    SCROW GetSharedLength() const;

    // An estimate of the number of cells referenced by the formula
    sal_Int32 GetWeight() const;

    ScTokenArray* GetSharedCode();
    const ScTokenArray* GetSharedCode() const;

    void SyncSharedCode();

    bool IsPostponedDirty() const { return mbPostponedDirty;}

    void SetIsExtRef() { mbIsExtRef = true; }

#if DUMP_COLUMN_STORAGE
    void Dump() const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
