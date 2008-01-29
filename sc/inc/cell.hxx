/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cell.hxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:14:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_CELL_HXX
#define SC_CELL_HXX

#include <stddef.h>

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _SVT_LISTENER_HXX
#include <svtools/listener.hxx>
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_TOKENARRAY_HXX
#include "tokenarray.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SV_FONTCVT_HXX
#include <vcl/fontcvt.hxx>
#endif

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
class ScTokenArray;
class ScProgress;
class ScPostIt;

class ScMultipleReadHeader;
class ScMultipleWriteHeader;

class ScBaseCell
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

    void            LoadNote( SvStream& rStream, ScDocument* pDoc );

                        // String- oder EditCell
    static ScBaseCell* CreateTextCell( const String& rString, ScDocument* );

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void            StartListeningTo( ScDocument* pDoc );
    void            EndListeningTo( ScDocument* pDoc,
                                    ScTokenArray* pArr = NULL,
                                    ScAddress aPos = ScAddress() );

    BOOL            HasValueData() const;
    BOOL            HasStringData() const;
    String          GetStringData() const;          // nur echte Strings

    static BOOL     CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 );
};



class ScValueCell : public ScBaseCell
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
                    ScValueCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*     Clone(ScDocument* pDoc) const;

    void            SetValue( const double& rValue );
    double          GetValue() const;

    void            Save( SvStream& rStream ) const;
};


class ScStringCell : public ScBaseCell
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
                    ScStringCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*     Clone(ScDocument* pDoc) const;

    void            SetString( const String& rString );
    void            GetString( String& rString ) const;

    void            Save( SvStream& rStream, FontToSubsFontConverter hConv = 0 ) const;
                    // convert symbol font after loading binary format
    void            ConvertFont( FontToSubsFontConverter hConv );
};


class ScEditCell : public ScBaseCell
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

                    ScEditCell( ScDocument* );
                    ScEditCell( const EditTextObject* pObject, ScDocument*,
                                const SfxItemPool* pFromPool /* = NULL */ );
                    ScEditCell( const ScEditCell& rEditCell, ScDocument* );
                    ScEditCell( SvStream& rStream, USHORT nVer, ScDocument* );
                    // fuer Zeilenumbrueche
                    ScEditCell( const String& rString, ScDocument* );
    ScBaseCell*     Clone( ScDocument* ) const;

    void            SetData( const EditTextObject* pObject,
                            const SfxItemPool* pFromPool /* = NULL */ );
    void            GetData( const EditTextObject*& rpObject ) const;
    void            GetString( String& rString ) const;

    const EditTextObject* GetData() const   { return pData; }

    void            Save( SvStream& rStream ) const;
};

enum ScMatrixMode {
    MM_NONE      = 0,                   // keine Matrixformel
    MM_FORMULA   = 1,                   // Matrixformel
    MM_REFERENCE = 2,                   // Referenz auf Matrixformel (MATVALUE)
    MM_FAKE      = 3                    // Formel als Matrixformel
};


class ScIndexMap;

class ScFormulaCell : public ScBaseCell, public SvtListener
{
private:
    String          aErgString;
    double          nErgValue;
    ScAddress::Convention nErgConv;
    ScTokenArray*   pCode;                  // das neue Token-Array
    ScDocument*     pDocument;
    ScMatrixRef     xMatrix;                // Pointer to refcounted result matrix
    ScFormulaCell*  pPrevious;
    ScFormulaCell*  pNext;
    ScFormulaCell*  pPreviousTrack;
    ScFormulaCell*  pNextTrack;
    ULONG           nFormatIndex;       // durch Berechnung gesetztes Format
    SCCOL           nMatCols;           // wenn MM_FORMULA Matrixzelle
    SCROW           nMatRows;           // belegte Area
    USHORT          nSeenInIteration;   // Iteration cycle in which the cell was last encountered
    short           nFormatType;
    BOOL            bIsValue    : 1;    // Result is numerical, not textual
    BOOL            bDirty      : 1;    // Must be (re)calculated
    BOOL            bChanged    : 1;    // Whether something changed regarding display/representation
    BOOL            bRunning    : 1;    // Already interpreting right now
    BOOL            bCompile    : 1;    // Must be (re)compiled
    BOOL            bSubTotal   : 1;    // Cell is part of or contains a SubTotal
    BOOL            bIsIterCell : 1;    // Cell is part of a circular reference
    BOOL            bInChangeTrack: 1;  // Cell is in ChangeTrack
    BOOL            bTableOpDirty : 1;  // Dirty flag for TableOp
    BOOL            bNeedListening : 1; // Listeners need to be re-established after UpdateReference
    BYTE            cMatrixFlag;        // 1 = links oben, 2 = Restmatrix, 0 = keine

                    enum ScInterpretTailParameter
                    {
                        SCITP_NORMAL,
                        SCITP_FROM_ITERATION,
                        SCITP_CLOSE_ITERATION_CIRCLE
                    };
    void            InterpretTail( ScInterpretTailParameter );

    /** Get English formula, if bCompileXML==TRUE in Plain Old Formula format,
        which means missing parameters are substituded. */
    void            GetEnglishFormulaForPof( rtl::OUStringBuffer &rBuffer,
                                             const ScAddress &rPos,
                                             BOOL bCompileXML ) const;

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

    ScAddress       aPos;

                    ~ScFormulaCell();
                    ScFormulaCell();

    // leere Zelle, ggf. mit fertigem TokenArray
    ScFormulaCell( ScDocument*, const ScAddress&, const ScTokenArray* = NULL, BYTE=0 );
    // mit Formel
    ScFormulaCell( ScDocument* pDoc, const ScAddress& aPos,
                   const String& rFormula,
                   // Use the conv associated with aPos::nTab by default
                   ScAddress::Convention eConvP = ScAddress::CONV_UNSPECIFIED,
                   BYTE bMatInd = 0 );
    // copy-ctor
    // nCopyFlags:  0 := nothing special
    //              0x0001 := readjust 3D references to point to old position even if relative
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                   const ScFormulaCell& rScFormulaCell, USHORT nCopyFlags = 0 );
    // lesender ctor
    ScFormulaCell( ScDocument* pDoc, const ScAddress&,
                   SvStream& rStream, ScMultipleReadHeader& rHdr );

    using ScBaseCell::Clone;
    ScBaseCell*     Clone(ScDocument* pDoc, const ScAddress&,
                            BOOL bNoListening = FALSE ) const;

    void            GetFormula( String& rFormula,
                                ScAddress::Convention eConv = ScAddress::CONV_OOO) const;
    void            GetEnglishFormula( String& rFormula, BOOL bCompileXML = FALSE,
                                       ScAddress::Convention eConv = ScAddress::CONV_OOO) const;
    void            GetEnglishFormula( rtl::OUStringBuffer& rBuffer, BOOL bCompileXML = FALSE,
                                       ScAddress::Convention eConv = ScAddress::CONV_OOO) const;

    void            Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

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
                            ScAddress::Convention eConv = ScAddress::CONV_OOO);
    void            CompileTokenArray( BOOL bNoListening = FALSE );
    void            CompileXML( ScProgress& rProgress );        // compile temporary string tokens
    void            CalcAfterLoad();
    void            Interpret();
    inline BOOL     IsIterCell() const { return bIsIterCell; }
    inline USHORT   GetSeenInIteration() const { return nSeenInIteration; }

    BOOL            HasOneReference( ScRange& r ) const;
    BOOL            HasRelNameReference() const;
    BOOL            HasDBArea() const;
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
    void            ReplaceRangeNamesInUse( const ScIndexMap& rMap );
    BOOL            IsSubTotal() const                      { return bSubTotal; }
    BOOL            IsChanged() const                       { return bChanged; }
    void            ResetChanged()                          { bChanged = FALSE; }
    BOOL            IsValue();
    double          GetValue();
    double          GetValueAlways();   // ignore errors
    void            GetString( String& rString );
    const ScMatrix* GetMatrix();
    BOOL            GetMatrixOrigin( ScAddress& rPos ) const;
    void            GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows );
    USHORT          GetMatrixEdge( ScAddress& rOrgPos );
    USHORT          GetErrCode();
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
    void            SetMatColsRows( SCCOL nCols, SCROW nRows )
                                    { nMatCols = nCols; nMatRows = nRows; }
    void            GetMatColsRows( SCCOL& nCols, SCROW& nRows ) const
                                    { nCols = nMatCols; nRows = nMatRows; }

                    // ob Zelle im ChangeTrack und nicht im echten Dokument ist
    void            SetInChangeTrack( BOOL bVal ) { bInChangeTrack = bVal; }
    BOOL            IsInChangeTrack() const { return bInChangeTrack; }

                    // Zu Typ und Format das entsprechende Standardformat.
                    // Bei Format "Standard" evtl. das in die Formelzelle
                    // uebernommene Format.
    ULONG           GetStandardFormat( SvNumberFormatter& rFormatter, ULONG nFormat ) const;

    // fuer die Importfilter!
    void            AddRecalcMode( ScRecalcMode );
    void            SetDouble( double n )                   { nErgValue = n; bIsValue = TRUE; }
    void            SetString( const String& r,
                               ScAddress::Convention eConv = ScAddress::CONV_OOO)
                        { aErgString = r; nErgConv = eConv; bIsValue = FALSE; }
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
                    ScNoteCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*     Clone(ScDocument* pDoc) const;

    void            Save( SvStream& rStream ) const;
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

