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
#ifndef _CHART2_PLOTTERBASE_HXX
#define _CHART2_PLOTTERBASE_HXX

#include "chartview/ExplicitScaleValues.hxx"

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XTransformation.hpp>

#include <cppuhelper/implbase1.hxx>
#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

class ShapeFactory;
class PlottingPositionHelper;

class PlotterBase
{
public:
    PlotterBase( sal_Int32 nDimension );
    virtual ~PlotterBase();

    virtual void initPlotter(
          const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xLogicTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xFinalTarget
        , const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory >& xFactory
        , const rtl::OUString& rCID
                ) throw (::com::sun::star::uno::RuntimeException );

    virtual void setScales( const ::std::vector< ExplicitScaleData >& rScales, bool bSwapXAndYAxis );

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix );

    virtual void createShapes() = 0;

    static bool isValidPosition( const ::com::sun::star::drawing::Position3D& rPos );

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
private: //methods
    //no default constructor
    PlotterBase();

protected: //methods
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroupShape( const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , ::rtl::OUString rName=::rtl::OUString() );

protected: //member
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xLogicTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                m_xFinalTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>       m_xShapeFactory;
    ShapeFactory*                                                       m_pShapeFactory;
    //::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>          m_xCC;

    rtl::OUString   m_aCID;

    sal_Int32                                           m_nDimension;
    // needs to be created and deleted by the derived class
    PlottingPositionHelper*                             m_pPosHelper;
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif
