/*************************************************************************
 *
 *  $RCSfile: unfmco.cxx,v $
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

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unfmco.cxx,v 1.1.1.1 2000-09-19 00:08:27 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.22  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.21  1998/04/02 13:13:30  JP
      Redo: Undo-Flag wird schon von der EditShell abgeschaltet


      Rev 1.20   02 Apr 1998 15:13:30   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.19   16 Jan 1998 11:07:54   JP
   pAttrHistory am Doc entfernt

      Rev 1.18   03 Nov 1997 13:06:28   MA
   precomp entfernt

      Rev 1.17   11 Jun 1997 10:44:48   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.16   23 Sep 1996 20:06:28   JP
   SetTmpEnd: DocPtr entfernt

      Rev 1.15   24 Nov 1995 17:14:04   OM
   PCH->PRECOMPILED

      Rev 1.14   08 Feb 1995 23:52:36   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.13   28 Jan 1995 21:21:12   JP
   UndoInsFmt enternt

      Rev 1.12   20 Jan 1995 09:57:02   JP
   Bug10011: erfrage am Dok. ob der Pointer auf Fmt/Coll gueltig ist

      Rev 1.11   15 Dec 1994 20:47:52   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.10   25 Oct 1994 14:50:34   MA
   PreHdr.

      Rev 1.9   25 Aug 1994 18:06:08   JP
   Umstellung Attribute (von SwHint -> SfxPoolItem)

      Rev 1.8   02 Mar 1994 19:48:54   MI
   Underscore im Namen der #pragmas

      Rev 1.7   17 Feb 1994 08:30:04   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.6   16 Feb 1994 13:11:30   MI
   Pragmas zurechtgerueckt

*************************************************************************/


