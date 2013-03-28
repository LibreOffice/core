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

#include "cell.hxx"
#include "formularesult.hxx"

#include "formula/tokenarray.hxx"
#include "svl/listener.hxx"

#include <set>

class ScTokenArray;
struct ScSimilarFormulaDelta;

struct SC_DLLPUBLIC ScFormulaCellGroup
{
    sal_Int32              mnRefCount;
    ScSimilarFormulaDelta *mpDelta;  // difference between items in column
    sal_Int32              mnStart;  // Start offset of that cell
    sal_Int32              mnLength; // How many of these do we have ?

    ScFormulaCellGroup();
    ~ScFormulaCellGroup();

    bool IsCompatible( ScSimilarFormulaDelta *pDelta );
};
inline void intrusive_ptr_add_ref(ScFormulaCellGroup *p)
{
    p->mnRefCount++;
}
inline void intrusive_ptr_release(ScFormulaCellGroup *p)
{
    if( --p->mnRefCount == 0 )
        delete p;
}

typedef ::boost::intrusive_ptr<ScFormulaCellGroup> ScFormulaCellGroupRef;

enum ScMatrixMode {
    MM_NONE      = 0,                   // No matrix formula
    MM_FORMULA   = 1,                   // Upper left matrix formula cell
    MM_REFERENCE = 2,                   // Remaining cells, via ocMatRef reference token
    MM_FAKE      = 3                    // Interpret "as-if" matrix formula (legacy)
};

class SC_DLLPUBLIC ScFormulaCell : public ScBaseCell, public SvtListener
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
    sal_uLong       nFormatIndex;       // Number format set by calculation
    short           nFormatType;        // Number format type set by calculation
    sal_uInt16      nSeenInIteration;   // Iteration cycle in which the cell was last encountered
    sal_uInt8       cMatrixFlag;        // One of ScMatrixMode
    bool            bDirty         : 1; // Must be (re)calculated
    bool            bChanged       : 1; // Whether something changed regarding display/representation
    bool            bRunning       : 1; // Already interpreting right now
    bool            bCompile       : 1; // Must be (re)compiled
    bool            bSubTotal      : 1; // Cell is part of or contains a SubTotal
    bool            bIsIterCell    : 1; // Cell is part of a circular reference
    bool            bInChangeTrack : 1; // Cell is in ChangeTrack
    bool            bTableOpDirty  : 1; // Dirty flag for TableOp
    bool            bNeedListening : 1; // Listeners need to be re-established after UpdateReference

                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
    void            InterpretTail( ScInterpretTailParameter );

    ScFormulaCell( const ScFormulaCell& );

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

    ScAddress       aPos;

                    ~ScFormulaCell();

    using ScBaseCell::Clone;

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
                    const rtl::OUString& rFormula,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    sal_uInt8 cMatInd = MM_NONE );

    ScFormulaCell( const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, int nCloneFlags = SC_CLONECELL_DEFAULT );

    size_t GetHash() const;

    ScFormulaVectorState GetVectorState() const;

    void            GetFormula( rtl::OUString& rFormula,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    void            GetFormula( rtl::OUStringBuffer& rBuffer,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    void            SetDirty( bool bDirtyFlag=true );
    void            SetDirtyVar();
    // If setting entire document dirty after load, no broadcasts but still append to FormulaTree.
    void            SetDirtyAfterLoad();
    inline void     ResetTableOpDirtyVar() { bTableOpDirty = false; }
    void            SetTableOpDirty();
    bool            IsDirtyOrInTableOpDirty() const;
    bool            GetDirty() const { return bDirty; }
    void            ResetDirty() { bDirty = false; }
    bool            NeedsListening() const { return bNeedListening; }
    void            SetNeedsListening( bool bVar ) { bNeedListening = bVar; }
    void            Compile(const rtl::OUString& rFormula,
                            bool bNoListening = false,
                            const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    void            CompileTokenArray( bool bNoListening = false );
    void            CompileXML( ScProgress& rProgress );        // compile temporary string tokens
    void            CalcAfterLoad();
    bool            MarkUsedExternalReferences();
    void            Interpret();
    inline bool     IsIterCell() const { return bIsIterCell; }
    inline sal_uInt16   GetSeenInIteration() const { return nSeenInIteration; }

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

    bool            UpdateReference(UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScDocument* pUndoDoc = NULL,
                                    const ScAddress* pUndoCellPos = NULL );

    void            TransposeReference();
    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                        ScDocument* pUndoDoc );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void            UpdateInsertTab(SCTAB nTable, SCTAB nNewSheets = 1);
    void            UpdateInsertTabAbs(SCTAB nTable);
    bool            UpdateDeleteTab(SCTAB nTable, bool bIsMove = false, SCTAB nSheets = 1);
    void            UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
    void            UpdateRenameTab(SCTAB nTable, const rtl::OUString& rName);
    bool            TestTabRefAbs(SCTAB nTable);
    void            UpdateCompile( bool bForceIfNameInUse = false );
    void            FindRangeNamesInUse(std::set<sal_uInt16>& rIndexes) const;
    bool            IsSubTotal() const                      { return bSubTotal; }
    bool            IsChanged() const;
    void            ResetChanged();
    bool            IsEmpty();      // formula::svEmptyCell result
                    // display as empty string if formula::svEmptyCell result
    bool            IsEmptyDisplayedAsString();
    bool            IsValue();      // also true if formula::svEmptyCell
    bool            IsHybridValueCell(); // for cells after import to deal with inherited number formats
    double          GetValue();
    double          GetValueAlways();   // ignore errors
    rtl::OUString   GetString();
    const ScMatrix* GetMatrix();
    bool            GetMatrixOrigin( ScAddress& rPos ) const;
    void            GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
    sal_uInt16      GetMatrixEdge( ScAddress& rOrgPos );
    sal_uInt16      GetErrCode();   // interpret first if necessary
    sal_uInt16      GetRawError();  // don't interpret, just return code or result error
    short           GetFormatType() const                   { return nFormatType; }
    sal_uLong       GetFormatIndex() const                  { return nFormatIndex; }
    void            GetFormatInfo( short& nType, sal_uLong& nIndex ) const
                        { nType = nFormatType; nIndex = nFormatIndex; }
    sal_uInt8       GetMatrixFlag() const                   { return cMatrixFlag; }
    ScTokenArray*   GetCode() const                         { return pCode; }

    bool            IsRunning() const                       { return bRunning; }
    void            SetRunning( bool bVal )                 { bRunning = bVal; }
    void            CompileDBFormula();
    void            CompileDBFormula( bool bCreateFormulaString );
    void            CompileNameFormula( bool bCreateFormulaString );
    void            CompileColRowNameFormula();
    ScFormulaCell*  GetPrevious() const                 { return pPrevious; }
    ScFormulaCell*  GetNext() const                     { return pNext; }
    void            SetPrevious( ScFormulaCell* pF )    { pPrevious = pF; }
    void            SetNext( ScFormulaCell* pF )        { pNext = pF; }
    ScFormulaCell*  GetPreviousTrack() const                { return pPreviousTrack; }
    ScFormulaCell*  GetNextTrack() const                    { return pNextTrack; }
    void            SetPreviousTrack( ScFormulaCell* pF )   { pPreviousTrack = pF; }
    void            SetNextTrack( ScFormulaCell* pF )       { pNextTrack = pF; }

    virtual void    Notify( SvtBroadcaster& rBC, const SfxHint& rHint);
    void            SetCompile( bool bVal ) { bCompile = bVal; }
    ScDocument*     GetDocument() const     { return pDocument; }
    void            SetMatColsRows( SCCOL nCols, SCROW nRows, bool bDirtyFlag=true );
    void            GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const;

                    // cell belongs to ChangeTrack and not to the real document
    void            SetInChangeTrack( bool bVal ) { bInChangeTrack = bVal; }
    bool            IsInChangeTrack() const { return bInChangeTrack; }

                    // standard format for type and format
                    // for format "Standard" possibly the format used in the formula cell
    sal_uLong       GetStandardFormat( SvNumberFormatter& rFormatter, sal_uLong nFormat ) const;

    // For import filters!
    void            AddRecalcMode( formula::ScRecalcMode );
    /** For import only: set a double result. */
    void            SetHybridDouble( double n )     { aResult.SetHybridDouble( n); }
    /** For import only: set a string result.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void            SetHybridString( const rtl::OUString& r )
                        { aResult.SetHybridString( r); }
    /** For import only: set a temporary formula string to be compiled later.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void            SetHybridFormula( const rtl::OUString& r,
                                    const formula::FormulaGrammar::Grammar eGrammar )
                        { aResult.SetHybridFormula( r); eTempGrammar = eGrammar; }

    /**
     * For import only: use for formula cells that return a number
     * formatted as some kind of string
     */
    void SetHybridValueString( double nVal, const OUString& r )
                        { aResult.SetHybridValueString( nVal, r ); }

    void SetResultMatrix( SCCOL nCols, SCROW nRows, const ScConstMatrixRef& pMat, formula::FormulaToken* pUL )
    {
        aResult.SetMatrix(nCols, nRows, pMat, pUL);
    }

    /** For import only: set a double result.
        Use this instead of SetHybridDouble() if there is no (temporary)
        formula string because the formula is present as a token array, as it
        is the case for binary Excel import.
     */
    void            SetResultDouble( double n )     { aResult.SetDouble( n); }

    void            SetErrCode( sal_uInt16 n );
    bool IsHyperLinkCell() const;
    EditTextObject* CreateURLObject();
    void GetURLResult( OUString& rURL, OUString& rCellText );

    /** Determines whether or not the result string contains more than one paragraph */
    bool            IsMultilineResult();

    void            MaybeInterpret();

    // Temporary formula cell grouping API
    ScFormulaCellGroupRef  GetCellGroup()
        { return xGroup; }
    void                   SetCellGroup( const ScFormulaCellGroupRef &xRef )
        { xGroup = xRef; }
    ScSimilarFormulaDelta *BuildDeltaTo( ScFormulaCell *pOther );
    void                   ReleaseDelta( ScSimilarFormulaDelta *pDelta );
    bool                   InterpretFormulaGroup();

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void StartListeningTo( ScDocument* pDoc );
    void EndListeningTo(
        ScDocument* pDoc, ScTokenArray* pArr = NULL, ScAddress aPos = ScAddress() );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
