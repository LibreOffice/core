/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "MergeCellsDialog.hxx"

ScMergeCellsDialog::ScMergeCellsDialog( vcl::Window * pParent )
            : ModalDialog( pParent, "MergeCellsDialog",
               "modules/scalc/ui/mergecellsdialog.ui" )
{
    get(mpRBMoveContent, "move-cells-radio");
    get(mpRBKeepContent, "keep-content-radio");
    get(mpRBEmptyContent, "empty-cells-radio");

    mpRBKeepContent->Check();
}

ScMergeCellsDialog::~ScMergeCellsDialog()
{
    disposeOnce();
}

void ScMergeCellsDialog::dispose()
{
    mpRBMoveContent.disposeAndClear();
    mpRBKeepContent.disposeAndClear();
    mpRBEmptyContent.disposeAndClear();
    ModalDialog::dispose();
}

ScMergeCellsOption ScMergeCellsDialog::GetMergeCellsOption()
{
    if ( mpRBMoveContent->IsChecked() )
        return MoveContentHiddenCells;
    if ( mpRBKeepContent->IsChecked() )
        return KeepContentHiddenCells;
    if ( mpRBEmptyContent->IsChecked() )
        return EmptyContentHiddenCells;
    OSL_FAIL("Unknown selection for merge cells.");
    return KeepContentHiddenCells; // default value

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
