/*************************************************************************
 *
 *  $RCSfile: uiborder.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:38 $
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

#ifndef _SVX_BORDER_HXX //autogen
#include <svx/border.hxx>
#endif


#include "swtypes.hxx"
#include "uiborder.hxx"
#include "frmui.hrc"




SwBorderDlg::SwBorderDlg(Window* pParent, SfxItemSet& rSet, USHORT nType) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    SetText(SW_RESSTR(STR_FRMUI_BORDER));

    // TabPage erzeugen
    SvxBorderTabPage* pPage = (SvxBorderTabPage*) SvxBorderTabPage::Create(this, rSet);
    pPage->SetSWMode(nType);
    if(SW_BORDER_MODE_TABLE == nType)
        pPage->HideShadowControls();
    SetTabPage(pPage);
}



SwBorderDlg::~SwBorderDlg()
{
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/ui/frmdlg/uiborder.cxx,v 1.1.1.1 2000-09-18 17:14:38 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.84  2000/09/18 16:05:35  willem.vandorp
      OpenOffice header added.

      Revision 1.83  2000/01/31 12:29:56  os
      #70943# SwBorderDlg: call SvxBorderTabPage::SetSWMode

      Revision 1.82  1997/11/24 16:40:16  MA
      include


      Rev 1.81   24 Nov 1997 17:40:16   MA
   include

      Rev 1.80   03 Nov 1997 13:19:46   MA
   precomp entfernt

      Rev 1.79   14 Nov 1996 15:32:36   TRI
   includes

      Rev 1.78   11 Nov 1996 10:44:14   MA
   ResMgr

      Rev 1.77   28 Aug 1996 11:52:58   OS
   includes

      Rev 1.76   24 Nov 1995 16:58:12   OM
   PCH->PRECOMPILED

      Rev 1.75   16 Aug 1995 17:48:26   MA
   Riesenheader dialogs entfernt.

      Rev 1.74   31 Mar 1995 13:10:40   OM
   SfxModalDialog -> SfxSingleTabDialog

      Rev 1.73   29 Mar 1995 08:32:18   OS
   SfxModalDialog - Parameter zusatzlich

      Rev 1.72   07 Feb 1995 19:19:12   PK
   caption fuer uiborder und pattern

      Rev 1.71   01 Feb 1995 17:45:16   PK
   hideshadowcontrol()

      Rev 1.70   27 Jan 1995 07:24:36   MS
   HideShadowControls auskommentiert

      Rev 1.69   26 Jan 1995 17:37:32   PK
   hideshadowcontrols() scharfgeschaltet

      Rev 1.68   26 Jan 1995 17:02:00   PK
   ctor erweiert

*************************************************************************/


