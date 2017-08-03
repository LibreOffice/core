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

#include "srchxtra.hxx"
#include <vcl/msgbox.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/whiter.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>
#include <svx/strarray.hxx>
#include <editeng/flstitem.hxx>
#include "chardlg.hxx"
#include "paragrph.hxx"
#include <dialmgr.hxx>
#include "backgrnd.hxx"
#include <svx/dialogs.hrc>
#include <tools/resary.hxx>
#include <rtl/strbuf.hxx>
#include "svtools/treelistentry.hxx"

SvxSearchFormatDialog::SvxSearchFormatDialog(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabDialog(pParent, "SearchFormatDialog", "cui/ui/searchformatdialog.ui", &rSet)
    , m_pFontList(nullptr)
    , m_nNamePageId(0)
    , m_nParaStdPageId(0)
    , m_nParaAlignPageId(0)
    , m_nBackPageId(0)
{
    m_nNamePageId = AddTabPage("font", SvxCharNamePage::Create, nullptr);
    AddTabPage("fonteffects", SvxCharEffectsPage::Create, nullptr);
    AddTabPage("position", SvxCharPositionPage::Create, nullptr);
    AddTabPage("asianlayout", SvxCharTwoLinesPage::Create, nullptr);
    m_nParaStdPageId = AddTabPage("labelTP_PARA_STD", SvxStdParagraphTabPage::Create, nullptr);
    m_nParaAlignPageId = AddTabPage("labelTP_PARA_ALIGN", SvxParaAlignTabPage::Create, nullptr);
    AddTabPage("labelTP_PARA_EXT", SvxExtParagraphTabPage::Create, nullptr);
    AddTabPage("labelTP_PARA_ASIAN", SvxAsianTabPage::Create, nullptr );
    //TODO m_nBackPageId = AddTabPage("background", SvxBackgroundTabPage::Create, nullptr);

    // remove asian tabpages if necessary
    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage("asianlayout");
    if ( !aCJKOptions.IsAsianTypographyEnabled() )
        RemoveTabPage("labelTP_PARA_ASIAN");
}

SvxSearchFormatDialog::~SvxSearchFormatDialog()
{
    disposeOnce();
}

void SvxSearchFormatDialog::dispose()
{
    delete m_pFontList;
    m_pFontList = nullptr;
    SfxTabDialog::dispose();
}

void SvxSearchFormatDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    if (nId == m_nNamePageId)
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
                m_pFontList = new FontList( this );
            pList = m_pFontList;
        }

        if ( pList )
            static_cast<SvxCharNamePage&>(rPage).
                SetFontList( SvxFontListItem( pList, SID_ATTR_CHAR_FONTLIST ) );
        static_cast<SvxCharNamePage&>(rPage).EnableSearchMode();
    }
    else if (nId == m_nParaStdPageId)
    {
        static_cast<SvxStdParagraphTabPage&>(rPage).EnableAutoFirstLine();
    }
    else if (nId == m_nParaAlignPageId)
    {
        static_cast<SvxParaAlignTabPage&>(rPage).EnableJustifyExt();
    }
    else if (nId == m_nBackPageId)
    {
        SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
        aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,static_cast<sal_uInt32>(SvxBackgroundTabFlags::SHOW_HIGHLIGHTING)));
        rPage.PageCreated(aSet);
    }
}

SvxSearchAttributeDialog::SvxSearchAttributeDialog(vcl::Window* pParent,
    SearchAttrItemList& rLst, const sal_uInt16* pWhRanges)
    : ModalDialog(pParent, "SearchAttrDialog", "cui/ui/searchattrdialog.ui")
    , rList(rLst)
{
    get(m_pOKBtn, "ok");
    get(m_pAttrLB, "treeview");
    m_pAttrLB->set_height_request(m_pAttrLB->GetTextHeight() * 12);
    m_pAttrLB->set_width_request(m_pAttrLB->approximate_char_width() * 56);

    m_pAttrLB->SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    m_pAttrLB->GetModel()->SetSortMode( SortAscending );

    m_pOKBtn->SetClickHdl( LINK( this, SvxSearchAttributeDialog, OKHdl ) );

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
            SvTreeListEntry* pEntry = nullptr;
            if ( RESARRAY_INDEX_NOTFOUND != nId )
                pEntry = m_pAttrLB->SvTreeListBox::InsertEntry(SvxAttrNameTable::GetString(nId));
            else
                SAL_WARN( "cui.dialogs", "no resource for slot id " << static_cast<sal_Int32>(nSlot) );

            if ( pEntry )
            {
                m_pAttrLB->SetCheckButtonState( pEntry, bChecked ? SvButtonState::Checked : SvButtonState::Unchecked );
                pEntry->SetUserData( reinterpret_cast<void*>(nSlot) );
            }
        }
        nWhich = aIter.NextWhich();
    }

    m_pAttrLB->SetHighlightRange();
    m_pAttrLB->SelectEntryPos( 0 );
}

SvxSearchAttributeDialog::~SvxSearchAttributeDialog()
{
    disposeOnce();
}

void SvxSearchAttributeDialog::dispose()
{
    m_pAttrLB.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG(SvxSearchAttributeDialog, OKHdl, Button*, void)
{
    SearchAttrItem aInvalidItem;
    aInvalidItem.pItem = INVALID_POOL_ITEM;

    for ( sal_uLong i = 0; i < m_pAttrLB->GetEntryCount(); ++i )
    {
        sal_uInt16 nSlot = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pAttrLB->GetEntryData(i));
        bool bChecked = m_pAttrLB->IsChecked(i);

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

    EndDialog( RET_OK );
}

// class SvxSearchSimilarityDialog ---------------------------------------

SvxSearchSimilarityDialog::SvxSearchSimilarityDialog
(
    vcl::Window* pParent,
    bool bRelax,
    sal_uInt16 nOther,
    sal_uInt16 nShorter,
    sal_uInt16 nLonger
) :
    ModalDialog( pParent, "SimilaritySearchDialog", "cui/ui/similaritysearchdialog.ui" )
{
    get( m_pOtherFld, "otherfld");
    get( m_pLongerFld, "longerfld");
    get( m_pShorterFld, "shorterfld");
    get( m_pRelaxBox, "relaxbox");

    m_pOtherFld->SetValue( nOther );
    m_pShorterFld->SetValue( nShorter );
    m_pLongerFld->SetValue( nLonger );
    m_pRelaxBox->Check( bRelax );
}

SvxSearchSimilarityDialog::~SvxSearchSimilarityDialog()
{
    disposeOnce();
}

void SvxSearchSimilarityDialog::dispose()
{
    m_pOtherFld.clear();
    m_pLongerFld.clear();
    m_pShorterFld.clear();
    m_pRelaxBox.clear();
    ModalDialog::dispose();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
