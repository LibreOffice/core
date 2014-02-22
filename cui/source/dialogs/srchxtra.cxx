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
#include <tools/rcid.h>
#include <vcl/msgbox.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/whiter.hxx>
#include <sfx2/objsh.hxx>
#include <cuires.hrc>
#include <svx/svxitems.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/flstitem.hxx>
#include "chardlg.hxx"
#include "paragrph.hxx"
#include <dialmgr.hxx>
#include "backgrnd.hxx"
#include <svx/dialogs.hrc>
#include <tools/resary.hxx>
#include <rtl/strbuf.hxx>
#include "svtools/treelistentry.hxx"

SvxSearchFormatDialog::SvxSearchFormatDialog(Window* pParent, const SfxItemSet& rSet)
    : SfxTabDialog(pParent, "SearchFormatDialog", "cui/ui/searchformatdialog.ui", &rSet)
    , m_pFontList(NULL)
    , m_nNamePageId(0)
    , m_nParaStdPageId(0)
    , m_nParaAlignPageId(0)
    , m_nBackPageId(0)
{
    m_nNamePageId = AddTabPage("font", SvxCharNamePage::Create, 0);
    AddTabPage("fonteffects", SvxCharEffectsPage::Create, 0);
    AddTabPage("position", SvxCharPositionPage::Create, 0);
    AddTabPage("asianlayout", SvxCharTwoLinesPage::Create, 0);
    m_nParaStdPageId = AddTabPage("labelTP_PARA_STD", SvxStdParagraphTabPage::Create, 0);
    m_nParaAlignPageId = AddTabPage("labelTP_PARA_ALIGN", SvxParaAlignTabPage::Create, 0);
    AddTabPage("labelTP_PARA_EXT", SvxExtParagraphTabPage::Create, 0);
    AddTabPage("labelTP_PARA_ASIAN", SvxAsianTabPage::Create, 0 );
    m_nBackPageId = AddTabPage("background", SvxBackgroundTabPage::Create, 0);

    // remove asian tabpages if necessary
    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage("asianlayout");
    if ( !aCJKOptions.IsAsianTypographyEnabled() )
        RemoveTabPage("labelTP_PARA_ASIAN");
}

SvxSearchFormatDialog::~SvxSearchFormatDialog()
{
    delete m_pFontList;
}

void SvxSearchFormatDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    if (nId == m_nNamePageId)
    {
        const FontList* pApm_pFontList = 0;
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
        {
            const SvxFontListItem* pFLItem = (const SvxFontListItem*)
                pSh->GetItem( SID_ATTR_CHAR_FONTLIST );
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
            ( (SvxCharNamePage&)rPage ).
                SetFontList( SvxFontListItem( pList, SID_ATTR_CHAR_FONTLIST ) );
        ( (SvxCharNamePage&)rPage ).EnableSearchMode();
    }
    else if (nId == m_nParaStdPageId)
    {
        ( (SvxStdParagraphTabPage&)rPage ).EnableAutoFirstLine();
    }
    else if (nId == m_nParaAlignPageId)
    {
        ( (SvxParaAlignTabPage&)rPage ).EnableJustifyExt();
    }
    else if (nId == m_nBackPageId)
    {
        ( (SvxBackgroundTabPage&)rPage ).ShowParaControl(sal_True);
    }
}

SvxSearchAttributeDialog::SvxSearchAttributeDialog(Window* pParent,
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

    ResStringArray aAttrNames( SVX_RES( RID_ATTR_NAMES ) );
    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet( rPool, pWhRanges );
    SfxWhichIter aIter( aSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        sal_uInt16 nSlot = rPool.GetSlotId( nWhich );
        if ( nSlot >= SID_SVX_START )
        {
            sal_Bool bChecked = sal_False, bFound = sal_False;
            for ( sal_uInt16 i = 0; !bFound && i < rList.Count(); ++i )
            {
                if ( nSlot == rList[i].nSlot )
                {
                    bFound = sal_True;
                    if ( IsInvalidItem( rList[i].pItem ) )
                        bChecked = sal_True;
                }
            }

            // item resources are in svx
            sal_uInt32 nId  = aAttrNames.FindIndex( nSlot );
            SvTreeListEntry* pEntry = NULL;
            if ( RESARRAY_INDEX_NOTFOUND != nId )
                pEntry = m_pAttrLB->SvTreeListBox::InsertEntry( aAttrNames.GetString(nId) );
            else
                SAL_WARN( "cui.dialogs", "no resource for slot id " << static_cast<sal_Int32>(nSlot) );

            if ( pEntry )
            {
                m_pAttrLB->SetCheckButtonState( pEntry, bChecked ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
                pEntry->SetUserData( (void*)(sal_uLong)nSlot );
            }
        }
        nWhich = aIter.NextWhich();
    }

    m_pAttrLB->SetHighlightRange();
    m_pAttrLB->SelectEntryPos( 0 );
}



IMPL_LINK_NOARG(SvxSearchAttributeDialog, OKHdl)
{
    SearchAttrItem aInvalidItem;
    aInvalidItem.pItem = (SfxPoolItem*)-1;

    for ( sal_uInt16 i = 0; i < m_pAttrLB->GetEntryCount(); ++i )
    {
        sal_uInt16 nSlot = (sal_uInt16)(sal_uLong)m_pAttrLB->GetEntryData(i);
        sal_Bool bChecked = m_pAttrLB->IsChecked(i);

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
                    rItem.pItem = (SfxPoolItem*)-1;
                }
                else if( IsInvalidItem( rItem.pItem ) )
                    rItem.pItem = 0;
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
    return 0;
}

// class SvxSearchSimilarityDialog ---------------------------------------

SvxSearchSimilarityDialog::SvxSearchSimilarityDialog
(
    Window* pParent,
    sal_Bool bRelax,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
