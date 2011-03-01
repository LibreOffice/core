/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
        OSL_FAIL( "Chart cannot be painted pretty!" );
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
