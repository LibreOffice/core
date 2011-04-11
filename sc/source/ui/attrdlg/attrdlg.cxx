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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



#include "scitems.hxx"

#include <sfx2/objsh.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/cjkoptions.hxx>

#include "tabpages.hxx"
#include "attrdlg.hxx"
#include "scresid.hxx"
#include "attrdlg.hrc"
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/app.hxx>

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-pre.hxx>
#endif

//==================================================================

ScAttrDlg::ScAttrDlg( SfxViewFrame*     pFrameP,
                      Window*           pParent,
                      const SfxItemSet* pCellAttrs )

    :   SfxTabDialog( pFrameP,
                      pParent,
                      ScResId( RID_SCDLG_ATTR ),
                      pCellAttrs )
{
    SvtCJKOptions aCJKOptions;
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");

    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");
#if LAYOUT_SFX_TABDIALOG_BROKEN
    AddTabPage( TP_NUMBER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), 0 );
#else
    String number(RTL_CONSTASCII_USTRINGPARAM("Numbers"));
    AddTabPage( TP_NUMBER, number, pFact->GetTabPageCreatorFunc (RID_SVXPAGE_NUMBERFORMAT), 0, false, TAB_APPEND);
#endif
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
    AddTabPage( TP_FONT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
    AddTabPage( TP_FONTEFF, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");
    AddTabPage( TP_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ),    0 );

    if ( aCJKOptions.IsAsianTypographyEnabled() )
    {
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
        AddTabPage( TP_ASIAN,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       0 );
    }
    else
        RemoveTabPage( TP_ASIAN );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
    AddTabPage( TP_BORDER,      pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ),     0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
    AddTabPage( TP_BACKGROUND,  pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 );
    AddTabPage( TP_PROTECTION,  ScTabPageProtection::Create,    0 );
    FreeResource();
}

// -----------------------------------------------------------------------

ScAttrDlg::~ScAttrDlg()
{
}

// -----------------------------------------------------------------------

void ScAttrDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch ( nPageId )
    {
        case TP_NUMBER:
        {
            aSet.Put (SfxLinkItem( SID_LINK_TYPE, LINK( this, ScAttrDlg, OkHandler )));
            rTabPage.PageCreated(aSet);
        }
        break;

        case TP_FONT:
        {
            const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            DBG_ASSERT( pInfoItem, "FontListItem  not found :-(" );

            aSet.Put (SvxFontListItem(((const SvxFontListItem*)pInfoItem)->GetFontList(), SID_ATTR_CHAR_FONTLIST ));
            rTabPage.PageCreated(aSet);
        }
        break;

        default:
        break;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ScAttrDlg, OkHandler, void*, EMPTYARG )
{
    ((Link&)GetOKButton().GetClickHdl()).Call( NULL );

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
