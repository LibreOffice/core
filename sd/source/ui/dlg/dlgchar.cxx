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
#include <editeng/flstitem.hxx>
#include <svx/flagsdef.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxdlg.hxx>

#include <dlg_char.hxx>
#include <svx/svxids.hrc>
#include <svl/intitem.hxx>

#include <vcl/tabs.hrc>

/**
 * Constructor of tab dialog: append pages to dialog
 */
SdCharDlg::SdCharDlg(weld::Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell)
    : SfxTabDialogController(pParent, u"modules/sdraw/ui/drawchardialog.ui"_ustr,
                             u"DrawCharDialog"_ustr, pAttr)
    , rDocShell(*pDocShell)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    AddTabPage(u"font"_ustr, TabResId(RID_TAB_FONT.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_NAME), RID_L + RID_TAB_FONT.sIconName);
    AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_FONTEFFECTS.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_EFFECTS),
               RID_L + RID_TAB_FONTEFFECTS.sIconName);
    AddTabPage(u"position"_ustr, TabResId(RID_TAB_POSITION.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_CHAR_POSITION),
               RID_L + RID_TAB_POSITION.sIconName);
    AddTabPage(u"background"_ustr, TabResId(RID_TAB_HIGHLIGHTING.aLabel),
               pFact->GetTabPageCreatorFunc(RID_SVXPAGE_BKG),
               RID_L + RID_TAB_HIGHLIGHTING.sIconName);
}

void SdCharDlg::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "font")
    {
        SvxFontListItem aItem(* static_cast<const SvxFontListItem*>( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (rId == "fonteffects")
    {
        // Opt in for character transparency.
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE, SVX_ENABLE_CHAR_TRANSPARENCY));
        rPage.PageCreated(aSet);
    }
    else if (rId == "background")
    {
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR)));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
