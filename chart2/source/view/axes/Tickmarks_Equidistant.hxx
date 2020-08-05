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
#pragma once

#include "Tickmarks.hxx"
#include <memory>

#include <o3tl/safeint.hxx>

namespace chart
{

class EquidistantTickIter : public TickIter
{
public:
    EquidistantTickIter( const css::uno::Sequence< css::uno::Sequence< double > >& rTicks
                , const ExplicitIncrementData& rIncrement
                , sal_Int32 nMaxDepth );
    EquidistantTickIter( TickInfoArraysType& rTickInfos
            , const ExplicitIncrementData& rIncrement
            , sal_Int32 nMaxDepth );
    virtual ~EquidistantTickIter() override;

    double*     firstValue();
    double*     nextValue();

    virtual TickInfo*   firstInfo() override;
    virtual TickInfo*   nextInfo() override;

private: //methods
    sal_Int32   getIntervalCount( sal_Int32 nDepth );
    bool        isAtLastPartTick();

    void        initIter( sal_Int32 nMaxDepth );
    sal_Int32   getStartDepth() const;

    bool        gotoFirst();
    bool        gotoNext();

    double      getTickValue(sal_Int32 nDepth, sal_Int32 nIndex) const
                {
                    if(m_pSimpleTicks)
                        return (*m_pSimpleTicks)[nDepth][nIndex];
                    else
                    {
                        if ((*m_pInfoTicks)[nDepth].size() <= o3tl::make_unsigned(nIndex))
                            return std::numeric_limits<double>::max();
                        return (((*m_pInfoTicks)[nDepth])[nIndex]).fScaledTickValue;
                    }
                }
    sal_Int32   getTickCount( sal_Int32 nDepth ) const
                {
                    if(m_pSimpleTicks)
                        return (*m_pSimpleTicks)[nDepth].getLength();
                    else
                        return (*m_pInfoTicks)[nDepth].size();
                }
    sal_Int32   getMaxDepth() const
                {
                    if(m_pSimpleTicks)
                        return (*m_pSimpleTicks).getLength()-1;
                    else
                        return (*m_pInfoTicks).size()-1;
                }

private: //member
    const css::uno::Sequence< css::uno::Sequence< double > >*  m_pSimpleTicks;
    TickInfoArraysType*       m_pInfoTicks;
    const ExplicitIncrementData& m_rIncrement;
    sal_Int32   m_nMaxDepth;
    sal_Int32   m_nTickCount;
    std::unique_ptr<sal_Int32[]>
                m_pnPositions; //current positions in the different sequences
    std::unique_ptr<sal_Int32[]>
                m_pnPreParentCount; //the tickmarks do not start with a major tick always,
                                    //the PreParentCount states for each depth how many subtickmarks are available in front of the first parent tickmark
    std::unique_ptr<bool[]>
                m_pbIntervalFinished;
    sal_Int32   m_nCurrentDepth;
    sal_Int32   m_nCurrentPos;
    double      m_fCurrentValue;
};

class EquidistantTickFactory
{
public:
    EquidistantTickFactory(
         const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement );
    ~EquidistantTickFactory();

    void getAllTicks( TickInfoArraysType& rAllTickInfos ) const;
    void getAllTicksShifted( TickInfoArraysType& rAllTickInfos ) const;

    static double getMinimumAtIncrement( double fMin, const ExplicitIncrementData& rIncrement );
    static double getMaximumAtIncrement( double fMax, const ExplicitIncrementData& rIncrement );

private: //methods
    void        addSubTicks( sal_Int32 nDepth,
                        css::uno::Sequence< css::uno::Sequence< double > >& rParentTicks ) const;
    double*     getMajorTick( sal_Int32 nTick ) const;
    double*     getMinorTick( sal_Int32 nTick, sal_Int32 nDepth
                    , double fStartParentTick, double fNextParentTick ) const;
    sal_Int32   getMaxTickCount( sal_Int32 nDepth ) const;
    sal_Int32   getTickDepth() const;

    bool        isVisible( double fValue ) const;
    bool        isWithinOuterBorder( double fScaledValue ) const; //all within the outer major tick marks

private: //member
    ExplicitScaleData     m_rScale;
    ExplicitIncrementData m_rIncrement;
    css::uno::Reference< css::chart2::XScaling > m_xInverseScaling;

    //minimum and maximum of the visible range after scaling
    double    m_fScaledVisibleMin;
    double    m_fScaledVisibleMax;

    std::unique_ptr<double[]>
              m_pfCurrentValues;
    //major-tick positions that may lay outside the visible range but complete partly visible intervals at the borders
    double    m_fOuterMajorTickBorderMin;
    double    m_fOuterMajorTickBorderMax;
    double    m_fOuterMajorTickBorderMin_Scaled;
    double    m_fOuterMajorTickBorderMax_Scaled;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
