#include "test_multiservicefac.hxx"


using namespace test_ftp;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


Any SAL_CALL
Test_MultiServiceFactory::queryInterface(
    const Type& rType
)
    throw(
        RuntimeException
    )
{
    Any aRet = ::cppu::queryInterface(rType,
                                      SAL_STATIC_CAST( XMultiServiceFactory*,
                                                       this ));

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );

}


void SAL_CALL Test_MultiServiceFactory::acquire( void ) throw()
{
    OWeakObject::acquire();
}


void SAL_CALL Test_MultiServiceFactory::release( void ) throw()
{
    OWeakObject::release();
}

        // XMultiServiceFactory

 Reference<
XInterface > SAL_CALL
Test_MultiServiceFactory::createInstance(
    const ::rtl::OUString& aServiceSpecifier
)
    throw (
        Exception,
        RuntimeException
    )
{
    return Reference<
        XInterface >(0);
}


Reference<
XInterface > SAL_CALL
Test_MultiServiceFactory::createInstanceWithArguments(
    const ::rtl::OUString& ServiceSpecifier,
    const Sequence
    < Any >& Arguments
)
    throw (
        Exception,
        RuntimeException
    )
{
    return Reference<
        XInterface >(0);
}

Sequence< ::rtl::OUString > SAL_CALL
Test_MultiServiceFactory::getAvailableServiceNames(
)
    throw (
        RuntimeException
    )
{
    return Sequence< ::rtl::OUString >(0);
}
