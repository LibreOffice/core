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
//CHINA001 #include <svx/align.hxx>
//CHINA001 #include <svx/backgrnd.hxx>
//CHINA001 #include <svx/border.hxx>
//CHINA001 #include <svx/chardlg.hxx>
//CHINA001 #include <svx/numfmt.hxx>
//CHINA001 #include <svx/paragrph.hxx>
#include <svl/cjkoptions.hxx>

#include "tabpages.hxx"
#include "attrdlg.hxx"
#include "scresid.hxx"
#include "attrdlg.hrc"
#include <svx/svxdlg.hxx> //CHINA001
#include <svx/dialogs.hrc> //CHINA001
#include <svx/flagsdef.hxx> //CHINA001
#include <editeng/flstitem.hxx> //CHINA001
#include <sfx2/app.hxx> //CHINA001

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
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001

    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");//CHINA001
#if LAYOUT_SFX_TABDIALOG_BROKEN
    AddTabPage( TP_NUMBER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), 0 ); //CHINA001 AddTabPage( TP_NUMBER,     SvxNumberFormatTabPage::Create, 0 );
#else
    String number = rtl::OUString::createFromAscii ("Numbers");
    AddTabPage( TP_NUMBER, number, pFact->GetTabPageCreatorFunc (RID_SVXPAGE_NUMBERFORMAT), 0, sal_False, TAB_APPEND);
#endif
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_FONT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), 0 ); //CHINA001 AddTabPage( TP_FONT,        SvxCharNamePage::Create,        0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_FONTEFF, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), 0 ); //CHINA001 AddTabPage( TP_FONTEFF,       SvxCharEffectsPage::Create,     0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ),    0 ); //CHINA001 AddTabPage( TP_ALIGNMENT,   SvxAlignmentTabPage::Create,    0 );

    if ( aCJKOptions.IsAsianTypographyEnabled() )
    {
        //CHINA001 AddTabPage( TP_ASIAN,    SvxAsianTabPage::Create,        0 );
//        ::CreateTabPage pCreateTabpage = pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN);
        DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");//CHINA001
        AddTabPage( TP_ASIAN,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       0 );
    }
    else
        RemoveTabPage( TP_ASIAN );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_BORDER,      pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ),     0 ); //CHINA001 AddTabPage( TP_BORDER,      SvxBorderTabPage::Create,       0 );
    DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");//CHINA001
    AddTabPage( TP_BACKGROUND,  pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), 0 ); //CHINA001 AddTabPage( TP_BACKGROUND,  SvxBackgroundTabPage::Create,   0 );
    AddTabPage( TP_PROTECTION,  ScTabPageProtection::Create,    0 );
    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScAttrDlg::~ScAttrDlg()
{
}

// -----------------------------------------------------------------------

void __EXPORT ScAttrDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool())); //CHINA001
    switch ( nPageId )
    {
        case TP_NUMBER:
        {
            //CHINA001 SvxNumberFormatTabPage& rNumPage  = (SvxNumberFormatTabPage&)rTabPage;

            //CHINA001 rNumPage.SetOkHdl( LINK( this, ScAttrDlg, OkHandler ) );
            aSet.Put (SfxLinkItem( SID_LINK_TYPE, LINK( this, ScAttrDlg, OkHandler )));
            rTabPage.PageCreated(aSet);
        }
        break;

        case TP_FONT:
        {
            const SfxPoolItem* pInfoItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            DBG_ASSERT( pInfoItem, "FontListItem  not found :-(" );

            //CHINA001 ((SvxCharNamePage&)rTabPage).
                //CHINA001 SetFontList( *((const SvxFontListItem*)pInfoItem) );
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


