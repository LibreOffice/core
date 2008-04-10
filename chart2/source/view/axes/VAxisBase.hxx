/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VAxisBase.hxx,v $
 * $Revision: 1.3 $
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
#include "TickmarkHelper.hxx"
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

    virtual void SAL_CALL createMaximumLabels()=0;
    virtual void SAL_CALL createLabels()=0;
    virtual void SAL_CALL updatePositions()=0;

    virtual sal_Bool SAL_CALL isAnythingToDraw();
    virtual void SAL_CALL initAxisLabelProperties(
                    const ::com::sun::star::awt::Size& rFontReferenceSize
                  , const ::com::sun::star::awt::Rectangle& rMaximumSpaceForLabels );

    virtual void SAL_CALL setExplicitScaleAndIncrement(
            const ::com::sun::star::chart2::ExplicitScaleData& rScale
          , const ::com::sun::star::chart2::ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 estimateMaximumAutoMainIncrementCount();

    void setExrtaLinePositionAtOtherAxis( const double& fCrossingAt );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
protected: //methods
    sal_Int32 getIndexOfLongestLabel( const ::com::sun::star::uno::Sequence< rtl::OUString >& rLabels );
    void removeTextShapesFromTicks( TickIter& rIter, const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget );
    void updateUnscaledValuesAtTicks( TickIter& rIter );

    virtual bool prepareShapeCreation();
    void recordMaximumTextSize( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShape >& xShape
                    , double fRotationAngleDegree );

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
