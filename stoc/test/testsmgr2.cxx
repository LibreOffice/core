/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testsmgr2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:23:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


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
