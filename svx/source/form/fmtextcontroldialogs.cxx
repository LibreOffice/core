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

#include <fmtextcontroldialogs.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>

#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>

#include <svl/cjkoptions.hxx>
#include <utility>

#include <vcl/tabs.hrc>
namespace svx
{

    TextControlCharAttribDialog::TextControlCharAttribDialog(weld::Window* pParent, const SfxItemSet& rCoreSet, SvxFontListItem aFontList)
        : SfxTabDialogController(pParent, u"svx/ui/textcontrolchardialog.ui"_ustr, u"TextControlCharacterPropertiesDialog"_ustr, &rCoreSet)
        , m_aFontList(std::move(aFontList))
    {
        AddTabPage(u"font"_ustr, TabResId(RID_TAB_SVXTEXT_FONT), RID_SVXPAGE_CHAR_NAME,
                   RID_L + RID_TAB_FONT_IMG);
        AddTabPage(u"fonteffects"_ustr, TabResId(RID_TAB_SVXTEXT_FONTEFFECTS),
                   RID_SVXPAGE_CHAR_EFFECTS, RID_L + RID_TAB_FONTEFFECTS_IMG);
        AddTabPage(u"position"_ustr, TabResId(RID_TAB_SVXTEXT_POSITION), RID_SVXPAGE_CHAR_POSITION,
                   RID_L + RID_TAB_POSITION_IMG);
    }

    void TextControlCharAttribDialog::PageCreated(const OUString& rId, SfxTabPage& rPage)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

        if (rId == "font")
        {
            aSet.Put (m_aFontList);
            rPage.PageCreated(aSet);
        }
        else if (rId == "fonteffects")
        {
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
            rPage.PageCreated(aSet);
        }
        else if (rId == "position")
        {
            aSet.Put( SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER) );
            rPage.PageCreated(aSet);
        }
    }

    TextControlParaAttribDialog::TextControlParaAttribDialog(weld::Window* pParent, const SfxItemSet& rCoreSet)
        : SfxTabDialogController(pParent, u"svx/ui/textcontrolparadialog.ui"_ustr, u"TextControlParagraphPropertiesDialog"_ustr, &rCoreSet)
    {
        AddTabPage(u"indents"_ustr, TabResId(RID_TAB_SVXTEXT_INDENTS), RID_SVXPAGE_STD_PARAGRAPH,
                   RID_L + RID_TAB_INDENTS_IMG);
        AddTabPage(u"alignment"_ustr, TabResId(RID_TAB_SVXTEXT_ALIGNMENT),
                   RID_SVXPAGE_ALIGN_PARAGRAPH, RID_L + RID_TAB_ALIGNMENT_IMG);

        if (SvtCJKOptions::IsAsianTypographyEnabled())
            AddTabPage(u"asian"_ustr, TabResId(RID_TAB_SVXTEXT_ASIANTYPO), RID_SVXPAGE_PARA_ASIAN,
                       RID_L + RID_TAB_ASIANTYPO_IMG);

        AddTabPage(u"tabs"_ustr, TabResId(RID_TAB_SVXTEXT_TABS), RID_SVXPAGE_TABULATOR,
                   RID_L + RID_TAB_TABS_IMG);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
