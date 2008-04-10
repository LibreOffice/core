/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: insrc.cxx,v $
 * $Revision: 1.10 $
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
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <sfx2/dispatch.hxx>
#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>

#include "cmdid.h"
#include "swtypes.hxx"
#include "insrc.hxx"
#include "view.hxx"
#include "table.hrc"
#include "insrc.hrc"

void SwInsRowColDlg::Apply()
{
    USHORT nSlot = bColumn ? FN_TABLE_INSERT_COL : FN_TABLE_INSERT_ROW;
    SfxUInt16Item aCountItem( nSlot, static_cast< UINT16 >(aCountEdit.GetValue()) );
    SfxBoolItem  aAfter( FN_PARAM_INSERT_AFTER, aAfterBtn.IsChecked() );
    rView.GetViewFrame()->GetDispatcher()->Execute( nSlot,
            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
            &aCountItem, &aAfter, 0L);
}



SwInsRowColDlg::SwInsRowColDlg(SwView& rVw, BOOL bCol )
    : SvxStandardDialog( rVw.GetWindow(), SW_RES(DLG_INS_ROW_COL) ),
    aCount( this, SW_RES( FT_COUNT ) ),
    aCountEdit( this, SW_RES( ED_COUNT ) ),
    aInsFL( this, SW_RES( FL_INS ) ),
    aBeforeBtn( this, SW_RES( CB_POS_BEFORE ) ),
    aAfterBtn( this, SW_RES( CB_POS_AFTER ) ),
    aPosFL( this, SW_RES( FL_POS ) ),
    aRow(SW_RES(STR_ROW)),
    aCol(SW_RES(STR_COL)),
    aOKBtn( this, SW_RES( BT_OK ) ),
    aCancelBtn( this, SW_RES( BT_CANCEL ) ),
    aHelpBtn( this, SW_RES( BT_HELP ) ),
    rView(rVw),
    bColumn( bCol )
{
    FreeResource();
    String aTmp( GetText() );
    if( bColumn )
    {
        aTmp += aCol;
        SetHelpId( FN_TABLE_INSERT_COL_DLG );
    }
    else
    {
        aTmp += aRow;
        SetHelpId( FN_TABLE_INSERT_ROW_DLG );
    }
    SetText( aTmp );
}




