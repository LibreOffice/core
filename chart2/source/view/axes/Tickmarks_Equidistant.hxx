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
#ifndef _CHART2_TICKMARKS_EQUIDISTANT_HXX
#define _CHART2_TICKMARKS_EQUIDISTANT_HXX

#include "Tickmarks.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

using ::basegfx::B2DVector;
//-----------------------------------------------------------------------------
/**
*/

class EquidistantTickIter : public TickIter
{
public:
    EquidistantTickIter( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< double > >& rTicks
                , const ExplicitIncrementData& rIncrement
            , sal_Int32 nMinDepth=0, sal_Int32 nMaxDepth=-1 );
    EquidistantTickIter( ::std::vector< ::std::vector< TickInfo > >& rTickInfos
            , const ExplicitIncrementData& rIncrement
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
    const ExplicitIncrementData& m_rIncrement;
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

class EquidistantTickFactory
{
public:
    EquidistantTickFactory(
         const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement );
    ~EquidistantTickFactory();

    void getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    void getAllTicksShifted( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    static double getMinimumAtIncrement( double fMin, const ExplicitIncrementData& rIncrement );
    static double getMaximumAtIncrement( double fMax, const ExplicitIncrementData& rIncrement );

private: //methods
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

private: //member
    ExplicitScaleData     m_rScale;
    ExplicitIncrementData m_rIncrement;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >
                                                m_xInverseScaling;

    //minimum and maximum of the visible range after scaling
    double    m_fScaledVisibleMin;
    double    m_fScaledVisibleMax;

    double*   m_pfCurrentValues;
    //major-tick positions that may lay outside the visible range but complete partly visible intervals at the borders
    double    m_fOuterMajorTickBorderMin;
    double    m_fOuterMajorTickBorderMax;
    double    m_fOuterMajorTickBorderMin_Scaled;
    double    m_fOuterMajorTickBorderMax_Scaled;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
