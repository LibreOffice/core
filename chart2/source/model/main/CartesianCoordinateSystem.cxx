/*************************************************************************
 *
 *  $RCSfile: CartesianCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:30 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "CartesianCoordinateSystem.hxx"
#include "IdentityTransformation.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

CartesianCoordinateSystem::CartesianCoordinateSystem( sal_Int32 nDim /* = 2 */ ) :
        m_aTransformationToCartesian( new IdentityTransformation( nDim ) ),
        m_aTransformationFromCartesian( new IdentityTransformation( nDim ) ),
        m_nDim( nDim )
{
}

CartesianCoordinateSystem::~CartesianCoordinateSystem()
{}

// ____ XCoordinateSystem ____
::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation > SAL_CALL
    CartesianCoordinateSystem::getTransformationToCartesian()
    throw (RuntimeException)
{
    return m_aTransformationToCartesian;
}

::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XTransformation > SAL_CALL
    CartesianCoordinateSystem::getTransformationFromCartesian()
    throw (RuntimeException)
{
    return m_aTransformationFromCartesian;
}

sal_Int32 SAL_CALL CartesianCoordinateSystem::getDimension() throw (RuntimeException)
{
    return m_nDim;
}


// ____ XServiceInfo ____

Sequence< OUString > CartesianCoordinateSystem::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices;
//     Sequence< OUString > aServices( 1 );
//     aServices[ 0 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "drafts.com.sun.star.chart2.CoordinateSystem" ));
    return aServices;
}

APPHELPER_XSERVICEINFO_IMPL( CartesianCoordinateSystem,
                             C2U( "com.sun.star.comp.chart.CartesianCoordinateSystem" ))

}  // namespace chart
