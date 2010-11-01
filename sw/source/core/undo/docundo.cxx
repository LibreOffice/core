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
#include "precompiled_sw.hxx"

#include <svx/svdmodel.hxx>

#include <vcl/wrkwin.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>       // fuer die UndoIds
#include <undobj.hxx>
#include <rolbck.hxx>
#include <docary.hxx>
#include <undo.hrc>


using namespace ::com::sun::star;


USHORT SwDoc::nUndoActions = UNDO_ACTION_COUNT;     // anzahl von Undo-Action

// the undo array should never grow beyond this limit:
#define UNDO_ACTION_LIMIT (USHRT_MAX - 1000)


SV_IMPL_PTRARR( SwUndoIds, SwUndoIdAndNamePtr )

//#define _SHOW_UNDORANGE
#ifdef _SHOW_UNDORANGE


class UndoArrStatus : public WorkWindow
{
    USHORT nUndo, nUndoNds;
    virtual void Paint( const Rectangle& );
public:
    UndoArrStatus();
    void Set( USHORT, USHORT );
};
static UndoArrStatus* pUndoMsgWin = 0;


UndoArrStatus::UndoArrStatus()
    : WorkWindow( APP_GETAPPWINDOW() ), nUndo(0), nUndoNds(0)
{
    SetSizePixel( Size( 200, 100 ));
    SetFont( Font( "Courier", Size( 0, 10 )) );
    Show();
}


void UndoArrStatus::Set( USHORT n1, USHORT n2 )
{
    nUndo = n1; nUndoNds = n2;
    Invalidate();
}


void UndoArrStatus::Paint( const Rectangle& )
{
    String s;
    DrawRect( Rectangle( Point(0,0), GetOutputSize() ));
    ( s = "Undos: " ) += nUndo;
    DrawText( Point( 0, 0 ), s );
    ( s = "UndoNodes: " ) += nUndoNds;
    DrawText( Point( 0, 15 ), s );
}

#endif

void SwDoc::SetUndoNoResetModified()
{
    nUndoSavePos = USHRT_MAX;
}

bool SwDoc::IsUndoNoResetModified() const
{
    return USHRT_MAX == nUndoSavePos;
}

void SwDoc::DoUndo(bool bUn)
{
    mbUndo = bUn;

    SdrModel* pSdrModel = GetDrawModel();
    if( pSdrModel )
        pSdrModel->EnableUndo(bUn);
}

bool SwDoc::DoesUndo() const
{
    return mbUndo;
}

void SwDoc::DoGroupUndo(bool bUn)
{
    mbGroupUndo = bUn;
}

bool SwDoc::DoesGroupUndo() const
{
    return mbGroupUndo;
}

sal_uInt16 SwDoc::GetUndoActionCount()
{
    return nUndoActions;
}

void SwDoc::SetUndoActionCount( sal_uInt16 nNew )
{
    nUndoActions = nNew;
}

const SwNodes* SwDoc::GetUndoNds() const
{
    return &aUndoNodes;
}

void SwDoc::AppendUndo( SwUndo* pUndo )
{
    if( nsRedlineMode_t::REDLINE_NONE == pUndo->GetRedlineMode() )
        pUndo->SetRedlineMode( GetRedlineMode() );

    // Unfortunately, the silly SvPtrArr can only store a little less than
    // USHRT_MAX elements. Of course it doesn't see any necessity for asserting
    // or even doing error handling. pUndos should definitely be replaced by an
    // STL container that doesn't have this problem. cf #95884#
    DBG_ASSERT( pUndos->Count() < USHRT_MAX - 16,
                "Writer will crash soon. I apologize for the inconvenience." );

    pUndos->Insert( pUndo, nUndoPos );
    ++nUndoPos;
    switch( pUndo->GetId() )
    {
    case UNDO_START:        ++nUndoSttEnd;
                            break;

    case UNDO_END:          ASSERT( nUndoSttEnd, "Undo-Ende ohne Start" );
                            --nUndoSttEnd;
                            // kein break !!!
    default:
        if( pUndos->Count() != nUndoPos && UNDO_END != pUndo->GetId() )
            ClearRedo();
        else {
            ASSERT( pUndos->Count() == nUndoPos || UNDO_END == pUndo->GetId(),
                    "Redo history not deleted!" );
        }
        if( !nUndoSttEnd )
            ++nUndoCnt;
        break;
    }

#ifdef _SHOW_UNDORANGE
    // zur Anzeige der aktuellen Undo-Groessen
    if( !pUndoMsgWin )
            pUndoMsgWin = new UndoArrStatus;
    pUndoMsgWin->Set( pUndos->Count(), aUndoNodes.Count() );
#endif

    // noch eine offene Klammerung, kann man sich den Rest schenken
    if( nUndoSttEnd )
        return;

    // folgende Array-Grenzen muessen ueberwacht werden:
    //  - Undo,             Grenze: fester Wert oder USHRT_MAX - 1000
    //  - UndoNodes,        Grenze:  USHRT_MAX - 1000
    //  - AttrHistory       Grenze:  USHRT_MAX - 1000
    // (defined in UNDO_ACTION_LIMIT at the top of this file)

    USHORT nEnde = UNDO_ACTION_LIMIT;

// nur zum Testen der neuen DOC-Member
#ifdef DBG_UTIL
{
    SwUndoId nId = UNDO_EMPTY;
    USHORT nUndosCnt = 0, nSttEndCnt = 0;
    for( USHORT nCnt = 0; nCnt < nUndoPos; ++nCnt )
    {
        if( UNDO_START == ( nId = (*pUndos)[ nCnt ]->GetId()) )
            ++nSttEndCnt;
        else if( UNDO_END == nId )
            --nSttEndCnt;
        if( !nSttEndCnt )
            ++nUndosCnt;
    }
    ASSERT( nSttEndCnt == nUndoSttEnd, "Start-Ende Count ungleich" );
    ASSERT( nUndosCnt == nUndoCnt, "Undo Count ungleich" );
}
#endif

    if( SwDoc::nUndoActions < nUndoCnt )
        // immer 1/10 loeschen
        //JP 23.09.95: oder wenn neu eingestellt wurde um die Differenz
        //JP 29.5.2001: Task #83891#: remove only the overlapping actions
        DelUndoObj( nUndoCnt - SwDoc::nUndoActions );
    else
    {
        USHORT nUndosCnt = nUndoCnt;
            // immer 1/10 loeschen bis der "Ausloeser" behoben ist
        while( aUndoNodes.Count() && nEnde < aUndoNodes.Count() )
            DelUndoObj( nUndosCnt / 10 );
    }
}



void SwDoc::ClearRedo()
{
    if( DoesUndo() && nUndoPos != pUndos->Count() )
    {
//?? why ??     if( !nUndoSttEnd )
        {
            // setze UndoCnt auf den neuen Wert
            SwUndo* pUndo;
            for( USHORT nCnt = pUndos->Count(); nUndoPos < nCnt; --nUndoCnt )
                // Klammerung ueberspringen
                if( UNDO_END == (pUndo = (*pUndos)[ --nCnt ])->GetId() )
                    nCnt = nCnt - ((SwUndoEnd*)pUndo)->GetSttOffset();
        }

        // loesche die Undo-Aktionen (immer von hinten !)
        pUndos->DeleteAndDestroy( nUndoPos, pUndos->Count() - nUndoPos );
    }
}


    // loescht die gesamten UndoObjecte
void SwDoc::DelAllUndoObj()
{
    ClearRedo();

    DoUndo( FALSE );

    // Offene Undo-Klammerungen erhalten !!
    SwUndo* pUndo;
    USHORT nSize = pUndos->Count();
    while( nSize )
        if( UNDO_START != ( pUndo = (*pUndos)[ --nSize ] )->GetId() ||
            ((SwUndoStart*)pUndo)->GetEndOffset() )
            // keine offenen Gruppierung ?
            pUndos->DeleteAndDestroy( nSize, 1 );

    nUndoCnt = 0;
    nUndoPos = pUndos->Count();

/*
    while( nUndoPos )
        aUndos.DelDtor( --nUndoPos, 1 );
    nUndoCnt = nUndoSttEnd = nUndoPos = 0;
*/
    nUndoSavePos = USHRT_MAX;
    DoUndo( TRUE );
}


    // loescht alle UndoObjecte vom Anfang bis zum angegebenen Ende
BOOL SwDoc::DelUndoObj( USHORT nEnde )
{
    if( !nEnde )                    // sollte mal 0 uebergeben werden,
    {
        if( !pUndos->Count() )
            return FALSE;
        ++nEnde;                    // dann korrigiere es auf 1
    }

    DoUndo( FALSE );

    // pruefe erstmal, wo das Ende steht
    SwUndoId nId = UNDO_EMPTY;
    USHORT nSttEndCnt = 0;
    USHORT nCnt;

    for( nCnt = 0; nEnde && nCnt < nUndoPos; ++nCnt )
    {
        if( UNDO_START == ( nId = (*pUndos)[ nCnt ]->GetId() ))
            ++nSttEndCnt;
        else if( UNDO_END == nId )
            --nSttEndCnt;
        if( !nSttEndCnt )
            --nEnde, --nUndoCnt;
    }

    ASSERT( nCnt < nUndoPos || nUndoPos == pUndos->Count(),
            "Undo-Del-Ende liegt in einer Redo-Aktion" );

    // dann setze ab Ende bis Undo-Ende bei allen Undo-Objecte die Werte um
    nSttEndCnt = nCnt;          // Position merken
    if( nUndoSavePos < nSttEndCnt )     // SavePos wird aufgegeben
        nUndoSavePos = USHRT_MAX;
    else if( nUndoSavePos != USHRT_MAX )
        nUndoSavePos = nUndoSavePos - nSttEndCnt;

    while( nSttEndCnt )
        pUndos->DeleteAndDestroy( --nSttEndCnt, 1 );
    nUndoPos = pUndos->Count();

    DoUndo( TRUE );
    return TRUE;
}

/**************** UNDO ******************/

void SwDoc::setUndoNoModifiedPosition( SwUndoNoModifiedPosition nNew )
{
    nUndoSavePos = nNew;
    if( !pUndos->Count() || nUndoSavePos > pUndos->Count() - 1 )
        nUndoSavePos = USHRT_MAX;
}

SwUndoNoModifiedPosition SwDoc::getUndoNoModifiedPosition() const
{
    return nUndoSavePos;
}


bool SwDoc::HasUndoId(SwUndoId eId) const
{
    USHORT nSize = nUndoPos;
    SwUndo * pUndo;
    while( nSize-- )
        if( ( pUndo = (*pUndos)[nSize])->GetId() == eId ||
            ( UNDO_START == pUndo->GetId() &&
                ((SwUndoStart*)pUndo)->GetUserId() == eId )
            || ( UNDO_END == pUndo->GetId() &&
                ((SwUndoEnd*)pUndo)->GetUserId() == eId ) )
        {
            return TRUE;
        }

    return FALSE;
}


bool SwDoc::Undo( SwUndoIter& rUndoIter )
{
    if ( (rUndoIter.GetId()!=0) && (!HasUndoId(rUndoIter.GetId())) )
    {
        rUndoIter.bWeiter = FALSE;
        return FALSE;
    }
    if( !nUndoPos )
    {
        rUndoIter.bWeiter = FALSE;
        return FALSE;
    }

    SwUndo *pUndo = (*pUndos)[ --nUndoPos ];

    RedlineMode_t eOld = GetRedlineMode();
    RedlineMode_t eTmpMode = (RedlineMode_t)pUndo->GetRedlineMode();
    if( (nsRedlineMode_t::REDLINE_SHOW_MASK & eTmpMode) != (nsRedlineMode_t::REDLINE_SHOW_MASK & eOld) &&
        UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
        SetRedlineMode( eTmpMode );

    SetRedlineMode_intern((RedlineMode_t)(eTmpMode | nsRedlineMode_t::REDLINE_IGNORE));
    // Undo ausfuehren

    // zum spaeteren ueberpruefen
    SwUndoId nAktId = pUndo->GetId();
    //JP 11.05.98: FlyFormate ueber die EditShell selektieren, nicht aus dem
    //              Undo heraus
    switch( nAktId )
    {
    case UNDO_START:
    case UNDO_END:
    case UNDO_INSDRAWFMT:
        break;

    default:
        rUndoIter.ClearSelections();
    }

    pUndo->Undo( rUndoIter );

    SetRedlineMode( eOld );

    // Besonderheit von Undo-Replace (interne History)
    if( UNDO_REPLACE == nAktId && ((SwUndoReplace*)pUndo)->nAktPos )
    {
        ++nUndoPos;
        return TRUE;
    }

    // Objekt aus History entfernen und zerstoeren
    if( nUndoPos && !rUndoIter.bWeiter &&
        UNDO_START == ( pUndo = (*pUndos)[ nUndoPos-1 ] )->GetId() )
        --nUndoPos;

    // JP 29.10.96: Start und End setzen kein Modify-Flag.
    //              Sonst gibt es Probleme mit der autom. Aufnahme von Ausnahmen
    //              bei der Autokorrektur
    if( UNDO_START != nAktId && UNDO_END != nAktId )
        SetModified();      // default: immer setzen, kann zurueck gesetzt werden

    // ist die History leer und wurde nicht wegen Speichermangel
    // verworfen, so kann das Dokument als unveraendert gelten
    if( nUndoSavePos == nUndoPos )
        ResetModified();

    return TRUE;
}


// setzt Undoklammerung auf, liefert nUndoId der Klammerung


SwUndoId SwDoc::StartUndo( SwUndoId eUndoId, const SwRewriter * pRewriter )
{
    if( !mbUndo )
        return UNDO_EMPTY;

    if( !eUndoId )
        eUndoId = UNDO_START;

    SwUndoStart * pUndo = new SwUndoStart( eUndoId );

    if (pRewriter)
        pUndo->SetRewriter(*pRewriter);

    AppendUndo(pUndo);

    return eUndoId;
}
// schliesst Klammerung der nUndoId, nicht vom UI benutzt


SwUndoId SwDoc::EndUndo(SwUndoId eUndoId, const SwRewriter * pRewriter)
{
    USHORT nSize = nUndoPos;
    if( !mbUndo || !nSize-- )
        return UNDO_EMPTY;

    if( UNDO_START == eUndoId || !eUndoId )
        eUndoId = UNDO_END;

    SwUndo* pUndo = (*pUndos)[ nSize ];
    if( UNDO_START == pUndo->GetId() )
    {
        // leere Start/End-Klammerung ??
        pUndos->DeleteAndDestroy( nSize );
        --nUndoPos;
        --nUndoSttEnd;
        return UNDO_EMPTY;
    }

    // exist above any redo objects? If yes, delete them
    if( nUndoPos != pUndos->Count() )
    {
        // setze UndoCnt auf den neuen Wert
        for( USHORT nCnt = pUndos->Count(); nUndoPos < nCnt; --nUndoCnt )
            // Klammerung ueberspringen
            if( UNDO_END == (pUndo = (*pUndos)[ --nCnt ])->GetId() )
                nCnt = nCnt - ((SwUndoEnd*)pUndo)->GetSttOffset();

        pUndos->DeleteAndDestroy( nUndoPos, pUndos->Count() - nUndoPos );
    }

    // suche den Anfang dieser Klammerung
    SwUndoId nId = UNDO_EMPTY;
    while( nSize )
        if( UNDO_START == ( nId = (pUndo = (*pUndos)[ --nSize ] )->GetId()) &&
            !((SwUndoStart*)pUndo)->GetEndOffset() )
            break;      // Start gefunden

    if( nId != UNDO_START )
    {
        // kann eigentlich nur beim Abspielen von Macros passieren, die
        // Undo/Redo/Repeat benutzen und die eine exitierende Selection
        // durch Einfuegen loeschen
        ASSERT( !this, "kein entsprechendes Ende gefunden" );
        // kein entsprechenden Start gefunden -> Ende nicht einfuegen
        // und die Member am Doc updaten

        nUndoSttEnd = 0;
        nUndoCnt = 0;
        // setze UndoCnt auf den neuen Wert
        SwUndo* pTmpUndo;
        for( USHORT nCnt = 0; nCnt < pUndos->Count(); ++nCnt, ++nUndoCnt )
            // Klammerung ueberspringen
            if( UNDO_START == (pTmpUndo = (*pUndos)[ nCnt ])->GetId() )
                nCnt = nCnt + ((SwUndoStart*)pTmpUndo)->GetEndOffset();
        return UNDO_EMPTY;

    }

    // Klammerung um eine einzelne Action muss nicht sein!
    // Aussnahme: es ist eine eigene ID definiert
    if(  2 == pUndos->Count() - nSize &&
        (UNDO_END == eUndoId || eUndoId == (*pUndos)[ nSize+1 ]->GetId() ))
    {
        pUndos->DeleteAndDestroy( nSize );
        nUndoPos = pUndos->Count();
        if( !--nUndoSttEnd )
        {
            ++nUndoCnt;
            if( SwDoc::nUndoActions < nUndoCnt )
                // immer 1/10 loeschen
                //JP 23.09.95: oder wenn neu eingestellt wurde um die Differenz
                //JP 29.5.2001: Task #83891#: remove only the overlapping actions
                DelUndoObj( nUndoCnt - SwDoc::nUndoActions );
            else
            {
                USHORT nEnde = USHRT_MAX - 1000;
                USHORT nUndosCnt = nUndoCnt;
                    // immer 1/10 loeschen bis der "Ausloeser" behoben ist
                while( aUndoNodes.Count() && nEnde < aUndoNodes.Count() )
                    DelUndoObj( nUndosCnt / 10 );
            }
        }
        return eUndoId;
    }

    // setze die Klammerung am Start/End-Undo
    nSize = pUndos->Count() - nSize;
    ((SwUndoStart*)pUndo)->SetEndOffset( nSize );

    SwUndoEnd* pUndoEnd = new SwUndoEnd( eUndoId );
    pUndoEnd->SetSttOffset( nSize );

// nur zum Testen der Start/End-Verpointerung vom Start/End Undo
#ifdef DBG_UTIL
    {
        USHORT nEndCnt = 1, nCnt = pUndos->Count();
        SwUndoId nTmpId = UNDO_EMPTY;
        while( nCnt )
        {
            if( UNDO_START == ( nTmpId = (*pUndos)[ --nCnt ]->GetId()) )
            {
                if( !nEndCnt ) // falls mal ein Start ohne Ende vorhanden ist
                    continue;
                --nEndCnt;
                if( !nEndCnt )      // hier ist der Anfang
                    break;
            }
            else if( UNDO_END == nTmpId )
                ++nEndCnt;
            else if( !nEndCnt )
                break;
        }
        ASSERT( nCnt == pUndos->Count() - nSize,
                "Start-Ende falsch geklammert" );
    }
#endif

    if (pRewriter)
    {
        ((SwUndoStart *) pUndo)->SetRewriter(*pRewriter);
        pUndoEnd->SetRewriter(*pRewriter);
    }
    else
        pUndoEnd->SetRewriter(((SwUndoStart *) pUndo)->GetRewriter());

    AppendUndo( pUndoEnd );
    return eUndoId;
}

// liefert die Id der letzten Undofaehigen Aktion zurueck oder 0
// fuellt ggf. VARARR mit User-UndoIds

String SwDoc::GetUndoIdsStr( String* pStr, SwUndoIds *pUndoIds) const
{
    String aTmpStr;

    if (pStr != NULL)
    {
        GetUndoIds( pStr, pUndoIds);
        aTmpStr = *pStr;
    }
    else
        GetUndoIds( &aTmpStr, pUndoIds);

    return aTmpStr;
}

/*-- 24.11.2004 16:11:21---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwDoc::RestoreInvisibleContent()
{
    sal_Bool bRet = sal_False;
    if(nUndoPos > 0 )
    {
        SwUndo * pUndo = (*pUndos)[ nUndoPos - 1 ];
        if( ( pUndo->GetId() == UNDO_END &&
            static_cast<SwUndoEnd *>(pUndo)->GetUserId() == UNDO_UI_DELETE_INVISIBLECNTNT) )
        {
            SwPaM aPam( GetNodes().GetEndOfPostIts() );
            SwUndoIter aUndoIter( &aPam );
            do
            {
                Undo( aUndoIter );
            }
            while ( aUndoIter.IsNextUndo() );
            ClearRedo();
            bRet = sal_True;
        }
    }
    return bRet;
}


/**
   Returns id and comment for a certain undo object in an undo stack.

   Remark: In the following the object type referred to is always the
   effective object type. If an UNDO_START or UNDO_END has a user type
   it is referred to as this type.

   If the queried object is an UNDO_END and has no user id the result
   is taken from the first object that is not an UNDO_END nor an
   UNDO_START preceeding the queried object.

   If the queried object is an UNDO_START and has no user id the
   result is taken from the first object that is not an UNDO_END nor
   an UNDO_START preceeding the UNDO_END object belonging to the
   queried object.

   In all other cases the result is taken from the queried object.

   @param rUndos           the undo stack
   @param nPos             position of the undo object to query

   @return SwUndoIdAndName object containing the query result
 */
SwUndoIdAndName * lcl_GetUndoIdAndName(const SwUndos & rUndos, sal_uInt16 nPos )
{
    SwUndo * pUndo = rUndos[ nPos ];
    SwUndoId nId = UNDO_EMPTY;
    String sStr("??", RTL_TEXTENCODING_ASCII_US);

    ASSERT( nPos < rUndos.Count(), "nPos out of range");

    switch (pUndo->GetId())
    {
    case UNDO_START:
        {
            SwUndoStart * pUndoStart = (SwUndoStart *) pUndo;
            nId = pUndoStart->GetUserId();

            if (nId <= UNDO_END)
            {
                /**
                   Start at the according UNDO_END.  Search backwards
                   for first objects that is not a UNDO_END.
                 */
                int nTmpPos = nPos + pUndoStart->GetEndOffset();
                int nSubstitute = -1;

                // --> OD 2009-09-30 #i105457#
                if ( nTmpPos > 0 )
                // <--
                {
                    SwUndo * pTmpUndo;
                    do
                    {
                        nTmpPos--;
                        pTmpUndo = rUndos[ static_cast<USHORT>(nTmpPos) ];

                        if (pTmpUndo->GetEffectiveId() > UNDO_END)
                            nSubstitute = nTmpPos;
                    }
                    while (nSubstitute < 0 && nTmpPos > nPos);

                    if (nSubstitute >= 0)
                    {
                        SwUndo * pSubUndo = rUndos[ static_cast<USHORT>(nSubstitute) ];
                        nId = pSubUndo->GetEffectiveId();
                        sStr = pSubUndo->GetComment();
                    }
                }
            }
            else
                sStr = pUndo->GetComment();
        }

        break;

    case UNDO_END:
        {
            SwUndoEnd * pUndoEnd = (SwUndoEnd *) pUndo;
            nId = pUndoEnd->GetUserId();

            if (nId <= UNDO_END)
            {
                /**
                   Start at this UNDO_END.  Search backwards
                   for first objects that is not a UNDO_END.
                 */

                int nTmpPos = nPos;
                int nUndoStart = nTmpPos - pUndoEnd->GetSttOffset();
                int nSubstitute = -1;

                if (nTmpPos > 0)
                {
                    SwUndo * pTmpUndo;

                    do
                    {
                        nTmpPos--;
                        pTmpUndo = rUndos[ static_cast<USHORT>(nTmpPos) ];

                        if (pTmpUndo->GetEffectiveId() > UNDO_END)
                            nSubstitute = nTmpPos;
                    }
                    while (nSubstitute < 0 && nTmpPos > nUndoStart);

                    if (nSubstitute >= 0)
                    {
                        SwUndo * pSubUndo = rUndos[ static_cast<USHORT>(nSubstitute) ];
                        nId = pSubUndo->GetEffectiveId();
                        sStr = pSubUndo->GetComment();
                    }
                }
            }
            else
                sStr = pUndo->GetComment();
        }

        break;

    default:
        nId = pUndo->GetId();
        sStr = pUndo->GetComment();
    }

    return new SwUndoIdAndName(nId, &sStr);
}

SwUndoId SwDoc::GetUndoIds( String* pStr, SwUndoIds *pUndoIds) const
{
    int nTmpPos = nUndoPos - 1;
    SwUndoId nId = UNDO_EMPTY;

    while (nTmpPos >= 0)
    {
        SwUndo * pUndo = (*pUndos)[ static_cast<USHORT>(nTmpPos) ];

        SwUndoIdAndName * pIdAndName = lcl_GetUndoIdAndName( *pUndos, static_cast<sal_uInt16>(nTmpPos) );

        if (nTmpPos == nUndoPos - 1)
        {
            nId = pIdAndName->GetUndoId();

            if (pStr)
                *pStr = *pIdAndName->GetUndoStr();
        }

        if (pUndoIds)
            pUndoIds->Insert(pIdAndName, pUndoIds->Count());
        else
            break;

        if (pUndo->GetId() == UNDO_END)
            nTmpPos -= ((SwUndoEnd *) pUndo)->GetSttOffset();

        nTmpPos--;
    }

    return nId;
}

bool SwDoc::HasTooManyUndos() const
{
    // AppendUndo checks the UNDO_ACTION_LIMIT, unless there's a nested undo.
    // So HasTooManyUndos() may only occur when undos are nested; else
    // AppendUndo has some sort of bug.
    DBG_ASSERT( (nUndoSttEnd != 0) || (pUndos->Count() < UNDO_ACTION_LIMIT),
                "non-nested undos should have been handled in AppendUndo" );
    return (pUndos->Count() >= UNDO_ACTION_LIMIT);
}


/**************** REDO ******************/


bool SwDoc::Redo( SwUndoIter& rUndoIter )
{
    if( rUndoIter.GetId() && !HasUndoId( rUndoIter.GetId() ) )
    {
        rUndoIter.bWeiter = FALSE;
        return FALSE;
    }
    if( nUndoPos == pUndos->Count() )
    {
        rUndoIter.bWeiter = FALSE;
        return FALSE;
    }

    SwUndo *pUndo = (*pUndos)[ nUndoPos++ ];

    RedlineMode_t eOld = GetRedlineMode();
    RedlineMode_t eTmpMode = (RedlineMode_t)pUndo->GetRedlineMode();
    if( (nsRedlineMode_t::REDLINE_SHOW_MASK & eTmpMode) != (nsRedlineMode_t::REDLINE_SHOW_MASK & eOld) &&
        UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
        SetRedlineMode( eTmpMode );
    SetRedlineMode_intern( (RedlineMode_t)(eTmpMode | nsRedlineMode_t::REDLINE_IGNORE));

    //JP 11.05.98: FlyFormate ueber die EditShell selektieren, nicht aus dem
    //              Undo heraus
    if( UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
        rUndoIter.ClearSelections();

    pUndo->Redo( rUndoIter );

    SetRedlineMode( eOld );

    // Besonderheit von Undo-Replace (interne History)
    if( UNDO_REPLACE == pUndo->GetId() &&
        USHRT_MAX != ((SwUndoReplace*)pUndo)->nAktPos )
    {
        --nUndoPos;
        return TRUE;
    }

    if( rUndoIter.bWeiter && nUndoPos >= pUndos->Count() )
        rUndoIter.bWeiter = FALSE;

    // ist die History leer und wurde nicht wegen Speichermangel
    // verworfen, so kann das Dokument als unveraendert gelten
    if( nUndoSavePos == nUndoPos )
        ResetModified();
    else
        SetModified();
    return TRUE;
}


// liefert die Id der letzten Redofaehigen Aktion zurueck oder 0
// fuellt ggf. VARARR mit User-RedoIds

String SwDoc::GetRedoIdsStr( String* pStr, SwUndoIds *pRedoIds ) const
{
    String aTmpStr;

    if (pStr != NULL)
    {
        GetRedoIds( pStr, pRedoIds );
        aTmpStr = *pStr;
    }
    else
        GetRedoIds( &aTmpStr, pRedoIds );


    return aTmpStr;
}


SwUndoId SwDoc::GetRedoIds( String* pStr, SwUndoIds *pRedoIds ) const
{
    sal_uInt16 nTmpPos = nUndoPos;
    SwUndoId nId = UNDO_EMPTY;

    while (nTmpPos < pUndos->Count())
    {
        SwUndo * pUndo = (*pUndos)[nTmpPos];

        SwUndoIdAndName * pIdAndName = lcl_GetUndoIdAndName(*pUndos, nTmpPos);

        if (nTmpPos == nUndoPos)
        {
            nId = pIdAndName->GetUndoId();

            if (pStr)
                *pStr = *pIdAndName->GetUndoStr();
        }

        if (pRedoIds)
            pRedoIds->Insert(pIdAndName, pRedoIds->Count());
        else
            break;

        if (pUndo->GetId() == UNDO_START)
            nTmpPos = nTmpPos + ((SwUndoStart *) pUndo)->GetEndOffset();

        nTmpPos++;
    }

    return nId;
}

/**************** REPEAT ******************/


bool SwDoc::Repeat( SwUndoIter& rUndoIter, sal_uInt16 nRepeatCnt )
{
    if( rUndoIter.GetId() && !HasUndoId( rUndoIter.GetId() ) )
    {
        rUndoIter.bWeiter = FALSE;
        return FALSE;
    }
    USHORT nSize = nUndoPos;
    if( !nSize )
    {
        rUndoIter.bWeiter = FALSE;
        return FALSE;
    }

    // dann suche jetzt ueber die End/Start-Gruppen die gueltige Repeat-Aktion
    SwUndo *pUndo = (*pUndos)[ --nSize ];
    if( UNDO_END == pUndo->GetId() )
        nSize = nSize - ((SwUndoEnd*)pUndo)->GetSttOffset();

    USHORT nEndCnt = nUndoPos;
    BOOL bOneUndo = nSize + 1 == nUndoPos;

    SwPaM* pTmpCrsr = rUndoIter.pAktPam;
    SwUndoId nId = UNDO_EMPTY;

    if( pTmpCrsr != pTmpCrsr->GetNext() || !bOneUndo )  // Undo-Klammerung aufbauen
    {
        if (pUndo->GetId() == UNDO_END)
        {
            SwUndoStart * pStartUndo =
                (SwUndoStart *) (*pUndos)[nSize];

            nId = pStartUndo->GetUserId();
        }

        StartUndo( nId, NULL );
    }
    do {        // dann durchlaufe mal den gesamten Ring
        for( USHORT nRptCnt = nRepeatCnt; nRptCnt > 0; --nRptCnt )
        {
            rUndoIter.pLastUndoObj = 0;
            for( USHORT nCnt = nSize; nCnt < nEndCnt; ++nCnt )
                (*pUndos)[ nCnt ]->Repeat( rUndoIter );     // Repeat ausfuehren
        }
    } while( pTmpCrsr !=
        ( rUndoIter.pAktPam = (SwPaM*)rUndoIter.pAktPam->GetNext() ));
    if( pTmpCrsr != pTmpCrsr->GetNext() || !bOneUndo )
        EndUndo( nId, NULL );

    return TRUE;
}

// liefert die Id der letzten Repeatfaehigen Aktion zurueck oder 0
// fuellt ggf. VARARR mit User-RedoIds

String SwDoc::GetRepeatIdsStr(String* pStr, SwUndoIds *pRepeatIds) const
{
    String aTmpStr;
    SwUndoId nId;

    if ( pStr != NULL)
    {
        nId = GetRepeatIds(pStr, pRepeatIds);
        aTmpStr = *pStr;
    }
    else
        nId = GetRepeatIds(&aTmpStr, pRepeatIds);

    if (nId <= UNDO_END)
        return String();

    return aTmpStr;
}

SwUndoId SwDoc::GetRepeatIds(String* pStr, SwUndoIds *pRepeatIds) const
{
    SwUndoId nRepeatId = GetUndoIds( pStr, pRepeatIds );
    if( REPEAT_START <= nRepeatId && REPEAT_END > nRepeatId )
        return nRepeatId;
    return UNDO_EMPTY;
}


SwUndo* SwDoc::RemoveLastUndo( SwUndoId eUndoId )
{
    SwUndo* pUndo = (*pUndos)[ nUndoPos - 1 ];
    if( eUndoId == pUndo->GetId() && nUndoPos == pUndos->Count() )
    {
        if( !nUndoSttEnd )
            --nUndoCnt;
        --nUndoPos;
        pUndos->Remove( nUndoPos, 1 );
    }
    else
    {
        pUndo = 0;
        ASSERT( !this, "falsches Undo-Object" );
    }
    return pUndo;
}

SwUndoIdAndName::SwUndoIdAndName( SwUndoId nId, const String* pStr )
    : eUndoId( nId ), pUndoStr( pStr ? new String( *pStr ) : 0 )
{
}

SwUndoIdAndName::~SwUndoIdAndName()
{
    delete pUndoStr;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
