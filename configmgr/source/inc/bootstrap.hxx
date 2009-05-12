/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bootstrap.hxx,v $
 * $Revision: 1.29 $
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

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#define CONFIGMGR_BOOTSTRAP_HXX_

#include "bootstrapcontext.hxx"

// ---------------------------------------------------------------------------------------
#define CONFIGMGR_INIFILE                   SAL_CONFIGFILE("configmgr")
#define BOOTSTRAP_ITEM_INIFILE              "CFG_INIFILE"
// ---------------------------------------------------------------------------------------
// standard settings
#define SETTING_UNOSERVICE                  "BackendService"
#define SETTING_UNOWRAPPER                  "BackendWrapper"
#define SETTING_OFFLINE                     "Offline"
#define SETTING_LOCALE_NEW                  "Locale"
#define SETTING_ASYNC_NEW                   "EnableAsync"
#define SETTING_INIFILE                     "Inifile"

// Prefixes
#define CONTEXT_MODULE_PREFIX_              "/modules/com.sun.star.configuration/"
#define CONTEXT_SECTION_BOOTSTRAP_          "bootstrap/"
#define CONTEXT_ITEM_PREFIX_                CONTEXT_MODULE_PREFIX_ CONTEXT_SECTION_BOOTSTRAP_
#define BOOTSTRAP_ITEM_PREFIX_              "CFG_"

// special internal context values
#define CONTEXT_SECTION_INTERNAL_           "factory/"
#define CONTEXT_INTERNAL_PREFIX_            CONTEXT_MODULE_PREFIX_ CONTEXT_SECTION_INTERNAL_
#define CONTEXT_ITEM_ADMINFLAG              CONTEXT_INTERNAL_PREFIX_"isAdminConfiguration"
#define CONTEXT_ITEM_BOOTSTRAP_ERROR        CONTEXT_INTERNAL_PREFIX_"theBootstrapError"

#define CONTEXT_ITEM_IS_WRAPPER_CONTEXT     CONTEXT_INTERNAL_PREFIX_"isWrapperContext"
#define CONTEXT_ITEM_IS_BOOTSTRAP_CONTEXT   CONTEXT_INTERNAL_PREFIX_"isBootstrapContext"

// ---------------------------------------------------------------------------------------
#define A_DefaultProviderSingletonName "com.sun.star.configuration.theDefaultProvider"
#define K_DefaultBackendSingletonName       "com.sun.star.configuration.backend.theDefaultBackend"
#define A_BootstrapContextSingletonName     "com.sun.star.configuration.bootstrap.theBootstrapContext"
// -------------------------------------------------------------------------
#define A_DefaultProviderServiceAndImplName     "com.sun.star.configuration.DefaultProvider"
#define K_DefaultBackendServiceAndImplName         "com.sun.star.configuration.backend.DefaultBackend"
// ---------------------------------------------------------------------------------------
namespace configmgr
{
    // -----------------------------------------------------------------------------------

    namespace uno = ::com::sun::star::uno;
    namespace lang = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;
    // -----------------------------------------------------------------------------------

    /** Customized ComponentContext for configuration bootstrap data and runtime arguments
    */
    class BootstrapContext : public ComponentContext
    {
    // creation and destruction
    private:
        friend uno::Reference<uno::XInterface> SAL_CALL
            instantiateBootstrapContext( uno::Reference< uno::XComponentContext > const& xContext );

        // constructor
        BootstrapContext(uno::Reference< uno::XComponentContext > const & _xContext);

        // two-phase construct
        void initialize();

    protected:
        using ComponentContext::initialize;

    public:
         // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName()
            throw (uno::RuntimeException) ;
        virtual sal_Bool SAL_CALL supportsService(
            const rtl::OUString& aServiceName)
            throw (uno::RuntimeException) ;
        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(void) throw (uno::RuntimeException) ;


        /** Constructs a Context based on the given arguments and context.
            @param _xContext
                The base context of this component context.

            @param _aArguments
                The arguments used to create this component context.
        */
        static uno::Reference< uno::XComponentContext > createWrapper(uno::Reference< uno::XComponentContext > const & _xContext, uno::Sequence < beans::NamedValue > const & _aOverrides);

        /** Checks, if the given context is a wrapper.
            @param _xContext
                The context that is checked.
        */
        static sal_Bool isWrapper(uno::Reference< uno::XComponentContext > const & _xContext);

        /** Retrieves the BootstrapContext for the given non-bootstrap context.
            @param _xContext
                The context from which the bootstrap context should be retrieved.

        */
        static uno::Reference< uno::XComponentContext > get(uno::Reference< uno::XComponentContext > const & _xContext);

        /// Destroys this BootstrapContext
        ~BootstrapContext();

        // gets the INI that should be used for bootstrap data by default
        static rtl::OUString getDefaultConfigurationBootstrapURL();

    // interface implementations
    public:
    // XComponentContext
        /** Retrieves a value from this context.

            @param name
                The name of the value to retrieve.
                A prefix of "com.sun.star.configuration.bootstrap." is stripped/ignored

            @returns
                The requested value, or <VOID/> if the value is not found.
        */
        virtual uno::Any SAL_CALL
            getValueByName( const rtl::OUString& name )
                throw (uno::RuntimeException);

    public: // used by ArgumentHelper
        static rtl::OUString makeContextName (rtl::OUString const & _aShortName);

    private:
        static rtl::OUString makeBootstrapName(rtl::OUString const & _aLongName);
        uno::Any makeBootstrapException();
    };
// -----------------------------------------------------------------------------
    class ContextReader
    {
    public:
        explicit
        ContextReader(uno::Reference< uno::XComponentContext > const & context);

    // the underlying contexts
        sal_Bool hasBootstrapContext()          const { return m_fullcontext.is(); }
        uno::Reference< uno::XComponentContext > const & getBootstrapContext()   const { return m_fullcontext; }
        uno::Reference< uno::XComponentContext > const & getBaseContext()        const { return m_basecontext; }
        uno::Reference< uno::XComponentContext > const & getBestContext()        const { return m_fullcontext.is() ? m_fullcontext : m_basecontext; }

        uno::Reference< lang::XMultiComponentFactory > getServiceManager() const;

        /** Checks, if the given context has the given 'admin' flag setting..
            @param _xContext
                The context that is checked.
        */
        static bool testAdminService(uno::Reference< uno::XComponentContext > const & context, bool bAdmin);

    // general settings
        sal_Bool    isUnoBackend() const;

        sal_Bool    hasUnoBackendService() const;
        sal_Bool    hasUnoBackendWrapper() const;

        sal_Bool    hasLocale() const;
        sal_Bool    hasAsyncSetting() const;
        sal_Bool    hasOfflineSetting() const;

        rtl::OUString    getUnoBackendService() const;
        rtl::OUString    getUnoBackendWrapper() const;

        rtl::OUString   getLocale() const;
        sal_Bool    getAsyncSetting() const;
        sal_Bool    getOfflineSetting() const;

    // internal settings - should only ever be in configmgr::BootstrapContext instances
        // get a special setting
        sal_Bool    isAdminService() const;

        // access to error diagnostics
        sal_Bool    isBootstrapValid() const;
        uno::Any    getBootstrapError() const;
    private:
        sal_Bool hasSetting(rtl::OUString const & _aSetting) const;
        sal_Bool getBoolSetting(rtl::OUString const & _aSetting, sal_Bool bValue) const;
        rtl::OUString getStringSetting(rtl::OUString const & _aSetting, rtl::OUString aValue) const;
        uno::Any getSetting(rtl::OUString const & _aSetting) const;
    private:
        uno::Reference< uno::XComponentContext > m_basecontext;
        uno::Reference< uno::XComponentContext > m_fullcontext;
    };
    //------------------------------------------------------------------------

    class ArgumentHelper
    {
    public:
        explicit
        ArgumentHelper(uno::Reference< uno::XComponentContext > const & context)
        : m_context(context)
        , m_bHasBackendArguments(false)
        {}

        bool hasBackendArguments() const { return m_bHasBackendArguments; }
        bool checkBackendArgument(beans::NamedValue const & aAdjustedValue);

        bool filterAndAdjustArgument(beans::NamedValue & rValue);

        static
        bool extractArgument(beans::NamedValue & rValue, uno::Any const & aArgument);

        static beans::NamedValue makeAdminServiceOverride(sal_Bool bAdmin);
    private:
        uno::Reference< uno::XComponentContext > m_context; // context used to strip identical arguments
        bool m_bHasBackendArguments;
    };
// -----------------------------------------------------------------------------------

}

#endif // CONFIGMGR_BOOTSTRAP_HXX_


