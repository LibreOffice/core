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


namespace svx
{

    TextControlCharAttribDialog::TextControlCharAttribDialog(weld::Window* pParent, const SfxItemSet& rCoreSet, const SvxFontListItem& rFontList)
        : SfxTabDialogController(pParent, "svx/ui/textcontrolchardialog.ui", "TextControlCharacterPropertiesDialog", &rCoreSet)
        , m_aFontList(rFontList)
    {
        AddTabPage("font", RID_SVXPAGE_CHAR_NAME);
        AddTabPage("fonteffects", RID_SVXPAGE_CHAR_EFFECTS);
        AddTabPage("position", RID_SVXPAGE_CHAR_POSITION);
    }

    void TextControlCharAttribDialog::PageCreated(const OString& rId, SfxTabPage& rPage)
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
        : SfxTabDialogController(pParent, "svx/ui/textcontrolparadialog.ui", "TextControlParagraphPropertiesDialog", &rCoreSet)
    {
        AddTabPage("labelTP_PARA_STD", RID_SVXPAGE_STD_PARAGRAPH);
        AddTabPage("labelTP_PARA_ALIGN", RID_SVXPAGE_ALIGN_PARAGRAPH);

        SvtCJKOptions aCJKOptions;
        if( aCJKOptions.IsAsianTypographyEnabled() )
            AddTabPage("labelTP_PARA_ASIAN", RID_SVXPAGE_PARA_ASIAN);
        else
            RemoveTabPage("labelTP_PARA_ASIAN");

        AddTabPage("labelTP_TABULATOR", RID_SVXPAGE_TABULATOR);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
