/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <sdundo.hxx>
#include <svx/svdpage.hxx>
#include <docmodel/theme/Theme.hxx>

class SdDrawDocument;

namespace sd
{
class UndoThemeChange : public SdUndoAction
{
private:
    SdrPage* mpMasterPage;
    std::shared_ptr<model::ColorSet> mpOldColorSet;
    std::shared_ptr<model::ColorSet> mpNewColorSet;

public:
    UndoThemeChange(SdDrawDocument& rDocument, SdrPage* pMasterPage,
                    std::shared_ptr<model::ColorSet> const& pOldColorSet,
                    std::shared_ptr<model::ColorSet> const& pNewColorSet);

    virtual void Undo() override;
    virtual void Redo() override;
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
