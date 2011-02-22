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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <sfx2/dispatch.hxx>
#include <svx/dlgutil.hxx>
#include <colwd.hxx>
#include <tablemgr.hxx>
#include <wrtsh.hxx>
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <swmodule.hxx>
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#include <usrpref.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _COLWD_HRC
#include <colwd.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif


IMPL_LINK_INLINE_START( SwTableWidthDlg, LoseFocusHdl, Edit *, EMPTYARG )
{
    sal_uInt16 nId = (sal_uInt16)aColEdit.GetValue()-1;
    const SwTwips lWidth = rFnc.GetColWidth(nId);
    aWidthEdit.SetValue(aWidthEdit.Normalize(lWidth), FUNIT_TWIP);
    aWidthEdit.SetMax(aWidthEdit.Normalize(rFnc.GetMaxColWidth(nId)), FUNIT_TWIP);
    return 0;
}
IMPL_LINK_INLINE_END( SwTableWidthDlg, LoseFocusHdl, Edit *, EMPTYARG )



SwTableWidthDlg::SwTableWidthDlg(Window *pParent, SwTableFUNC &rTableFnc ) :

    SvxStandardDialog( pParent, SW_RES(DLG_COL_WIDTH) ),
    aWidthFL(this,     SW_RES(FL_WIDTH)),

    aColFT(this,        SW_RES(FT_COL)),
    aColEdit(this,      SW_RES(ED_COL)),
    aWidthFT(this,      SW_RES(FT_WIDTH)),
    aWidthEdit(this,    SW_RES(ED_WIDTH)),
    aOKBtn(this,        SW_RES(BT_OK)),
    aCancelBtn(this,    SW_RES(BT_CANCEL)),
    aHelpBtn(this,      SW_RES(BT_HELP)),
    rFnc(rTableFnc)
{
    FreeResource();

    sal_Bool bIsWeb = rTableFnc.GetShell()
                    ? static_cast< sal_Bool >(0 != PTR_CAST( SwWebDocShell,
                            rTableFnc.GetShell()->GetView().GetDocShell()) )
                    : sal_False;
    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( bIsWeb )->GetMetric();
    ::SetFieldUnit(aWidthEdit, eFieldUnit );

    aColEdit.SetValue( rFnc.GetCurColNum() +1 );
    aWidthEdit.SetMin(aWidthEdit.Normalize(MINLAY), FUNIT_TWIP);
    if(!aWidthEdit.GetMin())
        aWidthEdit.SetMin(1);

    if(rFnc.GetColCount() == 0)
        aWidthEdit.SetMin(aWidthEdit.Normalize(rFnc.GetColWidth(0)), FUNIT_TWIP);
    aColEdit.SetMax(rFnc.GetColCount() +1 );
    aColEdit.SetModifyHdl(LINK(this,SwTableWidthDlg, LoseFocusHdl));
    LoseFocusHdl();
}



void SwTableWidthDlg::Apply()
{
    rFnc.InitTabCols();
    rFnc.SetColWidth(
            static_cast< sal_uInt16 >(aColEdit.GetValue() - 1),
            static_cast< sal_uInt16 >(aWidthEdit.Denormalize(aWidthEdit.GetValue(FUNIT_TWIP))));
}




