/*************************************************************************
 *
 *  $RCSfile: popbox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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

#include "cmdid.h"
#include "swtypes.hxx"
#include "segmentc.hxx"

#include "popbox.hxx"


// --- class SwHelpToolBox ---------------------------------------------


SwHelpToolBox::SwHelpToolBox( Window* pParent, const ResId& rResId ) :
                ToolBox( pParent, rResId )
{
}


void SwHelpToolBox::MouseButtonDown(const MouseEvent &rEvt)
{
        // Zuerst DoubleClick-Link auswerten
        // Dessen Returnwert entscheidet ueber andere Verarbeitung
        // Doppelclickhandler nur, wenn nicht auf einen Button geclickt wurde
    if(rEvt.GetButtons() == MOUSE_RIGHT &&
        0 == GetItemId(rEvt.GetPosPixel())) {
        aRightClickLink.Call((MouseEvent *)&rEvt);
        return;
    }
    else
        ToolBox::MouseButtonDown(rEvt);
}


long SwHelpToolBox::DoubleClick( ToolBox* pCaller )
{
        // kein Doppelklick auf einen Button
    if(0 == pCaller->GetCurItemId() && aDoubleClickLink.Call(0)) return TRUE;
    return FALSE;
}

/*-----------------26.02.94 00:36-------------------
 dtor ueberladen
--------------------------------------------------*/


SwHelpToolBox::~SwHelpToolBox() {}


BOOL    SwHelpToolBox::Drop( const DropEvent& rEvt)
{
    return GetParent()->Drop(rEvt);
}


BOOL    SwHelpToolBox::QueryDrop( DropEvent& rEvt)
{
    return GetParent()->QueryDrop(rEvt);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.17  2000/09/18 16:05:12  willem.vandorp
    OpenOffice header added.

    Revision 1.16  2000/04/11 08:03:01  os
    UNICODE

    Revision 1.15  1997/11/03 12:09:46  MA
    precomp entfernt


      Rev 1.14   03 Nov 1997 13:09:46   MA
   precomp entfernt

      Rev 1.13   28 Aug 1996 08:23:42   OS
   includes

      Rev 1.12   19 Jun 1996 14:57:58   OS
   +Drop/QueryDrop

      Rev 1.11   16 Apr 1996 17:03:48   OM
   Unbenutzten Ctor entfernt

      Rev 1.10   24 Nov 1995 16:57:38   OM
   PCH->PRECOMPILED

      Rev 1.9   07 Jul 1995 09:27:26   OM
   Mouse_Contextmenu entfernt

      Rev 1.8   06 Jul 1995 22:52:36   ER
   Hack wg. MOUSE_CONTEXTMENU

      Rev 1.7   09 Mar 1995 14:46:30   MA
   Aufgeraeumt

      Rev 1.6   04 Mar 1995 23:14:18   ER
   Help::EnableBalloonMode() hammernichmehr

      Rev 1.5   25 Jan 1995 17:13:58   OS
   Timerinstrumentierung

      Rev 1.4   09 Jan 1995 16:24:12   ER
    del: initui hxx

      Rev 1.3   25 Oct 1994 14:55:16   ER
   add: PCH

      Rev 1.2   19 Oct 1994 13:14:08   MA
   Syntax

      Rev 1.1   19 Oct 1994 12:21:12   MA
   SV-ChangeXXX -> SetXXX.

------------------------------------------------------------------------*/


