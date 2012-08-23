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

#include "swtypes.hxx"

#include "multmrk.hxx"
#include "toxmgr.hxx"

#include "index.hrc"
#include "multmrk.hrc"


SwMultiTOXMarkDlg::SwMultiTOXMarkDlg( Window* pParent, SwTOXMgr& rTOXMgr ) :

    SvxStandardDialog(pParent, SW_RES(DLG_MULTMRK)),

    aTOXFL(this,    SW_RES(FL_TOX)),
    aEntryFT(this,  SW_RES(FT_ENTRY)),
    aTextFT(this,   SW_RES(FT_TEXT)),
    aTOXFT(this,    SW_RES(FT_TOX)),
    aTOXLB(this,    SW_RES(LB_TOX)),
    aOkBT(this,     SW_RES(OK_BT)),
    aCancelBT(this, SW_RES(CANCEL_BT)),
    rMgr( rTOXMgr ),
    nPos(0)
{
    aTOXLB.SetSelectHdl(LINK(this, SwMultiTOXMarkDlg, SelectHdl));

    sal_uInt16 nSize = rMgr.GetTOXMarkCount();
    for(sal_uInt16 i=0; i < nSize; ++i)
        aTOXLB.InsertEntry(rMgr.GetTOXMark(i)->GetText());

    aTOXLB.SelectEntryPos(0);
    aTextFT.SetText(rMgr.GetTOXMark(0)->GetTOXType()->GetTypeName());

    FreeResource();
}


IMPL_LINK_INLINE_START( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )
{
    if(pBox->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {   SwTOXMark* pMark = rMgr.GetTOXMark(pBox->GetSelectEntryPos());
        aTextFT.SetText(pMark->GetTOXType()->GetTypeName());
        nPos = pBox->GetSelectEntryPos();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SwMultiTOXMarkDlg, SelectHdl, ListBox *, pBox )


void SwMultiTOXMarkDlg::Apply()
{
    rMgr.SetCurTOXMark(nPos);
}

/*--------------------------------------------------
 overload dtor
--------------------------------------------------*/


SwMultiTOXMarkDlg::~SwMultiTOXMarkDlg() {}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
