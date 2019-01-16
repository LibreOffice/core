/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "BarPositionHelper.hxx"
#include <DateHelper.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

BarPositionHelper::BarPositionHelper()
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

std::unique_ptr<PlottingPositionHelper> BarPositionHelper::clone() const
{
    return std::make_unique<BarPositionHelper>(*this);
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
    doLogicScaling(&fScaledLogicX,nullptr,nullptr);
    fScaledLogicX = CategoryPositionHelper::getScaledSlotPos( fScaledLogicX, fSeriesNumber );
    return fScaledLogicX;

}

void BarPositionHelper::setScaledCategoryWidth( double fScaledCategoryWidth )
{
    m_fScaledCategoryWidth = fScaledCategoryWidth;
    CategoryPositionHelper::setCategoryWidth( m_fScaledCategoryWidth );
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
