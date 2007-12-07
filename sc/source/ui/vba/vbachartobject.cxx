/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbachartobject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:46:09 $
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
#include <com/sun/star/script/BasicErrorException.hpp>
#include <basic/sberrors.hxx>
#include "vbachartobject.hxx"
#include "vbachartobjects.hxx"

using namespace ::com::sun::star;
using namespace ::org::openoffice;

const rtl::OUString CHART_NAME( RTL_CONSTASCII_USTRINGPARAM("Name") );
const rtl::OUString PERSIST_NAME( RTL_CONSTASCII_USTRINGPARAM("PersistName") );

ScVbaChartObject::ScVbaChartObject( const css::uno::Reference< oo::vba::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableChart >& _xTableChart, const css::uno::Reference< css::drawing::XDrawPageSupplier >& _xDrawPageSupplier ) : ChartObjectImpl_BASE( _xParent, _xContext ), xTableChart( _xTableChart ), xDrawPageSupplier( _xDrawPageSupplier )
{
        xDrawPage = xDrawPageSupplier->getDrawPage();
        xEmbeddedObjectSupplier.set( xTableChart, uno::UNO_QUERY_THROW );
        xNamed.set( xTableChart, uno::UNO_QUERY_THROW );
        sPersistName = getPersistName();
        xShape = setShape();
        setName(sPersistName);
        oShapeHelper.reset(new ShapeHelper(xShape));
}

rtl::OUString ScVbaChartObject::getPersistName()
{
    if ( !sPersistName.getLength() )
        sPersistName = xNamed->getName();
    return sPersistName;
}

uno::Reference< drawing::XShape >
ScVbaChartObject::setShape() throw ( script::BasicErrorException )
{
    try
    {
        sal_Int32 nItems = xDrawPage->getCount();
        for (int i = 0; i < nItems; i++)
        {
            xShape.set( xDrawPage->getByIndex(i), uno::UNO_QUERY_THROW );
            if (xShape->getShapeType().compareToAscii("com.sun.star.drawing.OLE2Shape") == 0 )
            {
                uno::Reference< beans::XPropertySet > xShapePropertySet(xShape, uno::UNO_QUERY_THROW );
                rtl::OUString sName;
                xShapePropertySet->getPropertyValue(PERSIST_NAME ) >>=sName;
                if ( sName.equals(sPersistName))
                {
                    xNamedShape.set( xShape, uno::UNO_QUERY_THROW );
                    return xShape;
                }
            }
        }
    }
    catch (uno::Exception& )
    {
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString() );
    }
    return NULL;
}

void SAL_CALL
ScVbaChartObject::setName( const rtl::OUString& sName ) throw (css::uno::RuntimeException)
{
    xNamedShape->setName(sName);
}


::rtl::OUString SAL_CALL
ScVbaChartObject::getName() throw (css::uno::RuntimeException)
{
    return xNamedShape->getName();
}

void SAL_CALL
ScVbaChartObject::Delete() throw ( css::script::BasicErrorException )
{
    // parent of this object is sheet
    uno::Reference< excel::XWorksheet > xParent( getParent(), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XChartObjects > xColl( xParent->ChartObjects( uno::Any() ), uno::UNO_QUERY_THROW );
    ScVbaChartObjects* pChartObjectsImpl = static_cast< ScVbaChartObjects* >( xColl.get() );
    if (pChartObjectsImpl)
        pChartObjectsImpl->removeByName( getPersistName() );
    else
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Parent is not ChartObjects" ) ) );
}

void
ScVbaChartObject::Activate() throw ( script::BasicErrorException )
{
    try
    {
        // #TODO #FIXME should be ThisWorkbook or equivelant
        // XModel
        //uno::Reference< view::XSelectionSupplier > xSelectionSupplier( getXModel().getCurrentController());
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier( getCurrentDocument()->getCurrentController(), uno::UNO_QUERY_THROW );
        xSelectionSupplier->select(uno::makeAny(xShape));
    }
    catch (uno::Exception& )
    {
        throw script::BasicErrorException( rtl::OUString(), uno::Reference< uno::XInterface >(), SbERR_METHOD_FAILED, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ChartObject Activate internal error" ) ) );
    }
}

uno::Reference< excel::XChart > SAL_CALL
ScVbaChartObject::getChart() throw (css::uno::RuntimeException)
{
    return new ScVbaChart( this, mxContext, xEmbeddedObjectSupplier->getEmbeddedObject(), xTableChart );
}

rtl::OUString&
ScVbaChartObject::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaChartObject") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaChartObject::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.ChartObject" ) );
    }
    return aServiceNames;
}

double
ScVbaChartObject::getHeight()
{
    return oShapeHelper->getHeight();
}

void
ScVbaChartObject::setHeight(double _fheight) throw ( script::BasicErrorException )
{
    oShapeHelper->setHeight(_fheight);
}

double
ScVbaChartObject::getWidth()
{
        return oShapeHelper->getWidth();
}

void
ScVbaChartObject::setWidth(double _fWidth) throw ( script::BasicErrorException )
{
    oShapeHelper->setWidth(_fWidth);
}

double
ScVbaChartObject::getLeft()
{
        return oShapeHelper->getLeft();
}

void
ScVbaChartObject::setLeft(double _fLeft)
{
    oShapeHelper->setLeft(_fLeft);
}

double
ScVbaChartObject::getTop()
{
        return oShapeHelper->getTop();
}

void
ScVbaChartObject::setTop(double _fTop)
{
    oShapeHelper->setTop(_fTop);
}

uno::Reference< uno::XInterface >
ScVbaChartObject::getUnoObject() throw (script::BasicErrorException)
{
    return uno::Reference< uno::XInterface >( xShape, uno::UNO_QUERY );
}
