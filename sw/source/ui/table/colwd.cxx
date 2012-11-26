/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    bool bIsWeb = rTableFnc.GetShell()
                    ? dynamic_cast< SwWebDocShell* >( rTableFnc.GetShell()->GetView().GetDocShell())
                    : false;
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




