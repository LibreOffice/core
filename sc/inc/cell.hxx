/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cell.hxx,v $
 * $Revision: 1.29.32.5 $
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
#include <tools/mempool.hxx>
#include <svtools/listener.hxx>
#include "global.hxx"
#include "formula/grammar.hxx"
#include "tokenarray.hxx"
#include "formularesult.hxx"
#include <rtl/ustrbuf.hxx>
#include <vcl/fontcvt.hxx>
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
class ScCodeArray;
class ScProgress;
class ScPostIt;

class ScMultipleReadHeader;
class ScMultipleWriteHeader;

class SC_DLLPUBLIC ScBaseCell
{
protected:
    ScPostIt*       pNote;
    SvtBroadcaster* pBroadcaster;
    USHORT          nTextWidth;
    BYTE            eCellType;      // enum CellType - BYTE spart Speicher
    BYTE            nScriptType;

public: // fuer Idle-Berechnung
    USHORT  GetTextWidth() const        { return nTextWidth; }
    void    SetTextWidth( USHORT nNew ) { nTextWidth = nNew; }

    BYTE    GetScriptType() const       { return nScriptType; }
    void    SetScriptType( BYTE nNew )  { nScriptType = nNew; }

protected:
                    ~ScBaseCell();  // nicht virtuell -> darf nicht direkt aufgerufen werden

public:
    explicit        ScBaseCell( CellType eNewType );
                    ScBaseCell( const ScBaseCell& rBaseCell, ScDocument* pDoc );

    ScBaseCell*     Clone(ScDocument* pDoc) const;
    void            Delete();                       // simulierter virtueller Destructor
    CellType        GetCellType() const;

    void            SetNote( const ScPostIt& rNote );
    BOOL            GetNote( ScPostIt& rNote ) const;
    inline const ScPostIt* GetNotePtr() const { return pNote; }
    void            DeleteNote();

    inline SvtBroadcaster*  GetBroadcaster() const;
    void            SetBroadcaster(SvtBroadcaster* pNew);
    inline void     ForgetBroadcaster();
    inline void     SwapBroadcaster(ScBaseCell& rOther);    // zum Sortieren

                        // String- oder EditCell
    static ScBaseCell* CreateTextCell( const String& rString, ScDocument* );

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void            StartListeningTo( ScDocument* pDoc );
    void            EndListeningTo( ScDocument* pDoc,
                                    ScTokenArray* pArr = NULL,
                                    ScAddress aPos = ScAddress() );

    /** Error code if ScFormulaCell, else 0. */
    USHORT          GetErrorCode() const;
    /** ScFormulaCell with formula::svEmptyCell result, or ScNoteCell (may have been
        created due to reference to empty cell). */
    BOOL            HasEmptyData() const;
    BOOL            HasValueData() const;
    BOOL            HasStringData() const;
    String          GetStringData() const;          // nur echte Strings

    static BOOL     CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 );
};



class SC_DLLPUBLIC ScValueCell : public ScBaseCell
{
private:
    double      aValue;

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScValueCell )
#endif
                    ~ScValueCell();

                    ScValueCell();
                    ScValueCell( const double& rValue );
                    ScValueCell( const ScValueCell& rScValueCell, ScDocument* pDoc );
    ScBaseCell*     Clone(ScDocument* pDoc) const;

    void            SetValue( const double& rValue );
    double          GetValue() const;
};


class SC_DLLPUBLIC ScStringCell : public ScBaseCell
{
private:
    String      aString;

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScStringCell )
#endif
#ifdef DBG_UTIL
                    ~ScStringCell();
#endif

                    ScStringCell();
                    ScStringCell( const String& rString );
                    ScStringCell( const ScStringCell& rScStringCell, ScDocument* pDoc );
    ScBaseCell*     Clone(ScDocument* pDoc) const;

    void            SetString( const String& rString );
    void            GetString( String& rString ) const;
};


class SC_DLLPUBLIC ScEditCell : public ScBaseCell
{
private:
    EditTextObject*     pData;
    String*             pString;        // fuer schnelleren Zugriff von Formeln
    ScDocument*         pDoc;           // fuer EditEngine Zugriff mit Pool

    void            SetTextObject( const EditTextObject* pObject,
                                    const SfxItemPool* pFromPool );

                    // not implemented
                    ScEditCell( const ScEditCell& );
    ScEditCell&     operator=( const ScEditCell& );

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScEditCell )
#endif

                    ~ScEditCell();              // wegen pData immer!

                    ScEditCell( const EditTextObject* pObject, ScDocument*,
                                const SfxItemPool* pFromPool /* = NULL */ );
                    ScEditCell( const ScEditCell& rEditCell, ScDocument* );
                    // fuer Zeilenumbrueche
                    ScEditCell( const String& rString, ScDocument* );
    ScBaseCell*     Clone( ScDocument* ) const;

    void            SetData( const EditTextObject* pObject,
                            const SfxItemPool* pFromPool /* = NULL */ );
    void            GetData( const EditTextObject*& rpObject ) const;
    void            GetString( String& rString ) const;

    const EditTextObject* GetData() const   { return pData; }
};

enum ScMatrixMode {
    MM_NONE      = 0,                   // No matrix formula
    MM_FORMULA   = 1,                   // Upper left matrix formula cell
    MM_REFERENCE = 2,                   // Remaining cells, via ocMatRef reference token
    MM_FAKE      = 3                    // Interpret "as-if" matrix formula (legacy)
};


class ScIndexMap;

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
    ULONG           nFormatIndex;       // Number format set by calculation
    short           nFormatType;        // Number format type set by calculation
    USHORT          nSeenInIteration;   // Iteration cycle in which the cell was last encountered
    BYTE            cMatrixFlag;        // One of ScMatrixMode
    BOOL            bDirty         : 1; // Must be (re)calculated
    BOOL            bChanged       : 1; // Whether something changed regarding display/representation
    BOOL            bRunning       : 1; // Already interpreting right now
    BOOL            bCompile       : 1; // Must be (re)compiled
    BOOL            bSubTotal      : 1; // Cell is part of or contains a SubTotal
    BOOL            bIsIterCell    : 1; // Cell is part of a circular reference
    BOOL            bInChangeTrack : 1; // Cell is in ChangeTrack
    BOOL            bTableOpDirty  : 1; // Dirty flag for TableOp
    BOOL            bNeedListening : 1; // Listeners need to be re-established after UpdateReference

                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
    void            InterpretTail( ScInterpretTailParameter );

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

    ScAddress       aPos;

                    ~ScFormulaCell();
                    ScFormulaCell();

    /** Empty formula cell, or with a preconstructed token array. */
    ScFormulaCell( ScDocument*, const ScAddress&, const ScTokenArray* = NULL,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    BYTE = MM_NONE );

    /** With formula string and grammar to compile with.
       formula::FormulaGrammar::GRAM_DEFAULT effectively isformula::FormulaGrammar::GRAM_NATIVE_UI that
        also includes formula::FormulaGrammar::CONV_UNSPECIFIED, therefor uses the address
        convention associated with rPos::nTab by default. */
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                    const String& rFormula,
                    const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT,
                    BYTE cMatInd = MM_NONE );

    // copy-ctor
    // nCopyFlags:  0 := nothing special
    //              0x0001 := readjust 3D references to point to old position even if relative
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                   const ScFormulaCell& rScFormulaCell, USHORT nCopyFlags = 0 );

    using ScBaseCell::Clone;
    ScBaseCell*     Clone(ScDocument* pDoc, const ScAddress&,
                            BOOL bNoListening = FALSE ) const;

    void            GetFormula( String& rFormula,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;
    void            GetFormula( rtl::OUStringBuffer& rBuffer,
                                const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT ) const;

    void            SetDirty();
    inline void     SetDirtyVar() { bDirty = TRUE; }
    // If setting entire document dirty after load, no broadcasts but still append to FormulaTree.
    void            SetDirtyAfterLoad();
    inline void     ResetTableOpDirtyVar() { bTableOpDirty = FALSE; }
    void            SetTableOpDirty();
    BOOL            IsDirtyOrInTableOpDirty() const;
    BOOL            GetDirty() const { return bDirty; }
    BOOL            NeedsListening() const { return bNeedListening; }
    void            SetNeedsListening( BOOL bVar ) { bNeedListening = bVar; }
    void            Compile(const String& rFormula,
                            BOOL bNoListening = FALSE,
                            const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    void            CompileTokenArray( BOOL bNoListening = FALSE );
    void            CompileXML( ScProgress& rProgress );        // compile temporary string tokens
    void            CalcAfterLoad();
    void            Interpret();
    inline BOOL     IsIterCell() const { return bIsIterCell; }
    inline USHORT   GetSeenInIteration() const { return nSeenInIteration; }

    BOOL            HasOneReference( ScRange& r ) const;
    BOOL            HasRelNameReference() const;
    BOOL            HasColRowName() const;

    void            UpdateReference(UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScDocument* pUndoDoc = NULL );

    void            TransposeReference();
    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                        ScDocument* pUndoDoc );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void            UpdateInsertTab(SCTAB nTable);
    void            UpdateInsertTabAbs(SCTAB nTable);
    BOOL            UpdateDeleteTab(SCTAB nTable, BOOL bIsMove = FALSE);
    void            UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo);
    void            UpdateRenameTab(SCTAB nTable, const String& rName);
    BOOL            TestTabRefAbs(SCTAB nTable);
    void            UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    BOOL            IsRangeNameInUse(USHORT nIndex) const;
    void            FindRangeNamesInUse(std::set<USHORT>& rIndexes) const;
    void            ReplaceRangeNamesInUse( const ScIndexMap& rMap );
    BOOL            IsSubTotal() const                      { return bSubTotal; }
    BOOL            IsChanged() const                       { return bChanged; }
    void            ResetChanged()                          { bChanged = FALSE; }
    BOOL            IsEmpty();      // formula::svEmptyCell result
                    // display as empty string if formula::svEmptyCell result
    BOOL            IsEmptyDisplayedAsString();
    BOOL            IsValue();      // also TRUE if formula::svEmptyCell
    double          GetValue();
    double          GetValueAlways();   // ignore errors
    void            GetString( String& rString );
    const ScMatrix* GetMatrix();
    BOOL            GetMatrixOrigin( ScAddress& rPos ) const;
    void            GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
    USHORT          GetMatrixEdge( ScAddress& rOrgPos );
    USHORT          GetErrCode();   // interpret first if necessary
    USHORT          GetRawError();  // don't interpret, just return code or result error
    short           GetFormatType() const                   { return nFormatType; }
    ULONG           GetFormatIndex() const                  { return nFormatIndex; }
    void            GetFormatInfo( short& nType, ULONG& nIndex ) const
                        { nType = nFormatType; nIndex = nFormatIndex; }
    BYTE            GetMatrixFlag() const                   { return cMatrixFlag; }
    ScTokenArray*   GetCode() const                         { return pCode; }

    BOOL            IsRunning() const                       { return bRunning; }
    void            SetRunning( BOOL bVal )                 { bRunning = bVal; }
    void            CompileDBFormula();
    void            CompileDBFormula( BOOL bCreateFormulaString );
    void            CompileNameFormula( BOOL bCreateFormulaString );
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
    void            SetCompile( BOOL bVal ) { bCompile = bVal; }
    ScDocument*     GetDocument() const     { return pDocument; }
    void            SetMatColsRows( SCCOL nCols, SCROW nRows );
    void            GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const;

                    // ob Zelle im ChangeTrack und nicht im echten Dokument ist
    void            SetInChangeTrack( BOOL bVal ) { bInChangeTrack = bVal; }
    BOOL            IsInChangeTrack() const { return bInChangeTrack; }

                    // Zu Typ und Format das entsprechende Standardformat.
                    // Bei Format "Standard" evtl. das in die Formelzelle
                    // uebernommene Format.
    ULONG           GetStandardFormat( SvNumberFormatter& rFormatter, ULONG nFormat ) const;

    // For import filters!
    void            AddRecalcMode( formula::ScRecalcMode );
    /** For import only: set a double result. */
    void            SetHybridDouble( double n )     { aResult.SetHybridDouble( n); }
    /** For import only: set a string result.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void            SetHybridString( const String& r )
                        { aResult.SetHybridString( r); }
    /** For import only: set a temporary formula string to be compiled later.
        If for whatever reason you have to use both, SetHybridDouble() and
        SetHybridString() or SetHybridFormula(), use SetHybridDouble() first
        for performance reasons.*/
    void            SetHybridFormula( const String& r,
                                    const formula::FormulaGrammar::Grammar eGrammar )
                        { aResult.SetHybridFormula( r); eTempGrammar = eGrammar; }
    void            SetErrCode( USHORT n );
    inline BOOL     IsHyperLinkCell() const { return pCode && pCode->IsHyperLink(); }
    EditTextObject*     CreateURLObject() ;
    void            GetURLResult( String& rURL, String& rCellText );
};

//          Iterator fuer Referenzen in einer Formelzelle
class ScDetectiveRefIter
{
private:
    ScTokenArray* pCode;
    ScAddress aPos;
public:
                ScDetectiveRefIter( ScFormulaCell* pCell );
    BOOL        GetNextRef( ScRange& rRange );
};

class ScNoteCell : public ScBaseCell
{
public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScNoteCell )
#endif
#ifdef DBG_UTIL
                    ~ScNoteCell();
#endif

                    ScNoteCell();
                    ScNoteCell( const ScPostIt& rNote );
                    ScNoteCell( const ScNoteCell& rScNoteCell, ScDocument* pDoc );
    ScBaseCell*     Clone(ScDocument* pDoc) const;
};


//      ScBaseCell

inline CellType ScBaseCell::GetCellType() const
{
    return (CellType)eCellType;
}

inline SvtBroadcaster* ScBaseCell::GetBroadcaster() const
{
    return pBroadcaster;
}

inline void ScBaseCell::ForgetBroadcaster()
{
    pBroadcaster = NULL;
}

inline void ScBaseCell::SwapBroadcaster(ScBaseCell& rOther)
{
    SvtBroadcaster* pTemp = pBroadcaster;
    pBroadcaster = rOther.pBroadcaster;
    rOther.pBroadcaster = pTemp;
}

//      ScValueCell

inline ScValueCell::ScValueCell() :
    ScBaseCell( CELLTYPE_VALUE )
{
    aValue = 0.0;
}

inline ScValueCell::ScValueCell( const double& rValue ) :
    ScBaseCell( CELLTYPE_VALUE )
{
    aValue = rValue;
}

inline ScValueCell::ScValueCell(const ScValueCell& rScValueCell, ScDocument* pDoc) :
    ScBaseCell( rScValueCell, pDoc ),
    aValue( rScValueCell.aValue )
{
}

inline ScBaseCell* ScValueCell::Clone(ScDocument* pDoc) const
{
    return new ScValueCell(*this, pDoc);
}

inline void ScValueCell::SetValue( const double& rValue )
{
    aValue = rValue;
}

inline double ScValueCell::GetValue() const
{
    return aValue;
}



//      ScStringCell

inline ScStringCell::ScStringCell() :
    ScBaseCell( CELLTYPE_STRING )
{
}

inline ScStringCell::ScStringCell( const ScStringCell& rScStringCell, ScDocument* pDoc ) :
    ScBaseCell( rScStringCell, pDoc ),
    aString( rScStringCell.aString )
{
}

inline ScStringCell::ScStringCell( const String& rString ) :
    ScBaseCell( CELLTYPE_STRING ),
    aString( rString.intern() )
{
}

inline ScBaseCell* ScStringCell::Clone(ScDocument* pDoc) const
{
    return new ScStringCell(*this, pDoc);
}

inline void ScStringCell::GetString( String& rString ) const
{
    rString = aString;
}

inline void ScStringCell::SetString( const String& rString )
{
    aString = rString;
}


/*

//      ScFormulaCell

inline ScBaseCell* ScFormulaCell::Clone(ScDocument* pDoc) const
{
    return new ScFormulaCell(pDoc, *this);
}
*/



//      ScNoteCell

inline ScNoteCell::ScNoteCell() :
    ScBaseCell( CELLTYPE_NOTE )
{
}

inline ScNoteCell::ScNoteCell( const ScNoteCell& rScNoteCell, ScDocument* pDoc ) :
    ScBaseCell( rScNoteCell, pDoc )
{
}

inline ScNoteCell::ScNoteCell( const ScPostIt& rNote ) :
    ScBaseCell( CELLTYPE_NOTE )
{
    ScBaseCell::SetNote(rNote);
}

inline ScBaseCell* ScNoteCell::Clone(ScDocument* pDoc) const
{
    return new ScNoteCell(*this, pDoc);
}


#endif

