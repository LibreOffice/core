/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_CELL_HXX
#define SC_CELL_HXX

#include <stddef.h>

#include <set>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <tools/mempool.hxx>
#include <svl/listener.hxx>
#include "global.hxx"
#include "rangenam.hxx"
#include "formula/grammar.hxx"
#include "tokenarray.hxx"
#include "formularesult.hxx"
#include <rtl/ustrbuf.hxx>
#include <unotools/fontcvt.hxx>
#include "scdllapi.h"

#define USE_MEMPOOL
#define TEXTWIDTH_DIRTY     0xffff

// in addition to SCRIPTTYPE_... flags from scripttypeitem.hxx:
// set (in nScriptType) if type has not been determined yet
#define SC_SCRIPTTYPE_UNKNOWN   0x08

class ScDocument;
class EditTextObject;
class ScMatrix;
class SvtBroadcaster;
class ScProgress;
class ScPatternAttr;

// ============================================================================

/** Default cell clone flags: do not start listening, do not adjust 3D refs to
    old position, clone note captions of cell notes. */
const int SC_CLONECELL_DEFAULT          = 0x0000;

/** If set, cloned formula cells will start to listen to the document. */
const int SC_CLONECELL_STARTLISTENING   = 0x0001;

/** If set, relative 3D references of cloned formula cells will be adjusted to
    old position (used while swapping cells for sorting a cell range). */
const int SC_CLONECELL_ADJUST3DREL      = 0x0002;

/** If set, the caption object of a cell note will not be cloned (used while
    copying cells to undo document, where captions are handled in drawing undo). */
const int SC_CLONECELL_NOCAPTION        = 0x0004;

/** If set, absolute refs will not transformed to external references */
const int SC_CLONECELL_NOMAKEABS_EXTERNAL = 0x0008;
// ============================================================================

class SC_DLLPUBLIC ScBaseCell
{
protected:
                    ~ScBaseCell();  // not virtual - not to be called directly.

public:
    explicit        ScBaseCell( CellType eNewType );

    /** Base copy constructor. Does NOT clone cell note or broadcaster! */
                    ScBaseCell( const ScBaseCell& rCell );

    /** Returns a clone of this cell at the same position,
        broadcaster will not be cloned. */
    ScBaseCell*     Clone( ScDocument& rDestDoc, int nCloneFlags = SC_CLONECELL_DEFAULT ) const;

    /** Returns a clone of this cell for the passed document position,
        broadcaster will not be cloned. */
    ScBaseCell*     Clone( ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags = SC_CLONECELL_DEFAULT ) const;

    /** Due to the fact that ScBaseCell does not have a vtable, this function
        deletes the cell by calling the appropriate d'tor of the derived class. */
    void            Delete();

    inline CellType GetCellType() const { return (CellType)eCellType; }

    /** Returns true, if the cell is empty (neither value nor formula nor cell note).
        Returns false for formula cells returning nothing, use HasEmptyData() for that. */
    bool            IsBlank() const;

// for idle-calculations
    inline sal_uInt16   GetTextWidth() const { return nTextWidth; }
    inline void     SetTextWidth( sal_uInt16 nNew ) { nTextWidth = nNew; }

    inline sal_uInt8     GetScriptType() const { return nScriptType; }
    inline void     SetScriptType( sal_uInt8 nNew ) { nScriptType = nNew; }

    /** Returns true, if the cell contains a broadcaster. */
    inline bool     HasBroadcaster() const { return mpBroadcaster != 0; }
    /** Returns the pointer to the cell broadcaster. */
    inline SvtBroadcaster* GetBroadcaster() const { return mpBroadcaster; }
    /** Takes ownership of the passed cell broadcaster. */
    void            TakeBroadcaster( SvtBroadcaster* pBroadcaster );
    /** Returns and forgets the own cell broadcaster. Caller takes ownership! */
    SvtBroadcaster* ReleaseBroadcaster();
    /** Deletes the own cell broadcaster. */
    void            DeleteBroadcaster();

                        // String- oder EditCell
    static ScBaseCell* CreateTextCell( const rtl::OUString& rString, ScDocument* );

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void            StartListeningTo( ScDocument* pDoc );
    void            EndListeningTo( ScDocument* pDoc,
                                    ScTokenArray* pArr = NULL,
                                    ScAddress aPos = ScAddress() );

    /** Error code if ScFormulaCell, else 0. */
    sal_uInt16          GetErrorCode() const;
    /** ScFormulaCell with formula::svEmptyCell result, or ScNoteCell (may have been
        created due to reference to empty cell). */
    bool            HasEmptyData() const;
    bool            HasValueData() const;
    bool            HasStringData() const;
    rtl::OUString   GetStringData() const;          // only real strings

    static bool     CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 );

private:
    ScBaseCell&     operator=( const ScBaseCell& );

private:
    SvtBroadcaster* mpBroadcaster;  /// Broadcaster for changed values. Cell takes ownership!

protected:
    sal_uInt16          nTextWidth;
    sal_uInt8            eCellType;      // enum CellType - sal_uInt8 spart Speicher
    sal_uInt8            nScriptType;
};

// ============================================================================

class SC_DLLPUBLIC ScNoteCell : public ScBaseCell
{
public:
#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScNoteCell )
#endif

    /** Cell takes ownership of the passed broadcaster. */
    explicit        ScNoteCell( SvtBroadcaster* pBC = 0 );

#if OSL_DEBUG_LEVEL > 0
                    ~ScNoteCell();
#endif

private:
                    ScNoteCell( const ScNoteCell& );
};

class SC_DLLPUBLIC ScValueCell : public ScBaseCell
{
public:
#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScValueCell )
#endif

    explicit        ScValueCell( double fValue );

#if OSL_DEBUG_LEVEL > 0
                    ~ScValueCell();
#endif

    inline void     SetValue( double fValue ) { mfValue = fValue; }
    inline double   GetValue() const { return mfValue; }

private:
    double          mfValue;
};

class SC_DLLPUBLIC ScStringCell : public ScBaseCell
{
public:
#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScStringCell )
#endif

    explicit        ScStringCell(const rtl::OUString& rString);

#if OSL_DEBUG_LEVEL > 0
                    ~ScStringCell();
#endif

    inline void     SetString( const rtl::OUString& rString ) { maString = rString; }
    inline const    rtl::OUString& GetString() const { return maString; }

private:
    rtl::OUString   maString;
};

class SC_DLLPUBLIC ScEditCell : public ScBaseCell
{
private:
    EditTextObject*     pData;
    mutable rtl::OUString* pString;        // for faster access to formulas
    ScDocument*         pDoc;           // for EditEngine access with Pool

    void            SetTextObject( const EditTextObject* pObject,
                                    const SfxItemPool* pFromPool );

                    // not implemented
                    ScEditCell( const ScEditCell& );

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScEditCell )
#endif

                    ~ScEditCell();              // always because of pData!

                    ScEditCell( const EditTextObject* pObject, ScDocument*,
                                const SfxItemPool* pFromPool /* = NULL */ );
    ScEditCell(const ScEditCell& rCell, ScDocument& rDoc, const ScAddress& rDestPos);
                    // for line breaks
                    ScEditCell( const rtl::OUString& rString, ScDocument* );

    void            SetData( const EditTextObject* pObject,
                            const SfxItemPool* pFromPool /* = NULL */ );
    void            GetData( const EditTextObject*& rpObject ) const;
    rtl::OUString   GetString() const;

    const EditTextObject* GetData() const   { return pData; }

    /** Removes character attribute based on new pattern attributes. */
    void            RemoveCharAttribs( const ScPatternAttr& rAttr );

    /** Update field items if any. */
    void UpdateFields(SCTAB nTab);
};

class ScEditDataArray
{
public:
    class Item
    {
    public:
        explicit Item(SCTAB nTab, SCCOL nCol, SCROW nRow,
                      EditTextObject* pOldData, EditTextObject* pNewData);
        ~Item();

        const EditTextObject* GetOldData() const;
        const EditTextObject* GetNewData() const;
        SCTAB GetTab() const;
        SCCOL GetCol() const;
        SCROW GetRow() const;

    private:
        Item(); // disabled

    private:
        ::boost::shared_ptr<EditTextObject> mpOldData;
        ::boost::shared_ptr<EditTextObject> mpNewData;
        SCTAB mnTab;
        SCCOL mnCol;
        SCROW mnRow;

    };

    ScEditDataArray();
    ~ScEditDataArray();

    void AddItem(SCTAB nTab, SCCOL nCol, SCROW nRow,
                 EditTextObject* pOldData, EditTextObject* pNewData);

    const Item* First();
    const Item* Next();

private:
    ::std::vector<Item>::const_iterator maIter;
    ::std::vector<Item> maArray;
};

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

    /** For import only: set a double result.
        Use this instead of SetHybridDouble() if there is no (temporary)
        formula string because the formula is present as a token array, as it
        is the case for binary Excel import.
     */
    void            SetResultDouble( double n )     { aResult.SetDouble( n); }

    void            SetErrCode( sal_uInt16 n );
    inline bool     IsHyperLinkCell() const { return pCode && pCode->IsHyperLink(); }
    EditTextObject* CreateURLObject() ;
    void            GetURLResult( rtl::OUString& rURL, rtl::OUString& rCellText );

    /** Determines whether or not the result string contains more than one paragraph */
    bool            IsMultilineResult();

    void            MaybeInterpret();
};

//          Iterator for references in a formula cell
class ScDetectiveRefIter
{
private:
    ScTokenArray* pCode;
    ScAddress aPos;
public:
                ScDetectiveRefIter( ScFormulaCell* pCell );
    bool        GetNextRef( ScRange& rRange );
    ScToken*    GetNextRefToken();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
