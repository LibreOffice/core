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

#include <chartview/ExplicitScaleValues.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

#include <vector>

namespace chart { struct AxisProperties; }
namespace chart { struct TickmarkProperties; }
namespace com::sun::star::chart2 { class XScaling; }
namespace com::sun::star::drawing { class XShape; }

namespace chart {

struct TickInfo
{
    double      fScaledTickValue;
    css::uno::Reference<css::chart2::XScaling> xInverseScaling;

    ::basegfx::B2DVector  aTickScreenPosition;
    bool        bPaintIt;

    css::uno::Reference<css::drawing::XShape> xTextShape;

    OUString aText;//used only for complex categories so far
    sal_Int32 nFactorForLimitedTextWidth;//categories in higher levels of complex categories can have more place than a single simple category

//methods:
    TickInfo() = delete;
    explicit TickInfo( const css::uno::Reference<css::chart2::XScaling>& xInverse );

    /**
     * Return a value associated with the tick mark. It's normally an original
     * value from the data source, or 1-based integer index in case the axis
     * is a category axis.
     */
    double getUnscaledTickValue() const;
    sal_Int32 getScreenDistanceBetweenTicks( const TickInfo& rOherTickInfo ) const;
};

typedef std::vector<TickInfo>           TickInfoArrayType;
typedef std::vector<TickInfoArrayType>  TickInfoArraysType;

class TickIter
{
public:
    virtual ~TickIter() {}
    virtual TickInfo* firstInfo() = 0;
    virtual TickInfo* nextInfo() = 0;
};

class PureTickIter : public TickIter
{
public:
    explicit PureTickIter( TickInfoArrayType& rTickInfoVector );
    virtual ~PureTickIter() override;
    virtual TickInfo* firstInfo() override;
    virtual TickInfo* nextInfo() override;

private:
    TickInfoArrayType& m_rTickVector;
    TickInfoArrayType::iterator m_aTickIter;
};

class TickFactory
{
public:
    TickFactory(
         const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement );
    virtual ~TickFactory();

    void getAllTicks( TickInfoArraysType& rAllTickInfos ) const;
    void getAllTicksShifted( TickInfoArraysType& rAllTickInfos ) const;

private: //methods
    bool        isDateAxis() const;

protected: //member
    ExplicitScaleData     m_rScale;
    ExplicitIncrementData m_rIncrement;
    css::uno::Reference< css::chart2::XScaling > m_xInverseScaling;

    //minimum and maximum of the visible range after scaling
    double    m_fScaledVisibleMin;
    double    m_fScaledVisibleMax;
};

class TickFactory2D final : public TickFactory
{
public:
    TickFactory2D(
        const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement
        , const ::basegfx::B2DVector& rStartScreenPos, const ::basegfx::B2DVector& rEndScreenPos
        , const ::basegfx::B2DVector& rAxisLineToLabelLineShift );

    virtual ~TickFactory2D() override;

    static sal_Int32    getTickScreenDistance( TickIter& rIter );

    void createPointSequenceForAxisMainLine( css::drawing::PointSequenceSequence& rPoints ) const;
    void addPointSequenceForTickLine( css::drawing::PointSequenceSequence& rPoints
                            , sal_Int32 nSequenceIndex
                            , double fScaledLogicTickValue, double fInnerDirectionSign
                            , const TickmarkProperties& rTickmarkProperties, bool bPlaceAtLabels ) const;
    ::basegfx::B2DVector  getDistanceAxisTickToText( const AxisProperties& rAxisProperties
        , bool bIncludeFarAwayDistanceIfSo = false
        , bool bIncludeSpaceBetweenTickAndText = true ) const;

    /**
     * Determine the screen positions of all ticks based on their numeric values.
     */
    void updateScreenValues( TickInfoArraysType& rAllTickInfos ) const;

    bool  isHorizontalAxis() const;
    bool  isVerticalAxis() const;
    const ::basegfx::B2DVector & getXaxisStartPos() const;

private:
    ::basegfx::B2DVector     getTickScreenPosition2D( double fScaledLogicTickValue ) const;

    ::basegfx::B2DVector    m_aAxisStartScreenPosition2D;
    ::basegfx::B2DVector    m_aAxisEndScreenPosition2D;

    //labels might be positioned high or low on the border of the diagram far away from the axis
    //add this vector to go from the axis line to the label line (border of the diagram)
    ::basegfx::B2DVector    m_aAxisLineToLabelLineShift;

    double      m_fStretch_LogicToScreen;
    double      m_fOffset_LogicToScreen;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
