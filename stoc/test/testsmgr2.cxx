/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"
#include <stdio.h>

#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;

SAL_IMPLEMENT_MAIN()
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
            Reference< XServiceInfo > r3;
            rEnum->nextElement() >>= r3;
            OString o = OUStringToOString( r3->getImplementationName() , RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n" , o.getStr() );
            Sequence< OUString > seq2 = r3->getSupportedServiceNames();
            for( int i = 0 ;i < seq2.getLength() ; i ++  )
            {
                o = OUStringToOString( seq2[i] , RTL_TEXTENCODING_ASCII_US );
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

        Reference< XComponent > xComp( rComp, UNO_QUERY );
        xComp->dispose();
        try
        {
            xCtAccess->createContentEnumeration(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "blabla" ) ) );
        }
        catch (DisposedException &)
        {
            printf( "already disposed results in DisposedException: ok.\n" );
            return 0;
        }
        fprintf( stderr, "missing DisposedException!\n" );
        return 1;
    }
    catch ( Exception & e )
    {
        OString o =  OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
        printf( "%s\n" , o.getStr() );
        OSL_ASSERT( 0 );
        return 1;
    }
}
