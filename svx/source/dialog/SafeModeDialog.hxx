/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SOURCE_DIALOG_SAFEMODEDIALOG_HXX
#define INCLUDED_SVX_SOURCE_DIALOG_SAFEMODEDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/fixedhyper.hxx>
#include <comphelper/backupfilehelper.hxx>

class SafeModeDialog : public Dialog
{
public:

    explicit SafeModeDialog(vcl::Window* pParent);

    virtual ~SafeModeDialog() override;

    virtual void dispose() override;

    virtual bool Close() override;

private:

    VclPtr<Button> mpBtnContinue;
    VclPtr<Button> mpBtnRestart;
    VclPtr<Button> mpBtnApply;

    VclPtr<VclBox> mpBoxRestore;
    VclPtr<VclBox> mpBoxConfigure;
    VclPtr<VclBox> mpBoxDeinstall;
    VclPtr<VclBox> mpBoxReset;

    VclPtr<RadioButton> mpRadioRestore;
    VclPtr<RadioButton> mpRadioConfigure;
    VclPtr<RadioButton> mpRadioExtensions;
    VclPtr<RadioButton> mpRadioReset;

    VclPtr<CheckBox> mpCBCheckProfilesafeConfig;
    VclPtr<CheckBox> mpCBCheckProfilesafeExtensions;
    VclPtr<CheckBox> mpCBDisableAllExtensions;
    VclPtr<CheckBox> mpCBDeinstallUserExtensions;
    VclPtr<CheckBox> mpCBResetSharedExtensions;
    VclPtr<CheckBox> mpCBResetBundledExtensions;
    VclPtr<CheckBox> mpCBDisableHWAcceleration;
    VclPtr<CheckBox> mpCBResetCustomizations;
    VclPtr<CheckBox> mpCBResetWholeUserProfile;

    VclPtr<FixedHyperlink> mpBugLink;
    VclPtr<FixedHyperlink> mpUserProfileLink;
    VclPtr<Button> mpBtnCreateZip;

    // local BackupFileHelper for handling possible restores
    comphelper::BackupFileHelper maBackupFileHelper;

    void enableDisableWidgets();
    void applyChanges();
    static void openWebBrowser(const OUString & sURL, const OUString &sTitle);

    DECL_LINK(RadioBtnHdl, Button*, void);
    DECL_LINK(CheckBoxHdl, CheckBox&, void);
    DECL_LINK(CreateZipBtnHdl, Button*, void);
    DECL_LINK(DialogBtnHdl, Button*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
