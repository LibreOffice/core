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

#include <scitems.hxx>
#include <svx/numinf.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <svl/cjkoptions.hxx>

#include <styledlg.hxx>
#include <tabpages.hxx>
#include <tphf.hxx>
#include <tptable.hxx>
#include <scres.hrc>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svl/intitem.hxx>
#include <editeng/flstitem.hxx>
#include <svl/aeitem.hxx>
#include <svx/flagsdef.hxx>

ScStyleDlg::ScStyleDlg( vcl::Window*             pParent,
                        SfxStyleSheetBase&  rStyleBase,
                        sal_uInt16              nRscId )

    : SfxStyleDialog  ( pParent,
                        nRscId == RID_SCDLG_STYLES_PAR ?
                          OUString("ParaTemplateDialog") :
                          OUString("PageTemplateDialog"),
                        nRscId == RID_SCDLG_STYLES_PAR ?
                          OUString("modules/scalc/ui/paratemplatedialog.ui") :
                          OUString("modules/scalc/ui/pagetemplatedialog.ui"),
                        rStyleBase )
    , nDlgRsc         ( nRscId )
    , m_nNumberId(0)
    , m_nFontId(0)
    , m_nBackgroundId(0)
    , m_nPageId(0)
    , m_nHeaderId(0)
    , m_nFooterId(0)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    switch ( nRscId )
    {
        case RID_SCDLG_STYLES_PAR:  // cell format styles
            {
                SvtCJKOptions aCJKOptions;
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ), "GetTabPageRangesFunc fail!");
                m_nNumberId = AddTabPage("numbers", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_NUMBERFORMAT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_NUMBERFORMAT ));
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");
                m_nFontId = AddTabPage("font", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ));
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");
                AddTabPage("fonteffects", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ));
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE( pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ), "GetTabPageRangesFunc fail!");
                 AddTabPage("alignment", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_ALIGNMENT ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_ALIGNMENT ));
                if ( aCJKOptions.IsAsianTypographyEnabled() )
                {
                    OSL_ENSURE(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
                    OSL_ENSURE(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
                     AddTabPage("asiantypo", pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),       pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN));
                }
                else
                    RemoveTabPage("asiantypo");
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                 AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ));
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
                m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ));
                AddTabPage("protection", &ScTabPageProtection::Create,    &ScTabPageProtection::GetRanges);
            }
            break;

        case RID_SCDLG_STYLES_PAGE: // page styles
            {
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");
                m_nPageId = AddTabPage("page", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                AddTabPage("borders", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                OSL_ENSURE(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), "GetTabPageCreatorFunc fail!");
                OSL_ENSURE(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
                m_nBackgroundId = AddTabPage("background", pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BKG ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );
                m_nHeaderId = AddTabPage("header", &ScHeaderPage::Create,      &ScHeaderPage::GetRanges );
                m_nFooterId = AddTabPage("footer", &ScFooterPage::Create,      &ScFooterPage::GetRanges );
                AddTabPage("sheet", &ScTablePage::Create,     &ScTablePage::GetRanges );
            }
            break;

        default:
            OSL_FAIL( "Family not supported" );
    }
}

void ScStyleDlg::PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage )
{
    if ( nDlgRsc == RID_SCDLG_STYLES_PAR )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        if (nPageId == m_nNumberId)
        {
            const SfxPoolItem* pInfoItem
                = pDocSh->GetItem( SID_ATTR_NUMBERFORMAT_INFO );

            OSL_ENSURE( pInfoItem, "NumberInfoItem not found!" );

            aSet.Put (SvxNumberInfoItem( static_cast<const SvxNumberInfoItem&>(*pInfoItem) ) );
            rTabPage.PageCreated(aSet);
        }
        else if (nPageId == m_nFontId)
        {
            const SfxPoolItem* pInfoItem
                = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );

            OSL_ENSURE( pInfoItem, "FontListItem not found!" );

            aSet.Put (SvxFontListItem(static_cast<const SvxFontListItem&>(*pInfoItem).GetFontList(), SID_ATTR_CHAR_FONTLIST));
            rTabPage.PageCreated(aSet);
        }
        else if (nPageId == m_nBackgroundId)
        {
            rTabPage.PageCreated(aSet);
        }
    }
    else if ( nDlgRsc == RID_SCDLG_STYLES_PAGE )
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        if (nPageId == m_nPageId)
        {
            aSet.Put (SfxAllEnumItem(sal_uInt16(SID_ENUM_PAGE_MODE), SVX_PAGE_MODE_CENTER));
            rTabPage.PageCreated(aSet);
        }
        else if (nPageId == m_nHeaderId || nPageId == m_nFooterId)
        {
            static_cast<ScHFPage&>(rTabPage).SetStyleDlg( this );
            static_cast<ScHFPage&>(rTabPage).SetPageStyle( GetStyleSheet().GetName() );
            static_cast<ScHFPage&>(rTabPage).DisableDeleteQueryBox();
        }
        else if (nPageId == m_nBackgroundId)
        {
            aSet.Put (SfxUInt32Item(SID_FLAG_TYPE, static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_SELECTOR)));
            rTabPage.PageCreated(aSet);
        }
    }
}

void ScStyleDlg::RefreshInputSet()
{
    SfxItemSet* pItemSet = GetInputSetImpl();
    pItemSet->ClearItem();
    pItemSet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
