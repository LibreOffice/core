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


#include <svx/dialogs.hrc>
#include <sfx2/sfxdlg.hxx>
#include <dlgpage.hxx>
#include <svl/cjkoptions.hxx>
#include <osl/diagnose.h>

#include <vcl/tabs.hrc>

namespace rptui
{
/*************************************************************************
|*
|* constructor of the tab dialogs: Add the page to the dialog
|*
\************************************************************************/

ORptPageDialog::ORptPageDialog(weld::Window* pParent, const SfxItemSet* pAttr, const OUString &rDialog)
    : SfxTabDialogController(pParent, "modules/dbreport/ui/" +
        rDialog.toAsciiLowerCase() + ".ui", rDialog, pAttr)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    if (rDialog == "BackgroundDialog")
    {
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_BACKGROUND.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG),
                   RID_L + RID_TAB_BACKGROUND.sIconName);
    }
    else if (rDialog == "PageDialog")
    {
        AddTabPage(u"page"_ustr, TabResId(RID_TAB_PAGE.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PAGE), RID_L + RID_TAB_PAGE.sIconName);
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_BACKGROUND.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG),
                   RID_L + RID_TAB_BACKGROUND.sIconName);
    }
    else if (rDialog == "CharDialog")
    {
        AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME),
                   RID_L + RID_TAB_FONT.sIconName);
        AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS),
                   RID_L + RID_TAB_FONTEFFECTS.sIconName);
        AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_POSITION),
                   RID_L + RID_TAB_POSITION.sIconName);
        if (SvtCJKOptions::IsDoubleLinesEnabled())
            AddTabPage(u"asianlayout"_ustr, TabResId(RID_TAB_ASIANLAYOUT.aLabel),
                       pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_TWOLINES),
                       RID_L + RID_TAB_ASIANLAYOUT.sIconName);
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_BACKGROUND.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG),
                   RID_L + RID_TAB_BACKGROUND.sIconName);
        AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_ALIGNMENT.aLabel),
                   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGNMENT),
                   RID_L + RID_TAB_ALIGNMENT.sIconName);
    }
    else
        OSL_FAIL("Unknown page id");
}

void ORptPageDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "background")
    {
        rPage.PageCreated(aSet);
    }
}

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
