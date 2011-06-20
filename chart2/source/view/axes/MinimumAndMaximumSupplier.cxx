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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
