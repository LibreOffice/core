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

#include "scitems.hxx"

#include <sfx2/objsh.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/cjkoptions.hxx>

#include "tabpages.hxx"
#include "attrdlg.hxx"
#include "scresid.hxx"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/app.hxx>

ScAttrDlg::ScAttrDlg(SfxViewFrame* pFrameP, Window* pParent, const SfxItemSet* pCellAttrs)
    : SfxTabDialog(pFrameP, pParent, "FormatCellsDialog", "modules/scalc/ui/formatcellsdialog.ui", pCellAttrs)
{
    SvtCJKOptions aCJKOptions;
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");

    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");
    m_nNumberPageId = AddTabPage( "numbers", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), 0 );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
    m_nFontPageId = AddTabPage( "font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "fonteffects", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "alignment", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ),    0 );

    if ( aCJKOptions.IsAsianTypographyEnabled() )
    {
        OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        AddTabPage( "asiantypography",   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       0 );
    }
    else
        RemoveTabPage( "asiantypography" );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "borders",      pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ),     0 );
    OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
    AddTabPage( "background",  pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage( "cellprotection" ,  ScTabPageProtection::Create,    0 );
}

ScAttrDlg::~ScAttrDlg()
{
}

void ScAttrDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (nPageId == m_nNumberPageId)
    {
        aSet.Put (SfxLinkItem( SID_LINK_TYPE, LINK( this, ScAttrDlg, OkHandler )));
        rTabPage.PageCreated(aSet);
    }
    else if (nPageId == m_nFontPageId)
    {
        const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

        OSL_ENSURE( pInfoItem, "FontListItem  not found :-(" );

        aSet.Put (SvxFontListItem(((const SvxFontListItem*)pInfoItem)->GetFontList(), SID_ATTR_CHAR_FONTLIST ));
        rTabPage.PageCreated(aSet);
    }
}

IMPL_LINK_NOARG(ScAttrDlg, OkHandler)
{
    ((Link&)GetOKButton().GetClickHdl()).Call( NULL );

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
