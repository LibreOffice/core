/*************************************************************************
 *
 *  $RCSfile: cell.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: er $ $Date: 2001-02-13 18:51:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_CELL_HXX
#define SC_CELL_HXX

#include <stddef.h>

#ifndef _SVMEMPOOL_HXX //autogen
#include <tools/mempool.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif

#define USE_MEMPOOL
#define TEXTWIDTH_DIRTY     0xffff

// in addition to SCRIPTTYPE_... flags from scripttypeitem.hxx:
// set (in nScriptType) if type has not been determined yet
#define SC_SCRIPTTYPE_UNKNOWN   0x08

class ScDocument;
class EditTextObject;
class ScMatrix;
class ScCompiler;
class SfxBroadcaster;
class ScBroadcasterList;
class ScCodeArray;
class ScTokenArray;

class ScMultipleReadHeader;
class ScMultipleWriteHeader;

class ScBaseCell
{
protected:
    ScPostIt*       pNote;
    ScBroadcasterList*  pBroadcaster;
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
    inline          ScBaseCell( CellType eNewType );
    inline          ScBaseCell( const ScBaseCell& rBaseCell );

    ScBaseCell*     Clone(ScDocument* pDoc) const;
    void            Delete();                       // simulierter virtueller Destructor
    CellType        GetCellType() const;

    void            SetNote( const ScPostIt& rNote );
    BOOL            GetNote( ScPostIt& rNote ) const;
    const ScPostIt* GetNotePtr() const;
    inline void     DeleteNote();

    inline ScBroadcasterList*   GetBroadcaster() const;
    void            SetBroadcaster(ScBroadcasterList* pNew);
    inline void     ForgetBroadcaster();
    inline void     SwapBroadcaster(ScBaseCell& rOther);    // zum Sortieren

    void            LoadNote( SvStream& rStream );

                        // String- oder EditCell
    static ScBaseCell* CreateTextCell( const String& rString, ScDocument* );

    void            StartListeningTo( ScDocument* pDoc,
                                      BOOL bOnlyRelNames = FALSE );
    void            EndListeningTo( ScDocument* pDoc,
                                    BOOL bOnlyRelNames = FALSE,
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
                    ScValueCell( const ScValueCell& rScValueCell );
                    ScValueCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*     Clone() const;

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
                    ScStringCell( const ScStringCell& rScStringCell );
                    ScStringCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*     Clone() const;

    void            SetString( const String& rString );
    void            GetString( String& rString ) const;

    void            Save( SvStream& rStream ) const;
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

class ScFormulaCell : public ScBaseCell, public SfxListener
{
private:
    static INT8     nIterMode;              // Markiert cirk. Iteration
    String          aErgString;
    double          nErgValue;
    ScTokenArray*   pCode;                  // das neue Token-Array
    ScDocument*     pDocument;
    ScMatrix*       pMatrix;                // Pointer auf Ergebnis-Matrix
    ScFormulaCell*  pPrevious;
    ScFormulaCell*  pNext;
    ScFormulaCell*  pPreviousTrack;
    ScFormulaCell*  pNextTrack;
    ULONG           nFormatIndex;       // durch Berechnung gesetztes Format
    USHORT          nMatCols;           // wenn MM_FORMULA Matrixzelle
    USHORT          nMatRows;           // belegte Area
    short           nFormatType;
    BOOL            bIsValue    : 1;    // Ergebnis ist numerisches
    BOOL            bDirty      : 1;    // muss berechnet werden
    BOOL            bChanged    : 1;    // hat sich fuer die Darstellung was geaendert
    BOOL            bRunning    : 1;    // wird gerade interpretiert
    BOOL            bCompile    : 1;    // muss compiliert werden
    BOOL            bSubTotal   : 1;    // ist ein SubTotal
    BOOL            bIsIterCell : 1;    // kennzeichnet Zellen mit cirk. Refs.
    BOOL            bInChangeTrack: 1;  // Zelle ist im ChangeTrack
    BOOL            bTableOpDirty : 1;  // dirty for TableOp
    BYTE            cMatrixFlag;        // 1 = links oben, 2 = Restmatrix, 0 = keine

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
    ScFormulaCell( ScDocument* pDoc, const ScAddress&,
                   const String& rFormula, BYTE bMatInd = 0 );
    // copy-ctor
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                   const ScFormulaCell& rScFormulaCell );
    // lesender ctor
    ScFormulaCell( ScDocument* pDoc, const ScAddress&,
                   SvStream& rStream, ScMultipleReadHeader& rHdr );

    ScBaseCell*     Clone(ScDocument* pDoc, const ScAddress&,
                            BOOL bNoListening = FALSE ) const;

    void            GetFormula( String& rFormula ) const;
    void            GetEnglishFormula( String& rFormula, BOOL bCompileXML = FALSE ) const;

    void            Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

    void            SetDirty();
    inline void     SetDirtyVar() { bDirty = TRUE; }
    void            SetTableOpDirty();
    BOOL            IsDirtyOrInTableOpDirty();
    BOOL            GetDirty() const { return bDirty; }
    void            Compile(const String& rFormula, BOOL bNoListening = FALSE );
    void            CompileTokenArray( BOOL bNoListening = FALSE );
    void            CompileXML();       // compile temporary string tokens
    void            CalcAfterLoad();
    void            Interpret();

    BOOL            HasOneReference( ScRange& r ) const;
    BOOL            HasRelNameReference() const;
    BOOL            HasDBArea() const;
    BOOL            HasColRowName() const;

    void            UpdateReference(UpdateRefMode eUpdateRefMode,
                                    const ScRange& r,
                                    short nDx, short nDy, short nDz,
                                    ScDocument* pUndoDoc = NULL );

    void            TransposeReference();
    void            UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                        ScDocument* pUndoDoc );

    void            UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );

    void            UpdateInsertTab(USHORT nTable);
    void            UpdateInsertTabAbs(USHORT nTable);
    BOOL            UpdateDeleteTab(USHORT nTable, BOOL bIsMove = FALSE);
    void            UpdateMoveTab(USHORT nOldPos, USHORT nNewPos, USHORT nTabNo);
    void            UpdateRenameTab(USHORT nTable, const String& rName);
    BOOL            TestTabRefAbs(USHORT nTable);
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
    void            GetMatrix( ScMatrix** pMatrix );
    BOOL            GetMatrixOrigin( ScAddress& rPos ) const;
    void            GetResultDimensions( USHORT& rCols, USHORT& rRows );
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

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );
    void            SetCompile( BOOL bVal ) { bCompile = bVal; }
    ScDocument*     GetDocument() const     { return pDocument; }
    void            SetMatColsRows( USHORT nCols, USHORT nRows )
                                    { nMatCols = nCols; nMatRows = nRows; }
    void            GetMatColsRows( USHORT& nCols, USHORT& nRows ) const
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
    void            SetString( const String& r )            { aErgString = r; bIsValue = FALSE; }
    void            SetErrCode( USHORT n );
};

//          Iterator fuer Referenzen in einer Formelzelle
class ScDetectiveRefIter
{
private:
    ScTokenArray* pCode;
    ScAddress aPos;
public:
                ScDetectiveRefIter( ScFormulaCell* pCell );
    BOOL        GetNextRef( ScTripel& rStart, ScTripel& rEnd );
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
                    ScNoteCell( const ScNoteCell& rScNoteCell );
                    ScNoteCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*     Clone() const;

    void            Save( SvStream& rStream ) const;
};


//      ScBaseCell

inline ScBaseCell::ScBaseCell( CellType eNewType ) :
    eCellType( eNewType ),
    pNote( NULL ),
    pBroadcaster( NULL ), nTextWidth( TEXTWIDTH_DIRTY ), nScriptType( SC_SCRIPTTYPE_UNKNOWN )
{
}

inline ScBaseCell::ScBaseCell( const ScBaseCell& rBaseCell ) :
    eCellType( rBaseCell.eCellType ),
    pBroadcaster( NULL ), nTextWidth( rBaseCell.nTextWidth ), nScriptType( SC_SCRIPTTYPE_UNKNOWN )
{
    if (rBaseCell.pNote)
        pNote = new ScPostIt( *rBaseCell.pNote );
    else
        pNote = NULL;
}


inline CellType ScBaseCell::GetCellType() const
{
    return (CellType)eCellType;
}

inline const ScPostIt* ScBaseCell::GetNotePtr() const
{
    return pNote;
}

inline void ScBaseCell::DeleteNote()
{
    delete pNote;
    pNote = NULL;
}

inline ScBroadcasterList* ScBaseCell::GetBroadcaster() const
{
    return pBroadcaster;
}

inline void ScBaseCell::ForgetBroadcaster()
{
    pBroadcaster = NULL;
}

inline void ScBaseCell::SwapBroadcaster(ScBaseCell& rOther)
{
    ScBroadcasterList*  pTemp = pBroadcaster;
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

inline ScValueCell::ScValueCell(const ScValueCell& rScValueCell) :
    ScBaseCell( rScValueCell ),
    aValue( rScValueCell.aValue )
{
}

inline ScBaseCell* ScValueCell::Clone() const
{
    return new ScValueCell(*this);
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

inline ScStringCell::ScStringCell( const ScStringCell& rScStringCell ) :
    ScBaseCell( rScStringCell ),
    aString( rScStringCell.aString )
{
}

inline ScStringCell::ScStringCell( const String& rString ) :
    ScBaseCell( CELLTYPE_STRING ),
    aString( rString )
{
}

inline ScBaseCell* ScStringCell::Clone() const
{
    return new ScStringCell(*this);
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

inline ScNoteCell::ScNoteCell( const ScNoteCell& rScNoteCell ) :
    ScBaseCell( rScNoteCell )
{
}

inline ScNoteCell::ScNoteCell( const ScPostIt& rNote ) :
    ScBaseCell( CELLTYPE_NOTE )
{
    ScBaseCell::SetNote(rNote);
}

inline ScBaseCell* ScNoteCell::Clone() const
{
    return new ScNoteCell(*this);
}


#endif

