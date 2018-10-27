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
#include <sfx2/basedlgs.hxx>

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/weld.hxx>

#include <sfx2/childwin.hxx>
#include <toxe.hxx>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>
#include <memory>

class SwWrtShell;
class SwTOXMgr;
class SwTOXMark;

// insert mark for index entry
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;

class SwIndexMarkPane
{
    Dialog& m_rDialog;

    friend class SwIndexMarkFloatDlg;
    friend class SwIndexMarkModalDlg;
    VclPtr<VclFrame>       m_pFrame;
    VclPtr<FixedText>      m_pTypeFT;
    VclPtr<ListBox>        m_pTypeDCB;
    VclPtr<PushButton>     m_pNewBT;

    VclPtr<Edit>           m_pEntryED;
    VclPtr<PushButton>     m_pSyncED;
    VclPtr<FixedText>      m_pPhoneticFT0;
    VclPtr<Edit>           m_pPhoneticED0;

    VclPtr<FixedText>      m_pKey1FT;
    VclPtr<ComboBox>       m_pKey1DCB;
    VclPtr<FixedText>      m_pPhoneticFT1;
    VclPtr<Edit>           m_pPhoneticED1;

    VclPtr<FixedText>      m_pKey2FT;
    VclPtr<ComboBox>       m_pKey2DCB;
    VclPtr<FixedText>      m_pPhoneticFT2;
    VclPtr<Edit>           m_pPhoneticED2;

    VclPtr<FixedText>      m_pLevelFT;
    VclPtr<NumericField>   m_pLevelNF;
    VclPtr<CheckBox>       m_pMainEntryCB;
    VclPtr<CheckBox>       m_pApplyToAllCB;
    VclPtr<CheckBox>       m_pSearchCaseSensitiveCB;
    VclPtr<CheckBox>       m_pSearchCaseWordOnlyCB;

    VclPtr<PushButton>     m_pOKBT;
    VclPtr<CloseButton>    m_pCloseBT;
    VclPtr<PushButton>     m_pDelBT;

    VclPtr<PushButton>     m_pPrevSameBT;
    VclPtr<PushButton>     m_pNextSameBT;
    VclPtr<PushButton>     m_pPrevBT;
    VclPtr<PushButton>     m_pNextBT;

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

    void            Apply();
    void            InitControls();
    void            InsertMark();
    void            UpdateMark();

    DECL_LINK( InsertHdl, Button *, void );
    DECL_LINK( CloseHdl, Button*, void );
    DECL_LINK( SyncSelectionHdl, Button*, void );
    DECL_LINK( DelHdl, Button*, void );
    DECL_LINK( NextHdl, Button*, void );
    DECL_LINK( NextSameHdl, Button*, void );
    DECL_LINK( PrevHdl, Button*, void );
    DECL_LINK( PrevSameHdl, Button*, void );
    DECL_LINK( ModifyListBoxHdl, ListBox&, void );
    DECL_LINK( ModifyEditHdl, Edit&, void );
    void ModifyHdl(Control const *);
    DECL_LINK( KeyDCBModifyHdl, Edit&, void );
    DECL_LINK( NewUserIdxHdl, Button*, void );
    DECL_LINK( SearchTypeHdl, Button*, void );
    DECL_LINK( PhoneticEDModifyHdl, Edit&, void );

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

    SwIndexMarkPane(Dialog &rDialog,
                    bool bNewDlg,
                    SwWrtShell& rWrtShell);

    weld::Window* GetFrameWeld() { return m_rDialog.GetFrameWeld(); }

    ~SwIndexMarkPane();

    void    ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark const * pCurTOXMark = nullptr);
    bool    IsTOXType(const OUString& rName)
                {return LISTBOX_ENTRY_NOTFOUND != m_pTypeDCB->GetEntryPos(rName);}
};

class SwIndexMarkFloatDlg : public SfxModelessDialog
{
    SwIndexMarkPane m_aContent;
    virtual void    Activate() override;
public:
    SwIndexMarkFloatDlg( SfxBindings* pBindings,
                         SfxChildWindow* pChild,
                         vcl::Window *pParent,
                         SfxChildWinInfo const * pInfo,
                         bool bNew);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwIndexMarkModalDlg : public SvxStandardDialog
{
    SwIndexMarkPane m_aContent;
public:
    SwIndexMarkModalDlg(vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark const * pCurTOXMark);

    virtual void        Apply() override;
    virtual void        dispose() override;
};

class SwAuthMarkModalDlg;

class SwAuthorMarkPane
{
    Dialog& m_rDialog;

    static bool     bIsFromComponent;

    friend class SwAuthMarkModalDlg;
    friend class SwAuthMarkFloatDlg;

    VclPtr<RadioButton>    m_pFromComponentRB;
    VclPtr<RadioButton>    m_pFromDocContentRB;
    VclPtr<FixedText>      m_pAuthorFI;
    VclPtr<FixedText>      m_pTitleFI;
    VclPtr<Edit>           m_pEntryED;
    VclPtr<ListBox>        m_pEntryLB;

    VclPtr<PushButton>     m_pActionBT;
    VclPtr<CloseButton>    m_pCloseBT;
    VclPtr<PushButton>     m_pCreateEntryPB;
    VclPtr<PushButton>     m_pEditEntryPB;

    bool const      bNewEntry;
    bool            bBibAccessInitialized;

    SwWrtShell*     pSh;

    OUString        m_sColumnTitles[AUTH_FIELD_END];
    OUString        m_sFields[AUTH_FIELD_END];

    OUString        m_sCreatedEntry[AUTH_FIELD_END];

    css::uno::Reference< css::container::XNameAccess >    xBibAccess;

    DECL_LINK(InsertHdl, Button*, void);
    DECL_LINK(CloseHdl, Button*, void);
    DECL_LINK(CreateEntryHdl, Button*, void);
    DECL_LINK(CompEntryHdl, ListBox&, void);
    DECL_LINK(ChangeSourceHdl, Button*, void);
    DECL_LINK(IsEditAllowedHdl, Edit*, bool);
    DECL_LINK(IsEntryAllowedHdl, weld::Entry&, bool);
    DECL_LINK(EditModifyHdl, Edit&, void);

    void InitControls();
    void Activate();

public:
    SwAuthorMarkPane( Dialog &rDialog, bool bNew);

    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class AuthorMarkPane
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

public:
    AuthorMarkPane(weld::DialogController& rDialog, weld::Builder& rBuilder, bool bNew);

    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkFloatDlg : public SfxModelessDialog
{
    SwAuthorMarkPane m_aContent;
    virtual void    Activate() override;
public:
    SwAuthMarkFloatDlg( SfxBindings* pBindings,
                        SfxChildWindow* pChild,
                        vcl::Window *pParent,
                        SfxChildWinInfo const * pInfo,
                        bool bNew);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg : public SfxDialogController
{
    AuthorMarkPane m_aContent;
    void Apply();
public:
    SwAuthMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh);

    virtual short int run() override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWUIIDXMRK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
