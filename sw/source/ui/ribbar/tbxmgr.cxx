/*************************************************************************
 *
 *  $RCSfile: tbxmgr.cxx,v $
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

#include "cmdid.h"
#include "swtypes.hxx"          // nur wegen aEmptyString??
#include "errhdl.hxx"
#include "wdocsh.hxx"
#include "tbxmgr.hxx"


/*************************************************************************
|*
|*
|*
\************************************************************************/

SwPopupWindowTbxMgr::SwPopupWindowTbxMgr( USHORT nId, WindowAlign eAlign,
                      ResId aRIdWin, ResId aRIdTbx,
                      SfxBindings& rBindings ) :
                SvxPopupWindowTbxMgr( nId, eAlign, aRIdWin, aRIdTbx, rBindings),
                bWeb(FALSE),
                aRIdWinTemp(aRIdWin),
                aRIdTbxTemp(aRIdTbx)
{
    SfxObjectShell* pObjShell = SfxObjectShell::Current();
    if(PTR_CAST(SwWebDocShell, pObjShell))
    {
        bWeb = TRUE;
        ToolBox& rTbx = GetTbxMgr().GetToolBox();
        // jetzt muessen ein paar Items aus der Toolbox versteckt werden:
        switch(nId)
        {
            case FN_INSERT_CTRL:
                rTbx.ShowItem(FN_INSERT_FRAME_INTERACT_NOCOL);
                rTbx.HideItem(FN_INSERT_FRAME_INTERACT);
                rTbx.HideItem(FN_INSERT_FOOTNOTE);
                rTbx.HideItem(FN_INSERT_ENDNOTE);
                rTbx.HideItem(FN_PAGE_STYLE_SET_COLS);
                rTbx.HideItem(FN_INSERT_IDX_ENTRY_DLG);

            break;
            case FN_INSERT_FIELD_CTRL:
                rTbx.HideItem(FN_INSERT_FLD_PGNUMBER);
                rTbx.HideItem(FN_INSERT_FLD_PGCOUNT);
                rTbx.HideItem(FN_INSERT_FLD_TOPIC);
                rTbx.HideItem(FN_INSERT_FLD_TITLE);
            break;
        }
    }
    else if( FN_INSERT_CTRL == nId)
    {
        ToolBox& rTbx = GetTbxMgr().GetToolBox();
        rTbx.ShowItem(FN_INSERT_FRAME_INTERACT);
        rTbx.HideItem(FN_INSERT_FRAME_INTERACT_NOCOL);
    }

    Size aSize = GetTbxMgr().CalcWindowSizePixel();
    GetTbxMgr().SetPosSizePixel( Point(), aSize );
    SetOutputSizePixel( aSize );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SwPopupWindowTbxMgr::StateChanged(USHORT nSID, SfxItemState eState,
                                                    const SfxPoolItem* pState)
{
    static USHORT __READONLY_DATA aInsertCtrl[] =
    {
        FN_INSERT_FRAME_INTERACT,
        FN_INSERT_FOOTNOTE,
        FN_INSERT_ENDNOTE,
        FN_PAGE_STYLE_SET_COLS,
        FN_INSERT_IDX_ENTRY_DLG,
        0
    };
    static USHORT __READONLY_DATA aInsertFld[] =
    {
        FN_INSERT_FLD_PGNUMBER,
        FN_INSERT_FLD_PGCOUNT,
        FN_INSERT_FLD_TOPIC,
        FN_INSERT_FLD_TITLE,
        0
    };

    SfxObjectShell* pObjShell = SfxObjectShell::Current();
    BOOL bNewWeb = 0 != PTR_CAST(SwWebDocShell, pObjShell);
    if(bWeb != bNewWeb)
    {
        bWeb = bNewWeb;
        ToolBox& rTbx = GetTbxMgr().GetToolBox();
        // jetzt muessen ein paar Items aus der Toolbox versteckt werden:
        const USHORT* pSid = 0;

        switch(nSID)
        {
            case FN_INSERT_CTRL:
            pSid = &aInsertCtrl[0];
                if(bWeb)
                    rTbx.ShowItem(FN_INSERT_FRAME_INTERACT_NOCOL);
                else
                    rTbx.HideItem(FN_INSERT_FRAME_INTERACT_NOCOL);
            break;
            case FN_INSERT_FIELD_CTRL:
                pSid = & aInsertFld[0];
            break;
        }
        if(pSid)
        {
            if(bWeb)
                while(*pSid)
                {
                    rTbx.HideItem(*pSid);
                    pSid++;
                }
            else
                while(*pSid)
                {
                    rTbx.ShowItem(*pSid);
                    pSid++;
                }
            Size aSize = GetTbxMgr().CalcWindowSizePixel();
            GetTbxMgr().SetPosSizePixel( Point(), aSize );
            SetOutputSizePixel( aSize );
        }
    }
    SfxPopupWindow::StateChanged(nSID, eState, pState);
}


SfxPopupWindow* SwPopupWindowTbxMgr::Clone() const
{
    return new SwPopupWindowTbxMgr(
            GetId(),
            ((SwPopupWindowTbxMgr*)this)->GetTbxMgr().GetToolBox().GetAlign(),
            aRIdWinTemp,
            aRIdTbxTemp,
            (SfxBindings&)GetBindings()
            );
}

/***********************************************************************

        $Log: not supported by cvs2svn $
        Revision 1.10  2000/09/18 16:06:02  willem.vandorp
        OpenOffice header added.

        Revision 1.9  2000/06/20 14:48:39  os
        #70060# less occurences of columns in HTML

        Revision 1.8  2000/03/03 15:17:03  os
        StarView remainders removed

        Revision 1.7  1999/02/12 07:21:26  OS
        #61547# InsertEndnote in der Wz.Leiste


      Rev 1.6   12 Feb 1999 08:21:26   OS
   #61547# InsertEndnote in der Wz.Leiste

      Rev 1.5   06 Apr 1998 09:57:30   OS
   INSERT_FRAME_INTERACT fuer HTML wieder erlaubt

      Rev 1.4   24 Nov 1997 14:53:58   MA
   includes

      Rev 1.3   03 Nov 1997 13:24:20   MA
   precomp entfernt

      Rev 1.2   23 Mar 1997 23:54:26   OS
   ToolBoxAlign statt ToolBoxControl uebergeben

      Rev 1.1   29 Jan 1997 17:16:36   NF
   includes...

      Rev 1.0   29 Jan 1997 14:51:48   OM
   Initial revision.

      Rev 1.36   28 Jan 1997 16:29:50   OS
   Anpassung der ToolBox an die DocShell

      Rev 1.35   15 Jan 1997 11:32:52   OM
   Bezier-Controller entfernt

      Rev 1.34   11 Dec 1996 12:33:46   OM
   Bezier-Controller

      Rev 1.33   30 Nov 1996 14:07:24   OS
   SID_OBJECT_ALIGN: dieser Controller tauscht die Images nicht aus!

      Rev 1.32   25 Nov 1996 10:56:50   OS
   SwPopupWindowTbx entfernt

      Rev 1.31   23 Nov 1996 12:41:26   OS
   StateChanged fuer ...TbxMgr

      Rev 1.30   14 Nov 1996 10:22:50   TRI
   includes

      Rev 1.29   13 Nov 1996 15:30:08   OS
   RegisterToolBox nur noch fuer ..OUTSTYLE

      Rev 1.28   12 Nov 1996 10:20:32   OS
   Image an der Toolbox kann nur der Toolboxmgr selbst setzen

      Rev 1.27   22 Oct 1996 10:47:38   OS
   Image an der Toolbox austauschen

      Rev 1.26   18 Oct 1996 08:52:02   OS
   Alignment berichtigt

      Rev 1.25   14 Oct 1996 13:38:26   OS
   UseDefault statt Initialize am SfxToolBoxManager

      Rev 1.24   10 Oct 1996 10:37:14   OS
   Registrierung der Toolbox am SfxImageManager

      Rev 1.23   26 Sep 1996 08:17:24   OS
   Execute am Dispatcher mit OL abschliessen

      Rev 1.22   23 Sep 1996 14:44:00   OS
   Slot fuer ToolboxItem auch im SelectHdl rufen

      Rev 1.21   19 Sep 1996 19:40:04   HJS
   fehlende includes

      Rev 1.20   19 Sep 1996 16:05:14   OS
   neu: SwPopupWindowTbxMgr

      Rev 1.19   28 Aug 1996 13:39:28   OS
   includes

      Rev 1.18   29 Jul 1996 15:16:04   OS
   Clone impl.

***********************************************************************/

