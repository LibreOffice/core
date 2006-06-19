/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bootstrap.hxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:23:22 $
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

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#define CONFIGMGR_BOOTSTRAP_HXX_

#ifndef CONFIGMGR_BOOTSTRAPCONTEXT_HXX_
#include "bootstrapcontext.hxx"
#endif

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
    using ::rtl::OUString;
    // -----------------------------------------------------------------------------------

    /** Customized ComponentContext for configuration bootstrap data and runtime arguments
    */
    class BootstrapContext : public ComponentContext
    {
    // creation and destruction
    private:
        friend uno::Reference<uno::XInterface> SAL_CALL
            instantiateBootstrapContext( Context const& xContext );

        // constructor
        BootstrapContext(Context const & _xContext);

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


        typedef uno::Sequence < beans::NamedValue >             Overrides;
        /** Constructs a Context based on the given arguments and context.
            @param _xContext
                The base context of this component context.

            @param _aArguments
                The arguments used to create this component context.
        */
        static Context createWrapper(Context const & _xContext, Overrides const & _aOverrides);

        /** Checks, if the given context is a wrapper.
            @param _xContext
                The context that is checked.
        */
        static sal_Bool isWrapper(Context const & _xContext);

        /** Retrieves the BootstrapContext for the given non-bootstrap context.
            @param _xContext
                The context from which the bootstrap context should be retrieved.

        */
        static Context get(Context const & _xContext);

        /// Destroys this BootstrapContext
        ~BootstrapContext();

        // gets the INI that should be used for bootstrap data by default
        static OUString getDefaultConfigurationBootstrapURL();

    // interface implementations
    public:
    // XComponentContext
        /** Retrieves a value from this context.

            @param Name
                The name of the value to retrieve.
                A prefix of "com.sun.star.configuration.bootstrap." is stripped/ignored

            @returns
                The requested value, or <VOID/> if the value is not found.
        */
        virtual uno::Any SAL_CALL
            getValueByName( const OUString& Name )
                throw (uno::RuntimeException);

    public: // used by ArgumentHelper
        static OUString makeContextName (OUString const & _aShortName);

    private:
        static OUString makeBootstrapName(OUString const & _aLongName);
        uno::Any makeBootstrapException();
    };
// -----------------------------------------------------------------------------
    class ContextReader
    {
    public:
        typedef uno::Reference< uno::XComponentContext > Context;
        explicit
        ContextReader(Context const & context);

    // the underlying contexts
        sal_Bool hasBootstrapContext()          const { return m_fullcontext.is(); }
        Context const & getBootstrapContext()   const { return m_fullcontext; }
        Context const & getBaseContext()        const { return m_basecontext; }
        Context const & getBestContext()        const { return m_fullcontext.is() ? m_fullcontext : m_basecontext; }

        uno::Reference< lang::XMultiComponentFactory > getServiceManager() const;

        /** Checks, if the given context is a BootstrapContext.
            @param _xContext
                The context that is checked.
        */
        static bool isBootstrapContext(Context const & context);

        /** Checks, if the given context has the given 'admin' flag setting..
            @param _xContext
                The context that is checked.
        */
        static bool testAdminService(Context const & context, bool bAdmin);

    // general settings
        sal_Bool    isUnoBackend() const;

        sal_Bool    hasUnoBackendService() const;
        sal_Bool    hasUnoBackendWrapper() const;

        sal_Bool    hasLocale() const;
        sal_Bool    hasAsyncSetting() const;
        sal_Bool    hasOfflineSetting() const;

        OUString    getUnoBackendService() const;
        OUString    getUnoBackendWrapper() const;

        OUString    getLocale() const;
        sal_Bool    getAsyncSetting() const;
        sal_Bool    getOfflineSetting() const;

    // internal settings - should only ever be in configmgr::BootstrapContext instances
        // get a special setting
        sal_Bool    isAdminService() const;

        // access to error diagnostics
        sal_Bool    isBootstrapValid() const;
        uno::Any    getBootstrapError() const;
    private:
        sal_Bool hasSetting(OUString const & _aSetting) const;
        sal_Bool getBoolSetting(OUString const & _aSetting, sal_Bool bValue) const;
        OUString getStringSetting(OUString const & _aSetting, OUString aValue) const;
        uno::Any getSetting(OUString const & _aSetting) const;
    private:
        Context m_basecontext;
        Context m_fullcontext;
    };
    //------------------------------------------------------------------------

    class ArgumentHelper
    {
    public:
        typedef uno::Reference< uno::XComponentContext > Context;

        explicit
        ArgumentHelper(Context const & context)
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
        Context m_context; // context used to strip identical arguments
        bool m_bHasBackendArguments;
    };
// -----------------------------------------------------------------------------------

}

#endif // CONFIGMGR_BOOTSTRAP_HXX_


