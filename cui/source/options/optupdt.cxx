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
#include <svl/zforlist.hxx>
#include "optupdt.hxx"
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
#include <com/sun/star/configuration/ReadWriteAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <tools/diagnose_ex.h>

using namespace ::css;

SvxOnlineUpdateTabPage::SvxOnlineUpdateTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/optonlineupdatepage.ui", "OptOnlineUpdatePage", &rSet)
    , m_xNeverChecked(m_xBuilder->weld_label("neverchecked"))
    , m_xAutoCheckCheckBox(m_xBuilder->weld_check_button("autocheck"))
    , m_xEveryDayButton(m_xBuilder->weld_radio_button("everyday"))
    , m_xEveryWeekButton(m_xBuilder->weld_radio_button("everyweek"))
    , m_xEveryMonthButton(m_xBuilder->weld_radio_button("everymonth"))
    , m_xCheckNowButton(m_xBuilder->weld_button("checknow"))
    , m_xAutoDownloadCheckBox(m_xBuilder->weld_check_button("autodownload"))
    , m_xDestPathLabel(m_xBuilder->weld_label("destpathlabel"))
    , m_xDestPath(m_xBuilder->weld_label("destpath"))
    , m_xChangePathButton(m_xBuilder->weld_button("changepath"))
    , m_xLastChecked(m_xBuilder->weld_label("lastchecked"))
    , m_xExtrasCheckBox(m_xBuilder->weld_check_button("extrabits"))
    , m_xUserAgentLabel(m_xBuilder->weld_label("useragent"))
{
    m_aNeverChecked = m_xNeverChecked->get_label();

    m_xAutoCheckCheckBox->connect_toggled( LINK( this, SvxOnlineUpdateTabPage, AutoCheckHdl_Impl ) );
    m_xExtrasCheckBox->connect_clicked( LINK( this, SvxOnlineUpdateTabPage, ExtrasCheckHdl_Impl ) );
    m_xCheckNowButton->connect_clicked( LINK( this, SvxOnlineUpdateTabPage, CheckNowHdl_Impl ) );
    m_xChangePathButton->connect_clicked( LINK( this, SvxOnlineUpdateTabPage, FileDialogHdl_Impl ) );

    uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    m_xUpdateAccess = setup::UpdateCheckConfig::create( xContext );
    m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(xContext, "*");

    bool bDownloadSupported = false;
    m_xUpdateAccess->getByName( "DownloadSupported" ) >>= bDownloadSupported;

    m_xAutoDownloadCheckBox->set_visible(bDownloadSupported);
    m_xDestPathLabel->set_visible(bDownloadSupported);
    m_xDestPath->set_visible(bDownloadSupported);
    m_xChangePathButton->set_visible(bDownloadSupported);

    m_aLastCheckedTemplate = m_xLastChecked->get_label();

    UpdateLastCheckedText();
    UpdateUserAgent();
}

SvxOnlineUpdateTabPage::~SvxOnlineUpdateTabPage()
{
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

        lastCheckedTV.Seconds = static_cast<sal_uInt32>(lastChecked);
        osl_getLocalTimeFromSystemTime( &lastCheckedTV, &lastCheckedTV );

        if ( osl_getDateTimeFromTimeValue(  &lastCheckedTV, &lastCheckedDT ) )
        {
            aDate = Date( lastCheckedDT.Day, lastCheckedDT.Month, lastCheckedDT.Year );
            aTime = ::tools::Time( lastCheckedDT.Hours, lastCheckedDT.Minutes );
        }

        LanguageType eUILang = Application::GetSettings().GetUILanguageTag().getLanguageType();
        std::unique_ptr<SvNumberFormatter> pNumberFormatter(new SvNumberFormatter( ::comphelper::getProcessComponentContext(), eUILang ));
        const Color*      pColor = nullptr;
        const Date& rNullDate = pNumberFormatter->GetNullDate();
        sal_uInt32  nFormat = pNumberFormatter->GetStandardFormat( SvNumFormatType::DATE, eUILang );

        pNumberFormatter->GetOutputString( aDate - rNullDate, nFormat, aDateStr, &pColor );

        nFormat = pNumberFormatter->GetStandardFormat( SvNumFormatType::TIME, eUILang );
        pNumberFormatter->GetOutputString( aTime.GetTimeInDays(), nFormat, aTimeStr, &pColor );

        pNumberFormatter.reset();

        aText = m_aLastCheckedTemplate;
        sal_Int32 nIndex = aText.indexOf( "%DATE%" );
        if ( nIndex != -1 )
            aText = aText.replaceAt( nIndex, 6, aDateStr );

        nIndex = aText.indexOf( "%TIME%" );
        if ( nIndex != -1 )
            aText = aText.replaceAt( nIndex, 6, aTimeStr );
    }

    m_xLastChecked->set_label(aText);
}

void SvxOnlineUpdateTabPage::UpdateUserAgent()
{
    try {
        uno::Reference< ucb::XWebDAVCommandEnvironment > xDav(
            css::deployment::UpdateInformationProvider::create(
                ::comphelper::getProcessComponentContext() ),
            css::uno::UNO_QUERY_THROW );

        OUString aPseudoURL = "useragent:normal";
        if( m_xExtrasCheckBox->get_active() )
            aPseudoURL = "useragent:extended";
        const uno::Sequence< beans::StringPair > aHeaders
            = xDav->getUserRequestHeaders( aPseudoURL, ucb::WebDAVHTTPMethod(0) );

        for (const css::beans::StringPair & aHeader : aHeaders)
        {
            if ( aHeader.First == "User-Agent" )
            {
                OUString aText = aHeader.Second;
                aText = aText.replaceAll(";", ";\n");
                aText = aText.replaceAll("(", "\n(");
                m_xUserAgentLabel->set_label(aText);
                break;
            }
        }
    } catch (const uno::Exception &) {
        TOOLS_WARN_EXCEPTION( "cui.options", "Unexpected exception fetching User Agent" );
    }
}

std::unique_ptr<SfxTabPage> SvxOnlineUpdateTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxOnlineUpdateTabPage>( pPage, pController, *rAttrSet );
}

bool SvxOnlineUpdateTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;

    bool bValue;
    sal_Int64 nValue;

    if( m_xAutoCheckCheckBox->get_state_changed_from_saved() )
    {
        bValue = m_xAutoCheckCheckBox->get_active();
        m_xUpdateAccess->replaceByName( "AutoCheckEnabled", uno::Any( bValue ) );
        bModified = true;
    }

    nValue = 0;
    if( m_xEveryDayButton->get_active() )
    {
        if( !m_xEveryDayButton->get_saved_state() )
            nValue = 86400;
    }
    else if( m_xEveryWeekButton->get_active() )
    {
        if( !m_xEveryWeekButton->get_saved_state() )
            nValue = 604800;
    }
    else if( m_xEveryMonthButton->get_active() )
    {
        if( !m_xEveryMonthButton->get_saved_state() )
            nValue = 2592000;
    }

    if( nValue > 0 )
    {
        m_xUpdateAccess->replaceByName( "CheckInterval", uno::Any( nValue ) );
        bModified = true;
    }

    if( m_xAutoDownloadCheckBox->get_state_changed_from_saved() )
    {
        bValue = m_xAutoDownloadCheckBox->get_active();
        m_xUpdateAccess->replaceByName( "AutoDownloadEnabled", uno::Any( bValue ) );
        bModified = true;
    }

    OUString sValue, aURL;
    m_xUpdateAccess->getByName( "DownloadDestination" ) >>= sValue;

    if( ( osl::FileBase::E_None == osl::FileBase::getFileURLFromSystemPath(m_xDestPath->get_label(), aURL) ) &&
        ( aURL != sValue ) )
    {
        m_xUpdateAccess->replaceByName( "DownloadDestination", uno::Any( aURL ) );
        bModified = true;
    }

    if( m_xExtrasCheckBox->get_state_changed_from_saved() )
    {
        bValue = m_xExtrasCheckBox->get_active();
        m_xUpdateAccess->replaceByName( "ExtendedUserAgent", uno::Any( bValue ) );
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
    beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/AutoCheckEnabled");
    bool bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;

    m_xAutoCheckCheckBox->set_active(bValue);
    m_xAutoCheckCheckBox->set_sensitive(!bReadOnly);

    sal_Int64 nValue = 0;
    m_xUpdateAccess->getByName( "CheckInterval" ) >>= nValue;
    aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/CheckInterval");
    bool bReadOnly2 = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
    m_xEveryDayButton->set_sensitive(bValue && !(bReadOnly || bReadOnly2));
    m_xEveryWeekButton->set_sensitive(bValue && !(bReadOnly || bReadOnly2));
    m_xEveryMonthButton->set_sensitive(bValue && !(bReadOnly || bReadOnly2));

    if( nValue == 86400 )
        m_xEveryDayButton->set_active(true);
    else if( nValue == 604800 )
        m_xEveryWeekButton->set_active(true);
    else
        m_xEveryMonthButton->set_active(true);

    m_xAutoCheckCheckBox->save_state();
    m_xEveryDayButton->save_state();
    m_xEveryWeekButton->save_state();
    m_xEveryMonthButton->save_state();

    m_xUpdateAccess->getByName( "AutoDownloadEnabled" ) >>= bValue;
    aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/AutoDownloadEnabled");
    bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
    m_xAutoDownloadCheckBox->set_active(bValue);
    m_xAutoDownloadCheckBox->set_sensitive(!bReadOnly);
    m_xDestPathLabel->set_sensitive(true);
    m_xDestPath->set_sensitive(true);

    OUString sValue, aPath;
    m_xUpdateAccess->getByName( "DownloadDestination" ) >>= sValue;
    aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/DownloadDestination");
    bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
    m_xChangePathButton->set_sensitive(!bReadOnly);

    if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sValue, aPath) )
        m_xDestPath->set_label(aPath);

    m_xUpdateAccess->getByName( "ExtendedUserAgent" ) >>= bValue;
    aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/ExtendedUserAgent");
    bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
    m_xExtrasCheckBox->set_active(bValue);
    m_xExtrasCheckBox->set_sensitive(!bReadOnly);
    m_xExtrasCheckBox->save_state();
    UpdateUserAgent();

    m_xAutoDownloadCheckBox->save_state();
}

void SvxOnlineUpdateTabPage::FillUserData()
{
}

IMPL_LINK(SvxOnlineUpdateTabPage, AutoCheckHdl_Impl, weld::ToggleButton&, rBox, void)
{
    bool bEnabled = rBox.get_active();
    beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName("/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/CheckInterval");
    bool bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
    m_xEveryDayButton->set_sensitive(bEnabled && !bReadOnly);
    m_xEveryWeekButton->set_sensitive(bEnabled && !bReadOnly);
    m_xEveryMonthButton->set_sensitive(bEnabled && !bReadOnly);
}

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, ExtrasCheckHdl_Impl, weld::Button&, void)
{
    UpdateUserAgent();
}

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, FileDialogHdl_Impl, weld::Button&, void)
{
    uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < ui::dialogs::XFolderPicker2 >  xFolderPicker = ui::dialogs::FolderPicker::create(xContext);

    OUString aURL;
    if( osl::FileBase::E_None != osl::FileBase::getFileURLFromSystemPath(m_xDestPath->get_label(), aURL) )
        osl::Security().getHomeDir(aURL);

    xFolderPicker->setDisplayDirectory( aURL );
    sal_Int16 nRet = xFolderPicker->execute();

    if ( ui::dialogs::ExecutableDialogResults::OK == nRet )
    {
        OUString aFolder;
        if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(xFolderPicker->getDirectory(), aFolder))
            m_xDestPath->set_label(aFolder);
    }
}

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, CheckNowHdl_Impl, weld::Button&, void)
{
    uno::Reference < uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
            css::configuration::theDefaultProvider::get( xContext ) );

        beans::NamedValue aProperty;
        aProperty.Name  = "nodepath";
        aProperty.Value <<= OUString("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob");

        uno::Sequence< uno::Any > aArgumentList( 1 );
        aArgumentList[0] <<= aProperty;

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
    catch( const uno::Exception& )
    {
         TOOLS_WARN_EXCEPTION("cui.options", "Caught exception, thread terminated");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
