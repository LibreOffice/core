/*************************************************************************
 *
 *  $RCSfile: unbkmk.cxx,v $
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
#include "docary.hxx"
#include "swundo.hxx"           // fuer die UndoIds
#include "pam.hxx"

#include "undobj.hxx"
#include "bookmrk.hxx"
#include "rolbck.hxx"



inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }


SwUndoBookmark::SwUndoBookmark( USHORT nUndoId, const SwBookmark& rBkmk )
    : SwUndo( nUndoId )
{
    int nType = SwHstryBookmark::BKMK_POS;
    if( rBkmk.GetOtherPos() )
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
    const SwBookmarks& rBkmkTbl = pDoc->GetBookmarks();
    for( USHORT n = 0; n < rBkmkTbl.Count(); ++n )
        if( pHBookmark->IsEqualBookmark( *rBkmkTbl[ n ] ) )
        {
                pDoc->DelBookmark( n );
                break;
        }
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



/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unbkmk.cxx,v 1.1.1.1 2000-09-19 00:08:27 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.24  2000/09/18 16:04:28  willem.vandorp
      OpenOffice header added.

      Revision 1.23  1998/04/02 13:12:48  JP
      Redo: Undo-Flag wird schon von der EditShell abgeschaltet


      Rev 1.22   02 Apr 1998 15:12:48   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.21   03 Nov 1997 13:06:16   MA
   precomp entfernt

      Rev 1.20   03 Sep 1997 10:29:10   JP
   zusaetzliches include von docary

      Rev 1.19   11 Jun 1997 10:42:46   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.18   24 Nov 1995 17:13:56   OM
   PCH->PRECOMPILED

      Rev 1.17   23 Nov 1995 12:05:42   AMA
   Fix/Opt: BLC-Warnings.

      Rev 1.16   26 Jun 1995 19:50:06   JP
   UndoBookmark: nur wenn die 2. Position vorhanden ist, diese sichern

      Rev 1.15   22 Jun 1995 19:33:34   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.14   04 Mar 1995 13:29:22   MA
   unnoetiges SEXPORT entfernt.

      Rev 1.13   23 Feb 1995 23:00:58   ER
   sexport

      Rev 1.12   08 Feb 1995 23:52:34   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.11   15 Dec 1994 20:46:58   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.10   25 Oct 1994 14:50:38   MA
   PreHdr.

      Rev 1.9   21 Mar 1994 18:20:32   JP
   umgestellt und jetzt auch mit Undo vom InsertBookmark.

      Rev 1.8   02 Mar 1994 19:50:20   MI
   Underscore im Namen der #pragmas

      Rev 1.7   17 Feb 1994 08:30:56   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.6   16 Feb 1994 13:17:02   MI
   Pragmas zurechtgerueckt

*************************************************************************/



