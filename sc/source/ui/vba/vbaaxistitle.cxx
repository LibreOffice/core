/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaaxistitle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:44:14 $
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
#include "vbaaxistitle.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

ScVbaAxisTitle::ScVbaAxisTitle( const uno::Reference< vba::XHelperInterface >& xParent,  const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& _xTitleShape ) :  AxisTitleBase( xParent, xContext, _xTitleShape )
{
}

rtl::OUString&
ScVbaAxisTitle::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaAxisTitle") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaAxisTitle::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        uno::Sequence< rtl::OUString > BaseServiceNames = AxisTitleBase::getServiceNames();
        aServiceNames.realloc( BaseServiceNames.getLength() + 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.AxisTitle" ) );
        for ( sal_Int32 index = 1; index < (BaseServiceNames.getLength() + 1); ++index )
            aServiceNames[ index ] = BaseServiceNames[ index ];
    }
    return aServiceNames;
}


