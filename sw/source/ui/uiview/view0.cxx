/*************************************************************************
 *
 *  $RCSfile: view0.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
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

#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _IDETEMP_HXX
#include <basctl/idetemp.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif

#ifndef _NAVIPI_HXX //autogen
#include <navipi.hxx>
#endif
#include "view.hxx"
#include "basesh.hxx"
#include "docsh.hxx"
#include "globals.hrc"
#include "cmdid.h"          // FN_       ...
#include "globdoc.hxx"
#include "wview.hxx"
#include "shells.hrc"

#define OLEObjects
#define SwView
#define SearchAttributes
#define ReplaceAttributes
#define SearchSettings
#define _ExecSearch ExecSearch
#define _StateSearch StateSearch
#define Frames
#define Graphics
#define Tables
#define Controls
#define GlobalContents
#define Text
#define Frame
#define Graphic
#define Object
#define Draw
#define TextDrawText
#define TextInTable
#define ListInText
#define ListInTable
#define WebTextInTable
#define WebListInText
#define WebListInTable
#define TextPage
#include "itemdef.hxx"
#include <svx/svxslots.hxx>
#include "swslots.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;

#define C2S(cChar) UniString::CreateFromAscii(cChar)

SFX_IMPL_VIEWFACTORY(SwView, SW_RES(STR_NONAME))
{
    SFX_VIEW_REGISTRATION(SwDocShell);
    SFX_VIEW_REGISTRATION(SwGlobalDocShell);
}

SFX_IMPL_INTERFACE( SwView, SfxViewShell, SW_RES(RID_TOOLS_TOOLBOX) )
{
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION(SID_NAVIGATOR);
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(FN_REDLINE_ACCEPT);
    SFX_CHILDWINDOW_REGISTRATION(SID_HYPERLINK_DIALOG);
    SFX_FEATURED_CHILDWINDOW_REGISTRATION(FN_SYNC_LABELS, 1);
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_TOOLS_TOOLBOX) );
}

TYPEINIT1(SwView,SfxViewShell)

/*-----------------13.12.97 11:06-------------------

--------------------------------------------------*/
ShellModes  SwView::GetShellMode()
{
    return pViewImpl->GetShellMode();
}

/*-----------------13.12.97 11:28-------------------

--------------------------------------------------*/
view::XSelectionSupplier* SwView::GetUNOObject()
{
    return pViewImpl->GetUNOObject();
}


/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.72  2000/09/18 16:06:12  willem.vandorp
    OpenOffice header added.

    Revision 1.71  2000/09/04 11:44:56  tbe
    basicide, isetbrw, si, vcdlged moved from svx to basctl

    Revision 1.70  2000/05/10 11:53:20  os
    Basic API removed

    Revision 1.69  2000/05/09 14:43:13  os
    BASIC interface partially removed

    Revision 1.68  2000/04/18 15:02:50  os
    UNICODE

    Revision 1.67  2000/03/23 07:50:25  os
    UNO III

    Revision 1.66  2000/02/09 08:07:01  os
    #72165# hyperlink dialog moved again

    Revision 1.65  1999/09/07 13:56:53  os
    Insert/EditIndexEntry as FloatingWindow

    Revision 1.64  1999/01/27 08:58:32  OS
    #56371# TF_ONE51


      Rev 1.63   27 Jan 1999 09:58:32   OS
   #56371# TF_ONE51

      Rev 1.62   15 Jul 1998 12:52:42   OS
   Navigator an der SwView registrieren #34794#

      Rev 1.61   09 Jun 1998 15:32:20   OM
   VC-Controls entfernt

      Rev 1.60   02 Jun 1998 15:49:54   OS
   TF_STARONE raus; GetUNOSelectionObject gestrichen

      Rev 1.59   03 Apr 1998 14:38:18   OS
   UnoObject fuer die View reaktiviert

      Rev 1.58   16 Mar 1998 16:18:04   OM
   Aktualisieren-Button kontextsensitiv

      Rev 1.57   15 Mar 1998 15:14:08   OM
   Synchron-Button

      Rev 1.56   27 Feb 1998 18:25:14   OM
   Redline-Browser

      Rev 1.55   29 Jan 1998 09:21:06   OS
   TF_STARONE

      Rev 1.54   16 Dec 1997 12:00:24   OS
   Impl-Pointer fuer UNO

      Rev 1.53   29 Nov 1997 16:49:14   MA
   includes

      Rev 1.52   21 Nov 1997 15:00:12   MA
   includes

      Rev 1.51   03 Nov 1997 13:58:28   MA
   precomp entfernt

      Rev 1.50   09 Sep 1997 11:33:08   OS
   TextPage heisst nur Page #43650#

      Rev 1.49   08 Sep 1997 10:52:36   OS
   DBG_ERROR -> DBG_ASSERT

      Rev 1.48   08 Sep 1997 07:43:38   OS
   TextTables nur ClassName

      Rev 1.47   04 Sep 1997 18:10:56   MBA
   GetSelectionObject erzeugt richtiges Objekt auch bei WebView

      Rev 1.46   04 Sep 1997 08:25:24   OS
   Tables heisst jetzt TextTables, kein GPF wg. fehlender TLB-Angaben

      Rev 1.45   03 Sep 1997 10:52:26   MBA
   OLEObjects in SVX

      Rev 1.44   29 Aug 1997 12:21:34   MH
   chg: SfxTypeLib_Impl

      Rev 1.43   05 Aug 1997 16:36:38   TJ
   include svx/srchitem.hxx

      Rev 1.42   07 Jul 1997 09:35:10   OS
   Collection fuer GlobalDoc

------------------------------------------------------------------------*/

