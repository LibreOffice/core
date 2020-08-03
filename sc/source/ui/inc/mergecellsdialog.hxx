/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/weld.hxx>

enum ScMergeCellsOption
{
    MoveContentHiddenCells,
    KeepContentHiddenCells,
    EmptyContentHiddenCells
};

class ScMergeCellsDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::RadioButton> m_xRBMoveContent;
    std::unique_ptr<weld::RadioButton> m_xRBKeepContent;
    std::unique_ptr<weld::RadioButton> m_xRBEmptyContent;

public:
    ScMergeCellsDialog(weld::Window* pParent);
    virtual ~ScMergeCellsDialog() override;

    ScMergeCellsOption GetMergeCellsOption() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
