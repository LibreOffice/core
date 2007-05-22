/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MinimumAndMaximumSupplier.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:08:41 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "MinimumAndMaximumSupplier.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

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

//.............................................................................
} //namespace chart
//.............................................................................
