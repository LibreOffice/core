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

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/zforlist.hxx>
#include "optupdt.hxx"
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/ucb/XWebDAVCommandEnvironment.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/setup/UpdateCheckConfig.hpp>
#include <osl/file.hxx>
#include <osl/security.hxx>

using namespace ::css;

SvxOnlineUpdateTabPage::SvxOnlineUpdateTabPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptOnlineUpdatePage", "cui/ui/optonlineupdatepage.ui", &rSet)
{
    m_aNeverChecked = get<FixedText>("neverchecked")->GetText();
    get(m_pAutoCheckCheckBox, "autocheck");
    get(m_pEveryDayButton, "everyday");
    get(m_pEveryWeekButton, "everyweek");
    get(m_pEveryMonthButton, "everymonth");
    get(m_pCheckNowButton, "checknow");
    get(m_pAutoDownloadCheckBox, "autodownload");
    get(m_pDestPathLabel, "destpathlabel");
    get(m_pDestPath, "destpath");
    get(m_pChangePathButton, "changepath");
    get(m_pLastChecked, "lastchecked");
    get(m_pExtrasCheckBox, "extrabits");
    get(m_pUserAgentLabel, "useragent");

    m_pAutoCheckCheckBox->SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, AutoCheckHdl_Impl ) );
    m_pExtrasCheckBox->SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, ExtrasCheckHdl_Impl ) );
    m_pCheckNowButton->SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, CheckNowHdl_Impl ) );
    m_pChangePathButton->SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, FileDialogHdl_Impl ) );

    uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    m_xUpdateAccess = setup::UpdateCheckConfig::create( xContext );

    bool bDownloadSupported = false;
    m_xUpdateAccess->getByName( "DownloadSupported" ) >>= bDownloadSupported;

    WinBits nStyle = m_pDestPath->GetStyle();
    nStyle |= WB_PATHELLIPSIS;
    m_pDestPath->SetStyle(nStyle);

    m_pAutoDownloadCheckBox->Show(bDownloadSupported);
    m_pDestPathLabel->Show(bDownloadSupported);
    m_pDestPath->Show(bDownloadSupported);
    m_pChangePathButton->Show(bDownloadSupported);

    m_aLastCheckedTemplate = m_pLastChecked->GetText();

    UpdateLastCheckedText();
    UpdateUserAgent();
}

SvxOnlineUpdateTabPage::~SvxOnlineUpdateTabPage()
{
    disposeOnce();
}

void SvxOnlineUpdateTabPage::dispose()
{
    m_pAutoCheckCheckBox.clear();
    m_pEveryDayButton.clear();
    m_pEveryWeekButton.clear();
    m_pEveryMonthButton.clear();
    m_pCheckNowButton.clear();
    m_pAutoDownloadCheckBox.clear();
    m_pDestPathLabel.clear();
    m_pDestPath.clear();
    m_pChangePathButton.clear();
    m_pLastChecked.clear();
    m_pExtrasCheckBox.clear();
    m_pUserAgentLabel.clear();

    SfxTabPage::dispose();
}

void SvxOnlineUpdateTabPage::UpdateLastCheckedText()
{
    OUString aDateStr;
    OUString aTimeStr;
    OUString aText;
    sal_Int64 lastChecked = 0;

    m_xUpdateAccess->getByName("LastCheck") >>= lastChecked;

    if( lastChecked == 0 ) // never checked
    {
        aText = m_aNeverChecked;
    }
    else
    {
        TimeValue   lastCheckedTV;
        oslDateTime lastCheckedDT;

        Date  aDate( Date::EMPTY );
        tools::Time  aTime( tools::Time::EMPTY );

        lastCheckedTV.Seconds = (sal_uInt32) lastChecked;
        osl_getLocalTimeFromSystemTime( &lastCheckedTV, &lastCheckedTV );

        if ( osl_getDateTimeFromTimeValue(  &lastCheckedTV, &lastCheckedDT ) )
        {
            aDate = Date( lastCheckedDT.Day, lastCheckedDT.Month, lastCheckedDT.Year );
            aTime = ::tools::Time( lastCheckedDT.Hours, lastCheckedDT.Minutes );
        }

        LanguageType eUILang = Application::GetSettings().GetUILanguageTag().getLanguageType();
        SvNumberFormatter *pNumberFormatter = new SvNumberFormatter( ::comphelper::getProcessComponentContext(), eUILang );
        Color*      pColor = nullptr;
        Date*       pNullDate = pNumberFormatter->GetNullDate();
        sal_uInt32  nFormat = pNumberFormatter->GetStandardFormat( css::util::NumberFormat::DATE, eUILang );

        pNumberFormatter->GetOutputString( aDate - *pNullDate, nFormat, aDateStr, &pColor );

        nFormat = pNumberFormatter->GetStandardFormat( css::util::NumberFormat::TIME, eUILang );
        pNumberFormatter->GetOutputString( aTime.GetTimeInDays(), nFormat, aTimeStr, &pColor );

        delete pColor;
        delete pNumberFormatter;

        aText = m_aLastCheckedTemplate;
        sal_Int32 nIndex = aText.indexOf( "%DATE%" );
        if ( nIndex != -1 )
            aText = aText.replaceAt( nIndex, 6, aDateStr );

        nIndex = aText.indexOf( "%TIME%" );
        if ( nIndex != -1 )
            aText = aText.replaceAt( nIndex, 6, aTimeStr );
    }

    m_pLastChecked->SetText( aText );
}

void SvxOnlineUpdateTabPage::UpdateUserAgent()
{
    try {
        uno::Reference< ucb::XWebDAVCommandEnvironment > xDav(
            css::deployment::UpdateInformationProvider::create(
                ::comphelper::getProcessComponentContext() ),
            css::uno::UNO_QUERY_THROW );

        OUString aPseudoURL = "useragent:normal";
        if( m_pExtrasCheckBox->IsChecked() )
            aPseudoURL = "useragent:extended";
        uno::Sequence< beans::StringPair > aHeaders
            = xDav->getUserRequestHeaders( aPseudoURL, ucb::WebDAVHTTPMethod(0) );

        for ( auto i = aHeaders.begin(); i != aHeaders.end(); ++i )
        {
            if ( i->First == "User-Agent" )
            {
                OUString aText = i->Second;
                aText = aText.replaceAll(";", ";\n");
                aText = aText.replaceAll("(", "\n(");
                m_pUserAgentLabel->SetText( aText );
                break;
            }
        }
    } catch (const uno::Exception &) {
        SAL_WARN( "cui.options", "Unexpected exception fetching User Agent" );
    }
}

VclPtr<SfxTabPage>
SvxOnlineUpdateTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxOnlineUpdateTabPage>::Create( pParent, *rAttrSet );
}

bool SvxOnlineUpdateTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;

    bool bValue;
    sal_Int64 nValue;

    if( m_pAutoCheckCheckBox->IsValueChangedFromSaved() )
    {
        bValue = m_pAutoCheckCheckBox->IsChecked();
        m_xUpdateAccess->replaceByName( "AutoCheckEnabled", uno::makeAny( bValue ) );
        bModified = true;
    }

    nValue = 0;
    if( m_pEveryDayButton->IsChecked() )
    {
        if( !m_pEveryDayButton->GetSavedValue() )
            nValue = 86400;
    }
    else if( m_pEveryWeekButton->IsChecked() )
    {
        if( !m_pEveryWeekButton->GetSavedValue() )
            nValue = 604800;
    }
    else if( m_pEveryMonthButton->IsChecked() )
    {
        if( !m_pEveryMonthButton->GetSavedValue() )
            nValue = 2592000;
    }

    if( nValue > 0 )
    {
        m_xUpdateAccess->replaceByName( "CheckInterval", uno::makeAny( nValue ) );
        bModified = true;
    }

    if( m_pAutoDownloadCheckBox->IsValueChangedFromSaved() )
    {
        bValue = m_pAutoDownloadCheckBox->IsChecked();
        m_xUpdateAccess->replaceByName( "AutoDownloadEnabled", uno::makeAny( bValue ) );
        bModified = true;
    }

    OUString sValue, aURL;
    m_xUpdateAccess->getByName( "DownloadDestination" ) >>= sValue;

    if( ( osl::FileBase::E_None == osl::FileBase::getFileURLFromSystemPath(m_pDestPath->GetText(), aURL) ) &&
        ( ! aURL.equals( sValue ) ) )
    {
        m_xUpdateAccess->replaceByName( "DownloadDestination", uno::makeAny( aURL ) );
        bModified = true;
    }

    if( m_pExtrasCheckBox->IsValueChangedFromSaved() )
    {
        bValue = m_pExtrasCheckBox->IsChecked();
        m_xUpdateAccess->replaceByName( "ExtendedUserAgent", uno::makeAny( bValue ) );
        bModified = true;
    }

    uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
        xChangesBatch->commitChanges();

    return bModified;
}

void SvxOnlineUpdateTabPage::Reset( const SfxItemSet* )
{
    bool bValue = false;
    m_xUpdateAccess->getByName( "AutoCheckEnabled" ) >>= bValue;

    m_pAutoCheckCheckBox->Check(bValue);
    m_pEveryDayButton->Enable(bValue);
    m_pEveryWeekButton->Enable(bValue);
    m_pEveryMonthButton->Enable(bValue);

    sal_Int64 nValue = 0;
    m_xUpdateAccess->getByName( "CheckInterval" ) >>= nValue;

    if( nValue == 86400 )
        m_pEveryDayButton->Check();
    else if( nValue == 604800 )
        m_pEveryWeekButton->Check();
    else
        m_pEveryMonthButton->Check();

    m_pAutoCheckCheckBox->SaveValue();
    m_pEveryDayButton->SaveValue();
    m_pEveryWeekButton->SaveValue();
    m_pEveryMonthButton->SaveValue();

    m_xUpdateAccess->getByName( "AutoDownloadEnabled" ) >>= bValue;
    m_pAutoDownloadCheckBox->Check(bValue);
    m_pDestPathLabel->Enable();
    m_pDestPath->Enable();
    m_pChangePathButton->Enable();

    OUString sValue, aPath;
    m_xUpdateAccess->getByName( "DownloadDestination" ) >>= sValue;

    if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sValue, aPath) )
        m_pDestPath->SetText(aPath);

    m_xUpdateAccess->getByName( "ExtendedUserAgent" ) >>= bValue;
    m_pExtrasCheckBox->Check(bValue);
    m_pExtrasCheckBox->SaveValue();
    UpdateUserAgent();

    m_pAutoDownloadCheckBox->SaveValue();
}

void SvxOnlineUpdateTabPage::FillUserData()
{
}

IMPL_LINK_TYPED( SvxOnlineUpdateTabPage, AutoCheckHdl_Impl, Button*, pBox, void )
{
    bool bEnabled = static_cast<CheckBox*>(pBox)->IsChecked();

    m_pEveryDayButton->Enable(bEnabled);
    m_pEveryWeekButton->Enable(bEnabled);
    m_pEveryMonthButton->Enable(bEnabled);
}

IMPL_LINK_TYPED( SvxOnlineUpdateTabPage, ExtrasCheckHdl_Impl, Button*, , void )
{
    UpdateUserAgent();
}

IMPL_LINK_NOARG_TYPED(SvxOnlineUpdateTabPage, FileDialogHdl_Impl, Button*, void)
{
    uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < ui::dialogs::XFolderPicker2 >  xFolderPicker = ui::dialogs::FolderPicker::create(xContext);

    OUString aURL;
    if( osl::FileBase::E_None != osl::FileBase::getFileURLFromSystemPath(m_pDestPath->GetText(), aURL) )
        osl::Security().getHomeDir(aURL);

    xFolderPicker->setDisplayDirectory( aURL );
    sal_Int16 nRet = xFolderPicker->execute();

    if ( ui::dialogs::ExecutableDialogResults::OK == nRet )
    {
        OUString aFolder;
        if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(xFolderPicker->getDirectory(), aFolder))
            m_pDestPath->SetText( aFolder );
    }
}

IMPL_LINK_NOARG_TYPED(SvxOnlineUpdateTabPage, CheckNowHdl_Impl, Button*, void)
{
    uno::Reference < uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
            css::configuration::theDefaultProvider::get( xContext ) );

        beans::NamedValue aProperty;
        aProperty.Name  = "nodepath";
        aProperty.Value = uno::makeAny( OUString("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob") );

        uno::Sequence< uno::Any > aArgumentList( 1 );
        aArgumentList[0] = uno::makeAny( aProperty );

        uno::Reference< container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess", aArgumentList ),
            uno::UNO_QUERY_THROW );

        util::URL aURL;
        xNameAccess->getByName("URL") >>= aURL.Complete;

        uno::Reference < util::XURLTransformer > xTransformer( util::URLTransformer::create( xContext ) );

        xTransformer->parseStrict(aURL);

        uno::Reference < frame::XDesktop2 > xDesktop = frame::Desktop::create( xContext );

        uno::Reference< frame::XDispatchProvider > xDispatchProvider(
            xDesktop->getCurrentFrame(), uno::UNO_QUERY );

        uno::Reference< frame::XDispatch > xDispatch;
        if( xDispatchProvider.is() )
            xDispatch = xDispatchProvider->queryDispatch(aURL, OUString(), 0);

        if( xDispatch.is() )
            xDispatch->dispatch(aURL, uno::Sequence< beans::PropertyValue > ());

        UpdateLastCheckedText();
    }
    catch( const uno::Exception& e )
    {
         OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
