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
#include <sfx2/filedlghelper.hxx>
#include <svl/zforlist.hxx>
#include "optupdt.hxx"
#include "optupdt.hrc"
#include <dialmgr.hxx>
#include <cuires.hrc>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <osl/file.hxx>
#include <osl/security.hxx>

namespace beans = ::com::sun::star::beans;
namespace container = ::com::sun::star::container;
namespace dialogs = ::com::sun::star::ui::dialogs;
namespace frame = ::com::sun::star::frame;
namespace lang = ::com::sun::star::lang;
namespace uno = ::com::sun::star::uno;
namespace util = ::com::sun::star::util;

// define ----------------------------------------------------------------

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

// class SvxOnlineUpdateTabPage --------------------------------------------------

SvxOnlineUpdateTabPage::SvxOnlineUpdateTabPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_ONLINEUPDATE ), rSet ),
        m_aOptionsLine( this, CUI_RES( FL_OPTIONS ) ),
        m_aAutoCheckCheckBox( this, CUI_RES( CB_AUTOCHECK ) ),
        m_aEveryDayButton( this, CUI_RES( RB_EVERYDAY ) ),
        m_aEveryWeekButton( this, CUI_RES( RB_EVERYWEEK ) ),
        m_aEveryMonthButton( this, CUI_RES( RB_EVERYMONTH ) ),
        m_aCheckNowButton( this, CUI_RES( PB_CHECKNOW ) ),
        m_aAutoDownloadCheckBox( this, CUI_RES( CB_AUTODOWNLOAD ) ),
        m_aDestPathLabel( this, CUI_RES( FT_DESTPATHLABEL ) ),
        m_aDestPath( this, CUI_RES( FT_DESTPATH ) ),
        m_aChangePathButton( this, CUI_RES( PB_CHANGEPATH ) ),
        m_aLastChecked( this, CUI_RES( FT_LASTCHECKED ) )
{
    m_aNeverChecked = String( CUI_RES( STR_NEVERCHECKED ) );
    FreeResource();

    m_aAutoCheckCheckBox.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, AutoCheckHdl_Impl ) );
    m_aCheckNowButton.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, CheckNowHdl_Impl ) );
    m_aChangePathButton.SetClickHdl( LINK( this, SvxOnlineUpdateTabPage, FileDialogHdl_Impl ) );

    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    m_xUpdateAccess = uno::Reference < container::XNameReplace >(
        xFactory->createInstance( UNISTRING( "com.sun.star.setup.UpdateCheckConfig" ) ),
        uno::UNO_QUERY_THROW );

    sal_Bool bDownloadSupported = sal_False;
    m_xUpdateAccess->getByName( UNISTRING( "DownloadSupported") ) >>= bDownloadSupported;

    WinBits nStyle = m_aDestPath.GetStyle();
    nStyle |= WB_PATHELLIPSIS;
    m_aDestPath.SetStyle(nStyle);

    m_aAutoDownloadCheckBox.Show(bDownloadSupported);
    m_aDestPathLabel.Show(bDownloadSupported);
    m_aDestPath.Show(bDownloadSupported);
    m_aChangePathButton.Show(bDownloadSupported);

    // dynamical length of the PushButtons
    CalcButtonWidth();

    m_aLastCheckedTemplate = m_aLastChecked.GetText();

    UpdateLastCheckedText();
}

// -----------------------------------------------------------------------

SvxOnlineUpdateTabPage::~SvxOnlineUpdateTabPage()
{
}

// -----------------------------------------------------------------------
void SvxOnlineUpdateTabPage::UpdateLastCheckedText()
{
    rtl::OUString aDateStr;
    rtl::OUString aTimeStr;
    rtl::OUString aText;
    sal_Int64 lastChecked = 0;

    m_xUpdateAccess->getByName( UNISTRING( "LastCheck") ) >>= lastChecked;

    if( lastChecked == 0 ) // never checked
    {
        aText = m_aNeverChecked;
    }
    else
    {
        TimeValue   lastCheckedTV;
        oslDateTime lastCheckedDT;

        Date  aDate( Date::EMPTY );
        Time  aTime( Time::EMPTY );

        lastCheckedTV.Seconds = (sal_uInt32) lastChecked;
        osl_getLocalTimeFromSystemTime( &lastCheckedTV, &lastCheckedTV );

        if ( osl_getDateTimeFromTimeValue(  &lastCheckedTV, &lastCheckedDT ) )
        {
            aDate = Date( lastCheckedDT.Day, lastCheckedDT.Month, lastCheckedDT.Year );
            aTime = Time( lastCheckedDT.Hours, lastCheckedDT.Minutes );
        }

        LanguageType eUILang = Application::GetSettings().GetUILanguage();
        SvNumberFormatter *pNumberFormatter = new SvNumberFormatter( ::comphelper::getProcessServiceFactory(), eUILang );
        String      aTmpStr;
        Color*      pColor = NULL;
        Date*       pNullDate = pNumberFormatter->GetNullDate();
        sal_uInt32  nFormat = pNumberFormatter->GetStandardFormat( NUMBERFORMAT_DATE, eUILang );

        pNumberFormatter->GetOutputString( aDate - *pNullDate, nFormat, aTmpStr, &pColor );
        aDateStr = aTmpStr;

        nFormat = pNumberFormatter->GetStandardFormat( NUMBERFORMAT_TIME, eUILang );
        pNumberFormatter->GetOutputString( aTime.GetTimeInDays(), nFormat, aTmpStr, &pColor );
        aTimeStr = aTmpStr;

        delete pColor;
        delete pNumberFormatter;

        aText = m_aLastCheckedTemplate;
        sal_Int32 nIndex = aText.indexOf( UNISTRING( "%DATE%" ) );
        if ( nIndex != -1 )
            aText = aText.replaceAt( nIndex, 6, aDateStr );

        nIndex = aText.indexOf( UNISTRING( "%TIME%" ) );
        if ( nIndex != -1 )
            aText = aText.replaceAt( nIndex, 6, aTimeStr );
    }

    m_aLastChecked.SetText( aText );
}

// -----------------------------------------------------------------------

SfxTabPage*
SvxOnlineUpdateTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxOnlineUpdateTabPage( pParent, rAttrSet );
}

// -----------------------------------------------------------------------


sal_Bool SvxOnlineUpdateTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bModified = sal_False;

    sal_Bool bValue;
    sal_Int64 nValue;

    if( m_aAutoCheckCheckBox.GetSavedValue() != m_aAutoCheckCheckBox.IsChecked() )
    {
        bValue = (sal_True == m_aAutoCheckCheckBox.IsChecked());
        m_xUpdateAccess->replaceByName( UNISTRING("AutoCheckEnabled"), uno::makeAny( bValue ) );
        bModified = sal_True;
    }

    nValue = 0;
    if( sal_True == m_aEveryDayButton.IsChecked() )
    {
        if( sal_False == m_aEveryDayButton.GetSavedValue() )
            nValue = 86400;
    }
    else if( sal_True == m_aEveryWeekButton.IsChecked() )
    {
        if( sal_False == m_aEveryWeekButton.GetSavedValue() )
            nValue = 604800;
    }
    else if( sal_True == m_aEveryMonthButton.IsChecked() )
    {
        if( sal_False == m_aEveryMonthButton.GetSavedValue() )
            nValue = 2592000;
    }

    if( nValue > 0 )
    {
        m_xUpdateAccess->replaceByName( UNISTRING("CheckInterval"), uno::makeAny( nValue ) );
        bModified = sal_True;
    }

    if( m_aAutoDownloadCheckBox.GetSavedValue() != m_aAutoDownloadCheckBox.IsChecked() )
    {
        bValue = (sal_True == m_aAutoDownloadCheckBox.IsChecked());
        m_xUpdateAccess->replaceByName( UNISTRING("AutoDownloadEnabled"), uno::makeAny( bValue ) );
        bModified = sal_True;
    }

    rtl::OUString sValue, aURL;
    m_xUpdateAccess->getByName( UNISTRING("DownloadDestination") ) >>= sValue;

    if( ( osl::FileBase::E_None == osl::FileBase::getFileURLFromSystemPath(m_aDestPath.GetText(), aURL) ) &&
        ( ! aURL.equals( sValue ) ) )
    {
        m_xUpdateAccess->replaceByName( UNISTRING("DownloadDestination"), uno::makeAny( aURL ) );
        bModified = sal_True;
    }

    uno::Reference< util::XChangesBatch > xChangesBatch(m_xUpdateAccess, uno::UNO_QUERY);
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
        xChangesBatch->commitChanges();

    return bModified;
}

// -----------------------------------------------------------------------

void SvxOnlineUpdateTabPage::Reset( const SfxItemSet& )
{
    sal_Bool bValue = sal_Bool();
    m_xUpdateAccess->getByName( UNISTRING("AutoCheckEnabled") ) >>= bValue;

    m_aAutoCheckCheckBox.Check(bValue);
    m_aEveryDayButton.Enable(bValue);
    m_aEveryWeekButton.Enable(bValue);
    m_aEveryMonthButton.Enable(bValue);

    sal_Int64 nValue = 0;
    m_xUpdateAccess->getByName( UNISTRING("CheckInterval") ) >>= nValue;

    if( nValue == 86400 )
        m_aEveryDayButton.Check();
    else if( nValue == 604800 )
        m_aEveryWeekButton.Check();
    else
        m_aEveryMonthButton.Check();

    m_aAutoCheckCheckBox.SaveValue();
    m_aEveryDayButton.SaveValue();
    m_aEveryWeekButton.SaveValue();
    m_aEveryMonthButton.SaveValue();

    m_xUpdateAccess->getByName( UNISTRING("AutoDownloadEnabled") ) >>= bValue;
    m_aAutoDownloadCheckBox.Check(bValue);
    m_aDestPathLabel.Enable(sal_True);
    m_aDestPath.Enable(sal_True);
    m_aChangePathButton.Enable(sal_True);

    rtl::OUString sValue, aPath;
    m_xUpdateAccess->getByName( UNISTRING("DownloadDestination") ) >>= sValue;

    if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(sValue, aPath) )
        m_aDestPath.SetText(aPath);

    m_aAutoDownloadCheckBox.SaveValue();
}

// -----------------------------------------------------------------------

void SvxOnlineUpdateTabPage::FillUserData()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxOnlineUpdateTabPage, AutoCheckHdl_Impl, CheckBox *, pBox )
{
    sal_Bool bEnabled = pBox->IsChecked();

    m_aEveryDayButton.Enable(bEnabled);
    m_aEveryWeekButton.Enable(bEnabled);
    m_aEveryMonthButton.Enable(bEnabled);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, FileDialogHdl_Impl)
{
    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    uno::Reference < dialogs::XFolderPicker > xFolderPicker(
        xFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( FOLDER_PICKER_SERVICE_NAME ) ) ),
        uno::UNO_QUERY );

    rtl::OUString aURL;
    if( osl::FileBase::E_None != osl::FileBase::getFileURLFromSystemPath(m_aDestPath.GetText(), aURL) )
        osl::Security().getHomeDir(aURL);

    xFolderPicker->setDisplayDirectory( aURL );
    sal_Int16 nRet = xFolderPicker->execute();

    if ( dialogs::ExecutableDialogResults::OK == nRet )
    {
        rtl::OUString aFolder;
        if( osl::FileBase::E_None == osl::FileBase::getSystemPathFromFileURL(xFolderPicker->getDirectory(), aFolder))
            m_aDestPath.SetText( aFolder );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxOnlineUpdateTabPage, CheckNowHdl_Impl)
{
    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
            com::sun::star::configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

        beans::NamedValue aProperty;
        aProperty.Name  = UNISTRING( "nodepath" );
        aProperty.Value = uno::makeAny( UNISTRING("org.openoffice.Office.Addons/AddonUI/OfficeHelp/UpdateCheckJob") );

        uno::Sequence< uno::Any > aArgumentList( 1 );
        aArgumentList[0] = uno::makeAny( aProperty );

        uno::Reference< container::XNameAccess > xNameAccess(
            xConfigProvider->createInstanceWithArguments(
                UNISTRING("com.sun.star.configuration.ConfigurationAccess"), aArgumentList ),
            uno::UNO_QUERY_THROW );

        util::URL aURL;
        xNameAccess->getByName(UNISTRING("URL")) >>= aURL.Complete;

        uno::Reference < util::XURLTransformer > xTransformer(
            util::URLTransformer::create( comphelper::getProcessComponentContext() ) );

        xTransformer->parseStrict(aURL);

        uno::Reference < frame::XDesktop > xDesktop(
            xFactory->createInstance(  UNISTRING( "com.sun.star.frame.Desktop" ) ),
            uno::UNO_QUERY_THROW );

        uno::Reference< frame::XDispatchProvider > xDispatchProvider(
            xDesktop->getCurrentFrame(), uno::UNO_QUERY );

        uno::Reference< frame::XDispatch > xDispatch = xDispatchProvider->queryDispatch(aURL, rtl::OUString(), 0);

        if( xDispatch.is() )
        {
            xDispatch->dispatch(aURL, uno::Sequence< beans::PropertyValue > ());
            UpdateLastCheckedText();
        }
    }
    catch( const uno::Exception& e )
    {
         OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    return 0;
}

void SvxOnlineUpdateTabPage::CalcButtonWidth()
{
    // detect the longest button text
    long nTxtWidth = ::std::max( m_aCheckNowButton.GetCtrlTextWidth( m_aCheckNowButton.GetText() ),
                                 m_aCheckNowButton.GetCtrlTextWidth( m_aChangePathButton.GetText() ) );
    // add a little offset
    nTxtWidth = nTxtWidth + 12;
    // compare with the button width
    Size aSize = m_aCheckNowButton.GetSizePixel();
    // and change it if it's necessary
    if ( nTxtWidth > aSize.Width() )
    {
        aSize.Width() = nTxtWidth;
        m_aCheckNowButton.SetSizePixel( aSize );
        m_aChangePathButton.SetSizePixel( aSize );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
