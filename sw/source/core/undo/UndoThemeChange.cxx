/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <UndoThemeChange.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/theme/Theme.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>

#include <memory>

namespace sw
{
UndoThemeChange::UndoThemeChange(SwDoc& rDocument,
                                 std::shared_ptr<model::ColorSet> const& pOldColorSet,
                                 std::shared_ptr<model::ColorSet> const& pNewColorSet)
    : SwUndo(SwUndoId::CHANGE_THEME, rDocument)
    , mrDocument(rDocument)
    , mpOldColorSet(pOldColorSet)
    , mpNewColorSet(pNewColorSet)
{
}

UndoThemeChange::~UndoThemeChange() {}

void UndoThemeChange::UndoImpl(UndoRedoContext& /*rUndoRedoContext*/)
{
    SdrModel* pModel = mrDocument.getIDocumentDrawModelAccess().GetDrawModel();
    auto pTheme = pModel->getTheme();
    pTheme->setColorSet(mpOldColorSet);
}

void UndoThemeChange::RedoImpl(UndoRedoContext& /*rUndoRedoContext*/)
{
    SdrModel* pModel = mrDocument.getIDocumentDrawModelAccess().GetDrawModel();
    auto pTheme = pModel->getTheme();
    pTheme->setColorSet(mpNewColorSet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
