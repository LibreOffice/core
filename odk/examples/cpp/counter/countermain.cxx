/***************************************************************************************************
 ***************************************************************************************************
 *
 * simple client application registering and using the counter component.
 *
 ***************************************************************************************************
 **************************************************************************************************/

#include <stdio.h>

#include <rtl/ustring.hxx>

#include <osl/diagnose.h>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>

// generated c++ interfaces
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <foo/XCountable.hpp>


using namespace foo;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using namespace ::rtl;


//==================================================================================================
int SAL_CALL main(int argc, char **argv)
{
    Reference< XSimpleRegistry > xReg = createSimpleRegistry();
    OSL_ENSURE( xReg.is(), "### cannot get service instance of \"com.sun.star.regiystry.SimpleRegistry\"!" );

    xReg->open(OUString::createFromAscii("counter.rdb"), sal_False, sal_False);
    OSL_ENSURE( xReg->isValid(), "### cannot open test registry \"counter.rdb\"!" );

    Reference< XComponentContext > xContext = bootstrap_InitialComponentContext(xReg);
    OSL_ENSURE( xContext.is(), "### cannot creage intial component context!" );

    Reference< XMultiComponentFactory > xMgr = xContext->getServiceManager();
    OSL_ENSURE( xMgr.is(), "### cannot get initial service manager!" );

    // register my counter component
    Reference< XImplementationRegistration > xImplReg(
        xMgr->createInstanceWithContext(OUString::createFromAscii("com.sun.star.registry.ImplementationRegistration"), xContext), UNO_QUERY);
    OSL_ENSURE( xImplReg.is(), "### cannot get service instance of \"com.sun.star.registry.ImplementationRegistration\"!" );

    if (xImplReg.is())
    {
        xImplReg->registerImplementation(
            OUString::createFromAscii("com.sun.star.loader.SharedLibrary"), // loader for component
#ifdef UNX
            OUString::createFromAscii("libcounter.so"),     // component location
#else
            OUString::createFromAscii("counter.dll"),       // component location
#endif
            Reference< XSimpleRegistry >()   // registry omitted,
                                             // defaulting to service manager registry used
            );

        // get a counter instance
        Reference< XInterface > xx ;
        xx = xMgr->createInstanceWithContext(OUString::createFromAscii("foo.Counter"), xContext);
        Reference< XCountable > xCount( xx, UNO_QUERY );
        OSL_ENSURE( xCount.is(), "### cannot get service instance of \"foo.Counter\"!" );

        if (xCount.is())
        {
            xCount->setCount( 42 );
            fprintf( stdout , "%d," , xCount->getCount() );
            fprintf( stdout , "%d," , xCount->increment() );
            fprintf( stdout , "%d\n" , xCount->decrement() );
        }
    }

    Reference< XComponent >::query( xContext )->dispose();
    return 0;
}
