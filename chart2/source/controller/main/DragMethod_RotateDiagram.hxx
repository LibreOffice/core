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
#ifndef CHART2_DRAGMETHOD_ROTATEDIAGRAM_HXX
#define CHART2_DRAGMETHOD_ROTATEDIAGRAM_HXX

#include "DragMethod_Base.hxx"

// header for class ::basegfx::B3DPolyPolygon
#include <basegfx/polygon/b3dpolypolygon.hxx>

namespace chart
{

class DragMethod_RotateDiagram : public DragMethod_Base
{
public:
    enum RotationDirection
    {
        ROTATIONDIRECTION_FREE,
        ROTATIONDIRECTION_X,
        ROTATIONDIRECTION_Y,
        ROTATIONDIRECTION_Z
    };

    DragMethod_RotateDiagram( DrawViewWrapper& rDrawViewWrapper
        , const rtl::OUString& rObjectCID
        , const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
        , RotationDirection eRotationDirection
        );
    virtual ~DragMethod_RotateDiagram();

    virtual void TakeSdrDragComment(String& rStr) const;

    virtual bool BeginSdrDrag();
    virtual void MoveSdrDrag(const Point& rPnt);
    virtual bool EndSdrDrag(bool bCopy);

    virtual void CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager);

private:
    E3dScene*   m_pScene;

    Rectangle   m_aReferenceRect;
    Point       m_aStartPos;
    basegfx::B3DPolyPolygon m_aWireframePolyPolygon;

    double      m_fInitialXAngleRad;
    double      m_fInitialYAngleRad;
    double      m_fInitialZAngleRad;

    double      m_fAdditionalXAngleRad;
    double      m_fAdditionalYAngleRad;
    double      m_fAdditionalZAngleRad;

    sal_Int32 m_nInitialHorizontalAngleDegree;
    sal_Int32 m_nInitialVerticalAngleDegree;

    sal_Int32 m_nAdditionalHorizontalAngleDegree;
    sal_Int32 m_nAdditionalVerticalAngleDegree;

    RotationDirection m_eRotationDirection;
    sal_Bool    m_bRightAngledAxes;
};

} //  namespace chart

// CHART2_DRAGMETHOD_ROTATEDIAGRAM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
