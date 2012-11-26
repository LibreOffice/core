/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
    virtual void MoveSdrDrag(const basegfx::B2DPoint& rPnt);
    virtual bool EndSdrDrag(bool bCopy);

    virtual void CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager);

private:
    E3dScene*   m_pScene;

    basegfx::B2DRange       m_aReferenceRect;
    basegfx::B2DPoint       m_aStartPos;
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
