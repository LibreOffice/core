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
#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_SWUILABIMP_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_SWUILABIMP_HXX

#include <label.hxx>
#include <labimg.hxx>
#include <labrec.hxx>
#include <sfx2/tabdlg.hxx>

class SwLabPage : public SfxTabPage
{
    SwDBManager*   pDBManager;
    OUString      sActDBName;
    SwLabItem     aItem;

    std::unique_ptr<weld::Widget> m_xAddressFrame;
    std::unique_ptr<weld::CheckButton> m_xAddrBox;
    std::unique_ptr<weld::TextView> m_xWritingEdit;
    std::unique_ptr<weld::ComboBox> m_xDatabaseLB;
    std::unique_ptr<weld::ComboBox> m_xTableLB;
    std::unique_ptr<weld::Button> m_xInsertBT;
    std::unique_ptr<weld::ComboBox> m_xDBFieldLB;
    std::unique_ptr<weld::RadioButton> m_xContButton;
    std::unique_ptr<weld::RadioButton> m_xSheetButton;
    std::unique_ptr<weld::ComboBox> m_xMakeBox;
    std::unique_ptr<weld::ComboBox> m_xTypeBox;
    std::unique_ptr<weld::ComboBox> m_xHiddenSortTypeBox;
    std::unique_ptr<weld::Label> m_xFormatInfo;

    DECL_LINK(AddrHdl, weld::ToggleButton&, void);
    DECL_LINK(DatabaseHdl, weld::ComboBox&, void );
    DECL_LINK(FieldHdl, weld::Button&, void);
    DECL_LINK(PageHdl, weld::ToggleButton&, void);
    DECL_LINK(MakeHdl, weld::ComboBox&, void);
    DECL_LINK(TypeHdl, weld::ComboBox&, void);

    void DisplayFormat  ();
    SwLabRec* GetSelectedEntryPos();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwLabPage(TabPageParent pParent, const SfxItemSet& rSet);

    virtual ~SwLabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    SwLabDlg* GetParentSwLabDlg() {return static_cast<SwLabDlg*>(GetDialogController());}

    void    SetToBusinessCard();

    void InitDatabaseBox();
    void SetDBManager(SwDBManager* pDBManager_) { pDBManager = pDBManager_; }
    SwDBManager* GetDBManager() const { return pDBManager; }
};

class SwPrivateDataPage : public SfxTabPage
{
    std::unique_ptr<weld::Entry> m_xFirstNameED;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Entry> m_xShortCutED;
    std::unique_ptr<weld::Entry> m_xFirstName2ED;
    std::unique_ptr<weld::Entry> m_xName2ED;
    std::unique_ptr<weld::Entry> m_xShortCut2ED;
    std::unique_ptr<weld::Entry> m_xStreetED;
    std::unique_ptr<weld::Entry> m_xZipED;
    std::unique_ptr<weld::Entry> m_xCityED;
    std::unique_ptr<weld::Entry> m_xCountryED;
    std::unique_ptr<weld::Entry> m_xStateED;
    std::unique_ptr<weld::Entry> m_xTitleED;
    std::unique_ptr<weld::Entry> m_xProfessionED;
    std::unique_ptr<weld::Entry> m_xPhoneED;
    std::unique_ptr<weld::Entry> m_xMobilePhoneED;
    std::unique_ptr<weld::Entry> m_xFaxED;
    std::unique_ptr<weld::Entry> m_xHomePageED;
    std::unique_ptr<weld::Entry> m_xMailED;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwPrivateDataPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwPrivateDataPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

class SwBusinessDataPage : public SfxTabPage
{
    std::unique_ptr<weld::Entry> m_xCompanyED;
    std::unique_ptr<weld::Entry> m_xCompanyExtED;
    std::unique_ptr<weld::Entry> m_xSloganED;
    std::unique_ptr<weld::Entry> m_xStreetED;
    std::unique_ptr<weld::Entry> m_xZipED;
    std::unique_ptr<weld::Entry> m_xCityED;
    std::unique_ptr<weld::Entry> m_xCountryED;
    std::unique_ptr<weld::Entry> m_xStateED;
    std::unique_ptr<weld::Entry> m_xPositionED;
    std::unique_ptr<weld::Entry> m_xPhoneED;
    std::unique_ptr<weld::Entry> m_xMobilePhoneED;
    std::unique_ptr<weld::Entry> m_xFaxED;
    std::unique_ptr<weld::Entry> m_xHomePageED;
    std::unique_ptr<weld::Entry> m_xMailED;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwBusinessDataPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwBusinessDataPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
