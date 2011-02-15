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

#ifndef _CHART2_MINIMUMANDMAXIMUMSUPPLIER_HXX
#define _CHART2_MINIMUMANDMAXIMUMSUPPLIER_HXX

#include <sal/types.h>
#include <tools/date.hxx>
#include <set>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
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
    virtual bool isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex ) = 0;

    //return a constant out of ::com::sun::star::chart::TimeUnit that allows to display the smallest distance between occuring dates
    virtual long calculateTimeResolutionOnXAxis() = 0;
    virtual void setTimeResolutionOnXAxis( long nTimeResolution, const Date& rNullDate ) = 0;
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
    virtual bool isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex );

    virtual long calculateTimeResolutionOnXAxis();
    virtual void setTimeResolutionOnXAxis( long nTimeResolution, const Date& rNullDate );

private:
    typedef ::std::set< MinimumAndMaximumSupplier* > MinimumAndMaximumSupplierSet;
    MinimumAndMaximumSupplierSet m_aMinimumAndMaximumSupplierList;

    inline MinimumAndMaximumSupplierSet::iterator begin() { return m_aMinimumAndMaximumSupplierList.begin(); }
    inline MinimumAndMaximumSupplierSet::iterator end() { return m_aMinimumAndMaximumSupplierList.end(); }
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
