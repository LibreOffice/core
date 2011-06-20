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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "DragMethod_RotateDiagram.hxx"

#include "SelectionHelper.hxx"
#include "CommonConverters.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

// header for class E3dScene
#include <svx/scene3d.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>

#define FIXED_SIZE_FOR_3D_CHART_VOLUME (10000.0)

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

DragMethod_RotateDiagram::DragMethod_RotateDiagram( DrawViewWrapper& rDrawViewWrapper
        , const rtl::OUString& rObjectCID
        , const Reference< frame::XModel >& xChartModel
        , RotationDirection eRotationDirection )
    : DragMethod_Base( rDrawViewWrapper, rObjectCID, xChartModel, ActionDescriptionProvider::ROTATE )
    , m_pScene(0)
    , m_aReferenceRect(100,100,100,100)
    , m_aStartPos(0,0)
    , m_aWireframePolyPolygon()
    , m_fInitialXAngleRad(0.0)
    , m_fInitialYAngleRad(0.0)
    , m_fInitialZAngleRad(0.0)
    , m_fAdditionalXAngleRad(0.0)
    , m_fAdditionalYAngleRad(0.0)
    , m_fAdditionalZAngleRad(0.0)
    , m_nInitialHorizontalAngleDegree(0)
    , m_nInitialVerticalAngleDegree(0)
    , m_nAdditionalHorizontalAngleDegree(0)
    , m_nAdditionalVerticalAngleDegree(0)
    , m_eRotationDirection(eRotationDirection)
    , m_bRightAngledAxes(sal_False)
{
    m_pScene = SelectionHelper::getSceneToRotate( rDrawViewWrapper.getNamedSdrObject( rObjectCID ) );
    SdrObject* pObj = rDrawViewWrapper.getSelectedObject();
    if(pObj && m_pScene)
    {
        m_aReferenceRect = pObj->GetLogicRect();

        m_aWireframePolyPolygon = m_pScene->CreateWireframe();

        uno::Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram(this->getChartModel()) );
        uno::Reference< beans::XPropertySet > xDiagramProperties( xDiagram, uno::UNO_QUERY );
        if( xDiagramProperties.is() )
        {
            ThreeDHelper::getRotationFromDiagram( xDiagramProperties
                , m_nInitialHorizontalAngleDegree, m_nInitialVerticalAngleDegree );

            ThreeDHelper::getRotationAngleFromDiagram( xDiagramProperties
                , m_fInitialXAngleRad, m_fInitialYAngleRad, m_fInitialZAngleRad );

            if( ChartTypeHelper::isSupportingRightAngledAxes(
                DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
                xDiagramProperties->getPropertyValue(C2U( "RightAngledAxes" )) >>= m_bRightAngledAxes;
            if(m_bRightAngledAxes)
            {
                if( m_eRotationDirection==ROTATIONDIRECTION_Z )
                    m_eRotationDirection=ROTATIONDIRECTION_FREE;
                ThreeDHelper::adaptRadAnglesForRightAngledAxes( m_fInitialXAngleRad, m_fInitialYAngleRad );
            }
        }
    }
}
DragMethod_RotateDiagram::~DragMethod_RotateDiagram()
{
}
void DragMethod_RotateDiagram::TakeSdrDragComment(String& /*rStr*/) const
{
}
bool DragMethod_RotateDiagram::BeginSdrDrag()
{
    m_aStartPos = DragStat().GetStart();
    Show();
    return true;
}
void DragMethod_RotateDiagram::MoveSdrDrag(const Point& rPnt)
{
    if( DragStat().CheckMinMoved(rPnt) )
    {
        Hide();

        //calculate new angle
        double fX = F_PI / 2.0 * (double)(rPnt.Y() - m_aStartPos.Y())
                / (double)m_aReferenceRect.GetHeight();
        double fY = F_PI * (double)(rPnt.X() - m_aStartPos.X())
                / (double)m_aReferenceRect.GetWidth();

        if( m_eRotationDirection != ROTATIONDIRECTION_Y )
            m_fAdditionalYAngleRad = fY;
        else
            m_fAdditionalYAngleRad = 0.0;
        if( m_eRotationDirection != ROTATIONDIRECTION_X )
            m_fAdditionalXAngleRad = fX;
        else
            m_fAdditionalXAngleRad = 0.0;
        m_fAdditionalZAngleRad = 0.0;

        if( m_eRotationDirection == ROTATIONDIRECTION_Z )
        {
            m_fAdditionalXAngleRad = 0.0;
            m_fAdditionalYAngleRad = 0.0;

            double fCx = m_aReferenceRect.Center().X();
            double fCy = m_aReferenceRect.Center().Y();

            m_fAdditionalZAngleRad = atan((double)(fCx - m_aStartPos.X())/(m_aStartPos.Y()-fCy))
                + atan((double)(fCx - rPnt.X())/(fCy-rPnt.Y()));
        }

        m_nAdditionalHorizontalAngleDegree = static_cast<sal_Int32>(m_fAdditionalXAngleRad*180.0/F_PI);
        m_nAdditionalVerticalAngleDegree = -static_cast<sal_Int32>(m_fAdditionalYAngleRad*180.0/F_PI);

        DragStat().NextMove(rPnt);
        Show();
    }
}
bool DragMethod_RotateDiagram::EndSdrDrag(bool /*bCopy*/)
{
    Hide();

    if( m_bRightAngledAxes || m_eRotationDirection==ROTATIONDIRECTION_Z )
    {
        double fResultX = m_fInitialXAngleRad + m_fAdditionalXAngleRad;
        double fResultY = m_fInitialYAngleRad + m_fAdditionalYAngleRad;
        double fResultZ = m_fInitialZAngleRad + m_fAdditionalZAngleRad;

        if(m_bRightAngledAxes)
            ThreeDHelper::adaptRadAnglesForRightAngledAxes( fResultX, fResultY );

        ThreeDHelper::setRotationAngleToDiagram( uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( this->getChartModel() ), uno::UNO_QUERY )
            , fResultX, fResultY, fResultZ );
    }
    else
    {
        ThreeDHelper::setRotationToDiagram( ( uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( this->getChartModel() ), uno::UNO_QUERY ) )
            , m_nInitialHorizontalAngleDegree+m_nAdditionalHorizontalAngleDegree, m_nInitialVerticalAngleDegree+m_nAdditionalVerticalAngleDegree );
    }

    return true;
}
void DragMethod_RotateDiagram::CreateOverlayGeometry(sdr::overlay::OverlayManager& rOverlayManager)
{
    ::basegfx::B3DHomMatrix aCurrentTransform;
    aCurrentTransform.translate( -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                                 -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                                 -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0 );

    double fResultX = m_fInitialXAngleRad + m_fAdditionalXAngleRad;
    double fResultY = m_fInitialYAngleRad + m_fAdditionalYAngleRad;
    double fResultZ = m_fInitialZAngleRad + m_fAdditionalZAngleRad;

    if(!m_bRightAngledAxes)
    {
        if( m_eRotationDirection!=ROTATIONDIRECTION_Z )
        {
            ThreeDHelper::convertElevationRotationDegToXYZAngleRad(
                m_nInitialHorizontalAngleDegree+m_nAdditionalHorizontalAngleDegree, -(m_nInitialVerticalAngleDegree+m_nAdditionalVerticalAngleDegree)
                , fResultX, fResultY, fResultZ );
        }
        aCurrentTransform.rotate( fResultX, fResultY, fResultZ );
    }
    else
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fResultX, fResultY );
        aCurrentTransform.shearXY(fResultY,-(fResultX));
    }

    if(m_aWireframePolyPolygon.count() && m_pScene)
    {
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(m_pScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
        const basegfx::B3DHomMatrix aWorldToView(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection() * aViewInfo3D.getOrientation());
        const basegfx::B3DHomMatrix aTransform(aWorldToView * aCurrentTransform);

        // transform to relative scene coordinates
        basegfx::B2DPolyPolygon aPolyPolygon(basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(m_aWireframePolyPolygon, aTransform));

        // transform to 2D view coordinates
        aPolyPolygon.transform(rVCScene.getObjectTransformation());

        sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aPolyPolygon);
        rOverlayManager.add(*pNew);
        addToOverlayObjectList(*pNew);
    }
}
//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
