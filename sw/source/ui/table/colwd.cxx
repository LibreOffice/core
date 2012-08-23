/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sfx2/dispatch.hxx>
#include <svx/dlgutil.hxx>
#include <colwd.hxx>
#include <tablemgr.hxx>
#include <wrtsh.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <usrpref.hxx>

#include <cmdid.h>
#include <colwd.hrc>
#include <table.hrc>


IMPL_LINK_NOARG_INLINE_START(SwTableWidthDlg, LoseFocusHdl)
{
    sal_uInt16 nId = (sal_uInt16)aColEdit.GetValue()-1;
    const SwTwips lWidth = rFnc.GetColWidth(nId);
    aWidthEdit.SetValue(aWidthEdit.Normalize(lWidth), FUNIT_TWIP);
    aWidthEdit.SetMax(aWidthEdit.Normalize(rFnc.GetMaxColWidth(nId)), FUNIT_TWIP);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwTableWidthDlg, LoseFocusHdl)



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




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
