/*************************************************************************
 *
 *  $RCSfile: dbui.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:34 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#include "wrtsh.hxx"


#include "dbui.hrc"
#include "dbui.hxx"


// STATIC DATA -----------------------------------------------------------


/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

#if 0
SwDBSelect::SwDBSelect(Window *pParent, SwWrtShell *pSh, const String& rDefDBName):

    SvxStandardDialog(pParent, SW_RES(DLG_DBSELECTION)),

    aTree           (this, SW_RES(TLB_DBLIST), rDefDBName),

    aOkBTN          (this, SW_RES(BTN_OK)),
    aCancelBTN      (this, SW_RES(BTN_CANCEL)),
    aHelpBTN        (this, SW_RES(BTN_HELP))
{
    FreeResource();

    aTree.SetSelectHdl(LINK(this, SwDBSelect, TreeSelectHdl));
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


__EXPORT SwDBSelect::~SwDBSelect()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


void __EXPORT SwDBSelect::Apply()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


String SwDBSelect::GetDBName() const
{
    return aTree.GetDBName();
}

/*---------------------------------------------------------------------
    Beschreibung:
---------------------------------------------------------------------*/


IMPL_LINK( SwDBSelect, TreeSelectHdl, SvTreeListBox *, pBox )
{
    SvLBoxEntry* pEntry = pBox->GetCurEntry();
    if (pEntry)
        aOkBTN.Enable(aTree.GetParent(pEntry) != 0);
    return 0;
}
#endif
/*---------------------------------------------------------------------
    Beschreibung:
---------------------------------------------------------------------*/

PrintMonitor::PrintMonitor( Window *pParent, BOOL bEMail )
:   ModelessDialog( pParent, SW_RES(DLG_PRINTMONITOR) ),
    aDocName    (this, SW_RES( FT_DOCNAME )),
    aPrinting   (this, SW_RES( bEMail ? FT_SENDING : FT_PRINTING )),
    aPrinter    (this, SW_RES( FT_PRINTER       )),
    aPrintInfo  (this, SW_RES( FT_PRINTINFO     )),
    aCancel     (this, SW_RES( PB_CANCELPRNMON  ))
{
    if (bEMail)
    {
        SetText(SW_RES(STR_EMAILMON));
    }
    FreeResource();
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.15  2000/09/18 16:05:19  willem.vandorp
    OpenOffice header added.

    Revision 1.14  2000/06/26 13:31:03  os
    new DataBase API

    Revision 1.13  1999/03/08 08:56:26  MH
    chg: header


      Rev 1.12   08 Mar 1999 09:56:26   MH
   chg: header

      Rev 1.11   27 Oct 1998 12:17:08   OM
   #58132# Progress fuer Serienmailings

      Rev 1.10   09 Dec 1997 12:30:52   OM
   #45200# Serienbrief: Speichern-Monitor

      Rev 1.9   02 Sep 1997 09:57:12   OM
   SDB-Headeranpassung

      Rev 1.8   01 Sep 1997 13:16:04   OS
   DLL-Umstellung

      Rev 1.7   11 Nov 1996 09:25:38   MA
   ResMgr

      Rev 1.6   02 Oct 1996 18:00:56   MA
   Umstellung Enable/Disable

      Rev 1.5   28 Aug 1996 09:18:04   OS
   includes

      Rev 1.4   05 Aug 1996 15:47:52   OM
   Datenbankumstellung

      Rev 1.3   25 Jul 1996 16:36:26   OM
   Eigene TreeListbox fuer Datenbankauswahl

      Rev 1.2   23 Jul 1996 13:28:38   OM
   Neue Segs

      Rev 1.1   23 Jul 1996 13:26:08   OM
   Datenbank und Tabelle auswaehlen

      Rev 1.0   22 Jul 1996 11:01:52   OM
   Initial revision.

------------------------------------------------------------------------*/

