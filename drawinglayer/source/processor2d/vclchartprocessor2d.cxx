/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclchartprocessor2d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-13 08:22:03 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#include <drawinglayer/primitive2d/chartprimitive2d.hxx>
#include <svtools/chartprettypainter.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        void VclProcessor2D::RenderChartPrimitive2D(const primitive2d::ChartPrimitive2D& rChartCandidate)
        {
            bool bChartRendered(false);

            // code from chart PrettyPrinter
            try
            {
                uno::Reference< lang::XMultiServiceFactory > xFact( rChartCandidate.getChartModel(), uno::UNO_QUERY );
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
                        {
                            // create logic object range
                            basegfx::B2DRange aObjectRange(0.0, 0.0, 1.0, 1.0);
                            aObjectRange.transform(rChartCandidate.getTransformation());
                            const Rectangle aRectangle(
                                    (sal_Int32)aObjectRange.getMinX(), (sal_Int32)aObjectRange.getMinY(),
                                    (sal_Int32)aObjectRange.getMaxX(), (sal_Int32)aObjectRange.getMaxY());

                            bChartRendered = pPrettyPainter->DoPaint(mpOutputDevice, aRectangle);
                        }
                    }
                }
            }
            catch( uno::Exception& e )
            {
                (void)e;
                DBG_ERROR( "Chart cannot be painted pretty!" );
            }

            if(!bChartRendered)
            {
                // fallback to decompositopn (MetaFile)
                process(rChartCandidate.get2DDecomposition(getViewInformation2D()));
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
