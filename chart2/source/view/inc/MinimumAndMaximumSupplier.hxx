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

#ifndef _CHART2_MINIMUMANDMAXIMUMSUPPLIER_HXX
#define _CHART2_MINIMUMANDMAXIMUMSUPPLIER_HXX

#include <sal/types.h>
#include <tools/date.hxx>
#include <set>

namespace chart
{

/**
*/

class MinimumAndMaximumSupplier
{
public:
    virtual double getMinimumX() = 0;
    virtual double getMaximumX() = 0;

    //problem y maybe not is always the second border to ask for
    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex ) = 0;
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex ) = 0;

    //problem: z maybe not independent in future
    virtual double getMinimumZ() = 0;
    virtual double getMaximumZ() = 0;

    virtual bool isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex ) = 0;
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex ) = 0;
    virtual bool isExpandWideValuesToZero( sal_Int32 nDimensionIndex ) = 0;
    virtual bool isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex ) = 0;
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex ) = 0;

    //return a constant out of ::com::sun::star::chart::TimeUnit that allows to display the smallest distance between occuring dates
    virtual long calculateTimeResolutionOnXAxis() = 0;
    virtual void setTimeResolutionOnXAxis( long nTimeResolution, const Date& rNullDate ) = 0;

protected:
    ~MinimumAndMaximumSupplier() {}
};

class MergedMinimumAndMaximumSupplier : public MinimumAndMaximumSupplier
{
public:
    MergedMinimumAndMaximumSupplier();
    virtual ~MergedMinimumAndMaximumSupplier();

    void addMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    bool hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier );
    void clearMinimumAndMaximumSupplierList();

    //--MinimumAndMaximumSupplier
    virtual double getMinimumX();
    virtual double getMaximumX();
    virtual double getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex );
    virtual double getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex );
    virtual double getMinimumZ();
    virtual double getMaximumZ();

    virtual bool isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex );
    virtual bool isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex );
    virtual bool isExpandWideValuesToZero( sal_Int32 nDimensionIndex );
    virtual bool isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex );
    virtual bool isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

    virtual long calculateTimeResolutionOnXAxis();
    virtual void setTimeResolutionOnXAxis( long nTimeResolution, const Date& rNullDate );

private:
    typedef ::std::set< MinimumAndMaximumSupplier* > MinimumAndMaximumSupplierSet;
    MinimumAndMaximumSupplierSet m_aMinimumAndMaximumSupplierList;

    inline MinimumAndMaximumSupplierSet::iterator begin() { return m_aMinimumAndMaximumSupplierList.begin(); }
    inline MinimumAndMaximumSupplierSet::iterator end() { return m_aMinimumAndMaximumSupplierList.end(); }
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
