/*************************************************************************
 *
 *  $RCSfile: testidlclass.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <assert.h>

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
    static Reference < XIdlClass > r =
                ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0
                                            );

    assert( r.is() );


    {  // test the xidlclassprovider interface !
        Reference< XIdlClassProvider > rProv( r , UNO_QUERY );
        assert( rProv.is() );

        {
            Sequence < Reference < XIdlClass > > seq = rProv->getIdlClasses();

            // is always one
            assert( seq.getLength() == 1 );

            // test the weak reference
            rProv->getIdlClasses();

        }

        rProv->getIdlClasses();


    }


    assert( r->getName() == sImplName );

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
    assert( r2->equals( r ) );

    Sequence < Reference < XIdlClass > > seqIdlClass = r->getInterfaces();

    //TODO !
    // one idl class for one interface
    // this test fails, if core reflection fails !
//  assert( 1 == seqIdlClass.getLength() );
//  Reference < XIdlClass > rIdlInterface = seqIdlClass.getArray()[0];

    // check for IdlClass interface returned by Core Reflection
//  assert( rIdlInterface.is() );



    // Test all ten templates
    ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );

    // Test all ten templates
    ::cppu::createStandardClass(
                                rSMgr ,
                                sImplName,
                                Reference < XIdlClass > () ,
                                (XMultiServiceFactory * ) 0 ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0  ,
                                (XServiceInfo * ) 0
                                            );


    // Test all ten templates
    ::cppu::createStandardClass(
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
    ::cppu::createStandardClass(
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
    ::cppu::createStandardClass(
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
    ::cppu::createStandardClass(
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
    ::cppu::createStandardClass(
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
    ::cppu::createStandardClass(
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
