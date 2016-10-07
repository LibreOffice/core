/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SafeModeDialog.hxx"

#include <config_folders.h>

#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>

SafeModeDialog::SafeModeDialog(vcl::Window* pParent):
    Dialog(pParent, "SafeModeDialog", "svx/ui/safemodedialog.ui")
{
    get(mpBtnContinue, "btn_continue");
    get(mpBtnQuit, "btn_quit");
    get(mpBtnRestart, "btn_restart");
    get(mpCBCustomizations, "check_customizations");
    get(mpCBExtensions, "check_extensions");
    get(mpCBFull, "check_full");

    mpBtnContinue->SetClickHdl(LINK(this, SafeModeDialog, BtnHdl));
    mpBtnQuit->SetClickHdl(LINK(this, SafeModeDialog, BtnHdl));
    mpBtnRestart->SetClickHdl(LINK(this, SafeModeDialog, BtnHdl));
}

SafeModeDialog::~SafeModeDialog()
{
    disposeOnce();
}

void SafeModeDialog::dispose()
{
    mpBtnContinue.clear();
    mpBtnQuit.clear();
    mpBtnRestart.clear();
    mpCBCustomizations.clear();
    mpCBExtensions.clear();
    mpCBFull.clear();

    Dialog::dispose();
}

IMPL_LINK(SafeModeDialog, BtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnContinue.get())
    {
        Close();
    }
    else if (pBtn == mpBtnQuit.get())
    {
        Close();
    }
    else if (pBtn == mpBtnRestart.get())
    {
        Close();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
