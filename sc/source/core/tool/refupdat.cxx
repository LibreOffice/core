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

#include "refupdat.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "bigrange.hxx"
#include "chgtrack.hxx"

//------------------------------------------------------------------------

template< typename R, typename S, typename U >
static bool lcl_MoveStart( R& rRef, U nStart, S nDelta, U nMask )
{
    bool bCut = false;
    if ( rRef >= nStart )
        rRef = sal::static_int_cast<R>( rRef + nDelta );
    else if ( nDelta < 0 && rRef >= nStart + nDelta )
        rRef = nStart + nDelta;             //! begrenzen ???
    if ( rRef < 0 )
    {
        rRef = 0;
        bCut = true;
    }
    else if ( rRef > nMask )
    {
        rRef = nMask;
        bCut = true;
    }
    return bCut;
}

template< typename R, typename S, typename U >
static bool lcl_MoveEnd( R& rRef, U nStart, S nDelta, U nMask )
{
    bool bCut = false;
    if ( rRef >= nStart )
        rRef = sal::static_int_cast<R>( rRef + nDelta );
    else if ( nDelta < 0 && rRef >= nStart + nDelta )
        rRef = nStart + nDelta - 1;         //! begrenzen ???
    if ( rRef < 0 )
    {
        rRef = 0;
        bCut = true;
    }
    else if ( rRef > nMask )
    {
        rRef = nMask;
        bCut = true;
    }
    return bCut;
}

template< typename R, typename S, typename U >
static bool lcl_MoveReorder( R& rRef, U nStart, U nEnd, S nDelta )
{
    if ( rRef >= nStart && rRef <= nEnd )
    {
        rRef = sal::static_int_cast<R>( rRef + nDelta );
        return true;
    }

    if ( nDelta > 0 )                   // nach hinten schieben
    {
        if ( rRef >= nStart && rRef <= nEnd + nDelta )
        {
            if ( rRef <= nEnd )
                rRef = sal::static_int_cast<R>( rRef + nDelta );    // in the moved range
            else
                rRef -= nEnd - nStart + 1;      // nachruecken
            return true;
        }
    }
    else                                // nach vorne schieben
    {
        if ( rRef >= nStart + nDelta && rRef <= nEnd )
        {
            if ( rRef >= nStart )
                rRef = sal::static_int_cast<R>( rRef + nDelta );    // in the moved range
            else
                rRef += nEnd - nStart + 1;      // nachruecken
            return true;
        }
    }

    return false;
}

template< typename R, typename S, typename U >
static bool lcl_MoveItCut( R& rRef, S nDelta, U nMask )
{
    bool bCut = false;
    rRef = sal::static_int_cast<R>( rRef + nDelta );
    if ( rRef < 0 )
    {
        rRef = 0;
        bCut = true;
    }
    else if ( rRef > nMask )
    {
        rRef = nMask;
        bCut = true;
    }
    return bCut;
}

template< typename R, typename S, typename U >
static void lcl_MoveItWrap( R& rRef, S nDelta, U nMask )
{
    rRef = sal::static_int_cast<R>( rRef + nDelta );
    if ( rRef < 0 )
        rRef += nMask+1;
    else if ( rRef > nMask )
        rRef -= nMask+1;
}

template< typename R, typename S, typename U >
static bool lcl_MoveRefPart( R& rRef1Val, bool& rRef1Del, bool bDo1,
                      R& rRef2Val, bool& rRef2Del, bool bDo2,
                      U nStart, U nEnd, S nDelta, U nMask )
{
    if ( nDelta )
    {
        bool bDel, bCut1, bCut2;
        bDel = bCut1 = bCut2 = false;
        S n;
        if (bDo1 && bDo2)
        {
            if ( nDelta < 0 )
            {
                n = nStart + nDelta;
                if ( n <= rRef1Val && rRef1Val < nStart
                  && n <= rRef2Val && rRef2Val < nStart )
                    bDel = true;
            }
            else
            {
                n = nEnd + nDelta;
                if ( nEnd < rRef1Val && rRef1Val <= n
                  && nEnd < rRef2Val && rRef2Val <= n )
                    bDel = true;
            }
        }
        if ( bDel )
        {   // move deleted along
            rRef1Val = sal::static_int_cast<R>( rRef1Val + nDelta );
            rRef2Val = sal::static_int_cast<R>( rRef2Val + nDelta );
        }
        else
        {
            if (bDo1)
            {
                if ( rRef1Del )
                    rRef1Val = sal::static_int_cast<R>( rRef1Val + nDelta );
                else
                    bCut1 = lcl_MoveStart( rRef1Val, nStart, nDelta, nMask );
            }
            if (bDo2)
            {
                if ( rRef2Del )
                    rRef2Val = sal::static_int_cast<R>( rRef2Val + nDelta );
                else
                    bCut2 = lcl_MoveEnd( rRef2Val, nStart, nDelta, nMask );
            }
        }
        if ( bDel || (bCut1 && bCut2) )
            rRef1Del = rRef2Del = true;
        return bDel || bCut1 || bCut2 || rRef1Del || rRef2Del;
    }
    else
        return false;
}

template< typename R, typename S, typename U >
bool IsExpand( R n1, R n2, U nStart, S nD )
{   //! vor normalem Move...
    return
        nD > 0          // Insert
     && n1 < n2         // mindestens zwei Cols/Rows/Tabs in Ref
     && (
        (nStart <= n1 && n1 < nStart + nD)      // n1 innerhalb des Insert
        || (n2 + 1 == nStart)                   // n2 direkt vor Insert
        );      // n1 < nStart <= n2 wird sowieso expanded!
}


template< typename R, typename S, typename U >
void Expand( R& n1, R& n2, U nStart, S nD )
{   //! nach normalem Move..., nur wenn IsExpand vorher true war!
    //! erst das Ende
    if ( n2 + 1 == nStart )
    {   // am Ende
        n2 = sal::static_int_cast<R>( n2 + nD );
        return;
    }
    // am Anfang
    n1 = sal::static_int_cast<R>( n1 - nD );
}


static bool lcl_IsWrapBig( sal_Int32 nRef, sal_Int32 nDelta )
{
    if ( nRef > 0 && nDelta > 0 )
        return nRef + nDelta <= 0;
    else if ( nRef < 0 && nDelta < 0 )
        return nRef + nDelta >= 0;
    return false;
}


static bool lcl_MoveBig( sal_Int32& rRef, sal_Int32 nStart, sal_Int32 nDelta )
{
    bool bCut = false;
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

static bool lcl_MoveItCutBig( sal_Int32& rRef, sal_Int32 nDelta )
{
    bool bCut = lcl_IsWrapBig( rRef, nDelta );
    rRef += nDelta;
    return bCut;
}


ScRefUpdateRes ScRefUpdate::Update( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                        SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                        SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                        SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                        SCCOL& theCol1, SCROW& theRow1, SCTAB& theTab1,
                                        SCCOL& theCol2, SCROW& theRow2, SCTAB& theTab2 )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    SCCOL oldCol1 = theCol1;
    SCROW oldRow1 = theRow1;
    SCTAB oldTab1 = theTab1;
    SCCOL oldCol2 = theCol2;
    SCROW oldRow2 = theRow2;
    SCTAB oldTab2 = theTab2;

    bool bCut1, bCut2;

    if (eUpdateRefMode == URM_INSDEL)
    {
        bool bExpand = pDoc->IsExpandRefs();
        if ( nDx && (theRow1 >= nRow1) && (theRow2 <= nRow2) &&
                    (theTab1 >= nTab1) && (theTab2 <= nTab2) )
        {
            bool bExp = (bExpand && IsExpand( theCol1, theCol2, nCol1, nDx ));
            bCut1 = lcl_MoveStart( theCol1, nCol1, nDx, MAXCOL );
            bCut2 = lcl_MoveEnd( theCol2, nCol1, nDx, MAXCOL );
            if ( theCol2 < theCol1 )
            {
                eRet = UR_INVALID;
                theCol2 = theCol1;
            }
            else if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            if ( bExp )
            {
                Expand( theCol1, theCol2, nCol1, nDx );
                eRet = UR_UPDATED;
            }
        }
        if ( nDy && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theTab1 >= nTab1) && (theTab2 <= nTab2) )
        {
            bool bExp = (bExpand && IsExpand( theRow1, theRow2, nRow1, nDy ));
            bCut1 = lcl_MoveStart( theRow1, nRow1, nDy, MAXROW );
            bCut2 = lcl_MoveEnd( theRow2, nRow1, nDy, MAXROW );
            if ( theRow2 < theRow1 )
            {
                eRet = UR_INVALID;
                theRow2 = theRow1;
            }
            else if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            if ( bExp )
            {
                Expand( theRow1, theRow2, nRow1, nDy );
                eRet = UR_UPDATED;
            }
        }
        if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theRow1 >= nRow1) && (theRow2 <= nRow2) )
        {
            SCsTAB nMaxTab = pDoc->GetTableCount() - 1;
            nMaxTab = sal::static_int_cast<SCsTAB>(nMaxTab + nDz);      // adjust to new count
            bool bExp = (bExpand && IsExpand( theTab1, theTab2, nTab1, nDz ));
            bCut1 = lcl_MoveStart( theTab1, nTab1, nDz, static_cast<SCTAB>(nMaxTab) );
            bCut2 = lcl_MoveEnd( theTab2, nTab1, nDz, static_cast<SCTAB>(nMaxTab) );
            if ( theTab2 < theTab1 )
            {
                eRet = UR_INVALID;
                theTab2 = theTab1;
            }
            else if ( bCut1 || bCut2 )
                eRet = UR_UPDATED;
            if ( bExp )
            {
                Expand( theTab1, theTab2, nTab1, nDz );
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
                bCut1 = lcl_MoveItCut( theCol1, nDx, MAXCOL );
                bCut2 = lcl_MoveItCut( theCol2, nDx, MAXCOL );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
            }
            if ( nDy )
            {
                bCut1 = lcl_MoveItCut( theRow1, nDy, MAXROW );
                bCut2 = lcl_MoveItCut( theRow2, nDy, MAXROW );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
            }
            if ( nDz )
            {
                SCsTAB nMaxTab = (SCsTAB) pDoc->GetTableCount() - 1;
                bCut1 = lcl_MoveItCut( theTab1, nDz, static_cast<SCTAB>(nMaxTab) );
                bCut2 = lcl_MoveItCut( theTab2, nDz, static_cast<SCTAB>(nMaxTab) );
                if ( bCut1 || bCut2 )
                    eRet = UR_UPDATED;
            }
        }
    }
    else if (eUpdateRefMode == URM_REORDER)
    {
        //  bisher nur fuer nDz (MoveTab)
        OSL_ENSURE ( !nDx && !nDy, "URM_REORDER for x and y not yet implemented" );

        if ( nDz && (theCol1 >= nCol1) && (theCol2 <= nCol2) &&
                    (theRow1 >= nRow1) && (theRow2 <= nRow2) )
        {
            bCut1 = lcl_MoveReorder( theTab1, nTab1, nTab2, nDz );
            bCut2 = lcl_MoveReorder( theTab2, nTab1, nTab2, nDz );
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
        const ScBigRange& rWhere, sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz,
        ScBigRange& rWhat )
{
    ScRefUpdateRes eRet = UR_NOTHING;
    const ScBigRange aOldRange( rWhat );

    sal_Int32 nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    sal_Int32 theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
    rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
    rWhat.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );

    bool bCut1, bCut2;

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


ScRefUpdateRes ScRefUpdate::Update( ScDocument* pDoc, UpdateRefMode eMode,
                                    const ScAddress& rPos, const ScRange& r,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                    ScComplexRefData& rRef, WhatType eWhat )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    SCCOL nCol1 = r.aStart.Col();
    SCROW nRow1 = r.aStart.Row();
    SCTAB nTab1 = r.aStart.Tab();
    SCCOL nCol2 = r.aEnd.Col();
    SCROW nRow2 = r.aEnd.Row();
    SCTAB nTab2 = r.aEnd.Tab();

    if( eMode == URM_INSDEL )
    {
        bool bExpand = pDoc->IsExpandRefs();

        const ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
        bool bInDeleteUndo =
            ( pChangeTrack ? pChangeTrack->IsInDeleteUndo() : false );

        SCCOL oldCol1 = rRef.Ref1.nCol;
        SCROW oldRow1 = rRef.Ref1.nRow;
        SCTAB oldTab1 = rRef.Ref1.nTab;
        SCCOL oldCol2 = rRef.Ref2.nCol;
        SCROW oldRow2 = rRef.Ref2.nRow;
        SCTAB oldTab2 = rRef.Ref2.nTab;

        bool bRef1ColDel = rRef.Ref1.IsColDeleted();
        bool bRef2ColDel = rRef.Ref2.IsColDeleted();
        bool bRef1RowDel = rRef.Ref1.IsRowDeleted();
        bool bRef2RowDel = rRef.Ref2.IsRowDeleted();
        bool bRef1TabDel = rRef.Ref1.IsTabDeleted();
        bool bRef2TabDel = rRef.Ref2.IsTabDeleted();

        if( nDx &&
            ((rRef.Ref1.nRow >= nRow1
           && rRef.Ref2.nRow <= nRow2) || (bRef1RowDel || bRef2RowDel))
            &&
            ((rRef.Ref1.nTab >= nTab1
           && rRef.Ref2.nTab <= nTab2) || (bRef1TabDel || bRef2TabDel))
           )
        {
            bool bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nCol,
                rRef.Ref2.nCol, nCol1, nDx ));
            bool bDo1 = (eWhat == ScRefUpdate::ALL || (eWhat ==
                        ScRefUpdate::ABSOLUTE && !rRef.Ref1.IsColRel()));
            bool bDo2 = (eWhat == ScRefUpdate::ALL || (eWhat ==
                        ScRefUpdate::ABSOLUTE && !rRef.Ref2.IsColRel()));
            if ( lcl_MoveRefPart( rRef.Ref1.nCol, bRef1ColDel, bDo1,
                                  rRef.Ref2.nCol, bRef2ColDel, bDo2,
                                  nCol1, nCol2, nDx, MAXCOL ) )
            {
                eRet = UR_UPDATED;
                if ( bInDeleteUndo && (bRef1ColDel || bRef2ColDel) )
                {
                    if ( bRef1ColDel && nCol1 <= rRef.Ref1.nCol &&
                            rRef.Ref1.nCol <= nCol1 + nDx )
                        rRef.Ref1.SetColDeleted( false );
                    if ( bRef2ColDel && nCol1 <= rRef.Ref2.nCol &&
                            rRef.Ref2.nCol <= nCol1 + nDx )
                        rRef.Ref2.SetColDeleted( false );
                }
                else
                {
                    if ( bRef1ColDel )
                        rRef.Ref1.SetColDeleted( true );
                    if ( bRef2ColDel )
                        rRef.Ref2.SetColDeleted( true );
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
            bool bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nRow,
                rRef.Ref2.nRow, nRow1, nDy ));
            bool bDo1 = (eWhat == ScRefUpdate::ALL || (eWhat ==
                        ScRefUpdate::ABSOLUTE && !rRef.Ref1.IsRowRel()));
            bool bDo2 = (eWhat == ScRefUpdate::ALL || (eWhat ==
                        ScRefUpdate::ABSOLUTE && !rRef.Ref2.IsRowRel()));
            if ( lcl_MoveRefPart( rRef.Ref1.nRow, bRef1RowDel, bDo1,
                                rRef.Ref2.nRow, bRef2RowDel, bDo2,
                                nRow1, nRow2, nDy, MAXROW ) )
            {
                eRet = UR_UPDATED;
                if ( bInDeleteUndo && (bRef1RowDel || bRef2RowDel) )
                {
                    if ( bRef1RowDel && nRow1 <= rRef.Ref1.nRow &&
                            rRef.Ref1.nRow <= nRow1 + nDy )
                        rRef.Ref1.SetRowDeleted( false );
                    if ( bRef2RowDel && nRow1 <= rRef.Ref2.nRow &&
                            rRef.Ref2.nRow <= nRow1 + nDy )
                        rRef.Ref2.SetRowDeleted( false );
                }
                else
                {
                    if ( bRef1RowDel )
                        rRef.Ref1.SetRowDeleted( true );
                    if ( bRef2RowDel )
                        rRef.Ref2.SetRowDeleted( true );
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
            bool bExp = (bExpand && !bInDeleteUndo && IsExpand( rRef.Ref1.nTab,
                rRef.Ref2.nTab, nTab1, nDz ));
            SCTAB nMaxTab = pDoc->GetTableCount() - 1;
            bool bDo1 = (eWhat == ScRefUpdate::ALL || (eWhat ==
                        ScRefUpdate::ABSOLUTE && !rRef.Ref1.IsTabRel()));
            bool bDo2 = (eWhat == ScRefUpdate::ALL || (eWhat ==
                        ScRefUpdate::ABSOLUTE && !rRef.Ref2.IsTabRel()));
            if ( lcl_MoveRefPart( rRef.Ref1.nTab, bRef1TabDel, bDo1,
                                  rRef.Ref2.nTab, bRef2TabDel, bDo2,
                                  nTab1, nTab2, nDz, nMaxTab ) )
            {
                eRet = UR_UPDATED;
                if ( bInDeleteUndo && (bRef1TabDel || bRef2TabDel) )
                {
                    if ( bRef1TabDel && nTab1 <= rRef.Ref1.nTab &&
                            rRef.Ref1.nTab <= nTab1 + nDz )
                        rRef.Ref1.SetTabDeleted( false );
                    if ( bRef2TabDel && nTab1 <= rRef.Ref2.nTab &&
                            rRef.Ref2.nTab <= nTab1 + nDz )
                        rRef.Ref2.SetTabDeleted( false );
                }
                else
                {
                    if ( bRef1TabDel )
                        rRef.Ref1.SetTabDeleted( true );
                    if ( bRef2TabDel )
                        rRef.Ref2.SetTabDeleted( true );
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
        if (eWhat != ScRefUpdate::ABSOLUTE)
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
                eRet = Move( pDoc, rPos, nDx, nDy, nDz, rRef, false, true );        // immer verschieben
            }
            else if ( nDz && r.In( rPos ) )
            {
                rRef.Ref1.SetFlag3D( true );
                rRef.Ref2.SetFlag3D( true );
                eRet = UR_UPDATED;
                if (eWhat != ScRefUpdate::ABSOLUTE)
                    rRef.CalcRelFromAbs( rPos );
            }
            else if (eWhat != ScRefUpdate::ABSOLUTE)
                rRef.CalcRelFromAbs( rPos );
        }
        else if( eMode == URM_COPY && r.In( rPos ) )
            eRet = Move( pDoc, rPos, nDx, nDy, nDz, rRef, false, false );       // nur relative
            // sollte nicht mehr verwendet werden muessen
        else if (eWhat != ScRefUpdate::ABSOLUTE)
            rRef.CalcRelFromAbs( rPos );
    }
    return eRet;
}


ScRefUpdateRes ScRefUpdate::Move( ScDocument* pDoc, const ScAddress& rPos,
                                  SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                  ScComplexRefData& rRef, bool bWrap, bool bAbsolute )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    SCCOL oldCol1 = rRef.Ref1.nCol;
    SCROW oldRow1 = rRef.Ref1.nRow;
    SCTAB oldTab1 = rRef.Ref1.nTab;
    SCCOL oldCol2 = rRef.Ref2.nCol;
    SCROW oldRow2 = rRef.Ref2.nRow;
    SCTAB oldTab2 = rRef.Ref2.nTab;

    bool bCut1, bCut2;
    if ( nDx )
    {
        bCut1 = bCut2 = false;
        if( bAbsolute || rRef.Ref1.IsColRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( rRef.Ref1.nCol, nDx, MAXCOL );
            else
                bCut1 = lcl_MoveItCut( rRef.Ref1.nCol, nDx, MAXCOL );
        }
        if( bAbsolute || rRef.Ref2.IsColRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( rRef.Ref2.nCol, nDx, MAXCOL );
            else
                bCut2 = lcl_MoveItCut( rRef.Ref2.nCol, nDx, MAXCOL );
        }
        if ( bCut1 || bCut2 )
            eRet = UR_UPDATED;
        if ( bCut1 && bCut2 )
        {
            rRef.Ref1.SetColDeleted( true );
            rRef.Ref2.SetColDeleted( true );
        }
    }
    if ( nDy )
    {
        bCut1 = bCut2 = false;
        if( bAbsolute || rRef.Ref1.IsRowRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( rRef.Ref1.nRow, nDy, MAXROW );
            else
                bCut1 = lcl_MoveItCut( rRef.Ref1.nRow, nDy, MAXROW );
        }
        if( bAbsolute || rRef.Ref2.IsRowRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( rRef.Ref2.nRow, nDy, MAXROW );
            else
                bCut2 = lcl_MoveItCut( rRef.Ref2.nRow, nDy, MAXROW );
        }
        if ( bCut1 || bCut2 )
            eRet = UR_UPDATED;
        if ( bCut1 && bCut2 )
        {
            rRef.Ref1.SetRowDeleted( true );
            rRef.Ref2.SetRowDeleted( true );
        }
    }
    if ( nDz )
    {
        bCut1 = bCut2 = false;
        SCsTAB nMaxTab = (SCsTAB) pDoc->GetTableCount() - 1;
        if( bAbsolute || rRef.Ref1.IsTabRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( rRef.Ref1.nTab, nDz, static_cast<SCTAB>(nMaxTab) );
            else
                bCut1 = lcl_MoveItCut( rRef.Ref1.nTab, nDz, static_cast<SCTAB>(nMaxTab) );
            rRef.Ref1.SetFlag3D( rPos.Tab() != rRef.Ref1.nTab );
        }
        if( bAbsolute || rRef.Ref2.IsTabRel() )
        {
            if( bWrap )
                lcl_MoveItWrap( rRef.Ref2.nTab, nDz, static_cast<SCTAB>(nMaxTab) );
            else
                bCut2 = lcl_MoveItCut( rRef.Ref2.nTab, nDz, static_cast<SCTAB>(nMaxTab) );
            rRef.Ref2.SetFlag3D( rPos.Tab() != rRef.Ref2.nTab );
        }
        if ( bCut1 || bCut2 )
            eRet = UR_UPDATED;
        if ( bCut1 && bCut2 )
        {
            rRef.Ref1.SetTabDeleted( true );
            rRef.Ref2.SetTabDeleted( true );
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
                               SCCOL nMaxCol, SCROW nMaxRow, ScComplexRefData& rRef )
{
    if( rRef.Ref1.IsColRel() )
    {
        rRef.Ref1.nCol = rRef.Ref1.nRelCol + rPos.Col();
        lcl_MoveItWrap( rRef.Ref1.nCol, static_cast<SCsCOL>(0), nMaxCol );
    }
    if( rRef.Ref2.IsColRel() )
    {
        rRef.Ref2.nCol = rRef.Ref2.nRelCol + rPos.Col();
        lcl_MoveItWrap( rRef.Ref2.nCol, static_cast<SCsCOL>(0), nMaxCol );
    }
    if( rRef.Ref1.IsRowRel() )
    {
        rRef.Ref1.nRow = rRef.Ref1.nRelRow + rPos.Row();
        lcl_MoveItWrap( rRef.Ref1.nRow, static_cast<SCsROW>(0), nMaxRow );
    }
    if( rRef.Ref2.IsRowRel() )
    {
        rRef.Ref2.nRow = rRef.Ref2.nRelRow + rPos.Row();
        lcl_MoveItWrap( rRef.Ref2.nRow, static_cast<SCsROW>(0), nMaxRow );
    }
    SCsTAB nMaxTab = (SCsTAB) pDoc->GetTableCount() - 1;
    if( rRef.Ref1.IsTabRel() )
    {
        rRef.Ref1.nTab = rRef.Ref1.nRelTab + rPos.Tab();
        lcl_MoveItWrap( rRef.Ref1.nTab, static_cast<SCsTAB>(0), static_cast<SCTAB>(nMaxTab) );
    }
    if( rRef.Ref2.IsTabRel() )
    {
        rRef.Ref2.nTab = rRef.Ref2.nRelTab + rPos.Tab();
        lcl_MoveItWrap( rRef.Ref2.nTab, static_cast<SCsTAB>(0), static_cast<SCTAB>(nMaxTab) );
    }
    rRef.PutInOrder();
    rRef.CalcRelFromAbs( rPos );
}

//------------------------------------------------------------------

void ScRefUpdate::DoTranspose( SCsCOL& rCol, SCsROW& rRow, SCsTAB& rTab,
                        ScDocument* pDoc, const ScRange& rSource, const ScAddress& rDest )
{
    SCsTAB nDz = ((SCsTAB)rDest.Tab())-(SCsTAB)rSource.aStart.Tab();
    if (nDz)
    {
        SCsTAB nNewTab = rTab+nDz;
        SCsTAB nCount = pDoc->GetTableCount();
        while (nNewTab<0) nNewTab = sal::static_int_cast<SCsTAB>( nNewTab + nCount );
        while (nNewTab>=nCount) nNewTab = sal::static_int_cast<SCsTAB>( nNewTab - nCount );
        rTab = nNewTab;
    }
    OSL_ENSURE( rCol>=rSource.aStart.Col() && rRow>=rSource.aStart.Row(),
                "UpdateTranspose: Pos. falsch" );

    SCsCOL nRelX = rCol - (SCsCOL)rSource.aStart.Col();
    SCsROW nRelY = rRow - (SCsROW)rSource.aStart.Row();

    rCol = static_cast<SCsCOL>(static_cast<SCsCOLROW>(rDest.Col()) +
            static_cast<SCsCOLROW>(nRelY));
    rRow = static_cast<SCsROW>(static_cast<SCsCOLROW>(rDest.Row()) +
            static_cast<SCsCOLROW>(nRelX));
}

ScRefUpdateRes ScRefUpdate::UpdateTranspose(
    ScDocument* pDoc, const ScRange& rSource, const ScAddress& rDest, ScRange& rRef )
{
    ScRefUpdateRes eRet = UR_NOTHING;
    if (rRef.aStart.Col() >= rSource.aStart.Col() && rRef.aEnd.Col() <= rSource.aEnd.Col() &&
        rRef.aStart.Row() >= rSource.aStart.Row() && rRef.aEnd.Row() <= rSource.aEnd.Row() &&
        rRef.aStart.Tab() >= rSource.aStart.Tab() && rRef.aEnd.Tab() <= rSource.aEnd.Tab())
    {
        // Source range contains the reference range.
        SCCOL nCol1 = rRef.aStart.Col(), nCol2 = rRef.aEnd.Col();
        SCROW nRow1 = rRef.aStart.Row(), nRow2 = rRef.aEnd.Row();
        SCTAB nTab1 = rRef.aStart.Tab(), nTab2 = rRef.aEnd.Tab();
        DoTranspose(nCol1, nRow1, nTab1, pDoc, rSource, rDest);
        DoTranspose(nCol2, nRow2, nTab2, pDoc, rSource, rDest);
        rRef.aStart = ScAddress(nCol1, nRow1, nTab1);
        rRef.aEnd = ScAddress(nCol2, nRow2, nTab2);
        eRet = UR_UPDATED;
    }
    return eRet;
}

//------------------------------------------------------------------

//  UpdateGrow - erweitert Referenzen, die genau auf den Bereich zeigen
//  kommt ohne Dokument aus


ScRefUpdateRes ScRefUpdate::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY,
                                        ScComplexRefData& rRef )
{
    ScRefUpdateRes eRet = UR_NOTHING;

    //  in Y-Richtung darf die Ref auch eine Zeile weiter unten anfangen,
    //  falls ein Bereich Spaltenkoepfe enthaelt

    bool bUpdateX = ( nGrowX &&
            rRef.Ref1.nCol == rArea.aStart.Col() && rRef.Ref2.nCol == rArea.aEnd.Col() &&
            rRef.Ref1.nRow >= rArea.aStart.Row() && rRef.Ref2.nRow <= rArea.aEnd.Row() &&
            rRef.Ref1.nTab >= rArea.aStart.Tab() && rRef.Ref2.nTab <= rArea.aEnd.Tab() );
    bool bUpdateY = ( nGrowY &&
            rRef.Ref1.nCol >= rArea.aStart.Col() && rRef.Ref2.nCol <= rArea.aEnd.Col() &&
            ( rRef.Ref1.nRow == rArea.aStart.Row() || rRef.Ref1.nRow == rArea.aStart.Row()+1 ) &&
                rRef.Ref2.nRow == rArea.aEnd.Row() &&
            rRef.Ref1.nTab >= rArea.aStart.Tab() && rRef.Ref2.nTab <= rArea.aEnd.Tab() );

    if ( bUpdateX )
    {
        rRef.Ref2.nCol = sal::static_int_cast<SCsCOL>( rRef.Ref2.nCol + nGrowX );
        eRet = UR_UPDATED;
    }
    if ( bUpdateY )
    {
        rRef.Ref2.nRow = sal::static_int_cast<SCsROW>( rRef.Ref2.nRow + nGrowY );
        eRet = UR_UPDATED;
    }

    return eRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
