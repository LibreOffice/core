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
#ifndef _CHART2_VDIAGRAM_HXX
#define _CHART2_VDIAGRAM_HXX

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include "ShapeFactory.hxx"
#include <basegfx/range/b2irectangle.hxx>

namespace chart
{

class ShapeFactory;

/** The VDiagram is responsible to generate the visible parts of the Diagram
that is wall, floor, axes and data series.
The axes and data series are subobjects which are created and managed by the
diagram.
*/

class VDiagram
{
public: //methods
    VDiagram( const com::sun::star::uno::Reference<com::sun::star::chart2::XDiagram>& xDiagram,
              const com::sun::star::drawing::Direction3D& rPreferredAspectRatio,
              sal_Int32 nDimension = 3 );
    virtual ~VDiagram();

    void init(
        const com::sun::star::uno::Reference<com::sun::star::drawing::XShapes>& xTarget,
        const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xFactory );

    void    createShapes( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
            getCoordinateRegion();

    ::basegfx::B2IRectangle    getCurrentRectangle();

    void    reduceToMimimumSize();

    ::basegfx::B2IRectangle    adjustPosAndSize( const ::com::sun::star::awt::Point& rPos
                                    , const ::com::sun::star::awt::Size& rAvailableSize );

    ::basegfx::B2IRectangle    adjustInnerSize( const ::basegfx::B2IRectangle& rConsumedOuterRect );

private: //methods
    void    createShapes_2d();
    void    createShapes_3d();

    ::basegfx::B2IRectangle    adjustPosAndSize_2d( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rAvailableSize );
    ::basegfx::B2IRectangle    adjustPosAndSize_3d( const ::com::sun::star::awt::Point& rPos
                        , const ::com::sun::star::awt::Size& rAvailableSize );

    void    adjustAspectRatio3d( const ::com::sun::star::awt::Size& rAvailableSize );

private: //members
    VDiagram(const VDiagram& rD);

    ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::XShapes >                    m_xTarget;
    ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory>           m_xShapeFactory;
    ShapeFactory*                                                           m_pShapeFactory;

    // this is the surrounding shape which contains floor, wall and coordinate
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xOuterGroupShape;
    // this is an additional inner shape that represents the coordinate region -  that is - where to place data points
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xCoordinateRegionShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >   m_xWall2D;

    sal_Int32                                                               m_nDimensionCount;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDiagram >                                m_xDiagram;

    ::com::sun::star::drawing::Direction3D                                  m_aPreferredAspectRatio;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xAspectRatio3D;

    double m_fXAnglePi;
    double m_fYAnglePi;
    double m_fZAnglePi;

    ::com::sun::star::awt::Point    m_aAvailablePosIncludingAxes;
    ::com::sun::star::awt::Size     m_aAvailableSizeIncludingAxes;

    ::com::sun::star::awt::Point    m_aCurrentPosWithoutAxes;
    ::com::sun::star::awt::Size     m_aCurrentSizeWithoutAxes;

    bool m_bRightAngledAxes;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
