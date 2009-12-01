/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bootstrap.cxx,v $
 * $Revision: 1.35 $
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
#include "precompiled_configmgr.hxx"

#include <stdio.h>

#include "bootstrap.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include "serviceinfohelper.hxx"
#include "matchlocale.hxx"
#include "tracer.hxx"
#include <cppuhelper/component_context.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/configuration/MissingBootstrapFileException.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/InstallationIncompleteException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

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

#define NAME( N ) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(N))
#define ITEM( N ) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(N))
// ---------------------------------------------------------------------------------------
// -------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
namespace configmgr
{
// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------
    static void convertToBool(const uno::Any& aValue, sal_Bool& bValue)
    {
        rtl::OUString aStrValue;
        if (aValue >>= aStrValue)
        {
            if (aStrValue.equalsIgnoreAsciiCaseAscii("true"))
            {
                bValue = sal_True;
            }
            else if (aStrValue.equalsIgnoreAsciiCaseAscii("false"))
            {
                bValue = sal_False;
            }
        }
    }
    // ----------------------------------------------------------------------------------
    const sal_Char k_BootstrapContextImplName[]         = "com.sun.star.comp.configuration.bootstrap.BootstrapContext" ;
    const sal_Char k_BootstrapContextServiceName[]      = "com.sun.star.configuration.bootstrap.BootstrapContext" ;

    // -------------------------------------------------------------------------
    static sal_Char const * const k_BootstrapContextServiceNames [] =
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
        instantiateBootstrapContext( uno::Reference< uno::XComponentContext > const& xTargetContext )
    {
        uno::Reference< uno::XComponentContext > xContext = UnoContextTunnel::recoverContext(xTargetContext);

        BootstrapContext * pContext = new BootstrapContext(xContext);
        uno::Reference< uno::XComponentContext > xResult(pContext);

        pContext->initialize();

        return uno::Reference< uno::XInterface >( xResult, uno::UNO_QUERY );
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

        rtl::OUString const aServiceName   = rtl::OUString::createFromAscii(pSingletonInfo->instantiatedServiceName);

        return cppu::ContextEntry_Init(aSingletonName.makeStringAndClear(), uno::makeAny(aServiceName), true);
    }
// ---------------------------------------------------------------------------

uno::Reference< uno::XComponentContext > BootstrapContext::createWrapper(uno::Reference< uno::XComponentContext > const & _xContext, uno::Sequence < beans::NamedValue > const & _aOverrides)
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

sal_Bool BootstrapContext::isWrapper(uno::Reference< uno::XComponentContext > const & _xContext)
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

BootstrapContext::BootstrapContext(uno::Reference< uno::XComponentContext > const & _xContext)
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
    rtl::OUString sURL;
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
// ---------------------------------------------------------------------------------------

rtl::OUString BootstrapContext::getDefaultConfigurationBootstrapURL()
{
    rtl::OUString url(
        RTL_CONSTASCII_USTRINGPARAM(
            "$OOO_BASE_DIR/program/" SAL_CONFIGFILE("configmgr")));
    rtl::Bootstrap::expandMacros(url); //TODO: detect failure
    return url;
}
// ---------------------------------------------------------------------------------------

rtl::OUString BootstrapContext::makeContextName(rtl::OUString const & _aName)
{
    // check that it isn't long already
    OSL_ENSURE(!_aName.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM(CONTEXT_MODULE_PREFIX_) ),
                "configmgr::BootstrapContext: passing argument in long context form won't work");

    return NAME(CONTEXT_ITEM_PREFIX_).concat(_aName);
}
// ---------------------------------------------------------------------------

rtl::OUString BootstrapContext::makeBootstrapName(rtl::OUString const & _aName)
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
    BootstrapContext::getValueByName( const rtl::OUString& aName )
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
        return uno::makeAny( uno::Reference< uno::XComponentContext >(this) );
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
    return aResult;
}

// ---------------------------------------------------------------------------
// class ContextReader
// ---------------------------------------------------------------------------

    ContextReader::ContextReader(uno::Reference< uno::XComponentContext > const & context)
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
    uno::Any ContextReader::getSetting(rtl::OUString const & _aSetting) const
    {
        OSL_ASSERT(m_basecontext.is());
        return getBestContext()->getValueByName(_aSetting);
    }

    inline
    sal_Bool ContextReader::hasSetting(rtl::OUString const & _aSetting) const
    {
        return getSetting(_aSetting).hasValue();
    }

    inline
    sal_Bool ContextReader::getBoolSetting(rtl::OUString const & _aSetting, sal_Bool bValue = false) const
    {
        uno::Any aValue = getSetting(_aSetting);
        if (!(aValue >>= bValue))
            convertToBool(aValue, bValue);

        return bValue;
    }

    inline
    rtl::OUString ContextReader::getStringSetting(rtl::OUString const & _aSetting, rtl::OUString aValue = rtl::OUString()) const
    {
        getSetting(_aSetting) >>= aValue;
        return aValue;
    }
// ---------------------------------------------------------------------------------------

    sal_Bool ContextReader::isUnoBackend() const
    {
        rtl::OUString aSettingName = NAME(BOOTSTRAP_SERVERTYPE_COMPAT);

        rtl::OUString aValue;
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

    rtl::OUString    ContextReader::getUnoBackendService() const
    {
        return getStringSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_UNOSERVICE) );
    }
    rtl::OUString    ContextReader::getUnoBackendWrapper() const
    {
        return getStringSetting( NAME(CONTEXT_ITEM_PREFIX_ SETTING_UNOWRAPPER) );
    }

    rtl::OUString   ContextReader::getLocale() const
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

    bool ContextReader::testAdminService(uno::Reference< uno::XComponentContext > const & context, bool bAdmin)
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
            rtl::OUString aServertype;
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
    rtl::OUString getFallbackErrorMessage( BootstrapResult _rc )
    {
        rtl::OUString sMessage(RTL_CONSTASCII_USTRINGPARAM("The program cannot start. "));

        switch (_rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            sMessage = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("A main configuration file is missing"));
            break;

        case INCOMPLETE_BOOTSTRAP_FILE:
            sMessage = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("A main configuration file is invalid"));
            break;

        case INCOMPLETE_BOOTSTRAP_DATA:
            sMessage = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Required bootstrap data is not available"));
            break;

        default:
            sMessage = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Unexpected bootstrap failure"));
            break;

        case BOOTSTRAP_DATA_OK:
            break;
        }
        sMessage += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" (No detailed error message available.)"));

        return sMessage;
    }
// ---------------------------------------------------------------------------------------

    static
    uno::Any impl_makeBootstrapException( BootstrapResult _rc, rtl::OUString const& _sMessage, rtl::OUString const& _sURL, uno::Reference< uno::XInterface > _xContext )
    {
        rtl::OUString sMessage(_sMessage);
        // ensure a message
        if (sMessage.getLength()== 0)
        {
            OSL_ENSURE(false, "Bootstrap error message missing");

            sMessage = getFallbackErrorMessage(_rc);
        }

        // raise the error
        switch (_rc)
        {
        case MISSING_BOOTSTRAP_FILE:
            return uno::makeAny( com::sun::star::configuration::MissingBootstrapFileException(sMessage, _xContext, _sURL) );

        case INCOMPLETE_BOOTSTRAP_FILE:
            return uno::makeAny( com::sun::star::configuration::InvalidBootstrapFileException(sMessage, _xContext, _sURL) );

        default: OSL_ENSURE(false, "Undefined BootstrapResult code");
        case INCOMPLETE_BOOTSTRAP_DATA:
        case BOOTSTRAP_FAILURE:
            return uno::makeAny( com::sun::star::configuration::CannotLoadConfigurationException(sMessage, _xContext) );

        case BOOTSTRAP_DATA_OK:
            break;
        }
        return uno::Any();
    }
// ---------------------------------------------------------------------------------------

    static
    inline
    bool urlExists(rtl::OUString const& _sURL)
    {
        osl::DirectoryItem aCheck;
        return (osl::DirectoryItem::get(_sURL,aCheck) == osl::DirectoryItem::E_None);
    }
// ---------------------------------------------------------------------------------------

    static
    rtl::OUString buildBootstrapError( sal_Char const* _sWhat, rtl::OUString const& _sName, sal_Char const* _sHow)
    {
        rtl::OUStringBuffer sMessage;

        sMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM("The program cannot start. "));
        sMessage.appendAscii(_sWhat);
        sMessage.appendAscii(RTL_CONSTASCII_STRINGPARAM(" '")).append(_sName).appendAscii(RTL_CONSTASCII_STRINGPARAM("' "));
        sMessage.appendAscii(_sHow).appendAscii(". ");

        return sMessage.makeStringAndClear();
    }
// ---------------------------------------------------------------------------------------

    BootstrapResult getBootstrapErrorMessage(BootstrapContext const & aContext, ContextReader const & aSettings, rtl::OUString& _rMessage, rtl::OUString& _rIniFile )
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
             _rMessage = buildBootstrapError("Needed information to access",rtl::OUString::createFromAscii("application"), "configuration data is missing");
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

    rtl::OUString sMessage,sURL;

    BootstrapResult rc = getBootstrapErrorMessage(*this,aReader,sMessage,sURL);

    return impl_makeBootstrapException(rc,sMessage,sURL,*this);
}
// ---------------------------------------------------------------------------
rtl::OUString SAL_CALL
    BootstrapContext::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&k_BootstrapContextServiceInfo).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL
    BootstrapContext::supportsService(const rtl::OUString& aServiceName)
        throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(&k_BootstrapContextServiceInfo).supportsService(aServiceName) ;
}
//------------------------------------------------------------------------------
uno::Sequence<rtl::OUString> SAL_CALL
    BootstrapContext::getSupportedServiceNames(void)
        throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&k_BootstrapContextServiceInfo).getSupportedServiceNames() ;
}
// ---------------------------------------------------------------------------------------
} // namespace configmgr


