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
#include <svtools/simptabl.hxx>
#include "optHeaderTabListbox.hxx"

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/setup/UpdateCheckConfig.hpp>

#include "cuires.hrc"
#include "optopencl.hxx"
#include <svtools/treelistentry.hxx>

SvxOpenCLTabPage::SvxOpenCLTabPage(vcl::Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, "OptOpenCLPage", "cui/ui/optopenclpage.ui", &rSet),
    maConfig(OpenCLConfig::get())
{
    get(mpUseOpenCL, "useopencl");
    get(mpBlackListTable, "blacklist");
    get(mpBlackListFrame,"blacklistframe");
    get(mpBlackListEdit, "bledit");
    get(mpBlackListAdd, "bladd");
    get(mpBlackListDelete, "bldelete");
    get(mpWhiteListTable, "whitelist");
    get(mpWhiteListFrame,"whitelistframe");
    get(mpWhiteListEdit, "wledit");
    get(mpWhiteListAdd, "wladd");
    get(mpWhiteListDelete, "wldelete");
    get(mpOS,"os");
    get(mpOSVersion,"osversion");
    get(mpDevice,"device");
    get(mpVendor,"vendor");
    get(mpDrvVersion,"driverversion");

    mpUseOpenCL->Check(maConfig.mbUseOpenCL);
    mpUseOpenCL->SetClickHdl(LINK(this, SvxOpenCLTabPage, EnableOpenCLHdl));

    mpBlackListEdit->SetClickHdl(LINK(this, SvxOpenCLTabPage, BlackListEditHdl));
    mpBlackListAdd->SetClickHdl(LINK(this, SvxOpenCLTabPage, BlackListAddHdl));
    mpBlackListDelete->SetClickHdl(LINK(this, SvxOpenCLTabPage, BlackListDeleteHdl));

    mpWhiteListEdit->SetClickHdl(LINK(this, SvxOpenCLTabPage, WhiteListEditHdl));
    mpWhiteListAdd->SetClickHdl(LINK(this, SvxOpenCLTabPage, WhiteListAddHdl));
    mpWhiteListDelete->SetClickHdl(LINK(this, SvxOpenCLTabPage, WhiteListDeleteHdl));

    WinBits nBits = WB_SORT | WB_HSCROLL | WB_CLIPCHILDREN | WB_TABSTOP;
    mpBlackList = VclPtr<SvSimpleTable>::Create( *mpBlackListTable, nBits );
    mpWhiteList = VclPtr<SvSimpleTable>::Create( *mpWhiteListTable, nBits );

    HeaderBar &rBlBar = mpBlackList->GetTheHeaderBar();
    HeaderBar &rWiBar = mpWhiteList->GetTheHeaderBar();

    HeaderBarItemBits nHBits = HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER | HeaderBarItemBits::CLICKABLE;
    rBlBar.InsertItem( 1, mpOS->GetText(),         0, nHBits );
    rBlBar.InsertItem( 2, mpOSVersion->GetText(),  0, nHBits );
    rBlBar.InsertItem( 3, mpVendor->GetText(),     0, nHBits );
    rBlBar.InsertItem( 4, mpDevice->GetText(),     0, nHBits );
    rBlBar.InsertItem( 5, mpDrvVersion->GetText(), 0, nHBits );

    rWiBar.InsertItem( 1, mpOS->GetText(),         0, nHBits );
    rWiBar.InsertItem( 2, mpOSVersion->GetText(),  0, nHBits );
    rWiBar.InsertItem( 3, mpVendor->GetText(),     0, nHBits );
    rWiBar.InsertItem( 4, mpDevice->GetText(),     0, nHBits );
    rWiBar.InsertItem( 5, mpDrvVersion->GetText(), 0, nHBits );

    long aTabs[] = {5, 0, 0, 0, 0, 0};
    aTabs[1] = rBlBar.GetTextWidth(rBlBar.GetItemText(1)) + 12;
    aTabs[2] = rBlBar.GetTextWidth(rBlBar.GetItemText(2)) + aTabs[1] + 12;
    aTabs[3] = rBlBar.GetTextWidth(rBlBar.GetItemText(3)) + aTabs[2] + 12;
    aTabs[4] = rBlBar.GetTextWidth(rBlBar.GetItemText(4)) + aTabs[3] + 12;
    aTabs[5] = rBlBar.GetTextWidth(rBlBar.GetItemText(5)) + aTabs[4] + 12;
    mpBlackList->SetTabs(aTabs, MAP_PIXEL);
    mpWhiteList->SetTabs(aTabs, MAP_PIXEL);

}

SvxOpenCLTabPage::~SvxOpenCLTabPage()
{
    disposeOnce();
}

void SvxOpenCLTabPage::dispose()
{
   for ( sal_uLong i = 0; i < mpBlackList->GetEntryCount(); ++i )
        delete static_cast<OpenCLConfig::ImplMatcher*>(mpBlackList->GetEntry(i)->GetUserData());
    for ( sal_uLong i = 0; i < mpWhiteList->GetEntryCount(); ++i )
        delete static_cast<OpenCLConfig::ImplMatcher*>(mpWhiteList->GetEntry(i)->GetUserData());
    mpBlackList.disposeAndClear();
    mpWhiteList.disposeAndClear();

    mpUseOpenCL.clear();
    mpBlackListFrame.clear();
    mpBlackListTable.clear();
    mpBlackListEdit.clear();
    mpBlackListAdd.clear();
    mpBlackListDelete.clear();
    mpOS.clear();
    mpOSVersion.clear();
    mpDevice.clear();
    mpVendor.clear();
    mpDrvVersion.clear();
    mpWhiteListFrame.clear();
    mpWhiteListTable.clear();
    mpWhiteListEdit.clear();
    mpWhiteListAdd.clear();
    mpWhiteListDelete.clear();

    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxOpenCLTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxOpenCLTabPage>::Create(pParent, *rAttrSet);
}

bool SvxOpenCLTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());

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
    return (rImpl.maOS + "\t" +
            rImpl.maOSVersion + "\t" +
            rImpl.maPlatformVendor + "\t" +
            rImpl.maDevice + "\t" +
            rImpl.maDriverVersion);
}

void fillListBox(SvSimpleTable* pListBox, const OpenCLConfig::ImplMatcherSet& rSet)
{
    pListBox->SetUpdateMode(false);
// kill added UserData to tree item
    for ( sal_uLong i = 0; i < pListBox->GetEntryCount(); ++i )
        delete static_cast<OpenCLConfig::ImplMatcher*>(pListBox->GetEntry(i)->GetUserData());

    pListBox->Clear();
    for (auto i = rSet.cbegin(); i != rSet.cend(); ++i)
    {
        OpenCLConfig::ImplMatcher* pImpl = new OpenCLConfig::ImplMatcher(*i);
        pListBox->InsertEntry(format(*i), NULL, false, TREELIST_APPEND, pImpl);
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

    VclPtr<ListBox> mpOS;
    VclPtr<Edit> mpOSVersion;
    VclPtr<Edit> mpPlatformVendor;
    VclPtr<Edit> mpDevice;
    VclPtr<Edit> mpDriverVersion;

    DECL_LINK_TYPED(OSSelectHdl, ListBox&, void);
    DECL_LINK(EditModifiedHdl, Edit*);

    ListEntryDialog(vcl::Window* pParent, const OpenCLConfig::ImplMatcher& rEntry, const OString& rTag);
    virtual ~ListEntryDialog() { disposeOnce(); }
    virtual void dispose() SAL_OVERRIDE
    {
        mpOS.clear();
        mpOSVersion.clear();
        mpPlatformVendor.clear();
        mpDevice.clear();
        mpDriverVersion.clear();
        ModalDialog::dispose();
    }

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
    if (maEntry.maOS.isEmpty())
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

IMPL_LINK_TYPED(ListEntryDialog, OSSelectHdl, ListBox&, rListBox, void)
{
    if (&rListBox == mpOS)
    {
        if (mpOS->GetSelectEntryPos() == 0)
            maEntry.maOS.clear();
        else
            maEntry.maOS = mpOS->GetSelectEntry();
    }
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
    ScopedVclPtrInstance< ListEntryDialog > aDlg(pTabPage, rEntry, rTag);

    if (aDlg->Execute() == RET_OK)
        rEntry = aDlg->maEntry;
}

const OpenCLConfig::ImplMatcher& findCurrentEntry(OpenCLConfig::ImplMatcherSet& rSet, SvSimpleTable* pListBox)
{
    SvTreeListEntry* pEntry = pListBox->FirstSelected() ;

    const OpenCLConfig::ImplMatcher* pSet = static_cast<OpenCLConfig::ImplMatcher*>(pEntry->GetUserData());

    for (auto i = rSet.cbegin(); i != rSet.cend(); ++i)
    {
        if (*i == *pSet)
        {
            return *i;
        }
    }
    auto i = rSet.begin();
    return *i;
}

}

long SvxOpenCLTabPage::EditHdl(SvSimpleTable* pListBox, OpenCLConfig::ImplMatcherSet& rSet, const OString& rTag)
{
    if (!pListBox->FirstSelected())
        return 0;

    OpenCLConfig::ImplMatcher rEntry(findCurrentEntry(rSet, pListBox));
    rSet.erase(rEntry);
    openListDialog(this, rEntry, rTag);
    rSet.insert(rEntry);
    fillListBox(pListBox, rSet);

    return 0;
}

long SvxOpenCLTabPage::AddHdl(SvSimpleTable* pListBox, OpenCLConfig::ImplMatcherSet& rSet, const OString& rTag)
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

long SvxOpenCLTabPage::DeleteHdl(SvSimpleTable* pListBox, OpenCLConfig::ImplMatcherSet& rSet)
{
    if (!pListBox->FirstSelected())
        return 0;

    OpenCLConfig::ImplMatcher rEntry(findCurrentEntry(rSet, pListBox));
    rSet.erase(rEntry);
    fillListBox(pListBox, rSet);

    return 0;
}

void SvxOpenCLTabPage::EnableOpenCLHdl(VclFrame* pFrame, bool aEnable)
{
    pFrame->Enable(aEnable);
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, BlackListEditHdl, Button*, void)
{
    EditHdl(mpBlackList, maConfig.maBlackList, "bledit");
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, BlackListAddHdl, Button*, void)
{
    AddHdl(mpBlackList, maConfig.maBlackList, "bladd");
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, BlackListDeleteHdl, Button*, void)
{
    DeleteHdl(mpBlackList, maConfig.maBlackList);
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, WhiteListEditHdl, Button*, void)
{
    EditHdl(mpWhiteList, maConfig.maWhiteList, "wledit");
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, WhiteListAddHdl, Button*, void)
{
    AddHdl(mpWhiteList, maConfig.maWhiteList, "wladd");
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, WhiteListDeleteHdl, Button*, void)
{
    DeleteHdl(mpWhiteList, maConfig.maWhiteList);
}

IMPL_LINK_NOARG_TYPED(SvxOpenCLTabPage, EnableOpenCLHdl, Button*, void)
{
    EnableOpenCLHdl(mpBlackListFrame, mpUseOpenCL->IsChecked());
    EnableOpenCLHdl(mpWhiteListFrame, mpUseOpenCL->IsChecked());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
