// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"

// =============================================================================
// component exports
// =============================================================================
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace evtlstner
{
    // =============================================================================
    // component operations
    // =============================================================================

    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () );

    // -----------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL getImplementationName();

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();

    Reference<XInterface> SAL_CALL create(
        Sequence<Any> const &, Reference<XComponentContext> const & );
} // end evtlstner

namespace ooevtdescgen
{
    // =============================================================================
    // component operations
    // =============================================================================

    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW( () );

    // -----------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL getImplementationName();

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();

    Reference<XInterface> SAL_CALL create(
        Sequence<Any> const &, Reference<XComponentContext> const & );
} // end ooevtdescgen



    // =============================================================================

    const ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            ::evtlstner::create, ::evtlstner::getImplementationName,
            ::evtlstner::getSupportedServiceNames,
            ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            ::ooevtdescgen::create, ::ooevtdescgen::getImplementationName,
            ::ooevtdescgen::getSupportedServiceNames,
            ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        OSL_TRACE("In component_getImplementationEnv");
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo(
        lang::XMultiServiceFactory * pServiceManager, registry::XRegistryKey * pRegistryKey )
    {
        OSL_TRACE("In component_writeInfo");
        if ( ::cppu::component_writeInfoHelper(
            pServiceManager, pRegistryKey, s_component_entries ) )
            return sal_True;
        return sal_False;
    }

    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        OSL_TRACE("In component_getFactory");
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, s_component_entries );
    }
}
