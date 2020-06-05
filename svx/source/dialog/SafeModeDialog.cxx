/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SafeModeDialog.hxx"

#include <osl/file.hxx>
#include <sfx2/safemode.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ZipPackageHelper.hxx>
#include <unotools/configmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>

#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>

using namespace css;

SafeModeDialog::SafeModeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svx/ui/safemodedialog.ui", "SafeModeDialog")
    , mxBtnContinue(m_xBuilder->weld_button("btn_continue"))
    , mxBtnRestart(m_xBuilder->weld_button("btn_restart"))
    , mxBtnApply(m_xBuilder->weld_button("btn_apply"))
    , mxBoxRestore(m_xBuilder->weld_container("group_restore"))
    , mxBoxConfigure(m_xBuilder->weld_container("group_configure"))
    , mxBoxDeinstall(m_xBuilder->weld_container("group_deinstall"))
    , mxBoxReset(m_xBuilder->weld_container("group_reset"))
    , mxRadioRestore(m_xBuilder->weld_radio_button("radio_restore"))
    , mxRadioConfigure(m_xBuilder->weld_radio_button("radio_configure"))
    , mxRadioExtensions(m_xBuilder->weld_radio_button("radio_extensions"))
    , mxRadioReset(m_xBuilder->weld_radio_button("radio_reset"))
    , mxCBCheckProfilesafeConfig(m_xBuilder->weld_check_button("check_profilesafe_config"))
    , mxCBCheckProfilesafeExtensions(m_xBuilder->weld_check_button("check_profilesafe_extensions"))
    , mxCBDisableAllExtensions(m_xBuilder->weld_check_button("check_disable_all_extensions"))
    , mxCBDeinstallUserExtensions(m_xBuilder->weld_check_button("check_deinstall_user_extensions"))
    , mxCBResetSharedExtensions(m_xBuilder->weld_check_button("check_reset_shared_extensions"))
    , mxCBResetBundledExtensions(m_xBuilder->weld_check_button("check_reset_bundled_extensions"))
    , mxCBDisableHWAcceleration(m_xBuilder->weld_check_button("check_disable_hw_acceleration"))
    , mxCBResetCustomizations(m_xBuilder->weld_check_button("check_reset_customizations"))
    , mxCBResetWholeUserProfile(m_xBuilder->weld_check_button("check_reset_whole_userprofile"))
    , mxBugLink(m_xBuilder->weld_link_button("linkbutton_bugs"))
    , mxUserProfileLink(m_xBuilder->weld_link_button("linkbutton_profile"))
    , mxBtnCreateZip(m_xBuilder->weld_button("btn_create_zip"))
    , mxExpander(m_xBuilder->weld_expander("expander"))
    , maBackupFileHelper()
{
    m_xDialog->set_centered_on_parent(false);
    mxRadioRestore->connect_clicked(LINK(this, SafeModeDialog, RadioBtnHdl));
    mxRadioConfigure->connect_clicked(LINK(this, SafeModeDialog, RadioBtnHdl));
    mxRadioExtensions->connect_clicked(LINK(this, SafeModeDialog, RadioBtnHdl));
    mxRadioReset->connect_clicked(LINK(this, SafeModeDialog, RadioBtnHdl));

    mxBtnContinue->connect_clicked(LINK(this, SafeModeDialog, DialogBtnHdl));
    mxBtnRestart->connect_clicked(LINK(this, SafeModeDialog, DialogBtnHdl));
    mxBtnApply->connect_clicked(LINK(this, SafeModeDialog, DialogBtnHdl));

    mxCBCheckProfilesafeConfig->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBCheckProfilesafeExtensions->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBDisableAllExtensions->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBDeinstallUserExtensions->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBResetSharedExtensions->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBResetBundledExtensions->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBDisableHWAcceleration->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBResetCustomizations->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));
    mxCBResetWholeUserProfile->connect_toggled(LINK(this, SafeModeDialog, CheckBoxHdl));

    mxBtnCreateZip->connect_clicked(LINK(this, SafeModeDialog, CreateZipBtnHdl));

    // Disable restart btn until some checkbox is active
    mxBtnApply->set_sensitive(false);

    // Check the first radio button and call its handler,
    // it'll disable the relevant parts
    mxRadioRestore->set_active(true);
    RadioBtnHdl(*mxRadioRestore);

    // Set URL for help button (module=safemode)
    OUString sURL("http://hub.libreoffice.org/send-feedback/?LOversion=" + utl::ConfigManager::getAboutBoxProductVersion() +
        "&LOlocale=" + utl::ConfigManager::getUILocale() + "&LOmodule=safemode");
    mxBugLink->set_uri(sURL);

    mxUserProfileLink->set_uri(comphelper::BackupFileHelper::getUserProfileURL());
}

SafeModeDialog::~SafeModeDialog()
{
}

void SafeModeDialog::enableDisableWidgets()
{
    mxCBCheckProfilesafeConfig->set_sensitive(maBackupFileHelper.isPopPossible());
    mxCBCheckProfilesafeExtensions->set_sensitive(maBackupFileHelper.isPopPossibleExtensionInfo());
    mxCBDisableAllExtensions->set_sensitive(comphelper::BackupFileHelper::isTryDisableAllExtensionsPossible());
    mxCBDeinstallUserExtensions->set_sensitive(comphelper::BackupFileHelper::isTryDeinstallUserExtensionsPossible());
    mxCBResetSharedExtensions->set_sensitive(comphelper::BackupFileHelper::isTryResetSharedExtensionsPossible());
    mxCBResetBundledExtensions->set_sensitive(comphelper::BackupFileHelper::isTryResetBundledExtensionsPossible());
    mxCBResetCustomizations->set_sensitive(comphelper::BackupFileHelper::isTryResetCustomizationsPossible());

    // no disable of mxCBResetWholeUserProfile, always possible (as last choice)
}

short SafeModeDialog::run()
{
    short nRet = weld::GenericDialogController::run();
    // Remove the safe mode flag before exiting this dialog
    sfx2::SafeMode::removeFlag();
    return nRet;
}

void SafeModeDialog::applyChanges()
{
    // Restore
    if (mxRadioRestore->get_active())
    {
        if (mxCBCheckProfilesafeConfig->get_active())
        {
            // reset UserConfiguration to last known working state
            // ProfileSafeMode/BackupFileHelper
            maBackupFileHelper.tryPop();
        }

        if (mxCBCheckProfilesafeExtensions->get_active())
        {
            // reset State of installed Extensions to last known working state
            // ProfileSafeMode/BackupFileHelper
            maBackupFileHelper.tryPopExtensionInfo();
        }
    }

    // Configure
    if (mxRadioConfigure->get_active())
    {
        if (mxCBDisableAllExtensions->get_active())
        {
            // Disable all extensions
            comphelper::BackupFileHelper::tryDisableAllExtensions();
        }

        if (mxCBDisableHWAcceleration->get_active())
        {
            comphelper::BackupFileHelper::tryDisableHWAcceleration();
        }
    }

    // Deinstall
    if (mxRadioExtensions->get_active())
    {
        if (mxCBDeinstallUserExtensions->get_active())
        {
            // Deinstall all User Extensions (installed for User only)
            comphelper::BackupFileHelper::tryDeinstallUserExtensions();
        }

        if (mxCBResetSharedExtensions->get_active())
        {
            // Reset shared Extensions
            comphelper::BackupFileHelper::tryResetSharedExtensions();
        }
        if (mxCBResetBundledExtensions->get_active())
        {
            // Reset bundled Extensions
            comphelper::BackupFileHelper::tryResetBundledExtensions();
        }
    }

    // Reset
    if (mxRadioReset->get_active())
    {
        if (mxCBResetCustomizations->get_active())
        {
            // Reset customizations (Settings and UserInterface modifications)
            comphelper::BackupFileHelper::tryResetCustomizations();
        }

        if (mxCBResetWholeUserProfile->get_active())
        {
            // Reset the whole UserProfile
            comphelper::BackupFileHelper::tryResetUserProfile();
        }
    }

    // finally, restart
    css::task::OfficeRestartManager::get(comphelper::getProcessComponentContext())->requestRestart(
        css::uno::Reference< css::task::XInteractionHandler >());
}

IMPL_LINK(SafeModeDialog, RadioBtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxRadioRestore.get())
    {
        // Enable the currently selected box
        mxBoxRestore->set_sensitive(true);
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mxBoxReset->set_sensitive(false);
        mxBoxConfigure->set_sensitive(false);
        mxBoxDeinstall->set_sensitive(false);
    }
    else if (&rBtn == mxRadioConfigure.get())
    {
        // Enable the currently selected box
        mxBoxConfigure->set_sensitive(true);
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mxBoxRestore->set_sensitive(false);
        mxBoxReset->set_sensitive(false);
        mxBoxDeinstall->set_sensitive(false);

    }
    else if (&rBtn == mxRadioExtensions.get())
    {
        // Enable the currently selected box
        mxBoxDeinstall->set_sensitive(true);
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mxBoxRestore->set_sensitive(false);
        mxBoxConfigure->set_sensitive(false);
        mxBoxReset->set_sensitive(false);
    }
    else if (&rBtn == mxRadioReset.get())
    {
        // Enable the currently selected box
        mxBoxReset->set_sensitive(true);
        // Make sure only possible choices are active
        enableDisableWidgets();
        // Disable the unselected boxes
        mxBoxConfigure->set_sensitive(false);
        mxBoxRestore->set_sensitive(false);
        mxBoxDeinstall->set_sensitive(false);
    }
}

IMPL_LINK(SafeModeDialog, DialogBtnHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnContinue.get())
    {
        m_xDialog->response(RET_CLOSE);
    }
    else if (&rBtn == mxBtnRestart.get())
    {
        sfx2::SafeMode::putRestartFlag();
        m_xDialog->response(RET_CLOSE);
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        css::task::OfficeRestartManager::get(xContext)->requestRestart(
            css::uno::Reference< css::task::XInteractionHandler >());
    }
    else if (&rBtn == mxBtnApply.get())
    {
        sfx2::SafeMode::putRestartFlag();
        m_xDialog->response(RET_CLOSE);
        applyChanges();
    }
}

namespace {
    class ProfileExportedDialog : public weld::GenericDialogController
    {
    private:
        std::unique_ptr<weld::Button> m_xButton;

        DECL_LINK(OpenHdl, weld::Button&, void);
    public:
        explicit ProfileExportedDialog(weld::Window* pParent);
    };

    ProfileExportedDialog::ProfileExportedDialog(weld::Window* pParent)
        : GenericDialogController(pParent, "svx/ui/profileexporteddialog.ui", "ProfileExportedDialog")
        , m_xButton(m_xBuilder->weld_button("ok"))
    {
        m_xButton->connect_clicked(LINK(this, ProfileExportedDialog, OpenHdl));
    }

    IMPL_LINK_NOARG(ProfileExportedDialog, OpenHdl, weld::Button&, void)
    {
        const OUString uri(comphelper::BackupFileHelper::getUserProfileURL());
        css::uno::Reference< css::system::XSystemShellExecute > exec(
        css::system::SystemShellExecute::create(comphelper::getProcessComponentContext()));
        try {
            exec->execute(uri, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
        } catch (const css::uno::Exception &) {
            TOOLS_WARN_EXCEPTION("svx.dialog", "opening <" << uri << "> failed:");
        }
        m_xDialog->response(RET_OK);
    }
}

IMPL_LINK(SafeModeDialog, CreateZipBtnHdl, weld::Button&, /*rBtn*/, void)
{
    const OUString zipFileURL(comphelper::BackupFileHelper::getUserProfileURL() + "/libreoffice-profile.zip");
    osl::File::remove(zipFileURL); // Remove previous exports
    try
    {
        utl::ZipPackageHelper aZipHelper(comphelper::getProcessComponentContext(), zipFileURL);
        aZipHelper.addFolderWithContent(aZipHelper.getRootFolder(), comphelper::BackupFileHelper::getUserProfileWorkURL());
        aZipHelper.savePackage();
    }
    catch (const uno::Exception &)
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 SvxResId(RID_SVXSTR_SAFEMODE_ZIP_FAILURE)));
        xBox->run();
        return;
    }

    ProfileExportedDialog aDialog(m_xDialog.get());
    aDialog.run();
}

IMPL_LINK(SafeModeDialog, CheckBoxHdl, weld::ToggleButton&, /*pCheckBox*/, void)
{
    const bool bEnable(
        mxCBCheckProfilesafeConfig->get_active() ||
        mxCBCheckProfilesafeExtensions->get_active() ||
        mxCBDisableAllExtensions->get_active() ||
        mxCBDeinstallUserExtensions->get_active() ||
        mxCBResetSharedExtensions->get_active() ||
        mxCBResetBundledExtensions->get_active() ||
        mxCBDisableHWAcceleration->get_active() ||
        mxCBResetCustomizations->get_active() ||
        mxCBResetWholeUserProfile->get_active());

    mxBtnApply->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
