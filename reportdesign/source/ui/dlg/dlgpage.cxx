/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"


#include <svx/dialogs.hrc>
#include <svx/tabarea.hxx>
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

ORptPageDialog::ORptPageDialog( Window* pParent, const SfxItemSet* pAttr,USHORT _nPageId) :
SfxTabDialog ( pParent, ModuleRes( _nPageId ), pAttr ),
        rOutAttrs           ( *pAttr )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
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
            OSL_ENSURE(0,"Unknown page id");
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
