#include "factory.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <cppuhelper/factory.hxx>

#include "root.hxx"

using namespace ::com::sun::star;
using namespace layoutimpl;

extern "C"
{

    TOOLKIT_DLLPUBLIC void
    SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    TOOLKIT_DLLPUBLIC void * SAL_CALL
    component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
    {
        void * pRet = 0;

        ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        if ( pServiceManager && aImplName.equals( LayoutFactory::impl_staticGetImplementationName() ) )
            xFactory = ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                         LayoutFactory::impl_staticGetImplementationName(),
                                                         LayoutFactory::impl_staticCreateSelfInstance,
                                                         LayoutFactory::impl_staticGetSupportedServiceNames() );
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }

        return pRet;
    }

    TOOLKIT_DLLPUBLIC sal_Bool SAL_CALL
    component_writeInfo( void * /*pServiceManager*/, void * pRegistryKey )
    {
        if ( pRegistryKey )
        {
            try
            {
                uno::Reference< registry::XRegistryKey > xKey( reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ) );
                uno::Reference< registry::XRegistryKey >  xNewKey;

                xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                           LayoutFactory::impl_staticGetImplementationName() +
                                           ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

                const uno::Sequence< ::rtl::OUString > aServices = LayoutFactory::impl_staticGetSupportedServiceNames();
                for( sal_Int32 i = 0; i < aServices.getLength(); i++ )
                    xNewKey->createKey( aServices.getConstArray()[i] );

                return sal_True;
            }
            catch (registry::InvalidRegistryException &)
            {   OSL_ENSURE( sal_False, "### InvalidRegistryException!" );   }
        }
        return sal_False;
    }

} // extern "C"

// Component registration
::rtl::OUString SAL_CALL LayoutFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.awt.Layout" );
}

uno::Sequence< ::rtl::OUString > SAL_CALL LayoutFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.awt.Layout");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.awt.Layout");
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL LayoutFactory::impl_staticCreateSelfInstance(
    const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new LayoutFactory( xServiceManager ) );
}

// XServiceInfo
::rtl::OUString SAL_CALL LayoutFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

uno::Sequence< ::rtl::OUString > SAL_CALL LayoutFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

sal_Bool SAL_CALL LayoutFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();
    for( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
        if ( ServiceName.compareTo( aSeq[i] ) == 0 )
            return sal_True;

    return sal_False;
}

// XSingleServiceFactory
uno::Reference< uno::XInterface > SAL_CALL LayoutFactory::createInstance()
    throw ( uno::Exception,
            uno::RuntimeException )
{
    return uno::Reference< uno::XInterface >(
        static_cast< OWeakObject* >( new LayoutRoot( m_xFactory ) ),
        uno::UNO_QUERY );
}

uno::Reference< uno::XInterface > SAL_CALL LayoutFactory::createInstanceWithArguments(
    const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException )
{
    uno::Reference< uno::XInterface > layout = createInstance();
    uno::Reference< lang::XInitialization > xInit( layout, uno::UNO_QUERY );
    xInit->initialize( aArguments );
    return layout;
}
