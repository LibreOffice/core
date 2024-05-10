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
#include <sfx2/sfxdlg.hxx>
#include <dlgpage.hxx>
#include <svl/cjkoptions.hxx>
#include <osl/diagnose.h>

namespace rptui
{
/*************************************************************************
|*
|* constructor of the tab dialogs: Add the page to the dialog
|*
\************************************************************************/

ORptPageDialog::ORptPageDialog(weld::Window* pParent, const SfxItemSet* pAttr, const OUString &rDialog)
    : SfxTabDialogController(pParent, "modules/dbreport/ui/" +
        rDialog.toAsciiLowerCase() + ".ui", rDialog, pAttr)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    if (rDialog == "BackgroundDialog")
    {
        AddTabPage(u"background"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    }
    else if (rDialog == "PageDialog")
    {
        AddTabPage(u"page"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), nullptr );
        AddTabPage(u"background"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    }
    else if (rDialog == "CharDialog")
    {
        AddTabPage(u"font"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), nullptr );
        AddTabPage(u"fonteffects"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), nullptr );
        AddTabPage(u"position"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), nullptr );
        AddTabPage(u"asianlayout"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), nullptr );
        AddTabPage(u"background"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
        AddTabPage(u"alignment"_ustr, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), nullptr );
    }
    else
        OSL_FAIL("Unknown page id");

    if ( !SvtCJKOptions::IsDoubleLinesEnabled() )
        RemoveTabPage(u"asianlayout"_ustr);
}

void ORptPageDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "background")
    {
        rPage.PageCreated(aSet);
    }
}

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
