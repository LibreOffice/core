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
#include <sfx2/safemode.hxx>
#include <vcl/svapp.hxx>
#include <vcl/layout.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ZipPackageHelper.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/configmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

using namespace css;

SafeModeDialog::SafeModeDialog(vcl::Window* pParent)
:   Dialog(pParent, "SafeModeDialog", "svx/ui/safemodedialog.ui"),

    mpBtnContinue(),
    mpBtnQuit(),
    mpBtnRestart(),

    mpBoxRestore(),
    mpBoxConfigure(),
    mpBoxReset(),

    mpRadioRestore(),
    mpRadioConfigure(),
    mpRadioReset(),

    mpCBCheckProfilesafeConfig(),
    mpCBCheckProfilesafeExtensions(),
    mpCBDisableAllExtensions(),
    mpCBDeinstallUserExtensions(),
    mpCBDeinstallAllExtensions(),
    mpCBDisableHWAcceleration(),
    mpCBResetCustomizations(),
    mpCBResetWholeUserProfile(),

    maBackupFileHelper()
{
    get(mpBtnContinue, "btn_continue");
    get(mpBtnQuit, "btn_quit");
    get(mpBtnRestart, "btn_restart");

    get(mpBoxRestore, "group_restore");
    get(mpBoxConfigure, "group_configure");
    get(mpBoxReset, "group_reset");

    get(mpRadioRestore, "radio_restore");
    get(mpRadioConfigure, "radio_configure");
    get(mpRadioReset, "radio_reset");

    get(mpCBCheckProfilesafeConfig, "check_profilesafe_config");
    get(mpCBCheckProfilesafeExtensions, "check_profilesafe_extensions");
    get(mpCBDisableAllExtensions, "check_disable_all_extensions");
    get(mpCBDeinstallUserExtensions, "check_deinstall_user_extensions");
    get(mpCBDeinstallAllExtensions, "check_deinstall_all_extensions");
    get(mpCBDisableHWAcceleration, "check_disable_hw_acceleration");
    get(mpCBResetCustomizations, "check_reset_customizations");
    get(mpCBResetWholeUserProfile, "check_reset_whole_userprofile");

    get(mpBugLink, "linkbutton_bugs");
    get(mpUserProfileLink, "linkbutton_profile");
    get(mpBtnCreateZip, "btn_create_zip");

    mpRadioRestore->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));
    mpRadioConfigure->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));
    mpRadioReset->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));

    mpBtnContinue->SetClickHdl(LINK(this, SafeModeDialog, DialogBtnHdl));
    mpBtnQuit->SetClickHdl(LINK(this, SafeModeDialog, DialogBtnHdl));
    mpBtnRestart->SetClickHdl(LINK(this, SafeModeDialog, DialogBtnHdl));

    mpCBCheckProfilesafeConfig->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBCheckProfilesafeExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDisableAllExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDeinstallUserExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDeinstallAllExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDisableHWAcceleration->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetCustomizations->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetWholeUserProfile->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));

    mpBtnCreateZip->SetClickHdl(LINK(this, SafeModeDialog, CreateZipBtnHdl));

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

    if (!comphelper::BackupFileHelper::isTryDisableAllExtensionsPossible())
    {
        mpCBDisableAllExtensions->Disable();
    }

    if (!comphelper::BackupFileHelper::isTryDeinstallUserExtensionsPossible())
    {
        mpCBDeinstallUserExtensions->Disable();
    }

    if (!comphelper::BackupFileHelper::isTryDeinstallAllExtensionsPossible())
    {
        mpCBDeinstallAllExtensions->Disable();
    }

    if (!comphelper::BackupFileHelper::isTryResetCustomizationsPossible())
    {
        mpCBResetCustomizations->Disable();
    }
    // no disabe of mpCBResetWholeUserProfile, always possible (as last choice)

    // Check the first radio button and disable the other parts
    mpRadioRestore->Check();
    mpBoxConfigure->Disable();
    mpBoxReset->Disable();

    // Set URL for help button (module=safemode)
    OUString sURL("http://hub.libreoffice.org/send-feedback/?LOversion=" + utl::ConfigManager::getAboutBoxProductVersion() +
        "&LOlocale=" + utl::ConfigManager::getLocale() + "&LOmodule=safemode");
    mpBugLink->SetURL(sURL);

    mpUserProfileLink->SetURL(comphelper::BackupFileHelper::getUserProfileURL());
}

SafeModeDialog::~SafeModeDialog()
{
    disposeOnce();
}

void SafeModeDialog::dispose()
{
    mpRadioRestore.clear();
    mpRadioConfigure.clear();
    mpRadioReset.clear();

    mpBoxRestore.clear();
    mpBoxConfigure.clear();
    mpBoxReset.clear();

    mpBtnContinue.clear();
    mpBtnQuit.clear();
    mpBtnRestart.clear();

    mpCBCheckProfilesafeConfig.clear();
    mpCBCheckProfilesafeExtensions.clear();
    mpCBDisableAllExtensions.clear();
    mpCBDeinstallUserExtensions.clear();
    mpCBDeinstallAllExtensions.clear();
    mpCBDisableHWAcceleration.clear();
    mpCBResetCustomizations.clear();
    mpCBResetWholeUserProfile.clear();

    mpBugLink.clear();
    mpUserProfileLink.clear();
    mpBtnCreateZip.clear();

    Dialog::dispose();
}

bool SafeModeDialog::Close()
{
    // Remove the safe mode flag before exiting this dialog
    sfx2::SafeMode::removeFlag();

    return Dialog::Close();
}

void SafeModeDialog::applyChanges()
{
    /// Restore
    if (mpRadioRestore->IsChecked())
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
    }

    // Configure
    if (mpRadioConfigure->IsChecked())
    {
        if (mpCBDisableAllExtensions->IsChecked())
        {
            // Disable all extensions
            comphelper::BackupFileHelper::tryDisableAllExtensions();
        }

        if (mpCBDeinstallUserExtensions->IsChecked())
        {
            // Deinstall all User Extensions (installed for User only)
            comphelper::BackupFileHelper::tryDeinstallUserExtensions();
        }

        if (mpCBDeinstallAllExtensions->IsChecked())
        {
            // Deinstall all Extensions (user|shared|bundled)
            comphelper::BackupFileHelper::tryDeinstallAllExtensions();
        }

        if (mpCBDisableHWAcceleration->IsChecked())
        {
            comphelper::BackupFileHelper::tryDisableHWAcceleration();
        }
    }

    // Reset
    if (mpRadioReset->IsChecked())
    {
        if (mpCBResetCustomizations->IsChecked())
        {
            // Reset customizations (Settings and UserInterface modifications)
            comphelper::BackupFileHelper::tryResetCustomizations();
        }

        if (mpCBResetWholeUserProfile->IsChecked())
        {
            // Reset the whole UserProfile
            comphelper::BackupFileHelper::tryResetUserProfile();
        }
    }

    // Then restart
    css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
        css::uno::Reference< css::task::XInteractionHandler >());
}

IMPL_LINK(SafeModeDialog, RadioBtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpRadioConfigure.get())
    {
        mpBoxConfigure->Enable();
        mpBoxRestore->Disable();
        mpBoxReset->Disable();
    }
    else if (pBtn == mpRadioReset.get())
    {
        mpBoxReset->Enable();
        mpBoxConfigure->Disable();
        mpBoxRestore->Disable();
    }
    else if (pBtn == mpRadioRestore.get())
    {
        mpBoxRestore->Enable();
        mpBoxReset->Disable();
        mpBoxConfigure->Disable();
    }
}

IMPL_LINK(SafeModeDialog, DialogBtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnContinue.get())
    {
        Close();
    }
    else if (pBtn == mpBtnQuit.get())
    {
        Close();
        Application::Quit();
    }
    else if (pBtn == mpBtnRestart.get())
    {
        Close();
        applyChanges();
    }
}

namespace {
    class ProfileExportedDialog : public ModalDialog
    {
    private:
        DECL_LINK(OpenHdl, Button*, void);
    public:
        explicit ProfileExportedDialog();
    };

    ProfileExportedDialog::ProfileExportedDialog()
        : ModalDialog(nullptr, "SafeModeQueryDialog", "svx/ui/profileexporteddialog.ui")
    {
        get<Button>("openfolder")->SetClickHdl(LINK(this, ProfileExportedDialog, OpenHdl));
    }

    IMPL_LINK_NOARG(ProfileExportedDialog, OpenHdl, Button*, void)
    {
        const OUString uri(comphelper::BackupFileHelper::getUserProfileURL());
        css::uno::Reference< css::system::XSystemShellExecute > exec(
        css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
        try {
            exec->execute(uri, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
        } catch (css::uno::Exception) {
        }
        EndDialog(RET_OK);
    }
}

IMPL_LINK(SafeModeDialog, CreateZipBtnHdl, Button*, /*pBtn*/, void)
{
    const OUString zipFileName("libreoffice-profile.zip");
    const OUString zipFileURL(comphelper::BackupFileHelper::getUserProfileURL() + "/" + zipFileName);
    osl::File::remove(zipFileURL); // Remove previous exports
    try
    {
        utl::ZipPackageHelper aZipHelper(comphelper::getProcessComponentContext(), zipFileURL);
        aZipHelper.addFolderWithContent(aZipHelper.getRootFolder(), comphelper::BackupFileHelper::getUserProfileWorkURL());
        aZipHelper.savePackage();
    }
    catch (uno::Exception)
    {
        ScopedVclPtrInstance< MessageDialog > aErrorBox(this, SVX_RESSTR(RID_SVXSTR_SAFEMODE_ZIP_FAILURE));
        aErrorBox->Execute();
        return;
    }

    ScopedVclPtrInstance< ProfileExportedDialog > aDialog;
    aDialog->Execute();
}

IMPL_LINK(SafeModeDialog, CheckBoxHdl, CheckBox&, /*pCheckBox*/, void)
{
    const bool bEnable(
        mpCBCheckProfilesafeConfig->IsChecked() ||
        mpCBCheckProfilesafeExtensions->IsChecked() ||
        mpCBDisableAllExtensions->IsChecked() ||
        mpCBDeinstallUserExtensions->IsChecked() ||
        mpCBDeinstallAllExtensions->IsChecked() ||
        mpCBDisableHWAcceleration->IsChecked() ||
        mpCBResetCustomizations->IsChecked() ||
        mpCBResetWholeUserProfile->IsChecked());

    mpBtnRestart->Enable(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
