/*************************************************************************
 *
 *  $RCSfile: unfmco.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:38:50 $
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


#pragma hdrstop

#include "doc.hxx"
#include "swundo.hxx"           // fuer die UndoIds
#include "pam.hxx"
#include "ndtxt.hxx"

#include "undobj.hxx"
#include "rolbck.hxx"


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

//--------------------------------------------------


SwUndoFmtColl::SwUndoFmtColl( const SwPaM& rRange, SwFmtColl* pColl )
    : SwUndo( UNDO_SETFMTCOLL ), SwUndRng( rRange ), pFmtColl( pColl ),
    pHistory( new SwHistory )
{
#ifdef COMPACT
    ((SwDoc*)rRange.GetDoc())->DelUndoGroups();
#endif
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

