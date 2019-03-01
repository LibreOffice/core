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
#include <svtools/miscopt.hxx>

#include <strings.hrc>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <tools/urlobj.hxx>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/restartdialog.hxx>
#include <vcl/treelistentry.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/inputdlg.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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

class SvxJavaListBox : public svx::SvxRadioButtonListBox
{
private:
    const OUString m_sAccessibilityText;
public:
    SvxJavaListBox(SvSimpleTableContainer& rParent, const OUString &rAccessibilityText)
        : SvxRadioButtonListBox(rParent, 0)
        , m_sAccessibilityText(rAccessibilityText)
    {
    }
    void setColSizes()
    {
        HeaderBar &rBar = GetTheHeaderBar();
        if (rBar.GetItemCount() < 4)
            return;
        long nCheckWidth = std::max(GetControlColumnWidth() + 12,
            rBar.LogicToPixel(Size(15, 0), MapMode(MapUnit::MapAppFont)).Width());
        long nVersionWidth = 12 +
            std::max(rBar.GetTextWidth(rBar.GetItemText(3)),
            GetTextWidth("0.0.0_00-icedtea"));
        long nFeatureWidth = 12 +
            std::max(rBar.GetTextWidth(rBar.GetItemText(4)),
            GetTextWidth(m_sAccessibilityText));
        long nVendorWidth =
            std::max(GetSizePixel().Width() - (nCheckWidth + nVersionWidth + nFeatureWidth),
            6 + std::max(rBar.GetTextWidth(rBar.GetItemText(2)),
            GetTextWidth("Sun Microsystems Inc.")));
        long aStaticTabs[]= { 0, 0, 0, 0 };
        aStaticTabs[1] = nCheckWidth;
        aStaticTabs[2] = aStaticTabs[1] + nVendorWidth;
        aStaticTabs[3] = aStaticTabs[2] + nVersionWidth;
        SvSimpleTable::SetTabs(SAL_N_ELEMENTS(aStaticTabs), aStaticTabs, MapUnit::MapPixel);
    }
    virtual void Resize() override
    {
        svx::SvxRadioButtonListBox::Resize();
        setColSizes();
    }
};

// class SvxJavaOptionsPage ----------------------------------------------

SvxJavaOptionsPage::SvxJavaOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptAdvancedPage", "cui/ui/optadvancedpage.ui", &rSet)
    , m_aResetIdle("cui options SvxJavaOptionsPage Reset")
    , xDialogListener(new ::svt::DialogClosedListener())
{
    get(m_pJavaEnableCB, "javaenabled");
    get(m_pJavaBox, "javabox");
    get(m_pJavaPathText, "javapath");
    m_sInstallText = m_pJavaPathText->GetText();
    get(m_pAddBtn, "add");
    get(m_pParameterBtn, "parameters");
    get(m_pClassPathBtn, "classpath");
    get(m_pExperimentalCB, "experimental");
    get(m_pMacroCB, "macrorecording");
    get(m_pExpertConfigBtn, "expertconfig");
    m_pParentDlg.reset( dynamic_cast<OfaTreeOptionsDialog*>(getNonLayoutParent(pParent)) );
    m_sAccessibilityText = get<FixedText>("a11y")->GetText();
    m_sAddDialogText = get<FixedText>("selectruntime")->GetText();

    SvSimpleTableContainer *pJavaListContainer = get<SvSimpleTableContainer>("javas");
    Size aControlSize(177, 60);
    aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    pJavaListContainer->set_width_request(aControlSize.Width());
    pJavaListContainer->set_height_request(aControlSize.Height());
    m_pJavaList = VclPtr<SvxJavaListBox>::Create(*pJavaListContainer, m_sAccessibilityText);

    long const aStaticTabs[]= { 0, 0, 0, 0 };
    m_pJavaList->SvSimpleTable::SetTabs( SAL_N_ELEMENTS(aStaticTabs), aStaticTabs );

    OUString sHeader ( "\t" + get<FixedText>("vendor")->GetText() +
        "\t" + get<FixedText>("version")->GetText() +
        "\t" + get<FixedText>("features")->GetText() +
        "\t" );
    m_pJavaList->InsertHeaderEntry(sHeader, HEADERBAR_APPEND, HeaderBarItemBits::LEFT);
    m_pJavaList->setColSizes();

    m_pJavaEnableCB->SetClickHdl( LINK( this, SvxJavaOptionsPage, EnableHdl_Impl ) );
    m_pJavaList->SetCheckButtonHdl( LINK( this, SvxJavaOptionsPage, CheckHdl_Impl ) );
    m_pJavaList->SetSelectHdl( LINK( this, SvxJavaOptionsPage, SelectHdl_Impl ) );
    m_pAddBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, AddHdl_Impl ) );
    m_pParameterBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, ParameterHdl_Impl ) );
    m_pClassPathBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, ClassPathHdl_Impl ) );
    m_aResetIdle.SetInvokeHandler( LINK( this, SvxJavaOptionsPage, ResetHdl_Impl ) );

    m_pExpertConfigBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, ExpertConfigHdl_Impl) );
    if (!officecfg::Office::Common::Security::EnableExpertConfiguration::get())
        m_pExpertConfigBtn->Disable();

    if (officecfg::Office::Common::Misc::MacroRecorderMode::isReadOnly())
        m_pMacroCB->Disable();

    if (officecfg::Office::Common::Misc::ExperimentalMode::isReadOnly())
        m_pExperimentalCB->Disable();

    xDialogListener->SetDialogClosedLink( LINK( this, SvxJavaOptionsPage, DialogClosedHdl ) );

    EnableHdl_Impl(m_pJavaEnableCB);
#if HAVE_FEATURE_JAVA
    jfw_lock();
#else
    get<vcl::Window>("javaframe")->Disable();
#endif
}


SvxJavaOptionsPage::~SvxJavaOptionsPage()
{
    disposeOnce();
}

void SvxJavaOptionsPage::dispose()
{
    m_pJavaList.disposeAndClear();
    m_xParamDlg.reset();
    m_xPathDlg.reset();
    ClearJavaInfo();
#if HAVE_FEATURE_JAVA
    m_aAddedInfos.clear();

    jfw_unlock();
#endif
    m_pParentDlg.clear();
    m_pJavaEnableCB.clear();
    m_pJavaBox.clear();
    m_pJavaPathText.clear();
    m_pAddBtn.clear();
    m_pParameterBtn.clear();
    m_pClassPathBtn.clear();
    m_pExpertConfigBtn.clear();
    m_pExperimentalCB.clear();
    m_pMacroCB.clear();
    SfxTabPage::dispose();
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, EnableHdl_Impl, Button*, void)
{
    bool bEnable = m_pJavaEnableCB->IsChecked();
    m_pJavaBox->Enable(bEnable);
    bEnable ? m_pJavaList->EnableTable() : m_pJavaList->DisableTable();
}


IMPL_LINK( SvxJavaOptionsPage, CheckHdl_Impl, SvTreeListBox*, pList, void )
{
    SvTreeListEntry* pEntry = pList ? m_pJavaList->GetEntry( m_pJavaList->GetCurMousePoint() )
                                : m_pJavaList->FirstSelected();
    if ( pEntry )
        m_pJavaList->HandleEntryChecked( pEntry );
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, SelectHdl_Impl, SvTreeListBox*, void)
{
    // set installation directory info
    SvTreeListEntry* pEntry = m_pJavaList->FirstSelected();
    DBG_ASSERT( pEntry, "SvxJavaOptionsPage::SelectHdl_Impl(): no entry" );
    OUString* pLocation = static_cast< OUString* >( pEntry->GetUserData() );
    DBG_ASSERT( pLocation, "invalid location string" );
    OUString sInfo = m_sInstallText;
    // tdf#80646 insert LTR mark after label
    sInfo += OUStringLiteral1(0x200E);
    if ( pLocation )
        sInfo += *pLocation;
    m_pJavaPathText->SetText(sInfo);
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, AddHdl_Impl, Button*, void)
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
    catch (const Exception& e)
    {
        SAL_WARN( "cui.options", "SvxJavaOptionsPage::AddHdl_Impl(): " << e);
    }
}


IMPL_LINK_NOARG(SvxJavaOptionsPage, ParameterHdl_Impl, Button*, void)
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


IMPL_LINK_NOARG(SvxJavaOptionsPage, ClassPathHdl_Impl, Button*, void)
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
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "cui.options", "SvxJavaOptionsPage::StartFolderPickerHdl(): caught exception " << exceptionToString(ex) );
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


IMPL_LINK_NOARG( SvxJavaOptionsPage, ExpertConfigHdl_Impl, Button*, void )
{
    ScopedVclPtrInstance< CuiAboutConfigTabPage > pExpertConfigDlg(GetTabDialog());
    pExpertConfigDlg->Reset();//initialize and reset function

    if( RET_OK == pExpertConfigDlg->Execute() )
    {
        pExpertConfigDlg->FillItemSet();//save changes if there are any
    }

    pExpertConfigDlg.disposeAndClear();
}


void SvxJavaOptionsPage::ClearJavaInfo()
{
#if HAVE_FEATURE_JAVA
    m_parJavaInfo.clear();
#else
    (void) this;
#endif
}


void SvxJavaOptionsPage::ClearJavaList()
{
    SvTreeListEntry* pEntry = m_pJavaList->First();
    while ( pEntry )
    {
        OUString* pLocation = static_cast< OUString* >( pEntry->GetUserData() );
        delete pLocation;
        pEntry = m_pJavaList->Next( pEntry );
    }
    m_pJavaList->Clear();
}


void SvxJavaOptionsPage::LoadJREs()
{
#if HAVE_FEATURE_JAVA
    WaitObject aWaitObj(m_pJavaList);
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
    if ( JFW_E_NONE == eErr && pSelectedJava )
    {
        sal_Int32 i = 0;
        for (auto const & pCmpInfo: m_parJavaInfo)
        {
            if ( jfw_areEqualJavaInfo( pCmpInfo.get(), pSelectedJava.get() ) )
            {
                SvTreeListEntry* pEntry = m_pJavaList->GetEntry(i);
                if ( pEntry )
                    m_pJavaList->HandleEntryChecked( pEntry );
                break;
            }
            ++i;
        }
    }
#else
    (void) this;
#endif
}


void SvxJavaOptionsPage::AddJRE( JavaInfo const * _pInfo )
{
#if HAVE_FEATURE_JAVA
    OUString sEntry = "\t" + _pInfo->sVendor + "\t" + _pInfo->sVersion + "\t";
    if ( ( _pInfo->nFeatures & JFW_FEATURE_ACCESSBRIDGE ) == JFW_FEATURE_ACCESSBRIDGE )
        sEntry += m_sAccessibilityText;
    SvTreeListEntry* pEntry = m_pJavaList->InsertEntry(sEntry);
    INetURLObject aLocObj( _pInfo->sLocation );
    OUString* pLocation = new OUString( aLocObj.getFSysPath( FSysStyle::Detect ) );
    pEntry->SetUserData( pLocation );
#else
    (void) this;
    (void)_pInfo;
#endif
}


void SvxJavaOptionsPage::HandleCheckEntry( SvTreeListEntry* _pEntry )
{
    m_pJavaList->Select( _pEntry );
    SvButtonState eState = m_pJavaList->GetCheckButtonState( _pEntry );

    if ( SvButtonState::Checked == eState )
    {
        // we have radio button behavior -> so uncheck the other entries
        SvTreeListEntry* pEntry = m_pJavaList->First();
        while ( pEntry )
        {
            if ( pEntry != _pEntry )
                m_pJavaList->SetCheckButtonState( pEntry, SvButtonState::Unchecked );
            pEntry = m_pJavaList->Next( pEntry );
        }
    }
    else
        m_pJavaList->SetCheckButtonState( _pEntry, SvButtonState::Checked );
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
            nPos = m_pJavaList->GetEntryCount() - 1;
        }

        SvTreeListEntry* pEntry = m_pJavaList->GetEntry( nPos );
        m_pJavaList->Select( pEntry );
        m_pJavaList->SetCheckButtonState( pEntry, SvButtonState::Checked );
        HandleCheckEntry( pEntry );
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

void SvxJavaOptionsPage::RequestRestart( svtools::RestartReason eReason )
{
    if ( m_pParentDlg )
        m_pParentDlg->SetNeedsRestart( eReason );
}


VclPtr<SfxTabPage> SvxJavaOptionsPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxJavaOptionsPage>::Create( pParent.pParent, *rAttrSet );
}


bool SvxJavaOptionsPage::FillItemSet( SfxItemSet* /*rCoreSet*/ )
{
    bool bModified = false;

    if ( m_pExperimentalCB->IsValueChangedFromSaved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetExperimentalMode( m_pExperimentalCB->IsChecked() );
        bModified = true;
        RequestRestart( svtools::RESTART_REASON_EXP_FEATURES );
    }

    if ( m_pMacroCB->IsValueChangedFromSaved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetMacroRecorderMode( m_pMacroCB->IsChecked() );
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

    sal_uLong nCount = m_pJavaList->GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        if ( m_pJavaList->GetCheckButtonState( m_pJavaList->GetEntry(i) ) == SvButtonState::Checked )
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
    if ( bEnabled != m_pJavaEnableCB->IsChecked() )
    {
        eErr = jfw_setEnabled( m_pJavaEnableCB->IsChecked() );
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
    ClearJavaList();

    SvtMiscOptions aMiscOpt;

#if HAVE_FEATURE_JAVA
    bool bEnabled = false;
    javaFrameworkError eErr = jfw_getEnabled( &bEnabled );
    if ( eErr != JFW_E_NONE )
        bEnabled = false;
    m_pJavaEnableCB->Check( bEnabled );
    EnableHdl_Impl(m_pJavaEnableCB);
#else
    m_pJavaEnableCB->Check( false );
    m_pJavaEnableCB->Disable();
#endif

    m_pExperimentalCB->Check( aMiscOpt.IsExperimentalMode() );
    m_pExperimentalCB->SaveValue();
    m_pMacroCB->Check( aMiscOpt.IsMacroRecorderMode() );
    m_pMacroCB->SaveValue();

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
    if (!sParam.isEmpty())
    {
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

IMPL_LINK_NOARG(SvxJavaParameterDlg, DblClickHdl_Impl, weld::TreeView&, void)
{
    EditParameter();
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

    if (nPos != -1)
    {
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
        INetURLObject aObj(m_xPathList->get_selected_text(), FSysStyle::Detect);
        sFolder = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    else
         sFolder = SvtPathOptions().GetWorkPath();
    aDlg.SetDisplayDirectory( sFolder );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString sURL = aDlg.GetPath();
        INetURLObject aURL( sURL );
        OUString sFile = aURL.getFSysPath( FSysStyle::Detect );
        if ( !IsPathDuplicate( sURL ) )
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
    EnableRemoveButton();
}

IMPL_LINK_NOARG(SvxJavaClassPathDlg, AddPathHdl_Impl, weld::Button&, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 > xFolderPicker = FolderPicker::create(xContext);

    OUString sOldFolder;
    if (m_xPathList->count_selected_rows() > 0)
    {
        INetURLObject aObj(m_xPathList->get_selected_text(), FSysStyle::Detect);
        sOldFolder = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    else
        sOldFolder = SvtPathOptions().GetWorkPath();
    xFolderPicker->setDisplayDirectory( sOldFolder );
    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        OUString sFolderURL( xFolderPicker->getDirectory() );
        INetURLObject aURL( sFolderURL );
        OUString sNewFolder = aURL.getFSysPath( FSysStyle::Detect );
        if ( !IsPathDuplicate( sFolderURL ) )
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
    INetURLObject aFileObj( _rPath );
    int nCount = m_xPathList->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        INetURLObject aOtherObj(m_xPathList->get_text(i), FSysStyle::Detect);
        if ( aOtherObj == aFileObj )
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
            INetURLObject aURL( sToken, FSysStyle::Detect );
            OUString sPath = aURL.getFSysPath( FSysStyle::Detect );
            m_xPathList->append("", sPath, SvFileInformationManager::GetImageId(aURL));
        }
        while (nIdx>=0);
        // select first entry
        m_xPathList->select(0);
    }
    SelectHdl_Impl(*m_xPathList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
