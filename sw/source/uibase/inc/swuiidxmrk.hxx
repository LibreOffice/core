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
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>

#include <sfx2/childwin.hxx>
#include "toxe.hxx"
#include <svtools/stdctrl.hxx>
#include <com/sun/star/i18n/XExtendedIndexEntrySupplier.hpp>

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

    OUString        aOrgStr;
    bool            bDel;
    bool            bNewMark;
    bool            bSelected;

    bool            bPhoneticED0_ChangedByUser;
    bool            bPhoneticED1_ChangedByUser;
    bool            bPhoneticED2_ChangedByUser;
    LanguageType    nLangForPhoneticReading; //Language of current text used for phonetic reading proposal
    bool            bIsPhoneticReadingEnabled; //this value states whether phonetic reading is enabled in principle dependent of global cjk settings and language of current entry
    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedIndexEntrySupplier >
                    xExtendedIndexEntrySupplier;

    SwTOXMgr*       pTOXMgr;
    SwWrtShell*     pSh;

    void            Apply();
    void            InitControls();
    void            InsertMark();
    void            UpdateMark();

    DECL_LINK_TYPED( InsertHdl, Button *, void );
    DECL_LINK_TYPED(CloseHdl, Button*, void);
    DECL_LINK_TYPED(DelHdl, Button*, void);
    DECL_LINK_TYPED(NextHdl, Button*, void);
    DECL_LINK_TYPED(NextSameHdl, Button*, void);
    DECL_LINK_TYPED(PrevHdl, Button*, void);
    DECL_LINK_TYPED(PrevSameHdl, Button*, void);
    DECL_LINK_TYPED( ModifyListBoxHdl, ListBox&, void );
    DECL_LINK( ModifyHdl, void* );
    DECL_LINK( KeyDCBModifyHdl, ComboBox * );
    DECL_LINK_TYPED(NewUserIdxHdl, Button*, void);
    DECL_LINK_TYPED( SearchTypeHdl, Button*, void);
    DECL_LINK( PhoneticEDModifyHdl, Edit * );

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

    Dialog &GetDialog() { return m_rDialog; }

    ~SwIndexMarkPane();

    void    ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark* pCurTOXMark = 0);
    bool    IsTOXType(const OUString& rName)
                {return LISTBOX_ENTRY_NOTFOUND != m_pTypeDCB->GetEntryPos(rName);}
};

class SwIndexMarkFloatDlg : public SfxModelessDialog
{
    SwIndexMarkPane m_aContent;
    virtual void    Activate() override;
    public:
        SwIndexMarkFloatDlg(       SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   vcl::Window *pParent,
                                   SfxChildWinInfo* pInfo,
                                   bool bNew=true);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwIndexMarkModalDlg : public SvxStandardDialog
{
    SwIndexMarkPane m_aContent;
public:
    SwIndexMarkModalDlg(vcl::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark);

    virtual void        Apply() override;
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

    bool        bNewEntry;
    bool        bBibAccessInitialized;

    SwWrtShell*     pSh;

    OUString        m_sColumnTitles[AUTH_FIELD_END];
    OUString        m_sFields[AUTH_FIELD_END];

    OUString        m_sCreatedEntry[AUTH_FIELD_END];

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xBibAccess;

    DECL_LINK_TYPED(InsertHdl, Button*, void);
    DECL_LINK_TYPED(CloseHdl, Button*, void);
    DECL_LINK_TYPED(CreateEntryHdl, Button*, void);
    DECL_LINK_TYPED(CompEntryHdl, ListBox&, void);
    DECL_LINK_TYPED(ChangeSourceHdl, Button*, void);
    DECL_LINK_TYPED(IsEntryAllowedHdl, Edit*, bool);
    DECL_LINK(EditModifyHdl, Edit*);

    void InitControls();
    void Activate();
public:

    SwAuthorMarkPane( Dialog &rDialog,
                       bool bNew=true);

    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkFloatDlg : public SfxModelessDialog
{
    SwAuthorMarkPane m_aContent;
    virtual void    Activate() override;
    public:
        SwAuthMarkFloatDlg(        SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   vcl::Window *pParent,
                                   SfxChildWinInfo* pInfo,
                                   bool bNew=true);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg : public SvxStandardDialog
{
    SwAuthorMarkPane m_aContent;
public:
    SwAuthMarkModalDlg(vcl::Window *pParent, SwWrtShell& rSh);

    virtual void        Apply() override;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWUIIDXMRK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
