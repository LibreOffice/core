#include <stdio.h>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;

int main()
{
    try
    {

        Reference< XSimpleRegistry > r1 =  createSimpleRegistry();
        Reference< XSimpleRegistry > r2 =  createSimpleRegistry();
        r1->open( OUString( RTL_CONSTASCII_USTRINGPARAM( "test1.rdb" ) ), sal_True, sal_False );
        r2->open( OUString( RTL_CONSTASCII_USTRINGPARAM( "test2.rdb" ) ), sal_True, sal_False );
        Reference< XSimpleRegistry > r = createNestedRegistry( );
        Reference< XInitialization > rInit( r, UNO_QUERY );
        Sequence< Any > seq( 2 );
        seq[0] <<= r1;
        seq[1] <<= r2;
        rInit->initialize( seq );

        Reference< XComponentContext > rComp = bootstrap_InitialComponentContext( r );

        Reference< XContentEnumerationAccess > xCtAccess( rComp->getServiceManager(), UNO_QUERY );

        Reference< XEnumeration > rEnum =
            xCtAccess->createContentEnumeration( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.bridge.Bridge" ) ) );

        sal_Int32 n = 0;
        while( rEnum->hasMoreElements() )
        {
            Reference< XServiceInfo > r;
            rEnum->nextElement() >>= r;
            OString o = OUStringToOString( r->getImplementationName() , RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n" , o.getStr() );
            Sequence< OUString > seq = r->getSupportedServiceNames();
            for( int i = 0 ;i < seq.getLength() ; i ++  )
            {
                o = OUStringToOString( seq[i] , RTL_TEXTENCODING_ASCII_US );
                printf( "   %s\n" , o.getStr() );
            }
            n ++;
        }
        // there are two services in two registries !
        OSL_ASSERT( 2 == n );
        if( 2 == n )
        {
            printf( "test passed\n" );
        }
    }
    catch ( Exception & e )
    {
        OString o =  OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
        printf( "%s\n" , o.getStr() );
        OSL_ASSERT( 0 );
    }
    return 0;
}
