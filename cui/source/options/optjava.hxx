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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTJAVA_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTJAVA_HXX

#include <config_features.h>

#include <memory>
#include <vector>
#include <ucbhelper/content.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>
#include <svtools/simptabl.hxx>
#include "radiobtnbox.hxx"

// forward ---------------------------------------------------------------

#if HAVE_FEATURE_JAVA
struct JavaInfo;
#else
typedef void* JavaInfo;
#endif

class   SvxJavaParameterDlg;
class   SvxJavaClassPathDlg;
class   SvxJavaListBox;

// class SvxJavaOptionsPage ----------------------------------------------

class SvxJavaOptionsPage : public SfxTabPage
{
private:
    VclPtr<CheckBox>                   m_pJavaEnableCB;
    VclPtr<VclContainer>               m_pJavaBox;
    VclPtr<SvxJavaListBox>             m_pJavaList;
    VclPtr<FixedText>                  m_pJavaPathText;
    VclPtr<PushButton>                 m_pAddBtn;
    VclPtr<PushButton>                 m_pParameterBtn;
    VclPtr<PushButton>                 m_pClassPathBtn;
    VclPtr<PushButton>                 m_pExpertConfigBtn;

    VclPtr<SvxJavaParameterDlg>        m_pParamDlg;
    VclPtr<SvxJavaClassPathDlg>        m_pPathDlg;

#if HAVE_FEATURE_JAVA
    std::vector<std::unique_ptr<JavaInfo>> m_parJavaInfo;
    std::vector<OUString>   m_parParameters;
    OUString                m_pClassPath;
#endif
    OUString                m_sInstallText;
    OUString                m_sAccessibilityText;
    OUString                m_sAddDialogText;
    Idle                    m_aResetIdle;

    VclPtr<CheckBox>               m_pExperimentalCB;
    VclPtr<CheckBox>               m_pMacroCB;

    std::vector<std::unique_ptr<JavaInfo>>
                            m_aAddedInfos;

    rtl::Reference< ::svt::DialogClosedListener >           xDialogListener;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    DECL_LINK(        EnableHdl_Impl, Button*, void);
    DECL_LINK(        CheckHdl_Impl, SvTreeListBox*, void );
    DECL_LINK(        SelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK(        AddHdl_Impl, Button*, void);
    DECL_LINK(        ParameterHdl_Impl, Button*, void);
    DECL_LINK(        ClassPathHdl_Impl, Button*, void);
    DECL_LINK(        ResetHdl_Impl, Timer *, void);

    DECL_LINK(        StartFolderPickerHdl, void *, void );
    DECL_LINK(        DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void );

    DECL_LINK(        ExpertConfigHdl_Impl, Button*, void);

    void                    ClearJavaInfo();
    void                    ClearJavaList();
    void                    LoadJREs();
    void                    AddJRE( JavaInfo const * _pInfo );
    void                    HandleCheckEntry( SvTreeListEntry* _pEntry );
    void                    AddFolder( const OUString& _rFolder );

public:
    SvxJavaOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxJavaOptionsPage() override;
    virtual void            dispose() override;

    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent, const SfxItemSet* rSet );

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            FillUserData() override;
};

// class SvxJavaParameterDlg ---------------------------------------------

class SvxJavaParameterDlg : public ModalDialog
{
private:
    VclPtr<Edit>                   m_pParameterEdit;
    VclPtr<PushButton>             m_pAssignBtn;

    VclPtr<ListBox>                m_pAssignedList;
    VclPtr<PushButton>             m_pRemoveBtn;

    VclPtr<PushButton>             m_pEditBtn;

    DECL_LINK(ModifyHdl_Impl, Edit&, void);
    DECL_LINK(AssignHdl_Impl, Button*, void);
    DECL_LINK(SelectHdl_Impl, ListBox&, void);
    DECL_LINK(DblClickHdl_Impl, ListBox&, void);
    DECL_LINK(RemoveHdl_Impl, Button*, void);

    DECL_LINK(EditHdl_Impl, Button*, void);

    void             EnableRemoveButton()
                                { m_pRemoveBtn->Enable(
                                    m_pAssignedList->GetSelectEntryPos()
                                    != LISTBOX_ENTRY_NOTFOUND ); }


    void             EnableEditButton()
                                { m_pEditBtn->Enable(
                                    m_pAssignedList->GetSelectEntryPos()
                                    != LISTBOX_ENTRY_NOTFOUND ); }

    void             DisableAssignButton()
                                { m_pAssignBtn->Disable(); }

    void             DisableRemoveButton()
                                { m_pRemoveBtn->Disable(); }

    void             DisableEditButton()
                                { m_pEditBtn->Disable(); }


public:
    explicit SvxJavaParameterDlg( vcl::Window* pParent );
    virtual ~SvxJavaParameterDlg() override;
    virtual void dispose() override;

    virtual short           Execute() override;

    std::vector< OUString > GetParameters() const;
    void SetParameters( std::vector< OUString > const & rParams );
    void DisableButtons();
    void EditParameter();
};

// class SvxJavaClassPathDlg ---------------------------------------------

class SvxJavaClassPathDlg : public ModalDialog
{
private:
    VclPtr<ListBox>                 m_pPathList;
    VclPtr<PushButton>              m_pAddArchiveBtn;
    VclPtr<PushButton>              m_pAddPathBtn;
    VclPtr<PushButton>              m_pRemoveBtn;

    OUString                m_sOldPath;

    DECL_LINK(AddArchiveHdl_Impl, Button*, void);
    DECL_LINK(AddPathHdl_Impl, Button*, void);
    DECL_LINK(RemoveHdl_Impl, Button*, void);
    DECL_LINK(SelectHdl_Impl, ListBox&, void);

    bool                    IsPathDuplicate( const OUString& _rPath );
    void             EnableRemoveButton()
                                { m_pRemoveBtn->Enable(
                                    m_pPathList->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND ); }


public:
    explicit SvxJavaClassPathDlg( vcl::Window* pParent );
    virtual ~SvxJavaClassPathDlg() override;
    virtual void            dispose() override;

    const OUString&  GetOldPath() const { return m_sOldPath; }
    void             SetFocus() { m_pPathList->GrabFocus(); }

    OUString                GetClassPath() const;
    void                    SetClassPath( const OUString& _rPath );
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTJAVA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
