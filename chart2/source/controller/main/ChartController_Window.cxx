/*************************************************************************
 *
 *  $RCSfile: ChartController_Window.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: iha $ $Date: 2003-10-28 18:03:29 $
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
#include "chartview/ObjectIdentifier.hxx"
#include "ChartWindow.hxx"
#include "chartview/ChartView.hxx"
#include "Chart.hrc"
#include "ResId.hxx"

// header for class PopupMenu
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#include "macros.hxx"
#include "SchSlotIds.hxx"
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#define DRGPIX    2     // Drag MinMove in Pixel

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
//using namespace ::drafts::com::sun::star::chart2;

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
    //Window::Paint(rRect);
    //??? @todo initialize doc if not ready or update view state if not ready and invalidate all

    if(m_bViewDirty)
    {
        impl_rebuildView();
        return;
    }

    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    pDrawViewWrapper->InitRedraw(pWindow, Region(rRect) );
}

bool isDoubleClick( const MouseEvent& rMEvt )
{
    return rMEvt.GetClicks() == 2 && rMEvt.IsLeft() &&
        !rMEvt.IsMod1() && !rMEvt.IsMod2() && !rMEvt.IsShift();
}

void ChartController::execute_MouseButtonDown( const MouseEvent& rMEvt )
{
    const short HITPIX=2; //hit-tolerance in pixel

    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;
    Point   aMPos   = pWindow->PixelToLogic(rMEvt.GetPosPixel());

    if ( MOUSE_LEFT == rMEvt.GetButtons() )
    {
        pWindow->GrabFocus();
        pWindow->CaptureMouse();
    }

    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->IsTextEditHit( aMPos, HITPIX) )
        {
            m_pDrawViewWrapper->MouseButtonDown(rMEvt,m_pChartWindow);
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

    //selection
    if( !isDoubleClick(rMEvt) ) //do not change selection if double click
    {
        pDrawViewWrapper->UnmarkAll();
        SdrObject* pNewObj = SelectionHelper::getObjectToSelect(
                                    aMPos, m_aSelectedObjectCID
                                    , *pDrawViewWrapper
                                    , !rMEvt.IsRight() ); //do not change selection if right clicked on the selected object
        if(!pNewObj)
            return;
        SelectionHelper aSelectionHelper( pNewObj, m_aSelectedObjectCID );
        SdrObject* pMarkObj = aSelectionHelper.getObjectToMark();
        pDrawViewWrapper->setMarkHandleProvider(&aSelectionHelper);
        pDrawViewWrapper->MarkObject(pMarkObj);
        pDrawViewWrapper->setMarkHandleProvider(NULL);
    }

    //dragging
    if(false)
    {
        SdrDragMode eDragMode(SDRDRAG_MOVE);//SDRDRAG_CREATE,SDRDRAG_MOVE,SDRDRAG_RESIZE,SDRDRAG_ROTATE,SDRDRAG_MIRROR,SDRDRAG_SHEAR,SDRDRAG_CROOK

        pDrawViewWrapper->SetDragMode(eDragMode);
        SdrHdl* pHdl = NULL;//pDrawViewWrapper->PickHandle( aMPos, *pWindow );
        USHORT  nDrgLog = (USHORT)pWindow->PixelToLogic(Size(DRGPIX,0)).Width();
        pDrawViewWrapper->BegDragObj(aMPos, NULL, pHdl, nDrgLog);
    }

    //pDrawViewWrapper->UnmarkAll();
    //pDrawViewWrapper->GetPageViewPvNum(0)->GetEnteredLevel()
    //pDrawViewWrapper->IsMarkedHit(aMPos, nHitLog)
    //pDrawViewWrapper->HasMarkedObj();
    //pDrawViewWrapper->MarkLogicalGroup();
    //pDrawViewWrapper->EnterMarkedGroup();
    //pDrawViewWrapper->LeaveOneGroup();

    //pDrawViewWrapper->MovAction( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
    //pDrawViewWrapper->EndAction();

    //pDrawViewWrapper->IsDragObj();
    //pDrawViewWrapper->EndDragObj(rMEvt.IsMod1());
    //pDrawViewWrapper->SetDragWithCopy( rMEvt.IsMod1());
    //pDrawViewWrapper->GetDragMode()
    //pDrawViewWrapper->SetEditMode(FALSE);

    //pDrawViewWrapper->GetPageView()->DragPoly() ;

    //pView->BeginDrag(pWindow, aMPos);


    //if(!pFunction) //if no Fu active/set ...
    //  Window::MouseButtonDown(rMEvt);
}

// Timer-Handler fuer Drag&Drop
/*
IMPL_LINK( ChartWindow, DragHdl, Timer *, pTimer )
{
    *//*
    USHORT nHitLog = (USHORT)pWindow->PixelToLogic(Size(HITPIX,0)).Width();
    SdrHdl* pHdl = pView->HitHandle(aMPos, *pWindow);

    if (pHdl==NULL && pView->IsMarkedHit(aMPos, nHitLog))
    {
        pWindow->ReleaseMouse();
        m_bIsInDragMode = TRUE;

        pView->BeginDrag(pWindow, aMPos);
    }
    *//*
    this->ReleaseMouse();
    return 0;
}
*/

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

    //ForcePointer(&rMEvt); should be sufficient and correct to do it at the end
    //??? select Fu ( SchFuDraw SchFuText SchFuSelection ) and call MouseMove there

/*
    if(m_aDragTimer.IsActive())
    {
        m_aDragTimer.Stop();
        m_bIsInDragMode = FALSE;
    }
*/

    if(pDrawViewWrapper->IsAction())
        pDrawViewWrapper->MovAction( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );

    //text:    pView->MouseMove(rMEvt, pWindow) // Event von der SdrView ausgewertet
    //??    pDrawViewWrapper->GetPageView()->DragPoly();

    //if(!pFunction) //if no Fu active/set ...
    //  Window::MouseMove(rMEvt);

    //@todo ForcePointer(&rMEvt);
}
void ChartController::execute_Tracking( const TrackingEvent& rTEvt )
{
}
void ChartController::execute_MouseButtonUp( const MouseEvent& rMEvt )
{
    Window* pWindow = m_pChartWindow;
    DrawViewWrapper* pDrawViewWrapper = m_pDrawViewWrapper;

    //??? select Fu ( SchFuDraw SchFuText SchFuSelection ) and call MouseButtonUp there
    //----------------------------
    /*
    uno::Reference< uno::XInterface > xI;
    uno::Reference< drawing::XShape > xShape;
    uno::Reference< beans::XPropertySet > xShapeProps;

    if(s_pObj)
    {
        xI = s_pObj->getUnoShape();
        xShape =  uno::Reference<drawing::XShape>( xI, uno::UNO_QUERY );
        xShapeProps = uno::Reference<beans::XPropertySet>( xI, uno::UNO_QUERY );
    }

    awt::Point aOldPoint;
    awt::Point aNewPoint;
    awt::Size aOldSize;
    awt::Size aNewSize;
    drawing::HomogenMatrix3 aOldMatrix;
    drawing::HomogenMatrix3 aNewMatrix;

    if(xShape.is())
    {
        aOldPoint = xShape->getPosition();
        aOldSize =  xShape->getSize();
    }
    if(xShapeProps.is())
    {
        uno::Any aVal = xShapeProps->getPropertyValue(
            ::rtl::OUString::createFromAscii("Transformation") );
        aVal >>= aOldMatrix;
    }
    */
    //----------------------------

    if(!pDrawViewWrapper)
        return;
    if( m_pDrawViewWrapper->IsTextEdit() )
    {
        if( m_pDrawViewWrapper->MouseButtonUp(rMEvt,m_pChartWindow) )
            return;
    }
    if(pDrawViewWrapper->IsDragObj())
    {
        if( pDrawViewWrapper->EndDragObj(rMEvt.IsMod1()) )
        {

            /*
            if(xShape.is())
            {
                aNewPoint = xShape->getPosition();
                aNewSize =  xShape->getSize();
            }
            if(xShapeProps.is())
            {
                uno::Any aVal = xShapeProps->getPropertyValue(
                    ::rtl::OUString::createFromAscii("Transformation") );
                aVal >>= aNewMatrix;
            }
            */

            pDrawViewWrapper->SetDragWithCopy( rMEvt.IsMod1());
        }
    }

    pWindow->ReleaseMouse();

    //if(pDrawViewWrapper->IsAction())
    //  pDrawViewWrapper->EndAction();

    /* SchFuText:
    SfxBindings& rBindings = pViewShell->GetViewFrame()->GetBindings();

    rBindings.Invalidate(SID_CUT);
    rBindings.Invalidate(SID_COPY);

    if (pView->MouseButtonUp(rMEvt, pWindow))
        return TRUE; // Event von der SdrView ausgewertet

    */
    if( isDoubleClick(rMEvt) )
        execute_DoubleClick();

    //if(!pFunction) //if no Fu active/set ...
    //  Window::MouseButtonUp(rMEvt);

    //@todo ForcePointer(&rMEvt);
    //pWindow->ReleaseMouse();
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
    m_aDragTimer.Stop();

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
    if (rCEvt.GetCommand() == COMMAND_CONTEXTMENU && !m_pDrawViewWrapper->IsAction())
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
