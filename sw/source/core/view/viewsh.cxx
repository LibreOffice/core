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


#define _SVX_PARAITEM_HXX
#define _SVX_TEXTITEM_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>

#include <sfx2/viewfrm.hxx>
#include <sfx2/progress.hxx>
#include <svx/srchdlg.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/viewsh.hxx>
#include <swwait.hxx>
#include <swmodule.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <cntfrm.hxx>
#include <viewimp.hxx>
#include <frmtool.hxx>
#include <viewopt.hxx>
#include <dview.hxx>
#include <swregion.hxx>
#include <hints.hxx>
#include <fmtfsize.hxx>
#include <docufld.hxx>
#include <txtfrm.hxx>
#include <layact.hxx>
#include <mdiexp.hxx>
#include <fntcache.hxx>
#include <ptqueue.hxx>
#include <tabfrm.hxx>
#include <docsh.hxx>
#include <pagedesc.hxx>
#include <ndole.hxx>
#include <ndindex.hxx>
#include <accmap.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <accessibilityoptions.hxx>
#include <statstr.hrc>
#include <comcore.hrc>
#include <pagepreviewlayout.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>

#include "../../ui/inc/view.hxx"
#include <PostItMgr.hxx>
#include <vcl/virdev.hxx>

#include <vcl/svapp.hxx>

#include <svx/sdrpaintwindow.hxx>

sal_Bool ViewShell::bLstAct = sal_False;
ShellResource *ViewShell::pShellRes = 0;
Window *ViewShell::pCareWindow = 0;
BitmapEx* ViewShell::pErrorBmp = NULL;
BitmapEx* ViewShell::pReplaceBmp = NULL;

sal_Bool bInSizeNotify = sal_False;

DBG_NAME(LayoutIdle)

TYPEINIT0(ViewShell);

using namespace ::com::sun::star;

void ViewShell::ToggleHeaderFooterEdit()
{
    bHeaderFooterEdit = !bHeaderFooterEdit;
    if ( !bHeaderFooterEdit )
    {
        SetShowHeaderFooterSeparator( Header, false );
        SetShowHeaderFooterSeparator( Footer, false );
    }

    // Avoid corner case
    if ( !IsShowHeaderFooterSeparator( Header ) &&
         !IsShowHeaderFooterSeparator( Footer ) )
    {
        bHeaderFooterEdit = false;
    }

    // Repaint everything
    GetWin()->Invalidate();
}

//////////////////////////////////////////////////////////////////////////////
// #i72754# 2nd set of Pre/PostPaints
// This time it uses the lock counter (mPrePostPaintRegions empty/non-empty) to allow only one activation
// and deactivation and mpPrePostOutDev to remember the OutDev from the BeginDrawLayers
// call. That way, all places where paint take place can be handled the same way, even
// when calling other paint methods. This is the case at the places where SW paints
// buffered into VDevs to avoid flicker. Tis is in general problematic and should be
// solved once using the BufferedOutput functionality of the DrawView.

void ViewShell::PrePaint()
{
    // forward PrePaint event from VCL Window to DrawingLayer
    if(HasDrawView())
    {
        Imp()->GetDrawView()->PrePaint();
    }
}

void ViewShell::DLPrePaint2(const Region& rRegion)
{
    if(mPrePostPaintRegions.empty())
    {
        mPrePostPaintRegions.push( rRegion );
        // #i75172# ensure DrawView to use DrawingLayer bufferings
        if ( !HasDrawView() )
            MakeDrawView();

        // Prefer window; if tot available, get pOut (e.g. printer)
        mpPrePostOutDev = (GetWin() ? GetWin() : GetOut());

        // #i74769# use SdrPaintWindow now direct
        mpTargetPaintWindow = Imp()->GetDrawView()->BeginDrawLayers(mpPrePostOutDev, rRegion);
        OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

        // #i74769# if prerender, save OutDev and redirect to PreRenderDevice
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            mpBufferedOut = pOut;
            pOut = &(mpTargetPaintWindow->GetTargetOutputDevice());
        }

        // remember original paint MapMode for wrapped FlyFrame paints
        maPrePostMapMode = pOut->GetMapMode();
    }
    else
    {
        // region needs to be updated to the given one
        if( mPrePostPaintRegions.top() != rRegion )
            Imp()->GetDrawView()->UpdateDrawLayersRegion(mpPrePostOutDev, rRegion);
        mPrePostPaintRegions.push( rRegion );
    }
}

void ViewShell::DLPostPaint2(bool bPaintFormLayer)
{
    OSL_ENSURE(!mPrePostPaintRegions.empty(), "ViewShell::DLPostPaint2: Pre/PostPaint encapsulation broken (!)");

    if( mPrePostPaintRegions.size() > 1 )
    {
        Region current = mPrePostPaintRegions.top();
        mPrePostPaintRegions.pop();
        if( current != mPrePostPaintRegions.top())
            Imp()->GetDrawView()->UpdateDrawLayersRegion(mpPrePostOutDev, mPrePostPaintRegions.top());
        return;
    }
    mPrePostPaintRegions.pop(); // clear
    if(0 != mpTargetPaintWindow)
    {
        // #i74769# restore buffered OutDev
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            pOut = mpBufferedOut;
        }

        // #i74769# use SdrPaintWindow now direct
        Imp()->GetDrawView()->EndDrawLayers(*mpTargetPaintWindow, bPaintFormLayer);
        mpTargetPaintWindow = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////


void ViewShell::ImplEndAction( const sal_Bool bIdleEnd )
{
    // Nothing to do for the printer?
    if ( !GetWin() || IsPreView() )
    {
        bPaintWorks = sal_True;
        UISizeNotify();
        return;
    }

    bInEndAction = sal_True;
    //Laeuft hiermit das EndAction der Letzten Shell im Ring?

    ViewShell::bLstAct = sal_True;
    ViewShell *pSh = (ViewShell*)this->GetNext();
    while ( pSh != this )
    {
        if ( pSh->ActionPend() )
        {
            ViewShell::bLstAct = sal_False;
            pSh = this;
        }
        else
            pSh = (ViewShell*)pSh->GetNext();
    }

    const bool bIsShellForCheckViewLayout = ( this == GetLayout()->GetCurrShell() );

    SET_CURR_SHELL( this );
    if ( Imp()->HasDrawView() && !Imp()->GetDrawView()->areMarkHandlesHidden() )
        Imp()->StartAction();

    if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegion();

    const sal_Bool bExtraData = ::IsExtraData( GetDoc() );

    if ( !bIdleEnd )
    {
        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( sal_False );
        if ( nLockPaint )
            aAction.SetPaint( sal_False );
        aAction.SetInputType( VCL_INPUT_KEYBOARD );
        aAction.Action();
    }

    if ( bIsShellForCheckViewLayout )
        GetLayout()->CheckViewLayout( GetViewOptions(), &aVisArea );

    //Wenn wir selbst keine Paints erzeugen, so warten wir auf das Paint
    //vom System. Dann ist das Clipping korrekt gesetzt; Beispiel: verschieben
    //eines DrawObjektes.
    if ( Imp()->GetRegion()     ||
         aInvalidRect.HasArea() ||
         bExtraData )
    {
        if ( !nLockPaint )
        {
            SolarMutexGuard aGuard;

            sal_Bool bPaintsFromSystem = aInvalidRect.HasArea();
            GetWin()->Update();
            if ( aInvalidRect.HasArea() )
            {
                if ( bPaintsFromSystem )
                    Imp()->AddPaintRect( aInvalidRect );

                ResetInvalidRect();
                bPaintsFromSystem = sal_True;
            }
            bPaintWorks = sal_True;

            SwRegionRects *pRegion = Imp()->GetRegion();

            //JP 27.11.97: wer die Selection hided, muss sie aber auch
            //              wieder Showen. Sonst gibt es Paintfehler!
            //  z.B.: addional Mode, Seite vertikal hab zu sehen, in der
            // Mitte eine Selektion und mit einem anderen Cursor an linken
            // rechten Rand springen. Ohne ShowCrsr verschwindet die
            // Selektion
            sal_Bool bShowCrsr = pRegion && IsA( TYPE(SwCrsrShell) );
            if( bShowCrsr )
                ((SwCrsrShell*)this)->HideCrsrs();

            if ( pRegion )
            {
                SwRootFrm* pCurrentLayout = GetLayout();

                Imp()->pRegion = NULL;

                //Erst Invert dann Compress, niemals andersherum!
                pRegion->Invert();

                pRegion->Compress();

                VirtualDevice *pVout = 0;
                while ( !pRegion->empty() )
                {
                    SwRect aRect( pRegion->back() );
                    pRegion->pop_back();

                    sal_Bool bPaint = sal_True;
                    if ( IsEndActionByVirDev() )
                    {
                        //virtuelles device erzeugen und einstellen.
                        if ( !pVout )
                            pVout = new VirtualDevice( *GetOut() );
                        MapMode aMapMode( GetOut()->GetMapMode() );
                        pVout->SetMapMode( aMapMode );

                        sal_Bool bSizeOK = sal_True;

                        Rectangle aTmp1( aRect.SVRect() );
                        aTmp1 = GetOut()->LogicToPixel( aTmp1 );
                        Rectangle aTmp2( GetOut()->PixelToLogic( aTmp1 ) );
                        if ( aTmp2.Left() > aRect.Left() )
                            aTmp1.Left() = Max( 0L, aTmp1.Left() - 1L );
                        if ( aTmp2.Top() > aRect.Top() )
                            aTmp1.Top() = Max( 0L, aTmp1.Top() - 1L );
                        aTmp1.Right() += 1;
                        aTmp1.Bottom() += 1;
                        aTmp1 = GetOut()->PixelToLogic( aTmp1 );
                        aRect = SwRect( aTmp1 );

                        const Size aTmp( pVout->GetOutputSize() );
                        if ( aTmp.Height() < aRect.Height() ||
                             aTmp.Width()  < aRect.Width() )
                        {
                            bSizeOK = pVout->SetOutputSize( aRect.SSize() );
                        }
                        if ( bSizeOK )
                        {
                            bPaint = sal_False;

                            // --> OD 2007-07-26 #i79947#
                            // #i72754# start Pre/PostPaint encapsulation before pOut is changed to the buffering VDev
                            const Region aRepaintRegion(aRect.SVRect());
                            DLPrePaint2(aRepaintRegion);
                            // <--

                            OutputDevice  *pOld = GetOut();
                            pVout->SetLineColor( pOld->GetLineColor() );
                            pVout->SetFillColor( pOld->GetFillColor() );
                            Point aOrigin( aRect.Pos() );
                            aOrigin.X() = -aOrigin.X(); aOrigin.Y() = -aOrigin.Y();
                            aMapMode.SetOrigin( aOrigin );
                            pVout->SetMapMode( aMapMode );

                            pOut = pVout;
                            if ( bPaintsFromSystem )
                                PaintDesktop( aRect );
                            pCurrentLayout->Paint( aRect );
                            pOld->DrawOutDev( aRect.Pos(), aRect.SSize(),
                                              aRect.Pos(), aRect.SSize(), *pVout );
                            pOut = pOld;

                            // #i72754# end Pre/PostPaint encapsulation when pOut is back and content is painted
                            DLPostPaint2(true);
                        }
                    }
                    if ( bPaint )
                    {
                        // #i75172# begin DrawingLayer paint
                        // need to do begin/end DrawingLayer preparation for each single rectangle of the
                        // repaint region. I already tried to prepare only once for the whole Region. This
                        // seems to work (and does technically) but fails with transparent objects. Since the
                        // region given to BeginDarwLayers() defines the clip region for DrawingLayer paint,
                        // transparent objects in the single rectangles will indeed be painted multiple times.
                        DLPrePaint2(Region(aRect.SVRect()));

                        if ( bPaintsFromSystem )
                            PaintDesktop( aRect );
                        pCurrentLayout->Paint( aRect );

                        // #i75172# end DrawingLayer paint
                        DLPostPaint2(true);
                    }

                    // #i107365#
                    // Direct paint has been performed. Thus, take care of
                    // transparent child windows.
                    if ( GetWin() )
                    {
                        Window& rWindow = *(GetWin());
                        if (rWindow.IsChildTransparentModeEnabled())
                        {
                            Window* pCandidate = rWindow.GetWindow( WINDOW_FIRSTCHILD );
                            if (pCandidate)
                            {
                                const Rectangle aRectanglePixel(rWindow.LogicToPixel(aRect.SVRect()));

                                while (pCandidate)
                                {
                                    if ( pCandidate->IsPaintTransparent() )
                                    {
                                        const Rectangle aCandidatePosSizePixel(
                                                        pCandidate->GetPosPixel(),
                                                        pCandidate->GetSizePixel());

                                        if ( aCandidatePosSizePixel.IsOver(aRectanglePixel) )
                                        {
                                            pCandidate->Invalidate( INVALIDATE_NOTRANSPARENT|INVALIDATE_CHILDREN );
                                            pCandidate->Update();
                                        }
                                    }

                                    pCandidate = pCandidate->GetWindow( WINDOW_NEXT );
                                }
                            }
                        }
                    }
                }

                delete pVout;
                delete pRegion;
                Imp()->DelRegion();
            }
            if( bShowCrsr )
                ((SwCrsrShell*)this)->ShowCrsrs( sal_True );
        }
        else
        {
            Imp()->DelRegion();
            bPaintWorks =  sal_True;
        }
    }
    else
        bPaintWorks = sal_True;

    bInEndAction = sal_False;
    ViewShell::bLstAct = sal_False;
    Imp()->EndAction();


    //Damit sich die automatischen Scrollbars auch richtig anordnen k?nnen
    //muessen wir die Aktion hier kuenstlich beenden (EndAction loesst ein
    //Notify aus, und das muss Start-/EndAction rufen um die  Scrollbars
    //klarzubekommen.
    --nStartAction;
    UISizeNotify();
    ++nStartAction;

    if( Imp()->IsAccessible() )
        Imp()->FireAccessibleEvents();
}


void ViewShell::ImplStartAction()
{
    bPaintWorks = sal_False;
    Imp()->StartAction();
}


void ViewShell::ImplLockPaint()
{
    if ( GetWin() && GetWin()->IsVisible() )
        GetWin()->EnablePaint( sal_False ); //Auch die Controls abklemmen.
    Imp()->LockPaint();
}


void ViewShell::ImplUnlockPaint( sal_Bool bVirDev )
{
    SET_CURR_SHELL( this );
    if ( GetWin() && GetWin()->IsVisible() )
    {
        if ( (bInSizeNotify || bVirDev ) && VisArea().HasArea() )
        {
            //Refresh mit virtuellem Device um das Flackern zu verhindern.
            VirtualDevice *pVout = new VirtualDevice( *pOut );
            pVout->SetMapMode( pOut->GetMapMode() );
            Size aSize( VisArea().SSize() );
            aSize.Width() += 20;
            aSize.Height()+= 20;
            if( pVout->SetOutputSize( aSize ) )
            {
                GetWin()->EnablePaint( sal_True );
                GetWin()->Validate();

                Imp()->UnlockPaint();
                pVout->SetLineColor( pOut->GetLineColor() );
                pVout->SetFillColor( pOut->GetFillColor() );

                // #i72754# start Pre/PostPaint encapsulation before pOut is changed to the buffering VDev
                const Region aRepaintRegion(VisArea().SVRect());
                DLPrePaint2(aRepaintRegion);

                OutputDevice *pOld = pOut;
                pOut = pVout;
                Paint( VisArea().SVRect() );
                pOut = pOld;
                pOut->DrawOutDev( VisArea().Pos(), aSize,
                                  VisArea().Pos(), aSize, *pVout );

                // #i72754# end Pre/PostPaint encapsulation when pOut is back and content is painted
                DLPostPaint2(true);
            }
            else
            {
                Imp()->UnlockPaint();
                GetWin()->EnablePaint( sal_True );
                GetWin()->Invalidate( INVALIDATE_CHILDREN );
            }
            delete pVout;
        }
        else
        {
            Imp()->UnlockPaint();
            GetWin()->EnablePaint( sal_True );
            GetWin()->Invalidate( INVALIDATE_CHILDREN );
        }
    }
    else
        Imp()->UnlockPaint();
}


sal_Bool ViewShell::AddPaintRect( const SwRect & rRect )
{
    sal_Bool bRet = sal_False;
    ViewShell *pSh = this;
    do
    {
        if( pSh->Imp() )
        {
        if ( pSh->IsPreView() && pSh->GetWin() )
            ::RepaintPagePreview( pSh, rRect );
        else
                bRet |= pSh->Imp()->AddPaintRect( rRect );
        }
        pSh = (ViewShell*)pSh->GetNext();
    } while ( pSh != this );
    return bRet;
}


void ViewShell::InvalidateWindows( const SwRect &rRect )
{
    if ( !Imp()->IsCalcLayoutProgress() )
    {
        ViewShell *pSh = this;
        do
        {
            if ( pSh->GetWin() )
            {
                if ( pSh->IsPreView() )
                    ::RepaintPagePreview( pSh, rRect );
                else if ( pSh->VisArea().IsOver( rRect ) )
                    pSh->GetWin()->Invalidate( rRect.SVRect() );
            }
            pSh = (ViewShell*)pSh->GetNext();

        } while ( pSh != this );
    }
}


void ViewShell::MakeVisible( const SwRect &rRect )
{
    if ( !VisArea().IsInside( rRect ) || IsScrollMDI( this, rRect ) || GetCareWin(*this) )
    {
        if ( !IsViewLocked() )
        {
            if( pWin )
            {
                const SwFrm* pRoot = GetLayout();
                int nLoopCnt = 3;
                long nOldH;
                do{
                    nOldH = pRoot->Frm().Height();
                    StartAction();
                    ScrollMDI( this, rRect, USHRT_MAX, USHRT_MAX );
                    EndAction();
                } while( nOldH != pRoot->Frm().Height() && nLoopCnt-- );    //swmod 071108//swmod 071225
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                //MA: 04. Nov. 94, braucht doch keiner oder??
                OSL_ENSURE( !this, "MakeVisible fuer Drucker wird doch gebraucht?" );
            }

#endif
        }
    }
}


Window* ViewShell::CareChildWin(ViewShell& rVSh)
{
    if(rVSh.pSfxViewShell)
    {
        const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = rVSh.pSfxViewShell->GetViewFrame();
        const SfxChildWindow* pChWin = pVFrame->GetChildWindow( nId );
        Window *pWin = pChWin ? pChWin->GetWindow() : NULL;
        if ( pWin && pWin->IsVisible() )
            return pWin;
    }
    return NULL;
}


Point ViewShell::GetPagePos( sal_uInt16 nPageNum ) const
{
    return GetLayout()->GetPagePos( nPageNum );
}


sal_uInt16 ViewShell::GetNumPages()
{
    //Es kann sein, das noch kein Layout existiert weil die Methode vom
    //Root-Ctor gerufen wird.
    return GetLayout() ? GetLayout()->GetPageNum() : 0;
}

sal_Bool ViewShell::IsDummyPage( sal_uInt16 nPageNum ) const
{
    return GetLayout() ? GetLayout()->IsDummyPage( nPageNum ) : 0;
}

/**
 * Forces update of each field.
 * It notifies all fields with pNewHt. If that is 0 (default), the field
 * type is sent (???).
 * @param[in] bCloseDB Passed in to GetDoc()->UpdateFlds. [TODO] Purpose???
 */
void ViewShell::UpdateFlds(sal_Bool bCloseDB)
{
    SET_CURR_SHELL( this );

    sal_Bool bCrsr = ISA(SwCrsrShell);
    if ( bCrsr )
        ((SwCrsrShell*)this)->StartAction();
    else
        StartAction();

    GetDoc()->UpdateFlds(0, bCloseDB);

    if ( bCrsr )
        ((SwCrsrShell*)this)->EndAction();
    else
        EndAction();
}

/** update all charts for which any table exists */
void ViewShell::UpdateAllCharts()
{
    SET_CURR_SHELL( this );
    // Start-/EndAction handled in the SwDoc-Method!
    GetDoc()->UpdateAllCharts();
}

sal_Bool ViewShell::HasCharts() const
{
    sal_Bool bRet = sal_False;
    const SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetDoc()->GetNodes().GetEndOfAutotext().
                        StartOfSectionNode(), 1 );
    while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        const SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && pNd->GetChartTblName().Len() )
        {
            bRet = sal_True;
            break;
        }
    }
    return bRet;
}


void ViewShell::LayoutIdle()
{
#ifdef TCOVER
    //fuer TCV-Version: Ende der Startphase des Programmes
    TCovCall::Idle();
#endif
    if( !pOpt->IsIdle() || !GetWin() ||
        ( Imp()->HasDrawView() && Imp()->GetDrawView()->IsDragObj() ) )
        return;

    //Kein Idle wenn gerade gedruckt wird.
    ViewShell *pSh = this;
    do
    {   if ( !pSh->GetWin() )
            return;
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != this );

    SET_CURR_SHELL( this );

#ifdef DBG_UTIL
    // Wenn Test5 gedrueckt ist, wird der IdleFormatierer abgeknipst.
    if( pOpt->IsTest5() )
        return;
#endif

    {
        DBG_PROFSTART( LayoutIdle );

        //Cache vorbereiten und restaurieren, damit er nicht versaut wird.
        SwSaveSetLRUOfst aSave( *SwTxtFrm::GetTxtCache(),
                             SwTxtFrm::GetTxtCache()->GetCurMax() - 50 );
        // #125243# there are lots of stacktraces indicating that Imp() returns NULL
        // this ViewShell seems to be invalid - but it's not clear why
        // this return is only a workaround!
        OSL_ENSURE(Imp(), "ViewShell already deleted?");
        if(!Imp())
            return;
        SwLayIdle aIdle( GetLayout(), Imp() );
        DBG_PROFSTOP( LayoutIdle );
    }
}


static void lcl_InvalidateAllCntnt( ViewShell& rSh, sal_uInt8 nInv )
{
    sal_Bool bCrsr = rSh.ISA(SwCrsrShell);
    if ( bCrsr )
        ((SwCrsrShell&)rSh).StartAction();
    else
        rSh.StartAction();
    rSh.GetLayout()->InvalidateAllCntnt( nInv );
    if ( bCrsr )
        ((SwCrsrShell&)rSh).EndAction();
    else
        rSh.EndAction();

    rSh.GetDoc()->SetModified();
}

/** local method to invalidate/re-calculate positions of floating screen
 *  objects (Writer fly frame and drawing objects), which are anchored
 *  to paragraph or to character. #i11860#
 */
static void lcl_InvalidateAllObjPos( ViewShell &_rSh )
{
    const bool bIsCrsrShell = _rSh.ISA(SwCrsrShell);
    if ( bIsCrsrShell )
        static_cast<SwCrsrShell&>(_rSh).StartAction();
    else
        _rSh.StartAction();

    _rSh.GetLayout()->InvalidateAllObjPos();

    if ( bIsCrsrShell )
        static_cast<SwCrsrShell&>(_rSh).EndAction();
    else
        _rSh.EndAction();

    _rSh.GetDoc()->SetModified();
}

void ViewShell::SetParaSpaceMax( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::PARA_SPACE_MAX, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

void ViewShell::SetParaSpaceMaxAtPages( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

void ViewShell::SetTabCompat( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::TAB_COMPAT) != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::TAB_COMPAT, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void ViewShell::SetAddExtLeading( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::ADD_EXT_LEADING) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::ADD_EXT_LEADING, bNew );
        SdrModel* pTmpDrawModel = getIDocumentDrawModelAccess()->GetDrawModel();
        if ( pTmpDrawModel )
            pTmpDrawModel->SetAddExtLeading( bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void ViewShell::SetUseVirDev( bool bNewVirtual )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) != bNewVirtual )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        // this sets the flag at the document and calls PrtDataChanged
        IDocumentDeviceAccess* pIDDA = getIDocumentDeviceAccess();
        pIDDA->setReferenceDeviceType( bNewVirtual, true );
    }
}

/** Sets if paragraph and table spacing is added at bottom of table cells.
 * #106629#
 * @param[in] (bool) setting of the new value
 */
void ViewShell::SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) != _bAddParaSpacingToTableCells )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, _bAddParaSpacingToTableCells );
        const sal_uInt8 nInv = INV_PRTAREA;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

/**
 * Sets if former formatting of text lines with proportional line spacing should used.
 * #i11859#
 * @param[in] (bool) setting of the new value
 */
void ViewShell::SetUseFormerLineSpacing( bool _bUseFormerLineSpacing )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING) != _bUseFormerLineSpacing )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::OLD_LINE_SPACING, _bUseFormerLineSpacing );
        const sal_uInt8 nInv = INV_PRTAREA;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}


/**
 * Sets IDocumentSettingAccess if former object positioning should be used.
 * #i11860#
 * @param[in] (bool) setting the new value
 */
void ViewShell::SetUseFormerObjectPositioning( bool _bUseFormerObjPos )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) != _bUseFormerObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, _bUseFormerObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}


// #i28701#
void ViewShell::SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) != _bConsiderWrapOnObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, _bConsiderWrapOnObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}


void ViewShell::SetUseFormerTextWrapping( bool _bUseFormerTextWrapping )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) != _bUseFormerTextWrapping )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, _bUseFormerTextWrapping );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}


// #i45491#
void ViewShell::SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK) != _bDoNotJustifyLinesWithManualBreak )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        pIDSA->set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, _bDoNotJustifyLinesWithManualBreak );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}


void ViewShell::Reformat()
{
    SwWait aWait( *GetDoc()->GetDocShell(), sal_True );

    // Wir gehen auf Nummer sicher:
    // Wir muessen die alten Fontinformationen wegschmeissen,
    // wenn die Druckeraufloesung oder der Zoomfaktor sich aendert.
    // Init() und Reformat() sind die sichersten Stellen.
#ifdef FNTMET
    aFntMetList.Flush();
#else
    pFntCache->Flush( );
#endif

    if( GetLayout()->IsCallbackActionEnabled() )
    {

        StartAction();
        GetLayout()->InvalidateAllCntnt( INV_SIZE | INV_POS | INV_PRTAREA );
        EndAction();
    }
}

 void ViewShell::ChgNumberDigits()
 {
     SdrModel* pTmpDrawModel = getIDocumentDrawModelAccess()->GetDrawModel();
     if ( pTmpDrawModel )
            pTmpDrawModel->ReformatAllTextObjects();
     Reformat();
 }


void ViewShell::CalcLayout()
{
    SET_CURR_SHELL( this );
    SwWait aWait( *GetDoc()->GetDocShell(), sal_True );

    //Cache vorbereiten und restaurieren, damit er nicht versaut wird.
    SwSaveSetLRUOfst aSaveLRU( *SwTxtFrm::GetTxtCache(),
                                  SwTxtFrm::GetTxtCache()->GetCurMax() - 50 );

    //Progress einschalten wenn noch keiner Lauft.
    const sal_Bool bEndProgress = SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) == 0;
    if ( bEndProgress )
    {
        sal_uInt16 nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        ::StartProgress( STR_STATSTR_REFORMAT, 0, nEndPage, GetDoc()->GetDocShell() );
    }

    SwLayAction aAction( GetLayout(), Imp() );
    aAction.SetPaint( sal_False );
    aAction.SetStatBar( sal_True );
    aAction.SetCalcLayout( sal_True );
    aAction.SetReschedule( sal_True );
    GetDoc()->LockExpFlds();
    aAction.Action();
    GetDoc()->UnlockExpFlds();

    //Das SetNewFldLst() am Doc wurde unterbunden und muss nachgeholt
    //werden (siehe flowfrm.cxx, txtfld.cxx)
    if ( aAction.IsExpFlds() )
    {
        aAction.Reset();
        aAction.SetPaint( sal_False );
        aAction.SetStatBar( sal_True );
        aAction.SetReschedule( sal_True );

        SwDocPosUpdate aMsgHnt( 0 );
        GetDoc()->UpdatePageFlds( &aMsgHnt );
        GetDoc()->UpdateExpFlds(NULL, true);

        aAction.Action();
    }

    if ( VisArea().HasArea() )
        InvalidateWindows( VisArea() );
    if ( bEndProgress )
        ::EndProgress( GetDoc()->GetDocShell() );
}


void ViewShell::SetFirstVisPageInvalid()
{
    ViewShell *pSh = this;
    do
    {
        if ( pSh->Imp() )
            pSh->Imp()->SetFirstVisPageInvalid();
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != this );
}


void ViewShell::SizeChgNotify()
{
    if ( !pWin )
        bDocSizeChgd = sal_True;
    else if( ActionPend() || Imp()->IsCalcLayoutProgress() || bPaintInProgress )
    {
        bDocSizeChgd = sal_True;

        if ( !Imp()->IsCalcLayoutProgress() && ISA( SwCrsrShell ) )
        {
            const SwFrm *pCnt = ((SwCrsrShell*)this)->GetCurrFrm( sal_False );
            const SwPageFrm *pPage;
            if ( pCnt && 0 != (pPage = pCnt->FindPageFrm()) )
            {
                sal_uInt16 nVirtNum = pPage->GetVirtPageNum();
                 const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
                String sDisplay = rNum.GetNumStr( nVirtNum );
                PageNumNotify( this, pCnt->GetPhyPageNum(), nVirtNum, sDisplay );
            }
        }
    }
    else
    {
        bDocSizeChgd = sal_False;
        ::SizeNotify( this, GetDocSize() );
    }
}


void ViewShell::VisPortChgd( const SwRect &rRect)
{
    OSL_ENSURE( GetWin(), "VisPortChgd ohne Window." );

    if ( rRect == VisArea() )
        return;

#if OSL_DEBUG_LEVEL > 1
    if ( bInEndAction )
    {
        //Da Rescheduled doch schon wieder irgendwo einer?
        OSL_ENSURE( !this, "Scroll waehrend einer EndAction." );
    }
#endif

    //Ersteinmal die alte sichtbare Seite holen, dann braucht nacher nicht
    //lange gesucht werden.
    const SwFrm *pOldPage = Imp()->GetFirstVisPage();

    const SwRect aPrevArea( VisArea() );
    const sal_Bool bFull = aPrevArea.IsEmpty();
    aVisArea = rRect;
    SetFirstVisPageInvalid();

    //Wenn noch eine PaintRegion herumsteht und sich die VisArea geaendert hat,
    //so ist die PaintRegion spaetestens jetzt obsolete. Die PaintRegion kann
    //vom RootFrm::Paint erzeugt worden sein.
    if ( !bInEndAction &&
         Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegion();

    SET_CURR_SHELL( this );

    bool bScrolled = false;

    SwPostItMgr* pPostItMgr = GetPostItMgr();

    if ( bFull )
        GetWin()->Invalidate();
    else
    {
        // Betrag ausrechnen, um den gescrolled werden muss.
        const long nXDiff = aPrevArea.Left() - VisArea().Left();
        const long nYDiff = aPrevArea.Top()  - VisArea().Top();

        if( !nXDiff && !GetViewOptions()->getBrowseMode() &&
            (!Imp()->HasDrawView() || !Imp()->GetDrawView()->IsGridVisible() ) )
        {
            //Falls moeglich die Wiese nicht mit Scrollen.
            //Also linke und rechte Kante des Scrollbereiches auf die
            //Seiten begrenzen.
            const SwPageFrm *pPage = (SwPageFrm*)GetLayout()->Lower();  //swmod 071108//swmod 071225
            if ( pPage->Frm().Top() > pOldPage->Frm().Top() )
                pPage = (SwPageFrm*)pOldPage;
            SwRect aBoth( VisArea() );
            aBoth.Union( aPrevArea );
            const SwTwips nBottom = aBoth.Bottom();
            SwTwips nMinLeft = LONG_MAX;
            SwTwips nMaxRight= 0;

            const bool bBookMode = GetViewOptions()->IsViewLayoutBookMode();

            while ( pPage && pPage->Frm().Top() <= nBottom )
            {
                SwRect aPageRect( pPage->GetBoundRect() );
                if ( bBookMode )
                {
                    const SwPageFrm& rFormatPage = static_cast<const SwPageFrm*>(pPage)->GetFormatPage();
                    aPageRect.SSize() = rFormatPage.GetBoundRect().SSize();
                }

                    // OD 12.02.2003 #i9719#, #105645# - consider new border
                    // and shadow width
                if ( aPageRect.IsOver( aBoth ) )
                {
                    SwTwips nPageLeft = 0;
                    SwTwips nPageRight = 0;
                    const sw::sidebarwindows::SidebarPosition aSidebarPos = pPage->SidebarPosition();

                    if( aSidebarPos != sw::sidebarwindows::SIDEBAR_NONE )
                    {
                        nPageLeft = aPageRect.Left();
                        nPageRight = aPageRect.Right();
                    }

                    if( nPageLeft < nMinLeft )
                        nMinLeft = nPageLeft;
                    if( nPageRight > nMaxRight )
                        nMaxRight = nPageRight;
                    //Zus. auf die Zeichenobjekte abgleichen.
                    //Einen Ofst beruecksichtigen, weil die Objekte u.U.
                    //selektiert sind und die Henkel dann hinausstehen.
                    if ( pPage->GetSortedObjs() )
                    {
                        const long nOfst = GetOut()->PixelToLogic(
                            Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width();
                        for ( sal_uInt16 i = 0;
                              i < pPage->GetSortedObjs()->Count(); ++i )
                        {
                            SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                            // ignore objects that are not actually placed on the page
                            if (pObj->IsFormatPossible())
                            {
                                const Rectangle &rBound = pObj->GetObjRect().SVRect();
                            // OD 03.03.2003 #107927# - use correct datatype
                                const SwTwips nL = Max( 0L, rBound.Left() - nOfst );
                                if ( nL < nMinLeft )
                                    nMinLeft = nL;
                                if( rBound.Right() + nOfst > nMaxRight )
                                    nMaxRight = rBound.Right() + nOfst;
                            }
                        }
                    }
                }
                pPage = (SwPageFrm*)pPage->GetNext();
            }
            Rectangle aRect( aPrevArea.SVRect() );
            aRect.Left()  = nMinLeft;
            aRect.Right() = nMaxRight;
            if( VisArea().IsOver( aPrevArea ) && !nLockPaint )
            {
                bScrolled = true;
                aVisArea.Pos() = aPrevArea.Pos();
                if ( SmoothScroll( nXDiff, nYDiff, &aRect ) )
                    return;
                aVisArea.Pos() = rRect.Pos();
            }
            else
                GetWin()->Invalidate( aRect );
        }
        else if ( !nLockPaint ) //Wird im UnLock erledigt
        {
            if( VisArea().IsOver( aPrevArea ) )
            {
                bScrolled = true;
                aVisArea.Pos() = aPrevArea.Pos();
                if ( SmoothScroll( nXDiff, nYDiff, 0 ) )
                    return;
                aVisArea.Pos() = rRect.Pos();
            }
            else
                GetWin()->Invalidate();
        }
    }

    Point aPt( VisArea().Pos() );
    aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();
    MapMode aMapMode( GetWin()->GetMapMode() );
    aMapMode.SetOrigin( aPt );
    GetWin()->SetMapMode( aMapMode );
    if ( HasDrawView() )
    {
        Imp()->GetDrawView()->VisAreaChanged( GetWin() );
        Imp()->GetDrawView()->SetActualWin( GetWin() );
    }
    GetWin()->Update();

    // --> OD 2010-02-11 #i88070#
    if ( pPostItMgr )
    {
        pPostItMgr->Rescale();
        pPostItMgr->CalcRects();
        pPostItMgr->LayoutPostIts();
    }
    // <--

    if ( !bScrolled && pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        pPostItMgr->CorrectPositions();

    if( Imp()->IsAccessible() )
        Imp()->UpdateAccessible();

}


sal_Bool ViewShell::SmoothScroll( long lXDiff, long lYDiff, const Rectangle *pRect )
{
    const sal_uLong nColCnt = pOut->GetColorCount();
    long lMult = 1, lMax = LONG_MAX;
    if ( nColCnt == 65536 )
    {
        lMax = 7000;
        lMult = 2;
    }
    if ( nColCnt == 16777216 )
    {
        lMax = 5000;
        lMult = 6;
    }
    else if ( nColCnt == 1 )
    {
        lMax = 3000;
        lMult = 12;
    }

    // #i75172# isolated static conditions
    const bool bOnlyYScroll(!lXDiff && Abs(lYDiff) != 0 && Abs(lYDiff) < lMax);
    const bool bAllowedWithChildWindows(GetWin()->GetWindowClipRegionPixel(WINDOW_GETCLIPREGION_NOCHILDREN|WINDOW_GETCLIPREGION_NULL).IsNull());
// --> OD 2009-08-12 #i98766# - disable smooth scrolling for Mac port builds
#ifdef QUARTZ
    const bool bSmoothScrollAllowed(false);
    (void) bOnlyYScroll;
    (void) bAllowedWithChildWindows;
#else
    const bool bSmoothScrollAllowed(bOnlyYScroll && bEnableSmooth && GetViewOptions()->IsSmoothScroll() &&  bAllowedWithChildWindows);
#endif
// <-
    const bool bIAmCursorShell(ISA(SwCrsrShell));
    (void) bIAmCursorShell;

    // #i75172# with selection on overlay, smooth scroll should be allowed with it
    const bool bAllowedForSelection(true || (bIAmCursorShell && !((SwCrsrShell*)this)->HasSelection()));

    // #i75172# with cursors on overlay, smooth scroll should be allowed with it
    const bool bAllowedForMultipleCursors(true || (bIAmCursorShell && ((SwCrsrShell*)this)->GetCrsrCnt() < 2));

    if(bSmoothScrollAllowed  && bAllowedForSelection && bAllowedForMultipleCursors)
    {
        Imp()->bStopSmooth = sal_False;

        const SwRect aOldVis( VisArea() );

        //Virtuelles Device erzeugen und einstellen.
        const Size aPixSz = GetWin()->PixelToLogic(Size(1,1));
        VirtualDevice *pVout = new VirtualDevice( *GetWin() );
        pVout->SetLineColor( GetWin()->GetLineColor() );
        pVout->SetFillColor( GetWin()->GetFillColor() );
        MapMode aMapMode( GetWin()->GetMapMode() );
        pVout->SetMapMode( aMapMode );
        Size aSize( aVisArea.Width()+2*aPixSz.Width(), Abs(lYDiff)+(2*aPixSz.Height()) );
        if ( pRect )
            aSize.Width() = Min(aSize.Width(), pRect->GetWidth()+2*aPixSz.Width());
        if ( pVout->SetOutputSize( aSize ) )
        {
            nLockPaint++;

            //Ersteinmal alles neue in das VirDev Painten.
            SwRect aRect( VisArea() );
            aRect.Height( aSize.Height() );
            if ( pRect )
            {
                aRect.Pos().X() = Max(aRect.Left(),pRect->Left()-aPixSz.Width());
                aRect.Right( Min(aRect.Right()+2*aPixSz.Width(), pRect->Right()+aPixSz.Width()));
            }
            else
                aRect.SSize().Width() += 2*aPixSz.Width();
            aRect.Pos().Y() = lYDiff < 0 ? aOldVis.Bottom() - aPixSz.Height()
                                         : aRect.Top() - aSize.Height() + aPixSz.Height();
            aRect.Pos().X() = Max( 0L, aRect.Left()-aPixSz.Width() );
            aRect.Pos()  = GetWin()->PixelToLogic( GetWin()->LogicToPixel( aRect.Pos()));
            aRect.SSize()= GetWin()->PixelToLogic( GetWin()->LogicToPixel( aRect.SSize()));
            aVisArea = aRect;
            const Point aPt( -aRect.Left(), -aRect.Top() );
            aMapMode.SetOrigin( aPt );
            pVout->SetMapMode( aMapMode );
            OutputDevice *pOld = pOut;
            pOut = pVout;

            {
                // #i75172# To get a clean repaint, a new ObjectContact is needed here. Without, the
                // repaint would not be correct since it would use the wrong DrawPage visible region.
                // This repaint IS about painting something currently outside the visible part (!).
                // For that purpose, AddWindowToPaintView is used which creates a new SdrPageViewWindow
                // and all the necessary stuff. It's not cheap, but necessary here. Alone because repaint
                // target really is NOT the current window.
                // Also will automatically NOT use PreRendering and overlay (since target is VirtualDevice)
                if(!HasDrawView())
                    MakeDrawView();
                SdrView* pDrawView = GetDrawView();
                pDrawView->AddWindowToPaintView(pVout);

                // clear pWin during DLPrePaint2 to get paint preparation for pOut, but set it again
                // immediately afterwards. There are many decisions in SW which imply that Printing
                // is used when pWin == 0 (wrong but widely used).
                Window* pOldWin = pWin;
                pWin = 0;
                DLPrePaint2(Region(aRect.SVRect()));
                pWin = pOldWin;

                // SW paint stuff
                PaintDesktop( aRect );
                ViewShell::bLstAct = sal_True;
                GetLayout()->Paint( aRect );
                ViewShell::bLstAct = sal_False;

                // end paint and destroy ObjectContact again
                DLPostPaint2(true);
                pDrawView->DeleteWindowFromPaintView(pVout);
            }

            pOut = pOld;
            aVisArea = aOldVis;

            //Jetzt Stueckchenweise schieben und die neuen Pixel aus dem
            //VirDev  kopieren.

            // ??????????????????????
            // or is it better to get the scrollfactor from the User
            // as option?
            // ??????????????????????
            long lMaDelta = aPixSz.Height();
            if ( Abs(lYDiff) > ( aVisArea.Height() / 3 ) )
                lMaDelta *= 6;
            else
                lMaDelta *= 2;

            lMaDelta *= lMult;

            if ( lYDiff < 0 )
                lMaDelta = -lMaDelta;

            long lDiff = lYDiff;
            while ( lDiff )
            {
                long lScroll;
                if ( Imp()->bStopSmooth || Abs(lDiff) <= Abs(lMaDelta) )
                {
                    lScroll = lDiff;
                    lDiff = 0;
                }
                else
                {
                    lScroll = lMaDelta;
                    lDiff -= lMaDelta;
                }

                const SwRect aTmpOldVis = VisArea();
                aVisArea.Pos().Y() -= lScroll;
                aVisArea.Pos() = GetWin()->PixelToLogic( GetWin()->LogicToPixel( VisArea().Pos()));
                lScroll = aTmpOldVis.Top() - VisArea().Top();
                if ( pRect )
                {
                    Rectangle aTmp( aTmpOldVis.SVRect() );
                    aTmp.Left() = pRect->Left();
                    aTmp.Right()= pRect->Right();
                    GetWin()->Scroll( 0, lScroll, aTmp, SCROLL_CHILDREN);
                }
                else
                    GetWin()->Scroll( 0, lScroll, SCROLL_CHILDREN );

                const Point aTmpPt( -VisArea().Left(), -VisArea().Top() );
                MapMode aTmpMapMode( GetWin()->GetMapMode() );
                aTmpMapMode.SetOrigin( aTmpPt );
                GetWin()->SetMapMode( aTmpMapMode );

                if ( Imp()->HasDrawView() )
                    Imp()->GetDrawView()->VisAreaChanged( GetWin() );

                SetFirstVisPageInvalid();
                if ( !Imp()->bStopSmooth )
                {
                    const bool bScrollDirectionIsUp(lScroll > 0);
                    Imp()->aSmoothRect = VisArea();

                    if(bScrollDirectionIsUp)
                    {
                        Imp()->aSmoothRect.Bottom( VisArea().Top() + lScroll + aPixSz.Height());
                    }
                    else
                    {
                        Imp()->aSmoothRect.Top( VisArea().Bottom() + lScroll - aPixSz.Height());
                    }

                    Imp()->bSmoothUpdate = sal_True;
                    GetWin()->Update();
                    Imp()->bSmoothUpdate = sal_False;

                    if(!Imp()->bStopSmooth)
                    {
                            // start paint on logic base
                            const Rectangle aTargetLogic(Imp()->aSmoothRect.SVRect());
                            DLPrePaint2(Region(aTargetLogic));

                            // get target rectangle in discrete pixels
                            OutputDevice& rTargetDevice = mpTargetPaintWindow->GetTargetOutputDevice();
                            const Rectangle aTargetPixel(rTargetDevice.LogicToPixel(aTargetLogic));

                            // get source top-left in discrete pixels
                            const Point aSourceTopLeft(pVout->LogicToPixel(aTargetLogic.TopLeft()));

                            // switch off MapModes
                            const bool bMapModeWasEnabledDest(rTargetDevice.IsMapModeEnabled());
                            const bool bMapModeWasEnabledSource(pVout->IsMapModeEnabled());
                            rTargetDevice.EnableMapMode(false);
                            pVout->EnableMapMode(false);

                            rTargetDevice.DrawOutDev(
                                aTargetPixel.TopLeft(), aTargetPixel.GetSize(), // dest
                                aSourceTopLeft, aTargetPixel.GetSize(), // source
                                *pVout);

                            // restore MapModes
                            rTargetDevice.EnableMapMode(bMapModeWasEnabledDest);
                            pVout->EnableMapMode(bMapModeWasEnabledSource);

                            // end paint on logoc base
                            DLPostPaint2(true);
                    }
                    else
                        --nLockPaint;
                }
            }
            delete pVout;
            GetWin()->Update();
            if ( !Imp()->bStopSmooth )
                --nLockPaint;
            SetFirstVisPageInvalid();
            return sal_True;
        }
        delete pVout;
    }

    aVisArea.Pos().X() -= lXDiff;
    aVisArea.Pos().Y() -= lYDiff;
    if ( pRect )
        GetWin()->Scroll( lXDiff, lYDiff, *pRect, SCROLL_CHILDREN);
    else
        GetWin()->Scroll( lXDiff, lYDiff, SCROLL_CHILDREN);
    return sal_False;
}


void ViewShell::PaintDesktop( const SwRect &rRect )
{
    if ( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return;                     //Fuer den Drucker tun wir hier nix

    //Sonderfaelle abfangen, damit es nicht gar so ueberraschend aussieht.
    //Kann z.B. waehrend des Idle'ns zwischenzeitlich auftreten.
    //Die Rechtecke neben den Seiten muessen wir leider auf jedenfall Painten,
    //den diese werden spaeter beim VisPortChgd ausgespart.
    sal_Bool bBorderOnly = sal_False;
    const SwRootFrm *pRoot = GetLayout();//swmod 080305
    if ( rRect.Top() > pRoot->Frm().Bottom() )
    {
        const SwFrm *pPg = pRoot->Lower();
        while ( pPg && pPg->GetNext() )
            pPg = pPg->GetNext();
        if ( !pPg || !pPg->Frm().IsOver( VisArea() ) )
            bBorderOnly = sal_True;
    }

    const bool bBookMode = GetViewOptions()->IsViewLayoutBookMode();

    SwRegionRects aRegion( rRect );

    //mod #i6193: remove sidebar area to avoid flickering
    const SwPostItMgr* pPostItMgr = GetPostItMgr();
    const SwTwips nSidebarWidth = pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() ?
                                  pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth() :
                                  0;

    if ( bBorderOnly )
    {
        const SwFrm *pPage =pRoot->Lower(); //swmod 071108//swmod 071225
        SwRect aLeft( rRect ), aRight( rRect );
        while ( pPage )
        {
            long nTmp = pPage->Frm().Left();
            if ( nTmp < aLeft.Right() )
                aLeft.Right( nTmp );
            nTmp = pPage->Frm().Right();
            if ( nTmp > aRight.Left() )
            {
                aRight.Left( nTmp + nSidebarWidth );
            }
            pPage = pPage->GetNext();
        }
        aRegion.clear();
        if ( aLeft.HasArea() )
            aRegion.push_back( aLeft );
        if ( aRight.HasArea() )
            aRegion.push_back( aRight );
    }
    else
    {
        const SwFrm *pPage = Imp()->GetFirstVisPage();
        const SwTwips nBottom = rRect.Bottom();
        //const SwTwips nRight  = rRect.Right();
        while ( pPage && !aRegion.empty() &&
                (pPage->Frm().Top() <= nBottom) ) // PAGES01 && (pPage->Frm().Left() <= nRight))
        {
            SwRect aPageRect( pPage->Frm() );
            if ( bBookMode )
            {
                const SwPageFrm& rFormatPage = static_cast<const SwPageFrm*>(pPage)->GetFormatPage();
                aPageRect.SSize() = rFormatPage.Frm().SSize();
            }

            const bool bSidebarRight =
                static_cast<const SwPageFrm*>(pPage)->SidebarPosition() == sw::sidebarwindows::SIDEBAR_RIGHT;
            aPageRect.Pos().X() -= bSidebarRight ? 0 : nSidebarWidth;
            aPageRect.SSize().Width() += nSidebarWidth;

            if ( aPageRect.IsOver( rRect ) )
                aRegion -= aPageRect;

            pPage = pPage->GetNext();
        }
    }
    if ( !aRegion.empty() )
        _PaintDesktop( aRegion );
}


// PaintDesktop gesplittet, dieser Teil wird auch von PreViewPage benutzt
void ViewShell::_PaintDesktop( const SwRegionRects &rRegion )
{
    // OD 2004-04-23 #116347#
    GetOut()->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
    GetOut()->SetLineColor();

    for ( sal_uInt16 i = 0; i < rRegion.size(); ++i )
    {
        const Rectangle aRectangle(rRegion[i].SVRect());

        // #i93170#
        // Here we have a real Problem. On the one hand we have the buffering for paint
        // and overlay which needs an embracing pair of DLPrePaint2/DLPostPaint2 calls,
        // on the other hand the MapMode is not set correctly when this code is executed.
        // This is done in the users of this method, for each SWpage before painting it.
        // Since the MapMode is not correct here, the call to DLPostPaint2 will paint
        // existing FormControls due to the current MapMode.
        //
        // There are basically three solutions for this:
        //
        // (1) Set the MapMode correct, move the background painting to the users of
        //     this code
        //
        // (2) Do no DLPrePaint2/DLPostPaint2 here; no SdrObjects are allowed to lie in
        //     the desktop region. Disadvantage: the desktop will not be part of the
        //     buffers, e.g. overlay. Thus, as soon as overlay will be used over the
        //     desktop, it will not work.
        //
        // (3) expand DLPostPaint2 with a flag to signal if FormControl paints shall
        //     be done or not
        //
        // Currently, (3) will be the best possible solution. It will keep overlay and
        // buffering intact and work without MapMode for single pages. In the medium
        // to long run, (1) will need to be used and the bool bPaintFormLayer needs
        // to be removed again

        // #i68597# inform Drawinglayer about display change
        DLPrePaint2(Region(aRectangle));

        // #i75172# needed to move line/Fill color setters into loop since DLPrePaint2
        // may exchange GetOut(), that's it's purpose. This happens e.g. at print preview.
        GetOut()->SetFillColor( SwViewOption::GetAppBackgroundColor());
        GetOut()->SetLineColor();
        GetOut()->DrawRect(aRectangle);

        DLPostPaint2(false);
    }

    GetOut()->Pop();
}


sal_Bool ViewShell::CheckInvalidForPaint( const SwRect &rRect )
{
    if ( !GetWin() )
        return sal_False;

    const SwPageFrm *pPage = Imp()->GetFirstVisPage();
    const SwTwips nBottom = VisArea().Bottom();
    const SwTwips nRight  = VisArea().Right();
    sal_Bool bRet = sal_False;
    while ( !bRet && pPage && !((pPage->Frm().Top()  > nBottom) ||
                                   (pPage->Frm().Left() > nRight)))
    {
        if ( pPage->IsInvalid() || pPage->IsInvalidFly() )
            bRet = sal_True;
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    if ( bRet )
    {
        //Start/EndAction wuerden hier leider nix helfen, weil das Paint vom
        //GUI 'reinkam und somit ein Clipping gesetzt ist gegen das wir nicht
        //nicht ankommen.
        //Ergo: Alles selbst machen (siehe ImplEndAction())
        if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea())
             Imp()->DelRegion();

        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( sal_False );
        // We increment the action counter to avoid a recursive call of actions
        // e.g. from a SwFEShell::RequestObjectResize(..) in bug 95829.
        // A recursive call of actions is no good idea because the inner action
        // can't format frames which are locked by the outer action. This may
        // cause and endless loop.
        ++nStartAction;
        aAction.Action();
        --nStartAction;

        SwRegionRects *pRegion = Imp()->GetRegion();
        if ( pRegion && aAction.IsBrowseActionStop() )
        {
            //Nur dann interessant, wenn sich im sichtbaren Bereich etwas
            //veraendert hat.
            sal_Bool bStop = sal_True;
            for ( sal_uInt16 i = 0; i < pRegion->size(); ++i )
            {
                const SwRect &rTmp = (*pRegion)[i];
                if ( sal_False == (bStop = rTmp.IsOver( VisArea() )) )
                    break;
            }
            if ( bStop )
            {
                Imp()->DelRegion();
                pRegion = 0;
            }
        }

        if ( pRegion )
        {
            //Erst Invert dann Compress, niemals andersherum!
            pRegion->Invert();
            pRegion->Compress();
            bRet = sal_False;
            if ( !pRegion->empty() )
            {
                SwRegionRects aRegion( rRect );
                for ( sal_uInt16 i = 0; i < pRegion->size(); ++i )
                {   const SwRect &rTmp = (*pRegion)[i];
                    if ( !rRect.IsInside( rTmp ) )
                    {
                        InvalidateWindows( rTmp );
                        if ( rTmp.IsOver( VisArea() ) )
                        {   aRegion -= rTmp;
                            bRet = sal_True;
                        }
                    }
                }
                if ( bRet )
                {
                    for ( sal_uInt16 i = 0; i < aRegion.size(); ++i )
                        GetWin()->Invalidate( aRegion[i].SVRect() );

                    if ( rRect != VisArea() )
                    {
                        //rRect == VisArea ist der spezialfall fuer neu bzw.
                        //Shift-Ctrl-R, dafuer sollte es nicht notwendig sein
                        //das Rechteck nocheinmal in Dokumentkoordinaten v
                        //vorzuhalten.
                        if ( aInvalidRect.IsEmpty() )
                            aInvalidRect = rRect;
                        else
                            aInvalidRect.Union( rRect );
                    }
                }
            }
            else
                bRet = sal_False;
            Imp()->DelRegion();
        }
        else
            bRet = sal_False;
    }
    return bRet;
}


void ViewShell::Paint(const Rectangle &rRect)
{
    if ( nLockPaint )
    {
        if ( Imp()->bSmoothUpdate )
        {
            SwRect aTmp( rRect );
            if ( !Imp()->aSmoothRect.IsInside( aTmp ) )
                Imp()->bStopSmooth = sal_True;
            else
            {
                Imp()->aSmoothRect = aTmp;
                return;
            }
        }
        else
            return;
    }

    if ( SwRootFrm::IsInPaint() )
    {
        //Waehrend der Ausgabe einer Seite beim Druckvorgang wird das
        //Paint gepuffert.
        SwPaintQueue::Add( this, SwRect( rRect ) );
        return;
    }

    // mit !nStartAction versuche ich mal mich gegen
    //fehlerhaften Code an anderen Stellen zu wehren. Hoffentlich fuehrt das
    //nicht zu Problemen!?
    if ( bPaintWorks && !nStartAction )
    {
        if( GetWin() && GetWin()->IsVisible() )
        {
            SwRect aRect( rRect );
            if ( bPaintInProgress ) //Schutz gegen doppelte Paints!
            {
                GetWin()->Invalidate( rRect );
                return;
            }

            bPaintInProgress = sal_True;
            SET_CURR_SHELL( this );
            SwRootFrm::SetNoVirDev( sal_True );

            //Wir wollen nicht staendig hin und her Clippen, wir verlassen
            //uns darauf, das sich alle auf das Rechteck beschraeken und
            //brauchen das Clipping hier nur einmalig einkalkulieren. Das
            //ClipRect wird hier einmal entfernt und nicht Restauriert, denn
            //von aussen braucht es sowieso keiner mehr.
            //Nicht wenn wir ein MetaFile aufzeichnen.
            if( !GetOut()->GetConnectMetaFile() && GetOut()->IsClipRegion())
                GetOut()->SetClipRegion();

            if ( IsPreView() )
            {
                //Falls sinnvoll gleich das alte InvalidRect verarbeiten bzw.
                //vernichten.
                if ( aRect.IsInside( aInvalidRect ) )
                    ResetInvalidRect();
                ViewShell::bLstAct = sal_True;
                GetLayout()->Paint( aRect );
                ViewShell::bLstAct = sal_False;
            }
            else
            {
                //SwSaveHdl *pSaveHdl = 0;
                //if ( Imp()->HasDrawView() )
                //  pSaveHdl = new SwSaveHdl( Imp() );

                //Wenn eine der sichtbaren Seiten noch irgendetwas zum Repaint
                //angemeldet hat, so muessen Repaints ausgeloest werden.
                if ( !CheckInvalidForPaint( aRect ) )
                {
                    // --> OD 2009-08-12 #i101192#
                    // start Pre/PostPaint encapsulation to avoid screen blinking
                    const Region aRepaintRegion(aRect.SVRect());
                    DLPrePaint2(aRepaintRegion);

                    // <--
                    PaintDesktop( aRect );

                    //Falls sinnvoll gleich das alte InvalidRect verarbeiten bzw.
                    //vernichten.
                    if ( aRect.IsInside( aInvalidRect ) )
                        ResetInvalidRect();
                    ViewShell::bLstAct = sal_True;
                    GetLayout()->Paint( aRect );
                    ViewShell::bLstAct = sal_False;
                    // --> OD 2009-08-12 #i101192#
                    // end Pre/PostPaint encapsulation
                    DLPostPaint2(true);
                    // <--
                }
                //delete pSaveHdl;
            }
            SwRootFrm::SetNoVirDev( sal_False );
            bPaintInProgress = sal_False;
            UISizeNotify();
        }
    }
    else
    {
        if ( aInvalidRect.IsEmpty() )
            aInvalidRect = SwRect( rRect );
        else
            aInvalidRect.Union( SwRect( rRect ) );

        if ( bInEndAction && GetWin() )
        {
            Region aRegion( GetWin()->GetPaintRegion() );
            RegionHandle hHdl( aRegion.BeginEnumRects() );
            Rectangle aRect;
            while ( aRegion.GetNextEnumRect( hHdl, aRect ) )
                Imp()->AddPaintRect( aRect );
            aRegion.EndEnumRects( hHdl );
        }
        else if ( SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) &&
                  GetOut() == GetWin() )
        {
            // #i68597#
            const Region aDLRegion(rRect);
            DLPrePaint2(aDLRegion);

            pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            pOut->SetFillColor( Imp()->GetRetoucheColor() );
            pOut->SetLineColor();
            pOut->DrawRect( rRect );
            pOut->Pop();
            // #i68597#
            DLPostPaint2(true);
        }
    }
}


void ViewShell::SetBrowseBorder( const Size& rNew )
{
    if( rNew != aBrowseBorder )
    {
        aBrowseBorder = rNew;
        if ( aVisArea.HasArea() )
            CheckBrowseView( sal_False );
    }
}

const Size& ViewShell::GetBrowseBorder() const
{
    return aBrowseBorder;
}

sal_Int32 ViewShell::GetBrowseWidth() const
{
    const SwPostItMgr* pPostItMgr = GetPostItMgr();
    if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
    {
        Size aBorder( aBrowseBorder );
        aBorder.Width() += aBrowseBorder.Width();
        aBorder.Width() += pPostItMgr->GetSidebarWidth(true) + pPostItMgr->GetSidebarBorderWidth(true);
        return aVisArea.Width() - GetOut()->PixelToLogic(aBorder).Width();
    }
    else
        return aVisArea.Width() - 2 * GetOut()->PixelToLogic(aBrowseBorder).Width();
}


void ViewShell::CheckBrowseView( sal_Bool bBrowseChgd )
{
    if ( !bBrowseChgd && !GetViewOptions()->getBrowseMode() )
        return;

    SET_CURR_SHELL( this );

    OSL_ENSURE( GetLayout(), "Layout not ready" );

    // Wenn das Layout noch nicht einmal eine Hoehe hat,
    // ist sowieso nichts formatiert.
    // Dann eruebrigt sich die Invalidierung
    // Falsch, z.B. beim Anlegen einer neuen View wird der Inhalt eingef?gt
    // und formatiert (trotz einer leeren VisArea). Hier muessen deshalb
    // die Seiten zur Formatierung angeregt werden.
    if( !GetLayout()->Frm().Height() )
    {
        SwFrm* pPage = GetLayout()->Lower();
        while( pPage )
        {
            pPage->_InvalidateSize();
            pPage = pPage->GetNext();
        }
        return;
    }

    LockPaint();
    StartAction();

    SwPageFrm *pPg = (SwPageFrm*)GetLayout()->Lower();
    do
    {   pPg->InvalidateSize();
        pPg->_InvalidatePrt();
        pPg->InvaPercentLowers();
        if ( bBrowseChgd )
        {
            pPg->PrepareHeader();
            pPg->PrepareFooter();
        }
        pPg = (SwPageFrm*)pPg->GetNext();
    } while ( pPg );

    // Wenn sich die Groessenverhaeltnise im BrowseModus aendern,
    // muss die Position und PrtArea der Cntnt- und Tab-Frames invalidiert werden.
    sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_POS;
    // Beim BrowseModus-Wechsel benoetigen die CntntFrms
    // wg. der Drucker/Bildschirmformatierung eine Size-Invalidierung
    if( bBrowseChgd )
        nInv |= INV_SIZE | INV_DIRECTION;

    GetLayout()->InvalidateAllCntnt( nInv );

    SwFrm::CheckPageDescs( (SwPageFrm*)GetLayout()->Lower() );

    EndAction();
    UnlockPaint();
}


SwRootFrm *ViewShell::GetLayout() const
{
    return pLayout.get();   //swmod 080116
}
/***********************************************************************/

OutputDevice& ViewShell::GetRefDev() const
{
    OutputDevice* pTmpOut = 0;
    if (  GetWin() &&
          GetViewOptions()->getBrowseMode() &&
         !GetViewOptions()->IsPrtFormat() )
        pTmpOut = GetWin();
    else if ( 0 != mpTmpRef )
        pTmpOut = mpTmpRef;
    else
        pTmpOut = GetDoc()->getReferenceDevice( true );

    return *pTmpOut;
}

const SwNodes& ViewShell::GetNodes() const
{
    return pDoc->GetNodes();
}


void ViewShell::DrawSelChanged()
{
}


Size ViewShell::GetDocSize() const
{
    Size aSz;
    const SwRootFrm* pRoot = GetLayout();
    if( pRoot )
        aSz = pRoot->Frm().SSize();

    return aSz;
}


SfxItemPool& ViewShell::GetAttrPool()
{
    return GetDoc()->GetAttrPool();
}


void ViewShell::ApplyViewOptions( const SwViewOption &rOpt )
{

    ViewShell *pSh = this;
    do
    {   pSh->StartAction();
        pSh = (ViewShell*)pSh->GetNext();
    } while ( pSh != this );

    ImplApplyViewOptions( rOpt );

    // With one layout per view it is not longer necessary
    // to sync these "layout related" view options
    // But as long as we have to disable "multiple layout"
    pSh = (ViewShell*)this->GetNext();
    while ( pSh != this )
    {
        SwViewOption aOpt( *pSh->GetViewOptions() );
        aOpt.SetFldName( rOpt.IsFldName() );
            aOpt.SetShowHiddenField( rOpt.IsShowHiddenField() );
        aOpt.SetShowHiddenPara( rOpt.IsShowHiddenPara() );
            aOpt.SetShowHiddenChar( rOpt.IsShowHiddenChar() );
            aOpt.SetViewLayoutBookMode( rOpt.IsViewLayoutBookMode() );
            aOpt.SetViewLayoutColumns( rOpt.GetViewLayoutColumns() );
        aOpt.SetPostIts(rOpt.IsPostIts());
        if ( !(aOpt == *pSh->GetViewOptions()) )
            pSh->ImplApplyViewOptions( aOpt );
        pSh = (ViewShell*)pSh->GetNext();
    }
    // End of disabled multiple window

    pSh = this;
    do
    {   pSh->EndAction();
        pSh = (ViewShell*)pSh->GetNext();
    } while ( pSh != this );

}

void ViewShell::ImplApplyViewOptions( const SwViewOption &rOpt )
{
    if (*pOpt == rOpt)
        return;

    Window *pMyWin = GetWin();
    if( !pMyWin )
    {
        OSL_ENSURE( pMyWin, "ViewShell::ApplyViewOptions: no window" );
        return;
    }

    SET_CURR_SHELL( this );

    sal_Bool bReformat   = sal_False;

    if( pOpt->IsShowHiddenField() != rOpt.IsShowHiddenField() )
    {
        ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
                                            SetHiddenFlag( !rOpt.IsShowHiddenField() );
        bReformat = sal_True;
    }
    if ( pOpt->IsShowHiddenPara() != rOpt.IsShowHiddenPara() )
    {
        SwHiddenParaFieldType* pFldType = (SwHiddenParaFieldType*)GetDoc()->
                                          GetSysFldType(RES_HIDDENPARAFLD);
        if( pFldType && pFldType->GetDepends() )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->ModifyNotification( &aHnt, 0);
        }
        bReformat = sal_True;
    }
    if ( !bReformat && pOpt->IsShowHiddenChar() != rOpt.IsShowHiddenChar() )
    {
        bReformat = GetDoc()->ContainsHiddenChars();
    }

    // bReformat wird sal_True, wenn ...
    // - Feldnamen anzeigen oder nicht ...
    // ( - SwEndPortion muessen _nicht_ mehr generiert werden. )
    // - Das Window ist natuerlich was ganz anderes als der Drucker...
    bReformat = bReformat || pOpt->IsFldName() != rOpt.IsFldName();

    // Der Mapmode wird veraendert, Minima/Maxima werden von der UI beachtet
    if( pOpt->GetZoom() != rOpt.GetZoom() && !IsPreView() )
    {
        MapMode aMode( pMyWin->GetMapMode() );
        Fraction aNewFactor( rOpt.GetZoom(), 100 );
        aMode.SetScaleX( aNewFactor );
        aMode.SetScaleY( aNewFactor );
        pMyWin->SetMapMode( aMode );
        // Wenn kein ReferenzDevice (Drucker) zum Formatieren benutzt wird,
        // sondern der Bildschirm, muss bei Zoomfaktoraenderung neu formatiert
        // werden.
        if( pOpt->getBrowseMode() )
            bReformat = sal_True;
    }

    bool bBrowseModeChanged = false;
    if( pOpt->getBrowseMode() != rOpt.getBrowseMode() )
    {
        bBrowseModeChanged = true;
        bReformat = sal_True;
    }
    else if( pOpt->getBrowseMode() && pOpt->IsPrtFormat() != rOpt.IsPrtFormat() )
        bReformat = sal_True;

    if ( HasDrawView() || rOpt.IsGridVisible() )
    {
        if ( !HasDrawView() )
            MakeDrawView();

        SwDrawView *pDView = Imp()->GetDrawView();
        if ( pDView->IsDragStripes() != rOpt.IsCrossHair() )
            pDView->SetDragStripes( rOpt.IsCrossHair() );

        if ( pDView->IsGridSnap() != rOpt.IsSnap() )
            pDView->SetGridSnap( rOpt.IsSnap() );

        if ( pDView->IsGridVisible() != rOpt.IsGridVisible() )
            pDView->SetGridVisible( rOpt.IsGridVisible() );

        const Size &rSz = rOpt.GetSnapSize();
        pDView->SetGridCoarse( rSz );

        const Size aFSize
            ( rSz.Width() ? rSz.Width() / (rOpt.GetDivisionX()+1) : 0,
              rSz.Height()? rSz.Height()/ (rOpt.GetDivisionY()+1) : 0);
        pDView->SetGridFine( aFSize );
        Fraction aSnGrWdtX(rSz.Width(), rOpt.GetDivisionX() + 1);
        Fraction aSnGrWdtY(rSz.Height(), rOpt.GetDivisionY() + 1);
        pDView->SetSnapGridWidth( aSnGrWdtX, aSnGrWdtY );

            // set handle size to 9 pixels, always
            pDView->SetMarkHdlSizePixel(9);
    }

    sal_Bool bOnlineSpellChgd = pOpt->IsOnlineSpell() != rOpt.IsOnlineSpell();

    *pOpt = rOpt;   // Erst jetzt werden die Options uebernommen.
    pOpt->SetUIOptions(rOpt);

    pDoc->set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode(pDoc->GetDocShell()));

    if( bBrowseModeChanged )
    {
        // #i44963# Good occasion to check if page sizes in
        // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
        pDoc->CheckDefaultPageFmt();
        CheckBrowseView( sal_True );
    }

    pMyWin->Invalidate();
    if ( bReformat )
    {
        // Es hilft alles nichts, wir muessen an alle CntntFrms ein
        // Prepare verschicken, wir formatieren neu:
        StartAction();
        Reformat();
        EndAction();
    }

    if( bOnlineSpellChgd )
    {
        ViewShell *pSh = (ViewShell*)this->GetNext();
        sal_Bool bOnlineSpl = rOpt.IsOnlineSpell();
        while( pSh != this )
        {   pSh->pOpt->SetOnlineSpell( bOnlineSpl );
            Window *pTmpWin = pSh->GetWin();
            if( pTmpWin )
                pTmpWin->Invalidate();
            pSh = (ViewShell*)pSh->GetNext();
        }
    }

}


void ViewShell::SetUIOptions( const SwViewOption &rOpt )
{
    pOpt->SetUIOptions(rOpt);
    //the API-Flag of the view options is set but never reset
    //it is required to set scroll bars in readonly documents
    if(rOpt.IsStarOneSetting())
        pOpt->SetStarOneSetting(sal_True);

    pOpt->SetSymbolFont(rOpt.GetSymbolFont());
}


void ViewShell::SetReadonlyOption(sal_Bool bSet)
{
    //JP 01.02.99: bei ReadOnly Flag richtig abfragen und ggfs. neu
    //              formatieren; Bug 61335

    // Schalten wir gerade von Readonly auf Bearbeiten um?
    if( bSet != pOpt->IsReadonly() )
    {
        // damit die Flags richtig erfragt werden koennen.
        pOpt->SetReadonly( sal_False );

        sal_Bool bReformat = pOpt->IsFldName();

        pOpt->SetReadonly( bSet );

        if( bReformat )
        {
            StartAction();
            Reformat();
            if ( GetWin() )
                GetWin()->Invalidate();
            EndAction();
        }
        else if ( GetWin() )
            GetWin()->Invalidate();
        if( Imp()->IsAccessible() )
            Imp()->InvalidateAccessibleEditableState( sal_False );
    }
}


void  ViewShell::SetPDFExportOption(sal_Bool bSet)
{
    if( bSet != pOpt->IsPDFExport() )
    {
        if( bSet && pOpt->getBrowseMode() )
            pOpt->SetPrtFormat( sal_True );
        pOpt->SetPDFExport(bSet);
    }
}


void  ViewShell::SetReadonlySelectionOption(sal_Bool bSet)
{
    if( bSet != pOpt->IsSelectionInReadonly() )
    {
        pOpt->SetSelectionInReadonly(bSet);
    }
}


void ViewShell::SetPrtFormatOption( sal_Bool bSet )
{
    pOpt->SetPrtFormat( bSet );
}


void ViewShell::UISizeNotify()
{
    if ( bDocSizeChgd )
    {
        bDocSizeChgd = sal_False;
        sal_Bool bOld = bInSizeNotify;
        bInSizeNotify = sal_True;
        ::SizeNotify( this, GetDocSize() );
        bInSizeNotify = bOld;
    }
}


void    ViewShell::SetRestoreActions(sal_uInt16 nSet)
{
    OSL_ENSURE(!GetRestoreActions()||!nSet, "multiple restore of the Actions ?");
    Imp()->SetRestoreActions(nSet);
}
sal_uInt16  ViewShell::GetRestoreActions() const
{
    return Imp()->GetRestoreActions();
}

sal_Bool ViewShell::IsNewLayout() const
{
    return GetLayout()->IsNewLayout();
}

uno::Reference< ::com::sun::star::accessibility::XAccessible > ViewShell::CreateAccessible()
{
    uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc;

    // We require a layout and an XModel to be accessible.
    OSL_ENSURE( pLayout, "no layout, no access" );
    OSL_ENSURE( GetWin(), "no window, no access" );

    if( pDoc->GetCurrentViewShell() && GetWin() )   //swmod 071108
        xAcc = Imp()->GetAccessibleMap().GetDocumentView();

    return xAcc;
}

uno::Reference< ::com::sun::star::accessibility::XAccessible >
ViewShell::CreateAccessiblePreview()
{
    OSL_ENSURE( IsPreView(),
                "Can't create accessible preview for non-preview ViewShell" );

    // We require a layout and an XModel to be accessible.
    OSL_ENSURE( pLayout, "no layout, no access" );
    OSL_ENSURE( GetWin(), "no window, no access" );

    if ( IsPreView() && GetLayout()&& GetWin() )
    {
        return Imp()->GetAccessibleMap().GetDocumentPreview(
                    PagePreviewLayout()->maPrevwPages,
                    GetWin()->GetMapMode().GetScaleX(),
                    GetLayout()->GetPageByPageNum( PagePreviewLayout()->mnSelectedPageNum ),
                    PagePreviewLayout()->maWinSize );   //swmod 080305
    }
    return NULL;
}

void ViewShell::InvalidateAccessibleFocus()
{
    if( Imp()->IsAccessible() )
        Imp()->GetAccessibleMap().InvalidateFocus();
}

/*
 * invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs #i27138#
 */
void ViewShell::InvalidateAccessibleParaFlowRelation( const SwTxtFrm* _pFromTxtFrm,
                                                      const SwTxtFrm* _pToTxtFrm )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaFlowRelation( _pFromTxtFrm, _pToTxtFrm );
    }
}

/*
 * invalidate text selection for paragraphs #i27301#
 */
void ViewShell::InvalidateAccessibleParaTextSelection()
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaTextSelection();
    }
}

/**
 * invalidate attributes for paragraphs #i88069#
 */
void ViewShell::InvalidateAccessibleParaAttrs( const SwTxtFrm& rTxtFrm )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaAttrs( rTxtFrm );
    }
}

SwAccessibleMap* ViewShell::GetAccessibleMap()
{
    if ( Imp()->IsAccessible() )
    {
        return &(Imp()->GetAccessibleMap());
    }

    return 0;
}


void ViewShell::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    if(pOpt->IsPagePreview() && !rAccessibilityOptions.GetIsForPagePreviews())
    {
        pAccOptions->SetAlwaysAutoColor(sal_False);
        pAccOptions->SetStopAnimatedGraphics(sal_False);
        pAccOptions->SetStopAnimatedText(sal_False);
    }
    else
    {
        pAccOptions->SetAlwaysAutoColor(rAccessibilityOptions.GetIsAutomaticFontColor());
        pAccOptions->SetStopAnimatedGraphics(! rAccessibilityOptions.GetIsAllowAnimatedGraphics());
        pAccOptions->SetStopAnimatedText(! rAccessibilityOptions.GetIsAllowAnimatedText());

        // Formular view
        // Always set this option, not only if document is read-only:
        pOpt->SetSelectionInReadonly(rAccessibilityOptions.IsSelectionInReadonly());
    }
}

ShellResource* ViewShell::GetShellRes()
{
    return pShellRes;
}

void ViewShell::SetCareWin( Window* pNew )
{
    pCareWindow = pNew;
}

sal_uInt16 ViewShell::GetPageCount() const
{
    return GetLayout() ? GetLayout()->GetPageNum() : 1;
}

const Size ViewShell::GetPageSize( sal_uInt16 nPageNum, bool bSkipEmptyPages ) const
{
    Size aSize;
    const SwRootFrm* pTmpRoot = GetLayout();
    if( pTmpRoot && nPageNum )
    {
        const SwPageFrm* pPage = static_cast<const SwPageFrm*>
                                 (pTmpRoot->Lower());

        while( --nPageNum && pPage->GetNext() )
            pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );

        if( !bSkipEmptyPages && pPage->IsEmptyPage() && pPage->GetNext() )
            pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );

        aSize = pPage->Frm().SSize();
    }
    return aSize;
}


// #i12836# enhanced pdf export
sal_Int32 ViewShell::GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const
{
    OSL_ENSURE( GetLayout(), "GetPageNumAndSetOffsetForPDF assumes presence of layout" );

    sal_Int32 nRet = -1;

    // #i40059# Position out of bounds:
    SwRect aRect( rRect );
    aRect.Pos().X() = Max( aRect.Left(), GetLayout()->Frm().Left() );

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aRect.Center() );
    if ( pPage )
    {
        OSL_ENSURE( pPage, "GetPageNumAndSetOffsetForPDF: No page found" );

        Point aOffset( pPage->Frm().Pos() );
        aOffset.X() = -aOffset.X();
        aOffset.Y() = -aOffset.Y();

        MapMode aMapMode( rOut.GetMapMode() );
        aMapMode.SetOrigin( aOffset );
        rOut.SetMapMode( aMapMode );

        nRet = pPage->GetPhyPageNum() - 1;
    }

    return nRet;
}


// --> PB 2007-05-30 #146850#
const BitmapEx& ViewShell::GetReplacementBitmap( bool bIsErrorState )
{
    BitmapEx** ppRet;
    sal_uInt16 nResId = 0;
    if( bIsErrorState )
    {
        ppRet = &pErrorBmp;
        nResId = RID_GRAPHIC_ERRORBMP;
    }
    else
    {
        ppRet = &pReplaceBmp;
        nResId = RID_GRAPHIC_REPLACEBMP;
    }

    if( !*ppRet )
    {
        *ppRet = new BitmapEx( SW_RES( nResId ) );
    }
    return **ppRet;
}

void ViewShell::DeleteReplacementBitmaps()
{
    DELETEZ( pErrorBmp );
    DELETEZ( pReplaceBmp );
}

SwPostItMgr* ViewShell::GetPostItMgr()
{
    SwView* pView =  GetDoc()->GetDocShell() ? GetDoc()->GetDocShell()->GetView() : 0;
    if ( pView )
        return pView->GetPostItMgr();

    return 0;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* ViewShell::getIDocumentSettingAccess() const { return pDoc; }
IDocumentSettingAccess* ViewShell::getIDocumentSettingAccess() { return pDoc; }
const IDocumentDeviceAccess* ViewShell::getIDocumentDeviceAccess() const { return pDoc; }
IDocumentDeviceAccess* ViewShell::getIDocumentDeviceAccess() { return pDoc; }
const IDocumentMarkAccess* ViewShell::getIDocumentMarkAccess() const { return pDoc->getIDocumentMarkAccess(); }
IDocumentMarkAccess* ViewShell::getIDocumentMarkAccess() { return pDoc->getIDocumentMarkAccess(); }
const IDocumentDrawModelAccess* ViewShell::getIDocumentDrawModelAccess() const { return pDoc; }
IDocumentDrawModelAccess* ViewShell::getIDocumentDrawModelAccess() { return pDoc; }
const IDocumentRedlineAccess* ViewShell::getIDocumentRedlineAccess() const { return pDoc; }
IDocumentRedlineAccess* ViewShell::getIDocumentRedlineAccess() { return pDoc; }
const IDocumentLayoutAccess* ViewShell::getIDocumentLayoutAccess() const { return pDoc; }
IDocumentLayoutAccess* ViewShell::getIDocumentLayoutAccess() { return pDoc; }
IDocumentContentOperations* ViewShell::getIDocumentContentOperations() { return pDoc; }
IDocumentStylePoolAccess* ViewShell::getIDocumentStylePoolAccess() { return pDoc; }
const IDocumentStatistics* ViewShell::getIDocumentStatistics() const { return pDoc; }

IDocumentUndoRedo      & ViewShell::GetIDocumentUndoRedo()
{ return pDoc->GetIDocumentUndoRedo(); }
IDocumentUndoRedo const& ViewShell::GetIDocumentUndoRedo() const
{ return pDoc->GetIDocumentUndoRedo(); }

// --> OD 2007-11-14 #i83479#
const IDocumentListItems* ViewShell::getIDocumentListItemsAccess() const
{
    return pDoc;
}
const IDocumentOutlineNodes* ViewShell::getIDocumentOutlineNodesAccess() const
{
    return pDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
