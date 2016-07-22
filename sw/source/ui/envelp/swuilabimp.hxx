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

#include "labimp.hxx"
#include <com/sun/star/text/XAutoTextContainer2.hpp>
#include <vcl/layout.hxx>

class SwLabPage : public SfxTabPage
{
    SwDBManager*   pDBManager;
    OUString      sActDBName;
    SwLabItem     aItem;

    VclPtr<VclContainer> m_pAddressFrame;

    VclPtr<CheckBox>         m_pAddrBox;
    VclPtr<VclMultiLineEdit> m_pWritingEdit;
    VclPtr<ListBox>          m_pDatabaseLB;
    VclPtr<ListBox>          m_pTableLB;
    VclPtr<PushButton>       m_pInsertBT;
    VclPtr<ListBox>          m_pDBFieldLB;

    VclPtr<RadioButton>      m_pContButton;
    VclPtr<RadioButton>      m_pSheetButton;
    VclPtr<ListBox>          m_pMakeBox;
    VclPtr<ListBox>          m_pTypeBox;
    VclPtr<ListBox>          m_pHiddenSortTypeBox;
    VclPtr<FixedText>        m_pFormatInfo;

    DECL_LINK_TYPED(AddrHdl, Button*, void);
    DECL_LINK_TYPED(DatabaseHdl, ListBox&, void );
    DECL_LINK_TYPED(FieldHdl, Button *, void);
    DECL_LINK_TYPED(PageHdl, Button *, void);
    DECL_LINK_TYPED(MakeHdl, ListBox&, void);
    DECL_LINK_TYPED(TypeHdl, ListBox&, void);

    void DisplayFormat  ();
    SwLabRec* GetSelectedEntryPos();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwLabPage(vcl::Window* pParent, const SfxItemSet& rSet);

    virtual ~SwLabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    SwLabDlg* GetParentSwLabDlg() {return static_cast<SwLabDlg*>(GetParentDialog());}

    void    SetToBusinessCard();

    void InitDatabaseBox();
    inline void SetDBManager(SwDBManager* pDBManager_) { pDBManager = pDBManager_; }
    inline SwDBManager* GetDBManager() const { return pDBManager; }
};

class SwOneExampleFrame;
class SwVisitingCardPage : public SfxTabPage
{
    VclPtr<SvTreeListBox>  m_pAutoTextLB;
    VclPtr<ListBox>        m_pAutoTextGroupLB;
    VclPtr<vcl::Window>    m_pExampleWIN;

    SwLabItem       aLabItem;

    SwOneExampleFrame*  pExampleFrame;
    css::uno::Reference< css::text::XAutoTextContainer2 > m_xAutoText;

    DECL_LINK_TYPED( AutoTextSelectTreeListBoxHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( AutoTextSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( FrameControlInitializedHdl, SwOneExampleFrame&, void );

    void            InitFrameControl();
    void            UpdateFields();

    void            ClearUserData();

    using SfxTabPage::SetUserData;
    void            SetUserData( sal_uInt32 nCnt,
                                    const OUString* pNames,
                                    const OUString* pValues );

    virtual ~SwVisitingCardPage();
    virtual void dispose() override;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwVisitingCardPage(vcl::Window* pParent, const SfxItemSet& rSet);

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

class SwPrivateDataPage : public SfxTabPage
{
    VclPtr<Edit> m_pFirstNameED;
    VclPtr<Edit> m_pNameED;
    VclPtr<Edit> m_pShortCutED;

    VclPtr<Edit> m_pFirstName2ED;
    VclPtr<Edit> m_pName2ED;
    VclPtr<Edit> m_pShortCut2ED;

    VclPtr<Edit> m_pStreetED;
    VclPtr<Edit> m_pZipED;
    VclPtr<Edit> m_pCityED;
    VclPtr<Edit> m_pCountryED;
    VclPtr<Edit> m_pStateED;
    VclPtr<Edit> m_pTitleED;
    VclPtr<Edit> m_pProfessionED;
    VclPtr<Edit> m_pPhoneED;
    VclPtr<Edit> m_pMobilePhoneED;
    VclPtr<Edit> m_pFaxED;
    VclPtr<Edit> m_pHomePageED;
    VclPtr<Edit> m_pMailED;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwPrivateDataPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwPrivateDataPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

class SwBusinessDataPage : public SfxTabPage
{
    VclPtr<Edit> m_pCompanyED;
    VclPtr<Edit> m_pCompanyExtED;
    VclPtr<Edit> m_pSloganED;

    VclPtr<Edit> m_pStreetED;
    VclPtr<Edit> m_pZipED;
    VclPtr<Edit> m_pCityED;
    VclPtr<Edit> m_pCountryED;
    VclPtr<Edit> m_pStateED;

    VclPtr<Edit> m_pPositionED;

    VclPtr<Edit> m_pPhoneED;
    VclPtr<Edit> m_pMobilePhoneED;
    VclPtr<Edit> m_pFaxED;

    VclPtr<Edit> m_pHomePageED;
    VclPtr<Edit> m_pMailED;

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwBusinessDataPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwBusinessDataPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
