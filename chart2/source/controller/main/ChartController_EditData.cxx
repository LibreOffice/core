/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartController_EditData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:32:01 $
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
#include "ChartController.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XRANGESELECTION_HPP_
#include <com/sun/star/sheet/XRangeSelection.hpp>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

void ChartController::executeDispatch_EditData()
{
    Reference< chart2::XChartDocument > xChartDoc( m_aModel->getModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        Reference< embed::XEmbeddedObject > xEmbObj( xChartDoc->getDataEditorForInternalData() );
        if( xEmbObj.is())
        {
            try
            {
                xEmbObj->setContainerName( C2U( "Chart Data" ));
                xEmbObj->changeState( embed::EmbedStates::ACTIVE );

                Reference< embed::XVisualObject > xVisObj( xEmbObj, uno::UNO_QUERY );
                if( xVisObj.is())
                {
                    xVisObj->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT,
                                                awt::Size( 5000, 3000 ));
                }

                // just a test
//                 Reference< embed::XComponentSupplier > xCompSupp( xEmbObj, uno::UNO_QUERY );
//                 Reference< frame::XModel > xCalcModel( xCompSupp->getComponent(), uno::UNO_QUERY_THROW );
//                 Reference< sheet::XRangeSelection > xRangeSel(
//                     xCalcModel->getCurrentController(), uno::UNO_QUERY_THROW );

//                 Sequence< beans::PropertyValue > aArgs;
//                 xRangeSel->startRangeSelection( aArgs );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

} //  namespace chart
