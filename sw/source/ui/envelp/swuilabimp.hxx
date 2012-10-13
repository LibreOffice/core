/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SWUILABIMP_HXX
#define _SWUILABIMP_HXX
#include "labimp.hxx"

class SwLabPage : public SfxTabPage
{
    SwNewDBMgr*   pNewDBMgr;
    String        sActDBName;
    SwLabItem     aItem;

    FixedLine     aWritingFL;
    FixedText     aWritingText;
    CheckBox      aAddrBox;
    MultiLineEdit aWritingEdit;
    FixedText     aDatabaseFT;
    ListBox       aDatabaseLB;
    FixedText     aTableFT;
    ListBox       aTableLB;
    ImageButton   aInsertBT;
    FixedText     aDBFieldFT;
    ListBox       aDBFieldLB;
//  PushButton    aDatabaseButton;

    FixedLine     aFormatFL;
    RadioButton   aContButton;
    RadioButton   aSheetButton;
    FixedText     aMakeText;
    ListBox       aMakeBox;
    FixedText     aTypeText;
    ListBox       aTypeBox;
    ListBox       aHiddenSortTypeBox;
    FixedInfo     aFormatInfo;

    sal_Bool        m_bLabel;

     SwLabPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabPage();

    DECL_LINK(AddrHdl, void *);
    DECL_LINK( DatabaseHdl, ListBox *pListBox );
//    DECL_LINK( DatabaseButtonHdl, Button * );
    DECL_LINK(FieldHdl, void *);
    DECL_LINK(PageHdl, void *);
    DECL_LINK(MakeHdl, void *);
    DECL_LINK(TypeHdl, void *);

    void DisplayFormat  ();
    SwLabRec* GetSelectedEntryPos();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwLabItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);

    SwLabDlg* GetParentSwLabDlg() {return (SwLabDlg*) GetParentDialog();}

    void    SetToBusinessCard();

    void InitDatabaseBox();
    inline void SetNewDBMgr(SwNewDBMgr* pDBMgr) { pNewDBMgr = pDBMgr; }
    inline SwNewDBMgr* GetNewDBMgr() const { return pNewDBMgr; }
};

class SwOneExampleFrame;
class SwVisitingCardPage : public SfxTabPage
{
    FixedLine       aContentFL;
    SvTreeListBox   aAutoTextLB;
    FixedText       aAutoTextGroupFT;
    ListBox         aAutoTextGroupLB;

    Window          aExampleWIN;

    String          sVisCardGroup;
    String          sTempURL;

    SwLabItem       aLabItem;

    SwOneExampleFrame*  pExampleFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    _xAutoText;


    DECL_LINK( AutoTextSelectHdl, void* );
    DECL_LINK( FrameControlInitializedHdl, void* );

    void            InitFrameControl();
    void            UpdateFields();

    void            ClearUserData();

    using SfxTabPage::SetUserData;
    void            SetUserData( sal_uInt32 nCnt,
                                    const rtl::OUString* pNames,
                                    const rtl::OUString* pValues );

    SwVisitingCardPage(Window* pParent, const SfxItemSet& rSet);
    ~SwVisitingCardPage();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

class SwPrivateDataPage : public SfxTabPage
{
    FixedLine       aDataFL;

    FixedText       aNameFT;
    Edit            aFirstNameED;
    Edit            aNameED;
    Edit            aShortCutED;

    FixedText       aName2FT;
    Edit            aFirstName2ED;
    Edit            aName2ED;
    Edit            aShortCut2ED;

    FixedText       aStreetFT;
    Edit            aStreetED;
    FixedText       aZipCityFT;
    Edit            aZipED;
    Edit            aCityED;
    FixedText       aCountryStateFT;
    Edit            aCountryED;
    Edit            aStateED;
    FixedText       aTitleProfessionFT;
    Edit            aTitleED;
    Edit            aProfessionED;
    FixedText       aPhoneFT;
    Edit            aPhoneED;
    Edit            aMobilePhoneED;
    FixedText       aFaxFT;
    Edit            aFaxED;
    FixedText       aWWWMailFT;
    Edit            aHomePageED;
    Edit            aMailED;

    SwPrivateDataPage(Window* pParent, const SfxItemSet& rSet);
    ~SwPrivateDataPage();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

class SwBusinessDataPage : public SfxTabPage
{
    FixedLine       aDataFL;
    FixedText       aCompanyFT;
    Edit            aCompanyED;
    FixedText       aCompanyExtFT;
    Edit            aCompanyExtED;
    FixedText       aSloganFT;
    Edit            aSloganED;

    FixedText       aStreetFT;
    Edit            aStreetED;
    FixedText       aZipCityFT;
    Edit            aZipED;
    Edit            aCityED;
    FixedText       aCountryStateFT;
    Edit            aCountryED;
    Edit            aStateED;

    FixedText       aPositionFT;
    Edit            aPositionED;

    FixedText       aPhoneFT;
    Edit            aPhoneED;
    Edit            aMobilePhoneED;
    FixedText       aFaxFT;
    Edit            aFaxED;

    FixedText       aWWWMailFT;
    Edit            aHomePageED;
    Edit            aMailED;

    SwBusinessDataPage(Window* pParent, const SfxItemSet& rSet);
    ~SwBusinessDataPage();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
