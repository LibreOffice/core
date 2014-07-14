/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "vbachart.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <basic/sberrors.hxx>
#include "vbachartobject.hxx"
#include "vbachartobjects.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

const rtl::OUString CHART_NAME( RTL_CONSTASCII_USTRINGPARAM("Name") );
const rtl::OUString PERSIST_NAME( RTL_CONSTASCII_USTRINGPARAM("PersistName") );

ScVbaChartObject::ScVbaChartObject( const css::uno::Reference< ov::XHelperInterface >& _xParent, const css::uno::Reference< css::uno::XComponentContext >& _xContext, const css::uno::Reference< css::table::XTableChart >& _xTableChart, const css::uno::Reference< css::drawing::XDrawPageSupplier >& _xDrawPageSupplier ) : ChartObjectImpl_BASE( _xParent, _xContext ), xTableChart( _xTableChart ), xDrawPageSupplier( _xDrawPageSupplier )
{
    xDrawPage = xDrawPageSupplier->getDrawPage();
    xEmbeddedObjectSupplier.set( xTableChart, uno::UNO_QUERY_THROW );
    xNamed.set( xTableChart, uno::UNO_QUERY_THROW );
    sPersistName = getPersistName();
    xShape = setShape();
// #i121178#: don't set the persist name to the object but the OLE object's name(displaying name)
//    setName(sPersistName);
    setName(xNamed->getDisplayName());
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
        // #TODO #FIXME should be ThisWorkbook or equivalent, or in
        // fact probably the chart object should be created with
        // the XModel owner
        //uno::Reference< view::XSelectionSupplier > xSelectionSupplier( getXModel().getCurrentController());
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier( getCurrentExcelDoc(mxContext)->getCurrentController(), uno::UNO_QUERY_THROW );
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
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.ChartObject" ) );
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
