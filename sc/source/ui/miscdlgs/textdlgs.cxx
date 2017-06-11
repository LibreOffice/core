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

#include "textdlgs.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include <svl/intitem.hxx>
#include <svx/flagsdef.hxx>

ScCharDlg::ScCharDlg( vcl::Window* pParent, const SfxItemSet* pAttr,
                    const SfxObjectShell* pDocShell )
    : SfxTabDialog(pParent, "CharDialog",
        "modules/scalc/ui/chardialog.ui", pAttr)
    , rDocShell(*pDocShell)
    , m_nNamePageId(0)
    , m_nEffectsPageId(0)
{
    m_nNamePageId = AddTabPage("font", RID_SVXPAGE_CHAR_NAME);
    m_nEffectsPageId = AddTabPage("fonteffects", RID_SVXPAGE_CHAR_EFFECTS);
    AddTabPage("position", RID_SVXPAGE_CHAR_POSITION);
}

void ScCharDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nId == m_nNamePageId)
    {
        SvxFontListItem aItem(*static_cast<const SvxFontListItem*>(
            ( rDocShell.GetItem( SID_ATTR_CHAR_FONTLIST) ) ) );

        aSet.Put (SvxFontListItem( aItem.GetFontList(), SID_ATTR_CHAR_FONTLIST));
        rPage.PageCreated(aSet);
    }
    else if (nId == m_nEffectsPageId)
    {
        aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
        rPage.PageCreated(aSet);
    }
}

ScParagraphDlg::ScParagraphDlg(vcl::Window* pParent, const SfxItemSet* pAttr)
    : SfxTabDialog(pParent, "ParagraphDialog",
        "modules/scalc/ui/paradialog.ui", pAttr)
    , m_nTabPageId(0)
{
    AddTabPage("labelTP_PARA_STD", RID_SVXPAGE_STD_PARAGRAPH);
    AddTabPage("labelTP_PARA_ALIGN", RID_SVXPAGE_ALIGN_PARAGRAPH);
    SvtCJKOptions aCJKOptions;
    if (aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage("labelTP_PARA_ASIAN", RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage("labelTP_PARA_ASIAN");
    m_nTabPageId = AddTabPage("labelTP_TABULATOR", RID_SVXPAGE_TABULATOR);
}

void ScParagraphDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nTabPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        TabulatorDisableFlags const nFlags((TabulatorDisableFlags::TypeMask &~TabulatorDisableFlags::TypeLeft) |
                                           (TabulatorDisableFlags::FillMask &~TabulatorDisableFlags::FillNone));
        aSet.Put(SfxUInt16Item(SID_SVXTABULATORTABPAGE_DISABLEFLAGS, (sal_uInt16)nFlags));
        rPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
