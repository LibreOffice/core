#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>
#include "provider.hxx"
#include "renderer.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star;

namespace unographic {

// --------------------
// - *_createInstance -
// --------------------

static uno::Reference< uno::XInterface > SAL_CALL GraphicProvider_createInstance( const uno::Reference< lang::XMultiServiceFactory >& rxManager)
{
    return SAL_STATIC_CAST( ::cppu::OWeakObject*, new GraphicProvider );
}

// -----------------------------------------------------------------------------

static uno::Reference< uno::XInterface > SAL_CALL GraphicRendererVCL_createInstance( const uno::Reference< lang::XMultiServiceFactory >& rxManager)
{
    return SAL_STATIC_CAST( ::cppu::OWeakObject*, new GraphicRendererVCL );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// -----------------------
// - component_writeInfo -
// -----------------------

extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    sal_Bool bRet = sal_False;

    if( pRegistryKey )
    {
        try
        {
            uno::Reference< registry::XRegistryKey >    xNewKey;
            uno::Sequence< ::rtl::OUString >            aServices;

            // GraphicProvider
            xNewKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey )->createKey(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                        GraphicProvider::getImplementationName_Static() +
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) );

            aServices = GraphicProvider::getSupportedServiceNames_Static();

            int i;
            for( i = 0; i < aServices.getLength(); i++ )
                xNewKey->createKey( aServices.getConstArray()[ i ] );

            // GraphicRendererVCL
            xNewKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey )->createKey(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                        GraphicRendererVCL::getImplementationName_Static() +
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) );

            aServices = ( GraphicRendererVCL::getSupportedServiceNames_Static() );

            for( i = 0; i < aServices.getLength(); i++ )
                xNewKey->createKey( aServices.getConstArray()[ i ] );

            bRet = true;
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }

    return bRet;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    void * pRet = 0;

    if( pServiceManager && ( 0 == GraphicProvider::getImplementationName_Static().compareToAscii( pImplName ) ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( ::cppu::createOneInstanceFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            GraphicProvider::getImplementationName_Static(),
            GraphicProvider_createInstance,
            GraphicProvider::getSupportedServiceNames_Static() ) );

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    else if( pServiceManager && ( 0 == GraphicRendererVCL::getImplementationName_Static().compareToAscii( pImplName ) ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( ::cppu::createOneInstanceFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            GraphicRendererVCL::getImplementationName_Static(),
            GraphicRendererVCL_createInstance,
            GraphicRendererVCL::getSupportedServiceNames_Static() ) );

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}
