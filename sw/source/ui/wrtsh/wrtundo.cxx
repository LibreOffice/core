/*************************************************************************
 *
 *  $RCSfile: wrtundo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>                   // fuer Undo-Ids
#endif

#ifndef _WRTSH_HRC
#include <wrtsh.hrc>
#endif
#include <sfx2/sfx.hrc>


// Undo beendet alle Modi. Falls eine Selektion durch das Undo entstanden
// ist, muss die fuer die weiteren Aktionen beruecksichtigt werden.


void SwWrtShell::Do( DoType eDoType )
{
    StartAllAction();
    switch( eDoType )
    {
        case UNDO:
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Undo(0);
            break;
        case REDO:
            // Modi zuruecksetzen
            EnterStdMode();
            SwEditShell::Redo();
            break;
        case REPEAT:
            SwEditShell::Repeat( 1 /*Anzahl!*/ );
            break;
    }
    EndAllAction();

    const FASTBOOL bFrmSelected = IsFrmSelected() || IsObjSelected();
    if ( IsSelection() )
    {
        if ( bFrmSelected )
            UnSelectFrm();

        // Funktionspointer fuer das Aufheben der Selektion setzen
        // bei Cursor setzen
        fnKillSel = &SwWrtShell::ResetSelect;
        fnSetCrsr = &SwWrtShell::SetCrsrKillSel;
    }
    else if ( bFrmSelected )
        EnterSelFrmMode();
    else if( (CNT_GRF | CNT_OLE ) & GetCntType() )
    {
        SelectObj( GetCharRect().Pos() );
        EnterSelFrmMode();
    }

    // Bug 32918: nach loeschen der Numerierung bleibt die Obj. Leiste stehen
    //          Warum wird hier nicht immer ein CallChgLink gerufen?
    CallChgLnk();
}


String SwWrtShell::GetDoString( DoType eDoType ) const
{
    String aStr;
    USHORT nId = 0, nResStr;
    switch( eDoType )
    {
    case UNDO:
        nResStr = STR_UNDO;
        nId = GetUndoIds( &aStr );
        break;
    case REDO:
        nResStr = STR_REDO;
        nId = GetRedoIds( &aStr );
        break;
    }

    if( UNDO_END < nId )
    {
        aStr.Insert( String(ResId( nResStr, SFX_APP()->GetSfxResManager() )), 0 );
        if( UNDO_DRAWUNDO != nId )
            aStr += SW_RESSTR( UNDO_BASE + nId );
    }
    return aStr;
}


String SwWrtShell::GetRepeatString() const
{
    String aStr;
    USHORT nId = GetRepeatIds( &aStr );
    if( UNDO_END < nId )
    {
        aStr.Insert( ResId( STR_REPEAT, SFX_APP()->GetSfxResManager()), 0 );
        if( UNDO_DRAWUNDO != nId )
            aStr += SW_RESSTR( UNDO_BASE + nId );
    }
    return aStr;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.53  2000/09/18 16:06:27  willem.vandorp
      OpenOffice header added.

      Revision 1.52  2000/07/27 21:01:41  jp
      Bug #76923#: Do - clamp the enterstdmode and Undo/Redo/Repeat call

      Revision 1.51  2000/03/03 15:17:06  os
      StarView remainders removed

      Revision 1.50  1998/04/15 14:35:34  OS
      STR_UNDO/REDO/REPEAT aus dem Sfx


      Rev 1.49   15 Apr 1998 16:35:34   OS
   STR_UNDO/REDO/REPEAT aus dem Sfx

      Rev 1.48   24 Nov 1997 14:35:02   MA
   includes

      Rev 1.47   03 Nov 1997 14:02:56   MA
   precomp entfernt

      Rev 1.46   22 Jan 1997 11:55:56   MA
   opt: bSelection entfernt

      Rev 1.45   11 Nov 1996 10:18:48   MA
   ResMgr

      Rev 1.44   31 Oct 1996 18:32:30   JP
   Bug #32918#: nach Undo der View sagen, das sich was getan hat

      Rev 1.43   29 Aug 1996 09:25:56   OS
   includes

      Rev 1.42   24 Nov 1995 16:59:06   OM
   PCH->PRECOMPILED

      Rev 1.41   19 Sep 1995 19:11:52   JP
   Bug 19431: Repeat funkt. wieder

      Rev 1.40   12 Sep 1995 17:59:32   JP
   Bug19137: vor Undo den Cursor in den StandardMode setzen

      Rev 1.39   28 Aug 1995 15:59:40   MA
   Renovierung: IDL, Shells, Textshell-Doktrin aufgegeben

      Rev 1.38   22 Aug 1995 17:30:04   JP
   GetUndo-/-Redo-/-RepeatIds: optional mit String-Ptr - DrawUndo-Objecte erzeuge die Strings selbst

      Rev 1.37   15 Aug 1995 19:52:20   JP
   Nach Undo/Redo kann der Cursor in OLE oder GRF stehen, selektieren dann den Frame

      Rev 1.36   27 Apr 1995 13:14:16   AMA
   Fix (JP): ResId-Ueberpruef. bei Undo

      Rev 1.35   23 Feb 1995 17:51:58   MA
   Rudimentaer Undo/Redo fuer Zeichenobjekte.

      Rev 1.34   08 Feb 1995 23:36:12   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.33   08 Feb 1995 19:01:30   JP
   UI-UndoIds ins undo.hxx verschoben

*************************************************************************/


