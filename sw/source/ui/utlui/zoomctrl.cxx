/*************************************************************************
 *
 *  $RCSfile: zoomctrl.cxx,v $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

#include "swtypes.hxx"
#include "zoomctrl.hxx"


SFX_IMPL_STATUSBAR_CONTROL( SwZoomControl, SvxZoomItem );


SwZoomControl::SwZoomControl( USHORT nId,
                                  StatusBar& rStb,
                                  SfxBindings& rBind ) :
    SvxZoomStatusBarControl( nId, rStb, rBind )
{
}

// -----------------------------------------------------------------------

SwZoomControl::~SwZoomControl()
{
}

// -----------------------------------------------------------------------

void SwZoomControl::StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    if(SFX_ITEM_AVAILABLE == eState && pState->ISA( SfxStringItem ))
    {
        sPreviewZoom = ((const SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
    }
    else
    {
        sPreviewZoom = aEmptyStr;
        SvxZoomStatusBarControl::StateChanged(nSID, eState, pState);
    }
}

// -----------------------------------------------------------------------

void SwZoomControl::Paint( const UserDrawEvent& rUsrEvt )
{
    if(!sPreviewZoom.Len())
        SvxZoomStatusBarControl::Paint(rUsrEvt);
    else
        GetStatusBar().SetItemText( GetId(), sPreviewZoom );
}

// -----------------------------------------------------------------------

void SwZoomControl::Command( const CommandEvent& rCEvt )
{
    if(!sPreviewZoom.Len())
        SvxZoomStatusBarControl::Command(rCEvt);
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.9  2000/09/18 16:06:19  willem.vandorp
      OpenOffice header added.

      Revision 1.8  2000/02/11 15:01:17  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.7  1997/11/21 11:10:18  MA
      includes


      Rev 1.6   21 Nov 1997 12:10:18   MA
   includes

      Rev 1.5   03 Nov 1997 13:59:24   MA
   precomp entfernt

      Rev 1.4   05 Sep 1997 12:23:06   MH
   chg: header

      Rev 1.3   06 Aug 1997 11:13:02   MH
   chg: header

      Rev 1.2   18 Jun 1996 16:59:44   OS
   ui_pch includen

      Rev 1.1   20 Mar 1996 13:42:54   HJS
   einmal segmentc.hxx reicht

      Rev 1.0   14 Jan 1996 15:30:32   OS
   Initial revision.

*************************************************************************/



