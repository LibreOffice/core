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
typedef struct _JavaInfo JavaInfo;
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
    JavaInfo**              m_parJavaInfo;
    rtl_uString**           m_parParameters;
    rtl_uString*            m_pClassPath;
    sal_Int32               m_nInfoSize;
    sal_Int32               m_nParamSize;
#endif
    OUString                m_sInstallText;
    OUString                m_sAccessibilityText;
    OUString                m_sAddDialogText;
    Idle                    m_aResetIdle;

    VclPtr<CheckBox>               m_pExperimentalCB;
    VclPtr<CheckBox>               m_pMacroCB;

    ::std::vector< JavaInfo* >
                            m_aAddedInfos;

    css::uno::Reference< ::svt::DialogClosedListener > xDialogListener;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    DECL_LINK_TYPED(        EnableHdl_Impl, Button*, void);
    DECL_LINK_TYPED(        CheckHdl_Impl, SvTreeListBox*, void );
    DECL_LINK_TYPED(        SelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK_TYPED(        AddHdl_Impl, Button*, void);
    DECL_LINK_TYPED(        ParameterHdl_Impl, Button*, void);
    DECL_LINK_TYPED(        ClassPathHdl_Impl, Button*, void);
    DECL_LINK_TYPED(        ResetHdl_Impl, Idle *, void);

    DECL_LINK_TYPED(        StartFolderPickerHdl, void *, void );
    DECL_LINK_TYPED(        DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void );

    DECL_LINK_TYPED(        ExpertConfigHdl_Impl, Button*, void);

    void                    ClearJavaInfo();
    void                    ClearJavaList();
    void                    LoadJREs();
    void                    AddJRE( JavaInfo* _pInfo );
    void                    HandleCheckEntry( SvTreeListEntry* _pEntry );
    void                    AddFolder( const OUString& _rFolder );

public:
    SvxJavaOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxJavaOptionsPage();
    virtual void            dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>      Create( vcl::Window* pParent, const SfxItemSet* rSet );

    virtual bool            FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void            Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void            FillUserData() SAL_OVERRIDE;
};

// class SvxJavaParameterDlg ---------------------------------------------

class SvxJavaParameterDlg : public ModalDialog
{
private:
    VclPtr<Edit>                   m_pParameterEdit;
    VclPtr<PushButton>             m_pAssignBtn;

    VclPtr<ListBox>                m_pAssignedList;
    VclPtr<PushButton>             m_pRemoveBtn;

    DECL_LINK(ModifyHdl_Impl, void *);
    DECL_LINK_TYPED(AssignHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SelectHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(DblClickHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(RemoveHdl_Impl, Button*, void);

    inline void             EnableRemoveButton()
                                { m_pRemoveBtn->Enable(
                                    m_pAssignedList->GetSelectEntryPos()
                                    != LISTBOX_ENTRY_NOTFOUND ); }


public:
    explicit SvxJavaParameterDlg( vcl::Window* pParent );
    virtual ~SvxJavaParameterDlg();
    virtual void dispose() SAL_OVERRIDE;

    virtual short           Execute() SAL_OVERRIDE;

    css::uno::Sequence< OUString > GetParameters() const;
    void SetParameters( css::uno::Sequence< OUString >& rParams );
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

    DECL_LINK_TYPED(AddArchiveHdl_Impl, Button*, void);
    DECL_LINK_TYPED(AddPathHdl_Impl, Button*, void);
    DECL_LINK_TYPED(RemoveHdl_Impl, Button*, void);
    DECL_LINK_TYPED(SelectHdl_Impl, ListBox&, void);

    bool                    IsPathDuplicate( const OUString& _rPath );
    inline void             EnableRemoveButton()
                                { m_pRemoveBtn->Enable(
                                    m_pPathList->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND ); }

public:
    explicit SvxJavaClassPathDlg( vcl::Window* pParent );
    virtual ~SvxJavaClassPathDlg();
    virtual void            dispose() SAL_OVERRIDE;

    inline const OUString&  GetOldPath() const { return m_sOldPath; }
    inline void             SetFocus() { m_pPathList->GrabFocus(); }

    OUString                GetClassPath() const;
    void                    SetClassPath( const OUString& _rPath );
};

#endif // INCLUDED_CUI_SOURCE_OPTIONS_OPTJAVA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
