/*************************************************************************
 *
 *  $RCSfile: cell.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifdef MAC
    // StackSpace
#include <mac_start.h>
#include <Memory.h>
#include <fp.h>
#include <mac_end.h>
#endif

#ifdef SOLARIS
#include <ieeefp.h>
#elif ( defined ( LINUX ) && ( GLIBC < 2 ) )
#include <i386/ieeefp.h>
#endif

#include <svtools/zforlist.hxx>
#include <float.h>          // _finite

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
#include "bclist.hxx"

// jetzt fuer alle Systeme niedriger Wert, Rest wird per FormulaTree ausgebuegelt,
// falls wirklich eine tiefe Rekursion ist, geht das schneller, als zu versuchen
// und dann doch nicht zu schaffen..
#define SIMPLEMAXRECURSION
#ifdef SIMPLEMAXRECURSION
 #define MAXRECURSION 50
#else
#if defined( WIN ) || defined( OS2 )
 #define MAXRECURSION 50
#elif defined( MAC )
 // wird per StackSpace() ermittelt
#else
 #define MAXRECURSION 1000
#endif
#endif

// STATIC DATA -----------------------------------------------------------

#pragma code_seg("SCSTATICS")

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

#pragma code_seg()

INT8 ScFormulaCell::nIterMode = 0;

#ifndef PRODUCT
static const sal_Char __FAR_DATA msgDbgInfinity[] =
    "Formelzelle INFINITY ohne Err503 !!! (os/2?)\n"
    "NICHTS anruehren und ER bescheid sagen!";
#endif

// -----------------------------------------------------------------------

DECLARE_LIST (ScFormulaCellList, ScFormulaCell*);


ScBaseCell* ScBaseCell::Clone(ScDocument* pDoc) const
{
    switch (eCellType)
    {
        case CELLTYPE_VALUE:
            return new ScValueCell(*(const ScValueCell*)this);
        case CELLTYPE_STRING:
            return new ScStringCell(*(const ScStringCell*)this);
        case CELLTYPE_EDIT:
            return new ScEditCell(*(const ScEditCell*)this, pDoc);
        case CELLTYPE_FORMULA:
            return new ScFormulaCell(pDoc, ((ScFormulaCell*)this)->aPos,
                *(const ScFormulaCell*)this);
        case CELLTYPE_NOTE:
            return new ScNoteCell(*(const ScNoteCell*)this);
        default:
            DBG_ERROR("Unbekannter Zellentyp");
            return NULL;
    }
}

ScBaseCell::~ScBaseCell()
{
    delete pNote;
    delete pBroadcaster;
    DBG_ASSERT( eCellType == CELLTYPE_DESTROYED, "BaseCell Destructor" );
}

void ScBaseCell::Delete()
{
    DELETEZ(pNote);
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
            DBG_ERROR("Unbekannter Zellentyp");
            break;
    }
}

void ScBaseCell::SetNote( const ScPostIt& rNote )
{
    if (rNote.GetText().Len() > 0)
    {
        if (!pNote)
            pNote = new ScPostIt(rNote);
        else
            *pNote = rNote;
    }
    else
        DELETEZ(pNote);
}

BOOL ScBaseCell::GetNote( ScPostIt& rNote ) const
{
    if ( pNote )
        rNote = *pNote;
    else
        rNote.Clear();

    return ( pNote != NULL );
}

ScBaseCell* ScBaseCell::CreateTextCell( const String& rString, ScDocument* pDoc )
{
    if ( rString.Search('\n') != STRING_NOTFOUND || rString.Search(CHAR_CR) != STRING_NOTFOUND )
        return new ScEditCell( rString, pDoc );
    else
        return new ScStringCell( rString );
}

void ScBaseCell::LoadNote( SvStream& rStream )
{
    pNote = new ScPostIt;
    rStream >> *pNote;
}

void ScBaseCell::SetBroadcaster(ScBroadcasterList* pNew)
{
    delete pBroadcaster;
    pBroadcaster = pNew;
}

void ScBaseCell::StartListeningTo( ScDocument* pDoc, BOOL bOnlyRelNames )
{
    if ( eCellType == CELLTYPE_FORMULA && !pDoc->IsClipOrUndo()
            && !pDoc->GetNoListening()
            && !((ScFormulaCell*)this)->IsInChangeTrack()
        )
    {
        pDoc->SetDetectiveDirty(TRUE);  // es hat sich was geaendert...

        ScFormulaCell* pFormCell = (ScFormulaCell*)this;
        if( pFormCell->GetCode()->IsRecalcModeAlways() )
            pDoc->StartListeningArea( BCA_LISTEN_ALWAYS, pFormCell );
        else
        {
            ScTokenArray* pArr = pFormCell->GetCode();
            pArr->Reset();
            for( ScToken* t = pArr->GetNextReferenceRPN(); t;
                          t = pArr->GetNextReferenceRPN() )
            {
                if ( !bOnlyRelNames || (bOnlyRelNames
                    && (t->aRef.Ref1.IsRelName() || t->aRef.Ref2.IsRelName())))
                {
                    switch( t->GetType() )
                    {
                        case svSingleRef:
                            t->aRef.Ref1.CalcAbsIfRel( pFormCell->aPos );
                            if ( t->aRef.Ref1.Valid() )
                            {
                                pDoc->StartListeningCell(
                                    ScAddress( t->aRef.Ref1.nCol,
                                               t->aRef.Ref1.nRow,
                                               t->aRef.Ref1.nTab ), pFormCell );
                            }
                            break;
                        case svDoubleRef:
                            t->aRef.CalcAbsIfRel( pFormCell->aPos );
                            if ( t->aRef.Valid() )
                            {
                                if ( t->GetOpCode() == ocColRowNameAuto )
                                {   // automagically
                                    if ( t->aRef.Ref1.IsColRel() )
                                    {   // ColName
                                        pDoc->StartListeningArea( ScRange (
                                            0,
                                            t->aRef.Ref1.nRow,
                                            t->aRef.Ref1.nTab,
                                            MAXCOL,
                                            t->aRef.Ref2.nRow,
                                            t->aRef.Ref2.nTab ), pFormCell );
                                    }
                                    else
                                    {   // RowName
                                        pDoc->StartListeningArea( ScRange (
                                            t->aRef.Ref1.nCol,
                                            0,
                                            t->aRef.Ref1.nTab,
                                            t->aRef.Ref2.nCol,
                                            MAXROW,
                                            t->aRef.Ref2.nTab ), pFormCell );
                                    }
                                }
                                else
                                {
                                    pDoc->StartListeningArea( ScRange (
                                        t->aRef.Ref1.nCol,
                                        t->aRef.Ref1.nRow,
                                        t->aRef.Ref1.nTab,
                                        t->aRef.Ref2.nCol,
                                        t->aRef.Ref2.nRow,
                                        t->aRef.Ref2.nTab ), pFormCell );
                                }
                            }
                            break;
                    }
                }
            }
        }
    }
}

//  pArr gesetzt -> Referenzen von anderer Zelle nehmen
// dann muss auch aPos uebergeben werden!

void ScBaseCell::EndListeningTo( ScDocument* pDoc, BOOL bOnlyRelNames,
        ScTokenArray* pArr, ScAddress aPos )
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
            for( ScToken* t = pArr->GetNextReferenceRPN(); t;
                          t = pArr->GetNextReferenceRPN() )
            {
                if ( !bOnlyRelNames || (bOnlyRelNames
                    && (t->aRef.Ref1.IsRelName() || t->aRef.Ref2.IsRelName())))
                {
                    switch( t->GetType() )
                    {
                        case svSingleRef:
                            t->aRef.Ref1.CalcAbsIfRel( aPos );
                            if ( t->aRef.Ref1.Valid() )
                            {
                                pDoc->EndListeningCell(
                                    ScAddress( t->aRef.Ref1.nCol,
                                               t->aRef.Ref1.nRow,
                                               t->aRef.Ref1.nTab ), pFormCell );
                            }
                            break;
                        case svDoubleRef:
                            t->aRef.CalcAbsIfRel( aPos );
                            if ( t->aRef.Valid() )
                            {
                                if ( t->GetOpCode() == ocColRowNameAuto )
                                {   // automagically
                                    if ( t->aRef.Ref1.IsColRel() )
                                    {   // ColName
                                        pDoc->EndListeningArea( ScRange (
                                            0,
                                            t->aRef.Ref1.nRow,
                                            t->aRef.Ref1.nTab,
                                            MAXCOL,
                                            t->aRef.Ref2.nRow,
                                            t->aRef.Ref2.nTab ), pFormCell );
                                    }
                                    else
                                    {   // RowName
                                        pDoc->EndListeningArea( ScRange (
                                            t->aRef.Ref1.nCol,
                                            0,
                                            t->aRef.Ref1.nTab,
                                            t->aRef.Ref2.nCol,
                                            MAXROW,
                                            t->aRef.Ref2.nTab ), pFormCell );
                                    }
                                }
                                else
                                {
                                    pDoc->EndListeningArea( ScRange (
                                        t->aRef.Ref1.nCol,
                                        t->aRef.Ref1.nRow,
                                        t->aRef.Ref1.nTab,
                                        t->aRef.Ref2.nCol,
                                        t->aRef.Ref2.nRow,
                                        t->aRef.Ref2.nTab ), pFormCell );
                                }
                            }
                            break;
                    }
                }
            }
        }
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

//  static
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
                    ScToken** ppToken1 = pCode1->GetArray();
                    ScToken** ppToken2 = pCode2->GetArray();
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
            DBG_ERROR("huch, was fuer Zellen???");
    }
    return FALSE;
}

//-----------------------------------------------------------------------------------

//
//      ScFormulaCell
//

ScFormulaCell::ScFormulaCell() :
    ScBaseCell( CELLTYPE_FORMULA ),
    pCode( NULL ),
    nErgValue( 0.0 ),
    bIsValue( TRUE ),
    bDirty( FALSE ),
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    pDocument( NULL ),
    nFormatType( NUMBERFORMAT_NUMBER ),
    nFormatIndex(0),
    cMatrixFlag ( MM_NONE ),
    pMatrix     ( NULL ),
    bIsIterCell (FALSE),
    bInChangeTrack( FALSE ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    aPos(0),
    nMatCols(0),
    nMatRows(0)
{
}

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const String& rFormula, BYTE cMatInd ) :
    ScBaseCell( CELLTYPE_FORMULA ),
    aPos( rPos ),
    pCode( NULL ),
    nErgValue( 0.0 ),
    bIsValue( TRUE ),
    bDirty( TRUE ), // -> wg. Benutzung im Fkt.AutoPiloten, war: cMatInd != 0
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    pDocument( pDoc ),
    nFormatType( NUMBERFORMAT_NUMBER ),
    nFormatIndex(0),
    cMatrixFlag ( cMatInd ),
    pMatrix( NULL ),
    bIsIterCell (FALSE),
    bInChangeTrack( FALSE ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nMatCols(0),
    nMatRows(0)
{
    Compile( rFormula, TRUE );  // bNoListening, erledigt Insert
}

// Wird von den Importfiltern verwendet

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              const ScTokenArray* pArr, BYTE cInd ) :
    ScBaseCell( CELLTYPE_FORMULA ),
    aPos( rPos ),
    pCode( pArr ? new ScTokenArray( *pArr ) : new ScTokenArray ),
    nErgValue( 0.0 ),
    bIsValue( TRUE ),
    bDirty( NULL != pArr ), // -> wg. Benutzung im Fkt.AutoPiloten, war: cInd != 0
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    pDocument( pDoc ),
    nFormatType( NUMBERFORMAT_NUMBER ),
    nFormatIndex(0),
    cMatrixFlag ( cInd ),
    pMatrix ( NULL ),
    bIsIterCell (FALSE),
    bInChangeTrack( FALSE ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nMatCols(0),
    nMatRows(0)
{
    // UPN-Array erzeugen
    if( pCode->GetLen() && !pCode->GetError() && !pCode->GetCodeLen() )
    {
        ScCompiler aComp(pDocument, aPos, *pCode);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
    }
    else
    {
        pCode->Reset();
        if ( pCode->GetNextOpCodeRPN( ocSubTotal ) )
            bSubTotal = TRUE;
    }
}

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rNewPos,
                              const ScFormulaCell& rScFormulaCell ) :
    ScBaseCell( rScFormulaCell ),
    SfxListener(),
    aErgString( rScFormulaCell.aErgString ),
    nErgValue( rScFormulaCell.nErgValue ),
    bIsValue( rScFormulaCell.bIsValue ),
    bDirty( rScFormulaCell.bDirty ),
    bChanged( rScFormulaCell.bChanged ),
    bRunning( rScFormulaCell.bRunning ),
    bCompile( rScFormulaCell.bCompile ),
    bSubTotal( rScFormulaCell.bSubTotal ),
    pDocument( pDoc ),
    nFormatType( rScFormulaCell.nFormatType ),
    nFormatIndex( pDoc == rScFormulaCell.pDocument ? rScFormulaCell.nFormatIndex : 0 ),
    cMatrixFlag ( rScFormulaCell.cMatrixFlag ),
    bIsIterCell (FALSE),
    bInChangeTrack( FALSE ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    aPos( rNewPos ),
    nMatCols( rScFormulaCell.nMatCols ),
    nMatRows( rScFormulaCell.nMatRows )
{
    if (rScFormulaCell.pMatrix)
        pMatrix = rScFormulaCell.pMatrix->Clone();
    else
        pMatrix = NULL;
    pCode = rScFormulaCell.pCode->Clone();
    // evtl. Fehler zuruecksetzen und neu kompilieren
    //  nicht im Clipboard - da muss das Fehlerflag erhalten bleiben
    //  Spezialfall Laenge=0: als Fehlerzelle erzeugt, dann auch Fehler behalten
    if ( pCode->GetError() && !pDocument->IsClipboard() && pCode->GetLen() )
    {
        pCode->SetError( 0 );
        bCompile = TRUE;
    }
    // Hat der Code Name-Referenzen mit Referenzen?
    BOOL bCompileLater = FALSE; //! bei ColRowNames und URM_MOVE nach UpdateReference
    BOOL bCutMode = (rScFormulaCell.pDocument->IsClipboard()
        && rScFormulaCell.pDocument->IsCutMode());
    if( !bCompile )
    {
        for( ScToken* t = pCode->GetNextReferenceOrName(); t && !bCompile;
                      t = pCode->GetNextReferenceOrName() )
        {
            if ( t->GetType() == svIndex )
            {
                ScRangeData* pRangeData = pDoc->GetRangeName()->FindIndex( t->nIndex );
                if( pRangeData )
                {
                    if( pRangeData->HasReferences() )
                        bCompile = TRUE;
                }
                else
                    bCompile = TRUE;    // ungueltige Referenz!
            }
            else if ( t->GetOpCode() == ocColRowName )
            {
                bCompile = TRUE;        // neuer LookUp noetig
                bCompileLater = bCutMode;
            }
        }
    }
    if( bCompile )
    {
        if ( !bCompileLater && bCutMode )
        {
            pCode->Reset();
            bCompileLater = (pCode->GetNextColRowName() != NULL);
        }
        if ( !bCompileLater )
        {
            // bNoListening, bei in Clip/Undo sowieso nicht,
            // bei aus Clip auch nicht, sondern nach Insert(Clone) und UpdateReference
            CompileTokenArray( TRUE );
        }
    }
}

// +---+---+---+---+---+---+---+---+
// |           |Str|Num|Dir|cMatrix|
// +---+---+---+---+---+---+---+---+

ScFormulaCell::ScFormulaCell( ScDocument* pDoc, const ScAddress& rPos,
                              SvStream& rStream, ScMultipleReadHeader& rHdr ) :
    ScBaseCell( CELLTYPE_FORMULA ),
    aPos( rPos ),
    pCode( new ScTokenArray ),
    nErgValue( 0.0 ),
    bIsValue( TRUE ),
    bDirty( FALSE ),
    bChanged( FALSE ),
    bRunning( FALSE ),
    bCompile( FALSE ),
    bSubTotal( FALSE ),
    pDocument( pDoc ),
    nFormatType( 0 ),
    nFormatIndex(0),
    pMatrix ( NULL ),
    bIsIterCell (FALSE),
    bInChangeTrack( FALSE ),
    pPrevious(0),
    pNext(0),
    pPreviousTrack(0),
    pNextTrack(0),
    nMatCols(0),
    nMatRows(0)
{
//  ScReadHeader aHdr( rStream );
    rHdr.StartEntry();

    USHORT nVer = (USHORT) pDoc->GetSrcVersion();

    if( nVer >= SC_NUMFMT )
    {
        BYTE cData;
        rStream >> cData;
#ifndef PRODUCT
        static BOOL bShown = 0;
        if ( !bShown && SOFFICE_FILEFORMAT_NOW > SOFFICE_FILEFORMAT_50 )
        {
            bShown = 1;
            DBG_ERRORFILE( "bei inkompatiblem FileFormat den FormatIndex umheben!" );
        }
#endif
        if( cData & 0x0F )
        {
            BYTE nSkip = cData & 0x0F;
            if ( (cData & 0x10) && nSkip >= sizeof(UINT32) )
            {
                UINT32 n;
                rStream >> n;
                nFormatIndex = n;
                nSkip -= sizeof(UINT32);
            }
            if ( nSkip )
                rStream.SeekRel( nSkip );
        }
        BYTE cFlags;
        rStream >> cFlags >> nFormatType;
        cMatrixFlag = (BYTE) ( cFlags & 0x03 );
        bDirty = BOOL( ( cFlags & 0x04 ) != 0 );
        if( cFlags & 0x08 )
            rStream >> nErgValue;
        if( cFlags & 0x10 )
        {
            rStream.ReadByteString( aErgString, rStream.GetStreamCharSet() );
            bIsValue = FALSE;
        }
        pCode->Load( rStream, nVer, aPos );
        if ( (cFlags & 0x18) == 0 )
            bDirty = TRUE;      // #67161# no result stored => recalc
        if( cFlags & 0x20 )
            bSubTotal = TRUE;
        else if ( nVer < SC_SUBTOTAL_BUGFIX )
        {   // #65285# in alten Dokumenten war Flag nicht gesetzt, wenn Formel
            // manuell eingegeben wurde (nicht via Daten->Teilergebnisse)
            if ( pCode->HasOpCodeRPN( ocSubTotal ) )
            {
                bDirty = TRUE;      // neu berechnen
                bSubTotal = TRUE;
            }
        }
        if ( cMatrixFlag == MM_FORMULA && rHdr.BytesLeft() )
            rStream >> nMatCols >> nMatRows;
    }
    else
    {
        UINT16 nCodeLen;
        if( pDoc->GetSrcVersion() >= SC_FORMULA_LCLVER )
            rStream.SeekRel( 2 );
        rStream >> cMatrixFlag >> nCodeLen;
        if( cMatrixFlag == 5 )
            cMatrixFlag = 0;
        cMatrixFlag &= 3;
        if( nCodeLen )
            pCode->Load30( rStream, aPos );
        // Wir koennen hier bei Calc 3.0-Docs noch kein UPN-Array
        // erzeugen, da die Named Ranges noch nicht eingelesen sind
    }

    rHdr.EndEntry();

    //  after loading, it must be known if ocMacro is in any formula
    //  (for macro warning, and to insert the hidden view)
    if ( !pDoc->GetHasMacroFunc() && pCode->HasOpCodeRPN( ocMacro ) )
        pDoc->SetHasMacroFunc( TRUE );
}

BOOL lcl_IsBeyond( ScTokenArray* pCode, USHORT nMaxRow )
{
    ScToken* t;
    pCode->Reset();
    while ( t = pCode->GetNextReferenceRPN() )  // RPN -> auch in Namen
        if ( t->aRef.Ref1.nRow > nMaxRow || t->aRef.Ref2.nRow > nMaxRow )
            return TRUE;
    return FALSE;
}

void ScFormulaCell::Save( SvStream& rStream, ScMultipleWriteHeader& rHdr ) const
{
    USHORT nSaveMaxRow = pDocument->GetSrcMaxRow();
    if ( nSaveMaxRow < MAXROW && lcl_IsBeyond( pCode, nSaveMaxRow ) )
    {
        //  Zelle mit Ref-Error erzeugen und speichern
        //  StartEntry/EndEntry passiert beim Speichern der neuen Zelle

        SingleRefData aRef;
        aRef.InitAddress(ScAddress());
        aRef.SetColRel(TRUE);
        aRef.SetColDeleted(TRUE);
        aRef.SetRowRel(TRUE);
        aRef.SetRowDeleted(TRUE);
        aRef.CalcRelFromAbs(aPos);
        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);
        aArr.AddOpCode(ocStop);
        ScFormulaCell* pErrCell = new ScFormulaCell( pDocument, aPos, &aArr );
        pErrCell->Save( rStream, rHdr );
        delete pErrCell;

        pDocument->SetLostData();           // Warnung ausgeben
        return;
    }

    rHdr.StartEntry();

    if ( bIsValue && !pCode->GetError() && !SOMA_FINITE( nErgValue ) )
    {
        DBG_ERRORFILE( msgDbgInfinity );
        pCode->SetError( errIllegalFPOperation );
    }
    BYTE cFlags = cMatrixFlag & 0x03;
    if( bDirty )
        cFlags |= 0x04;
    // Daten speichern?
    if( pCode->IsRecalcModeNormal() && !pCode->GetError() )
        cFlags |= bIsValue ? 0x08 : 0x10;
    if ( bSubTotal )
        cFlags |= 0x20;
#ifndef PRODUCT
    static BOOL bShown = 0;
    if ( !bShown && rStream.GetVersion() > SOFFICE_FILEFORMAT_50 )
    {
        bShown = 1;
        DBG_ERRORFILE( "bei inkompatiblem FileFormat den FormatIndex umheben!" );
    }
//  rStream << (BYTE) 0x00;
#endif
    if ( nFormatIndex )
        rStream << (BYTE) (0x10 | sizeof(UINT32)) << nFormatIndex;
    else
        rStream << (BYTE) 0x00;
    rStream << cFlags << (UINT16) nFormatType;
    if( cFlags & 0x08 )
        rStream << nErgValue;
    if( cFlags & 0x10 )
        rStream.WriteByteString( aErgString, rStream.GetStreamCharSet() );
    pCode->Store( rStream, aPos );
    if ( cMatrixFlag == MM_FORMULA )
        rStream << nMatCols << nMatRows;

    rHdr.EndEntry();
}

ScBaseCell* ScFormulaCell::Clone( ScDocument* pDoc, const ScAddress& rPos,
        BOOL bNoListening ) const
{
    ScFormulaCell* pCell = new ScFormulaCell( pDoc, rPos, *this );
    if ( !bNoListening )
        pCell->StartListeningTo( pDoc );
    return pCell;
}

void ScFormulaCell::GetFormula( String& rFormula ) const
{
    if( pCode->GetError() && !pCode->GetLen() )
    {
        rFormula = ScGlobal::GetErrorString( pCode->GetError() ); return;
    }
    else if( cMatrixFlag == MM_REFERENCE )
    {
        // Referenz auf eine andere Zelle, die eine Matrixformel enthaelt
        pCode->Reset();
        ScToken* p = pCode->GetNextReferenceRPN();
        if( p )
        {
            ScBaseCell* pCell;
            p->aRef.Ref1.CalcAbsIfRel( aPos );
            if ( p->aRef.Ref1.Valid() )
                pCell = pDocument->GetCell( ScAddress( p->aRef.Ref1.nCol,
                    p->aRef.Ref1.nRow, p->aRef.Ref1.nTab ) );
            else
                pCell = NULL;
            if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                ((ScFormulaCell*)pCell)->GetFormula(rFormula);
                return;
            }
            else
            {
                ScCompiler aComp( pDocument, aPos, *pCode );
                aComp.CreateStringFromTokenArray( rFormula );
            }
        }
        else
        {
            DBG_ERROR("ScFormulaCell::GetFormula: Keine Matrix");
        }
    }
    else
    {
        ScCompiler aComp( pDocument, aPos, *pCode );
        aComp.CreateStringFromTokenArray( rFormula );
    }

    rFormula.Insert( '=',0 );
    if( cMatrixFlag )
    {
        rFormula.Insert('{', 0);
        rFormula += '}';
    }
}

void ScFormulaCell::GetResultDimensions( USHORT& rCols, USHORT& rRows )
{
    if (bDirty && pDocument->GetAutoCalc())
        Interpret();

    if ( !pCode->GetError() && pMatrix )
        pMatrix->GetDimensions( rCols, rRows );
    else
        rCols = rRows = 0;
}

void ScFormulaCell::Compile( const String& rFormula, BOOL bNoListening )
{
    if ( pDocument->IsClipOrUndo() ) return;
    BOOL bWasInFormulaTree = pDocument->IsInFormulaTree( this );
    if ( bWasInFormulaTree )
        pDocument->RemoveFromFormulaTree( this );
    // pCode darf fuer Abfragen noch nicht geloescht, muss aber leer sein
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    ScCompiler aComp(pDocument, aPos);
    if ( pDocument->IsImportingXML() )
        aComp.SetCompileEnglish( TRUE );
    pCode = aComp.CompileString( rFormula );
    if ( pCodeOld )
        delete pCodeOld;
    if( !pCode->GetError() )
    {
        if ( !pCode->GetLen() && aErgString.Len() && rFormula == aErgString )
        {   // #65994# nicht rekursiv CompileTokenArray/Compile/CompileTokenArray
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
    }
    if ( bWasInFormulaTree )
        pDocument->PutInFormulaTree( this );
}


void ScFormulaCell::CompileTokenArray( BOOL bNoListening )
{
    // Noch nicht compiliert?
    if( !pCode->GetLen() && aErgString.Len() )
        Compile( aErgString );
    else if( bCompile && !pDocument->IsClipOrUndo() && !pCode->GetError() )
    {
        // RPN-Laenge kann sich aendern
        BOOL bWasInFormulaTree = pDocument->IsInFormulaTree( this );
        if ( bWasInFormulaTree )
            pDocument->RemoveFromFormulaTree( this );

        // Laden aus Filter? Dann noch nix machen!
        if( pDocument->IsInsertingFromOtherDoc() )
            bNoListening = TRUE;

        if( !bNoListening && pCode->GetCodeLen() )
            EndListeningTo( pDocument );
        ScCompiler aComp(pDocument, aPos, *pCode );
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetError() )
        {
            nFormatType = aComp.GetNumFormatType();
            nFormatIndex = 0;
            bChanged = TRUE;
            nErgValue = 0.0;
            aErgString.Erase();
            bCompile = FALSE;
            if ( !bNoListening )
                StartListeningTo( pDocument );
        }
        if ( bWasInFormulaTree )
            pDocument->PutInFormulaTree( this );
    }
}


void ScFormulaCell::CompileXML()
{
    if ( cMatrixFlag == MM_REFERENCE )
        return ;

    ScCompiler aComp( pDocument, aPos, *pCode );
    aComp.SetCompileEnglish( TRUE );
    aComp.SetImportXML( TRUE );
    String aFormula;
    aComp.CreateStringFromTokenArray( aFormula );
    // pCode darf fuer Abfragen noch nicht geloescht, muss aber leer sein
    if ( pCode )
        pCode->Clear();
    ScTokenArray* pCodeOld = pCode;
    pCode = aComp.CompileString( aFormula );
    delete pCodeOld;
    if( !pCode->GetError() )
    {
        if ( !pCode->GetLen() )
        {
            if ( aFormula.GetChar(0) == '=' )
                pCode->AddBad( aFormula.GetBuffer() + 1 );
            else
                pCode->AddBad( aFormula.GetBuffer() );
        }
        bSubTotal = aComp.CompileTokenArray();
        if( !pCode->GetError() )
        {
            nFormatType = aComp.GetNumFormatType();
            nFormatIndex = 0;
            bChanged = TRUE;
            bCompile = FALSE;
            StartListeningTo( pDocument );
        }
    }
    else
    {
        bChanged = TRUE;
        SetTextWidth( TEXTWIDTH_DIRTY );
    }
}


void ScFormulaCell::CalcAfterLoad()
{
    BOOL bNewCompiled = FALSE;
    // Falls ein Calc 1.0-Doc eingelesen wird, haben wir ein Ergebnis,
    // aber kein TokenArray
    if( !pCode->GetLen() && aErgString.Len() )
    {
        Compile( aErgString, TRUE );
        aErgString.Erase();
        bDirty = TRUE;
        bNewCompiled = TRUE;
    }
    // Das UPN-Array wird nicht erzeugt, wenn ein Calc 3.0-Doc eingelesen
    // wurde, da die RangeNames erst jetzt existieren.
    if( pCode->GetLen() && !pCode->GetCodeLen() && !pCode->GetError() )
    {
        ScCompiler aComp(pDocument, aPos, *pCode);
        bSubTotal = aComp.CompileTokenArray();
        nFormatType = aComp.GetNumFormatType();
        nFormatIndex = 0;
        bDirty = TRUE;
        bCompile = FALSE;
        bNewCompiled = TRUE;
    }
    // irgendwie koennen unter os/2 mit rotter FPU-Exception /0 ohne Err503
    // gespeichert werden, woraufhin spaeter im NumberFormatter die BLC Lib
    // bei einem fabs(-NAN) abstuerzt (#32739#)
    // hier fuer alle Systeme ausbuegeln, damit da auch Err503 steht
    if ( bIsValue && !SOMA_FINITE( nErgValue ) )
    {
        DBG_ERRORFILE("Formelzelle INFINITY !!! Woher kommt das Dokument?");
        nErgValue = 0.0;
        pCode->SetError( errIllegalFPOperation );
        bDirty = TRUE;
    }
    // DoubleRefs bei binaeren Operatoren waren vor v5.0 immer Matrix,
    // jetzt nur noch wenn in Matrixformel, sonst implizite Schnittmenge
    if ( pDocument->GetSrcVersion() < SC_MATRIX_DOUBLEREF &&
            GetMatrixFlag() == MM_NONE && pCode->HasMatrixDoubleRefOps() )
    {
        cMatrixFlag = MM_FORMULA;
        nMatCols = 1;
        nMatRows = 1;
    }
    // Muss die Zelle berechnet werden?
    // Nach Load koennen Zellen einen Fehlercode enthalten, auch dann
    // Listener starten und ggbf. neu berechnen wenn nicht RECALCMODE_NORMAL
    if( !bNewCompiled || !pCode->GetError() )
    {
        StartListeningTo( pDocument );
        if( !pCode->IsRecalcModeNormal() )
            bDirty = TRUE;
    }
    if ( pCode->GetError() == errInterpOverflow )
    {   // versuchen Err527 wegzubuegeln
        bDirty = TRUE;
    }
    else if ( pCode->IsRecalcModeAlways() )
    {   // zufall(), heute(), jetzt() bleiben immer im FormulaTree, damit sie
        // auch bei jedem F9 berechnet werden.
        bDirty = TRUE;
    }
    // Noch kein SetDirty weil noch nicht alle Listener bekannt, erst in
    // SetDirtyAfterLoad.
}

void ScFormulaCell::Interpret()
{
    static USHORT nRecCount = 0;
    static ScFormulaCell* pLastIterInterpreted = NULL;
    if ( !bDirty )
        return;         // fuer IterCircRef, nix doppelt

    //! HACK:
    //  Wenn der Aufruf aus einem Reschedule im DdeLink-Update kommt, dirty stehenlassen
    //  Besser: Dde-Link Update ohne Reschedule oder ganz asynchron !!!

    if ( ScDdeLink::IsInUpdate() )
        return;

    if (bRunning)
    {
        // Keine Iterierung?
        if (!pDocument->GetDocOptions().IsIter())
            pCode->SetError( errCircularReference );
        else
        {
            if (pCode->GetError() == errCircularReference)
                pCode->SetError( 0 );
            nIterMode = 1;
            bIsIterCell = TRUE;
            pLastIterInterpreted = NULL;
        }
        return;
    }
    // #63038# fuer GetErrCode, IsValue, GetValue nicht mehrfach interpretieren
    if ( nIterMode && pLastIterInterpreted == this )
        return ;

    if( !pCode->GetCodeLen() && !pCode->GetError() )
        CompileTokenArray();

    if( pCode->GetCodeLen() && pDocument )
    {
#if defined(MAC) && !defined(SIMPLEMAXRECURSION)
        if( StackSpace() < 2048 )   // 2K Stack noch uebriglassen
#else
        if( nRecCount > MAXRECURSION )
#endif
        {
            pCode->SetError( errInterpOverflow );
            // Zelle bleibt in FormulaTree, naechstes Mal sind evtl.
            // Vorgaenger bereits berechnet worden bzw. von der View wird
            // via ScCellFormat::GetString CalcFormulaTree angeworfen
            bDirty = FALSE;
            nErgValue = 0.0;
            bIsValue = TRUE;
            nIterMode = 0;
            bIsIterCell = FALSE;
            pLastIterInterpreted = NULL;
            bChanged = TRUE;
            SetTextWidth( TEXTWIDTH_DIRTY );
            return;
        }
        nRecCount++;
        pDocument->IncInterpretLevel();
        ScInterpreter* p = new ScInterpreter( this, pDocument, aPos, *pCode );
        USHORT nOldErrCode = pCode->GetError();
        USHORT nIterCount = 0;
        if ( nIterMode == 0 )
        {   // nur beim ersten Mal
// wenn neu kompilierte Zelle 0.0 ergibt wird kein Changed erkannt
// und die Zelle wird nicht sofort repainted!
//          bChanged = FALSE;
            if ( nOldErrCode == errNoConvergence
              && pDocument->GetDocOptions().IsIter() )
                pCode->SetError( 0 );
        }
        BOOL bRepeat = TRUE;
        while( bRepeat )
        {
            if ( pMatrix )
            {
                delete pMatrix;
                pMatrix = NULL;
            }

            switch ( pCode->GetError() )
            {
                case errCircularReference :     // wird neu festgestellt
                case errInterpOverflow :        // Err527 eine Chance geben
                    pCode->SetError( 0 );
                break;
            }

            bRunning = TRUE;
            p->Interpret();
            bRunning = FALSE;
            if( pCode->GetError()
             && pCode->GetError() != errCircularReference )
            {
                bDirty = FALSE;
                nIterMode = 0;
                bIsIterCell = FALSE;
                pLastIterInterpreted = NULL;
                bChanged = TRUE;
                bIsValue = TRUE;
                break;
            }
            if( nIterMode == 1 && bIsIterCell )
            {
                pLastIterInterpreted = NULL;
                ++nIterCount;
                // schoen konvergiert?
                if( (p->GetResultType() == svDouble
                  && fabs( p->GetNumResult() - nErgValue ) <=
                     pDocument->GetDocOptions().GetIterEps())
                    )
                {
                    nIterMode = 0;
                    bIsIterCell = FALSE;
                    bDirty = FALSE;
                    bRepeat = FALSE;
                }
                // Zu oft rumgelaufen?
                else if( nIterCount >= pDocument->GetDocOptions().GetIterCount() )
                {
                    nIterMode = 0;
                    bIsIterCell = FALSE;
                    bDirty = FALSE;
                    bRepeat = FALSE;
                    pCode->SetError( errNoConvergence );
                }
                if ( p->GetResultType() == svDouble )
                {
                    if( !bIsValue || nErgValue != p->GetNumResult() )
                        bChanged = TRUE;
                    bIsValue = TRUE;
                    nErgValue = p->GetNumResult();
                }
            }
            else
                bRepeat = FALSE;
        }

        switch( p->GetResultType() )
        {
            case svDouble:
                if( nErgValue != p->GetNumResult() || !bIsValue )
                {
                    bChanged = TRUE;
                    bIsValue = TRUE;
                    nErgValue = p->GetNumResult();
                }
            break;
            case svString:
                if( aErgString != p->GetStringResult() || bIsValue )
                {
                    bChanged = TRUE;
                    bIsValue = FALSE;
                    aErgString = p->GetStringResult();
                }
            break;
        }

        // Neuer Fehlercode?
        if( !bChanged && pCode->GetError() != nOldErrCode )
            bChanged = TRUE;
        // Anderes Zahlenformat?
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
        // Genauigkeit wie angezeigt?
        if ( bIsValue && !pCode->GetError()
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
            nErgValue = pDocument->RoundValueAsShown( nErgValue, nFormat );
        }
        if ( nIterMode == 0 )
            bDirty = FALSE;
        else
            pLastIterInterpreted = this;
        pMatrix = p->GetMatrixResult();
        if( pMatrix )
        {
#if 0
//! MatrixFormel immer changed?!?
// ist bei MD's Rundumschlag von r1.167 --> r1.168 reingekommen
// => ewiges Repaint von MatrixFormel, besonders bei DDE laestig
// ab r1.260 (sv369b) probieren wir's mal ohne..
            if( cMatrixFlag == MM_FORMULA )
                bChanged = TRUE;
            else
#else
            if( cMatrixFlag != MM_FORMULA )
#endif
            {   // mit linker oberer Ecke weiterleben
                delete pMatrix;
                pMatrix = NULL;
            }
        }
        if( bChanged )
            SetTextWidth( TEXTWIDTH_DIRTY );
        delete p;
        nRecCount--;
        pDocument->DecInterpretLevel();
        if ( pCode->GetError() != errInterpOverflow
          && !pCode->IsRecalcModeAlways() )
            pDocument->RemoveFromFormulaTree( this );
#ifndef PRODUCT
        if ( bIsValue && !pCode->GetError() && !SOMA_FINITE( nErgValue ) )
        {
            DBG_ERRORFILE( msgDbgInfinity );
            nErgValue = 0.0;
            pCode->SetError( errIllegalFPOperation );
        }
#endif

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
            pDocument->GetFormulaCodeInTree() );
    }
    else
    {
        //  Zelle bei Compiler-Fehlern nicht ewig auf dirty stehenlassen
        DBG_ASSERT( pCode->GetError(), "kein UPN-Code und kein Fehler ?!?!" );
        bDirty = FALSE;
    }
}

ULONG ScFormulaCell::GetStandardFormat( SvNumberFormatter& rFormatter, ULONG nFormat ) const
{
    if ( nFormatIndex && (nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
        return nFormatIndex;
    if ( bIsValue )     //! nicht IsValue()
        return ScGlobal::GetStandardFormat( nErgValue, rFormatter, nFormat, nFormatType );
    else
        return ScGlobal::GetStandardFormat( rFormatter, nFormat, nFormatType );
}


void __EXPORT ScFormulaCell::SFX_NOTIFY( SfxBroadcaster& rBC,
        const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType )
{
    if ( !pDocument->IsInDtorClear() && !pDocument->GetHardRecalcState() )
    {
        const ScHint* p = PTR_CAST( ScHint, &rHint );
        if( p && (p->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING)) )
        {
            bDirty = TRUE;
            // #35962# nicht ewig aus FormulaTree rausholen um in FormulaTrack
            // zu stellen und wieder in FormulaTree reinzupacken..
            // kann ausser bei RECALCMODE_ALWAYS jemals eine Zelle im
            // FormulaTree sein, wenn sie durch FormulaTrack eine Zelle
            // anstossen wuerde, die nicht in FormulaTrack/FormulaTree ist?!?
            if ( (!pDocument->IsInFormulaTree( this )
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
            bDirty = TRUE;
        else
        {
            // Mehrfach-FormulaTracking in Load und in CompileAll
            // nach CopyScenario und CopyBlockFromClip vermeiden.
            // Wenn unbedingtes FormulaTracking noetig, vor SetDirty bDirty=FALSE
            // setzen, z.B. in CompileTokenArray
            if ( !bDirty || !pDocument->IsInFormulaTree( this ) )
            {
                bDirty = TRUE;
                pDocument->AppendToFormulaTrack( this );
                pDocument->TrackFormulas();
            }
        }
    }
}

void ScFormulaCell::SetErrCode( USHORT n )
{
    pCode->SetError( n );
    bIsValue = FALSE;
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

//------------------------------------------------------------------------

ScDetectiveRefIter::ScDetectiveRefIter( ScFormulaCell* pCell )
{
    pCode = pCell->GetCode();
    pCode->Reset();
    aPos = pCell->aPos;
}

BOOL ScDetectiveRefIter::GetNextRef( ScTripel& rStart, ScTripel& rEnd )
{
    BOOL bRet = FALSE;

    ScToken* p = pCode->GetNextReferenceRPN();
    if (p)
        p->aRef.CalcAbsIfRel( aPos );

    while ( p && ( p->aRef.Ref1.IsColDeleted() || p->aRef.Ref2.IsColDeleted() ||
                   p->aRef.Ref1.IsRowDeleted() || p->aRef.Ref2.IsRowDeleted() ||
                   p->aRef.Ref1.IsTabDeleted() || p->aRef.Ref2.IsTabDeleted() ||
                   !p->aRef.Ref1.Valid() || !p->aRef.Ref2.Valid() ) )
    {
        p = pCode->GetNextReferenceRPN();
        if (p)
            p->aRef.CalcAbsIfRel( aPos );
    }

    if( p )
    {
        rStart.Put( p->aRef.Ref1.nCol,
                    p->aRef.Ref1.nRow,
                    p->aRef.Ref1.nTab );
        rEnd.Put( p->aRef.Ref2.nCol,
                  p->aRef.Ref2.nRow,
                  p->aRef.Ref2.nTab );
        bRet = TRUE;
    }

    return bRet;
}

//-----------------------------------------------------------------------------------

ScFormulaCell::~ScFormulaCell()
{
    pDocument->RemoveFromFormulaTree( this );
    delete pCode;
    delete pMatrix;
    pMatrix = NULL;
#ifdef DBG_UTIL
    eCellType = CELLTYPE_DESTROYED;
#endif
}


#ifdef DBG_UTIL

ScStringCell::~ScStringCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif
                                    //! ValueCell auch nur bei DBG_UTIL,
                                    //! auch in cell.hxx aendern !!!!!!!!!!!!!!!!!!!!

ScValueCell::~ScValueCell()
{
    eCellType = CELLTYPE_DESTROYED;
}

#ifdef DBG_UTIL

ScNoteCell::~ScNoteCell()
{
    eCellType = CELLTYPE_DESTROYED;
}
#endif




