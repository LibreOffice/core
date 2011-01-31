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

#include "BarPositionHelper.hxx"
#include "Linear3DTransformation.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"
#include "DateHelper.hxx"
#include <com/sun/star/chart/TimeUnit.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

BarPositionHelper::BarPositionHelper( bool /* bSwapXAndY */ )
        : CategoryPositionHelper( 1 )
{
    AllowShiftXAxisPos(true);
    AllowShiftZAxisPos(true);
}

BarPositionHelper::BarPositionHelper( const BarPositionHelper& rSource )
        : CategoryPositionHelper( rSource )
        , PlottingPositionHelper( rSource )
{
}

BarPositionHelper::~BarPositionHelper()
{
}

PlottingPositionHelper* BarPositionHelper::clone() const
{
    BarPositionHelper* pRet = new BarPositionHelper(*this);
    return pRet;
}

void BarPositionHelper::updateSeriesCount( double fSeriesCount )
{
    m_fSeriesCount = fSeriesCount;
}

double BarPositionHelper::getScaledSlotPos( double fUnscaledLogicX, double fSeriesNumber ) const
{
    if( m_bDateAxis )
        fUnscaledLogicX = DateHelper::RasterizeDateValue( fUnscaledLogicX, m_aNullDate, m_nTimeResolution );
    double fScaledLogicX(fUnscaledLogicX);
    doLogicScaling(&fScaledLogicX,NULL,NULL);
    fScaledLogicX = CategoryPositionHelper::getScaledSlotPos( fScaledLogicX, fSeriesNumber );
    return fScaledLogicX;

}

void BarPositionHelper::setScaledCategoryWidth( double fScaledCategoryWidth )
{
    m_fScaledCategoryWidth = fScaledCategoryWidth;
    CategoryPositionHelper::setCategoryWidth( m_fScaledCategoryWidth );
}
//.............................................................................
} //namespace chart
//.............................................................................
