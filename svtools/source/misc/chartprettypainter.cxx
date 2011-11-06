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
#include "precompiled_svtools.hxx"

#include <svtools/chartprettypainter.hxx>

#include <tools/globname.hxx>
#include <sot/clsids.hxx>
// header for function rtl_createUuid
#include <rtl/uuid.h>
#include <vcl/pdfextoutdevdata.hxx>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svtools/embedhlp.hxx>

using namespace ::com::sun::star;

ChartPrettyPainter::ChartPrettyPainter()
{
}

ChartPrettyPainter::~ChartPrettyPainter()
{
}

bool ChartPrettyPainter::DoPaint(OutputDevice* /*pOutDev*/, const Rectangle& /*rLogicObjectRect*/) const
{
    return false;
}

//static
const uno::Sequence<sal_Int8>& ChartPrettyPainter::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

bool ChartPrettyPainter::IsChart( const svt::EmbeddedObjectRef& xObjRef )
{
    if ( !xObjRef.is() )
        return false;

    SvGlobalName aObjClsId( xObjRef->getClassID() );
    if(
        SvGlobalName(SO3_SCH_CLASSID_30) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_40) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_50) == aObjClsId
        || SvGlobalName(SO3_SCH_CLASSID_60) == aObjClsId)
    {
        return true;
    }

    return false;
}

bool ChartPrettyPainter::ShouldPrettyPaintChartOnThisDevice( OutputDevice* pOutDev )
{
    if( !pOutDev )
        return false;
    //at least the print preview in calc has a paint loop due to too much invalidate calls deep in sdr
    //to avoid the paint loop we use the metafile replacement in this case instead of direct rendering
    if( OUTDEV_WINDOW == pOutDev->GetOutDevType() )
        return false;
    if( OUTDEV_PRINTER == pOutDev->GetOutDevType() )
        return true;
    vcl::PDFExtOutDevData* pPDFData = PTR_CAST( vcl::PDFExtOutDevData, pOutDev->GetExtOutDevData() );
    if( pPDFData )
        return true;
    return false;
}

bool ChartPrettyPainter::DoPrettyPaintChart( uno::Reference< frame::XModel > xChartModel, OutputDevice* pOutDev, const Rectangle& rLogicObjectRect )
{
    //charts must be painted resolution dependent!! #i82893#, #i75867#
    if( !xChartModel.is() || !ShouldPrettyPaintChartOnThisDevice( pOutDev ) )
        return false;

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( xChartModel, uno::UNO_QUERY );
        OSL_ENSURE( xFact.is(), "Chart cannot be painted pretty!\n" );
        if( xFact.is() )
        {
            uno::Reference< lang::XUnoTunnel > xChartRenderer( xFact->createInstance(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.ChartRenderer" ) ) ), uno::UNO_QUERY );
            OSL_ENSURE( xChartRenderer.is(), "Chart cannot be painted pretty!\n" );
            if( xChartRenderer.is() )
            {
                ChartPrettyPainter* pPrettyPainter = reinterpret_cast<ChartPrettyPainter*>(
                    xChartRenderer->getSomething( ChartPrettyPainter::getUnoTunnelId() ));
                if( pPrettyPainter )
                    return pPrettyPainter->DoPaint(pOutDev, rLogicObjectRect);
            }
        }
    }
    catch( uno::Exception& e )
    {
        (void)e;
        DBG_ERROR( "Chart cannot be painted pretty!" );
    }
    return false;
}

