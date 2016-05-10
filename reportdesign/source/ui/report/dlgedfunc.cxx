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
#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/seleng.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
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
#include "UITools.hxx"

#include <uistrings.hrc>
#include "UndoEnv.hxx"
#include <RptModel.hxx>
#include <tools/diagnose_ex.h>

#define DEFAUL_MOVE_SIZE    100
namespace rptui
{
using namespace ::com::sun::star;


IMPL_LINK_NOARG_TYPED( DlgEdFunc, ScrollTimeout, Timer *, void )
{
    ForceScroll( m_pParent->PixelToLogic( m_pParent->GetPointerPosPixel() ) );
}


void DlgEdFunc::ForceScroll( const Point& rPos )
{
    aScrollTimer.Stop();

    OReportWindow* pReportWindow = m_pParent->getSectionWindow()->getViewsWindow()->getView();
    OScrollWindowHelper* pScrollWindow = pReportWindow->getScrollWindow();

    Size aOut = pReportWindow->GetOutputSizePixel();
    Fraction aStartWidth(long(REPORT_STARTMARKER_WIDTH));
    aStartWidth *= m_pParent->GetMapMode().GetScaleX();

    aOut.Width() -= (long)aStartWidth;
    aOut.Height() = m_pParent->GetOutputSizePixel().Height();

    Point aPos = pScrollWindow->getThumbPos();
    aPos.X() *= 0.5;
    aPos.Y() *= 0.5;
    Rectangle aOutRect( aPos, aOut );
    aOutRect = m_pParent->PixelToLogic( aOutRect );
    Rectangle aWorkArea(Point(), pScrollWindow->getTotalSize());
    aWorkArea.Right() -= (long)aStartWidth;
    aWorkArea = pScrollWindow->PixelToLogic( aWorkArea );
    if( !aOutRect.IsInside( rPos ) && aWorkArea.IsInside( rPos ) )
    {
        ScrollBar& rHScroll = pScrollWindow->GetHScroll();
        ScrollBar& rVScroll = pScrollWindow->GetVScroll();
        ScrollType eH = SCROLL_LINEDOWN,eV = SCROLL_LINEDOWN;
        if( rPos.X() < aOutRect.Left() )
            eH = SCROLL_LINEUP;
        else if( rPos.X() <= aOutRect.Right() )
            eH = SCROLL_DONTKNOW;

        if( rPos.Y() < aOutRect.Top() )
            eV = SCROLL_LINEUP;
        else if( rPos.Y() <= aOutRect.Bottom() )
            eV = SCROLL_DONTKNOW;

        rHScroll.DoScrollAction(eH);
        rVScroll.DoScrollAction(eV);
    }

    aScrollTimer.Start();
}

DlgEdFunc::DlgEdFunc( OReportSection* _pParent )
    : m_pParent(_pParent)
    , m_rView(_pParent->getSectionView())
    , m_xOverlappingObj(nullptr)
    , m_pOverlappingObj(nullptr)
    , m_nOverlappedControlColor(0)
    , m_nOldColor(0)
    , m_bSelectionMode(false)
    , m_bUiActive(false)
    , m_bShowPropertyBrowser(false)
{
    aScrollTimer.SetTimeoutHdl( LINK( this, DlgEdFunc, ScrollTimeout ) );
    m_rView.SetActualWin( m_pParent);
    aScrollTimer.SetTimeout( SELENG_AUTOREPEAT_INTERVAL );
}

void DlgEdFunc::setOverlappedControlColor(sal_Int32 _nColor)
{
    m_nOverlappedControlColor = _nColor;
}

sal_Int32 lcl_setColorOfObject(uno::Reference< uno::XInterface > _xObj, long _nColorTRGB)
{
    sal_Int32 nBackColor = 0;
    try
    {
        uno::Reference<report::XReportComponent> xComponent(_xObj, uno::UNO_QUERY_THROW);
        uno::Reference< beans::XPropertySet > xProp(xComponent, uno::UNO_QUERY_THROW);
        uno::Any aAny = xProp->getPropertyValue(PROPERTY_CONTROLBACKGROUND);
        if (aAny.hasValue())
        {
            aAny >>= nBackColor;
            // try to set background color at the ReportComponent
            uno::Any aBlackColorAny = uno::makeAny(_nColorTRGB);
            xProp->setPropertyValue(PROPERTY_CONTROLBACKGROUND, aBlackColorAny);
        }
    }
    catch(uno::Exception&)
    {
    }
    return nBackColor;
}

DlgEdFunc::~DlgEdFunc()
{
    unColorizeOverlappedObj();
    aScrollTimer.Stop();
}


bool DlgEdFunc::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_aMDPos = m_pParent->PixelToLogic( rMEvt.GetPosPixel() );
    m_pParent->GrabFocus();
    bool bHandled = false;
    if ( rMEvt.IsLeft() )
    {
        if ( rMEvt.GetClicks() > 1 )
        {
            // show property browser
            if ( m_pParent->GetMode() != RPTUI_READONLY )
            {
                uno::Sequence<beans::PropertyValue> aArgs(1);
                aArgs[0].Name = "ShowProperties";
                aArgs[0].Value <<= sal_True;
                m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController().executeUnChecked(SID_SHOW_PROPERTYBROWSER,aArgs);
                m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(m_rView);
                // TODO character in shapes
                //    SdrViewEvent aVEvt;
                // m_rView.PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
                //    if ( aVEvt.pRootObj && aVEvt.pRootObj->ISA(SdrTextObj) )
                //        SetInEditMode(static_cast<SdrTextObj *>(aVEvt.pRootObj),rMEvt, sal_False);
                bHandled = true;
            }
        }
        else
        {
            SdrHdl* pHdl = m_rView.PickHandle(m_aMDPos);

            // if selected object was hit, drag object
            if ( pHdl!=nullptr || m_rView.IsMarkedHit(m_aMDPos) )
            {
                bHandled = true;
                m_pParent->CaptureMouse();
                m_pParent->getSectionWindow()->getViewsWindow()->BegDragObj(m_aMDPos, pHdl,&m_rView);
            }
        }
    }
    else if ( rMEvt.IsRight() && !rMEvt.IsLeft() && rMEvt.GetClicks() == 1 ) // mark object when context menu was selected
    {
        SdrPageView* pPV = m_rView.GetSdrPageView();
        SdrViewEvent aVEvt;
        if ( m_rView.PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt) != SDRHIT_MARKEDOBJECT && !rMEvt.IsShift() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(nullptr);
        if ( aVEvt.pRootObj )
            m_rView.MarkObj(aVEvt.pRootObj, pPV);
        else
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(nullptr);

        bHandled = true;
    }
    else if( !rMEvt.IsLeft() )
        bHandled = true;
    if ( !bHandled )
        m_pParent->CaptureMouse();
    return bHandled;
}


bool DlgEdFunc::MouseButtonUp( const MouseEvent& /*rMEvt*/ )
{
    bool bHandled = false;
    m_pParent->getSectionWindow()->getViewsWindow()->stopScrollTimer();
    return bHandled;
}

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
            }
        }
    }
}

void DlgEdFunc::stopScrollTimer()
{
    unColorizeOverlappedObj();
    aScrollTimer.Stop();
    if ( m_pParent->IsMouseCaptured() )
        m_pParent->ReleaseMouse();
}


bool DlgEdFunc::MouseMove( const MouseEvent& /*rMEvt*/ )
{
    return false;
}

bool DlgEdFunc::handleKeyEvent(const KeyEvent& _rEvent)
{
    bool bReturn = false;

    if ( !m_bUiActive )
    {
        const vcl::KeyCode& rCode = _rEvent.GetKeyCode();
        sal_uInt16 nCode = rCode.GetCode();

        switch ( nCode )
        {
            case KEY_ESCAPE:
            {
                if ( m_pParent->getSectionWindow()->getViewsWindow()->IsAction() )
                {
                    m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
                    bReturn = true;
                }
                else if ( m_rView.IsTextEdit() )
                {
                    m_rView.SdrEndTextEdit();
                    bReturn = true;
                }
                else if ( m_rView.AreObjectsMarked() )
                {
                    const SdrHdlList& rHdlList = m_rView.GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();
                    if ( pHdl )
                        ((SdrHdlList&)rHdlList).ResetFocusHdl();
                    else
                        m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(nullptr);

                    deactivateOle(true);
                    bReturn = false;
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

                    bReturn = true;
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

                    bReturn = true;
                }
            }
            break;
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            {
                m_pParent->getSectionWindow()->getViewsWindow()->handleKey(rCode);
                bReturn = true;
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
                    bReturn = true;
                    break;
                }
                SAL_FALLTHROUGH;
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

void DlgEdFunc::activateOle(SdrObject* _pObj)
{
    if ( _pObj )
    {
        const sal_uInt16 nSdrObjKind = _pObj->GetObjIdentifier();

        //  OLE: activate

        if (nSdrObjKind == OBJ_OLE2)
        {
            bool bIsInplaceOle = false;
            if (!bIsInplaceOle)
            {
                SdrOle2Obj* pOleObj = dynamic_cast<SdrOle2Obj*>(_pObj);
                if (pOleObj && pOleObj->GetObjRef().is())
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
        }
    }
}

void DlgEdFunc::deactivateOle(bool _bSelect)
{
    OLEObjCache& rObjCache = GetSdrGlobalData().GetOLEObjCache();
    OReportController& rController = m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->getController();
    const sal_uLong nCount = rObjCache.size();
    for(sal_uLong i = 0 ; i< nCount;++i)
    {
        SdrOle2Obj* pObj = rObjCache[i];
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
            }
        }
    }
}

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
            m_xOverlappingObj = nullptr;
            m_pOverlappingObj = nullptr;
        }
    }
}

bool DlgEdFunc::isOverlapping(const MouseEvent& rMEvt)
{
    SdrViewEvent aVEvt;
    bool bOverlapping = m_rView.PickAnything(rMEvt, SdrMouseEventKind::BUTTONUP, aVEvt) != SDRHIT_NONE;
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
        m_pParent->Invalidate(InvalidateFlags::Children);
    }
    else
        m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
}

bool DlgEdFunc::isOnlyCustomShapeMarked()
{
    bool bReturn = true;
    const SdrMarkList& rMarkList = m_rView.GetMarkedObjectList();
    for (size_t i = 0; i < rMarkList.GetMarkCount(); ++i )
    {
        SdrMark* pMark = rMarkList.GetMark(i);
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

bool DlgEdFunc::isRectangleHit(const MouseEvent& rMEvt)
{
    if (isOnlyCustomShapeMarked())
    {
        return false;
    }

    SdrViewEvent aVEvt;
    const SdrHitKind eHit = m_rView.PickAnything(rMEvt, SdrMouseEventKind::MOVE, aVEvt);
    bool bIsSetPoint = (eHit == SDRHIT_UNMARKEDOBJECT);
    if ( !bIsSetPoint )
    {
        // no drag rect, we have to check every single select rect
        const SdrDragStat& rDragStat = m_rView.GetDragStat();
        if (rDragStat.GetDragMethod() != nullptr)
        {
            SdrObjListIter aIter(*m_pParent->getPage(),IM_DEEPNOGROUPS);
            SdrObject* pObjIter = nullptr;
            // loop through all marked objects and check if there new rect overlapps an old one.
            while( (pObjIter = aIter.Next()) != nullptr && !bIsSetPoint)
            {
                if ( m_rView.IsObjMarked(pObjIter)
                     && (dynamic_cast<OUnoObject*>(pObjIter) != nullptr || dynamic_cast<OOle2Obj*>(pObjIter) != nullptr) )
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
                    bIsSetPoint = pObjOverlapped != nullptr;
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

bool DlgEdFunc::setMovementPointer(const MouseEvent& rMEvt)
{
    bool bIsSetPoint = isRectangleHit(rMEvt);
    if ( bIsSetPoint )
        m_pParent->SetPointer( Pointer(PointerStyle::NotAllowed));
    else
    {
        bool bCtrlKey = rMEvt.IsMod1();
        (void)bCtrlKey;
        if (bCtrlKey)
        {
            m_pParent->SetPointer( Pointer(PointerStyle::MoveDataLink ));
            bIsSetPoint = true;
        }
    }
    return bIsSetPoint;
}


DlgEdFuncInsert::DlgEdFuncInsert( OReportSection* _pParent ) :
    DlgEdFunc( _pParent )
{
    m_rView.SetCreateMode();
}


DlgEdFuncInsert::~DlgEdFuncInsert()
{
    m_rView.SetEditMode();
}


bool DlgEdFuncInsert::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return true;

    SdrViewEvent aVEvt;
    sal_Int16 nId = m_rView.GetCurrentObjIdentifier();

    const SdrHitKind eHit = m_rView.PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);

    if (eHit == SDRHIT_UNMARKEDOBJECT && nId != OBJ_CUSTOMSHAPE)
    {
        // there is an object under the mouse cursor, but not a customshape
        m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
        return false;
    }

    // if no action, create object
    if (!m_pParent->getSectionWindow()->getViewsWindow()->IsAction())
    {
        deactivateOle(true);
        if ( m_pParent->getSectionWindow()->getViewsWindow()->HasSelection() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(&m_rView);
        m_rView.BegCreateObj(m_aMDPos);
        m_pParent->getSectionWindow()->getViewsWindow()->createDefault();
    }

    return true;
}


bool DlgEdFuncInsert::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonUp( rMEvt ) )
        return true;

    const Point aPos( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
    const sal_uInt16 nHitLog = sal_uInt16 ( m_pParent->PixelToLogic(Size(3,0)).Width() );

    bool bReturn = true;
    // object creation active?
    if ( m_rView.IsCreateObj() )
    {
        if ( isOver(m_rView.GetCreateObj(),*m_pParent->getPage(),m_rView) )
        {
            m_pParent->getSectionWindow()->getViewsWindow()->BrkAction();
            // BrkAction disables the create mode
            m_rView.SetCreateMode();
            return true;
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
            for (size_t i = 0; i < rMarkList.GetMarkCount(); ++i )
            {
                SdrMark* pMark = rMarkList.GetMark(i);
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
         std::abs(m_aMDPos.X() - aPos.X()) < nHitLog &&
         std::abs(m_aMDPos.Y() - aPos.Y()) < nHitLog &&
         !rMEvt.IsShift() && !rMEvt.IsMod2() )
    {
        SdrPageView* pPV = m_rView.GetSdrPageView();
        SdrViewEvent aVEvt;
        m_rView.PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
        m_rView.MarkObj(aVEvt.pRootObj, pPV);
    }
    checkTwoCklicks(rMEvt);
    m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(m_rView);
    return bReturn;
}


bool DlgEdFuncInsert::MouseMove( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseMove(rMEvt ) )
        return true;
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
        m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aPos,&m_rView, m_rView.GetDragMethod() == nullptr, false);
    }

    if ( !bIsSetPoint )
        m_pParent->SetPointer( m_rView.GetPreferredPointer( aPos, m_pParent) );

    return true;
}


DlgEdFuncSelect::DlgEdFuncSelect( OReportSection* _pParent ) :
    DlgEdFunc( _pParent )
{
}


DlgEdFuncSelect::~DlgEdFuncSelect()
{
}


bool DlgEdFuncSelect::MouseButtonDown( const MouseEvent& rMEvt )
{
    m_bSelectionMode = false;
    if ( DlgEdFunc::MouseButtonDown(rMEvt) )
        return true;

    SdrViewEvent aVEvt;
    const SdrHitKind eHit = m_rView.PickAnything(rMEvt, SdrMouseEventKind::BUTTONDOWN, aVEvt);
    if( eHit == SDRHIT_UNMARKEDOBJECT )
    {
        // if not multi selection, unmark all
        if ( !rMEvt.IsShift() )
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(nullptr);

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
            m_pParent->getSectionWindow()->getViewsWindow()->unmarkAllObjects(nullptr);

        if ( rMEvt.GetClicks() == 1 )
        {
            m_bSelectionMode = true;
            m_pParent->getSectionWindow()->getViewsWindow()->BegMarkObj( m_aMDPos ,&m_rView);
        }
        else
        {
            m_rView.SdrBeginTextEdit( aVEvt.pRootObj,m_rView.GetSdrPageView(),m_pParent );
        }
    }

    return true;
}


bool DlgEdFuncSelect::MouseButtonUp( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseButtonUp( rMEvt ) )
        return true;

    // get view from parent
    const Point aPnt( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );

    if ( rMEvt.IsLeft() )                     // left mousebutton pressed
        checkMovementAllowed(rMEvt);

    m_pParent->getSectionWindow()->getViewsWindow()->EndAction();
    checkTwoCklicks(rMEvt);

    m_pParent->SetPointer( m_rView.GetPreferredPointer( aPnt, m_pParent) );

    if ( !m_bUiActive )
        m_pParent->getSectionWindow()->getViewsWindow()->getView()->getReportView()->UpdatePropertyBrowserDelayed(m_rView);
    m_bSelectionMode = false;
    return true;
}


bool DlgEdFuncSelect::MouseMove( const MouseEvent& rMEvt )
{
    if ( DlgEdFunc::MouseMove(rMEvt ) )
        return true;

    Point aPnt( m_pParent->PixelToLogic( rMEvt.GetPosPixel() ) );
    bool bIsSetPoint = false;

    if ( m_rView.IsAction() ) // Drag Mode
    {
        bIsSetPoint = setMovementPointer(rMEvt);
        ForceScroll(aPnt);
        if (m_rView.GetDragMethod()==nullptr)
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
            }
            // drag or resize an object
            bool bControlKey = rMEvt.IsMod1();
            m_pParent->getSectionWindow()->getViewsWindow()->MovAction(aPnt, &m_rView, false, bControlKey);
        }
    }

    if ( !bIsSetPoint )
    {
        m_pParent->SetPointer( m_rView.GetPreferredPointer( aPnt, m_pParent) );

        // restore color
        unColorizeOverlappedObj();
    }

    return true;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
