/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccStatisticsObject.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:14:06 $
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
#include "precompiled_chart2.hxx"

#include "AccStatisticsObject.hxx"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

using ::rtl::OUString;

namespace chart
{

AccStatisticsObject::AccStatisticsObject( const AccessibleElementInfo& rAccInfo )
        : AccessibleChartElement( rAccInfo, false/*NoChildren*/, true/*AlwaysTransparent*/ )
        //, m_eType( eType )
        //, m_nSeriesIndex( nSeriesIndex )
{
//    ChartModel * pModel = GetChartModel();
//    OSL_ASSERT( pModel );

    // /-- solar
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
/*    switch( eType )
    {
        case MEAN_VAL_LINE:
            SetItemSet( pModel->GetAverageAttr( m_nSeriesIndex ));
            break;
        case ERROR_BARS:
            SetItemSet( pModel->GetErrorAttr( m_nSeriesIndex ));
            break;
        case REGRESSION:
            SetItemSet( pModel->GetRegressAttr( m_nSeriesIndex ));
            break;
    }*/
    // \-- solar
}

AccStatisticsObject::~AccStatisticsObject()
{
}

OUString SAL_CALL AccStatisticsObject::getAccessibleName()
    throw (::com::sun::star::uno::RuntimeException)
{
    return getToolTipText();
}

OUString SAL_CALL AccStatisticsObject::getImplementationName()
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "StatisticsObject" ));
}

}  // namespace chart
