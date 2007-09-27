/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unfmco.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:31:30 $
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
#include "swundo.hxx"           // fuer die UndoIds
#include "pam.hxx"
#include "ndtxt.hxx"

#include "undobj.hxx"
#include "rolbck.hxx"


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

//--------------------------------------------------


SwUndoFmtColl::SwUndoFmtColl( const SwPaM& rRange, SwFmtColl* pColl )
    : SwUndo( UNDO_SETFMTCOLL ), SwUndRng( rRange ),
    pHistory( new SwHistory ), pFmtColl( pColl )
{
    // --> FME 2004-08-06 #i31191#
    if ( pColl )
        aFmtName = pColl->GetName();
    // <--
}


SwUndoFmtColl::~SwUndoFmtColl()
{
    delete pHistory;
}


void SwUndoFmtColl::Undo( SwUndoIter& rUndoIter )
{
    // die alten Werte wieder zurueck
    pHistory->TmpRollback( &rUndoIter.GetDoc(), 0 );
    pHistory->SetTmpEnd( pHistory->Count() );

    // setze noch den Cursor auf den Undo-Bereich
    SetPaM( rUndoIter );
}


void SwUndoFmtColl::Redo( SwUndoIter& rUndoIter )
{
    // setze Attribut in dem Bereich:
    SetPaM( rUndoIter );
    rUndoIter.pLastUndoObj = 0;

    Repeat( rUndoIter );    // Collection setzen

    rUndoIter.pLastUndoObj = 0;
}


void SwUndoFmtColl::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_SETFMTCOLL == rUndoIter.GetLastUndoId() &&
        pFmtColl == ((SwUndoFmtColl*)rUndoIter.pLastUndoObj)->pFmtColl )
        return;

    // es kann nur eine TextFmtColl auf einen Bereich angewendet werden,
    // also erfrage auch nur in dem Array
    USHORT nPos = rUndoIter.GetDoc().GetTxtFmtColls()->GetPos(
                                                     (SwTxtFmtColl*)pFmtColl );
    // ist das Format ueberhaupt noch vorhanden?
    if( USHRT_MAX != nPos )
        rUndoIter.GetDoc().SetTxtFmtColl( *rUndoIter.pAktPam,
                                        (SwTxtFmtColl*)pFmtColl );

    rUndoIter.pLastUndoObj = this;
}

SwRewriter SwUndoFmtColl::GetRewriter() const
{
    SwRewriter aResult;

    // --> FME 2004-08-06 #i31191# Use stored format name instead of
    // pFmtColl->GetName(), because pFmtColl does not have to be available
    // anymore.
    aResult.AddRule(UNDO_ARG1, aFmtName );
    // <--

    return aResult;
}
