/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <comphelper/backupfilehelper.hxx>
#include <vcl/weld.hxx>

class SafeModeDialog : public weld::GenericDialogController
{
public:
    explicit SafeModeDialog(weld::Window* pParent);
    virtual short run() override;
    virtual ~SafeModeDialog() override;

private:
    std::unique_ptr<weld::Button> mxBtnContinue;
    std::unique_ptr<weld::Button> mxBtnRestart;
    std::unique_ptr<weld::Button> mxBtnApply;

    std::unique_ptr<weld::Container> mxBoxRestore;
    std::unique_ptr<weld::Container> mxBoxConfigure;
    std::unique_ptr<weld::Container> mxBoxDeinstall;
    std::unique_ptr<weld::Container> mxBoxReset;

    std::unique_ptr<weld::RadioButton> mxRadioRestore;
    std::unique_ptr<weld::RadioButton> mxRadioConfigure;
    std::unique_ptr<weld::RadioButton> mxRadioExtensions;
    std::unique_ptr<weld::RadioButton> mxRadioReset;

    std::unique_ptr<weld::CheckButton> mxCBCheckProfilesafeConfig;
    std::unique_ptr<weld::CheckButton> mxCBCheckProfilesafeExtensions;
    std::unique_ptr<weld::CheckButton> mxCBDisableAllExtensions;
    std::unique_ptr<weld::CheckButton> mxCBDeinstallUserExtensions;
    std::unique_ptr<weld::CheckButton> mxCBResetSharedExtensions;
    std::unique_ptr<weld::CheckButton> mxCBResetBundledExtensions;
    std::unique_ptr<weld::CheckButton> mxCBDisableHWAcceleration;
    std::unique_ptr<weld::CheckButton> mxCBResetCustomizations;
    std::unique_ptr<weld::CheckButton> mxCBResetWholeUserProfile;

    std::unique_ptr<weld::LinkButton> mxBugLink;
    std::unique_ptr<weld::LinkButton> mxUserProfileLink;
    std::unique_ptr<weld::Button> mxBtnCreateZip;
    std::unique_ptr<weld::Expander> mxExpander;

    // local BackupFileHelper for handling possible restores
    comphelper::BackupFileHelper maBackupFileHelper;

    void enableDisableWidgets();
    void applyChanges();

    DECL_LINK(RadioBtnHdl, weld::Toggleable&, void);
    DECL_LINK(CheckBoxHdl, weld::Toggleable&, void);
    DECL_LINK(CreateZipBtnHdl, weld::Button&, void);
    DECL_LINK(DialogBtnHdl, weld::Button&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
