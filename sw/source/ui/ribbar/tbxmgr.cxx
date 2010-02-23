/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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
/*
SwPopupWindowTbxMgr::SwPopupWindowTbxMgr( USHORT nId, WindowAlign eAlign,
                      ResId aRIdWin, ResId aRIdTbx,
                      SfxBindings& rBindings ) :
                SvxPopupWindowTbxMgr( nId, eAlign, aRIdWin, aRIdTbx ),
                bWeb(FALSE),
                aRIdWinTemp(aRIdWin),
                aRIdTbxTemp(aRIdTbx),
                eAlignment( eAlign ),
                mrBindings( rBindings )
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
*/
/*************************************************************************
|*
|*
|*
\************************************************************************/
/*
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
*/
/*
SfxPopupWindow* SwPopupWindowTbxMgr::Clone() const
{
    return new SwPopupWindowTbxMgr(
            GetId(),
            eAlignment,
//          ((SwPopupWindowTbxMgr*)this)->GetTbxMgr().GetToolBox().GetAlign(),
            aRIdWinTemp,
            aRIdTbxTemp,
            mrBindings
//          (SfxBindings&)GetBindings()
            );
}
*/

