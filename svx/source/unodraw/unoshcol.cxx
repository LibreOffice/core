/*************************************************************************
 *
 *  $RCSfile: unoshcol.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-06 17:37:38 $
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

#include "unoshcol.hxx"
#include "unoprov.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

/***********************************************************************
*                                                                      *
***********************************************************************/
SvxShapeCollection::SvxShapeCollection() throw()
: maShapeContainer( maShapeContainerMutex )
{
}

//----------------------------------------------------------------------
SvxShapeCollection::~SvxShapeCollection() throw()
{
}


//----------------------------------------------------------------------
Reference< uno::XInterface > SvxShapeCollection_NewInstance() throw()
{
    Reference< drawing::XShapes > xShapes( new SvxShapeCollection() );
    Reference< uno::XInterface > xRef( xShapes, UNO_QUERY );
    return xRef;
}

// XShapes
//----------------------------------------------------------------------
void SAL_CALL SvxShapeCollection::add( const Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException )
{
    maShapeContainer.addInterface( xShape );
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeCollection::remove( const uno::Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException )
{
    maShapeContainer.removeInterface( xShape );
}

//----------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShapeCollection::getCount() throw( uno::RuntimeException )
{
    return maShapeContainer.getLength();
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeCollection::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence< Reference< uno::XInterface> > xElements( maShapeContainer.getElements() );

    Reference< drawing::XShape > xShape( xElements.getArray()[Index], UNO_QUERY );
    return uno::Any( &xShape, getElementType() );
}

// XElementAccess

//----------------------------------------------------------------------
uno::Type SAL_CALL SvxShapeCollection::getElementType() throw( uno::RuntimeException )
{
    return ::getCppuType(( const Reference< drawing::XShape >*)0);
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxShapeCollection::hasElements() throw( uno::RuntimeException )
{
    return getCount() != 0;
}

//----------------------------------------------------------------------
// XServiceInfo
//----------------------------------------------------------------------
OUString SAL_CALL SvxShapeCollection::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxShapeCollection") );
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq(1);
    aSeq.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shapes") );
    return aSeq;
}


