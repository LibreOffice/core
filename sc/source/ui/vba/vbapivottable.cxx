/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapivottable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:59:15 $
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
#include "vbapivottable.hxx"
#include "vbapivotcache.hxx"


using namespace ::com::sun::star;
using namespace ::org::openoffice;

ScVbaPivotTable::ScVbaPivotTable( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XDataPilotTable >& xTable ) : PivotTableImpl_BASE( uno::Reference< vba::XHelperInterface >(),  xContext), m_xTable( xTable )
{
}

uno::Reference< excel::XPivotCache >
ScVbaPivotTable::PivotCache() throw (uno::RuntimeException)
{
    // #FIXME with a quick example failed to determine what the parent
    // should be, leaving as null at the moment
    return new ScVbaPivotCache( uno::Reference< vba::XHelperInterface >(), mxContext, m_xTable );
}

rtl::OUString&
ScVbaPivotTable::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPivotTable") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaPivotTable::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.PivotTable" ) );
    }
    return aServiceNames;
}
