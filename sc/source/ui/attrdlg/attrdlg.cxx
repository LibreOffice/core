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

#undef SC_DLLIMPLEMENTATION

#include <sfx2/objsh.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/cjkoptions.hxx>

#include <tabpages.hxx>
#include <attrdlg.hxx>
#include <svx/dialogs.hrc>
#include <editeng/editids.hrc>
#include <editeng/flstitem.hxx>
#include <osl/diagnose.h>

#include <vcl/tabs.hrc>

ScAttrDlg::ScAttrDlg(weld::Window* pParent, const SfxItemSet* pCellAttrs)
    : SfxTabDialogController(pParent, u"modules/scalc/ui/formatcellsdialog.ui"_ustr,
                             u"FormatCellsDialog"_ustr, pCellAttrs)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_NUMBERFORMAT), "GetTabPageCreatorFunc fail!");
    AddTabPage(u"numbers"_ustr, TabResId(RID_TAB_NUMBERS.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_NUMBERFORMAT),
               RID_M + RID_TAB_NUMBERS.sIconName);
    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), "GetTabPageCreatorFunc fail!");
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME),
               RID_M + RID_TAB_FONT.sIconName);
    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS), "GetTabPageCreatorFunc fail!");
    AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS),
               RID_M + RID_TAB_FONTEFFECTS.sIconName);
    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGNMENT), "GetTabPageCreatorFunc fail!");
    AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_ALIGNMENT.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGNMENT),
               RID_M + RID_TAB_ALIGNMENT.sIconName);

    if (SvtCJKOptions::IsAsianTypographyEnabled())
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        AddTabPage(u"asiantypography"_ustr, TabResId(RID_TAB_ASIANTYPO.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),
                   RID_M + RID_TAB_ASIANTYPO.sIconName);
    }
    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BORDER), "GetTabPageCreatorFunc fail!");
    AddTabPage(u"borders"_ustr, TabResId(RID_TAB_BORDER.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BORDER), RID_M + RID_TAB_BORDER.sIconName);
    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), "GetTabPageCreatorFunc fail!");
    AddTabPage(u"background"_ustr, TabResId(RID_TAB_BACKGROUND.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG), RID_M + RID_TAB_BACKGROUND.sIconName);
    AddTabPage(u"cellprotection"_ustr, TabResId(RID_TAB_PROTECTION.aLabel),
               ScTabPageProtection::Create, RID_M + RID_TAB_PROTECTION.sIconName);
}

ScAttrDlg::~ScAttrDlg()
{
}

void ScAttrDlg::PageCreated(const OUString& rPageId, SfxTabPage& rTabPage)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rPageId == "numbers")
    {
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "font" && pDocSh)
    {
        const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
        SAL_WARN_IF(!pInfoItem, "sc.ui", "we should have a FontListItem normally here");
        if (pInfoItem)
        {
            aSet.Put (SvxFontListItem(static_cast<const SvxFontListItem*>(pInfoItem)->GetFontList(), SID_ATTR_CHAR_FONTLIST ));
            rTabPage.PageCreated(aSet);
        }
    }
    else if (rPageId == "background")
    {
        rTabPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
