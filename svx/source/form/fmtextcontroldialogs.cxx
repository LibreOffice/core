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

#include "fmtextcontroldialogs.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#include <editeng/eeitem.hxx>

#include "svx/flagsdef.hxx"
#include <svl/intitem.hxx>

#include <svl/cjkoptions.hxx>


namespace svx
{

    TextControlCharAttribDialog::TextControlCharAttribDialog( vcl::Window* pParent, const SfxItemSet& _rCoreSet, const SvxFontListItem& _rFontList )
        : SfxTabDialog( pParent, "TextControlCharacterPropertiesDialog",
            "svx/ui/textcontrolchardialog.ui", &_rCoreSet )
        , m_aFontList(_rFontList)
        , m_nCharNamePageId(0)
        , m_nCharEffectsPageId(0)
        , m_nCharPositionPageId(0)
    {
        m_nCharNamePageId = AddTabPage("font", RID_SVXPAGE_CHAR_NAME);
        m_nCharEffectsPageId = AddTabPage("fonteffects", RID_SVXPAGE_CHAR_EFFECTS);
        m_nCharPositionPageId = AddTabPage("position", RID_SVXPAGE_CHAR_POSITION);
    }

    void TextControlCharAttribDialog::PageCreated( sal_uInt16 _nId, SfxTabPage& _rPage )
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

        if (_nId == m_nCharNamePageId)
        {
            aSet.Put (m_aFontList);
            _rPage.PageCreated(aSet);
        }
        else if (_nId == m_nCharEffectsPageId)
        {
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP));
            _rPage.PageCreated(aSet);
        }
        else if (_nId == m_nCharPositionPageId)
        {
            aSet.Put( SfxUInt32Item(SID_FLAG_TYPE, SVX_PREVIEW_CHARACTER) );
            _rPage.PageCreated(aSet);
        }
    }

    TextControlParaAttribDialog::TextControlParaAttribDialog(vcl::Window* _pParent,
        const SfxItemSet& _rCoreSet)
        : SfxTabDialog( _pParent, "TextControlParagraphPropertiesDialog",
            "svx/ui/textcontrolparadialog.ui", &_rCoreSet )
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
