/***************************************************************************************************
 ***************************************************************************************************
 *
 * service implementation:   foo.Counter
 * exported interfaces:      foo.XCounter
 *
 * simple example component implementing a counter
 *
 ***************************************************************************************************
 **************************************************************************************************/

#include <stdio.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx> // helper for queryInterface() impl
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx> // helper for component factory
#endif
// generated c++ interfaces

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _FOO_XCOUNTABLE_HPP_
#include <foo/XCountable.hpp>
#endif

#define SERVICENAME "foo.Counter"
#define IMPLNAME    "com.sun.star.comp.example.cpp.Counter"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::foo;


//==================================================================================================
class MyCounterImpl
    : public XCountable
    , public XServiceInfo
{
    // to obtain other services if needed
    Reference< XMultiServiceFactory > m_xServiceManager;

    sal_Int32 m_nRefCount;
    sal_Int32 m_nCount;

public:
    MyCounterImpl( const Reference< XMultiServiceFactory > & xServiceManager )
        : m_xServiceManager( xServiceManager ), m_nRefCount( 0 )
        { printf( "< MyCounterImpl ctor called >\n" ); }
    ~MyCounterImpl()
        { printf( "< MyCounterImpl dtor called >\n" ); }

    // XInterface implementation
    virtual void SAL_CALL acquire() throw ()
        { ++m_nRefCount; }
    virtual void SAL_CALL release() throw ()
        { if (! --m_nRefCount) delete this; }
    virtual Any SAL_CALL queryInterface( const Type & rType ) throw (RuntimeException)
        { return cppu::queryInterface(rType,
                                      static_cast< XInterface* >( static_cast< XServiceInfo* >( this ) ),
                                      static_cast< XCountable* >( this ),
                                      static_cast< XServiceInfo* >( this ) ); }

    // XServiceInfo implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XCountable implementation
    virtual sal_Int32 SAL_CALL getCount() throw (RuntimeException)
        { return m_nCount; }
    virtual void SAL_CALL setCount( sal_Int32 nCount ) throw (RuntimeException)
        { m_nCount = nCount; }
    virtual sal_Int32 SAL_CALL increment() throw (RuntimeException)
        { return (++m_nCount); }
    virtual sal_Int32 SAL_CALL decrement() throw (RuntimeException)
        { return (--m_nCount); }
};

//*************************************************************************
OUString SAL_CALL MyCounterImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}

//*************************************************************************
sal_Bool SAL_CALL MyCounterImpl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL MyCounterImpl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL MyCounterImpl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}




/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL MyCounterImpl_create(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference< XCountable >( new MyCounterImpl( xMgr ) );
}


//##################################################################################################
//#### EXPORTED ####################################################################################
//##################################################################################################


/**
 * Gives the environment this component belongs to.
 */
extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 *
 * for each supported service.
 * @param pServiceManager   the service manager
 * @param pRegistryKey      the registry key
 */
extern "C" sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey)
{
    sal_Bool result = sal_False;

    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                MyCounterImpl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            // we should not ignore exceptions
        }
    }
    return result;
}

/**
 * This function is called to get service factories for an implementation.
 *
 * @param pImplName       name of implementation
 * @param pServiceManager a service manager, need for component creation
 * @param pRegistryKey    the registry key for this component, need for persistent data
 * @return a component factory
 */
extern "C" void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey)
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            MyCounterImpl_create,
            MyCounterImpl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
