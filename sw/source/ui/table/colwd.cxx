/*************************************************************************
 *
 *  $RCSfile: colwd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 13:37:52 $
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

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX //autogen
#include <svx/dlgutil.hxx>
#endif

#ifndef _COLWD_HXX
#include <colwd.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _COLWD_HRC
#include <colwd.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif


IMPL_LINK_INLINE_START( SwTableWidthDlg, LoseFocusHdl, Edit *, EMPTYARG )
{
    USHORT nId = (USHORT)aColEdit.GetValue()-1;
    const SwTwips lWidth = rFnc.GetColWidth(nId);
    aWidthEdit.SetValue(aWidthEdit.Normalize(lWidth), FUNIT_TWIP);
    aWidthEdit.SetMax(aWidthEdit.Normalize(rFnc.GetMaxColWidth(nId)), FUNIT_TWIP);
    return 0;
}
IMPL_LINK_INLINE_END( SwTableWidthDlg, LoseFocusHdl, Edit *, EMPTYARG )



SwTableWidthDlg::SwTableWidthDlg(Window *pParent, SwTableFUNC &rTableFnc ) :

    SvxStandardDialog( pParent, SW_RES(DLG_COL_WIDTH) ),

    aColFT(this,        SW_RES(FT_COL)),
    aColEdit(this,      SW_RES(ED_COL)),
    aWidthFT(this,      SW_RES(FT_WIDTH)),
    aWidthEdit(this,    SW_RES(ED_WIDTH)),
    aWidthFrm(this,     SW_RES(GB_WIDTH)),
    aOKBtn(this,        SW_RES(BT_OK)),
    aCancelBtn(this,    SW_RES(BT_CANCEL)),
    aHelpBtn(this,      SW_RES(BT_HELP)),
    rFnc(rTableFnc)
{
    FreeResource();

    BOOL bIsWeb = rTableFnc.GetShell()
                    ? 0 != PTR_CAST( SwWebDocShell,
                            rTableFnc.GetShell()->GetView().GetDocShell() )
                    : FALSE;
    FieldUnit eFieldUnit = SW_MOD()->GetModuleConfig()->GetMetric( bIsWeb );
    ::SetFieldUnit(aWidthEdit, eFieldUnit );

    aColEdit.SetValue( rFnc.GetCurColNum() +1 );
    aWidthEdit.SetMin(aWidthEdit.Normalize(MINLAY), FUNIT_TWIP);
    if(!aWidthEdit.GetMin())
        aWidthEdit.SetMin(1);

    if(rFnc.GetColCount() == 0)
        aWidthEdit.SetMin(aWidthEdit.Normalize(rFnc.GetColWidth(0)), FUNIT_TWIP);
    aColEdit.SetMax(rFnc.GetColCount() +1 );
    aColEdit.SetModifyHdl(LINK(this,SwTableWidthDlg, LoseFocusHdl));
    LoseFocusHdl();
}



void SwTableWidthDlg::Apply()
{
    rFnc.InitTabCols();
    rFnc.SetColWidth( aColEdit.GetValue()-1,
            aWidthEdit.Denormalize(aWidthEdit.GetValue(FUNIT_TWIP)));
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.40  2000/09/18 16:06:07  willem.vandorp
    OpenOffice header added.

    Revision 1.39  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.38  1998/05/13 14:00:46  OS
    HelpButton


      Rev 1.37   13 May 1998 16:00:46   OS
   HelpButton

      Rev 1.36   24 Nov 1997 15:52:22   MA
   includes

      Rev 1.35   03 Nov 1997 13:56:50   MA
   precomp entfernt

      Rev 1.34   11 Dec 1996 20:58:14   MH
   includes

      Rev 1.33   11 Dec 1996 08:43:16   OS
   Minimalwert nie NULL, Einheit richtig einstellen

      Rev 1.32   11 Nov 1996 11:19:56   MA
   ResMgr

      Rev 1.31   05 Sep 1996 08:19:30   OS
   richtige Min/Max-Einstellungen fuer einspaltige Tabellen

      Rev 1.30   28 Aug 1996 14:33:16   OS
   includes

      Rev 1.29   26 Jun 1996 15:25:20   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.28   06 Jun 1996 14:55:42   OS
   MINLAY als Minimum fuer Breite setzen

      Rev 1.27   22 Mar 1996 14:16:30   HJS
   umstellung 311

      Rev 1.26   07 Feb 1996 08:11:40   OS
   INLINE-LoseFocusHdl muss vor erster Benutzung definiert werden

      Rev 1.25   06 Feb 1996 15:21:22   JP
   Link Umstellung 305

      Rev 1.24   24 Nov 1995 16:58:58   OM
   PCH->PRECOMPILED

      Rev 1.23   10 Nov 1995 15:26:44   OS
   Execute mit 0L abschliessen

      Rev 1.22   08 Nov 1995 13:34:38   OS
   Change => Set

      Rev 1.21   05 Nov 1995 16:49:08   OS
   Dtor ueberfluessig, -RecordFlag, Aktion ueber Dispatcher

      Rev 1.20   13 Sep 1995 17:09:44   OS
   Recording ausgeweitet

------------------------------------------------------------------------*/



