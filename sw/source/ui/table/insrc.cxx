/*************************************************************************
 *
 *  $RCSfile: insrc.cxx,v $
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

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#include "cmdid.h"
#include "swtypes.hxx"
#include "insrc.hxx"
#include "view.hxx"
#include "table.hrc"
#include "insrc.hrc"




void SwInsRowColDlg::Apply()
{
    USHORT nSlot = bColumn ? FN_TABLE_INSERT_COL : FN_TABLE_INSERT_ROW;
    SfxUInt16Item aCount( nSlot, aCountEdit.GetValue() );
    SfxBoolItem  aAfter( FN_PARAM_INSERT_AFTER, aAfterBtn.IsChecked() );
    rView.GetViewFrame()->GetDispatcher()->Execute( nSlot,
            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
            &aCount, &aAfter, 0L);
}



SwInsRowColDlg::SwInsRowColDlg(SwView& rVw, BOOL bCol )
    : SvxStandardDialog( rVw.GetWindow(), SW_RES(DLG_INS_ROW_COL) ),
    aCount( this, SW_RES( FT_COUNT ) ),
    aCountEdit( this, SW_RES( ED_COUNT ) ),
    aInsGrp( this, SW_RES( GB_INS ) ),
    aBeforeBtn( this, SW_RES( CB_POS_BEFORE ) ),
    aAfterBtn( this, SW_RES( CB_POS_AFTER ) ),
    aPosGrp( this, SW_RES( GB_POS ) ),
    aRow(SW_RES(STR_ROW)),
    aCol(SW_RES(STR_COL)),
    aOKBtn( this, SW_RES( BT_OK ) ),
    aCancelBtn( this, SW_RES( BT_CANCEL ) ),
    aHelpBtn( this, SW_RES( BT_HELP ) ),
    rView(rVw),
    bColumn( bCol )
{
    FreeResource();
    String aTmp( GetText() );
    if( bColumn )
    {
        aTmp += aCol;
        SetHelpId( FN_TABLE_INSERT_COL_DLG );
    }
    else
    {
        aTmp += aRow;
        SetHelpId( FN_TABLE_INSERT_ROW_DLG );
    }
    SetText( aTmp );
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.30  2000/09/18 16:06:08  willem.vandorp
    OpenOffice header added.

    Revision 1.29  2000/09/07 15:59:31  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.28  1998/05/13 14:00:48  OS
    HelpButton


      Rev 1.27   13 May 1998 16:00:48   OS
   HelpButton

      Rev 1.26   24 Nov 1997 15:52:22   MA
   includes

      Rev 1.25   03 Nov 1997 13:56:52   MA
   precomp entfernt

      Rev 1.24   14 Nov 1996 15:43:44   TRI
   includes

      Rev 1.23   11 Nov 1996 11:20:10   MA
   ResMgr

      Rev 1.22   04 Sep 1996 11:16:52   JP
   TableManager wird nicht mehr benoetigt

      Rev 1.21   28 Aug 1996 14:33:14   OS
   includes

      Rev 1.20   26 Jun 1996 15:25:20   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.19   22 Mar 1996 14:16:32   HJS
   umstellung 311

      Rev 1.18   24 Nov 1995 16:58:58   OM
   PCH->PRECOMPILED

      Rev 1.17   10 Nov 1995 15:26:16   OS
   Execute mit 0L abschliessen

      Rev 1.16   05 Nov 1995 16:49:32   OS
   Dtor ueberfluessig, -RecordFlag, Aktion ueber Dispatcher

      Rev 1.15   13 Sep 1995 17:10:04   OS
   Recording ausgeweitet

      Rev 1.14   30 Aug 1995 14:02:52   MA
   fix: sexport'iert

      Rev 1.13   21 Aug 1995 09:34:28   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.12   25 Oct 1994 18:52:28   ER
   add: PCH

      Rev 1.11   20 Apr 1994 15:34:20   MA
   operator Optimierung.

------------------------------------------------------------------------*/




