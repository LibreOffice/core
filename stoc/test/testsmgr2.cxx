/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
