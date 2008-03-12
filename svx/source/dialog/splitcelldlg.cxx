/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: splitcelldlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:41:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#include <sfx2/dispatch.hxx>

#include <svtools/intitem.hxx>
#include <svtools/eitem.hxx>

#include "svx/dialmgr.hxx"
#include "splitcelldlg.hxx"
#include "svx/dialogs.hrc"
#include "splitcelldlg.hrc"

SvxSplitTableDlg::SvxSplitTableDlg( Window *pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal )
: SvxStandardDialog(pParent, SVX_RES(RID_SVX_SPLITCELLDLG))
, maCountLbl(this, SVX_RES(FT_COUNT))
, maCountEdit(this, SVX_RES(ED_COUNT))
, maCountFL(this, SVX_RES(FL_COUNT))
, maHorzBox(this, SVX_RES(RB_HORZ))
, maVertBox(this, SVX_RES(RB_VERT))
, maPropCB(this, SVX_RES(CB_PROP))
, maDirFL(this, SVX_RES(FL_DIR))
, maOKBtn(this, SVX_RES(BT_OK))
, maCancelBtn(this, SVX_RES(BT_CANCEL))
, maHelpBtn( this, SVX_RES( BT_HELP ) )
, mnMaxVertical( nMaxVertical )
, mnMaxHorizontal( nMaxHorizontal )
{
    maVertBox.SetModeRadioImage(Image(SVX_RES(BMP_SPLIT_VERT)), BMP_COLOR_HIGHCONTRAST);
    maHorzBox.SetModeRadioImage(Image(SVX_RES(BMP_SPLIT_HORZ)), BMP_COLOR_HIGHCONTRAST);
    FreeResource();
    maHorzBox.SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));
    maPropCB.SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));
    maVertBox.SetClickHdl( LINK( this, SvxSplitTableDlg, ClickHdl ));

    if( mnMaxVertical < 2 )
        maVertBox.Enable(FALSE);

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
