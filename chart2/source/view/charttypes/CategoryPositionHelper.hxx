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

#ifndef INCLUDED_CHART2_SOURCE_VIEW_CHARTTYPES_CATEGORYPOSITIONHELPER_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_CHARTTYPES_CATEGORYPOSITIONHELPER_HXX

namespace chart
{

class CategoryPositionHelper
{
public:
    CategoryPositionHelper( double fSeriesCount, double CategoryWidth = 1.0);
    virtual ~CategoryPositionHelper();

    CategoryPositionHelper(CategoryPositionHelper const &) = default;
    CategoryPositionHelper(CategoryPositionHelper &&) = default;
    CategoryPositionHelper & operator =(CategoryPositionHelper const &) = default;
    CategoryPositionHelper & operator =(CategoryPositionHelper &&) = default;

    double getScaledSlotWidth() const;
    virtual double getScaledSlotPos( double fCategoryX, double fSeriesNumber ) const;
    void setCategoryWidth( double fCategoryWidth );

    //Distance between two neighboring bars in same category, seen relative to width of the bar
    void setInnerDistance( double fInnerDistance );

    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    void setOuterDistance( double fOuterDistance );

protected:
    double m_fSeriesCount;
    double m_fCategoryWidth;
    //Distance between two neighboring bars in same category, seen relative to width of the bar:
    double m_fInnerDistance; //[-1,1] m_fInnerDistance=1 --> distance == width; m_fInnerDistance=-1-->all rects are painted on the same position
    //Distance between two neighboring bars in different category, seen relative to width of the bar:
    double m_fOuterDistance; //>=0 m_fOuterDistance=1 --> distance == width
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
