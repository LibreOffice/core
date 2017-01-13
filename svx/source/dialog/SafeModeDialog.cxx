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
    mpBtnRestart(),
    mpBtnApply(),

    mpBoxRestore(),
    mpBoxConfigure(),
    mpBoxDeinstall(),
    mpBoxReset(),

    mpRadioRestore(),
    mpRadioConfigure(),
    mpRadioExtensions(),
    mpRadioReset(),

    mpCBCheckProfilesafeConfig(),
    mpCBCheckProfilesafeExtensions(),
    mpCBDisableAllExtensions(),
    mpCBDeinstallUserExtensions(),
    mpCBResetSharedExtensions(),
    mpCBResetBundledExtensions(),
    mpCBDisableHWAcceleration(),
    mpCBResetCustomizations(),
    mpCBResetWholeUserProfile(),

    maBackupFileHelper()
{
    get(mpBtnContinue, "btn_continue");
    get(mpBtnRestart, "btn_restart");
    get(mpBtnApply, "btn_apply");

    get(mpBoxRestore, "group_restore");
    get(mpBoxConfigure, "group_configure");
    get(mpBoxDeinstall, "group_deinstall");
    get(mpBoxReset, "group_reset");

    get(mpRadioRestore, "radio_restore");
    get(mpRadioConfigure, "radio_configure");
    get(mpRadioExtensions, "radio_extensions");
    get(mpRadioReset, "radio_reset");

    get(mpCBCheckProfilesafeConfig, "check_profilesafe_config");
    get(mpCBCheckProfilesafeExtensions, "check_profilesafe_extensions");
    get(mpCBDisableAllExtensions, "check_disable_all_extensions");
    get(mpCBDeinstallUserExtensions, "check_deinstall_user_extensions");
    get(mpCBResetSharedExtensions, "check_reset_shared_extensions");
    get(mpCBResetBundledExtensions, "check_reset_bundled_extensions");
    get(mpCBDisableHWAcceleration, "check_disable_hw_acceleration");
    get(mpCBResetCustomizations, "check_reset_customizations");
    get(mpCBResetWholeUserProfile, "check_reset_whole_userprofile");

    get(mpBugLink, "linkbutton_bugs");
    get(mpUserProfileLink, "linkbutton_profile");
    get(mpBtnCreateZip, "btn_create_zip");

    mpRadioRestore->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));
    mpRadioConfigure->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));
    mpRadioExtensions->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));
    mpRadioReset->SetClickHdl(LINK(this, SafeModeDialog, RadioBtnHdl));

    mpBtnContinue->SetClickHdl(LINK(this, SafeModeDialog, DialogBtnHdl));
    mpBtnRestart->SetClickHdl(LINK(this, SafeModeDialog, DialogBtnHdl));
    mpBtnApply->SetClickHdl(LINK(this, SafeModeDialog, DialogBtnHdl));

    mpCBCheckProfilesafeConfig->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBCheckProfilesafeExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDisableAllExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDeinstallUserExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetSharedExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetBundledExtensions->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBDisableHWAcceleration->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetCustomizations->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));
    mpCBResetWholeUserProfile->SetToggleHdl(LINK(this, SafeModeDialog, CheckBoxHdl));

    mpBtnCreateZip->SetClickHdl(LINK(this, SafeModeDialog, CreateZipBtnHdl));

    // Disable restart btn until some checkbox is active
    mpBtnApply->Disable();

    // Check the first radio button and disable the other parts
    mpRadioRestore->Check();
    mpBoxConfigure->Disable();
    mpBoxDeinstall->Disable();
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
    mpRadioExtensions.clear();
    mpRadioReset.clear();

    mpBoxRestore.clear();
    mpBoxConfigure.clear();
    mpBoxDeinstall.clear();
    mpBoxReset.clear();

    mpBtnContinue.clear();
    mpBtnRestart.clear();
    mpBtnApply.clear();

    mpCBCheckProfilesafeConfig.clear();
    mpCBCheckProfilesafeExtensions.clear();
    mpCBDisableAllExtensions.clear();
    mpCBDeinstallUserExtensions.clear();
    mpCBResetSharedExtensions.clear();
    mpCBResetBundledExtensions.clear();
    mpCBDisableHWAcceleration.clear();
    mpCBResetCustomizations.clear();
    mpCBResetWholeUserProfile.clear();

    mpBugLink.clear();
    mpUserProfileLink.clear();
    mpBtnCreateZip.clear();

    Dialog::dispose();
}

void SafeModeDialog::enableDisableWidgets()
{
    mpCBCheckProfilesafeConfig->Enable(maBackupFileHelper.isPopPossible());
    mpCBCheckProfilesafeExtensions->Enable(maBackupFileHelper.isPopPossibleExtensionInfo());
    mpCBDisableAllExtensions->Enable(comphelper::BackupFileHelper::isTryDisableAllExtensionsPossible());
    mpCBDeinstallUserExtensions->Enable(comphelper::BackupFileHelper::isTryDeinstallUserExtensionsPossible());
    mpCBResetSharedExtensions->Enable(comphelper::BackupFileHelper::isTryResetSharedExtensionsPossible());
    mpCBResetBundledExtensions->Enable(comphelper::BackupFileHelper::isTryResetBundledExtensionsPossible());
    mpCBResetCustomizations->Enable(comphelper::BackupFileHelper::isTryResetCustomizationsPossible());

    // no disable of mpCBResetWholeUserProfile, always possible (as last choice)
}

bool SafeModeDialog::Close()
{
    // Remove the safe mode flag before exiting this dialog
    sfx2::SafeMode::removeFlag();

    return Dialog::Close();
}

void SafeModeDialog::applyChanges()
{
    // Restore
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

        if (mpCBDisableHWAcceleration->IsChecked())
        {
            comphelper::BackupFileHelper::tryDisableHWAcceleration();
        }
    }

    // Deinstall
    if (mpRadioExtensions->IsChecked())
    {
        if (mpCBDeinstallUserExtensions->IsChecked())
        {
            // Deinstall all User Extensions (installed for User only)
            comphelper::BackupFileHelper::tryDeinstallUserExtensions();
        }

        if (mpCBResetSharedExtensions->IsChecked())
        {
            // Reset shared Extensions
            comphelper::BackupFileHelper::tryResetSharedExtensions();
        }
        if (mpCBResetBundledExtensions->IsChecked())
        {
            // Reset bundled Extensions
            comphelper::BackupFileHelper::tryResetBundledExtensions();
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

    // finally, restart
    css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
        css::uno::Reference< css::task::XInteractionHandler >());
}

IMPL_LINK(SafeModeDialog, RadioBtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpRadioRestore.get())
    {
        // Enable the currently selected box
        mpBoxRestore->Enable();
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mpBoxReset->Disable();
        mpBoxConfigure->Disable();
        mpBoxDeinstall->Disable();
    }
    else if (pBtn == mpRadioConfigure.get())
    {
        // Enable the currently selected box
        mpBoxConfigure->Enable();
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mpBoxRestore->Disable();
        mpBoxReset->Disable();
        mpBoxDeinstall->Disable();

    }
    else if (pBtn == mpRadioExtensions.get())
    {
        // Enable the currently selected box
        mpBoxDeinstall->Enable();
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mpBoxRestore->Disable();
        mpBoxConfigure->Disable();
        mpBoxReset->Disable();
    }
    else if (pBtn == mpRadioReset.get())
    {
        // Enable the currently selected box
        mpBoxReset->Enable();
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mpBoxConfigure->Disable();
        mpBoxRestore->Disable();
        mpBoxDeinstall->Disable();
    }
}

IMPL_LINK(SafeModeDialog, DialogBtnHdl, Button*, pBtn, void)
{
    if (pBtn == mpBtnContinue.get())
    {
        Close();
    }
    else if (pBtn == mpBtnRestart.get())
    {
        sfx2::SafeMode::putRestartFlag();
        Close();
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        css::task::OfficeRestartManager::get(xContext)->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
    }
    else if (pBtn == mpBtnApply.get())
    {
        sfx2::SafeMode::putRestartFlag();
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
        : ModalDialog(nullptr, "ProfileExportedDialog", "svx/ui/profileexporteddialog.ui")
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
        mpCBResetSharedExtensions->IsChecked() ||
        mpCBResetBundledExtensions->IsChecked() ||
        mpCBDisableHWAcceleration->IsChecked() ||
        mpCBResetCustomizations->IsChecked() ||
        mpCBResetWholeUserProfile->IsChecked());

    mpBtnApply->Enable(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
