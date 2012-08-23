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

#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/itemset.hxx>
#include <svx/flagsdef.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/intitem.hxx>

#include "swtypes.hxx"
#include "uiborder.hxx"
#include "frmui.hrc"




SwBorderDlg::SwBorderDlg(Window* pParent, SfxItemSet& rSet, sal_uInt16 nType) :

    SfxSingleTabDialog(pParent, rSet, 0)

{
    SetText(SW_RESSTR(STR_FRMUI_BORDER));

    // create TabPage
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER );
    if ( fnCreatePage )
    {
        SfxTabPage* pNewPage = (*fnCreatePage)( this, rSet );
        SfxAllItemSet aSet(*(rSet.GetPool()));
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE,nType));
        if(SW_BORDER_MODE_TABLE == nType)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_HIDESHADOWCTL));
        pNewPage->PageCreated(aSet);
        SetTabPage(pNewPage);
    }
}



SwBorderDlg::~SwBorderDlg()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
