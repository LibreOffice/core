/*************************************************************************
 *
 *  $RCSfile: multmrk.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:44 $
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

#include "multmrk.hxx"
#include "toxmgr.hxx"

#include "index.hrc"
#include "multmrk.hrc"


SwMultiTOXMarkDlg::SwMultiTOXMarkDlg( Window* pParent, SwTOXMgr& rTOXMgr ) :

    SvxStandardDialog(pParent, SW_RES(DLG_MULTMRK)),

    aEntryFT(this,  SW_RES(FT_ENTRY)),
    aTextFT(this,   SW_RES(FT_TEXT)),
    aTOXFT(this,    SW_RES(FT_TOX)),
    aOkBT(this,     SW_RES(OK_BT)),
    aCancelBT(this, SW_RES(CANCEL_BT)),
    aTOXLB(this,    SW_RES(LB_TOX)),
    aTOXGB(this,    SW_RES(GB_TOX)),
    rMgr( rTOXMgr ),
    nPos(0)
{
    aTOXLB.SetSelectHdl(LINK(this, SwMultiTOXMarkDlg, SelectHdl));

    USHORT nSize = rMgr.GetTOXMarkCount();
    for(USHORT i=0; i < nSize; ++i)
        aTOXLB.InsertEntry(rMgr.GetTOXMark(i)->GetText());

    aTOXLB.SelectEntryPos(0);
    aTextFT.SetText(rMgr.GetTOXMark(0)->GetTOXType()->GetTypeName());

    FreeResource();
}


IMPL_LINK_INLINE_START( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )
{
    if(pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {   SwTOXMark* pMark = rMgr.GetTOXMark(pBox->GetSelectEntryPos());
        aTextFT.SetText(pMark->GetTOXType()->GetTypeName());
        nPos = pBox->GetSelectEntryPos();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )


void SwMultiTOXMarkDlg::Apply()
{
    rMgr.SetCurTOXMark(nPos);
}

/*-----------------25.02.94 22:06-------------------
 dtor ueberladen
--------------------------------------------------*/


SwMultiTOXMarkDlg::~SwMultiTOXMarkDlg() {}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.28  2000/09/18 16:05:53  willem.vandorp
    OpenOffice header added.

    Revision 1.27  1997/11/05 11:43:50  OS
    Segmentierung entfernt


      Rev 1.26   05 Nov 1997 12:43:50   OS
   Segmentierung entfernt

      Rev 1.25   03 Nov 1997 13:20:56   MA
   precomp entfernt

      Rev 1.24   14 Nov 1996 15:34:12   TRI
   includes

      Rev 1.23   11 Nov 1996 10:55:34   MA
   ResMgr

      Rev 1.22   26 Jul 1996 21:04:22   MA
   includes

      Rev 1.21   06 Feb 1996 15:20:48   JP
   Link Umstellung 305

      Rev 1.20   24 Nov 1995 16:59:04   OM
   PCH->PRECOMPILED

      Rev 1.19   08 Nov 1995 13:47:00   OM
   Change->Set

      Rev 1.18   30 Aug 1995 14:07:02   MA
   fix: sexport'iert

      Rev 1.17   21 Aug 1995 09:36:02   MA
   chg: swstddlg -> svxstandarddialog, Optimierungen

      Rev 1.16   25 Oct 1994 18:18:46   ER
   add: PCH

      Rev 1.15   26 Feb 1994 02:03:52   ER
   virt. dtor mit export

      Rev 1.14   25 Feb 1994 22:07:08   ER
   virtuellen dtor wegen export ueberladen

      Rev 1.13   22 Feb 1994 14:39:38   ER
   EXPORT --> SEXPORT

      Rev 1.12   21 Feb 1994 22:39:20   ER
   LINK EXPORT

      Rev 1.11   17 Feb 1994 08:46:28   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.10   16 Feb 1994 20:07:06   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.9   16 Feb 1994 16:46:26   MI
   Pragmas zurechtgerueckt

      Rev 1.8   28 Jan 1994 11:43:50   MI
   TCOV() entfernt, SW_... nach SEG_... umbenannt

      Rev 1.7   13 Jan 1994 08:44:40   MI
   Segmentierung per #define ermoeglicht

      Rev 1.6   09 Jan 1994 21:12:12   MI
   Provisorische Segmentierung

      Rev 1.5   26 Sep 1993 18:42:02   MI
   Segmentierung

      Rev 1.4   28 May 1993 11:48:22   VB
   Umstellung Precompiled Headers

      Rev 1.3   26 May 1993 10:36:32   MS
   Anpassung SwTOXMgr

      Rev 1.2   27 Apr 1993 08:37:28   OK
   NEU: #pragma hdrstop

      Rev 1.1   26 Apr 1993 11:49:12   MS
   Markentraveling aktiviert

      Rev 1.0   25 Feb 1993 14:47:06   MS
   Initial revision.

      Rev 1.12   23 Feb 1993 13:42:18   MS
   Form auswerten

------------------------------------------------------------------------*/


