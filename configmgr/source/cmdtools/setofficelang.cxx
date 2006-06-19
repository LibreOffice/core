/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: setofficelang.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:21:43 $
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

#include <stdio.h>

#include "unomain.hxx"

#include <osl/thread.h>
#include <rtl/ustring.hxx>

//=============================================================================
const int OPERATION_SUCCEEDED       = 0;
const int ERROR_LANGUAGE_NOT_SUPPORTED  = 1;
const int ERROR_INVALID_ARGS        = 2;
const int ERROR_RUNTIME_FAILURE     = 3;
//=============================================================================

rtl::OUString const k_BaseSettingsPath( RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Setup"));
rtl::OUString const k_AppSettingsPath( RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Linguistic/General"));
rtl::OUString const k_LocaleListSetting( RTL_CONSTASCII_USTRINGPARAM("Office/InstalledLocales") );
rtl::OUString const k_LocaleSetting( RTL_CONSTASCII_USTRINGPARAM("L10N/ooLocale") );
rtl::OUString const k_UILocaleSetting( RTL_CONSTASCII_USTRINGPARAM("UILocale") );

//=============================================================================
static inline rtl::OString narrow(rtl::OUString const & aString)
{
    return rtl::OUStringToOString( aString, osl_getThreadTextEncoding() );
}
//=============================================================================
static void usage()
{
    rtl::OString sApp = narrow( unoapp::getAppCommandPath() );

    fprintf(stderr, "%s - set the user interface language "
            "of a OpenOffice.org/StarOffice/StarSuite "
            "installation\n", sApp.getStr());
    fprintf(stderr, "\nusage : %s [-afcpl] <language> ..\n", sApp.getStr());
    fprintf(stderr, "\n  Options:"
                    "\n      -a       Change the default language of the installation. "
                    "\n               This may require administrative rights. "
                    "\n      -f       Force selection of language. "
                    "\n               Skips the check if this language is installed. "
                    "\n      -c       Display current active language. "
                    "\n      -p       Display current preferred language. "
                    "\n      -l       List all installed languages. ");
    fprintf(stderr, "\n  <language> - A language specified by its ISO code (e.g. 'en-US','de') or '-' for default. "
                    "\n               This language must be supported by the office installation\n"
                    "\n               If multiple languages are listed, the first one that is installed is used.\n");
    fprintf(stderr, "\nResult codes:  0 - Setting language successfull"
                    "\n               1 - The language is not supported by this office installation"
                    "\n               2 - Invalid arguments"
                    "\n               3 - An internal error occurred\n");
    fflush(stdout);
}

//---------------------------------------------------------------------------

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Sequence.hxx>
namespace uno = ::com::sun::star::uno;
namespace lang = ::com::sun::star::lang;
namespace util = ::com::sun::star::util;
namespace beans = ::com::sun::star::beans;
namespace container = ::com::sun::star::container;
using rtl::OUString;
//---------------------------------------------------------------------------

static uno::Reference< lang::XMultiServiceFactory >  createProvider(uno::Reference< uno::XComponentContext > const & xContext, bool bAdmin=false);
static uno::Reference< uno::XInterface > createView(uno::Reference< lang::XMultiServiceFactory > const & xProvider, OUString const & aNodepath, bool bUpdate = false);
static uno::Reference< util::XChangesBatch > createUpdateView(uno::Reference< lang::XMultiServiceFactory > const & xProvider, OUString const & aNodepath);
static void disposeComponent(uno::BaseReference const & xComp);
// --------------------------------------------------------------------------

int SAL_CALL unoapp::uno_main(uno::Reference< uno::XComponentContext > const & xContext)
        SAL_THROW( (uno::Exception) )
{
    // get args
    uno::Sequence< OUString > const aArgs( getAppCommandArgs() );

    sal_Int32 const nArgC = aArgs.getLength();

    sal_Int32 ix = 0;

    // get options
    bool bAdminMode = false;
    bool bForce = false;
    bool bShowCurrentLang = false;
    bool bShowSelectedLang = false;
    bool bListInstalledLangs = false;

    bool bInfoRequest = false;

    while (ix < nArgC && aArgs[ix].matchAsciiL("-",1))
    {
        sal_Int32 const nOptCount = aArgs[ix].getLength() -1;
        if (nOptCount <= 0)
        {
            // a lone "-" means set-to-default-locale
            break;
        }

        sal_Unicode const * const pOpts = aArgs[ix].getStr() + 1;
        for (sal_Int32 opt = 0; opt < nOptCount; ++opt)
        {
            switch (pOpts[opt])
            {
            case 'a': bAdminMode = true; break;
            case 'f': bForce = true; break;
            case 'c': bShowCurrentLang = true; bInfoRequest = true; break;
            case 'p': bShowSelectedLang = true; bInfoRequest = true; break;
            case 'l': bListInstalledLangs = true; bInfoRequest = true; break;
            default:
                usage();
                return ERROR_INVALID_ARGS;
            }
        }

        ++ix;
    }

    if ((nArgC <= ix) && !bInfoRequest)
    {
        // no more remaining args and no info request
        usage();
        return ERROR_INVALID_ARGS;
    }

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xProvider = createProvider(xContext);
        uno::Reference< lang::XMultiServiceFactory > xAppProvider = createProvider(xContext,bAdminMode);
        OSL_ASSERT( xProvider.is());
        OSL_ASSERT( xAppProvider.is());
        uno::Reference< container::XHierarchicalNameAccess > xBaseSettings(
                                    createView(xProvider,k_BaseSettingsPath), uno::UNO_QUERY_THROW );

        if (bShowCurrentLang)
        {
            rtl::OUString aCurrentLocaleU;
            if (!(xBaseSettings->getByHierarchicalName(k_LocaleSetting) >>= aCurrentLocaleU) )
                return ERROR_RUNTIME_FAILURE;

            rtl::OString aCurrentLocale = ::rtl::OUStringToOString(aCurrentLocaleU, RTL_TEXTENCODING_ASCII_US);
            printf("%s\n", aCurrentLocale.getStr());
        }
        if (bShowSelectedLang)
        {
            uno::Reference< container::XNameAccess > xAppSettings( createView(xAppProvider,k_AppSettingsPath), uno::UNO_QUERY_THROW );
            rtl::OUString aSelectedLocaleU;
            if (!(xAppSettings->getByName(k_UILocaleSetting) >>= aSelectedLocaleU) )
                return ERROR_RUNTIME_FAILURE;

            rtl::OString aSelectedLocale = ::rtl::OUStringToOString(aSelectedLocaleU, RTL_TEXTENCODING_ASCII_US);
            if (aSelectedLocale.getLength() == 0) aSelectedLocale="-";
            printf("%s\n", aSelectedLocale.getStr());

            disposeComponent(xAppSettings);
        }

        uno::Reference< container::XNameAccess > xSupportedLocales;
        xBaseSettings->getByHierarchicalName(k_LocaleListSetting) >>= xSupportedLocales;

        if (bListInstalledLangs)
        {
            if (!xSupportedLocales.is())
                return ERROR_RUNTIME_FAILURE;

            uno::Sequence< OUString > aSupportedLocales = xSupportedLocales->getElementNames();
            for (sal_Int32 i=0; i<aSupportedLocales.getLength(); ++i)
            {
                rtl::OString aLocale = ::rtl::OUStringToOString(aSupportedLocales[i], RTL_TEXTENCODING_ASCII_US);
                printf("%s\n", aLocale.getStr());
            }
        }
        if (nArgC > ix)
        {
            bool bLocaleFound = false;

            uno::Reference< util::XChangesBatch > xAppView  = createUpdateView(xAppProvider,k_AppSettingsPath);
            uno::Reference< container::XNameReplace > xSettings(xAppView, uno::UNO_QUERY_THROW);

            if (!xSupportedLocales.is())
                return ERROR_RUNTIME_FAILURE;

            for ( ; ix < nArgC; ++ix)
            {
                OUString const aTargetLocale = aArgs[ix];
                if (aTargetLocale.equalsAscii("-"))
                {
                    uno::Reference< beans::XPropertyState > xSettingsState(xSettings, uno::UNO_QUERY);
                    // if we can't reset, we treat this as unsupported language
                    if (!xSettingsState.is())
                        continue;

                    bLocaleFound = true;
                    xSettingsState->setPropertyToDefault(k_UILocaleSetting);
                    xAppView->commitChanges();
                    break;
                }
                else if (bForce || xSupportedLocales->hasByName(aTargetLocale))
                {
                    bLocaleFound = true;
                    xSettings->replaceByName(k_UILocaleSetting, uno::makeAny(aTargetLocale));
                    xAppView->commitChanges();
                    break;
                }
            }

            disposeComponent(xAppView);

            if (!bLocaleFound)
                return ERROR_LANGUAGE_NOT_SUPPORTED;
        }
        disposeComponent(xBaseSettings);
        if (xAppProvider != xProvider)
            disposeComponent(xAppProvider);
        disposeComponent(xProvider);
    }
    catch (uno::Exception & e)
    {
        fprintf(stderr, "\nERROR: - An exception occurred: '%s'\n", narrow(e.Message).getStr());
        return ERROR_RUNTIME_FAILURE;
    }
    return OPERATION_SUCCEEDED;
}
// --------------------------------------------------------------------------

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <rtl/ustrbuf.hxx>
namespace lang = com::sun::star::lang;
namespace beans = com::sun::star::beans;

// --------------------------------------------------------------------------
static uno::Reference< uno::XInterface > createService(uno::Reference< uno::XComponentContext > const & xContext, OUString aService)
{

    uno::Reference< lang::XMultiComponentFactory > xFactory = xContext->getServiceManager();
    if (!xFactory.is())
    {
        rtl::OUStringBuffer sMsg;
        sMsg.appendAscii("Missing object ! ");
        sMsg.appendAscii("UNO context has no service manager.");

        throw uno::RuntimeException(sMsg.makeStringAndClear(),NULL);
    }

    uno::Reference< uno::XInterface > xInstance = xFactory->createInstanceWithContext(aService,xContext);
    if (!xInstance.is())
    {
        rtl::OUStringBuffer sMsg;
        sMsg.appendAscii("Missing service ! ");
        sMsg.appendAscii("Service manager can't instantiate service ");
        sMsg.append(aService).appendAscii(". ");

        throw lang::ServiceNotRegisteredException(sMsg.makeStringAndClear(),NULL);
    }

    return xInstance;
}

// --------------------------------------------------------------------------
static uno::Reference< lang::XMultiServiceFactory >  createProvider(uno::Reference< uno::XComponentContext > const & xContext, bool bAdmin)
{
    static const rtl::OUString kProvider(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider")) ;
    static const rtl::OUString kAdminProvider(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.AdministrationProvider")) ;

    const rtl::OUString & selectedProvider = bAdmin ? kAdminProvider : kProvider;

    uno::Reference< lang::XMultiServiceFactory > xRet( createService(xContext,selectedProvider), uno::UNO_QUERY_THROW );

    return xRet;
}
// --------------------------------------------------------------------------
static uno::Reference< uno::XInterface > createView(uno::Reference< lang::XMultiServiceFactory > const & xProvider, OUString const & aNodepath, bool bUpdate)
{
    static const OUString kInfoViewService(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")) ;
    static const OUString kUpdateViewService(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationUpdateAccess")) ;
    static const OUString kNodepath(RTL_CONSTASCII_USTRINGPARAM("nodepath")) ;
    static const OUString kAsync(RTL_CONSTASCII_USTRINGPARAM("EnableAsync")) ;

    const OUString & kViewService = bUpdate ? kUpdateViewService : kInfoViewService;
    uno::Sequence< uno::Any > aViewArgs(2);
    aViewArgs[0] <<= beans::NamedValue( kNodepath, uno::makeAny(aNodepath) );
    aViewArgs[1] <<= beans::NamedValue( kAsync, uno::makeAny(sal_False) );

    uno::Reference< uno::XInterface > xView( xProvider->createInstanceWithArguments(kViewService,aViewArgs) );
    return xView;
}
// --------------------------------------------------------------------------
//#if OSL_DEBUG_LEVEL > 0
#if 0 // currently not used in debug builds!
static uno::Reference< container::XHierarchicalNameAccess > createInfoView(uno::Reference< lang::XMultiServiceFactory > const & xProvider, OUString const & aNodepath)
{
    uno::Reference< container::XHierarchicalNameAccess > xView( createView(xProvider,aNodepath, false), uno::UNO_QUERY_THROW );
    return xView;
}
#endif
// --------------------------------------------------------------------------
static uno::Reference< util::XChangesBatch > createUpdateView(uno::Reference< lang::XMultiServiceFactory > const & xProvider, OUString const & aNodepath)
{
    uno::Reference< util::XChangesBatch > xView( createView(xProvider,aNodepath,true), uno::UNO_QUERY_THROW );
    return xView;
}

// --------------------------------------------------------------------------
static void disposeComponent(uno::BaseReference const & xComp)
{
    uno::Reference< lang::XComponent > xComponent(xComp,uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

// --------------------------------------------------------------------------


