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

#include <sal/config.h>
#include <sal/log.hxx>

#include <memory>
#include <vector>

#include <config_java.h>

#include "optaboutconfig.hxx"
#include "optjava.hxx"
#include <treeopt.hxx>
#include <dialmgr.hxx>

#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
#include <svtools/miscopt.hxx>

#include <strings.hrc>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/weld.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/restartdialog.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/inputdlg.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#if HAVE_FEATURE_JAVA
#include <jvmfwk/framework.hxx>
#endif

// define ----------------------------------------------------------------

#define CLASSPATH_DELIMITER SAL_PATHSEPARATOR

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

// class SvxJavaOptionsPage ----------------------------------------------
SvxJavaOptionsPage::SvxJavaOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optadvancedpage.ui", "OptAdvancedPage", &rSet)
    , m_aResetIdle("cui options SvxJavaOptionsPage Reset")
    , xDialogListener(new ::svt::DialogClosedListener())
    , m_xJavaEnableCB(m_xBuilder->weld_check_button("javaenabled"))
    , m_xJavaList(m_xBuilder->weld_tree_view("javas"))
    , m_xJavaPathText(m_xBuilder->weld_label("javapath"))
    , m_xAddBtn(m_xBuilder->weld_button("add"))
    , m_xParameterBtn(m_xBuilder->weld_button("parameters"))
    , m_xClassPathBtn(m_xBuilder->weld_button("classpath"))
    , m_xExpertConfigBtn(m_xBuilder->weld_button("expertconfig"))
    , m_xExperimentalCB(m_xBuilder->weld_check_button("experimental"))
    , m_xMacroCB(m_xBuilder->weld_check_button("macrorecording"))
    , m_xAccessibilityText(m_xBuilder->weld_label("a11y"))
    , m_xAddDialogText(m_xBuilder->weld_label("selectruntime"))
    , m_xJavaFrame(m_xBuilder->weld_widget("javaframe"))
{
    m_sInstallText = m_xJavaPathText->get_label();
    m_sAccessibilityText = m_xAccessibilityText->get_label();
    m_sAddDialogText = m_xAddDialogText->get_label();

    m_xJavaList->set_size_request(m_xJavaList->get_approximate_digit_width() * 30,
                                  m_xJavaList->get_height_rows(8));

    std::vector<int> aWidths;
    aWidths.push_back(m_xJavaList->get_checkbox_column_width());
    aWidths.push_back(m_xJavaList->get_pixel_size("Sun Microsystems Inc.").Width());
    m_xJavaList->set_column_fixed_widths(aWidths);

    std::vector<int> aRadioColumns;
    aRadioColumns.push_back(0);
    m_xJavaList->set_toggle_columns_as_radio(aRadioColumns);

    m_xJavaEnableCB->connect_clicked( LINK( this, SvxJavaOptionsPage, EnableHdl_Impl ) );
    m_xJavaList->connect_toggled( LINK( this, SvxJavaOptionsPage, CheckHdl_Impl ) );
    m_xJavaList->connect_changed( LINK( this, SvxJavaOptionsPage, SelectHdl_Impl ) );
    m_xAddBtn->connect_clicked( LINK( this, SvxJavaOptionsPage, AddHdl_Impl ) );
    m_xParameterBtn->connect_clicked( LINK( this, SvxJavaOptionsPage, ParameterHdl_Impl ) );
    m_xClassPathBtn->connect_clicked( LINK( this, SvxJavaOptionsPage, ClassPathHdl_Impl ) );
    m_aResetIdle.SetInvokeHandler( LINK( this, SvxJavaOptionsPage, ResetHdl_Impl ) );

    m_xExpertConfigBtn->connect_clicked( LINK( this, SvxJavaOptionsPage, ExpertConfigHdl_Impl) );
    if (!officecfg::Office::Common::Security::EnableExpertConfiguration::get())
        m_xExpertConfigBtn->set_sensitive(false);

    if (officecfg::Office::Common::Misc::MacroRecorderMode::isReadOnly())
        m_xMacroCB->set_sensitive(false);

    if (officecfg::Office::Common::Misc::ExperimentalMode::isReadOnly())
        m_xExperimentalCB->set_sensitive(false);

    xDialogListener->SetDialogClosedLink( LINK( this, SvxJavaOptionsPage, DialogClosedHdl ) );

    EnableHdl_Impl(*m_xJavaEnableCB);
#if HAVE_FEATURE_JAVA
    jfw_lock();
#else
    m_xJavaFrame->set_sensitive(false);
#endif
}

SvxJavaOptionsPage::~SvxJavaOptionsPage()
{
    ClearJavaInfo();
#if HAVE_FEATURE_JAVA
    m_aAddedInfos.clear();

    jfw_unlock();
#endif
}

IMPL_LINK_NOARG(SvxJavaOptionsPage, EnableHdl_Impl, weld::Button&, void)
{
    bool bEnable = m_xJavaEnableCB->get_active();
    m_xJavaList->set_sensitive(bEnable);
}

IMPL_LINK(SvxJavaOptionsPage, CheckHdl_Impl, const row_col&, rRowCol, void)
{
    HandleCheckEntry(rRowCol.first);
}

IMPL_LINK_NOARG(SvxJavaOptionsPage, SelectHdl_Impl, weld::TreeView&, void)
{
    // set installation directory info
    OUString sLocation = m_xJavaList->get_selected_id();
    // tdf#80646 insert LTR mark after label
    OUString sInfo = m_sInstallText + u"\u200E" + sLocation;
    m_xJavaPathText->set_label(sInfo);
}

IMPL_LINK_NOARG(SvxJavaOptionsPage, AddHdl_Impl, weld::Button&, void)
{
    try
    {
        Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        xFolderPicker = FolderPicker::create(xContext);

        OUString sWorkFolder = SvtPathOptions().GetWorkPath();
        xFolderPicker->setDisplayDirectory( sWorkFolder );
        xFolderPicker->setDescription( m_sAddDialogText );

        Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
        if ( xAsyncDlg.is() )
            xAsyncDlg->startExecuteModal( xDialogListener.get() );
        else if ( xFolderPicker.is() && xFolderPicker->execute() == ExecutableDialogResults::OK )
            AddFolder( xFolderPicker->getDirectory() );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION( "cui.options", "SvxJavaOptionsPage::AddHdl_Impl()");
    }
}

IMPL_LINK_NOARG(SvxJavaOptionsPage, ParameterHdl_Impl, weld::Button&, void)
{
#if HAVE_FEATURE_JAVA
    std::vector< OUString > aParameterList;
    if (!m_xParamDlg)
    {
        m_xParamDlg.reset(new SvxJavaParameterDlg(GetFrameWeld()));
        javaFrameworkError eErr = jfw_getVMParameters( &m_parParameters );
        if ( JFW_E_NONE == eErr && !m_parParameters.empty() )
        {
            aParameterList = m_parParameters;
            m_xParamDlg->SetParameters( aParameterList );
        }
    }
    else
    {
        aParameterList = m_xParamDlg->GetParameters();
        m_xParamDlg->DisableButtons();   //disable add, edit and remove button when dialog is reopened
    }

    if (m_xParamDlg->run() == RET_OK)
    {
        if ( aParameterList != m_xParamDlg->GetParameters() )
        {
            aParameterList = m_xParamDlg->GetParameters();
            if ( jfw_isVMRunning() )
            {
                RequestRestart( svtools::RESTART_REASON_ASSIGNING_JAVAPARAMETERS );
            }
        }
    }
    else
        m_xParamDlg->SetParameters( aParameterList );
#else
    (void) this;                // Silence loplugin:staticmethods
#endif
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, ClassPathHdl_Impl, weld::Button&, void)
{
#if HAVE_FEATURE_JAVA
    OUString sClassPath;

    if ( !m_xPathDlg )
    {
        m_xPathDlg.reset(new SvxJavaClassPathDlg(GetFrameWeld()));
        javaFrameworkError eErr = jfw_getUserClassPath( &m_pClassPath );
        if ( JFW_E_NONE == eErr )
        {
            sClassPath = m_pClassPath;
            m_xPathDlg->SetClassPath( sClassPath );
        }
    }
    else
        sClassPath = m_xPathDlg->GetClassPath();

    m_xPathDlg->SetFocus();
    if (m_xPathDlg->run() == RET_OK)
    {

        if (m_xPathDlg->GetClassPath() != sClassPath)
        {
            sClassPath = m_xPathDlg->GetClassPath();
            if ( jfw_isVMRunning() )
            {
                RequestRestart( svtools::RESTART_REASON_ASSIGNING_FOLDERS );
            }
        }
    }
    else
        m_xPathDlg->SetClassPath( sClassPath );
#else
    (void) this;
#endif
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, ResetHdl_Impl, Timer *, void)
{
    LoadJREs();
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, StartFolderPickerHdl, void*, void)
{
    try
    {
        Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
        if ( xAsyncDlg.is() )
            xAsyncDlg->startExecuteModal( xDialogListener.get() );
        else if ( xFolderPicker.is() && xFolderPicker->execute() == ExecutableDialogResults::OK )
            AddFolder( xFolderPicker->getDirectory() );
    }
    catch ( Exception const & )
    {
        TOOLS_WARN_EXCEPTION( "cui.options", "SvxJavaOptionsPage::StartFolderPickerHdl()" );
    }
}

IMPL_LINK( SvxJavaOptionsPage, DialogClosedHdl, DialogClosedEvent*, pEvt, void )
{
    if ( RET_OK == pEvt->DialogResult )
    {
        DBG_ASSERT( xFolderPicker.is(), "SvxJavaOptionsPage::DialogClosedHdl(): no folder picker" );

        AddFolder( xFolderPicker->getDirectory() );
    }
}

IMPL_LINK_NOARG(SvxJavaOptionsPage, ExpertConfigHdl_Impl, weld::Button&, void)
{
    CuiAboutConfigTabPage aExpertConfigDlg(GetFrameWeld());
    {
        weld::WaitObject aWait(GetFrameWeld());
        aExpertConfigDlg.Reset();//initialize and reset function
    }

    if (RET_OK == aExpertConfigDlg.run())
    {
        aExpertConfigDlg.FillItemSet();//save changes if there are any
    }
}

void SvxJavaOptionsPage::ClearJavaInfo()
{
#if HAVE_FEATURE_JAVA
    m_parJavaInfo.clear();
#else
    (void) this;
#endif
}

void SvxJavaOptionsPage::LoadJREs()
{
#if HAVE_FEATURE_JAVA
    weld::WaitObject aWaitObj(GetFrameWeld());
    javaFrameworkError eErr = jfw_findAllJREs( &m_parJavaInfo );
    if ( JFW_E_NONE == eErr )
    {
        for (auto const & pInfo: m_parJavaInfo)
        {
            AddJRE( pInfo.get() );
        }
    }

    for (auto const & pInfo: m_aAddedInfos)
    {
        AddJRE( pInfo.get() );
    }

    std::unique_ptr<JavaInfo> pSelectedJava;
    eErr = jfw_getSelectedJRE( &pSelectedJava );
    if ( !(JFW_E_NONE == eErr && pSelectedJava) )
        return;

    sal_Int32 i = 0;
    for (auto const & pCmpInfo: m_parJavaInfo)
    {
        if ( jfw_areEqualJavaInfo( pCmpInfo.get(), pSelectedJava.get() ) )
        {
            HandleCheckEntry(i);
            break;
        }
        ++i;
    }
#else
    (void) this;
#endif
}


void SvxJavaOptionsPage::AddJRE( JavaInfo const * _pInfo )
{
#if HAVE_FEATURE_JAVA
    int nPos = m_xJavaList->n_children();
    m_xJavaList->append();
    m_xJavaList->set_toggle(nPos, TRISTATE_FALSE, 0);
    m_xJavaList->set_text(nPos, _pInfo->sVendor, 1);
    m_xJavaList->set_text(nPos, _pInfo->sVersion, 2);

    INetURLObject aLocObj(_pInfo->sLocation);
    OUString sLocation = aLocObj.getFSysPath(FSysStyle::Detect);
    m_xJavaList->set_id(nPos, sLocation);
#else
    (void) this;
    (void)_pInfo;
#endif
}

void SvxJavaOptionsPage::HandleCheckEntry(int nCheckedRow)
{
    m_xJavaList->select(nCheckedRow);
    for (int i = 0, nCount = m_xJavaList->n_children(); i < nCount; ++i)
    {
        // we have radio button behavior -> so uncheck the other entries
        m_xJavaList->set_toggle(i, i == nCheckedRow ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
    }
}

void SvxJavaOptionsPage::AddFolder( const OUString& _rFolder )
{
#if HAVE_FEATURE_JAVA
    bool bStartAgain = true;
    std::unique_ptr<JavaInfo> pInfo;
    javaFrameworkError eErr = jfw_getJavaInfoByPath( _rFolder, &pInfo );
    if ( JFW_E_NONE == eErr && pInfo )
    {
        sal_Int32 nPos = 0;
        bool bFound = false;
        for (auto const & pCmpInfo: m_parJavaInfo)
        {
            if ( jfw_areEqualJavaInfo( pCmpInfo.get(), pInfo.get() ) )
            {
                bFound = true;
                break;
            }
            ++nPos;
        }

        if ( !bFound )
        {
            for (auto const & pCmpInfo: m_aAddedInfos)
            {
                if ( jfw_areEqualJavaInfo( pCmpInfo.get(), pInfo.get() ) )
                {
                    bFound = true;
                    break;
                }
                ++nPos;
            }
        }

        if ( !bFound )
        {
            jfw_addJRELocation( pInfo->sLocation );
            AddJRE( pInfo.get() );
            m_aAddedInfos.push_back( std::move(pInfo) );
            nPos = m_xJavaList->n_children() - 1;
        }

        HandleCheckEntry(nPos);
        bStartAgain = false;
    }
    else if ( JFW_E_NOT_RECOGNIZED == eErr )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  CuiResId(RID_SVXSTR_JRE_NOT_RECOGNIZED)));
        xBox->run();
    }
    else if ( JFW_E_FAILED_VERSION == eErr )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  CuiResId(RID_SVXSTR_JRE_FAILED_VERSION)));
        xBox->run();
    }

    if ( bStartAgain )
    {
        xFolderPicker->setDisplayDirectory( _rFolder );
        Application::PostUserEvent( LINK( this, SvxJavaOptionsPage, StartFolderPickerHdl ) );
    }
#else
    (void) this;
    (void)_rFolder;
#endif
}

void SvxJavaOptionsPage::RequestRestart(svtools::RestartReason eReason)
{
    OfaTreeOptionsDialog* pParentDlg(static_cast<OfaTreeOptionsDialog*>(GetDialogController()));
    if (pParentDlg)
        pParentDlg->SetNeedsRestart(eReason);
}

std::unique_ptr<SfxTabPage> SvxJavaOptionsPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet)
{
    return std::make_unique<SvxJavaOptionsPage>(pPage, pController, *rAttrSet);
}

bool SvxJavaOptionsPage::FillItemSet( SfxItemSet* /*rCoreSet*/ )
{
    bool bModified = false;

    if ( m_xExperimentalCB->get_state_changed_from_saved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetExperimentalMode( m_xExperimentalCB->get_active() );
        bModified = true;
        RequestRestart( svtools::RESTART_REASON_EXP_FEATURES );
    }

    if ( m_xMacroCB->get_state_changed_from_saved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetMacroRecorderMode(m_xMacroCB->get_active());
        bModified = true;
    }

#if HAVE_FEATURE_JAVA
    javaFrameworkError eErr = JFW_E_NONE;
    if (m_xParamDlg)
    {
        eErr = jfw_setVMParameters(m_xParamDlg->GetParameters());
        SAL_WARN_IF(JFW_E_NONE != eErr, "cui.options", "SvxJavaOptionsPage::FillItemSet(): error in jfw_setVMParameters");
        bModified = true;
    }

    if (m_xPathDlg)
    {
        OUString sPath(m_xPathDlg->GetClassPath());
        if (m_xPathDlg->GetOldPath() != sPath)
        {
            eErr = jfw_setUserClassPath( sPath );
            SAL_WARN_IF(JFW_E_NONE != eErr, "cui.options", "SvxJavaOptionsPage::FillItemSet(): error in jfw_setUserClassPath");
            bModified = true;
        }
    }

    sal_uInt32 nCount = m_xJavaList->n_children();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        if (m_xJavaList->get_toggle(i, 0) == TRISTATE_TRUE)
        {
            JavaInfo const * pInfo;
            if ( i < m_parJavaInfo.size() )
                pInfo = m_parJavaInfo[i].get();
            else
                pInfo = m_aAddedInfos[ i - m_parJavaInfo.size() ].get();

            std::unique_ptr<JavaInfo> pSelectedJava;
            eErr = jfw_getSelectedJRE( &pSelectedJava );
            if ( JFW_E_NONE == eErr || JFW_E_INVALID_SETTINGS == eErr )
            {
                if (!pSelectedJava || !jfw_areEqualJavaInfo( pInfo, pSelectedJava.get() ) )
                {
                    if ( jfw_isVMRunning() ||
                        ( ( pInfo->nRequirements & JFW_REQUIRE_NEEDRESTART ) == JFW_REQUIRE_NEEDRESTART ) )
                    {
                        RequestRestart( svtools::RESTART_REASON_JAVA );
                    }

                    eErr = jfw_setSelectedJRE( pInfo );
                    SAL_WARN_IF(JFW_E_NONE != eErr, "cui.options", "SvxJavaOptionsPage::FillItemSet(): error in jfw_setSelectedJRE");
                    bModified = true;
                }
            }
            break;
        }
    }

    bool bEnabled = false;
    eErr = jfw_getEnabled( &bEnabled );
    DBG_ASSERT( JFW_E_NONE == eErr,
                "SvxJavaOptionsPage::FillItemSet(): error in jfw_getEnabled" );
    if ( bEnabled != m_xJavaEnableCB->get_active() )
    {
        eErr = jfw_setEnabled( m_xJavaEnableCB->get_active() );
        DBG_ASSERT( JFW_E_NONE == eErr,
                    "SvxJavaOptionsPage::FillItemSet(): error in jfw_setEnabled" );
        bModified = true;
    }
#endif

    return bModified;
}


void SvxJavaOptionsPage::Reset( const SfxItemSet* /*rSet*/ )
{
    ClearJavaInfo();
    m_xJavaList->clear();

    SvtMiscOptions aMiscOpt;

#if HAVE_FEATURE_JAVA
    bool bEnabled = false;
    javaFrameworkError eErr = jfw_getEnabled( &bEnabled );
    if ( eErr != JFW_E_NONE )
        bEnabled = false;
    m_xJavaEnableCB->set_active(bEnabled);
    EnableHdl_Impl(*m_xJavaEnableCB);
#else
    m_xJavaEnableCB->set_active(false);
    m_xJavaEnableCB->set_sensitive(false);
#endif

    m_xExperimentalCB->set_active( aMiscOpt.IsExperimentalMode() );
    m_xExperimentalCB->save_state();
    m_xMacroCB->set_active(aMiscOpt.IsMacroRecorderMode());
    m_xMacroCB->save_state();

    m_aResetIdle.Start();
}


void SvxJavaOptionsPage::FillUserData()
{
    SetUserData( OUString() );
}

// class SvxJavaParameterDlg ---------------------------------------------

SvxJavaParameterDlg::SvxJavaParameterDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/javastartparametersdialog.ui",
        "JavaStartParameters")
    , m_xParameterEdit(m_xBuilder->weld_entry("parameterfield"))
    , m_xAssignBtn(m_xBuilder->weld_button("assignbtn"))
    , m_xAssignedList(m_xBuilder->weld_tree_view("assignlist"))
    , m_xRemoveBtn(m_xBuilder->weld_button("removebtn"))
    , m_xEditBtn(m_xBuilder->weld_button("editbtn"))
{
    m_xAssignedList->set_size_request(m_xAssignedList->get_approximate_digit_width() * 54,
                                      m_xAssignedList->get_height_rows(6));
    m_xParameterEdit->connect_changed( LINK( this, SvxJavaParameterDlg, ModifyHdl_Impl ) );
    m_xAssignBtn->connect_clicked( LINK( this, SvxJavaParameterDlg, AssignHdl_Impl ) );
    m_xRemoveBtn->connect_clicked( LINK( this, SvxJavaParameterDlg, RemoveHdl_Impl ) );
    m_xEditBtn->connect_clicked( LINK( this, SvxJavaParameterDlg, EditHdl_Impl ) );
    m_xAssignedList->connect_changed( LINK( this, SvxJavaParameterDlg, SelectHdl_Impl ) );
    m_xAssignedList->connect_row_activated( LINK( this, SvxJavaParameterDlg, DblClickHdl_Impl ) );

    ModifyHdl_Impl(*m_xParameterEdit);
    EnableEditButton();
    EnableRemoveButton();
}

SvxJavaParameterDlg::~SvxJavaParameterDlg()
{
}

IMPL_LINK_NOARG(SvxJavaParameterDlg, ModifyHdl_Impl, weld::Entry&, void)
{
    OUString sParam = comphelper::string::strip(m_xParameterEdit->get_text(), ' ');
    m_xAssignBtn->set_sensitive(!sParam.isEmpty());
}

IMPL_LINK_NOARG(SvxJavaParameterDlg, AssignHdl_Impl, weld::Button&, void)
{
    OUString sParam = comphelper::string::strip(m_xParameterEdit->get_text(), ' ');
    if (sParam.isEmpty())
        return;

    int nPos = m_xAssignedList->find_text(sParam);
    if (nPos == -1)
    {
        m_xAssignedList->append_text(sParam);
        m_xAssignedList->select(m_xAssignedList->n_children() - 1);
    }
    else
        m_xAssignedList->select(nPos);
    m_xParameterEdit->set_text(OUString());
    ModifyHdl_Impl(*m_xParameterEdit);
    EnableEditButton();
    EnableRemoveButton();
}

IMPL_LINK_NOARG(SvxJavaParameterDlg, EditHdl_Impl, weld::Button&, void)
{
    EditParameter();
}

IMPL_LINK_NOARG(SvxJavaParameterDlg, SelectHdl_Impl, weld::TreeView&, void)
{
    EnableEditButton();
    EnableRemoveButton();
}

IMPL_LINK_NOARG(SvxJavaParameterDlg, DblClickHdl_Impl, weld::TreeView&, bool)
{
    EditParameter();
    return true;
}

IMPL_LINK_NOARG(SvxJavaParameterDlg, RemoveHdl_Impl, weld::Button&, void)
{
    int nPos = m_xAssignedList->get_selected_index();
    if (nPos != -1)
    {
        m_xAssignedList->remove(nPos);
        int nCount = m_xAssignedList->n_children();
        if (nCount)
        {
            if (nPos >= nCount)
                nPos = nCount - 1;
            m_xAssignedList->select(nPos);
        }
        else
        {
            DisableEditButton();
        }
    }
    EnableRemoveButton();
}

void SvxJavaParameterDlg::EditParameter()
{
    int nPos = m_xAssignedList->get_selected_index();
    m_xParameterEdit->set_text(OUString());

    if (nPos == -1)
        return;

    InputDialog aParamEditDlg(m_xDialog.get(), CuiResId(RID_SVXSTR_JAVA_START_PARAM));
    OUString editableClassPath = m_xAssignedList->get_selected_text();
    aParamEditDlg.SetEntryText(editableClassPath);
    aParamEditDlg.HideHelpBtn();

    if (!aParamEditDlg.run())
        return;
    OUString editedClassPath = comphelper::string::strip(aParamEditDlg.GetEntryText(), ' ');

    if ( !editedClassPath.isEmpty() && editableClassPath != editedClassPath )
    {
        m_xAssignedList->remove(nPos);
        m_xAssignedList->insert_text(nPos, editedClassPath);
        m_xAssignedList->select(nPos);
    }
}

short SvxJavaParameterDlg::run()
{
    m_xParameterEdit->grab_focus();
    m_xAssignedList->select(-1);
    return GenericDialogController::run();
}

std::vector< OUString > SvxJavaParameterDlg::GetParameters() const
{
    int nCount = m_xAssignedList->n_children();
    std::vector< OUString > aParamList;
    aParamList.reserve(nCount);
    for (int i = 0; i < nCount; ++i)
        aParamList.push_back(m_xAssignedList->get_text(i));
    return aParamList;
}

void SvxJavaParameterDlg::DisableButtons()
{
    DisableAssignButton();
    DisableEditButton();
    DisableRemoveButton();
}

void SvxJavaParameterDlg::SetParameters( std::vector< OUString > const & rParams )
{
    m_xAssignedList->clear();
    for (auto const & sParam: rParams)
    {
        m_xAssignedList->append_text(sParam);
    }
    DisableEditButton();
    DisableRemoveButton();
}


// class SvxJavaClassPathDlg ---------------------------------------------

SvxJavaClassPathDlg::SvxJavaClassPathDlg(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/javaclasspathdialog.ui", "JavaClassPath")
    , m_xPathList(m_xBuilder->weld_tree_view("paths"))
    , m_xAddArchiveBtn(m_xBuilder->weld_button("archive"))
    , m_xAddPathBtn(m_xBuilder->weld_button("folder"))
    , m_xRemoveBtn(m_xBuilder->weld_button("remove"))
{
    m_xPathList->set_size_request(m_xPathList->get_approximate_digit_width() * 54,
                                  m_xPathList->get_height_rows(8));
    m_xAddArchiveBtn->connect_clicked( LINK( this, SvxJavaClassPathDlg, AddArchiveHdl_Impl ) );
    m_xAddPathBtn->connect_clicked( LINK( this, SvxJavaClassPathDlg, AddPathHdl_Impl ) );
    m_xRemoveBtn->connect_clicked( LINK( this, SvxJavaClassPathDlg, RemoveHdl_Impl ) );
    m_xPathList->connect_changed( LINK( this, SvxJavaClassPathDlg, SelectHdl_Impl ) );

    // set initial focus to path list
    m_xPathList->grab_focus();
}

SvxJavaClassPathDlg::~SvxJavaClassPathDlg()
{
}

IMPL_LINK_NOARG(SvxJavaClassPathDlg, AddArchiveHdl_Impl, weld::Button&, void)
{
    sfx2::FileDialogHelper aDlg(TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, m_xDialog.get());
    aDlg.SetTitle( CuiResId( RID_SVXSTR_ARCHIVE_TITLE ) );
    aDlg.AddFilter( CuiResId( RID_SVXSTR_ARCHIVE_HEADLINE ), "*.jar;*.zip" );
    OUString sFolder;
    if (m_xPathList->count_selected_rows() > 0)
    {
        osl::FileBase::getFileURLFromSystemPath(m_xPathList->get_selected_text(), sFolder);
            // best effort
    }
    if (sFolder.isEmpty())
         sFolder = SvtPathOptions().GetWorkPath();
    aDlg.SetDisplayDirectory( sFolder );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString sURL = aDlg.GetPath();
        OUString sFile;
        if (osl::FileBase::getSystemPathFromFileURL(sURL, sFile) == osl::FileBase::E_None)
        {
            INetURLObject aURL( sURL );
            if ( !IsPathDuplicate( sFile ) )
            {
                m_xPathList->append("", sFile, SvFileInformationManager::GetImageId(aURL));
                m_xPathList->select(m_xPathList->n_children() - 1);
            }
            else
            {
                OUString sMsg( CuiResId( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
                sMsg = sMsg.replaceFirst( "%1", sFile );
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok, sMsg));
                xBox->run();
            }
        }
        else
        {
            OUString sMsg( CuiResId( RID_SVXSTR_CANNOTCONVERTURL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sURL );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok, sMsg));
            xBox->run();
        }
    }
    EnableRemoveButton();
}

IMPL_LINK_NOARG(SvxJavaClassPathDlg, AddPathHdl_Impl, weld::Button&, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 > xFolderPicker = FolderPicker::create(xContext);

    OUString sOldFolder;
    if (m_xPathList->count_selected_rows() > 0)
    {
        osl::FileBase::getFileURLFromSystemPath(m_xPathList->get_selected_text(), sOldFolder);
            // best effort
    }
    if (sOldFolder.isEmpty())
        sOldFolder = SvtPathOptions().GetWorkPath();
    xFolderPicker->setDisplayDirectory( sOldFolder );
    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        OUString sFolderURL( xFolderPicker->getDirectory() );
        INetURLObject aURL( sFolderURL );
        OUString sNewFolder;
        if (osl::FileBase::getSystemPathFromFileURL(sFolderURL, sNewFolder)
            == osl::FileBase::E_None)
        {
            if ( !IsPathDuplicate( sNewFolder ) )
            {
                m_xPathList->append("", sNewFolder, SvFileInformationManager::GetImageId(aURL));
                m_xPathList->select(m_xPathList->n_children() - 1);
            }
            else
            {
                OUString sMsg( CuiResId( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
                sMsg = sMsg.replaceFirst( "%1", sNewFolder );
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok, sMsg));
                xBox->run();
            }
        }
        else
        {
            OUString sMsg( CuiResId( RID_SVXSTR_CANNOTCONVERTURL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sFolderURL );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok, sMsg));
            xBox->run();
        }
    }
    EnableRemoveButton();
}

IMPL_LINK_NOARG(SvxJavaClassPathDlg, RemoveHdl_Impl, weld::Button&, void)
{
    int nPos = m_xPathList->get_selected_index();
    if (nPos != -1)
    {
        m_xPathList->remove(nPos);
        int nCount = m_xPathList->n_children();
        if (nCount)
        {
            if (nPos >= nCount)
                nPos = nCount - 1;
            m_xPathList->select( nPos );
        }
    }

    EnableRemoveButton();
}

IMPL_LINK_NOARG(SvxJavaClassPathDlg, SelectHdl_Impl, weld::TreeView&, void)
{
    EnableRemoveButton();
}

bool SvxJavaClassPathDlg::IsPathDuplicate( const OUString& _rPath )
{
    bool bRet = false;
    int nCount = m_xPathList->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if ( m_xPathList->get_text(i) == _rPath )
        {
            bRet = true;
            break;
        }
    }

    return bRet;
}

OUString SvxJavaClassPathDlg::GetClassPath() const
{
    OUStringBuffer sPath;
    int nCount = m_xPathList->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if (!sPath.isEmpty())
            sPath.append(CLASSPATH_DELIMITER);
        sPath.append(m_xPathList->get_text(i));
    }
    return sPath.makeStringAndClear();
}

void SvxJavaClassPathDlg::SetClassPath( const OUString& _rPath )
{
    if ( m_sOldPath.isEmpty() )
        m_sOldPath = _rPath;
    m_xPathList->clear();
    if (!_rPath.isEmpty())
    {
        sal_Int32 nIdx = 0;
        do
        {
            OUString sToken = _rPath.getToken( 0, CLASSPATH_DELIMITER, nIdx );
            OUString sURL;
            osl::FileBase::getFileURLFromSystemPath(sToken, sURL); // best effort
            INetURLObject aURL( sURL );
            m_xPathList->append("", sToken, SvFileInformationManager::GetImageId(aURL));
        }
        while (nIdx>=0);
        // select first entry
        m_xPathList->select(0);
    }
    SelectHdl_Impl(*m_xPathList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
