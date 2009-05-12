/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartViewHelper.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_chart2.hxx"

#include "ChartViewHelper.hxx"
#include "macros.hxx"
#include "servicenames.hxx"

// header for define DBG_ASSERT
#include <tools/debug.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

//static
void ChartViewHelper::setViewToDirtyState( const uno::Reference< frame::XModel >& xChartModel )
{
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFact( xChartModel, uno::UNO_QUERY );
        if( xFact.is() )
        {
            Reference< util::XModifyListener > xModifyListener(
                    xFact->createInstance( CHART_VIEW_SERVICE_NAME ), uno::UNO_QUERY );
            if( xModifyListener.is() )
            {
                lang::EventObject aEvent( Reference< lang::XComponent >( xChartModel, uno::UNO_QUERY ) );
                xModifyListener->modified( aEvent );
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}
//.............................................................................
} //namespace chart
//.............................................................................
