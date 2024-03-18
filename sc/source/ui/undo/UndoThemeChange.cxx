/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <undo/UndoThemeChange.hxx>
#include <docmodel/theme/Theme.hxx>
#include <scresid.hxx>
#include <globstr.hrc>

namespace sc
{
UndoThemeChange::UndoThemeChange(ScDocShell& rDocShell,
                                 std::shared_ptr<model::ColorSet> const& pOldColorSet,
                                 std::shared_ptr<model::ColorSet> const& pNewColorSet)
    : ScSimpleUndo(&rDocShell)
    , mpOldColorSet(pOldColorSet)
    , mpNewColorSet(pNewColorSet)
{
}

UndoThemeChange::~UndoThemeChange() = default;

namespace
{
std::shared_ptr<model::Theme> getTheme(ScDocShell& rDocShell)
{
    ScDrawLayer* pModel = rDocShell.GetDocument().GetDrawLayer();

    auto pTheme = pModel->getTheme();
    if (!pTheme)
    {
        pTheme = std::make_shared<model::Theme>("Office");
        pModel->setTheme(pTheme);
    }
    return pTheme;
}
}

void UndoThemeChange::Undo()
{
    BeginUndo();

    auto pTheme = getTheme(*pDocShell);
    pTheme->setColorSet(mpOldColorSet);

    EndUndo();
}

void UndoThemeChange::Redo()
{
    BeginUndo();

    auto pTheme = getTheme(*pDocShell);
    pTheme->setColorSet(mpNewColorSet);

    EndRedo();
}

void UndoThemeChange::Repeat(SfxRepeatTarget& /*rTarget*/) {}

bool UndoThemeChange::CanRepeat(SfxRepeatTarget& /*rTarget*/) const { return false; }

OUString UndoThemeChange::GetComment() const { return ScResId(STR_UNDO_THEME_CHANGE); }

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
