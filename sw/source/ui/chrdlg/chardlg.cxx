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

#include <editeng/flstitem.hxx>
#include <svl/cjkoptions.hxx>

#include <cmdid.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <chrdlg.hxx>

#include <strings.hrc>
#include <chrdlgmodes.hxx>
#include <sfx2/filedlghelper.hxx>

#include <svx/svxdlg.hxx>
#include <svx/flagsdef.hxx>
#include <svx/dialogs.hrc>

#include <vcl/tabs.hrc>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;

SwCharDlg::SwCharDlg(weld::Window* pParent, SwView& rView, const SfxItemSet& rCoreSet,
    SwCharDlgMode nDialogMode, const OUString* pStr)
    : SfxTabDialogController(pParent, u"modules/swriter/ui/characterproperties.ui"_ustr,
                             u"CharacterPropertiesDialog"_ustr, &rCoreSet, pStr != nullptr)
    , m_rView(rView)
    , m_nDialogMode(nDialogMode)
{
    if (pStr)
    {
        m_xDialog->set_title(m_xDialog->get_title() + SwResId(STR_TEXTCOLL_HEADER) + *pStr + ")");
    }
    AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel), RID_SVXPAGE_CHAR_NAME,
               RID_M + RID_TAB_FONT.sIconName);
    AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel), RID_SVXPAGE_CHAR_EFFECTS,
               RID_M + RID_TAB_FONTEFFECTS.sIconName);
    AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel), RID_SVXPAGE_CHAR_POSITION,
               RID_M + RID_TAB_POSITION.sIconName);
    AddTabPage(u"asianlayout"_ustr, TabResId(RID_TAB_ASIANLAYOUT.aLabel), RID_SVXPAGE_CHAR_TWOLINES,
               RID_M + RID_TAB_ASIANLAYOUT.sIconName);
    AddTabPage(u"background"_ustr, TabResId(RID_TAB_BACKGROUND.aLabel), RID_SVXPAGE_BKG,
               RID_M + RID_TAB_BACKGROUND.sIconName);
    AddTabPage(u"borders"_ustr, TabResId(RID_TAB_BORDER.aLabel), RID_SVXPAGE_BORDER,
               RID_M + RID_TAB_BORDER.sIconName);

    if (m_nDialogMode == SwCharDlgMode::Draw || m_nDialogMode == SwCharDlgMode::Ann)
    {
        RemoveTabPage(u"asianlayout"_ustr);
    }
    else
    {
        if (!SvtCJKOptions::IsDoubleLinesEnabled())
            RemoveTabPage(u"asianlayout"_ustr);
    }

    if (m_nDialogMode != SwCharDlgMode::Std)
        RemoveTabPage(u"borders"_ustr);
}

SwCharDlg::~SwCharDlg()
{
}

// set FontList
void SwCharDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "font")
    {
        SvxFontListItem aFontListItem( *static_cast<const SvxFontListItem*>(
           ( m_rView.GetDocShell()->GetItem( SID_ATTR_CHAR_FONTLIST ) ) ) );
        aSet.Put (SvxFontListItem( aFontListItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        if(m_nDialogMode != SwCharDlgMode::Draw && m_nDialogMode != SwCharDlgMode::Ann)
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fonteffects")
    {
        aSet.Put(
            SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER | SVX_ENABLE_CHAR_TRANSPARENCY));
        rPage.PageCreated(aSet);
    }
    else if (rId == "position")
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (rId == "asianlayout")
    {
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
        rPage.PageCreated(aSet);
    }
    else if (rId == "background")
    {
        SvxBackgroundTabFlags eFlags(SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
        if (m_nDialogMode == SwCharDlgMode::Draw || m_nDialogMode == SwCharDlgMode::Ann)
            eFlags = SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR;
        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(eFlags)));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
