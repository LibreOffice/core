/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
#include "vbapages.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;
using namespace ::vos;

ScVbaPages::ScVbaPages( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xPages ) throw( lang::IllegalArgumentException ) : ScVbaPages_BASE( xParent, xContext, xPages )
{
}

uno::Type SAL_CALL
ScVbaPages::getElementType() throw (uno::RuntimeException)
{
    // return msforms::XPage::static_type(0);
    return uno::XInterface::static_type(0);
}

uno::Any
ScVbaPages::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString&
ScVbaPages::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPages") );
    return sImplName;
}

uno::Reference< container::XEnumeration > SAL_CALL
ScVbaPages::createEnumeration() throw (uno::RuntimeException)
{
    return uno::Reference< container::XEnumeration >();
}

uno::Sequence< rtl::OUString >
ScVbaPages::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msform.Pages" ) );
    }
    return aServiceNames;
}
