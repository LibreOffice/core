/*************************************************************************
 *
 *  $RCSfile: ChartController_Window.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:36:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartController.hxx"
#include "SelectionHelper.hxx"
#include "PositionAndSizeHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "ChartWindow.hxx"
#include "chartview/ChartView.hxx"
#include "Chart.hrc"
#include "ResId.hxx"
#include "CommonConverters.hxx"
#include "Rotation.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_SCENEDESCRIPTOR_HPP_
#include <com/sun/star/chart2/SceneDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LAYOUT_RELATIVEPOSITION_HPP_
#include <com/sun/star/layout/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_LAYOUT_RELATIVESIZE_HPP_
#include <com/sun/star/layout/RelativeSize.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <com/sun/star/chart2/LegendPosition.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif


// header for class PopupMenu
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#include "macros.hxx"
#include "SchSlotIds.hxx"
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

// header for class E3dObject
#ifndef _E3D_OBJ3D_HXX
#include <svx/obj3d.hxx>
#endif

#include <rtl/math.hxx>

// header for class SdrDragMethod
#ifndef _SVDDRGMT_HXX
#include <svx/svddrgmt.hxx>
#endif

#define DRGPIX    2     // Drag MinMove in Pixel


//#define CHART2_DEBUG_VIEW_PERFORMANCE 1
#ifdef CHART2_DEBUG_VIEW_PERFORMANCE
#include <time.h>
#endif

// header for class B3dCamera
#ifndef _B3D_B3DTRANS_HXX
#include <goodies/b3dtrans.hxx>
#endif
// header for class E3dScene
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif
// header for class ExtOutputDevice
#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif

#define FIXED_SIZE_FOR_3D_CHART_VOLUME (10000.0)

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------
// awt::XWindow
//-----------------------------------------------------------------
    void SAL_CALL ChartController
::setPosSize( sal_Int32 X, sal_Int32 Y
            , sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setPosSize( X, Y, Width, Height, Flags );
}

    awt::Rectangle SAL_CALL ChartController
::getPosSize()
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        return xWindow->getPosSize();
    return awt::Rectangle(0,0,0,0);
}

    void SAL_CALL ChartController
::setVisible( sal_Bool Visible )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setVisible( Visible );
}

    void SAL_CALL ChartController
::setEnable( sal_Bool Enable )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setEnable( Enable );
}

    void SAL_CALL ChartController
::setFocus()    throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->setFocus();
}

    void SAL_CALL ChartController
::addWindowListener( const uno::Reference<
            awt::XWindowListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addWindowListener( xListener );
}

    void SAL_CALL ChartController
::removeWindowListener( const uno::Reference<
            awt::XWindowListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeWindowListener( xListener );
}

    void SAL_CALL ChartController
::addFocusListener( const uno::Reference<
            awt::XFocusListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addFocusListener( xListener );
}

    void SAL_CALL ChartController
::removeFocusListener( const uno::Reference<
            awt::XFocusListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeFocusListener( xListener );
}

    void SAL_CALL ChartController
::addKeyListener( const uno::Reference<
            awt::XKeyListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addKeyListener( xListener );
}

    void SAL_CALL ChartController
::removeKeyListener( const uno::Reference<
            awt::XKeyListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeKeyListener( xListener );
}

    void SAL_CALL ChartController
::addMouseListener( const uno::Reference<
            awt::XMouseListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addMouseListener( xListener );
}

    void SAL_CALL ChartController
::removeMouseListener( const uno::Reference<
            awt::XMouseListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeMouseListener( xListener );
}

    void SAL_CALL ChartController
::addMouseMotionListener( const uno::Reference<
            awt::XMouseMotionListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addMouseMotionListener( xListener );
}

    void SAL_CALL ChartController
::removeMouseMotionListener( const uno::Reference<
            awt::XMouseMotionListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removeMouseMotionListener( xListener );
}

    void SAL_CALL ChartController
::addPaintListener( const uno::Reference<
            awt::XPaintListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->addPaintListener( xListener );
}

    void SAL_CALL ChartController
::removePaintListener( const uno::Reference<
            awt::XPaintListener >& xListener )
            throw (uno::RuntimeException)
{
    //@todo
    uno::Reference<awt::XWindow> xWindow = m_xViewWindow;

    if(xWindow.is())
        xWindow->removePaintListener( xListener );
}

//-----------------------------------------------------------------
// impl vcl window controller methods
//-----------------------------------------------------------------
void ChartController::execute_Paint( const Rectangle& rRect )
{
#ifdef CHART2_DEBUG_VIEW_PERFORMANCE
    static clock_t nBeforeBuild = 0;
    static clock_t nAfterBuild = 0;
    static clock_t nBeforePaint = 0;
    static clock_t nAfterPaint = 0;
    static double fBuildTimeInSec = 0.0;
    static double fPaintTimeInSec = 0.0;
#endif

    if(m_bViewDirty)
    {
#ifdef CHART2_DEBUG_VIEW_PERFORMANCE
        nBeforeBuild =clock();
#endif
        impl_rebuildView();

#ifdef CHART2_DEBUG_VIEW_PERFORMANCE
        nAfterBuild =clock();
        fBuildTimeInSec =double(nAfterBuild-nBeforeBuild)/double(CLOCKS_PER_SEC);
#endif
        return;
    }

#ifdef CHART2_DEBUG_VIEW_PERFORMANCE
    nBeforePaint=clock();
#endif
    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(pDrawViewWrapper)
        pDrawViewWrapper->CompleteRedraw(pWindow, Region(rRect) );

#ifdef CHART2_DEBUG_VIEW_PERFORMANCE
    nAfterPaint=clock();
    fPaintTimeInSec =double(nAfterPaint-nBeforePaint)/double(CLOCKS_PER_SEC);
#endif
}

bool isDoubleClick( const MouseEvent& rMEvt )
{
    return rMEvt.GetClicks() == 2 && rMEvt.IsLeft() &&
        !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool lcl_isResizeableObject( const rtl::OUString& rClassifiedIdentifier )
{
    ObjectType eObjectType = ObjectIdentifier::getObjectType( rClassifiedIdentifier );
    switch( eObjectType )
    {
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DIAGRAM_WALL:
            return true;
        default:
            return false;
    }
    return false;
}

bool lcl_isRotateableObject( const rtl::OUString& rSelectedObjectCID, const uno::Reference< frame::XModel >& xChartModel )
{
    if( rSelectedObjectCID.indexOf(C2U("Diagram"))==-1 )
        return false;

    sal_Int32 nDimensionCount = ChartTypeHelper::getDimensionCount(
        ChartModelHelper::getFirstChartType(
            ChartModelHelper::findDiagram( xChartModel ) ) );
    if( nDimensionCount == 3 )
        return true;
    return false;
}

class RotateDiagramDragMethod : public SdrDragMethod
{
public:
    RotateDiagramDragMethod( DrawViewWrapper& rDrawViewWrapper );
    virtual ~RotateDiagramDragMethod();
    //,pDrawViewWrapper->GetMarkedObjectList(),E3DDETAIL_ONEBOX,E3DDRAG_CONSTR_XZ

    //TYPEINFO();
    virtual void TakeComment(String& rStr) const;

    virtual FASTBOOL Beg();
    virtual void Mov(const Point& rPnt);
    virtual FASTBOOL End(FASTBOOL bCopy);

    virtual Pointer GetPointer() const;
    virtual void DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const;

    /*
    virtual void Brk();
    virtual void DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const;
    E3dView& Get3DView()  { return (E3dView&)rView;  }

    DECL_LINK(TimerInterruptHdl, void*);


    E3dDragRotate(SdrDragView &rView,
        const SdrMarkList& rMark,
        E3dDragDetail eDetail,
        E3dDragConstraint eConstr = E3DDRAG_CONSTR_XYZ,
        BOOL bFull=FALSE);
    */
    private:
        DrawViewWrapper&    m_rDrawViewWrapper;
        E3dScene*           m_pScene;

        Rectangle           m_aReferenceRect;
        Point               m_aStartPos;

        double              m_fXAngleDegree;
        double              m_fYAngleDegree;
        double              m_fZAngleDegree;
        Matrix4D            m_aParentTransform;

        Matrix4D            m_aCurrentTransform;
        Polygon3D           m_aWireframePoly;
};

RotateDiagramDragMethod::RotateDiagramDragMethod( DrawViewWrapper& rDrawViewWrapper )
    : SdrDragMethod( rDrawViewWrapper )
    , m_rDrawViewWrapper(rDrawViewWrapper)
    , m_pScene(0)
    , m_aReferenceRect(100,100,100,100)
    , m_aStartPos(0,0)
    , m_fXAngleDegree(0.0)
    , m_fYAngleDegree(0.0)
    , m_fZAngleDegree(0.0)
    , m_aParentTransform()
    , m_aCurrentTransform()
    , m_aWireframePoly()
{
    SdrObject* pObj = rDrawViewWrapper.getSelectedObject();
    if(pObj)
    {
        m_aReferenceRect = pObj->GetLogicRect();
        m_aWireframePoly.SetPointCount(0);
        if(pObj->ISA(E3dObject))
        {
            E3dObject* pE3dObject = (E3dObject*)pObj;
            pE3dObject->CreateWireframe(m_aWireframePoly, NULL, E3DDETAIL_DEFAULT ); //E3DDETAIL_ONEBOX, E3DDETAIL_ALLBOXES, E3DDETAIL_ALLLINES
            m_pScene = pE3dObject->GetScene();

            //get pure rotation matrix
            Matrix4D aPureRotateMatrix = m_pScene->GetTransform();
            Matrix4D aTranslateM4Inverse;
            aTranslateM4Inverse.Translate(FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, 0.0);
            aPureRotateMatrix = aPureRotateMatrix*aTranslateM4Inverse;

            //get euler angles from pure rotation matrix
            drawing::Direction3D aAxis; double fAngle;
            Rotation::getRotationAxisAngleFromMatrixRepresentation( aAxis, fAngle, aPureRotateMatrix );
            Rotation::getEulerFromAxisAngleRepresentation( aAxis, fAngle, m_fXAngleDegree, m_fYAngleDegree, m_fZAngleDegree );

            if(pE3dObject->GetParentObj())
            {
                E3dObject* pParent = pE3dObject->GetParentObj();
                m_aReferenceRect = pParent->GetLogicRect();
                m_aParentTransform = pParent->GetTransform();
            }

            Matrix4D aTranslateM4;
            aTranslateM4.Translate(-FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, 0.0);
            m_aParentTransform = aTranslateM4*m_aParentTransform;

            m_aCurrentTransform = aPureRotateMatrix*m_aParentTransform;
        }
    }
}
RotateDiagramDragMethod::~RotateDiagramDragMethod()
{
}
void RotateDiagramDragMethod::TakeComment(String& rStr) const
{
}
FASTBOOL RotateDiagramDragMethod::Beg()
{
    m_aStartPos = DragStat().GetStart();
    Show();
    return true;
}
void RotateDiagramDragMethod::Mov(const Point& rPnt)
{
    if( DragStat().CheckMinMoved(rPnt) )
    {
        //calculate new angle
        double fWAngle = 180.0 * (double)(rPnt.X() - m_aStartPos.X())
            / (double)m_aReferenceRect.GetWidth();
        double fHAngle = 90.0 * (double)(rPnt.Y() - m_aStartPos.Y())
            / (double)m_aReferenceRect.GetHeight();

        //get new transformation matrix from euler angles
        drawing::Direction3D aAxis; double fAngle;
        Rotation::getAxisAngleFromEulerRepresentation( aAxis, fAngle, m_fXAngleDegree+fHAngle, m_fYAngleDegree+fWAngle, m_fZAngleDegree );
        Matrix4D aRotateM4 = Rotation::getRotationMatrixFromAxisAngleRepresentation( aAxis, fAngle );

        //use new matrix
        Hide();
        m_aCurrentTransform = aRotateM4*m_aParentTransform;
        Show();

    }
    DragStat().NextMove(rPnt);
}
FASTBOOL RotateDiagramDragMethod::End(FASTBOOL bCopy)
{

    m_pScene->SetTransform(m_aCurrentTransform);
    //m_pScene->CorrectSceneDimensions();

    Hide();
    return true;
}
Pointer RotateDiagramDragMethod::GetPointer() const
{
    return Pointer();
}
void RotateDiagramDragMethod::DrawXor(ExtOutputDevice& rXOut, FASTBOOL bFull) const
{
    //rXOut.SetOffset(pPV->GetOffset());
    UINT16 nPntCnt = m_aWireframePoly.GetPointCount();
    if(nPntCnt > 1 && m_pScene)
    {
        B3dCamera& rCameraSet = m_pScene->GetCameraSet();
        XPolygon aLine(2);
        for(UINT16 b=0;b < nPntCnt;b += 2)
        {
            Vector3D aPnt1 = m_aCurrentTransform * m_aWireframePoly[b];
            aPnt1 = rCameraSet.WorldToViewCoor(aPnt1);
            aLine[0].X() = (long)(aPnt1.X() + 0.5);
            aLine[0].Y() = (long)(aPnt1.Y() + 0.5);

            Vector3D aPnt2 = m_aCurrentTransform * m_aWireframePoly[b+1];
            aPnt2 = rCameraSet.WorldToViewCoor(aPnt2);
            aLine[1].X() = (long)(aPnt2.X() + 0.5);
            aLine[1].Y() = (long)(aPnt2.Y() + 0.5);

            rXOut.DrawXPolyLine(aLine);
        }
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//------------------------------------------------------------------------

void ChartController::execute_MouseButtonDown( const MouseEvent& rMEvt )
{
    const short HITPIX=2; //hit-tolerance in pixel

    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    Point   aMPos   = pWindow->PixelToLogic(rMEvt.GetPosPixel());

    if( !pDrawViewWrapper )
        return;

    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        pWindow->GrabFocus();
        pWindow->CaptureMouse();
    }

    if( pDrawViewWrapper->IsTextEdit() )
    {
        if( pDrawViewWrapper->IsTextEditHit( aMPos, HITPIX) )
        {
            pDrawViewWrapper->MouseButtonDown(rMEvt,m_pChartWindow);
            return;
        }
        else
        {
            this->EndTextEdit();
            return;
        }
    }

    //abort running action
    if( pDrawViewWrapper->IsAction() )
    {
        if( rMEvt.IsRight() )
            pDrawViewWrapper->BckAction();
        return;
    }

    if( isDoubleClick(rMEvt) ) //do not change selection if double click
        return;//double click is handled further in mousebutton up

    SdrHdl* pHitSelectionHdl = 0;
    //switch from move to resize if handle is hit on a resizeable object
    if( lcl_isResizeableObject( m_aSelectedObjectCID ) )
        pHitSelectionHdl = pDrawViewWrapper->PickHandle( aMPos, *pWindow );
    bool bClickedTwiceOnDragableObject = SelectionHelper::isDragableObjectHitTwice( aMPos, m_aSelectedObjectCID, *pDrawViewWrapper );
    //do not change selection if clicked twice on a dragable object
    //or if selection handles are hit
    if( !pHitSelectionHdl && !bClickedTwiceOnDragableObject )
    {
        SelectionHelper::changeSelection( aMPos, m_aSelectedObjectCID
                                , *pDrawViewWrapper
                                , !rMEvt.IsRight() ); //do not change selection if right clicked on the selected object

        if( !lcl_isRotateableObject( m_aSelectedObjectCID, m_aModel->getModel() ) )
                pDrawViewWrapper->SetDragMode(SDRDRAG_MOVE);
    }
    if( bClickedTwiceOnDragableObject
        || ObjectIdentifier::isDragableObject( m_aSelectedObjectCID ) )
    {
        //start drag
        USHORT  nDrgLog = (USHORT)pWindow->PixelToLogic(Size(DRGPIX,0)).Width();
        SdrDragMethod* pDragMethod = NULL;

        //@todo ... cleanup
        //change selection to 3D scene if 3D object
        if(pDrawViewWrapper->getSelectedObject()->ISA(E3dObject))
        {
            E3dObject* pE3dObject = (E3dObject*)pDrawViewWrapper->getSelectedObject();
            E3dScene* pScene = pE3dObject->GetScene();
            pDrawViewWrapper->UnmarkAll();
            pDrawViewWrapper->MarkObject(pScene);
            pHitSelectionHdl = pDrawViewWrapper->PickHandle( aMPos, *pWindow );//get new handle as selection has changed

            SdrDragMode eDragMode = pDrawViewWrapper->GetDragMode();
            if( SDRDRAG_ROTATE==eDragMode )
                pDragMethod = new RotateDiagramDragMethod( *pDrawViewWrapper );
            else
                pDrawViewWrapper->SetDragMode(eDragMode);
        }
        pDrawViewWrapper->SdrView::BegDragObj(aMPos, NULL, pHitSelectionHdl, nDrgLog, pDragMethod);
    }
}

void ChartController::execute_MouseMove( const MouseEvent& rMEvt )
{
    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    if(!pDrawViewWrapper)
        return;

    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->MouseMove(rMEvt,m_pChartWindow) )
            return;
    }

    if(pDrawViewWrapper->IsAction())
        pDrawViewWrapper->MovAction( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    //??    pDrawViewWrapper->GetPageView()->DragPoly();
    //@todo ForcePointer(&rMEvt);
}
void ChartController::execute_Tracking( const TrackingEvent& rTEvt )
{
}

//-----------------

void ChartController::execute_MouseButtonUp( const MouseEvent& rMEvt )
{
    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    Point   aMPos   = pWindow->PixelToLogic(rMEvt.GetPosPixel());

    //----------------------------

    if(!pDrawViewWrapper)
        return;
    if(pDrawViewWrapper->IsTextEdit())
    {
        if( pDrawViewWrapper->MouseButtonUp(rMEvt,m_pChartWindow) )
            return;
    }

    if(pDrawViewWrapper->IsDragObj())
    {
        if( pDrawViewWrapper->EndDragObj(false) )
        {
            try
            {
                SdrObject* pObj = pDrawViewWrapper->getSelectedObject();
                SdrDragMode eDragMode = pDrawViewWrapper->GetDragMode();

                if( SDRDRAG_ROTATE==eDragMode )
                {
                    if(pObj->ISA(E3dObject))
                    {
                        Matrix4D aSceneMatrix(((E3dObject*)pObj)->GetScene()->GetFullTransform());
                        Matrix4D aTranslateM4Inverse;
                        aTranslateM4Inverse.Translate(FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, 0.0);
                        aSceneMatrix = aSceneMatrix*aTranslateM4Inverse;

                        SceneDescriptor aSceneDescriptor;
                        uno::Reference< beans::XPropertySet > xProp = uno::Reference< beans::XPropertySet >( ChartModelHelper::findDiagram( m_aModel->getModel() ), uno::UNO_QUERY );
                        if( xProp.is() && (xProp->getPropertyValue( C2U( "SceneProperties" ) )>>=aSceneDescriptor) )
                        {
                            Rotation::getRotationAxisAngleFromMatrixRepresentation( aSceneDescriptor.aDirection, aSceneDescriptor.fRotationAngle, aSceneMatrix );
                            xProp->setPropertyValue( C2U( "SceneProperties" ), uno::makeAny(aSceneDescriptor) );
                        }
                    }
                }
                else //end move or size
                {
                    Rectangle aObjectRect = pObj->GetSnapRect();
                    Rectangle aPageRect( Point(0,0),m_pChartWindow->GetOutputSize() );

                    if(pObj->ISA(E3dObject))
                        aObjectRect = ((E3dObject*)pObj)->GetScene()->GetSnapRect();

                    PositionAndSizeHelper::moveObject( m_aSelectedObjectCID
                                    , m_aModel->getModel()
                                    , awt::Rectangle(aObjectRect.getX(),aObjectRect.getY(),aObjectRect.getWidth(),aObjectRect.getHeight())
                                    , awt::Rectangle(aPageRect.getX(),aPageRect.getY(),aPageRect.getWidth(),aPageRect.getHeight())
                                    );
                }
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            //all wanted model changes will take effect
            //and all unwanted view modifications are cleaned
            m_bViewDirty = true;
            m_pChartWindow->Invalidate();
        }
        else //mouse wasn't moved while dragging
        {
            bool bClickedTwiceOnDragableObject = SelectionHelper::isDragableObjectHitTwice( aMPos, m_aSelectedObjectCID, *pDrawViewWrapper );
            bool bIsRotateable = lcl_isRotateableObject( m_aSelectedObjectCID, m_aModel->getModel() );

            //toogle between move and rotate
            SdrDragMode eDragMode = pDrawViewWrapper->GetDragMode();
            if( bIsRotateable && bClickedTwiceOnDragableObject && SDRDRAG_MOVE==eDragMode )
                eDragMode=SDRDRAG_ROTATE;
            else
                eDragMode=SDRDRAG_MOVE;
            pDrawViewWrapper->SetDragMode(eDragMode);

            rtl::OUString aPreviousSelectedObjectCID(m_aSelectedObjectCID);
            //m_aSelectedObjectCID = rtl::OUString();
            SelectionHelper::changeSelection( aMPos, m_aSelectedObjectCID, *pDrawViewWrapper, true );
            if( !aPreviousSelectedObjectCID.equals(m_aSelectedObjectCID) )
                pDrawViewWrapper->SetDragMode(SDRDRAG_MOVE);

            //change selection to 3D scene if 3D object
            SdrObject* pObj = pDrawViewWrapper->getSelectedObject();
            if(pObj && pObj->ISA(E3dObject))
            {
                E3dObject* pE3dObject = (E3dObject*)pObj;
                E3dScene* pScene = pE3dObject->GetScene();
                pDrawViewWrapper->UnmarkAll();
                pDrawViewWrapper->MarkObject(pScene);
            }
        }
    }
    else if( isDoubleClick(rMEvt) )
        execute_DoubleClick();

    //@todo ForcePointer(&rMEvt);
    pWindow->ReleaseMouse();
}

void ChartController::execute_DoubleClick()
{
    ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelectedObjectCID );
    if( OBJECTTYPE_TITLE==eObjectType )
        executeDispatch_EditText();
    else
        executeDispatch_ObjectProperties();
}

void ChartController::execute_Resize()
{
    m_bViewDirty = true;
    m_pChartWindow->Invalidate();
}
void ChartController::execute_Activate()
{
    ///// pDrawViewWrapper->SetEditMode(TRUE);
}
void ChartController::execute_Deactivate()
{
    /*
    pDrawViewWrapper->SetEditMode(FALSE);
    this->ReleaseMouse();
    */
}
void ChartController::execute_GetFocus()
{
}
void ChartController::execute_LoseFocus()
{
    //this->ReleaseMouse();
}
util::URL createDispatchURL( USHORT nSID )
{
    util::URL aRet;
    aRet.Protocol = C2U("slot:");
    aRet.Path = rtl::OUString::valueOf((sal_Int32)nSID);
    return aRet;
}
void ChartController::execute_Command( const CommandEvent& rCEvt )
{
    // pop-up menu
    if (rCEvt.GetCommand() == COMMAND_CONTEXTMENU && m_pDrawViewWrapper && !m_pDrawViewWrapper->IsAction())
    {
        m_pChartWindow->ReleaseMouse();

        BOOL bAxisChart = true;//@todo get from somewhere
        BOOL bIs3DChart = false;//@todo get from somewhere
        BOOL bHasOwnData = false;
        BOOL bHasSomethingToPaste = true;

        USHORT nRId = 0;
        if( !m_aSelectedObjectCID.getLength() )
            nRId = RID_FORMAT_POPUP;
        else
            nRId = RID_OBJECT_POPUP;

        PopupMenu aMenu=PopupMenu ( SchResId( nRId ) );
        if(!bIs3DChart)
        {
            aMenu.RemoveItem( aMenu.GetItemPos( SID_DIAGRAM_FLOOR ) );
            aMenu.RemoveItem( aMenu.GetItemPos( SID_3D_WIN ) );
        }
        if(!bAxisChart)
        {
            aMenu.RemoveItem( aMenu.GetItemPos( SID_INSERT_AXIS ) );
            aMenu.RemoveItem( aMenu.GetItemPos( SID_INSERT_GRIDS ) );
            aMenu.RemoveItem( aMenu.GetItemPos( SID_DIAGRAM_WALL ) );
            aMenu.RemoveItem( aMenu.GetItemPos( SID_DIAGRAM_FLOOR ) );
        }
        if(!bHasOwnData)
        {
            aMenu.RemoveItem( aMenu.GetItemPos( SID_DIAGRAM_DATA ) );
        }
        if( !bHasSomethingToPaste )
        {
            aMenu.RemoveItem( aMenu.GetItemPos( SID_PASTE ) );
        }

        USHORT nId = aMenu.Execute( m_pChartWindow, rCEvt.GetMousePosPixel() );
        util::URL aURL( createDispatchURL( nId ) );
        uno::Sequence< beans::PropertyValue > aArgs;
        this->dispatch( aURL, aArgs );
    }
}

bool ChartController::execute_KeyInput( const KeyEvent& rKEvt )
{
    bool bReturn=false;
    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->KeyInput(rKEvt,m_pChartWindow) )
        {
            bReturn = true;
            if( rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE )
            {
                this->EndTextEdit();
            }
        }

    }

    //if( m_pDrawViewWrapper->IsAction() );

    /* old chart:
    if (pFuActual)
        bReturn = pFuActual->KeyInput(rKEvt);

    if (!bReturn && GetWindow())
    {
        KeyCode aKeyCode = rKEvt.GetKeyCode();

        if (aKeyCode.IsMod1() && aKeyCode.IsShift()
            && aKeyCode.GetCode() == KEY_R)
        {
                // 3D-Kontext wieder zerstoeren
            Base3D* pBase3D = (Base3D*) GetWindow()->Get3DContext();

            if (pBase3D)
            {
                pBase3D->Destroy(GetWindow());
            }
            GetWindow()->Invalidate();
            bReturn = TRUE;
        }
    }
    */
    return bReturn;
}
//-----------------------------------------------------------------
// XSelectionSupplier (optional interface)
//-----------------------------------------------------------------
        sal_Bool SAL_CALL ChartController
::select( const uno::Any& rSelection )
        throw( lang::IllegalArgumentException )
{
    //@todo
    return sal_False;
}

        uno::Any SAL_CALL ChartController
::getSelection() throw(uno::RuntimeException)
{
    //@todo
    return uno::Any();
}

        void SAL_CALL ChartController
::addSelectionChangeListener( const uno::Reference<
        view::XSelectionChangeListener > & xListener )
        throw(uno::RuntimeException)
{
    //@todo
}

        void SAL_CALL ChartController
::removeSelectionChangeListener( const uno::Reference<
        view::XSelectionChangeListener > & xListener )
        throw(uno::RuntimeException)
{
    //@todo
}

//.............................................................................
} //namespace chart
//.............................................................................
