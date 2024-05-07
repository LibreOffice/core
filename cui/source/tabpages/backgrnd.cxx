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

const WhichRangesContainer SvxBkgTabPage::pBkgRanges(svl::Items<
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
    m_aAttrSet(*rInAttrs.GetPool(),
               rInAttrs.GetRanges().MergeRange(XATTR_FILL_FIRST, XATTR_FILL_LAST))
{
    m_xBtnGradient->hide();
    m_xBtnHatch->hide();
    m_xBtnBitmap->hide();
    m_xBtnPattern->hide();
}

SvxBkgTabPage::~SvxBkgTabPage()
{
    m_xTblLBox.reset();
}

void SvxBkgTabPage::ActivatePage( const SfxItemSet& )
{
    SvxAreaTabPage::ActivatePage(m_aAttrSet);
}

DeactivateRC SvxBkgTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if (DeactivateRC::KeepPage == SvxAreaTabPage::DeactivatePage(&m_aAttrSet))
        return DeactivateRC::KeepPage;

    if ( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

void SvxBkgTabPage::Reset(const SfxItemSet* pItemSet)
{
    if (m_xTblLBox && m_xTblLBox->get_visible())
    {
        if (m_nActPos == -1) // initial reset
        {
            m_nActPos = 0;
            if (const SfxUInt16Item* pDestItem = pItemSet->GetItemIfSet(SID_BACKGRND_DESTINATION, false))
                m_nActPos = pDestItem->GetValue();
            m_xTblLBox->set_active(m_nActPos);
        }
        SetActiveTableDestinationBrushItem();
        return;
    }
    else if (m_bCharBackColor)
    {
        sal_uInt16 nWhich(pItemSet->GetPool()->GetWhichIDFromSlotID(SID_ATTR_CHAR_BACK_COLOR));
        Color aBackColor(static_cast<const SvxColorItem&>(pItemSet->Get(nWhich)).GetValue());
        SvxBrushItem aBrushItem(SvxBrushItem(aBackColor, SID_ATTR_BRUSH_CHAR));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, m_aAttrSet);
    }
    else
    {
        sal_uInt16 nWhich(pItemSet->GetPool()->GetWhichIDFromSlotID(m_bHighlighting ? SID_ATTR_BRUSH_CHAR : SID_ATTR_BRUSH));
        SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(pItemSet->Get(nWhich)));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, m_aAttrSet);
    }
    SvxAreaTabPage::Reset(&m_aAttrSet);
}

bool SvxBkgTabPage::FillItemSet(SfxItemSet* pCoreSet)
{
    sal_uInt16 nSlot = SID_ATTR_BRUSH;
    if (m_xTblLBox && m_xTblLBox->get_visible())
        nSlot = lcl_GetTableDestSlot(m_xTblLBox->get_active());
    else if (m_bHighlighting)
        nSlot = SID_ATTR_BRUSH_CHAR;
    else if (m_bCharBackColor)
        nSlot = SID_ATTR_CHAR_BACK_COLOR;

    sal_uInt16 nWhich = GetWhich(nSlot);

    drawing::FillStyle eFillType = m_aAttrSet.Get(XATTR_FILLSTYLE).GetValue();
    switch( eFillType )
    {
        case drawing::FillStyle_NONE:
        {
            if ( IsBtnClicked() )
            {
                if ( SID_ATTR_CHAR_BACK_COLOR == nSlot )
                {
                    pCoreSet->Put(SvxColorItem(COL_TRANSPARENT, nWhich));
                }
                else
                {
                    pCoreSet->Put(SvxBrushItem(COL_TRANSPARENT, nWhich));
                }
            }
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            XFillColorItem aColorItem(m_aAttrSet.Get(XATTR_FILLCOLOR));

            // tdf#157801 - don't change direct formatting when color is unchanged
            if (const SfxPoolItem* pOldItem = GetOldItem(*pCoreSet, nSlot))
            {
                const SvxBrushItem* pOldBrushItem = static_cast<const SvxBrushItem*>(pOldItem);
                if (pOldBrushItem->GetColor() == aColorItem.GetColorValue())
                    break;
            }

            if ( SID_ATTR_CHAR_BACK_COLOR == nSlot )
            {
                pCoreSet->Put(SvxColorItem(aColorItem.GetColorValue(), aColorItem.getComplexColor(), nWhich));
            }
            else
            {
                pCoreSet->Put(SvxBrushItem(aColorItem.GetColorValue(), aColorItem.getComplexColor(), nWhich));
            }
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            std::unique_ptr<SvxBrushItem> aBrushItem(getSvxBrushItemFromSourceSet(m_aAttrSet, nWhich));
            if ( GraphicType::NONE != aBrushItem->GetGraphicObject()->GetType() )
                pCoreSet->Put(std::move(aBrushItem));
            break;
        }
        default:
            break;
    }

    if (!m_xTblLBox || !m_xTblLBox->get_visible())
        return true;

    pCoreSet->Put(SfxUInt16Item(SID_BACKGRND_DESTINATION, m_xTblLBox->get_active()));

    // *Put* in the core set all table brushes that are *SET* in the m_aAttrSet
    if (nSlot != SID_ATTR_BRUSH)
    {
        nWhich = m_aAttrSet.GetPool()->GetWhichIDFromSlotID(SID_ATTR_BRUSH);
        if (SfxItemState::SET == m_aAttrSet.GetItemState(nWhich))
        {
            SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(m_aAttrSet.Get(nWhich)));
            pCoreSet->Put(aBrushItem);
        }
    }
    if (nSlot != SID_ATTR_BRUSH_ROW)
    {
        if (SfxItemState::SET == m_aAttrSet.GetItemState(SID_ATTR_BRUSH_ROW))
        {
            SvxBrushItem aBrushItem(m_aAttrSet.Get(SID_ATTR_BRUSH_ROW));
            pCoreSet->Put(aBrushItem);
        }
    }
    if (nSlot != SID_ATTR_BRUSH_TABLE)
    {
        if (SfxItemState::SET == m_aAttrSet.GetItemState(SID_ATTR_BRUSH_TABLE))
        {
            SvxBrushItem aBrushItem(m_aAttrSet.Get(SID_ATTR_BRUSH_TABLE));
            pCoreSet->Put(aBrushItem);
        }
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
            m_xTblLBox = m_xBuilder->weld_combo_box(u"tablelb"_ustr);
            m_xTblLBox->connect_changed(LINK(this, SvxBkgTabPage, TblDestinationHdl_Impl));
            m_xTblLBox->show();
        }
        if ((nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING) ||
            (nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR))
        {
            m_bHighlighting = bool(nFlags & SvxBackgroundTabFlags::SHOW_HIGHLIGHTING);
            m_bCharBackColor = bool(nFlags & SvxBackgroundTabFlags::SHOW_CHAR_BKGCOLOR);
        }
        if (nFlags & SvxBackgroundTabFlags::SHOW_SELECTOR)
            m_xBtnBitmap->show();
        SetOptimalSize(GetDialogController());
    }

    SfxObjectShell* pObjSh = SfxObjectShell::Current();

    // we always have the color page
    XColorListRef xColorTable;
    if (pObjSh)
    {
        const SvxColorListItem* pItem = pObjSh->GetItem(SID_COLOR_TABLE);
        if (pItem)
            xColorTable = pItem->GetColorList();
    }
    if (!xColorTable.is())
        xColorTable = XColorList::CreateStdColorList();
    SetColorList(xColorTable);

    // sometimes we have the bitmap page
    if (m_xBtnBitmap->get_visible())
    {
        XBitmapListRef xBitmapList;
        if (pObjSh)
            if (const SvxBitmapListItem* pItem = pObjSh->GetItem(SID_BITMAP_LIST))
                xBitmapList = pItem->GetBitmapList();
        SetBitmapList(xBitmapList);
    }
}

IMPL_LINK(SvxBkgTabPage, TblDestinationHdl_Impl, weld::ComboBox&, rBox, void)
{
    sal_Int32 nSelPos = rBox.get_active();

    if (m_nActPos == nSelPos)
        return;

    // Fill the local item set with XATTR_FILL settings gathered from the tab page, convert to
    // SvxBrushItem and store in table destination slot Which. Do this so cell, row, and table
    // brush items can be set together.
    SvxAreaTabPage::FillItemSet(&m_aAttrSet);
    m_aAttrSet.Put(getSvxBrushItemFromSourceSet(m_aAttrSet, GetWhich(lcl_GetTableDestSlot(m_nActPos))));

    m_nActPos = nSelPos;

    SetActiveTableDestinationBrushItem();
}

void SvxBkgTabPage::SetActiveTableDestinationBrushItem()
{
    // set the table destination (cell, row, table) brush item as a fill item in the local item set
    sal_uInt16 nWhich = GetWhich(lcl_GetTableDestSlot(m_nActPos));
    if (SfxItemState::SET == GetItemSet().GetItemState(nWhich))
    {
        SvxBrushItem aBrushItem(static_cast<const SvxBrushItem&>(GetItemSet().Get(nWhich)));
        setSvxBrushItemAsFillAttributesToTargetSet(aBrushItem, m_aAttrSet);
    }
    else
    {
        SelectFillType(*m_xBtnNone, &m_aAttrSet);
        return;
    }

    XFillStyleItem aFillStyleItem(m_aAttrSet.Get(m_aAttrSet.GetPool()->GetWhichIDFromSlotID(XATTR_FILLSTYLE)));
    drawing::FillStyle eXFS = aFillStyleItem.GetValue();
    switch(eXFS)
    {
        default:
        case drawing::FillStyle_NONE:
        {
            SelectFillType(*m_xBtnNone, &m_aAttrSet);
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            SelectFillType(*m_xBtnColor, &m_aAttrSet);
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            SelectFillType(*m_xBtnBitmap, &m_aAttrSet);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
