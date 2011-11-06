/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "MinimumAndMaximumSupplier.hxx"

#include <com/sun/star/chart/TimeUnit.hpp>

#include <rtl/math.hxx>
#include <com/sun/star/awt/Size.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

MergedMinimumAndMaximumSupplier::MergedMinimumAndMaximumSupplier()
{
}

MergedMinimumAndMaximumSupplier::~MergedMinimumAndMaximumSupplier()
{
}

void MergedMinimumAndMaximumSupplier::addMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    m_aMinimumAndMaximumSupplierList.insert( pMinimumAndMaximumSupplier );
}

bool MergedMinimumAndMaximumSupplier::hasMinimumAndMaximumSupplier( MinimumAndMaximumSupplier* pMinimumAndMaximumSupplier )
{
    return m_aMinimumAndMaximumSupplierList.count( pMinimumAndMaximumSupplier ) != 0;
}

double MergedMinimumAndMaximumSupplier::getMinimumX()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, false);
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        double fLocalExtremum = (*aIt)->getMinimumX();
        if(fLocalExtremum<fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(::rtl::math::isInf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMaximumX()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, true);
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        double fLocalExtremum = (*aIt)->getMaximumX();
        if(fLocalExtremum>fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(::rtl::math::isInf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, false);
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        double fLocalExtremum = (*aIt)->getMinimumYInRange( fMinimumX, fMaximumX, nAxisIndex );
        if(fLocalExtremum<fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(::rtl::math::isInf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, true);
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        double fLocalExtremum = (*aIt)->getMaximumYInRange( fMinimumX, fMaximumX, nAxisIndex );
        if(fLocalExtremum>fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(::rtl::math::isInf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMinimumZ()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, false);
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        double fLocalExtremum = (*aIt)->getMinimumZ();
        if(fLocalExtremum<fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(::rtl::math::isInf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMaximumZ()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, true);
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        double fLocalExtremum = (*aIt)->getMaximumZ();
        if(fLocalExtremum>fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(::rtl::math::isInf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

bool MergedMinimumAndMaximumSupplier::isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex )
{
    // only return true, if *all* suppliers want to scale to the main tick marks
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( !(*aIt)->isExpandBorderToIncrementRhythm( nDimensionIndex ) )
            return false;
    return true;
}

bool MergedMinimumAndMaximumSupplier::isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex )
{
    // only return true, if *all* suppliers want to expand the range
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( !(*aIt)->isExpandIfValuesCloseToBorder( nDimensionIndex ) )
            return false;
    return true;
}

bool MergedMinimumAndMaximumSupplier::isExpandWideValuesToZero( sal_Int32 nDimensionIndex )
{
    // already return true, if at least one supplier wants to expand the range
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( (*aIt)->isExpandWideValuesToZero( nDimensionIndex ) )
            return true;
    return false;
}

bool MergedMinimumAndMaximumSupplier::isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex )
{
    // already return true, if at least one supplier wants to expand the range
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( (*aIt)->isExpandNarrowValuesTowardZero( nDimensionIndex ) )
            return true;
    return false;
}

bool MergedMinimumAndMaximumSupplier::isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex )
{
    // should not be called
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        if( (*aIt)->isSeperateStackingForDifferentSigns( nDimensionIndex ) )
            return true;
    return false;
}

void MergedMinimumAndMaximumSupplier::clearMinimumAndMaximumSupplierList()
{
    m_aMinimumAndMaximumSupplierList.clear();
}

long MergedMinimumAndMaximumSupplier::calculateTimeResolutionOnXAxis()
{
    long nRet = ::com::sun::star::chart::TimeUnit::YEAR;
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
    {
        long nCurrent = (*aIt)->calculateTimeResolutionOnXAxis();
        if(nRet>nCurrent)
            nRet=nCurrent;
    }
    return nRet;
}

void MergedMinimumAndMaximumSupplier::setTimeResolutionOnXAxis( long nTimeResolution, const Date& rNullDate )
{
    for( MinimumAndMaximumSupplierSet::iterator aIt = begin(), aEnd = end(); aIt != aEnd; ++aIt )
        (*aIt)->setTimeResolutionOnXAxis( nTimeResolution, rNullDate );
}

//.............................................................................
} //namespace chart
//.............................................................................
