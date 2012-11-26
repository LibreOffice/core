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
#include <svx/svdlegacy.hxx>
#include <svx/svdtrans.hxx>

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
    const basegfx::B2DPoint aPixelPos(m_pParent->GetPointerPosPixel().X(), m_pParent->GetPointerPosPixel().Y());
    const basegfx::B2DPoint aLogicPos(m_pParent->GetInverseViewTransformation() * aPixelPos);
    ForceScroll( aLogicPos );
    return 0;
}

//----------------------------------------------------------------------------

void DlgEdFunc::ForceScroll( const basegfx::B2DPoint& rPos )
{
    aScrollTimer.Stop();

    OReportWindow* pReportWindow = m_pParent->getSectionWindow()->getViewsWindow()->getView();
    OScrollWindowHelper* pScrollWindow = pReportWindow->getScrollWindow();
    const double fStartWidth(REPORT_STARTMARKER_WIDTH * double(m_pParent->GetMapMode().GetScaleX()));
    basegfx::B2DRange aOutRange(basegfx::B2DTuple(pScrollWindow->getThumbPos().X(), pScrollWindow->getThumbPos().Y()));
    basegfx::B2DRange aWorkArea(0.0, 0.0,
        pScrollWindow->getTotalSize().Width() - fStartWidth,
        pScrollWindow->getTotalSize().Height());

    aOutRange.expand(aOutRange.getMinimum() + basegfx::B2DVector(
        pReportWindow->GetOutputSizePixel().Width() - fStartWidth,
        pReportWindow->GetOutputSizePixel().Height()));

    aOutRange.transform(m_pParent->GetInverseViewTransformation());
    aWorkArea.transform(pScrollWindow->GetInverseViewTransformation());

    if( !aOutRange.isInside( rPos ) && aWorkArea.isInside( rPos ) )
    {
        ScrollBar* pHScroll = pScrollWindow->GetHScroll();
        ScrollBar* pVScroll = pScrollWindow->GetVScroll();
        ScrollType eH = SCROLL_LINEDOWN,eV = SCROLL_LINEDOWN;

        if( rPos.getX() < aOutRange.getMinX() )
            eH = SCROLL_LINEUP;
        else if( rPos.getX() <= aOutRange.getMaxX() )
            eH = SCROLL_DONTKNOW;

        if( rPos.getY() < aOutRange.getMinY() )
            eV = SCROLL_LINEUP;
        else if( rPos.getY() <= aOutRange.getMaxY() )
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
    m_rView.SetActualOutDev( m_pParent);
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
    m_aMDPos = m_pParent->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
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
            if ( pHdl!=NULL || m_rView.IsMarkedObjHit(m_aMDPos) )
            {
                bHandled = sal_True;
                m_pParent->CaptureMouse();
                m_pParent->getSectionWindow()->getViewsWindow()->BegDragObj(m_aMDPos, pHdl,&m_rView);
            }
        }
    }
    else if ( rMEvt.IsRight() && !rMEvt.IsLeft() && rMEvt.GetClicks() == 1 ) // mark object when context menu was selected
    {
        SdrViewEvent aVEvt;
        if ( m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt) != SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(NULL);
        if ( aVEvt.mpRootObj )
            m_rView.MarkObj(*aVEvt.mpRootObj);
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
        SdrObject* pSelected = m_rView.getSelectedIfSingle();

        if(pSelected)
            {
            activateOle(*pSelected);
        }
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
                else if ( m_rView.areSdrObjectsSelected() )
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

                    if ( m_rView.areSdrObjectsSelected() )
                    {
                        m_rView.MakeVisibleAtView( m_rView.getMarkedObjectSnapRange(), *m_pParent);
                    }

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
                        const basegfx::B2DRange aRange(
                            pHdl->getPosition() - basegfx::B2DPoint(DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE),
                            pHdl->getPosition() + basegfx::B2DPoint(DEFAUL_MOVE_SIZE, DEFAUL_MOVE_SIZE));

                        m_rView.MakeVisibleAtView( aRange, *m_pParent);
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
                    SdrObject* pSelected = m_rView.getSelectedIfSingle();

                    if ( pSelected )
                    {
                        activateOle(*pSelected);
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
void DlgEdFunc::activateOle(SdrObject& _rObj)
{
    //
    //  OLE: activate
    //
    SdrOle2Obj* pOleObj = dynamic_cast< SdrOle2Obj* >(&_rObj);

    if (pOleObj && pOleObj->GetObjRef().is() )
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
// -----------------------------------------------------------------------------
void DlgEdFunc::deactivateOle(bool _bSelect)
{
    OLEObjCache& rObjCache = GetSdrGlobalData().GetOLEObjCache();
    OReportController& rController = m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController();
    const sal_uLong nCount = rObjCache.Count();
    for(sal_uLong i = 0 ; i< nCount;++i)
    {
        SdrOle2Obj* pObj = reinterpret_cast<SdrOle2Obj*>(rObjCache.GetObject(i));
        if ( m_pParent->getPage() == pObj->getSdrPageFromSdrObject() )
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
                    m_rView.MarkObj(*pObj);
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
            OReportModel& rRptModel = static_cast< OReportModel& >(_pOverlappedObj->getSdrModelFromSdrObject());
            OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());

                // uncolorize an old object, if there is one
                unColorizeOverlappedObj();

                m_nOldColor = lcl_setColorOfObject(xComponent, m_nOverlappedControlColor);
                m_xOverlappingObj = xComponent;
                m_pOverlappingObj = _pOverlappedObj;
            }
        }
    }
// -----------------------------------------------------------------------------
void DlgEdFunc::unColorizeOverlappedObj()
{
    // uncolorize an old object, if there is one
    if (m_xOverlappingObj.is())
    {
        OReportModel& rRptModel = static_cast< OReportModel& >(m_pOverlappingObj->getSdrModelFromSdrObject());
        OXUndoEnvironment::OUndoEnvLock aLock(rRptModel.GetUndoEnv());

            lcl_setColorOfObject(m_xOverlappingObj, m_nOldColor);
            m_xOverlappingObj = NULL;
            m_pOverlappingObj = NULL;
        }
    }
// -----------------------------------------------------------------------------
bool DlgEdFunc::isOverlapping(const MouseEvent& rMEvt)
{
    bool bOverlapping = false;
    SdrViewEvent aVEvt;
    bOverlapping = m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONUP, aVEvt) != SDRHIT_NONE;
    if (bOverlapping && aVEvt.mpObj)
    {
        colorizeOverlappedObject(aVEvt.mpObj);
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
        basegfx::B2DPoint aPnt(m_pParent->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));

        if (m_bSelectionMode)
        {
            m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
        }
        else
        {
            bool bControlKeyPressed = rMEvt.IsMod1();
            // Don't allow points smaller 0
            if (bControlKeyPressed && (aPnt.getY() < 0.0))
            {
                aPnt.setY(0.0);
            }
            if (m_rView.IsDragResize())
            {
                // we resize the object don't resize to above sections
                if ( aPnt.getY() < 0.0 )
                {
                    aPnt.setY(0.0);
                }
            }
            m_pParent->getSectionWindow()->getViewsWindow()->EndDragObj( bControlKeyPressed, &m_rView, aPnt );
        }
        m_pParent->Invalidate(INVALIDATE_CHILDREN);
    }
    else
        m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
}
// -----------------------------------------------------------------------------
bool DlgEdFunc::isOnlyCustomShapeMarked()
{
    bool bReturn = true;
    const SdrObjectVector aSelection(m_rView.getSelectedSdrObjectVectorFromSdrMarkView());

    for (sal_uInt32 i =  0; i < aSelection.size(); ++i )
    {
        if (aSelection[i]->GetObjIdentifier() != OBJ_CUSTOMSHAPE)
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
        const SdrDragStat& rDragStat = m_rView.GetDragStat();
        if (rDragStat.GetDragMethod() != NULL)
        {
            SdrObjListIter aIter(*m_pParent->getPage(),IM_DEEPNOGROUPS);
            SdrObject* pObjIter = NULL;
            // loop through all marked objects and check if there new rect overlapps an old one.
            while( (pObjIter = aIter.Next()) != NULL && !bIsSetPoint)
            {
                if ( m_rView.IsObjMarked(*pObjIter)
                     && (dynamic_cast<OUnoObject*>(pObjIter) != NULL || dynamic_cast<OOle2Obj*>(pObjIter) != NULL) )
                {
                    basegfx::B2DRange aNewRange(pObjIter->getObjectRange(&m_rView));
                    double fDx(rDragStat.IsHorFixed() ? 0.0 : rDragStat.GetDX());
                    double fDy(rDragStat.IsVerFixed() ? 0.0 : rDragStat.GetDY());

                    if(fDx + aNewRange.getMinX() < 0.0)
                    {
                        fDx -= aNewRange.getMinX();
                    }

                    if(fDy + aNewRange.getMinY() < 0.0)
                    {
                        fDy -= aNewRange.getMinY();
                    }

                    if ( rDragStat.GetDragMethod()->getMoveOnly() )
                    {
                        aNewRange.transform(
                            basegfx::tools::createTranslateB2DHomMatrix(fDx, fDy));
                    }
                    else
                    {
                        basegfx::B2DHomMatrix aTransform;

                        aTransform.translate(-rDragStat.GetRef1());
                        aTransform.scale(rDragStat.GetXFact(), rDragStat.GetYFact());
                        aTransform.translate(rDragStat.GetRef1());
                        aNewRange.transform(aTransform);
                    }

                    SdrObject* pObjOverlapped = isOver(aNewRange,*m_pParent->getPage(),m_rView,false,pObjIter, ISOVER_IGNORE_CUSTOMSHAPES);
                    bIsSetPoint = pObjOverlapped ? true : false;
                    if (pObjOverlapped && !m_bSelectionMode)
                    {
                        colorizeOverlappedObject(pObjOverlapped);
                    }
                }
            }
        }
    }
    else if ( aVEvt.mpObj && (aVEvt.mpObj->GetObjIdentifier() != OBJ_CUSTOMSHAPE) && !m_bSelectionMode)
    {
        colorizeOverlappedObject(aVEvt.mpObj);
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
    m_rView.SetViewEditMode(SDREDITMODE_CREATE);
}

//----------------------------------------------------------------------------

DlgEdFuncInsert::~DlgEdFuncInsert()
{
    m_rView.SetViewEditMode(SDREDITMODE_EDIT);
}

//----------------------------------------------------------------------------

sal_Bool DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return sal_True;

    SdrViewEvent aVEvt;
    sal_Int16 nId = m_rView.getSdrObjectCreationInfo().getIdent();

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

    const basegfx::B2DPoint aPos(m_pParent->GetInverseViewTransformation() * basegfx::B2DPoint(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y()));
    const double fHitLog(basegfx::B2DVector(m_pParent->GetInverseViewTransformation() * basegfx::B2DVector(3.0, 0.0)).getLength());

    sal_Bool bReturn = sal_True;
    // object creation active?
    if ( m_rView.GetCreateObj() )
    {
        if ( isOver(m_rView.GetCreateObj(),*m_pParent->getPage(),m_rView) )
        {
            m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
            // BrkAction disables the create mode
            m_rView.SetViewEditMode(SDREDITMODE_CREATE);
            return sal_True;
        }

        m_rView.EndCreateObj(SDRCREATE_FORCEEND);

        if ( !m_rView.areSdrObjectsSelected() )
        {
            m_rView.MarkObj(aPos, fHitLog);
        }

        bReturn = m_rView.areSdrObjectsSelected();
        if ( bReturn )
        {
            OReportController& rController = m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController();
            const SdrObjectVector aSelection(m_rView.getSelectedSdrObjectVectorFromSdrMarkView());

            for (sal_uInt32 i =  0; i < aSelection.size();++i )
            {
                OOle2Obj* pObj = dynamic_cast< OOle2Obj* >(aSelection[i]);

                if ( pObj && !pObj->IsEmpty() )
                {
                    pObj->initializeChart(rController.getModel());
                }
            }
        }
    }
    else
        checkMovementAllowed(rMEvt);

    if ( !m_rView.areSdrObjectsSelected() &&
         fabs(m_aMDPos.getX() - aPos.getX()) < fHitLog &&
         fabs(m_aMDPos.getY() - aPos.getY()) < fHitLog &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrViewEvent aVEvt;
        m_rView.PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);
        m_rView.MarkObj(*aVEvt.mpRootObj);
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

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    basegfx::B2DPoint aLogicPos(m_pParent->GetInverseViewTransformation() * aPixelPos);

    if ( m_rView.GetCreateObj() )
    {
        m_rView.SetOrthogonal(SdrObjCustomShape::doConstructOrthogonal(m_rView.getReportSection()->getSectionWindow()->getViewsWindow()->getShapeType()) ? !rMEvt.IsShift() : rMEvt.IsShift());
        m_rView.SetAngleSnapEnabled(rMEvt.IsShift());
    }

    bool bIsSetPoint = false;
    if ( m_rView.IsAction() )
    {
        if ( m_rView.IsDragResize() )
        {
            // we resize the object don't resize to above sections
            if ( aLogicPos.getY() < 0.0 )
            {
                aLogicPos.setY(0.0);
            }
        }
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aLogicPos);
        m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aLogicPos,&m_rView, m_rView.GetDragMethod() == NULL, false);
    }

    //if ( isOver(m_rView.GetCreateObj(),*m_pParent->getPage(),*m_pView) )
    //    m_pParent->SetPointer( Pointer(POINTER_NOTALLOWED));
    //else
    if ( !bIsSetPoint )
        m_pParent->SetPointer( m_rView.GetPreferedPointer( aLogicPos, m_pParent) );

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
            m_rView.SdrBeginTextEdit( aVEvt.mpRootObj,m_pParent,sal_False );
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
    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aPnt(m_pParent->GetInverseViewTransformation() * aPixelPos);

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

    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    basegfx::B2DPoint aLogicPos(m_pParent->GetInverseViewTransformation() * aPixelPos);
    bool bIsSetPoint = false;

    if ( m_rView.IsAction() ) // Drag Mode
    {
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aLogicPos);
        if (m_rView.GetDragMethod()==NULL)
        {
            // create a selection
            m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aLogicPos, &m_rView, true, false);
        }
        else
        {
            if ( m_rView.IsDragResize() )
            {
                // we resize the object don't resize to above sections
                if ( aLogicPos.getY() < 0.0 )
                {
                    aLogicPos.setY(0.0);
                }
            }
            // drag or resize an object
            bool bControlKey = rMEvt.IsMod1();
            m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aLogicPos, &m_rView, false, bControlKey);
        }
    }

    if ( !bIsSetPoint )
    {
        m_pParent->SetPointer( m_rView.GetPreferedPointer( aLogicPos, m_pParent) );

        // restore color
        unColorizeOverlappedObj();
    }

    return sal_True;
}

//----------------------------------------------------------------------------
}
