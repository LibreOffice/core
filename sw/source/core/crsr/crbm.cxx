/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crbm.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:33:54 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include "crsrsh.hxx"
#include "ndtxt.hxx"
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#include "bookmrk.hxx"
#include "callnk.hxx"
#include "swcrsr.hxx"
#include <IDocumentBookmarkAccess.hxx>

/*
 * Methoden der SwCrsrShell fuer Bookmark
 */


// am CurCrsr.SPoint
BOOL SwCrsrShell::SetBookmark( const KeyCode& rCode, const String& rName,
                                const String& rShortName, IDocumentBookmarkAccess::BookmarkType eMark )
{
    StartAction();
    BOOL bRet = 0 != getIDocumentBookmarkAccess()->makeBookmark( *GetCrsr(), rCode, rName,
                                                                    rShortName, eMark);
    EndAction();
    return bRet;
}
// setzt CurCrsr.SPoint


BOOL SwCrsrShell::GotoBookmark(USHORT nPos, BOOL bAtStart)
{
    // Crsr-Moves ueberwachen, evt. Link callen
    BOOL bRet = TRUE;
    SwCallLink aLk( *this );

    SwBookmark* pBkmk = getIDocumentBookmarkAccess()->getBookmarks()[ nPos ];
    SwCursor* pCrsr = GetSwCrsr();
    SwCrsrSaveState aSaveState( *pCrsr );

    // --> OD 2007-09-27 #i81002# - refactoring
    // simplify by using <SwBookmark::BookmarkStart()/BookmarkEnd()>
//    if( pBkmk->GetOtherBookmarkPos() )
//  {
//      if( bAtStart )
//            *pCrsr->GetPoint() = *pBkmk->GetOtherBookmarkPos() < pBkmk->GetBookmarkPos()
//                                    ? *pBkmk->GetOtherBookmarkPos()
//                                    : pBkmk->GetBookmarkPos();
//      else
//            *pCrsr->GetPoint() = *pBkmk->GetOtherBookmarkPos() > pBkmk->GetBookmarkPos()
//                                    ? *pBkmk->GetOtherBookmarkPos()
//                                    : pBkmk->GetBookmarkPos();
//  }
//  else
//        *pCrsr->GetPoint() = pBkmk->GetBookmarkPos();
    if ( bAtStart )
    {
        *pCrsr->GetPoint() = *pBkmk->BookmarkStart();
    }
    else
    {
        *pCrsr->GetPoint() = *pBkmk->BookmarkEnd();
    }
    // <--

    if( pCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                         nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
    {
        pCrsr->DeleteMark();
        pCrsr->RestoreSavePos();
        bRet = FALSE;
    }
    else
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return bRet;
}


BOOL SwCrsrShell::GotoBookmark(USHORT nPos)
{
    // Crsr-Moves ueberwachen, evt. Link callen
    BOOL bRet = TRUE;
    SwCallLink aLk( *this );
    SwBookmark* pBkmk = getIDocumentBookmarkAccess()->getBookmarks()[ nPos ];
    SwCursor* pCrsr = GetSwCrsr();
    SwCrsrSaveState aSaveState( *pCrsr );

    // --> OD 2007-09-27 #i81002# - refactoring
    // simplify by using <SwBookmark::GetBookmarkStart()/GetBookmarkEnd()>
//    *pCrsr->GetPoint() = pBkmk->GetBookmarkPos();
    *pCrsr->GetPoint() = *pBkmk->BookmarkStart();
    if( pBkmk->GetOtherBookmarkPos() )
    {
        pCrsr->SetMark();
//        *pCrsr->GetMark() = *pBkmk->GetOtherBookmarkPos();
        *pCrsr->GetMark() = *pBkmk->BookmarkEnd();
//        if( *pCrsr->GetMark() > *pCrsr->GetPoint() )
//            pCrsr->Exchange();
    }
    // <--

    if( pCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                         nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
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


BOOL SwCrsrShell::GoNextBookmark()
{
    SwBookmark aBM(*GetCrsr()->GetPoint());
    USHORT nPos;
    const SwBookmarks& rBkmks = getIDocumentBookmarkAccess()->getBookmarks();
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


BOOL SwCrsrShell::GoPrevBookmark()
{
    const SwBookmarks& rBkmks = getIDocumentBookmarkAccess()->getBookmarks();
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

    BOOL bRet = FALSE;
    do {
        pBkmk = rBkmks[ nPos ];

        // --> OD 2007-09-27 #i81002# - refactoring
        // simplify by using <SwBookmark::BookmarkStart()/BookmarkEnd()>
        *pCrsr->GetPoint() = *pBkmk->BookmarkStart();
//        *pCrsr->GetPoint() = pBkmk->GetBookmarkPos();
        if( pBkmk->GetOtherBookmarkPos() )
        {
            pCrsr->SetMark();
//            *pCrsr->GetMark() = *pBkmk->GetOtherBookmarkPos();
            *pCrsr->GetMark() = *pBkmk->BookmarkEnd();
//            if( *pCrsr->GetMark() < *pCrsr->GetPoint() )
//                pCrsr->Exchange();
        }
        // <--
        if( !pCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                              nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
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
    return getIDocumentBookmarkAccess()->getBookmarkCount(bBkmrk);
}


SwBookmark& SwCrsrShell::GetBookmark(USHORT nPos, BOOL bBkmrk)
{
    return getIDocumentBookmarkAccess()->getBookmark(nPos, bBkmrk);
}


void SwCrsrShell::DelBookmark(USHORT nPos)
{
    StartAction();
    getIDocumentBookmarkAccess()->deleteBookmark(nPos);
    EndAction();
}


void SwCrsrShell::DelBookmark( const String& rName )
{
    StartAction();
    getIDocumentBookmarkAccess()->deleteBookmark( rName );
    EndAction();
}


USHORT SwCrsrShell::FindBookmark( const String& rName )
{
    return getIDocumentBookmarkAccess()->findBookmark( rName );
}


        // erzeugt einen eindeutigen Namen. Der Name selbst muss vorgegeben
        // werden, es wird dann bei gleichen Namen nur durchnumeriert.
void SwCrsrShell::MakeUniqueBookmarkName( String& rName )
{
    getIDocumentBookmarkAccess()->makeUniqueBookmarkName( rName );
}



