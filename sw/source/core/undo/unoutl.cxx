/*************************************************************************
 *
 *  $RCSfile: unoutl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:28 $
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


inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

SwUndoOutlineLeftRight::SwUndoOutlineLeftRight( const SwPaM& rPam,
                                                short nOff )
    : SwUndo( UNDO_OUTLINE_LR ), SwUndRng( rPam ), nOffset( nOff )
{
}


void SwUndoOutlineLeftRight::Undo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().OutlineUpDown( *rUndoIter.pAktPam, -nOffset );
}


void SwUndoOutlineLeftRight::Redo( SwUndoIter& rUndoIter )
{
    SetPaM( rUndoIter );
    rUndoIter.GetDoc().OutlineUpDown( *rUndoIter.pAktPam, nOffset );
}


void SwUndoOutlineLeftRight::Repeat( SwUndoIter& rUndoIter )
{
    rUndoIter.GetDoc().OutlineUpDown( *rUndoIter.pAktPam, nOffset );
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/core/undo/unoutl.cxx,v 1.1.1.1 2000-09-19 00:08:28 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.11  2000/09/18 16:04:29  willem.vandorp
      OpenOffice header added.

      Revision 1.10  1998/04/02 13:13:32  JP
      Redo: Undo-Flag wird schon von der EditShell abgeschaltet


      Rev 1.9   02 Apr 1998 15:13:32   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.8   23 Jan 1998 17:01:58   JP
   SwUndoOutlineUpDown entfernt, wird durch MoveParagraph ersetzt

      Rev 1.7   03 Nov 1997 13:06:14   MA
   precomp entfernt

      Rev 1.6   09 Oct 1997 15:45:28   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.5   11 Jun 1997 10:44:10   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.4   13 May 1997 15:03:26   JP
   UndoMoveOutl: keine Selektion beachten

      Rev 1.3   24 Nov 1995 17:14:04   OM
   PCH->PRECOMPILED

      Rev 1.2   22 Jun 1995 19:33:22   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.1   08 Feb 1995 23:52:54   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.0   07 Feb 1995 19:34:34   JP
   Undo fuers hoch/runter stufen/verschieben von Gliederung

*************************************************************************/


