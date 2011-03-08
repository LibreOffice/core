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
#ifndef _CHART2_TICKMARKHELPER_HXX
#define _CHART2_TICKMARKHELPER_HXX

#include "TickmarkProperties.hxx"
#include "VAxisProperties.hxx"
#include <com/sun/star/chart2/ExplicitIncrementData.hpp>
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#include <basegfx/vector/b2dvector.hxx>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/uno/Sequence.h>

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

using ::basegfx::B2DVector;
//-----------------------------------------------------------------------------
/**
*/

struct TickInfo
{
    double      fScaledTickValue;
    double      fUnscaledTickValue;

    ::basegfx::B2DVector  aTickScreenPosition;
    bool        bPaintIt;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > xTextShape;

    rtl::OUString aText;//used only for complex categories so far
    sal_Int32 nFactorForLimitedTextWidth;//categories in higher levels of complex categories can have more place than a single simple category

//methods:
    TickInfo();
    void        updateUnscaledValue( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XScaling >& xInverseScaling );

    sal_Int32 getScreenDistanceBetweenTicks( const TickInfo& rOherTickInfo ) const;
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

class EquidistantTickIter : public TickIter
{
public:
    EquidistantTickIter( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< double > >& rTicks
                , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement
            , sal_Int32 nMinDepth=0, sal_Int32 nMaxDepth=-1 );
    EquidistantTickIter( ::std::vector< ::std::vector< TickInfo > >& rTickInfos
            , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement
            , sal_Int32 nMinDepth=0, sal_Int32 nMaxDepth=-1 );
    virtual ~EquidistantTickIter();

    virtual double*     firstValue();
    virtual double*     nextValue();

    virtual TickInfo*   firstInfo();
    virtual TickInfo*   nextInfo();

    sal_Int32   getCurrentDepth() const { return m_nCurrentDepth; }

protected:
    bool        gotoIndex( sal_Int32 nTickIndex );
    sal_Int32   getCurrentIndex() const;
    sal_Int32   getMaxIndex() const;

private: //methods
    sal_Int32   getIntervalCount( sal_Int32 nDepth );
    bool        isAtLastPartTick();

    void        initIter( sal_Int32 nMinDepth, sal_Int32 nMaxDepth );
    sal_Int32   getStartDepth() const;

    bool        gotoFirst();
    bool        gotoNext();


    double      getTickValue(sal_Int32 nDepth, sal_Int32 nIndex) const
                {
                    if(m_pSimpleTicks)
                        return (*m_pSimpleTicks)[nDepth][nIndex];
                    else
                        return (((*m_pInfoTicks)[nDepth])[nIndex]).fScaledTickValue;
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
    const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Sequence< double > >*  m_pSimpleTicks;
    ::std::vector< ::std::vector< TickInfo > >*       m_pInfoTicks;
    const ::com::sun::star::chart2::ExplicitIncrementData& m_rIncrement;
    //iteration from m_nMinDepth to m_nMaxDepth
    sal_Int32   m_nMinDepth;
    sal_Int32   m_nMaxDepth;
    sal_Int32   m_nTickCount;
    sal_Int32*  m_pnPositions; //current positions in the different sequences
    sal_Int32*  m_pnPreParentCount; //the tickmarks do not start with a major tick always,
                                    //the PreParentCount states for each depth how many subtickmarks are available in front of the first parent tickmark
    bool*       m_pbIntervalFinished;
    sal_Int32   m_nCurrentDepth;
    sal_Int32   m_nCurrentPos;
    double      m_fCurrentValue;
};

class TickmarkHelper
{
public:
    TickmarkHelper(
         const ::com::sun::star::chart2::ExplicitScaleData& rScale
        , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement );
    virtual ~TickmarkHelper();

    virtual TickmarkHelper* createShiftedTickmarkHelper() const;

    void          getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    void          getAllTicksShifted( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    //
    static double getMinimumAtIncrement( double fMin, const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement );
    static double getMaximumAtIncrement( double fMax, const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement );

protected: //methods
    void        addSubTicks( sal_Int32 nDepth,
                        ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Sequence< double > >& rParentTicks ) const;
    double*     getMajorTick( sal_Int32 nTick ) const;
    double*     getMinorTick( sal_Int32 nTick, sal_Int32 nDepth
                    , double fStartParentTick, double fNextParentTick ) const;
    sal_Int32   getMaxTickCount( sal_Int32 nDepth = 0 ) const;
    sal_Int32   getTickDepth() const;
    bool        isVisible( double fValue ) const;
    bool        isWithinOuterBorder( double fScaledValue ) const; //all within the outer major tick marks

    virtual void updateScreenValues( ::std::vector< ::std::vector< TickInfo > >& /*rAllTickInfos*/ ) const {}

protected: //member
    ::com::sun::star::chart2::ExplicitScaleData     m_rScale;
    ::com::sun::star::chart2::ExplicitIncrementData m_rIncrement;

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >
                                                m_xInverseScaling;
    double*   m_pfCurrentValues;
    //major-tick positions that may lay outside the visible range but complete partly visible intervals at the borders
    double    m_fOuterMajorTickBorderMin;
    double    m_fOuterMajorTickBorderMax;
    double    m_fOuterMajorTickBorderMin_Scaled;
    double    m_fOuterMajorTickBorderMax_Scaled;

    //minimum and maximum of the visible range after scaling
    double    m_fScaledVisibleMin;
    double    m_fScaledVisibleMax;
};

class TickmarkHelper_2D : public TickmarkHelper
{
public:
    TickmarkHelper_2D(
        const ::com::sun::star::chart2::ExplicitScaleData& rScale
        , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement
        , const ::basegfx::B2DVector& rStartScreenPos, const ::basegfx::B2DVector& rEndScreenPos
        , const ::basegfx::B2DVector& rAxisLineToLabelLineShift );
        //, double fStrech_SceneToScreen, double fOffset_SceneToScreen );
    virtual ~TickmarkHelper_2D();

    virtual TickmarkHelper* createShiftedTickmarkHelper() const;

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

class TickmarkHelper_3D : public TickmarkHelper
{
public:
    TickmarkHelper_3D(
         const ::com::sun::star::chart2::ExplicitScaleData& rScale
        , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement );
    virtual ~TickmarkHelper_3D();

    virtual TickmarkHelper* createShiftedTickmarkHelper() const;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
