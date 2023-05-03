/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <undobj.hxx>
#include <memory>
#include <docmodel/theme/ColorSet.hxx>

namespace sw
{
class UndoThemeChange final : public SwUndo
{
private:
    SwDoc& mrDocument;
    std::shared_ptr<model::ColorSet> mpOldColorSet;
    std::shared_ptr<model::ColorSet> mpNewColorSet;

public:
    UndoThemeChange(SwDoc& rDocument, std::shared_ptr<model::ColorSet> const& pOld,
                    std::shared_ptr<model::ColorSet> const& pNew);
    virtual ~UndoThemeChange() override;

    virtual void UndoImpl(UndoRedoContext& rUndoRedoContext) override;
    virtual void RedoImpl(UndoRedoContext& rUndoRedoContext) override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
