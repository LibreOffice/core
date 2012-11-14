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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/charthelper.hxx>
#include <svtools/embedhlp.hxx>
#include <tools/globname.hxx>
#include <sot/clsids.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/graphic/XPrimitiveFactory2D.hpp>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

bool ChartHelper::IsChart(const svt::EmbeddedObjectRef& xObjRef)
{
    if(!xObjRef.is())
    {
        return false;
    }

    const SvGlobalName aObjClsId(xObjRef->getClassID());

    if(SvGlobalName(SO3_SCH_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_60) == aObjClsId)
    {
        return true;
    }

    return false;
}

drawinglayer::primitive2d::Primitive2DSequence ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
    const uno::Reference< ::frame::XModel >& rXModel,
    basegfx::B2DRange& rRange)
{
    drawinglayer::primitive2d::Primitive2DSequence aRetval;

    if(rXModel.is())
    {
        try
        {
            const uno::Reference< lang::XMultiServiceFactory > xChartFact(rXModel, uno::UNO_QUERY_THROW);
            const uno::Reference< lang::XUnoTunnel > xChartView(xChartFact->createInstance(::rtl::OUString::createFromAscii("com.sun.star.chart2.ChartView")), uno::UNO_QUERY_THROW);
            const uno::Reference< util::XUpdatable > xUpdatable(xChartView, uno::UNO_QUERY_THROW);

            if(xUpdatable.is())
            {
                xUpdatable->update();

                const uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier(rXModel, uno::UNO_QUERY_THROW);
                const uno::Reference< container::XIndexAccess > xShapeAccess(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW);

                if(xShapeAccess.is() && xShapeAccess->getCount())
                {
                    const sal_Int32 nShapeCount(xShapeAccess->getCount());
                    const uno::Reference< lang::XMultiServiceFactory > xMgr(::comphelper::getProcessServiceFactory());
                    const uno::Reference< graphic::XPrimitiveFactory2D > xPrimitiveFactory(
                        xMgr->createInstance(
                            String(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.graphic.PrimitiveFactory2D" ))),
                            uno::UNO_QUERY);

                    if(xPrimitiveFactory.is())
                    {
                        const uno::Sequence< beans::PropertyValue > aParams;
                        uno::Reference< drawing::XShape > xShape;

                        for(sal_Int32 a(0); a < nShapeCount; a++)
                        {
                            xShapeAccess->getByIndex(a) >>= xShape;

                            if(xShape.is())
                            {
                                const drawinglayer::primitive2d::Primitive2DSequence aNew(
                                    xPrimitiveFactory->createPrimitivesFromXShape(
                                        xShape,
                                        aParams));

                                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(
                                    aRetval,
                                    aNew);
                            }
                        }
                    }
                }
            }
        }
        catch(uno::Exception&)
        {
            OSL_ENSURE(false, "Unexpected exception!");
        }

        if(aRetval.hasElements())
        {
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D;

            rRange = drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(aRetval, aViewInformation2D);
        }
    }

    return aRetval;
}

void ChartHelper::AdaptDefaultsForChart(
    const uno::Reference < embed::XEmbeddedObject > & xEmbObj,
    bool bNoFillStyle,
    bool bNoLineStyle)
{
    if( xEmbObj.is())
    {
        uno::Reference< chart2::XChartDocument > xChartDoc( xEmbObj->getComponent(), uno::UNO_QUERY );
        OSL_ENSURE( xChartDoc.is(), "Trying to set chart property to non-chart OLE" );
        if( !xChartDoc.is())
            return;

        try
        {
            // set background to transparent (none)
            uno::Reference< beans::XPropertySet > xPageProp( xChartDoc->getPageBackground());
            if( xPageProp.is())
                xPageProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("FillStyle")),
                                             uno::makeAny( drawing::FillStyle_NONE ));
            // set no border
            if( xPageProp.is())
                xPageProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("LineStyle")),
                                             uno::makeAny( drawing::LineStyle_NONE ));
        }
        catch( const uno::Exception & )
        {
            OSL_ENSURE( false, "Exception caught in AdaptDefaultsForChart" );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof
