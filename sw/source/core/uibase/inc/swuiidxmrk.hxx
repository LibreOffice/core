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
#ifndef INCLUDED_SW_SOURCE_UI_INC_SWUIIDXMRK_HXX
#define INCLUDED_SW_SOURCE_UI_INC_SWUIIDXMRK_HXX

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
/*--------------------------------------------------------------------
     Description:   insert mark for index entry
 --------------------------------------------------------------------*/
class SwIndexMarkFloatDlg;
class SwIndexMarkModalDlg;

class SwIndexMarkPane
{
    Dialog& m_rDialog;

    friend class SwIndexMarkFloatDlg;
    friend class SwIndexMarkModalDlg;
    VclFrame*       m_pFrame;
    FixedText*      m_pTypeFT;
    ListBox*        m_pTypeDCB;
    PushButton*     m_pNewBT;

    Edit*           m_pEntryED;
    FixedText*      m_pPhoneticFT0;
    Edit*           m_pPhoneticED0;

    FixedText*      m_pKey1FT;
    ComboBox*       m_pKey1DCB;
    FixedText*      m_pPhoneticFT1;
    Edit*           m_pPhoneticED1;

    FixedText*      m_pKey2FT;
    ComboBox*       m_pKey2DCB;
    FixedText*      m_pPhoneticFT2;
    Edit*           m_pPhoneticED2;

    FixedText*      m_pLevelFT;
    NumericField*   m_pLevelNF;
    CheckBox*       m_pMainEntryCB;
    CheckBox*       m_pApplyToAllCB;
    CheckBox*       m_pSearchCaseSensitiveCB;
    CheckBox*       m_pSearchCaseWordOnlyCB;

    PushButton*     m_pOKBT;
    CloseButton*    m_pCloseBT;
    PushButton*     m_pDelBT;

    PushButton*     m_pPrevSameBT;
    PushButton*     m_pNextSameBT;
    PushButton*     m_pPrevBT;
    PushButton*     m_pNextBT;

    OUString        aOrgStr;
    sal_Bool            bDel;
    sal_Bool            bNewMark;
    sal_Bool            bSelected;

    sal_Bool            bPhoneticED0_ChangedByUser;
    sal_Bool            bPhoneticED1_ChangedByUser;
    sal_Bool            bPhoneticED2_ChangedByUser;
    LanguageType    nLangForPhoneticReading; //Language of current text used for phonetic reading proposal
    sal_Bool            bIsPhoneticReadingEnabled; //this value states whether phonetic reading is enabled in principle dependent of global cjk settings and language of current entry
    com::sun::star::uno::Reference< com::sun::star::i18n::XExtendedIndexEntrySupplier >
                    xExtendedIndexEntrySupplier;

    SwTOXMgr*       pTOXMgr;
    SwWrtShell*     pSh;

    void            Apply();
    void            InitControls();
    void            InsertMark();
    void            UpdateMark();

    DECL_LINK( InsertHdl, Button * );
    DECL_LINK(CloseHdl, void *);
    DECL_LINK(DelHdl, void *);
    DECL_LINK(NextHdl, void *);
    DECL_LINK(NextSameHdl, void *);
    DECL_LINK(PrevHdl, void *);
    DECL_LINK(PrevSameHdl, void *);
    DECL_LINK( ModifyHdl, ListBox* pBox = 0 );
    DECL_LINK( KeyDCBModifyHdl, ComboBox * );
    DECL_LINK(NewUserIdxHdl, void *);
    DECL_LINK( SearchTypeHdl, CheckBox*);
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
                    sal_Bool bNewDlg,
                    SwWrtShell& rWrtShell);

    Dialog &GetDialog() { return m_rDialog; }

    ~SwIndexMarkPane();

    void    ReInitDlg(SwWrtShell& rWrtShell, SwTOXMark* pCurTOXMark = 0);
    sal_Bool    IsTOXType(const OUString& rName)
                {return LISTBOX_ENTRY_NOTFOUND != m_pTypeDCB->GetEntryPos(rName);}
};

class SwIndexMarkFloatDlg : public SfxModelessDialog
{
    SwIndexMarkPane m_aContent;
    virtual void    Activate() SAL_OVERRIDE;
    public:
        SwIndexMarkFloatDlg(    SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew=sal_True);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwIndexMarkModalDlg : public SvxStandardDialog
{
    SwIndexMarkPane m_aContent;
public:
    SwIndexMarkModalDlg(Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark);

    virtual void        Apply() SAL_OVERRIDE;
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg;

class SwAuthorMarkPane
{
    Dialog& m_rDialog;

    static sal_Bool     bIsFromComponent;

    friend class SwAuthMarkModalDlg;
    friend class SwAuthMarkFloatDlg;

    RadioButton*    m_pFromComponentRB;
    RadioButton*    m_pFromDocContentRB;
    FixedText*      m_pAuthorFI;
    FixedText*      m_pTitleFI;
    Edit*           m_pEntryED;
    ListBox*        m_pEntryLB;

    PushButton*     m_pActionBT;
    CloseButton*    m_pCloseBT;
    PushButton*     m_pCreateEntryPB;
    PushButton*     m_pEditEntryPB;

    sal_Bool        bNewEntry;
    sal_Bool        bBibAccessInitialized;

    SwWrtShell*     pSh;

    OUString        m_sColumnTitles[AUTH_FIELD_END];
    OUString        m_sFields[AUTH_FIELD_END];

    OUString        m_sCreatedEntry[AUTH_FIELD_END];

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xBibAccess;

    DECL_LINK(InsertHdl, void *);
    DECL_LINK(CloseHdl, void *);
    DECL_LINK(CreateEntryHdl, PushButton*);
    DECL_LINK(CompEntryHdl, ListBox*);
    DECL_LINK(ChangeSourceHdl, RadioButton*);
    DECL_LINK(IsEntryAllowedHdl, Edit*);
    DECL_LINK(EditModifyHdl, Edit*);

    void InitControls();
    void Activate();
public:

    SwAuthorMarkPane( Dialog &rDialog,
                       sal_Bool bNew=sal_True);

    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkFloatDlg : public SfxModelessDialog
{
    SwAuthorMarkPane m_aContent;
    virtual void    Activate() SAL_OVERRIDE;
    public:
        SwAuthMarkFloatDlg(     SfxBindings* pBindings,
                                   SfxChildWindow* pChild,
                                   Window *pParent,
                                SfxChildWinInfo* pInfo,
                                   sal_Bool bNew=sal_True);
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

class SwAuthMarkModalDlg : public SvxStandardDialog
{
    SwAuthorMarkPane m_aContent;
public:
    SwAuthMarkModalDlg(Window *pParent, SwWrtShell& rSh);

    virtual void        Apply() SAL_OVERRIDE;
    void    ReInitDlg(SwWrtShell& rWrtShell);
};

#endif // INCLUDED_SW_SOURCE_UI_INC_SWUIIDXMRK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
