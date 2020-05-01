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
        rDialog.toAsciiLowerCase() + ".ui", rDialog.toUtf8(), pAttr)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();

    if (rDialog == "BackgroundDialog")
    {
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    }
    else if (rDialog == "PageDialog")
    {
        AddTabPage("page", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), nullptr );
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
    }
    else if (rDialog == "CharDialog")
    {
        AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), nullptr );
        AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), nullptr );
        AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), nullptr );
        AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), nullptr );
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), nullptr );
        AddTabPage("alignment", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), nullptr );
    }
    else
        OSL_FAIL("Unknown page id");

    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage("asianlayout");
}

void ORptPageDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    if (rId == "background")
    {
        rPage.PageCreated(aSet);
    }
}

} // namespace rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
