/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_MERGECELLSDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_MERGECELLSDIALOG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

enum ScMergeCellsOption
{
    MoveContentHiddenCells,
    KeepContentHiddenCells,
    EmptyContentHiddenCells
};

class ScMergeCellsDialog : public ModalDialog
{
    VclPtr<RadioButton> mpRBMoveContent;
    VclPtr<RadioButton> mpRBKeepContent;
    VclPtr<RadioButton> mpRBEmptyContent;

public:
    ScMergeCellsDialog( vcl::Window * pParent );
    virtual ~ScMergeCellsDialog();
    virtual void dispose() override;

    ScMergeCellsOption GetMergeCellsOption();

protected:
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
