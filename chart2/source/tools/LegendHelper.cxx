/*************************************************************************
 *
 *  $RCSfile: LegendHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-08 17:40:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "LegendHelper.hxx"
#include "macros.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

// static
rtl::OUString LegendHelper::getIdentifierForLegend()
{
    static rtl::OUString m_aIdentifier( C2U( "@legend" ) );
    return m_aIdentifier;
}

// static
uno::Reference< chart2::XLegend > LegendHelper::getLegend(
    const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< chart2::XLegend > xResult;

    uno::Reference< chart2::XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        uno::Reference< chart2::XDiagram > xDia( xChartDoc->getDiagram());
        if( xDia.is())
            xResult.set( xDia->getLegend() );
    }

    return xResult;
}

//.............................................................................
} //namespace chart
//.............................................................................

