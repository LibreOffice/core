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
#include "precompiled_chart2.hxx"

#include "AccStatisticsObject.hxx"
#include <vcl/svapp.hxx>
#include <svx/svditer.hxx>

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
    SolarMutexGuard aSolarGuard;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
