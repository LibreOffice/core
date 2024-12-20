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
#pragma once

#include <basegfx/range/b2irectangle.hxx>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <svx/unoshape.hxx>
#include <rtl/ref.hxx>


namespace chart
{
class Diagram;

/** The VDiagram is responsible to generate the visible parts of the Diagram
that is wall, floor, axes and data series.
The axes and data series are subobjects which are created and managed by the
diagram.
*/

class VDiagram final
{
public: //methods
    VDiagram( const rtl::Reference<::chart::Diagram>& xDiagram,
              const css::drawing::Direction3D& rPreferredAspectRatio,
              sal_Int32 nDimension );
    ~VDiagram();

    void init(
        const rtl::Reference<SvxShapeGroupAnyD>& xTarget );

    void    createShapes( const css::awt::Point& rPos
                        , const css::awt::Size& rSize );

    const rtl::Reference<SvxShapeGroupAnyD> &
            getCoordinateRegion() const { return m_xCoordinateRegionShape; }

    /**
     * Get current bounding rectangle for the diagram without axes.
     */
    basegfx::B2IRectangle getCurrentRectangle() const;

    void    reduceToMinimumSize();

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

    rtl::Reference<SvxShapeGroupAnyD>                    m_xTarget;

    // this is the surrounding shape which contains floor, wall and coordinate
    rtl::Reference<SvxShapeGroupAnyD>   m_xOuterGroupShape;
    // this is an additional inner shape that represents the coordinate region -  that is - where to place data points
    rtl::Reference<SvxShapeGroupAnyD>   m_xCoordinateRegionShape;
    rtl::Reference<SvxShapeRect> m_xWall2D;

    sal_Int32                          m_nDimensionCount;
    rtl::Reference< ::chart::Diagram > m_xDiagram;

    css::drawing::Direction3D                                  m_aPreferredAspectRatio;
    rtl::Reference< Svx3DSceneObject > m_xAspectRatio3D;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
