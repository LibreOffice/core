/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <UndoThemeChange.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

namespace sd
{
UndoThemeChange::UndoThemeChange(SdDrawDocument& rDocument, SdrPage* pMasterPage,
                                 std::shared_ptr<model::ColorSet> const& pOldColorSet,
                                 std::shared_ptr<model::ColorSet> const& pNewColorSet)
    : SdUndoAction(rDocument)
    , mpMasterPage(pMasterPage)
    , mpOldColorSet(pOldColorSet)
    , mpNewColorSet(pNewColorSet)
{
    SetComment(SvxResId(RID_SVXSTR_UNDO_THEME_COLOR_CHANGE));
}

namespace
{
std::shared_ptr<model::Theme> getTheme(SdrPage* pMasterPage)
{
    auto pTheme = pMasterPage->getSdrPageProperties().getTheme();
    if (!pTheme)
    {
        pTheme = std::make_shared<model::Theme>("Office");
        pMasterPage->getSdrPageProperties().setTheme(pTheme);
    }
    return pTheme;
}
}

void UndoThemeChange::Undo()
{
    auto pTheme = getTheme(mpMasterPage);
    pTheme->setColorSet(mpOldColorSet);
}

void UndoThemeChange::Redo()
{
    auto pTheme = getTheme(mpMasterPage);
    pTheme->setColorSet(mpNewColorSet);
}

} // end sd namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
