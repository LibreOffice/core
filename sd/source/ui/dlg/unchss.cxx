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

#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/hint.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpool.hxx>
#include <tools/debug.hxx>

#include <unchss.hxx>

#include <strings.hrc>
#include <glob.hxx>
#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <stlsheet.hxx>
#include <strings.hxx>

StyleSheetUndoAction::StyleSheetUndoAction(SdDrawDocument& rTheDoc,
                                           SfxStyleSheet& rTheStyleSheet,
                                           const SfxItemSet* pTheNewItemSet)
    : SdUndoAction(&rTheDoc),
      mrStyleSheet(rTheStyleSheet)
{
    // Create ItemSets; Attention, it is possible that the new one is from a,
    // different pool. Therefore we clone it with its items.
    mpNewSet = std::make_unique<SfxItemSet>(static_cast<SfxItemPool&>(SdrObject::GetGlobalDrawObjectItemPool()), pTheNewItemSet->GetRanges());
    SdrModel::MigrateItemSet( pTheNewItemSet, mpNewSet.get(), rTheDoc );

    mpOldSet = std::make_unique<SfxItemSet>(static_cast<SfxItemPool&>(SdrObject::GetGlobalDrawObjectItemPool()), mrStyleSheet.GetItemSet().GetRanges());
    SdrModel::MigrateItemSet( &mrStyleSheet.GetItemSet(), mpOldSet.get(), rTheDoc );

    OUString aComment(SdResId(STR_UNDO_CHANGE_PRES_OBJECT));
    OUString aName(mrStyleSheet.GetName());

    // delete layout name and separator
    sal_Int32 nPos = aName.indexOf(SD_LT_SEPARATOR);
    if (nPos != -1)
        aName = aName.copy(nPos + SD_LT_SEPARATOR.getLength());

    if (aName == STR_LAYOUT_TITLE)
    {
        aName = SdResId(STR_PSEUDOSHEET_TITLE);
    }
    else if (aName == STR_LAYOUT_SUBTITLE)
    {
        aName = SdResId(STR_PSEUDOSHEET_SUBTITLE);
    }
    else if (aName == STR_LAYOUT_BACKGROUND)
    {
        aName = SdResId(STR_PSEUDOSHEET_BACKGROUND);
    }
    else if (aName == STR_LAYOUT_BACKGROUNDOBJECTS)
    {
        aName = SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS);
    }
    else if (aName == STR_LAYOUT_NOTES)
    {
        aName = SdResId(STR_PSEUDOSHEET_NOTES);
    }
    else
    {
        OUString aOutlineStr(SdResId(STR_PSEUDOSHEET_OUTLINE));
        nPos = aName.indexOf(aOutlineStr);
        if (nPos != -1)
        {
            std::u16string_view aNumStr(aName.subView(aOutlineStr.getLength()));
            aName = STR_LAYOUT_OUTLINE + aNumStr;
        }
    }

    // replace placeholder with template name
    SetComment(aComment.replaceFirst("$", aName));
}

void StyleSheetUndoAction::Undo()
{
    SfxItemSet aNewSet( mpDoc->GetItemPool(), mpOldSet->GetRanges() );
    SdrModel::MigrateItemSet( mpOldSet.get(), &aNewSet, *mpDoc );

    mrStyleSheet.GetItemSet().Set(aNewSet);
    if( mrStyleSheet.GetFamily() == SfxStyleFamily::Pseudo )
        static_cast<SdStyleSheet&>(mrStyleSheet).GetRealStyleSheet()->Broadcast(SfxHint(SfxHintId::DataChanged));
    else
        mrStyleSheet.Broadcast(SfxHint(SfxHintId::DataChanged));
}

void StyleSheetUndoAction::Redo()
{
    SfxItemSet aNewSet( mpDoc->GetItemPool(), mpOldSet->GetRanges() );
    SdrModel::MigrateItemSet( mpNewSet.get(), &aNewSet, *mpDoc );

    mrStyleSheet.GetItemSet().Set(aNewSet);
    if( mrStyleSheet.GetFamily() == SfxStyleFamily::Pseudo )
        static_cast<SdStyleSheet&>(mrStyleSheet).GetRealStyleSheet()->Broadcast(SfxHint(SfxHintId::DataChanged));
    else
        mrStyleSheet.Broadcast(SfxHint(SfxHintId::DataChanged));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
