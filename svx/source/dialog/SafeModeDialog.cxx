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
#include <comphelper/processfactory.hxx>
#include <sfx2/safemode.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

using namespace css;

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

bool SafeModeDialog::Close()
{
    // Remove the safe mode flag before exiting this dialog
    sfx2::SafeMode::removeFlag();

    return Dialog::Close();
}

void SafeModeDialog::terminateOffice()
{
    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create( comphelper::getProcessComponentContext() );
    xDesktop->terminate();
}

IMPL_LINK(SafeModeDialog, BtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnContinue.get())
    {
        Close();
    }
    else if (pBtn == mpBtnQuit.get())
    {
        terminateOffice();
    }
    else if (pBtn == mpBtnRestart.get())
    {
        Close();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
