/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/AdditionsDialogHelper.hxx>
#include <vcl/abstdlg.hxx>

void AdditionsDialogHelper::RunAdditionsDialog(weld::Window* pParent, const OUString& rAdditionsTag)
{
    VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
    VclPtr<AbstractAdditionsDialog> pDialog = pFact->CreateAdditionsDialog(pParent, rAdditionsTag);
    pDialog->StartExecuteAsync(
        [pDialog](sal_Int32 /*nResult*/) -> void { pDialog->disposeOnce(); });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
