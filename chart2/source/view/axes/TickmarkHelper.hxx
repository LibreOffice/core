/*************************************************************************
 *
 *  $RCSfile: TickmarkHelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_TICKMARKHELPER_HXX
#define _CHART2_TICKMARKHELPER_HXX

#include <vector>

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITINCREMENTDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitIncrementData.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <drafts/com/sun/star/chart2/ExplicitScaleData.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct TickInfo
{
    double      fScaledTickValue;
    double      fUnscaledTickValue;

    sal_Int32   nScreenTickValue;

    bool        bPaintIt;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > xTextShape;

//methods:
    TickInfo();
    void        updateUnscaledValue( const ::com::sun::star::uno::Reference<
                    ::drafts::com::sun::star::chart2::XScaling >& xInverseScaling );
};

class TickIter
{
public:
    TickIter( const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< double > >& rTicks
                , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement
            , sal_Int32 nMinDepth=0, sal_Int32 nMaxDepth=-1 );
    TickIter( ::std::vector< ::std::vector< TickInfo > >& rTickInfos
            , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement
            , sal_Int32 nMinDepth=0, sal_Int32 nMaxDepth=-1 );
    virtual ~TickIter();

    double*     firstValue();
    double*     nextValue();

    TickInfo*   firstInfo();
    TickInfo*   nextInfo();

    sal_Int32   getCurrentDepth() const { return m_nCurrentDepth; }

private: //methods
    sal_Int32   getIntervalCount( sal_Int32 nDepth );
    bool        isAtLastPartTick();

    void        initIter( sal_Int32 nMinDepth, sal_Int32 nMaxDepth );
    sal_Int32   getStartDepth() const;

    bool        gotoFirst();
    bool        gotoNext();


    double      _getTickValue(sal_Int32 nDepth, sal_Int32 nIndex) const
                {
                    if(m_pSimpleTicks)
                        return (*m_pSimpleTicks)[nDepth][nIndex];
                    else
                        return (((*m_pInfoTicks)[nDepth])[nIndex]).fScaledTickValue;
                }
    sal_Int32   _getTickCount( sal_Int32 nDepth ) const
                {
                    if(m_pSimpleTicks)
                        return (*m_pSimpleTicks)[nDepth].getLength();
                    else
                        return (*m_pInfoTicks)[nDepth].size();
                }
    sal_Int32   _getMaxDepth() const
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
    const ::drafts::com::sun::star::chart2::ExplicitIncrementData& m_rIncrement;
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
         const ::drafts::com::sun::star::chart2::ExplicitScaleData& rScale
        , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement );
    virtual ~TickmarkHelper();

    void          getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    bool          isPostEquidistant( sal_Int32 nDepth ) const;

    //
    static double getMinimumAtIncrement( double fMin, const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement );
    static double getMaximumAtIncrement( double fMax, const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement );

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

    virtual void updateScreenValues( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const {}
    virtual void hideIdenticalScreenValues( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const {}

protected: //member
    const ::drafts::com::sun::star::chart2::ExplicitScaleData&     m_rScale;
    const ::drafts::com::sun::star::chart2::ExplicitIncrementData& m_rIncrement;

    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XScaling >
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
         const ::drafts::com::sun::star::chart2::ExplicitScaleData& rScale
        , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement
        , double fStrech_SceneToScreen, double fOffset_SceneToScreen );
    virtual ~TickmarkHelper_2D();

    static sal_Int32    getTickScreenDistance( TickIter& rIter );

    //methods more for axis line
    sal_Int32           getScreenValueForMinimum() const;
    sal_Int32           getScreenValueForMaximum() const;

protected: //methods
    virtual void        updateScreenValues( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    virtual void        hideIdenticalScreenValues( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

    sal_Int32           transformScaledLogicTickToScreen( double fValue ) const;

private: //member
    double  m_fStrech_LogicToScreen;
    double  m_fOffset_LogicToScreen;
};

class TickmarkHelper_3D : public TickmarkHelper
{
public:
    TickmarkHelper_3D(
         const ::drafts::com::sun::star::chart2::ExplicitScaleData& rScale
        , const ::drafts::com::sun::star::chart2::ExplicitIncrementData& rIncrement );
    virtual ~TickmarkHelper_3D();

};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
