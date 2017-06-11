/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include "strings.hrc"

SwBorderDlg::SwBorderDlg(vcl::Window* pParent, SfxItemSet& rSet, SwBorderModes nType) :
    SfxSingleTabDialog(pParent, rSet)

{
    SetText(SwResId(STR_FRMUI_BORDER));

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialog creation failed!");
    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER );

    if ( fnCreatePage )
    {
        VclPtr<SfxTabPage> pNewPage = (*fnCreatePage)( get_content_area(), &rSet );
        SfxAllItemSet aSet(*(rSet.GetPool()));
        aSet.Put (SfxUInt16Item(SID_SWMODE_TYPE, static_cast<sal_uInt16>(nType)));
        if(SwBorderModes::TABLE == nType)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_HIDESHADOWCTL));
        pNewPage->PageCreated(aSet);
        SetTabPage(pNewPage);
    }
}

SwBorderDlg::~SwBorderDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
