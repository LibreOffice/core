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

#pragma once

#include <map>
#include <memory>
#include <optional>

#include <formula/tokenarray.hxx>
#include <formula/errorcodes.hxx>
#include <svl/listener.hxx>

#include "types.hxx"
#include "interpretercontext.hxx"
#include "document.hxx"
#include "docoptio.hxx"
#include "formulalogger.hxx"
#include "formularesult.hxx"
#include "tokenarray.hxx"
#include "grouparealistener.hxx"

namespace sc {

class StartListeningContext;
class EndListeningContext;
struct RefUpdateContext;
struct RefUpdateInsertTabContext;
struct RefUpdateDeleteTabContext;
struct RefUpdateMoveTabContext;
class CompileFormulaContext;
class UpdatedRangeNames;

}

class ScFormulaCell;
class ScProgress;
enum class SvNumFormatType : sal_Int16;

struct AreaListenerKey
{
    ScRange maRange;
    bool mbStartFixed;
    bool mbEndFixed;

    AreaListenerKey( const ScRange& rRange, bool bStartFixed, bool bEndFixed ) :
        maRange(rRange), mbStartFixed(bStartFixed), mbEndFixed(bEndFixed) {}

    bool operator < ( const AreaListenerKey& r ) const;
};

typedef std::map<AreaListenerKey, sc::FormulaGroupAreaListener> AreaListenersType;

struct SC_DLLPUBLIC ScFormulaCellGroup
{
    AreaListenersType m_AreaListeners;
public:

    mutable size_t mnRefCount;

    std::optional<ScTokenArray> mpCode;
    ScFormulaCell *mpTopCell;
    SCROW mnLength; // How many of these do we have ?
    sal_Int32 mnWeight;
    SvNumFormatType mnFormatType;
    bool mbInvariant:1;
    bool mbSubTotal:1;
    bool mbPartOfCycle:1; // To flag FG's part of a cycle

    sal_uInt8 meCalcState;

    ScFormulaCellGroup();
    ScFormulaCellGroup(const ScFormulaCellGroup&) = delete;
    const ScFormulaCellGroup& operator=(const ScFormulaCellGroup&) = delete;
    ~ScFormulaCellGroup();

    void setCode( const ScTokenArray& rCode );
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

class SAL_DLLPUBLIC_RTTI ScFormulaCell final : public SvtListener
{
private:
    ScFormulaCellGroupRef mxGroup;       // Group of formulae we're part of
    bool            bDirty         : 1; // Must be (re)calculated
    bool            bTableOpDirty  : 1; // Dirty flag for TableOp
    bool            bChanged       : 1; // Whether something changed regarding display/representation
    bool            bRunning       : 1; // Already interpreting right now
    bool            bCompile       : 1; // Must be (re)compiled
    bool            bSubTotal      : 1; // Cell is part of or contains a SubTotal
    bool            bIsIterCell    : 1; // Cell is part of a circular reference
    bool            bInChangeTrack : 1; // Cell is in ChangeTrack
    bool            bNeedListening : 1; // Listeners need to be re-established after UpdateReference
    bool            mbNeedsNumberFormat : 1; // set the calculated number format as hard number format
    bool            mbAllowNumberFormatChange : 1; /* allow setting further calculated
                                                      number formats as hard number format */
    bool            mbPostponedDirty : 1;   // if cell needs to be set dirty later
    bool            mbIsExtRef       : 1; // has references in ScExternalRefManager; never cleared after set
    bool            mbSeenInPath     : 1; // For detecting cycle involving formula groups and singleton formulacells
    bool            mbFreeFlying     : 1; // Cell is out of sheets interpreted, like in conditional format
    ScMatrixMode    cMatrixFlag      : 8;
    sal_uInt16      nSeenInIteration : 16;   // Iteration cycle in which the cell was last encountered
    SvNumFormatType nFormatType      : 16;
    ScFormulaResult aResult;
    formula::FormulaGrammar::Grammar  eTempGrammar;   // used between string (creation) and (re)compilation
    // If this cell is in a cell group (mxGroup!=nullptr), then this pCode is a not-owning pointer
    // to the mxGroup's mpCode, which owns the array. If the cell is not in a group, this is an owning pointer.
    ScTokenArray*   pCode;              // The token array
    ScDocument&     rDocument;
    ScFormulaCell*  pPrevious;
    ScFormulaCell*  pNext;
    ScFormulaCell*  pPreviousTrack;
    ScFormulaCell*  pNextTrack;

    /**
     * Update reference in response to cell copy-n-paste.
     */
    bool UpdateReferenceOnCopy(
        const sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, const ScAddress* pUndoCellPos );

    ScFormulaCell( const ScFormulaCell& ) = delete;

    bool CheckComputeDependencies(sc::FormulaLogger::GroupScope& rScope, bool fromFirstRow,
                                  SCROW nStartOffset, SCROW nEndOffset, bool bCalcDependencyOnly = false,
                                  ScRangeList* pSuccessfulDependencies = nullptr,
                                  ScAddress* pFailedAndDirtiedAddress = nullptr);
    bool InterpretFormulaGroupThreading(sc::FormulaLogger::GroupScope& aScope,
                                        bool& bDependencyComputed,
                                        bool& bDependencyCheckFailed,
                                        SCROW nStartOffset, SCROW nEndOffset);
    bool InterpretFormulaGroupOpenCL(sc::FormulaLogger::GroupScope& aScope,
                                     bool& bDependencyComputed,
                                     bool& bDependencyCheckFailed);
    bool InterpretInvariantFormulaGroup();

public:


                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
                    void InterpretTail( ScInterpreterContext&, ScInterpretTailParameter );

    void            HandleStuffAfterParallelCalculation(ScInterpreter* pInterpreter);

    enum CompareState { NotEqual = 0, EqualInvariant, EqualRelativeRef };

    ScAddress       aPos;

    SC_DLLPUBLIC virtual ~ScFormulaCell() override;

    ScFormulaCell* Clone() const;
    ScFormulaCell* Clone( const ScAddress& rPos ) const;

    SC_DLLPUBLIC ScFormulaCell( ScDocument& rDoc, const ScAddress& rPos );

    /**
     * Transfer the ownership of the passed token array instance to the
     * formula cell being constructed.  The caller <i>must not</i> pass a NULL
     * token array pointer.
     */
    SC_DLLPUBLIC ScFormulaCell( ScDocument& rDoc, const ScAddress& rPos, std::unique_ptr<ScTokenArray> pArray,
                   const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT,
                   ScMatrixMode cMatInd = ScMatrixMode::NONE );

    SC_DLLPUBLIC ScFormulaCell( ScDocument& rDoc, const ScAddress& rPos, const ScTokenArray& rArray,
                   const formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT,
                   ScMatrixMode cMatInd = ScMatrixMode::NONE );

    SC_DLLPUBLIC ScFormulaCell( ScDocument& rDoc, const ScAddress& rPos, const ScFormulaCellGroupRef& xGroup,
                   const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                   ScMatrixMode = ScMatrixMode::NONE );

    /** With formula string and grammar to compile with.
       formula::FormulaGrammar::GRAM_DEFAULT effectively isformula::FormulaGrammar::GRAM_NATIVE_UI that
        also includes formula::FormulaGrammar::CONV_UNSPECIFIED, therefore uses the address
        convention associated with rPos::nTab by default. */
    ScFormulaCell( ScDocument& rDoc, const ScAddress& rPos,
                    const OUString& rFormula,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    ScMatrixMode cMatInd = ScMatrixMode::NONE );

    ScFormulaCell(const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, ScCloneFlags nCloneFlags = ScCloneFlags::Default);

    void            SetFreeFlying( bool b ) { mbFreeFlying = b; }

    size_t GetHash() const;

    SC_DLLPUBLIC OUString GetFormula( const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                         const ScInterpreterContext* pContext = nullptr ) const;
    SC_DLLPUBLIC OUString GetFormula( sc::CompileFormulaContext& rCxt, const ScInterpreterContext* pContext = nullptr ) const;

    SC_DLLPUBLIC void SetDirty( bool bDirtyFlag=true );
    void            SetDirtyVar();
    // If setting entire document dirty after load, no broadcasts but still append to FormulaTree.
    void            SetDirtyAfterLoad();
    void ResetTableOpDirtyVar();
    void            SetTableOpDirty();

    bool IsDirtyOrInTableOpDirty() const
    {
        return bDirty || (bTableOpDirty && rDocument.IsInInterpreterTableOp());
    }

    bool GetDirty() const { return bDirty; }
    SC_DLLPUBLIC void ResetDirty();
    bool NeedsListening() const { return bNeedListening; }
    void SetNeedsListening( bool bVar );
    void SetNeedsDirty( bool bVar );
    SC_DLLPUBLIC void SetNeedNumberFormat( bool bVal );
    bool NeedsNumberFormat() const { return mbNeedsNumberFormat;}
    SvNumFormatType GetFormatType() const { return nFormatType; }
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
    // Returns true if the cell was interpreted as part of the formula group.
    // The parameters may limit which subset of the formula group should be interpreted, if possible.
    bool Interpret(SCROW nStartOffset = -1, SCROW nEndOffset = -1);
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

    void            UpdateInsertTab( const sc::RefUpdateInsertTabContext& rCxt );
    void            UpdateInsertTabAbs(SCTAB nTable);
    void            UpdateDeleteTab( const sc::RefUpdateDeleteTabContext& rCxt );
    void            UpdateMoveTab( const sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo );
    bool            TestTabRefAbs(SCTAB nTable);
    void            UpdateCompile( bool bForceIfNameInUse );
    void            FindRangeNamesInUse(sc::UpdatedRangeNames& rIndexes) const;
    bool            IsSubTotal() const { return bSubTotal;}
    bool            IsChanged() const { return bChanged;}
    SC_DLLPUBLIC void SetChanged(bool b);
    bool            IsEmpty();      // formula::svEmptyCell result
                    // display as empty string if formula::svEmptyCell result
    bool            IsEmptyDisplayedAsString();
    SC_DLLPUBLIC bool IsValue();      // also true if formula::svEmptyCell
    bool            IsValueNoError();
    bool            IsValueNoError() const;
    SC_DLLPUBLIC double GetValue();
    SC_DLLPUBLIC const svl::SharedString & GetString();

    /**
     * Get a numeric value without potentially triggering re-calculation.
     */
    double GetRawValue() const;

    /**
     * Get a string value without potentially triggering re-calculation.
     */
    const svl::SharedString & GetRawString() const;
    const ScMatrix* GetMatrix();
    SC_DLLPUBLIC bool GetMatrixOrigin( const ScDocument& rDoc, ScAddress& rPos ) const;
    void            GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
    sc::MatrixEdge  GetMatrixEdge( const ScDocument& rDoc, ScAddress& rOrgPos ) const;
    SC_DLLPUBLIC FormulaError GetErrCode();   // interpret first if necessary
    FormulaError    GetRawError() const;  // don't interpret, just return code or result error
    bool            GetErrorOrValue( FormulaError& rErr, double& rVal );
    SC_DLLPUBLIC sc::FormulaResultValue GetResult();
    SC_DLLPUBLIC sc::FormulaResultValue GetResult() const;
    ScMatrixMode    GetMatrixFlag() const { return cMatrixFlag;}
    ScTokenArray*   GetCode() { return pCode;}
    const ScTokenArray* GetCode() const { return pCode;}

    void SetCode( std::unique_ptr<ScTokenArray> pNew );

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
    ScDocument& GetDocument() const { return rDocument;}
    void            SetMatColsRows( SCCOL nCols, SCROW nRows );
    SC_DLLPUBLIC void GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const;

                    // cell belongs to ChangeTrack and not to the real document
    void SetInChangeTrack( bool bVal );
    bool IsInChangeTrack() const { return bInChangeTrack;}

    // For import filters!
    SC_DLLPUBLIC void AddRecalcMode( ScRecalcMode );
    /** For import only: set a double result. */
    SC_DLLPUBLIC void SetHybridDouble( double n );
    /** For import only: set a string result.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    SC_DLLPUBLIC void SetHybridString( const svl::SharedString& r );
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
    SC_DLLPUBLIC void SetResultDouble( double n );

    SC_DLLPUBLIC void SetResultToken( const formula::FormulaToken* pToken );

    const svl::SharedString & GetResultString() const;

    bool HasHybridStringResult() const;

    /* Sets the shared code array to error state in addition to the cell result */
    SC_DLLPUBLIC void SetErrCode( FormulaError n );

    /* Sets just the result to error */
    void SetResultError( FormulaError n );

    SC_DLLPUBLIC bool IsHyperLinkCell() const;
    std::unique_ptr<EditTextObject> CreateURLObject();
    SC_DLLPUBLIC void GetURLResult( OUString& rURL, OUString& rCellText );

    /** Determines whether or not the result string contains more than one paragraph */
    SC_DLLPUBLIC bool IsMultilineResult();

    bool NeedsInterpret() const
    {
        if (bIsIterCell)
            // Shortcut to force return of current value and not enter Interpret()
            // as we're looping over all iteration cells.
            return false;

        if (!IsDirtyOrInTableOpDirty())
            return false;

        return rDocument.GetAutoCalc() || (cMatrixFlag != ScMatrixMode::NONE)
            || (pCode->IsRecalcModeMustAfterImport() && !pCode->IsRecalcModeAlways());
    }

    void MaybeInterpret()
    {
        if (NeedsInterpret())
        {
            if (bRunning && !rDocument.GetDocOptions().IsIter() && rDocument.IsThreadedGroupCalcInProgress())
            {
                // This is actually copied from Interpret()'s if(bRunning)
                // block that once caught this circular reference but now is
                // prepended with various threaded group calc things which the
                // assert() below is supposed to fail on when entering again.
                // Nevertheless, we need some state here the caller can obtain.
                aResult.SetResultError( FormulaError::CircularReference );
            }
            else
            {
                assert(!rDocument.IsThreadedGroupCalcInProgress());
                Interpret();
            }
        }
    }

    /**
     * Turn a non-grouped cell into the top of a grouped cell.
     */
    SC_DLLPUBLIC ScFormulaCellGroupRef CreateCellGroup( SCROW nLen, bool bInvariant );
    const ScFormulaCellGroupRef& GetCellGroup() const { return mxGroup;}
    void SetCellGroup( const ScFormulaCellGroupRef &xRef );

    CompareState CompareByTokenArray( const ScFormulaCell& rOther ) const;

    bool InterpretFormulaGroup(SCROW nStartOffset = -1, SCROW nEndOffset = -1);

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void StartListeningTo( ScDocument& rDoc );
    void StartListeningTo( sc::StartListeningContext& rCxt );
    void EndListeningTo(
        ScDocument& rDoc, ScTokenArray* pArr = nullptr, ScAddress aPos = ScAddress() );
    void EndListeningTo( sc::EndListeningContext& rCxt );

    bool IsShared() const;
    SC_DLLPUBLIC bool IsSharedTop() const;
    SC_DLLPUBLIC SCROW GetSharedTopRow() const;
    SC_DLLPUBLIC SCROW GetSharedLength() const;

    // An estimate of the number of cells referenced by the formula
    sal_Int32 GetWeight() const;

    ScTokenArray* GetSharedCode();
    SC_DLLPUBLIC const ScTokenArray* GetSharedCode() const;

    void SyncSharedCode();

    bool IsPostponedDirty() const { return mbPostponedDirty;}

    void SetIsExtRef() { mbIsExtRef = true; }
    bool GetSeenInPath() const { return mbSeenInPath; }
    void SetSeenInPath(bool bSet) { mbSeenInPath = bSet; }

#if DUMP_COLUMN_STORAGE
    void Dump() const;
#endif
};

inline bool ScDocument::IsInFormulaTree( const ScFormulaCell* pCell ) const { return pCell->GetPrevious() || pFormulaTree == pCell; }
inline bool ScDocument::IsInFormulaTrack( const ScFormulaCell* pCell ) const { return pCell->GetPreviousTrack() || pFormulaTrack == pCell; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
