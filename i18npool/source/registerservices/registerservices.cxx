/*************************************************************************
#*
#*  $RCSfile: registerservices.cxx,v $
#*
#*  Register Services
#*
#*  Ersterstellung  MT 15.3.2000
#*
#*  Letzte Aenderung    $Author: bustamam $ $Date: 2001-08-30 23:33:17 $
#*
#*  $Revision: 1.1 $
#*
#*  $Source: /zpool/svn/migration/cvs_rep_09_09_08/code/i18npool/source/registerservices/registerservices.cxx,v $
#*
#*  Copyright (c) 2000 Sun Microsystems Inc.
#*
#*************************************************************************/

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#include <cppuhelper/factory.hxx>

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <localedata.hxx>
#include <numberformatcode.hxx>
#include <defaultnumberingprovider.hxx>
#include <indexentrysupplier.hxx>


#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL ImplName##_CreateInstance(                                 \
            const ::com::sun::star::uno::Reference<                         \
                    ::com::sun::star::lang::XMultiServiceFactory >& )       \
{                                                                           \
    return ::com::sun::star::uno::Reference <                               \
            ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* )   \
                                    new ImplName );                         \
}

#define IMPL_CREATEINSTANCE_MSF( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   \
        SAL_CALL ImplName##_CreateInstance(                                 \
            const ::com::sun::star::uno::Reference<                         \
                    ::com::sun::star::lang::XMultiServiceFactory >& rxMSF ) \
{                                                                           \
    return ::com::sun::star::uno::Reference <                               \
            ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* )   \
                                    new ImplName( rxMSF ) );                \
}

typedef ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > (SAL_CALL *FN_CreateInstance)(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& );

// -------------------------------------------------------------------------------------

using namespace ::com::sun::star::i18n;

IMPL_CREATEINSTANCE_MSF( NumberFormatCodeMapper )
IMPL_CREATEINSTANCE( LocaleData )
IMPL_CREATEINSTANCE_MSF( DefaultNumberingProvider )
IMPL_CREATEINSTANCE_MSF( IndexEntrySupplier )

static const struct InstancesArray {
        const sal_Char* pServiceNm;
        const sal_Char* pImplementationNm;
        FN_CreateInstance pFn;
} aInstances[] = {
    {   "com.sun.star.i18n.LocaleData",
        "com.sun.star.i18n.LocaleData",
        & LocaleData_CreateInstance },
    {   "com.sun.star.i18n.NumberFormatMapper",
        "com.sun.star.i18n.NumberFormatCodeMapper",
        & NumberFormatCodeMapper_CreateInstance },
    {   "com.sun.star.text.DefaultNumberingProvider",
        "com.sun.star.text.DefaultNumberingProvider",
        &DefaultNumberingProvider_CreateInstance },
    {   "com.sun.star.i18n.IndexEntrySupplier",
        "com.sun.star.i18n.IndexEntrySupplier",
        &IndexEntrySupplier_CreateInstance },

// add here new services !!
    { 0, 0, 0 }
};


extern "C"
{

void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* _pServiceManager, void* _pRegistryKey )
{
    if (_pRegistryKey)
    {
        ::com::sun::star::registry::XRegistryKey * pRegistryKey =
            reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >(
                                _pRegistryKey );
        ::com::sun::star::uno::Reference<
                        ::com::sun::star::registry::XRegistryKey > xNewKey;

        for( const InstancesArray* pArr = aInstances; pArr->pServiceNm; ++pArr )
        {
            xNewKey = pRegistryKey->createKey(
                    ::rtl::OUString::createFromAscii( pArr->pImplementationNm )  );
            xNewKey = xNewKey->createKey(
                    ::rtl::OUString::createFromAscii( "/UNO/SERVICES" ) );
            xNewKey->createKey(
                    ::rtl::OUString::createFromAscii( pArr->pServiceNm ) );
        }
    }
    return sal_True;
}

void* SAL_CALL component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
{
    void* pRet = NULL;

    ::com::sun::star::lang::XMultiServiceFactory* pServiceManager =
        reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >
            ( _pServiceManager );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XSingleServiceFactory > xFactory;

    for( const InstancesArray* pArr = aInstances; pArr->pServiceNm; ++pArr )
    {
        if( 0 == rtl_str_compare( sImplementationName, pArr->pImplementationNm ) )
        {
            ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                ::rtl::OUString::createFromAscii( pArr->pServiceNm );
            xFactory = ::cppu::createSingleFactory(
                    pServiceManager, aServiceNames.getArray()[0],
                    *pArr->pFn, aServiceNames );
            break;
        }
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

}   // "C"
