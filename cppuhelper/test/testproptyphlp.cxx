/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testproptyphlp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 17:21:12 $
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
#include "precompiled_cppuhelper.hxx"
#include <cppuhelper/proptypehlp.hxx>

#include <com/sun/star/beans/Property.hpp>

#if OSL_DEBUG_LEVEL == 0
#undef  NDEBUG
#define NDEBUG
#endif
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
