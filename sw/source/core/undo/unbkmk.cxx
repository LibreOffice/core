/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unbkmk.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:41:49 $
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


