/*************************************************************************
 *
 *  $RCSfile: wlistsh.cxx,v $
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
#ifndef _IMAPDLG_HXX_ //autogen
#include <svx/imapdlg.hxx>
#endif
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif

#include "cmdid.h"
#include "uiparam.hxx"
#include "wrtsh.hxx"
#include "swmodule.hxx"
#include "globals.hrc"
#include "shells.hrc"
#include "uinums.hxx"
#include "wlistsh.hxx"

#define SwWebListShell
#include "swslots.hxx"

SFX_IMPL_INTERFACE(SwWebListShell, SwListShell, SW_RES(STR_SHELLNAME_LIST))
{
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_NUM_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_LIST));
}

TYPEINIT1(SwWebListShell,SwListShell)

SwWebListShell::SwWebListShell(SwView &rView) :
    SwListShell(rView)
{
    SetName(String::CreateFromAscii("List"));
    SetHelpId(SW_LISTSHELL);
}

__EXPORT SwWebListShell::~SwWebListShell()
{
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.7  2000/09/18 16:06:20  willem.vandorp
    OpenOffice header added.

    Revision 1.6  2000/05/26 07:21:35  os
    old SW Basic API Slots removed

    Revision 1.5  2000/05/10 11:54:44  os
    Basic API removed

    Revision 1.4  2000/04/19 11:21:44  os
    UNICODE

    Revision 1.3  1997/11/21 10:33:20  MA
    includes


      Rev 1.2   21 Nov 1997 11:33:20   MA
   includes

      Rev 1.1   03 Nov 1997 14:00:16   MA
   precomp entfernt

      Rev 1.0   03 Sep 1997 14:50:56   MBA
   Initial revision.

------------------------------------------------------------------------*/


