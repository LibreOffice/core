/*************************************************************************
 *
 *  $RCSfile: tbxmgr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:40:08 $
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


