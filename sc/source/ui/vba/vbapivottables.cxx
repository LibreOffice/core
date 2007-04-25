/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapivottables.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:10:07 $
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
#include "vbapivottables.hxx"
#include "vbapivottable.hxx"
#include <com/sun/star/sheet/XDataPilotTable.hpp>
#include <org/openoffice/excel/XPivotTable.hpp>


using namespace ::com::sun::star;
using namespace ::org::openoffice;

uno::Any DataPilotToPivotTable( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext )
{
    uno::Reference< sheet::XDataPilotTable > xTable( aSource, uno::UNO_QUERY_THROW );
    return uno::makeAny( uno::Reference< excel::XPivotTable > ( new ScVbaPivotTable( xContext, xTable ) ) );
}

class PivotTableEnumeration : public EnumerationHelperImpl
{
public:
    PivotTableEnumeration( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xContext, xEnumeration ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return DataPilotToPivotTable( m_xEnumeration->nextElement(),  m_xContext );
    }

};

ScVbaPivotTables::ScVbaPivotTables( const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess  ):  ScVbaPivotTables_BASE( xContext, xIndexAccess )
{
}

uno::Reference< container::XEnumeration >
ScVbaPivotTables::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new PivotTableEnumeration( m_xContext, xEnumAccess->createEnumeration() );
}

uno::Any
ScVbaPivotTables::createCollectionObject( const css::uno::Any& aSource )
{
    return DataPilotToPivotTable( aSource,  m_xContext );
}

uno::Type
ScVbaPivotTables::getElementType() throw (uno::RuntimeException)
{
    return excel::XPivotTables::static_type(0);
}
