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
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include "optupdt.hxx"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#include <com/sun/star/ucb/XWebDAVCommandEnvironment.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/setup/UpdateCheck.hpp>
#include <com/sun/star/setup/UpdateCheckConfig.hpp>
#include <com/sun/star/configuration/ReadWriteAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <sfx2/filedlghelper.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Update.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/configmgr.hxx>

using namespace ::css;

SvxOnlineUpdateTabPage::SvxOnlineUpdateTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optonlineupdatepage.ui"_ustr, u"OptOnlineUpdatePage"_ustr, &rSet)
    , m_showTraditionalOnlineUpdate(isTraditionalOnlineUpdateAvailable())
    , m_showMarOnlineUpdate(isMarOnlineUpdateAvailable())
    , m_xNeverChecked(m_xBuilder->weld_label(u"neverchecked"_ustr))
    , m_xAutoCheckCheckBox(m_xBuilder->weld_check_button(u"autocheck"_ustr))
    , m_xAutoCheckImg(m_xBuilder->weld_widget(u"lockautocheck"_ustr))
    , m_xEveryDayButton(m_xBuilder->weld_radio_button(u"everyday"_ustr))
    , m_xEveryWeekButton(m_xBuilder->weld_radio_button(u"everyweek"_ustr))
    , m_xEveryMonthButton(m_xBuilder->weld_radio_button(u"everymonth"_ustr))
    , m_xCheckIntervalImg(m_xBuilder->weld_widget(u"lockcheckinterval"_ustr))
    , m_xCheckNowButton(m_xBuilder->weld_button(u"checknow"_ustr))
    , m_xAutoDownloadCheckBox(m_xBuilder->weld_check_button(u"autodownload"_ustr))
    , m_xAutoDownloadImg(m_xBuilder->weld_widget(u"lockautodownload"_ustr))
    , m_xDestPathLabel(m_xBuilder->weld_label(u"destpathlabel"_ustr))
    , m_xDestPath(m_xBuilder->weld_label(u"destpath"_ustr))
    , m_xChangePathButton(m_xBuilder->weld_button(u"changepath"_ustr))
    , m_xLastChecked(m_xBuilder->weld_label(u"lastchecked"_ustr))
    , m_xExtrasCheckBox(m_xBuilder->weld_check_button(u"extrabits"_ustr))
    , m_xExtrasImg(m_xBuilder->weld_widget(u"lockextrabits"_ustr))
    , m_xUserAgentLabel(m_xBuilder->weld_label(u"useragent"_ustr))
    , m_xPrivacyPolicyButton(m_xBuilder->weld_link_button(u"btnPrivacyPolicy"_ustr))
    , m_xBox2(m_xBuilder->weld_box(u"box2"_ustr))
    , m_xFrameDest(m_xBuilder->weld_frame(u"frameDest"_ustr))
    , m_xFrameAgent(m_xBuilder->weld_frame(u"frameAgent"_ustr))
    , m_xMar(m_xBuilder->weld_frame(u"frameMar"_ustr))
    , m_xEnableMar(m_xBuilder->weld_check_button(u"enableMar"_ustr))
{
    if (m_showTraditionalOnlineUpdate) {
        m_aNeverChecked = m_xNeverChecked->get_label();

        m_xAutoCheckCheckBox->connect_toggled( LINK( this, SvxOnlineUpdateTabPage, AutoCheckHdl_Impl ) );
        m_xExtrasCheckBox->connect_toggled( LINK( this, SvxOnlineUpdateTabPage, ExtrasCheckHdl_Impl ) );
        m_xCheckNowButton->connect_clicked( LINK( this, SvxOnlineUpdateTabPage, CheckNowHdl_Impl ) );
        m_xChangePathButton->connect_clicked( LINK( this, SvxOnlineUpdateTabPage, FileDialogHdl_Impl ) );
        m_xPrivacyPolicyButton->set_uri(
            officecfg::Office::Common::Menus::PrivacyPolicyURL::get()
            + "?type=updatecheck&LOvers=" + utl::ConfigManager::getProductVersion()
            + "&LOlocale=" + LanguageTag(utl::ConfigManager::getUILocale()).getBcp47());


        uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

        m_xUpdateAccess = setup::UpdateCheckConfig::create( xContext );
        m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(xContext, u"*"_ustr);

        bool bDownloadSupported = false;
        m_xUpdateAccess->getByName( u"DownloadSupported"_ustr ) >>= bDownloadSupported;

        m_xAutoDownloadCheckBox->set_visible(bDownloadSupported);
        m_xDestPathLabel->set_visible(bDownloadSupported);
        m_xDestPath->set_visible(bDownloadSupported);
        m_xChangePathButton->set_visible(bDownloadSupported);

        m_aLastCheckedTemplate = m_xLastChecked->get_label();

        UpdateLastCheckedText();
        UpdateUserAgent();
    } else {
        m_xAutoCheckCheckBox->hide();
        m_xEveryDayButton->hide();
        m_xEveryWeekButton->hide();
        m_xEveryMonthButton->hide();
        m_xCheckNowButton->hide();
        m_xBox2->hide();
        m_xAutoCheckImg->hide();
        m_xCheckIntervalImg->hide();
        m_xFrameDest->hide();
        m_xFrameAgent->hide();
        m_xPrivacyPolicyButton->hide();
    }

    if (m_showMarOnlineUpdate) {
        m_xMar->show();
        m_xEnableMar->set_sensitive(!officecfg::Office::Update::Update::Enabled::isReadOnly());
    } else {
        m_xMar->hide();
    }
}

SvxOnlineUpdateTabPage::~SvxOnlineUpdateTabPage()
{
}

void SvxOnlineUpdateTabPage::UpdateLastCheckedText()
{
    OUString aText;
    sal_Int64 lastChecked = 0;

    m_xUpdateAccess->getByName(u"LastCheck"_ustr) >>= lastChecked;

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
        std::optional<SvNumberFormatter> pNumberFormatter( std::in_place, ::comphelper::getProcessComponentContext(), eUILang );
        const Color*      pColor = nullptr;
        const Date& rNullDate = pNumberFormatter->GetNullDate();
        sal_uInt32  nFormat = pNumberFormatter->GetStandardFormat( SvNumFormatType::DATE, eUILang );

        OUString aDateStr;
        pNumberFormatter->GetOutputString( aDate - rNullDate, nFormat, aDateStr, &pColor );

        nFormat = pNumberFormatter->GetStandardFormat( SvNumFormatType::TIME, eUILang );
        OUString aTimeStr;
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

        OUString aPseudoURL = u"useragent:normal"_ustr;
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

OUString SvxOnlineUpdateTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr,        u"lastchecked"_ustr, u"neverchecked"_ustr,    u"labeldest"_ustr,
                          u"destpathlabel"_ustr, u"labelagent"_ustr,  u"useragent_label"_ustr, u"useragent_changed"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"autocheck"_ustr, u"autodownload"_ustr, u"extrabits"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"everyday"_ustr, u"everyweek"_ustr, u"everymonth"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    // some buttons are not included
    sAllStrings += m_xPrivacyPolicyButton->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SvxOnlineUpdateTabPage::FillItemSet( SfxItemSet* )
{
    bool bModified = false;

    if (m_showTraditionalOnlineUpdate) {
        bool bValue;
        sal_Int64 nValue;

        if( m_xAutoCheckCheckBox->get_state_changed_from_saved() )
        {
            bValue = m_xAutoCheckCheckBox->get_active();
            m_xUpdateAccess->replaceByName( u"AutoCheckEnabled"_ustr, uno::Any( bValue ) );
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
            m_xUpdateAccess->replaceByName( u"CheckInterval"_ustr, uno::Any( nValue ) );
            bModified = true;
        }

        if( m_xAutoDownloadCheckBox->get_state_changed_from_saved() )
        {
            bValue = m_xAutoDownloadCheckBox->get_active();
            m_xUpdateAccess->replaceByName( u"AutoDownloadEnabled"_ustr, uno::Any( bValue ) );
            bModified = true;
        }

        OUString sValue, aURL;
        m_xUpdateAccess->getByName( u"DownloadDestination"_ustr ) >>= sValue;

        if( ( osl::FileBase::E_None == osl::FileBase::getFileURLFromSystemPath(m_xDestPath->get_label(), aURL) ) &&
            ( aURL != sValue ) )
        {
            m_xUpdateAccess->replaceByName( u"DownloadDestination"_ustr, uno::Any( aURL ) );
            bModified = true;
        }

        if( m_xExtrasCheckBox->get_state_changed_from_saved() )
        {
            bValue = m_xExtrasCheckBox->get_active();
            m_xUpdateAccess->replaceByName( u"ExtendedUserAgent"_ustr, uno::Any( bValue ) );
            bModified = true;
        }

        uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
        if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
            xChangesBatch->commitChanges();
    }

    if (m_showMarOnlineUpdate && m_xEnableMar->get_state_changed_from_saved()) {
        auto batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Update::Update::Enabled::set(m_xEnableMar->get_active(), batch);
        batch->commit();
        bModified = true;
    }

    return bModified;
}

void SvxOnlineUpdateTabPage::Reset( const SfxItemSet* )
{
    if (m_showTraditionalOnlineUpdate) {
        bool bValue = false;
        m_xUpdateAccess->getByName( u"AutoCheckEnabled"_ustr ) >>= bValue;
        beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(u"/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/AutoCheckEnabled"_ustr);
        bool bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;

        m_xAutoCheckCheckBox->set_active(bValue);
        m_xAutoCheckCheckBox->set_sensitive(!bReadOnly);
        m_xAutoCheckImg->set_visible(bReadOnly);

        sal_Int64 nValue = 0;
        m_xUpdateAccess->getByName( u"CheckInterval"_ustr ) >>= nValue;
        aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(u"/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/CheckInterval"_ustr);
        bool bReadOnly2 = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
        m_xEveryDayButton->set_sensitive(bValue && !(bReadOnly || bReadOnly2));
        m_xEveryWeekButton->set_sensitive(bValue && !(bReadOnly || bReadOnly2));
        m_xEveryMonthButton->set_sensitive(bValue && !(bReadOnly || bReadOnly2));
        m_xCheckIntervalImg->set_visible(bReadOnly2);

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

        m_xUpdateAccess->getByName( u"AutoDownloadEnabled"_ustr ) >>= bValue;
        aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(u"/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/AutoDownloadEnabled"_ustr);
        bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
        m_xAutoDownloadCheckBox->set_active(bValue);
        m_xAutoDownloadCheckBox->set_sensitive(!bReadOnly);
        m_xAutoDownloadImg->set_visible(bReadOnly);
        m_xDestPathLabel->set_sensitive(true);
        m_xDestPath->set_sensitive(true);

        OUString sValue, aPath;
        m_xUpdateAccess->getByName( u"DownloadDestination"_ustr ) >>= sValue;
        aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(u"/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/DownloadDestination"_ustr);
        bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
        m_xChangePathButton->set_sensitive(!bReadOnly);

        if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sValue, aPath) )
            m_xDestPath->set_label(aPath);

        m_xUpdateAccess->getByName( u"ExtendedUserAgent"_ustr ) >>= bValue;
        aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(u"/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/ExtendedUserAgent"_ustr);
        bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
        m_xExtrasCheckBox->set_active(bValue);
        m_xExtrasCheckBox->set_sensitive(!bReadOnly);
        m_xExtrasImg->set_visible(bReadOnly);
        m_xExtrasCheckBox->save_state();
        UpdateUserAgent();

        m_xAutoDownloadCheckBox->save_state();
    }

    if (m_showMarOnlineUpdate) {
        m_xEnableMar->set_active(officecfg::Office::Update::Update::Enabled::get());
        m_xEnableMar->save_state();
    }
}

void SvxOnlineUpdateTabPage::FillUserData()
{
}

IMPL_LINK(SvxOnlineUpdateTabPage, AutoCheckHdl_Impl, weld::Toggleable&, rBox, void)
{
    bool bEnabled = rBox.get_active();
    beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(u"/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments/CheckInterval"_ustr);
    bool bReadOnly = (aProperty.Attributes & beans::PropertyAttribute::READONLY) != 0;
    m_xEveryDayButton->set_sensitive(bEnabled && !bReadOnly);
    m_xEveryWeekButton->set_sensitive(bEnabled && !bReadOnly);
    m_xEveryMonthButton->set_sensitive(bEnabled && !bReadOnly);
    m_xCheckIntervalImg->set_visible(bReadOnly);
}

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, ExtrasCheckHdl_Impl, weld::Toggleable&, void)
{
    UpdateUserAgent();
}

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, FileDialogHdl_Impl, weld::Button&, void)
{
    uno::Reference < uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference < ui::dialogs::XFolderPicker2 >  xFolderPicker = sfx2::createFolderPicker(xContext, GetFrameWeld());

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
        aProperty.Value <<= u"org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob"_ustr;

        uno::Sequence< uno::Any > aArgumentList{ uno::Any(aProperty) };

        uno::Reference< container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                u"com.sun.star.configuration.ConfigurationAccess"_ustr, aArgumentList ),
            uno::UNO_QUERY_THROW );

        util::URL aURL;
        xNameAccess->getByName(u"URL"_ustr) >>= aURL.Complete;

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

bool SvxOnlineUpdateTabPage::isTraditionalOnlineUpdateAvailable() {
    try
    {
        css::uno::Reference < css::uno::XInterface > xService( setup::UpdateCheck::create( ::comphelper::getProcessComponentContext() ) );
        if( xService.is() )
            return true;
    }
    catch ( css::uno::DeploymentException& )
    {
    }
    return false;
}

bool SvxOnlineUpdateTabPage::isMarOnlineUpdateAvailable() {
#if HAVE_FEATURE_UPDATE_MAR
    return true;
#else
    return false;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
