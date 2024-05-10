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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ZipPackageHelper.hxx>
#include <unotools/configmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/FileExportedDialog.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

using namespace css;

SafeModeDialog::SafeModeDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"svx/ui/safemodedialog.ui"_ustr, u"SafeModeDialog"_ustr)
    , mxBtnContinue(m_xBuilder->weld_button(u"btn_continue"_ustr))
    , mxBtnRestart(m_xBuilder->weld_button(u"btn_restart"_ustr))
    , mxBtnApply(m_xBuilder->weld_button(u"btn_apply"_ustr))
    , mxBoxRestore(m_xBuilder->weld_container(u"group_restore"_ustr))
    , mxBoxConfigure(m_xBuilder->weld_container(u"group_configure"_ustr))
    , mxBoxDeinstall(m_xBuilder->weld_container(u"group_deinstall"_ustr))
    , mxBoxReset(m_xBuilder->weld_container(u"group_reset"_ustr))
    , mxRadioRestore(m_xBuilder->weld_radio_button(u"radio_restore"_ustr))
    , mxRadioConfigure(m_xBuilder->weld_radio_button(u"radio_configure"_ustr))
    , mxRadioExtensions(m_xBuilder->weld_radio_button(u"radio_extensions"_ustr))
    , mxRadioReset(m_xBuilder->weld_radio_button(u"radio_reset"_ustr))
    , mxCBCheckProfilesafeConfig(m_xBuilder->weld_check_button(u"check_profilesafe_config"_ustr))
    , mxCBCheckProfilesafeExtensions(m_xBuilder->weld_check_button(u"check_profilesafe_extensions"_ustr))
    , mxCBDisableAllExtensions(m_xBuilder->weld_check_button(u"check_disable_all_extensions"_ustr))
    , mxCBDeinstallUserExtensions(m_xBuilder->weld_check_button(u"check_deinstall_user_extensions"_ustr))
    , mxCBResetSharedExtensions(m_xBuilder->weld_check_button(u"check_reset_shared_extensions"_ustr))
    , mxCBResetBundledExtensions(m_xBuilder->weld_check_button(u"check_reset_bundled_extensions"_ustr))
    , mxCBDisableHWAcceleration(m_xBuilder->weld_check_button(u"check_disable_hw_acceleration"_ustr))
    , mxCBResetCustomizations(m_xBuilder->weld_check_button(u"check_reset_customizations"_ustr))
    , mxCBResetWholeUserProfile(m_xBuilder->weld_check_button(u"check_reset_whole_userprofile"_ustr))
    , mxBugLink(m_xBuilder->weld_link_button(u"linkbutton_bugs"_ustr))
    , mxUserProfileLink(m_xBuilder->weld_link_button(u"linkbutton_profile"_ustr))
    , mxBtnCreateZip(m_xBuilder->weld_button(u"btn_create_zip"_ustr))
{
    m_xDialog->set_centered_on_parent(false);
    mxRadioRestore->connect_toggled(LINK(this, SafeModeDialog, RadioBtnHdl));
    mxRadioConfigure->connect_toggled(LINK(this, SafeModeDialog, RadioBtnHdl));
    mxRadioExtensions->connect_toggled(LINK(this, SafeModeDialog, RadioBtnHdl));
    mxRadioReset->connect_toggled(LINK(this, SafeModeDialog, RadioBtnHdl));

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
    OUString sURL(officecfg::Office::Common::Menus::SendFeedbackURL::get()  //officecfg/registry/data/org/openoffice/Office/Common.xcu => https://hub.libreoffice.org/send-feedback/
     + "?LOversion=" + utl::ConfigManager::getAboutBoxProductVersion() +
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

IMPL_LINK(SafeModeDialog, RadioBtnHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;
    if (mxRadioRestore->get_active())
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
    else if (mxRadioConfigure->get_active())
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
    else if (mxRadioExtensions->get_active())
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
    else if (mxRadioReset->get_active())
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

    FileExportedDialog aDialog(m_xDialog.get(), SvxResId(RID_SVXSTR_SAFEMODE_USER_PROFILE_EXPORTED));
    aDialog.run();
}

IMPL_LINK(SafeModeDialog, CheckBoxHdl, weld::Toggleable&, /*pCheckBox*/, void)
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
