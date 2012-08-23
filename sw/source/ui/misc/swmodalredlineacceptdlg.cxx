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

#include <redline.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/ctredlin.hxx>
#include <svx/postattr.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <swwait.hxx>
#include <uitool.hxx>

#include <helpid.h>
#include <cmdid.h>
#include <misc.hrc>
#include <redlndlg.hrc>
#include <shells.hrc>

#include <vector>
#include <redlndlg.hxx>
#include "swmodalredlineacceptdlg.hxx"

#include <unomid.h>

SwModalRedlineAcceptDlg::SwModalRedlineAcceptDlg(Window *pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_MOD_REDLINE_ACCEPT))
{
    pImplDlg = new SwRedlineAcceptDlg(this, sal_True);

    pImplDlg->Initialize(GetExtraData());
    pImplDlg->Activate();   // for data's initialisation

    FreeResource();
}

SwModalRedlineAcceptDlg::~SwModalRedlineAcceptDlg()
{
    AcceptAll(sal_False);   // refuse everything remaining
    pImplDlg->FillInfo(GetExtraData());

    delete pImplDlg;
}

void SwModalRedlineAcceptDlg::Activate()
{
}

void SwModalRedlineAcceptDlg::Resize()
{
    pImplDlg->Resize();
    SfxModalDialog::Resize();
}

void SwModalRedlineAcceptDlg::AcceptAll( sal_Bool bAccept )
{
    SvxTPFilter* pFilterTP = pImplDlg->GetChgCtrl()->GetFilterPage();

    if (pFilterTP->IsDate() || pFilterTP->IsAuthor() ||
        pFilterTP->IsRange() || pFilterTP->IsAction())
    {
        pFilterTP->CheckDate(sal_False);    // turn off all filters
        pFilterTP->CheckAuthor(sal_False);
        pFilterTP->CheckRange(sal_False);
        pFilterTP->CheckAction(sal_False);
        pImplDlg->FilterChangedHdl();
    }

    pImplDlg->CallAcceptReject( sal_False, bAccept );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
