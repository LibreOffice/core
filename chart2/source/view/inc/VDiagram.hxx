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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_VDIAGRAM_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_VDIAGRAM_HXX

#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include "AbstractShapeFactory.hxx"
#include <basegfx/range/b2irectangle.hxx>

namespace chart
{

class AbstractShapeFactory;

/** The VDiagram is responsible to generate the visible parts of the Diagram
that is wall, floor, axes and data series.
The axes and data series are subobjects which are created and managed by the
diagram.
*/

class VDiagram
{
public: //methods
    VDiagram( const css::uno::Reference<css::chart2::XDiagram>& xDiagram,
              const css::drawing::Direction3D& rPreferredAspectRatio,
              sal_Int32 nDimension = 3 );
    virtual ~VDiagram();

    void init(
        const css::uno::Reference<css::drawing::XShapes>& xTarget,
        const css::uno::Reference<css::lang::XMultiServiceFactory>& xFactory );

    void    createShapes( const css::awt::Point& rPos
                        , const css::awt::Size& rSize );

    css::uno::Reference< css::drawing::XShapes >
            getCoordinateRegion() { return css::uno::Reference<css::drawing::XShapes>( m_xCoordinateRegionShape, css::uno::UNO_QUERY );}

    /**
     * Get current bounding rectangle for the diagram without axes.
     */
    basegfx::B2IRectangle getCurrentRectangle() const;

    void    reduceToMimimumSize();

    ::basegfx::B2IRectangle    adjustPosAndSize( const css::awt::Point& rPos
                                    , const css::awt::Size& rAvailableSize );

    ::basegfx::B2IRectangle    adjustInnerSize( const ::basegfx::B2IRectangle& rConsumedOuterRect );

private: //methods
    void    createShapes_2d();
    void    createShapes_3d();

    ::basegfx::B2IRectangle    adjustPosAndSize_2d( const css::awt::Point& rPos
                        , const css::awt::Size& rAvailableSize );
    ::basegfx::B2IRectangle    adjustPosAndSize_3d( const css::awt::Point& rPos
                        , const css::awt::Size& rAvailableSize );

    void    adjustAspectRatio3d( const css::awt::Size& rAvailableSize );

private: //members
    VDiagram(const VDiagram& rD) = delete;

    css::uno::Reference< css::drawing::XShapes >                    m_xTarget;
    css::uno::Reference< css::lang::XMultiServiceFactory>           m_xShapeFactory;
    AbstractShapeFactory* m_pShapeFactory;

    // this is the surrounding shape which contains floor, wall and coordinate
    css::uno::Reference< css::drawing::XShape >   m_xOuterGroupShape;
    // this is an additional inner shape that represents the coordinate region -  that is - where to place data points
    css::uno::Reference< css::drawing::XShape >   m_xCoordinateRegionShape;
    css::uno::Reference< css::drawing::XShape >   m_xWall2D;

    sal_Int32                                                               m_nDimensionCount;
    css::uno::Reference< css::chart2::XDiagram >                                m_xDiagram;

    css::drawing::Direction3D                                  m_aPreferredAspectRatio;
    css::uno::Reference< css::beans::XPropertySet > m_xAspectRatio3D;

    double m_fXAnglePi;
    double m_fYAnglePi;
    double m_fZAnglePi;

    css::awt::Point    m_aAvailablePosIncludingAxes;
    css::awt::Size     m_aAvailableSizeIncludingAxes;

    css::awt::Point    m_aCurrentPosWithoutAxes;
    css::awt::Size     m_aCurrentSizeWithoutAxes;

    bool m_bRightAngledAxes;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
