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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svl/itempool.hxx>
#include <vcl/dialog.hxx>
#include <vcl/wrkwin.hxx>
#include <unotools/syslocale.hxx>
#include <rtl/math.hxx>
#include <unotools/localedatawrapper.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <svx/graphctl.hxx>
#include "GraphCtlAccessibleContext.hxx"
#include "svx/xoutbmp.hxx"
#include <svx/svxids.hrc>
#include <svx/svdpage.hxx>

// #i72889#
#include "svx/sdrpaintwindow.hxx"

/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    const SdrBaseHint* pSdrBaseHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrBaseHint)
    {
        const SdrObject* pObj = pSdrBaseHint->GetSdrHintObject();

        if(pObj)
        {
            const SdrHintKind eHint(pSdrBaseHint->GetSdrHintKind());

            switch(eHint)
    {
                case(HINT_OBJCHG_MOVE):
                case(HINT_OBJCHG_RESIZE):
                    SdrObjChanged( *pObj );
        break;

                case(HINT_OBJINSERTED):
                    SdrObjCreated( *pObj );
        break;

        default:
        break;
    }
}
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::GraphCtrl( Window* pParent, const WinBits nWinBits ) :
            Control         ( pParent, nWinBits ),
            SfxListener     ( ),
            aMap100         ( MAP_100TH_MM ),
            maSdrObjectCreationInfo(OBJ_NONE),
            nPolyEdit       ( 0 ),
            bEditMode       ( sal_False ),
            bSdrMode        ( sal_False ),
            mpAccContext    ( NULL ),
            pModel          ( NULL ),
            pView           ( NULL )
{
    aUpdateTimer.SetTimeout( 200 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateTimer.Start();

    SetWinStyle( nWinBits );

    EnableRTL( sal_False );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::GraphCtrl( Window* pParent, const ResId& rResId ) :
            Control         ( pParent, rResId ),
            SfxListener     ( ),
            aMap100         ( MAP_100TH_MM ),
            nWinStyle       ( 0 ),
            maSdrObjectCreationInfo(OBJ_NONE),
            nPolyEdit       ( 0 ),
            bEditMode       ( sal_False ),
            bSdrMode        ( sal_False ),
            bAnim           ( sal_False ),
            mpAccContext    ( NULL ),
            pModel          ( NULL ),
            pView           ( NULL )
{
    aUpdateTimer.SetTimeout( 500 );
    aUpdateTimer.SetTimeoutHdl( LINK( this, GraphCtrl, UpdateHdl ) );
    aUpdateTimer.Start();
    EnableRTL( sal_False );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

GraphCtrl::~GraphCtrl()
{
    if( mpAccContext )
    {
        mpAccContext->disposing();
        mpAccContext->release();
    }
    delete pView;
    delete pModel;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetWinStyle( WinBits nWinBits )
{
    nWinStyle = nWinBits;
    bAnim = ( nWinStyle & WB_ANIMATION ) == WB_ANIMATION;
    bSdrMode = ( nWinStyle & WB_SDRMODE ) == WB_SDRMODE;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
    SetMapMode( aMap100 );

    delete pView;
    pView = NULL;

    delete pModel;
    pModel = NULL;

    if ( bSdrMode )
        InitSdrModel();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::InitSdrModel()
{
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    SdrPage* pPage;

    // alten Kram zerstoeren
    delete pView;
    delete pModel;

    // Model anlegen
    pModel = new SdrModel;
    pModel->GetItemPool().FreezeIdRanges();
    pModel->SetExchangeObjectUnit( aMap100.GetMapUnit() );
    pModel->SetExchangeObjectScale( Fraction( 1, 1 ) );
    pModel->SetDefaultFontHeight( 500 );

    pPage = new SdrPage( *pModel );

    pPage->SetPageScale( basegfx::B2DVector(aGraphSize.Width(), aGraphSize.Height()) );
    pPage->SetPageBorder( 0, 0, 0, 0 );
    pModel->InsertPage( pPage );
    pModel->SetChanged( sal_False );

    // View anlegen
    pView = new GraphCtrlView( *pModel, this );
    pView->SetWorkArea( basegfx::B2DRange( 0.0, 0.0, aGraphSize.Width(), aGraphSize.Height() ) );
    pView->EnableExtendedMouseEventDispatcher(true);
    pView->ShowSdrPage(*pView->getSdrModelFromSdrView().GetPage(0));
    pView->SetFrameHandles(true);
    pView->SetMarkedPointsSmooth( SDRPATHSMOOTH_SYMMETRIC );
    pView->SetViewEditMode(SDREDITMODE_EDIT);

    // #i72889# set neeeded flags
    pView->SetPagePaintingAllowed(false);
    pView->SetBufferedOutputAllowed(true);
    pView->SetBufferedOverlayAllowed(true);

    // Tell the accessibility object about the changes.
    if (mpAccContext != NULL)
        mpAccContext->setModelAndView (pModel, pView);
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetGraphic( const Graphic& rGraphic, sal_Bool bNewModel )
{
    // Bitmaps dithern wir ggf. fuer die Anzeige
    if ( !bAnim && ( rGraphic.GetType() == GRAPHIC_BITMAP )  )
    {
        if ( rGraphic.IsTransparent() )
        {
            Bitmap  aBmp( rGraphic.GetBitmap() );

            DitherBitmap( aBmp );
            aGraphic = Graphic( BitmapEx( aBmp, rGraphic.GetBitmapEx().GetMask() ) );
        }
        else
        {
            Bitmap aBmp( rGraphic.GetBitmap() );
            DitherBitmap( aBmp );
            aGraphic = aBmp;
        }
    }
    else
        aGraphic = rGraphic;

    if ( aGraphic.GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
        aGraphSize = Application::GetDefaultDevice()->PixelToLogic( aGraphic.GetPrefSize(), aMap100 );
    else
        aGraphSize = OutputDevice::LogicToLogic( aGraphic.GetPrefSize(), aGraphic.GetPrefMapMode(), aMap100 );

    if ( bSdrMode && bNewModel )
        InitSdrModel();

    if ( aGraphSizeLink.IsSet() )
        aGraphSizeLink.Call( this );

    Resize();
    Invalidate();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::Resize()
{
    Control::Resize();

    if ( aGraphSize.Width() && aGraphSize.Height() )
    {
        MapMode         aDisplayMap( aMap100 );
        Point           aNewPos;
        Size            aNewSize;
        const Size      aWinSize = PixelToLogic( GetOutputSizePixel(), aDisplayMap );
        const long      nWidth = aWinSize.Width();
        const long      nHeight = aWinSize.Height();
        double          fGrfWH = (double) aGraphSize.Width() / aGraphSize.Height();
        double          fWinWH = (double) nWidth / nHeight;

        // Bitmap an Thumbgroesse anpassen
        if ( fGrfWH < fWinWH)
        {
            aNewSize.Width() = (long) ( (double) nHeight * fGrfWH );
            aNewSize.Height()= nHeight;
        }
        else
        {
            aNewSize.Width() = nWidth;
            aNewSize.Height()= (long) ( (double) nWidth / fGrfWH );
        }

        aNewPos.X() = ( nWidth - aNewSize.Width() )  >> 1;
        aNewPos.Y() = ( nHeight - aNewSize.Height() ) >> 1;

        // MapMode fuer Engine umsetzen
        aDisplayMap.SetScaleX( Fraction( aNewSize.Width(), aGraphSize.Width() ) );
        aDisplayMap.SetScaleY( Fraction( aNewSize.Height(), aGraphSize.Height() ) );

        aDisplayMap.SetOrigin( LogicToLogic( aNewPos, aMap100, aDisplayMap ) );
        SetMapMode( aDisplayMap );
    }

    Invalidate();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::Paint( const Rectangle& rRect )
{
    // #i72889# used splitted repaint to be able to paint an own background
    // even to the buffered view
    const bool bGraphicValid(GRAPHIC_NONE != aGraphic.GetType());

    if(bSdrMode)
    {
        SdrPaintWindow* pPaintWindow = pView->BeginCompleteRedraw(this);

        if(bGraphicValid)
        {
            OutputDevice& rTarget = pPaintWindow->GetTargetOutputDevice();

            rTarget.SetBackground(GetBackground());
            rTarget.Erase();

            aGraphic.Draw(&rTarget, Point(), aGraphSize);
        }

        const Region aRepaintRegion(rRect);
        pView->DoCompleteRedraw(*pPaintWindow, aRepaintRegion);
        pView->EndCompleteRedraw(*pPaintWindow, true);
    }
    else
    {
        // #i73381# in non-SdrMode, paint to local directly
        if(bGraphicValid)
        {
            aGraphic.Draw(this, Point(), aGraphSize);
        }
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SdrObjChanged( const SdrObject&  )
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SdrObjCreated( const SdrObject& )
{
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::selectionChange()
{
    if ( aMarkObjLink.IsSet() )
        aMarkObjLink.Call( this );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode( rKEvt.GetKeyCode() );
    sal_Bool    bProc = sal_False;

    switch ( aCode.GetCode() )
    {
        case KEY_DELETE:
        case KEY_BACKSPACE:
        {
            if ( bSdrMode )
            {
                pView->DeleteMarked();
                bProc = sal_True;
                if( !pView->areSdrObjectsSelected() )
                    ((Dialog*)GetParent())->GrabFocusToFirstControl();
            }
        }
        break;

        case KEY_ESCAPE:
        {
            if ( bSdrMode )
            {
                if ( pView->IsAction() )
                {
                    pView->BrkAction();
                }
                else if ( pView->areSdrObjectsSelected() )
                {
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    SdrHdl* pHdl = rHdlList.GetFocusHdl();

                    if(pHdl)
                    {
                        ((SdrHdlList&)rHdlList).ResetFocusHdl();
                    }
                    else
                    {
                        ((Dialog*)GetParent())->GrabFocusToFirstControl();
                    }
                }
                else
                {
                    ((Dialog*)GetParent())->GrabFocusToFirstControl();
                }
                bProc = sal_True;
            }
        }
        break;

        case KEY_F11:
        case KEY_TAB:
        {
            if( bSdrMode )
            {
                if( !aCode.IsMod1() && !aCode.IsMod2() )
                {
                    bool bForward = !aCode.IsShift();
                    // select next object
                    if ( ! pView->MarkNextObj( bForward ))
                    {
                        // At first or last object.  Cycle to the other end
                        // of the list.
                        pView->UnmarkAllObj();
                        pView->MarkNextObj (bForward);
                    }
                    bProc = sal_True;
                }
                else if(aCode.IsMod1())
                {
                    // select next handle
                    const SdrHdlList& rHdlList = pView->GetHdlList();
                    sal_Bool bForward(!aCode.IsShift());

                    ((SdrHdlList&)rHdlList).TravelFocusHdl(bForward);

                    bProc = true;
                }
            }
        }
        break;

        case KEY_END:
        {

            if ( aCode.IsMod1() )
            {
                // #97016# mark last object
                pView->UnmarkAllObj();
                pView->MarkNextObj(false);

                bProc = true;
            }
        }
        break;

        case KEY_HOME:
        {
            if ( aCode.IsMod1() )
            {
                pView->UnmarkAllObj();
                pView->MarkNextObj(true);

                bProc = true;
            }
        }
        break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
        {
            basegfx::B2DVector aMove(0.0, 0.0);

            if (aCode.GetCode() == KEY_UP)
            {
                // Scroll nach oben
                aMove = basegfx::B2DPoint(0.0, -1.0);
            }
            else if (aCode.GetCode() == KEY_DOWN)
            {
                // Scroll nach unten
                aMove = basegfx::B2DPoint(0.0, 1.0);
            }
            else if (aCode.GetCode() == KEY_LEFT)
            {
                // Scroll nach links
                aMove = basegfx::B2DPoint(-1.0, 0.0);
            }
            else if (aCode.GetCode() == KEY_RIGHT)
            {
                // Scroll nach rechts
                aMove = basegfx::B2DPoint(1.0, 0.0);
            }

            if (pView->areSdrObjectsSelected() && !aCode.IsMod1() )
            {
                if(aCode.IsMod2())
                {
                    // #97016# move in 1 pixel distance
                    aMove = GetInverseViewTransformation() * aMove;
                }
                else
                {
                    // old, fixed move distance
                    aMove *= 100.0;
                }

                // #97016# II
                const SdrHdlList& rHdlList = pView->GetHdlList();
                SdrHdl* pHdl = rHdlList.GetFocusHdl();

                if(0L == pHdl)
                {
                    // #90129# restrict movement to WorkArea
                    const basegfx::B2DRange& rWorkRange = pView->GetWorkArea();

                    if(!rWorkRange.isEmpty())
                    {
                        basegfx::B2DRange aMarkRange(pView->getMarkedObjectSnapRange());
                        aMarkRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aMove));

                        if(!aMarkRange.isInside(rWorkRange))
                        {
                            if(aMarkRange.getMinX() < rWorkRange.getMinX())
                            {
                                aMove.setX(aMove.getX() + rWorkRange.getMinX() - aMarkRange.getMinX());
                            }

                            if(aMarkRange.getMaxX() > rWorkRange.getMaxX())
                            {
                                aMove.setX(aMove.getX() - aMarkRange.getMaxX() - rWorkRange.getMaxX());
                            }

                            if(aMarkRange.getMinY() < rWorkRange.getMinY())
                            {
                                aMove.setY(aMove.getY() + rWorkRange.getMinY() - aMarkRange.getMinY());
                            }

                            if(aMarkRange.getMaxY() > rWorkRange.getMaxY())
                            {
                                aMove.setY(aMove.getY() - aMarkRange.getMaxY() - rWorkRange.getMaxY());
                            }
                        }
                    }

                    // no handle selected
                    if(!aMove.equalZero())
                    {
                        pView->MoveMarkedObj(aMove);
                    }
                }
                else
                {
                    // move handle with index nHandleIndex
                    if(pHdl && !aMove.equalZero())
                    {
                        // now move the Handle
                        const basegfx::B2DPoint aStartPoint(pHdl->getPosition());
                        const basegfx::B2DPoint aEndPoint(aStartPoint + aMove);
                        const SdrDragStat& rDragStat = pView->GetDragStat();

                        // start dragging
                        pView->BegDragObj(aStartPoint, pHdl, 0.0);

                        if(pView->IsDragObj())
                        {
                            const bool bWasNoSnap(rDragStat.IsNoSnap());
                            const bool bWasSnapEnabled(pView->IsSnapEnabled());

                            // switch snapping off
                            if(!bWasNoSnap)
                                ((SdrDragStat&)rDragStat).SetNoSnap(true);
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(false);

                            pView->MovAction(aEndPoint);
                            pView->EndDragObj();

                            // restore snap
                            if(!bWasNoSnap)
                                ((SdrDragStat&)rDragStat).SetNoSnap(bWasNoSnap);
                            if(bWasSnapEnabled)
                                pView->SetSnapEnabled(bWasSnapEnabled);
                        }
                    }
                }

                bProc = true;
            }
        }
        break;

        case KEY_SPACE:
        {
            const SdrHdlList& rHdlList = pView->GetHdlList();
            SdrHdl* pHdl = rHdlList.GetFocusHdl();

            if(pHdl)
            {
                if(pHdl->GetKind() == HDL_POLY)
                {
                    // rescue ID of point with focus
                    sal_uInt32 nPol(pHdl->GetPolyNum());
                    sal_uInt32 nPnt(pHdl->GetPointNum());

                    if(pView->IsPointMarked(*pHdl))
                    {
                        if(rKEvt.GetKeyCode().IsShift())
                        {
                            pView->MarkPoint(*pHdl, true); // unmark
                        }
                    }
                    else
                    {
                        if(!rKEvt.GetKeyCode().IsShift())
                        {
                            pView->MarkPoints(0, true); // unmarkall
                        }

                        pView->MarkPoint(*pHdl);
                    }

                    if(0L == rHdlList.GetFocusHdl())
                    {
                        // restore point with focus
                        SdrHdl* pNewOne = 0L;

                        for(sal_uInt32 a(0); !pNewOne && a < rHdlList.GetHdlCount(); a++)
                        {
                            SdrHdl* pAct = rHdlList.GetHdlByIndex(a);

                            if(pAct
                                && pAct->GetKind() == HDL_POLY
                                && pAct->GetPolyNum() == nPol
                                && pAct->GetPointNum() == nPnt)
                            {
                                pNewOne = pAct;
                            }
                        }

                        if(pNewOne)
                        {
                            ((SdrHdlList&)rHdlList).SetFocusHdl(pNewOne);
                        }
                    }

                    bProc = sal_True;
                }
            }
        }
        break;

        default:
        break;
    }

    if ( !bProc )
        Control::KeyInput( rKEvt );
    else
        ReleaseMouse();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( bSdrMode && ( rMEvt.GetClicks() < 2 ) )
    {
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogPt(GetInverseViewTransformation() * aPixelPos);

        if ( !basegfx::B2DRange( 0.0, 0.0, aGraphSize.Width(), aGraphSize.Height() ).isInside( aLogPt ) && !pView->IsEditMode() )
            Control::MouseButtonDown( rMEvt );
        else
        {
            // Focus anziehen fuer Key-Inputs
            GrabFocus();

            if ( nPolyEdit )
            {
                SdrViewEvent    aVEvt;
                SdrHitKind      eHit = pView->PickAnything( rMEvt, SDRMOUSEBUTTONDOWN, aVEvt );

                if ( nPolyEdit == SID_BEZIER_INSERT && eHit == SDRHIT_MARKEDOBJECT )
                    pView->BegInsObjPoint( aLogPt, rMEvt.IsMod1());
                else
                    pView->MouseButtonDown( rMEvt, this );
            }
            else
                pView->MouseButtonDown( rMEvt, this );
        }

        SdrObject* pCreateObj = pView->GetCreateObj();

        // Wir wollen das Inserten mitbekommen
        ConnectToSdrObject(pCreateObj);

        SetPointer( pView->GetPreferedPointer( aLogPt, this ) );
    }
    else
        Control::MouseButtonDown( rMEvt );
}

void GraphCtrl::ConnectToSdrObject(SdrObject* pObject)
{
    if(pObject)
    {
        GraphCtrl* pAlreadyConnected = 0;

        if(pObject->HasListeners())
        {
            const sal_uInt16 nListenerCount(pObject->GetListenerCount());

            for(sal_uInt16 a(0); !pAlreadyConnected && a < nListenerCount; a++)
            {
                SfxListener* pCandidate = pObject->GetListener(a);

                if(pCandidate) // not all slots in a broadcaster have to be used
                {
                    GraphCtrl* pCurrent = dynamic_cast< GraphCtrl* >(pCandidate);

                    if(pCurrent && pCurrent == this)
                    {
                        pAlreadyConnected = pCurrent;
                    }
                }
            }
        }

        if(!pAlreadyConnected)
        {
            StartListening(*pObject);
        }
    }
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MouseMove(const MouseEvent& rMEvt)
{
    const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
    const basegfx::B2DPoint aLogPos(GetInverseViewTransformation() * aPixelPos);

    if ( bSdrMode )
    {
        pView->MouseMove( rMEvt, this );

        if( ( SID_BEZIER_INSERT == nPolyEdit ) &&
            !pView->PickHandle( aLogPos ) &&
            !pView->IsInsObjPoint() )
        {
            SetPointer( POINTER_CROSS );
        }
        else
            SetPointer( pView->GetPreferedPointer( aLogPos, this ) );
    }
    else
        Control::MouseButtonUp( rMEvt );

    if ( aMousePosLink.IsSet() )
    {
        if ( basegfx::B2DRange( 0.0, 0.0, aGraphSize.Width(), aGraphSize.Height() ).isInside( aLogPos ) )
            aMousePos = Point(basegfx::fround(aLogPos.getX()), basegfx::fround(aLogPos.getY()));
        else
            aMousePos = Point();

        aMousePosLink.Call( this );
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::MouseButtonUp(const MouseEvent& rMEvt)
{
    if ( bSdrMode )
    {
        if ( pView->IsInsObjPoint() )
            pView->EndInsObjPoint( SDRCREATE_FORCEEND );
        else
            pView->MouseButtonUp( rMEvt, this );

        ReleaseMouse();
        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        const basegfx::B2DPoint aLogPos(GetInverseViewTransformation() * aPixelPos);

        SetPointer( pView->GetPreferedPointer(aLogPos, this ) );
    }
    else
        Control::MouseButtonUp( rMEvt );
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SdrObject* GraphCtrl::GetSelectedSdrObject() const
{
    SdrObject* pSdrObj = NULL;

    if ( bSdrMode )
    {
        pSdrObj = pView->getSelectedIfSingle();
    }

    return pSdrObj;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetEditMode( const sal_Bool _bEditMode )
{
    if ( bSdrMode )
    {
        bEditMode = _bEditMode;
        pView->SetViewEditMode(bEditMode ? SDREDITMODE_EDIT : SDREDITMODE_CREATE);
        maSdrObjectCreationInfo.setIdent(OBJ_NONE);
        pView->setSdrObjectCreationInfo(maSdrObjectCreationInfo);
    }
    else
        bEditMode = sal_False;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::SetPolyEditMode( const sal_uInt16 _nPolyEdit )
{
    if ( bSdrMode && ( _nPolyEdit != nPolyEdit ) )
    {
        nPolyEdit = _nPolyEdit;
        pView->SetFrameHandles( nPolyEdit == 0 );
    }
    else
        nPolyEdit = 0;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

void GraphCtrl::setSdrObjectCreationInfo( const SdrObjectCreationInfo& rSdrObjectCreationInfo )
{
    if ( bSdrMode )
    {
        bEditMode = false;
        pView->SetViewEditMode(SDREDITMODE_CREATE);
        maSdrObjectCreationInfo = rSdrObjectCreationInfo;
        pView->setSdrObjectCreationInfo(maSdrObjectCreationInfo);
    }
    else
    {
        maSdrObjectCreationInfo.setIdent(OBJ_NONE);
    }
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

String GraphCtrl::GetStringFromDouble( const double& rDouble )
{
    sal_Unicode cSep =
        SvtSysLocale().GetLocaleData().getNumDecimalSep().GetChar(0);
    String aStr( ::rtl::math::doubleToUString( rDouble,
                rtl_math_StringFormat_F, 2, cSep ));
    return aStr;
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

IMPL_LINK( GraphCtrl, UpdateHdl, Timer*, pTimer )
{
    if ( aUpdateLink.IsSet() )
        aUpdateLink.Call( this );

    pTimer->Start();

    return 0L;
}


::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GraphCtrl::CreateAccessible()
{
    if( mpAccContext == NULL )
    {
        Window* pParent = GetParent();

        DBG_ASSERT( pParent, "-GraphCtrl::CreateAccessible(): No Parent!" );

        if( pParent )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccParent( pParent->GetAccessible() );

            // #103856# Disable accessibility if no model/view data available
            if( pView &&
                pModel &&
                xAccParent.is() )
            {
                mpAccContext = new SvxGraphCtrlAccessibleContext( xAccParent, *this );
                mpAccContext->acquire();
            }
        }
    }

    return mpAccContext;
}

void GraphCtrlView::handleSelectionChange()
{
    // call parent
    SdrView::handleSelectionChange();

    // local reactions
    rGraphCtrl.selectionChange();
}
