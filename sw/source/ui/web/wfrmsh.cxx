/*************************************************************************
 *
 *  $RCSfile: wfrmsh.cxx,v $
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

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx> //***
#endif

#include "swtypes.hxx"
#include "cmdid.h"
#include "view.hxx"
#include "wfrmsh.hxx"
#include "globals.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "web.hrc"


// STATIC DATA -----------------------------------------------------------
#define SwWebFrameShell
#include "itemdef.hxx"
#include "swslots.hxx"


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SFX_IMPL_INTERFACE(SwWebFrameShell, SwFrameShell, SW_RES(STR_SHELLNAME_WEBFRAME))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_FRM_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_WEBFRAME_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_FRAME));
}



TYPEINIT1(SwWebFrameShell, SwFrameShell)

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwWebFrameShell::SwWebFrameShell(SwView &rView) :
    SwFrameShell(rView)
{
    SetHelpId(SW_WEBFRAMESHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/



SwWebFrameShell::~SwWebFrameShell()
{
}



/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.8  2000/09/18 16:06:20  willem.vandorp
    OpenOffice header added.

    Revision 1.7  2000/05/10 11:54:44  os
    Basic API removed

    Revision 1.6  1997/11/21 10:00:48  MA
    includes


      Rev 1.5   21 Nov 1997 11:00:48   MA
   includes

      Rev 1.4   03 Nov 1997 14:00:14   MA
   precomp entfernt

      Rev 1.3   03 Sep 1997 10:51:06   MBA
   swslots.hxx statt wswslots.hxx

      Rev 1.2   05 Aug 1997 16:41:48   TJ
   include svx/srchitem.hxx

      Rev 1.1   31 Jul 1997 14:19:22   MH
   chg: header

      Rev 1.0   31 Jan 1997 13:31:56   OS
   Initial revision.

------------------------------------------------------------------------*/




