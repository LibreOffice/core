/*************************************************************************
 *
 *  $RCSfile: docredln.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-13 10:49:14 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDARR_HXX
#include <ndarr.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif

#ifdef PRODUCT

    #define _CHECK_REDLINE( pDoc )

#else

    void lcl_CheckRedline( const SwDoc* pDoc )
    {
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
        for( USHORT n = 1; n < rTbl.Count(); ++n )
        {
            const SwRedline* pPrev = rTbl[ n-1 ], *pCur = rTbl[ n ];
            if( *pPrev->Start() > *pCur->Start() )
            {
                ASSERT( pDoc, "falche Reihenfolge" );
            }
        }
    }

    #define _CHECK_REDLINE( pDoc ) lcl_CheckRedline( pDoc );

#endif

SV_IMPL_OP_PTRARR_SORT( _SwRedlineTbl, SwRedlinePtr )

void SwDoc::SetRedlineMode( USHORT eMode )
{
    if( eRedlineMode != eMode )
    {
        if( (REDLINE_SHOW_MASK & eRedlineMode) != (REDLINE_SHOW_MASK & eMode)
            || 0 == (REDLINE_SHOW_MASK & eMode) )
        {
            // und dann alles verstecken, anzeigen
            void (SwRedline::*pFnc)( USHORT ) = 0;

            switch( REDLINE_SHOW_MASK & eMode )
            {
            case REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE:
                pFnc = &SwRedline::Show;
                break;
            case REDLINE_SHOW_INSERT:
                pFnc = &SwRedline::Hide;
                break;
            case REDLINE_SHOW_DELETE:
                pFnc = &SwRedline::ShowOriginal;
                break;

            default:
                pFnc = &SwRedline::Hide;
                eMode |= REDLINE_SHOW_INSERT;
                break;
            }

            _CHECK_REDLINE( this )

            if( pFnc )
                for( USHORT nLoop = 1; nLoop <= 2; ++nLoop )
                    for( USHORT i = 0; i < pRedlineTbl->Count(); ++i )
                        ((*pRedlineTbl)[ i ]->*pFnc)( nLoop );
            _CHECK_REDLINE( this )
        }
        eRedlineMode = (SwRedlineMode)eMode;
    }
}

inline BOOL IsPrevPos( const SwPosition rPos1, const SwPosition rPos2 )
{
    const SwCntntNode* pCNd;
    return !rPos2.nContent.GetIndex() &&
            rPos2.nNode.GetIndex() - 1 == rPos1.nNode.GetIndex() &&
            0 != ( pCNd = rPos1.nNode.GetNode().GetCntntNode() )
                ? rPos1.nContent.GetIndex() == pCNd->Len()
                : 0;
}

/*

Text heisst, nicht von Redline "verseuchter" Text.

Verhalten von Insert-Redline:
    - im Text                           - Redline Object einfuegen
    - im InsertRedline (eigenes)        - ignorieren, bestehendes wird
                                          aufgespannt
    - im InsertRedline (andere)         - Insert Redline aufsplitten
                                          Redline Object einfuegen
    - in DeleteRedline                  - Delete Redline aufsplitten oder
                                          am Ende/Anfang verschieben

Verhalten von Delete-Redline:
    - im Text                           - Redline Object einfuegen
    - im DeleteRedline (eigenes/andere) - ignorieren
    - im InsertRedline (eigenes)        - ignorieren, Zeichen aber loeschen
    - im InsertRedline (andere)         - Insert Redline aufsplitten
                                          Redline Object einfuegen
    - Ueberlappung von Text und         - Text in eigenen Insert loeschen,
      eigenem Insert                      im andereren Text aufspannen (bis
                                          zum Insert!
    - Ueberlappung von Text und         - Redline Object einfuegen, der
      anderem Insert                      andere Insert wird vom Delete
                                          ueberlappt
*/

BOOL SwDoc::AppendRedline( SwRedline* pNewRedl, BOOL bCallDelete )
{
    _CHECK_REDLINE( this )

    if( IsRedlineOn() && !IsShowOriginal( eRedlineMode ) &&
         pNewRedl->GetAuthorString().Len() )
    {
        pNewRedl->InvalidateRange();

        if( bIsAutoFmtRedline )
        {
            pNewRedl->SetAutoFmtFlag();
            if( pAutoFmtRedlnComment && pAutoFmtRedlnComment->Len() )
            {
                pNewRedl->SetComment( *pAutoFmtRedlnComment );
                pNewRedl->SetSeqNo( nAutoFmtRedlnCommentNo );
            }
        }

        BOOL bCompress = FALSE;
        SwPosition* pStt = pNewRedl->Start(),
                  * pEnd = pStt == pNewRedl->GetPoint() ? pNewRedl->GetMark()
                                                        : pNewRedl->GetPoint();
        USHORT n = 0;
            // zur StartPos das erste Redline suchen
        if( !GetRedline( *pStt, &n ) && n )
            --n;

        for( ; pNewRedl && n < pRedlineTbl->Count(); ++n )
        {
            SwRedline* pRedl = (*pRedlineTbl)[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();

            SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );

            switch( pNewRedl->GetType() )
            {
            case REDLINE_INSERT:
                switch( pRedl->GetType() )
                {
                case REDLINE_INSERT:
                    if( pRedl->IsOwnRedline( *pNewRedl ) )
                    {
                        // ggfs. verschmelzen?
                        if( POS_BEHIND == eCmpPos &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( *pStt == *pREnd ||
                             IsPrevPos( *pREnd, *pStt )) &&
                            ( n+1 >= pRedlineTbl->Count() ||
                             *(*pRedlineTbl)[ n+1 ]->Start() != *pREnd ))
                        {
                            pRedl->SetEnd( *pEnd, pREnd );
                            if( !pRedl->HasValidRange() )
                            {
                                // neu einsortieren
                                pRedlineTbl->Remove( n );
                                pRedlineTbl->Insert( pRedl );
                            }
                        }
                        else if( POS_BEFORE == eCmpPos &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( *pEnd == *pRStt ||
                              IsPrevPos( *pEnd, *pRStt )) &&
                            ( !n ||
                             *(*pRedlineTbl)[ n-1 ]->End() != *pRStt ))
                        {
                            pRedl->SetStart( *pStt, pRStt );
                            // neu einsortieren
                            pRedlineTbl->Remove( n );
                            pRedlineTbl->Insert( pRedl );
                        }
                        else if( POS_INSIDE != eCmpPos && POS_EQUAL != eCmpPos)
                            break;

                        delete pNewRedl, pNewRedl = 0;
                        bCompress = TRUE;
                    }
                    else if( POS_INSIDE == eCmpPos )
                    {
                        // aufsplitten
                        if( *pEnd != *pREnd )
                        {
                            SwRedline* pCpy = new SwRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            pRedlineTbl->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( !pRedl->HasValidRange() )
                        {
                            // neu einsortieren
                            pRedlineTbl->Remove( n );
                            pRedlineTbl->Insert( pRedl );
                        }
                    }
                    break;
                case REDLINE_DELETE:
                    if( POS_INSIDE == eCmpPos )
                    {
                        // aufsplitten
                        SwRedline* pCpy = new SwRedline( *pRedl );
                        pCpy->SetStart( *pEnd );
                        pRedlineTbl->Insert( pCpy );
                        pRedl->SetEnd( *pStt, pREnd );
                        if( !pRedl->HasValidRange() )
                        {
                            // neu einsortieren
                            pRedlineTbl->Remove( n );
                            pRedlineTbl->Insert( pRedl, n );
                        }
                    }
                    break;
                case REDLINE_FORMAT:
                    switch( eCmpPos )
                    {
                    case POS_OVERLAP_BEFORE:
                        pRedl->SetStart( *pEnd, pRStt );
                        // neu einsortieren
                        pRedlineTbl->Remove( n );
                        pRedlineTbl->Insert( pRedl, n );
                        break;

                    case POS_OVERLAP_BEHIND:
                        pRedl->SetEnd( *pStt, pREnd );
                        break;

                    case POS_EQUAL:
                    case POS_OUTSIDE:
                        // ueberlappt den akt. komplett oder hat gleiche
                        // Ausdehung, dann muss der alte geloescht werden
                        pRedlineTbl->DeleteAndDestroy( n-- );
                        break;

                    case POS_INSIDE:
                        // ueberlappt den akt. komplett, dann muss
                        // der neue gesplittet oder verkuertzt werden
                        if( *pEnd != *pREnd )
                        {
                            SwRedline* pNew = new SwRedline( *pRedl );
                            pNew->SetStart( *pEnd );
                            pRedl->SetEnd( *pStt, pREnd );
                            AppendRedline( pNew, bCallDelete );
                            n = (USHORT)-1;     // neu Aufsetzen
                        }
                        else
                            pRedl->SetEnd( *pStt, pREnd );
                        break;
                    }
                    break;

                }
                break;

            case REDLINE_DELETE:
                switch( pRedl->GetType() )
                {
                case REDLINE_DELETE:
                    switch( eCmpPos )
                    {
                    case POS_OUTSIDE:
                        {
                            // ueberlappt den akt. komplett
                            // dann muss der neue gesplittet werden
                            SwRedline* pNew = new SwRedline( *pNewRedl );
                            pNew->SetStart( *pREnd );
                            pNewRedl->SetEnd( *pRStt, pEnd );
                            AppendRedline( pNew, bCallDelete );
                            n = (USHORT)-1;     // neu Aufsetzen
                        }
                        break;

                    case POS_INSIDE:
                    case POS_EQUAL:
                        delete pNewRedl, pNewRedl = 0;
                        bCompress = TRUE;
                        break;

                    case POS_OVERLAP_BEFORE:
                    case POS_OVERLAP_BEHIND:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
//                          1 == pRedl->GetStackCount() &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // dann kann das zusammengefasst werden, sprich
                            // der neue deckt das schon ab.
                            if( POS_OVERLAP_BEHIND == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            pRedlineTbl->DeleteAndDestroy( n-- );
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;
                    case POS_BEFORE:
                    case POS_BEHIND:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
//                          1 == pRedl->GetStackCount() &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            (POS_BEHIND == eCmpPos ? *pREnd == *pStt
                                                   : *pRStt == *pEnd ))
                        {
                            if( IsHideChanges( eRedlineMode ))
                            {
                                // dann erstmal sichtbar machen, bevor
                                // die zusammengefasst werden koennen!
                                // Damit pNew auch beim Verschieben der
                                // Indizies behandelt wird, erstmal
                                // temporaer einfuegen
                                pRedlineTbl->SavePtrInArr( pNewRedl );
                                pRedl->Show();
                                pRedlineTbl->Remove( pRedlineTbl->GetPos(pNewRedl ));
                                pRStt = pRedl->Start();
                                pREnd = pRedl->End();
                            }

                            // dann kann das zusammengefasst werden, sprich
                            // der neue deckt das schon ab.
                            if( POS_BEHIND == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            pRedlineTbl->DeleteAndDestroy( n-- );
                        }
                        break;
                    }
                    break;

                case REDLINE_INSERT:
                    if( pRedl->IsOwnRedline( *pNewRedl ) )
                    {
                        SwRedlineMode eOld = eRedlineMode;
// auf NONE setzen, damit das Delete::Redo die RedlineDaten wieder richtig
// zusammen fasst! Der ShowMode muss erhalten bleiben!
                        eRedlineMode = (SwRedlineMode)(eOld & ~(REDLINE_ON | REDLINE_IGNORE));
                        switch( eCmpPos )
                        {
                        case POS_EQUAL:
                            bCompress = TRUE;
                            pRedlineTbl->DeleteAndDestroy( n-- );
                            // kein break!

                        case POS_INSIDE:
                            if( bCallDelete )
                            {
                                eRedlineMode = (SwRedlineMode)
                                    (eRedlineMode | REDLINE_IGNOREDELETE_REDLINES);
                                DeleteAndJoin( *pNewRedl );
                                bCompress = TRUE;
                            }
                            delete pNewRedl, pNewRedl = 0;
                            break;

                        case POS_OUTSIDE:
                            {
                                pRedlineTbl->Remove( n-- );
                                // damit pNew auch beim Verschieben der Indizies
                                // behandelt wird, erstmal temp. einfuegen
                                if( bCallDelete )
                                {
                                    pRedlineTbl->SavePtrInArr( pNewRedl );
                                    DeleteAndJoin( *pRedl );
                                    USHORT nFnd = pRedlineTbl->GetPos(pNewRedl );
                                    if( USHRT_MAX != nFnd )
                                        pRedlineTbl->Remove( nFnd );
                                    else
                                        pNewRedl = 0;
                                }
                                delete pRedl;
                            }
                            break;

                        case POS_OVERLAP_BEFORE:
                            {
                                SwPaM aPam( *pRStt, *pEnd );

                                if( *pEnd == *pREnd )
                                    pRedlineTbl->DeleteAndDestroy( n-- );
                                else
                                {
                                    pRedl->SetStart( *pEnd, pRStt );
                                    // neu einsortieren
                                    pRedlineTbl->Remove( n );
                                    pRedlineTbl->Insert( pRedl, n );
                                }

                                if( bCallDelete )
                                {
                                    // damit pNew auch beim Verschieben der Indizies
                                    // behandelt wird, erstmal temp. einfuegen
                                    pRedlineTbl->SavePtrInArr( pNewRedl );
                                    DeleteAndJoin( aPam );
                                    USHORT nFnd = pRedlineTbl->GetPos(pNewRedl );
                                    if( USHRT_MAX != nFnd )
                                        pRedlineTbl->Remove( nFnd );
                                    else
                                        pNewRedl = 0;
                                    n = (USHORT)-1;     // neu Aufsetzen
                                }
                            }
                            break;

                        case POS_OVERLAP_BEHIND:
                            {
                                SwPaM aPam( *pStt, *pREnd );

                                if( *pStt == *pRStt )
                                    pRedlineTbl->DeleteAndDestroy( n-- );
                                else
                                    pRedl->SetEnd( *pStt, pREnd );

                                if( bCallDelete )
                                {
                                    // damit pNew auch beim Verschieben der Indizies
                                    // behandelt wird, erstmal temp. einfuegen
                                    pRedlineTbl->SavePtrInArr( pNewRedl );
                                    DeleteAndJoin( aPam );
                                    USHORT nFnd = pRedlineTbl->GetPos(pNewRedl );
                                    if( USHRT_MAX != nFnd )
                                        pRedlineTbl->Remove( nFnd );
                                    else
                                        pNewRedl = 0;
                                    n = (USHORT)-1;     // neu Aufsetzen
                                }
                            }
                            break;
                        }

                        eRedlineMode = eOld;
                    }
                    else
                    {
                        SwRedline* pNew = 0;
                        switch( eCmpPos )
                        {
                        case POS_EQUAL:
                            {
                                pRedl->PushData( *pNewRedl );
                                delete pNewRedl, pNewRedl = 0;
                                if( IsHideChanges( eRedlineMode ))
                                    pRedl->Hide();
                                bCompress = TRUE;
                            }
                            break;

                        case POS_INSIDE:
                            {
                                pNewRedl->PushData( *pRedl, FALSE );
                                if( *pRStt == *pStt )
                                {
                                    pRedl->SetStart( *pEnd, pRStt );
                                    // neu einsortieren
                                    pRedlineTbl->Remove( n );
                                    pRedlineTbl->Insert( pRedl, n );
                                }
                                else
                                {
                                    if( *pREnd == *pEnd )
                                        pRedl->SetEnd( *pStt, pREnd );
                                    else
                                    {
                                        pNew = new SwRedline( *pRedl );
                                        pNew->SetStart( *pEnd );
                                        pRedl->SetEnd( *pStt, pREnd );
                                    }
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // neu einsortieren
                                        pRedlineTbl->Remove( n );
                                        pRedlineTbl->Insert( pRedl, n );
                                    }
                                }
                            }
                            break;

                        case POS_OUTSIDE:
                            {
                                pNew = new SwRedline( *pNewRedl );
                                pRedl->PushData( *pNewRedl );

                                pNew->SetEnd( *pRStt );
                                pNewRedl->SetStart( *pREnd, pStt );
                                bCompress = TRUE;
                            }
                            break;

                        case POS_OVERLAP_BEFORE:
                            {
                                if( *pEnd == *pREnd )
                                {
                                    pRedl->PushData( *pNewRedl );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    if( IsHideChanges( eRedlineMode ))
                                    {
                                        pRedlineTbl->SavePtrInArr( pNewRedl );
                                        pRedl->Hide();
                                        pRedlineTbl->Remove(
                                            pRedlineTbl->GetPos(pNewRedl ));
                                    }
                                }
                                else
                                {
                                    pNew = new SwRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetEnd( *pEnd );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    pRedl->SetStart( *pNew->End(), pRStt ) ;
                                    // neu einsortieren
                                    pRedlineTbl->Remove( n );
                                    pRedlineTbl->Insert( pRedl );
                                }
                            }
                            break;

                        case POS_OVERLAP_BEHIND:
                            {
                                if( *pStt == *pRStt )
                                {
                                    pRedl->PushData( *pNewRedl );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    if( IsHideChanges( eRedlineMode ))
                                    {
                                        pRedlineTbl->SavePtrInArr( pNewRedl );
                                        pRedl->Hide();
                                        pRedlineTbl->Remove(
                                            pRedlineTbl->GetPos(pNewRedl ));
                                    }
                                }
                                else
                                {
                                    pNew = new SwRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetStart( *pStt );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    pRedl->SetEnd( *pNew->Start(), pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // neu einsortieren
                                        pRedlineTbl->Remove( n );
                                        pRedlineTbl->Insert( pRedl );
                                    }
                                }
                            }
                            break;
                        }
                        if( pNew )
                        {
                            AppendRedline( pNew, bCallDelete );
                            n = (USHORT)-1;     // neu Aufsetzen
                        }
                    }
                    break;

                case REDLINE_FORMAT:
                    switch( eCmpPos )
                    {
                    case POS_OVERLAP_BEFORE:
                        pRedl->SetStart( *pEnd, pRStt );
                        // neu einsortieren
                        pRedlineTbl->Remove( n );
                        pRedlineTbl->Insert( pRedl, n );
                        break;

                    case POS_OVERLAP_BEHIND:
                        pRedl->SetEnd( *pStt, pREnd );
                        break;

                    case POS_EQUAL:
                    case POS_OUTSIDE:
                        // ueberlappt den akt. komplett oder hat gleiche
                        // Ausdehung, dann muss der alte geloescht werden
                        pRedlineTbl->DeleteAndDestroy( n-- );
                        break;

                    case POS_INSIDE:
                        // ueberlappt den akt. komplett, dann muss
                        // der neue gesplittet oder verkuertzt werden
                        if( *pEnd != *pREnd )
                        {
                            SwRedline* pNew = new SwRedline( *pRedl );
                            pNew->SetStart( *pEnd );
                            pRedl->SetEnd( *pStt, pREnd );
                            AppendRedline( pNew, bCallDelete );
                            n = (USHORT)-1;     // neu Aufsetzen
                        }
                        else
                            pRedl->SetEnd( *pStt, pREnd );
                        break;
                    }
                    break;
                }
                break;

            case REDLINE_FORMAT:
                switch( pRedl->GetType() )
                {
                case REDLINE_INSERT:
                case REDLINE_DELETE:
                    switch( eCmpPos )
                    {
                    case POS_OVERLAP_BEFORE:
                        pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case POS_OVERLAP_BEHIND:
                        pNewRedl->SetStart( *pREnd, pStt );
                        break;

                    case POS_EQUAL:
                    case POS_INSIDE:
                        delete pNewRedl, pNewRedl = 0;
                        break;

                    case POS_OUTSIDE:
                        // ueberlappt den akt. komplett, dann muss
                        // der neue gesplittet oder verkuertzt werden
                        if( *pEnd != *pREnd )
                        {
                            SwRedline* pNew = new SwRedline( *pNewRedl );
                            pNew->SetStart( *pREnd );
                            pNewRedl->SetEnd( *pRStt, pEnd );
                            AppendRedline( pNew, bCallDelete );
                            n = (USHORT)-1;     // neu Aufsetzen
                        }
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;
                    }
                    break;

                case REDLINE_FORMAT:
                    switch( eCmpPos )
                    {
                    case POS_OUTSIDE:
                    case POS_EQUAL:
                        {
                            // ueberlappt den akt. komplett oder hat gleiche
                            // Ausdehung, dann muss der alte geloescht werden
                            pRedlineTbl->DeleteAndDestroy( n-- );
                        }
                        break;

                    case POS_INSIDE:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                            // ein eigenes kann komplett ignoriert werden
                            delete pNewRedl, pNewRedl = 0;

                        else if( *pREnd == *pEnd )
                            // ansonsten nur den akt. verkuerzen
                            pRedl->SetEnd( *pStt, pREnd );
                        else if( *pRStt == *pStt )
                        {
                            // ansonsten nur den akt. verkuerzen
                            pRedl->SetStart( *pEnd, pRStt );
                            // neu einsortieren
                            pRedlineTbl->Remove( n );
                            pRedlineTbl->Insert( pRedl, n );
                        }
                        else
                        {
                            // liegt komplett im akt.
                            // dann muss der gesplittet werden
                            SwRedline* pNew = new SwRedline( *pRedl );
                            pNew->SetStart( *pEnd );
                            pRedl->SetEnd( *pStt, pREnd );
                            AppendRedline( pNew, bCallDelete );
                            n = (USHORT)-1;     // neu Aufsetzen
                        }
                        break;

                    case POS_OVERLAP_BEFORE:
                    case POS_OVERLAP_BEHIND:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // dann kann das zusammengefasst werden, sprich
                            // der neue deckt das schon ab.
                            if( POS_OVERLAP_BEHIND == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            pRedlineTbl->DeleteAndDestroy( n-- );
                        }
                        else if( POS_OVERLAP_BEHIND == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;
                    case POS_BEFORE:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            *pRStt == *pEnd && n &&
                            *(*pRedlineTbl)[ n-1 ]->End() < *pStt )
                        {
                            // dann kann das zusammengefasst werden, sprich
                            // der neue deckt das schon ab.
                            pNewRedl->SetEnd( *pREnd, pEnd );
                            pRedlineTbl->DeleteAndDestroy( n-- );
                        }
                        break;
                    case POS_BEHIND:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            *pREnd == *pStt && n+1 < pRedlineTbl->Count() &&
                            *(*pRedlineTbl)[ n+1 ]->Start() < *pEnd )
                        {
                            // dann kann das zusammengefasst werden, sprich
                            // der neue deckt das schon ab.
                            pNewRedl->SetStart( *pRStt, pStt );
                            pRedlineTbl->DeleteAndDestroy( n-- );
                        }
                        break;
                    }
                    break;
                }
                break;


            case REDLINE_FMTCOLL:
                // wie soll das verhalten sein????
                // erstmal so einfuegen
                break;
            }
        }

        if( pNewRedl )
            pRedlineTbl->Insert( pNewRedl );

        if( bCompress )
            CompressRedlines();
    }
    else
    {
        if( bCallDelete && REDLINE_DELETE == pNewRedl->GetType() )
        {
            SwRedlineMode eOld = eRedlineMode;
// auf NONE setzen, damit das Delete::Redo die RedlineDaten wieder richtig
// zusammen fasst! Der ShowMode muss erhalten bleiben!
            eRedlineMode = (SwRedlineMode)(eOld & ~(REDLINE_ON | REDLINE_IGNORE));
            DeleteAndJoin( *pNewRedl );
            eRedlineMode = eOld;
        }
        delete pNewRedl, pNewRedl = 0;
    }
    _CHECK_REDLINE( this )

    return 0 != pNewRedl;
}

void SwDoc::CompressRedlines()
{
    _CHECK_REDLINE( this )

    void (SwRedline::*pFnc)(USHORT) = 0;
    switch( REDLINE_SHOW_MASK & eRedlineMode )
    {
    case REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE:
        pFnc = &SwRedline::Show;
        break;
    case REDLINE_SHOW_INSERT:
        pFnc = &SwRedline::Hide;
        break;
    }

    // versuche gleiche zusammenzufassen
    for( USHORT n = 1; n < pRedlineTbl->Count(); ++n )
    {
        SwRedline* pPrev = (*pRedlineTbl)[ n-1 ],
                    * pCur = (*pRedlineTbl)[ n ];
        const SwPosition* pPrevStt = pPrev->Start(),
                        * pPrevEnd = pPrevStt == pPrev->GetPoint()
                            ? pPrev->GetMark() : pPrev->GetPoint();
        const SwPosition* pCurStt = pCur->Start(),
                        * pCurEnd = pCurStt == pCur->GetPoint()
                            ? pCur->GetMark() : pCur->GetPoint();
        if( *pPrevEnd == *pCurStt && pPrev->CanCombine( *pCur ) &&
            pPrevStt->nNode.GetNode().FindStartNode() ==
            pCurEnd->nNode.GetNode().FindStartNode() &&
            !pCurEnd->nNode.GetNode().FindStartNode()->IsTableNode() )
        {
            // dann koennen die zusammen gefasst werden
            pPrev->Show();
            pCur->Show();

            pPrev->SetEnd( *pCur->End() );
            pRedlineTbl->DeleteAndDestroy( n );
            --n;
            if( pFnc )
                (pPrev->*pFnc)(0);
        }
    }
    _CHECK_REDLINE( this )
}

BOOL SwDoc::SplitRedline( const SwPaM& rRange )
{
    BOOL bChg = FALSE;
    USHORT n = 0;
    const SwPosition* pStt = rRange.Start(),
                  * pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
                                                     : rRange.GetPoint();
    GetRedline( *pStt, &n );
    for( ; n < pRedlineTbl->Count() ; ++n )
    {
        SwRedline* pTmp = (*pRedlineTbl)[ n ];
        SwPosition* pTStt = pTmp->Start(),
                  * pTEnd = pTStt == pTmp->GetPoint() ? pTmp->GetMark()
                                                      : pTmp->GetPoint();
        if( *pTStt <= *pStt && *pStt <= *pTEnd &&
            *pTStt <= *pEnd && *pEnd <= *pTEnd )
        {
            bChg = TRUE;
            int nn = 0;
            if( *pStt == *pTStt )
                nn += 1;
            if( *pEnd == *pTEnd )
                nn += 2;

            SwRedline* pNew = 0;
            switch( nn )
            {
            case 0:
                pNew = new SwRedline( *pTmp );
                pTmp->SetEnd( *pStt, pTEnd );
                pNew->SetStart( *pEnd );
                break;

            case 1:
                *pTStt = *pEnd;
                break;

            case 2:
                *pTEnd = *pStt;
                break;

            case 3:
                pTmp->InvalidateRange();
                pRedlineTbl->DeleteAndDestroy( n-- );
                pTmp = 0;
                break;
            }
            if( pTmp && !pTmp->HasValidRange() )
            {
                // neu einsortieren
                pRedlineTbl->Remove( n );
                pRedlineTbl->Insert( pTmp, n );
            }
            if( pNew )
                pRedlineTbl->Insert( pNew, n );
        }
        else if( *pEnd < *pTStt )
            break;
    }
    return bChg;
}

BOOL SwDoc::DeleteRedline( const SwPaM& rRange, BOOL bSaveInUndo,
                            USHORT nDelType )
{
    if( REDLINE_IGNOREDELETE_REDLINES & eRedlineMode ||
        !rRange.HasMark() || *rRange.GetMark() == *rRange.GetPoint() )
        return FALSE;

    BOOL bChg = FALSE;

    if( bSaveInUndo && DoesUndo() )
    {
        SwUndoRedline* pUndo = new SwUndoRedline( UNDO_REDLINE, rRange );
        if( pUndo->GetRedlSaveCount() )
        {
            ClearRedo();
            AppendUndo( pUndo );
        }
        else
            delete pUndo;
    }

    const SwPosition* pStt = rRange.Start(),
                    * pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
                                                       : rRange.GetPoint();
    USHORT n = 0;
    GetRedline( *pStt, &n );
    for( ; n < pRedlineTbl->Count() ; ++n )
    {
        SwRedline* pRedl = (*pRedlineTbl)[ n ];
        if( USHRT_MAX != nDelType && nDelType != pRedl->GetType() )
            continue;

        SwPosition* pRStt = pRedl->Start(),
                  * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                       : pRedl->GetPoint();
        BOOL bDel = FALSE;
        switch( ComparePosition( *pStt, *pEnd, *pRStt, *pREnd ) )
        {
        case POS_EQUAL:
        case POS_OUTSIDE:
            bDel = TRUE;
            break;

        case POS_OVERLAP_BEFORE:
            if( *pEnd == *pREnd )
                bDel = TRUE;
            else
            {
                pRedl->InvalidateRange();
                pRedl->SetStart( *pEnd, pRStt );
                // neu einsortieren
                pRedlineTbl->Remove( n );
                pRedlineTbl->Insert( pRedl );
                --n;
            }
            break;

        case POS_OVERLAP_BEHIND:
            if( *pStt == *pRStt )
                bDel = TRUE;
            else
            {
                pRedl->InvalidateRange();
                pRedl->SetEnd( *pStt, pREnd );
                if( !pRedl->HasValidRange() )
                {
                    // neu einsortieren
                    pRedlineTbl->Remove( n );
                    pRedlineTbl->Insert( pRedl );
                    --n;
                }
            }
            break;

        case POS_INSIDE:
            {
                // der muss gesplittet werden
                pRedl->InvalidateRange();
                if( *pRStt == *pStt )
                {
                    pRedl->SetStart( *pEnd, pRStt );
                    // neu einsortieren
                    pRedlineTbl->Remove( n );
                    pRedlineTbl->Insert( pRedl );
                    --n;
                }
                else
                {
                    SwRedline* pCpy;
                    if( *pREnd != *pEnd )
                    {
                        pCpy = new SwRedline( *pRedl );
                        pCpy->SetStart( *pEnd );
                    }
                    else
                        pCpy = 0;
                    pRedl->SetEnd( *pStt, pREnd );
                    if( !pRedl->HasValidRange() )
                    {
                        // neu einsortieren
                        pRedlineTbl->Remove( pRedlineTbl->GetPos( pRedl ));
                        pRedlineTbl->Insert( pRedl );
                        --n;
                    }
                    if( pCpy )
                        pRedlineTbl->Insert( pCpy );
                }
            }
            break;

        case POS_BEFORE:
            n = pRedlineTbl->Count();
            break;
        }

        if( bDel )
        {
            pRedl->InvalidateRange();
            pRedlineTbl->DeleteAndDestroy( n-- );
            bChg = TRUE;
        }
    }

    if( bChg )
        SetModified();

    return bChg;
}

BOOL SwDoc::DeleteRedline( const SwStartNode& rNode, BOOL bSaveInUndo,
                            USHORT nDelType )
{
    return DeleteRedline( SwPaM( *rNode.EndOfSectionNode(), rNode ),
                            bSaveInUndo, nDelType );
}

void SwDoc::DeleteRedline( USHORT nPos )
{
    SwRedline* pTmp = (*pRedlineTbl)[ nPos ];
    pTmp->InvalidateRange();
    pRedlineTbl->DeleteAndDestroy( nPos );
}

USHORT SwDoc::GetRedlinePos( const SwNode& rNd, USHORT nType ) const
{
    const ULONG nNdIdx = rNd.GetIndex();
    for( USHORT n = 0; n < pRedlineTbl->Count() ; ++n )
    {
        const SwRedline* pTmp = (*pRedlineTbl)[ n ];
        ULONG nPt = pTmp->GetPoint()->nNode.GetIndex(),
              nMk = pTmp->GetMark()->nNode.GetIndex();
        if( nPt < nMk ) { long nTmp = nMk; nMk = nPt; nPt = nTmp; }

        if( ( USHRT_MAX == nType || nType == pTmp->GetType()) &&
            nMk <= nNdIdx && nNdIdx <= nPt )
            return n;

        if( nMk > nNdIdx )
            break;
    }
    return USHRT_MAX;
}

const SwRedline* SwDoc::GetRedline( const SwPosition& rPos,
                                    USHORT* pFndPos ) const
{
    register USHORT nO = pRedlineTbl->Count(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            const SwRedline* pRedl = (*pRedlineTbl)[ nM ];
            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();
            if( pEnd == pStt
                    ? *pStt == rPos
                    : ( *pStt <= rPos && rPos < *pEnd ) )
            {
                while( nM && *pStt == *(*pRedlineTbl)[ nM - 1 ]->Start() )
                {
                    --nM;
                    pRedl = (*pRedlineTbl)[ nM ];
                }

                if( pFndPos )
                    *pFndPos = nM;
                return pRedl;
            }
            else if( *pEnd <= rPos )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return 0;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return 0;
}

typedef BOOL (*Fn_AcceptReject)( SwRedlineTbl& rArr, USHORT& rPos,
                        BOOL bCallDelete,
                        const SwPosition* pSttRng,
                        const SwPosition* pEndRng);

BOOL lcl_AcceptRedline( SwRedlineTbl& rArr, USHORT& rPos,
                        BOOL bCallDelete,
                        const SwPosition* pSttRng = 0,
                        const SwPosition* pEndRng = 0 )
{
    BOOL bRet = TRUE;
    SwRedline* pRedl = rArr[ rPos ];
    SwPosition *pRStt = 0, *pREnd = 0;
    SwComparePosition eCmp = POS_OUTSIDE;
    if( pSttRng && pEndRng )
    {
        pRStt = pRedl->Start();
        pREnd = pRedl->End();
        eCmp = ComparePosition( *pSttRng, *pEndRng, *pRStt, *pREnd );
    }

    pRedl->InvalidateRange();

    switch( pRedl->GetType() )
    {
    case REDLINE_INSERT:
    case REDLINE_FORMAT:
        {
            BOOL bCheck = FALSE, bReplace = FALSE;
            switch( eCmp )
            {
            case POS_INSIDE:
                if( *pSttRng == *pRStt )
                    pRedl->SetStart( *pEndRng, pRStt );
                else
                {
                    if( *pEndRng != *pREnd )
                    {
                        // aufsplitten
                        SwRedline* pNew = new SwRedline( *pRedl );
                        pNew->SetStart( *pEndRng );
                        rArr.Insert( pNew ); ++rPos;
                    }
                    pRedl->SetEnd( *pSttRng, pREnd );
                    bCheck = TRUE;
                }
                break;

            case POS_OVERLAP_BEFORE:
                pRedl->SetStart( *pEndRng, pRStt );
                bReplace = TRUE;
                break;

            case POS_OVERLAP_BEHIND:
                pRedl->SetEnd( *pSttRng, pREnd );
                bCheck = TRUE;
                break;

            case POS_OUTSIDE:
            case POS_EQUAL:
                rArr.DeleteAndDestroy( rPos-- );
                break;

            default:
                bRet = FALSE;
            }

            if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
            {
                // neu einsortieren
                rArr.Remove( rArr.GetPos( pRedl ));
                rArr.Insert( pRedl );
            }
        }
        break;
    case REDLINE_DELETE:
        {
            SwDoc& rDoc = *pRedl->GetDoc();
            const SwPosition *pDelStt = 0, *pDelEnd = 0;
            BOOL bDelRedl = FALSE;
            switch( eCmp )
            {
            case POS_INSIDE:
                if( bCallDelete )
                {
                    pDelStt = pSttRng;
                    pDelEnd = pEndRng;
                }
                break;

            case POS_OVERLAP_BEFORE:
                if( bCallDelete )
                {
                    pDelStt = pRStt;
                    pDelEnd = pEndRng;
                }
                break;
            case POS_OVERLAP_BEHIND:
                if( bCallDelete )
                {
                    pDelStt = pREnd;
                    pDelEnd = pSttRng;
                }
                break;

            case POS_OUTSIDE:
            case POS_EQUAL:
                {
                    rArr.Remove( rPos-- );
                    bDelRedl = TRUE;
                    if( bCallDelete )
                    {
                        pDelStt = pRedl->Start();
                        pDelEnd = pRedl->End();
                    }
                }
                break;
            default:
                bRet = FALSE;
            }

            if( pDelStt && pDelEnd )
            {
                SwPaM aPam( *pDelStt, *pDelEnd );

                if( bDelRedl )
                    delete pRedl;

                SwRedlineMode eOld = rDoc.GetRedlineMode();
                rDoc.SetRedlineMode_intern( eOld & ~(REDLINE_ON | REDLINE_IGNORE) );
                rDoc.DeleteAndJoin( aPam );
                rDoc.SetRedlineMode_intern( eOld );
            }
            else if( bDelRedl )
                delete pRedl;
        }
        break;

    case REDLINE_FMTCOLL:
        rArr.DeleteAndDestroy( rPos-- );
        break;

    default:
        bRet = FALSE;
    }
    return bRet;
}

BOOL lcl_RejectRedline( SwRedlineTbl& rArr, USHORT& rPos,
                        BOOL bCallDelete,
                        const SwPosition* pSttRng = 0,
                        const SwPosition* pEndRng = 0 )
{
    BOOL bRet = TRUE;
    SwRedline* pRedl = rArr[ rPos ];
    SwPosition *pRStt = 0, *pREnd = 0;
    SwComparePosition eCmp = POS_OUTSIDE;
    if( pSttRng && pEndRng )
    {
        pRStt = pRedl->Start();
        pREnd = pRedl->End();
        eCmp = ComparePosition( *pSttRng, *pEndRng, *pRStt, *pREnd );
    }

    pRedl->InvalidateRange();

    switch( pRedl->GetType() )
    {
    case REDLINE_INSERT:
        {
            SwDoc& rDoc = *pRedl->GetDoc();
            const SwPosition *pDelStt = 0, *pDelEnd = 0;
            BOOL bDelRedl = FALSE;
            switch( eCmp )
            {
            case POS_INSIDE:
                if( bCallDelete )
                {
                    pDelStt = pSttRng;
                    pDelEnd = pEndRng;
                }
                break;

            case POS_OVERLAP_BEFORE:
                if( bCallDelete )
                {
                    pDelStt = pRStt;
                    pDelEnd = pEndRng;
                }
                break;
            case POS_OVERLAP_BEHIND:
                if( bCallDelete )
                {
                    pDelStt = pREnd;
                    pDelEnd = pSttRng;
                }
                break;
            case POS_OUTSIDE:
            case POS_EQUAL:
                {
                    // dann den Bereich wieder loeschen
                    rArr.Remove( rPos-- );
                    bDelRedl = TRUE;
                    if( bCallDelete )
                    {
                        pDelStt = pRedl->Start();
                        pDelEnd = pRedl->End();
                    }
                }
                break;

            default:
                bRet = FALSE;
            }
            if( pDelStt && pDelEnd )
            {
                SwPaM aPam( *pDelStt, *pDelEnd );

                if( bDelRedl )
                    delete pRedl;

                SwRedlineMode eOld = rDoc.GetRedlineMode();
                rDoc.SetRedlineMode_intern( eOld & ~(REDLINE_ON | REDLINE_IGNORE) );
                rDoc.DeleteAndJoin( aPam );
                rDoc.SetRedlineMode_intern( eOld );
            }
            else if( bDelRedl )
                delete pRedl;
        }
        break;
    case REDLINE_DELETE:
        {
            SwRedline* pNew = 0;
            BOOL bCheck = FALSE, bReplace = FALSE;

            switch( eCmp )
            {
            case POS_INSIDE:
                {
                    if( 1 < pRedl->GetStackCount() )
                    {
                        pNew = new SwRedline( *pRedl );
                        pNew->PopData();
                    }
                    if( *pSttRng == *pRStt )
                    {
                        pRedl->SetStart( *pEndRng, pRStt );
                        bReplace = TRUE;
                        if( pNew )
                            pNew->SetEnd( *pEndRng );
                    }
                    else
                    {
                        if( *pEndRng != *pREnd )
                        {
                            // aufsplitten
                            SwRedline* pCpy = new SwRedline( *pRedl );
                            pCpy->SetStart( *pEndRng );
                            rArr.Insert( pCpy ); ++rPos;
                            if( pNew )
                                pNew->SetEnd( *pEndRng );
                        }

                        pRedl->SetEnd( *pSttRng, pREnd );
                        bCheck = TRUE;
                        if( pNew )
                            pNew->SetStart( *pSttRng );
                    }
                }
                break;

            case POS_OVERLAP_BEFORE:
                if( 1 < pRedl->GetStackCount() )
                {
                    pNew = new SwRedline( *pRedl );
                    pNew->PopData();
                }
                pRedl->SetStart( *pEndRng, pRStt );
                bReplace = TRUE;
                if( pNew )
                    pNew->SetEnd( *pEndRng );
                break;

            case POS_OVERLAP_BEHIND:
                if( 1 < pRedl->GetStackCount() )
                {
                    pNew = new SwRedline( *pRedl );
                    pNew->PopData();
                }
                pRedl->SetEnd( *pSttRng, pREnd );
                bCheck = TRUE;
                if( pNew )
                    pNew->SetStart( *pSttRng );
                break;

            case POS_OUTSIDE:
            case POS_EQUAL:
                if( !pRedl->PopData() )
                    // das RedlineObject loeschen reicht
                    rArr.DeleteAndDestroy( rPos-- );
                break;

            default:
                bRet = FALSE;
            }

            if( pNew )
            {
                rArr.Insert( pNew ); ++rPos;
            }

            if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
            {
                // neu einsortieren
                rArr.Remove( rArr.GetPos( pRedl ));
                rArr.Insert( pRedl );
            }
        }
        break;

    case REDLINE_FORMAT:
    case REDLINE_FMTCOLL:
        {
            if( pRedl->GetExtraData() )
                pRedl->GetExtraData()->Reject( *pRedl );
            rArr.DeleteAndDestroy( rPos-- );
        }
        break;

    default:
        bRet = FALSE;
    }
    return bRet;
}


const SwRedline* lcl_FindCurrRedline( const SwPosition& rSttPos,
                                        USHORT& rPos,
                                        BOOL bNext = TRUE )
{
    const SwRedline* pFnd = 0;
    const SwRedlineTbl& rArr = rSttPos.nNode.GetNode().GetDoc()->GetRedlineTbl();
    for( ; rPos < rArr.Count() ; ++rPos )
    {
        const SwRedline* pTmp = rArr[ rPos ];
        if( pTmp->HasMark() && pTmp->IsVisible() )
        {
            const SwPosition* pRStt = pTmp->Start(),
                      * pREnd = pRStt == pTmp->GetPoint() ? pTmp->GetMark()
                                                          : pTmp->GetPoint();
            if( bNext ? *pRStt <= rSttPos : *pRStt < rSttPos )
            {
                if( bNext ? *pREnd > rSttPos : *pREnd >= rSttPos )
                {
                    pFnd = pTmp;
                    break;
                }
            }
            else
                break;
        }
    }
    return pFnd;
}


BOOL lcl_AcceptRejectRedl( Fn_AcceptReject fn_AcceptReject,
                            SwRedlineTbl& rArr, BOOL bCallDelete,
                            const SwPaM& rPam )
{
    BOOL bRet = FALSE;
    USHORT n = 0;
    const SwPosition* pStt = rPam.Start(),
                    * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                     : rPam.GetPoint();
    const SwRedline* pFnd = lcl_FindCurrRedline( *pStt, n, TRUE );
    if( pFnd &&     // neu ein Teil davon?
        ( *pFnd->Start() != *pStt || *pFnd->End() > *pEnd ))
    {
        // dann nur die TeilSelektion aufheben
        if( (*fn_AcceptReject)( rArr, n, bCallDelete, pStt, pEnd ))
            bRet = TRUE;
        ++n;
    }

    for( ; n < rArr.Count(); ++n )
    {
        SwRedline* pTmp = rArr[ n ];
        if( pTmp->HasMark() && pTmp->IsVisible() )
        {
            if( *pTmp->End() <= *pEnd )
            {
                if( (*fn_AcceptReject)( rArr, n, bCallDelete, 0, 0 ))
                    bRet = TRUE;
            }
            else
            {
                if( *pTmp->Start() < *pEnd )
                {
                    // dann nur in der TeilSelektion aufheben
                    if( (*fn_AcceptReject)( rArr, n, bCallDelete, pStt, pEnd ))
                        bRet = TRUE;
                }
                break;
            }
        }
    }
    return bRet;
}

void lcl_AdjustRedlineRange( SwPaM& rPam )
{
    // die Selektion steht nur im ContentBereich. Wenn es aber Redlines
    // davor oder dahinter auf nicht ContentNodes stehen, dann erweiter die
    // die Selection auf diese
    SwPosition* pStt = rPam.Start(),
              * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                               : rPam.GetPoint();
    SwDoc* pDoc = rPam.GetDoc();
    if( !pStt->nContent.GetIndex() &&
        !pDoc->GetNodes()[ pStt->nNode.GetIndex() - 1 ]->IsCntntNode() )
    {
        const SwRedline* pRedl = pDoc->GetRedline( *pStt );
        if( pRedl )
        {
            const SwPosition* pRStt = pRedl->Start();
            if( !pRStt->nContent.GetIndex() && pRStt->nNode.GetIndex() ==
                pStt->nNode.GetIndex() - 1 )
                *pStt = *pRStt;
        }
    }
    if( pEnd->nNode.GetNode().IsCntntNode() &&
        !pDoc->GetNodes()[ pEnd->nNode.GetIndex() + 1 ]->IsCntntNode() &&
        pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetCntntNode()->Len()    )
    {
        const SwRedline* pRedl = pDoc->GetRedline( *pEnd );
        if( pRedl )
        {
            const SwPosition* pREnd = pRedl->End();
            if( !pREnd->nContent.GetIndex() && pREnd->nNode.GetIndex() ==
                pEnd->nNode.GetIndex() + 1 )
                *pEnd = *pREnd;
        }
    }
}


BOOL SwDoc::AcceptRedline( USHORT nPos, BOOL bCallDelete )
{
    BOOL bRet = FALSE;

    // aufjedenfall auf sichtbar umschalten
    if( (REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) !=
        (REDLINE_SHOW_MASK & eRedlineMode) )
        SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE | eRedlineMode );

    SwRedline* pTmp = (*pRedlineTbl)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if( DoesUndo() )
            StartUndo( UNDO_ACCEPT_REDLINE );

        int nLoopCnt = 2;
        USHORT nSeqNo = pTmp->GetSeqNo();

        do {

            if( DoesUndo() )
                AppendUndo( new SwUndoAcceptRedline( *pTmp ));

            bRet |= lcl_AcceptRedline( *pRedlineTbl, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( USHRT_MAX == nPos )
                    nPos = 0;
                USHORT nFndPos = 2 == nLoopCnt
                                    ? pRedlineTbl->FindNextSeqNo( nSeqNo, nPos )
                                    : pRedlineTbl->FindPrevSeqNo( nSeqNo, nPos );
                if( USHRT_MAX != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    USHRT_MAX != ( nFndPos =
                        pRedlineTbl->FindPrevSeqNo( nSeqNo, nPos ))) )
                    pTmp = (*pRedlineTbl)[ nPos = nFndPos ];
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            SetModified();
        }

        if( DoesUndo() )
            EndUndo( UNDO_ACCEPT_REDLINE );
    }
    return bRet;
}

BOOL SwDoc::AcceptRedline( const SwPaM& rPam, BOOL bCallDelete )
{
    // aufjedenfall auf sichtbar umschalten
    if( (REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) !=
        (REDLINE_SHOW_MASK & eRedlineMode) )
        SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE | eRedlineMode );

    // die Selektion steht nur im ContentBereich. Wenn es aber Redlines
    // davor oder dahinter auf nicht ContentNodes stehen, dann erweiter die
    // die Selection auf diese
    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    lcl_AdjustRedlineRange( aPam );

    if( DoesUndo() )
    {
        StartUndo( UNDO_ACCEPT_REDLINE );
        AppendUndo( new SwUndoAcceptRedline( aPam ));
    }

    BOOL bRet = lcl_AcceptRejectRedl( lcl_AcceptRedline, *pRedlineTbl,
                                        bCallDelete, aPam );
    if( bRet )
    {
        CompressRedlines();
        SetModified();
    }
    if( DoesUndo() )
        EndUndo( UNDO_ACCEPT_REDLINE );
    return bRet;
}

BOOL SwDoc::RejectRedline( USHORT nPos, BOOL bCallDelete )
{
    BOOL bRet = FALSE;

    // aufjedenfall auf sichtbar umschalten
    if( (REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) !=
        (REDLINE_SHOW_MASK & eRedlineMode) )
        SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE | eRedlineMode );

    SwRedline* pTmp = (*pRedlineTbl)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if( DoesUndo() )
            StartUndo( UNDO_REJECT_REDLINE );

        int nLoopCnt = 2;
        USHORT nSeqNo = pTmp->GetSeqNo();

        do {

            if( DoesUndo() )
                AppendUndo( new SwUndoRejectRedline( *pTmp ));

            bRet |= lcl_RejectRedline( *pRedlineTbl, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( USHRT_MAX == nPos )
                    nPos = 0;
                USHORT nFndPos = 2 == nLoopCnt
                                    ? pRedlineTbl->FindNextSeqNo( nSeqNo, nPos )
                                    : pRedlineTbl->FindPrevSeqNo( nSeqNo, nPos );
                if( USHRT_MAX != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    USHRT_MAX != ( nFndPos =
                            pRedlineTbl->FindPrevSeqNo( nSeqNo, nPos ))) )
                    pTmp = (*pRedlineTbl)[ nPos = nFndPos ];
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            SetModified();
        }

        if( DoesUndo() )
            EndUndo( UNDO_REJECT_REDLINE );
    }
    return bRet;
}

BOOL SwDoc::RejectRedline( const SwPaM& rPam, BOOL bCallDelete )
{
    // aufjedenfall auf sichtbar umschalten
    if( (REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE) !=
        (REDLINE_SHOW_MASK & eRedlineMode) )
        SetRedlineMode( REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE | eRedlineMode );

    // die Selektion steht nur im ContentBereich. Wenn es aber Redlines
    // davor oder dahinter auf nicht ContentNodes stehen, dann erweiter die
    // die Selection auf diese
    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    lcl_AdjustRedlineRange( aPam );

    if( DoesUndo() )
    {
        StartUndo( UNDO_REJECT_REDLINE );
        AppendUndo( new SwUndoRejectRedline( aPam ));
    }

    BOOL bRet = lcl_AcceptRejectRedl( lcl_RejectRedline, *pRedlineTbl,
                                        bCallDelete, aPam );
    if( bRet )
    {
        CompressRedlines();
        SetModified();
    }
    if( DoesUndo() )
        EndUndo( UNDO_REJECT_REDLINE );
    return bRet;
}

const SwRedline* SwDoc::SelNextRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    BOOL bRestart;

    // sollte die StartPos auf dem letzen gueligen ContentNode stehen,
    // dann aufjedenfall das naechste Redline nehmen
    USHORT n = 0;
    const SwRedline* pFnd = lcl_FindCurrRedline( rSttPos, n, TRUE );
    if( pFnd )
    {
        const SwPosition* pEnd = pFnd->End();
        if( !pEnd->nNode.GetNode().IsCntntNode() )
        {
            SwNodeIndex aTmp( pEnd->nNode );
            SwCntntNode* pCNd = GetNodes().GoPrevSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                pCNd->Len() == rSttPos.nContent.GetIndex() ))
                pFnd = 0;
        }
        if( pFnd )
            rSttPos = *pFnd->End();
    }

    do {
        bRestart = FALSE;

        for( ; !pFnd && n < pRedlineTbl->Count(); ++n )
        {
            pFnd = (*pRedlineTbl)[ n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->Start();
                rSttPos = *pFnd->End();
                break;
            }
            else
                pFnd = 0;
        }

        if( pFnd )
        {
            // alle vom gleichen Typ und Author, die hinter einander liegen
            // zu einer Selektion zusammenfassen.
            const SwPosition* pPrevEnd = pFnd->End();
            while( ++n < pRedlineTbl->Count() )
            {
                const SwRedline* pTmp = (*pRedlineTbl)[ n ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    const SwPosition *pRStt;
                    if( pFnd->GetType() == pTmp->GetType() &&
                        pFnd->GetAuthor() == pTmp->GetAuthor() &&
                        ( *pPrevEnd == *( pRStt = pTmp->Start() ) ||
                          IsPrevPos( *pPrevEnd, *pRStt )) )
                    {
                        pPrevEnd = pTmp->End();
                        rSttPos = *pPrevEnd;
                    }
                    else
                        break;
                }
            }
        }

        if( pFnd )
        {
            const SwRedline* pSaveFnd = pFnd;

            SwCntntNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = GetNodes().GoNextSection( pIdx )) )
            {
                if( *pIdx <= rPam.GetPoint()->nNode )
                    rPam.GetMark()->nContent.Assign( pCNd, 0 );
                else
                    pFnd = 0;
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsCntntNode() &&
                    0 != ( pCNd = GetNodes().GoPrevSection( pIdx )) )
                {
                    if( *pIdx >= rPam.GetMark()->nNode )
                        rPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                    else
                        pFnd = 0;
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n < pRedlineTbl->Count() )
                {
                    bRestart = TRUE;
                    *rPam.GetPoint() = *pSaveFnd->End();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = 0;
            }
        }
    } while( bRestart );

    return pFnd;
}

const SwRedline* SwDoc::SelPrevRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    BOOL bRestart;

    // sollte die StartPos auf dem ersten gueligen ContentNode stehen,
    // dann aufjedenfall das vorherige Redline nehmen
    USHORT n = 0;
    const SwRedline* pFnd = lcl_FindCurrRedline( rSttPos, n, FALSE );
    if( pFnd )
    {
        const SwPosition* pStt = pFnd->Start();
        if( !pStt->nNode.GetNode().IsCntntNode() )
        {
            SwNodeIndex aTmp( pStt->nNode );
            SwCntntNode* pCNd = GetNodes().GoNextSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                !rSttPos.nContent.GetIndex() ))
                pFnd = 0;
        }
        if( pFnd )
            rSttPos = *pFnd->Start();
    }

    do {
        bRestart = FALSE;

        while( !pFnd && 0 < n )
        {
            pFnd = (*pRedlineTbl)[ --n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->End();
                rSttPos = *pFnd->Start();
            }
            else
                pFnd = 0;
        }

        if( pFnd )
        {
            // alle vom gleichen Typ und Author, die hinter einander liegen
            // zu einer Selektion zusammenfassen.
            const SwPosition* pNextStt = pFnd->Start();
            while( 0 < n )
            {
                const SwRedline* pTmp = (*pRedlineTbl)[ --n ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    const SwPosition *pREnd;
                    if( pFnd->GetType() == pTmp->GetType() &&
                        pFnd->GetAuthor() == pTmp->GetAuthor() &&
                        ( *pNextStt == *( pREnd = pTmp->End() ) ||
                          IsPrevPos( *pREnd, *pNextStt )) )
                    {
                        pNextStt = pTmp->Start();
                        rSttPos = *pNextStt;
                    }
                    else
                    {
                        ++n;
                        break;
                    }
                }
            }
        }

        if( pFnd )
        {
            const SwRedline* pSaveFnd = pFnd;

            SwCntntNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsCntntNode() &&
                0 != ( pCNd = GetNodes().GoPrevSection( pIdx )) )
            {
                if( *pIdx >= rPam.GetPoint()->nNode )
                    rPam.GetMark()->nContent.Assign( pCNd, pCNd->Len() );
                else
                    pFnd = 0;
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsCntntNode() &&
                    0 != ( pCNd = GetNodes().GoNextSection( pIdx )) )
                {
                    if( *pIdx <= rPam.GetMark()->nNode )
                        rPam.GetPoint()->nContent.Assign( pCNd, 0 );
                    else
                        pFnd = 0;
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n )
                {
                    bRestart = TRUE;
                    *rPam.GetPoint() = *pSaveFnd->Start();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = 0;
            }
        }
    } while( bRestart );

    return pFnd;
}

// Kommentar am Redline setzen
BOOL SwDoc::SetRedlineComment( const SwPaM& rPaM, const String& rS )
{
    BOOL bRet = FALSE;
    const SwPosition* pStt = rPaM.Start(),
                    * pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();
    USHORT n = 0;
    if( lcl_FindCurrRedline( *pStt, n, TRUE ) )
    {
        for( ; n < pRedlineTbl->Count(); ++n )
        {
            bRet = TRUE;
            SwRedline* pTmp = (*pRedlineTbl)[ n ];
            if( pStt != pEnd && *pTmp->Start() > *pEnd )
                break;

            pTmp->SetComment( rS );
            if( *pTmp->End() >= *pEnd )
                break;
        }
    }
    if( bRet )
        SetModified();

    return bRet;
}

// legt gebenenfalls einen neuen Author an
USHORT SwDoc::GetRedlineAuthor()
{
    return SW_MOD()->GetRedlineAuthor();
}

    // fuer die Reader usw. - neuen Author in die Tabelle eintragen
USHORT SwDoc::InsertRedlineAuthor( const String& rNew )
{
    return SW_MOD()->InsertRedlineAuthor(rNew);
}

void SwDoc::UpdateRedlineAttr()
{
    const SwRedlineTbl& rTbl = GetRedlineTbl();
    for( USHORT n = 0; n < rTbl.Count(); ++n )
    {
        SwRedline* pRedl = rTbl[ n ];
        if( pRedl->IsVisible() )
            pRedl->InvalidateRange();
    }
}

    // setze Kommentar-Text fuers Redline, das dann per AppendRedline
    // hereinkommt. Wird vom Autoformat benutzt. 0-Pointer setzt den Modus
    // wieder zurueck. Pointer wird nicht kopiert, muss also gueltig bleiben!
void SwDoc::SetAutoFmtRedlineComment( const String* pTxt, USHORT nSeqNo )
{
    bIsAutoFmtRedline = 0 != pTxt;
    if( pTxt )
    {
        if( !pAutoFmtRedlnComment )
            pAutoFmtRedlnComment = new String( *pTxt );
        else
            *pAutoFmtRedlnComment = *pTxt;
    }
    else if( pAutoFmtRedlnComment )
        delete pAutoFmtRedlnComment, pAutoFmtRedlnComment = 0;

    nAutoFmtRedlnCommentNo = nSeqNo;
}

/*  */

BOOL SwRedlineTbl::Insert( SwRedlinePtr& p, BOOL bIns )
{
    BOOL bRet = FALSE;
    if( p->HasValidRange() )
    {
        bRet = _SwRedlineTbl::Insert( p );
        p->CallDisplayFunc();
    }
    else if( bIns )
        bRet = InsertWithValidRanges( p );
    else
    {
        ASSERT( !this, "Redline: falscher Bereich" );
    }
    return bRet;
}

BOOL SwRedlineTbl::Insert( SwRedlinePtr& p, USHORT& rP, BOOL bIns )
{
    BOOL bRet = FALSE;
    if( p->HasValidRange() )
    {
        bRet = _SwRedlineTbl::Insert( p, rP );
        p->CallDisplayFunc();
    }
    else if( bIns )
        bRet = InsertWithValidRanges( p, &rP );
    else
    {
        ASSERT( !this, "Redline: falscher Bereich" );
    }
    return bRet;
}

BOOL SwRedlineTbl::InsertWithValidRanges( SwRedlinePtr& p, USHORT* pInsPos )
{
    // erzeuge aus den Selektion gueltige "Teilbereiche".
    BOOL bAnyIns = FALSE;
    SwPosition* pStt = p->Start(),
              * pEnd = pStt == p->GetPoint() ? p->GetMark() : p->GetPoint();
    SwPosition aNewStt( *pStt );
    SwNodes& rNds = aNewStt.nNode.GetNodes();
    SwCntntNode* pC;

    if( !aNewStt.nNode.GetNode().IsCntntNode() )
    {
        pC = rNds.GoNext( &aNewStt.nNode );
        if( pC )
            aNewStt.nContent.Assign( pC, 0 );
        else
            aNewStt.nNode = rNds.GetEndOfContent();
    }

    SwRedline* pNew = 0;
    USHORT nInsPos;

    if( aNewStt < *pEnd )
        do {
            if( !pNew )
                pNew = new SwRedline( p->GetRedlineData(), aNewStt );
            else
            {
                pNew->DeleteMark();
                *pNew->GetPoint() = aNewStt;
            }

            pNew->SetMark();
            GoEndSection( pNew->GetPoint() );
            if( *pNew->GetPoint() > *pEnd )
            {
                BOOL bWeiter = TRUE;
                pC = 0;
                if( aNewStt.nNode != pEnd->nNode )
                    do {
                        SwNode& rCurNd = aNewStt.nNode.GetNode();
                        if( rCurNd.IsStartNode() )
                        {
                            if( rCurNd.EndOfSectionIndex() < pEnd->nNode.GetIndex() )
                                aNewStt.nNode = *rCurNd.EndOfSectionNode();
                            else
                                break;
                        }
                        else if( rCurNd.IsCntntNode() )
                            pC = rCurNd.GetCntntNode();
                        aNewStt.nNode++;
                    } while( aNewStt.nNode.GetIndex() < pEnd->nNode.GetIndex() );

                if( aNewStt.nNode == pEnd->nNode )
                    aNewStt.nContent = pEnd->nContent;
                else if( pC )
                {
                    aNewStt.nNode = *pC;
                    aNewStt.nContent.Assign( pC, pC->Len() );
                }

                if( aNewStt <= *pEnd )
                    *pNew->GetPoint() = aNewStt;
            }
            else
                aNewStt = *pNew->GetPoint();

            if( *pNew->GetPoint() != *pNew->GetMark() &&
                _SwRedlineTbl::Insert( pNew, nInsPos ) )
            {
                pNew->CallDisplayFunc();
                bAnyIns = TRUE;
                pNew = 0;
                if( pInsPos && *pInsPos < nInsPos )
                    *pInsPos = nInsPos;
            }

            if( aNewStt >= *pEnd ||
                0 == (pC = rNds.GoNext( &aNewStt.nNode )) )
                break;

            aNewStt.nContent.Assign( pC, 0 );

        } while( aNewStt < *pEnd );

    delete pNew;
    delete p, p = 0;
    return bAnyIns;
}

void SwRedlineTbl::Remove( USHORT nP, USHORT nL )
{
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == _SwRedlineTbl::Count() )
        pDoc = _SwRedlineTbl::GetObject( 0 )->GetDoc();

    _SwRedlineTbl::Remove( nP, nL );

    ViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() && pDoc->GetRootFrm() &&
        0 != ( pSh = pDoc->GetRootFrm()->GetCurrShell()) )
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
}

void SwRedlineTbl::DeleteAndDestroy( USHORT nP, USHORT nL )
{
    SwDoc* pDoc = 0;
    if( !nP && nL && nL == _SwRedlineTbl::Count() )
        pDoc = _SwRedlineTbl::GetObject( 0 )->GetDoc();

    _SwRedlineTbl::DeleteAndDestroy( nP, nL );

    ViewShell* pSh;
    if( pDoc && !pDoc->IsInDtor() && pDoc->GetRootFrm() &&
        0 != ( pSh = pDoc->GetRootFrm()->GetCurrShell()) )
        pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
}

// suche den naechsten oder vorherigen Redline mit dergleichen Seq.No
// Mit dem Lookahead kann die Suche eingeschraenkt werden. 0 oder
// USHRT_MAX suchen im gesamten Array.
USHORT SwRedlineTbl::FindNextOfSeqNo( USHORT nSttPos, USHORT nLookahead ) const
{
#if 0
    USHORT nRet = USHRT_MAX, nEnd,
           nSeqNo = _SwRedlineTbl::GetObject( nSttPos )->GetSeqNo();
    if( nSeqNo )
    {
        nEnd = _SwRedlineTbl::Count();
        if( nLookahead && USHRT_MAX != nLookahead &&
            nSttPos + nLookahead < _SwRedlineTbl::Count() )
            nEnd = nSttPos + nLookahead;

        while( ++nSttPos < nEnd )
            if( nSeqNo == _SwRedlineTbl::GetObject( nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
#else
    return nSttPos + 1 < _SwRedlineTbl::Count()
                ? FindNextSeqNo( _SwRedlineTbl::GetObject( nSttPos )
                                    ->GetSeqNo(), nSttPos+1, nLookahead )
                : USHRT_MAX;
#endif
}

USHORT SwRedlineTbl::FindPrevOfSeqNo( USHORT nSttPos, USHORT nLookahead ) const
{
#if 0
    USHORT nRet = USHRT_MAX, nEnd,
           nSeqNo = _SwRedlineTbl::GetObject( nSttPos )->GetSeqNo();
    if( nSeqNo )
    {
        nEnd = 0;
        if( nLookahead && USHRT_MAX != nLookahead && nSttPos > nLookahead )
            nEnd = nSttPos - nLookahead;

        while( nSttPos-- > nEnd )
            if( nSeqNo == _SwRedlineTbl::GetObject( nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
#else
    return nSttPos ? FindPrevSeqNo( _SwRedlineTbl::GetObject(
                                        nSttPos )->GetSeqNo(),
                                    nSttPos-1, nLookahead )
                   : USHRT_MAX;
#endif
}

USHORT SwRedlineTbl::FindNextSeqNo( USHORT nSeqNo, USHORT nSttPos,
                                    USHORT nLookahead ) const
{
    USHORT nRet = USHRT_MAX, nEnd;
    if( nSeqNo && nSttPos < _SwRedlineTbl::Count() )
    {
        nEnd = _SwRedlineTbl::Count();
        if( nLookahead && USHRT_MAX != nLookahead &&
            nSttPos + nLookahead < _SwRedlineTbl::Count() )
            nEnd = nSttPos + nLookahead;

        for( ; nSttPos < nEnd; ++nSttPos )
            if( nSeqNo == _SwRedlineTbl::GetObject( nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
}

USHORT SwRedlineTbl::FindPrevSeqNo( USHORT nSeqNo, USHORT nSttPos,
                                    USHORT nLookahead ) const
{
    USHORT nRet = USHRT_MAX, nEnd;
    if( nSeqNo && nSttPos < _SwRedlineTbl::Count() )
    {
        nEnd = 0;
        if( nLookahead && USHRT_MAX != nLookahead && nSttPos > nLookahead )
            nEnd = nSttPos - nLookahead;

        ++nSttPos;
        while( nSttPos > nEnd )
            if( nSeqNo == _SwRedlineTbl::GetObject( --nSttPos )->GetSeqNo() )
            {
                nRet = nSttPos;
                break;
            }
    }
    return nRet;
}

/*  */

SwRedlineExtraData::~SwRedlineExtraData()
{
}

void SwRedlineExtraData::Accept( SwPaM& ) const
{
}

void SwRedlineExtraData::Reject( SwPaM& ) const
{
}

int SwRedlineExtraData::operator == ( const SwRedlineExtraData& ) const
{
    return FALSE;
}


SwRedlineExtraData_FmtColl::SwRedlineExtraData_FmtColl( const String& rColl,
                                                USHORT nPoolFmtId,
                                                const SfxItemSet* pItemSet )
    : sFmtNm( rColl ), nPoolId( nPoolFmtId ), pSet( 0 )
{
    if( pItemSet && pItemSet->Count() )
        pSet = new SfxItemSet( *pItemSet );
}

SwRedlineExtraData_FmtColl::~SwRedlineExtraData_FmtColl()
{
    delete pSet;
}

SwRedlineExtraData* SwRedlineExtraData_FmtColl::CreateNew() const
{
    return new SwRedlineExtraData_FmtColl( sFmtNm, nPoolId, pSet );
}

void SwRedlineExtraData_FmtColl::Reject( SwPaM& rPam ) const
{
    SwDoc* pDoc = rPam.GetDoc();

// was ist mit Undo ? ist das abgeschaltet ??
    SwTxtFmtColl* pColl = USHRT_MAX == nPoolId
                            ? pDoc->FindTxtFmtCollByName( sFmtNm )
                            : pDoc->GetTxtCollFromPool( nPoolId );
    if( pColl )
        pDoc->SetTxtFmtColl( rPam, pColl, FALSE );

    if( pSet )
    {
        rPam.SetMark();
        SwPosition& rMark = *rPam.GetMark();
        SwTxtNode* pTNd = rMark.nNode.GetNode().GetTxtNode();
        if( pTNd )
        {
            rMark.nContent.Assign( pTNd, pTNd->GetTxt().Len() );

            if( pTNd->GetpSwAttrSet() )
            {
                // nur die setzen, die nicht mehr vorhanden sind. Andere
                // koennen jetzt veraendert drin stehen, aber die werden
                // nicht angefasst.
                SfxItemSet aTmp( *pSet );
                aTmp.Differentiate( *pTNd->GetpSwAttrSet() );
                pDoc->Insert( rPam, aTmp );
            }
            else
                pDoc->Insert( rPam, *pSet );
        }
        rPam.DeleteMark();
    }
}

int SwRedlineExtraData_FmtColl::operator == ( const SwRedlineExtraData& r) const
{
    const SwRedlineExtraData_FmtColl& rCmp = (SwRedlineExtraData_FmtColl&)r;
    return sFmtNm == rCmp.sFmtNm && nPoolId == rCmp.nPoolId &&
            ( ( !pSet && !rCmp.pSet ) ||
               ( pSet && rCmp.pSet && *pSet == *rCmp.pSet ) );
}

void SwRedlineExtraData_FmtColl::SetItemSet( const SfxItemSet& rSet )
{
    delete pSet;
    if( rSet.Count() )
        pSet = new SfxItemSet( rSet );
    else
        pSet = 0;
}


SwRedlineExtraData_Format::SwRedlineExtraData_Format( USHORT nW )
    : nWhich( nW )
{
}


SwRedlineExtraData* SwRedlineExtraData_Format::CreateNew() const
{
    return new SwRedlineExtraData_Format( nWhich );
}

void SwRedlineExtraData_Format::Reject( SwPaM& rPam ) const
{
    SwDoc* pDoc = rPam.GetDoc();

    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( eOld & ~(REDLINE_ON | REDLINE_IGNORE) );

    // eigentlich muesste hier das Attribut zurueck gesetzt werden!!!
    pDoc->Insert( rPam, *GetDfltAttr( nWhich ), SETATTR_DONTEXPAND );

    pDoc->SetRedlineMode_intern( eOld );
}

int SwRedlineExtraData_Format::operator == ( const SwRedlineExtraData& rCmp ) const
{
    return nWhich == ((SwRedlineExtraData_Format&)rCmp).nWhich;
}

/*  */

SwRedlineData::SwRedlineData( SwRedlineType eT, USHORT nAut )
    : eType( eT ), pNext( 0 ), nAuthor( nAut ), pExtraData( 0 ), nSeqNo( 0 )
{
    aStamp.SetSec( 0 );
    aStamp.Set100Sec( 0 );
}

SwRedlineData::SwRedlineData( const SwRedlineData& rCpy, BOOL bCpyNext )
    : nAuthor( rCpy.nAuthor ), eType( rCpy.eType ), aStamp( rCpy.aStamp ),
    sComment( rCpy.sComment ), nSeqNo( rCpy.nSeqNo ),
    pExtraData( rCpy.pExtraData ? rCpy.pExtraData->CreateNew() : 0 ),
    pNext( (bCpyNext && rCpy.pNext) ? new SwRedlineData( *rCpy.pNext ) : 0 )
{
}

    // fuer sw3io: pNext geht in eigenen Besitz ueber!
SwRedlineData::SwRedlineData( SwRedlineType eT, USHORT nAut, const DateTime& rDT,
                   const String& rCmnt, SwRedlineData *pNxt,
                   SwRedlineExtraData* pData )
    : eType( eT ), pNext( pNxt ), nAuthor( nAut ), aStamp( rDT ),
      sComment( rCmnt ), pExtraData( pData ), nSeqNo( 0 )
{
}

SwRedlineData::~SwRedlineData()
{
    delete pExtraData;
    delete pNext;
}

    // ExtraData wird kopiert, der Pointer geht also NICHT in den Besitz
    // des RedlineObjectes!
void SwRedlineData::SetExtraData( const SwRedlineExtraData* pData )
{
    delete pExtraData;

    if( pData )
        pExtraData = pData->CreateNew();
    else
        pExtraData = 0;
}

/*  */

SwRedline::SwRedline( SwRedlineType eTyp, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pCntntSect( 0 ),
    pRedlineData( new SwRedlineData( eTyp, GetDoc()->GetRedlineAuthor() ) )
{
    bDelLastPara = bIsLastParaDelete = FALSE;
    bIsVisible = TRUE;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRedline::SwRedline( SwRedlineType eTyp, const SwPosition& rPos )
    : SwPaM( rPos ),
    pCntntSect( 0 ),
    pRedlineData( new SwRedlineData( eTyp, GetDoc()->GetRedlineAuthor() ) )
{
    bDelLastPara = bIsLastParaDelete = FALSE;
    bIsVisible = TRUE;
}

SwRedline::SwRedline( const SwRedlineData& rData, const SwPaM& rPam )
    : SwPaM( *rPam.GetMark(), *rPam.GetPoint() ),
    pCntntSect( 0 ),
    pRedlineData( new SwRedlineData( rData ))
{
    bDelLastPara = bIsLastParaDelete = FALSE;
    bIsVisible = TRUE;
    if( !rPam.HasMark() )
        DeleteMark();
}

SwRedline::SwRedline( const SwRedlineData& rData, const SwPosition& rPos )
    : SwPaM( rPos ),
    pCntntSect( 0 ),
    pRedlineData( new SwRedlineData( rData ))
{
    bDelLastPara = bIsLastParaDelete = FALSE;
    bIsVisible = TRUE;
}

SwRedline::SwRedline( const SwRedline& rCpy )
    : SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
    pCntntSect( 0 ),
    pRedlineData( new SwRedlineData( *rCpy.pRedlineData ))
{
    bDelLastPara = bIsLastParaDelete = FALSE;
    bIsVisible = TRUE;
    if( !rCpy.HasMark() )
        DeleteMark();
}

SwRedline::~SwRedline()
{
    if( pCntntSect )
    {
        // dann den Content Bereich loeschen
        if( !GetDoc()->IsInDtor() )
            GetDoc()->DeleteSection( &pCntntSect->GetNode() );
        delete pCntntSect;
    }
    delete pRedlineData;
}

// liegt eine gueltige Selektion vor?
BOOL SwRedline::HasValidRange() const
{
    const SwNode* pPtNd = &GetPoint()->nNode.GetNode(),
                * pMkNd = &GetMark()->nNode.GetNode();
    if( pPtNd->FindStartNode() == pMkNd->FindStartNode() &&
        !pPtNd->FindStartNode()->IsTableNode() )
        return TRUE;
/*
    if( ( pPtNd->IsStartNode() && pMkNd->IsEndNode() &&
            pMkNd->FindStartNode() == pPtNd ) ||
        ( pMkNd->IsStartNode() && pPtNd->IsEndNode() &&
            pPtNd->FindStartNode() == pMkNd ) )
        return TRUE;
*/
    return FALSE;
}

void SwRedline::CallDisplayFunc( USHORT nLoop )
{
    switch( REDLINE_SHOW_MASK & GetDoc()->GetRedlineMode() )
    {
    case REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE:
        Show( nLoop );
        break;
    case REDLINE_SHOW_INSERT:
        Hide( nLoop );
        break;
    case REDLINE_SHOW_DELETE:
        ShowOriginal( nLoop );
        break;
    }
}

void SwRedline::Show( USHORT nLoop )
{
    if( 1 <= nLoop )
    {
        SwDoc* pDoc = GetDoc();
        SwRedlineMode eOld = pDoc->GetRedlineMode();
        pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );
        BOOL bUndo = pDoc->DoesUndo();
        pDoc->DoUndo( FALSE );

        switch( GetType() )
        {
        case REDLINE_INSERT:            // Inhalt wurde eingefuegt
            bIsVisible = TRUE;
            MoveFromSection();
            break;

        case REDLINE_DELETE:            // Inhalt wurde geloescht
            bIsVisible = TRUE;
            MoveFromSection();
            break;

        case REDLINE_FORMAT:            // Attributierung wurde angewendet
        case REDLINE_TABLE:             // TabellenStruktur wurde veraendert
            InvalidateRange();
            break;
        }
        pDoc->SetRedlineMode_intern( eOld );
        pDoc->DoUndo( bUndo );
    }
}

void SwRedline::Hide( USHORT nLoop )
{
    SwDoc* pDoc = GetDoc();
    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );
    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    switch( GetType() )
    {
    case REDLINE_INSERT:            // Inhalt wurde eingefuegt
        bIsVisible = TRUE;
        if( 1 <= nLoop )
            MoveFromSection();
        break;

    case REDLINE_DELETE:            // Inhalt wurde geloescht
        bIsVisible = FALSE;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(); break;
        }
        break;

    case REDLINE_FORMAT:            // Attributierung wurde angewendet
    case REDLINE_TABLE:             // TabellenStruktur wurde veraendert
        if( 1 <= nLoop )
            InvalidateRange();
        break;
    }
    pDoc->SetRedlineMode_intern( eOld );
    pDoc->DoUndo( bUndo );
}

void SwRedline::ShowOriginal( USHORT nLoop )
{
    SwDoc* pDoc = GetDoc();
    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );
    BOOL bUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    // bestimme den Type, ist der erste auf Stack
    for( SwRedlineData* pCur = pRedlineData; pCur->pNext; )
        pCur = pCur->pNext;

    switch( pCur->eType )
    {
    case REDLINE_INSERT:            // Inhalt wurde eingefuegt
        bIsVisible = FALSE;
        switch( nLoop )
        {
        case 0: MoveToSection();    break;
        case 1: CopyToSection();    break;
        case 2: DelCopyOfSection(); break;
        }
        break;

    case REDLINE_DELETE:            // Inhalt wurde geloescht
        bIsVisible = TRUE;
        if( 1 <= nLoop )
            MoveFromSection();
        break;

    case REDLINE_FORMAT:            // Attributierung wurde angewendet
    case REDLINE_TABLE:             // TabellenStruktur wurde veraendert
        if( 1 <= nLoop )
            InvalidateRange();
        break;
    }
    pDoc->SetRedlineMode_intern( eOld );
    pDoc->DoUndo( bUndo );
}


void SwRedline::InvalidateRange()       // das Layout anstossen
{
    ULONG nSttNd = GetMark()->nNode.GetIndex(),
            nEndNd = GetPoint()->nNode.GetIndex();
    USHORT nSttCnt = GetMark()->nContent.GetIndex(),
            nEndCnt = GetPoint()->nContent.GetIndex();

    if( nSttNd > nEndNd || ( nSttNd == nEndNd && nSttCnt > nEndCnt ))
    {
        ULONG nTmp = nSttNd; nSttNd = nEndNd; nEndNd = nTmp;
        nTmp = nSttCnt; nSttCnt = nEndCnt; nEndCnt = (USHORT)nTmp;
    }

    SwUpdateAttr aHt( 0, 0, RES_FMT_CHG );
    SwNodes& rNds = GetDoc()->GetNodes();
    SwNode* pNd;
    for( ULONG n = nSttNd; n <= nEndNd; ++n )
        if( ND_TEXTNODE == ( pNd = rNds[ n ] )->GetNodeType() )
        {
            aHt.nStart = n == nSttNd ? nSttCnt : 0;
            aHt.nEnd = n == nEndNd ? nEndCnt : ((SwTxtNode*)pNd)->GetTxt().Len();
            ((SwTxtNode*)pNd)->Modify( &aHt, &aHt );
        }
}

void SwRedline::MoveToSection()
{
    if( !pCntntSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwDoc* pDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwCntntNode* pCSttNd = pStt->nNode.GetNode().GetCntntNode();
        SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();

        BOOL bNoLastPara = FALSE;

        if( !pCSttNd )
        {
            // damit die Indizies der anderen Redlines nicht mitverschoben
            // werden, diese aufs Ende setzen (ist exclusive).
            const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
            for( USHORT n = 0; n < rTbl.Count(); ++n )
            {
                SwRedline* pRedl = rTbl[ n ];
                if( pRedl->GetBound(TRUE) == *pStt )
                    pRedl->GetBound(TRUE) = *pEnd;
                if( pRedl->GetBound(FALSE) == *pStt )
                    pRedl->GetBound(FALSE) = *pEnd;
            }
        }

        SwStartNode* pSttNd;
        SwNodes& rNds = pDoc->GetNodes();
        if( pCSttNd || pCEndNd )
        {
            SwTxtFmtColl* pColl = (pCSttNd && pCSttNd->IsTxtNode() )
                                    ? ((SwTxtNode*)pCSttNd)->GetTxtColl()
                                    : (pCEndNd && pCEndNd->IsTxtNode() )
                                        ? ((SwTxtNode*)pCEndNd)->GetTxtColl()
                                        : pDoc->GetTxtCollFromPool(
                                                RES_POOLCOLL_STANDARD );

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );
            SwTxtNode* pTxtNd = rNds[ pSttNd->GetIndex() + 1 ]->GetTxtNode();

            SwNodeIndex aNdIdx( *pTxtNd );
            SwPosition aPos( aNdIdx, SwIndex( pTxtNd ));
            if( pCSttNd && pCEndNd )
                pDoc->MoveAndJoin( aPam, aPos );
            else
            {
                if( pCSttNd && !pCEndNd )
                    bDelLastPara = TRUE;
                pDoc->Move( aPam, aPos );
            }
        }
        else
        {
            pSttNd = rNds.MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode );

            SwPosition aPos( *pSttNd->EndOfSectionNode() );
            pDoc->Move( aPam, aPos );
        }
        pCntntSect = new SwNodeIndex( *pSttNd );

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
    else
        InvalidateRange();
}

void SwRedline::CopyToSection()
{
    if( !pCntntSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwCntntNode* pCSttNd = pStt->nNode.GetNode().GetCntntNode();
        SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();

        SwStartNode* pSttNd;
        SwDoc* pDoc = GetDoc();
        SwNodes& rNds = pDoc->GetNodes();

        BOOL bSaveCopyFlag = pDoc->IsCopyIsMove(),
             bSaveRdlMoveFlg = pDoc->IsRedlineMove();
        pDoc->SetCopyIsMove( TRUE );
        pDoc->SetRedlineMove( TRUE );

        if( pCSttNd )
        {
            SwTxtFmtColl* pColl = (pCSttNd && pCSttNd->IsTxtNode() )
                                    ? ((SwTxtNode*)pCSttNd)->GetTxtColl()
                                    : pDoc->GetTxtCollFromPool(
                                                RES_POOLCOLL_STANDARD );

            pSttNd = rNds.MakeTextSection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode, pColl );

            SwNodeIndex aNdIdx( *pSttNd, 1 );
            SwTxtNode* pTxtNd = aNdIdx.GetNode().GetTxtNode();
            SwPosition aPos( aNdIdx, SwIndex( pTxtNd ));
            pDoc->Copy( *this, aPos );

            // JP 08.10.98: die Vorlage vom EndNode ggfs. mit uebernehmen
            //              - ist im Doc::Copy nicht erwuenscht
            if( pCEndNd && pCEndNd != pCSttNd )
            {
                SwCntntNode* pDestNd = aPos.nNode.GetNode().GetCntntNode();
                if( pDestNd )
                {
                    if( pDestNd->IsTxtNode() && pCEndNd->IsTxtNode() )
                        ((SwTxtNode*)pCEndNd)->CopyCollFmt(
                                            *(SwTxtNode*)pDestNd );
                    else
                        pDestNd->ChgFmtColl( pCEndNd->GetFmtColl() );
                }
            }
        }
        else
        {
            pSttNd = rNds.MakeEmptySection( SwNodeIndex( rNds.GetEndOfRedlines() ),
                                            SwNormalStartNode );

            if( pCEndNd )
            {
                SwPosition aPos( *pSttNd->EndOfSectionNode() );
                pDoc->Copy( *this, aPos );
            }
            else
            {
                SwNodeIndex aInsPos( *pSttNd->EndOfSectionNode() );
                SwNodeRange aRg( pStt->nNode, 0, pEnd->nNode, 1 );
                pDoc->CopyWithFlyInFly( aRg, aInsPos );
            }
        }
        pCntntSect = new SwNodeIndex( *pSttNd );

        pDoc->SetCopyIsMove( bSaveCopyFlag );
        pDoc->SetRedlineMove( bSaveRdlMoveFlg );
    }
}

void SwRedline::DelCopyOfSection()
{
    if( pCntntSect )
    {
        const SwPosition* pStt = Start(),
                        * pEnd = pStt == GetPoint() ? GetMark() : GetPoint();

        SwDoc* pDoc = GetDoc();
        SwPaM aPam( *pStt, *pEnd );
        SwCntntNode* pCSttNd = pStt->nNode.GetNode().GetCntntNode();
        SwCntntNode* pCEndNd = pEnd->nNode.GetNode().GetCntntNode();

        if( !pCSttNd )
        {
            // damit die Indizies der anderen Redlines nicht mitverschoben
            // werden, diese aufs Ende setzen (ist exclusive).
            const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
            for( USHORT n = 0; n < rTbl.Count(); ++n )
            {
                SwRedline* pRedl = rTbl[ n ];
                if( pRedl->GetBound(TRUE) == *pStt )
                    pRedl->GetBound(TRUE) = *pEnd;
                if( pRedl->GetBound(FALSE) == *pStt )
                    pRedl->GetBound(FALSE) = *pEnd;
            }
        }

        SwNodes& rNds = pDoc->GetNodes();
        if( pCSttNd && pCEndNd )
            pDoc->DeleteAndJoin( aPam );
        else if( pCSttNd || pCEndNd )
        {
            if( pCSttNd && !pCEndNd )
                bDelLastPara = TRUE;
            pDoc->Delete( aPam );

            if( bDelLastPara )
            {
                SvPtrarr aBehindArr( 16, 16 );
                const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
                USHORT n = rTbl.GetPos( this );
                for( BOOL bBreak = FALSE; !bBreak && n < rTbl.Count(); ++n )
                {
                    bBreak = TRUE;
                    if( rTbl[ n ]->GetBound(TRUE) == *pEnd )
                    {
                        void* pTmp = &rTbl[ n ]->GetBound(TRUE);
                        aBehindArr.Insert( pTmp, aBehindArr.Count());
                        bBreak = FALSE;
                    }
                    if( rTbl[ n ]->GetBound(FALSE) == *pEnd )
                    {
                        void* pTmp = &rTbl[ n ]->GetBound(FALSE);
                        aBehindArr.Insert( pTmp, aBehindArr.Count() );
                        bBreak = FALSE;
                    }
                }

                SwPosition aEnd( *pEnd );

                aPam.GetPoint()->nContent.Assign( 0, 0 );
                aPam.GetMark()->nContent.Assign( 0, 0 );
                aPam.DeleteMark();
                pDoc->DelFullPara( aPam );

                for( n = 0; n < aBehindArr.Count(); ++n )
                    *(SwPosition*)aBehindArr[ n ] = aEnd;
            }
        }
        else
            pDoc->Delete( aPam );

        if( pStt == GetPoint() )
            Exchange();

        DeleteMark();
    }
}

void SwRedline::MoveFromSection()
{
    if( pCntntSect )
    {
        SwDoc* pDoc = GetDoc();
        const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
        SvPtrarr aBeforeArr( 16, 16 ), aBehindArr( 16, 16 );
        USHORT nMyPos = rTbl.GetPos( this );
        ASSERT( this, "this nicht im Array?" );
        register BOOL bBreak = FALSE;

        for( USHORT n = nMyPos+1; !bBreak && n < rTbl.Count(); ++n )
        {
            bBreak = TRUE;
            if( rTbl[ n ]->GetBound(TRUE) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(TRUE);
                aBehindArr.Insert( pTmp, aBehindArr.Count());
                bBreak = FALSE;
            }
            if( rTbl[ n ]->GetBound(FALSE) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(FALSE);
                aBehindArr.Insert( pTmp, aBehindArr.Count() );
                bBreak = FALSE;
            }
        }
        for( bBreak = FALSE, n = nMyPos; !bBreak && n ; )
        {
            --n;
            bBreak = TRUE;
            if( rTbl[ n ]->GetBound(TRUE) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(TRUE);
                aBeforeArr.Insert( pTmp, aBeforeArr.Count() );
                bBreak = FALSE;
            }
            if( rTbl[ n ]->GetBound(FALSE) == *GetPoint() )
            {
                void* pTmp = &rTbl[ n ]->GetBound(FALSE);
                aBeforeArr.Insert( pTmp, aBeforeArr.Count() );
                bBreak = FALSE;
            }
        }

        {
            BOOL bAddFlag = TRUE;
            SwPaM aPam( pCntntSect->GetNode(),
                        *pCntntSect->GetNode().EndOfSectionNode(), 1,
                        ( bDelLastPara ? -2 : -1 ) );
            SwCntntNode* pCNd = aPam.GetCntntNode();
            if( pCNd )
                aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            else
                aPam.GetPoint()->nNode++;

            SwFmtColl* pColl = pCNd && aPam.GetPoint()->nNode !=
                                        aPam.GetMark()->nNode
                                ? pCNd->GetFmtColl() : 0;

            SwNodeIndex aNdIdx( GetPoint()->nNode, -1 );
            USHORT nPos = GetPoint()->nContent.GetIndex();

            SwPosition aPos( *GetPoint() );
            pDoc->Move( aPam, aPos, DOC_MOVEALLFLYS );
            SetMark();
            *GetPoint() = aPos;
            GetMark()->nNode = aNdIdx.GetIndex() + 1;
            pCNd = GetMark()->nNode.GetNode().GetCntntNode();
            GetMark()->nContent.Assign( pCNd, nPos );

            if( bDelLastPara )
            {
                GetPoint()->nNode++;
                GetPoint()->nContent.Assign( pCNd = GetCntntNode(), 0 );
                bDelLastPara = FALSE;
            }
            else if( pColl )
                pCNd = GetCntntNode();

            if( pColl && pCNd )
                pCNd->ChgFmtColl( pColl );
        }
        pDoc->DeleteSection( &pCntntSect->GetNode() );
        delete pCntntSect, pCntntSect = 0;

        for( n = 0; n < aBeforeArr.Count(); ++n )
            *(SwPosition*)aBeforeArr[ n ] = *GetMark();
        for( n = 0; n < aBehindArr.Count(); ++n )
            *(SwPosition*)aBehindArr[ n ] = *GetPoint();
    }
    else
        InvalidateRange();
}

// fuers Undo
void SwRedline::SetContentIdx( const SwNodeIndex* pIdx )
{
    if( pIdx && !pCntntSect )
    {
        pCntntSect = new SwNodeIndex( *pIdx );
        bIsVisible = FALSE;
    }
    else if( !pIdx && pCntntSect )
    {
        delete pCntntSect, pCntntSect = 0;
        bIsVisible = FALSE;
    }
#ifndef PRODUCT
    else
        ASSERT( !this, "das ist keine gueltige Operation" );
#endif
}

BOOL SwRedline::CanCombine( const SwRedline& rRedl ) const
{
    return  IsVisible() && rRedl.IsVisible() &&
            pRedlineData->CanCombine( *rRedl.pRedlineData );
}

void SwRedline::PushData( const SwRedline& rRedl, BOOL bOwnAsNext )
{
//  SwRedlineData* pNew = new SwRedlineData( rRedl.GetType(),
//                                           rRedl.GetAuthor() );
    SwRedlineData* pNew = new SwRedlineData( *rRedl.pRedlineData, FALSE );
    if( bOwnAsNext )
    {
        pNew->pNext = pRedlineData;
        pRedlineData = pNew;
    }
    else
    {
        pNew->pNext = pRedlineData->pNext;
        pRedlineData->pNext = pNew;
    }
}

BOOL SwRedline::PopData()
{
    if( !pRedlineData->pNext )
        return FALSE;
    SwRedlineData* pCur = pRedlineData;
    pRedlineData = pCur->pNext;
    pCur->pNext = 0;
    delete pCur;
    return TRUE;
}

USHORT SwRedline::GetStackCount() const
{
    USHORT nRet = 1;
    for( SwRedlineData* pCur = pRedlineData; pCur->pNext; ++nRet )
        pCur = pCur->pNext;
    return nRet;
}

USHORT SwRedline::GetAuthor( USHORT nPos ) const
{
    for( SwRedlineData* pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->nAuthor;
}

const String& SwRedline::GetAuthorString( USHORT nPos ) const
{
    for( SwRedlineData* pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return SW_MOD()->GetRedlineAuthor(pCur->nAuthor);
}

const DateTime& SwRedline::GetTimeStamp( USHORT nPos ) const
{
    for( SwRedlineData* pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->aStamp;
}

SwRedlineType SwRedline::GetRealType( USHORT nPos ) const
{
    for( SwRedlineData* pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->eType;
}

const String& SwRedline::GetComment( USHORT nPos ) const
{
    for( SwRedlineData* pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->sComment;
}

int SwRedline::operator==( const SwRedline& rCmp ) const
{
    return this == &rCmp;
}

int SwRedline::operator<( const SwRedline& rCmp ) const
{
    BOOL bLower = *Start() < *rCmp.Start();
    if( !bLower && *Start() == *rCmp.Start() && !HasMark() )
        bLower = TRUE;
    return bLower;
}




