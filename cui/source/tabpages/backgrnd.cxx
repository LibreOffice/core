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

#include <memory>
#include <sfx2/objsh.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <backgrnd.hxx>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include <svx/unobrushitemhelper.hxx>

using namespace css;

// table background
#define TBL_DEST_CELL   0
#define TBL_DEST_ROW    1
#define TBL_DEST_TBL    2

const WhichRangesContainer SvxBkgTabPage::pPageRanges(svl::Items<
    SID_ATTR_BRUSH, SID_ATTR_BRUSH,
    SID_ATTR_BRUSH_CHAR, SID_ATTR_BRUSH_CHAR
>);

static sal_uInt16 lcl_GetTableDestSlot(sal_Int32 nTblDest)
{
    switch (nTblDest)
    {
        default:
        case TBL_DEST_CELL:
        {
            return SID_ATTR_BRUSH;
        }
        case TBL_DEST_ROW:
        {
            return SID_ATTR_BRUSH_ROW;
        }
        case TBL_DEST_TBL:
        {
            return SID_ATTR_BRUSH_TABLE;
        }
    }
}

SvxBkgTabPage::SvxBkgTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SvxAreaTabPage(pPage, pController, rInAttrs),
    bHighlighting(false),
    bCharBackColor(false),
    maSet(rInAttrs)
{
    m_xBtnGradient->hide();
    m_xBtnHatch->hide();
    m_xBtnBitmap->hide();
    m_xBtnPattern->hide();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();

    XColorListRef pColorTable;
    if ( pDocSh )
        if (auto pItem = pDocSh->GetItem( SID_COLOR_TABLE ))
            pColorTable = pItem->GetColorList();

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    XBitmapListRef pBitmapList;
    if ( pDocSh )
        if (auto pItem = pDocSh->GetItem( SID_BITMAP_LIST ) )
            pBitmapList = pItem->GetBitmapList();

    SetColorList(pColorTable);
    SetBitmapList(pBitmapList);
}

SvxBkgTabPage::~SvxBkgTabPage()
{
    m_xTblLBox.reset();
}

void SvxBkgTabPage::ActivatePage( const SfxItemSet& )
{
    SvxAreaTabPage::ActivatePage( maSet );
}

DeactivateRC SvxBkgTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( DeactivateRC::KeepPage == SvxAreaTabPage::DeactivatePage( &maSet ) )
        return DeactivateRC::KeepPage;

    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

void SvxBkgTabPage::Reset( const SfxItemSet* )
{
    maSet.Set( *m_pResetSet );
    if ( m_xTblLBox && m_xTblLBox->get_visible() )
    {
        m_nActPos = -1;
        if ( const SfxUInt16Item* pDestItem = m_pResetSet->GetItemIfSet( SID_BACKGRND_DESTINATION, false ) )
        {
            sal_uInt16 nDestValue = pDestItem->GetValue();
            m_xTblLBox->set_active( nDestValue );
            TblDestinationHdl_Impl( *m_xTblLBox );
        }
        m_xTblLBox->save_value();
    }
    SvxAreaTabPage::Reset( &maSet );
}

bool SvxBkgTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    if (m_xTblLBox && m_xTblLBox->get_visible())
        nSlot = lcl_GetTableDestSlot(m_xTblLBox->get_active());
    else if ( bHighlighting )
        nSlot = SID_ATTR_BRUSH_CHAR;
    else if( bCharBackColor )
        nSlot = SID_ATTR_CHAR_BACK_COLOR;

    sal_uInt16 nWhich = GetWhich(nSlot);

    drawing::FillStyle eFillType = maSet.Get( XATTR_FILLSTYLE ).GetValue();
    switch( eFillType )
    {
        case drawing::FillStyle_NONE:
        {
            if ( IsBtnClicked() )
            {
                if ( SID_ATTR_CHAR_BACK_COLOR == nSlot )
                {
                    maSet.Put( SvxColorItem( COL_TRANSPARENT, nWhich ) );
                    rCoreSet->Put( SvxColorItem( COL_TRANSPARENT, nWhich ) );
                }
                else
                {
                    maSet.Put( SvxBrushItem( COL_TRANSPARENT, nWhich ) );
                    rCoreSet->Put( SvxBrushItem( COL_TRANSPARENT, nWhich ) );
                }
            }
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aColorItem( maSet.Get( XATTR_FILLCOLOR ) );
            if ( SID_ATTR_CHAR_BACK_COLOR == nSlot )
            {
                maSet.Put( SvxColorItem( aColorItem.GetColorValue(), nWhich ) );
                rCoreSet->Put( SvxColorItem( aColorItem.GetColorValue(), nWhich ) );
            }
            else
            {
                maSet.Put( SvxBrushItem( aColorItem.GetColorValue(), nWhich ) );
                rCoreSet->Put( SvxBrushItem( aColorItem.GetColorValue(), nWhich ) );
            }
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            std::unique_ptr<SvxBrushItem> aBrushItem( getSvxBrushItemFromSourceSet( maSet, nWhich ) );
            if ( GraphicType::NONE != aBrushItem->GetGraphicObject()->GetType() )
                rCoreSet->Put( std::move(aBrushItem) );
            break;
        }
        default:
            break;
    }

    if (!m_xTblLBox || !m_xTblLBox->get_visible())
        return true;

    if (nSlot != SID_ATTR_BRUSH)
    {
        nWhich = maSet.GetPool()->GetWhich(SID_ATTR_BRUSH);
        if (SfxItemState::SET == maSet.GetItemState(nWhich))
        {
            SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
            rCoreSet->Put(aBrushItem);
        }
    }
    if (nSlot != SID_ATTR_BRUSH_ROW)
    {
        if (SfxItemState::SET == maSet.GetItemState(SID_ATTR_BRUSH_ROW))
        {
            SvxBrushItem aBrushItem(maSet.Get(SID_ATTR_BRUSH_ROW));
            rCoreSet->Put(aBrushItem);
        }
    }
    if (nSlot != SID_ATTR_BRUSH_TABLE)
    {
        if (SfxItemState::SET == maSet.GetItemState(SID_ATTR_BRUSH_TABLE))
        {
            SvxBrushItem aBrushItem(maSet.Get(SID_ATTR_BRUSH_TABLE));
            rCoreSet->Put(aBrushItem);
        }
    }

    if (m_xTblLBox->get_value_changed_from_saved())
    {
        rCoreSet->Put(SfxUInt16Item(SID_BACKGRND_DESTINATION, m_xTblLBox->get_active()));
    }

    return true;
}

std::unique_ptr<SfxTabPage> SvxBkgTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    auto xRet = std::make_unique<SvxBkgTabPage>(pPage, pController, *rAttrs);
    xRet->SetOptimalSize(pController);
    return xRet;
}

void SvxBkgTabPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        SvxBackgroundTabFlags nFlags = static_cast<SvxBackgroundTabFlags>(pFlagItem->GetValue());
        if ( nFlags & SvxBackgroundTabFlags::SHOW_TBLCTL )
        {
            m_xBtnBitmap->show();
            m_xTblLBox = m_xBuilder->weld_combo_box("tablelb");
            m_xTblLBox->connect_changed(LINK(this, SvxBkgTabPage, TblDestinationHdl_Impl));
            m_xTblLBox->show();
        }
        if ((nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING) ||
            (nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR))
        {
            bHighlighting = bool(nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
            bCharBackColor = bool(nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR);
        }
        if (nFlags & SvxBackgroundTabFlags::SHOW_SELECTOR)
            m_xBtnBitmap->show();
        SetOptimalSize(GetDialogController());
    }

    if ( bCharBackColor )
    {
        sal_uInt16 nWhich(maSet.GetPool()->GetWhich(SID_ATTR_CHAR_BACK_COLOR));
        Color aBackColor(static_cast<const SvxColorItem&>(maSet.Get(nWhich)).GetValue());
        SvxBrushItem aBrushItem(SvxBrushItem(aBackColor, SID_ATTR_BRUSH_CHAR));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, maSet);
    }
    else
    {
        sal_uInt16 nWhich(maSet.GetPool()->GetWhich(bHighlighting ? SID_ATTR_BRUSH_CHAR : SID_ATTR_BRUSH));
        SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, maSet);
    }

    m_pResetSet = maSet.Clone();

    SvxAreaTabPage::PageCreated(aSet);
}

IMPL_LINK(SvxBkgTabPage, TblDestinationHdl_Impl, weld::ComboBox&, rBox, void)
{
    if (m_nActPos > -1)
    {
        // fill local item set with XATTR_FILL settings gathered from tab page
        // and convert to SvxBrushItem and store in table destination slot Which
        SvxAreaTabPage::FillItemSet(&maSet);
        maSet.Put(getSvxBrushItemFromSourceSet(maSet, maSet.GetPool()->GetWhich(lcl_GetTableDestSlot(m_nActPos))));
    }

    sal_Int32 nSelPos = rBox.get_active();
    if (m_nActPos == nSelPos)
        return;

    m_nActPos = nSelPos;

    // fill local item set with XATTR_FILL created from SvxBushItem for table destination slot Which
    sal_uInt16 nWhich = maSet.GetPool()->GetWhich(lcl_GetTableDestSlot(nSelPos));
    if (SfxItemState::SET == maSet.GetItemState(nWhich))
    {
        SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(maSet.Get(nWhich)));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, maSet);
    }
    else
    {
        SelectFillType(*m_xBtnNone, &maSet);
        return;
    }

    // show tab page
    drawing::FillStyle eXFS = drawing::FillStyle_NONE;
    if (maSet.GetItemState(XATTR_FILLSTYLE) != SfxItemState::DONTCARE)
    {
        XFillStyleItem aFillStyleItem(maSet.Get(GetWhich( XATTR_FILLSTYLE)));
        eXFS = aFillStyleItem.GetValue();
    }
    switch(eXFS)
    {
        default:
        case drawing::FillStyle_NONE:
        {
            SelectFillType(*m_xBtnNone, &maSet);
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            SelectFillType(*m_xBtnColor, &maSet);
            // color tab page Active and New preview controls are same after SelectFillType
            // hack to restore color tab page Active preview
            setSvxBrushItemAsFillAttributesToTargetSet(static_cast<const SvxBrushItem&>(m_pResetSet->Get(nWhich)), *m_pResetSet);
            static_cast<SvxColorTabPage*>(GetFillTabPage())->SetCtlPreviewOld(*m_pResetSet);
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            SelectFillType(*m_xBtnBitmap, &maSet);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
