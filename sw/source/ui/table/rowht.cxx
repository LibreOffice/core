/*************************************************************************
 *
 *  $RCSfile: rowht.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-09 16:54:59 $
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

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX //autogen
#include <svx/dlgutil.hxx>
#endif


#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ROWHT_HXX
#include <rowht.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
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
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _ROWHT_HRC
#include <rowht.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif




void SwTableHeightDlg::Apply()
{
    SwTwips nHeight = aHeightEdit.Denormalize(aHeightEdit.GetValue(FUNIT_TWIP));
    SwFmtFrmSize aSz(ATT_FIX_SIZE, 0, nHeight);

    SwFrmSize eFrmSize = (SwFrmSize) aAutoHeightCB.IsChecked() ?
        ATT_MIN_SIZE : ATT_FIX_SIZE;
    if(eFrmSize != aSz.GetSizeType())
    {
        aSz.SetSizeType(eFrmSize);
    }
    rSh.SetRowHeight( aSz );
}

// CTOR / DTOR -----------------------------------------------------------


SwTableHeightDlg::SwTableHeightDlg( Window *pParent, SwWrtShell &rS ) :

    SvxStandardDialog(pParent, SW_RES(DLG_ROW_HEIGHT)),

    aHeightEdit(this, SW_RES(ED_HEIGHT)),
    aAutoHeightCB(this, SW_RES(CB_AUTOHEIGHT)),
    aHeightFrm(this, SW_RES(GB_HEIGHT)),
    aOKBtn(this, SW_RES(BT_OK)),
    aCancelBtn(this, SW_RES(BT_CANCEL)),
    aHelpBtn( this, SW_RES( BT_HELP ) ),
    rSh( rS )
{
    FreeResource();

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( 0 != PTR_CAST( SwWebDocShell,
                                rSh.GetView().GetDocShell() ) )->GetMetric();
    ::SetFieldUnit( aHeightEdit, eFieldUnit );

    aHeightEdit.SetMin(MINLAY, FUNIT_TWIP);
    if(!aHeightEdit.GetMin())
        aHeightEdit.SetMin(1);
    SwFmtFrmSize *pSz;
    rSh.GetRowHeight( pSz );
    if ( pSz )
    {
        long nHeight = pSz->GetHeight();
        aAutoHeightCB.Check(pSz->GetSizeType() != ATT_FIX_SIZE);
        aHeightEdit.SetValue(aHeightEdit.Normalize(nHeight), FUNIT_TWIP);

        delete pSz;
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/10/06 13:37:52  jp
    should changes: don't use IniManager

    Revision 1.1.1.1  2000/09/18 17:14:48  hr
    initial import

    Revision 1.38  2000/09/18 16:06:08  willem.vandorp
    OpenOffice header added.

    Revision 1.37  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.36  1998/05/15 05:27:00  OS
    HelpButton


      Rev 1.35   15 May 1998 07:27:00   OS
   HelpButton

      Rev 1.34   24 Nov 1997 15:52:22   MA
   includes

      Rev 1.33   03 Nov 1997 13:56:50   MA
   precomp entfernt

      Rev 1.32   15 Aug 1997 12:18:22   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.31   08 Jan 1997 10:34:48   OS
   falschen default entfernt

      Rev 1.30   17 Dec 1996 16:59:52   OS
   per Default MM50 als Normhoehe

      Rev 1.29   11 Dec 1996 20:58:14   MH
   includes

      Rev 1.28   11 Dec 1996 08:43:16   OS
   Minimalwert nie NULL, Einheit richtig einstellen

      Rev 1.27   26 Nov 1996 19:45:10   OS
   minimale Zellenhoehe jetzt MINLAY

      Rev 1.26   11 Nov 1996 11:20:18   MA
   ResMgr

      Rev 1.25   28 Aug 1996 14:33:12   OS
   includes

      Rev 1.24   26 Jun 1996 15:25:22   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.23   26 Jun 1996 10:59:48   OM
   Autohoehe fuer Tabellenzeile

      Rev 1.22   25 Jun 1996 18:13:34   OM
   Autohoehe fuer Tabellenzeile

      Rev 1.21   20 Jun 1996 13:57:38   MA
   chg: RowHeight jetzt mit minimalgroesse

      Rev 1.20   24 Nov 1995 16:58:58   OM
   PCH->PRECOMPILED

      Rev 1.19   10 Nov 1995 15:26:16   OS
   Execute mit 0L abschliessen

      Rev 1.18   05 Nov 1995 16:48:32   OS
   Dtor ueberfluessig, -RecordFlag, Aktion ueber Dispatcher

      Rev 1.17   13 Sep 1995 17:09:42   OS
   Recording ausgeweitet

      Rev 1.16   30 Aug 1995 14:04:24   MA
   fix: sexport'iert

      Rev 1.15   21 Aug 1995 09:34:04   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

------------------------------------------------------------------------*/



