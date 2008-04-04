/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.217 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 11:04:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <memory>
#include <unistd.h>
#include "app.hxx"
#include "desktop.hrc"
#include "appinit.hxx"
#include "intro.hxx"
#include "officeipcthread.hxx"
#include "cmdlineargs.hxx"
#include "desktopresid.hxx"
#include "dispatchwatcher.hxx"
#include "configinit.hxx"
#include "lockfile.hxx"
#include "checkinstall.hxx"
#include "cmdlinehelp.hxx"
#include "userinstall.hxx"
#include "desktopcontext.hxx"
#include "exithelper.hxx"
#include "../migration/pages.hxx"

#include <svtools/javacontext.hxx>

#ifndef _COM_SUN_STAR_FRAME_XSESSIONMANAGERLISTENER_HPP_
#include <com/sun/star/frame/XSessionManagerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSYNCHRONOUSDISPATCH_HPP_
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_CORRUPTEDFILTERCONFIGURATIONEXCEPTION_HPP_
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_CORRUPTEDCONFIGURATIONEXCEPTION_HPP_
#include <com/sun/star/configuration/CorruptedConfigurationException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_MISSINGBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_INSTALLATIONINCOMPLETEEXCEPTION_HPP_
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDSETUPEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XUIELEMENTFACTORYREGISTRATION_HPP_
#include <com/sun/star/ui/XUIElementFactoryRegistration.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XUICONTROLLERREGISTRATION_HPP_
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#endif

#include <com/sun/star/java/XJavaVM.hpp>

#ifndef _TOOLS_TESTTOOLLOADER_HXX_
#include <tools/testtoolloader.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif
#ifndef _UTL__HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_SIGNAL_H_
#include <osl/signal.h>
#endif
#include <rtl/uuid.h>

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#include <svtools/syslocaleoptions.hxx>
#endif
#ifndef SVTOOLS_FOLDER_RESTRICTION_HXX
#include <svtools/folderrestriction.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif

#include <svtools/fontsubstconfig.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <svtools/misccfg.hxx>
#include <svtools/filter.hxx>

#include "langselect.hxx"

#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))
#define U2S(STRING)                                ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

using namespace vos;
using namespace rtl;

//Gives an ICE with MSVC6
//namespace css = ::com::sun::star;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::container;

namespace css = ::com::sun::star;

ResMgr*                 desktop::Desktop::pResMgr = 0;

namespace desktop
{

static SalMainPipeExchangeSignalHandler* pSignalHandler = 0;
static sal_Bool _bCrashReporterEnabled = sal_True;

static const ::rtl::OUString CFG_PACKAGE_COMMON_HELP   ( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Office.Common/Help"));
static const ::rtl::OUString CFG_PATH_REG              ( RTL_CONSTASCII_USTRINGPARAM( "Registration"                     ));
static const ::rtl::OUString CFG_ENTRY_INSTANCEUUID    ( RTL_CONSTASCII_USTRINGPARAM( "InstanceUUID"                     ));
static const ::rtl::OUString CFG_ENTRY_PRODVERSIONID   ( RTL_CONSTASCII_USTRINGPARAM( "ProductVersionID"                 ));
static const ::rtl::OUString CFG_ENTRY_REGURL          ( RTL_CONSTASCII_USTRINGPARAM( "URL"                              ));
static const ::rtl::OUString CFG_ENTRY_TEMPLATEREGURL  ( RTL_CONSTASCII_USTRINGPARAM( "TemplateURL"                      ));
static const ::rtl::OUString PLACEHOLDER_INSTANCE_URN  ( RTL_CONSTASCII_USTRINGPARAM( "${INSTANCE_URN}"                  ));
static const ::rtl::OUString CONFIGPATH_PRODUCTVERSIONS( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Office.Common/Help/Registration/ProductVersions"));

// ----------------------------------------------------------------------------

ResMgr* Desktop::GetDesktopResManager()
{
    if ( !Desktop::pResMgr )
    {
        String aMgrName = String::CreateFromAscii( "dkt" );

        // Create desktop resource manager and bootstrap process
        // was successful. Use default way to get language specific message.
        if ( Application::IsInExecute() )
            Desktop::pResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));

        if ( !Desktop::pResMgr )
        {
            // Use VCL to get the correct language specific message as we
            // are in the bootstrap process and not able to get the installed
            // language!!
/*
            LanguageType aLanguageType = LANGUAGE_DONTKNOW;

            Desktop::pResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLanguageType );
            AllSettings as = GetSettings();
            as.SetUILanguage(aLanguageType);
            SetSettings(as);
*/
            // LanguageSelection langselect;
            OUString aUILocaleString = LanguageSelection::getLanguageString();
            sal_Int32 nIndex = 0;
            OUString aLanguage = aUILocaleString.getToken( 0, '-', nIndex);
            OUString aCountry = aUILocaleString.getToken( 0, '-', nIndex);
            OUString aVariant = aUILocaleString.getToken( 0, '-', nIndex);

            ::com::sun::star::lang::Locale aLocale( aLanguage, aCountry, aVariant );

            Desktop::pResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLocale);
            AllSettings as = GetSettings();
            as.SetUILocale(aLocale);
            SetSettings(as);
        }
    }

    return Desktop::pResMgr;
}

// ----------------------------------------------------------------------------
// Get a message string securely. There is a fallback string if the resource
// is not available.

OUString Desktop::GetMsgString( USHORT nId, const OUString& aFaultBackMsg )
{
    ResMgr* resMgr = GetDesktopResManager();
    if ( !resMgr )
        return aFaultBackMsg;
    else
        return OUString( String( ResId( nId, *resMgr )));
}

OUString MakeStartupErrorMessage(OUString const & aErrorMessage)
{
    OUStringBuffer    aDiagnosticMessage( 100 );

    ResMgr* pResMgr = Desktop::GetDesktopResManager();
    if ( pResMgr )
        aDiagnosticMessage.append( OUString(String(ResId(STR_BOOTSTRAP_ERR_CANNOT_START, *pResMgr))) );
    else
        aDiagnosticMessage.appendAscii( "The program cannot be started." );

    aDiagnosticMessage.appendAscii( "\n" );

    aDiagnosticMessage.append( aErrorMessage );

    return aDiagnosticMessage.makeStringAndClear();
}

OUString MakeStartupConfigAccessErrorMessage( OUString const & aInternalErrMsg )
{
    OUStringBuffer aDiagnosticMessage( 200 );

    ResMgr* pResMgr = Desktop::GetDesktopResManager();
    if ( pResMgr )
        aDiagnosticMessage.append( OUString(String(ResId(STR_BOOTSTRAP_ERR_CFG_DATAACCESS, *pResMgr ))) );
    else
        aDiagnosticMessage.appendAscii( "The program cannot be started." );

    if ( aInternalErrMsg.getLength() > 0 )
    {
        aDiagnosticMessage.appendAscii( "\n\n" );
        if ( pResMgr )
            aDiagnosticMessage.append( OUString(String(ResId(STR_INTERNAL_ERRMSG, *pResMgr ))) );
        else
            aDiagnosticMessage.appendAscii( "The following internal error has occured:\n\n" );
        aDiagnosticMessage.append( aInternalErrMsg );
    }

    return aDiagnosticMessage.makeStringAndClear();
}

void FatalError(OUString const & aMessage)
{
    OUString aProductKey = ::utl::Bootstrap::getProductKey();

    if (!aProductKey.getLength())
    {
        ::vos::OStartupInfo aInfo;
        aInfo.getExecutableFile( aProductKey );

        sal_uInt32     lastIndex = aProductKey.lastIndexOf('/');
        if ( lastIndex > 0 )
            aProductKey = aProductKey.copy( lastIndex+1 );
    }

    ErrorBox aBootstrapFailedBox( NULL, WB_OK, aMessage );
    aBootstrapFailedBox.SetText( aProductKey );
    aBootstrapFailedBox.Execute();
}

void FatalErrorExit(OUString const & aMessage)
{
    FatalError(aMessage);
    _exit( ExitHelper::E_FATAL_ERROR );
}

static bool ShouldSuppressUI(CommandLineArgs* pCmdLine)
{
    return  pCmdLine->IsInvisible() ||
            pCmdLine->IsHeadless() ||
            pCmdLine->IsQuickstart();
}

CommandLineArgs* Desktop::GetCommandLineArgs()
{
    static CommandLineArgs* pArgs = 0;
    if ( !pArgs )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pArgs )
    {
            ::vos::OExtCommandLine aArgs;
            pArgs = new CommandLineArgs( aArgs );
    }
    }

    return pArgs;
}

sal_Bool InitConfiguration()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (jb99855) ::InitConfiguration" );

    Reference< XMultiServiceFactory > xProvider( CreateApplicationConfigurationProvider( ) );
    return xProvider.is();
}

namespace
{
    struct BrandName
        : public rtl::Static< String, BrandName > {};
    struct Version
        : public rtl::Static< String, Version > {};
    struct AboutBoxVersion
        : public rtl::Static< String, AboutBoxVersion > {};
    struct Extension
        : public rtl::Static< String, Extension > {};
    struct XMLFileFormatName
        : public rtl::Static< String, XMLFileFormatName > {};
    struct XMLFileFormatVersion
        : public rtl::Static< String, XMLFileFormatVersion > {};
    struct WriterCompatibilityVersionOOo11
        : public rtl::Static< String, WriterCompatibilityVersionOOo11 > {};
}

void ReplaceStringHookProc( UniString& rStr )
{
    static int nAll = 0, nPro = 0;

    nAll++;
    if ( rStr.SearchAscii( "%PRODUCT" ) != STRING_NOTFOUND )
    {
        String &rBrandName = BrandName::get();
        String &rVersion = Version::get();
        String &rAboutBoxVersion = AboutBoxVersion::get();
        String &rExtension = Extension::get();
        String &rXMLFileFormatName = XMLFileFormatName::get();
        String &rXMLFileFormatVersion = XMLFileFormatVersion::get();

        if ( !rBrandName.Len() )
        {
            rtl::OUString aTmp;
            Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
            aRet >>= aTmp;
            rBrandName = aTmp;

            aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTXMLFILEFORMATNAME );
            aRet >>= aTmp;
            rXMLFileFormatName = aTmp;

            aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTXMLFILEFORMATVERSION );
            aRet >>= aTmp;
            rXMLFileFormatVersion = aTmp;

            aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTVERSION );
            aRet >>= aTmp;
            rVersion = aTmp;

            aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::ABOUTBOXPRODUCTVERSION );
            aRet >>= aTmp;
            rAboutBoxVersion = aTmp;

            if ( !rExtension.Len() )
            {
                aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTEXTENSION );
                aRet >>= aTmp;
                rExtension = aTmp;
            }
        }

        nPro++;
        rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", rBrandName );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTVERSION", rVersion );
        rStr.SearchAndReplaceAllAscii( "%ABOUTBOXPRODUCTVERSION", rAboutBoxVersion );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTEXTENSION", rExtension );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTXMLFILEFORMATNAME", rXMLFileFormatName );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTXMLFILEFORMATVERSION", rXMLFileFormatVersion );
    }

    if ( rStr.SearchAscii( "%WRITERCOMPATIBILITYVERSIONOOO11" ) != STRING_NOTFOUND )
    {
        String &rWriterCompatibilityVersionOOo11 = WriterCompatibilityVersionOOo11::get();
        if ( !rWriterCompatibilityVersionOOo11.Len() )
        {
            rtl::OUString aTmp;
            Any aRet = ::utl::ConfigManager::GetDirectConfigProperty(
                    ::utl::ConfigManager::WRITERCOMPATIBILITYVERSIONOOO11 );
            aRet >>= aTmp;
            rWriterCompatibilityVersionOOo11 = aTmp;
        }

        rStr.SearchAndReplaceAllAscii( "%WRITERCOMPATIBILITYVERSIONOOO11",
                                        rWriterCompatibilityVersionOOo11 );
    }
}

Desktop::Desktop()
: m_bServicesRegistered( false )
, m_pIntro( 0 )
, m_aBootstrapError( BE_OK )
, m_pLockfile( NULL )
{
    RTL_LOGFILE_TRACE( "desktop (cd100003) ::Desktop::Desktop" );
}

Desktop::~Desktop()
{
}

void Desktop::Init()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::Init" );
    SetBootstrapStatus(BS_OK);

    // create service factory...
    Reference < XMultiServiceFactory > rSMgr = CreateApplicationServiceManager();
    if( rSMgr.is() )
    {
        ::comphelper::setProcessServiceFactory( rSMgr );
    }
    else
    {
        SetBootstrapError( BE_UNO_SERVICEMANAGER );
    }

    if ( GetBootstrapError() == BE_OK )
    {
        // prepare language
        if ( !LanguageSelection::prepareLanguage() )
            SetBootstrapError( BE_LANGUAGE_MISSING );
    }

    if ( GetBootstrapError() == BE_OK )
    {
        CommandLineArgs* pCmdLineArgs = GetCommandLineArgs();
#ifdef UNX
    //  check whether we need to print cmdline help
    if ( pCmdLineArgs->IsHelp() ) {
        displayCmdlineHelp();
        SetBootstrapStatus(BS_TERMINATE);
    }
#endif
        // start ipc thread only for non-remote offices
        RTL_LOGFILE_CONTEXT( aLog2, "desktop (cd100003) ::OfficeIPCThread::EnableOfficeIPCThread" );
        OfficeIPCThread::Status aStatus = OfficeIPCThread::EnableOfficeIPCThread();
        if ( aStatus == OfficeIPCThread::IPC_STATUS_BOOTSTRAP_ERROR )
        {
            SetBootstrapError( BE_PATHINFO_MISSING );
        }
        else if ( aStatus == OfficeIPCThread::IPC_STATUS_2ND_OFFICE )
        {
            // 2nd office startup should terminate after sending cmdlineargs through pipe
            SetBootstrapStatus(BS_TERMINATE);
        }
        else if ( pCmdLineArgs->IsHelp() )
        {
            // disable IPC thread in an instance that is just showing a help message
            OfficeIPCThread::DisableOfficeIPCThread();
        }
        pSignalHandler = new SalMainPipeExchangeSignalHandler;
    }
}

void Desktop::DeInit()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::DeInit" );

    try {
        // instead of removing of the configManager just let it commit all the changes
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- store config items" );
        utl::ConfigManager::GetConfigManager()->StoreConfigItems();
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- store config items" );

        // close splashscreen if it's still open
        CloseSplashScreen();
        Reference<XMultiServiceFactory> xXMultiServiceFactory(::comphelper::getProcessServiceFactory());
        DestroyApplicationServiceManager( xXMultiServiceFactory );
        // nobody should get a destroyd service factory...
        ::comphelper::setProcessServiceFactory( NULL );

        // clear lockfile
        if (m_pLockfile != NULL)
            m_pLockfile->clean();

        OfficeIPCThread::DisableOfficeIPCThread();
        if( pSignalHandler )
            DELETEZ( pSignalHandler );
    } catch (RuntimeException&) {
        // someone threw an exception during shutdown
        // this will leave some garbage behind..
    }

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "FINISHED WITH Destop::DeInit" );
}

BOOL Desktop::QueryExit()
{
    const sal_Char SUSPEND_QUICKSTARTVETO[] = "SuspendQuickstartVeto";

    Reference< ::com::sun::star::frame::XDesktop >
            xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                UNO_QUERY );

    Reference < ::com::sun::star::beans::XPropertySet > xPropertySet( xDesktop, UNO_QUERY );
    if ( xPropertySet.is() )
    {
        Any a;
        a <<= (sal_Bool)sal_True;
        xPropertySet->setPropertyValue( OUSTRING(RTL_CONSTASCII_USTRINGPARAM( SUSPEND_QUICKSTARTVETO )), a );
    }

    BOOL bExit = ( !xDesktop.is() || xDesktop->terminate() );


    if ( !bExit && xPropertySet.is() )
    {
        Any a;
        a <<= (sal_Bool)sal_False;
        xPropertySet->setPropertyValue( OUSTRING(RTL_CONSTASCII_USTRINGPARAM( SUSPEND_QUICKSTARTVETO )), a );
    } else {
        if (m_pLockfile != NULL) m_pLockfile->clean();
    }

    return bExit;
}

void Desktop::HandleBootstrapPathErrors( ::utl::Bootstrap::Status aBootstrapStatus, const OUString& aDiagnosticMessage )
{
    if ( aBootstrapStatus != ::utl::Bootstrap::DATA_OK )
    {
        sal_Bool            bWorkstationInstallation = sal_False;
        ::rtl::OUString        aBaseInstallURL;
        ::rtl::OUString        aUserInstallURL;
        ::rtl::OUString        aProductKey;
        ::rtl::OUString        aTemp;
        ::vos::OStartupInfo aInfo;

        aInfo.getExecutableFile( aProductKey );
        sal_uInt32     lastIndex = aProductKey.lastIndexOf('/');
        if ( lastIndex > 0 )
            aProductKey = aProductKey.copy( lastIndex+1 );

        aTemp = ::utl::Bootstrap::getProductKey( aProductKey );
        if ( aTemp.getLength() > 0 )
            aProductKey = aTemp;

        ::utl::Bootstrap::PathStatus aBaseInstallStatus = ::utl::Bootstrap::locateBaseInstallation( aBaseInstallURL );
        ::utl::Bootstrap::PathStatus aUserInstallStatus = ::utl::Bootstrap::locateUserInstallation( aUserInstallURL );

        if (( aBaseInstallStatus == ::utl::Bootstrap::PATH_EXISTS &&
              aUserInstallStatus == ::utl::Bootstrap::PATH_EXISTS        ))
        {
            if ( aBaseInstallURL != aUserInstallURL )
                bWorkstationInstallation = sal_True;
        }

        OUString        aMessage;
        OUStringBuffer    aBuffer( 100 );
        aBuffer.append( aDiagnosticMessage );

        aBuffer.appendAscii( "\n" );

        ErrorBox aBootstrapFailedBox( NULL, WB_OK, aMessage );
        aBootstrapFailedBox.SetText( aProductKey );
        aBootstrapFailedBox.Execute();
    }
}

// Create a error message depending on bootstrap failure code and an optional file url
::rtl::OUString    Desktop::CreateErrorMsgString(
    utl::Bootstrap::FailureCode nFailureCode,
    const ::rtl::OUString& aFileURL )
{
    OUString        aMsg;
    OUString        aFilePath;
    sal_Bool        bFileInfo = sal_True;

    switch ( nFailureCode )
    {
        /// the shared installation directory could not be located
        case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_PATH_INVALID,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The installation path is not available." )) );
            bFileInfo = sal_False;
        }
        break;

        /// the bootstrap INI file could not be found or read
        case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration file \"$1\" is missing." )) );
        }
        break;

        /// the bootstrap INI is missing a required entry
        /// the bootstrap INI contains invalid data
         case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
         case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_CORRUPT,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration file \"$1\" is corrupt." )) );
        }
        break;

        /// the version locator INI file could not be found or read
        case ::utl::Bootstrap::MISSING_VERSION_FILE:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_FILE_MISSING,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration file \"$1\" is missing." )) );
        }
        break;

        /// the version locator INI has no entry for this version
         case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SUPPORT,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The main configuration file \"$1\" does not support the current version." )) );
        }
        break;

        /// the user installation directory does not exist
           case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_DIR_MISSING,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration directory \"$1\" is missing." )) );
        }
        break;

        /// some bootstrap data was invalid in unexpected ways
        case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
        {
            aMsg = GetMsgString( STR_BOOTSTRAP_ERR_INTERNAL,
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "An internal failure occurred." )) );
            bFileInfo = sal_False;
        }
        break;

        case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
        {
            // This needs to be improved, see #i67575#:
            aMsg = OUString(
                RTL_CONSTASCII_USTRINGPARAM( "Invalid version file entry" ) );
            bFileInfo = sal_False;
        }
        break;

        case ::utl::Bootstrap::NO_FAILURE:
        {
            OSL_ASSERT(false);
        }
        break;
    }

    if ( bFileInfo )
    {
        String aMsgString( aMsg );

        osl::File::getSystemPathFromFileURL( aFileURL, aFilePath );

        aMsgString.SearchAndReplaceAscii( "$1", aFilePath );
        aMsg = aMsgString;
    }

    return MakeStartupErrorMessage( aMsg );
}

void Desktop::HandleBootstrapErrors( BootstrapError aBootstrapError )
{
    if ( aBootstrapError == BE_PATHINFO_MISSING )
    {
        OUString                    aErrorMsg;
        OUString                    aBuffer;
        utl::Bootstrap::Status        aBootstrapStatus;
        utl::Bootstrap::FailureCode    nFailureCode;

        aBootstrapStatus = ::utl::Bootstrap::checkBootstrapStatus( aBuffer, nFailureCode );
        if ( aBootstrapStatus != ::utl::Bootstrap::DATA_OK )
        {
            switch ( nFailureCode )
            {
                case ::utl::Bootstrap::MISSING_INSTALL_DIRECTORY:
                case ::utl::Bootstrap::INVALID_BOOTSTRAP_DATA:
                {
                    aErrorMsg = CreateErrorMsgString( nFailureCode, OUString() );
                }
                break;

                /// the bootstrap INI file could not be found or read
                /// the bootstrap INI is missing a required entry
                /// the bootstrap INI contains invalid data
                 case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE_ENTRY:
                 case ::utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY:
                case ::utl::Bootstrap::MISSING_BOOTSTRAP_FILE:
                {
                    OUString aBootstrapFileURL;

                    utl::Bootstrap::locateBootstrapFile( aBootstrapFileURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aBootstrapFileURL );
                }
                break;

                /// the version locator INI file could not be found or read
                /// the version locator INI has no entry for this version
                /// the version locator INI entry is not a valid directory URL
                   case ::utl::Bootstrap::INVALID_VERSION_FILE_ENTRY:
                 case ::utl::Bootstrap::MISSING_VERSION_FILE_ENTRY:
                 case ::utl::Bootstrap::MISSING_VERSION_FILE:
                {
                    OUString aVersionFileURL;

                    utl::Bootstrap::locateVersionFile( aVersionFileURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aVersionFileURL );
                }
                break;

                /// the user installation directory does not exist
                   case ::utl::Bootstrap::MISSING_USER_DIRECTORY:
                {
                    OUString aUserInstallationURL;

                    utl::Bootstrap::locateUserInstallation( aUserInstallationURL );
                    aErrorMsg = CreateErrorMsgString( nFailureCode, aUserInstallationURL );
                }
                break;

                case ::utl::Bootstrap::NO_FAILURE:
                {
                    OSL_ASSERT(false);
                }
                break;
            }

            HandleBootstrapPathErrors( aBootstrapStatus, aErrorMsg );
        }
    }
    else if ( aBootstrapError == BE_UNO_SERVICEMANAGER || aBootstrapError == BE_UNO_SERVICE_CONFIG_MISSING )
    {
        // Uno service manager is not available. VCL needs a uno service manager to display a message box!!!
        // Currently we are not able to display a message box with a service manager due to this limitations inside VCL.

        // When UNO is not properly initialized, all kinds of things can fail
        // and cause the process to crash (e.g., a call to GetMsgString may
        // crash when somewhere deep within that call Any::operator <= is used
        // with a PropertyValue, and no binary UNO type description for
        // PropertyValue is available).  To give the user a hint even if
        // generating and displaying a message box below crashes, print a
        // hard-coded message on stderr first:
        fputs(
            aBootstrapError == BE_UNO_SERVICEMANAGER
            ? ("The application cannot be started. " "\n"
               "The component manager is not available." "\n")
                // STR_BOOTSTRAP_ERR_CANNOT_START, STR_BOOTSTRAP_ERR_NO_SERVICE
            : ("The application cannot be started. " "\n"
               "The configuration service is not available." "\n"),
                // STR_BOOTSTRAP_ERR_CANNOT_START,
                // STR_BOOTSTRAP_ERR_NO_CFG_SERVICE
            stderr);

        // First sentence. We cannot bootstrap office further!
        OUString            aMessage;
        OUStringBuffer        aDiagnosticMessage( 100 );

        OUString aErrorMsg;

        if ( aBootstrapError == BE_UNO_SERVICEMANAGER )
            aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_SERVICE,
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "The service manager is not available." )) );
        else
            aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_NO_CFG_SERVICE,
                            OUString( RTL_CONSTASCII_USTRINGPARAM( "The configuration service is not available." )) );

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.appendAscii( "\n" );

        // Due to the fact the we haven't a backup applicat.rdb file anymore it is not possible to
        // repair the installation with the setup executable besides the office executable. Now
        // we have to ask the user to start the setup on CD/installation directory manually!!
        OUString aStartSetupManually( GetMsgString(
            STR_ASK_START_SETUP_MANUALLY,
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Start setup application to repair the installation from CD, or the folder containing the installation packages." )) ));

        aDiagnosticMessage.append( aStartSetupManually );
        aMessage = MakeStartupErrorMessage( aDiagnosticMessage.makeStringAndClear() );

        FatalError( aMessage);
    }
    else if ( aBootstrapError == BE_USERINSTALL_FAILED )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = GetMsgString( STR_BOOTSTRAP_ERR_INTERNAL,
            OUString( RTL_CONSTASCII_USTRINGPARAM( "User installation could not be completed" )) );
        aDiagnosticMessage.append( aErrorMsg );
        aMessage = MakeStartupErrorMessage( aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
    else if ( aBootstrapError == BE_LANGUAGE_MISSING )
    {
        OUString aMessage;
        OUStringBuffer aDiagnosticMessage( 100 );
        OUString aErrorMsg;
        aErrorMsg = GetMsgString(
            //@@@ FIXME: should use an own resource string => #i36213#
            STR_BOOTSTRAP_ERR_LANGUAGE_MISSING,
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "Language could not be determined." )) );
        aDiagnosticMessage.append( aErrorMsg );
        aMessage = MakeStartupErrorMessage(
            aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }
    else if (( aBootstrapError == BE_USERINSTALL_NOTENOUGHDISKSPACE ) ||
             ( aBootstrapError == BE_USERINSTALL_NOWRITEACCESS      ))
    {
        OUString       aUserInstallationURL;
        OUString       aUserInstallationPath;
        OUString       aMessage;
        OUString       aErrorMsg;
        OUStringBuffer aDiagnosticMessage( 100 );

        utl::Bootstrap::locateUserInstallation( aUserInstallationURL );

        if ( aBootstrapError == BE_USERINSTALL_NOTENOUGHDISKSPACE )
            aErrorMsg = GetMsgString(
                STR_BOOSTRAP_ERR_NOTENOUGHDISKSPACE,
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "User installation could not be completed due to insufficient free disk space." )) );
        else
            aErrorMsg = GetMsgString(
                STR_BOOSTRAP_ERR_NOACCESSRIGHTS,
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "User installation could not be processed due to missing access rights." )) );

        osl::File::getSystemPathFromFileURL( aUserInstallationURL, aUserInstallationPath );

        aDiagnosticMessage.append( aErrorMsg );
        aDiagnosticMessage.append( aUserInstallationPath );
        aMessage = MakeStartupErrorMessage(
            aDiagnosticMessage.makeStringAndClear() );
        FatalError(aMessage);
    }

    return;
}


void Desktop::retrieveCrashReporterState()
{
    static const ::rtl::OUString CFG_PACKAGE_RECOVERY   = ::rtl::OUString::createFromAscii("org.openoffice.Office.Recovery/");
    static const ::rtl::OUString CFG_PATH_CRASHREPORTER = ::rtl::OUString::createFromAscii("CrashReporter"                  );
    static const ::rtl::OUString CFG_ENTRY_ENABLED      = ::rtl::OUString::createFromAscii("Enabled"                                   );

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

    sal_Bool bEnabled( sal_True );
    if ( xSMGR.is() )
    {
        css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                    xSMGR,
                                    CFG_PACKAGE_RECOVERY,
                                    CFG_PATH_CRASHREPORTER,
                                    CFG_ENTRY_ENABLED,
                                    ::comphelper::ConfigurationHelper::E_READONLY);
        aVal >>= bEnabled;
    }
    _bCrashReporterEnabled = bEnabled;
}

//-----------------------------------------------
/** @short  check if crash reporter feature is enabled or
            disabled.
*/
sal_Bool Desktop::isCrashReporterEnabled()
{
    return _bCrashReporterEnabled;
}

//-----------------------------------------------
/** @short  check if recovery must be started or not.

    @param  bCrashed [boolean ... out!]
            the office crashed last times.
            But may be there are no recovery data.
            Usefull to trigger the error report tool without
            showing the recovery UI.

    @param  bRecoveryDataExists [boolean ... out!]
            there exists some recovery data.

    @param  bSessionDataExists [boolean ... out!]
            there exists some session data.
            Because the user may be logged out last time from it's
            unix session...
*/
void impl_checkRecoveryState(sal_Bool& bCrashed           ,
                             sal_Bool& bRecoveryDataExists,
                             sal_Bool& bSessionDataExists )
{
    static const ::rtl::OUString SERVICENAME_RECOVERYCORE = ::rtl::OUString::createFromAscii("com.sun.star.frame.AutoRecovery");
    static const ::rtl::OUString PROP_CRASHED             = ::rtl::OUString::createFromAscii("Crashed"                        );
    static const ::rtl::OUString PROP_EXISTSRECOVERY      = ::rtl::OUString::createFromAscii("ExistsRecoveryData"             );
    static const ::rtl::OUString PROP_EXISTSSESSION       = ::rtl::OUString::createFromAscii("ExistsSessionData"              );
    static const ::rtl::OUString CFG_PACKAGE_RECOVERY     = ::rtl::OUString::createFromAscii("org.openoffice.Office.Recovery/");
    static const ::rtl::OUString CFG_PATH_RECOVERYINFO    = ::rtl::OUString::createFromAscii("RecoveryInfo"                   );

    bCrashed            = sal_False;
    bRecoveryDataExists = sal_False;
    bSessionDataExists  = sal_False;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xRecovery(
            xSMGR->createInstance(SERVICENAME_RECOVERYCORE),
            css::uno::UNO_QUERY_THROW);

        xRecovery->getPropertyValue(PROP_CRASHED       ) >>= bCrashed           ;
        xRecovery->getPropertyValue(PROP_EXISTSRECOVERY) >>= bRecoveryDataExists;
        xRecovery->getPropertyValue(PROP_EXISTSSESSION ) >>= bSessionDataExists ;
    }
    catch(const css::uno::Exception&) {}
}

//-----------------------------------------------
/*  @short  start the recovery wizard.

    @param  bEmergencySave
            differs between EMERGENCY_SAVE and RECOVERY
*/
sal_Bool impl_callRecoveryUI(sal_Bool bEmergencySave     ,
                             sal_Bool bCrashed           ,
                             sal_Bool bExistsRecoveryData)
{
    static ::rtl::OUString SERVICENAME_RECOVERYUI = ::rtl::OUString::createFromAscii("com.sun.star.comp.svx.RecoveryUI"          );
    static ::rtl::OUString SERVICENAME_URLPARSER  = ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer"          );
    static ::rtl::OUString COMMAND_EMERGENCYSAVE  = ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doEmergencySave");
    static ::rtl::OUString COMMAND_RECOVERY       = ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doAutoRecovery" );
    static ::rtl::OUString COMMAND_CRASHREPORT    = ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/doCrashReport"  );

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

    css::uno::Reference< css::frame::XSynchronousDispatch > xRecoveryUI(
        xSMGR->createInstance(SERVICENAME_RECOVERYUI),
        css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::util::XURLTransformer > xURLParser(
        xSMGR->createInstance(SERVICENAME_URLPARSER),
        css::uno::UNO_QUERY_THROW);

    css::util::URL aURL;
    if (bEmergencySave)
        aURL.Complete = COMMAND_EMERGENCYSAVE;
    else
    {
        if (bExistsRecoveryData)
            aURL.Complete = COMMAND_RECOVERY;
        else
        if (bCrashed && Desktop::isCrashReporterEnabled() )
            aURL.Complete = COMMAND_CRASHREPORT;
    }

    sal_Bool bRet = sal_False;
    if ( aURL.Complete.getLength() > 0 )
    {
        xURLParser->parseStrict(aURL);

        css::uno::Any aRet = xRecoveryUI->dispatchWithReturnValue(aURL, css::uno::Sequence< css::beans::PropertyValue >());
        aRet >>= bRet;
    }
    return bRet;
}

/*
 * Save all open documents so they will be reopened
 * the next time the application ist started
 *
 * returns sal_True if at least one document could be saved...
 *
 */

sal_Bool Desktop::_bTasksSaved = sal_False;

sal_Bool Desktop::SaveTasks()
{
    return impl_callRecoveryUI(
        sal_True , // sal_True => force emergency save
        sal_False, // 2. and 3. param not used if 1. = true!
        sal_False);
}

USHORT Desktop::Exception(USHORT nError)
{
    // protect against recursive calls
    static BOOL bInException = FALSE;

    sal_uInt16 nOldMode = Application::GetSystemWindowMode();
    Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
    Application::SetDefDialogParent( NULL );

    if ( bInException )
    {
        String aDoubleExceptionString;
        Application::Abort( aDoubleExceptionString );
    }

    bInException = TRUE;
    CommandLineArgs* pArgs = GetCommandLineArgs();

    // save all modified documents ... if it's allowed doing so.
    sal_Bool bRestart                           = sal_False;
    sal_Bool bAllowRecoveryAndSessionManagement = (
                                                    ( !pArgs->IsNoRestore()                    ) && // some use cases of office must work without recovery
                                                    ( !pArgs->IsHeadless()                     ) &&
                                                    ( !pArgs->IsServer()                       ) &&
                                                    (( nError & EXC_MAJORTYPE ) != EXC_DISPLAY ) && // recovery cant work without UI ... but UI layer seams to be the reason for this crash
                                                    ( Application::IsInExecute()               )    // crashes during startup and shutdown should be ignored (they indicates a corrupt installation ...)
                                                  );
    if ( bAllowRecoveryAndSessionManagement )
        bRestart = SaveTasks();

    // because there is no method to flush the condiguration data, we must dispose the ConfigManager
    Reference < XFlushable > xCFGFlush( ::utl::ConfigManager::GetConfigManager()->GetConfigurationProvider(), UNO_QUERY );
    if (xCFGFlush.is())
    {
        xCFGFlush->flush();
    }
    else
    {
        Reference < XComponent > xCFGDispose( ::utl::ConfigManager::GetConfigManager()->GetConfigurationProvider(), UNO_QUERY );
        if (xCFGDispose.is())
            xCFGDispose->dispose();
    }

    switch( nError & EXC_MAJORTYPE )
    {
/*
        case EXC_USER:
            if( nError == EXC_OUTOFMEMORY )
            {
                // not possible without a special NewHandler!
                String aMemExceptionString;
                Application::Abort( aMemExceptionString );
            }
            break;
*/
        case EXC_RSCNOTLOADED:
        {
            String aResExceptionString;
            Application::Abort( aResExceptionString );
            break;
        }

        case EXC_SYSOBJNOTCREATED:
        {
            String aSysResExceptionString;
            Application::Abort( aSysResExceptionString );
            break;
        }

        default:
        {
            if ( pArgs->IsNoRestore() ) {
                if (m_pLockfile != NULL) {
                    m_pLockfile->clean();
                }
                _exit( ExitHelper::E_LOCKFILE );
            }

            if( bRestart )
            {
                OfficeIPCThread::DisableOfficeIPCThread();
                if( pSignalHandler )
                    DELETEZ( pSignalHandler );

                if (m_pLockfile != NULL) {
                    m_pLockfile->clean();
                }
                _exit( ExitHelper::E_CRASH_WITH_RESTART );
            }
            else
            {
                bInException = sal_False;
                _exit( ExitHelper::E_CRASH );
            }

            break;
        }
    }

    return 0;

    // ConfigManager is disposed, so no way to continue
}

void Desktop::AppEvent( const ApplicationEvent& rAppEvent )
{
    HandleAppEvent( rAppEvent );
}

::rtl::OUString Desktop::GetProductVersionID()
{
    ::rtl::OUString aProdVersionID;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    if ( xSMGR.is() )
    {
        try
        {
            // Read URL from configuration
            css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                        xSMGR,
                                        CFG_PACKAGE_COMMON_HELP,
                                        CFG_PATH_REG,
                                        CFG_ENTRY_PRODVERSIONID,
                                        ::comphelper::ConfigurationHelper::E_READONLY);

            aVal >>= aProdVersionID;
        }
        catch ( css::uno::Exception& )
        {
        }
    }

    return aProdVersionID;
}

::rtl::OUString Desktop::GetInstanceUUID( const ::rtl::OUString& rProductVersionID )
{
    ::rtl::OUString aUUID;
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

    if ( xSMGR.is() )
    {
        try
        {
            css::uno::Reference< css::uno::XInterface > xCfg = ::comphelper::ConfigurationHelper::openConfig(
                                                                    ::comphelper::getProcessServiceFactory(),
                                                                    CONFIGPATH_PRODUCTVERSIONS,
                                                                    ::comphelper::ConfigurationHelper::E_STANDARD);

            css::uno::Reference< css::container::XNameAccess > xRead(xCfg, css::uno::UNO_QUERY_THROW);
            css::uno::Any a = xRead->getByName( rProductVersionID );

            css::uno::Reference< css::container::XNameAccess > xProductVersionAccess;
            if ( a >>= xProductVersionAccess )
            {
                a = xProductVersionAccess->getByName( CFG_ENTRY_INSTANCEUUID );
                a >>= aUUID;
            }
        }
        catch ( css::uno::Exception& )
        {
        }
    }

    return aUUID;
}

::rtl::OUString Desktop::GenerateUUID()
{
    const sal_Int32 UUID_LEN        = 16;
    const sal_Int32 BASE_HEX        = 16;
    const sal_Int32 UUID_STRING_LEN = 36;

    sal_uInt8 nUUID[UUID_LEN];

    rtl_createUuid( nUUID, NULL, false );

    ::rtl::OUStringBuffer aUUIDBuf(UUID_STRING_LEN);
    for ( sal_Int32 i = 0; i < UUID_LEN; i++ )
    {
        ::rtl::OUString aStrValue = ::rtl::OUString::valueOf( sal_Int32( nUUID[i] ), BASE_HEX );
        if ( aStrValue.getLength() == 1 )
            aUUIDBuf.appendAscii( "0" );
        aUUIDBuf.append( aStrValue );
        if (( i == 3 ) || ( i == 5 ) || ( i == 7 ) || ( i == 9 ))
            aUUIDBuf.appendAscii( "-" );
    }

    return aUUIDBuf.makeStringAndClear();
}

void impl_setInstanceUIDForSTClientJob(const ::rtl::OUString& sInstUID)
{
    static const ::rtl::OUString CONFIGPATH_STCLIENT_JOBARGS     = ::rtl::OUString::createFromAscii("org.openoffice.Office.Jobs/Jobs/com.sun.star.jobs.STClient/Arguments");
    static const ::rtl::OUString CONFIGPATH_STCLIENT_COMMANDLINE = ::rtl::OUString::createFromAscii("Arguments");
    static const ::rtl::OUString CONFIGPROP_STCLIENT_COMMANDLINE = ::rtl::OUString::createFromAscii("Arguments");
    static const ::rtl::OUString PARAM_INSTANCE_URN              = ::rtl::OUString::createFromAscii( "-i" );

    ::rtl::OUStringBuffer sGenerator(256);
    sGenerator.appendAscii ("urn:st:");
    sGenerator.append      (sInstUID   );

    const ::rtl::OUString sInstURN = sGenerator.makeStringAndClear ();
    css::uno::Reference< css::uno::XInterface > xCfg;
    css::uno::Reference< css::container::XNameAccess > xRead;
    try
    {
         xCfg = ::comphelper::ConfigurationHelper::openConfig( ::comphelper::getProcessServiceFactory(),
                                                               CONFIGPATH_STCLIENT_JOBARGS,
                                                               ::comphelper::ConfigurationHelper::E_STANDARD);

        xRead = css::uno::Reference< css::container::XNameAccess >(xCfg, css::uno::UNO_QUERY_THROW);
    }
    catch ( css::uno::Exception& )
    {
        return;
    }
    css::uno::Sequence< ::rtl::OUString > lCmdLine;
    xRead->getByName (CONFIGPROP_STCLIENT_COMMANDLINE) >>= lCmdLine;

    const ::sal_Int32 c        = lCmdLine.getLength ();
          ::sal_Int32 i        = 0;
          ::sal_Bool  bPatched = sal_False;
    for (i=0; i<c; ++i)
    {
        const ::rtl::OUString& sArg = lCmdLine[i];
        if (sArg.equals(PLACEHOLDER_INSTANCE_URN))
        {
            lCmdLine[i] = sInstURN;
            bPatched = sal_True;
            break;
        }
    }

    if (bPatched)
    {
        css::uno::Reference< css::container::XNameReplace > xWrite (xCfg, css::uno::UNO_QUERY_THROW);
        xWrite->replaceByName(CONFIGPROP_STCLIENT_COMMANDLINE, css::uno::makeAny(lCmdLine));
        ::comphelper::ConfigurationHelper::flush(xCfg);
    }
}

void impl_setInstanceUUIDForProductVersionID( const ::rtl::OUString& rProductVersionID, const ::rtl::OUString& rUUID )
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    if ( xSMGR.is() )
    {
        css::uno::Reference< css::uno::XInterface > xCfg = ::comphelper::ConfigurationHelper::openConfig(
                                                                ::comphelper::getProcessServiceFactory(),
                                                                CONFIGPATH_PRODUCTVERSIONS,
                                                                ::comphelper::ConfigurationHelper::E_STANDARD);

        css::uno::Reference< css::container::XNameContainer > xWrite(xCfg, css::uno::UNO_QUERY_THROW);

        css::uno::Reference< css::lang::XSingleServiceFactory > xFactory(xCfg, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::beans::XPropertySet > xPropSet( xFactory->createInstance(), css::uno::UNO_QUERY_THROW );

        xPropSet->setPropertyValue( CFG_ENTRY_INSTANCEUUID, css::uno::makeAny( rUUID ));

        xWrite->insertByName( rProductVersionID, css::uno::makeAny( xPropSet ));
        ::comphelper::ConfigurationHelper::flush(xCfg);
    }
}

void Desktop::StoreInstanceUUID( const ::rtl::OUString& rProductVersionID, const ::rtl::OUString& rUUID )
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

    if ( xSMGR.is() )
    {
        try
        {
            // exchange place holder in job configuration
            impl_setInstanceUIDForSTClientJob( rUUID );

            // Exchange place holder ${INSTANCE_URN} in registration URL
            ::rtl::OUString aURL;

            // Read URL from configuration
            css::uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                        xSMGR,
                                        CFG_PACKAGE_COMMON_HELP,
                                        CFG_PATH_REG,
                                        CFG_ENTRY_TEMPLATEREGURL,
                                        ::comphelper::ConfigurationHelper::E_READONLY);
            aVal >>= aURL;

            // Detect and replace id parameter with the latest instance uuid
            sal_Int32 nIndex = aURL.indexOf( PLACEHOLDER_INSTANCE_URN );
            if ( nIndex >= 0 )
            {
                sal_Int32 nIdLen = PLACEHOLDER_INSTANCE_URN.getLength();

                rtl::OUStringBuffer aBuf( 40 );
                aBuf.appendAscii( "urn:st:" );
                aBuf.append( rUUID );
                aURL = aURL.replaceAt( nIndex, nIdLen, aBuf.makeStringAndClear() );
            }

            // Write changed URL to configuration
            ::comphelper::ConfigurationHelper::writeDirectKey(
                    xSMGR,
                    CFG_PACKAGE_COMMON_HELP,
                    CFG_PATH_REG,
                    CFG_ENTRY_REGURL,
                    ::com::sun::star::uno::makeAny( aURL ),
                    ::comphelper::ConfigurationHelper::E_STANDARD );

            // Write instance UUID for product version to configuration
            impl_setInstanceUUIDForProductVersionID( rProductVersionID, rUUID );
        }
        catch ( css::uno::Exception& )
        {
        }
    }
}

void Desktop::Main()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::Main" );

    // Remember current context object
    com::sun::star::uno::ContextLayer layer(
        com::sun::star::uno::getCurrentContext() );

    BootstrapError eError = GetBootstrapError();
    if ( eError != BE_OK )
    {
        HandleBootstrapErrors( eError );
        return;
    }

    BootstrapStatus eStatus = GetBootstrapStatus();
    if (eStatus == BS_TERMINATE) {
        return;
    }

    // Detect desktop environment - need to do this as early as possible
    com::sun::star::uno::setCurrentContext(
        new DesktopContext( com::sun::star::uno::getCurrentContext() ) );

    CommandLineArgs* pCmdLineArgs = GetCommandLineArgs();

    // setup configuration error handling
    ConfigurationErrorHandler aConfigErrHandler;
    if (!ShouldSuppressUI(pCmdLineArgs))
        aConfigErrHandler.activate();

    ResMgr::SetReadStringHook( ReplaceStringHookProc );

    // Startup screen
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main { OpenSplashScreen" );
    OpenSplashScreen();
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main } OpenSplashScreen" );

    {
        UserInstall::UserInstallError instErr_fin = UserInstall::finalize();
        if ( instErr_fin != UserInstall::E_None)
        {
            OSL_ENSURE(sal_False, "userinstall failed");
            if ( instErr_fin == UserInstall::E_NoDiskSpace )
                HandleBootstrapErrors( BE_USERINSTALL_NOTENOUGHDISKSPACE );
            else if ( instErr_fin == UserInstall::E_NoWriteAccess )
                HandleBootstrapErrors( BE_USERINSTALL_NOWRITEACCESS );
            else
                HandleBootstrapErrors( BE_USERINSTALL_FAILED );
            return;
        }
        // refresh path information
        utl::Bootstrap::reloadData();
        SetSplashScreenProgress(25);
    }

    Reference< XMultiServiceFactory > xSMgr =
        ::comphelper::getProcessServiceFactory();

    std::auto_ptr<SvtLanguageOptions> pLanguageOptions;
    std::auto_ptr<SvtPathOptions> pPathOptions;

    try
    {
        RegisterServices( xSMgr );

        //SetSplashScreenProgress(15);

#ifndef UNX
        if ( pCmdLineArgs->IsHelp() ) {
            displayCmdlineHelp();
            return;
        }
#endif

        // check user installation directory for lockfile so we can be sure
        // there is no other instance using our data files from a remote host
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main -> Lockfile" );
        m_pLockfile = new Lockfile;
        if ( !pCmdLineArgs->IsInvisible() && !pCmdLineArgs->IsNoLockcheck() && !m_pLockfile->check( Lockfile_execWarning )) {
            // Lockfile exists, and user clicked 'no'
            return;
        }
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109) Desktop::Main <- Lockfile" );

        // check if accessibility is enabled but not working and allow to quit
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ GetEnableATToolSupport" );
        if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
        {
            BOOL bQuitApp;

            if( !InitAccessBridge( true, bQuitApp ) )
                if( bQuitApp )
                    return;
        }
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} GetEnableATToolSupport" );

        // terminate if requested...
        if( pCmdLineArgs->IsTerminateAfterInit() ) return;


        //  Read the common configuration items for optimization purpose
        if ( !InitializeConfiguration() ) return;

        //SetSplashScreenProgress(20);

        // set static variable to enabled/disable crash reporter
        retrieveCrashReporterState();
        if ( !isCrashReporterEnabled() )
        {
            osl_setErrorReporting( sal_False );
            // disable stack trace feature
        }

        // create title string
        sal_Bool bCheckOk = sal_False;
        ::com::sun::star::lang::Locale aLocale;
        String aMgrName = String::CreateFromAscii( "iso" );
        ResMgr* pLabelResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLocale );
        if ( !pLabelResMgr )
        {
            // no "iso" resource -> search for "ooo" resource
            aMgrName = String::CreateFromAscii( "ooo" );
            pLabelResMgr = ResMgr::SearchCreateResMgr( U2S( aMgrName ), aLocale);
        }
        String aTitle = pLabelResMgr ? String( ResId( RID_APPTITLE, *pLabelResMgr ) ) : String();
        delete pLabelResMgr;

        // set UI language and locale
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ set locale settings" );
        //LanguageSelection langselect;
        OUString aUILocaleString = LanguageSelection::getLanguageString();
        Locale aUILocale = LanguageSelection::IsoStringToLocale(aUILocaleString);
        LanguageType eLanguage = SvtSysLocaleOptions().GetLocaleLanguageType();

        // #i39040#, do not call anything between GetSettings and SetSettings that might have
        // a side effect on the settings (like, eg, SvtSysLocaleOptions().GetLocaleLanguageType(),
        // which changes the MiscSettings !!! )
        AllSettings aSettings( Application::GetSettings() );
        aSettings.SetUILocale( aUILocale );
        aSettings.SetLanguage( eLanguage );
        Application::SetSettings( aSettings );
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} set locale settings" );


        // Check for StarOffice/Suite specific extensions runs also with OpenOffice installation sets
        OUString aTitleString( aTitle );
        bCheckOk = CheckInstallation( aTitleString );
        if ( !bCheckOk )
            return;
        else
            aTitle = aTitleString;

#ifndef PRODUCT
        //include version ID in non product builds
        ::rtl::OUString aDefault;
        aTitle += DEFINE_CONST_UNICODE(" [");
        String aVerId( utl::Bootstrap::getBuildIdData( aDefault ));
        aTitle += aVerId;
        aTitle += ']';
#endif

        SetDisplayName( aTitle );
//        SetSplashScreenProgress(30);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ create SvtPathOptions and SvtLanguageOptions" );
        pPathOptions.reset( new SvtPathOptions);
//        SetSplashScreenProgress(40);
//        pLanguageOptions = new SvtLanguageOptions(sal_True);
//        SetSplashScreenProgress(45);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} create SvtPathOptions and SvtLanguageOptions" );

        // Check special env variable #111015#
        std::vector< String > aUnrestrictedFolders;
        svt::getUnrestrictedFolders( aUnrestrictedFolders );

        if ( aUnrestrictedFolders.size() > 0 )
        {
            // Set different working directory. The first entry is
            // the new work path.
            String aWorkPath = aUnrestrictedFolders[0];
            SvtPathOptions().SetWorkPath( aWorkPath );
        }

        // create service for loadin SFX (still needed in startup)
        Reference < css::document::XEventListener > xGlobalBroadcaster( xSMgr->createInstance(
            DEFINE_CONST_UNICODE( "com.sun.star.frame.GlobalEventBroadcaster" ) ), UNO_QUERY );

        // initialize test-tool library (if available)
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ tools::InitTestToolLib" );
        tools::InitTestToolLib();
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} tools::InitTestToolLib" );

        // Important: The UUID generation must be placed before the first start wizard
        ::rtl::OUString aProductVersionID = GetProductVersionID();
        if ( aProductVersionID.getLength() > 0 )
        {
            ::rtl::OUString aUUID = GetInstanceUUID( aProductVersionID );
            if ( aUUID.getLength() == 0 )
            {
                // UUID generation
                aUUID = GenerateUUID();
                StoreInstanceUUID( aProductVersionID, aUUID );
            }
        }

        // First Start Wizard
        if ( IsFirstStartWizardNeeded() && !pCmdLineArgs->IsNoFirstStartWizard() )
        {
            Reference< XJob > xFirstStartJob( xSMgr->createInstance(
                DEFINE_CONST_UNICODE( "com.sun.star.comp.desktop.FirstStart" ) ), UNO_QUERY );
            if (xFirstStartJob.is())
            {
                sal_Bool bDone = sal_False;
                Sequence< NamedValue > lArgs(2);
                lArgs[0].Name    = ::rtl::OUString::createFromAscii("LicenseNeedsAcceptance");
                lArgs[0].Value <<= LicenseNeedsAcceptance();
                lArgs[1].Name    = ::rtl::OUString::createFromAscii("LicensePath");
                lArgs[1].Value <<= GetLicensePath();

                xFirstStartJob->execute(lArgs) >>= bDone;
                if (!bDone) {
                    return;
                }
            }
        }
        else if ( RegistrationPage::hasReminderDateCome() )
            RegistrationPage::executeSingleMode();

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} FirstStartWizard" );

        // keep a language options instance...
        pLanguageOptions.reset( new SvtLanguageOptions(sal_True));

        if (xGlobalBroadcaster.is())
        {
            css::document::EventObject aEvent;
            aEvent.EventName = ::rtl::OUString::createFromAscii("OnStartApp");
            xGlobalBroadcaster->notifyEvent(aEvent);
        }

        SetSplashScreenProgress(50);

        // Backing Component
        sal_Bool bCrashed            = sal_False;
        sal_Bool bExistsRecoveryData = sal_False;
        sal_Bool bExistsSessionData  = sal_False;

        RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ impl_checkRecoveryState" );
        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);
        RTL_LOGFILE_CONTEXT_TRACE( aLog, "} impl_checkRecoveryState" );

        if (
            (pCmdLineArgs->IsEmptyOrAcceptOnly()                                   ) &&
            (SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE)) &&
            (!bExistsRecoveryData                                                  ) &&
            (!bExistsSessionData                                                   ) &&
            (!Application::AnyInput( INPUT_APPEVENT )                              )
           )
        {
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ create BackingComponent" );
            Reference< XFrame > xDesktopFrame( xSMgr->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY );
            if (xDesktopFrame.is())
            {
//                SetSplashScreenProgress(60);
                Reference< XFrame > xBackingFrame;
                Reference< ::com::sun::star::awt::XWindow > xContainerWindow;

                xBackingFrame = xDesktopFrame->findFrame(OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" )), 0);
                if (xBackingFrame.is())
                    xContainerWindow = xBackingFrame->getContainerWindow();
                if (xContainerWindow.is())
                {
                    SetSplashScreenProgress(75);
                    Sequence< Any > lArgs(1);
                    lArgs[0] <<= xContainerWindow;

                    Reference< XController > xBackingComp(
                        xSMgr->createInstanceWithArguments(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.StartModule") ), lArgs),
                        UNO_QUERY);
//                    SetSplashScreenProgress(80);
                    if (xBackingComp.is())
                    {
                        Reference< ::com::sun::star::awt::XWindow > xBackingWin(xBackingComp, UNO_QUERY);
                        // Attention: You MUST(!) call setComponent() before you call attachFrame().
                        // Because the backing component set the property "IsBackingMode" of the frame
                        // to true inside attachFrame(). But setComponent() reset this state everytimes ...
                        xBackingFrame->setComponent(xBackingWin, xBackingComp);
                        SetSplashScreenProgress(100);
                        xBackingComp->attachFrame(xBackingFrame);
                        CloseSplashScreen();
                        xContainerWindow->setVisible(sal_True);

                        Window* pCompWindow = VCLUnoHelper::GetWindow(xBackingFrame->getComponentWindow());
                        if (pCompWindow)
                            pCompWindow->Update();
                    }
                }
            }
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "} create BackingComponent" );
        }
    }
    catch ( com::sun::star::lang::WrappedTargetException& wte )
    {
        com::sun::star::uno::Exception te;
        wte.TargetException >>= te;
        FatalError( MakeStartupConfigAccessErrorMessage(wte.Message + te.Message) );
        return;
    }
    catch ( com::sun::star::uno::Exception& e )
    {
        FatalError( MakeStartupErrorMessage(e.Message) );
        return;
    }
    /*
    catch ( ... )
    {
        FatalError( MakeStartupErrorMessage(
            OUString::createFromAscii(
            "Unknown error during startup (Office wrapper service).\nInstallation could be damaged.")));
        return;
    }
    */
//    SetSplashScreenProgress(55);

    SvtFontSubstConfig().Apply();

    SvtTabAppearanceCfg aAppearanceCfg;
    //aAppearanceCfg.SetInitialized();
    aAppearanceCfg.SetApplicationDefaults( this );
    SvtAccessibilityOptions aOptions;
    aOptions.SetVCLSettings();
//    SetSplashScreenProgress(60);

    Application::SetFilterHdl( LINK( this, Desktop, ImplInitFilterHdl ) );

    sal_Bool bTerminateRequested = sal_False;

    // Preload function depends on an initialized sfx application!
    SetSplashScreenProgress(75);

    sal_Bool bUseSystemFileDialog(sal_True);
    if ( pCmdLineArgs->IsHeadless() )
    {
        // Ensure that we use not the system file dialogs as
        // headless mode relies on Application::EnableHeadlessMode()
        // which does only work for VCL dialogs!!
        SvtMiscOptions aMiscOptions;
        bUseSystemFileDialog = aMiscOptions.UseSystemFileDialog();
        aMiscOptions.SetUseSystemFileDialog( sal_False );
    }

    // use system window dialogs
    Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_DIALOG );

//    SetSplashScreenProgress(80);

    if ( !bTerminateRequested && !pCmdLineArgs->IsInvisible() )
        InitializeQuickstartMode( xSMgr );

    RTL_LOGFILE_CONTEXT( aLog2, "desktop (cd100003) createInstance com.sun.star.frame.Desktop" );
    try
    {
        Reference< XDesktop > xDesktop( xSMgr->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY );
        if ( xDesktop.is() )
            xDesktop->addTerminateListener( new OfficeIPCThreadController );
        SetSplashScreenProgress(100);
    }
    catch ( com::sun::star::uno::Exception& e )
    {
        FatalError( MakeStartupErrorMessage(e.Message) );
        return;
    }
    /*
    catch ( ... )
    {
        FatalError( MakeStartupErrorMessage(
            OUString::createFromAscii(
            "Unknown error during startup (TD/Desktop service).\nInstallation could be damaged.")));
        return;
    }
    */

    // Release solar mutex just before we wait for our client to connect
    int nAcquireCount = 0;
    ::vos::IMutex& rMutex = Application::GetSolarMutex();
    if ( rMutex.tryToAcquire() )
        nAcquireCount = Application::ReleaseSolarMutex() - 1;

    // Post user event to startup first application component window
    // We have to send this OpenClients message short before execute() to
    // minimize the risk that this message overtakes type detection contruction!!
    Application::PostUserEvent( LINK( this, Desktop, OpenClients_Impl ) );

    // Post event to enable acceptors
    Application::PostUserEvent( LINK( this, Desktop, EnableAcceptors_Impl) );

    // The configuration error handler currently is only for startup
    aConfigErrHandler.deactivate();

   // Acquire solar mutex just before we enter our message loop
    if ( nAcquireCount )
        Application::AcquireSolarMutex( nAcquireCount );

    // call Application::Execute to process messages in vcl message loop
    RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Application::Execute()" );

    try
    {
        // The JavaContext contains an interaction handler which is used when
        // the creation of a Java Virtual Machine fails
        com::sun::star::uno::ContextLayer layer2(
            new svt::JavaContext( com::sun::star::uno::getCurrentContext() ) );

        Execute();
    }
    catch(const com::sun::star::document::CorruptedFilterConfigurationException& exFilterCfg)
    {
        OfficeIPCThread::BlockAllRequests();
        FatalError( MakeStartupErrorMessage(exFilterCfg.Message) );
    }
    catch(const com::sun::star::configuration::CorruptedConfigurationException& exAnyCfg)
    {
        OfficeIPCThread::BlockAllRequests();
        FatalError( MakeStartupErrorMessage(exAnyCfg.Message) );
    }

    delete pResMgr;

    // Restore old value
    if ( pCmdLineArgs->IsHeadless() )
        SvtMiscOptions().SetUseSystemFileDialog( bUseSystemFileDialog );

    // remove temp directory
    RemoveTemporaryDirectory();

    DeregisterServices();

    tools::DeInitTestToolLib();

    // be sure that path/language options gets destroyed before
    // UCB is deinitialized
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "-> dispose path/language options" );
    pLanguageOptions.reset( 0 );
    pPathOptions.reset( 0 );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- dispose path/language options" );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "-> deinit ucb" );
    ::ucbhelper::ContentBroker::deinitialize();
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "<- deinit ucb" );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "FINISHED WITH Destop::Main" );
}

IMPL_LINK( Desktop, ImplInitFilterHdl, ConvertData*, pData )
{
    return GraphicFilter::GetGraphicFilter()->GetFilterCallback().Call( pData );
}

sal_Bool Desktop::InitializeConfiguration()
{
    sal_Bool bOk = sal_False;

    try
    {
        bOk = InitConfiguration();
    }
    catch( ::com::sun::star::lang::ServiceNotRegisteredException& )
    {
        this->HandleBootstrapErrors( Desktop::BE_UNO_SERVICE_CONFIG_MISSING );
    }
    catch( ::com::sun::star::configuration::MissingBootstrapFileException& e )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::MISSING_BOOTSTRAP_FILE,
                                                e.BootstrapFileURL ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_USER_INSTALL, aMsg );
    }
    catch( ::com::sun::star::configuration::InvalidBootstrapFileException& e )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_FILE_ENTRY,
                                                e.BootstrapFileURL ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    catch( ::com::sun::star::configuration::InstallationIncompleteException& )
    {
        OUString aVersionFileURL;
        OUString aMsg;
        utl::Bootstrap::PathStatus aPathStatus = utl::Bootstrap::locateVersionFile( aVersionFileURL );
        if ( aPathStatus == utl::Bootstrap::PATH_EXISTS )
            aMsg = CreateErrorMsgString( utl::Bootstrap::MISSING_VERSION_FILE_ENTRY, aVersionFileURL );
        else
            aMsg = CreateErrorMsgString( utl::Bootstrap::MISSING_VERSION_FILE, aVersionFileURL );

        HandleBootstrapPathErrors( ::utl::Bootstrap::MISSING_USER_INSTALL, aMsg );
    }
    catch ( com::sun::star::configuration::backend::BackendAccessException& exception)
    {
        // [cm122549] It is assumed in this case that the message
        // coming from InitConfiguration (in fact CreateApplicationConf...)
        // is suitable for display directly.
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( com::sun::star::configuration::backend::BackendSetupException& exception)
    {
        // [cm122549] It is assumed in this case that the message
        // coming from InitConfiguration (in fact CreateApplicationConf...)
        // is suitable for display directly.
        FatalError( MakeStartupErrorMessage( exception.Message ) );
    }
    catch ( ::com::sun::star::configuration::CannotLoadConfigurationException& )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                OUString() ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        OUString aMsg( CreateErrorMsgString( utl::Bootstrap::INVALID_BOOTSTRAP_DATA,
                                                OUString() ));
        HandleBootstrapPathErrors( ::utl::Bootstrap::INVALID_BASE_INSTALL, aMsg );
    }

    return bOk;
}

sal_Bool Desktop::InitializeQuickstartMode( Reference< XMultiServiceFactory >& rSMgr )
{
    try
    {
        // the shutdown icon sits in the systray and allows the user to keep
        // the office instance running for quicker restart
        // this will only be activated if -quickstart was specified on cmdline
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) createInstance com.sun.star.office.Quickstart" );

        sal_Bool bQuickstart = GetCommandLineArgs()->IsQuickstart();
        Sequence< Any > aSeq( 1 );
        aSeq[0] <<= bQuickstart;

        // Try to instanciate quickstart service. This service is not mandatory, so
        // do nothing if service is not available.
        Reference < XComponent > xQuickstart( rSMgr->createInstanceWithArguments(
                                                DEFINE_CONST_UNICODE( "com.sun.star.office.Quickstart" ), aSeq ),
                                                UNO_QUERY );
        return sal_True;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
        return sal_False;
    }
}

void Desktop::SystemSettingsChanging( AllSettings& rSettings, Window* )
{
    if ( !SvtTabAppearanceCfg::IsInitialized () )
        return;

#   define DRAGFULL_OPTION_ALL \
         ( DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE  \
         | DRAGFULL_OPTION_OBJECTMOVE  | DRAGFULL_OPTION_OBJECTSIZE \
         | DRAGFULL_OPTION_DOCKING     | DRAGFULL_OPTION_SPLIT      \
         | DRAGFULL_OPTION_SCROLL )
#   define DRAGFULL_OPTION_NONE ((sal_uInt32)~DRAGFULL_OPTION_ALL)

    StyleSettings hStyleSettings   = rSettings.GetStyleSettings();
    MouseSettings hMouseSettings = rSettings.GetMouseSettings();

    sal_uInt32 nDragFullOptions = hStyleSettings.GetDragFullOptions();

    SvtTabAppearanceCfg aAppearanceCfg;
    sal_uInt16 nGet = aAppearanceCfg.GetDragMode();
    switch ( nGet )
    {
    case DragFullWindow:
        nDragFullOptions |= DRAGFULL_OPTION_ALL;
        break;
    case DragFrame:
        nDragFullOptions &= DRAGFULL_OPTION_NONE;
        break;
    case DragSystemDep:
    default:
        break;
    }

    sal_uInt32 nFollow = hMouseSettings.GetFollow();
    hMouseSettings.SetFollow( aAppearanceCfg.IsMenuMouseFollow() ? (nFollow|MOUSE_FOLLOW_MENU) : (nFollow&~MOUSE_FOLLOW_MENU));
    rSettings.SetMouseSettings(hMouseSettings);

    sal_uInt16 nTabStyle = hStyleSettings.GetTabControlStyle();
    nTabStyle &= ~STYLE_TABCONTROL_SINGLELINE;
    if( aAppearanceCfg.IsSingleLineTabCtrl() )
        nTabStyle |=STYLE_TABCONTROL_SINGLELINE;

    nTabStyle &= ~STYLE_TABCONTROL_COLOR;
    if( aAppearanceCfg.IsColoredTabCtrl() )
        nTabStyle |= STYLE_TABCONTROL_COLOR;

    hStyleSettings.SetTabControlStyle(nTabStyle);

    hStyleSettings.SetDragFullOptions( nDragFullOptions );
    rSettings.SetStyleSettings ( hStyleSettings );

    MiscSettings aMiscSettings( rSettings.GetMiscSettings() );
    aMiscSettings.SetTwoDigitYearStart( (USHORT) SfxMiscCfg().GetYear2000() );
    rSettings.SetMiscSettings( aMiscSettings );
}

// ========================================================================
IMPL_LINK( Desktop, AsyncInitFirstRun, void*, EMPTYARG )
{
    DoFirstRunInitializations();
    return 0L;
}

// ========================================================================

IMPL_STATIC_LINK_NOINSTANCE( Desktop, AsyncTerminate, void*, EMPTYARG )
{
    Reference<XMultiServiceFactory> rFactory = ::comphelper::getProcessServiceFactory();
    Reference< XDesktop > xDesktop( rFactory->createInstance(
        OUString::createFromAscii("com.sun.star.frame.Desktop")),
        UNO_QUERY );
    xDesktop.is() && xDesktop->terminate();
    return 0L;
}

class ExitTimer : public Timer
{
  public:
    ExitTimer()
    {
        SetTimeout(500);
        Start();
    }
    virtual void Timeout()
    {
        exit(42);
    }
};

IMPL_LINK( Desktop, OpenClients_Impl, void*, EMPTYARG )
{
    RTL_LOGFILE_PRODUCT_CONTEXT( aLog, "PERFORMANCE - DesktopOpenClients_Impl()" );

    OpenClients();

    OfficeIPCThread::SetReady();

    // CloseStartupScreen();
    CloseSplashScreen();

    CheckFirstRun( );

    // allow ipc interaction
//    OfficeIPCThread::SetReady();

    EnableOleAutomation();

    if (getenv ("OOO_EXIT_POST_STARTUP"))
        new ExitTimer();
    return 0;
}

// enable acceptos
IMPL_LINK( Desktop, EnableAcceptors_Impl, void*, EMPTYARG )
{
    enableAcceptors();
    return 0;
}


// Registers a COM class factory of the service manager with the windows operating system.
void Desktop::EnableOleAutomation()
{
      RTL_LOGFILE_CONTEXT( aLog, "desktop (jl97489) ::Desktop::EnableOleAutomation" );
#ifdef WNT
    Reference< XMultiServiceFactory > xSMgr=  comphelper::getProcessServiceFactory();
    xSMgr->createInstance(DEFINE_CONST_UNICODE("com.sun.star.bridge.OleApplicationRegistration"));
    xSMgr->createInstance(DEFINE_CONST_UNICODE("com.sun.star.comp.ole.EmbedServer"));
#endif
}

sal_Bool Desktop::CheckOEM()
{
    Reference<XMultiServiceFactory> rFactory = ::comphelper::getProcessServiceFactory();
    Reference<XJob> rOemJob(rFactory->createInstance(
        OUString::createFromAscii("com.sun.star.office.OEMPreloadJob")),
        UNO_QUERY );
    Sequence<NamedValue> args;
    sal_Bool bResult = sal_False;
    if (rOemJob.is()) {
        Any aResult = rOemJob->execute(args);
        aResult >>= bResult;
        return bResult;
    } else {
        return sal_True;
    }
}

void Desktop::PreloadModuleData( CommandLineArgs* pArgs )
{
    Reference< XMultiServiceFactory > rFactory = ::comphelper::getProcessServiceFactory();

    Sequence < com::sun::star::beans::PropertyValue > args(1);
    args[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
    args[0].Value <<= sal_True;
    Reference < XComponentLoader > xLoader( ::comphelper::getProcessServiceFactory()->createInstance(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ), UNO_QUERY );

    if ( !xLoader.is() )
        return;

    if ( pArgs->IsWriter() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( DEFINE_CONST_UNICODE("private:factory/swriter"),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }
    if ( pArgs->IsCalc() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( DEFINE_CONST_UNICODE("private:factory/scalc"),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }
    if ( pArgs->IsDraw() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( DEFINE_CONST_UNICODE("private:factory/sdraw"),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }
    if ( pArgs->IsImpress() )
    {
        try
        {
            Reference < ::com::sun::star::util::XCloseable > xDoc( xLoader->loadComponentFromURL( DEFINE_CONST_UNICODE("private:factory/simpress"),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")), 0, args ), UNO_QUERY_THROW );
            xDoc->close( sal_False );
        }
        catch ( com::sun::star::uno::Exception& )
        {
        }
    }
}

void Desktop::PreloadConfigurationData()
{
    Reference< XMultiServiceFactory > rFactory = ::comphelper::getProcessServiceFactory();
    Reference< XNameAccess > xNameAccess( rFactory->createInstance(
        DEFINE_CONST_UNICODE( "com.sun.star.frame.UICommandDescription" )), UNO_QUERY );

    rtl::OUString aWriterDoc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.text.TextDocument" ));
    rtl::OUString aCalcDoc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.SpreadsheetDocument" ));
    rtl::OUString aDrawDoc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ));
    rtl::OUString aImpressDoc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ));

    // preload commands configuration
    if ( xNameAccess.is() )
    {
        Any a;
        Reference< XNameAccess > xCmdAccess;

        try
        {
            a = xNameAccess->getByName( aWriterDoc );
            a >>= xCmdAccess;
            if ( xCmdAccess.is() )
            {
                xCmdAccess->getByName( DEFINE_CONST_UNICODE( ".uno:BasicShapes" ));
                xCmdAccess->getByName( DEFINE_CONST_UNICODE( ".uno:EditGlossary" ));
            }
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }

        try
        {
            a = xNameAccess->getByName( aCalcDoc );
            a >>= xCmdAccess;
            if ( xCmdAccess.is() )
                xCmdAccess->getByName( DEFINE_CONST_UNICODE( ".uno:InsertObjectStarMath" ));
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }

        try
        {
            // draw and impress share the same configuration file (DrawImpressCommands.xcu)
            a = xNameAccess->getByName( aDrawDoc );
            a >>= xCmdAccess;
            if ( xCmdAccess.is() )
                xCmdAccess->getByName( DEFINE_CONST_UNICODE( ".uno:Polygon" ));
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload window state configuration
    xNameAccess = Reference< XNameAccess >( rFactory->createInstance(
                    DEFINE_CONST_UNICODE( "com.sun.star.ui.WindowStateConfiguration" )), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        Any a;
        Reference< XNameAccess > xWindowAccess;
        try
        {
            a = xNameAccess->getByName( aWriterDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( DEFINE_CONST_UNICODE( "private:resource/toolbar/standardbar" ));
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
        try
        {
            a = xNameAccess->getByName( aCalcDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( DEFINE_CONST_UNICODE( "private:resource/toolbar/standardbar" ));
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
        try
        {
            a = xNameAccess->getByName( aDrawDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( DEFINE_CONST_UNICODE( "private:resource/toolbar/standardbar" ));
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
        try
        {
            a = xNameAccess->getByName( aImpressDoc );
            a >>= xWindowAccess;
            if ( xWindowAccess.is() )
                xWindowAccess->getByName( DEFINE_CONST_UNICODE( "private:resource/toolbar/standardbar" ));
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload user interface element factories
    Sequence< Sequence< css::beans::PropertyValue > > aSeqSeqPropValue;
    Reference< ::com::sun::star::ui::XUIElementFactoryRegistration > xUIElementFactory(
        rFactory->createInstance(
            DEFINE_CONST_UNICODE( "com.sun.star.ui.UIElementFactoryManager" )),
            UNO_QUERY );
    if ( xUIElementFactory.is() )
    {
        try
        {
            aSeqSeqPropValue = xUIElementFactory->getRegisteredFactories();
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload popup menu controller factories. As all controllers are in the same
    // configuration file they also get preloaded!
    Reference< ::com::sun::star::frame::XUIControllerRegistration > xPopupMenuControllerFactory(
        rFactory->createInstance(
            DEFINE_CONST_UNICODE( "com.sun.star.frame.PopupMenuControllerFactory" )),
            UNO_QUERY );
    if ( xPopupMenuControllerFactory.is() )
    {
        try
        {
            xPopupMenuControllerFactory->hasController(
                        DEFINE_CONST_UNICODE( ".uno:CharFontName" ),
                        OUString() );
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload filter configuration
    Sequence< OUString > aSeq;
    xNameAccess = Reference< XNameAccess >( rFactory->createInstance(
                    DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" )), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        try
        {
             aSeq = xNameAccess->getElementNames();
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    // preload type detection configuration
    xNameAccess = Reference< XNameAccess >( rFactory->createInstance(
                    DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" )), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        try
        {
             aSeq = xNameAccess->getElementNames();
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
        }
    }

    static const OUString sConfigSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) );
    static const OUString sAccessSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" ) );

    // get configuration provider
    Reference< XMultiServiceFactory > xConfigProvider;
    xConfigProvider = Reference< XMultiServiceFactory > (
                rFactory->createInstance( sConfigSrvc ),UNO_QUERY );

    if ( xConfigProvider.is() )
    {
        // preload writer configuration
        Sequence< Any > theArgs(1);
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.Writer/MailMergeWizard" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // WriterWeb
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.WriterWeb/Content" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // preload compatibility
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.Compatibility/WriterCompatibilityVersion" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // preload calc configuration
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.Calc/Content" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // preload impress configuration
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.UI.Effects/UserInterface" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.Impress/Layout" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // preload draw configuration
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.Draw/Layout" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // preload ui configuration
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.UI/FilterClassification" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }

        // preload addons configuration
        theArgs[ 0 ] <<= OUString::createFromAscii( "org.openoffice.Office.Addons/AddonUI" );
        try
        {
            xNameAccess = Reference< XNameAccess >(
                xConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY );
        }
        catch (::com::sun::star::uno::Exception& )
        {
        }
    }
}

void Desktop::OpenClients()
{

    // check if a document has been recovered - if there is one of if a document was loaded by cmdline, no default document
    // should be created
    Reference < XComponent > xFirst;
    BOOL bLoaded = FALSE;

    CommandLineArgs* pArgs = GetCommandLineArgs();
    SvtInternalOptions  aInternalOptions;

    Reference<XMultiServiceFactory> rFactory = ::comphelper::getProcessServiceFactory();

    if (!pArgs->IsQuickstart()) {
        sal_Bool bShowHelp = sal_False;
        ::rtl::OUStringBuffer aHelpURLBuffer;
        if (pArgs->IsHelpWriter()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://swriter/start");
        } else if (pArgs->IsHelpCalc()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://scalc/start");
        } else if (pArgs->IsHelpDraw()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://sdraw/start");
        } else if (pArgs->IsHelpImpress()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://simpress/start");
        } else if (pArgs->IsHelpBase()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://sdatabase/start");
        } else if (pArgs->IsHelpBasic()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://sbasic/start");
        } else if (pArgs->IsHelpMath()) {
            bShowHelp = sal_True;
            aHelpURLBuffer.appendAscii("vnd.sun.star.help://smath/start");
        }
        if (bShowHelp) {
            Help *pHelp = Application::GetHelp();

            Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
            rtl::OUString aTmp;
            aRet >>= aTmp;
            aHelpURLBuffer.appendAscii("?Language=");
            aHelpURLBuffer.append(aTmp);
#if defined UNX
            aHelpURLBuffer.appendAscii("&System=UNX");
#elif defined WNT
            aHelpURLBuffer.appendAscii("&System=WIN");
#elif defined MAC
            aHelpURLBuffer.appendAscii("&System=MAC");
#elif defined OS2
            aHelpURLBuffer.appendAscii("&System=OS2");
#endif
            pHelp->Start(aHelpURLBuffer.makeStringAndClear(), NULL);
            return;
        }
    }
    else
    {
        OUString            aIniName;
        ::vos::OStartupInfo aInfo;

        aInfo.getExecutableFile( aIniName );
        sal_uInt32     lastIndex = aIniName.lastIndexOf('/');
        if ( lastIndex > 0 )
        {
            aIniName    = aIniName.copy( 0, lastIndex+1 );
            aIniName    += OUString( RTL_CONSTASCII_USTRINGPARAM( "perftune" ));
#if defined(WNT) || defined(OS2)
            aIniName    += OUString( RTL_CONSTASCII_USTRINGPARAM( ".ini" ));
#else
            aIniName    += OUString( RTL_CONSTASCII_USTRINGPARAM( "rc" ));
#endif
        }

        rtl::Bootstrap aPerfTuneIniFile( aIniName );

        OUString aDefault( RTL_CONSTASCII_USTRINGPARAM( "0" ));
        OUString aPreloadData;

        aPerfTuneIniFile.getFrom( OUString( RTL_CONSTASCII_USTRINGPARAM( "QuickstartPreloadConfiguration" )), aPreloadData, aDefault );
        if ( aPreloadData.equalsAscii( "1" ))
        {
            if ( pArgs->IsWriter()  ||
                 pArgs->IsCalc()    ||
                 pArgs->IsDraw()    ||
                 pArgs->IsImpress()    )
            {
                PreloadModuleData( pArgs );
            }

            PreloadConfigurationData();
        }
    }

    // Disable AutoSave feature in case "-norestore" or a similare command line switch is set on the command line.
    // The reason behind: AutoSave/EmergencySave/AutoRecovery share the same data.
    // But the require that all documents, which are saved as backup should exists inside
    // memory. May be this mechanism will be inconsistent if the configuration exists ...
    // but no document inside memory corrspond to this data.
    // Furter it's not acceptable to recover such documents without any UI. It can
    // need some time, where the user wont see any results and wait for finishing the office startup ...
    sal_Bool bAllowRecoveryAndSessionManagement = (
                                                    ( !pArgs->IsNoRestore() ) &&
                                                    ( !pArgs->IsHeadless()  ) &&
                                                    ( !pArgs->IsServer()    )
                                                  );

    if ( ! bAllowRecoveryAndSessionManagement )
    {
     /*
        ::comphelper::ConfigurationHelper::writeDirectKey(
                ::comphelper::getProcessServiceFactory(),
                ::rtl::OUString::createFromAscii("org.openoffice.Office.Recovery"),
                ::rtl::OUString::createFromAscii("AutoSave"),
                ::rtl::OUString::createFromAscii("Enabled"),
                ::com::sun::star::uno::makeAny(sal_False),
                ::comphelper::ConfigurationHelper::E_STANDARD);

        */
        try
        {
            Reference< XDispatch > xRecovery(
                    ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.AutoRecovery")) ),
                    ::com::sun::star::uno::UNO_QUERY_THROW );

            Reference< XURLTransformer > xParser(
                    ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer")) ),
                    ::com::sun::star::uno::UNO_QUERY_THROW );

            css::util::URL aCmd;
            aCmd.Complete = ::rtl::OUString::createFromAscii("vnd.sun.star.autorecovery:/disableRecovery");
            xParser->parseStrict(aCmd);

            xRecovery->dispatch(aCmd, css::uno::Sequence< css::beans::PropertyValue >());
        }
        catch(const css::uno::Exception& e)
        {
            OUString aMessage = OUString::createFromAscii("Could not disable AutoRecovery.\n")
                + e.Message;
            OSL_ENSURE(sal_False, OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
        }
    }
    else
    {
        sal_Bool bCrashed            = sal_False;
        sal_Bool bExistsRecoveryData = sal_False;
        sal_Bool bExistsSessionData  = sal_False;

        impl_checkRecoveryState(bCrashed, bExistsRecoveryData, bExistsSessionData);

        if ( !getenv ("OOO_DISABLE_RECOVERY") &&
            ( ! bLoaded ) &&
            (
                ( bExistsRecoveryData ) || // => crash with files    => recovery
                ( bCrashed            )    // => crash without files => error report
            )
           )
        {
            try
            {
                impl_callRecoveryUI(
                    sal_False          , // false => force recovery instead of emergency save
                    bCrashed           ,
                    bExistsRecoveryData);
                /* TODO we cant be shure, that at least one document could be recovered here successfully
                    So we set bLoaded=TRUE to supress opening of the default document.
                    But we should make it more safe. Otherwhise we have an office without an UI ...
                    ...
                    May be we can check the desktop if some documents are existing there.
                 */
                Reference< XFramesSupplier > xTasksSupplier(
                        ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                        ::com::sun::star::uno::UNO_QUERY_THROW );
                Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY_THROW );
                if ( xList->hasElements() )
                    bLoaded = sal_True;
            }
            catch(const css::uno::Exception& e)
            {
                OUString aMessage = OUString::createFromAscii("Error during recovery\n")
                    + e.Message;
                OSL_ENSURE(sal_False, OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
            }
        }

        Reference< XInitialization > xSessionListener;
        try
        {
            xSessionListener = Reference< XInitialization >(::comphelper::getProcessServiceFactory()->createInstance(
                        OUString::createFromAscii("com.sun.star.frame.SessionListener")), UNO_QUERY_THROW);
            xSessionListener->initialize(Sequence< Any >(0));
        }
        catch(const com::sun::star::uno::Exception& e)
        {
            OUString aMessage = OUString::createFromAscii("Registration of session listener failed\n")
                + e.Message;
            OSL_ENSURE(sal_False, OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
        }

        if (
            ( ! bLoaded            ) &&
            (   bExistsSessionData )
           )
        {
            // session management
            try
            {
                Reference< XSessionManagerListener > r(xSessionListener, UNO_QUERY_THROW);
                bLoaded = r->doRestore();
            }
            catch(const com::sun::star::uno::Exception& e)
            {
                OUString aMessage = OUString::createFromAscii("Error in session management\n")
                    + e.Message;
                OSL_ENSURE(sal_False, OUStringToOString(aMessage, RTL_TEXTENCODING_ASCII_US).getStr());
            }
        }
    }

    sal_Bool bShutdown( sal_False );
    if ( !pArgs->IsServer() )
    {
        ProcessDocumentsRequest aRequest;
        aRequest.pcProcessed = NULL;

        pArgs->GetOpenList( aRequest.aOpenList );
        pArgs->GetViewList( aRequest.aViewList );
        pArgs->GetStartList( aRequest.aStartList );
        pArgs->GetPrintList( aRequest.aPrintList );
        pArgs->GetPrintToList( aRequest.aPrintToList );
        pArgs->GetPrinterName( aRequest.aPrinterName );
        pArgs->GetForceOpenList( aRequest.aForceOpenList );
        pArgs->GetForceNewList( aRequest.aForceNewList );

        if ( aRequest.aOpenList.getLength() > 0 ||
             aRequest.aViewList.getLength() > 0 ||
             aRequest.aStartList.getLength() > 0 ||
             aRequest.aPrintList.getLength() > 0 ||
             aRequest.aForceOpenList.getLength() > 0 ||
             aRequest.aForceNewList.getLength() > 0 ||
             ( aRequest.aPrintToList.getLength() > 0 && aRequest.aPrinterName.getLength() > 0 ))
        {
            bLoaded = sal_True;

            if ( pArgs->HasModuleParam() )
            {
                SvtModuleOptions    aOpt;

                // Support command line parameters to start a module (as preselection)
                if ( pArgs->IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
                    aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::E_WRITER );
                else if ( pArgs->IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
                    aRequest.aModule = aOpt.GetFactoryName( SvtModuleOptions::E_CALC );
                else if ( pArgs->IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
                    aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::E_IMPRESS );
                else if ( pArgs->IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
                    aRequest.aModule= aOpt.GetFactoryName( SvtModuleOptions::E_DRAW );
            }

            // check for printing disabled
            if( ( aRequest.aPrintList.getLength() || aRequest.aPrintToList.getLength() )
                && Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
            {
                aRequest.aPrintList = rtl::OUString();
                aRequest.aPrintToList = rtl::OUString();
                ResMgr* pDtResMgr = GetDesktopResManager();
                if( pDtResMgr )
                {
                    ErrorBox aBox( NULL, ResId( EBX_ERR_PRINTDISABLED, *pDtResMgr ) );
                    aBox.Execute();
                }
            }

            // Process request
            bShutdown = OfficeIPCThread::ExecuteCmdLineRequests( aRequest );
        }
    }

    // Don't do anything if we have successfully called terminate at desktop
    if ( bShutdown )
        return;

    // no default document if a document was loaded by recovery or by command line or if soffice is used as server
    Reference< XFramesSupplier > xTasksSupplier(
            ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
            ::com::sun::star::uno::UNO_QUERY_THROW );
    Reference< XElementAccess > xList( xTasksSupplier->getFrames(), UNO_QUERY_THROW );
    if ( xList->hasElements() || pArgs->IsServer() )
        return;

    if ( pArgs->IsQuickstart() || pArgs->IsInvisible() || pArgs->IsBean() || Application::AnyInput( INPUT_APPEVENT ) )
        // soffice was started as tray icon ...
        return;
    {
        OpenDefault();
    }
}

void Desktop::OpenDefault()
{

    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::Desktop::OpenDefault" );

    ::rtl::OUString        aName;
    SvtModuleOptions    aOpt;

    CommandLineArgs* pArgs = GetCommandLineArgs();
    if ( pArgs->IsNoDefault() ) return;
    if ( pArgs->HasModuleParam() )
    {
        // Support new command line parameters to start a module
        if ( pArgs->IsWriter() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITER );
        else if ( pArgs->IsCalc() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_CALC );
        else if ( pArgs->IsImpress() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_IMPRESS );
        else if ( pArgs->IsBase() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DATABASE );
        else if ( pArgs->IsDraw() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DRAW );
        else if ( pArgs->IsMath() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_MATH );
        else if ( pArgs->IsGlobal() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITERGLOBAL );
        else if ( pArgs->IsWeb() && aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITERWEB );
    }

    if ( !aName.getLength() )
    {
        // Old way to create a default document
        if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_WRITER );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_CALC );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_IMPRESS );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DATABASE );
        else if ( aOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
            aName = aOpt.GetFactoryEmptyDocumentURL( SvtModuleOptions::E_DRAW );
        else
            return;
    }

    ProcessDocumentsRequest aRequest;
    aRequest.pcProcessed = NULL;
    aRequest.aOpenList   = aName;
    OfficeIPCThread::ExecuteCmdLineRequests( aRequest );
}


String GetURL_Impl( const String& rName )
{
    // if rName is a vnd.sun.star.script URL do not attempt to parse it
    // as INetURLObj does not handle handle there URLs
    if (rName.CompareToAscii("vnd.sun.star.script" , 19) == COMPARE_EQUAL)
    {
        return rName;
    }

    // dont touch file urls, those should already be in internal form
    // they won't get better here (#112849#)
    if (rName.CompareToAscii("file:" , 5) == COMPARE_EQUAL)
    {
        return rName;
    }

    if ( rName.CompareToAscii("service:" , 8) == COMPARE_EQUAL )
    {
        return rName;
    }

    // if the filename is a physical name, it is the client file system, not the file system
    // of the machine where the office is running ( if this are different machines )
    // so in the remote case we can't handle relative filenames as arguments, because they
    // are parsed relative to the program path
    // the file system of the client is addressed through the "file:" protocol

    // Get current working directory to support relativ pathes
    ::rtl::OUString aWorkingDir;
    osl_getProcessWorkingDir( &aWorkingDir.pData );

    // Add path seperator to these directory and make given URL (rName) absolute by using of current working directory
    // Attention: "setFianlSlash()" is neccessary for calling "smartRel2Abs()"!!!
    // Otherwhise last part will be ignored and wrong result will be returned!!!
    // "smartRel2Abs()" interpret given URL as file not as path. So he truncate last element to get the base path ...
    // But if we add a seperator - he doesn't do it anymore.
    INetURLObject aObj( aWorkingDir );
    aObj.setFinalSlash();

    // Use the provided parameters for smartRel2Abs to support the usage of '%' in system paths.
    // Otherwise this char won't get encoded and we are not able to load such files later,
    // see #110156#
    bool bWasAbsolute;
    INetURLObject aURL     = aObj.smartRel2Abs( rName, bWasAbsolute, false, INetURLObject::WAS_ENCODED,
                                                RTL_TEXTENCODING_UTF8, true );
    String        aFileURL = aURL.GetMainURL(INetURLObject::NO_DECODE);

    ::osl::FileStatus aStatus( FileStatusMask_FileURL );
    ::osl::DirectoryItem aItem;
    if( ::osl::FileBase::E_None == ::osl::DirectoryItem::get( aFileURL, aItem ) &&
        ::osl::FileBase::E_None == aItem.getFileStatus( aStatus ) )
            aFileURL = aStatus.getFileURL();

    return aFileURL;
}

void Desktop::HandleAppEvent( const ApplicationEvent& rAppEvent )
{
    if ( rAppEvent.GetEvent() == "APPEAR" && !GetCommandLineArgs()->IsInvisible() )
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

        // find active task - the active task is always a visible task
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >
                xDesktop( xSMGR->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xTask = xDesktop->getActiveFrame();
        if ( !xTask.is() )
        {
            // get any task if there is no active one
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > xList( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
            if ( xList->getCount()>0 )
                xList->getByIndex(0) >>= xTask;
        }

        if ( xTask.is() )
        {
            Reference< com::sun::star::awt::XTopWindow > xTop( xTask->getContainerWindow(), UNO_QUERY );
            xTop->toFront();
        }
        else
        {
            // no visible task that could be activated found
            Reference< XFrame > xBackingFrame;
            Reference< ::com::sun::star::awt::XWindow > xContainerWindow;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xDesktopFrame( xDesktop, UNO_QUERY );

            xBackingFrame = xDesktopFrame->findFrame(OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" )), 0);
            if (xBackingFrame.is())
                xContainerWindow = xBackingFrame->getContainerWindow();
            if (xContainerWindow.is())
            {
                Sequence< Any > lArgs(1);
                lArgs[0] <<= xContainerWindow;
                Reference< XController > xBackingComp(
                    xSMGR->createInstanceWithArguments(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.StartModule") ), lArgs),
                    UNO_QUERY);
                if (xBackingComp.is())
                {
                    Reference< ::com::sun::star::awt::XWindow > xBackingWin(xBackingComp, UNO_QUERY);
                    // Attention: You MUST(!) call setComponent() before you call attachFrame().
                    // Because the backing component set the property "IsBackingMode" of the frame
                    // to true inside attachFrame(). But setComponent() reset this state everytimes ...
                    xBackingFrame->setComponent(xBackingWin, xBackingComp);
                    xBackingComp->attachFrame(xBackingFrame);
                    xContainerWindow->setVisible(sal_True);

                    Window* pCompWindow = VCLUnoHelper::GetWindow(xBackingFrame->getComponentWindow());
                    if (pCompWindow)
                        pCompWindow->Update();
                }
            }
        }
    }
    else if ( rAppEvent.GetEvent() == "QUICKSTART" && !GetCommandLineArgs()->IsInvisible()  )
    {
        // If the office has been started the second time its command line arguments are sent through a pipe
        // connection to the first office. We want to reuse the quickstart option for the first office.
        // NOTICE: The quickstart service must be initialized inside the "main thread", so we use the
        // application events to do this (they are executed inside main thread)!!!
        // Don't start quickstart service if the user specified "-invisible" on the command line!
        sal_Bool bQuickstart( sal_True );
        Sequence< Any > aSeq( 1 );
        aSeq[0] <<= bQuickstart;

        Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                                            DEFINE_CONST_UNICODE( "com.sun.star.office.Quickstart" )),
                                            UNO_QUERY );
        if ( xQuickstart.is() )
            xQuickstart->initialize( aSeq );
    }
    else if ( rAppEvent.GetEvent() == "ACCEPT" )
    {
        // every time an accept parameter is used we create an acceptor
        // with the corresponding accept-string
        OUString aAcceptString(rAppEvent.GetData().GetBuffer());
        createAcceptor(aAcceptString);
    }
    else if ( rAppEvent.GetEvent() == "UNACCEPT" )
    {
        // try to remove corresponding acceptor
        OUString aUnAcceptString(rAppEvent.GetData().GetBuffer());
        destroyAcceptor(aUnAcceptString);
    }
    else if ( rAppEvent.GetEvent() == "SaveDocuments" )
    {
        Desktop::_bTasksSaved = sal_False;
        Desktop::_bTasksSaved = SaveTasks();
    }
    else if ( rAppEvent.GetEvent() == "OPENHELPURL" )
    {
        // start help for a specific URL
        OUString aHelpURL(rAppEvent.GetData().GetBuffer());
        Help *pHelp = Application::GetHelp();
        pHelp->Start(aHelpURL, NULL);
    }
    else if ( rAppEvent.GetEvent() == APPEVENT_OPEN_STRING )
    {
        OUString aOpenURL(rAppEvent.GetData().GetBuffer());

        CommandLineArgs* pCmdLine = GetCommandLineArgs();
        if ( !pCmdLine->IsInvisible() && !pCmdLine->IsTerminateAfterInit() )
        {
            ProcessDocumentsRequest* pDocsRequest = new ProcessDocumentsRequest;
            pDocsRequest->aOpenList = aOpenURL;
            pDocsRequest->pcProcessed = NULL;

            OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
            delete pDocsRequest;
        }
    }
    else if ( rAppEvent.GetEvent() == APPEVENT_PRINT_STRING )
    {
        OUString aPrintURL(rAppEvent.GetData().GetBuffer());

        CommandLineArgs* pCmdLine = GetCommandLineArgs();
        if ( !pCmdLine->IsInvisible() && !pCmdLine->IsTerminateAfterInit() )
        {
            ProcessDocumentsRequest* pDocsRequest = new ProcessDocumentsRequest;
            pDocsRequest->aPrintList = aPrintURL;
            pDocsRequest->pcProcessed = NULL;

            OfficeIPCThread::ExecuteCmdLineRequests( *pDocsRequest );
            delete pDocsRequest;
        }
    }
#ifndef UNX
    else if ( rAppEvent.GetEvent() == "HELP" )
    {
        // in non unix version allow showing of cmdline help window
        displayCmdlineHelp();
    }
#endif
    else if ( rAppEvent.GetEvent() == "SHOWDIALOG" )
    {
        // ignore all errors here. It's clicking a menu entry only ...
        // The user will try it again, in case nothing happens .-)
        try
        {
            css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();

            com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >
                xDesktop( xSMGR->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                ::com::sun::star::uno::UNO_QUERY );

            // check provider ... we know it's weak reference only
            if ( ! xDesktop.is())
                return;

            css::uno::Reference< css::util::XURLTransformer > xParser(xSMGR->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))), css::uno::UNO_QUERY_THROW);
            css::util::URL aCommand;
            if( rAppEvent.GetData().EqualsAscii( "PREFERENCES" ) )
                aCommand.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:OptionsTreeDialog" ) );
            else if( rAppEvent.GetData().EqualsAscii( "ABOUT" ) )
                aCommand.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:About" ) );
            if( aCommand.Complete.getLength() )
            {
                xParser->parseStrict(aCommand);

                css::uno::Reference< css::frame::XDispatch > xDispatch = xDesktop->queryDispatch(aCommand, rtl::OUString(), 0);
                if (xDispatch.is())
                    xDispatch->dispatch(aCommand, css::uno::Sequence< css::beans::PropertyValue >());
            }
        }
        catch(const css::uno::Exception&)
        {}
    }
}

void Desktop::OpenSplashScreen()
{
    ::rtl::OUString     aTmpString;
    CommandLineArgs*    pCmdLine = GetCommandLineArgs();
    sal_Bool bVisible = sal_False;
    // Show intro only if this is normal start (e.g. no server, no quickstart, no printing )
    if ( !pCmdLine->IsInvisible() &&
         !pCmdLine->IsQuickstart() &&
         !pCmdLine->IsMinimized() &&
         !pCmdLine->IsNoLogo() &&
         !pCmdLine->IsTerminateAfterInit() &&
         !pCmdLine->GetPrintList( aTmpString ) &&
         !pCmdLine->GetPrintToList( aTmpString ) )
    {
        // Determine application name from command line parameters
        OUString aAppName;
        if ( pCmdLine->IsWriter() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "writer" ));
        else if ( pCmdLine->IsCalc() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "calc" ));
        else if ( pCmdLine->IsDraw() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "draw" ));
        else if ( pCmdLine->IsImpress() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "impress" ));
        else if ( pCmdLine->IsBase() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "base" ));
        else if ( pCmdLine->IsGlobal() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "global" ));
        else if ( pCmdLine->IsMath() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "math" ));
        else if ( pCmdLine->IsWeb() )
            aAppName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "web" ));

        bVisible = sal_True;
        Sequence< Any > aSeq( 2 );
        aSeq[0] <<= bVisible;
        aSeq[1] <<= aAppName;
        m_rSplashScreen = Reference<XStatusIndicator>(
            comphelper::getProcessServiceFactory()->createInstanceWithArguments(
            OUString::createFromAscii("com.sun.star.office.SplashScreen"),
            aSeq), UNO_QUERY);

        if(m_rSplashScreen.is())
                m_rSplashScreen->start(OUString::createFromAscii("SplashScreen"), 100);
    }

}

void Desktop::SetSplashScreenProgress(sal_Int32 iProgress)
{
    if(m_rSplashScreen.is())
    {
        m_rSplashScreen->setValue(iProgress);
    }
}

void Desktop::CloseSplashScreen()
{
    if(m_rSplashScreen.is())
    {
        m_rSplashScreen->end();
        m_rSplashScreen = NULL;
    }
}

// ========================================================================
void Desktop::DoFirstRunInitializations()
{
    try
    {
        Reference< XJobExecutor > xExecutor( ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.task.JobExecutor" ) ), UNO_QUERY );
        if( xExecutor.is() )
            xExecutor->trigger( ::rtl::OUString::createFromAscii("onFirstRunInitialization") );
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        OSL_ENSURE( sal_False, "Desktop::DoFirstRunInitializations: caught an exception while trigger job executor ..." );
    }
}

// ========================================================================
void Desktop::CheckFirstRun( )
{
    const ::rtl::OUString sCommonMiscNodeName = ::rtl::OUString::createFromAscii( "/org.openoffice.Office.Common/Misc" );
    const ::rtl::OUString sFirstRunNodeName = ::rtl::OUString::createFromAscii( "FirstRun" );

    // --------------------------------------------------------------------
    // check if this is the first office start

    // for this, open the Common/Misc node where this info is stored
    ::utl::OConfigurationTreeRoot aCommonMisc = ::utl::OConfigurationTreeRoot::createWithServiceFactory(
        ::comphelper::getProcessServiceFactory( ),
        sCommonMiscNodeName,
        2,
        ::utl::OConfigurationTreeRoot::CM_UPDATABLE
    );

    // read the flag
    OSL_ENSURE( aCommonMisc.isValid(), "Desktop::CheckFirstRun: could not open the config node needed!" );
    sal_Bool bIsFirstRun = sal_False;
    aCommonMisc.getNodeValue( sFirstRunNodeName ) >>= bIsFirstRun;

    if ( !bIsFirstRun )
        // nothing to do ....
        return;

    // --------------------------------------------------------------------
    // it is the first run
    // this has once been done using a vos timer. this could lead to problems when
    // the timer would trigger when the app is already going down again, since VCL would
    // no longer be available. Since the old handler would do a postUserEvent to the main
    // thread anyway, we can use a vcl timer here to prevent the race contition (#107197#)
    m_firstRunTimer.SetTimeout(3000); // 3 sec.
    m_firstRunTimer.SetTimeoutHdl(LINK(this, Desktop, AsyncInitFirstRun));
    m_firstRunTimer.Start();

    // --------------------------------------------------------------------
    // reset the config flag

    // set the value
    aCommonMisc.setNodeValue( sFirstRunNodeName, makeAny( (sal_Bool)sal_False ) );
    // commit the changes
    aCommonMisc.commit();
}

}
