/*************************************************************************
 *
 *  $RCSfile: refupdat.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
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

#include <tools/debug.hxx>

#include "refupdat.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "bigrange.hxx"
#include "chgtrack.hxx"

//------------------------------------------------------------------------

BOOL lcl_MoveStart( short& rRef, short nStart, short nDelta, short nMask )
{
    BOOL bCut = FALSE;
    if ( rRef >= nStart )
        rRef += nDelta;
    else if ( nDelta < 0 && rRef >= nStart + nDelta )
        rRef = nStart + nDelta;             //! begrenzen ???
    if ( rRef < 0 )
    {
        rRef = 0;
        bCut = TRUE;
    }
    else if ( rRef > nMask )
    {
        rRef = nMask;
        bCut = TRUE;
    }
    return bCut;
}

BOOL lcl_MoveEnd( short& rRef, short nStart, short nDelta, short nMask )
{
    BOOL bCut = FALSE;
    if ( rRef >= nStart )
        rRef += nDelta;
    else if ( nDelta < 0 && rRef >= nStart + nDelta )
        rRef = nStart + nDelta - 1;         //! begrenzen ???
    if ( rRef < 0 )
    {
        rRef = 0;
        bCut = TRUE;
    }
    else if ( rRef > nMask )
    {
        rRef = nMask;
        bCut = TRUE;
    }
    return bCut;
}

BOOL lcl_MoveReorder( short& rRef, short nStart, short nEnd, short nDelta )
{
    if ( rRef >= nStart && rRef <= nEnd )
    {
        rRef += nDelta;
        return TRUE;
    }

    if ( nDelta > 0 )                   // nach hinten schieben
    {
        if ( rRef >= nStart && rRef <= nEnd + nDelta )
        {
            if ( rRef <= nEnd )
                rRef += nDelta;                 // verschobener Teil
            else
                rRef -= nEnd - nStart + 1;      // nachruecken
            return TRUE;
        }
    }
    else                                // nach vorne schieben
    {
        if ( rRef >= nStart + nDelta && rRef <= nEnd )
        {
            if ( rRef >= nStart )
                rRef += nDelta;                 // verschobener Teil
            else
                rRef += nEnd - nStart + 1;      // nachruecken
            return TRUE;
        }
    }

    return FALSE;
}

BOOL lcl_MoveItCut( short& rRef, short nDelta, short nMask )
{
    BOOL bCut = FALSE;
    rRef += nDelta;
    if ( rRef < 0 )
    {
        rRef = 0;
        bCut = TRUE;
    }
    else if ( rRef > nMask )
    {
        rRef = nMask;
        bCut = TRUE;
    }
    return bCut;
}

void lcl_MoveItWrap( short& rRef, short nDelta, short nMask )
{
    rRef += nDelta;
    if ( rRef < 0 )
        rRef += nMask+1;
    else if ( rRef > nMask )
        rRef -= nMask+1;
}

BOOL lcl_MoveRefPart( short& rRef1Val, BOOL& rRef1Del,
                      short& rRef2Val, BOOL& rRef2Del,
                      short nStart, short nEnd, short nDelta, short nMask )
{
    if ( nDelta )
    {
        BOOL bDel, bCut1, bCut2;
        bDel = bCut1 = bCut2 = FALSE;
        short n;
        if ( nDelta < 0 )
        {
            n = nStart + nDelta;
            if ( n <= rRef1Val && rRef1Val < nStart
              && n <= rRef2Val && rRef2Val < nStart )
                bDel = TRUE;
        }
        else
        {
            n = nEnd + nDelta;
            if ( nEnd < rRef1Val && rRef1Val <= n
              && nEnd < rRef2Val && rRef2Val <= n )
                bDel = TRUE;
        }
        if ( bDel )
        {   // geloeschte mitverschieben
            rRef1Val += nDelta;
            rRef2Val += nDelta;
        }
        else
        {
            if ( rRef1Del )
                rRef1Val += nDelta;
            else
                bCut1 = lcl_MoveStart( rRef1Val, nStart, nDelta, nMask );
            if ( rRef2Del )
                rRef2Val += nDelta;
            else
                bCut2 = lcl_MoveEnd( rRef2Val, nStart, nDelta, nMask );
        }
        if ( bDel || (bCut1 && bCut2) )
            rRef1Del = rRef2Del = TRUE;
        return bDel || bCut1 || bCut2 || rRef1Del || rRef2Del;
    }
    else
        return FALSE;
}


#ifndef DEBUG
inline
#endif
BOOL IsExpand( short n1, short n2, short nStart, short nD )
{   //! vor normalem Move...
    return
        nD > 0          // Insert
     && n1 < n2         // mindestens zwei Cols/Rows/Tabs in Ref
     && (
        (nStart <= n1 && n1 < nStart + nD)      // n1 innerhalb des Insert
        || (n2 + 1 == nStart)                   // n2 direkt vor Insert
        );      // n1 < nStart <= n2 wird sowieso expanded!
}


#ifndef DEBUG
inline
#endif
void Expand( short& n1, short& n2, short nStart, short nD )
{   //! nach normalem Move..., nur wenn IsExpand vorher TRUE war!
    //! erst das Ende
    if ( n2 + 1 == nStart )
    {   // am Ende
        n2 += nD;
        return;
    }
    // am Anfang
    n1 -= nD;
}


BOOL lcl_IsWrapBig( INT32 nRef, INT32 nDelta )
{
    if ( nRef > 0 && nDelta > 0 )
        return nRef + nDelta <= 0;
    else if ( nRef < 0 && nDelta < 0 )
        return nRef + nDelta >= 0;
    return FALSE;
}


BOOL lcl_MoveBig( INT32& rRef, INT32 nStart, INT32 nDelta )
{
    BOOL bCut = FALSE;
    if ( rRef >= nStart )
    {
        if ( nDelta > 0 )
            bCut = lcl_IsWrapBig( rRef, nDelta );
        if ( bCut )
            rRef = nInt32Max;
        else
            rRef += nDelta;
    }
    return bCut;
}

BOOL lcl_MoveItCutBig( INT32& rRef, INT32 nDelta )
{
    BOOL bCut = lcl_IsWrapBig( rRef, nDelta );
    rRef += nDelta;
    return bCut;
}


ScRefUpdateRes ScRefUpdate::Update( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                        USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                        USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                        short nDx, short nDy, short nDz,
                                        USHORT& theCol1, USHORT& theRow1, USHORT& theTab1,
                                        USHORT& theCol2, USHORT& theRow2, USHORT& theTab2 )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    USHORT oldCol1 = theCol1;
    USHORT oldRow1 = theRow1;
    USHORT oldTab1 = theTab1;
    USHORT oldCol2 = theCol2;
    USHORT oldRow2 = theRow2;
    USHORT oldTab2 = theTab2;

    BOOL bCut1, bCut2;

    if (eUpdateRefMode == URM_INSDEL)
    {
        BOOL bExpand = pDoc->IsExpandRefs();
        if ( nDx && (theRow1 >= nRow1) && (theRow2 <= nRow2) &&
                    (theTab1 >= nTab1) && (theTab2 <= nTab2) )
        {
            BOOL bExp = (bExpand && IsExpand( theCol1, theCol2, nCol1, nDx ));
            bCut1 = lcl_MoveStart( (short&) theCol1, nCol1, nDx, MAXCOL );
            bCut2 = lcl_MoveEnd( (short&) theCol2, nCol1, nDx, MAXCOL );
            if ( theCol2 < theCol1 )
            {
                eRet = UR_INVALID;
                theCol2 = theCol1;
            }
            else if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            if ( bExp )
            {
                Expand( (short&) theCol1, (short&) theCol2, nCol1, nDx );
                eRet = UR_UPDATED;
            }
        }
        if ( nDy && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theTab1 >= nTab1) && (theTab2 <= nTab2) )
        {
            BOOL bExp = (bExpand && IsExpand( theRow1, theRow2, nRow1, nDy ));
            bCut1 = lcl_MoveStart( (short&) theRow1, nRow1, nDy, MAXROW );
            bCut2 = lcl_MoveEnd( (short&) theRow2, nRow1, nDy, MAXROW );
            if ( theRow2 < theRow1 )
            {
                eRet = UR_INVALID;
                theRow2 = theRow1;
            }
            else if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            if ( bExp )
            {
                Expand( (short&) theRow1, (short&) theRow2, nRow1, nDy );
                eRet = UR_UPDATED;
            }
        }
        if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theRow1 >= nRow1) && (theRow2 <= nRow2) )
        {
            short nMaxTab = pDoc->GetTableCount() - 1;
            nMaxTab += nDz;     // auf die neue Anzahl anpassen
            BOOL bExp = (bExpand && IsExpand( theTab1, theTab2, nTab1, nDz ));
            bCut1 = lcl_MoveStart( (short&) theTab1, nTab1, nDz, nMaxTab );
            bCut2 = lcl_MoveEnd( (short&) theTab2, nTab1, nDz, nMaxTab );
            if ( theTab2 < theTab1 )
            {
                eRet = UR_INVALID;
                theTab2 = theTab1;
            }
            else if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            if ( bExp )
            {
                Expand( (short&) theTab1, (short&) theTab2, nTab1, nDz );
                eRet = UR_UPDATED;
            }
        }
    }
    else if (eUpdateRefMode == URM_MOVE)
    {
        if ((theCol1 >= nCol1-nDx) && (theRow1 >= nRow1-nDy) && (theTab1 >= nTab1-nDz) &&
            (theCol2 <= nCol2-nDx) && (theRow2 <= nRow2-nDy) && (theTab2 <= nTab2-nDz))
        {
            if ( nDx )
            {
                bCut1 = lcl_MoveItCut( (short&) theCol1, nDx, MAXCOL );
                bCut2 = lcl_MoveItCut( (short&) theCol2, nDx, MAXCOL );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
            }
            if ( nDy )
            {
                bCut1 = lcl_MoveItCut( (short&) theRow1, nDy, MAXROW );
                bCut2 = lcl_MoveItCut( (short&) theRow2, nDy, MAXROW );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
            }
            if ( nDz )
            {
                short nMaxTab = (short) pDoc->GetTableCount() - 1;
                bCut1 = lcl_MoveItCut( (short&) theTab1, nDz, nMaxTab );
                bCut2 = lcl_MoveItCut( (short&) theTab2, nDz, nMaxTab );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
            }
        }
    }
    else if (eUpdateRefMode == URM_REORDER)
    {
        //  bisher nur fuer nDz (MoveTab)
        DBG_ASSERT ( !nDx && !nDy, "URM_REORDER fuer x und y noch nicht implementiert" );

        if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theRow1 >= nRow1) && (theRow2 <= nRow2) )
        {
            bCut1 = lcl_MoveReorder( (short&) theTab1, nTab1, nTab2, nDz );
            bCut2 = lcl_MoveReorder( (short&) theTab2, nTab1, nTab2, nDz );
            if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
        }
    }

    if ( eRet == UR_NOTHING )
    {
        if (oldCol1 != theCol1
         || oldRow1 != theRow1
         || oldTab1 != theTab1
         || oldCol2 != theCol2
         || oldRow2 != theRow2
         || oldTab2 != theTab2
            )
            eRet = UR_UPDATED;
    }
    return eRet;
}


// simples UpdateReference fuer ScBigRange (ScChangeAction/ScChangeTrack)
// Referenzen koennen auch ausserhalb des Dokuments liegen!
// Ganze Spalten/Zeilen (nInt32Min..nInt32Max) bleiben immer solche!
ScRefUpdateRes ScRefUpdate::Update( UpdateRefMode eUpdateRefMode,
        const ScBigRange& rWhere, INT32 nDx, INT32 nDy, INT32 nDz,
        ScBigRange& rWhat )
{
    ScRefUpdateRes eRet = UR_NOTHING;
    const ScBigRange aOldRange( rWhat );

    INT32 nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    INT32 theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
    rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    rWhat.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );

    BOOL bCut1, bCut2;

    if (eUpdateRefMode == URM_INSDEL)
    {
        if ( nDx && (theRow1 >= nRow1) && (theRow2 <= nRow2) &&
                    (theTab1 >= nTab1) && (theTab2 <= nTab2) &&
                    !(theCol1 == nInt32Min && theCol2 == nInt32Max) )
        {
            bCut1 = lcl_MoveBig( theCol1, nCol1, nDx );
            bCut2 = lcl_MoveBig( theCol2, nCol1, nDx );
            if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            rWhat.aStart.SetCol( theCol1 );
            rWhat.aEnd.SetCol( theCol2 );
        }
        if ( nDy && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theTab1 >= nTab1) && (theTab2 <= nTab2) &&
                    !(theRow1 == nInt32Min && theRow2 == nInt32Max) )
        {
            bCut1 = lcl_MoveBig( theRow1, nRow1, nDy );
            bCut2 = lcl_MoveBig( theRow2, nRow1, nDy );
            if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            rWhat.aStart.SetRow( theRow1 );
            rWhat.aEnd.SetRow( theRow2 );
        }
        if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theRow1 >= nRow1) && (theRow2 <= nRow2) &&
                    !(theTab1 == nInt32Min && theTab2 == nInt32Max) )
        {
            bCut1 = lcl_MoveBig( theTab1, nTab1, nDz );
            bCut2 = lcl_MoveBig( theTab2, nTab1, nDz );
            if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            rWhat.aStart.SetTab( theTab1 );
            rWhat.aEnd.SetTab( theTab2 );
        }
    }
    else if (eUpdateRefMode == URM_MOVE)
    {
        if ( rWhere.In( rWhat ) )
        {
            if ( nDx && !(theCol1 == nInt32Min && theCol2 == nInt32Max) )
            {
                bCut1 = lcl_MoveItCutBig( theCol1, nDx );
                bCut2 = lcl_MoveItCutBig( theCol2, nDx );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
                rWhat.aStart.SetCol( theCol1 );
                rWhat.aEnd.SetCol( theCol2 );
            }
            if ( nDy && !(theRow1 == nInt32Min && theRow2 == nInt32Max) )
            {
                bCut1 = lcl_MoveItCutBig( theRow1, nDy );
                bCut2 = lcl_MoveItCutBig( theRow2, nDy );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
                rWhat.aStart.SetRow( theRow1 );
                rWhat.aEnd.SetRow( theRow2 );
            }
            if ( nDz && !(theTab1 == nInt32Min && theTab2 == nInt32Max) )
            {
                bCut1 = lcl_MoveItCutBig( theTab1, nDz );
                bCut2 = lcl_MoveItCutBig( theTab2, nDz );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
                rWhat.aStart.SetTab( theTab1 );
                rWhat.aEnd.SetTab( theTab2 );
            }
        }
    }

    if ( eRet == UR_NOTHING && rWhat != aOldRange )
        eRet = UR_UPDATED;

    return eRet;
}


// vor dem Aufruf muessen die Abs-Refs aktualisiert werden!
ScRefUpdateRes ScRefUpdate::Update( ScDocument* pDoc, UpdateRefMode eMode,
                                    const ScAddress& rPos, const ScRange& r,
                                    short nDx, short nDy, short nDz,
                                    ComplRefData& rRef )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    short nCol1 = r.aStart.Col();
    short nRow1 = r.aStart.Row();
    short nTab1 = r.aStart.Tab();
    short nCol2 = r.aEnd.Col();
    short nRow2 = r.aEnd.Row();
    short nTab2 = r.aEnd.Tab();

    if( eMode == URM_INSDEL )
    {
        BOOL bExpand = pDoc->IsExpandRefs();

        const ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        BOOL bInDeleteUndo =
            ( pChangeTrack ? pChangeTrack->IsInDeleteUndo() : FALSE );

        USHORT oldCol1 = rRef.Ref1.nCol;
        USHORT oldRow1 = rRef.Ref1.nRow;
        USHORT oldTab1 = rRef.Ref1.nTab;
        USHORT oldCol2 = rRef.Ref2.nCol;
        USHORT oldRow2 = rRef.Ref2.nRow;
        USHORT oldTab2 = rRef.Ref2.nTab;

        BOOL bRef1ColDel = rRef.Ref1.IsColDeleted();
        BOOL bRef2ColDel = rRef.Ref2.IsColDeleted();
        BOOL bRef1RowDel = rRef.Ref1.IsRowDeleted();
        BOOL bRef2RowDel = rRef.Ref2.IsRowDeleted();
        BOOL bRef1TabDel = rRef.Ref1.IsTabDeleted();
        BOOL bRef2TabDel = rRef.Ref2.IsTabDeleted();

        if( nDx &&
            ((rRef.Ref1.nRow >= nRow1
           && rRef.Ref2.nRow <= nRow2) || (bRef1RowDel || bRef2RowDel))
            &&
            ((rRef.Ref1.nTab >= nTab1
           && rRef.Ref2.nTab <= nTab2) || (bRef1TabDel || bRef2TabDel))
           )
        {
            BOOL bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nCol,
                rRef.Ref2.nCol, nCol1, nDx ));
            if ( lcl_MoveRefPart( rRef.Ref1.nCol, bRef1ColDel,
                                  rRef.Ref2.nCol, bRef2ColDel,
                                  nCol1, nCol2, nDx, MAXCOL ) )
            {
                eRet = UR_UPDATED;
                if ( bInDeleteUndo && (bRef1ColDel || bRef2ColDel) )
                {
                    if ( bRef1ColDel && nCol1 <= rRef.Ref1.nCol &&
                            rRef.Ref1.nCol <= nCol1 + nDx )
                        rRef.Ref1.SetColDeleted( FALSE );
                    if ( bRef2ColDel && nCol1 <= rRef.Ref2.nCol &&
                            rRef.Ref2.nCol <= nCol1 + nDx )
                        rRef.Ref2.SetColDeleted( FALSE );
                }
                else
                {
                    if ( bRef1ColDel )
                        rRef.Ref1.SetColDeleted( TRUE );
                    if ( bRef2ColDel )
                        rRef.Ref2.SetColDeleted( TRUE );
                }
            }
            if ( bExp )
            {
                Expand( rRef.Ref1.nCol, rRef.Ref2.nCol, nCol1, nDx );
                eRet = UR_UPDATED;
            }
        }
        if( nDy &&
            ((rRef.Ref1.nCol >= nCol1
           && rRef.Ref2.nCol <= nCol2) || (bRef1ColDel || bRef2ColDel))
            &&
            ((rRef.Ref1.nTab >= nTab1
           && rRef.Ref2.nTab <= nTab2) || (bRef1TabDel || bRef2TabDel))
           )
        {
            BOOL bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nRow,
                rRef.Ref2.nRow, nRow1, nDy ));
            if ( lcl_MoveRefPart( rRef.Ref1.nRow, bRef1RowDel,
                                rRef.Ref2.nRow, bRef2RowDel,
                                nRow1, nRow2, nDy, MAXROW ) )
            {
                eRet = UR_UPDATED;
                if ( bInDeleteUndo && (bRef1RowDel || bRef2RowDel) )
                {
                    if ( bRef1RowDel && nRow1 <= rRef.Ref1.nRow &&
                            rRef.Ref1.nRow <= nRow1 + nDy )
                        rRef.Ref1.SetRowDeleted( FALSE );
                    if ( bRef2RowDel && nRow1 <= rRef.Ref2.nRow &&
                            rRef.Ref2.nRow <= nRow1 + nDy )
                        rRef.Ref2.SetRowDeleted( FALSE );
                }
                else
                {
                    if ( bRef1RowDel )
                        rRef.Ref1.SetRowDeleted( TRUE );
                    if ( bRef2RowDel )
                        rRef.Ref2.SetRowDeleted( TRUE );
                }
            }
            if ( bExp )
            {
                Expand( rRef.Ref1.nRow, rRef.Ref2.nRow, nRow1, nDy );
                eRet = UR_UPDATED;
            }
        }
        if( nDz &&
            ((rRef.Ref1.nCol >= nCol1
           && rRef.Ref2.nCol <= nCol2) || (bRef1ColDel || bRef2ColDel))
            &&
            ((rRef.Ref1.nRow >= nRow1
           && rRef.Ref2.nRow <= nRow2) || (bRef1RowDel || bRef2RowDel))
           )
        {
            BOOL bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nTab,
                rRef.Ref2.nTab, nTab1, nDz ));
            short nMaxTab = (short) pDoc->GetTableCount() - 1;
            if ( lcl_MoveRefPart( rRef.Ref1.nTab, bRef1TabDel,
                                  rRef.Ref2.nTab, bRef2TabDel,
                                  nTab1, nTab2, nDz, nMaxTab ) )
            {
                eRet = UR_UPDATED;
                if ( bInDeleteUndo && (bRef1TabDel || bRef2TabDel) )
                {
                    if ( bRef1TabDel && nTab1 <= rRef.Ref1.nTab &&
                            rRef.Ref1.nTab <= nTab1 + nDz )
                        rRef.Ref1.SetTabDeleted( FALSE );
                    if ( bRef2TabDel && nTab1 <= rRef.Ref2.nTab &&
                            rRef.Ref2.nTab <= nTab1 + nDz )
                        rRef.Ref2.SetTabDeleted( FALSE );
                }
                else
                {
                    if ( bRef1TabDel )
                        rRef.Ref1.SetTabDeleted( TRUE );
                    if ( bRef2TabDel )
                        rRef.Ref2.SetTabDeleted( TRUE );
                }
            }
            if ( bExp )
            {
                Expand( rRef.Ref1.nTab, rRef.Ref2.nTab, nTab1, nDz );
                eRet = UR_UPDATED;
            }
        }
        if ( eRet == UR_NOTHING )
        {
            if (oldCol1 != rRef.Ref1.nCol
             || oldRow1 != rRef.Ref1.nRow
             || oldTab1 != rRef.Ref1.nTab
             || oldCol2 != rRef.Ref2.nCol
             || oldRow2 != rRef.Ref2.nRow
             || oldTab2 != rRef.Ref2.nTab
                )
                eRet = UR_UPDATED;
        }
        rRef.CalcRelFromAbs( rPos );
    }
    else
    {
        if( eMode == URM_MOVE )
        {
            if ( rRef.Ref1.nCol >= nCol1-nDx
              && rRef.Ref1.nRow >= nRow1-nDy
              && rRef.Ref1.nTab >= nTab1-nDz
              && rRef.Ref2.nCol <= nCol2-nDx
              && rRef.Ref2.nRow <= nRow2-nDy
              && rRef.Ref2.nTab <= nTab2-nDz )
            {
                eRet = Move( pDoc, rPos, nDx, nDy, nDz, rRef, FALSE, TRUE );        // immer verschieben
            }
            else if ( nDz && r.In( rPos ) )
            {
                rRef.Ref1.SetFlag3D( TRUE );
                rRef.Ref2.SetFlag3D( TRUE );
                eRet = UR_UPDATED;
                rRef.CalcRelFromAbs( rPos );
            }
            else
                rRef.CalcRelFromAbs( rPos );
        }
        else if( eMode == URM_COPY && r.In( rPos ) )
            eRet = Move( pDoc, rPos, nDx, nDy, nDz, rRef, FALSE, FALSE );       // nur relative
            // sollte nicht mehr verwendet werden muessen
        else
            rRef.CalcRelFromAbs( rPos );
    }
    return eRet;
}

// vor dem Aufruf muessen die Abs-Refs aktualisiert werden!
ScRefUpdateRes ScRefUpdate::Move( ScDocument* pDoc, const ScAddress& rPos,
                                  short nDx, short nDy, short nDz,
                                  ComplRefData& rRef, BOOL bWrap, BOOL bAbsolute )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    USHORT oldCol1 = rRef.Ref1.nCol;
    USHORT oldRow1 = rRef.Ref1.nRow;
    USHORT oldTab1 = rRef.Ref1.nTab;
    USHORT oldCol2 = rRef.Ref2.nCol;
    USHORT oldRow2 = rRef.Ref2.nRow;
    USHORT oldTab2 = rRef.Ref2.nTab;

    BOOL bCut1, bCut2;
    if ( nDx )
    {
        bCut1 = bCut2 = FALSE;
        if( bAbsolute || rRef.Ref1.IsColRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( (short&) rRef.Ref1.nCol, nDx, MAXCOL );
            else
                bCut1 = lcl_MoveItCut( (short&) rRef.Ref1.nCol, nDx, MAXCOL );
        }
        if( bAbsolute || rRef.Ref2.IsColRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( (short&) rRef.Ref2.nCol, nDx, MAXCOL );
            else
                bCut2 = lcl_MoveItCut( (short&) rRef.Ref2.nCol, nDx, MAXCOL );
        }
        if ( bCut1 || bCut2 )
            eRet = UR_UPDATED;
        if ( bCut1 && bCut2 )
        {
            rRef.Ref1.SetColDeleted( TRUE );
            rRef.Ref2.SetColDeleted( TRUE );
        }
    }
    if ( nDy )
    {
        bCut1 = bCut2 = FALSE;
        if( bAbsolute || rRef.Ref1.IsRowRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( (short&) rRef.Ref1.nRow, nDy, MAXROW );
            else
                bCut1 = lcl_MoveItCut( (short&) rRef.Ref1.nRow, nDy, MAXROW );
        }
        if( bAbsolute || rRef.Ref2.IsRowRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( (short&) rRef.Ref2.nRow, nDy, MAXROW );
            else
                bCut2 = lcl_MoveItCut( (short&) rRef.Ref2.nRow, nDy, MAXROW );
        }
        if ( bCut1 || bCut2 )
            eRet = UR_UPDATED;
        if ( bCut1 && bCut2 )
        {
            rRef.Ref1.SetRowDeleted( TRUE );
            rRef.Ref2.SetRowDeleted( TRUE );
        }
    }
    if ( nDz )
    {
        bCut1 = bCut2 = FALSE;
        short nMaxTab = (short) pDoc->GetTableCount() - 1;
        if( bAbsolute || rRef.Ref1.IsTabRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( (short&) rRef.Ref1.nTab, nDz, nMaxTab );
            else
                bCut1 = lcl_MoveItCut( (short&) rRef.Ref1.nTab, nDz, nMaxTab );
            rRef.Ref1.SetFlag3D( rPos.Tab() != rRef.Ref1.nTab );
        }
        if( bAbsolute || rRef.Ref2.IsTabRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( (short&) rRef.Ref2.nTab, nDz, nMaxTab );
            else
                bCut2 = lcl_MoveItCut( (short&) rRef.Ref2.nTab, nDz, nMaxTab );
            rRef.Ref2.SetFlag3D( rPos.Tab() != rRef.Ref2.nTab );
        }
        if ( bCut1 || bCut2 )
            eRet = UR_UPDATED;
        if ( bCut1 && bCut2 )
        {
            rRef.Ref1.SetTabDeleted( TRUE );
            rRef.Ref2.SetTabDeleted( TRUE );
        }
    }

    if ( eRet == UR_NOTHING )
    {
        if (oldCol1 != rRef.Ref1.nCol
         || oldRow1 != rRef.Ref1.nRow
         || oldTab1 != rRef.Ref1.nTab
         || oldCol2 != rRef.Ref2.nCol
         || oldRow2 != rRef.Ref2.nRow
         || oldTab2 != rRef.Ref2.nTab
            )
            eRet = UR_UPDATED;
    }
    if ( bWrap && eRet != UR_NOTHING )
        rRef.PutInOrder();
    rRef.CalcRelFromAbs( rPos );
    return eRet;
}

void ScRefUpdate::MoveRelWrap( ScDocument* pDoc, const ScAddress& rPos,
                                  ComplRefData& rRef )
{
    if( rRef.Ref1.IsColRel() )
    {
        rRef.Ref1.nCol = rRef.Ref1.nRelCol + rPos.Col();
        lcl_MoveItWrap( (short&) rRef.Ref1.nCol, 0, MAXCOL );
    }
    if( rRef.Ref2.IsColRel() )
    {
        rRef.Ref2.nCol = rRef.Ref2.nRelCol + rPos.Col();
        lcl_MoveItWrap( (short&) rRef.Ref2.nCol, 0, MAXCOL );
    }
    if( rRef.Ref1.IsRowRel() )
    {
        rRef.Ref1.nRow = rRef.Ref1.nRelRow + rPos.Row();
        lcl_MoveItWrap( (short&) rRef.Ref1.nRow, 0, MAXROW );
    }
    if( rRef.Ref2.IsRowRel() )
    {
        rRef.Ref2.nRow = rRef.Ref2.nRelRow + rPos.Row();
        lcl_MoveItWrap( (short&) rRef.Ref2.nRow, 0, MAXROW );
    }
    short nMaxTab = (short) pDoc->GetTableCount() - 1;
    if( rRef.Ref1.IsTabRel() )
    {
        rRef.Ref1.nTab = rRef.Ref1.nRelTab + rPos.Tab();
        lcl_MoveItWrap( (short&) rRef.Ref1.nTab, 0, nMaxTab );
    }
    if( rRef.Ref2.IsTabRel() )
    {
        rRef.Ref2.nTab = rRef.Ref2.nRelTab + rPos.Tab();
        lcl_MoveItWrap( (short&) rRef.Ref2.nTab, 0, nMaxTab );
    }
    rRef.PutInOrder();
    rRef.CalcRelFromAbs( rPos );
}

//------------------------------------------------------------------

void ScRefUpdate::DoTranspose( short& rCol, short& rRow, short& rTab,
                        ScDocument* pDoc, const ScRange& rSource, const ScAddress& rDest )
{
    short nDz = ((short)rDest.Tab())-(short)rSource.aStart.Tab();
    if (nDz)
    {
        short nNewTab = rTab+nDz;
        short nCount = pDoc->GetTableCount();
        while (nNewTab<0) nNewTab += nCount;
        while (nNewTab>=nCount) nNewTab -= nCount;
        rTab = nNewTab;
    }
    DBG_ASSERT( rCol>=rSource.aStart.Col() && rRow>=rSource.aStart.Row(),
                "UpdateTranspose: Pos. falsch" );

    short nRelX = rCol - (short)rSource.aStart.Col();
    short nRelY = rRow - (short)rSource.aStart.Row();

    rCol = (short)rDest.Col() + nRelY;
    rRow = (short)rDest.Row() + nRelX;
}

// vor dem Aufruf muessen die Abs-Refs aktualisiert werden!
ScRefUpdateRes ScRefUpdate::UpdateTranspose( ScDocument* pDoc,
                                const ScRange& rSource, const ScAddress& rDest,
                                ComplRefData& rRef )
{
    ScRefUpdateRes eRet = UR_NOTHING;
    if ( rRef.Ref1.nCol >= rSource.aStart.Col() && rRef.Ref2.nCol <= rSource.aEnd.Col() &&
         rRef.Ref1.nRow >= rSource.aStart.Row() && rRef.Ref2.nRow <= rSource.aEnd.Row() &&
         rRef.Ref1.nTab >= rSource.aStart.Tab() && rRef.Ref2.nTab <= rSource.aEnd.Tab() )
    {
        DoTranspose( rRef.Ref1.nCol, rRef.Ref1.nRow, rRef.Ref1.nTab, pDoc, rSource, rDest );
        DoTranspose( rRef.Ref2.nCol, rRef.Ref2.nRow, rRef.Ref2.nTab, pDoc, rSource, rDest );
        eRet = UR_UPDATED;
    }
    return eRet;
}

//------------------------------------------------------------------

//  UpdateGrow - erweitert Referenzen, die genau auf den Bereich zeigen
//  kommt ohne Dokument aus

// vor dem Aufruf muessen die Abs-Refs aktualisiert werden!
ScRefUpdateRes ScRefUpdate::UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY,
                                        ComplRefData& rRef )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    //  in Y-Richtung darf die Ref auch eine Zeile weiter unten anfangen,
    //  falls ein Bereich Spaltenkoepfe enthaelt

    BOOL bUpdateX = ( nGrowX &&
            rRef.Ref1.nCol == rArea.aStart.Col() && rRef.Ref2.nCol == rArea.aEnd.Col() &&
            rRef.Ref1.nRow >= rArea.aStart.Row() && rRef.Ref2.nRow <= rArea.aEnd.Row() &&
            rRef.Ref1.nTab >= rArea.aStart.Tab() && rRef.Ref2.nTab <= rArea.aEnd.Tab() );
    BOOL bUpdateY = ( nGrowY &&
            rRef.Ref1.nCol >= rArea.aStart.Col() && rRef.Ref2.nCol <= rArea.aEnd.Col() &&
            ( rRef.Ref1.nRow == rArea.aStart.Row() || rRef.Ref1.nRow == rArea.aStart.Row()+1 ) &&
                rRef.Ref2.nRow == rArea.aEnd.Row() &&
            rRef.Ref1.nTab >= rArea.aStart.Tab() && rRef.Ref2.nTab <= rArea.aEnd.Tab() );

    if ( bUpdateX )
    {
        rRef.Ref2.nCol += nGrowX;
        eRet = UR_UPDATED;
    }
    if ( bUpdateY )
    {
        rRef.Ref2.nRow += nGrowY;
        eRet = UR_UPDATED;
    }

    return eRet;
}

ScRefUpdateRes ScRefUpdate::DoGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY,
                                    ScRange& rRef )
{
    //  wie UpdateGrow, nur mit Range statt RefData

    ScRefUpdateRes eRet = UR_NOTHING;

    BOOL bUpdateX = ( nGrowX &&
            rRef.aStart.Col() == rArea.aStart.Col() && rRef.aEnd.Col() == rArea.aEnd.Col() &&
            rRef.aStart.Row() >= rArea.aStart.Row() && rRef.aEnd.Row() <= rArea.aEnd.Row() &&
            rRef.aStart.Tab() >= rArea.aStart.Tab() && rRef.aEnd.Tab() <= rArea.aEnd.Tab() );
    BOOL bUpdateY = ( nGrowY &&
            rRef.aStart.Col() >= rArea.aStart.Col() && rRef.aEnd.Col() <= rArea.aEnd.Col() &&
            ( rRef.aStart.Row() == rArea.aStart.Row() || rRef.aStart.Row() == rArea.aStart.Row()+1 ) &&
                rRef.aEnd.Row() == rArea.aEnd.Row() &&
            rRef.aStart.Tab() >= rArea.aStart.Tab() && rRef.aEnd.Tab() <= rArea.aEnd.Tab() );

    if ( bUpdateX )
    {
        rRef.aEnd.SetCol( rRef.aEnd.Col() + nGrowX );
        eRet = UR_UPDATED;
    }
    if ( bUpdateY )
    {
        rRef.aEnd.SetRow( rRef.aEnd.Row() + nGrowY );
        eRet = UR_UPDATED;
    }

    return eRet;
}



