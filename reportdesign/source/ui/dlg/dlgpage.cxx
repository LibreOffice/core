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
#include <svx/flagsdef.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/svxenum.hxx>
#include "dlgpage.hxx"
#include "ModuleHelper.hxx"
#include "RptResId.hrc"
#include <svl/intitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/aeitem.hxx>

namespace rptui
{
/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

ORptPageDialog::ORptPageDialog( vcl::Window* pParent, const SfxItemSet* pAttr, const OUString &rDialog)
    : SfxTabDialog (pParent, rDialog, "modules/dbreport/ui/" +
        rDialog.toAsciiLowerCase() +
        ".ui", pAttr)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    assert(pFact && "Dialog creation failed!");

    if (rDialog == "BackgroundDialog")
    {
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    }
    else if (rDialog == "PageDialog")
    {
        AddTabPage("page", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), 0 );
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    }
    else if (rDialog == "CharDialog")
    {
        AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
        AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
        AddTabPage("position", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
        AddTabPage("asianlayout", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
        AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
        AddTabPage("alignment", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), 0 );
    }
    else
        OSL_FAIL("Unknown page id");

    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage("asianlayout");
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
