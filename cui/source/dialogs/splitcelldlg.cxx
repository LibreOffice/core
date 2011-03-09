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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
