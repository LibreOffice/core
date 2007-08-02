/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedfunc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:41:30 $
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
#include "precompiled_reportdesign.hxx"

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#include <svx/svdpagv.hxx>
#include <svx/outlobj.hxx>
#include <vcl/svapp.hxx>
#include <svx/unolingu.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <svx/svdetc.hxx>
#include <svx/editstat.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svddrgmt.hxx>
#ifndef _SVDOASHP_HXX
#include <svx/svdoashp.hxx>
#endif


#ifndef _REPORT_RPTUIFUNC_HXX
#include "dlgedfunc.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef RPTUI_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef RPTUI_VIEWSWINDOW_HXX
#include "ViewsWindow.hxx"
#endif
#ifndef RPTUI_REPORT_WINDOW_HXX
#include "ReportWindow.hxx"
#endif
#include "RptObject.hxx"
#ifndef DBAUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef _SV_SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif
#ifndef RPTUI_RULER_HXX
#include "ReportRuler.hxx"
#endif
#include "UITools.hxx"
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

#include <com/sun/star/beans/XPropertySet.hpp>
#include <uistrings.hrc>
#include "UndoEnv.hxx"
#include <RptModel.hxx>

#define DEFAUL_MOVE_SIZE    100
namespace rptui
{
using namespace ::com::sun::star;
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

IMPL_LINK( DlgEdFunc, ScrollTimeout, Timer *,  )
{
    ForceScroll( pParent->PixelToLogic( pParent->GetPointerPosPixel() ) );
    return 0;
}

//----------------------------------------------------------------------------

void DlgEdFunc::ForceScroll( const Point& rPos )
{
    aScrollTimer.Stop();

    OReportWindow* pReportWindow = pParent->getViewsWindow()->getView();
    OScrollWindowHelper* pScrollWindow = pReportWindow->getScrollWindow();

    Size aOut = pReportWindow->GetOutputSizePixel();
    aOut.Width() -= REPORT_STARTMARKER_WIDTH;

    Rectangle aOutRect( pScrollWindow->getScrollOffset(), aOut );
    aOutRect = pParent->PixelToLogic( aOutRect );
    //Rectangle aWorkArea = pParent->getView()->GetWorkArea();
    Rectangle aWorkArea(Point(),pScrollWindow->getTotalSize());
    aWorkArea.Right() -= REPORT_STARTMARKER_WIDTH;
    aWorkArea = pScrollWindow->PixelToLogic( aWorkArea );
    if( !aOutRect.IsInside( rPos ) && aWorkArea.IsInside( rPos ) )
    {
        ScrollBar* pHScroll = pScrollWindow->GetHScroll();
        ScrollBar* pVScroll = pScrollWindow->GetVScroll();
        long nDeltaX = pHScroll->GetLineSize();
        long nDeltaY = pVScroll->GetLineSize();

        if( rPos.X() < aOutRect.Left() )
            nDeltaX = -nDeltaX;
        else if( rPos.X() <= aOutRect.Right() )
            nDeltaX = 0;

        if( rPos.Y() < aOutRect.Top() )
            nDeltaY = -nDeltaY;
        else if( rPos.Y() <= aOutRect.Bottom() )
            nDeltaY = 0;

        pScrollWindow->EndScroll(nDeltaX,nDeltaY);
    }

    aScrollTimer.Start();
}

//----------------------------------------------------------------------------

DlgEdFunc::DlgEdFunc( OReportSection* _pParent )
:pParent(_pParent),
 m_xOverlappingObj(NULL),
 m_pOverlappingObj(NULL),
 m_bSelectionMode(false)
{
    // pParent = _pParent;
    aScrollTimer.SetTimeoutHdl( LINK( this, DlgEdFunc, ScrollTimeout ) );
    aScrollTimer.SetTimeout( SELENG_AUTOREPEAT_INTERVAL );
}

//----------------------------------------------------------------------------
    void DlgEdFunc::setOverlappedControlColor(sal_Int32 _nColor)
    {
        m_nOverlappedControlColor = _nColor;
    }

// -----------------------------------------------------------------------------
sal_Int32 lcl_setColorOfObject(uno::Reference< uno::XInterface > _xObj, long _nColorTRGB)
{
    sal_Int32 nBackColor = 0;
    try
    {
        uno::Reference<report::XReportComponent> xComponent(_xObj, uno::UNO_QUERY_THROW);
        // NOT NEED if UNO_QUERY_THROW:
        // if (xComponent.is())
        // {
        uno::Reference< beans::XPropertySet > xProp(xComponent, uno::UNO_QUERY_THROW);
        // if (xProp.is())
        // {
        // ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlBackground"))
        // is PROPERTY_CONTROLBACKGROUND
        uno::Any aAny = xProp->getPropertyValue(PROPERTY_CONTROLBACKGROUND);
        if (aAny.hasValue())
        {
            aAny >>= nBackColor;
            // try to set background color at the ReportComponent
            uno::Any aBlackColorAny = uno::makeAny(_nColorTRGB);
            xProp->setPropertyValue(PROPERTY_CONTROLBACKGROUND, aBlackColorAny);
        }
        // aCopies.push_back(xComponent->createClone());
    }
    catch(uno::Exception&)
    {
        // bit my shiny metal as!
        // OSL_ENSURE(0,"Can't copy report elements!");
    }
    return nBackColor;
}
// -----------------------------------------------------------------------------
DlgEdFunc::~DlgEdFunc()
{
    unColorizeOverlappedObj();
    aScrollTimer.Stop();
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_aMDPos = pParent->PixelToLogic( rMEvt.GetPosPixel() );
    pParent->GrabFocus();
    BOOL bHandled = FALSE;
    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 2 )
    {
        // show property browser
        if ( pParent->GetMode() != RPTUI_READONLY )
        {
            uno::Sequence<beans::PropertyValue> aArgs(1);
            aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowProperties"));
            aArgs[0].Value <<= sal_True;
            pParent->getViewsWindow()->getView()->getReportView()->getController()->executeUnChecked(SID_SHOW_PROPERTYBROWSER,aArgs);
            pParent->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(pParent->getView());
            bHandled = TRUE;
        }
    }
    else if ( rMEvt.IsRight() && !rMEvt.IsLeft() && rMEvt.GetClicks() == 1 ) // mark object when context menu was selected
    {
        OSectionView* pView = pParent->getView();
        SdrPageView* pPV = pView->GetSdrPageView();
        SdrViewEvent aVEvt;
        if ( pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt) != SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() )
            pParent->getViewsWindow()->unmarkAllObjects(NULL);
        if ( aVEvt.pRootObj )
            pView->MarkObj(aVEvt.pRootObj, pPV);
        else
            pParent->getViewsWindow()->unmarkAllObjects(NULL);

        bHandled = TRUE;
    }
    return bHandled;
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::MouseButtonUp( const MouseEvent& /*rMEvt*/ )
{
    unColorizeOverlappedObj();
    aScrollTimer.Stop();
    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFunc::MouseMove( const MouseEvent& /*rMEvt*/ )
{
    return TRUE;
}
//------------------------------------------------------------------------------
sal_Bool DlgEdFunc::handleKeyEvent(const KeyEvent& _rEvent)
{
    BOOL bReturn = FALSE;

    OSectionView* pView = pParent->getView();

    const KeyCode& rCode = _rEvent.GetKeyCode();
    USHORT nCode = rCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
        {
            if ( pView->IsAction() )
            {
                pView->BrkAction();
                bReturn = TRUE;
            }
            else if ( pView->AreObjectsMarked() )
            {
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();
                if ( pHdl )
                    ((SdrHdlList&)rHdlList).ResetFocusHdl();
                else
                    pView->UnmarkAll();

                bReturn = FALSE;
            }
        }
        break;
        case KEY_TAB:
        {
            if ( !rCode.IsMod1() && !rCode.IsMod2() )
            {
                // mark next object
                if ( !pView->MarkNextObj( !rCode.IsShift() ) )
                {
                    // if no next object, mark first/last
                    pView->UnmarkAllObj();
                    pView->MarkNextObj( !rCode.IsShift() );
                }

                if ( pView->AreObjectsMarked() )
                    pView->MakeVisible( pView->GetAllMarkedRect(), *pParent);

                bReturn = TRUE;
            }
            else if ( rCode.IsMod1() )
            {
                // selected handle
                const SdrHdlList& rHdlList = pView->GetHdlList();
                ((SdrHdlList&)rHdlList).TravelFocusHdl( !rCode.IsShift() );

                // guarantee visibility of focused handle
                SdrHdl* pHdl = rHdlList.GetFocusHdl();
                if ( pHdl )
                {
                    Point aHdlPosition( pHdl->GetPos() );
                    Rectangle aVisRect( aHdlPosition - Point( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE ), Size( 200, 200 ) );
                    pView->MakeVisible( aVisRect, *pParent);
                }

                bReturn = TRUE;
            }
        }
        break;
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            long nX = 0;
            long nY = 0;

            if ( nCode == KEY_UP )
                nY = -1;
            else if ( nCode == KEY_DOWN )
                nY =  1;
            else if ( nCode == KEY_LEFT )
                nX = -1;
            else if ( nCode == KEY_RIGHT )
                nX =  1;

            if ( pView->AreObjectsMarked() && !rCode.IsMod1() )
            {
                if ( rCode.IsMod2() )
                {
                    // move in 1 pixel distance
                    const Size aPixelSize = pParent? pParent->PixelToLogic( Size( 1, 1 ) ) : Size( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE );
                    nX *= aPixelSize.Width();
                    nY *= aPixelSize.Height();
                }
                else
                {
                    // move in 1 mm distance
                    nX *= DEFAUL_MOVE_SIZE;
                    nY *= DEFAUL_MOVE_SIZE;
                }

                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if ( pHdl == 0 )
                {
                    // no handle selected
                    if ( pView->IsMoveAllowed() )
                    {
                        // restrict movement to work area
                        const Rectangle& rWorkArea = pView->GetWorkArea();

                        if ( !rWorkArea.IsEmpty() )
                        {
                            Rectangle aMarkRect( pView->GetMarkedObjRect() );
                            aMarkRect.Move( nX, nY );

                            if ( !rWorkArea.IsInside( aMarkRect ) )
                            {
                                if ( aMarkRect.Left() < rWorkArea.Left() )
                                    nX += rWorkArea.Left() - aMarkRect.Left();

                                if ( aMarkRect.Right() > rWorkArea.Right() )
                                    nX -= aMarkRect.Right() - rWorkArea.Right();

                                if ( aMarkRect.Top() < rWorkArea.Top() )
                                    nY += rWorkArea.Top() - aMarkRect.Top();

                                if ( aMarkRect.Bottom() > rWorkArea.Bottom() )
                                    nY -= aMarkRect.Bottom() - rWorkArea.Bottom();
                            }
                            bool bCheck = false;
                            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                            for (sal_uInt32 i =  0; !bCheck && i < rMarkList.GetMarkCount();++i )
                            {
                                SdrMark* pMark = rMarkList.GetMark(i);
                                bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL;
                            }

                            if ( bCheck && isOver(aMarkRect,*pParent->getPage(),*pView) )
                                break;
                        }

                        if ( nX != 0 || nY != 0 )
                        {
                            pView->MoveAllMarked( Size( nX, nY ) );
                            pView->MakeVisible( pView->GetAllMarkedRect(), *pParent);
                        }
                    }
                }
                else
                {
                    // move the handle
                    if ( pHdl && ( nX || nY ) )
                    {
                        const Point aStartPoint( pHdl->GetPos() );
                        const Point aEndPoint( pHdl->GetPos() + Point( nX, nY ) );
                        const SdrDragStat& rDragStat = pView->GetDragStat();

                        // start dragging
                        pView->BegDragObj( aStartPoint, 0, pHdl, 0 );

                        if ( pView->IsDragObj() )
                        {
                            const FASTBOOL bWasNoSnap = rDragStat.IsNoSnap();
                            const BOOL bWasSnapEnabled = pView->IsSnapEnabled();

                            // switch snapping off
                            if ( !bWasNoSnap )
                                ((SdrDragStat&)rDragStat).SetNoSnap( TRUE );
                            if ( bWasSnapEnabled )
                                pView->SetSnapEnabled( FALSE );

                            Rectangle aNewRect;
                            bool bCheck = false;
                            const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
                            for (sal_uInt32 i =  0; !bCheck && i < rMarkList.GetMarkCount();++i )
                            {
                                SdrMark* pMark = rMarkList.GetMark(i);
                                bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL;
                                if ( bCheck )
                                    aNewRect.Union(pMark->GetMarkedSdrObj()->GetLastBoundRect());
                            }

                            switch(pHdl->GetKind())
                            {
                                case HDL_LEFT:
                                case HDL_UPLFT:
                                case HDL_LWLFT:
                                case HDL_UPPER:
                                    aNewRect.Left() += nX;
                                    aNewRect.Top()  += nY;
                                    break;
                                case HDL_UPRGT:
                                case HDL_RIGHT:
                                case HDL_LWRGT:
                                case HDL_LOWER:
                                    aNewRect.setWidth(aNewRect.getWidth() + nX);
                                    aNewRect.setHeight(aNewRect.getHeight() + nY);
                                    break;
                                default:
                                    break;
                            }
                            if ( !(bCheck && isOver(aNewRect,*pParent->getPage(),*pView)) )
                                pView->MovAction( aEndPoint );
                            pView->EndDragObj();

                            // restore snap
                            if ( !bWasNoSnap )
                                ((SdrDragStat&)rDragStat).SetNoSnap( bWasNoSnap );
                            if ( bWasSnapEnabled )
                                pView->SetSnapEnabled( bWasSnapEnabled );
                        }

                        // make moved handle visible
                        const Rectangle aVisRect( aEndPoint - Point( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE ), Size( 200, 200 ) );
                        pView->MakeVisible( aVisRect, *pParent);
                    }
                }
                pView->AdjustMarkHdl();
            }
            else
            {
                // scroll page
                OScrollWindowHelper* pScrollWindow = pParent->getViewsWindow()->getView()->getScrollWindow();
                ScrollBar* pScrollBar = ( nX != 0 ) ? pScrollWindow->GetHScroll() : pScrollWindow->GetVScroll();
                if ( pScrollBar && pScrollBar->IsVisible() )
                    pScrollBar->DoScrollAction(( nX < 0 || nY < 0 ) ? SCROLL_LINEUP : SCROLL_LINEDOWN );
            }

            bReturn = TRUE;
        }
        break;
        default:
        {
            bReturn = pView->KeyInput(_rEvent, pParent);
        }
        break;
    }

    if ( bReturn )
        pParent->ReleaseMouse();

    return bReturn;
}
// -----------------------------------------------------------------------------
void DlgEdFunc::colorizeOverlappedObject(SdrObject* _pOverlappedObj)
{
    OObjectBase* pObj = dynamic_cast<OObjectBase*>(_pOverlappedObj);
    if ( pObj )
    {
        uno::Reference<report::XReportComponent> xComponent = pObj->getReportComponent();
        if (xComponent.is() && xComponent != m_xOverlappingObj)
        {
            OReportModel* pRptModel = static_cast<OReportModel*>(_pOverlappedObj->GetModel());
            if ( pRptModel )
            {
                OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());

                // uncolorize an old object, if there is one
                unColorizeOverlappedObj();

                m_nOldColor = lcl_setColorOfObject(xComponent, m_nOverlappedControlColor);
                m_xOverlappingObj = xComponent;
                m_pOverlappingObj = _pOverlappedObj;
            }
        }
    }
}
// -----------------------------------------------------------------------------
void DlgEdFunc::unColorizeOverlappedObj()
{
    // uncolorize an old object, if there is one
    if (m_xOverlappingObj.is())
    {
        OReportModel* pRptModel = static_cast<OReportModel*>(m_pOverlappingObj->GetModel());
        if ( pRptModel )
        {
            OXUndoEnvironment::OUndoEnvLock aLock(pRptModel->GetUndoEnv());

            lcl_setColorOfObject(m_xOverlappingObj, m_nOldColor);
            m_xOverlappingObj = NULL;
            m_pOverlappingObj = NULL;
        }
    }
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::isOverlapping(const MouseEvent& rMEvt)
{
    bool bOverlapping = false;
    OSectionView* pView  = pParent->getView();
    SdrViewEvent aVEvt;
    bOverlapping = pView->PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt) != SDRHIT_NONE;
    if (bOverlapping && aVEvt.pObj)
    {
        colorizeOverlappedObject(aVEvt.pObj);
    }
    else
    {
        unColorizeOverlappedObj();
    }

    return bOverlapping;
}
// -----------------------------------------------------------------------------
void DlgEdFunc::checkMovementAllowed(const MouseEvent& rMEvt)
{
    OSectionView* pView  = pParent->getView();
    if ( pView->IsDragObj() )
    {
        if ( isRectangleHit(rMEvt) )
            pView->BrkAction();
        // object was dragged
        pView->EndDragObj( FALSE /* rMEvt.IsMod1() */   ); // Copy by CTRL-Key not allowed at the moment.
                                                           // we create a wrong object here, with no content
                                                           // which results in a wrong report, which can't execute
                                                           // therefore no copy. LLA: 20070710
        pView->ForceMarkedToAnotherPage();
        pParent->Invalidate();
    }
    else if ( pView->IsAction() )
        pView->EndAction();
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::isOnlyCustomShapeMarked()
{
    OSectionView* pView  = pParent->getView();
    bool bReturn = true;
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    for (sal_uInt32 i =  0; i < rMarkList.GetMarkCount();++i )
    {
        SdrMark* pMark = rMarkList.GetMark(i);
        // bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL;
        SdrObject* pObj = pMark->GetMarkedSdrObj();
        if (pObj->GetObjIdentifier() != OBJ_CUSTOMSHAPE)
        {
            // we found an object in the marked objects, which is not a custom shape.
            bReturn = false;
            break;
        }
    }
    return bReturn;
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::isRectangleHit(const MouseEvent& rMEvt)
{
    if (isOnlyCustomShapeMarked())
    {
        return false;
    }
    OSectionView* pView  = pParent->getView();

    SdrViewEvent aVEvt;
    const SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEMOVE, aVEvt);
    bool bIsSetPoint = (eHit == SDRHIT_UNMARKEDOBJECT);
    if ( !bIsSetPoint )
    {
        // no drag rect, we have to check every single select rect
        //const Rectangle& rRect = pView->GetDragStat().GetActionRect();
        const SdrDragStat& rDragStat = pView->GetDragStat();
        if (rDragStat.GetDragMethod() != NULL)
        {
            SdrObjListIter aIter(*pParent->getPage(),IM_DEEPNOGROUPS);
            SdrObject* pObjIter = NULL;
            // loop through all marked objects and check if there new rect overlapps an old one.
            while( (pObjIter = aIter.Next()) != NULL && !bIsSetPoint)
            {
                if ( pView->IsObjMarked(pObjIter)
                     && dynamic_cast<OUnoObject*>(pObjIter) != NULL )
                {
                    Rectangle aNewRect = pObjIter->GetLastBoundRect();
                    long nDx = rDragStat.IsHorFixed() ? 0 : rDragStat.GetDX();
                    long nDy = rDragStat.IsVerFixed() ? 0 : rDragStat.GetDY();
                    if ( (nDx + aNewRect.Left()) < 0 )
                        nDx = -aNewRect.Left();
                    if ( (nDy + aNewRect.Top()) < 0 )
                        nDy = -aNewRect.Top();

                    Point aTest;
                    rDragStat.GetDragMethod()->MovPoint(aTest);
                    if ( rDragStat.GetDragMethod()->IsMoveOnly() )
                        aNewRect.Move(nDx,nDy);
                    else
                        ::ResizeRect(aNewRect,rDragStat.GetRef1(),rDragStat.GetXFact(),rDragStat.GetYFact());


                    SdrObject* pObjOverlapped = isOver(aNewRect,*pParent->getPage(),*pView,false,pObjIter);
                    bIsSetPoint = pObjOverlapped ? true : false;
                    if (pObjOverlapped && !m_bSelectionMode)
                    {
                        colorizeOverlappedObject(pObjOverlapped);
                }
            }
        }
    }
    }
    else if ( aVEvt.pObj && !m_bSelectionMode)
    {
        colorizeOverlappedObject(aVEvt.pObj);
    }
    return bIsSetPoint;
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::setMovementPointer(const MouseEvent& rMEvt)
{
    bool bIsSetPoint = isRectangleHit(rMEvt);
    if ( bIsSetPoint )
        pParent->SetPointer( Pointer(POINTER_NOTALLOWED));

    return bIsSetPoint;
}
//----------------------------------------------------------------------------

DlgEdFuncInsert::DlgEdFuncInsert( OReportSection* _pParent ) :
    DlgEdFunc( _pParent )
{
    pParent->getView()->SetCreateMode( TRUE );
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::~DlgEdFuncInsert()
{
    pParent->getView()->SetEditMode( TRUE );
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return TRUE;
    if( !rMEvt.IsLeft() )
        return TRUE;

    OSectionView* pView  = pParent->getView();
    pView->SetActualWin( pParent);

    const USHORT nHitLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );
    const USHORT nDrgLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );

#ifdef MAC
    pParent->GrabFocus();
#endif
    pParent->CaptureMouse();

    SdrViewEvent aVEvt;
    const SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    if ( rMEvt.IsLeft() && rMEvt.GetClicks() == 1 )
    {
        SdrHdl* pHdl = pView->PickHandle(m_aMDPos);

        pParent->getViewsWindow()->unmarkAllObjects(pView);

        // if selected object was hit, drag object
        if ( pHdl!=NULL || pView->IsMarkedHit(m_aMDPos, nHitLog) )
            pView->BegDragObj(m_aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);

        if( eHit != SDRHIT_UNMARKEDOBJECT )
        {
            // if no action, create object
            if ( !pView->IsAction() )
            {
                if ( pView->AreObjectsMarked() )
                    pParent->getViewsWindow()->unmarkAllObjects(pView);
                pView->BegCreateObj(m_aMDPos);
                pParent->getViewsWindow()->createDefault();
            }
        }
        else
        {
            if( !rMEvt.IsShift() )
                pView->UnmarkAll();

            pView->BegMarkObj( m_aMDPos );
        }
    }

    return TRUE;
}

//----------------------------------------------------------------------------
BOOL DlgEdFuncInsert::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    const Point aPos( pParent->PixelToLogic( rMEvt.GetPosPixel() ) );

    OSectionView* pView  = pParent->getView();
    pView->SetActualWin( pParent );

    const USHORT nHitLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );

    pParent->ReleaseMouse();

    BOOL bReturn = TRUE;
    // object creation active?
    if ( pView->IsCreateObj() )
    {
        if ( isOver(pView->GetCreateObj(),*pParent->getPage(),*pView) )
        {
            pView->BrkAction();
            // BrkAction disables the create mode
            pView->SetCreateMode( TRUE );
            return TRUE;
        }

        pView->EndCreateObj(SDRCREATE_FORCEEND);

        if ( !pView->AreObjectsMarked() )
        {
            pView->MarkObj(aPos, nHitLog);
        }

        bReturn = pView->AreObjectsMarked();
    }
    else
        checkMovementAllowed(rMEvt);

    if ( pView && !pView->AreObjectsMarked() &&
         Abs(m_aMDPos.X() - aPos.X()) < nHitLog &&
         Abs(m_aMDPos.Y() - aPos.Y()) < nHitLog &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrPageView* pPV = pView->GetSdrPageView();
        SdrViewEvent aVEvt;
        pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        pView->MarkObj(aVEvt.pRootObj, pPV);
    }
    pParent->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(pView);
    return bReturn;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncInsert::MouseMove( const MouseEvent& rMEvt )
{
    const Point aPos( pParent->PixelToLogic( rMEvt.GetPosPixel() ) );

    OSectionView* pView  = pParent->getView();
    pView->SetActualWin( pParent );

    if ( pView->IsCreateObj() )
    {
        pView->SetOrtho(SdrObjCustomShape::doConstructOrthogonal(pView->getSectionWindow()->getViewsWindow()->getShapeType()) ? !rMEvt.IsShift() : rMEvt.IsShift());
        pView->SetAngleSnapEnabled(rMEvt.IsShift());
    }

    const USHORT nHitLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );

    bool bIsSetPoint = false;
    if ( pView->IsAction() )
    {
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aPos);
        pView->MovAction(aPos);
    }

    //if ( isOver(pView->GetCreateObj(),*pParent->getPage(),*pView) )
    //    pParent->SetPointer( Pointer(POINTER_NOTALLOWED));
    //else
    if ( !bIsSetPoint )
        pParent->SetPointer( pView->GetPreferedPointer( aPos, pParent, nHitLog ) );

    return TRUE;
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::DlgEdFuncSelect( OReportSection* _pParent ) :
    DlgEdFunc( _pParent ),
    bMarkAction(FALSE)
{
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::~DlgEdFuncSelect()
{
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_bSelectionMode = false;
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return TRUE;
    // get view from parent
    OSectionView* pView = pParent->getView();
    pView->SetActualWin( pParent);

    const USHORT nDrgLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );
    const USHORT nHitLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );

    if ( rMEvt.IsLeft() )
    {
#ifdef MAC
        pParent->GrabFocus();
#endif
        pParent->CaptureMouse();
        SdrHdl* pHdl = pView->PickHandle(m_aMDPos);
        // hit selected object?
        if ( (pHdl!=NULL || pView->IsMarkedHit(m_aMDPos, nHitLog)) )
        {
            if ( rMEvt.GetClicks() == 1 )
            {
                pParent->getViewsWindow()->unmarkAllObjects(pView);
                pView->BegDragObj(m_aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
            }
            // TODO character in shapes
            //else
            //{
            //    SdrViewEvent aVEvt;
               // pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
            //    if ( aVEvt.pRootObj && aVEvt.pRootObj->ISA(SdrTextObj) )
            //        SetInEditMode(static_cast<SdrTextObj *>(aVEvt.pRootObj),rMEvt, FALSE);
            //}

        }
        else
        {
            SdrViewEvent aVEvt;
            const SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
            if( eHit == SDRHIT_UNMARKEDOBJECT )
            {
                // if not multi selection, unmark all
                if ( !rMEvt.IsShift() )
                    pParent->getViewsWindow()->unmarkAllObjects(NULL);

                if ( pView->MarkObj(m_aMDPos, nHitLog) && rMEvt.IsLeft() )
                {
                    // drag object
                    pHdl = pView->PickHandle(m_aMDPos);
                    pView->BegDragObj(m_aMDPos, (OutputDevice*) NULL, pHdl, nDrgLog);
                }
                else
                {
                    // select object
                    pView->BegMarkObj(m_aMDPos);
                    bMarkAction = TRUE;
                }
            }
            else
            {
                if( !rMEvt.IsShift() )
                    pParent->getViewsWindow()->unmarkAllObjects(NULL);
                    //pView->UnmarkAll();

                if ( rMEvt.GetClicks() == 1 )
                {
                    m_bSelectionMode = true;
                    pView->BegMarkObj( m_aMDPos );
                }
                else
                    pView->SdrBeginTextEdit( aVEvt.pRootObj,pView->GetSdrPageView(),pParent,sal_False );
            }
        }
    }

    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    DlgEdFunc::MouseButtonUp( rMEvt );

    // get view from parent
    const Point aPnt( pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
    OSectionView* pView  = pParent->getView();
    pView->SetActualWin( pParent );

    const USHORT nHitLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );

    if ( rMEvt.IsLeft() )
        checkMovementAllowed(rMEvt);

    bMarkAction = FALSE;

    if ( pView && pView->IsAction() )
        pView->EndAction();

    pParent->SetPointer( pView->GetPreferedPointer( aPnt, pParent, nHitLog ) );
    pParent->ReleaseMouse();

    pParent->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(pView);
    m_bSelectionMode = false;
    return TRUE;
}

//----------------------------------------------------------------------------

BOOL DlgEdFuncSelect::MouseMove( const MouseEvent& rMEvt )
{
    OSectionView* pView  = pParent->getView();
    pView->SetActualWin( pParent);

    const Point aPnt( pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
    bool bIsSetPoint = false;

    if ( pView->IsAction() )
    {
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aPnt);
        pView->MovAction(aPnt);
    }

    if ( !bIsSetPoint )
    {
        const USHORT nHitLog = USHORT ( pParent->PixelToLogic(Size(3,0)).Width() );
        pParent->SetPointer( pView->GetPreferedPointer( aPnt, pParent, nHitLog ) );

        // restore color
        unColorizeOverlappedObj();
    }

    return TRUE;
}
// -----------------------------------------------------------------------------
//void DlgEdFuncSelect::SetInEditMode(SdrTextObj* _pTextObj,const MouseEvent& rMEvt, BOOL bQuickDrag)
//{
//      OSectionView* pView  = pParent->getView();
//  pView->SetActualWin( pParent);
//
//  SdrPageView* pPV = pView->GetSdrPageView();
//  if( _pTextObj && _pTextObj->GetPage() == pPV->GetPage() )
//  {
//      pView->SetCurrentObj(OBJ_TEXT);
//      pView->SetEditMode(SDREDITMODE_EDIT);
//
//      BOOL bEmptyOutliner = FALSE;
//
//      if (!_pTextObj->GetOutlinerParaObject() && pView->GetTextEditOutliner())
//      {
//          ::SdrOutliner* pOutl = pView->GetTextEditOutliner();
//          ULONG nParaAnz = pOutl->GetParagraphCount();
//          Paragraph* p1stPara = pOutl->GetParagraph( 0 );
//
//          if (nParaAnz==1 && p1stPara)
//          {
//              // Bei nur einem Pararaph
//              if (pOutl->GetText(p1stPara).Len() == 0)
//              {
//                  bEmptyOutliner = TRUE;
//              }
//          }
//      }
//
//      if (_pTextObj != pView->GetTextEditObject() || bEmptyOutliner)
//      {
//          UINT32 nInv = _pTextObj->GetObjInventor();
//          UINT16 nSdrObjKind = _pTextObj->GetObjIdentifier();
//
//          if (nInv == SdrInventor && _pTextObj->HasTextEdit() &&
//              (nSdrObjKind == OBJ_TEXT ||
//              nSdrObjKind == OBJ_TITLETEXT ||
//              nSdrObjKind == OBJ_OUTLINETEXT ||
//              (_pTextObj->ISA(SdrTextObj) && !_pTextObj->IsEmptyPresObj())))
//          {
//              // Neuen Outliner machen (gehoert der SdrObjEditView)
//                SdrModel* pModel = _pTextObj->GetModel();
//                SdrOutliner* pOutl = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, pModel );
//              pOutl->SetMinDepth(0);
//              pOutl->SetStyleSheetPool((SfxStyleSheetPool*) pModel->GetStyleSheetPool());
//              //pOutl->SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
//              ULONG nCntrl = pOutl->GetControlWord();
//              nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
//              nCntrl |= EE_CNTRL_URLSFXEXECUTE;
//              nCntrl |= EE_CNTRL_MARKFIELDS;
//              nCntrl |= EE_CNTRL_AUTOCORRECT;
//
//              nCntrl &= ~EE_CNTRL_ULSPACESUMMATION;
//              //if ( pModel->IsSummationOfParagraphs() )
//              //    nCntrl |= EE_CNTRL_ULSPACESUMMATION;
//
//              //SetSpellOptions( nCntrl );
//
//              pOutl->SetControlWord(nCntrl);
//
//                uno::Reference< linguistic2::XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
//              if ( xSpellChecker.is() )
//                  pOutl->SetSpeller( xSpellChecker );
//
//                uno::Reference< linguistic2::XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
//              if( xHyphenator.is() )
//                  pOutl->SetHyphenator( xHyphenator );
//
//              pOutl->SetDefaultLanguage( Application::GetSettings().GetLanguage() );
//
//              // in einem Gliederungstext darf nicht auf die 0-te
//              // Ebene ausgerueckt werden
//              if (_pTextObj->GetObjInventor() == SdrInventor &&
//                  _pTextObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
//              {
//                  pOutl->SetMinDepth(1);
//              }
//
//              if (bEmptyOutliner)
//              {
//                  pView->SdrEndTextEdit(sal_True);
//              }
//
//              if( _pTextObj )
//              {
//                  OutlinerParaObject* pOPO = _pTextObj->GetOutlinerParaObject();
//                  if( ( pOPO && pOPO->IsVertical() ) /*||
//                      nSlotId == SID_ATTR_CHAR_VERTICAL ||
//                      nSlotId == SID_TEXT_FITTOSIZE_VERTICAL */)
//                      pOutl->SetVertical( TRUE );
//
//
//                  if (pView->SdrBeginTextEdit(_pTextObj, pPV, pParent, sal_True, pOutl) && _pTextObj->GetObjInventor() == SdrInventor)
//                  {
//                      //bFirstObjCreated = TRUE;
//                      //DeleteDefaultText();
//
//                      OutlinerView* pOLV = pView->GetTextEditOutlinerView();
//
//                      nSdrObjKind = _pTextObj->GetObjIdentifier();
//
//                      SdrViewEvent aVEvt;
//                      SdrHitKind eHit = pView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
//
//                      if (eHit == SDRHIT_TEXTEDIT)
//                      {
//                          // Text getroffen
//                          if (nSdrObjKind == OBJ_TEXT ||
//                              nSdrObjKind == OBJ_TITLETEXT ||
//                              nSdrObjKind == OBJ_OUTLINETEXT ||
//                              //nSlotId == SID_TEXTEDIT ||
//                              !bQuickDrag)
//                          {
//                              pOLV->MouseButtonDown(rMEvt);
//                              pOLV->MouseMove(rMEvt);
//                              pOLV->MouseButtonUp(rMEvt);
//                          }
//
//                          if ( bQuickDrag && _pTextObj->GetOutlinerParaObject())
//                          {
//                              pOLV->MouseButtonDown(rMEvt);
//                          }
//                      }
//                      else
//                      {
//                          // #98198# Move cursor to end of text
//                          ESelection aNewSelection(EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND, EE_PARA_NOT_FOUND, EE_INDEX_NOT_FOUND);
//                          pOLV->SetSelection(aNewSelection);
//                      }
//                  }
//                  else
//                  {
//                      //RestoreDefaultText();
//                  }
//              }
//          }
//      }
//  }
//  else
//      _pTextObj = NULL;
//}


//----------------------------------------------------------------------------
}
