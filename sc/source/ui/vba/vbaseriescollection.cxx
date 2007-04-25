/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaseriescollection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:10:51 $
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
#include "vbaseriescollection.hxx"
#include <org/openoffice/excel/XSeries.hpp>

#include "vbaglobals.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

uno::Reference< oo::excel::XApplication >
ScVbaSeriesCollection::getApplication() throw (uno::RuntimeException)
{
    return ScVbaGlobals::getGlobalsImpl( m_xContext )->getApplication();
}
::sal_Int32
ScVbaSeriesCollection::getCount() throw (uno::RuntimeException)
{
    return 0;
}

uno::Any
ScVbaSeriesCollection::Item( const uno::Any& /*aIndex*/ ) throw (uno::RuntimeException)
{
    // #TODO #FIXME some implementation would be nice !!
    return uno::Any();
}

// XEnumerationAccess

uno::Reference< container::XEnumeration >
ScVbaSeriesCollection::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnum;
    return xEnum;
}

// XElementAccess

uno::Type
ScVbaSeriesCollection::getElementType() throw (uno::RuntimeException)
{
    return excel::XSeries::static_type(0);
}
::sal_Bool
ScVbaSeriesCollection::hasElements() throw (uno::RuntimeException)
{
    // #TODO #TOFIX Really?, how can we say that!
    // needs to delegate to xIndex
    return sal_True;
}

uno::Any SAL_CALL
ScVbaSeriesCollection::getParent() throw (uno::RuntimeException)
{
    uno::Reference< excel::XApplication > xApplication =
        getApplication();
    uno::Reference< excel::XWorkbook > xWorkbook;
    if ( xApplication.is() )
    {
        xWorkbook = xApplication->getActiveWorkbook();
    }
    return uno::Any( xWorkbook );
}

sal_Int32 SAL_CALL
ScVbaSeriesCollection::getCreator() throw (uno::RuntimeException)
{
    // #TODO# #FIXME# implementation?
    return 0;
}





