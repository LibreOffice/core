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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_DRAGMETHOD_ROTATEDIAGRAM_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_DRAGMETHOD_ROTATEDIAGRAM_HXX

#include "DragMethod_Base.hxx"

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
        , const OUString& rObjectCID
        , const css::uno::Reference< css::frame::XModel >& xChartModel
        , RotationDirection eRotationDirection
        );
    virtual ~DragMethod_RotateDiagram();

    virtual void TakeSdrDragComment(OUString& rStr) const override;

    virtual bool BeginSdrDrag() override;
    virtual void MoveSdrDrag(const Point& rPnt) override;
    virtual bool EndSdrDrag(bool bCopy) override;

    virtual void CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager) override;

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
    bool    m_bRightAngledAxes;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_DRAGMETHOD_ROTATEDIAGRAM_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
