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
#include "precompiled_cui.hxx"

#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "dialmgr.hxx"
#include "splitcelldlg.hxx"
#include "cuires.hrc"
#include "splitcelldlg.hrc"

SvxSplitTableDlg::SvxSplitTableDlg( Window *pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal )
: SvxStandardDialog(pParent, CUI_RES(RID_SVX_SPLITCELLDLG))
, maCountFL(this, CUI_RES(FL_COUNT))
, maCountLbl(this, CUI_RES(FT_COUNT))
, maCountEdit(this, CUI_RES(ED_COUNT))
, maDirFL(this, CUI_RES(FL_DIR))
, maHorzBox(this, CUI_RES(RB_HORZ))
, maVertBox(this, CUI_RES(RB_VERT))
, maPropCB(this, CUI_RES(CB_PROP))
, maOKBtn(this, CUI_RES(BT_OK))
, maCancelBtn(this, CUI_RES(BT_CANCEL))
, maHelpBtn( this, CUI_RES( BT_HELP ) )
, mnMaxVertical( nMaxVertical )
, mnMaxHorizontal( nMaxHorizontal )
{
    maVertBox.SetModeRadioImage(Image(CUI_RES(BMP_SPLIT_VERT)), BMP_COLOR_HIGHCONTRAST);
    maHorzBox.SetModeRadioImage(Image(CUI_RES(BMP_SPLIT_HORZ)), BMP_COLOR_HIGHCONTRAST);
    FreeResource();
    maHorzBox.SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));
    maPropCB.SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));
    maVertBox.SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));

    if( mnMaxVertical < 2 )
        maVertBox.Enable(sal_False);

    //exchange the meaning of horizontal and vertical for vertical text
    if(bIsTableVertical)
    {
        Image aTmpImg(maHorzBox.GetModeRadioImage());
        String sTmp(maHorzBox.GetText());
        maHorzBox.SetText(maVertBox.GetText());
        maHorzBox.SetModeRadioImage(maVertBox.GetModeRadioImage());
        maVertBox.SetText(sTmp);
        maVertBox.SetModeRadioImage(aTmpImg);
    }
}

SvxSplitTableDlg::~SvxSplitTableDlg()
{
}

IMPL_LINK( SvxSplitTableDlg, ClickHdl, Button *, pButton )
{
    const bool bIsVert =  pButton == &maVertBox ;
    long nMax = bIsVert ? mnMaxVertical : mnMaxHorizontal;
    maPropCB.Enable(!bIsVert);
    maCountEdit.SetMax( nMax );
    return 0;
}

bool SvxSplitTableDlg::IsHorizontal() const
{
    return maHorzBox.IsChecked();
}

bool SvxSplitTableDlg::IsProportional() const
{
    return maPropCB.IsChecked() && maHorzBox.IsChecked();
}

long SvxSplitTableDlg::GetCount() const
{
    return sal::static_int_cast<long>( maCountEdit.GetValue() );
}

short SvxSplitTableDlg::Execute()
{
    return SvxStandardDialog::Execute();
}

void SvxSplitTableDlg::Apply()
{
}
