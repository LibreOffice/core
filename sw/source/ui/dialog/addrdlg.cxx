/*************************************************************************
 *
 *  $RCSfile: addrdlg.cxx,v $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVX_OPTGENRL_HXX //autogen
#include <svx/optgenrl.hxx>
#endif

#include "addrdlg.hxx"


/****************************************************************************
Ctor
****************************************************************************/


SwAddrDlg::SwAddrDlg(Window* pParent, SfxItemSet& rSet ) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    // TabPage erzeugen
    SvxGeneralTabPage* pPage = (SvxGeneralTabPage*) SvxGeneralTabPage::Create(this, rSet);
    SetTabPage(pPage);
}

/****************************************************************************
Dtor
****************************************************************************/


__EXPORT SwAddrDlg::~SwAddrDlg()
{
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/ui/dialog/addrdlg.cxx,v 1.1.1.1 2000-09-18 17:14:34 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.10  2000/09/18 16:05:20  willem.vandorp
      OpenOffice header added.

      Revision 1.9  1997/11/25 09:43:20  MA
      includes


      Rev 1.8   25 Nov 1997 10:43:20   MA
   includes

      Rev 1.7   03 Nov 1997 13:12:26   MA
   precomp entfernt

      Rev 1.6   16 Jul 1997 16:26:50   TRI
   mit __EXPORT gehts viel besser

      Rev 1.5   15 Jul 1997 14:48:12   OS
   HideExtra entfernt

      Rev 1.4   28 Aug 1996 10:10:00   OS
   includes

      Rev 1.3   04 Jul 1996 09:03:38   SWG
   include svfield - Workaround wg. optgenrl.hxx

      Rev 1.2   24 Nov 1995 16:58:16   OM
   PCH->PRECOMPILED

      Rev 1.1   27 Jul 1995 18:20:56   OS
   HideExtras jetzt aktiv

      Rev 1.0   26 Jul 1995 19:20:02   OS
   Initial revision.

*************************************************************************/

































