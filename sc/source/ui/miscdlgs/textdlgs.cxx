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

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#include <editeng/flstitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/cjkoptions.hxx>

#include <textdlgs.hxx>
#include <svl/intitem.hxx>
#include <svx/flagsdef.hxx>

#include <vcl/tabs.hrc>

ScCharDlg::ScCharDlg(weld::Window* pParent, const SfxItemSet* pAttr, const SfxObjectShell* pDocShell, bool bDrawText)
    : SfxTabDialogController(pParent, u"modules/scalc/ui/chardialog.ui"_ustr, u"CharDialog"_ustr, pAttr)
    , m_rDocShell(*pDocShell)
    , m_bDrawText(bDrawText)
{
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel), RID_SVXPAGE_CHAR_NAME,
               RID_L + RID_TAB_FONT.sIconName);
    AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel), RID_SVXPAGE_CHAR_EFFECTS,
               RID_L + RID_TAB_FONTEFFECTS.sIconName);
    AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel), RID_SVXPAGE_CHAR_POSITION,
               RID_L + RID_TAB_POSITION.sIconName);

    if (bDrawText)
        AddTabPage(u"background"_ustr, TabResId(RID_TAB_HIGHLIGHTING.aLabel), RID_SVXPAGE_BKG,
                   RID_L + RID_TAB_HIGHLIGHTING.sIconName);
}

void ScCharDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "font")
    {
        SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(
            ( m_rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fonteffects")
    {
        // Allow CaseMap in drawings, but not in normal text
        if (!m_bDrawText)
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
        rPage.PageCreated(aSet);
    }
    else if (rId == "background")
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
}

ScParagraphDlg::ScParagraphDlg(weld::Window* pParent, const SfxItemSet* pAttr)
    : SfxTabDialogController(pParent, u"modules/scalc/ui/paradialog.ui"_ustr, u"ParagraphDialog"_ustr, pAttr)
{
    AddTabPage(u"indents"_ustr, TabResId(RID_TAB_INDENTS.aLabel), RID_SVXPAGE_STD_PARAGRAPH,
               RID_L + RID_TAB_INDENTS.sIconName);
    AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_ALIGNMENT.aLabel), RID_SVXPAGE_ALIGN_PARAGRAPH,
               RID_L + RID_TAB_ALIGNMENT.sIconName);
    if (SvtCJKOptions::IsAsianTypographyEnabled())
        AddTabPage(u"asiantypo"_ustr, TabResId(RID_TAB_ASIANTYPO.aLabel), RID_SVXPAGE_PARA_ASIAN,
                   RID_L + RID_TAB_ASIANTYPO.sIconName);
    AddTabPage(u"tabs"_ustr, TabResId(RID_TAB_TABS.aLabel), RID_SVXPAGE_TABULATOR,
               RID_L + RID_TAB_TABS.sIconName);
}

void ScParagraphDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (rId == "tabs")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        TabulatorDisableFlags const nFlags((TabulatorDisableFlags::TypeMask &~TabulatorDisableFlags::TypeLeft) |
                                           (TabulatorDisableFlags::FillMask &~TabulatorDisableFlags::FillNone));
        aSet.Put(SfxUInt16Item(SID_SVXTABULATORTABPAGE_DISABLEFLAGS, static_cast<sal_uInt16>(nFlags)));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
