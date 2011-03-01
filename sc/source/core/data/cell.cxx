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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svl/zforlist.hxx>

#include "scitems.hxx"
#include "attrib.hxx"
#include "cell.hxx"
#include "compiler.hxx"
#include "interpre.hxx"
#include "document.hxx"
#include "scmatrix.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include "rechead.hxx"
#include "rangenam.hxx"
#include "brdcst.hxx"
#include "ddelink.hxx"
#include "validat.hxx"
#include "progress.hxx"
#include "editutil.hxx"
#include "recursionhelper.hxx"
#include "postit.hxx"
#include "externalrefmgr.hxx"
#include "macromgr.hxx"
#include <editeng/editobj.hxx>
#include <svl/intitem.hxx>
#include <editeng/flditem.hxx>
#include <svl/broadcast.hxx>

using namespace formula;
// More or less arbitrary, of course all recursions must fit into available
// stack space (which is what on all systems we don't know yet?). Choosing a
// lower value may be better than trying a much higher value that also isn't
// sufficient but temporarily leads to high memory consumption. On the other
// hand, if the value fits all recursions, execution is quicker as no resumes
// are necessary. Could be made a configurable option.
// Allow for a year's calendar (366).
const USHORT MAXRECURSION = 400;

// STATIC DATA -----------------------------------------------------------

#ifdef USE_MEMPOOL
// MemPools auf 4k Boundaries  - 64 Bytes ausrichten
const USHORT nMemPoolValueCell = (0x8000 - 64) / sizeof(ScValueCell);
const USHORT nMemPoolFormulaCell = (0x8000 - 64) / sizeof(ScFormulaCell);
const USHORT nMemPoolStringCell = (0x4000 - 64) / sizeof(ScStringCell);
const USHORT nMemPoolNoteCell = (0x1000 - 64) / sizeof(ScNoteCell);
IMPL_FIXEDMEMPOOL_NEWDEL( ScValueCell,   nMemPoolValueCell, nMemPoolValueCell )
IMPL_FIXEDMEMPOOL_NEWDEL( ScFormulaCell, nMemPoolFormulaCell, nMemPoolFormulaCell )
IMPL_FIXEDMEMPOOL_NEWDEL( ScStringCell,  nMemPoolStringCell, nMemPoolStringCell )
IMPL_FIXEDMEMPOOL_NEWDEL( ScNoteCell,    nMemPoolNoteCell, nMemPoolNoteCell )
#endif

// ============================================================================

ScBaseCell::ScBaseCell( CellType eNewType ) :
    mpNote( 0 ),
    mpBroadcaster( 0 ),
    nTextWidth( TEXTWIDTH_DIRTY ),
    eCellType( sal::static_int_cast<BYTE>(eNewType) ),
    nScriptType( SC_SCRIPTTYPE_UNKNOWN )
{
}

ScBaseCell::ScBaseCell( const ScBaseCell& rCell ) :
    mpNote( 0 ),
    mpBroadcaster( 0 ),
    nTextWidth( rCell.nTextWidth ),
    eCellType( rCell.eCellType ),
    nScriptType( SC_SCRIPTTYPE_UNKNOWN )
{
}

ScBaseCell::~ScBaseCell()
{
    delete mpNote;
    delete mpBroadcaster;
    DBG_ASSERT( eCellType == CELLTYPE_DESTROYED, "BaseCell Destructor" );
}

namespace {

ScBaseCell* lclCloneCell( const ScBaseCell& rSrcCell, ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags )
{
    switch( rSrcCell.GetCellType() )
    {
        case CELLTYPE_VALUE:
            return new ScValueCell( static_cast< const ScValueCell& >( rSrcCell ) );
        case CELLTYPE_STRING:
            return new ScStringCell( static_cast< const ScStringCell& >( rSrcCell ) );
        case CELLTYPE_EDIT:
            return new ScEditCell( static_cast< const ScEditCell& >( rSrcCell ), rDestDoc );
        case CELLTYPE_FORMULA:
            return new ScFormulaCell( static_cast< const ScFormulaCell& >( rSrcCell ), rDestDoc, rDestPos, nCloneFlags );
        case CELLTYPE_NOTE:
            return new ScNoteCell;
        default:;
    }
    OSL_FAIL( "lclCloneCell - unknown cell type" );
    return 0;
}

} // namespace

ScBaseCell* ScBaseCell::CloneWithoutNote( ScDocument& rDestDoc, int nCloneFlags ) const
{
    // notes will not be cloned -> cell address only needed for formula cells
    ScAddress aDestPos;
    if( eCellType == CELLTYPE_FORMULA )
        aDestPos = static_cast< const ScFormulaCell* >( this )->aPos;
    return lclCloneCell( *this, rDestDoc, aDestPos, nCloneFlags );
}

ScBaseCell* ScBaseCell::CloneWithoutNote( ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags ) const
{
    return lclCloneCell( *this, rDestDoc, rDestPos, nCloneFlags );
}

ScBaseCell* ScBaseCell::CloneWithNote( const ScAddress& rOwnPos, ScDocument& rDestDoc, const ScAddress& rDestPos, int nCloneFlags ) const
{
    ScBaseCell* pNewCell = lclCloneCell( *this, rDestDoc, rDestPos, nCloneFlags );
    if( mpNote )
    {
        if( !pNewCell )
            pNewCell = new ScNoteCell;
        bool bCloneCaption = (nCloneFlags & SC_CLONECELL_NOCAPTION) == 0;
        pNewCell->TakeNote( mpNote->Clone( rOwnPos, rDestDoc, rDestPos, bCloneCaption ) );
    }
    return pNewCell;
}

void ScBaseCell::Delete()
{
    DeleteNote();
    switch (eCellType)
    {
        case CELLTYPE_VALUE:
            delete (ScValueCell*) this;
            break;
        case CELLTYPE_STRING:
            delete (ScStringCell*) this;
            break;
        case CELLTYPE_EDIT:
            delete (ScEditCell*) this;
            break;
        case CELLTYPE_FORMULA:
            delete (ScFormulaCell*) this;
            break;
        case CELLTYPE_NOTE:
            delete (ScNoteCell*) this;
            break;
        default:
            OSL_FAIL("Unbekannter Zellentyp");
            break;
    }
}

bool ScBaseCell::IsBlank( bool bIgnoreNotes ) const
{
    return (eCellType == CELLTYPE_NOTE) && (bIgnoreNotes || !mpNote);
}

void ScBaseCell::TakeNote( ScPostIt* pNote )
{
    delete mpNote;
    mpNote = pNote;
}

ScPostIt* ScBaseCell::ReleaseNote()
{
    ScPostIt* pNote = mpNote;
    mpNote = 0;
    return pNote;
}

void ScBaseCell::DeleteNote()
{
    DELETEZ( mpNote );
}

void ScBaseCell::TakeBroadcaster( SvtBroadcaster* pBroadcaster )
{
    delete mpBroadcaster;
    mpBroadcaster = pBroadcaster;
}

SvtBroadcaster* ScBaseCell::ReleaseBroadcaster()
{
    SvtBroadcaster* pBroadcaster = mpBroadcaster;
    mpBroadcaster = 0;
    return pBroadcaster;
}

void ScBaseCell::DeleteBroadcaster()
{
    DELETEZ( mpBroadcaster );
}

ScBaseCell* ScBaseCell::CreateTextCell( const String& rString, ScDocument* pDoc )
{
    if ( rString.Search('\n') != STRING_NOTFOUND || rString.Search(CHAR_CR) != STRING_NOTFOUND )
        return new ScEditCell( rString, pDoc );
    else
        return new ScStringCell( rString );
}

void ScBaseCell::StartListeningTo( ScDocument* pDoc )
{
    if ( eCellType == CELLTYPE_FORMULA && !pDoc->IsClipOrUndo()
            && !pDoc->GetNoListening()
            && !((ScFormulaCell*)this)->IsInChangeTrack()
        )
    {
        pDoc->SetDetectiveDirty(TRUE);  // es hat sich was geaendert...

        ScFormulaCell* pFormCell = (ScFormulaCell*)this;
        ScTokenArray* pArr = pFormCell->GetCode();
        if( pArr->IsRecalcModeAlways() )
            pDoc->StartListeningArea( BCA_LISTEN_ALWAYS, pFormCell );
        else
        {
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
                        rRef1.CalcAbsIfRel( pFormCell->aPos );
                        if ( rRef1.Valid() )
                        {
                            pDoc->StartListeningCell(
                                ScAddress( rRef1.nCol,
                                           rRef1.nRow,
                                           rRef1.nTab ), pFormCell );
                        }
                    break;
                    case svDoubleRef:
                        t->CalcAbsIfRel( pFormCell->aPos );
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
                                        rRef2.nTab ), pFormCell );
                                }
                                else
                                {   // RowName
                                    pDoc->StartListeningArea( ScRange (
                                        rRef1.nCol,
                                        rRef1.nRow,
                                        rRef1.nTab,
                                        MAXCOL,
                                        rRef2.nRow,
                                        rRef2.nTab ), pFormCell );
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
                                    rRef2.nTab ), pFormCell );
                            }
                        }
                    break;
                    default:
                        ;   // nothing
                }
            }
        }
        pFormCell->SetNeedsListening( FALSE);
    }
}

//  pArr gesetzt -> Referenzen von anderer Zelle nehmen
// dann muss auch aPos uebergeben werden!

void ScBaseCell::EndListeningTo( ScDocument* pDoc, ScTokenArray* pArr,
        ScAddress aPos )
{
    if ( eCellType == CELLTYPE_FORMULA && !pDoc->IsClipOrUndo()
            && !((ScFormulaCell*)this)->IsInChangeTrack()
        )
    {
        pDoc->SetDetectiveDirty(TRUE);  // es hat sich was geaendert...

        ScFormulaCell* pFormCell = (ScFormulaCell*)this;
        if( pFormCell->GetCode()->IsRecalcModeAlways() )
            pDoc->EndListeningArea( BCA_LISTEN_ALWAYS, pFormCell );
        else
        {
            if (!pArr)
            {
                pArr = pFormCell->GetCode();
                aPos = pFormCell->aPos;
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
                        rRef1.CalcAbsIfRel( aPos );
                        if ( rRef1.Valid() )
                        {
                            pDoc->EndListeningCell(
                                ScAddress( rRef1.nCol,
                                           rRef1.nRow,
                                           rRef1.nTab ), pFormCell );
                        }
                    break;
                    case svDoubleRef:
                        t->CalcAbsIfRel( aPos );
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
                                        rRef2.nTab ), pFormCell );
                                }
                                else
                                {   // RowName
                                    pDoc->EndListeningArea( ScRange (
                                        rRef1.nCol,
                                        rRef1.nRow,
                                        rRef1.nTab,
                                        MAXCOL,
                                        rRef2.nRow,
                                        rRef2.nTab ), pFormCell );
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
                                    rRef2.nTab ), pFormCell );
                            }
                        }
                    break;
                    default:
                        ;   // nothing
                }
            }
        }
    }
}


USHORT ScBaseCell::GetErrorCode() const
{
    switch ( eCellType )
    {
        case CELLTYPE_FORMULA :
            return ((ScFormulaCell*)this)->GetErrCode();
        default:
            return 0;
    }
}


BOOL ScBaseCell::HasEmptyData() const
{
    switch ( eCellType )
    {
        case CELLTYPE_NOTE :
            return TRUE;
        case CELLTYPE_FORMULA :
            return ((ScFormulaCell*)this)->IsEmpty();
        default:
            return FALSE;
    }
}


BOOL ScBaseCell::HasValueData() const
{
    switch ( eCellType )
    {
        case CELLTYPE_VALUE :
            return TRUE;
        case CELLTYPE_FORMULA :
            return ((ScFormulaCell*)this)->IsValue();
        default:
            return FALSE;
    }
}


BOOL ScBaseCell::HasStringData() const
{
    switch ( eCellType )
    {
        case CELLTYPE_STRING :
        case CELLTYPE_EDIT :
            return TRUE;
        case CELLTYPE_FORMULA :
            return !((ScFormulaCell*)this)->IsValue();
        default:
            return FALSE;
    }
}

String ScBaseCell::GetStringData() const
{
    String aStr;
    switch ( eCellType )
    {
        case CELLTYPE_STRING:
            ((const ScStringCell*)this)->GetString( aStr );
            break;
        case CELLTYPE_EDIT:
            ((const ScEditCell*)this)->GetString( aStr );
            break;
        case CELLTYPE_FORMULA:
            ((ScFormulaCell*)this)->GetString( aStr );      // an der Formelzelle nicht-const
            break;
    }
    return aStr;
}

BOOL ScBaseCell::CellEqual( const ScBaseCell* pCell1, const ScBaseCell* pCell2 )
{
    CellType eType1 = CELLTYPE_NONE;
    CellType eType2 = CELLTYPE_NONE;
    if ( pCell1 )
    {
        eType1 = pCell1->GetCellType();
        if (eType1 == CELLTYPE_EDIT)
            eType1 = CELLTYPE_STRING;
        else if (eType1 == CELLTYPE_NOTE)
            eType1 = CELLTYPE_NONE;
    }
    if ( pCell2 )
    {
        eType2 = pCell2->GetCellType();
        if (eType2 == CELLTYPE_EDIT)
            eType2 = CELLTYPE_STRING;
        else if (eType2 == CELLTYPE_NOTE)
            eType2 = CELLTYPE_NONE;
    }
    if ( eType1 != eType2 )
        return FALSE;

    switch ( eType1 )               // beide Typen gleich
    {
        case CELLTYPE_NONE:         // beide leer
            return TRUE;
        case CELLTYPE_VALUE:        // wirklich Value-Zellen
            return ( ((const ScValueCell*)pCell1)->GetValue() ==
                     ((const ScValueCell*)pCell2)->GetValue() );
        case CELLTYPE_STRING:       // String oder Edit
            {
                String aText1;
                if ( pCell1->GetCellType() == CELLTYPE_STRING )
                    ((const ScStringCell*)pCell1)->GetString(aText1);
                else
                    ((const ScEditCell*)pCell1)->GetString(aText1);
                String aText2;
                if ( pCell2->GetCellType() == CELLTYPE_STRING )
                    ((const ScStringCell*)pCell2)->GetString(aText2);
                else
                    ((const ScEditCell*)pCell2)->GetString(aText2);
                return ( aText1 == aText2 );
            }
        case CELLTYPE_FORMULA:
            {
                //! eingefuegte Zeilen / Spalten beruecksichtigen !!!!!
                //! Vergleichsfunktion an der Formelzelle ???
                //! Abfrage mit ScColumn::SwapRow zusammenfassen!

                ScTokenArray* pCode1 = ((ScFormulaCell*)pCell1)->GetCode();
                ScTokenArray* pCode2 = ((ScFormulaCell*)pCell2)->GetCode();

                if (pCode1->GetLen() == pCode2->GetLen())       // nicht-UPN
                {
                    BOOL bEqual = TRUE;
                    USHORT nLen = pCode1->GetLen();
                    FormulaToken** ppToken1 = pCode1->GetArray();
                    FormulaToken** ppToken2 = pCode2->GetArray();
                    for (USHORT i=0; i<nLen; i++)
                        if ( !ppToken1[i]->TextEqual(*(ppToken2[i])) )
                        {
                            bEqual = FALSE;
                            break;
                        }

                    if (bEqual)
                        return TRUE;
                }

                return FALSE;       // unterschiedlich lang oder unterschiedliche Tokens
            }
        default:
            OSL_FAIL("huch, was fuer Zellen???");
    }
    return FALSE;
}

// ============================================================================

ScNoteCell::ScNoteCell( SvtBroadcaster* pBC ) :
    ScBaseCell( CELLTYPE_NOTE )
{
    TakeBroadcaster( pBC );
}

ScNoteCell::ScNoteCell( ScPostIt* pNote, SvtBroadcaster* pBC ) :
    ScBaseCell( CELLTYPE_NOTE )
{
    TakeNote( pNote );
    TakeBroadcaster( pBC );
}

#ifdef DBG_UTIL
ScNoteCell::~ScNoteCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif

// ============================================================================

ScValueCell::ScValueCell() :
    ScBaseCell( CELLTYPE_VALUE ),
    mfValue( 0.0 )
{
}

ScValueCell::ScValueCell( double fValue ) :
    ScBaseCell( CELLTYPE_VALUE ),
    mfValue( fValue )
{
}

#ifdef DBG_UTIL
ScValueCell::~ScValueCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif

// ============================================================================

ScStringCell::ScStringCell() :
    ScBaseCell( CELLTYPE_STRING )
{
}

ScStringCell::ScStringCell( const String& rString ) :
    ScBaseCell( CELLTYPE_STRING ),
    maString( rString.intern() )
{
}

#ifdef DBG_UTIL
ScStringCell::~ScStringCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif

// ============================================================================

//
//      ScFormulaCell
//

ScFormulaCell::ScFormulaCell() :
    ScBaseCell( CELLTYPE_FORMULA ),
    eTempGrammar( FormulaGrammar::GRAM_DEFAULT),
    pCode( NULL ),
    pDocument( NULL ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nFormatIndex(0),
    nFormatType( NUMBERFORMAT_NUMBER ),
    nSeenInIteration(0),
    cMatrixFlag ( MM_NONE ),
    bDirty( FALSE ),
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    bIsIterCell( FALSE ),
    bInChangeTrack( FALSE ),
    bTableOpDirty( FALSE ),
    bNeedListening( FALSE ),
    aPos(0,0,0)
{
}

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const String& rFormula,
                              const FormulaGrammar::Grammar eGrammar,
                              BYTE cMatInd ) :
    ScBaseCell( CELLTYPE_FORMULA ),
    eTempGrammar( eGrammar),
    pCode( NULL ),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nFormatIndex(0),
    nFormatType( NUMBERFORMAT_NUMBER ),
    nSeenInIteration(0),
    cMatrixFlag ( cMatInd ),
    bDirty( TRUE ), // -> wg. Benutzung im Fkt.AutoPiloten, war: cMatInd != 0
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    bIsIterCell( FALSE ),
    bInChangeTrack( FALSE ),
    bTableOpDirty( FALSE ),
    bNeedListening( FALSE ),
    aPos( rPos )
{
    Compile( rFormula, TRUE, eGrammar );    // bNoListening, Insert does that
}

// Wird von den Importfiltern verwendet

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const ScTokenArray* pArr,
                              const FormulaGrammar::Grammar eGrammar, BYTE cInd ) :
    ScBaseCell( CELLTYPE_FORMULA ),
    eTempGrammar( eGrammar),
    pCode( pArr ? new ScTokenArray( *pArr ) : new ScTokenArray ),
    pDocument( pDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nFormatIndex(0),
    nFormatType( NUMBERFORMAT_NUMBER ),
    nSeenInIteration(0),
    cMatrixFlag ( cInd ),
    bDirty( NULL != pArr ), // -> wg. Benutzung im Fkt.AutoPiloten, war: cInd != 0
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    bIsIterCell( FALSE ),
    bInChangeTrack( FALSE ),
    bTableOpDirty( FALSE ),
    bNeedListening( FALSE ),
    aPos( rPos )
{
    // UPN-Array erzeugen
    if( pCode->GetLen() && !pCode->GetCodeError() && !pCode->GetCodeLen() )
    {
        ScCompiler aComp( pDocument, aPos, *pCode);
        aComp.SetGrammar(eTempGrammar);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
    }
    else
    {
        pCode->Reset();
        if ( pCode->GetNextOpCodeRPN( ocSubTotal ) )
            bSubTotal = TRUE;
    }

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);
}

ScFormulaCell::ScFormulaCell( const ScFormulaCell& rCell, ScDocument& rDoc, const ScAddress& rPos, int nCloneFlags ) :
    ScBaseCell( rCell ),
    SvtListener(),
    aResult( rCell.aResult ),
    eTempGrammar( rCell.eTempGrammar),
    pDocument( &rDoc ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nFormatIndex( &rDoc == rCell.pDocument ? rCell.nFormatIndex : 0 ),
    nFormatType( rCell.nFormatType ),
    nSeenInIteration(0),
    cMatrixFlag ( rCell.cMatrixFlag ),
    bDirty( rCell.bDirty ),
    bChanged( rCell.bChanged ),
    bRunning( FALSE ),
    bCompile( rCell.bCompile ),
    bSubTotal( rCell.bSubTotal ),
    bIsIterCell( FALSE ),
    bInChangeTrack( FALSE ),
    bTableOpDirty( FALSE ),
    bNeedListening( FALSE ),
    aPos( rPos )
{
    pCode = rCell.pCode->Clone();

    if ( nCloneFlags & SC_CLONECELL_ADJUST3DREL )
        pCode->ReadjustRelative3DReferences( rCell.aPos, aPos );

    // evtl. Fehler zuruecksetzen und neu kompilieren
    //  nicht im Clipboard - da muss das Fehlerflag erhalten bleiben
    //  Spezialfall Laenge=0: als Fehlerzelle erzeugt, dann auch Fehler behalten
    if ( pCode->GetCodeError() && !pDocument->IsClipboard() && pCode->GetLen() )
    {
        pCode->SetCodeError( 0 );
        bCompile = TRUE;
    }
    //! Compile ColRowNames on URM_MOVE/URM_COPY _after_ UpdateReference
    BOOL bCompileLater = FALSE;
    BOOL bClipMode = rCell.pDocument->IsClipboard();
    if( !bCompile )
    {   // Name references with references and ColRowNames
        pCode->Reset();
        ScToken* t;
        while ( ( t = static_cast<ScToken*>(pCode->GetNextReferenceOrName()) ) != NULL && !bCompile )
        {
            if ( t->IsExternalRef() )
            {
                // External name, cell, and area references.
                bCompile = true;
            }
            else if ( t->GetType() == svIndex )
            {
                ScRangeData* pRangeData = rDoc.GetRangeName()->FindIndex( t->GetIndex() );
                if( pRangeData )
                {
                    if( pRangeData->HasReferences() )
                        bCompile = TRUE;
                }
                else
                    bCompile = TRUE;    // invalid reference!
            }
            else if ( t->GetOpCode() == ocColRowName )
            {
                bCompile = TRUE;        // new lookup needed
                bCompileLater = bClipMode;
            }
        }
    }
    if( bCompile )
    {
        if ( !bCompileLater && bClipMode )
        {
            // Merging ranges needs the actual positions after UpdateReference.
            // ColRowNames need new lookup after positions are adjusted.
            bCompileLater = pCode->HasOpCode( ocRange) || pCode->HasOpCode( ocColRowName);
        }
        if ( !bCompileLater )
        {
            // bNoListening, not at all if in Clipboard/Undo,
            // and not from Clipboard either, instead after Insert(Clone) and UpdateReference.
            CompileTokenArray( TRUE );
        }
    }

    if( nCloneFlags & SC_CLONECELL_STARTLISTENING )
        StartListeningTo( &rDoc );

    if (bSubTotal)
        pDocument->AddSubTotalCell(this);
}

ScFormulaCell::~ScFormulaCell()
{
    pDocument->RemoveFromFormulaTree( this );
    pDocument->RemoveSubTotalCell(this);
    if (pCode->HasOpCode(ocMacro))
        pDocument->GetMacroManager()->RemoveDependentCell(this);

    if (pDocument->HasExternalRefManager())
        pDocument->GetExternalRefManager()->removeRefCell(this);

    delete pCode;
#ifdef DBG_UTIL
    eCellType = CELLTYPE_DESTROYED;
#endif
}

void ScFormulaCell::GetFormula( rtl::OUStringBuffer& rBuffer,
                                const FormulaGrammar::Grammar eGrammar ) const
{
    if( pCode->GetCodeError() && !pCode->GetLen() )
    {
        rBuffer = rtl::OUStringBuffer( ScGlobal::GetErrorString( pCode->GetCodeError()));
        return;
    }
    else if( cMatrixFlag == MM_REFERENCE )
    {
        // Reference to another cell that contains a matrix formula.
        pCode->Reset();
        ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        if( p )
        {
            /* FIXME: original GetFormula() code obtained
             * pCell only if (!this->IsInChangeTrack()),
             * GetEnglishFormula() omitted that test.
             * Can we live without in all cases? */
            ScBaseCell* pCell;
            ScSingleRefData& rRef = p->GetSingleRef();
            rRef.CalcAbsIfRel( aPos );
            if ( rRef.Valid() )
                pCell = pDocument->GetCell( ScAddress( rRef.nCol,
                            rRef.nRow, rRef.nTab ) );
            else
                pCell = NULL;
            if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                ((ScFormulaCell*)pCell)->GetFormula( rBuffer, eGrammar);
                return;
            }
            else
            {
                ScCompiler aComp( pDocument, aPos, *pCode);
                aComp.SetGrammar(eGrammar);
                aComp.CreateStringFromTokenArray( rBuffer );
            }
        }
        else
        {
            OSL_FAIL("ScFormulaCell::GetFormula: not a matrix");
        }
    }
    else
    {
        ScCompiler aComp( pDocument, aPos, *pCode);
        aComp.SetGrammar(eGrammar);
        aComp.CreateStringFromTokenArray( rBuffer );
    }

    sal_Unicode ch('=');
    rBuffer.insert( 0, &ch, 1 );
    if( cMatrixFlag )
    {
        sal_Unicode ch2('{');
        rBuffer.insert( 0, &ch2, 1);
        rBuffer.append( sal_Unicode('}'));
    }
}

void ScFormulaCell::GetFormula( String& rFormula, const FormulaGrammar::Grammar eGrammar ) const
{
    rtl::OUStringBuffer rBuffer( rFormula );
    GetFormula( rBuffer, eGrammar );
    rFormula = rBuffer;
}

void ScFormulaCell::GetResultDimensions( SCSIZE& rCols, SCSIZE& rRows )
{
    MaybeInterpret();

    const ScMatrix* pMat = NULL;
    if (!pCode->GetCodeError() && aResult.GetType() == svMatrixCell &&
            ((pMat = static_cast<const ScToken*>(aResult.GetToken().get())->GetMatrix()) != 0))
        pMat->GetDimensions( rCols, rRows );
    else
    {
        rCols = 0;
        rRows = 0;
    }
}

void ScFormulaCell::Compile( const String& rFormula, BOOL bNoListening,
                            const FormulaGrammar::Grammar eGrammar )
{
    if ( pDocument->IsClipOrUndo() ) return;
    BOOL bWasInFormulaTree = pDocument->IsInFormulaTree( this );
    if ( bWasInFormulaTree )
        pDocument->RemoveFromFormulaTree( this );
    // pCode darf fuer Abfragen noch nicht geloescht, muss aber leer sein
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    ScCompiler aComp( pDocument, aPos);
    aComp.SetGrammar(eGrammar);
    pCode = aComp.CompileString( rFormula );
    if ( pCodeOld )
        delete pCodeOld;
    if( !pCode->GetCodeError() )
    {
        if ( !pCode->GetLen() && aResult.GetHybridFormula().Len() && rFormula == aResult.GetHybridFormula() )
        {   // nicht rekursiv CompileTokenArray/Compile/CompileTokenArray
            if ( rFormula.GetChar(0) == '=' )
                pCode->AddBad( rFormula.GetBuffer() + 1 );
            else
                pCode->AddBad( rFormula.GetBuffer() );
        }
        bCompile = TRUE;
        CompileTokenArray( bNoListening );
    }
    else
    {
        bChanged = TRUE;
        SetTextWidth( TEXTWIDTH_DIRTY );
        SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
    }
    if ( bWasInFormulaTree )
        pDocument->PutInFormulaTree( this );
}


void ScFormulaCell::CompileTokenArray( BOOL bNoListening )
{
    // Not already compiled?
    if( !pCode->GetLen() && aResult.GetHybridFormula().Len() )
        Compile( aResult.GetHybridFormula(), bNoListening, eTempGrammar);
    else if( bCompile && !pDocument->IsClipOrUndo() && !pCode->GetCodeError() )
    {
        // RPN length may get changed
        BOOL bWasInFormulaTree = pDocument->IsInFormulaTree( this );
        if ( bWasInFormulaTree )
            pDocument->RemoveFromFormulaTree( this );

        // Loading from within filter? No listening yet!
        if( pDocument->IsInsertingFromOtherDoc() )
            bNoListening = TRUE;

        if( !bNoListening && pCode->GetCodeLen() )
            EndListeningTo( pDocument );
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetCodeError() )
        {
            nFormatType = aComp.GetNumFormatType();
            nFormatIndex = 0;
            bChanged = TRUE;
            aResult.SetToken( NULL);
            bCompile = FALSE;
            if ( !bNoListening )
                StartListeningTo( pDocument );
        }
        if ( bWasInFormulaTree )
            pDocument->PutInFormulaTree( this );

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }
}


void ScFormulaCell::CompileXML( ScProgress& rProgress )
{
    if ( cMatrixFlag == MM_REFERENCE )
    {   // is already token code via ScDocFunc::EnterMatrix, ScDocument::InsertMatrixFormula
        // just establish listeners
        StartListeningTo( pDocument );
        return ;
    }

    ScCompiler aComp( pDocument, aPos, *pCode);
    aComp.SetGrammar(eTempGrammar);
    String aFormula, aFormulaNmsp;
    aComp.CreateStringFromXMLTokenArray( aFormula, aFormulaNmsp );
    pDocument->DecXMLImportedFormulaCount( aFormula.Len() );
    rProgress.SetStateCountDownOnPercent( pDocument->GetXMLImportedFormulaCount() );
    // pCode darf fuer Abfragen noch nicht geloescht, muss aber leer sein
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    pCode = aComp.CompileString( aFormula, aFormulaNmsp );
    delete pCodeOld;
    if( !pCode->GetCodeError() )
    {
        if ( !pCode->GetLen() )
        {
            if ( aFormula.GetChar(0) == '=' )
                pCode->AddBad( aFormula.GetBuffer() + 1 );
            else
                pCode->AddBad( aFormula.GetBuffer() );
        }
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetCodeError() )
        {
            nFormatType = aComp.GetNumFormatType();
            nFormatIndex = 0;
            bChanged = TRUE;
            bCompile = FALSE;
            StartListeningTo( pDocument );
        }

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }
    else
    {
        bChanged = TRUE;
        SetTextWidth( TEXTWIDTH_DIRTY );
        SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
    }

    //  Same as in Load: after loading, it must be known if ocMacro is in any formula
    //  (for macro warning, CompileXML is called at the end of loading XML file)
    if ( !pDocument->GetHasMacroFunc() && pCode->HasOpCodeRPN( ocMacro ) )
        pDocument->SetHasMacroFunc( TRUE );
}


void ScFormulaCell::CalcAfterLoad()
{
    BOOL bNewCompiled = FALSE;
    // Falls ein Calc 1.0-Doc eingelesen wird, haben wir ein Ergebnis,
    // aber kein TokenArray
    if( !pCode->GetLen() && aResult.GetHybridFormula().Len() )
    {
        Compile( aResult.GetHybridFormula(), TRUE, eTempGrammar);
        aResult.SetToken( NULL);
        bDirty = TRUE;
        bNewCompiled = TRUE;
    }
    // Das UPN-Array wird nicht erzeugt, wenn ein Calc 3.0-Doc eingelesen
    // wurde, da die RangeNames erst jetzt existieren.
    if( pCode->GetLen() && !pCode->GetCodeLen() && !pCode->GetCodeError() )
    {
        ScCompiler aComp(pDocument, aPos, *pCode);
        aComp.SetGrammar(pDocument->GetGrammar());
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
        nFormatIndex = 0;
        bDirty = TRUE;
        bCompile = FALSE;
        bNewCompiled = TRUE;

        if (bSubTotal)
            pDocument->AddSubTotalCell(this);
    }
    // irgendwie koennen unter os/2 mit rotter FPU-Exception /0 ohne Err503
    // gespeichert werden, woraufhin spaeter im NumberFormatter die BLC Lib
    // bei einem fabs(-NAN) abstuerzt (#32739#)
    // hier fuer alle Systeme ausbuegeln, damit da auch Err503 steht
    if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
    {
        DBG_ERRORFILE("Formelzelle INFINITY !!! Woher kommt das Dokument?");
        aResult.SetResultError( errIllegalFPOperation );
        bDirty = TRUE;
    }
    // DoubleRefs bei binaeren Operatoren waren vor v5.0 immer Matrix,
    // jetzt nur noch wenn in Matrixformel, sonst implizite Schnittmenge
    if ( pDocument->GetSrcVersion() < SC_MATRIX_DOUBLEREF &&
            GetMatrixFlag() == MM_NONE && pCode->HasMatrixDoubleRefOps() )
    {
        cMatrixFlag = MM_FORMULA;
        SetMatColsRows( 1, 1);
    }
    // Muss die Zelle berechnet werden?
    // Nach Load koennen Zellen einen Fehlercode enthalten, auch dann
    // Listener starten und ggbf. neu berechnen wenn nicht RECALCMODE_NORMAL
    if( !bNewCompiled || !pCode->GetCodeError() )
    {
        StartListeningTo( pDocument );
        if( !pCode->IsRecalcModeNormal() )
            bDirty = TRUE;
    }
    if ( pCode->IsRecalcModeAlways() )
    {   // zufall(), heute(), jetzt() bleiben immer im FormulaTree, damit sie
        // auch bei jedem F9 berechnet werden.
        bDirty = TRUE;
    }
    // Noch kein SetDirty weil noch nicht alle Listener bekannt, erst in
    // SetDirtyAfterLoad.
}


bool ScFormulaCell::MarkUsedExternalReferences()
{
    return pCode && pDocument->MarkUsedExternalReferences( *pCode);
}


void ScFormulaCell::Interpret()
{
    if (!IsDirtyOrInTableOpDirty() || pDocument->GetRecursionHelper().IsInReturn())
        return;     // no double/triple processing

    //! HACK:
    //  Wenn der Aufruf aus einem Reschedule im DdeLink-Update kommt, dirty stehenlassen
    //  Besser: Dde-Link Update ohne Reschedule oder ganz asynchron !!!

    if ( pDocument->IsInDdeLinkUpdate() )
        return;

    if (bRunning)
    {
        if (!pDocument->GetDocOptions().IsIter())
        {
            aResult.SetResultError( errCircularReference );
            return;
        }

        if (aResult.GetResultError() == errCircularReference)
            aResult.SetResultError( 0 );

        // Start or add to iteration list.
        if (!pDocument->GetRecursionHelper().IsDoingIteration() ||
                !pDocument->GetRecursionHelper().GetRecursionInIterationStack().top()->bIsIterCell)
            pDocument->GetRecursionHelper().SetInIterationReturn( true);

        return;
    }
    // no multiple interprets for GetErrCode, IsValue, GetValue and
    // different entry point recursions. Would also lead to premature
    // convergence in iterations.
    if (pDocument->GetRecursionHelper().GetIteration() && nSeenInIteration ==
            pDocument->GetRecursionHelper().GetIteration())
        return ;

    ScRecursionHelper& rRecursionHelper = pDocument->GetRecursionHelper();
    BOOL bOldRunning = bRunning;
    if (rRecursionHelper.GetRecursionCount() > MAXRECURSION)
    {
        bRunning = TRUE;
        rRecursionHelper.SetInRecursionReturn( true);
    }
    else
    {
        InterpretTail( SCITP_NORMAL);
    }

    // While leaving a recursion or iteration stack, insert its cells to the
    // recursion list in reverse order.
    if (rRecursionHelper.IsInReturn())
    {
        if (rRecursionHelper.GetRecursionCount() > 0 ||
                !rRecursionHelper.IsDoingRecursion())
            rRecursionHelper.Insert( this, bOldRunning, aResult);
        bool bIterationFromRecursion = false;
        bool bResumeIteration = false;
        do
        {
            if ((rRecursionHelper.IsInIterationReturn() &&
                        rRecursionHelper.GetRecursionCount() == 0 &&
                        !rRecursionHelper.IsDoingIteration()) ||
                    bIterationFromRecursion || bResumeIteration)
            {
                ScFormulaCell* pIterCell = this; // scope for debug convenience
                bool & rDone = rRecursionHelper.GetConvergingReference();
                rDone = false;
                if (!bIterationFromRecursion && bResumeIteration)
                {
                    bResumeIteration = false;
                    // Resuming iteration expands the range.
                    ScFormulaRecursionList::const_iterator aOldStart(
                            rRecursionHelper.GetLastIterationStart());
                    rRecursionHelper.ResumeIteration();
                    // Mark new cells being in iteration.
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            aOldStart; ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
                        pIterCell->bIsIterCell = TRUE;
                    }
                    // Mark older cells dirty again, in case they converted
                    // without accounting for all remaining cells in the circle
                    // that weren't touched so far, e.g. conditional. Restore
                    // backuped result.
                    USHORT nIteration = rRecursionHelper.GetIteration();
                    for (ScFormulaRecursionList::const_iterator aIter(
                                aOldStart); aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
                        if (pIterCell->nSeenInIteration == nIteration)
                        {
                            if (!pIterCell->bDirty || aIter == aOldStart)
                            {
                                pIterCell->aResult = (*aIter).aPreviousResult;
                            }
                            --pIterCell->nSeenInIteration;
                        }
                        pIterCell->bDirty = TRUE;
                    }
                }
                else
                {
                    bResumeIteration = false;
                    // Close circle once.
                    rRecursionHelper.GetList().back().pCell->InterpretTail(
                            SCITP_CLOSE_ITERATION_CIRCLE);
                    // Start at 1, init things.
                    rRecursionHelper.StartIteration();
                    // Mark all cells being in iteration.
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            rRecursionHelper.GetIterationEnd(); ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
                        pIterCell->bIsIterCell = TRUE;
                    }
                }
                bIterationFromRecursion = false;
                USHORT nIterMax = pDocument->GetDocOptions().GetIterCount();
                for ( ; rRecursionHelper.GetIteration() <= nIterMax && !rDone;
                        rRecursionHelper.IncIteration())
                {
                    rDone = true;
                    for ( ScFormulaRecursionList::iterator aIter(
                                rRecursionHelper.GetIterationStart()); aIter !=
                            rRecursionHelper.GetIterationEnd() &&
                            !rRecursionHelper.IsInReturn(); ++aIter)
                    {
                        pIterCell = (*aIter).pCell;
                        if (pIterCell->IsDirtyOrInTableOpDirty() &&
                                rRecursionHelper.GetIteration() !=
                                pIterCell->GetSeenInIteration())
                        {
                            (*aIter).aPreviousResult = pIterCell->aResult;
                            pIterCell->InterpretTail( SCITP_FROM_ITERATION);
                        }
                        rDone = rDone && !pIterCell->IsDirtyOrInTableOpDirty();
                    }
                    if (rRecursionHelper.IsInReturn())
                    {
                        bResumeIteration = true;
                        break;  // for
                        // Don't increment iteration.
                    }
                }
                if (!bResumeIteration)
                {
                    if (rDone)
                    {
                        for (ScFormulaRecursionList::const_iterator aIter(
                                    rRecursionHelper.GetIterationStart());
                                aIter != rRecursionHelper.GetIterationEnd();
                                ++aIter)
                        {
                            pIterCell = (*aIter).pCell;
                            pIterCell->bIsIterCell = FALSE;
                            pIterCell->nSeenInIteration = 0;
                            pIterCell->bRunning = (*aIter).bOldRunning;
                        }
                    }
                    else
                    {
                        for (ScFormulaRecursionList::const_iterator aIter(
                                    rRecursionHelper.GetIterationStart());
                                aIter != rRecursionHelper.GetIterationEnd();
                                ++aIter)
                        {
                            pIterCell = (*aIter).pCell;
                            pIterCell->bIsIterCell = FALSE;
                            pIterCell->nSeenInIteration = 0;
                            pIterCell->bRunning = (*aIter).bOldRunning;
                            // If one cell didn't converge, all cells of this
                            // circular dependency don't, no matter whether
                            // single cells did.
                            pIterCell->bDirty = FALSE;
                            pIterCell->bTableOpDirty = FALSE;
                            pIterCell->aResult.SetResultError( errNoConvergence);
                            pIterCell->bChanged = TRUE;
                            pIterCell->SetTextWidth( TEXTWIDTH_DIRTY);
                            pIterCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN);
                        }
                    }
                    // End this iteration and remove entries.
                    rRecursionHelper.EndIteration();
                    bResumeIteration = rRecursionHelper.IsDoingIteration();
                }
            }
            if (rRecursionHelper.IsInRecursionReturn() &&
                    rRecursionHelper.GetRecursionCount() == 0 &&
                    !rRecursionHelper.IsDoingRecursion())
            {
                bIterationFromRecursion = false;
                // Iterate over cells known so far, start with the last cell
                // encountered, inserting new cells if another recursion limit
                // is reached. Repeat until solved.
                rRecursionHelper.SetDoingRecursion( true);
                do
                {
                    rRecursionHelper.SetInRecursionReturn( false);
                    for (ScFormulaRecursionList::const_iterator aIter(
                                rRecursionHelper.GetStart());
                            !rRecursionHelper.IsInReturn() && aIter !=
                            rRecursionHelper.GetEnd(); ++aIter)
                    {
                        ScFormulaCell* pCell = (*aIter).pCell;
                        if (pCell->IsDirtyOrInTableOpDirty())
                        {
                            pCell->InterpretTail( SCITP_NORMAL);
                            if (!pCell->IsDirtyOrInTableOpDirty() && !pCell->IsIterCell())
                                pCell->bRunning = (*aIter).bOldRunning;
                        }
                    }
                } while (rRecursionHelper.IsInRecursionReturn());
                rRecursionHelper.SetDoingRecursion( false);
                if (rRecursionHelper.IsInIterationReturn())
                {
                    if (!bResumeIteration)
                        bIterationFromRecursion = true;
                }
                else if (bResumeIteration ||
                        rRecursionHelper.IsDoingIteration())
                    rRecursionHelper.GetList().erase(
                            rRecursionHelper.GetStart(),
                            rRecursionHelper.GetLastIterationStart());
                else
                    rRecursionHelper.Clear();
            }
        } while (bIterationFromRecursion || bResumeIteration);
    }
}

void ScFormulaCell::InterpretTail( ScInterpretTailParameter eTailParam )
{
    class RecursionCounter
    {
        ScRecursionHelper&  rRec;
        bool                bStackedInIteration;
        public:
        RecursionCounter( ScRecursionHelper& r, ScFormulaCell* p ) : rRec(r)
        {
            bStackedInIteration = rRec.IsDoingIteration();
            if (bStackedInIteration)
                rRec.GetRecursionInIterationStack().push( p);
            rRec.IncRecursionCount();
        }
        ~RecursionCounter()
        {
            rRec.DecRecursionCount();
            if (bStackedInIteration)
                rRec.GetRecursionInIterationStack().pop();
        }
    } aRecursionCounter( pDocument->GetRecursionHelper(), this);
    nSeenInIteration = pDocument->GetRecursionHelper().GetIteration();
    if( !pCode->GetCodeLen() && !pCode->GetCodeError() )
    {
        // #i11719# no UPN and no error and no token code but result string present
        // => interpretation of this cell during name-compilation and unknown names
        // => can't exchange underlying code array in CompileTokenArray() /
        // Compile() because interpreter's token iterator would crash.
        // This should only be a temporary condition and, since we set an
        // error, if ran into it again we'd bump into the dirty-clearing
        // condition further down.
        if ( !pCode->GetLen() && aResult.GetHybridFormula().Len() )
        {
            pCode->SetCodeError( errNoCode );
            // This is worth an assertion; if encountered in daily work
            // documents we might need another solution. Or just confirm correctness.
            DBG_ERRORFILE( "ScFormulaCell::Interpret: no UPN, no error, no token, but string" );
            return;
        }
        CompileTokenArray();
    }

    if( pCode->GetCodeLen() && pDocument )
    {
        class StackCleaner
        {
            ScDocument*     pDoc;
            ScInterpreter*  pInt;
            public:
            StackCleaner( ScDocument* pD, ScInterpreter* pI )
                : pDoc(pD), pInt(pI)
                {}
            ~StackCleaner()
            {
                delete pInt;
                pDoc->DecInterpretLevel();
            }
        };
        pDocument->IncInterpretLevel();
        ScInterpreter* p = new ScInterpreter( this, pDocument, aPos, *pCode );
        StackCleaner aStackCleaner( pDocument, p);
        USHORT nOldErrCode = aResult.GetResultError();
        if ( nSeenInIteration == 0 )
        {   // Only the first time
            // With bChanged=FALSE, if a newly compiled cell has a result of
            // 0.0, no change is detected and the cell will not be repainted.
            // bChanged = FALSE;
            aResult.SetResultError( 0 );
        }

        switch ( aResult.GetResultError() )
        {
            case errCircularReference :     // will be determined again if so
                aResult.SetResultError( 0 );
            break;
        }

        BOOL bOldRunning = bRunning;
        bRunning = TRUE;
        p->Interpret();
        if (pDocument->GetRecursionHelper().IsInReturn() && eTailParam != SCITP_CLOSE_ITERATION_CIRCLE)
        {
            if (nSeenInIteration > 0)
                --nSeenInIteration;     // retry when iteration is resumed
            return;
        }
        bRunning = bOldRunning;

        // #i102616# For single-sheet saving consider only content changes, not format type,
        // because format type isn't set on loading (might be changed later)
        BOOL bContentChanged = FALSE;

        // Do not create a HyperLink() cell if the formula results in an error.
        if( p->GetError() && pCode->IsHyperLink())
            pCode->SetHyperLink(FALSE);

        if( p->GetError() && p->GetError() != errCircularReference)
        {
            bDirty = FALSE;
            bTableOpDirty = FALSE;
            bChanged = TRUE;
        }
        if (eTailParam == SCITP_FROM_ITERATION && IsDirtyOrInTableOpDirty())
        {
            bool bIsValue = aResult.IsValue();  // the previous type
            // Did it converge?
            if ((bIsValue && p->GetResultType() == svDouble && fabs(
                            p->GetNumResult() - aResult.GetDouble()) <=
                        pDocument->GetDocOptions().GetIterEps()) ||
                    (!bIsValue && p->GetResultType() == svString &&
                     p->GetStringResult() == aResult.GetString()))
            {
                // A convergence in the first iteration doesn't necessarily
                // mean that it's done, it may be because not all related cells
                // of a circle changed their values yet. If the set really
                // converges it will do so also during the next iteration. This
                // fixes situations like of #i44115#. If this wasn't wanted an
                // initial "uncalculated" value would be needed for all cells
                // of a circular dependency => graph needed before calculation.
                if (nSeenInIteration > 1 ||
                        pDocument->GetDocOptions().GetIterCount() == 1)
                {
                    bDirty = FALSE;
                    bTableOpDirty = FALSE;
                }
            }
        }

        // New error code?
        if( p->GetError() != nOldErrCode )
        {
            bChanged = TRUE;
            // bContentChanged only has to be set if the file content would be changed
            if ( aResult.GetCellResultType() != svUnknown )
                bContentChanged = TRUE;
        }
        // Different number format?
        if( nFormatType != p->GetRetFormatType() )
        {
            nFormatType = p->GetRetFormatType();
            bChanged = TRUE;
        }
        if( nFormatIndex != p->GetRetFormatIndex() )
        {
            nFormatIndex = p->GetRetFormatIndex();
            bChanged = TRUE;
        }

        // In case of changes just obtain the result, no temporary and
        // comparison needed anymore.
        if (bChanged)
        {
            // #i102616# Compare anyway if the sheet is still marked unchanged for single-sheet saving
            // Also handle special cases of initial results after loading.

            if ( !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                ScFormulaResult aNewResult( p->GetResultToken().get());
                StackVar eOld = aResult.GetCellResultType();
                StackVar eNew = aNewResult.GetCellResultType();
                if ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) )
                {
                    // ScXMLTableRowCellContext::EndElement doesn't call SetFormulaResultDouble for 0
                    // -> no change
                }
                else
                {
                    if ( eOld == svHybridCell )     // string result from SetFormulaResultString?
                        eOld = svString;            // ScHybridCellToken has a valid GetString method

                    // #i106045# use approxEqual to compare with stored value
                    bContentChanged = (eOld != eNew ||
                            (eNew == svDouble && !rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble() )) ||
                            (eNew == svString && aResult.GetString() != aNewResult.GetString()));
                }
            }

            aResult.SetToken( p->GetResultToken().get() );
        }
        else
        {
            ScFormulaResult aNewResult( p->GetResultToken().get());
            StackVar eOld = aResult.GetCellResultType();
            StackVar eNew = aNewResult.GetCellResultType();
            bChanged = (eOld != eNew ||
                    (eNew == svDouble && aResult.GetDouble() != aNewResult.GetDouble()) ||
                    (eNew == svString && aResult.GetString() != aNewResult.GetString()));

            // #i102616# handle special cases of initial results after loading (only if the sheet is still marked unchanged)
            if ( bChanged && !bContentChanged && pDocument->IsStreamValid(aPos.Tab()) )
            {
                if ( ( eOld == svUnknown && ( eNew == svError || ( eNew == svDouble && aNewResult.GetDouble() == 0.0 ) ) ) ||
                     ( eOld == svHybridCell && eNew == svString && aResult.GetString() == aNewResult.GetString() ) ||
                     ( eOld == svDouble && eNew == svDouble && rtl::math::approxEqual( aResult.GetDouble(), aNewResult.GetDouble() ) ) )
                {
                    // no change, see above
                }
                else
                    bContentChanged = TRUE;
            }

            aResult.Assign( aNewResult);
        }

        // Precision as shown?
        if ( aResult.IsValue() && !p->GetError()
          && pDocument->GetDocOptions().IsCalcAsShown()
          && nFormatType != NUMBERFORMAT_DATE
          && nFormatType != NUMBERFORMAT_TIME
          && nFormatType != NUMBERFORMAT_DATETIME )
        {
            ULONG nFormat = pDocument->GetNumberFormat( aPos );
            if ( nFormatIndex && (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                nFormat = nFormatIndex;
            if ( (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
                nFormat = ScGlobal::GetStandardFormat(
                    *pDocument->GetFormatTable(), nFormat, nFormatType );
            aResult.SetDouble( pDocument->RoundValueAsShown(
                        aResult.GetDouble(), nFormat));
        }
        if (eTailParam == SCITP_NORMAL)
        {
            bDirty = FALSE;
            bTableOpDirty = FALSE;
        }
        if( aResult.GetMatrix() )
        {
            // If the formula wasn't entered as a matrix formula, live on with
            // the upper left corner and let reference counting delete the matrix.
            if( cMatrixFlag != MM_FORMULA && !pCode->IsHyperLink() )
                aResult.SetToken( aResult.GetCellResultToken().get());
        }
        if ( aResult.IsValue() && !::rtl::math::isFinite( aResult.GetDouble() ) )
        {
            // Coded double error may occur via filter import.
            USHORT nErr = GetDoubleErrorValue( aResult.GetDouble());
            aResult.SetResultError( nErr);
            bChanged = bContentChanged = true;
        }
        if( bChanged )
        {
            SetTextWidth( TEXTWIDTH_DIRTY );
            SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
        }
        if (bContentChanged && pDocument->IsStreamValid(aPos.Tab()))
        {
            // pass bIgnoreLock=TRUE, because even if called from pending row height update,
            // a changed result must still reset the stream flag
            pDocument->SetStreamValid(aPos.Tab(), FALSE, TRUE);
        }
        if ( !pCode->IsRecalcModeAlways() )
            pDocument->RemoveFromFormulaTree( this );

        //  FORCED Zellen auch sofort auf Gueltigkeit testen (evtl. Makro starten)

        if ( pCode->IsRecalcModeForced() )
        {
            ULONG nValidation = ((const SfxUInt32Item*) pDocument->GetAttr(
                    aPos.Col(), aPos.Row(), aPos.Tab(), ATTR_VALIDDATA ))->GetValue();
            if ( nValidation )
            {
                const ScValidationData* pData = pDocument->GetValidationEntry( nValidation );
                if ( pData && !pData->IsDataValid( this, aPos ) )
                    pData->DoCalcError( this );
            }
        }

        // Reschedule verlangsamt das ganze erheblich, nur bei Prozentaenderung ausfuehren
        ScProgress::GetInterpretProgress()->SetStateCountDownOnPercent(
            pDocument->GetFormulaCodeInTree()/MIN_NO_CODES_PER_PROGRESS_UPDATE );

        switch (p->GetVolatileType())
        {
            case ScInterpreter::VOLATILE:
                // Volatile via built-in volatile functions.  No actions needed.
            break;
            case ScInterpreter::VOLATILE_MACRO:
                // The formula contains a volatile macro.
                pCode->SetRecalcModeAlways();
                pDocument->PutInFormulaTree(this);
                StartListeningTo(pDocument);
            break;
            case ScInterpreter::NOT_VOLATILE:
                if (pCode->IsRecalcModeAlways())
                {
                    // The formula was previously volatile, but no more.
                    EndListeningTo(pDocument);
                    pCode->SetRecalcModeNormal();
                }
                else
                {
                    // non-volatile formula.  End listening to the area in case
                    // it's listening due to macro module change.
                    pDocument->EndListeningArea(BCA_LISTEN_ALWAYS, this);
                }
                pDocument->RemoveFromFormulaTree(this);
            break;
            default:
                ;
        }
    }
    else
    {
        //  Zelle bei Compiler-Fehlern nicht ewig auf dirty stehenlassen
        DBG_ASSERT( pCode->GetCodeError(), "kein UPN-Code und kein Fehler ?!?!" );
        bDirty = FALSE;
        bTableOpDirty = FALSE;
    }
}


void ScFormulaCell::SetMatColsRows( SCCOL nCols, SCROW nRows )
{
    ScMatrixFormulaCellToken* pMat = aResult.GetMatrixFormulaCellTokenNonConst();
    if (pMat)
        pMat->SetMatColsRows( nCols, nRows);
    else if (nCols || nRows)
        aResult.SetToken( new ScMatrixFormulaCellToken( nCols, nRows));
}


void ScFormulaCell::GetMatColsRows( SCCOL & nCols, SCROW & nRows ) const
{
    const ScMatrixFormulaCellToken* pMat = aResult.GetMatrixFormulaCellToken();
    if (pMat)
        pMat->GetMatColsRows( nCols, nRows);
    else
    {
        nCols = 0;
        nRows = 0;
    }
}


ULONG ScFormulaCell::GetStandardFormat( SvNumberFormatter& rFormatter, ULONG nFormat ) const
{
    if ( nFormatIndex && (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
        return nFormatIndex;
    //! not ScFormulaCell::IsValue(), that could reinterpret the formula again.
    if ( aResult.IsValue() )
        return ScGlobal::GetStandardFormat( aResult.GetDouble(), rFormatter, nFormat, nFormatType );
    else
        return ScGlobal::GetStandardFormat( rFormatter, nFormat, nFormatType );
}


void ScFormulaCell::Notify( SvtBroadcaster&, const SfxHint& rHint)
{
    if ( !pDocument->IsInDtorClear() && !pDocument->GetHardRecalcState() )
    {
        const ScHint* p = PTR_CAST( ScHint, &rHint );
        ULONG nHint = (p ? p->GetId() : 0);
        if (nHint & (SC_HINT_DATACHANGED | SC_HINT_DYING | SC_HINT_TABLEOPDIRTY))
        {
            BOOL bForceTrack = FALSE;
            if ( nHint & SC_HINT_TABLEOPDIRTY )
            {
                bForceTrack = !bTableOpDirty;
                if ( !bTableOpDirty )
                {
                    pDocument->AddTableOpFormulaCell( this );
                    bTableOpDirty = TRUE;
                }
            }
            else
            {
                bForceTrack = !bDirty;
                SetDirtyVar();
            }
            // Don't remove from FormulaTree to put in FormulaTrack to
            // put in FormulaTree again and again, only if necessary.
            // Any other means except RECALCMODE_ALWAYS by which a cell could
            // be in FormulaTree if it would notify other cells through
            // FormulaTrack which weren't in FormulaTrack/FormulaTree before?!?
            // Yes. The new TableOpDirty made it necessary to have a
            // forced mode where formulas may still be in FormulaTree from
            // TableOpDirty but have to notify dependents for normal dirty.
            if ( (bForceTrack || !pDocument->IsInFormulaTree( this )
                    || pCode->IsRecalcModeAlways())
                    && !pDocument->IsInFormulaTrack( this ) )
                pDocument->AppendToFormulaTrack( this );
        }
    }
}

void ScFormulaCell::SetDirty()
{
    if ( !IsInChangeTrack() )
    {
        if ( pDocument->GetHardRecalcState() )
            SetDirtyVar();
        else
        {
            // Mehrfach-FormulaTracking in Load und in CompileAll
            // nach CopyScenario und CopyBlockFromClip vermeiden.
            // Wenn unbedingtes FormulaTracking noetig, vor SetDirty bDirty=FALSE
            // setzen, z.B. in CompileTokenArray
            if ( !bDirty || !pDocument->IsInFormulaTree( this ) )
            {
                SetDirtyVar();
                pDocument->AppendToFormulaTrack( this );
                pDocument->TrackFormulas();
            }
        }

        if (pDocument->IsStreamValid(aPos.Tab()))
            pDocument->SetStreamValid(aPos.Tab(), FALSE);
    }
}

void ScFormulaCell::SetDirtyVar()
{
    bDirty = TRUE;
    // mark the sheet of this cell to be calculated
    //#FIXME do we need to revert this remnant of old fake vba events? pDocument->AddCalculateTable( aPos.Tab() );
}

void ScFormulaCell::SetDirtyAfterLoad()
{
    bDirty = TRUE;
    if ( !pDocument->GetHardRecalcState() )
        pDocument->PutInFormulaTree( this );
}

void ScFormulaCell::SetTableOpDirty()
{
    if ( !IsInChangeTrack() )
    {
        if ( pDocument->GetHardRecalcState() )
            bTableOpDirty = TRUE;
        else
        {
            if ( !bTableOpDirty || !pDocument->IsInFormulaTree( this ) )
            {
                if ( !bTableOpDirty )
                {
                    pDocument->AddTableOpFormulaCell( this );
                    bTableOpDirty = TRUE;
                }
                pDocument->AppendToFormulaTrack( this );
                pDocument->TrackFormulas( SC_HINT_TABLEOPDIRTY );
            }
        }
    }
}


BOOL ScFormulaCell::IsDirtyOrInTableOpDirty() const
{
    return bDirty || (bTableOpDirty && pDocument->IsInInterpreterTableOp());
}


void ScFormulaCell::SetErrCode( USHORT n )
{
    /* FIXME: check the numerous places where ScTokenArray::GetCodeError() is
     * used whether it is solely for transport of a simple result error and get
     * rid of that abuse. */
    pCode->SetCodeError( n );
    // Hard set errors are transported as result type value per convention,
    // e.g. via clipboard. ScFormulaResult::IsValue() and
    // ScFormulaResult::GetDouble() handle that.
    aResult.SetResultError( n );
}

void ScFormulaCell::AddRecalcMode( ScRecalcMode nBits )
{
    if ( (nBits & RECALCMODE_EMASK) != RECALCMODE_NORMAL )
        bDirty = TRUE;
    if ( nBits & RECALCMODE_ONLOAD_ONCE )
    {   // OnLoadOnce nur zum Dirty setzen nach Filter-Import
        nBits = (nBits & ~RECALCMODE_EMASK) | RECALCMODE_NORMAL;
    }
    pCode->AddRecalcMode( nBits );
}

// Dynamically create the URLField on a mouse-over action on a hyperlink() cell.
void ScFormulaCell::GetURLResult( String& rURL, String& rCellText )
{
    String aCellString;

    Color* pColor;

    // Cell Text uses the Cell format while the URL uses
    // the default format for the type.
    ULONG nCellFormat = pDocument->GetNumberFormat( aPos );
    SvNumberFormatter* pFormatter = pDocument->GetFormatTable();

    if ( (nCellFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
        nCellFormat = GetStandardFormat( *pFormatter,nCellFormat );

   ULONG nURLFormat = ScGlobal::GetStandardFormat( *pFormatter,nCellFormat, NUMBERFORMAT_NUMBER);

    if ( IsValue() )
    {
        double fValue = GetValue();
        pFormatter->GetOutputString( fValue, nCellFormat, rCellText, &pColor );
    }
    else
    {
        GetString( aCellString );
        pFormatter->GetOutputString( aCellString, nCellFormat, rCellText, &pColor );
    }
    ScConstMatrixRef xMat( aResult.GetMatrix());
    if (xMat)
    {
        // determine if the matrix result is a string or value.
        ScMatrixValue nMatVal = xMat->Get(0, 1);
        if (!ScMatrix::IsValueType( nMatVal.nType))
            rURL = nMatVal.GetString();
        else
            pFormatter->GetOutputString( nMatVal.fVal, nURLFormat, rURL, &pColor );
    }

    if(!rURL.Len())
    {
        if(IsValue())
            pFormatter->GetOutputString( GetValue(), nURLFormat, rURL, &pColor );
        else
            pFormatter->GetOutputString( aCellString, nURLFormat, rURL, &pColor );
    }
}

bool ScFormulaCell::IsMultilineResult()
{
    if (!IsValue())
        return aResult.IsMultiline();
    return false;
}

void ScFormulaCell::MaybeInterpret()
{
    if (!IsDirtyOrInTableOpDirty())
        return;

    if (pDocument->GetAutoCalc() || (cMatrixFlag != MM_NONE))
        Interpret();
}

EditTextObject* ScFormulaCell::CreateURLObject()
{
    String aCellText;
    String aURL;
    GetURLResult( aURL, aCellText );

    SvxURLField aUrlField( aURL, aCellText, SVXURLFORMAT_APPDEFAULT);
    EditEngine& rEE = pDocument->GetEditEngine();
    rEE.SetText( EMPTY_STRING );
    rEE.QuickInsertField( SvxFieldItem( aUrlField, EE_FEATURE_FIELD ), ESelection( 0xFFFF, 0xFFFF ) );

    return rEE.CreateTextObject();
}

// ============================================================================

ScDetectiveRefIter::ScDetectiveRefIter( ScFormulaCell* pCell )
{
    pCode = pCell->GetCode();
    pCode->Reset();
    aPos = pCell->aPos;
}

BOOL lcl_ScDetectiveRefIter_SkipRef( ScToken* p )
{
    ScSingleRefData& rRef1 = p->GetSingleRef();
    if ( rRef1.IsColDeleted() || rRef1.IsRowDeleted() || rRef1.IsTabDeleted()
            || !rRef1.Valid() )
        return TRUE;
    if ( p->GetType() == svDoubleRef || p->GetType() == svExternalDoubleRef )
    {
        ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
        if ( rRef2.IsColDeleted() || rRef2.IsRowDeleted() || rRef2.IsTabDeleted()
                || !rRef2.Valid() )
            return TRUE;
    }
    return FALSE;
}

BOOL ScDetectiveRefIter::GetNextRef( ScRange& rRange )
{
    BOOL bRet = FALSE;
    ScToken* p = GetNextRefToken();
    if( p )
    {
        SingleDoubleRefProvider aProv( *p );
        rRange.aStart.Set( aProv.Ref1.nCol, aProv.Ref1.nRow, aProv.Ref1.nTab );
        rRange.aEnd.Set( aProv.Ref2.nCol, aProv.Ref2.nRow, aProv.Ref2.nTab );
        bRet = TRUE;
    }

    return bRet;
}

ScToken* ScDetectiveRefIter::GetNextRefToken()
{
    ScToken* p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
    if (p)
        p->CalcAbsIfRel( aPos );

    while ( p && lcl_ScDetectiveRefIter_SkipRef( p ) )
    {
        p = static_cast<ScToken*>(pCode->GetNextReferenceRPN());
        if (p)
            p->CalcAbsIfRel( aPos );
    }
    return p;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
