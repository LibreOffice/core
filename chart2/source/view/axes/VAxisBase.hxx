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
#ifndef _CHART2_VAXISBASE_HXX
#define _CHART2_VAXISBASE_HXX

#include "VAxisOrGridBase.hxx"
#include "VAxisProperties.hxx"
#include "Tickmarks.hxx"
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class VAxisBase : public VAxisOrGridBase
{
public:
    VAxisBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
           , const AxisProperties& rAxisProperties
           , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    virtual ~VAxisBase();

    sal_Int32 getDimensionCount();

    virtual void createMaximumLabels()=0;
    virtual void createLabels()=0;
    virtual void updatePositions()=0;

    virtual sal_Bool isAnythingToDraw();
    virtual void initAxisLabelProperties(
                    const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels );

    virtual void setExplicitScaleAndIncrement(
            const ExplicitScaleData& rScale
          , const ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();
    virtual void createAllTickInfos( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos );

    void setExrtaLinePositionAtOtherAxis( const double& fCrossingAt );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
protected: //methods
    sal_Int32 getIndexOfLongestLabel( const ::com::sun::star::uno::Sequence< rtl::OUString >& rLabels );
    void removeTextShapesFromTicks();
    void updateUnscaledValuesAtTicks( TickIter& rIter );

    virtual bool prepareShapeCreation();
    void recordMaximumTextSize( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape
                    , double fRotationAngleDegree );

    bool isDateAxis() const;
    bool isComplexCategoryAxis() const;

protected: //member
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xNumberFormatsSupplier;
    AxisProperties                                                                      m_aAxisProperties;
    AxisLabelProperties                                                                 m_aAxisLabelProperties;
    ::com::sun::star::uno::Sequence< rtl::OUString >                                    m_aTextLabels;
    bool                                                                                m_bUseTextLabels;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xGroupShape_Shapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > m_xTextTarget;

    ::std::vector< ::std::vector< TickInfo > >                                          m_aAllTickInfos;
    bool m_bReCreateAllTickInfos;

    bool m_bRecordMaximumTextSize;
    sal_Int32 m_nMaximumTextWidthSoFar;
    sal_Int32 m_nMaximumTextHeightSoFar;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
