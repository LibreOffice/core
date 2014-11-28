/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <svl/zforlist.hxx>
#include <opencl/openclconfig.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/setup/UpdateCheckConfig.hpp>

#include "cuires.hrc"
#include "optopencl.hxx"

SvxOpenCLTabPage::SvxOpenCLTabPage(vcl::Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, "OptOpenCLPage", "cui/ui/optopenclpage.ui", &rSet),
    maConfig(OpenCLConfig::get())
{
    get(mpUseOpenCL, "useopencl");
    get(mpBlackList, "blacklist");
    get(mpBlackListEdit, "bledit");
    get(mpBlackListAdd, "bladd");
    get(mpBlackListDelete, "bldelete");
    get(mpWhiteList, "whitelist");
    get(mpWhiteListEdit, "wledit");
    get(mpWhiteListAdd, "wladd");
    get(mpWhiteListDelete, "wldelete");

    mpBlackListEdit->SetClickHdl(LINK(this, SvxOpenCLTabPage, BlackListEditHdl));
    mpBlackListAdd->SetClickHdl(LINK(this, SvxOpenCLTabPage, BlackListAddHdl));
    mpBlackListDelete->SetClickHdl(LINK(this, SvxOpenCLTabPage, BlackListDeleteHdl));

    mpWhiteListEdit->SetClickHdl(LINK(this, SvxOpenCLTabPage, WhiteListEditHdl));
    mpWhiteListAdd->SetClickHdl(LINK(this, SvxOpenCLTabPage, WhiteListAddHdl));
    mpWhiteListDelete->SetClickHdl(LINK(this, SvxOpenCLTabPage, WhiteListDeleteHdl));

    mpBlackList->set_height_request(4 * mpBlackList->GetTextHeight());
    mpWhiteList->set_height_request(4 * mpWhiteList->GetTextHeight());
}

SvxOpenCLTabPage::~SvxOpenCLTabPage()
{
}

SfxTabPage*
SvxOpenCLTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return new SvxOpenCLTabPage(pParent, *rAttrSet);
}

bool SvxOpenCLTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;
    boost::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

    if (mpUseOpenCL->IsValueChangedFromSaved())
        maConfig.mbUseOpenCL = mpUseOpenCL->IsChecked();

    if (maConfig != OpenCLConfig::get())
    {
        maConfig.set();
        bModified = true;
    }

    if (bModified)
        batch->commit();

    return bModified;
}

namespace {

OUString format(const OpenCLConfig::ImplMatcher& rImpl)
{
    return (rImpl.maOS + " " +
            rImpl.maOSVersion + " " +
            rImpl.maPlatformVendor + " " +
            rImpl.maDevice + " " +
            rImpl.maDriverVersion);
}

void fillListBox(ListBox* pListBox, const OpenCLConfig::ImplMatcherSet& rSet)
{
    pListBox->SetUpdateMode(false);
    pListBox->Clear();

    for (auto i = rSet.cbegin(); i != rSet.cend(); ++i)
    {
        pListBox->InsertEntry(format(*i), LISTBOX_APPEND);
    }

    pListBox->SetUpdateMode(true);
}

}

void SvxOpenCLTabPage::Reset( const SfxItemSet* )
{
    maConfig = OpenCLConfig::get();

    mpUseOpenCL->Check(maConfig.mbUseOpenCL);
    mpUseOpenCL->SaveValue();

    fillListBox(mpBlackList, maConfig.maBlackList);
    fillListBox(mpWhiteList, maConfig.maWhiteList);
}

void SvxOpenCLTabPage::FillUserData()
{
}

namespace {

class ListEntryDialog : public ModalDialog
{
public:
    OpenCLConfig::ImplMatcher maEntry;

    ListBox* mpOS;
    Edit* mpOSVersion;
    Edit* mpPlatformVendor;
    Edit* mpDevice;
    Edit* mpDriverVersion;

    DECL_LINK(OSSelectHdl, ListBox*);
    DECL_LINK(EditModifiedHdl, Edit*);

    ListEntryDialog(vcl::Window* pParent, const OpenCLConfig::ImplMatcher& rEntry, const OString& rTag);
};

ListEntryDialog::ListEntryDialog(vcl::Window* pParent, const OpenCLConfig::ImplMatcher& rEntry, const OString& rTag)
    : ModalDialog(pParent, "BlackOrWhiteListEntryDialog",
                  "cui/ui/blackorwhitelistentrydialog.ui"),
      maEntry(rEntry)
{
    get(mpOS, "os");
    get(mpOSVersion, "osversion");
    get(mpPlatformVendor, "platformvendor");
    get(mpDevice, "device");
    get(mpDriverVersion, "driverversion");

    // Hardcode knowledge that entry 0 is the "Any"
    if (maEntry.maOS == "")
    {
        mpOS->SelectEntryPos(0, false);
    }
    else
    {
        for (int i = 0; i < mpOS->GetEntryCount(); ++i)
        {
            if (maEntry.maOS == mpOS->GetEntry(i))
                mpOS->SelectEntryPos(i, false);
        }
    }
    mpOSVersion->SetText(rEntry.maOSVersion);
    mpPlatformVendor->SetText(rEntry.maPlatformVendor);
    mpDevice->SetText(rEntry.maDevice);
    mpDriverVersion->SetText(rEntry.maDriverVersion);

    mpOS->SetSelectHdl(LINK( this, ListEntryDialog, OSSelectHdl));
    mpOSVersion->SetModifyHdl(LINK(this, ListEntryDialog, EditModifiedHdl));
    mpPlatformVendor->SetModifyHdl(LINK(this, ListEntryDialog, EditModifiedHdl));
    mpDevice->SetModifyHdl(LINK(this, ListEntryDialog, EditModifiedHdl));
    mpDriverVersion->SetModifyHdl(LINK(this, ListEntryDialog, EditModifiedHdl));

    SetText(get<FixedText>(rTag + "title")->GetText());
}

IMPL_LINK(ListEntryDialog, OSSelectHdl, ListBox*, pListBox)
{
    if (pListBox == mpOS)
    {
        if (mpOS->GetSelectEntryPos() == 0)
            maEntry.maOS = "";
        else
            maEntry.maOS = mpOS->GetEntry(mpOS->GetSelectEntryPos());
    }

    return 0;
}

IMPL_LINK(ListEntryDialog, EditModifiedHdl, Edit*, pEdit)
{
    if (pEdit == mpOSVersion)
        maEntry.maOSVersion = pEdit->GetText();
    else if (pEdit == mpPlatformVendor)
        maEntry.maPlatformVendor = pEdit->GetText();
    else if (pEdit == mpDevice)
        maEntry.maDevice = pEdit->GetText();
    else if (pEdit == mpDriverVersion)
        maEntry.maDriverVersion = pEdit->GetText();

    return 0;
}

void openListDialog(SvxOpenCLTabPage* pTabPage, OpenCLConfig::ImplMatcher& rEntry, const OString& rTag)
{
    ListEntryDialog aDlg(pTabPage, rEntry, rTag);

    if (aDlg.Execute() == RET_OK)
        rEntry = aDlg.maEntry;
}

const OpenCLConfig::ImplMatcher& findCurrentEntry(OpenCLConfig::ImplMatcherSet& rSet, ListBox* pListBox)
{
    auto i = rSet.begin();
    std::advance(i, pListBox->GetSelectEntryPos());

    return *i;
}

}

long SvxOpenCLTabPage::EditHdl(ListBox* pListBox, OpenCLConfig::ImplMatcherSet& rSet, const OString& rTag)
{
    if (pListBox->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
        return 0;

    OpenCLConfig::ImplMatcher rEntry(findCurrentEntry(rSet, pListBox));
    rSet.erase(rEntry);
    openListDialog(this, rEntry, rTag);
    rSet.insert(rEntry);
    fillListBox(pListBox, rSet);

    return 0;
}

long SvxOpenCLTabPage::AddHdl(ListBox* pListBox, OpenCLConfig::ImplMatcherSet& rSet, const OString& rTag)
{
    OpenCLConfig::ImplMatcher rEntry;
    openListDialog(this, rEntry, rTag);
    if (rEntry != OpenCLConfig::ImplMatcher())
    {
        rSet.insert(rEntry);
        fillListBox(pListBox, rSet);
    }

    return 0;
}

long SvxOpenCLTabPage::DeleteHdl(ListBox* pListBox, OpenCLConfig::ImplMatcherSet& rSet)
{
    if (pListBox->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
        return 0;

    OpenCLConfig::ImplMatcher rEntry(findCurrentEntry(rSet, pListBox));
    rSet.erase(rEntry);
    fillListBox(pListBox, rSet);

    return 0;
}

IMPL_LINK_NOARG(SvxOpenCLTabPage, BlackListEditHdl)
{
    return EditHdl(mpBlackList, maConfig.maBlackList, "bledit");
}

IMPL_LINK_NOARG(SvxOpenCLTabPage, BlackListAddHdl)
{
    return AddHdl(mpBlackList, maConfig.maBlackList, "bladd");
}

IMPL_LINK_NOARG(SvxOpenCLTabPage, BlackListDeleteHdl)
{
    return DeleteHdl(mpBlackList, maConfig.maBlackList);
}

IMPL_LINK_NOARG(SvxOpenCLTabPage, WhiteListEditHdl)
{
    return EditHdl(mpWhiteList, maConfig.maWhiteList, "wledit");
}

IMPL_LINK_NOARG(SvxOpenCLTabPage, WhiteListAddHdl)
{
    return AddHdl(mpWhiteList, maConfig.maWhiteList, "wladd");
}

IMPL_LINK_NOARG(SvxOpenCLTabPage, WhiteListDeleteHdl)
{
    return DeleteHdl(mpWhiteList, maConfig.maWhiteList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
