/*************************************************************************
 *
 *  $RCSfile: tmplctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:50 $
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

// include ---------------------------------------------------------------
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif

#include "wrtsh.hxx"
#include "view.hxx"
#include "swmodule.hxx"
#include "cmdid.h"
#include "docsh.hxx"
#include "tmplctrl.hxx"


// STATIC DATA -----------------------------------------------------------


SFX_IMPL_STATUSBAR_CONTROL( SwTemplateControl, SfxStringItem );

// class TemplatePopup_Impl --------------------------------------------------

class TemplatePopup_Impl : public PopupMenu
{
public:
    TemplatePopup_Impl();

    USHORT          GetCurId() const { return nCurId; }

private:
    USHORT          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

TemplatePopup_Impl::TemplatePopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

// -----------------------------------------------------------------------

void TemplatePopup_Impl::Select()
{
    nCurId = GetCurItemId();
}

// class SvxZoomStatusBarControl ------------------------------------------

SwTemplateControl::SwTemplateControl( USHORT nId,
                                                  StatusBar& rStb,
                                                  SfxBindings& rBind ) :
    SfxStatusBarControl( nId, rStb, rBind )
{
}

// -----------------------------------------------------------------------

SwTemplateControl::~SwTemplateControl()
{
}

// -----------------------------------------------------------------------

void SwTemplateControl::StateChanged( const SfxPoolItem* pState )
{
    SfxItemState eState = GetItemState(pState);

    if( eState != SFX_ITEM_AVAILABLE || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else if ( pState->ISA( SfxStringItem ) )
    {
        sTemplate = ((SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sTemplate );
    }
}

// -----------------------------------------------------------------------

void SwTemplateControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), sTemplate );
}

// -----------------------------------------------------------------------

void SwTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
            GetStatusBar().GetItemText( GetId() ).Len() )
    {
        CaptureMouse();
        TemplatePopup_Impl aPop;
        {
            SwView* pView = ::GetActiveView();
            SwWrtShell* pWrtShell;
            if( pView && 0 != (pWrtShell = pView->GetWrtShellPtr()) &&
                !pWrtShell->SwCrsrShell::HasSelection()&&
                !pWrtShell->IsSelFrmMode() &&
                !pWrtShell->IsObjSelected())
            {
                SfxStyleSheetBasePool* pPool = pView->GetDocShell()->
                                                            GetStyleSheetPool();
                pPool->SetSearchMask(SFX_STYLE_FAMILY_PAGE, SFXSTYLEBIT_ALL);
                if( pPool->Count() > 1 )
                {
                    USHORT nCount = 0;
                    SfxStyleSheetBase* pStyle = pPool->First();
                    while( pStyle )
                    {
                        nCount++;
                        aPop.InsertItem( nCount, pStyle->GetName() );
                        pStyle = pPool->Next();
                    }

                    aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
                    USHORT nId = aPop.GetCurId();
                    if( nId != USHRT_MAX)
                    {
                        // sieht etwas umstaendlich aus, anders geht's aber nicht
                        pStyle = pPool->operator[]( nId - 1 );
                        SfxStringItem aStyle( FN_SET_PAGE_STYLE, pStyle->GetName() );
                        pWrtShell->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                    FN_SET_PAGE_STYLE,
                                    SFX_CALLMODE_SLOT|SFX_CALLMODE_RECORD,
                                    &aStyle, 0L );
                    }
                }
            }
        }
        ReleaseMouse();
    }
}

/*************************************************************************


      $Log: not supported by cvs2svn $
      Revision 1.27  2000/09/18 16:06:19  willem.vandorp
      OpenOffice header added.

      Revision 1.26  2000/09/07 15:59:35  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.25  2000/02/11 15:01:09  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.24  1998/11/11 15:16:02  OS
      #59267# include


      Rev 1.23   11 Nov 1998 16:16:02   OS
   #59267# include

      Rev 1.22   10 Nov 1998 15:14:20   OS
   #59267# Vorlagenmenue nur ohne Selektion

      Rev 1.21   03 Sep 1998 12:30:32   OS
   #56005# Alle Seitenvorlagen in der Statuszeile anbieten

      Rev 1.20   21 Nov 1997 12:10:14   MA
   includes

      Rev 1.19   03 Nov 1997 13:59:22   MA
   precomp entfernt

      Rev 1.18   29 Aug 1997 15:57:30   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.17   29 Aug 1997 13:58:50   OS
   DLL-Umbau

      Rev 1.16   08 Aug 1997 17:25:18   OM
   Headerfile-Umstellung

      Rev 1.15   09 Jul 1997 17:45:32   HJS
   includes

      Rev 1.14   14 Jan 1997 09:39:00   TRI
   includes wegen Internal Compiler Error

      Rev 1.13   28 Aug 1996 15:42:38   OS
   includes

      Rev 1.12   26 Jun 1996 15:04:04   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.11   22 Mar 1996 15:18:38   HJS
   umstellung 311

      Rev 1.10   24 Nov 1995 16:57:24   OM
   PCH->PRECOMPILED

      Rev 1.9   17 Nov 1995 15:16:22   MA
   Segmentierung

      Rev 1.8   07 Nov 1995 18:04:04   MA
   opt/fix/chg: statics und segs

      Rev 1.7   30 Oct 1995 18:46:50   OM
   GetActiveView entfernt

      Rev 1.6   06 Oct 1995 00:20:14   HJS
   pState => eState

      Rev 1.5   05 Oct 1995 18:41:14   OM
   StateChanged umgestellt

      Rev 1.4   09 Aug 1995 19:15:18   OS
   Umstellung auf CommandHdl

      Rev 1.3   20 Jul 1995 19:00:20   OS
   kein Absturz ohne View

      Rev 1.2   17 Jul 1995 15:19:58   ER
   wie waer's mit exports?

      Rev 1.1   17 Jul 1995 14:51:48   ER
   wie waer's mit exports?

      Rev 1.0   14 Jul 1995 18:17:30   OS
   Initial revision.

*************************************************************************/


