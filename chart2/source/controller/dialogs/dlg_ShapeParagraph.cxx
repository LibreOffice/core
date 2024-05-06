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

#include <dlg_ShapeParagraph.hxx>

#include <svl/cjkoptions.hxx>
#include <svl/intitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <svx/flagsdef.hxx>

using namespace ::com::sun::star;

namespace chart
{

ShapeParagraphDialog::ShapeParagraphDialog(weld::Window* pParent,
    const SfxItemSet* pAttr)
    : SfxTabDialogController(pParent, u"modules/schart/ui/paradialog.ui"_ustr, u"ParagraphDialog"_ustr, pAttr)
{
    AddTabPage(u"labelTP_PARA_STD"_ustr,  RID_SVXPAGE_STD_PARAGRAPH);
    AddTabPage(u"labelTP_PARA_ALIGN"_ustr, RID_SVXPAGE_ALIGN_PARAGRAPH );
    if (SvtCJKOptions::IsAsianTypographyEnabled())
    {
        AddTabPage(u"labelTP_PARA_ASIAN"_ustr, RID_SVXPAGE_PARA_ASIAN);
    }
    else
    {
        RemoveTabPage(u"labelTP_PARA_ASIAN"_ustr);
    }
    AddTabPage(u"labelTP_TABULATOR"_ustr, RID_SVXPAGE_TABULATOR);
}

void ShapeParagraphDialog::PageCreated(const OUString& rId, SfxTabPage& rPage)
{
    if (rId == "labelTP_TABULATOR")
    {
        SfxAllItemSet aSet( *( GetInputSetImpl()->GetPool() ) );
        TabulatorDisableFlags const nFlags(( TabulatorDisableFlags::TypeMask &~TabulatorDisableFlags::TypeLeft ) |
                                     ( TabulatorDisableFlags::FillMask &~TabulatorDisableFlags::FillNone ));
        aSet.Put( SfxUInt16Item( SID_SVXTABULATORTABPAGE_DISABLEFLAGS, static_cast<sal_uInt16>(nFlags)) );
        rPage.PageCreated( aSet );
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
