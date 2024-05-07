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

#include <dlgattr.hxx>

#include <sfx2/tabdlg.hxx>

#include <svx/numinf.hxx>

#include <svx/dialogs.hrc>
#include <svl/itemset.hxx>
#include <svx/svxids.hrc>

using namespace dbaui;


SbaSbAttrDlg::SbaSbAttrDlg(weld::Widget* pParent, const SfxItemSet* pCellAttrs,
    SvNumberFormatter* pFormatter, bool bHasFormat)
    : SfxTabDialogController(pParent, u"dbaccess/ui/fielddialog.ui"_ustr, u"FieldDialog"_ustr, pCellAttrs)
{
    pNumberInfoItem.reset( new SvxNumberInfoItem( pFormatter, SID_ATTR_NUMBERFORMAT_INFO ) );

    if (bHasFormat)
        AddTabPage(u"format"_ustr, RID_SVXPAGE_NUMBERFORMAT);
    else
        RemoveTabPage(u"format"_ustr);
    AddTabPage(u"alignment"_ustr, RID_SVXPAGE_ALIGNMENT);
}

SbaSbAttrDlg::~SbaSbAttrDlg()
{
}

void SbaSbAttrDlg::PageCreated(const OUString& rPageId, SfxTabPage& rTabPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rPageId == "format")
    {
        aSet.Put (SvxNumberInfoItem( pNumberInfoItem->GetNumberFormatter(), SID_ATTR_NUMBERFORMAT_INFO));
        rTabPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
