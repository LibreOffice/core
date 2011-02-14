/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#define _SVX_OPTIMPROVE_CXX

#include <optimprove.hxx>
#include <dialmgr.hxx>
#include <vcl/msgbox.hxx>

#include "optimprove.hrc"
#include "helpid.hrc"
#include <cuires.hrc>
#include <svx/dialogs.hrc>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/oooimprovement/XCore.hpp>
#include <com/sun/star/oooimprovement/XCoreController.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/synchronousdispatch.hxx>
#include <comphelper/uieventslogger.hxx>
#include <tools/testtoolloader.hxx>
#include <osl/file.hxx>

#define C2S(s)  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace beans  = ::com::sun::star::beans;
namespace lang  = ::com::sun::star::lang;
namespace uno   = ::com::sun::star::uno;
namespace util  = ::com::sun::star::util;
using namespace com::sun::star::system;


namespace
{
    bool lcl_doesLogfileExist(const ::rtl::OUString& sLogPath)
    {
        ::rtl::OUString sLogFile( sLogPath );
        sLogFile += C2S("/Current.csv");
        ::osl::File aLogFile(sLogFile);
        return aLogFile.open(osl_File_OpenFlag_Read) == ::osl::FileBase::E_None;
    }
}

// class SvxImprovementOptionsPage ---------------------------------------

SvxImprovementOptionsPage::SvxImprovementOptionsPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_IMPROVEMENT ), rSet ),

    m_aImproveFL                ( this, CUI_RES( FL_IMPROVE ) ),
    m_aInvitationFT             ( this, CUI_RES( FT_INVITATION ) ),
    m_aYesRB                    ( this, CUI_RES( RB_YES ) ),
    m_aNoRB                     ( this, CUI_RES( RB_NO ) ),
    m_aInfoFI                   ( this, CUI_RES( FI_INFO ) ),
    m_aDataFL                   ( this, CUI_RES( FL_DATA ) ),
    m_aNumberOfReportsFT        ( this, CUI_RES( FT_NR_REPORTS ) ),
    m_aNumberOfReportsValueFT   ( this, CUI_RES( FT_NR_REPORTS_VALUE ) ),
    m_aNumberOfActionsFT        ( this, CUI_RES( FT_NR_ACTIONS ) ),
    m_aNumberOfActionsValueFT   ( this, CUI_RES( FT_NR_ACTIONS_VALUE ) ),
    m_aShowDataPB               ( this, CUI_RES( PB_SHOWDATA ) ),

    m_sInfo                     (       CUI_RES( STR_INFO ) ),
    m_sMoreInfo                 (       CUI_RES( STR_MOREINFO ) )

{
    FreeResource();

    m_aInfoFI.SetURL( C2S( "www.sun.com/privacy/" ) );
    m_aInfoFI.SetClickHdl( LINK( this, SvxImprovementOptionsPage, HandleHyperlink ) );
    m_aShowDataPB.SetClickHdl( LINK( this, SvxImprovementOptionsPage, HandleShowData ) );
}

SvxImprovementOptionsPage::~SvxImprovementOptionsPage()
{
}

IMPL_LINK( SvxImprovementOptionsPage, HandleHyperlink, svt::FixedHyperlinkImage*, EMPTYARG )
{
    ::rtl::OUString sURL( m_aInfoFI.GetURL() );

    if ( sURL.getLength() > 0 )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xSMGR =
                ::comphelper::getProcessServiceFactory();
            uno::Reference< XSystemShellExecute > xSystemShell(
                xSMGR->createInstance( ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.system.SystemShellExecute" ) ) ),
                uno::UNO_QUERY_THROW );
            if ( xSystemShell.is() )
            {
                xSystemShell->execute(
                    sURL, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
            }
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }

    return 0;
}

IMPL_LINK( SvxImprovementOptionsPage, HandleShowData, PushButton*, EMPTYARG )
{
    uno::Reference < uno::XInterface > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
        C2S("com.sun.star.frame.Desktop") ), uno::UNO_QUERY );
    if ( xDesktop.is() )
    {
        ::rtl::OUString sLogFile( m_sLogPath );
        sLogFile += C2S("/Current.csv");
        uno::Sequence< beans::PropertyValue > aArgs(3);
        aArgs[0].Name = ::rtl::OUString::createFromAscii("FilterName");
        aArgs[0].Value = uno::makeAny(::rtl::OUString::createFromAscii("Text - txt - csv (StarCalc)"));
        aArgs[1].Name = ::rtl::OUString::createFromAscii("FilterOptions");
        aArgs[1].Value = uno::makeAny(::rtl::OUString::createFromAscii("44,34,12,1,"));
        aArgs[2].Name = ::rtl::OUString::createFromAscii("ReadOnly");
        aArgs[2].Value = uno::makeAny(true);

        uno::Reference< lang::XComponent > xDoc = ::comphelper::SynchronousDispatch::dispatch(
            xDesktop, sLogFile, C2S("_default"), 0, aArgs );
        if ( xDoc.is() )
        {
            dynamic_cast<Dialog*>(GetParent())->EndDialog( RET_CANCEL );
            return 1;
        }
    }

    return 0;
}

SfxTabPage* SvxImprovementOptionsPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxImprovementOptionsPage( pParent, rSet );
}

sal_Bool SvxImprovementOptionsPage::FillItemSet( SfxItemSet& /*rSet*/ )
{
    uno::Reference< lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    uno::Reference< uno::XInterface > xConfig;

    try
    {
        xConfig = ::comphelper::ConfigurationHelper::openConfig(
            xSMGR, C2S("/org.openoffice.Office.OOoImprovement.Settings"),
            ::comphelper::ConfigurationHelper::E_STANDARD );
        ::comphelper::ConfigurationHelper::writeRelativeKey(
            xConfig, C2S("Participation"), C2S("ShowedInvitation"), uno::makeAny( true ) );
        ::comphelper::ConfigurationHelper::writeRelativeKey(
            xConfig, C2S("Participation"), C2S("InvitationAccepted"), uno::makeAny( m_aYesRB.IsChecked() != sal_False ) );
        ::comphelper::ConfigurationHelper::flush( xConfig );
        // TODO: refactor
        ::comphelper::UiEventsLogger::reinit();
        ::tools::InitTestToolLib();
    }
    catch( uno::Exception& )
    {
    }

    return sal_False;
}

void SvxImprovementOptionsPage::Reset( const SfxItemSet& /*rSet*/ )
{
    uno::Reference< lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    uno::Reference< com::sun::star::oooimprovement::XCore > xCore(
        xSMGR->createInstance( C2S("com.sun.star.oooimprovement.Core") ),
        uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xConfig;

    try
    {
        m_aNoRB.Check();
        xConfig = ::comphelper::ConfigurationHelper::openConfig(
            xSMGR, C2S("/org.openoffice.Office.OOoImprovement.Settings"),
            ::comphelper::ConfigurationHelper::E_READONLY );
        if ( xConfig.is() )
        {
            bool bYesChecked = false;
            uno::Any aAny = ::comphelper::ConfigurationHelper::
                readRelativeKey( xConfig, C2S("Participation"), C2S("ShowedInvitation") );
            if ( ( aAny >>= bYesChecked ) && bYesChecked )
            {
                bool bTemp = false;
                aAny = ::comphelper::ConfigurationHelper::
                    readRelativeKey( xConfig, C2S("Participation"), C2S("InvitationAccepted") );
                if ( aAny >>= bTemp )
                {
                    bYesChecked &= bTemp;
                    if ( bYesChecked )
                        m_aYesRB.Check();
                }
            }

            ::rtl::OUString sURL;
            aAny = ::comphelper::ConfigurationHelper::
                readRelativeKey( xConfig, C2S("Participation"), C2S("HelpUrl") );
            if ( aAny >>= sURL )
                m_aInfoFI.SetURL( sURL );

            sal_Int32 nCount = 0;
            aAny = ::comphelper::ConfigurationHelper::
                readRelativeKey( xConfig, C2S("Counters"), C2S("UploadedReports") );
            if ( aAny >>= nCount )
                m_aNumberOfReportsValueFT.SetText( String::CreateFromInt32( nCount ) );
            aAny = ::comphelper::ConfigurationHelper::
                readRelativeKey( xConfig, C2S("Counters"), C2S("LoggedEvents") );
            if ( aAny >>= nCount )
            {
                if ( xCore.is() )
                    nCount += xCore->getSessionLogEventCount();
                m_aNumberOfActionsValueFT.SetText( String::CreateFromInt32( nCount ) );
            }

            ::rtl::OUString sPath;
            aAny = ::comphelper::ConfigurationHelper::readDirectKey(
                xSMGR, C2S("/org.openoffice.Office.Logging"), C2S("OOoImprovement"),
                C2S("LogPath"), ::comphelper::ConfigurationHelper::E_READONLY );
            if ( aAny >>= sPath )
            {
                uno::Reference< util::XStringSubstitution > xSubst(
                    xSMGR->createInstance( C2S("com.sun.star.util.PathSubstitution") ),
                    uno::UNO_QUERY );
                if ( xSubst.is() )
                    sPath = xSubst->substituteVariables( sPath, sal_False );
                m_sLogPath = sPath;
                m_aShowDataPB.Enable(lcl_doesLogfileExist(m_sLogPath));
            }
        }
    }
    catch( uno::Exception& )
    {
        m_aShowDataPB.Enable(false);
    }
}

