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
#ifndef _CHART2_TICKMARKS_HXX
#define _CHART2_TICKMARKS_HXX

#include "TickmarkProperties.hxx"
#include "VAxisProperties.hxx"
#include "chartview/ExplicitScaleValues.hxx"
#include <basegfx/vector/b2dvector.hxx>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <vector>

namespace chart
{

struct TickInfo
{
    double      fScaledTickValue;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XScaling > xInverseScaling;

    ::basegfx::B2DVector  aTickScreenPosition;
    bool        bPaintIt;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > xTextShape;

    OUString aText;//used only for complex categories so far
    sal_Int32 nFactorForLimitedTextWidth;//categories in higher levels of complex categories can have more place than a single simple category

//methods:
    TickInfo( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XScaling >& xInverseScaling );

    double getUnscaledTickValue() const;
    sal_Int32 getScreenDistanceBetweenTicks( const TickInfo& rOherTickInfo ) const;
private:
    TickInfo();
};
class TickIter
{
public:
    virtual ~TickIter(){};
    virtual TickInfo* firstInfo()=0;
    virtual TickInfo* nextInfo()=0;
};

class PureTickIter : public TickIter
{
public:
    PureTickIter( ::std::vector< TickInfo >& rTickInfoVector );
    virtual ~PureTickIter();
    virtual TickInfo* firstInfo();
    virtual TickInfo* nextInfo();

private:
    ::std::vector< TickInfo >& m_rTickVector;
    ::std::vector< TickInfo >::iterator m_aTickIter;
};

class TickFactory
{
public:
    TickFactory(
         const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement );
    virtual ~TickFactory();

    void getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    void getAllTicksShifted( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    virtual void updateScreenValues( ::std::vector< ::std::vector< TickInfo > >& /*rAllTickInfos*/ ) const {}

private: //methods
    bool        isDateAxis() const;

protected: //member
    ExplicitScaleData     m_rScale;
    ExplicitIncrementData m_rIncrement;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >
                                                m_xInverseScaling;

    //minimum and maximum of the visible range after scaling
    double    m_fScaledVisibleMin;
    double    m_fScaledVisibleMax;
};

class TickFactory_2D : public TickFactory
{
public:
    TickFactory_2D(
        const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement
        , const ::basegfx::B2DVector& rStartScreenPos, const ::basegfx::B2DVector& rEndScreenPos
        , const ::basegfx::B2DVector& rAxisLineToLabelLineShift );
        //, double fStrech_SceneToScreen, double fOffset_SceneToScreen );
    virtual ~TickFactory_2D();

    static sal_Int32    getTickScreenDistance( TickIter& rIter );

    void createPointSequenceForAxisMainLine( ::com::sun::star::drawing::PointSequenceSequence& rPoints ) const;
    void addPointSequenceForTickLine( ::com::sun::star::drawing::PointSequenceSequence& rPoints
                            , sal_Int32 nSequenceIndex
                            , double fScaledLogicTickValue, double fInnerDirectionSign
                            , const TickmarkProperties& rTickmarkProperties, bool bPlaceAtLabels ) const;
    ::basegfx::B2DVector  getDistanceAxisTickToText( const AxisProperties& rAxisProperties
        , bool bIncludeFarAwayDistanceIfSo = false
        , bool bIncludeSpaceBetweenTickAndText = true ) const;

    virtual void        updateScreenValues( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    bool  isHorizontalAxis() const;
    bool  isVerticalAxis() const;

protected: //methods
    ::basegfx::B2DVector     getTickScreenPosition2D( double fScaledLogicTickValue ) const;

private: //member
    ::basegfx::B2DVector    m_aAxisStartScreenPosition2D;
    ::basegfx::B2DVector    m_aAxisEndScreenPosition2D;

    //labels might be posioned high or low on the border of the diagram far away from the axis
    //add this vector to go from the axis line to the label line (border of the diagram)
    ::basegfx::B2DVector    m_aAxisLineToLabelLineShift;

    double      m_fStrech_LogicToScreen;
    double      m_fOffset_LogicToScreen;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
