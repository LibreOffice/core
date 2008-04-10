/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unbkmk.cxx,v $
 * $Revision: 1.10 $
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


#include "doc.hxx"
#include "docary.hxx"
#include "swundo.hxx"           // fuer die UndoIds
#include "pam.hxx"

#include "undobj.hxx"
#include "bookmrk.hxx"
#include "rolbck.hxx"

#include "SwRewriter.hxx"

inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }


SwUndoBookmark::SwUndoBookmark( SwUndoId nUndoId, const SwBookmark& rBkmk )
    : SwUndo( nUndoId )
{
    BYTE nType = SwHstryBookmark::BKMK_POS;
    if( rBkmk.GetOtherBookmarkPos() )
        nType |= SwHstryBookmark::BKMK_OTHERPOS;
    pHBookmark = new SwHstryBookmark( rBkmk, nType );
}



SwUndoBookmark::~SwUndoBookmark()
{
    delete pHBookmark;
}


void SwUndoBookmark::SetInDoc( SwDoc* pDoc )
{
    pHBookmark->SetInDoc( pDoc, FALSE );
}


void SwUndoBookmark::ResetInDoc( SwDoc* pDoc )
{
    const SwBookmarks& rBkmkTbl = pDoc->getBookmarks();
    for( USHORT n = 0; n < rBkmkTbl.Count(); ++n )
        if( pHBookmark->IsEqualBookmark( *rBkmkTbl[ n ] ) )
        {
                pDoc->deleteBookmark( n );
                break;
        }
}

SwRewriter SwUndoBookmark::GetRewriter() const
{
    SwRewriter aResult;

    aResult.AddRule(UNDO_ARG1, pHBookmark->GetName());

    return aResult;
}

SwUndoDelBookmark::SwUndoDelBookmark( const SwBookmark& rBkmk )
    : SwUndoBookmark( UNDO_DELBOOKMARK, rBkmk )
{
}


void SwUndoDelBookmark::Undo( SwUndoIter& rUndoIter )
{
    SetInDoc( &rUndoIter.GetDoc() );
}


void SwUndoDelBookmark::Redo( SwUndoIter& rUndoIter )
{
    ResetInDoc( &rUndoIter.GetDoc() );
}


SwUndoInsBookmark::SwUndoInsBookmark( const SwBookmark& rBkmk )
    : SwUndoBookmark( UNDO_INSBOOKMARK, rBkmk )
{
}


void SwUndoInsBookmark::Undo( SwUndoIter& rUndoIter )
{
    ResetInDoc( &rUndoIter.GetDoc() );
}


void SwUndoInsBookmark::Redo( SwUndoIter& rUndoIter )
{
    SetInDoc( &rUndoIter.GetDoc() );
}


