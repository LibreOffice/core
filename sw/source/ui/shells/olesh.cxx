/*************************************************************************
 *
 *  $RCSfile: olesh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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

#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif
#ifndef _SFX_OBJITEM_HXX //autogen
#include <sfx2/objitem.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _OLESH_HXX
#include <olesh.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif


#define SwOleShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif


SFX_IMPL_INTERFACE(SwOleShell, SwFrameShell, SW_RES(STR_SHELLNAME_OBJECT))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_OLE_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_OLE_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_OLE));
}


SwOleShell::SwOleShell(SwView &rView) :
    SwFrameShell(rView)

{
    SetName(String::CreateFromAscii("Object"));
    SetHelpId(SW_OLESHELL);
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.35  2000/09/18 16:06:04  willem.vandorp
    OpenOffice header added.

    Revision 1.34  2000/05/26 07:21:32  os
    old SW Basic API Slots removed

    Revision 1.33  2000/05/16 09:15:14  os
    project usr removed

    Revision 1.32  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.31  2000/05/09 14:41:35  os
    BASIC interface partially removed

    Revision 1.30  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.29  2000/02/11 14:57:36  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.28  1998/11/03 10:53:02  JP
    Task #57916#: Vorbereitungen fuer den Selektionshandler vom Chart


      Rev 1.27   03 Nov 1998 11:53:02   JP
   Task #57916#: Vorbereitungen fuer den Selektionshandler vom Chart

      Rev 1.26   24 Nov 1997 09:46:52   MA
   includes

      Rev 1.25   03 Nov 1997 13:55:48   MA
   precomp entfernt

      Rev 1.24   26 Aug 1997 16:31:10   TRI
   VCL Anpassungen

      Rev 1.23   08 Aug 1997 17:29:08   OM
   Headerfile-Umstellung

      Rev 1.22   05 Aug 1997 16:29:12   TJ
   include svx/srchitem.hxx

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

