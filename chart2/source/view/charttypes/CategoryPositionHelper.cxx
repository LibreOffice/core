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

#include "CategoryPositionHelper.hxx"

namespace chart
{

CategoryPositionHelper::CategoryPositionHelper( double fSeriesCount, double fCategoryWidth )
    : m_fSeriesCount(fSeriesCount)
    , m_fCategoryWidth(fCategoryWidth)
    , m_fInnerDistance(0.0)
    , m_fOuterDistance(1.0)
{
}

CategoryPositionHelper::CategoryPositionHelper( const CategoryPositionHelper& rSource )
    : m_fSeriesCount( rSource.m_fSeriesCount )
    , m_fCategoryWidth( rSource.m_fCategoryWidth )
    , m_fInnerDistance( rSource.m_fInnerDistance )
    , m_fOuterDistance( rSource.m_fOuterDistance )
{
}

CategoryPositionHelper::~CategoryPositionHelper()
{
}

double CategoryPositionHelper::getScaledSlotWidth() const
{
    double fWidth = m_fCategoryWidth /
                (  m_fSeriesCount
                 + m_fOuterDistance
                 + m_fInnerDistance*( m_fSeriesCount - 1.0) );
    return fWidth;
}

double CategoryPositionHelper::getScaledSlotPos( double fScaledXPos, double fSeriesNumber ) const
{
    //the returned position is in the middle of the rect
    //fSeriesNumber 0...n-1
    double fPos = fScaledXPos
           - (m_fCategoryWidth/2.0)
           + (m_fOuterDistance/2.0 + fSeriesNumber*(1.0+m_fInnerDistance)) * getScaledSlotWidth()
           + getScaledSlotWidth()/2.0;
    return fPos;
}

void CategoryPositionHelper::setInnerDistance( double fInnerDistance )
{
    if( fInnerDistance < -1.0 )
        fInnerDistance = -1.0;
    if( fInnerDistance > 1.0 )
        fInnerDistance = 1.0;
    m_fInnerDistance = fInnerDistance;
}

void CategoryPositionHelper::setOuterDistance( double fOuterDistance )
{
    if( fOuterDistance < 0.0 )
        fOuterDistance = 0.0;
    if( fOuterDistance > 6.0 )
        fOuterDistance = 6.0;
    m_fOuterDistance = fOuterDistance;
}

void CategoryPositionHelper::setCategoryWidth( double fCategoryWidth )
{
    m_fCategoryWidth = fCategoryWidth;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
