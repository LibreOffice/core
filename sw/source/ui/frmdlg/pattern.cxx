/*************************************************************************
 *
 *  $RCSfile: pattern.cxx,v $
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

#ifndef _SVX_BACKGRND_HXX //autogen
#include <svx/backgrnd.hxx>
#endif


#include "swtypes.hxx"
#include "pattern.hxx"
#include "frmui.hrc"


/****************************************************************************
Ctor
****************************************************************************/



SwBackgroundDlg::SwBackgroundDlg(Window* pParent, const SfxItemSet& rSet) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    SetText(SW_RESSTR(STR_FRMUI_PATTERN));
    SetTabPage(SvxBackgroundTabPage::Create(this, rSet));
}

/****************************************************************************
Dtor
****************************************************************************/



SwBackgroundDlg::~SwBackgroundDlg()
{
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/ui/frmdlg/pattern.cxx,v 1.1.1.1 2000-09-18 17:14:38 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.62  2000/09/18 16:05:34  willem.vandorp
      OpenOffice header added.

      Revision 1.61  1997/11/24 16:40:16  MA
      include


      Rev 1.60   24 Nov 1997 17:40:16   MA
   include

      Rev 1.59   03 Nov 1997 13:19:48   MA
   precomp entfernt

      Rev 1.58   14 Nov 1996 15:31:26   TRI
   includes

      Rev 1.57   11 Nov 1996 10:44:16   MA
   ResMgr

      Rev 1.56   28 Aug 1996 11:52:44   OS
   includes

      Rev 1.55   24 Nov 1995 16:58:10   OM
   PCH->PRECOMPILED

      Rev 1.54   16 Aug 1995 17:48:32   MA
   Riesenheader dialogs entfernt.

      Rev 1.53   31 Mar 1995 13:10:38   OM
   SfxModalDialog -> SfxSingleTabDialog

      Rev 1.52   29 Mar 1995 08:32:04   OS
   SfxModalDialog - Parameter zusatzlich

      Rev 1.51   07 Feb 1995 19:19:10   PK
   caption fuer uiborder und pattern

      Rev 1.50   24 Jan 1995 13:10:24   SWG
   Map entfernt

      Rev 1.49   21 Nov 1994 13:11:24   PK
   sfxmodaldialog

      Rev 1.48   17 Nov 1994 13:06:00   PK
   swpatterndlg -> swbackgrounddlg

      Rev 1.47   25 Oct 1994 18:14:40   ER
   add: PCH

      Rev 1.46   19 Oct 1994 12:09:48   MA
   SV-ChangeXXX -> SetXXX.

*************************************************************************/



