/*************************************************************************
 *
 *  $RCSfile: textglos.cxx,v $
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

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif

#include "errhdl.hxx"
#include "view.hxx"
#include "initui.hxx"
#include "cmdid.h"
#include "textsh.hxx"
#include "initui.hxx"
#include "glossary.hxx"
#include "gloshdl.hxx"
#include "glosdoc.hxx"
#include "gloslst.hxx"

// STATIC DATA -----------------------------------------------------------

void SwTextShell::ExecGlossary(SfxRequest &rReq)
{
    USHORT nSlot = rReq.GetSlot();
    ::GetGlossaries()->UpdateGlosPath(!rReq.IsAPI() ||
                                        FN_GLOSSARY_DLG == nSlot );
    SwGlossaryHdl* pGlosHdl = GetView().GetGlosHdl();
    // SwGlossaryList updaten?
    BOOL bUpdateList = FALSE;

    switch( nSlot )
    {
        case FN_GLOSSARY_DLG:
            pGlosHdl->GlossaryDlg();
            bUpdateList = TRUE;
            break;
        case FN_EXPAND_GLOSSARY:
        {
            BOOL bReturn;
            bReturn = pGlosHdl->ExpandGlossary( TRUE );
            rReq.SetReturnValue( SfxBoolItem( nSlot, bReturn ) );
        }
        break;
        default:
            ASSERT(FALSE, falscher Dispatcher);
            return;
    }
    if(bUpdateList)
    {
        SwGlossaryList* pList = ::GetGlossaryList();
        if(pList->IsActive())
            pList->Update();
    }
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.24  2000/09/18 16:06:06  willem.vandorp
    OpenOffice header added.

    Revision 1.23  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.22  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.21  2000/02/10 10:37:03  os
    #70359# titles added to AutoText groups

    Revision 1.20  1997/11/24 08:47:08  MA
    includes


      Rev 1.19   24 Nov 1997 09:47:08   MA
   includes

      Rev 1.18   03 Nov 1997 13:55:46   MA
   precomp entfernt

      Rev 1.17   20 Aug 1997 15:24:28   OS
   GetGlossaryGroupName: Pfad-Extension an Position Null nicht ausgeben #42968#

      Rev 1.16   19 Aug 1997 10:39:04   OS
   ExpandGlossary darf per API keine MessageBoxen erzeugen #42903#

      Rev 1.15   07 Apr 1997 17:42:26   MH
   chg: header

      Rev 1.14   14 Mar 1997 14:38:18   OS
   neu: CreateGlossaryGroup

      Rev 1.13   27 Sep 1996 12:51:52   OS
   Updates fuer die GlossaryList

      Rev 1.12   21 Sep 1996 15:57:32   OS
   InsertGlossary returnt BOOL

      Rev 1.11   30 Aug 1996 12:41:02   OS
   UpdateGlosPath kann Blockliste aktualisieren

      Rev 1.10   28 Aug 1996 15:54:48   OS
   includes

      Rev 1.9   29 Jul 1996 12:12:18   OS
   vor jeder Textblock-Operation den GlossaryPath updaten

      Rev 1.8   17 Jul 1996 07:48:52   OS
   neu: GetGlossaryShortName

      Rev 1.7   22 May 1996 15:33:06   OS
   InsertGlossary: akt. Gruppe auch am pGlosHdl setzen

      Rev 1.6   30 Apr 1996 08:04:00   OS
   auch im RenameGlossary darf die aktuelle Gruppe nicht am gloshdl eingestell werden, Bug#27350#

      Rev 1.5   13 Mar 1996 16:25:48   OS
   neu: RenameGlossary

      Rev 1.4   10 Mar 1996 15:46:34   OS
   neue Slots fuer Autotext: Gruppenanzahl und Elemente erfragen

      Rev 1.3   17 Feb 1996 12:36:00   OS
   neu: Textbausteine und -gruppen loeschen, Bug #25606#, NewGlossary legt jetzt auch Bereiche neu an, Bug #25576#

      Rev 1.2   26 Jan 1996 11:00:16   OS
   akt. Gruppe fuer Textbausteine nur am GlossaryDlg und nicht am GlossaryHdl setzen, Bug #24473#

      Rev 1.1   30 Nov 1995 15:26:28   OS
   +pragma

      Rev 1.0   30 Nov 1995 12:37:28   OM
   Initial revision.

------------------------------------------------------------------------*/


