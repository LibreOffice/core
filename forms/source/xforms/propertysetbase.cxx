/*************************************************************************
 *
 *  $RCSfile: propertysetbase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:55:38 $
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

#include "propertysetbase.hxx"

#include <cppuhelper/typeprovider.hxx>  // for getImplementationId()

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Reference.hxx>


using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;

#define CPPUTYPE(x) getCppuType(static_cast<x*>(NULL))


PropertySetBase::PropertySetBase()
{
}

PropertySetBase::PropertySetBase( comphelper::PropertySetInfo* pInfo )
    throw()
    : PropertySetHelper( pInfo )
{
}


PropertySetBase::~PropertySetBase() throw()
{
}


Any PropertySetBase::queryInterface( Type const & rType )
  throw( RuntimeException )
{
    Any aResult;
    if( rType == CPPUTYPE( Reference<XPropertySet> ) )
        aResult <<= Reference<XPropertySet>( this );
    else if( rType == CPPUTYPE( Reference<XMultiPropertySet> ) )
        aResult <<= Reference<XMultiPropertySet>( this );
    else if( rType == CPPUTYPE( Reference<XPropertyState> ) )
        aResult <<= Reference<XPropertyState>( this );
    else
        aResult = cppu::OWeakObject::queryInterface( rType );
    return aResult;
}


void PropertySetBase::acquire() throw ()
{
    cppu::OWeakObject::acquire();
}

void PropertySetBase::release() throw ()
{
    cppu::OWeakObject::release();
}


Sequence<Type> PropertySetBase::getTypes()
    throw( RuntimeException )
{
    Sequence<Type> aTypes(3);
    Type* pTypes = aTypes.getArray();
    pTypes[0] = CPPUTYPE( Reference<XPropertySet> );
    pTypes[1] = CPPUTYPE( Reference<XMultiPropertySet> );
    pTypes[2] = CPPUTYPE( Reference<XPropertyState> );
    return aTypes;
}

Sequence<sal_Int8> PropertySetBase::getImplementationId()
    throw( RuntimeException )
{
    static cppu::OImplementationId aImplementationId;

    return aImplementationId.getImplementationId();
}
