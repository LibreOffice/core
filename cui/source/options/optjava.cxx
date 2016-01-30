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

#include <config_features.h>

#include "optaboutconfig.hxx"
#include "optjava.hxx"
#include <dialmgr.hxx>

#include <officecfg/Office/Common.hxx>
#include <svtools/miscopt.hxx>

#include <cuires.hrc>
#include "helpid.hrc"
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
#include <vcl/waitobj.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/imagemgr.hxx>
#include "svtools/restartdialog.hxx"
#include "svtools/treelistentry.hxx"
#include <sfx2/filedlghelper.hxx>
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

#if HAVE_FEATURE_JAVA

static bool areListsEqual( const Sequence< OUString >& rListA, const Sequence< OUString >& rListB )
{
    bool bRet = true;
    const sal_Int32 nLen = rListA.getLength();

    if (  rListB.getLength() != nLen )
        bRet = false;
    else
    {
        const OUString* pStringA = rListA.getConstArray();
        const OUString* pStringB = rListB.getConstArray();

        for ( sal_Int32 i = 0; i < nLen; ++i )
        {
            if ( *pStringA++ != *pStringB++ )
            {
                bRet = false;
                break;
            }
        }
    }

    return bRet;
}

#endif

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
            rBar.LogicToPixel(Size(15, 0), MAP_APPFONT).Width());
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
        long aStaticTabs[]= { 4, 0, 0, 0, 0, 0 };
        aStaticTabs[2] = nCheckWidth;
        aStaticTabs[3] = aStaticTabs[2] + nVendorWidth;
        aStaticTabs[4] = aStaticTabs[3] + nVersionWidth;
        SvSimpleTable::SetTabs(aStaticTabs, MAP_PIXEL);
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
    , m_pParamDlg(nullptr)
    , m_pPathDlg(nullptr)
#if HAVE_FEATURE_JAVA
    , m_parJavaInfo(nullptr)
    , m_parParameters(nullptr)
    , m_pClassPath(nullptr)
    , m_nInfoSize(0)
    , m_nParamSize(0)
#endif
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
    m_sAccessibilityText = get<FixedText>("a11y")->GetText();
    m_sAddDialogText = get<FixedText>("selectruntime")->GetText();

    SvSimpleTableContainer *pJavaListContainer = get<SvSimpleTableContainer>("javas");
    Size aControlSize(177, 60);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pJavaListContainer->set_width_request(aControlSize.Width());
    pJavaListContainer->set_height_request(aControlSize.Height());
    m_pJavaList = VclPtr<SvxJavaListBox>::Create(*pJavaListContainer, m_sAccessibilityText);

    long aStaticTabs[]= { 4, 0, 0, 0, 0 };

    m_pJavaList->SvSimpleTable::SetTabs( aStaticTabs );

    OUStringBuffer sHeader;
    sHeader.append("\t").append(get<FixedText>("vendor")->GetText())
        .append("\t").append(get<FixedText>("version")->GetText())
        .append("\t").append(get<FixedText>("features")->GetText())
        .append("\t");
    m_pJavaList->InsertHeaderEntry(sHeader.makeStringAndClear(), HEADERBAR_APPEND, HeaderBarItemBits::LEFT);
    m_pJavaList->setColSizes();

    m_pJavaEnableCB->SetClickHdl( LINK( this, SvxJavaOptionsPage, EnableHdl_Impl ) );
    m_pJavaList->SetCheckButtonHdl( LINK( this, SvxJavaOptionsPage, CheckHdl_Impl ) );
    m_pJavaList->SetSelectHdl( LINK( this, SvxJavaOptionsPage, SelectHdl_Impl ) );
    m_pAddBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, AddHdl_Impl ) );
    m_pParameterBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, ParameterHdl_Impl ) );
    m_pClassPathBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, ClassPathHdl_Impl ) );
    m_aResetIdle.SetIdleHdl( LINK( this, SvxJavaOptionsPage, ResetHdl_Impl ) );
    m_aResetIdle.SetPriority(SchedulerPriority::LOWER);

    m_pExpertConfigBtn->SetClickHdl( LINK( this, SvxJavaOptionsPage, ExpertConfigHdl_Impl) );
    if (!officecfg::Office::Common::Security::EnableExpertConfiguration::get())
        m_pExpertConfigBtn->Disable();

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
    m_pParamDlg.disposeAndClear();
    m_pPathDlg.disposeAndClear();
    ClearJavaInfo();
#if HAVE_FEATURE_JAVA
    std::vector< JavaInfo* >::iterator pIter;
    for ( pIter = m_aAddedInfos.begin(); pIter != m_aAddedInfos.end(); ++pIter )
    {
        JavaInfo* pInfo = *pIter;
        jfw_freeJavaInfo( pInfo );
    }
    m_aAddedInfos.clear();

    jfw_unlock();
#endif
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


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, EnableHdl_Impl, Button*, void)
{
    bool bEnable = m_pJavaEnableCB->IsChecked();
    m_pJavaBox->Enable(bEnable);
    bEnable ? m_pJavaList->EnableTable() : m_pJavaList->DisableTable();
}


IMPL_LINK_TYPED( SvxJavaOptionsPage, CheckHdl_Impl, SvTreeListBox*, pList, void )
{
    SvTreeListEntry* pEntry = pList ? m_pJavaList->GetEntry( m_pJavaList->GetCurMousePoint() )
                                : m_pJavaList->FirstSelected();
    if ( pEntry )
        m_pJavaList->HandleEntryChecked( pEntry );
}


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, SelectHdl_Impl, SvTreeListBox*, void)
{
    // set installation directory info
    SvTreeListEntry* pEntry = m_pJavaList->FirstSelected();
    DBG_ASSERT( pEntry, "SvxJavaOptionsPage::SelectHdl_Impl(): no entry" );
    OUString* pLocation = static_cast< OUString* >( pEntry->GetUserData() );
    DBG_ASSERT( pLocation, "invalid location string" );
    OUString sInfo = m_sInstallText;
    if ( pLocation )
        sInfo += *pLocation;
    m_pJavaPathText->SetText(sInfo);
}


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, AddHdl_Impl, Button*, void)
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
        SAL_WARN( "cui.options", "SvxJavaOptionsPage::AddHdl_Impl(): caught exception: " << e.Message);
    }
}


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, ParameterHdl_Impl, Button*, void)
{
#if HAVE_FEATURE_JAVA
    Sequence< OUString > aParameterList;
    if ( !m_pParamDlg )
    {
        m_pParamDlg = VclPtr<SvxJavaParameterDlg>::Create( this );
        javaFrameworkError eErr = jfw_getVMParameters( &m_parParameters, &m_nParamSize );
        if ( JFW_E_NONE == eErr && m_parParameters && m_nParamSize > 0 )
        {
            rtl_uString** pParamArr = m_parParameters;
            aParameterList.realloc( m_nParamSize );
            OUString* pParams = aParameterList.getArray();
            for ( sal_Int32 i = 0; i < m_nParamSize; ++i )
            {
                rtl_uString* pParam = *pParamArr++;
                pParams[i] = OUString( pParam );
            }
            m_pParamDlg->SetParameters( aParameterList );
        }
    }
    else
        aParameterList = m_pParamDlg->GetParameters();

    if ( m_pParamDlg->Execute() == RET_OK )
    {
        if ( !areListsEqual( aParameterList, m_pParamDlg->GetParameters() ) )
        {
            aParameterList = m_pParamDlg->GetParameters();
            sal_Bool bRunning = sal_False;
            javaFrameworkError eErr = jfw_isVMRunning( &bRunning );
            DBG_ASSERT( JFW_E_NONE == eErr,
                        "SvxJavaOptionsPage::ParameterHdl_Impl(): error in jfw_isVMRunning" );
            (void)eErr;
            if ( bRunning )
            {
                ScopedVclPtrInstance< MessageDialog > aWarnBox( this, CUI_RES( RID_SVXSTR_OPTIONS_RESTART ), VCL_MESSAGE_INFO );
                aWarnBox->Execute();
            }
        }
    }
    else
        m_pParamDlg->SetParameters( aParameterList );
#else
    (void) this;                // Silence loplugin:staticmethods
#endif
}


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, ClassPathHdl_Impl, Button*, void)
{
#if HAVE_FEATURE_JAVA
    OUString sClassPath;

    if ( !m_pPathDlg )
    {
          m_pPathDlg = VclPtr<SvxJavaClassPathDlg>::Create( this );
        javaFrameworkError eErr = jfw_getUserClassPath( &m_pClassPath );
        if ( JFW_E_NONE == eErr && m_pClassPath )
        {
            sClassPath = m_pClassPath;
            m_pPathDlg->SetClassPath( sClassPath );
        }
    }
    else
        sClassPath = m_pPathDlg->GetClassPath();

    m_pPathDlg->SetFocus();
    if ( m_pPathDlg->Execute() == RET_OK )
    {

        if ( m_pPathDlg->GetClassPath() != sClassPath )
        {
            sClassPath = m_pPathDlg->GetClassPath();
            sal_Bool bRunning = sal_False;
            javaFrameworkError eErr = jfw_isVMRunning( &bRunning );
            DBG_ASSERT( JFW_E_NONE == eErr,
                        "SvxJavaOptionsPage::ParameterHdl_Impl(): error in jfw_isVMRunning" );
            (void)eErr;
            if ( bRunning )
            {
                ScopedVclPtrInstance< MessageDialog > aWarnBox( this, CUI_RES( RID_SVXSTR_OPTIONS_RESTART ), VCL_MESSAGE_INFO );
                aWarnBox->Execute();
            }
        }
    }
    else
        m_pPathDlg->SetClassPath( sClassPath );
#else
    (void) this;
#endif
}


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, ResetHdl_Impl, Idle *, void)
{
    LoadJREs();
}


IMPL_LINK_NOARG_TYPED(SvxJavaOptionsPage, StartFolderPickerHdl, void*, void)
{
    try
    {
        Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
        if ( xAsyncDlg.is() )
            xAsyncDlg->startExecuteModal( xDialogListener.get() );
        else if ( xFolderPicker.is() && xFolderPicker->execute() == ExecutableDialogResults::OK )
            AddFolder( xFolderPicker->getDirectory() );
    }
    catch ( Exception& )
    {
        SAL_WARN( "cui.options", "SvxJavaOptionsPage::StartFolderPickerHdl(): caught exception" );
    }
}


IMPL_LINK_TYPED( SvxJavaOptionsPage, DialogClosedHdl, DialogClosedEvent*, pEvt, void )
{
    if ( RET_OK == pEvt->DialogResult )
    {
        DBG_ASSERT( xFolderPicker.is(), "SvxJavaOptionsPage::DialogClosedHdl(): no folder picker" );

        AddFolder( xFolderPicker->getDirectory() );
    }
}


IMPL_LINK_NOARG_TYPED( SvxJavaOptionsPage, ExpertConfigHdl_Impl, Button*, void )
{
    ScopedVclPtrInstance< CuiAboutConfigTabPage > m_pExpertConfigDlg(this);
    m_pExpertConfigDlg->Reset();//initialize and reset function

    if( RET_OK == m_pExpertConfigDlg->Execute() )
    {
        m_pExpertConfigDlg->FillItemSet();//save changes if there are any
    }

    m_pExpertConfigDlg.disposeAndClear();
}


void SvxJavaOptionsPage::ClearJavaInfo()
{
#if HAVE_FEATURE_JAVA
    if ( m_parJavaInfo )
    {
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pInfo = *parInfo++;
            jfw_freeJavaInfo( pInfo );
        }

        rtl_freeMemory( m_parJavaInfo );
        m_parJavaInfo = nullptr;
        m_nInfoSize = 0;
    }
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
    javaFrameworkError eErr = jfw_findAllJREs( &m_parJavaInfo, &m_nInfoSize );
    if ( JFW_E_NONE == eErr && m_parJavaInfo )
    {
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pInfo = *parInfo++;
            AddJRE( pInfo );
        }
    }

    std::vector< JavaInfo* >::iterator pIter;
    for ( pIter = m_aAddedInfos.begin(); pIter != m_aAddedInfos.end(); ++pIter )
    {
        JavaInfo* pInfo = *pIter;
        AddJRE( pInfo );
    }

    JavaInfo* pSelectedJava = nullptr;
    eErr = jfw_getSelectedJRE( &pSelectedJava );
    if ( JFW_E_NONE == eErr && pSelectedJava )
    {
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pCmpInfo = *parInfo++;
            if ( jfw_areEqualJavaInfo( pCmpInfo, pSelectedJava ) )
            {
                SvTreeListEntry* pEntry = m_pJavaList->GetEntry(i);
                if ( pEntry )
                    m_pJavaList->HandleEntryChecked( pEntry );
                break;
            }
        }
    }

    jfw_freeJavaInfo( pSelectedJava );
#else
    (void) this;
#endif
}


void SvxJavaOptionsPage::AddJRE( JavaInfo* _pInfo )
{
#if HAVE_FEATURE_JAVA
    OUStringBuffer sEntry;
    sEntry.append('\t');
    sEntry.append(_pInfo->sVendor);
    sEntry.append('\t');
    sEntry.append(_pInfo->sVersion);
    sEntry.append('\t');
    if ( ( _pInfo->nFeatures & JFW_FEATURE_ACCESSBRIDGE ) == JFW_FEATURE_ACCESSBRIDGE )
        sEntry.append(m_sAccessibilityText);
    SvTreeListEntry* pEntry = m_pJavaList->InsertEntry(sEntry.makeStringAndClear());
    INetURLObject aLocObj( OUString( _pInfo->sLocation ) );
    OUString* pLocation = new OUString( aLocObj.getFSysPath( INetURLObject::FSYS_DETECT ) );
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

    if ( SV_BUTTON_CHECKED == eState )
    {
        // we have radio button behavior -> so uncheck the other entries
        SvTreeListEntry* pEntry = m_pJavaList->First();
        while ( pEntry )
        {
            if ( pEntry != _pEntry )
                m_pJavaList->SetCheckButtonState( pEntry, SV_BUTTON_UNCHECKED );
            pEntry = m_pJavaList->Next( pEntry );
        }
    }
    else
        m_pJavaList->SetCheckButtonState( _pEntry, SV_BUTTON_CHECKED );
}


void SvxJavaOptionsPage::AddFolder( const OUString& _rFolder )
{
#if HAVE_FEATURE_JAVA
    bool bStartAgain = true;
    JavaInfo* pInfo = nullptr;
    javaFrameworkError eErr = jfw_getJavaInfoByPath( _rFolder.pData, &pInfo );
    if ( JFW_E_NONE == eErr && pInfo )
    {
        sal_Int32 nPos = 0;
        bool bFound = false;
        JavaInfo** parInfo = m_parJavaInfo;
        for ( sal_Int32 i = 0; i < m_nInfoSize; ++i )
        {
            JavaInfo* pCmpInfo = *parInfo++;
            if ( jfw_areEqualJavaInfo( pCmpInfo, pInfo ) )
            {
                bFound = true;
                nPos = i;
                break;
            }
        }

        if ( !bFound )
        {
            std::vector< JavaInfo* >::iterator pIter;
            for ( pIter = m_aAddedInfos.begin(); pIter != m_aAddedInfos.end(); ++pIter )
            {
                JavaInfo* pCmpInfo = *pIter;
                if ( jfw_areEqualJavaInfo( pCmpInfo, pInfo ) )
                {
                    bFound = true;
                    break;
                }
            }
        }

        if ( !bFound )
        {
            jfw_addJRELocation( pInfo->sLocation.pData );
            AddJRE( pInfo );
            m_aAddedInfos.push_back( pInfo );
            nPos = m_pJavaList->GetEntryCount() - 1;
        }
        else
            jfw_freeJavaInfo( pInfo );

        SvTreeListEntry* pEntry = m_pJavaList->GetEntry( nPos );
        m_pJavaList->Select( pEntry );
        m_pJavaList->SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );
        HandleCheckEntry( pEntry );
        bStartAgain = false;
    }
    else if ( JFW_E_NOT_RECOGNIZED == eErr )
    {
        ScopedVclPtrInstance< MessageDialog > aErrBox( this, CUI_RES( RID_SVXSTR_JRE_NOT_RECOGNIZED ) );
        aErrBox->Execute();
    }
    else if ( JFW_E_FAILED_VERSION == eErr )
    {
        ScopedVclPtrInstance< MessageDialog > aErrBox( this, CUI_RES( RID_SVXSTR_JRE_FAILED_VERSION ) );
        aErrBox->Execute();
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


VclPtr<SfxTabPage> SvxJavaOptionsPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxJavaOptionsPage>::Create( pParent, *rAttrSet );
}


bool SvxJavaOptionsPage::FillItemSet( SfxItemSet* /*rCoreSet*/ )
{
    bool bModified = false;

    if ( m_pExperimentalCB->IsValueChangedFromSaved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetExperimentalMode( m_pExperimentalCB->IsChecked() );
        bModified = true;
    }

    if ( m_pMacroCB->IsValueChangedFromSaved() )
    {
        SvtMiscOptions aMiscOpt;
        aMiscOpt.SetMacroRecorderMode( m_pMacroCB->IsChecked() );
        bModified = true;
    }

#if HAVE_FEATURE_JAVA
    javaFrameworkError eErr = JFW_E_NONE;
    if ( m_pParamDlg )
    {
        Sequence< OUString > aParamList = m_pParamDlg->GetParameters();
        sal_Int32 i, nSize = aParamList.getLength();
        rtl_uString** pParamArr = static_cast<rtl_uString**>(rtl_allocateMemory( sizeof(rtl_uString*) * nSize ));
        rtl_uString** pParamArrIter = pParamArr;
        const OUString* pList = aParamList.getConstArray();
        for ( i = 0; i < nSize; ++i )
            pParamArr[i] = pList[i].pData;
        eErr = jfw_setVMParameters( pParamArrIter, nSize );
        SAL_WARN_IF(JFW_E_NONE != eErr, "cui.options", "SvxJavaOptionsPage::FillItemSet(): error in jfw_setVMParameters"); (void)eErr;
        pParamArrIter = pParamArr;
        rtl_freeMemory( pParamArr );
        bModified = true;
    }

    if ( m_pPathDlg )
    {
        OUString sPath( m_pPathDlg->GetClassPath() );
        if ( m_pPathDlg->GetOldPath() != sPath )
        {
            eErr = jfw_setUserClassPath( sPath.pData );
            SAL_WARN_IF(JFW_E_NONE != eErr, "cui.options", "SvxJavaOptionsPage::FillItemSet(): error in jfw_setUserClassPath"); (void)eErr;
            bModified = true;
        }
    }

    sal_uLong nCount = m_pJavaList->GetEntryCount();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        if ( m_pJavaList->GetCheckButtonState( m_pJavaList->GetEntry(i) ) == SV_BUTTON_CHECKED )
        {
            JavaInfo* pInfo = nullptr;
            if ( i < static_cast< sal_uLong >( m_nInfoSize ) )
                pInfo = m_parJavaInfo[i];
            else
                pInfo = m_aAddedInfos[ i - m_nInfoSize ];

            JavaInfo* pSelectedJava = nullptr;
            eErr = jfw_getSelectedJRE( &pSelectedJava );
            if ( JFW_E_NONE == eErr || JFW_E_INVALID_SETTINGS == eErr )
            {
                if (pSelectedJava == nullptr || !jfw_areEqualJavaInfo( pInfo, pSelectedJava ) )
                {
                    sal_Bool bRunning = sal_False;
                    eErr = jfw_isVMRunning( &bRunning );
                    DBG_ASSERT( JFW_E_NONE == eErr,
                                "SvxJavaOptionsPage::FillItemSet(): error in jfw_isVMRunning" );
                    if ( bRunning ||
                        ( ( pInfo->nRequirements & JFW_REQUIRE_NEEDRESTART ) == JFW_REQUIRE_NEEDRESTART ) )
                    {
                        svtools::executeRestartDialog(
                            comphelper::getProcessComponentContext(), this,
                            svtools::RESTART_REASON_JAVA);
                    }

                    eErr = jfw_setSelectedJRE( pInfo );
                    SAL_WARN_IF(JFW_E_NONE != eErr, "cui.options", "SvxJavaOptionsPage::FillItemSet(): error in jfw_setSelectedJRE"); (void)eErr;
                    bModified = true;
                }
            }
            jfw_freeJavaInfo( pSelectedJava );
            break;
        }
    }

    sal_Bool bEnabled = sal_False;
    eErr = jfw_getEnabled( &bEnabled );
    DBG_ASSERT( JFW_E_NONE == eErr,
                "SvxJavaOptionsPage::FillItemSet(): error in jfw_getEnabled" );
    if ( bool(bEnabled) != m_pJavaEnableCB->IsChecked() )
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
    sal_Bool bEnabled = sal_False;
    javaFrameworkError eErr = jfw_getEnabled( &bEnabled );
    if ( eErr != JFW_E_NONE )
        bEnabled = sal_False;
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
    OUString aUserData;
    SetUserData( aUserData );
}

// class SvxJavaParameterDlg ---------------------------------------------

SvxJavaParameterDlg::SvxJavaParameterDlg( vcl::Window* pParent ) :

    ModalDialog( pParent, "JavaStartParameters",
                 "cui/ui/javastartparametersdialog.ui" )
{
    get( m_pParameterEdit, "parameterfield");
    get( m_pAssignBtn, "assignbtn");
    get( m_pAssignedList, "assignlist");
    m_pAssignedList->SetDropDownLineCount(6);
    m_pAssignedList->set_width_request(m_pAssignedList->approximate_char_width() * 54);
    get( m_pRemoveBtn, "removebtn");

    m_pParameterEdit->SetModifyHdl( LINK( this, SvxJavaParameterDlg, ModifyHdl_Impl ) );
    m_pAssignBtn->SetClickHdl( LINK( this, SvxJavaParameterDlg, AssignHdl_Impl ) );
    m_pRemoveBtn->SetClickHdl( LINK( this, SvxJavaParameterDlg, RemoveHdl_Impl ) );
    m_pAssignedList->SetSelectHdl( LINK( this, SvxJavaParameterDlg, SelectHdl_Impl ) );
    m_pAssignedList->SetDoubleClickHdl( LINK( this, SvxJavaParameterDlg, DblClickHdl_Impl ) );

    ModifyHdl_Impl( *m_pParameterEdit );
    EnableRemoveButton();
}

SvxJavaParameterDlg::~SvxJavaParameterDlg()
{
    disposeOnce();
}

void SvxJavaParameterDlg::dispose()
{
    m_pParameterEdit.clear();
    m_pAssignBtn.clear();
    m_pAssignedList.clear();
    m_pRemoveBtn.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG_TYPED(SvxJavaParameterDlg, ModifyHdl_Impl, Edit&, void)
{
    OUString sParam = comphelper::string::strip(m_pParameterEdit->GetText(), ' ');
    m_pAssignBtn->Enable(!sParam.isEmpty());
}


IMPL_LINK_NOARG_TYPED(SvxJavaParameterDlg, AssignHdl_Impl, Button*, void)
{
    OUString sParam = comphelper::string::strip(m_pParameterEdit->GetText(), ' ');
    if (!sParam.isEmpty())
    {
        sal_Int32 nPos = m_pAssignedList->GetEntryPos( sParam );
        if ( LISTBOX_ENTRY_NOTFOUND == nPos )
            nPos = m_pAssignedList->InsertEntry( sParam );
        m_pAssignedList->SelectEntryPos( nPos );
        m_pParameterEdit->SetText( OUString() );
        ModifyHdl_Impl( *m_pParameterEdit );
        EnableRemoveButton();
    }
}



IMPL_LINK_NOARG_TYPED(SvxJavaParameterDlg, SelectHdl_Impl, ListBox&, void)
{
    EnableRemoveButton();
}


IMPL_LINK_NOARG_TYPED(SvxJavaParameterDlg, DblClickHdl_Impl, ListBox&, void)
{
    sal_Int32 nPos = m_pAssignedList->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        m_pParameterEdit->SetText( m_pAssignedList->GetEntry( nPos ) );
}


IMPL_LINK_NOARG_TYPED(SvxJavaParameterDlg, RemoveHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pAssignedList->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pAssignedList->RemoveEntry( nPos );
        sal_Int32 nCount = m_pAssignedList->GetEntryCount();
        if ( nCount )
        {
            if ( nPos >= nCount )
                nPos = ( nCount - 1 );
            m_pAssignedList->SelectEntryPos( nPos );
        }
    }
    EnableRemoveButton();
}



short SvxJavaParameterDlg::Execute()
{
    m_pParameterEdit->GrabFocus();
    m_pAssignedList->SetNoSelection();
    return ModalDialog::Execute();
}


Sequence< OUString > SvxJavaParameterDlg::GetParameters() const
{
    sal_Int32 nCount = m_pAssignedList->GetEntryCount();
    Sequence< OUString > aParamList( nCount );
    OUString* pArray = aParamList.getArray();
     for ( sal_Int32 i = 0; i < nCount; ++i )
         pArray[i] = m_pAssignedList->GetEntry(i);
    return aParamList;
}


void SvxJavaParameterDlg::SetParameters( Sequence< OUString >& rParams )
{
    m_pAssignedList->Clear();
    sal_uLong i, nCount = rParams.getLength();
    const OUString* pArray = rParams.getConstArray();
    for ( i = 0; i < nCount; ++i )
    {
        OUString sParam = OUString( *pArray++ );
        m_pAssignedList->InsertEntry( sParam );
    }
}

// class SvxJavaClassPathDlg ---------------------------------------------

SvxJavaClassPathDlg::SvxJavaClassPathDlg(vcl::Window* pParent)
    : ModalDialog(pParent, "JavaClassPath", "cui/ui/javaclasspathdialog.ui")
{
    get( m_pPathList, "paths");
    m_pPathList->SetDropDownLineCount(8);
    m_pPathList->set_width_request(m_pPathList->approximate_char_width() * 54);
    get( m_pAddArchiveBtn, "archive");
    get( m_pAddPathBtn, "folder");
    get( m_pRemoveBtn, "remove");

    m_pAddArchiveBtn->SetClickHdl( LINK( this, SvxJavaClassPathDlg, AddArchiveHdl_Impl ) );
    m_pAddPathBtn->SetClickHdl( LINK( this, SvxJavaClassPathDlg, AddPathHdl_Impl ) );
    m_pRemoveBtn->SetClickHdl( LINK( this, SvxJavaClassPathDlg, RemoveHdl_Impl ) );
    m_pPathList->SetSelectHdl( LINK( this, SvxJavaClassPathDlg, SelectHdl_Impl ) );

    // set initial focus to path list
    m_pPathList->GrabFocus();
}


SvxJavaClassPathDlg::~SvxJavaClassPathDlg()
{
    disposeOnce();
}

void SvxJavaClassPathDlg::dispose()
{
    if (m_pPathList)
    {
        sal_Int32 i, nCount = m_pPathList->GetEntryCount();
        for ( i = 0; i < nCount; ++i )
            delete static_cast< OUString* >( m_pPathList->GetEntryData(i) );
        m_pPathList = nullptr;
    }
    m_pPathList.clear();
    m_pAddArchiveBtn.clear();
    m_pAddPathBtn.clear();
    m_pRemoveBtn.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED(SvxJavaClassPathDlg, AddArchiveHdl_Impl, Button*, void)
{
    sfx2::FileDialogHelper aDlg( TemplateDescription::FILEOPEN_SIMPLE, 0 );
    aDlg.SetTitle( CUI_RES( RID_SVXSTR_ARCHIVE_TITLE ) );
    aDlg.AddFilter( CUI_RES( RID_SVXSTR_ARCHIVE_HEADLINE ), "*.jar;*.zip" );
    OUString sFolder;
    if ( m_pPathList->GetSelectEntryCount() > 0 )
    {
        INetURLObject aObj( m_pPathList->GetSelectEntry(), INetURLObject::FSYS_DETECT );
        sFolder = aObj.GetMainURL( INetURLObject::NO_DECODE );
    }
    else
         sFolder = SvtPathOptions().GetWorkPath();
    aDlg.SetDisplayDirectory( sFolder );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        OUString sURL = aDlg.GetPath();
        INetURLObject aURL( sURL );
        OUString sFile = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
        if ( !IsPathDuplicate( sURL ) )
        {
            sal_Int32 nPos = m_pPathList->InsertEntry( sFile, SvFileInformationManager::GetImage( aURL ) );
            m_pPathList->SelectEntryPos( nPos );
        }
        else
        {
            OUString sMsg( CUI_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sFile );
            ScopedVclPtrInstance<MessageDialog>::Create(this, sMsg)->Execute();
        }
    }
    EnableRemoveButton();
}


IMPL_LINK_NOARG_TYPED(SvxJavaClassPathDlg, AddPathHdl_Impl, Button*, void)
{
    Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference < XFolderPicker2 > xFolderPicker = FolderPicker::create(xContext);

    OUString sOldFolder;
    if ( m_pPathList->GetSelectEntryCount() > 0 )
    {
        INetURLObject aObj( m_pPathList->GetSelectEntry(), INetURLObject::FSYS_DETECT );
        sOldFolder = aObj.GetMainURL( INetURLObject::NO_DECODE );
    }
    else
        sOldFolder = SvtPathOptions().GetWorkPath();
    xFolderPicker->setDisplayDirectory( sOldFolder );
    if ( xFolderPicker->execute() == ExecutableDialogResults::OK )
    {
        OUString sFolderURL( xFolderPicker->getDirectory() );
        INetURLObject aURL( sFolderURL );
        OUString sNewFolder = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
        if ( !IsPathDuplicate( sFolderURL ) )
        {
            sal_Int32 nPos = m_pPathList->InsertEntry( sNewFolder, SvFileInformationManager::GetImage( aURL ) );
            m_pPathList->SelectEntryPos( nPos );
        }
        else
        {
            OUString sMsg( CUI_RES( RID_SVXSTR_MULTIFILE_DBL_ERR ) );
            sMsg = sMsg.replaceFirst( "%1", sNewFolder );
            ScopedVclPtrInstance<MessageDialog>::Create(this, sMsg)->Execute();
        }
    }
    EnableRemoveButton();
}


IMPL_LINK_NOARG_TYPED(SvxJavaClassPathDlg, RemoveHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pPathList->GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        m_pPathList->RemoveEntry( nPos );
        sal_Int32 nCount = m_pPathList->GetEntryCount();
        if ( nCount )
        {
            if ( nPos >= nCount )
                nPos = ( nCount - 1 );
            m_pPathList->SelectEntryPos( nPos );
        }
    }

    EnableRemoveButton();
}


IMPL_LINK_NOARG_TYPED(SvxJavaClassPathDlg, SelectHdl_Impl, ListBox&, void)
{
    EnableRemoveButton();
}


bool SvxJavaClassPathDlg::IsPathDuplicate( const OUString& _rPath )
{
    bool bRet = false;
    INetURLObject aFileObj( _rPath );
    sal_Int32 nCount = m_pPathList->GetEntryCount();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        INetURLObject aOtherObj( m_pPathList->GetEntry(i), INetURLObject::FSYS_DETECT );
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
    OUString sPath;
    sal_Int32 nCount = m_pPathList->GetEntryCount();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        if ( !sPath.isEmpty() )
            sPath += OUStringLiteral1<CLASSPATH_DELIMITER>();
        OUString* pFullPath = static_cast< OUString* >( m_pPathList->GetEntryData(i) );
        if ( pFullPath )
            sPath += *pFullPath;
        else
            sPath += m_pPathList->GetEntry(i);
    }
    return sPath;
}


void SvxJavaClassPathDlg::SetClassPath( const OUString& _rPath )
{
    if ( m_sOldPath.isEmpty() )
        m_sOldPath = _rPath;
    m_pPathList->Clear();
    sal_Int32 nIdx = 0;
    sal_Int32 nCount = comphelper::string::getTokenCount(_rPath, CLASSPATH_DELIMITER);
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        OUString sToken = _rPath.getToken( 0, CLASSPATH_DELIMITER, nIdx );
        INetURLObject aURL( sToken, INetURLObject::FSYS_DETECT );
        OUString sPath = aURL.getFSysPath( INetURLObject::FSYS_DETECT );
        m_pPathList->InsertEntry( sPath, SvFileInformationManager::GetImage( aURL ) );
    }
    // select first entry
    m_pPathList->SelectEntryPos(0);
    SelectHdl_Impl( *m_pPathList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


