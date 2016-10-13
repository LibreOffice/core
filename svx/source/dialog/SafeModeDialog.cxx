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
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

using namespace css;

SafeModeDialog::SafeModeDialog(vcl::Window* pParent)
:   Dialog(pParent, "SafeModeDialog", "svx/ui/safemodedialog.ui"),

    mpBtnContinue(),
    mpBtnQuit(),
    mpBtnRestart(),

    mpCBCheckProfilesafeConfig(),
    mpCBCheckProfilesafeExtensions(),
    mpCBDisableAllExtensions(),
    mpCBResetCustomizations(),
    mpCBResetWholeUserProfile(),

    maBackupFileHelper()
{
    get(mpBtnContinue, "btn_continue");
    get(mpBtnQuit, "btn_quit");
    get(mpBtnRestart, "btn_restart");

    get(mpCBCheckProfilesafeConfig, "check_profilesafe_config");
    get(mpCBCheckProfilesafeExtensions, "check_profilesafe_extensions");
    get(mpCBDisableAllExtensions, "check_disable_all_extensions");
    get(mpCBResetCustomizations, "check_reset_customizations");
    get(mpCBResetWholeUserProfile, "check_reset_whole_userprofile");

    mpBtnContinue->SetClickHdl(LINK(this, SafeModeDialog, BtnHdl));
    mpBtnQuit->SetClickHdl(LINK(this, SafeModeDialog, BtnHdl));
    mpBtnRestart->SetClickHdl(LINK(this, SafeModeDialog, BtnHdl));

    mpCBCheckProfilesafeConfig->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBCheckProfilesafeExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDisableAllExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetCustomizations->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetWholeUserProfile->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));

    // Disable restart btn until some checkbox is active
    mpBtnRestart->Disable();

    if (!maBackupFileHelper.isPopPossible())
    {
        mpCBCheckProfilesafeConfig->Disable();
    }

    if (!maBackupFileHelper.isPopPossibleExtensionInfo())
    {
        mpCBCheckProfilesafeExtensions->Disable();
    }

    if (comphelper::BackupFileHelper::isTryDisableAllExtensionsPossible())
    {
        mpCBDisableAllExtensions->Disable();
    }

    if (maBackupFileHelper.isTryResetCustomizationsPossible())
    {
        mpCBResetCustomizations->Disable();
    }
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

    mpCBCheckProfilesafeConfig.clear();
    mpCBCheckProfilesafeExtensions.clear();
    mpCBDisableAllExtensions.clear();
    mpCBResetCustomizations.clear();
    mpCBResetWholeUserProfile.clear();

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
    // We are not hitting Close() in this case, need to manually remove the flag
    sfx2::SafeMode::removeFlag();

    uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create( comphelper::getProcessComponentContext() );
    xDesktop->terminate();
}

void SafeModeDialog::applyChanges()
{
    if (mpCBCheckProfilesafeConfig->IsChecked())
    {
        // reset UserConfiguration to last known working state
        // ProfileSafeMode/BackupFileHelper
        maBackupFileHelper.tryPop();
    }

    if (mpCBCheckProfilesafeExtensions->IsChecked())
    {
        // reset State of installed Extensions to last known working state
        // ProfileSafeMode/BackupFileHelper
        maBackupFileHelper.tryPopExtensionInfo();
    }

    if (mpCBDisableAllExtensions->IsChecked())
    {
        // Disable all extensions
        comphelper::BackupFileHelper::tryDisableAllExtensions();
    }

    if (mpCBResetCustomizations->IsChecked())
    {
        // Reset customizations (Settings and UserInterface modifications)
        maBackupFileHelper.tryResetCustomizations();
    }

    if (mpCBResetWholeUserProfile->IsChecked())
    {
        // Reset the whole UserProfile
        maBackupFileHelper.tryResetUserProfile();
    }

    // Then restart
    css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
        css::uno::Reference< css::task::XInteractionHandler >());
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
        applyChanges();
    }
}

IMPL_LINK(SafeModeDialog, CheckBoxHdl, CheckBox&, /*pCheckBox*/, void)
{
    const bool bEnable(
        mpCBCheckProfilesafeConfig->IsChecked() ||
        mpCBCheckProfilesafeExtensions->IsChecked() ||
        mpCBDisableAllExtensions->IsChecked() ||
        mpCBResetCustomizations->IsChecked() ||
        mpCBResetWholeUserProfile->IsChecked());

    mpBtnRestart->Enable(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
