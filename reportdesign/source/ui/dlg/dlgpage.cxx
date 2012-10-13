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

ORptPageDialog::ORptPageDialog( Window* pParent, const SfxItemSet* pAttr,sal_uInt16 _nPageId) :
SfxTabDialog ( pParent, ModuleRes( _nPageId ), pAttr ),
        rOutAttrs           ( *pAttr )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    switch( _nPageId )
    {
        case RID_PAGEDIALOG_BACKGROUND:
            AddTabPage( RID_SVXPAGE_BACKGROUND,String(ModuleRes(1)));
            break;
        case RID_PAGEDIALOG_PAGE:
            AddTabPage(RID_SVXPAGE_PAGE, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), 0 );
            AddTabPage(RID_SVXPAGE_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
            break;
        case RID_PAGEDIALOG_CHAR:
            AddTabPage(RID_PAGE_CHAR, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
            AddTabPage(RID_PAGE_EFFECTS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
            AddTabPage(RID_PAGE_POSITION, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), 0 );
            AddTabPage(RID_PAGE_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), 0 );
            AddTabPage(RID_PAGE_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
            AddTabPage(RID_PAGE_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), 0 );

            break;
        case RID_PAGEDIALOG_LINE:
            AddTabPage( RID_SVXPAGE_LINE,pFact->GetTabPageCreatorFunc( RID_SVXPAGE_LINE ), 0 );
            break;
        default:
            OSL_FAIL("Unknown page id");
    }

    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage(RID_PAGE_TWOLN);

    FreeResource();
}
// =============================================================================
} // namespace rptui
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
