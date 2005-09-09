/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crbm.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:03:06 $
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


#pragma hdrstop

#include "doc.hxx"
#include "crsrsh.hxx"
#include "ndtxt.hxx"
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#include "bookmrk.hxx"
#include "callnk.hxx"
#include "swcrsr.hxx"


/*
 * Methoden der SwCrsrShell fuer Bookmark
 */


// am CurCrsr.SPoint
FASTBOOL SwCrsrShell::SetBookmark( const KeyCode& rCode, const String& rName,
                                const String& rShortName, BOOKMARK_TYPE eMark )
{
    StartAction();
    FASTBOOL bRet = 0 != pDoc->MakeBookmark( *GetCrsr(), rCode, rName,
                                            rShortName, eMark);
    EndAction();
    return bRet;
}
// setzt CurCrsr.SPoint


FASTBOOL SwCrsrShell::GotoBookmark(USHORT nPos, BOOL bAtStart)
{
    // Crsr-Moves ueberwachen, evt. Link callen
    FASTBOOL bRet = TRUE;
    SwCallLink aLk( *this );

    SwBookmark* pBkmk = pDoc->GetBookmarks()[ nPos ];
    SwCursor* pCrsr = GetSwCrsr();
    SwCrsrSaveState aSaveState( *pCrsr );

    if( pBkmk->GetOtherPos() )
    {
        if( bAtStart )
            *pCrsr->GetPoint() = *pBkmk->GetOtherPos() < pBkmk->GetPos()
                                    ? *pBkmk->GetOtherPos()
                                    : pBkmk->GetPos();
        else
            *pCrsr->GetPoint() = *pBkmk->GetOtherPos() > pBkmk->GetPos()
                                    ? *pBkmk->GetOtherPos()
                                    : pBkmk->GetPos();
    }
    else
        *pCrsr->GetPoint() = pBkmk->GetPos();

    if( pCrsr->IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE ) )
    {
        pCrsr->DeleteMark();
        pCrsr->RestoreSavePos();
        bRet = FALSE;
    }
    else
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


FASTBOOL SwCrsrShell::GotoBookmark(USHORT nPos)
{
    // Crsr-Moves ueberwachen, evt. Link callen
    FASTBOOL bRet = TRUE;
    SwCallLink aLk( *this );
    SwBookmark* pBkmk = pDoc->GetBookmarks()[ nPos ];
    SwCursor* pCrsr = GetSwCrsr();
    SwCrsrSaveState aSaveState( *pCrsr );

    *pCrsr->GetPoint() = pBkmk->GetPos();
    if( pBkmk->GetOtherPos() )
    {
        pCrsr->SetMark();
        *pCrsr->GetMark() = *pBkmk->GetOtherPos();
        if( *pCrsr->GetMark() > *pCrsr->GetPoint() )
            pCrsr->Exchange();
    }

    if( pCrsr->IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE ) )
    {
        pCrsr->DeleteMark();
        pCrsr->RestoreSavePos();
        bRet = FALSE;
    }
    else
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}
// TRUE, wenn's noch eine gab


FASTBOOL SwCrsrShell::GoNextBookmark()
{
    SwBookmark aBM(*GetCrsr()->GetPoint());
    USHORT nPos;
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    rBkmks.Seek_Entry( &aBM, &nPos );
    if ( nPos == rBkmks.Count() )
        return FALSE;

    // alle die Inhaltlich auf der gleichen Position stehen, ueberspringen
    while( aBM.IsEqualPos( *rBkmks[ nPos ] ))
        if( ++nPos == rBkmks.Count() )
            return FALSE;

    while( !GotoBookmark( nPos ))
        if( ++nPos == rBkmks.Count() )
            return FALSE;

    return TRUE;
}


FASTBOOL SwCrsrShell::GoPrevBookmark()
{
    const SwBookmarks& rBkmks = pDoc->GetBookmarks();
    if ( !rBkmks.Count() )
        return FALSE;

    USHORT nPos;
    SwCursor* pCrsr = GetSwCrsr();
    SwBookmark aBM( *pCrsr->GetPoint() );
    rBkmks.Seek_Entry( &aBM, &nPos );

    const SwBookmark* pBkmk;
    // alle die Inhaltlich auf der gleichen Position stehen, ueberspringen
    do {
        if ( nPos == 0 )
            return FALSE;
    } while( aBM < *(pBkmk = rBkmks[--nPos]) || aBM.IsEqualPos( *pBkmk ));

    SwCallLink aLk( *this );
    SwCrsrSaveState aSaveState( *pCrsr );

    FASTBOOL bRet = FALSE;
    do {
        pBkmk = rBkmks[ nPos ];

        *pCrsr->GetPoint() = pBkmk->GetPos();
        if( pBkmk->GetOtherPos() )
        {
            pCrsr->SetMark();
            *pCrsr->GetMark() = *pBkmk->GetOtherPos();
            if( *pCrsr->GetMark() < *pCrsr->GetPoint() )
                pCrsr->Exchange();
        }
        if( !pCrsr->IsSelOvr( SELOVER_CHECKNODESSECTION | SELOVER_TOGGLE ) )
        {
            UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
            bRet = TRUE;
        }

    } while( !bRet && nPos-- );

    if( !bRet )
    {
        pCrsr->DeleteMark();
        pCrsr->RestoreSavePos();
    }

    return bRet;
}



USHORT SwCrsrShell::GetBookmarkCnt(BOOL bBkmrk) const
{
    return pDoc->GetBookmarkCnt(bBkmrk);
}


SwBookmark& SwCrsrShell::GetBookmark(USHORT nPos, BOOL bBkmrk)
{
    return pDoc->GetBookmark(nPos, bBkmrk);
}


void SwCrsrShell::DelBookmark(USHORT nPos)
{
    StartAction();
    pDoc->DelBookmark(nPos);
    EndAction();
}


void SwCrsrShell::DelBookmark( const String& rName )
{
    StartAction();
    pDoc->DelBookmark( rName );
    EndAction();
}


USHORT SwCrsrShell::FindBookmark( const String& rName )
{
    return pDoc->FindBookmark( rName );
}


        // erzeugt einen eindeutigen Namen. Der Name selbst muss vorgegeben
        // werden, es wird dann bei gleichen Namen nur durchnumeriert.
void SwCrsrShell::MakeUniqueBookmarkName( String& rName )
{
    pDoc->MakeUniqueBookmarkName( rName );
}



