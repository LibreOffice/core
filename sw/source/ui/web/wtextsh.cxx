/*************************************************************************
 *
 *  $RCSfile: wtextsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:51 $
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

#include "hintids.hxx"

#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx> //***
#endif
#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif

#include "swtypes.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wtextsh.hxx"
#include "basesh.hxx"
#include "globals.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "web.hrc"

#define C2S(cChar) UniString::CreateFromAscii(cChar)

// STATIC DATA -----------------------------------------------------------
#define SwWebTextShell
#define Paragraph
#define HyphenZone
#define TextFont
#define DropCap
#include "itemdef.hxx"
#include "swslots.hxx"


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SFX_IMPL_INTERFACE(SwWebTextShell, SwBaseShell, SW_RES(STR_SHELLNAME_WEBTEXT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_WEB_TEXT_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_WEBTEXT_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_WEBTEXT));
    SFX_CHILDWINDOW_REGISTRATION(FN_EDIT_FORMULA);
    SFX_CHILDWINDOW_REGISTRATION(SID_INSERT_DIAGRAM);
    SFX_CHILDWINDOW_REGISTRATION(FN_INSERT_FIELD);
}

TYPEINIT1(SwWebTextShell, SwTextShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwWebTextShell::SwWebTextShell(SwView &rView) :
    SwTextShell(rView)
{
    SetHelpId(SW_WEBTEXTSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwWebTextShell::~SwWebTextShell()
{
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.17  2000/09/18 16:06:20  willem.vandorp
    OpenOffice header added.

    Revision 1.16  2000/05/10 11:54:44  os
    Basic API removed

    Revision 1.15  2000/05/09 14:43:32  os
    BASIC interface partially removed

    Revision 1.14  2000/04/19 11:21:44  os
    UNICODE

    Revision 1.13  2000/02/01 10:00:38  os
    #72282# SwWebBaseShell removed

    Revision 1.12  2000/02/01 09:50:54  os
    #72282# SwWebBaseShell removed

    Revision 1.11  2000/01/31 20:19:38  jp
    Bug #72455#,#72456# Text-PopupMenue moved into swmn.hrc, Web-Textshell with own PopupMenu

    Revision 1.10  2000/01/18 11:57:17  os
    #71740# interface inheritance corrected

    Revision 1.9  1997/11/21 10:33:20  MA
    includes


      Rev 1.8   21 Nov 1997 11:33:20   MA
   includes

      Rev 1.7   03 Nov 1997 14:00:14   MA
   precomp entfernt

      Rev 1.6   03 Sep 1997 10:50:40   MBA
   swslots.hxx statt wswslots.hxx

      Rev 1.5   29 Aug 1997 12:05:54   MH
   chg: header

      Rev 1.4   05 Aug 1997 16:41:48   TJ
   include svx/srchitem.hxx

      Rev 1.3   31 Jul 1997 14:19:20   MH
   chg: header

      Rev 1.2   28 Jan 1997 16:33:40   OS
   Name der Shell geaendert

      Rev 1.1   24 Jan 1997 14:55:54   OS
   eigene TextToolBox

      Rev 1.0   23 Jan 1997 16:18:08   OS
   Initial revision.


------------------------------------------------------------------------*/

