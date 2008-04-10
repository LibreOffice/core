/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DragMethod_RotateDiagram.cxx,v $
 * $Revision: 1.5 $
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
    , m_aWireframePoly()
    , m_fInitialXAngleRad(0.0)
    , m_fInitialYAngleRad(0.0)
    , m_fInitialZAngleRad(0.0)
    , m_fAdditionalXAngleRad(0.0)
    , m_fAdditionalYAngleRad(0.0)
    , m_fAdditionalZAngleRad(0.0)
    , m_eRotationDirection(eRotationDirection)
    , m_bRightAngledAxes(sal_False)
{
    m_pScene = SelectionHelper::getSceneToRotate( rDrawViewWrapper.getNamedSdrObject( rObjectCID ) );
    SdrObject* pObj = rDrawViewWrapper.getSelectedObject();
    if(pObj && m_pScene)
    {
        m_aReferenceRect = pObj->GetLogicRect();
        Rectangle aTemp = m_pScene->GetLogicRect();

        m_pScene->CreateWireframe(m_aWireframePoly, NULL );

        uno::Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram(this->getChartModel()) );
        uno::Reference< beans::XPropertySet > xDiagramProperties( xDiagram, uno::UNO_QUERY );
        if( xDiagramProperties.is() )
        {
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
void DragMethod_RotateDiagram::TakeComment(String& /*rStr*/) const
{
}
FASTBOOL DragMethod_RotateDiagram::Beg()
{
    m_aStartPos = DragStat().GetStart();
    SetDragPolys();
    Show();
    return true;
}
void DragMethod_RotateDiagram::Mov(const Point& rPnt)
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

        DragStat().NextMove(rPnt);
        MovAllPoints();

        Show();
    }
}
void DragMethod_RotateDiagram::CreateOverlayGeometry(::sdr::overlay::OverlayManager& rOverlayManager, ::sdr::overlay::OverlayObjectList& rOverlayList)
{
    ::basegfx::B3DHomMatrix aCurrentTransform;
    aCurrentTransform.translate( -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                                 -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                                 -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0 );

    double fResultX = m_fInitialXAngleRad + m_fAdditionalXAngleRad;
    double fResultY = m_fInitialYAngleRad + m_fAdditionalYAngleRad;
    double fResultZ = m_fInitialZAngleRad + m_fAdditionalZAngleRad;

    if(!m_bRightAngledAxes)
        aCurrentTransform.rotate( fResultX, fResultY, fResultZ );
    else
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fResultX, fResultY );
        aCurrentTransform.shearXY(fResultY,-(fResultX));
    }

    const sal_uInt32 nPntCnt(m_aWireframePoly.count());
     if(nPntCnt > 1L && m_pScene)
    {
        B3dCamera& rCameraSet = m_pScene->GetCameraSet();
        for(sal_uInt32 b(0L); b < nPntCnt; b += 2L)
        {
            ::basegfx::B2DPolygon aLine;
            ::basegfx::B3DPoint aPnt1(aCurrentTransform * m_aWireframePoly.getB3DPoint(b));
            aPnt1 = rCameraSet.WorldToViewCoor(aPnt1);
            aLine.append(::basegfx::B2DPoint(aPnt1.getX(), aPnt1.getY()));

            ::basegfx::B3DPoint aPnt2(aCurrentTransform * m_aWireframePoly.getB3DPoint(b + 1L));
            aPnt2 = rCameraSet.WorldToViewCoor(aPnt2);
            aLine.append(::basegfx::B2DPoint(aPnt2.getX(), aPnt2.getY()));

            basegfx::B2DPolyPolygon aResult(aLine);
            ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aResult);
            rOverlayManager.add(*pNew);
            rOverlayList.append(*pNew);
        }
    }
}

FASTBOOL DragMethod_RotateDiagram::End(FASTBOOL /* bCopy */)
{
    Hide();

    double fResultX = m_fInitialXAngleRad + m_fAdditionalXAngleRad;
    double fResultY = m_fInitialYAngleRad + m_fAdditionalYAngleRad;
    double fResultZ = m_fInitialZAngleRad + m_fAdditionalZAngleRad;

    if(m_bRightAngledAxes)
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fResultX, fResultY );

    ThreeDHelper::setRotationAngleToDiagram( uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( this->getChartModel() ), uno::UNO_QUERY )
        , fResultX, fResultY, fResultZ );

    return true;
}
//.............................................................................
} //namespace chart
//.............................................................................
