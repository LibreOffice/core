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

#pragma once

#include <config_java.h>

#include <memory>
#include <vector>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld.hxx>
#include <sfx2/tabdlg.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <svtools/dialogclosedlistener.hxx>
#include <svtools/restartdialog.hxx>

// forward ---------------------------------------------------------------

#if HAVE_FEATURE_JAVA
struct JavaInfo;
#else
typedef void* JavaInfo;
#endif

class   SvxJavaParameterDlg;
class   SvxJavaClassPathDlg;
class   SvxJavaListBox;
class   OfaTreeOptionsDialog;

// class SvxJavaOptionsPage ----------------------------------------------

class SvxJavaOptionsPage : public SfxTabPage
{
private:
#if HAVE_FEATURE_JAVA
    std::vector<std::unique_ptr<JavaInfo>> m_parJavaInfo;
    std::vector<OUString>   m_parParameters;
    OUString                m_pClassPath;
#endif
    OUString                m_sInstallText;
    OUString                m_sAddDialogText;
    Idle                    m_aResetIdle;

    std::vector<std::unique_ptr<JavaInfo>> m_aAddedInfos;

    rtl::Reference< ::svt::DialogClosedListener >           xDialogListener;
    css::uno::Reference< css::ui::dialogs::XFolderPicker2 > xFolderPicker;

    std::unique_ptr<weld::CheckButton> m_xJavaEnableCB;
    std::unique_ptr<weld::TreeView> m_xJavaList;
    std::unique_ptr<weld::Label> m_xJavaPathText;
    std::unique_ptr<weld::Button> m_xAddBtn;
    std::unique_ptr<weld::Button> m_xParameterBtn;
    std::unique_ptr<weld::Button> m_xClassPathBtn;
    std::unique_ptr<weld::Button> m_xExpertConfigBtn;

    std::unique_ptr<SvxJavaParameterDlg> m_xParamDlg;
    std::unique_ptr<SvxJavaClassPathDlg> m_xPathDlg;

    std::unique_ptr<weld::CheckButton> m_xExperimentalCB;
    std::unique_ptr<weld::CheckButton> m_xMacroCB;

    std::unique_ptr<weld::Label> m_xAddDialogText;

    std::unique_ptr<weld::Widget> m_xJavaFrame;

    DECL_LINK(EnableHdl_Impl, weld::Button&, void);
    typedef std::pair<int, int> row_col;
    DECL_LINK(CheckHdl_Impl, const row_col&, void);
    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);
    DECL_LINK(AddHdl_Impl, weld::Button&, void);
    DECL_LINK(ParameterHdl_Impl, weld::Button&, void);
    DECL_LINK(ClassPathHdl_Impl, weld::Button&, void);
    DECL_LINK(ResetHdl_Impl, Timer *, void);

    DECL_LINK(StartFolderPickerHdl, void *, void);
    DECL_LINK(DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, void);

    DECL_LINK(ExpertConfigHdl_Impl, weld::Button&, void);

    void                    ClearJavaInfo();
    void                    LoadJREs();
    void                    AddJRE( JavaInfo const * _pInfo );
    void                    HandleCheckEntry(int nCheckedRow);
    void                    AddFolder( const OUString& _rFolder );
    void                    RequestRestart( svtools::RestartReason eReason );

public:
    SvxJavaOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SvxJavaOptionsPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet );

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual void            FillUserData() override;
};

// class SvxJavaParameterDlg ---------------------------------------------

class SvxJavaParameterDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xParameterEdit;
    std::unique_ptr<weld::Button> m_xAssignBtn;
    std::unique_ptr<weld::TreeView> m_xAssignedList;
    std::unique_ptr<weld::Button> m_xRemoveBtn;
    std::unique_ptr<weld::Button> m_xEditBtn;

    DECL_LINK(ModifyHdl_Impl, weld::Entry&, void);
    DECL_LINK(AssignHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);
    DECL_LINK(DblClickHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(RemoveHdl_Impl, weld::Button&, void);
    DECL_LINK(EditHdl_Impl, weld::Button&, void);

    void EnableRemoveButton()
    {
        m_xRemoveBtn->set_sensitive(m_xAssignedList->get_selected_index() != -1);
    }

    void EnableEditButton()
    {
        m_xEditBtn->set_sensitive(m_xAssignedList->get_selected_index() != -1);
    }

    void DisableAssignButton()
    {
        m_xAssignBtn->set_sensitive(false);
    }

    void DisableRemoveButton()
    {
        m_xRemoveBtn->set_sensitive(false);
    }

    void DisableEditButton()
    {
        m_xEditBtn->set_sensitive(false);
    }

public:
    explicit SvxJavaParameterDlg(weld::Window* pParent);
    virtual ~SvxJavaParameterDlg() override;

    virtual short run() override;

    std::vector< OUString > GetParameters() const;
    void SetParameters( std::vector< OUString > const & rParams );
    void DisableButtons();
    void EditParameter();
};

// class SvxJavaClassPathDlg ---------------------------------------------

class SvxJavaClassPathDlg : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::TreeView> m_xPathList;
    std::unique_ptr<weld::Button> m_xAddArchiveBtn;
    std::unique_ptr<weld::Button> m_xAddPathBtn;
    std::unique_ptr<weld::Button> m_xRemoveBtn;

    OUString                m_sOldPath;

    DECL_LINK(AddArchiveHdl_Impl, weld::Button&, void);
    DECL_LINK(AddPathHdl_Impl, weld::Button&, void);
    DECL_LINK(RemoveHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);

    bool IsPathDuplicate(const OUString& _rPath);
    void EnableRemoveButton()
    {
        m_xRemoveBtn->set_sensitive(m_xPathList->get_selected_index() != -1);
    }

public:
    explicit SvxJavaClassPathDlg(weld::Window* pParent);
    virtual ~SvxJavaClassPathDlg() override;

    const OUString&  GetOldPath() const { return m_sOldPath; }
    void             SetFocus() { m_xPathList->grab_focus(); }

    OUString                GetClassPath() const;
    void                    SetClassPath( const OUString& _rPath );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
