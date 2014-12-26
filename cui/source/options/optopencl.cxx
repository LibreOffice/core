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
    mpBlackList = new SvSimpleTable( *mpBlackListTable, nBits );
    mpWhiteList = new SvSimpleTable( *mpWhiteListTable, nBits );

    HeaderBar &rBlBar = mpBlackList->GetTheHeaderBar();
    HeaderBar &rWiBar = mpWhiteList->GetTheHeaderBar();


    WinBits nHBits = HIB_LEFT | HIB_VCENTER | HIB_CLICKABLE;
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
    aTabs[2] = rBlBar.GetTextWidth(rBlBar.GetItemText(1)) + 12;
    aTabs[3] = rBlBar.GetTextWidth(rBlBar.GetItemText(2)) + aTabs[2] + 12;
    aTabs[4] = rBlBar.GetTextWidth(rBlBar.GetItemText(3)) + aTabs[3] + 12;
    aTabs[5] = rBlBar.GetTextWidth(rBlBar.GetItemText(4)) + aTabs[4] + 12;
    aTabs[6] = rBlBar.GetTextWidth(rBlBar.GetItemText(5)) + aTabs[5] + 12;
    mpBlackList->SetTabs(aTabs, MAP_PIXEL);
    mpWhiteList->SetTabs(aTabs, MAP_PIXEL);

}

SvxOpenCLTabPage::~SvxOpenCLTabPage()
{
    for ( sal_uInt16 i = 0; i < mpBlackList->GetEntryCount(); ++i )
        delete (OpenCLConfig::ImplMatcher*)mpBlackList->GetEntry(i)->GetUserData();
    for ( sal_uInt16 i = 0; i < mpWhiteList->GetEntryCount(); ++i )
        delete (OpenCLConfig::ImplMatcher*)mpWhiteList->GetEntry(i)->GetUserData();
    delete mpBlackList;
    delete mpWhiteList;
}

SfxTabPage* SvxOpenCLTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
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
    return (rImpl.maOS + "\t" +
            rImpl.maOSVersion + "\t" +
            rImpl.maPlatformVendor + "\t" +
            rImpl.maDevice + "\t" +
            rImpl.maDriverVersion);
}

void fillListBox(SvSimpleTable* pListBox, const OpenCLConfig::ImplMatcherSet& rSet)
{
    pListBox->SetUpdateMode(false);
// kill added UserData to treeitem
    for ( sal_uInt16 i = 0; i < pListBox->GetEntryCount(); ++i )
        delete (OpenCLConfig::ImplMatcher*)pListBox->GetEntry(i)->GetUserData();

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

IMPL_LINK(ListEntryDialog, OSSelectHdl, ListBox*, pListBox)
{
    if (pListBox == mpOS)
    {
        if (mpOS->GetSelectEntryPos() == 0)
            maEntry.maOS.clear();
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

const OpenCLConfig::ImplMatcher& findCurrentEntry(OpenCLConfig::ImplMatcherSet& rSet, SvSimpleTable* pListBox)
{
    SvTreeListEntry* pEntry = pListBox->FirstSelected() ;

    const OpenCLConfig::ImplMatcher* pSet = (OpenCLConfig::ImplMatcher*)pEntry->GetUserData();

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

IMPL_LINK_NOARG(SvxOpenCLTabPage, EnableOpenCLHdl)
{
    EnableOpenCLHdl(mpBlackListFrame, mpUseOpenCL->IsChecked());
    EnableOpenCLHdl(mpWhiteListFrame, mpUseOpenCL->IsChecked());
    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
