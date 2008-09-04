/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: crbm.cxx,v $
 * $Revision: 1.9 $
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


#include "crsrsh.hxx"
#include "ndtxt.hxx"
#include <docary.hxx>

#include "bookmrk.hxx"
#include "callnk.hxx"
#include "swcrsr.hxx"
#include <IDocumentBookmarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>

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


bool SwCrsrShell::IsFormProtected()
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::PROTECT_FORM);
}

SwBookmark* SwCrsrShell::IsInFieldBookmark()
{
    // TODO: Refactor
    SwPosition pos(*GetCrsr()->GetPoint());
    return getIDocumentBookmarkAccess()->getFieldBookmarkFor(pos);
}

SwFieldBookmark* SwCrsrShell::IsInFormFieldBookmark()
{
    // TODO: Refactor
    SwPosition pos(*GetCrsr()->GetPoint());
    return (SwFieldBookmark*)getIDocumentBookmarkAccess()->getFormFieldBookmarkFor(pos);
}

SwBookmark* SwCrsrShell::GetNextFieldBookmark()
{
    SwPosition pos(*GetCrsr()->GetPoint());
    return getIDocumentBookmarkAccess()->getNextFieldBookmarkFor(pos);
}

SwBookmark* SwCrsrShell::GetPrevFieldBookmark()
{
    SwPosition pos(*GetCrsr()->GetPoint());
    return getIDocumentBookmarkAccess()->getPrevFieldBookmarkFor(pos);
}

bool SwCrsrShell::GotoFieldBookmark(SwBookmark *pBkmk)
{
    if(pBkmk==NULL) return false;
    // Crsr-Moves ueberwachen, evt. Link callen
    bool bRet = true;
    SwCallLink aLk( *this );
    SwCursor* pCrsr = GetSwCrsr();
    SwCrsrSaveState aSaveState( *pCrsr );

    *pCrsr->GetPoint() = pBkmk->GetBookmarkPos();
    if( pBkmk->GetOtherBookmarkPos() )
    {
        pCrsr->SetMark();
        *pCrsr->GetMark() = *pBkmk->GetOtherBookmarkPos();
        if( *pCrsr->GetMark() > *pCrsr->GetPoint() )
            pCrsr->Exchange();
    }
    pCrsr->GetPoint()->nContent--;
    pCrsr->GetMark()->nContent++;


    if( pCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION | nsSwCursorSelOverFlags::SELOVER_TOGGLE ) )
    {
        pCrsr->DeleteMark();
        pCrsr->RestoreSavePos();
        bRet = false;
    }
    else
        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);

    return bRet;
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
    do
    {
        if ( nPos == 0 )
            return FALSE;
    } while( aBM < *(pBkmk = rBkmks[--nPos]) || aBM.IsEqualPos( *pBkmk ));

    SwCallLink aLk( *this );
    SwCrsrSaveState aSaveState( *pCrsr );

    BOOL bRet = FALSE;
    do
    {
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
