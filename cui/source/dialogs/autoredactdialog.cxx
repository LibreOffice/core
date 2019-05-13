/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <autoredactdialog.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

AutoRedactDialog::AutoRedactDialog(vcl::Window* pParent)
    : SfxModalDialog(pParent, "AutoRedactDialog", "cui/ui/autoredactdialog.ui")
{
    //TODO: Implement
}

AutoRedactDialog::~AutoRedactDialog() { disposeOnce(); }

void AutoRedactDialog::dispose() { SfxModalDialog::dispose(); }

bool AutoRedactDialog::Close()
{
    EndDialog(RET_OK);
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
