#include <cppuhelper/implbase3.hxx> // "3" implementing three interfaces
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <my_module/XSomething.hpp>


using namespace ::rtl; // for OUString
using namespace ::com::sun::star; // for odk interfaces
using namespace ::com::sun::star::uno; // for basic types


namespace my_sc_impl
{

extern Sequence< OUString > SAL_CALL  getSupportedServiceNames_MyService1Impl();
extern OUString SAL_CALL getImplementationName_MyService1Impl();
extern Reference< XInterface > SAL_CALL create_MyService1Impl(
                                        Reference< XComponentContext > const & xContext )
                                            SAL_THROW( () );

static Sequence< OUString > getSupportedServiceNames_MyService2Impl()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
//      MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("my_module.MyService2"));
            pNames = &seqNames;
        }
    }
    return *pNames;
}

static OUString getImplementationName_MyService2Impl()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
//      MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pImplName )
        {
            static OUString implName( RTL_CONSTASCII_USTRINGPARAM("my_module.my_sc_implementation.MyService2") );
            pImplName = &implName;
        }
    }
    return *pImplName;
}

class MyService2Impl : public ::cppu::WeakImplHelper3<
      ::my_module::XSomething, lang::XServiceInfo, lang::XInitialization >
{
    OUString m_arg;
public:
    // focus on three given interfaces,
    // no need to implement XInterface, XTypeProvider, XWeak

    // XInitialization will be called upon createInstanceWithArguments[AndContext]()
    virtual void SAL_CALL initialize( Sequence< Any > const & args )
        throw (Exception);
    // XSomething
    virtual OUString SAL_CALL methodOne( OUString const & str )
        throw (RuntimeException);
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};
// XInitialization implemention
void MyService2Impl::initialize( Sequence< Any > const & args )
    throw (Exception)
{
    if (1 != args.getLength())
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("give a string instanciating this component!") ),
            (::cppu::OWeakObject *)this, // resolve to XInterface reference
            0 ); // argument pos
    }
    if (! (args[ 0 ] >>= m_arg))
    {
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no string given as argument!") ),
            (::cppu::OWeakObject *)this, // resolve to XInterface reference
            0 ); // argument pos
    }
}
// XSomething implementation
OUString MyService2Impl::methodOne( OUString const & str )
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "called methodOne() of MyService2 implementation: ") ) + m_arg + str;
}
// XServiceInfo implementation
OUString MyService2Impl::getImplementationName()
    throw (RuntimeException)
{
    // unique implementation name
    return OUString( RTL_CONSTASCII_USTRINGPARAM("my_module.my_sc_impl.MyService2") );
}
sal_Bool MyService2Impl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    // this object only supports one service, so the test is simple
    return serviceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("my_module.MyService2") );
}
Sequence< OUString > MyService2Impl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return getSupportedServiceNames_MyService2Impl();
}

Reference< XInterface > SAL_CALL create_MyService2Impl(
    Reference< XComponentContext > const & xContext )
    SAL_THROW( () )
{
    return static_cast< lang::XTypeProvider * >( new MyService2Impl() );
}

}

/* shared lib exports implemented without helpers in service_impl1.cxx */
namespace my_sc_impl
{
static struct ::cppu::ImplementationEntry s_component_entries [] =
{
    {
        create_MyService1Impl, getImplementationName_MyService1Impl,
        getSupportedServiceNames_MyService1Impl, ::cppu::createSingleComponentFactory,
        0, 0
    },
    {
        create_MyService2Impl, getImplementationName_MyService2Impl,
        getSupportedServiceNames_MyService2Impl, ::cppu::createSingleComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};
}

extern "C"
{
void SAL_CALL component_getImplementationEnvironment(
    sal_Char const ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
sal_Bool SAL_CALL component_writeInfo(
    lang::XMultiServiceFactory * xMgr, registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_writeInfoHelper(
        xMgr, xRegistry, ::my_sc_impl::s_component_entries );
}
void * SAL_CALL component_getFactory(
    sal_Char const * implName, lang::XMultiServiceFactory * xMgr,
    registry::XRegistryKey * xRegistry )
{
    return ::cppu::component_getFactoryHelper(
        implName, xMgr, xRegistry, ::my_sc_impl::s_component_entries );
}
}


