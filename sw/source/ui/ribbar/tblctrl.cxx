/*************************************************************************
 *
 *  $RCSfile: tblctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:46 $
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


#include "cmdid.h"
#include "swtypes.hxx"
#include "tbxmgr.hxx"
#include "tblctrl.hxx"
#include "tblctrl.hrc"



SFX_IMPL_TOOLBOX_CONTROL( SwTableOptimizeCtrl, SfxUInt16Item );

/**********************************************************************

**********************************************************************/




SwTableOptimizeCtrl::SwTableOptimizeCtrl( USHORT nId,
                                    ToolBox& rTbx,
                                    SfxBindings& rBind ) :
        SfxToolBoxControl( nId, rTbx, rBind )
{
}
/**********************************************************************

**********************************************************************/



SwTableOptimizeCtrl::~SwTableOptimizeCtrl()
{
}

/**********************************************************************

**********************************************************************/



SfxPopupWindow* SwTableOptimizeCtrl::CreatePopupWindow()
{
    USHORT nWinResId, nTbxResId;
        nWinResId = RID_TBL_OPT_CTRL;
        nTbxResId = TBX_OPTIMIZE_TABLE;

    WindowAlign eAlign = WINDOWALIGN_TOP;
    if(GetToolBox().IsHorizontal())
        eAlign = WINDOWALIGN_LEFT;
    SwPopupWindowTbxMgr *pWin = new SwPopupWindowTbxMgr( GetId(),
                                    eAlign,
                                    SW_RES(nWinResId),
                                    SW_RES(nTbxResId),
                                    GetBindings());
    pWin->StartPopupMode(&GetToolBox(), TRUE);
    pWin->StartSelection();
    pWin->Show();
    return pWin;

}

/**********************************************************************

**********************************************************************/



SfxPopupWindowType  SwTableOptimizeCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}


/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.10  2000/09/18 16:06:01  willem.vandorp
        OpenOffice header added.

        Revision 1.9  2000/03/03 15:17:03  os
        StarView remainders removed

        Revision 1.8  1997/11/24 13:54:00  MA
        includes


      Rev 1.7   24 Nov 1997 14:54:00   MA
   includes

      Rev 1.6   03 Nov 1997 13:24:22   MA
   precomp entfernt

      Rev 1.5   24 Mar 1997 01:43:52   OS
   Alignment aussen einstellen

      Rev 1.4   23 Mar 1997 23:54:26   OS
   ToolBoxAlign statt ToolBoxControl uebergeben

      Rev 1.3   29 Jan 1997 14:52:24   OM
   Draw-Toolboxen ins Svx verschoben

      Rev 1.2   14 Nov 1996 15:40:48   TRI
   includes

      Rev 1.1   11 Nov 1996 11:15:14   MA
   ResMgr

      Rev 1.0   08 Nov 1996 13:39:36   OS
   Initial revision.


***********************************************************************/


























