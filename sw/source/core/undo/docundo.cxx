/*************************************************************************
 *
 *  $RCSfile: docundo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

USHORT SwDoc::nUndoActions = UNDO_ACTION_COUNT;     // anzahl von Undo-Action

//MA: Zur Zeit nicht verwendet.
//SV_IMPL_VARARR( SwUndoIds, USHORT )

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


void SwDoc::AppendUndo(SwUndo* pUndo)
{
    ASSERT( pUndos->Count() == nUndoPos,
            "das Undo-Array wurde nach einem Redo nicht geloescht!" );

#ifdef COMPACT
    DelUndoGroups( FALSE );     // nur die History loeschen !!
#endif

    if( REDLINE_NONE == pUndo->GetRedlineMode() )
        pUndo->SetRedlineMode( GetRedlineMode() );

    pUndos->Insert( pUndo, pUndos->Count() );
    nUndoPos = pUndos->Count();
    switch( pUndo->GetId() )
    {
    case UNDO_START:        ++nUndoSttEnd;
                            break;

    case UNDO_END:          ASSERT( nUndoSttEnd, "Undo-Ende ohne Start" );
                            --nUndoSttEnd;

                            // kein break !!!
    default:                if( !nUndoSttEnd )
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

    USHORT nEnde = USHRT_MAX - 1000;

// nur zum Testen der neuen DOC-Member
#ifndef PRODUCT
{
    USHORT nId, nUndosCnt = 0, nSttEndCnt = 0;
    for( USHORT nCnt = 0; nCnt < nUndoPos; ++nCnt )
    {
        if( UNDO_START == ( nId = (*pUndos)[ nCnt ]->GetId() ))
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
        DelUndoObj( (nUndoCnt - SwDoc::nUndoActions) + nUndoCnt / 10 );
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
        if( !nUndoSttEnd )
        {
            // setze UndoCnt auf den neuen Wert
            SwUndo* pUndo;
            for( USHORT nCnt = pUndos->Count(); nUndoPos < nCnt; --nUndoCnt )
                // Klammerung ueberspringen
                if( UNDO_END == (pUndo = (*pUndos)[ --nCnt ])->GetId() )
                    nCnt -= ((SwUndoEnd*)pUndo)->GetSttOffset();
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
    USHORT nId = 0, nSttEndCnt = 0;
    for( USHORT nCnt = 0; nEnde && nCnt < nUndoPos; ++nCnt )
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
        nUndoSavePos -= nSttEndCnt;

    while( nSttEndCnt )
        pUndos->DeleteAndDestroy( --nSttEndCnt, 1 );
    nUndoPos = pUndos->Count();

    DoUndo( TRUE );
    return TRUE;
}

/**************** UNDO ******************/


BOOL SwDoc::HasUndoId(USHORT nId) const
{
    USHORT nSize = nUndoPos;
    SwUndo * pUndo;
    while( nSize-- )
        if( ( pUndo = (*pUndos)[nSize])->GetId() == nId ||
            ( UNDO_START == pUndo->GetId() &&
                ((SwUndoStart*)pUndo)->GetUserId() == nId )
            || ( UNDO_END == pUndo->GetId() &&
                ((SwUndoEnd*)pUndo)->GetUserId() == nId ) )
        {
            return TRUE;
        }

    return FALSE;
}


BOOL SwDoc::Undo( SwUndoIter& rUndoIter )
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

    SwRedlineMode eOld = GetRedlineMode();
    SwRedlineMode eTmpMode = (SwRedlineMode)pUndo->GetRedlineMode();
    if( (REDLINE_SHOW_MASK & eTmpMode) != (REDLINE_SHOW_MASK & eOld) &&
        UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
        SetRedlineMode( eTmpMode );

    SetRedlineMode_intern( eTmpMode | REDLINE_IGNORE );
    // Undo ausfuehren

    // zum spaeteren ueberpruefen
    USHORT nAktId = pUndo->GetId();
    //JP 11.05.98: FlyFormate ueber die EditShell selektieren, nicht aus dem
    //              Undo heraus
    if( UNDO_START != nAktId && UNDO_END != nAktId )
        rUndoIter.ClearSelections();

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

#ifdef COMPACT

    // in der Compact-Version gibt es nur ein einstufiges Undo. Ueber das
    // Flag wird erkannt, wann ein Dokument als unveraendert gekennzeichnet
    // werden kann; nach einer Aktion und deren Undo
    // Bei mehr als einer Aktion ist das Dokument immer veraendert.

// wird nicht mehr beneotigt oder ???       Member am DOC geloescht
//  if( UNDO_FIRST == eUndoFlag )
//  {
//      ResetModified();
//      eUndoFlag = UNDO_INIT;
//  }

#else
    // ist die History leer und wurde nicht wegen Speichermangel
    // verworfen, so kann das Dokument als unveraendert gelten
    if( nUndoSavePos == nUndoPos )
        ResetModified();
#endif
    return TRUE;
}


// setzt Undoklammerung auf, liefert nUndoId der Klammerung


USHORT SwDoc::StartUndo( USHORT nUndoId )
{
    if( !bUndo )
        return 0;

    if( !nUndoId )
        nUndoId = UNDO_START;

    ClearRedo();
    AppendUndo( new SwUndoStart( nUndoId ));
    return nUndoId;
}
// schliesst Klammerung der nUndoId, nicht vom UI benutzt


USHORT SwDoc::EndUndo(USHORT nUndoId)
{
    USHORT nSize = pUndos->Count();
    if( !bUndo || !nSize-- )
        return 0;

    if( UNDO_START == nUndoId || !nUndoId )
        nUndoId = UNDO_END;

    SwUndo* pUndo = (*pUndos)[ nSize ];
    if( UNDO_START == pUndo->GetId() )
    {
        // leere Start/End-Klammerung ??
        pUndos->DeleteAndDestroy( nSize );
        nUndoPos = pUndos->Count();
        --nUndoSttEnd;
        return 0;
    }

    // suche den Anfang dieser Klammerung
    USHORT nId;
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
        SwUndo* pUndo;
        for( USHORT nCnt = 0; nCnt < pUndos->Count(); ++nCnt, ++nUndoCnt )
            // Klammerung ueberspringen
            if( UNDO_START == (pUndo = (*pUndos)[ nCnt ])->GetId() )
                nCnt += ((SwUndoStart*)pUndo)->GetEndOffset();
        return 0;

    }

    // Klammerung um eine einzelne Action muss nicht sein!
    // Aussnahme: es ist eine eigene ID definiert
    if(  2 == pUndos->Count() - nSize &&
        (UNDO_END == nUndoId || nUndoId == (*pUndos)[ nSize+1 ]->GetId() ))
    {
        pUndos->DeleteAndDestroy( nSize );
        nUndoPos = pUndos->Count();
        if( !--nUndoSttEnd )
        {
            ++nUndoCnt;
            if( SwDoc::nUndoActions < nUndoCnt )
                // immer 1/10 loeschen
                //JP 23.09.95: oder wenn neu eingestellt wurde um die Differenz
                DelUndoObj( (nUndoCnt - SwDoc::nUndoActions) + nUndoCnt / 10 );
            else
            {
                USHORT nEnde = USHRT_MAX - 1000;
                USHORT nUndosCnt = nUndoCnt;
                    // immer 1/10 loeschen bis der "Ausloeser" behoben ist
                while( aUndoNodes.Count() && nEnde < aUndoNodes.Count() )
                    DelUndoObj( nUndosCnt / 10 );
            }
        }
        return nUndoId;
    }

    // setze die Klammerung am Start/End-Undo
    nSize = pUndos->Count() - nSize;
    ((SwUndoStart*)pUndo)->SetEndOffset( nSize );

    SwUndoEnd* pUndoEnd = new SwUndoEnd( nUndoId );
    pUndoEnd->SetSttOffset( nSize );

// nur zum Testen der Start/End-Verpointerung vom Start/End Undo
#ifndef PRODUCT
{
    USHORT nEndCnt = 1, nCnt = pUndos->Count(), nId;
    while( nCnt )
    {
        if( UNDO_START == ( nId = (*pUndos)[ --nCnt ]->GetId()) )
        {
            if( !nEndCnt )      // falls mal ein Start ohne Ende vorhanden ist
                continue;
            --nEndCnt;
            if( !nEndCnt )      // hier ist der Anfang
                break;
        }
        else if( UNDO_END == nId )
            ++nEndCnt;
        else if( !nEndCnt )
            break;
    }
    ASSERT( nCnt == pUndos->Count() - nSize, "Start-Ende falsch geklammert" );
}
#endif

    AppendUndo( pUndoEnd );
    return nUndoId;
}

// liefert die Id der letzten Undofaehigen Aktion zurueck oder 0
// fuellt ggf. VARARR mit User-UndoIds


USHORT SwDoc::GetUndoIds( String* pStr, SwUndoIds *pUndoIds) const
{
    USHORT nSize = nUndoPos;
    if ( nSize-- == 0 )
        return 0;

    USHORT nId;
    SwUndo* pUndo;
    while( UNDO_END == (nId = (pUndo = (*pUndos)[nSize])->GetId()) && nSize
        && UNDO_END == (nId = ((SwUndoEnd*)pUndo)->GetUserId()) )
        nSize--;

    switch( pUndo->GetId() )
    {
    case UNDO_START:
        nId = ((SwUndoStart*)pUndo)->GetUserId();
        break;
    case UNDO_REDLINE:
        nId = ((SwUndoRedline*)pUndo)->GetUserId();
        break;
    case UNDO_DRAWUNDO:
        if( pStr )
            *pStr = ((SwSdrUndo*)pUndo)->GetComment();
        break;
    }

    return ( !nSize && UNDO_END == nId ? 0 : nId );
}

#ifdef COMPACT

BOOL SwDoc::DelUndoGroups( BOOL bDelUndoNds, BOOL bDelHistory )
{
    USHORT nEnd = 0, nStart = 0;
    USHORT nSize = pUndos->GetSize();
    SwUndo* pUndo;

    if( !nSize )
        return FALSE;

    while( nSize-- )
    {
         pUndo = (*pUndos)[ nSize ];
         if( UNDO_STD_END <= pUndo->GetId() || UNDO_END == pUndo->GetId() )
            // es kann sich nur um ein Ende handeln
            nEnd++;
         else if( UNDO_START == pUndo->GetId() )
            nStart++;
    }

    // eine vollstaendige Gruppe ist, wenn nStart und nEnd gleich sind
    if( nStart != nEnd )
        return TRUE;

    // jetzt kommt erst das eigentliche loeschen
    if( bDelHistory )
        pUndos->DelDtor( 0, pUndos->GetSize() );        // die UndoListe

    // loesche das Undo-Nodes-Array
    if( bDelUndoNds )
    {
        // es wird aus dem Undo-Array der gesamte Inhalt geloescht
        SwNodeIndex aIdx( *aUndoNodes.GetEndOfContent().StartOfSectionNode(), 1 );
        SwTxtNode * pTxtNd = 0;
        if( aIdx.GetIndex() + 1 < aUndoNodes.GetEndOfContent().GetIndex() ||
            0 == ( pTxtNd = aIdx.GetNode().GetTxtNode() ))
        {
            // loesche alle Nodes und suche oder erzeuge einen neuen TextNode
            while( aIdx < aUndoNodes.GetEndOfContent().GetIndex() )
            {
                aUndoNodes.Delete( aIdx , 1 );
                if( !pTxtNd && 0 != ( pTxtNd = aIdx.GetNode().GetTxtNode()))
                    aIdx++;
            }
            if( !pTxtNd )
            {
                // sollte eigentlich nie auftreten.
                pTxtNd = aUndoNodes.MakeTxtNode( aIdx,
                                    0, 0, 0, (*GetTxtFmtColls())[0] );
            }
        }
        // es braucht nur noch der Inhalt aus dem Node geloescht werden
        if( pTxtNd->Len() )
        {
            aIdx.Assign( pTxtNd, 0 );
            pTxtNd->Erase( aIdx, pTxtNd->Len() );
        }
    }
    nUndoPos = pUndos->GetSize();

    if( nUndoSavePos > nUndoPos )       // SavePos wird aufgegeben
        nUndoSavePos = USHRT_MAX;

    return TRUE;
}
#endif

/**************** REDO ******************/


BOOL SwDoc::Redo( SwUndoIter& rUndoIter )
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

    SwRedlineMode eOld = GetRedlineMode();
    SwRedlineMode eTmpMode = (SwRedlineMode)pUndo->GetRedlineMode();
    if( (REDLINE_SHOW_MASK & eTmpMode) != (REDLINE_SHOW_MASK & eOld) &&
        UNDO_START != pUndo->GetId() && UNDO_END != pUndo->GetId() )
        SetRedlineMode( eTmpMode );
    SetRedlineMode_intern( eTmpMode | REDLINE_IGNORE );

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


USHORT SwDoc::GetRedoIds( String* pStr, SwUndoIds *pRedoIds ) const
{
    USHORT nSize, nId;
    if( ( nSize = nUndoPos) == pUndos->Count() )
        return 0;

    SwUndo* pUndo;
    if( UNDO_START != ( nId = ( pUndo = (*pUndos)[nSize] )->GetId() ) ||
        UNDO_START != ( nId = ((SwUndoStart*)pUndo)->GetUserId() ) )
    {
        if( UNDO_REDLINE == nId )
            nId = ((SwUndoRedline*)pUndo)->GetUserId();
        else if( pStr && UNDO_DRAWUNDO == nId )
            *pStr = ((SwSdrUndo*)pUndo)->GetComment();
        return nId;
    }

    ASSERT( UNDO_END != nId, "falsches Ende der Undoklammerung!");

    // auf den vorm Ende der Klammerung
    nSize += ((SwUndoStart*)pUndo)->GetEndOffset();
    while( nSize &&
            UNDO_END == ( nId = ( pUndo = (*pUndos)[ --nSize ] )->GetId()) &&
            UNDO_END == ( nId = ((SwUndoEnd*)pUndo)->GetUserId() ) )
        ;

    if( !nSize )
        nId = 0;
    else if( pStr && UNDO_DRAWUNDO == nId )
        *pStr = ((SwSdrUndo*)pUndo)->GetComment();
    else if( UNDO_REDLINE == nId )
        nId = ((SwUndoRedline*)pUndo)->GetUserId();

    return nId;
}

/**************** REPEAT ******************/


BOOL SwDoc::Repeat( SwUndoIter& rUndoIter, USHORT nRepeatCnt )
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
        nSize -= ((SwUndoEnd*)pUndo)->GetSttOffset();

    USHORT nEndCnt = nUndoPos;
    BOOL bOneUndo = nSize + 1 == nUndoPos;

    SwPaM* pTmpCrsr = rUndoIter.pAktPam;
    if( pTmpCrsr != pTmpCrsr->GetNext() || !bOneUndo )  // Undo-Klammerung aufbauen
        StartUndo( 0 );
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
        EndUndo( 0 );

    return TRUE;
}

// liefert die Id der letzten Repeatfaehigen Aktion zurueck oder 0
// fuellt ggf. VARARR mit User-RedoIds


USHORT SwDoc::GetRepeatIds(String* pStr, SwUndoIds *pRepeatIds) const
{
    USHORT nRepeatId = GetUndoIds( pStr, pRepeatIds );
    if( REPEAT_START <= nRepeatId && REPEAT_END > nRepeatId )
        return nRepeatId;
    return 0;
}


SwUndo* SwDoc::RemoveLastUndo( USHORT nUndoId )
{
    SwUndo* pUndo = (*pUndos)[ pUndos->Count() - 1 ];
    if( nUndoId == pUndo->GetId() && nUndoPos == pUndos->Count() )
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



