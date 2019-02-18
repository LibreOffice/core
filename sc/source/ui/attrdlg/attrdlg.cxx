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

#include <sfx2/objsh.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/cjkoptions.hxx>

#include <tabpages.hxx>
#include <attrdlg.hxx>
#include <svx/dialogs.hrc>
#include <editeng/flstitem.hxx>
#include <osl/diagnose.h>

ScAttrDlg::ScAttrDlg(weld::Window* pParent, const SfxItemSet* pCellAttrs)
    : SfxTabDialogController(pParent, "modules/scalc/ui/formatcellsdialog.ui",
                             "FormatCellsDialog", pCellAttrs)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "numbers", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), nullptr );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), nullptr );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "fonteffects", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), nullptr );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "alignment", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ),    nullptr );

    SvtCJKOptions aCJKOptions;
    if (aCJKOptions.IsAsianTypographyEnabled())
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        AddTabPage( "asiantypography",   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       nullptr );
    }
    else
        RemoveTabPage( "asiantypography" );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "borders",      pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ),     nullptr );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "background",  pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    AddTabPage( "cellprotection" ,  ScTabPageProtection::Create,    nullptr );
}

ScAttrDlg::~ScAttrDlg()
{
}

void ScAttrDlg::PageCreated(const OString& rPageId, SfxTabPage& rTabPage)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rPageId == "numbers")
    {
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "font")
    {
        const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
        assert(pInfoItem && "FontListItem  not found :-(");
        aSet.Put (SvxFontListItem(static_cast<const SvxFontListItem*>(pInfoItem)->GetFontList(), SID_ATTR_CHAR_FONTLIST ));
        rTabPage.PageCreated(aSet);
    }
    else if (rPageId == "background")
    {
        rTabPage.PageCreated(aSet);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
