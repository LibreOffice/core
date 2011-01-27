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
#include "precompiled_cppuhelper.hxx"

#include <cppuhelper/stdidlclass.hxx>

#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include "testhelper.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::reflection;
using namespace ::rtl;




void testidlclass( const Reference < XMultiServiceFactory > &rSMgr)
{
    const OUString sImplName( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.cpputest") ) );

    // this reference is static to test behaviour during exiting application
    Reference < XIdlClass > r =
                ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0
                                            );

    OSL_ASSERT( r.is() );


    {  // test the xidlclassprovider interface !
        Reference< XIdlClassProvider > rProv( r , UNO_QUERY );
        OSL_ASSERT( rProv.is() );

        {
            Sequence < Reference < XIdlClass > > seq = rProv->getIdlClasses();

            // is always one
            OSL_ASSERT( seq.getLength() == 1 );

            // test the weak reference
            rProv->getIdlClasses();

        }

        rProv->getIdlClasses();


    }


    OSL_ASSERT( r->getName() == sImplName );

    // test equals
    Reference < XIdlClass > r2 =
                ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0
                                            );
    // test for implementation name
    OSL_ASSERT( r2->equals( r ) );

    Sequence < Reference < XIdlClass > > seqIdlClass = r->getInterfaces();

    //TODO !
    // one idl class for one interface
    // this test fails, if core reflection fails !
//  OSL_ASSERT( 1 == seqIdlClass.getLength() );
//  Reference < XIdlClass > rIdlInterface = seqIdlClass.getArray()[0];

    // check for IdlClass interface returned by Core Reflection
//  OSL_ASSERT( rIdlInterface.is() );



    // Test all ten templates
    Reference< XIdlClass > x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );

    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );


    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );



    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );



    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );


    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );




    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );

    // Test all ten templates
    x = ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
