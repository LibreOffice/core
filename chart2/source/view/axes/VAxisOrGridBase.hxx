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
#ifndef _CHART2_VAXISORGRIDBASE_HXX
#define _CHART2_VAXISORGRIDBASE_HXX

#include "PlotterBase.hxx"
#include "ThreeDHelper.hxx"
#include "chartview/ExplicitScaleValues.hxx"

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <basegfx/matrix/b3dhommatrix.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class ShapeFactory;
class TickFactory;

class VAxisOrGridBase : public PlotterBase
{
public:
    VAxisOrGridBase( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount );
    virtual ~VAxisOrGridBase();

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );
    virtual void setExplicitScaleAndIncrement(
            const ExplicitScaleData& rScale
          , const ExplicitIncrementData& rIncrement )
                throw (::com::sun::star::uno::RuntimeException);
    void set3DWallPositions( CuboidPlanePosition eLeftWallPos, CuboidPlanePosition eBackWallPos, CuboidPlanePosition eBottomPos );

    virtual TickFactory* createTickFactory();

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
protected: //member
    ExplicitScaleData       m_aScale;
    ExplicitIncrementData   m_aIncrement;
    sal_Int32               m_nDimensionIndex;

    ::basegfx::B3DHomMatrix m_aMatrixScreenToScene;

    CuboidPlanePosition m_eLeftWallPos;
    CuboidPlanePosition m_eBackWallPos;
    CuboidPlanePosition m_eBottomPos;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
