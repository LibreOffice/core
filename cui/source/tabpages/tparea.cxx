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

#include <stdlib.h>

#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/dialogs.hrc>

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <cuires.hrc>
#include <svx/xflbckit.hxx>
#include <svx/svdattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xlineit0.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "dlgname.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include "paragrph.hrc"
#include <sfx2/tabdlg.hxx>
#include "sfx2/opengrf.hxx"
#include <vcl/layout.hxx>

using namespace com::sun::star;

// static ----------------------------------------------------------------

const sal_uInt16 SvxAreaTabPage::pAreaRanges[] =
{
    XATTR_GRADIENTSTEPCOUNT,
    XATTR_GRADIENTSTEPCOUNT,
    SID_ATTR_FILL_STYLE,
    SID_ATTR_FILL_BITMAP,
    0
};

/*************************************************************************
|*
|*  Dialog to modify fill-attributes
|*
\************************************************************************/

SvxAreaTabPage::SvxAreaTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage( pParent,
                "AreaTabPage",
                "cui/ui/areatabpage.ui",
               rInAttrs ),
    m_rOutAttrs (rInAttrs ),
    m_eRP( RP_LT ),

    m_pColorList( nullptr ),
    m_pGradientList( nullptr ),
    m_pHatchingList( nullptr ),
    m_pBitmapList( nullptr ),

    // local fixed not o be changed values for local pointers
    maFixed_ChangeType(ChangeType::NONE),
    maFixed_sal_Bool(false),

    // init with pointers to fixed ChangeType
    m_pnColorListState(&maFixed_ChangeType),
    m_pnBitmapListState(&maFixed_ChangeType),
    m_pnGradientListState(&maFixed_ChangeType),
    m_pnHatchingListState(&maFixed_ChangeType),

    m_nPageType(0),
    m_nDlgType(0),
    m_nPos(0),

    // init with pointer to fixed bool
    m_pbAreaTP(&maFixed_sal_Bool),

    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pBtnNone, "btnnone");
    get(m_pBtnColor, "btncolor");
    get(m_pBtnGradient, "btngradient");
    get(m_pFillTab, "fillstylebox");

    Link< Button*, void > aLink = LINK(this, SvxAreaTabPage, SelectFillTypeHdl_Impl);
    m_pBtnNone->SetClickHdl(aLink);
    m_pBtnColor->SetClickHdl(aLink);
    m_pBtnGradient->SetClickHdl(aLink);
    // setting the output device
    m_rXFSet.Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    m_rXFSet.Put( XFillColorItem( OUString(), COL_BLACK ) );

    // #i76307# always paint the preview in LTR, because this is what the document does

    // Calculate size of dropdown listboxes
}

SvxAreaTabPage::~SvxAreaTabPage()
{
    disposeOnce();
}

void SvxAreaTabPage::dispose()
{
    m_pFillTab.clear();
    SvxTabPage::dispose();
}


void SvxAreaTabPage::Construct()
{
}


void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_Int32 nCount(0);
    /*
    const SfxUInt16Item* pPageTypeItem = rSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pPosItem = rSet.GetItem<SfxUInt16Item>(SID_TABPAGE_POS, false);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());*/
    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = true;
    }
}


DeactivateRC SvxAreaTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    return DeactivateRC::LeavePage;
}


bool SvxAreaTabPage::FillItemSet( SfxItemSet* rAttrs )
{
}


void SvxAreaTabPage::Reset( const SfxItemSet* rAttrs )
{
}

void SvxAreaTabPage::ChangesApplied()
{
}

VclPtr<SfxTabPage> SvxAreaTabPage::Create( vcl::Window* pWindow,
                                           const SfxItemSet* rAttrs )
{
    return VclPtr<SvxAreaTabPage>::Create( pWindow, *rAttrs );
}


IMPL_LINK_TYPED(SvxAreaTabPage, SelectFillTypeHdl_Impl, Button*, pButton, void)
{
    CreateTabPage fnCreate = nullptr;
    sal_Int32 nPos = -1;
        if(pButton == m_pBtnNone)
        {
            m_pBtnNone->SetPressed(true);
            m_pBtnColor->SetPressed(false);
            m_pBtnGradient->SetPressed(false);
            nPos = 0;
        }
        else if(pButton == m_pBtnColor)
        {
            m_pBtnNone->SetPressed(false);
            m_pBtnColor->SetPressed(true);
            m_pBtnGradient->SetPressed(false);
            fnCreate = &SvxColorTabPage::Create;
            nPos = 1;
        }
        else if(pButton == m_pBtnGradient)
        {
            m_pBtnNone->SetPressed(false);
            m_pBtnColor->SetPressed(false);
            m_pBtnGradient->SetPressed(true);
            fnCreate = &SvxGradientTabPage::Create;
            nPos = 2;
        }
        if(fnCreate)
        {
            VclPtr<SfxTabPage> pRet = (*fnCreate)(m_pFillTab, &m_rXFSet);
            m_pFillTabPage.disposeAndReset( pRet );
            CreatePage( nPos , *m_pFillTabPage);
        }

}

void SvxAreaTabPage::PointChanged( vcl::Window* , RECT_POINT )
{
}

void SvxAreaTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxColorListItem* pColorListItem = aSet.GetItem<SvxColorListItem>(SID_COLOR_TABLE, false);
    const SvxGradientListItem* pGradientListItem = aSet.GetItem<SvxGradientListItem>(SID_GRADIENT_LIST, false);
    const SvxHatchListItem* pHatchingListItem = aSet.GetItem<SvxHatchListItem>(SID_HATCH_LIST, false);
    const SvxBitmapListItem* pBitmapListItem = aSet.GetItem<SvxBitmapListItem>(SID_BITMAP_LIST, false);
    const SfxUInt16Item* pPageTypeItem = aSet.GetItem<SfxUInt16Item>(SID_PAGE_TYPE, false);
    const SfxUInt16Item* pDlgTypeItem = aSet.GetItem<SfxUInt16Item>(SID_DLG_TYPE, false);
    const SfxUInt16Item* pPosItem = aSet.GetItem<SfxUInt16Item>(SID_TABPAGE_POS, false);

    if (pColorListItem)
        SetColorList(pColorListItem->GetColorList());
    if (pGradientListItem)
        SetGradientList(pGradientListItem->GetGradientList());
    if (pHatchingListItem)
        SetHatchingList(pHatchingListItem->GetHatchList());
    if (pBitmapListItem)
        SetBitmapList(pBitmapListItem->GetBitmapList());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    if (pPosItem)
        SetPos(pPosItem->GetValue());

    Construct();
}

void SvxAreaTabPage::CreatePage( sal_Int32 nId, SfxTabPage& pTab )
{
    if(nId == 0)
    {

    }
    else if(nId == 1 )
    {
        static_cast<SvxColorTabPage&>(pTab).SetColorList( m_pColorList );
        static_cast<SvxColorTabPage&>(pTab).SetPageType( &m_nPageType );
        static_cast<SvxColorTabPage&>(pTab).SetDlgType( 0 );
        static_cast<SvxColorTabPage&>(pTab).SetPos( &m_nPos );
        static_cast<SvxColorTabPage&>(pTab).SetAreaTP( false );
        static_cast<SvxColorTabPage&>(pTab).SetColorChgd( m_pnColorListState );
        static_cast<SvxColorTabPage&>(pTab).Construct();
        static_cast<SvxColorTabPage&>(pTab).Show();
    }
    else if(nId == 2)
    {
        static_cast<SvxGradientTabPage&>(pTab).SetColorList( m_pColorList );
        static_cast<SvxGradientTabPage&>(pTab).SetGradientList( m_pGradientList );
        static_cast<SvxGradientTabPage&>(pTab).SetPageType( &m_nPageType );
        static_cast<SvxGradientTabPage&>(pTab).SetDlgType( 0 );
        static_cast<SvxGradientTabPage&>(pTab).SetPos( &m_nPos );
        static_cast<SvxGradientTabPage&>(pTab).SetAreaTP( false );
        static_cast<SvxGradientTabPage&>(pTab).SetGrdChgd( m_pnGradientListState );
        static_cast<SvxGradientTabPage&>(pTab).SetColorChgd( m_pnColorListState );
        static_cast<SvxGradientTabPage&>(pTab).Construct();
        static_cast<SvxGradientTabPage&>(pTab).Show();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
