/*************************************************************************
 *
 *  $RCSfile: testproptyphlp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:11 $
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
#include <cppuhelper/proptypehlp.hxx>

#include <com/sun/star/beans/Property.hpp>
#include <assert.h>

using namespace ::com::sun::star::uno;
using namespace ::cppu;

void testPropertyTypeHelper()
{
    Any a;

    a <<= ( sal_Int8 ) 25;

    sal_Int32 i;
    convertPropertyValue( i ,  a );
    assert( 25 == i );

    sal_Int16 i16;
    convertPropertyValue( i16 ,  a );
    assert( 25 == i16 );

    sal_Int8 i8;
    convertPropertyValue( i8 ,  a );
    assert( 25 == i8 );

    sal_uInt32 i32;
    convertPropertyValue( i32 ,  a );
    assert( 25 == i32 );

    double d;
    convertPropertyValue( d , a );
    assert( 25. == d );

    float f;
    convertPropertyValue( f , a );
    assert( 25. == f );

    ::com::sun::star::beans::Property prop;

    prop.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Huhu") );
    prop.Handle = 5;
    prop.Attributes = 3;

    a <<= prop;

    ::com::sun::star::beans::Property prop2;
    convertPropertyValue( prop2 , a );
     assert( prop.Handle == prop2.Handle && prop.Name == prop2.Name && prop.Attributes == prop2.Attributes );


    ::rtl::OUString ow;
    a <<= prop.Name;
    convertPropertyValue( ow , a );
    assert( ow == prop.Name );
}
