/*************************************************************************
 *
 *  $RCSfile: selglos.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#include "swtypes.hxx"
#include "segmentc.hxx"

#include "selglos.hxx"

#include "selglos.hrc"
#include "dochdl.hrc"

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

// STATIC DATA -----------------------------------------------------------


// CTOR / DTOR -----------------------------------------------------------

SwSelGlossaryDlg::SwSelGlossaryDlg(Window * pParent, const String &rShortName)
    : ModalDialog(pParent, SW_RES(DLG_SEL_GLOS)),
    aGlosBox(this, SW_RES( LB_GLOS)),
    aGlosFrm(this, SW_RES( GB_GLOS)),
    aOKBtn(this, SW_RES( BT_OK)),
    aCancelBtn(this, SW_RES( BT_CANCEL)),
    aHelpBtn(this, SW_RES(BT_HELP))
{
    String sText(aGlosFrm.GetText());
    sText += rShortName;
    aGlosFrm.SetText(sText);
    FreeResource();

    aGlosBox.SetDoubleClickHdl(LINK(this, SwSelGlossaryDlg, DoubleClickHdl));
}

/*-----------------25.02.94 20:50-------------------
 dtor ueberladen
--------------------------------------------------*/
SwSelGlossaryDlg::~SwSelGlossaryDlg() {}
/* -----------------25.10.99 08:33-------------------

 --------------------------------------------------*/
IMPL_LINK(SwSelGlossaryDlg, DoubleClickHdl, ListBox*, pBox)
{
    EndDialog(RET_OK);
    return 0;
}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.16  2000/09/18 16:05:22  willem.vandorp
    OpenOffice header added.

    Revision 1.15  2000/04/13 08:22:23  os
    UNICODE

    Revision 1.14  1999/10/25 06:44:30  os
    #68967# HelpButton and DoubleClickHdl

    Revision 1.13  1997/11/03 12:13:06  MA
    precomp entfernt


      Rev 1.12   03 Nov 1997 13:13:06   MA
   precomp entfernt

      Rev 1.11   14 Nov 1996 15:23:02   TRI
   includes

      Rev 1.10   11 Nov 1996 09:35:36   MA
   ResMgr

      Rev 1.9   28 Aug 1996 10:27:36   OS
   includes

      Rev 1.8   24 Nov 1995 16:58:42   OM
   PCH->PRECOMPILED

      Rev 1.7   25 Oct 1994 17:08:08   ER
   add: PCH

      Rev 1.6   26 Feb 1994 01:59:36   ER
   virt. dtor mit export

      Rev 1.5   25 Feb 1994 20:51:54   ER
   virtuellen dtor wegen export ueberladen

      Rev 1.4   17 Feb 1994 08:41:20   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.3   16 Feb 1994 20:02:32   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.2   16 Feb 1994 16:21:24   MI
   Pragmas zurechtgerueckt

      Rev 1.1   28 Jan 1994 11:41:02   MI
   TCOV() entfernt, SW_... nach SEG_... umbenannt

      Rev 1.0   25 Jan 1994 16:01:06   VB
   Initial revision.

------------------------------------------------------------------------*/


