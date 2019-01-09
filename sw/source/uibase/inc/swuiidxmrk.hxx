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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWUIIDXMRK_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWUIIDXMRK_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <svx/stddlg.hxx>
#include <vcl/weld.hxx>
#include <toxe.hxx>
#include <memory>

class SwWrtShell;
class SwTOXMgr;
class SwTOXMark;

// insert mark for index entry
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;

class SwIndexMarkPane
{
    std::shared_ptr<weld::Dialog> m_xDialog;

    friend class SwIndexMarkFloatDlg;
    friend class SwIndexMarkModalDlg;

    OUString        m_aOrgStr;
    bool            m_bDel;
    bool const      m_bNewMark;
    bool            m_bSelected;

    bool            m_bPhoneticED0_ChangedByUser;
    bool            m_bPhoneticED1_ChangedByUser;
    bool            m_bPhoneticED2_ChangedByUser;
    LanguageType    m_nLangForPhoneticReading; //Language of current text used for phonetic reading proposal
    bool            m_bIsPhoneticReadingEnabled; //this value states whether phonetic reading is enabled in principle dependent of global cjk settings and language of current entry
    css::uno::Reference< css::i18n::XExtendedIndexEntrySupplier >
                    m_xExtendedIndexEntrySupplier;

    std::unique_ptr<SwTOXMgr>
                    m_pTOXMgr;
    SwWrtShell*     m_pSh;

    std::unique_ptr<weld::Label> m_xTypeFT;
    std::unique_ptr<weld::ComboBox> m_xTypeDCB;
    std::unique_ptr<weld::Button> m_xNewBT;
    std::unique_ptr<weld::Entry> m_xEntryED;
    std::unique_ptr<weld::Button> m_xSyncED;
    std::unique_ptr<weld::Label> m_xPhoneticFT0;
    std::unique_ptr<weld::Entry> m_xPhoneticED0;
    std::unique_ptr<weld::Label> m_xKey1FT;
    std::unique_ptr<weld::ComboBox> m_xKey1DCB;
    std::unique_ptr<weld::Label> m_xPhoneticFT1;
    std::unique_ptr<weld::Entry> m_xPhoneticED1;
    std::unique_ptr<weld::Label> m_xKey2FT;
    std::unique_ptr<weld::ComboBox> m_xKey2DCB;
    std::unique_ptr<weld::Label> m_xPhoneticFT2;
    std::unique_ptr<weld::Entry> m_xPhoneticED2;
    std::unique_ptr<weld::Label> m_xLevelFT;
    std::unique_ptr<weld::SpinButton> m_xLevelNF;
    std::unique_ptr<weld::CheckButton> m_xMainEntryCB;
    std::unique_ptr<weld::CheckButton> m_xApplyToAllCB;
    std::unique_ptr<weld::CheckButton> m_xSearchCaseSensitiveCB;
    std::unique_ptr<weld::CheckButton> m_xSearchCaseWordOnlyCB;
    std::unique_ptr<weld::Button> m_xOKBT;
    std::unique_ptr<weld::Button> m_xCloseBT;
    std::unique_ptr<weld::Button> m_xDelBT;
    std::unique_ptr<weld::Button> m_xPrevSameBT;
    std::unique_ptr<weld::Button> m_xNextSameBT;
    std::unique_ptr<weld::Button> m_xPrevBT;
    std::unique_ptr<weld::Button> m_xNextBT;

    void            Apply();
    void            InitControls();
    void            InsertMark();
    void            UpdateMark();

    DECL_LINK(InsertHdl, weld::Button&, void);
    DECL_LINK(CloseHdl, weld::Button&, void);
    DECL_LINK(SyncSelectionHdl, weld::Button&, void);
    DECL_LINK(DelHdl, weld::Button&, void);
    DECL_LINK( NextHdl, weld::Button&, void );
    DECL_LINK( NextSameHdl, weld::Button&, void );
    DECL_LINK( PrevHdl, weld::Button&, void );
    DECL_LINK( PrevSameHdl, weld::Button&, void );
    DECL_LINK( ModifyListBoxHdl, weld::ComboBox&, void );
    DECL_LINK( ModifyEditHdl, weld::Entry&, void );
    void ModifyHdl(const weld::Widget& rWidget);
    DECL_LINK( KeyDCBModifyHdl, weld::ComboBox&, void );
    DECL_LINK( NewUserIdxHdl, weld::Button&, void );
    DECL_LINK( SearchTypeHdl, weld::ToggleButton&, void );
    DECL_LINK( PhoneticEDModifyHdl, weld::Entry&, void );

    //this method updates the values from 'nLangForPhoneticReading' and 'bIsPhoneticReadingEnabled'
    //it needs to be called ones if this dialog is opened to create a new entry (in InitControls),
    //or otherwise it has to be called for each changed TOXMark (in UpdateDialog)
    void            UpdateLanguageDependenciesForPhoneticReading();
    OUString        GetDefaultPhoneticReading( const OUString& rText );

    void            UpdateKeyBoxes();

    void            UpdateDialog();
    void            InsertUpdate();

    void            Activate();

public:

    SwIndexMarkPane(const std::shared_ptr<weld::Dialog>& rDialog, weld::Builder& rBuilder,
                    bool bNewDlg, SwWrtShell& rWrtShell);

    ~SwIndexMarkPane();

    void    ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark const * pCurTOXMark = nullptr);
    bool    IsTOXType(const OUString& rName) { return m_xTypeDCB->find_text(rName) != -1; }
};

class SwIndexMarkFloatDlg : public SfxModelessDialogController
{
    SwIndexMarkPane m_aContent;

    virtual void    Activate() override;
public:
    SwIndexMarkFloatDlg(SfxBindings* pBindings,
                        SfxChildWindow* pChild,
                        weld::Window *pParent,
                        SfxChildWinInfo const * pInfo,
                        bool bNew);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwIndexMarkModalDlg : public SfxDialogController
{
    SwIndexMarkPane m_aContent;
public:
    SwIndexMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh, SwTOXMark const * pCurTOXMark);
    virtual ~SwIndexMarkModalDlg() override;
    virtual short int run() override;
};

class SwAuthMarkModalDlg;

class SwAuthorMarkPane
{
    weld::DialogController& m_rDialog;

    static bool     bIsFromComponent;

    friend class SwAuthMarkModalDlg;
    friend class SwAuthMarkFloatDlg;

    bool const      bNewEntry;
    bool            bBibAccessInitialized;

    SwWrtShell*     pSh;

    OUString        m_sColumnTitles[AUTH_FIELD_END];
    OUString        m_sFields[AUTH_FIELD_END];

    OUString        m_sCreatedEntry[AUTH_FIELD_END];

    css::uno::Reference< css::container::XNameAccess >    xBibAccess;

    std::unique_ptr<weld::RadioButton> m_xFromComponentRB;
    std::unique_ptr<weld::RadioButton> m_xFromDocContentRB;
    std::unique_ptr<weld::Label> m_xAuthorFI;
    std::unique_ptr<weld::Label> m_xTitleFI;
    std::unique_ptr<weld::Entry> m_xEntryED;
    std::unique_ptr<weld::ComboBox> m_xEntryLB;
    std::unique_ptr<weld::Button> m_xActionBT;
    std::unique_ptr<weld::Button> m_xCloseBT;
    std::unique_ptr<weld::Button> m_xCreateEntryPB;
    std::unique_ptr<weld::Button> m_xEditEntryPB;

    DECL_LINK(InsertHdl, weld::Button&, void);
    DECL_LINK(CloseHdl, weld::Button&, void);
    DECL_LINK(CreateEntryHdl, weld::Button&, void);
    DECL_LINK(CompEntryHdl, weld::ComboBox&, void);
    DECL_LINK(ChangeSourceHdl, weld::ToggleButton&, void);
    DECL_LINK(IsEditAllowedHdl, weld::Entry&, bool);
    DECL_LINK(IsEntryAllowedHdl, weld::Entry&, bool);
    DECL_LINK(EditModifyHdl, weld::Entry&, void);

    void InitControls();
    void Activate();

public:
    SwAuthorMarkPane(weld::DialogController& rDialog, weld::Builder& rBuilder, bool bNew);
    void ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkFloatDlg : public SfxModelessDialogController
{
    SwAuthorMarkPane m_aContent;
    virtual void    Activate() override;
public:
    SwAuthMarkFloatDlg(SfxBindings* pBindings,
                       SfxChildWindow* pChild,
                       weld::Window *pParent,
                       SfxChildWinInfo const * pInfo,
                       bool bNew);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg : public SfxDialogController
{
    SwAuthorMarkPane m_aContent;

    void Apply();
public:
    SwAuthMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh);
    virtual short int run() override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWUIIDXMRK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
