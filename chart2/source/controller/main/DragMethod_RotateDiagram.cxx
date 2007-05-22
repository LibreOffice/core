/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragMethod_RotateDiagram.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:07:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _SDR_OVERLAY_OVERLAYPOLYPOLYGON_HXX
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#endif
#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

// header for class E3dScene
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

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
{
    m_pScene = SelectionHelper::getSceneToRotate( rDrawViewWrapper.getNamedSdrObject( rObjectCID ) );
    SdrObject* pObj = rDrawViewWrapper.getSelectedObject();
    if(pObj && m_pScene)
    {
        m_aReferenceRect = pObj->GetLogicRect();
        Rectangle aTemp = m_pScene->GetLogicRect();

        m_pScene->CreateWireframe(m_aWireframePoly, NULL );

        DiagramHelper::getRotationAngleFromDiagram(
                uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( this->getChartModel() ), uno::UNO_QUERY )
                , m_fInitialXAngleRad, m_fInitialYAngleRad, m_fInitialZAngleRad );
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
    aCurrentTransform.rotate( m_fInitialXAngleRad + m_fAdditionalXAngleRad
                            , m_fInitialYAngleRad + m_fAdditionalYAngleRad
                            , m_fInitialZAngleRad + m_fAdditionalZAngleRad );

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

FASTBOOL DragMethod_RotateDiagram::End(FASTBOOL bCopy)
{
    Hide();
    DiagramHelper::setRotationAngleToDiagram( uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( this->getChartModel() ), uno::UNO_QUERY )
        , m_fInitialXAngleRad + m_fAdditionalXAngleRad, m_fInitialYAngleRad + m_fAdditionalYAngleRad, m_fInitialZAngleRad + m_fAdditionalZAngleRad );

    return true;
}
//.............................................................................
} //namespace chart
//.............................................................................
