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

#include <srchxtra.hxx>
#include <sal/log.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/whiter.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svxitems.hrc>
#include <svx/strarray.hxx>
#include <editeng/flstitem.hxx>
#include <chardlg.hxx>
#include <paragrph.hxx>
#include <backgrnd.hxx>
#include <svx/dialogs.hrc>
#include <tools/resary.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/treelistentry.hxx>

SvxSearchFormatDialog::SvxSearchFormatDialog(weld::Window* pParent, const SfxItemSet& rSet)
    : SfxTabDialogController(pParent, "cui/ui/searchformatdialog.ui", "SearchFormatDialog", &rSet)
{
    AddTabPage("font", SvxCharNamePage::Create, nullptr);
    AddTabPage("fonteffects", SvxCharEffectsPage::Create, nullptr);
    AddTabPage("position", SvxCharPositionPage::Create, nullptr);
    AddTabPage("asianlayout", SvxCharTwoLinesPage::Create, nullptr);
    AddTabPage("labelTP_PARA_STD", SvxStdParagraphTabPage::Create, nullptr);
    AddTabPage("labelTP_PARA_ALIGN", SvxParaAlignTabPage::Create, nullptr);
    AddTabPage("labelTP_PARA_EXT", SvxExtParagraphTabPage::Create, nullptr);
    AddTabPage("labelTP_PARA_ASIAN", SvxAsianTabPage::Create, nullptr );
    AddTabPage("background", SvxBkgTabPage::Create, nullptr);

    // remove asian tabpages if necessary
    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage("asianlayout");
    if ( !aCJKOptions.IsAsianTypographyEnabled() )
        RemoveTabPage("labelTP_PARA_ASIAN");
}

SvxSearchFormatDialog::~SvxSearchFormatDialog()
{
}

void SvxSearchFormatDialog::PageCreated(const OString& rId, SfxTabPage& rPage)
{
    if (rId == "font")
    {
        const FontList* pApm_pFontList = nullptr;
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
        {
            const SvxFontListItem* pFLItem = static_cast<const SvxFontListItem*>(
                pSh->GetItem( SID_ATTR_CHAR_FONTLIST ));
            if ( pFLItem )
                pApm_pFontList = pFLItem->GetFontList();
        }

        const FontList* pList = pApm_pFontList;

        if ( !pList )
        {
            if ( !m_pFontList )
                m_pFontList.reset(new FontList(Application::GetDefaultDevice()));
            pList = m_pFontList.get();
        }

        static_cast<SvxCharNamePage&>(rPage).
                SetFontList( SvxFontListItem( pList, SID_ATTR_CHAR_FONTLIST ) );
        static_cast<SvxCharNamePage&>(rPage).EnableSearchMode();
    }
    else if (rId == "labelTP_PARA_STD")
    {
        static_cast<SvxStdParagraphTabPage&>(rPage).EnableAutoFirstLine();
    }
    else if (rId == "labelTP_PARA_ALIGN")
    {
        static_cast<SvxParaAlignTabPage&>(rPage).EnableJustifyExt();
    }
    else if (rId == "background")
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_HIGHLIGHTING)));
        rPage.PageCreated(aSet);
    }
}

SvxSearchAttributeDialog::SvxSearchAttributeDialog(weld::Window* pParent,
    SearchAttrItemList& rLst, const sal_uInt16* pWhRanges)
    : GenericDialogController(pParent, "cui/ui/searchattrdialog.ui", "SearchAttrDialog")
    , rList(rLst)
    , m_xAttrLB(m_xBuilder->weld_tree_view("treeview"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
{
    m_xAttrLB->set_size_request(m_xAttrLB->get_approximate_digit_width() * 50,
                                m_xAttrLB->get_height_rows(12));

    std::vector<int> aWidths;
    aWidths.push_back(m_xAttrLB->get_checkbox_column_width());
    m_xAttrLB->set_column_fixed_widths(aWidths);

    m_xOKBtn->connect_clicked(LINK( this, SvxSearchAttributeDialog, OKHdl));

    SfxObjectShell* pSh = SfxObjectShell::Current();
    DBG_ASSERT( pSh, "No DocShell" );

    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet( rPool, pWhRanges );
    SfxWhichIter aIter( aSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        sal_uInt16 nSlot = rPool.GetSlotId( nWhich );
        if ( nSlot >= SID_SVX_START )
        {
            bool bChecked = false, bFound = false;
            for ( sal_uInt16 i = 0; !bFound && i < rList.Count(); ++i )
            {
                if ( nSlot == rList[i].nSlot )
                {
                    bFound = true;
                    if ( IsInvalidItem( rList[i].pItem ) )
                        bChecked = true;
                }
            }

            // item resources are in svx
            sal_uInt32 nId  = SvxAttrNameTable::FindIndex(nSlot);
            if (RESARRAY_INDEX_NOTFOUND != nId)
            {
                m_xAttrLB->append();
                const int nRow = m_xAttrLB->n_children() - 1;
                m_xAttrLB->set_toggle(nRow, bChecked, 0);
                m_xAttrLB->set_text(nRow, SvxAttrNameTable::GetString(nId), 1);
                m_xAttrLB->set_id(nRow, OUString::number(nSlot));
            }
            else
                SAL_WARN( "cui.dialogs", "no resource for slot id " << static_cast<sal_Int32>(nSlot) );
        }
        nWhich = aIter.NextWhich();
    }

    m_xAttrLB->make_sorted();
    m_xAttrLB->select(0);
}

SvxSearchAttributeDialog::~SvxSearchAttributeDialog()
{
}

IMPL_LINK_NOARG(SvxSearchAttributeDialog, OKHdl, weld::Button&, void)
{
    SearchAttrItem aInvalidItem;
    aInvalidItem.pItem = INVALID_POOL_ITEM;

    for (int i = 0, nCount = m_xAttrLB->n_children(); i < nCount; ++i)
    {
        sal_uInt16 nSlot = m_xAttrLB->get_id(i).toUInt32();
        bool bChecked = m_xAttrLB->get_toggle(i, 0);

        sal_uInt16 j;
        for ( j = rList.Count(); j; )
        {
            SearchAttrItem& rItem = rList[ --j ];
            if( rItem.nSlot == nSlot )
            {
                if( bChecked )
                {
                    if( !IsInvalidItem( rItem.pItem ) )
                        delete rItem.pItem;
                    rItem.pItem = INVALID_POOL_ITEM;
                }
                else if( IsInvalidItem( rItem.pItem ) )
                    rItem.pItem = nullptr;
                j = 1;
                break;
            }
        }

        if ( !j && bChecked )
        {
            aInvalidItem.nSlot = nSlot;
            rList.Insert( aInvalidItem );
        }
    }

    // remove invalid items (pItem == NULL)
    for ( sal_uInt16 n = rList.Count(); n; )
        if ( !rList[ --n ].pItem )
            rList.Remove( n );

    m_xDialog->response(RET_OK);
}

// class SvxSearchSimilarityDialog ---------------------------------------

SvxSearchSimilarityDialog::SvxSearchSimilarityDialog(weld::Window* pParent, bool bRelax,
    sal_uInt16 nOther, sal_uInt16 nShorter, sal_uInt16 nLonger)
    : GenericDialogController(pParent, "cui/ui/similaritysearchdialog.ui", "SimilaritySearchDialog")
    , m_xOtherFld(m_xBuilder->weld_spin_button("otherfld"))
    , m_xLongerFld(m_xBuilder->weld_spin_button("longerfld"))
    , m_xShorterFld(m_xBuilder->weld_spin_button("shorterfld"))
    , m_xRelaxBox(m_xBuilder->weld_check_button("relaxbox"))
{
    m_xOtherFld->set_value(nOther);
    m_xShorterFld->set_value(nShorter);
    m_xLongerFld->set_value(nLonger);
    m_xRelaxBox->set_active(bRelax);
}

SvxSearchSimilarityDialog::~SvxSearchSimilarityDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
