/*************************************************************************
 *
 *  $RCSfile: bootstrap.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:30:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>

#include "bootstrap.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

// ---------------------------------------------------------------------------------------
// legacy argument names
#define ARGUMENT_LOCALE_COMPAT              "locale"
#define ARGUMENT_ASYNC_COMPAT               "lazywrite"
#define ARGUMENT_SERVERTYPE_COMPAT          "servertype"

// legacy servertype setting
#define SETTING_SERVERTYPE_COMPAT           "ServerType"
#define BOOTSTRAP_SERVERTYPE_COMPAT         CONTEXT_ITEM_PREFIX_ SETTING_SERVERTYPE_COMPAT

#define SERVERTYPE_UNO_COMPAT               "uno"
#define SERVERTYPE_PLUGIN_COMPAT            "plugin"
// ---------------------------------------------------------------------------------------

#define NAME( N ) OUString(RTL_CONSTASCII_USTRINGPARAM(N))
#define ITEM( N ) OUString(RTL_CONSTASCII_USTRINGPARAM(N))
// ---------------------------------------------------------------------------------------
// -------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
namespace configmgr
{
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
    const sal_Char k_BootstrapContextImplName[]         = "com.sun.star.comp.configuration.bootstrap.BootstrapContext" ;
    const sal_Char k_BootstrapContextServiceName[]      = "com.sun.star.configuration.bootstrap.BootstrapContext" ;

    // -------------------------------------------------------------------------
    static AsciiServiceName const k_BootstrapContextServiceNames [] =
    {
        k_BootstrapContextServiceName,
        0
    };
    static const ServiceImplementationInfo k_BootstrapContextServiceInfo =
    {
        k_BootstrapContextImplName,
        k_BootstrapContextServiceNames,
        0
    };
    static const SingletonRegistrationInfo k_BootstrapContextSingletonInfo =
    {
        A_BootstrapContextSingletonName,
        k_BootstrapContextImplName,
        k_BootstrapContextServiceName,
        0
    };
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
    uno::Reference<uno::XInterface> SAL_CALL
        instantiateBootstrapContext( CreationContext const& xTargetContext )
    {
        CreationContext xContext = UnoContextTunnel::recoverContext(xTargetContext);

        BootstrapContext * pContext = new BootstrapContext(xContext);
        CreationContext xResult(pContext);

        pContext->initialize();

        return xResult;
    }

    const SingletonRegistrationInfo * getBootstrapContextSingletonInfo()
    {
        return &k_BootstrapContextSingletonInfo;
    }
    const ServiceRegistrationInfo   * getBootstrapContextServiceInfo()
    {
        return getRegistrationInfo(&k_BootstrapContextServiceInfo);
    }
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------

    static
    inline
    cppu::ContextEntry_Init makeEntry(beans::NamedValue const & aOverride)
    {
        return cppu::ContextEntry_Init(aOverride.Name,aOverride.Value);
    }
// ---------------------------------------------------------------------------

    static
    cppu::ContextEntry_Init makeSingleton(SingletonRegistrationInfo const * pSingletonInfo)
    {
        OSL_ASSERT( pSingletonInfo &&
                    pSingletonInfo->singletonName &&
                    pSingletonInfo->instantiatedServiceName );

        rtl::OUStringBuffer aSingletonName;
        aSingletonName.appendAscii( RTL_CONSTASCII_STRINGPARAM(SINGLETON_) );
        aSingletonName.appendAscii(pSingletonInfo->singletonName);

        OUString const aServiceName   = OUString::createFromAscii(pSingletonInfo->instantiatedServiceName);

        return cppu::ContextEntry_Init(aSingletonName.makeStringAndClear(), uno::makeAny(aServiceName), true);
    }
// ---------------------------------------------------------------------------

BootstrapContext::Context BootstrapContext::createWrapper(Context const & _xContext, Overrides const & _aOverrides)
{
    std::vector< cppu::ContextEntry_Init > aContextEntries;
    aContextEntries.reserve(_aOverrides.getLength() + 5);

    // marker + bootstrap context
    aContextEntries.push_back( cppu::ContextEntry_Init(NAME(CONTEXT_ITEM_IS_WRAPPER_CONTEXT), uno::makeAny(sal_True)) );
    aContextEntries.push_back( cppu::ContextEntry_Init(NAME(CONTEXT_ITEM_IS_BOOTSTRAP_CONTEXT), uno::makeAny(sal_False)) );

    aContextEntries.push_back( makeSingleton(getBootstrapContextSingletonInfo()) );

    // singletons except for passthrough
    if (!isPassthrough(_xContext))
    {
        aContextEntries.push_back( makeSingleton(getDefaultProviderSingletonInfo()) );
        aContextEntries.push_back( makeSingleton(backend::getDefaultBackendSingletonInfo()) );
    }

    for (sal_Int32 i = 0; i<_aOverrides.getLength(); ++i)
        aContextEntries.push_back( makeEntry(_aOverrides[i]) );

    return cppu::createComponentContext(&aContextEntries.front(),aContextEntries.size(),_xContext);
}
// ---------------------------------------------------------------------------

sal_Bool BootstrapContext::isWrapper(Context const & _xContext)
{
    OSL_ASSERT(_xContext.is());
    if (!_xContext.is()) return false;

    uno::Any aSetting = _xContext->getValueByName( NAME(CONTEXT_ITEM_IS_WRAPPER_CONTEXT) );

    if (!aSetting.hasValue()) return false;

    sal_Bool bValue = false;
    OSL_VERIFY(aSetting >>= bValue);

    return bValue;
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

BootstrapContext::BootstrapContext(Context const & _xContext)
: ComponentContext(_xContext)
{
}
// ---------------------------------------------------------------------------

BootstrapContext::~BootstrapContext()
{
}
// ---------------------------------------------------------------------------

void BootstrapContext::initialize()
{
    // get default Bootstrap URL
    OUString sURL;
    uno::Any aExplicitURL;
    if ( this->lookupInContext(aExplicitURL,NAME(CONTEXT_ITEM_PREFIX_ SETTING_INIFILE)) )
    {
        OSL_VERIFY(aExplicitURL >>= sURL);
    }
    else if (!rtl::Bootstrap::get(NAME(BOOTSTRAP_ITEM_INIFILE),sURL))
    {
        sURL = getDefaultConfigurationBootstrapURL();
    }

    ComponentContext::initialize(sURL);
}
// ---------------------------------------------------------------------------

static OUString getCurrentModuleDirectory() // URL including terminating slash
{
    OUString aFileURL;
    if ( !osl::Module::getUrlFromAddress((void*)&getCurrentModuleDirectory,aFileURL) )
    {
        OSL_TRACE(false, "Cannot locate current module - using executable instead");

        OSL_VERIFY(osl_Process_E_None == osl_getExecutableFile(&aFileURL.pData));
    }

    OSL_ENSURE(0 < aFileURL.lastIndexOf('/'), "Cannot find directory for module URL");

    return aFileURL.copy(0, aFileURL.lastIndexOf('/') + 1);
}
// ---------------------------------------------------------------------------------------

OUString BootstrapContext::getDefaultConfigurationBootstrapURL()
{
    return getCurrentModuleDirectory() + OUString(RTL_CONSTASCII_USTRINGPARAM(CONFIGMGR_INIFILE));
}
// ---------------------------------------------------------------------------------------

OUString BootstrapContext::makeContextName(OUString const & _aName)
{
    // check that it isn't long already
    OSL_ENSURE(!_aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_MODULE_PREFIX_) ),
                "configmgr::BootstrapContext: passing argument in long context form won't work");

    return NAME(CONTEXT_ITEM_PREFIX_).concat(_aName);
}
// ---------------------------------------------------------------------------

OUString BootstrapContext::makeBootstrapName(OUString const & _aName)
{
    // check if already is short
    if (!_aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_PREFIX_) ) )
    {
        OSL_TRACE( "configmgr: Cannot map name to bootstrap name: %s",
                    rtl::OUStringToOString(_aName,RTL_TEXTENCODING_ASCII_US).getStr() );
        return _aName;
    }
    return NAME(BOOTSTRAP_ITEM_PREFIX_).concat(_aName.copy(RTL_CONSTASCII_LENGTH(CONTEXT_ITEM_PREFIX_)));
}
// ---------------------------------------------------------------------------

uno::Any SAL_CALL
    BootstrapContext::getValueByName( const OUString& aName )
        throw (uno::RuntimeException)
{
    sal_Bool const bOurName = aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_MODULE_PREFIX_) );

    if (bOurName)
    {
        if (aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_BOOTSTRAP_ERROR) ) )
            return this->makeBootstrapException();

        if (aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_PREFIX_ SETTING_INIFILE) ) )
            return uno::makeAny( this->getBootstrapURL() );

        if (aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_IS_BOOTSTRAP_CONTEXT) ) )
            return uno::makeAny( sal_True );
    }
    else if (aName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(SINGLETON_ A_BootstrapContextSingletonName) ) )
    {
        return uno::makeAny( Context(this) );
    }

    uno::Any aResult;

    bool bFound = lookupInContext  ( aResult, aName );

    if (!bFound && bOurName) // requires: CONTEXT_ITEM_PREFIX_ starts with CONTEXT_MODULE_PREFIX_
    {
        if ( aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_PREFIX_) ) )
        {
            bFound = lookupInBootstrap( aResult, makeBootstrapName(aName) );
        }
    }
#if 0
    if (!bFound && bOurName)
    {
        OSL_TRACE( "configmgr: Cannot find bootstrap data item: %s",
                    rtl::OUStringToOString(aName,RTL_TEXTENCODING_ASCII_US).getStr() );
    }
#endif
    return aResult;
}
// ---------------------------------------------------------------------------

#if 0
uno::Any BootstrapContext::makeDefaultProvider()
{
    {
        osl::MutexGuard lock(mutex());
        if (m_xDefaultProvider.is())
            return uno::makeAny(m_xDefaultProvider);
    }

    if (isPassthrough())
    {
        Context xDelegate = basecontext();
        if (!xDelegate.is())
            throw lang::DisposedException(NAME("BootstrapContext: No base context"),*this);

        uno::Any aResult = xDelegate->getValueByName( SINGLETON(A_DefaultProviderSingletonName) );

        osl::MutexGuard relock(mutex());
        if (!m_xDefaultProvider.is())
        {
            aResult >>= m_xDefaultProvider;
            return aResult;
        }
        else
            return uno::makeAny(m_xDefaultProvider);
    }
    else
    {
        ServiceManager xMgr = this->getServiceManager();
        if (!xMgr.is())
            throw lang::DisposedException(NAME("BootstrapContext: No service factory"),*this);

        try
        {
            uno::Reference<uno::XInterface> xDefaultProvider = xMgr->createInstanceWithContext(NAME(A_DefaultProviderServiceAndImplName),this);

            osl::MutexGuard relock(mutex());
            if (!m_xDefaultProvider.is())
                m_xDefaultProvider = xDefaultProvider;

            return uno::makeAny(m_xDefaultProvider);
        }
        catch (uno::RuntimeException &) { throw; }
        catch (uno::Exception & )
        {
            throw uno::RuntimeException(NAME("BootstrapContext:Exception occurred while instantiating DefaultProvider"),*this);
        }
    }
}
// ---------------------------------------------------------------------------

uno::Any BootstrapContext::makeDefaultBackend()
{
    {
        osl::MutexGuard lock(mutex());
        if (m_xDefaultBackend.is())
            return uno::makeAny(m_xDefaultBackend);
    }

    if (isPassthrough())
    {
        Context xDelegate = basecontext();
        if (!xDelegate.is())
            throw lang::DisposedException(NAME("BootstrapContext: No base context"),*this);

        uno::Any aResult = xDelegate->getValueByName( SINGLETON(K_DefaultBackendSingletonName) );

        osl::MutexGuard relock(mutex());
        if (!m_xDefaultBackend.is())
        {
            aResult >>= m_xDefaultBackend;
            return aResult;
        }
        else
            return uno::makeAny(m_xDefaultBackend);
    }
    else
    {
        ServiceManager xMgr = this->getServiceManager();
        if (!xMgr.is())
            throw lang::DisposedException(NAME("BootstrapContext: No service factory"),*this);

        try
        {
            uno::Reference<uno::XInterface> xDefaultBackend = xMgr->createInstanceWithContext(NAME(K_DefaultBackendServiceAndImplName),this);

            osl::MutexGuard relock(mutex());
            if (!m_xDefaultBackend.is())
                m_xDefaultBackend = xDefaultBackend;
            return uno::makeAny(m_xDefaultBackend);
        }
        catch (uno::RuntimeException &) { throw; }
        catch (uno::Exception & )
        {
            throw uno::RuntimeException(NAME("BootstrapContext:Exception occurred while instantiating DefaultBackend"),*this);
        }
    }
}
// ---------------------------------------------------------------------------
#endif
// ---------------------------------------------------------------------------
// class ContextReader
// ---------------------------------------------------------------------------

    ContextReader::ContextReader(Context const & context)
    : m_basecontext(context)
    , m_fullcontext()
    {
        OSL_ENSURE(context.is(), "ERROR: trying to create reader on NULL context\n");
        if (context.is())
        {
            uno::Any aBootstrapContext = context->getValueByName( SINGLETON(A_BootstrapContextSingletonName) );
            aBootstrapContext >>= m_fullcontext;
        }
    }
// ---------------------------------------------------------------------------

    uno::Reference< lang::XMultiComponentFactory > ContextReader::getServiceManager() const
    {
        OSL_ASSERT(m_basecontext.is());
        return m_basecontext->getServiceManager();
    }
// ---------------------------------------------------------------------------
    inline
    uno::Any ContextReader::getSetting(OUString const & _aSetting) const
    {
        OSL_ASSERT(m_basecontext.is());
        return getBestContext()->getValueByName(_aSetting);
    }

    inline
    sal_Bool ContextReader::hasSetting(OUString const & _aSetting) const
    {
        return getSetting(_aSetting).hasValue();
    }

    inline
    sal_Bool ContextReader::getBoolSetting(OUString const & _aSetting, sal_Bool bValue = false) const
    {
        getSetting(_aSetting) >>= bValue;
        return bValue;
    }

    inline
    OUString ContextReader::getStringSetting(OUString const & _aSetting, OUString aValue = OUString()) const
    {
        getSetting(_aSetting) >>= aValue;
        return aValue;
    }
// ---------------------------------------------------------------------------------------

    sal_Bool ContextReader::isUnoBackend() const
    {
        OUString aSettingName = NAME(BOOTSTRAP_SERVERTYPE_COMPAT);

        OUString aValue;
        if (getSetting(aSettingName) >>= aValue)
        {
            return aValue.equalsAscii(SERVERTYPE_UNO_COMPAT);
        }
        else
        {
            return true;
        }
    }
// ---------------------------------------------------------------------------------------

    sal_Bool    ContextReader::hasUnoBackendService() const
    {
        return hasSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_UNOSERVICE) );
    }
    sal_Bool    ContextReader::hasUnoBackendWrapper() const
    {
        return hasSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_UNOWRAPPER) );
    }

    sal_Bool    ContextReader::hasLocale() const
    {
        return hasSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_LOCALE_NEW) );
    }
    sal_Bool    ContextReader::hasAsyncSetting() const
    {
        return hasSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_ASYNC_NEW) );
    }
    sal_Bool    ContextReader::hasOfflineSetting() const
    {
        return hasSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_OFFLINE) );
    }
// ---------------------------------------------------------------------------------------

    OUString    ContextReader::getUnoBackendService() const
    {
        return getStringSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_UNOSERVICE) );
    }
    OUString    ContextReader::getUnoBackendWrapper() const
    {
        return getStringSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_UNOWRAPPER) );
    }

    OUString    ContextReader::getLocale() const
    {
        return getStringSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_LOCALE_NEW) );
    }
    sal_Bool    ContextReader::getAsyncSetting() const
    {
        return getBoolSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_ASYNC_NEW) );
    }
    sal_Bool    ContextReader::getOfflineSetting() const
    {
        return getBoolSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_OFFLINE) );
    }

    // get a special setting
    sal_Bool    ContextReader::isAdminService() const
    {
        return getBoolSetting( NAME(CONTEXT_ITEM_ADMINFLAG) );
    }

    sal_Bool    ContextReader::isBootstrapValid() const
    {
        return  this->isUnoBackend() &&
                this->hasUnoBackendService() &&
                (this->hasUnoBackendWrapper() || !this->getOfflineSetting());
    }

    uno::Any    ContextReader::getBootstrapError() const
    {
        return getSetting( NAME(CONTEXT_ITEM_BOOTSTRAP_ERROR) );
    }
// ---------------------------------------------------------------------------------------

    bool ContextReader::isBootstrapContext(Context const & _xContext)
    {
        OSL_ASSERT(_xContext.is());
        if (!_xContext.is()) return false;

        uno::Any aSetting = _xContext->getValueByName( NAME(CONTEXT_ITEM_IS_BOOTSTRAP_CONTEXT) );

        sal_Bool bValue = false;
        return (aSetting >>= bValue) && bValue;
    }
// ---------------------------------------------------------------------------

    bool ContextReader::testAdminService(Context const & context, bool bAdmin)
    {
        OSL_ASSERT(context.is());
        if (!context.is()) return false;

        uno::Any aSetting = context->getValueByName( NAME(CONTEXT_ITEM_ADMINFLAG) );

        sal_Bool bValue = false;
        bool bTest = (aSetting >>= bValue) && bValue;

        return bTest == bAdmin;
    }
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

    bool ArgumentHelper::extractArgument(beans::NamedValue & rValue, const uno::Any & aOverride)
    {
        if ( ! (aOverride >>= rValue) )
        {
            // it must be a PropertyValue, if it isn't a NamedValue
            beans::PropertyValue aPV;
            if ( !(aOverride >>= aPV) )
                return false;

            rValue.Name  = aPV.Name;
            rValue.Value = aPV.Value;
        }

        return true;
    }

// ---------------------------------------------------------------------------------------

    bool ArgumentHelper::checkBackendArgument(beans::NamedValue const & aAdjustedValue)
    {
        bool isWrappable =
            aAdjustedValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_PREFIX_ SETTING_ASYNC_NEW)) ||
            aAdjustedValue.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(CONTEXT_ITEM_PREFIX_ SETTING_LOCALE_NEW));

        if (isWrappable) return false;

        m_bHasBackendArguments = true;
        return true;
    }
// ---------------------------------------------------------------------------------------

    bool ArgumentHelper::filterAndAdjustArgument(beans::NamedValue & rValue)
    {
        // handle old servertype argument and filter the 'plugin' value
        if (rValue.Name.equalsAscii(ARGUMENT_SERVERTYPE_COMPAT))
        {
            OUString aServertype;
            if (! (rValue.Value >>= aServertype))
                return false;

            if (aServertype.equalsAscii(SERVERTYPE_PLUGIN_COMPAT))
                return false;

            rValue.Name = NAME(BOOTSTRAP_SERVERTYPE_COMPAT);
            // check, if it is already there
            uno::Any const aExistingValue = m_context->getValueByName(rValue.Name);

            if (aExistingValue.hasValue())
                return !(aExistingValue == rValue.Value);

            else
                return !aServertype.equalsAscii(SERVERTYPE_UNO_COMPAT);
        }

        // map old argument names for comatibility
        else if (rValue.Name.equalsAscii(ARGUMENT_LOCALE_COMPAT))
            rValue.Name = NAME(SETTING_LOCALE_NEW);

        else if (rValue.Name.equalsAscii(ARGUMENT_ASYNC_COMPAT))
            rValue.Name = NAME(SETTING_ASYNC_NEW);

        // give the item a long name
        rValue.Name = BootstrapContext::makeContextName(rValue.Name);

        // check, if it is already there
        uno::Any const aExistingValue = m_context->getValueByName(rValue.Name);

        return ! (aExistingValue == rValue.Value);
    }
// ---------------------------------------------------------------------------------------

    beans::NamedValue ArgumentHelper::makeAdminServiceOverride(sal_Bool bAdmin)
    {
        return beans::NamedValue( NAME(CONTEXT_ITEM_ADMINFLAG), uno::makeAny(bAdmin) );
    }
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
// - bootstrapping error checking helper
// ---------------------------------------------------------------------------------------
namespace {
// ---------------------------------------------------------------------------------------
// error handling
// ---------------------------------------------------------------------------------------
    enum BootstrapResult
    {
        BOOTSTRAP_DATA_OK,
        INCOMPLETE_BOOTSTRAP_DATA,
        INCOMPLETE_BOOTSTRAP_FILE,
        MISSING_BOOTSTRAP_FILE,
        BOOTSTRAP_FAILURE
    };
// ---------------------------------------------------------------------------------------
    static
    OUString getFallbackErrorMessage( BootstrapResult _rc )
    {
        OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("The program cannot start. "));

        switch (_rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("A main configuration file is missing"));
            break;

        case INCOMPLETE_BOOTSTRAP_FILE:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("A main configuration file is invalid"));
            break;

        case INCOMPLETE_BOOTSTRAP_DATA:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("Required bootstrap data is not available"));
            break;

        default:
            sMessage = OUString(RTL_CONSTASCII_USTRINGPARAM("Unexpected bootstrap failure"));
            break;

        case BOOTSTRAP_DATA_OK:
            break;
        }
        sMessage += OUString(RTL_CONSTASCII_USTRINGPARAM(" (No detailed error message available.)"));

        return sMessage;
    }
// ---------------------------------------------------------------------------------------

    static
    uno::Any impl_makeBootstrapException( BootstrapResult _rc, OUString const& _sMessage, OUString const& _sURL, uno::Reference< uno::XInterface > _xContext )
    {
        OUString sMessage(_sMessage);
        // ensure a message
        if (sMessage.getLength()== 0)
        {
            OSL_ENSURE(false, "Bootstrap error message missing");

            sMessage = getFallbackErrorMessage(_rc);
        }

        using namespace com::sun::star::configuration;

        // raise the error
        switch (_rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            return uno::makeAny( MissingBootstrapFileException(sMessage, _xContext, _sURL) );

        case INCOMPLETE_BOOTSTRAP_FILE:
            return uno::makeAny( InvalidBootstrapFileException(sMessage, _xContext, _sURL) );

        default: OSL_ENSURE(false, "Undefined BootstrapResult code");
        case INCOMPLETE_BOOTSTRAP_DATA:
        case BOOTSTRAP_FAILURE:
            return uno::makeAny( CannotLoadConfigurationException(sMessage, _xContext) );

        case BOOTSTRAP_DATA_OK:
            break;
        }
        return uno::Any();
    }
// ---------------------------------------------------------------------------------------

    static
    inline
    bool urlExists(OUString const& _sURL)
    {
        osl::DirectoryItem aCheck;
        return (osl::DirectoryItem::get(_sURL,aCheck) == osl::DirectoryItem::E_None);
    }
// ---------------------------------------------------------------------------------------

    static
    OUString buildBootstrapError( sal_Char const* _sWhat, OUString const& _sName, sal_Char const* _sHow)
    {
        rtl::OUStringBuffer sMessage;

        sMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM("The program cannot start. "));
        sMessage.appendAscii(_sWhat);
        sMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM(" '")).append(_sName).appendAscii(RTL_CONSTASCII_STRINGPARAM("' "));
        sMessage.appendAscii(_sHow).appendAscii(". ");

        return sMessage.makeStringAndClear();
    }
// ---------------------------------------------------------------------------------------

    BootstrapResult getBootstrapErrorMessage(BootstrapContext const & aContext, ContextReader const & aSettings, OUString& _rMessage, OUString& _rIniFile )
    {
        BootstrapResult eResult = BOOTSTRAP_DATA_OK;

        _rIniFile = aContext.getBootstrapURL();

        if ( !urlExists(_rIniFile) )
        {
            _rMessage = buildBootstrapError("The configuration file ",_rIniFile.copy(1+_rIniFile.lastIndexOf('/')),"is missing");
            eResult = MISSING_BOOTSTRAP_FILE;
        }
        else if (!aSettings.isUnoBackend())
        {
            _rMessage = buildBootstrapError("The configuration file ",_rIniFile.copy(1+_rIniFile.lastIndexOf('/')),"is for an older version of the configuration database");
            eResult = INCOMPLETE_BOOTSTRAP_FILE;
        }
        else if (!aSettings.isBootstrapValid() )
        {
             _rMessage = buildBootstrapError("Needed information to access",OUString::createFromAscii("application"), "configuration data is missing");
            eResult = INCOMPLETE_BOOTSTRAP_DATA;
        }

        return eResult;
    }
// ---------------------------------------------------------------------------------------
} // anonymous namespace
// ---------------------------------------------------------------------------------------
uno::Any BootstrapContext::makeBootstrapException()
{
    ContextReader aReader(this);

    if (aReader.isBootstrapValid()) return uno::Any();

    OUString sMessage,sURL;

    BootstrapResult rc = getBootstrapErrorMessage(*this,aReader,sMessage,sURL);

    return impl_makeBootstrapException(rc,sMessage,sURL,*this);
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
} // namespace configmgr


