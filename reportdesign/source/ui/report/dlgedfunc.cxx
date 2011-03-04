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
#include "precompiled_reportdesign.hxx"
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/seleng.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/unolingu.hxx>
#include <svx/svdetc.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svddrgmt.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svxids.hrc>
#include <svx/svditer.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include "dlgedfunc.hxx"
#include "ReportSection.hxx"
#include "DesignView.hxx"
#include "ReportController.hxx"
#include "SectionView.hxx"
#include "ViewsWindow.hxx"
#include "ReportWindow.hxx"
#include "RptObject.hxx"
#include "ScrollHelper.hxx"

#include "ReportRuler.hxx"
#include "UITools.hxx"

#include <uistrings.hrc>
#include "UndoEnv.hxx"
#include <RptModel.hxx>
#include <tools/diagnose_ex.h>

#define DEFAUL_MOVE_SIZE    100
namespace rptui
{
using namespace ::com::sun::star;
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

IMPL_LINK( DlgEdFunc, ScrollTimeout, Timer *,  )
{
    ForceScroll( m_pParent->PixelToLogic( m_pParent->GetPointerPosPixel() ) );
    return 0;
}

//----------------------------------------------------------------------------

void DlgEdFunc::ForceScroll( const Point& rPos )
{
    aScrollTimer.Stop();

    OReportWindow* pReportWindow = m_pParent->getSectionWindow()->getViewsWindow()->getView();
    OScrollWindowHelper* pScrollWindow = pReportWindow->getScrollWindow();

    Size aOut = pReportWindow->GetOutputSizePixel();
    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    aStartWidth *= m_pParent->GetMapMode().GetScaleX();

    aOut.Width() -= (long)aStartWidth;

    Rectangle aOutRect( pScrollWindow->getThumbPos(), aOut );
    aOutRect = m_pParent->PixelToLogic( aOutRect );
    //Rectangle aWorkArea = m_pParent->getView()->GetWorkArea();
    Point aGcc3WorkaroundTemporary;
    Rectangle aWorkArea(aGcc3WorkaroundTemporary,pScrollWindow->getTotalSize());
    aWorkArea.Right() -= (long)aStartWidth;
    aWorkArea = pScrollWindow->PixelToLogic( aWorkArea );
    if( !aOutRect.IsInside( rPos ) && aWorkArea.IsInside( rPos ) )
    {
        ScrollBar* pHScroll = pScrollWindow->GetHScroll();
        ScrollBar* pVScroll = pScrollWindow->GetVScroll();
        ScrollType eH = SCROLL_LINEDOWN,eV = SCROLL_LINEDOWN;
        if( rPos.X() < aOutRect.Left() )
            eH = SCROLL_LINEUP;
        else if( rPos.X() <= aOutRect.Right() )
            eH = SCROLL_DONTKNOW;

        if( rPos.Y() < aOutRect.Top() )
            eV = SCROLL_LINEUP;
        else if( rPos.Y() <= aOutRect.Bottom() )
            eV = SCROLL_DONTKNOW;

        pHScroll->DoScrollAction(eH);
        pVScroll->DoScrollAction(eV);
    }

    aScrollTimer.Start();
}

//----------------------------------------------------------------------------

DlgEdFunc::DlgEdFunc( OReportSection* _pParent )
:m_pParent(_pParent),
 m_rView(_pParent->getSectionView()),
 m_xOverlappingObj(NULL),
 m_pOverlappingObj(NULL),
 m_bSelectionMode(false),
 m_bUiActive(false),
 m_bShowPropertyBrowser(false)
{
    // m_pParent = _pParent;
    aScrollTimer.SetTimeoutHdl( LINK( this, DlgEdFunc, ScrollTimeout ) );
    m_rView.SetActualWin( m_pParent);
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

sal_Bool DlgEdFunc::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_aMDPos = m_pParent->PixelToLogic( rMEvt.GetPosPixel() );
    m_pParent->GrabFocus();
    sal_Bool bHandled = sal_False;
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() > 1 )
        {
            // show property browser
            if ( m_pParent->GetMode() != RPTUI_READONLY )
            {
                uno::Sequence<beans::PropertyValue> aArgs(1);
                aArgs[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowProperties"));
                aArgs[0].Value <<= sal_True;
                m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController().executeUnChecked(SID_SHOW_PROPERTYBROWSER,aArgs);
                m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(m_rView);
                // TODO character in shapes
                //    SdrViewEvent aVEvt;
                // m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
                //    if ( aVEvt.pRootObj && aVEvt.pRootObj->ISA(SdrTextObj) )
                //        SetInEditMode(static_cast<SdrTextObj *>(aVEvt.pRootObj),rMEvt, sal_False);
                bHandled = sal_True;
            }
        }
        else
        {
            SdrHdl* pHdl = m_rView.PickHandle(m_aMDPos);
            //m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(m_pView);

            // if selected object was hit, drag object
            if ( pHdl!=NULL || m_rView.IsMarkedHit(m_aMDPos) )
            {
                bHandled = sal_True;
                m_pParent->CaptureMouse();
                m_pParent->getSectionWindow()->getViewsWindow()->BegDragObj(m_aMDPos, pHdl,&m_rView);
            }
        }
    }
    else if ( rMEvt.IsRight() && !rMEvt.IsLeft() && rMEvt.GetClicks() == 1 ) // mark object when context menu was selected
    {
        SdrPageView* pPV = m_rView.GetSdrPageView();
        SdrViewEvent aVEvt;
        if ( m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt) != SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);
        if ( aVEvt.pRootObj )
            m_rView.MarkObj(aVEvt.pRootObj, pPV);
        else
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);

        bHandled = sal_True;
    }
    else if( !rMEvt.IsLeft() )
        bHandled = sal_True;
    if ( !bHandled )
        m_pParent->CaptureMouse();
    return bHandled;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFunc::MouseButtonUp( const MouseEvent& /*rMEvt*/ )
{
    sal_Bool bHandled = sal_False;
    m_pParent->getSectionWindow()->getViewsWindow()->stopScrollTimer();
    return bHandled;
}
// -----------------------------------------------------------------------------
void DlgEdFunc::checkTwoCklicks(const MouseEvent& rMEvt)
{
    deactivateOle();

    const sal_uInt16 nClicks = rMEvt.GetClicks();
    if ( nClicks == 2 && rMEvt.IsLeft() )
    {
        if ( m_rView.AreObjectsMarked() )
        {
            const SdrMarkList& rMarkList = m_rView.GetMarkedObjectList();
            if (rMarkList.GetMarkCount() == 1)
            {
                const SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();
                activateOle(pObj);
            } // if (rMarkList.GetMarkCount() == 1)
        } // if ( pView->AreObjectsMarked() )
    }
}
// -----------------------------------------------------------------------------
void DlgEdFunc::stopScrollTimer()
{
    unColorizeOverlappedObj();
    aScrollTimer.Stop();
    if ( m_pParent->IsMouseCaptured() )
        m_pParent->ReleaseMouse();
}
//----------------------------------------------------------------------------

sal_Bool DlgEdFunc::MouseMove( const MouseEvent& /*rMEvt*/ )
{
    return sal_False;
}
//------------------------------------------------------------------------------
sal_Bool DlgEdFunc::handleKeyEvent(const KeyEvent& _rEvent)
{
    sal_Bool bReturn = sal_False;

    if ( !m_bUiActive )
    {
        const KeyCode& rCode = _rEvent.GetKeyCode();
        sal_uInt16 nCode = rCode.GetCode();

        switch ( nCode )
        {
            case KEY_ESCAPE:
            {
                if ( m_pParent->getSectionWindow()->getViewsWindow()->IsAction() )
                {
                    m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
                    bReturn = sal_True;
                }
                else if ( m_rView.IsTextEdit() )
                {
                    m_rView.SdrEndTextEdit();
                    bReturn = sal_True;
                }
                else if ( m_rView.AreObjectsMarked() )
                {
                    const SdrHdlList& rHdlList = m_rView.GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();
                    if ( pHdl )
                        ((SdrHdlList&)rHdlList).ResetFocusHdl();
                    else
                        m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);
                        //m_rView.UnmarkAll();

                    deactivateOle(true);
                    bReturn = sal_False;
                }
                else
                {
                    deactivateOle(true);
                }
            }
            break;
            case KEY_TAB:
            {
                if ( !rCode.IsMod1() && !rCode.IsMod2() )
                {
                    // mark next object
                    if ( !m_rView.MarkNextObj( !rCode.IsShift() ) )
                    {
                        // if no next object, mark first/last
                        m_rView.UnmarkAllObj();
                        m_rView.MarkNextObj( !rCode.IsShift() );
                    }

                    if ( m_rView.AreObjectsMarked() )
                        m_rView.MakeVisible( m_rView.GetAllMarkedRect(), *m_pParent);

                    bReturn = sal_True;
                }
                else if ( rCode.IsMod1() && rCode.IsMod2())
                {
                    // selected handle
                    const SdrHdlList& rHdlList = m_rView.GetHdlList();
                    ((SdrHdlList&)rHdlList).TravelFocusHdl( !rCode.IsShift() );

                    // guarantee visibility of focused handle
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();
                    if ( pHdl )
                    {
                        Point aHdlPosition( pHdl->GetPos() );
                        Rectangle aVisRect( aHdlPosition - Point( DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE ), Size( 200, 200 ) );
                        m_rView.MakeVisible( aVisRect, *m_pParent);
                    }

                    bReturn = sal_True;
                }
            }
            break;
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            {
                m_pParent->getSectionWindow()->getViewsWindow()->handleKey(rCode);
                bReturn = sal_True;
            }
            break;
            case KEY_RETURN:
                if ( !rCode.IsMod1() )
                {
                    const SdrMarkList& rMarkList = m_rView.GetMarkedObjectList();
                    if ( rMarkList.GetMarkCount() == 1 )
                    {
                        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                        activateOle(pObj);
                    }
                }
                break;
            case KEY_DELETE:
                if ( !rCode.IsMod1() && !rCode.IsMod2() )
                {
                    bReturn = sal_True;
                    break;
                } // if ( !rCode.IsMod1() && !rCode.IsMod2() )
                // run through
            default:
            {
                bReturn = m_rView.KeyInput(_rEvent, m_pParent);
            }
            break;
        }
    }

    if ( bReturn && m_pParent->IsMouseCaptured() )
        m_pParent->ReleaseMouse();

    return bReturn;
}
// -----------------------------------------------------------------------------
void DlgEdFunc::activateOle(SdrObject* _pObj)
{
    if ( _pObj )
    {
        const sal_uInt16 nSdrObjKind = _pObj->GetObjIdentifier();
        //
        //  OLE: activate
        //
        if (nSdrObjKind == OBJ_OLE2)
        {
            bool bIsInplaceOle = false;
            if (!bIsInplaceOle)
            {
                SdrOle2Obj* pOleObj = dynamic_cast<SdrOle2Obj*>(_pObj);
                if ( pOleObj->GetObjRef().is() )
                {
                    if (m_rView.IsTextEdit())
                    {
                        m_rView.SdrEndTextEdit();
                    }

                    pOleObj->AddOwnLightClient();
                    pOleObj->SetWindow(VCLUnoHelper::GetInterface(m_pParent));
                    try
                    {
                        pOleObj->GetObjRef()->changeState( embed::EmbedStates::UI_ACTIVE );
                        m_bUiActive = true;
                        OReportController& rController = m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController();
                        m_bShowPropertyBrowser = rController.isCommandChecked(SID_SHOW_PROPERTYBROWSER);
                        if ( m_bShowPropertyBrowser )
                            rController.executeChecked(SID_SHOW_PROPERTYBROWSER,uno::Sequence< beans::PropertyValue >());
                    }
                    catch( uno::Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        } // if (nSdrObjKind == OBJ_OLE2)
    } // if ( _pObj )
}
// -----------------------------------------------------------------------------
void DlgEdFunc::deactivateOle(bool _bSelect)
{
    OLEObjCache& rObjCache = GetSdrGlobalData().GetOLEObjCache();
    OReportController& rController = m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController();
    const sal_uLong nCount = rObjCache.Count();
    for(sal_uLong i = 0 ; i< nCount;++i)
    {
        SdrOle2Obj* pObj = reinterpret_cast<SdrOle2Obj*>(rObjCache.GetObject(i));
        if ( m_pParent->getPage() == pObj->GetPage() )
        {
            uno::Reference< embed::XEmbeddedObject > xObj = pObj->GetObjRef();
            if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::UI_ACTIVE )
            {
                xObj->changeState( embed::EmbedStates::RUNNING );
                m_bUiActive = false;
                if ( m_bShowPropertyBrowser )
                {
                    rController.executeChecked(SID_SHOW_PROPERTYBROWSER,uno::Sequence< beans::PropertyValue >());
                }

                if ( _bSelect )
                {
                    SdrPageView* pPV = m_rView.GetSdrPageView();
                    m_rView.MarkObj(pObj, pPV);
                }
            } // if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::UI_ACTIVE )
        }
    } // for(sal_uLong i = 0 ; i< nCount;++i)
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
    SdrViewEvent aVEvt;
    bOverlapping = m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt) != SDRHIT_NONE;
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
    if ( m_pParent->getSectionWindow()->getViewsWindow()->IsDragObj() )
    {
        if ( isRectangleHit(rMEvt) )
        {
            // there is an other component under use, break action
            m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
        }
        // object was dragged
        Point aPnt( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
        if (m_bSelectionMode)
        {
            m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
        }
        else
        {
            bool bControlKeyPressed = rMEvt.IsMod1();
            // Don't allow points smaller 0
            if (bControlKeyPressed && (aPnt.Y() < 0))
            {
                aPnt.Y() = 0;
            }
            if (m_rView.IsDragResize())
            {
                // we resize the object don't resize to above sections
                if ( aPnt.Y() < 0 )
                {
                    aPnt.Y() = 0;
                }
            }
            m_pParent->getSectionWindow()->getViewsWindow()->EndDragObj( bControlKeyPressed, &m_rView, aPnt );
        }
        m_pParent->getSectionWindow()->getViewsWindow()->ForceMarkedToAnotherPage();
        m_pParent->Invalidate(INVALIDATE_CHILDREN);
    }
    else
        m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::isOnlyCustomShapeMarked()
{
    bool bReturn = true;
    const SdrMarkList& rMarkList = m_rView.GetMarkedObjectList();
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

    SdrViewEvent aVEvt;
    const SdrHitKind eHit = m_rView.PickAnything(rMEvt, SDRMOUSEMOVE, aVEvt);
    bool bIsSetPoint = (eHit == SDRHIT_UNMARKEDOBJECT);
    if ( !bIsSetPoint )
    {
        // no drag rect, we have to check every single select rect
        //const Rectangle& rRect = m_rView.GetDragStat().GetActionRect();
        const SdrDragStat& rDragStat = m_rView.GetDragStat();
        if (rDragStat.GetDragMethod() != NULL)
        {
            SdrObjListIter aIter(*m_pParent->getPage(),IM_DEEPNOGROUPS);
            SdrObject* pObjIter = NULL;
            // loop through all marked objects and check if there new rect overlapps an old one.
            while( (pObjIter = aIter.Next()) != NULL && !bIsSetPoint)
            {
                if ( m_rView.IsObjMarked(pObjIter)
                     && dynamic_cast<OUnoObject*>(pObjIter) != NULL )
                {
                    Rectangle aNewRect = pObjIter->GetLastBoundRect();
                    long nDx = rDragStat.IsHorFixed() ? 0 : rDragStat.GetDX();
                    long nDy = rDragStat.IsVerFixed() ? 0 : rDragStat.GetDY();
                    if ( (nDx + aNewRect.Left()) < 0 )
                        nDx = -aNewRect.Left();
                    if ( (nDy + aNewRect.Top()) < 0 )
                        nDy = -aNewRect.Top();

                    if ( rDragStat.GetDragMethod()->getMoveOnly() )
                        aNewRect.Move(nDx,nDy);
                    else
                        ::ResizeRect(aNewRect,rDragStat.GetRef1(),rDragStat.GetXFact(),rDragStat.GetYFact());


                    SdrObject* pObjOverlapped = isOver(aNewRect,*m_pParent->getPage(),m_rView,false,pObjIter, ISOVER_IGNORE_CUSTOMSHAPES);
                    bIsSetPoint = pObjOverlapped ? true : false;
                    if (pObjOverlapped && !m_bSelectionMode)
                    {
                        colorizeOverlappedObject(pObjOverlapped);
                    }
                }
            }
        }
    }
    else if ( aVEvt.pObj && (aVEvt.pObj->GetObjIdentifier() != OBJ_CUSTOMSHAPE) && !m_bSelectionMode)
    {
        colorizeOverlappedObject(aVEvt.pObj);
    }
    else
        bIsSetPoint = false;
    return bIsSetPoint;
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::setMovementPointer(const MouseEvent& rMEvt)
{
    bool bIsSetPoint = isRectangleHit(rMEvt);
    if ( bIsSetPoint )
        m_pParent->SetPointer( Pointer(POINTER_NOTALLOWED));
    else
    {
        bool bCtrlKey = rMEvt.IsMod1();
        (void)bCtrlKey;
        if (bCtrlKey)
        {
            m_pParent->SetPointer( Pointer(POINTER_MOVEDATALINK ));
            bIsSetPoint = true;
        }
    }
    return bIsSetPoint;
}
//----------------------------------------------------------------------------

DlgEdFuncInsert::DlgEdFuncInsert( OReportSection* _pParent ) :
    DlgEdFunc( _pParent )
{
    m_rView.SetCreateMode( sal_True );
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::~DlgEdFuncInsert()
{
    m_rView.SetEditMode( sal_True );
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return sal_True;

    SdrViewEvent aVEvt;
    sal_Int16 nId = m_rView.GetCurrentObjIdentifier();

    const SdrHitKind eHit = m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

// eHit == SDRHIT_UNMARKEDOBJECT under the mouse cursor is a unmarked object

    if (eHit == SDRHIT_UNMARKEDOBJECT &&
        nId != OBJ_CUSTOMSHAPE)
    {
        // there is an object under the mouse cursor, but not a customshape
        // rtl::OUString suWasN = m_rView.getInsertObjString();
        // rtl::OUString suWasN2 = m_pParent->getSectionWindow()->getViewsWindow()->GetInsertObjString();

        m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
        return sal_False;
    }

    if( eHit != SDRHIT_UNMARKEDOBJECT || nId == OBJ_CUSTOMSHAPE)
    {
        // if no action, create object
        if ( !m_pParent->getSectionWindow()->getViewsWindow()->IsAction() )
        {
            deactivateOle(true);
            if ( m_pParent->getSectionWindow()->getViewsWindow()->HasSelection() )
                m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(&m_rView);
            m_rView.BegCreateObj(m_aMDPos);
            m_pParent->getSectionWindow()->getViewsWindow()->createDefault();
        }
    }
    else
    {
        if( !rMEvt.IsShift() )
        {
            // shift key pressed?
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);
        }
        m_pParent->getSectionWindow()->getViewsWindow()->BegMarkObj( m_aMDPos,&m_rView );
    }

    return sal_True;
}

//----------------------------------------------------------------------------
sal_Bool DlgEdFuncInsert::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonUp( rMEvt ) )
        return sal_True;

    const Point aPos( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
    const sal_uInt16 nHitLog = sal_uInt16 ( m_pParent->PixelToLogic(Size(3,0)).Width() );

    sal_Bool bReturn = sal_True;
    // object creation active?
    if ( m_rView.IsCreateObj() )
    {
        if ( isOver(m_rView.GetCreateObj(),*m_pParent->getPage(),m_rView) )
        {
            m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
            // BrkAction disables the create mode
            m_rView.SetCreateMode( sal_True );
            return sal_True;
        }

        m_rView.EndCreateObj(SDRCREATE_FORCEEND);

        if ( !m_rView.AreObjectsMarked() )
        {
            m_rView.MarkObj(aPos, nHitLog);
        }

        bReturn = m_rView.AreObjectsMarked();
        if ( bReturn )
        {
            OReportController& rController = m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController();
            const SdrMarkList& rMarkList = m_rView.GetMarkedObjectList();
            for (sal_uInt32 i =  0; i < rMarkList.GetMarkCount();++i )
            {
                SdrMark* pMark = rMarkList.GetMark(i);
                // bCheck = dynamic_cast<OUnoObject*>(pMark->GetMarkedSdrObj()) != NULL;
                OOle2Obj* pObj = dynamic_cast<OOle2Obj*>(pMark->GetMarkedSdrObj());
                if ( pObj && !pObj->IsEmpty() )
                {
                    pObj->initializeChart(rController.getModel());
                }
            }
        }
    }
    else
        checkMovementAllowed(rMEvt);

    if ( !m_rView.AreObjectsMarked() &&
         Abs(m_aMDPos.X() - aPos.X()) < nHitLog &&
         Abs(m_aMDPos.Y() - aPos.Y()) < nHitLog &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrPageView* pPV = m_rView.GetSdrPageView();
        SdrViewEvent aVEvt;
        m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        m_rView.MarkObj(aVEvt.pRootObj, pPV);
    } // ift() && !rMEvt.IsMod2() )
    checkTwoCklicks(rMEvt);
    m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(m_rView);
    return bReturn;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncInsert::MouseMove( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseMove(rMEvt ) )
        return sal_True;
    Point   aPos( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( m_rView.IsCreateObj() )
    {
        m_rView.SetOrtho(SdrObjCustomShape::doConstructOrthogonal(m_rView.getReportSection()->getSectionWindow()->getViewsWindow()->getShapeType()) ? !rMEvt.IsShift() : rMEvt.IsShift());
        m_rView.SetAngleSnapEnabled(rMEvt.IsShift());
    }

    bool bIsSetPoint = false;
    if ( m_rView.IsAction() )
    {
        if ( m_rView.IsDragResize() )
        {
            // we resize the object don't resize to above sections
            if ( aPos.Y() < 0 )
            {
                aPos.Y() = 0;
            }
        }
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aPos);
        m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aPos,&m_rView, m_rView.GetDragMethod() == NULL, false);
    }

    //if ( isOver(m_rView.GetCreateObj(),*m_pParent->getPage(),*m_pView) )
    //    m_pParent->SetPointer( Pointer(POINTER_NOTALLOWED));
    //else
    if ( !bIsSetPoint )
        m_pParent->SetPointer( m_rView.GetPreferedPointer( aPos, m_pParent) );

    return sal_True;
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::DlgEdFuncSelect( OReportSection* _pParent ) :
    DlgEdFunc( _pParent )
{
}

//----------------------------------------------------------------------------

DlgEdFuncSelect::~DlgEdFuncSelect()
{
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncSelect::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_bSelectionMode = false;
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return sal_True;

    SdrViewEvent aVEvt;
    const SdrHitKind eHit = m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
    if( eHit == SDRHIT_UNMARKEDOBJECT )
    {
        // if not multi selection, unmark all
        if ( !rMEvt.IsShift() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);

        if ( m_rView.MarkObj(m_aMDPos) && rMEvt.IsLeft() )
        {
            // drag object
            m_pParent->getSectionWindow()->getViewsWindow()->BegDragObj(m_aMDPos, m_rView.PickHandle(m_aMDPos), &m_rView);
        }
        else
        {
            // select object
            m_pParent->getSectionWindow()->getViewsWindow()->BegMarkObj(m_aMDPos,&m_rView);
        }
    }
    else
    {
        if( !rMEvt.IsShift() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);
            //m_rView.UnmarkAll();

        if ( rMEvt.GetClicks() == 1 )
        {
            m_bSelectionMode = true;
            m_pParent->getSectionWindow()->getViewsWindow()->BegMarkObj( m_aMDPos ,&m_rView);
        }
        else
        {
            m_rView.SdrBeginTextEdit( aVEvt.pRootObj,m_rView.GetSdrPageView(),m_pParent,sal_False );
        }
    }

    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonUp( rMEvt ) )
        return sal_True;

    // get view from parent
    const Point aPnt( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( rMEvt.IsLeft() )                     // left mousebutton pressed
        checkMovementAllowed(rMEvt);

    m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
    checkTwoCklicks(rMEvt);

    m_pParent->SetPointer( m_rView.GetPreferedPointer( aPnt, m_pParent) );

    if ( !m_bUiActive )
        m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(m_rView);
    m_bSelectionMode = false;
    return sal_True;
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncSelect::MouseMove( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseMove(rMEvt ) )
        return sal_True;

    Point aPnt( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
    bool bIsSetPoint = false;

    if ( m_rView.IsAction() ) // Drag Mode
    {
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aPnt);
        if (m_rView.GetDragMethod()==NULL)
        {
            // create a selection
            m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aPnt, &m_rView, true, false);
        }
        else
        {
            if ( m_rView.IsDragResize() )
            {
                // we resize the object don't resize to above sections
                if ( aPnt.Y() < 0 )
                {
                    aPnt.Y() = 0;
                }
                // grow section if it is under the current section
//                else
//                {
//                    const Size& aSectionSize = m_rView->GetSizePixel();
//                    if ( aPnt.Y() > aSectionSize.Height() )
//                    {
//                        aPnt.Y() = aSectionSize.Height();
//                    }
//                }
            }
            // drag or resize an object
            bool bControlKey = rMEvt.IsMod1();
            m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aPnt, &m_rView, false, bControlKey);
        }
    }

    if ( !bIsSetPoint )
    {
        m_pParent->SetPointer( m_rView.GetPreferedPointer( aPnt, m_pParent) );

        // restore color
        unColorizeOverlappedObj();
    }

    return sal_True;
}
// -----------------------------------------------------------------------------
//void DlgEdFuncSelect::SetInEditMode(SdrTextObj* _pTextObj,const MouseEvent& rMEvt, sal_Bool bQuickDrag)
//{
//
//  SdrPageView* pPV = m_rView.GetSdrPageView();
//  if( _pTextObj && _pTextObj->GetPage() == pPV->GetPage() )
//  {
//      m_rView.SetCurrentObj(OBJ_TEXT);
//      m_rView.SetEditMode(SDREDITMODE_EDIT);
//
//      sal_Bool bEmptyOutliner = sal_False;
//
//      if (!_pTextObj->GetOutlinerParaObject() && m_rView.GetTextEditOutliner())
//      {
//          ::SdrOutliner* pOutl = m_rView.GetTextEditOutliner();
//          sal_uLong nParaAnz = pOutl->GetParagraphCount();
//          Paragraph* p1stPara = pOutl->GetParagraph( 0 );
//
//          if (nParaAnz==1 && p1stPara)
//          {
//              // Bei nur einem Pararaph
//              if (pOutl->GetText(p1stPara).Len() == 0)
//              {
//                  bEmptyOutliner = sal_True;
//              }
//          }
//      }
//
//      if (_pTextObj != m_rView.GetTextEditObject() || bEmptyOutliner)
//      {
//          sal_uInt32 nInv = _pTextObj->GetObjInventor();
//          sal_uInt16 nSdrObjKind = _pTextObj->GetObjIdentifier();
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
//              sal_uLong nCntrl = pOutl->GetControlWord();
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
//                  m_rView.SdrEndTextEdit(sal_True);
//              }
//
//              if( _pTextObj )
//              {
//                  OutlinerParaObject* pOPO = _pTextObj->GetOutlinerParaObject();
//                  if( ( pOPO && pOPO->IsVertical() ) /*||
//                      nSlotId == SID_ATTR_CHAR_VERTICAL ||
//                      nSlotId == SID_TEXT_FITTOSIZE_VERTICAL */)
//                      pOutl->SetVertical( sal_True );
//
//
//                  if (m_rView.SdrBeginTextEdit(_pTextObj, pPV, pParent, sal_True, pOutl) && _pTextObj->GetObjInventor() == SdrInventor)
//                  {
//                      //bFirstObjCreated = sal_True;
//                      //DeleteDefaultText();
//
//                      OutlinerView* pOLV = m_rView.GetTextEditOutlinerView();
//
//                      nSdrObjKind = _pTextObj->GetObjIdentifier();
//
//                      SdrViewEvent aVEvt;
//                      SdrHitKind eHit = m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
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
