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

#include <MinimumAndMaximumSupplier.hxx>

#include <com/sun/star/chart/TimeUnit.hpp>

#include <rtl/math.hxx>

namespace chart
{
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
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        double fLocalExtremum = elem->getMinimumX();
        if(fLocalExtremum<fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(std::isinf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMaximumX()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, true);
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        double fLocalExtremum = elem->getMaximumX();
        if(fLocalExtremum>fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(std::isinf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, false);
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        double fLocalExtremum = elem->getMinimumYInRange( fMinimumX, fMaximumX, nAxisIndex );
        if(fLocalExtremum<fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(std::isinf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, true);
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        double fLocalExtremum = elem->getMaximumYInRange( fMinimumX, fMaximumX, nAxisIndex );
        if(fLocalExtremum>fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(std::isinf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMinimumZ()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, false);
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        double fLocalExtremum = elem->getMinimumZ();
        if(fLocalExtremum<fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(std::isinf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

double MergedMinimumAndMaximumSupplier::getMaximumZ()
{
    double fGlobalExtremum;
    ::rtl::math::setInf(&fGlobalExtremum, true);
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        double fLocalExtremum = elem->getMaximumZ();
        if(fLocalExtremum>fGlobalExtremum)
            fGlobalExtremum=fLocalExtremum;
    }
    if(std::isinf(fGlobalExtremum))
        ::rtl::math::setNan(&fGlobalExtremum);
    return fGlobalExtremum;
}

bool MergedMinimumAndMaximumSupplier::isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex )
{
    // only return true, if *all* suppliers want to scale to the main tick marks
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
        if( !elem->isExpandBorderToIncrementRhythm( nDimensionIndex ) )
            return false;
    return true;
}

bool MergedMinimumAndMaximumSupplier::isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex )
{
    // only return true, if *all* suppliers want to expand the range
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
        if( !elem->isExpandIfValuesCloseToBorder( nDimensionIndex ) )
            return false;
    return true;
}

bool MergedMinimumAndMaximumSupplier::isExpandWideValuesToZero( sal_Int32 nDimensionIndex )
{
    // already return true, if at least one supplier wants to expand the range
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
        if( elem->isExpandWideValuesToZero( nDimensionIndex ) )
            return true;
    return false;
}

bool MergedMinimumAndMaximumSupplier::isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex )
{
    // already return true, if at least one supplier wants to expand the range
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
        if( elem->isExpandNarrowValuesTowardZero( nDimensionIndex ) )
            return true;
    return false;
}

bool MergedMinimumAndMaximumSupplier::isSeparateStackingForDifferentSigns( sal_Int32 nDimensionIndex )
{
    // should not be called
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
        if( elem->isSeparateStackingForDifferentSigns( nDimensionIndex ) )
            return true;
    return false;
}

void MergedMinimumAndMaximumSupplier::clearMinimumAndMaximumSupplierList()
{
    m_aMinimumAndMaximumSupplierList.clear();
}

tools::Long MergedMinimumAndMaximumSupplier::calculateTimeResolutionOnXAxis()
{
    tools::Long nRet = css::chart::TimeUnit::YEAR;
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
    {
        tools::Long nCurrent = elem->calculateTimeResolutionOnXAxis();
        if(nRet>nCurrent)
            nRet=nCurrent;
    }
    return nRet;
}

void MergedMinimumAndMaximumSupplier::setTimeResolutionOnXAxis( tools::Long nTimeResolution, const Date& rNullDate )
{
    for (auto const& elem : m_aMinimumAndMaximumSupplierList)
        elem->setTimeResolutionOnXAxis( nTimeResolution, rNullDate );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
