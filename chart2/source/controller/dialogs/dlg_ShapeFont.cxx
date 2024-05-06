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

#include <dlg_ShapeFont.hxx>
#include <ViewElementListProvider.hxx>

#include <svl/intitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>
#include <editeng/flstitem.hxx>

using namespace ::com::sun::star;

namespace chart
{

ShapeFontDialog::ShapeFontDialog(weld::Window* pParent, const SfxItemSet* pAttr,
    const ViewElementListProvider* pViewElementListProvider)
    : SfxTabDialogController(pParent, u"modules/schart/ui/chardialog.ui"_ustr, u"CharDialog"_ustr, pAttr)
    , m_pViewElementListProvider(pViewElementListProvider)
{
    AddTabPage(u"font"_ustr, RID_SVXPAGE_CHAR_NAME);
    AddTabPage(u"fonteffects"_ustr, RID_SVXPAGE_CHAR_EFFECTS);
    AddTabPage(u"position"_ustr, RID_SVXPAGE_CHAR_POSITION );
}

void ShapeFontDialog::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet( *( GetInputSetImpl()->GetPool() ) );
    if (rId == "font")
    {
        aSet.Put( SvxFontListItem( m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST ) );
        rPage.PageCreated( aSet );
    }
    else if (rId == "fonteffects")
    {
        aSet.Put( SfxUInt16Item( SID_DISABLE_CTL, DISABLE_CASEMAP ) );
        rPage.PageCreated( aSet );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
