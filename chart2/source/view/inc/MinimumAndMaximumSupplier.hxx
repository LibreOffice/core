/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MinimumAndMaximumSupplier.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:19:21 $
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

#ifndef _CHART2_MINIMUMANDMAXIMUMSUPPLIER_HXX
#define _CHART2_MINIMUMANDMAXIMUMSUPPLIER_HXX

#include <sal/types.h>

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
