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

#include <bf_svtools/lstner.hxx>
#include "compiler.hxx"


#include <unotools/fontcvt.hxx>
namespace binfilter {

class SfxBroadcaster;

#define USE_MEMPOOL
#define TEXTWIDTH_DIRTY		0xffff

// in addition to SCRIPTTYPE_... flags from scripttypeitem.hxx:
// set (in nScriptType) if type has not been determined yet
#define SC_SCRIPTTYPE_UNKNOWN	0x08

// For StartListeningTo()/EndListeningTo() restricted to absolute/relative names
#define SC_LISTENING_NAMES_REL  0x01
#define SC_LISTENING_NAMES_ABS  0x02
#define SC_LISTENING_EXCEPT     0x04

class ScDocument;
class EditTextObject;
class ScMatrix;
class ScCompiler;
class ScBroadcasterList;
class ScCodeArray;
class ScTokenArray;
class ScProgress;

class ScMultipleReadHeader;
class ScMultipleWriteHeader;

class ScBaseCell
{
protected:
    ScPostIt*		pNote;
    ScBroadcasterList*	pBroadcaster;
    USHORT			nTextWidth;
    BYTE			eCellType;		// enum CellType - BYTE spart Speicher
    BYTE			nScriptType;

public: // fuer Idle-Berechnung
    USHORT	GetTextWidth() const		{ return nTextWidth; }
    void	SetTextWidth( USHORT nNew ) { nTextWidth = nNew; }

    BYTE	GetScriptType() const		{ return nScriptType; }
    void	SetScriptType( BYTE nNew )	{ nScriptType = nNew; }

protected:
                    ~ScBaseCell();	// nicht virtuell -> darf nicht direkt aufgerufen werden

public:
    inline			ScBaseCell( CellType eNewType );
    inline			ScBaseCell( const ScBaseCell& rBaseCell );

    ScBaseCell*		Clone(ScDocument* pDoc) const;
    void			Delete();						// simulierter virtueller Destructor
    CellType		GetCellType() const;

    void			SetNote( const ScPostIt& rNote );
    BOOL			GetNote( ScPostIt& rNote ) const;
    const ScPostIt*	GetNotePtr() const;
    inline void		DeleteNote();

    inline ScBroadcasterList*	GetBroadcaster() const;
    void			SetBroadcaster(ScBroadcasterList* pNew);
    inline void		ForgetBroadcaster();
    inline void		SwapBroadcaster(ScBaseCell& rOther);	// zum Sortieren

    void			LoadNote( SvStream& rStream );

                        // String- oder EditCell
    static ScBaseCell* CreateTextCell( const String& rString, ScDocument* );

    // nOnlyNames may be one or more of SC_LISTENING_NAMES_*
    void			StartListeningTo( ScDocument* pDoc,
                                      USHORT nOnlyNames = 0 );
    void			EndListeningTo( ScDocument* pDoc,
                                    USHORT nOnlyNames = 0,
                                    ScTokenArray* pArr = NULL,
                                    ScAddress aPos = ScAddress() );

    BOOL			HasValueData() const;
    BOOL			HasStringData() const;
    String			GetStringData() const;			// nur echte Strings

    static BOOL		CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 );
};



class ScValueCell : public ScBaseCell
{
private:
    double		aValue;

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScValueCell )
#endif
                    ~ScValueCell();

                    ScValueCell();
                    ScValueCell( const double& rValue );
                    ScValueCell( const ScValueCell& rScValueCell );
                    ScValueCell( SvStream& rStream, USHORT nVer );
    ScBaseCell*		Clone() const;

    void			SetValue( const double& rValue );
    double			GetValue() const;

    void			Save( SvStream& rStream ) const;
};


class ScStringCell : public ScBaseCell
{
private:
    String		aString;

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
    ScBaseCell*		Clone() const;

    void			SetString( const String& rString );
    void			GetString( String& rString ) const;

    void            Save( SvStream& rStream, FontToSubsFontConverter hConv = 0 ) const;
                    // convert symbol font after loading binary format
    void            ConvertFont( FontToSubsFontConverter hConv );
};


class ScEditCell : public ScBaseCell
{
private:
    EditTextObject*		pData;
    String*				pString;		// fuer schnelleren Zugriff von Formeln
    ScDocument*			pDoc;			// fuer EditEngine Zugriff mit Pool

    void			SetTextObject( const EditTextObject* pObject,
                                    const SfxItemPool* pFromPool );

                    // not implemented
                    ScEditCell( const ScEditCell& );
    ScEditCell&		operator=( const ScEditCell& );

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScEditCell )
#endif

                    ~ScEditCell();				// wegen pData immer!

                    ScEditCell( const EditTextObject* pObject, ScDocument*,
                                const SfxItemPool* pFromPool /* = NULL */ );
                    ScEditCell( const ScEditCell& rEditCell, ScDocument* );
                    ScEditCell( SvStream& rStream, USHORT nVer, ScDocument* );
                    // fuer Zeilenumbrueche
                    ScEditCell( const String& rString, ScDocument* );

    void			GetData( const EditTextObject*& rpObject ) const;
    void			GetString( String& rString ) const;

    const EditTextObject* GetData() const	{ return pData; }

    void			Save( SvStream& rStream ) const;
};

enum ScMatrixMode {
    MM_NONE		 = 0,					// keine Matrixformel
    MM_FORMULA	 = 1,					// Matrixformel
    MM_REFERENCE = 2,					// Referenz auf Matrixformel (MATVALUE)
    MM_FAKE		 = 3					// Formel als Matrixformel
};


class ScIndexMap;

class ScFormulaCell : public ScBaseCell, public SfxListener
{
private:
    static INT8		nIterMode;				// Markiert cirk. Iteration
    String			aErgString;
    double			nErgValue;
    ScTokenArray*	pCode; 					// das neue Token-Array
    ScDocument*		pDocument;
    ScMatrix*		pMatrix;				// Pointer auf Ergebnis-Matrix
    ScFormulaCell*	pPrevious;
    ScFormulaCell*	pNext;
    ScFormulaCell*	pPreviousTrack;
    ScFormulaCell*	pNextTrack;
    ULONG			nFormatIndex;		// durch Berechnung gesetztes Format
    USHORT			nMatCols;			// wenn MM_FORMULA Matrixzelle
    USHORT			nMatRows;			// belegte Area
    short			nFormatType;
    BOOL			bIsValue	: 1;	// Ergebnis ist numerisches
    BOOL			bDirty		: 1;	// muss berechnet werden
    BOOL			bChanged	: 1;	// hat sich fuer die Darstellung was geaendert
    BOOL			bRunning	: 1;	// wird gerade interpretiert
    BOOL			bCompile	: 1;	// muss compiliert werden
    BOOL			bSubTotal	: 1;	// ist ein SubTotal
    BOOL			bIsIterCell	: 1;	// kennzeichnet Zellen mit cirk. Refs.
    BOOL			bInChangeTrack: 1;	// Zelle ist im ChangeTrack
    BOOL			bTableOpDirty : 1;	// dirty for TableOp
    BYTE			cMatrixFlag;		// 1 = links oben, 2 = Restmatrix, 0 = keine

public:

#ifdef USE_MEMPOOL
    DECL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell )
#endif

    ScAddress		aPos;

                    ~ScFormulaCell();

    // leere Zelle, ggf. mit fertigem TokenArray
    ScFormulaCell( ScDocument*, const ScAddress&, const ScTokenArray* = NULL, BYTE=0 );
    // mit Formel
    ScFormulaCell( ScDocument* pDoc, const ScAddress&,
                   const String& rFormula, BYTE bMatInd = 0 );
    // copy-ctor
    // nCopyFlags:  0 := nothing special
    //              0x0001 := readjust 3D references to point to old position even if relative
    ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                   const ScFormulaCell& rScFormulaCell, USHORT nCopyFlags = 0 );
    // lesender ctor
    ScFormulaCell( ScDocument* pDoc, const ScAddress&,
                   SvStream& rStream, ScMultipleReadHeader& rHdr );

    ScBaseCell*		Clone(ScDocument* pDoc, const ScAddress&,
                             BOOL bNoListening = FALSE ) const;

    void			GetFormula( String& rFormula ) const;
    void			GetEnglishFormula( String& rFormula, BOOL bCompileXML = FALSE ) const;
    void			GetEnglishFormula( ::rtl::OUStringBuffer& rBuffer, BOOL bCompileXML = FALSE ) const;

    void			Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const;

    void			SetDirty();
    inline void		SetDirtyVar() { bDirty = TRUE; }
    inline void		ResetTableOpDirtyVar() { bTableOpDirty = FALSE; }
     void			SetTableOpDirty();
    BOOL			IsDirtyOrInTableOpDirty();
    BOOL			GetDirty() const { return bDirty; }
    void			Compile(const String& rFormula, BOOL bNoListening = FALSE );
    void			CompileTokenArray( BOOL bNoListening = FALSE );
    void			CompileXML( ScProgress& rProgress );		// compile temporary string tokens
    void			CalcAfterLoad();
    void			Interpret();

     BOOL			HasRelNameReference() const;

     void			UpdateReference(UpdateRefMode eUpdateRefMode,
                                     const ScRange& r,
                                     short nDx, short nDy, short nDz,
                                     ScDocument* pUndoDoc = NULL );

    void			UpdateInsertTab(USHORT nTable);
     BOOL			UpdateDeleteTab(USHORT nTable, BOOL bIsMove = FALSE);
    void			UpdateRenameTab(USHORT nTable, const String& rName);
    BOOL 			TestTabRefAbs(USHORT nTable);
    void			UpdateCompile( BOOL bForceIfNameInUse = FALSE );
    BOOL			IsRangeNameInUse(USHORT nIndex) const;
    BOOL			IsSubTotal() const 						{ return bSubTotal; }
    BOOL			IsChanged() const  						{ return bChanged; }
    void			ResetChanged()							{ bChanged = FALSE; }
    BOOL			IsValue();
    double			GetValue();
    double			GetValueAlways();	// ignore errors
    void			GetString( String& rString );
    void			GetMatrix( ScMatrix** pMatrix );
    BOOL			GetMatrixOrigin( ScAddress& rPos ) const;
    USHORT 			GetMatrixEdge( ScAddress& rOrgPos );
    USHORT			GetErrCode();
    short			GetFormatType() const   				{ return nFormatType; }
    ULONG			GetFormatIndex() const					{ return nFormatIndex; }
    void			GetFormatInfo( short& nType, ULONG& nIndex ) const
                        { nType = nFormatType; nIndex = nFormatIndex; }
    BYTE			GetMatrixFlag() const   				{ return cMatrixFlag; }
    ScTokenArray*   GetCode() const                         { return pCode; }

    BOOL			IsRunning() const						{ return bRunning; }
    void			SetRunning( BOOL bVal )					{ bRunning = bVal; }
    void 			CompileDBFormula( BOOL bCreateFormulaString );
    void 			CompileNameFormula( BOOL bCreateFormulaString );
    void 			CompileColRowNameFormula();
    ScFormulaCell*	GetPrevious() const					{ return pPrevious; }
    ScFormulaCell*	GetNext() const						{ return pNext; }
    void			SetPrevious( ScFormulaCell* pF )	{ pPrevious = pF; }
    void			SetNext( ScFormulaCell* pF )		{ pNext = pF; }
    ScFormulaCell*	GetPreviousTrack() const				{ return pPreviousTrack; }
    ScFormulaCell*	GetNextTrack() const					{ return pNextTrack; }
    void			SetPreviousTrack( ScFormulaCell* pF )	{ pPreviousTrack = pF; }
    void			SetNextTrack( ScFormulaCell* pF )		{ pNextTrack = pF; }

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );
    void			SetCompile( BOOL bVal ) { bCompile = bVal; }
    ScDocument*		GetDocument() const		{ return pDocument; }
    void			SetMatColsRows( USHORT nCols, USHORT nRows )
                                    { nMatCols = nCols; nMatRows = nRows; }
    void			GetMatColsRows( USHORT& nCols, USHORT& nRows ) const
                                    { nCols = nMatCols; nRows = nMatRows; }

                    // ob Zelle im ChangeTrack und nicht im echten Dokument ist
    void			SetInChangeTrack( BOOL bVal ) { bInChangeTrack = bVal; }
    BOOL			IsInChangeTrack() const { return bInChangeTrack; }

                    // Zu Typ und Format das entsprechende Standardformat.
                    // Bei Format "Standard" evtl. das in die Formelzelle
                    // uebernommene Format.
    ULONG			GetStandardFormat( SvNumberFormatter& rFormatter, ULONG nFormat ) const;

    // fuer die Importfilter!
    void			SetDouble( double n )					{ nErgValue = n; bIsValue = TRUE; }
    void			SetString( const String& r ) 			{ aErgString = r; bIsValue = FALSE; }
    void			SetErrCode( USHORT n );
};

//			Iterator fuer Referenzen in einer Formelzelle
class ScDetectiveRefIter
{
private:
    ScTokenArray* pCode;
    ScAddress aPos;
public:
                ScDetectiveRefIter( ScFormulaCell* pCell );
    BOOL		GetNextRef( ScTripel& rStart, ScTripel& rEnd );
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
    ScBaseCell*		Clone() const;

    void			Save( SvStream& rStream ) const;
};


//		ScBaseCell

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
    ScBroadcasterList*	pTemp = pBroadcaster;
    pBroadcaster = rOther.pBroadcaster;
    rOther.pBroadcaster = pTemp;
}

//		ScValueCell

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



//		ScStringCell

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

//		ScFormulaCell

inline ScBaseCell* ScFormulaCell::Clone(ScDocument* pDoc) const
{
    return new ScFormulaCell(pDoc, *this);
}
*/



//		ScNoteCell

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


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
