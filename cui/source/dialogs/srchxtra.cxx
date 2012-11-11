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
#include "srchxtra.hrc"
#include <svx/svxitems.hrc> // RID_ATTR_BEGIN
#include <svx/dialmgr.hxx>  // item resources
#include <editeng/flstitem.hxx>
#include "chardlg.hxx"
#include "paragrph.hxx"
#include <dialmgr.hxx>
#include "backgrnd.hxx"
#include <svx/dialogs.hrc> // RID_SVXPAGE_...
#include <tools/resary.hxx>
#include <rtl/strbuf.hxx>

// class SvxSearchFormatDialog -------------------------------------------

SvxSearchFormatDialog::SvxSearchFormatDialog( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_SEARCHFORMAT ), &rSet ),

    pFontList( NULL )

{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION, SvxCharPositionPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_TWOLINES, SvxCharTwoLinesPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_EXT_PARAGRAPH, SvxExtParagraphTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_PARA_ASIAN, SvxAsianTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BACKGROUND, SvxBackgroundTabPage::Create, 0 );

    // remove asian tabpages if necessary
    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage( RID_SVXPAGE_CHAR_TWOLINES );
    if ( !aCJKOptions.IsAsianTypographyEnabled() )
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );
}

// -----------------------------------------------------------------------

SvxSearchFormatDialog::~SvxSearchFormatDialog()
{
    delete pFontList;
}

// -----------------------------------------------------------------------

void SvxSearchFormatDialog::PageCreated( sal_uInt16 nId, SfxTabPage& rPage )
{
    switch ( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
        {
            const FontList* pAppFontList = 0;
            SfxObjectShell* pSh = SfxObjectShell::Current();

            if ( pSh )
            {
                const SvxFontListItem* pFLItem = (const SvxFontListItem*)
                    pSh->GetItem( SID_ATTR_CHAR_FONTLIST );
                if ( pFLItem )
                    pAppFontList = pFLItem->GetFontList();
            }

            const FontList* pList = pAppFontList;

            if ( !pList )
            {
                if ( !pFontList )
                    pFontList = new FontList( this );
                pList = pFontList;
            }

            if ( pList )
                ( (SvxCharNamePage&)rPage ).
                    SetFontList( SvxFontListItem( pList, SID_ATTR_CHAR_FONTLIST ) );
            ( (SvxCharNamePage&)rPage ).EnableSearchMode();
            break;
        }

        case RID_SVXPAGE_STD_PARAGRAPH:
            ( (SvxStdParagraphTabPage&)rPage ).EnableAutoFirstLine();
            break;

        case RID_SVXPAGE_ALIGN_PARAGRAPH:
            ( (SvxParaAlignTabPage&)rPage ).EnableJustifyExt();
            break;
        case RID_SVXPAGE_BACKGROUND :
            ( (SvxBackgroundTabPage&)rPage ).ShowParaControl(sal_True);
            break;
    }
}

// class SvxSearchFormatDialog -------------------------------------------

SvxSearchAttributeDialog::SvxSearchAttributeDialog( Window* pParent,
                                                    SearchAttrItemList& rLst,
                                                    const sal_uInt16* pWhRanges ) :

    ModalDialog( pParent, CUI_RES( RID_SVXDLG_SEARCHATTR )  ),

    aAttrFL ( this, CUI_RES( FL_ATTR ) ),
    aAttrLB ( this, CUI_RES( LB_ATTR ) ),
    aOKBtn  ( this, CUI_RES( BTN_ATTR_OK ) ),
    aEscBtn ( this, CUI_RES( BTN_ATTR_CANCEL ) ),
    aHelpBtn( this, CUI_RES( BTN_ATTR_HELP ) ),

    rList( rLst )

{
    FreeResource();

    aAttrLB.SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    aAttrLB.GetModel()->SetSortMode( SortAscending );

    aOKBtn.SetClickHdl( LINK( this, SvxSearchAttributeDialog, OKHdl ) );

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
                pEntry = aAttrLB.SvTreeListBox::InsertEntry( aAttrNames.GetString(nId) );
            else
                SAL_WARN( "cui.dialogs", "no resource for slot id " << static_cast<sal_Int32>(nSlot) );

            if ( pEntry )
            {
                aAttrLB.SetCheckButtonState( pEntry, bChecked ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
                pEntry->SetUserData( (void*)(sal_uLong)nSlot );
            }
        }
        nWhich = aIter.NextWhich();
    }

    aAttrLB.SetHighlightRange();
    aAttrLB.SelectEntryPos( 0 );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxSearchAttributeDialog, OKHdl)
{
    SearchAttrItem aInvalidItem;
    aInvalidItem.pItem = (SfxPoolItem*)-1;

    for ( sal_uInt16 i = 0; i < aAttrLB.GetEntryCount(); ++i )
    {
        sal_uInt16 nSlot = (sal_uInt16)(sal_uLong)aAttrLB.GetEntryData(i);
        sal_Bool bChecked = aAttrLB.IsChecked(i);

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
    ModalDialog( pParent, CUI_RES( RID_SVXDLG_SEARCHSIMILARITY ) ),

    aFixedLine  ( this, CUI_RES( FL_SIMILARITY ) ),
    aOtherTxt   ( this, CUI_RES( FT_OTHER ) ),
    aOtherFld   ( this, CUI_RES( NF_OTHER   ) ),
    aLongerTxt  ( this, CUI_RES( FT_LONGER ) ),
    aLongerFld  ( this, CUI_RES( NF_LONGER ) ),
    aShorterTxt ( this, CUI_RES( FT_SHORTER ) ),
    aShorterFld ( this, CUI_RES( NF_SHORTER ) ),
    aRelaxBox   ( this, CUI_RES( CB_RELAX ) ),

    aOKBtn      ( this, CUI_RES( BTN_ATTR_OK ) ),
    aEscBtn     ( this, CUI_RES( BTN_ATTR_CANCEL ) ),
    aHelpBtn    ( this, CUI_RES( BTN_ATTR_HELP ) )

{
    FreeResource();

    aOtherFld.SetValue( nOther );
    aShorterFld.SetValue( nShorter );
    aLongerFld.SetValue( nLonger );
    aRelaxBox.Check( bRelax );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
