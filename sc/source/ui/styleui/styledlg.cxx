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
#include <svx/numinf.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <svl/cjkoptions.hxx>

#include "styledlg.hxx"
#include "tabpages.hxx"     // Zellvorlagen
#include "tphf.hxx"         // Seitenvorlage: Kopf-/Fusszeilen
#include "tptable.hxx"      // Seitenvorlage: Tabelle
#include "scresid.hxx"
#include "sc.hrc"
#include "styledlg.hrc"
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/intitem.hxx>
#include <editeng/flstitem.hxx>
#include <svl/aeitem.hxx>
#include <svx/flagsdef.hxx>
//==================================================================

ScStyleDlg::ScStyleDlg( Window*             pParent,
                        SfxStyleSheetBase&  rStyleBase,
                        sal_uInt16              nRscId )

    :   SfxStyleDialog  ( pParent,
                          ScResId( nRscId ),
                          rStyleBase,
                          false ),
        nDlgRsc         ( nRscId )
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "Dialogdiet fail!");
    switch ( nRscId )
    {
        case RID_SCDLG_STYLES_PAR:  // Zellformatvorlagen
            {
                SvtCJKOptions aCJKOptions;
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_NUMBER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_FONT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_FONTEFF, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE( pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_ALIGNMENT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ) );
                if ( aCJKOptions.IsAsianTypographyEnabled() )
                {
                    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
                    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
                    AddTabPage( TP_ASIAN,   pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );
                }
                else
                    RemoveTabPage( TP_ASIAN );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
                AddTabPage( TP_PROTECTION, &ScTabPageProtection::Create,    &ScTabPageProtection::GetRanges );
            }
            break;

        case RID_SCDLG_STYLES_PAGE: // Seitenvorlagen
            {
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_PAGE_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
                AddTabPage( TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
                AddTabPage( TP_PAGE_HEADER, &ScHeaderPage::Create,      &ScHeaderPage::GetRanges );
                AddTabPage( TP_PAGE_FOOTER, &ScFooterPage::Create,      &ScFooterPage::GetRanges );
                AddTabPage( TP_TABLE, &ScTablePage::Create,     &ScTablePage::GetRanges );
            }
            break;

        default:
            OSL_FAIL( "Family not supported" );
    }

    FreeResource();
}

// -----------------------------------------------------------------------

ScStyleDlg::~ScStyleDlg()
{
}

// -----------------------------------------------------------------------

void ScStyleDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    if ( nDlgRsc == RID_SCDLG_STYLES_PAR )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        switch ( nPageId )
        {
            case TP_NUMBER:
                {
                    const SfxPoolItem* pInfoItem
                        = pDocSh->GetItem( SID_ATTR_NUMBERFORMAT_INFO );

                    OSL_ENSURE( pInfoItem, "NumberInfoItem nicht gefunden!" );

                    aSet.Put (SvxNumberInfoItem( (const SvxNumberInfoItem&)*pInfoItem ) );
                    rTabPage.PageCreated(aSet);
                }
                break;

            case TP_FONT:
                {
                    const SfxPoolItem* pInfoItem
                        = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

                    OSL_ENSURE( pInfoItem, "FontListItem nicht gefunden!" );

                    aSet.Put (SvxFontListItem(((const SvxFontListItem&)*pInfoItem).GetFontList(), SID_ATTR_CHAR_FONTLIST));
                    rTabPage.PageCreated(aSet);
                }
                break;

            default:
            break;
        }
    }
    else if ( nDlgRsc == RID_SCDLG_STYLES_PAGE )
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        switch ( nPageId )
        {
            case TP_PAGE_STD:
                aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_CENTER));
                rTabPage.PageCreated(aSet);
                break;

            case TP_PAGE_HEADER:
            case TP_PAGE_FOOTER:
                ((ScHFPage&)rTabPage).SetStyleDlg( this );
                ((ScHFPage&)rTabPage).SetPageStyle( GetStyleSheet().GetName() );
                ((ScHFPage&)rTabPage).DisableDeleteQueryBox();
                break;
            case TP_BACKGROUND:
                    if( nDlgRsc == RID_SCDLG_STYLES_PAGE)
                    {
                        aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, SVX_SHOW_SELECTOR));
                        rTabPage.PageCreated(aSet);
                    }
                break;

            default:
                break;
        }
    }
}


// -----------------------------------------------------------------------

const SfxItemSet* ScStyleDlg::GetRefreshedSet()
{
    SfxItemSet* pItemSet = GetInputSetImpl();
    pItemSet->ClearItem();
    pItemSet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
    return pItemSet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
