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
#include <undoback.hxx>

#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>

#include <com/sun/star/drawing/FillStyle.hpp>

#include <svl/itemset.hxx>

#include <svx/xfillit0.hxx>

SdBackgroundObjUndoAction::SdBackgroundObjUndoAction(
    SdDrawDocument& rDoc,
    SdPage& rPage,
    const SfxItemSet& rItemSet)
:   SdUndoAction(&rDoc),
    mrPage(rPage),
    mpItemSet(std::make_unique<SfxItemSet>(rItemSet)),
    mbHasFillBitmap(false)
{
    OUString aString( SdResId( STR_UNDO_CHANGE_PAGEFORMAT ) );
    SetComment( aString );
    saveFillBitmap(*mpItemSet);
}

void SdBackgroundObjUndoAction::ImplRestoreBackgroundObj()
{
    std::unique_ptr<SfxItemSet> pNew = std::make_unique<SfxItemSet>(mrPage.getSdrPageProperties().GetItemSet());
    mrPage.getSdrPageProperties().ClearItem();
    if (bool(mpFillBitmapItem))
        restoreFillBitmap(*mpItemSet);
    mpFillBitmapItem.reset();
    mbHasFillBitmap = false;
    mrPage.getSdrPageProperties().PutItemSet(*mpItemSet);
    mpItemSet = std::move(pNew);
    saveFillBitmap(*mpItemSet);

    // tell the page that it's visualization has changed
    mrPage.ActionChanged();
}

void SdBackgroundObjUndoAction::Undo()
{
    ImplRestoreBackgroundObj();
}

void SdBackgroundObjUndoAction::Redo()
{
    ImplRestoreBackgroundObj();
}

SdUndoAction* SdBackgroundObjUndoAction::Clone() const
{
    std::unique_ptr<SdBackgroundObjUndoAction> pCopy = std::make_unique<SdBackgroundObjUndoAction>(*mpDoc, mrPage, *mpItemSet);
    if (mpFillBitmapItem)
        pCopy->mpFillBitmapItem.reset(mpFillBitmapItem->Clone());
    pCopy->mbHasFillBitmap = mbHasFillBitmap;
    return pCopy.release();
}

void SdBackgroundObjUndoAction::saveFillBitmap(SfxItemSet &rItemSet)
{
    const SfxPoolItem *pItem = nullptr;
    if (rItemSet.GetItemState(XATTR_FILLBITMAP, false, &pItem) == SfxItemState::SET)
        mpFillBitmapItem.reset(pItem->Clone());
    if (bool(mpFillBitmapItem))
    {
        if (rItemSet.GetItemState(XATTR_FILLSTYLE, false, &pItem) == SfxItemState::SET)
            mbHasFillBitmap = static_cast<const XFillStyleItem*>(pItem)->GetValue() == css::drawing::FillStyle_BITMAP;
        rItemSet.ClearItem(XATTR_FILLBITMAP);
        if (mbHasFillBitmap)
            rItemSet.ClearItem(XATTR_FILLSTYLE);
    }
}

void SdBackgroundObjUndoAction::restoreFillBitmap(SfxItemSet &rItemSet)
{
    rItemSet.Put(*mpFillBitmapItem);
    if (mbHasFillBitmap)
        rItemSet.Put(XFillStyleItem(css::drawing::FillStyle_BITMAP));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
