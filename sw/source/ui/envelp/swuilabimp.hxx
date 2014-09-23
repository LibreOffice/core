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

#include "../../uibase/envelp/labimp.hxx"
#include <com/sun/star/text/XAutoTextContainer2.hpp>
#include <vcl/layout.hxx>

class SwLabPage : public SfxTabPage
{
    SwDBManager*   pDBManager;
    OUString      sActDBName;
    SwLabItem     aItem;

    VclContainer* m_pAddressFrame;

    CheckBox*         m_pAddrBox;
    VclMultiLineEdit* m_pWritingEdit;
    ListBox*          m_pDatabaseLB;
    ListBox*          m_pTableLB;
    PushButton*       m_pInsertBT;
    ListBox*          m_pDBFieldLB;

    RadioButton*      m_pContButton;
    RadioButton*      m_pSheetButton;
    ListBox*          m_pMakeBox;
    ListBox*          m_pTypeBox;
    ListBox*          m_pHiddenSortTypeBox;
    FixedText*        m_pFormatInfo;

    bool        m_bLabel;

    SwLabPage(vcl::Window* pParent, const SfxItemSet& rSet);

    DECL_LINK(AddrHdl, void *);
    DECL_LINK( DatabaseHdl, ListBox *pListBox );
    DECL_LINK(FieldHdl, void *);
    DECL_LINK(PageHdl, void *);
    DECL_LINK(MakeHdl, void *);
    DECL_LINK(TypeHdl, void *);

    void DisplayFormat  ();
    SwLabRec* GetSelectedEntryPos();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rSet) SAL_OVERRIDE;

    SwLabDlg* GetParentSwLabDlg() {return (SwLabDlg*) GetParentDialog();}

    void    SetToBusinessCard();

    void InitDatabaseBox();
    inline void SetDBManager(SwDBManager* pDBManager_) { pDBManager = pDBManager_; }
    inline SwDBManager* GetDBManager() const { return pDBManager; }
};

class SwOneExampleFrame;
class SwVisitingCardPage : public SfxTabPage
{
    SvTreeListBox*  m_pAutoTextLB;
    ListBox*        m_pAutoTextGroupLB;
    vcl::Window*         m_pExampleWIN;

    OUString        sTempURL;

    SwLabItem       aLabItem;

    SwOneExampleFrame*  pExampleFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextContainer2 > m_xAutoText;

    DECL_LINK( AutoTextSelectHdl, void* );
    DECL_LINK( FrameControlInitializedHdl, void* );

    void            InitFrameControl();
    void            UpdateFields();

    void            ClearUserData();

    using SfxTabPage::SetUserData;
    void            SetUserData( sal_uInt32 nCnt,
                                    const OUString* pNames,
                                    const OUString* pValues );

    SwVisitingCardPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwVisitingCardPage();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
    virtual bool FillItemSet(SfxItemSet* rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rSet) SAL_OVERRIDE;
};

class SwPrivateDataPage : public SfxTabPage
{
    Edit* m_pFirstNameED;
    Edit* m_pNameED;
    Edit* m_pShortCutED;

    Edit* m_pFirstName2ED;
    Edit* m_pName2ED;
    Edit* m_pShortCut2ED;

    Edit* m_pStreetED;
    Edit* m_pZipED;
    Edit* m_pCityED;
    Edit* m_pCountryED;
    Edit* m_pStateED;
    Edit* m_pTitleED;
    Edit* m_pProfessionED;
    Edit* m_pPhoneED;
    Edit* m_pMobilePhoneED;
    Edit* m_pFaxED;
    Edit* m_pHomePageED;
    Edit* m_pMailED;

    SwPrivateDataPage(vcl::Window* pParent, const SfxItemSet& rSet);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
    virtual bool FillItemSet(SfxItemSet* rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rSet) SAL_OVERRIDE;
};

class SwBusinessDataPage : public SfxTabPage
{
    Edit* m_pCompanyED;
    Edit* m_pCompanyExtED;
    Edit* m_pSloganED;

    Edit* m_pStreetED;
    Edit* m_pZipED;
    Edit* m_pCityED;
    Edit* m_pCountryED;
    Edit* m_pStateED;

    Edit* m_pPositionED;

    Edit* m_pPhoneED;
    Edit* m_pMobilePhoneED;
    Edit* m_pFaxED;

    Edit* m_pHomePageED;
    Edit* m_pMailED;

    SwBusinessDataPage(vcl::Window* pParent, const SfxItemSet& rSet);

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
    virtual bool FillItemSet(SfxItemSet* rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet* rSet) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
