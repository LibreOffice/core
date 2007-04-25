/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbachartobject.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:02:44 $
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
#include "vbachart.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "vbachartobject.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

const rtl::OUString CHART_NAME( RTL_CONSTASCII_USTRINGPARAM("Name") );

::rtl::OUString SAL_CALL
ScVbaChartObject::getName() throw (css::uno::RuntimeException)
{
    rtl::OUString sName;
    uno::Reference< container::XNamed > xNamed( m_xTableChart, uno::UNO_QUERY );
    if ( xNamed.is() )
        sName = xNamed->getName();
    return sName;
}



uno::Reference< excel::XChart > SAL_CALL
ScVbaChartObject::getChart() throw (css::uno::RuntimeException)
{
    uno::Reference< document::XEmbeddedObjectSupplier > xEmbed( m_xTableChart, uno::UNO_QUERY_THROW );
    uno::Reference< chart::XChartDocument > xChart( xEmbed->getEmbeddedObject(), uno::UNO_QUERY_THROW );

    // #TODO check with vba to see whether its valid to return a
    // null object for the Chart property. atm, we throw ( i.e. if in
    // doubt... throw )

    return new ScVbaChart( m_xContext, xChart );
}
/*
#include<cppuhelper/implbase1.hxx>
#include<org/openoffice/vba/XChartObject.hpp>
#include"vbacollectionimpl.hxx"

typedef ::cppu::ImplInheritanceHelper1< ScVbaCollectionBaseImpl, oo::vba::XChartObject > XSeriesCollectionBase;
class MyCollectionImpl : public XSeriesCollectionBase
{
public:
    //MyCollectionImpl( css::uno::Reference< css::uno::XComponentContext >& xContext ) : ScVbaCollectionBaseImpl( xContext ) {}
    MyCollectionImpl( css::uno::Reference< css::uno::XComponentContext >& xContext ) : XSeriesCollectionBase( xContext ) {}
        virtual ::rtl::OUString SAL_CALL getName() throw (css::uno::RuntimeException)
    {
        OSL_TRACE("**** In Get Name for mangled object");
        rtl::OUString dummy;
        return dummy;
    }
        virtual css::uno::Any test() throw (css::uno::RuntimeException)
    {
        OSL_TRACE("**** In test  for mangled object");
        return uno::Any();
    }
    virtual css::uno::Reference< oo::vba::XChart > SAL_CALL getChart() throw (css::uno::RuntimeException)
    {
        OSL_TRACE("**** In getChart  for mangled object");
        return css::uno::Reference< oo::vba::XChart >();
    }

};
*/

uno::Any SAL_CALL
ScVbaChartObject::test()throw (css::uno::RuntimeException)
{
    return uno::Any();
}


