/*************************************************************************
 *
 *  $RCSfile: wolesh.cxx,v $
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

#include "itemdef.hxx"

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX //autogen
#include <svx/srchitem.hxx>
#endif

#include "wrtsh.hxx"
#include "view.hxx"
#include "helpid.h"
#include "globals.hrc"
#include "web.hrc"
#include "popup.hrc"
#include "shells.hrc"
#include "wolesh.hxx"
#include "cmdid.h"

#define SwWebOleShell
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebOleShell, SwOleShell, SW_RES(STR_SHELLNAME_OBJECT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_OLE_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_WEBOLE_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_OLE));
}

SwWebOleShell::SwWebOleShell(SwView &rView) :
    SwOleShell(rView)

{
    SetName(String::CreateFromAscii("Object"));
    SetHelpId(SW_OLESHELL);
}

__EXPORT SwWebOleShell::~SwWebOleShell()
{
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.8  2000/09/18 16:06:20  willem.vandorp
    OpenOffice header added.

    Revision 1.7  2000/05/10 11:54:44  os
    Basic API removed

    Revision 1.6  2000/04/19 11:21:44  os
    UNICODE

    Revision 1.5  1999/01/04 14:02:18  OS
    #59918# Kein Vorlagencontroller fuer Rahmen+Grafiken im HTML


      Rev 1.4   04 Jan 1999 15:02:18   OS
   #59918# Kein Vorlagencontroller fuer Rahmen+Grafiken im HTML

      Rev 1.3   21 Nov 1997 11:33:20   MA
   includes

      Rev 1.2   03 Nov 1997 14:00:16   MA
   precomp entfernt

      Rev 1.1   05 Sep 1997 12:36:00   MH
   chg: header

      Rev 1.0   03 Sep 1997 14:51:06   MBA
   Initial revision.

      Rev 1.21   01 Aug 1997 11:45:44   MH
   chg: header

      Rev 1.20   11 Nov 1996 13:37:34   MA
   ResMgr

      Rev 1.19   28 Aug 1996 15:55:14   OS
   includes

      Rev 1.18   13 Aug 1996 12:38:40   OS
   neue Shellnamen im IDL-Interface

      Rev 1.17   25 Jun 1996 19:51:50   HJS
   includes

      Rev 1.16   20 Mar 1996 12:41:14   HJS
   funcdefs muessen unique sein!

      Rev 1.15   06 Feb 1996 15:16:22   OS
   neu: Activate/Deactivate fuer Objekte

      Rev 1.14   26 Jan 1996 10:54:12   OS
   neu: GetState - Ole-Objekt dem Basic herausreichen

      Rev 1.13   03 Dec 1995 11:22:08   OS
   +include itemdef.hxx

      Rev 1.12   29 Nov 1995 13:47:54   OS
   -slotadd.hxx

      Rev 1.11   27 Nov 1995 19:40:06   OM
   swslots->303a

      Rev 1.10   24 Nov 1995 16:59:36   OM
   PCH->PRECOMPILED

      Rev 1.9   17 Nov 1995 13:03:40   MA
   Segmentierung

------------------------------------------------------------------------*/

