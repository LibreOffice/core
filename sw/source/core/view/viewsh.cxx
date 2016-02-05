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

#include <config_features.h>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <sfx2/viewfrm.hxx>
#include <sfx2/progress.hxx>
#include <svx/srchdlg.hxx>
#include <svx/svdobj.hxx>
#include <sfx2/viewsh.hxx>
#include <drawdoc.hxx>
#include <swwait.hxx>
#include <swmodule.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
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
#include <vcl/bitmapex.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/alpha.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <accessibilityoptions.hxx>
#include <statstr.hrc>
#include <comcore.hrc>
#include <pagepreviewlayout.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <wrtsh.hxx>
#include <DocumentSettingManager.hxx>

#include <view.hxx>
#include <PostItMgr.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>

#if !HAVE_FEATURE_DESKTOP
#include <vcl/sysdata.hxx>
#endif

bool SwViewShell::mbLstAct = false;
ShellResource *SwViewShell::mpShellRes = nullptr;
VclPtr<vcl::Window> SwViewShell::mpCareWindow = nullptr;

bool bInSizeNotify = false;


using namespace ::com::sun::star;

void SwViewShell::ToggleHeaderFooterEdit()
{
    mbHeaderFooterEdit = !mbHeaderFooterEdit;
    if ( !mbHeaderFooterEdit )
    {
        SetShowHeaderFooterSeparator( Header, false );
        SetShowHeaderFooterSeparator( Footer, false );
    }

    // Avoid corner case
    if ( !IsShowHeaderFooterSeparator( Header ) &&
         !IsShowHeaderFooterSeparator( Footer ) )
    {
        mbHeaderFooterEdit = false;
    }

    // Repaint everything
    GetWin()->Invalidate();
}

void SwViewShell::registerLibreOfficeKitCallback(LibreOfficeKitCallback pCallback, void* pData)
{
    getIDocumentDrawModelAccess().GetDrawModel()->registerLibreOfficeKitCallback(pCallback, pData);
    if (SwPostItMgr* pPostItMgr = GetPostItMgr())
        pPostItMgr->registerLibreOfficeKitCallback(getIDocumentDrawModelAccess().GetDrawModel());
}

void SwViewShell::libreOfficeKitCallback(int nType, const char* pPayload) const
{
    getIDocumentDrawModelAccess().GetDrawModel()->libreOfficeKitCallback(nType, pPayload);
}

void SwViewShell::setOutputToWindow(bool bOutputToWindow)
{
    mbOutputToWindow = bOutputToWindow;
}

bool SwViewShell::isOutputToWindow() const
{
    return mbOutputToWindow;
}

void SwViewShell::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("swViewShell"));
    xmlTextWriterEndElement(pWriter);
}

static void
lcl_PaintTransparentFormControls(SwViewShell & rShell, SwRect const& rRect)
{
    // Direct paint has been performed: the background of transparent child
    // windows has been painted, so need to paint the child windows now.
    if (rShell.GetWin())
    {
        vcl::Window& rWindow = *(rShell.GetWin());
        const Rectangle aRectanglePixel(rShell.GetOut()->LogicToPixel(rRect.SVRect()));
        PaintTransparentChildren(rWindow, aRectanglePixel);
    }
}

// #i72754# 2nd set of Pre/PostPaints
// This time it uses the lock counter (mPrePostPaintRegions empty/non-empty) to allow only one activation
// and deactivation and mpPrePostOutDev to remember the OutDev from the BeginDrawLayers
// call. That way, all places where paint take place can be handled the same way, even
// when calling other paint methods. This is the case at the places where SW paints
// buffered into VDevs to avoid flicker. Tis is in general problematic and should be
// solved once using the BufferedOutput functionality of the DrawView.

void SwViewShell::PrePaint()
{
    // forward PrePaint event from VCL Window to DrawingLayer
    if(HasDrawView())
    {
        Imp()->GetDrawView()->PrePaint();
    }
}

void SwViewShell::DLPrePaint2(const vcl::Region& rRegion)
{
    if(mPrePostPaintRegions.empty())
    {
        mPrePostPaintRegions.push( rRegion );
        // #i75172# ensure DrawView to use DrawingLayer bufferings
        if ( !HasDrawView() )
            MakeDrawView();

        // Prefer window; if not available, get mpOut (e.g. printer)
        const bool bWindow = GetWin() && !comphelper::LibreOfficeKit::isActive() && !isOutputToWindow();
        mpPrePostOutDev = bWindow ? GetWin(): GetOut();

        // #i74769# use SdrPaintWindow now direct
        mpTargetPaintWindow = Imp()->GetDrawView()->BeginDrawLayers(mpPrePostOutDev, rRegion);
        OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

        // #i74769# if prerender, save OutDev and redirect to PreRenderDevice
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            mpBufferedOut = mpOut;
            mpOut = &(mpTargetPaintWindow->GetTargetOutputDevice());
        }
        else if (isOutputToWindow())
            // In case mpOut is used without buffering and we're not printing, need to set clipping.
            mpOut->SetClipRegion(rRegion);

        // remember original paint MapMode for wrapped FlyFrame paints
        maPrePostMapMode = mpOut->GetMapMode();
    }
    else
    {
        // region needs to be updated to the given one
        if( mPrePostPaintRegions.top() != rRegion )
            Imp()->GetDrawView()->UpdateDrawLayersRegion(mpPrePostOutDev, rRegion);
        mPrePostPaintRegions.push( rRegion );
    }
}

void SwViewShell::DLPostPaint2(bool bPaintFormLayer)
{
    OSL_ENSURE(!mPrePostPaintRegions.empty(), "SwViewShell::DLPostPaint2: Pre/PostPaint encapsulation broken (!)");

    if( mPrePostPaintRegions.size() > 1 )
    {
        vcl::Region current = mPrePostPaintRegions.top();
        mPrePostPaintRegions.pop();
        if( current != mPrePostPaintRegions.top())
            Imp()->GetDrawView()->UpdateDrawLayersRegion(mpPrePostOutDev, mPrePostPaintRegions.top());
        return;
    }
    mPrePostPaintRegions.pop(); // clear
    if(nullptr != mpTargetPaintWindow)
    {
        // #i74769# restore buffered OutDev
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            mpOut = mpBufferedOut;
        }

        // #i74769# use SdrPaintWindow now direct
        Imp()->GetDrawView()->EndDrawLayers(*mpTargetPaintWindow, bPaintFormLayer);
        mpTargetPaintWindow = nullptr;
    }
}
// end of Pre/PostPaints

void SwViewShell::ImplEndAction( const bool bIdleEnd )
{
    // Nothing to do for the printer?
    if ( !GetWin() || IsPreview() )
    {
        mbPaintWorks = true;
        UISizeNotify();
        return;
    }

    mbInEndAction = true;
    //will this put the EndAction of the last shell in the sequence?

    SwViewShell::mbLstAct = true;
    for(SwViewShell& rShell : GetRingContainer())
    {
        if(&rShell != this && rShell.ActionPend())
        {
            SwViewShell::mbLstAct = false;
            break;
        }
    }

    const bool bIsShellForCheckViewLayout = ( this == GetLayout()->GetCurrShell() );

    SET_CURR_SHELL( this );
    if ( Imp()->HasDrawView() && !Imp()->GetDrawView()->areMarkHandlesHidden() )
        Imp()->StartAction();

    if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegion();

    const bool bExtraData = ::IsExtraData( GetDoc() );

    if ( !bIdleEnd )
    {
        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( false );
        if ( mnLockPaint )
            aAction.SetPaint( false );
        aAction.SetInputType( VclInputFlags::KEYBOARD );
        aAction.Action(GetWin());
    }

    if ( bIsShellForCheckViewLayout )
        GetLayout()->CheckViewLayout( GetViewOptions(), &maVisArea );

    //If we don't call Paints, we wait for the Paint of the system.
    //Then the clipping is set correctly; e.g. shifting of a Draw object
    if ( Imp()->GetRegion()     ||
         maInvalidRect.HasArea() ||
         bExtraData )
    {
        if ( !mnLockPaint )
        {
            SolarMutexGuard aGuard;

            bool bPaintsFromSystem = maInvalidRect.HasArea();
            GetWin()->Update();
            if ( maInvalidRect.HasArea() )
            {
                if ( bPaintsFromSystem )
                    Imp()->AddPaintRect( maInvalidRect );

                ResetInvalidRect();
                bPaintsFromSystem = true;
            }
            mbPaintWorks = true;

            SwRegionRects *pRegion = Imp()->GetRegion();

            //JP 27.11.97: what hid the selection, must also Show it,
            //             else we get Paint errors!
            // e.g. additional mode, page half visible vertically, in the
            // middle a selection and with an other cursor jump to left
            // right border. Without ShowCursor the selection disappears.
            bool bShowCursor = pRegion && dynamic_cast<const SwCursorShell*>(this) !=  nullptr;
            if( bShowCursor )
                static_cast<SwCursorShell*>(this)->HideCursors();

            if ( pRegion )
            {
                SwRootFrame* pCurrentLayout = GetLayout();

                Imp()->m_pRegion = nullptr;

                //First Invert then Compress, never the other way round!
                pRegion->Invert();

                pRegion->Compress();

                VclPtr<VirtualDevice> pVout;
                while ( !pRegion->empty() )
                {
                    SwRect aRect( pRegion->back() );
                    pRegion->pop_back();

                    bool bPaint = true;
                    if ( IsEndActionByVirDev() )
                    {
                        //create virtual device and set.
                        if ( !pVout )
                            pVout = VclPtr<VirtualDevice>::Create( *GetOut() );
                        MapMode aMapMode( GetOut()->GetMapMode() );
                        pVout->SetMapMode( aMapMode );

                        bool bSizeOK = true;

                        Rectangle aTmp1( aRect.SVRect() );
                        aTmp1 = GetOut()->LogicToPixel( aTmp1 );
                        Rectangle aTmp2( GetOut()->PixelToLogic( aTmp1 ) );
                        if ( aTmp2.Left() > aRect.Left() )
                            aTmp1.Left() = std::max( 0L, aTmp1.Left() - 1L );
                        if ( aTmp2.Top() > aRect.Top() )
                            aTmp1.Top() = std::max( 0L, aTmp1.Top() - 1L );
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
                            bPaint = false;

                            // --> OD 2007-07-26 #i79947#
                            // #i72754# start Pre/PostPaint encapsulation before mpOut is changed to the buffering VDev
                            const vcl::Region aRepaintRegion(aRect.SVRect());
                            DLPrePaint2(aRepaintRegion);
                            // <--

                            OutputDevice  *pOld = GetOut();
                            pVout->SetLineColor( pOld->GetLineColor() );
                            pVout->SetFillColor( pOld->GetFillColor() );
                            Point aOrigin( aRect.Pos() );
                            aOrigin.X() = -aOrigin.X(); aOrigin.Y() = -aOrigin.Y();
                            aMapMode.SetOrigin( aOrigin );
                            pVout->SetMapMode( aMapMode );

                            mpOut = pVout.get();
                            if ( bPaintsFromSystem )
                                PaintDesktop(*mpOut, aRect);
                            pCurrentLayout->Paint( *mpOut, aRect );
                            pOld->DrawOutDev( aRect.Pos(), aRect.SSize(),
                                              aRect.Pos(), aRect.SSize(), *pVout );
                            mpOut = pOld;

                            // #i72754# end Pre/PostPaint encapsulation when mpOut is back and content is painted
                            DLPostPaint2(true);
                        }
                    }
                    if ( bPaint )
                    {
                        if (GetWin() && GetWin()->SupportsDoubleBuffering())
                            InvalidateWindows(aRect.SVRect());
                        else
                        {
                            // #i75172# begin DrawingLayer paint
                            // need to do begin/end DrawingLayer preparation for each single rectangle of the
                            // repaint region. I already tried to prepare only once for the whole Region. This
                            // seems to work (and does technically) but fails with transparent objects. Since the
                            // region given to BeginDarwLayers() defines the clip region for DrawingLayer paint,
                            // transparent objects in the single rectangles will indeed be painted multiple times.
                            DLPrePaint2(vcl::Region(aRect.SVRect()));

                            if ( bPaintsFromSystem )
                                PaintDesktop(*GetOut(), aRect);
                            if (!comphelper::LibreOfficeKit::isActive())
                                pCurrentLayout->Paint( *mpOut, aRect );
                            else
                                pCurrentLayout->GetCurrShell()->InvalidateWindows(aRect.SVRect());

                            // #i75172# end DrawingLayer paint
                            DLPostPaint2(true);
                        }
                    }
                    else
                        lcl_PaintTransparentFormControls(*this, aRect); // i#107365
                }

                pVout.disposeAndClear();
                delete pRegion;
                Imp()->DelRegion();
            }
            if( bShowCursor )
                static_cast<SwCursorShell*>(this)->ShowCursors( true );
        }
        else
        {
            Imp()->DelRegion();
            mbPaintWorks =  true;
        }
    }
    else
        mbPaintWorks = true;

    mbInEndAction = false;
    SwViewShell::mbLstAct = false;
    Imp()->EndAction();

    //We artificially end the action here to enable the automatic scrollbars
    //to adjust themselves correctly
    //EndAction sends a Notify, and that must call Start-/EndAction to
    //adjust the scrollbars correctly
    --mnStartAction;
    UISizeNotify();
    ++mnStartAction;

    if( Imp()->IsAccessible() )
        Imp()->FireAccessibleEvents();
}

void SwViewShell::ImplStartAction()
{
    mbPaintWorks = false;
    Imp()->StartAction();
}

void SwViewShell::ImplLockPaint()
{
    if ( GetWin() && GetWin()->IsVisible() )
        GetWin()->EnablePaint( false ); //Also cut off the controls.
    Imp()->LockPaint();
}

void SwViewShell::ImplUnlockPaint( bool bVirDev )
{
    SET_CURR_SHELL( this );
    if ( GetWin() && GetWin()->IsVisible() )
    {
        if ( (bInSizeNotify || bVirDev ) && VisArea().HasArea() )
        {
            //Refresh with virtual device to avoid flickering.
            VclPtrInstance<VirtualDevice> pVout( *mpOut );
            pVout->SetMapMode( mpOut->GetMapMode() );
            Size aSize( VisArea().SSize() );
            aSize.Width() += 20;
            aSize.Height()+= 20;
            if( pVout->SetOutputSize( aSize ) )
            {
                GetWin()->EnablePaint( true );
                GetWin()->Validate();

                Imp()->UnlockPaint();
                pVout->SetLineColor( mpOut->GetLineColor() );
                pVout->SetFillColor( mpOut->GetFillColor() );

                // #i72754# start Pre/PostPaint encapsulation before mpOut is changed to the buffering VDev
                const vcl::Region aRepaintRegion(VisArea().SVRect());
                DLPrePaint2(aRepaintRegion);

                OutputDevice *pOld = mpOut;
                mpOut = pVout.get();
                Paint(*mpOut, VisArea().SVRect());
                mpOut = pOld;
                mpOut->DrawOutDev( VisArea().Pos(), aSize,
                                  VisArea().Pos(), aSize, *pVout );

                // #i72754# end Pre/PostPaint encapsulation when mpOut is back and content is painted
                DLPostPaint2(true);

                lcl_PaintTransparentFormControls(*this, VisArea()); // fdo#63949
            }
            else
            {
                Imp()->UnlockPaint();
                GetWin()->EnablePaint( true );
                GetWin()->Invalidate( InvalidateFlags::Children );
            }
            pVout.disposeAndClear();
        }
        else
        {
            Imp()->UnlockPaint();
            GetWin()->EnablePaint( true );
            GetWin()->Invalidate( InvalidateFlags::Children );
        }
    }
    else
        Imp()->UnlockPaint();
}

bool SwViewShell::AddPaintRect( const SwRect & rRect )
{
    bool bRet = false;
    for(SwViewShell& rSh : GetRingContainer())
    {
        if( rSh.Imp() )
        {
        if ( rSh.IsPreview() && rSh.GetWin() )
            ::RepaintPagePreview( &rSh, rRect );
        else
                bRet |= rSh.Imp()->AddPaintRect( rRect );
        }
    }
    return bRet;
}

void SwViewShell::InvalidateWindows( const SwRect &rRect )
{
    if ( !Imp()->IsCalcLayoutProgress() )
    {
        for(SwViewShell& rSh : GetRingContainer())
        {
            if ( rSh.GetWin() )
            {
                if ( rSh.IsPreview() )
                    ::RepaintPagePreview( &rSh, rRect );
                // In case of tiled rendering, invalidation is wanted even if
                // the rectangle is outside the visual area.
                else if ( rSh.VisArea().IsOver( rRect ) || comphelper::LibreOfficeKit::isActive() )
                    rSh.GetWin()->Invalidate( rRect.SVRect() );
            }
        }
    }
}

const SwRect& SwViewShell::VisArea() const
{
    // when using the tiled rendering, consider the entire document as our
    // visible area
    return comphelper::LibreOfficeKit::isActive()? GetLayout()->Frame(): maVisArea;
}

void SwViewShell::MakeVisible( const SwRect &rRect )
{
    if ( !VisArea().IsInside( rRect ) || IsScrollMDI( this, rRect ) || GetCareWin(*this) )
    {
        if ( !IsViewLocked() )
        {
            if( mpWin )
            {
                const SwFrame* pRoot = GetLayout();
                int nLoopCnt = 3;
                long nOldH;
                do{
                    nOldH = pRoot->Frame().Height();
                    StartAction();
                    ScrollMDI( this, rRect, USHRT_MAX, USHRT_MAX );
                    EndAction();
                } while( nOldH != pRoot->Frame().Height() && nLoopCnt-- );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                //MA: 04. Nov. 94, no one needs this, does one?
                OSL_ENSURE( false, "Is MakeVisible still needed for printers?" );
            }

#endif
        }
    }
}

vcl::Window* SwViewShell::CareChildWin(SwViewShell& rVSh)
{
    if(rVSh.mpSfxViewShell)
    {
#if HAVE_FEATURE_DESKTOP
        const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = rVSh.mpSfxViewShell->GetViewFrame();
        const SfxChildWindow* pChWin = pVFrame->GetChildWindow( nId );
        vcl::Window *pWin = pChWin ? pChWin->GetWindow() : nullptr;
        if ( pWin && pWin->IsVisible() )
            return pWin;
#endif
    }
    return nullptr;
}

Point SwViewShell::GetPagePos( sal_uInt16 nPageNum ) const
{
    return GetLayout()->GetPagePos( nPageNum );
}

sal_uInt16 SwViewShell::GetNumPages()
{
    //It is possible that no layout exists when the method from
    //root-Ctor is called.
    return GetLayout() ? GetLayout()->GetPageNum() : 0;
}

bool SwViewShell::IsDummyPage( sal_uInt16 nPageNum ) const
{
    return GetLayout() && GetLayout()->IsDummyPage( nPageNum );
}

/**
 * Forces update of each field.
 * It notifies all fields with pNewHt. If that is 0 (default), the field
 * type is sent (???).
 * @param[in] bCloseDB Passed in to GetDoc()->UpdateFields. [TODO] Purpose???
 */
void SwViewShell::UpdateFields(bool bCloseDB)
{
    SET_CURR_SHELL( this );

    bool bCursor = dynamic_cast<const SwCursorShell*>( this ) !=  nullptr;
    if ( bCursor )
        static_cast<SwCursorShell*>(this)->StartAction();
    else
        StartAction();

    GetDoc()->getIDocumentFieldsAccess().UpdateFields(nullptr, bCloseDB);

    if ( bCursor )
        static_cast<SwCursorShell*>(this)->EndAction();
    else
        EndAction();
}

/** update all charts for which any table exists */
void SwViewShell::UpdateAllCharts()
{
    SET_CURR_SHELL( this );
    // Start-/EndAction handled in the SwDoc-Method!
    GetDoc()->UpdateAllCharts();
}

bool SwViewShell::HasCharts() const
{
    bool bRet = false;
    SwNodeIndex aIdx( *GetDoc()->GetNodes().GetEndOfAutotext().
                        StartOfSectionNode(), 1 );
    while (aIdx.GetNode().GetStartNode())
    {
        ++aIdx;
        const SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && !pNd->GetChartTableName().isEmpty() )
        {
            bRet = true;
            break;
        }
    }
    return bRet;
}

void SwViewShell::LayoutIdle()
{
    if( !mpOpt->IsIdle() || !GetWin() ||
        ( Imp()->HasDrawView() && Imp()->GetDrawView()->IsDragObj() ) )
        return;

    //No idle when printing is going on.
    for(SwViewShell& rSh : GetRingContainer())
    {
        if ( !rSh.GetWin() )
            return;
    }

    SET_CURR_SHELL( this );

#ifdef DBG_UTIL
    // If Test5 has been set, the IdleFormatter is disabled.
    if( mpOpt->IsTest5() )
        return;
#endif

    {
        //Prepare and recover cache, so that it will not get fouled.
        SwSaveSetLRUOfst aSave( *SwTextFrame::GetTextCache(),
                             SwTextFrame::GetTextCache()->GetCurMax() - 50 );
        // #125243# there are lots of stacktraces indicating that Imp() returns NULL
        // this SwViewShell seems to be invalid - but it's not clear why
        // this return is only a workaround!
        OSL_ENSURE(Imp(), "SwViewShell already deleted?");
        if(!Imp())
            return;
        SwLayIdle aIdle( GetLayout(), Imp() );
    }
}

static void lcl_InvalidateAllContent( SwViewShell& rSh, sal_uInt8 nInv )
{
    bool bCursor = dynamic_cast<const SwCursorShell*>( &rSh) !=  nullptr;
    if ( bCursor )
        static_cast<SwCursorShell&>(rSh).StartAction();
    else
        rSh.StartAction();
    rSh.GetLayout()->InvalidateAllContent( nInv );
    if ( bCursor )
        static_cast<SwCursorShell&>(rSh).EndAction();
    else
        rSh.EndAction();

    rSh.GetDoc()->getIDocumentState().SetModified();
}

/** local method to invalidate/re-calculate positions of floating screen
 *  objects (Writer fly frame and drawing objects), which are anchored
 *  to paragraph or to character. #i11860#
 */
static void lcl_InvalidateAllObjPos( SwViewShell &_rSh )
{
    const bool bIsCursorShell = dynamic_cast<const SwCursorShell*>( &_rSh) !=  nullptr;
    if ( bIsCursorShell )
        static_cast<SwCursorShell&>(_rSh).StartAction();
    else
        _rSh.StartAction();

    _rSh.GetLayout()->InvalidateAllObjPos();

    if ( bIsCursorShell )
        static_cast<SwCursorShell&>(_rSh).EndAction();
    else
        _rSh.EndAction();

    _rSh.GetDoc()->getIDocumentState().SetModified();
}

void SwViewShell::SetParaSpaceMax( bool bNew )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if( rIDSA.get(DocumentSettingId::PARA_SPACE_MAX) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::PARA_SPACE_MAX, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllContent( *this,  nInv );
    }
}

void SwViewShell::SetParaSpaceMaxAtPages( bool bNew )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if( rIDSA.get(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::PARA_SPACE_MAX_AT_PAGES, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllContent( *this,  nInv );
    }
}

void SwViewShell::SetTabCompat( bool bNew )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if( rIDSA.get(DocumentSettingId::TAB_COMPAT) != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::TAB_COMPAT, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllContent( *this, nInv );
    }
}

void SwViewShell::SetAddExtLeading( bool bNew )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::ADD_EXT_LEADING) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::ADD_EXT_LEADING, bNew );
        SwDrawModel* pTmpDrawModel = getIDocumentDrawModelAccess().GetDrawModel();
        if ( pTmpDrawModel )
            pTmpDrawModel->SetAddExtLeading( bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllContent( *this, nInv );
    }
}

void SwViewShell::SetUseVirDev( bool bNewVirtual )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::USE_VIRTUAL_DEVICE) != bNewVirtual )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        // this sets the flag at the document and calls PrtDataChanged
        IDocumentDeviceAccess& rIDDA = getIDocumentDeviceAccess();
        rIDDA.setReferenceDeviceType( bNewVirtual, true );
    }
}

/** Sets if paragraph and table spacing is added at bottom of table cells.
 * #106629#
 * @param[in] (bool) setting of the new value
 */
void SwViewShell::SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS) != _bAddParaSpacingToTableCells )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS, _bAddParaSpacingToTableCells );
        const sal_uInt8 nInv = INV_PRTAREA;
        lcl_InvalidateAllContent( *this, nInv );
    }
}

/**
 * Sets if former formatting of text lines with proportional line spacing should used.
 * #i11859#
 * @param[in] (bool) setting of the new value
 */
void SwViewShell::SetUseFormerLineSpacing( bool _bUseFormerLineSpacing )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::OLD_LINE_SPACING) != _bUseFormerLineSpacing )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::OLD_LINE_SPACING, _bUseFormerLineSpacing );
        const sal_uInt8 nInv = INV_PRTAREA;
        lcl_InvalidateAllContent( *this, nInv );
    }
}

/**
 * Sets IDocumentSettingAccess if former object positioning should be used.
 * #i11860#
 * @param[in] (bool) setting the new value
 */
void SwViewShell::SetUseFormerObjectPositioning( bool _bUseFormerObjPos )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::USE_FORMER_OBJECT_POS) != _bUseFormerObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::USE_FORMER_OBJECT_POS, _bUseFormerObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}

// #i28701#
void SwViewShell::SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION) != _bConsiderWrapOnObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION, _bConsiderWrapOnObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}

void SwViewShell::SetUseFormerTextWrapping( bool _bUseFormerTextWrapping )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::USE_FORMER_TEXT_WRAPPING) != _bUseFormerTextWrapping )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::USE_FORMER_TEXT_WRAPPING, _bUseFormerTextWrapping );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllContent( *this, nInv );
    }
}

// #i45491#
void SwViewShell::SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    if ( rIDSA.get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK) != _bDoNotJustifyLinesWithManualBreak )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        rIDSA.set(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, _bDoNotJustifyLinesWithManualBreak );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllContent( *this, nInv );
    }
}

void SwViewShell::SetProtectForm( bool _bProtectForm )
{
    IDocumentSettingAccess& rIDSA = getIDocumentSettingAccess();
    rIDSA.set(DocumentSettingId::PROTECT_FORM, _bProtectForm );
}


void SwViewShell::Reformat()
{
    SwWait aWait( *GetDoc()->GetDocShell(), true );

    // we go for safe: get rid of the old font information,
    // when the printer resolution or zoom factor changes.
    // Init() and Reformat() are the safest locations.
    pFntCache->Flush( );

    if( GetLayout()->IsCallbackActionEnabled() )
    {
        StartAction();
        GetLayout()->InvalidateAllContent( INV_SIZE | INV_POS | INV_PRTAREA );
        EndAction();
    }
}

void SwViewShell::ChgNumberDigits()
{
    SdrModel* pTmpDrawModel = getIDocumentDrawModelAccess().GetDrawModel();
    if ( pTmpDrawModel )
           pTmpDrawModel->ReformatAllTextObjects();
    Reformat();
}

void SwViewShell::CalcLayout()
{
    // extremely likely to be a Bad Idea to call this without StartAction
    // (except the Page Preview apparently only has a non-subclassed ViewShell)
    assert((typeid(*this) == typeid(SwViewShell)) || mnStartAction);

    SET_CURR_SHELL( this );
    SwWait aWait( *GetDoc()->GetDocShell(), true );

    //prepare and recover cache, so that it will not get fouled.
    SwSaveSetLRUOfst aSaveLRU( *SwTextFrame::GetTextCache(),
                                  SwTextFrame::GetTextCache()->GetCurMax() - 50 );

    //switch on Progress when none is running yet.
    const bool bEndProgress = SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) == nullptr;
    if ( bEndProgress )
    {
        long nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        ::StartProgress( STR_STATSTR_REFORMAT, 0, nEndPage, GetDoc()->GetDocShell() );
    }

    SwLayAction aAction( GetLayout(), Imp() );
    aAction.SetPaint( false );
    aAction.SetStatBar( true );
    aAction.SetCalcLayout( true );
    aAction.SetReschedule( true );
    GetDoc()->getIDocumentFieldsAccess().LockExpFields();
    aAction.Action(GetOut());
    GetDoc()->getIDocumentFieldsAccess().UnlockExpFields();

    //the SetNewFieldLst() on the Doc was cut off and must be fetched again
    //(see flowfrm.cxx, txtfld.cxx)
    if ( aAction.IsExpFields() )
    {
        aAction.Reset();
        aAction.SetPaint( false );
        aAction.SetStatBar( true );
        aAction.SetReschedule( true );

        SwDocPosUpdate aMsgHint( 0 );
        GetDoc()->getIDocumentFieldsAccess().UpdatePageFields( &aMsgHint );
        GetDoc()->getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);

        aAction.Action(GetOut());
    }

    if ( VisArea().HasArea() )
        InvalidateWindows( VisArea() );
    if ( bEndProgress )
        ::EndProgress( GetDoc()->GetDocShell() );
}

void SwViewShell::SetFirstVisPageInvalid()
{
    for(SwViewShell& rSh : GetRingContainer())
    {
        if ( rSh.Imp() )
            rSh.Imp()->SetFirstVisPageInvalid();
    }
}

void SwViewShell::SizeChgNotify()
{
    if ( !mpWin )
        mbDocSizeChgd = true;
    else if( ActionPend() || Imp()->IsCalcLayoutProgress() || mbPaintInProgress )
    {
        mbDocSizeChgd = true;

        if ( !Imp()->IsCalcLayoutProgress() && dynamic_cast<const SwCursorShell*>( this ) !=  nullptr )
        {
            const SwFrame *pCnt = static_cast<SwCursorShell*>(this)->GetCurrFrame( false );
            const SwPageFrame *pPage;
            if ( pCnt && nullptr != (pPage = pCnt->FindPageFrame()) )
            {
                const sal_uInt16 nVirtNum = pPage->GetVirtPageNum();
                const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
                OUString sDisplay = rNum.GetNumStr( nVirtNum );
                PageNumNotify( this, pCnt->GetPhyPageNum(), nVirtNum, sDisplay );

                if (comphelper::LibreOfficeKit::isActive())
                {
                    Size aDocSize = GetDocSize();
                    std::stringstream ss;
                    ss << aDocSize.Width() + 2L * DOCUMENTBORDER << ", " << aDocSize.Height() + 2L * DOCUMENTBORDER;
                    OString sRect = ss.str().c_str();
                    if (comphelper::LibreOfficeKit::isViewCallback())
                        GetSfxViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, sRect.getStr());
                    else
                        libreOfficeKitCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, sRect.getStr());
                }
            }
        }
    }
    else
    {
        mbDocSizeChgd = false;
        ::SizeNotify( this, GetDocSize() );
    }
}

void SwViewShell::VisPortChgd( const SwRect &rRect)
{
    OSL_ENSURE( GetWin(), "VisPortChgd ohne Window." );

    if ( rRect == VisArea() )
        return;

    // Is someone spuriously rescheduling again?
    SAL_WARN_IF(mbInEndAction, "sw.core", "Scrolling during EndAction");

    //First get the old visible page, so we don't have to look
    //for it afterwards.
    const SwFrame *pOldPage = Imp()->GetFirstVisPage(GetWin());

    const SwRect aPrevArea( VisArea() );
    const bool bFull = aPrevArea.IsEmpty();
    maVisArea = rRect;
    SetFirstVisPageInvalid();

    //When there a PaintRegion still exists and the VisArea has changed,
    //the PaintRegion is at least by now obsolete. The PaintRegion can
    //have been created by RootFrame::Paint.
    if ( !mbInEndAction &&
         Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegion();

    SET_CURR_SHELL( this );

    bool bScrolled = false;

    SwPostItMgr* pPostItMgr = GetPostItMgr();

    if ( bFull )
        GetWin()->Invalidate();
    else
    {
        //Calculate amount to be scrolled.
        const long nXDiff = aPrevArea.Left() - VisArea().Left();
        const long nYDiff = aPrevArea.Top()  - VisArea().Top();

        if( !nXDiff && !GetViewOptions()->getBrowseMode() &&
            (!Imp()->HasDrawView() || !Imp()->GetDrawView()->IsGridVisible() ) )
        {
            // If possible, don't scroll the application background
            // (PaintDesktop).  Also limit the left and right side of
            // the scroll range to the pages.
            const SwPageFrame *pPage = static_cast<SwPageFrame*>(GetLayout()->Lower());
            if ( pPage->Frame().Top() > pOldPage->Frame().Top() )
                pPage = static_cast<const SwPageFrame*>(pOldPage);
            SwRect aBoth( VisArea() );
            aBoth.Union( aPrevArea );
            const SwTwips nBottom = aBoth.Bottom();
            SwTwips nMinLeft = SAL_MAX_INT32;
            SwTwips nMaxRight= 0;

            const bool bBookMode = GetViewOptions()->IsViewLayoutBookMode();

            while ( pPage && pPage->Frame().Top() <= nBottom )
            {
                SwRect aPageRect( pPage->GetBoundRect(GetWin()) );
                if ( bBookMode )
                {
                    const SwPageFrame& rFormatPage = static_cast<const SwPageFrame*>(pPage)->GetFormatPage();
                    aPageRect.SSize() = rFormatPage.GetBoundRect(GetWin()).SSize();
                }

                // #i9719# - consider new border and shadow width
                if ( aPageRect.IsOver( aBoth ) )
                {
                    SwTwips nPageLeft = 0;
                    SwTwips nPageRight = 0;
                    const sw::sidebarwindows::SidebarPosition aSidebarPos = pPage->SidebarPosition();

                    if( aSidebarPos != sw::sidebarwindows::SidebarPosition::NONE )
                    {
                        nPageLeft = aPageRect.Left();
                        nPageRight = aPageRect.Right();
                    }

                    if( nPageLeft < nMinLeft )
                        nMinLeft = nPageLeft;
                    if( nPageRight > nMaxRight )
                        nMaxRight = nPageRight;
                    //match with the draw objects
                    //take nOfst into account as the objects have been
                    //selected and have handles attached.
                    if ( pPage->GetSortedObjs() )
                    {
                        const long nOfst = GetOut()->PixelToLogic(
                            Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width();
                        for ( size_t i = 0; i < pPage->GetSortedObjs()->size(); ++i )
                        {
                            SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                            // ignore objects that are not actually placed on the page
                            if (pObj->IsFormatPossible())
                            {
                                const Rectangle &rBound = pObj->GetObjRect().SVRect();
                                if (rBound.Left() != FAR_AWAY) {
                                    // OD 03.03.2003 #107927# - use correct datatype
                                    const SwTwips nL = std::max( 0L, rBound.Left() - nOfst );
                                    if ( nL < nMinLeft )
                                        nMinLeft = nL;
                                    if( rBound.Right() + nOfst > nMaxRight )
                                        nMaxRight = rBound.Right() + nOfst;
                                }
                            }
                        }
                    }
                }
                pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
            }
            Rectangle aRect( aPrevArea.SVRect() );
            aRect.Left()  = nMinLeft;
            aRect.Right() = nMaxRight;
            if( VisArea().IsOver( aPrevArea ) && !mnLockPaint )
            {
                bScrolled = true;
                maVisArea.Pos() = aPrevArea.Pos();
                if ( SmoothScroll( nXDiff, nYDiff, &aRect ) )
                    return;
                maVisArea.Pos() = rRect.Pos();
            }
            else
                GetWin()->Invalidate( aRect );
        }
        else if ( !mnLockPaint ) //will be released in Unlock
        {
            if( VisArea().IsOver( aPrevArea ) )
            {
                bScrolled = true;
                maVisArea.Pos() = aPrevArea.Pos();
                if ( SmoothScroll( nXDiff, nYDiff, nullptr ) )
                    return;
                maVisArea.Pos() = rRect.Pos();
            }
            else
                GetWin()->Invalidate();
        }
    }

    // When tiled rendering, the map mode of the window is disabled, avoid
    // enabling it here.
    if (!comphelper::LibreOfficeKit::isActive())
    {
        Point aPt( VisArea().Pos() );
        aPt.X() = -aPt.X(); aPt.Y() = -aPt.Y();
        MapMode aMapMode( GetWin()->GetMapMode() );
        aMapMode.SetOrigin( aPt );
        GetWin()->SetMapMode( aMapMode );
    }

    if ( HasDrawView() )
    {
        Imp()->GetDrawView()->VisAreaChanged( GetWin() );
        Imp()->GetDrawView()->SetActualWin( GetWin() );
    }
    GetWin()->Update();

    if ( pPostItMgr ) // #i88070#
    {
        pPostItMgr->Rescale();
        pPostItMgr->CalcRects();
        pPostItMgr->LayoutPostIts();
    }

    if ( !bScrolled && pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        pPostItMgr->CorrectPositions();

    if( Imp()->IsAccessible() )
        Imp()->UpdateAccessible();
}

bool SwViewShell::SmoothScroll( long lXDiff, long lYDiff, const Rectangle *pRect )
{
#if !defined(MACOSX) && !defined(ANDROID) && !defined(IOS)
    // #i98766# - disable smooth scrolling for Mac

    const sal_uLong nColCnt = mpOut->GetColorCount();
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
    const bool bOnlyYScroll(!lXDiff && std::abs(lYDiff) != 0 && std::abs(lYDiff) < lMax);
    const bool bAllowedWithChildWindows(GetWin()->GetWindowClipRegionPixel().IsNull());
    const bool bSmoothScrollAllowed(bOnlyYScroll && mbEnableSmooth && GetViewOptions()->IsSmoothScroll() &&  bAllowedWithChildWindows);

    if(bSmoothScrollAllowed)
    {
        Imp()->m_bStopSmooth = false;

        const SwRect aOldVis( VisArea() );

        //create virtual device and set.
        const Size aPixSz = GetWin()->PixelToLogic(Size(1,1));
        VclPtrInstance<VirtualDevice> pVout( *GetWin() );
        pVout->SetLineColor( GetWin()->GetLineColor() );
        pVout->SetFillColor( GetWin()->GetFillColor() );
        MapMode aMapMode( GetWin()->GetMapMode() );
        pVout->SetMapMode( aMapMode );
        Size aSize( maVisArea.Width()+2*aPixSz.Width(), std::abs(lYDiff)+(2*aPixSz.Height()) );
        if ( pRect )
            aSize.Width() = std::min(aSize.Width(), pRect->GetWidth()+2*aPixSz.Width());
        if ( pVout->SetOutputSize( aSize ) )
        {
            mnLockPaint++;

            //First Paint everything in the virtual device.
            SwRect aRect( VisArea() );
            aRect.Height( aSize.Height() );
            if ( pRect )
            {
                aRect.Pos().X() = std::max(aRect.Left(),pRect->Left()-aPixSz.Width());
                aRect.Right( std::min(aRect.Right()+2*aPixSz.Width(), pRect->Right()+aPixSz.Width()));
            }
            else
                aRect.SSize().Width() += 2*aPixSz.Width();
            aRect.Pos().Y() = lYDiff < 0 ? aOldVis.Bottom() - aPixSz.Height()
                                         : aRect.Top() - aSize.Height() + aPixSz.Height();
            aRect.Pos().X() = std::max( 0L, aRect.Left()-aPixSz.Width() );
            aRect.Pos()  = GetWin()->PixelToLogic( GetWin()->LogicToPixel( aRect.Pos()));
            aRect.SSize()= GetWin()->PixelToLogic( GetWin()->LogicToPixel( aRect.SSize()));
            maVisArea = aRect;
            const Point aPt( -aRect.Left(), -aRect.Top() );
            aMapMode.SetOrigin( aPt );
            pVout->SetMapMode( aMapMode );
            OutputDevice *pOld = mpOut;
            mpOut = pVout.get();

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
                pDrawView->AddWindowToPaintView(pVout, nullptr);

                // clear mpWin during DLPrePaint2 to get paint preparation for mpOut, but set it again
                // immediately afterwards. There are many decisions in SW which imply that Printing
                // is used when mpWin == 0 (wrong but widely used).
                vcl::Window* pOldWin = mpWin;
                mpWin = nullptr;
                DLPrePaint2(vcl::Region(aRect.SVRect()));
                mpWin = pOldWin;

                // SW paint stuff
                PaintDesktop(*GetOut(), aRect);
                SwViewShell::mbLstAct = true;
                GetLayout()->Paint( *GetOut(), aRect );
                SwViewShell::mbLstAct = false;

                // end paint and destroy ObjectContact again
                DLPostPaint2(true);
                pDrawView->DeleteWindowFromPaintView(pVout);
            }

            mpOut = pOld;
            maVisArea = aOldVis;

            //Now shift in parts and copy the new Pixel from the virtual device.

            // ??????????????????????
            // or is it better to get the scrollfactor from the User
            // as option?
            // ??????????????????????
            long lMaDelta = aPixSz.Height();
            if ( std::abs(lYDiff) > ( maVisArea.Height() / 3 ) )
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
                if ( Imp()->m_bStopSmooth || std::abs(lDiff) <= std::abs(lMaDelta) )
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
                maVisArea.Pos().Y() -= lScroll;
                maVisArea.Pos() = GetWin()->PixelToLogic( GetWin()->LogicToPixel( VisArea().Pos()));
                lScroll = aTmpOldVis.Top() - VisArea().Top();
                if ( pRect )
                {
                    Rectangle aTmp( aTmpOldVis.SVRect() );
                    aTmp.Left() = pRect->Left();
                    aTmp.Right()= pRect->Right();
                    GetWin()->Scroll( 0, lScroll, aTmp, ScrollFlags::Children);
                }
                else
                    GetWin()->Scroll( 0, lScroll, ScrollFlags::Children );

                const Point aTmpPt( -VisArea().Left(), -VisArea().Top() );
                MapMode aTmpMapMode( GetWin()->GetMapMode() );
                aTmpMapMode.SetOrigin( aTmpPt );
                GetWin()->SetMapMode( aTmpMapMode );

                if ( Imp()->HasDrawView() )
                    Imp()->GetDrawView()->VisAreaChanged( GetWin() );

                SetFirstVisPageInvalid();
                if ( !Imp()->m_bStopSmooth )
                {
                    const bool bScrollDirectionIsUp(lScroll > 0);
                    Imp()->m_aSmoothRect = VisArea();

                    if(bScrollDirectionIsUp)
                    {
                        Imp()->m_aSmoothRect.Bottom( VisArea().Top() + lScroll + aPixSz.Height());
                    }
                    else
                    {
                        Imp()->m_aSmoothRect.Top( VisArea().Bottom() + lScroll - aPixSz.Height());
                    }

                    Imp()->m_bSmoothUpdate = true;
                    GetWin()->Update();
                    Imp()->m_bSmoothUpdate = false;

                    if(!Imp()->m_bStopSmooth)
                    {
                            // start paint on logic base
                            const Rectangle aTargetLogic(Imp()->m_aSmoothRect.SVRect());
                            DLPrePaint2(vcl::Region(aTargetLogic));

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
                        --mnLockPaint;
                }
            }
            pVout.disposeAndClear();
            GetWin()->Update();
            if ( !Imp()->m_bStopSmooth )
                --mnLockPaint;
            SetFirstVisPageInvalid();
            return true;
        }
        pVout.disposeAndClear();
    }
#endif

    maVisArea.Pos().X() -= lXDiff;
    maVisArea.Pos().Y() -= lYDiff;
    if ( pRect )
        GetWin()->Scroll( lXDiff, lYDiff, *pRect, ScrollFlags::Children);
    else
        GetWin()->Scroll( lXDiff, lYDiff, ScrollFlags::Children);
    return false;
}

void SwViewShell::PaintDesktop(vcl::RenderContext& rRenderContext, const SwRect &rRect)
{
    if ( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return;                     //for the printer we don't do anything here.

    //Catch exceptions, so that it doesn't look so surprising.
    //Can e.g. happen during Idle.
    //Unfortunately we must at any rate Paint the rectangles next to the pages,
    //as these are not painted at VisPortChgd.
    bool bBorderOnly = false;
    const SwRootFrame *pRoot = GetLayout();
    if ( rRect.Top() > pRoot->Frame().Bottom() )
    {
        const SwFrame *pPg = pRoot->Lower();
        while ( pPg && pPg->GetNext() )
            pPg = pPg->GetNext();
        if ( !pPg || !pPg->Frame().IsOver( VisArea() ) )
            bBorderOnly = true;
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
        const SwFrame *pPage =pRoot->Lower();
        SwRect aLeft( rRect ), aRight( rRect );
        while ( pPage )
        {
            long nTmp = pPage->Frame().Left();
            if ( nTmp < aLeft.Right() )
                aLeft.Right( nTmp );
            nTmp = pPage->Frame().Right();
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
        const SwFrame *pPage = Imp()->GetFirstVisPage(&rRenderContext);
        const SwTwips nBottom = rRect.Bottom();
        while ( pPage && !aRegion.empty() &&
                (pPage->Frame().Top() <= nBottom) )
        {
            SwRect aPageRect( pPage->Frame() );
            if ( bBookMode )
            {
                const SwPageFrame& rFormatPage = static_cast<const SwPageFrame*>(pPage)->GetFormatPage();
                aPageRect.SSize() = rFormatPage.Frame().SSize();
            }

            const bool bSidebarRight =
                static_cast<const SwPageFrame*>(pPage)->SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT;
            aPageRect.Pos().X() -= bSidebarRight ? 0 : nSidebarWidth;
            aPageRect.SSize().Width() += nSidebarWidth;

            if ( aPageRect.IsOver( rRect ) )
                aRegion -= aPageRect;

            pPage = pPage->GetNext();
        }
    }
    if ( !aRegion.empty() )
        _PaintDesktop(rRenderContext, aRegion);
}

// PaintDesktop is split in two, this part is also used by PreviewPage
void SwViewShell::_PaintDesktop(vcl::RenderContext& /*rRenderContext*/, const SwRegionRects &rRegion)
{
    // OD 2004-04-23 #116347#
    GetOut()->Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
    GetOut()->SetLineColor();

    for ( auto &rRgn : rRegion )
    {
        const Rectangle aRectangle(rRgn.SVRect());

        // #i93170#
        // Here we have a real Problem. On the one hand we have the buffering for paint
        // and overlay which needs an embracing pair of DLPrePaint2/DLPostPaint2 calls,
        // on the other hand the MapMode is not set correctly when this code is executed.
        // This is done in the users of this method, for each SWpage before painting it.
        // Since the MapMode is not correct here, the call to DLPostPaint2 will paint
        // existing FormControls due to the current MapMode.

        // There are basically three solutions for this:

        // (1) Set the MapMode correct, move the background painting to the users of
        //     this code

        // (2) Do no DLPrePaint2/DLPostPaint2 here; no SdrObjects are allowed to lie in
        //     the desktop region. Disadvantage: the desktop will not be part of the
        //     buffers, e.g. overlay. Thus, as soon as overlay will be used over the
        //     desktop, it will not work.

        // (3) expand DLPostPaint2 with a flag to signal if FormControl paints shall
        //     be done or not

        // Currently, (3) will be the best possible solution. It will keep overlay and
        // buffering intact and work without MapMode for single pages. In the medium
        // to long run, (1) will need to be used and the bool bPaintFormLayer needs
        // to be removed again

        // #i68597# inform Drawinglayer about display change
        DLPrePaint2(vcl::Region(aRectangle));

        // #i75172# needed to move line/Fill color setters into loop since DLPrePaint2
        // may exchange GetOut(), that's it's purpose. This happens e.g. at print preview.
        GetOut()->SetFillColor( SwViewOption::GetAppBackgroundColor());
        GetOut()->SetLineColor();
        GetOut()->DrawRect(aRectangle);

        DLPostPaint2(false);
    }

    GetOut()->Pop();
}

bool SwViewShell::CheckInvalidForPaint( const SwRect &rRect )
{
    if ( !GetWin() )
        return false;

    const SwPageFrame *pPage = Imp()->GetFirstVisPage(GetOut());
    const SwTwips nBottom = VisArea().Bottom();
    const SwTwips nRight  = VisArea().Right();
    bool bRet = false;
    while ( !bRet && pPage && !((pPage->Frame().Top()  > nBottom) ||
                                   (pPage->Frame().Left() > nRight)))
    {
        if ( pPage->IsInvalid() || pPage->IsInvalidFly() )
            bRet = true;
        pPage = static_cast<const SwPageFrame*>(pPage->GetNext());
    }

    if ( bRet )
    {
        //Unfortunately Start/EndAction won't help here, as the Paint originated
        //from GUI and so Clipping has been set against getting through.
        //Ergo: do it all yourself (see ImplEndAction())
        if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea())
             Imp()->DelRegion();

        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( false );
        // We increment the action counter to avoid a recursive call of actions
        // e.g. from a SwFEShell::RequestObjectResize(..) in bug 95829.
        // A recursive call of actions is no good idea because the inner action
        // can't format frames which are locked by the outer action. This may
        // cause and endless loop.
        ++mnStartAction;
        aAction.Action(GetWin());
        --mnStartAction;

        SwRegionRects *pRegion = Imp()->GetRegion();
        if ( pRegion && aAction.IsBrowseActionStop() )
        {
            //only of interest when something has changed in the visible range
            bool bStop = true;
            for ( size_t i = 0; i < pRegion->size(); ++i )
            {
                const SwRect &rTmp = (*pRegion)[i];
                if ( !(bStop = rTmp.IsOver( VisArea() )) )
                    break;
            }
            if ( bStop )
            {
                Imp()->DelRegion();
                pRegion = nullptr;
            }
        }

        if ( pRegion )
        {
            //First Invert then Compress, never the other way round!
            pRegion->Invert();
            pRegion->Compress();
            bRet = false;
            if ( !pRegion->empty() )
            {
                SwRegionRects aRegion( rRect );
                for ( size_t i = 0; i < pRegion->size(); ++i )
                {   const SwRect &rTmp = (*pRegion)[i];
                    if ( !rRect.IsInside( rTmp ) )
                    {
                        InvalidateWindows( rTmp );
                        if ( rTmp.IsOver( VisArea() ) )
                        {   aRegion -= rTmp;
                            bRet = true;
                        }
                    }
                }
                if ( bRet )
                {
                    for ( size_t i = 0; i < aRegion.size(); ++i )
                        GetWin()->Invalidate( aRegion[i].SVRect() );

                    if ( rRect != VisArea() )
                    {
                        //rRect == VisArea is the special case for new or
                        //Shift-Ctrl-R, when it shouldn't be necessary to
                        //hold the rRect again in Document coordinates.
                        if ( maInvalidRect.IsEmpty() )
                            maInvalidRect = rRect;
                        else
                            maInvalidRect.Union( rRect );
                    }
                }
            }
            else
                bRet = false;
            Imp()->DelRegion();
        }
        else
            bRet = false;
    }
    return bRet;
}

namespace
{
/// Similar to comphelper::FlagRestorationGuard, but for vcl::RenderContext.
class RenderContextGuard
{
    VclPtr<vcl::RenderContext>& m_pRef;
    VclPtr<vcl::RenderContext> m_pOriginalValue;
    SwViewShell* m_pShell;

public:
    RenderContextGuard(VclPtr<vcl::RenderContext>& pRef, vcl::RenderContext* pValue, SwViewShell* pShell)
        : m_pRef(pRef),
        m_pOriginalValue(m_pRef),
        m_pShell(pShell)
    {
        m_pRef = pValue;
        if (pValue != m_pShell->GetWin() && m_pShell->Imp()->GetDrawView())
            m_pShell->Imp()->GetDrawView()->AddWindowToPaintView(pValue, m_pShell->GetWin());
    }

    ~RenderContextGuard()
    {
        if (m_pRef != m_pShell->GetWin() && m_pShell->Imp()->GetDrawView())
        {
            // Need to explicitly draw the overlay on m_pRef, since by default
            // they would be only drawn for m_pOriginalValue.
            SdrPaintWindow* pOldPaintWindow = m_pShell->Imp()->GetDrawView()->GetPaintWindow(0);
            rtl::Reference<sdr::overlay::OverlayManager> xOldManager = pOldPaintWindow->GetOverlayManager();
            if (xOldManager.is())
            {
                if (SdrPaintWindow* pNewPaintWindow = m_pShell->Imp()->GetDrawView()->FindPaintWindow(*m_pRef))
                    xOldManager->completeRedraw(pNewPaintWindow->GetRedrawRegion(), m_pRef);
            }

            m_pShell->Imp()->GetDrawView()->DeleteWindowFromPaintView(m_pRef);
        }
        m_pRef = m_pOriginalValue;
    }
};
}

void SwViewShell::Paint(vcl::RenderContext& rRenderContext, const Rectangle &rRect)
{
    RenderContextGuard aGuard(mpOut, &rRenderContext, this);
    if ( mnLockPaint )
    {
        if ( Imp()->m_bSmoothUpdate )
        {
            SwRect aTmp( rRect );
            if ( !Imp()->m_aSmoothRect.IsInside( aTmp ) )
                Imp()->m_bStopSmooth = true;
            else
            {
                Imp()->m_aSmoothRect = aTmp;
                return;
            }
        }
        else
            return;
    }

    if ( SwRootFrame::IsInPaint() )
    {
        //During the publication of a page at printing the Paint is buffered.
        SwPaintQueue::Add( this, SwRect( rRect ) );
        return;
    }

    //With !nStartAction I try to protect me against erroneous code at other places.
    //Hopefully it will not lead to problems!?
    if ( mbPaintWorks && !mnStartAction )
    {
        if( GetWin() && GetWin()->IsVisible() )
        {
            SwRect aRect( rRect );
            if ( mbPaintInProgress ) //Guard against double Paints!
            {
                GetWin()->Invalidate( rRect );
                return;
            }

            mbPaintInProgress = true;
            SET_CURR_SHELL( this );
            SwRootFrame::SetNoVirDev( true );

            //We don't want to Clip to and fro, we trust that all are limited
            //to the rectangle and only need to calculate the clipping once.
            //The ClipRect is removed here once and not recovered, as externally
            //no one needs it anymore anyway.
            //Not when we paint a Metafile.
            if( !GetOut()->GetConnectMetaFile() && GetOut()->IsClipRegion())
                GetOut()->SetClipRegion();

            if ( IsPreview() )
            {
                //When useful, process or destroy the old InvalidRect.
                if ( aRect.IsInside( maInvalidRect ) )
                    ResetInvalidRect();
                SwViewShell::mbLstAct = true;
                GetLayout()->Paint( rRenderContext, aRect );
                SwViewShell::mbLstAct = false;
            }
            else
            {
                //When one of the visible pages still has anything entered for
                //Repaint, Repaint must be triggered.
                if ( !CheckInvalidForPaint( aRect ) )
                {
                    // --> OD 2009-08-12 #i101192#
                    // start Pre/PostPaint encapsulation to avoid screen blinking
                    const vcl::Region aRepaintRegion(aRect.SVRect());
                    DLPrePaint2(aRepaintRegion);

                    // <--
                    PaintDesktop(rRenderContext, aRect);

                    //When useful, process or destroy the old InvalidRect.
                    if ( aRect.IsInside( maInvalidRect ) )
                        ResetInvalidRect();
                    SwViewShell::mbLstAct = true;
                    GetLayout()->Paint( rRenderContext, aRect );
                    SwViewShell::mbLstAct = false;
                    // --> OD 2009-08-12 #i101192#
                    // end Pre/PostPaint encapsulation
                    DLPostPaint2(true);
                    // <--
                }
            }
            SwRootFrame::SetNoVirDev( false );
            mbPaintInProgress = false;
            UISizeNotify();
        }
    }
    else
    {
        if ( maInvalidRect.IsEmpty() )
            maInvalidRect = SwRect( rRect );
        else
            maInvalidRect.Union( SwRect( rRect ) );

        if ( mbInEndAction && GetWin() )
        {
            const vcl::Region aRegion(GetWin()->GetPaintRegion());
            RectangleVector aRectangles;
            aRegion.GetRegionRectangles(aRectangles);

            for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
            {
                Imp()->AddPaintRect(*aRectIter);
            }

            //RegionHandle hHdl( aRegion.BeginEnumRects() );
            //Rectangle aRect;
            //while ( aRegion.GetEnumRects( hHdl, aRect ) )
            //  Imp()->AddPaintRect( aRect );
            //aRegion.EndEnumRects( hHdl );
        }
        else if ( SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) &&
                  GetOut() == GetWin() )
        {
            // #i68597#
            const vcl::Region aDLRegion(rRect);
            DLPrePaint2(aDLRegion);

            rRenderContext.Push( PushFlags::FILLCOLOR|PushFlags::LINECOLOR );
            rRenderContext.SetFillColor( Imp()->GetRetoucheColor() );
            rRenderContext.SetLineColor();
            rRenderContext.DrawRect( rRect );
            rRenderContext.Pop();
            // #i68597#
            DLPostPaint2(true);
        }
    }
}

void SwViewShell::PaintTile(VirtualDevice &rDevice, int contextWidth, int contextHeight, int tilePosX, int tilePosY, long tileWidth, long tileHeight)
{
    // SwViewShell's output device setup
    // TODO clean up SwViewShell's approach to output devices (the many of
    // them - mpBufferedOut, mpOut, mpWin, ...)
    OutputDevice *pSaveOut = mpOut;
    mbInLibreOfficeKitCallback = true;
    mpOut = &rDevice;

    // resizes the virtual device so to contain the entries context
    rDevice.SetOutputSizePixel(Size(contextWidth, contextHeight));

    // setup the output device to draw the tile
    MapMode aMapMode(rDevice.GetMapMode());
    aMapMode.SetMapUnit(MAP_TWIP);
    aMapMode.SetOrigin(Point(-tilePosX, -tilePosY));

    // Scaling. Must convert from pixels to twips. We know
    // that VirtualDevices use a DPI of 96.
    Fraction scaleX = Fraction(contextWidth, 96) * Fraction(1440L) / Fraction(tileWidth);
    Fraction scaleY = Fraction(contextHeight, 96) * Fraction(1440L) / Fraction(tileHeight);
    aMapMode.SetScaleX(scaleX);
    aMapMode.SetScaleY(scaleY);
    rDevice.SetMapMode(aMapMode);

    // Update scaling of SwEditWin and its sub-widgets, needed for comments.
    if (GetWin() && GetWin()->GetMapMode().GetScaleX() != scaleX)
    {
        double fScale = scaleX;
        SwViewOption aOption(*GetViewOptions());
        aOption.SetZoom(fScale * 100);
        ApplyViewOptions(aOption);
        // Make sure the map mode (disabled in SwXTextDocument::initializeForTiledRendering()) is still disabled.
        GetWin()->EnableMapMode(false);
    }

    Rectangle aOutRect = Rectangle(Point(tilePosX, tilePosY),
                                   rDevice.PixelToLogic(Size(contextWidth, contextHeight)));

    // Make the requested area visible -- we can't use MakeVisible as that will
    // only scroll the contents, but won't zoom/resize if needed.
    // Without this, items/text that are outside the visible area (in the SwView)
    // won't be painted when rendering tiles (at least when using either the
    // tiledrendering app, or the gtktiledviewer) -- although ultimately we
    // probably want to fix things so that the SwView's area doesn't affect
    // tiled rendering?
    VisPortChgd(SwRect(aOutRect));

    // Invoke SwLayAction if layout is not yet ready.
    CheckInvalidForPaint(aOutRect);

    // draw - works in logic coordinates
    Paint(rDevice, aOutRect);

    if (SwPostItMgr* pPostItMgr = GetPostItMgr())
        pPostItMgr->PaintTile(rDevice, aOutRect);

    // SwViewShell's output device tear down
    mpOut = pSaveOut;
    mbInLibreOfficeKitCallback = false;
}

void SwViewShell::SetBrowseBorder( const Size& rNew )
{
    if( rNew != maBrowseBorder )
    {
        maBrowseBorder = rNew;
        if ( maVisArea.HasArea() )
            InvalidateLayout( false );
    }
}

const Size& SwViewShell::GetBrowseBorder() const
{
    return maBrowseBorder;
}

sal_Int32 SwViewShell::GetBrowseWidth() const
{
    const SwPostItMgr* pPostItMgr = GetPostItMgr();
    if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
    {
        Size aBorder( maBrowseBorder );
        aBorder.Width() += maBrowseBorder.Width();
        aBorder.Width() += pPostItMgr->GetSidebarWidth(true) + pPostItMgr->GetSidebarBorderWidth(true);
        return maVisArea.Width() - GetOut()->PixelToLogic(aBorder).Width();
    }
    else
        return maVisArea.Width() - 2 * GetOut()->PixelToLogic(maBrowseBorder).Width();
}

void SwViewShell::InvalidateLayout( bool bSizeChanged )
{
    if ( !bSizeChanged && !GetViewOptions()->getBrowseMode() &&
         !GetViewOptions()->IsWhitespaceHidden() )
        return;

    SET_CURR_SHELL( this );

    OSL_ENSURE( GetLayout(), "Layout not ready" );

    // When the Layout doesn't have a height yet, nothing is formatted.
    // That leads to problems with Invalidate, e.g. when setting up an new View
    // the content is inserted and formatted (regardless of empty VisArea).
    // Therefore the pages must be roused for formatting.
    if( !GetLayout()->Frame().Height() )
    {
        SwFrame* pPage = GetLayout()->Lower();
        while( pPage )
        {
            pPage->_InvalidateSize();
            pPage = pPage->GetNext();
        }
        return;
    }

    LockPaint();
    StartAction();

    SwPageFrame *pPg = static_cast<SwPageFrame*>(GetLayout()->Lower());
    do
    {   pPg->InvalidateSize();
        pPg->_InvalidatePrt();
        pPg->InvaPercentLowers();
        if ( bSizeChanged )
        {
            pPg->PrepareHeader();
            pPg->PrepareFooter();
        }
        pPg = static_cast<SwPageFrame*>(pPg->GetNext());
    } while ( pPg );

    // When the size ratios in browse mode change,
    // the Position and PrtArea of the Content and Tab frames must be Invalidated.
    sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_POS;
    // In case of layout or mode change, the ContentFrames need a size-Invalidate
    // because of printer/screen formatting.
    if ( bSizeChanged )
        nInv |= INV_SIZE | INV_DIRECTION;

    GetLayout()->InvalidateAllContent( nInv );

    SwFrame::CheckPageDescs( static_cast<SwPageFrame*>(GetLayout()->Lower()) );

    EndAction();
    UnlockPaint();
}

SwRootFrame *SwViewShell::GetLayout() const
{
    return mpLayout.get();
}

OutputDevice& SwViewShell::GetRefDev() const
{
    OutputDevice* pTmpOut = nullptr;
    if (  GetWin() &&
          GetViewOptions()->getBrowseMode() &&
         !GetViewOptions()->IsPrtFormat() )
        pTmpOut = GetWin();
    else if ( mpTmpRef )
        pTmpOut = mpTmpRef;
    else
        pTmpOut = GetDoc()->getIDocumentDeviceAccess().getReferenceDevice( true );

    return *pTmpOut;
}

const SwNodes& SwViewShell::GetNodes() const
{
    return mpDoc->GetNodes();
}

void SwViewShell::DrawSelChanged()
{
}

Size SwViewShell::GetDocSize() const
{
    Size aSz;
    const SwRootFrame* pRoot = GetLayout();
    if( pRoot )
        aSz = pRoot->Frame().SSize();

    return aSz;
}

SfxItemPool& SwViewShell::GetAttrPool()
{
    return GetDoc()->GetAttrPool();
}

void SwViewShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    for(SwViewShell& rSh : GetRingContainer())
        rSh.StartAction();

    ImplApplyViewOptions( rOpt );

    // With one layout per view it is not longer necessary
    // to sync these "layout related" view options
    // But as long as we have to disable "multiple layout"

    for(SwViewShell& rSh : GetRingContainer())
    {
        if(&rSh == this)
            continue;
        SwViewOption aOpt( *rSh.GetViewOptions() );
        aOpt.SetFieldName( rOpt.IsFieldName() );
        aOpt.SetShowHiddenField( rOpt.IsShowHiddenField() );
        aOpt.SetShowHiddenPara( rOpt.IsShowHiddenPara() );
        aOpt.SetShowHiddenChar( rOpt.IsShowHiddenChar() );
        aOpt.SetViewLayoutBookMode( rOpt.IsViewLayoutBookMode() );
        aOpt.SetHideWhitespaceMode(rOpt.IsHideWhitespaceMode());
        aOpt.SetViewLayoutColumns(rOpt.GetViewLayoutColumns());
        aOpt.SetPostIts(rOpt.IsPostIts());
        if ( !(aOpt == *rSh.GetViewOptions()) )
            rSh.ImplApplyViewOptions( aOpt );
    }
    // End of disabled multiple window

    for(SwViewShell& rSh : GetRingContainer())
        rSh.EndAction();
}

void SwViewShell::ImplApplyViewOptions( const SwViewOption &rOpt )
{
    if (*mpOpt == rOpt)
        return;

    vcl::Window *pMyWin = GetWin();
    if( !pMyWin )
    {
        OSL_ENSURE( pMyWin, "SwViewShell::ApplyViewOptions: no window" );
        return;
    }

    SET_CURR_SHELL( this );

    bool bReformat   = false;

    if( mpOpt->IsShowHiddenField() != rOpt.IsShowHiddenField() )
    {
        static_cast<SwHiddenTextFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_HIDDENTXTFLD ))->
                                            SetHiddenFlag( !rOpt.IsShowHiddenField() );
        bReformat = true;
    }
    if ( mpOpt->IsShowHiddenPara() != rOpt.IsShowHiddenPara() )
    {
        SwHiddenParaFieldType* pFieldType = static_cast<SwHiddenParaFieldType*>(GetDoc()->
                                          getIDocumentFieldsAccess().GetSysFieldType(RES_HIDDENPARAFLD));
        if( pFieldType && pFieldType->HasWriterListeners() )
        {
            SwMsgPoolItem aHint( RES_HIDDENPARA_PRINT );
            pFieldType->ModifyNotification( &aHint, nullptr);
        }
        bReformat = true;
    }
    if ( !bReformat && mpOpt->IsShowHiddenChar() != rOpt.IsShowHiddenChar() )
    {
        bReformat = GetDoc()->ContainsHiddenChars();
    }

    // bReformat becomes true, if ...
    // - fieldnames apply or not ...
    // ( - SwEndPortion must _no_ longer be generated. )
    // - Of course, the screen is something completely different than the printer ...
    bReformat = bReformat || mpOpt->IsFieldName() != rOpt.IsFieldName();

    // The map mode is changed, minima/maxima will be attended by UI
    if( mpOpt->GetZoom() != rOpt.GetZoom() && !IsPreview() )
    {
        MapMode aMode( pMyWin->GetMapMode() );
        Fraction aNewFactor( rOpt.GetZoom(), 100 );
        aMode.SetScaleX( aNewFactor );
        aMode.SetScaleY( aNewFactor );
        pMyWin->SetMapMode( aMode );
        // if not a reference device (printer) is used for formatting,
        // but the screen, new formatting is needed for zoomfactor changes.
        if (mpOpt->getBrowseMode() || mpOpt->IsWhitespaceHidden())
            bReformat = true;
    }

    bool bBrowseModeChanged = false;
    if( mpOpt->getBrowseMode() != rOpt.getBrowseMode() )
    {
        bBrowseModeChanged = true;
        bReformat = true;
    }
    else if( mpOpt->getBrowseMode() && mpOpt->IsPrtFormat() != rOpt.IsPrtFormat() )
        bReformat = true;

    bool bHideWhitespaceModeChanged = false;
    if (mpOpt->IsWhitespaceHidden() != rOpt.IsWhitespaceHidden())
    {
        // When whitespace is hidden, view change needs reformatting.
        bHideWhitespaceModeChanged = true;
        bReformat = true;
    }

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

    bool bOnlineSpellChgd = mpOpt->IsOnlineSpell() != rOpt.IsOnlineSpell();

    *mpOpt = rOpt;   // First the options are taken.
    mpOpt->SetUIOptions(rOpt);

    mpDoc->GetDocumentSettingManager().set(DocumentSettingId::HTML_MODE, 0 != ::GetHtmlMode(mpDoc->GetDocShell()));

    if( bBrowseModeChanged || bHideWhitespaceModeChanged )
    {
        // #i44963# Good occasion to check if page sizes in
        // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
        mpDoc->CheckDefaultPageFormat();
        InvalidateLayout( true );
    }

    pMyWin->Invalidate();
    if ( bReformat )
    {
        // Nothing helps, we need to send all ContentFrames a
        // Prepare, we format anew:
        StartAction();
        Reformat();
        EndAction();
    }

    if( bOnlineSpellChgd )
    {
        bool bOnlineSpl = rOpt.IsOnlineSpell();
        for(SwViewShell& rSh : GetRingContainer())
        {
            if(&rSh == this)
                continue;
            rSh.mpOpt->SetOnlineSpell( bOnlineSpl );
            vcl::Window *pTmpWin = rSh.GetWin();
            if( pTmpWin )
                pTmpWin->Invalidate();
        }
    }

}

void SwViewShell::SetUIOptions( const SwViewOption &rOpt )
{
    mpOpt->SetUIOptions(rOpt);
    //the API-Flag of the view options is set but never reset
    //it is required to set scroll bars in readonly documents
    if(rOpt.IsStarOneSetting())
        mpOpt->SetStarOneSetting(true);

    mpOpt->SetSymbolFont(rOpt.GetSymbolFont());
}

void SwViewShell::SetReadonlyOption(bool bSet)
{
    //JP 01.02.99: at readonly flag query properly
    //              and if need be format; Bug 61335

    // Are we switching from readonly to edit?
    if( bSet != mpOpt->IsReadonly() )
    {
        // so that the flags can be queried properly.
        mpOpt->SetReadonly( false );

        bool bReformat = mpOpt->IsFieldName();

        mpOpt->SetReadonly( bSet );

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
            Imp()->InvalidateAccessibleEditableState( false );
    }
}

void  SwViewShell::SetPDFExportOption(bool bSet)
{
    if( bSet != mpOpt->IsPDFExport() )
    {
        if( bSet && mpOpt->getBrowseMode() )
            mpOpt->SetPrtFormat( true );
        mpOpt->SetPDFExport(bSet);
    }
}

void  SwViewShell::SetReadonlySelectionOption(bool bSet)
{
    if( bSet != mpOpt->IsSelectionInReadonly() )
    {
        mpOpt->SetSelectionInReadonly(bSet);
    }
}

void SwViewShell::SetPrtFormatOption( bool bSet )
{
    mpOpt->SetPrtFormat( bSet );
}

void SwViewShell::UISizeNotify()
{
    if ( mbDocSizeChgd )
    {
        mbDocSizeChgd = false;
        bool bOld = bInSizeNotify;
        bInSizeNotify = true;
        ::SizeNotify( this, GetDocSize() );
        bInSizeNotify = bOld;
    }
}

void    SwViewShell::SetRestoreActions(sal_uInt16 nSet)
{
    OSL_ENSURE(!GetRestoreActions()||!nSet, "multiple restore of the Actions ?");
    Imp()->SetRestoreActions(nSet);
}
sal_uInt16  SwViewShell::GetRestoreActions() const
{
    return Imp()->GetRestoreActions();
}

bool SwViewShell::IsNewLayout() const
{
    return GetLayout()->IsNewLayout();
}

uno::Reference< css::accessibility::XAccessible > SwViewShell::CreateAccessible()
{
    uno::Reference< css::accessibility::XAccessible > xAcc;

    // We require a layout and an XModel to be accessible.
    OSL_ENSURE( mpLayout, "no layout, no access" );
    OSL_ENSURE( GetWin(), "no window, no access" );

    if( mpDoc->getIDocumentLayoutAccess().GetCurrentViewShell() && GetWin() )
        xAcc = Imp()->GetAccessibleMap().GetDocumentView();

    return xAcc;
}

uno::Reference< css::accessibility::XAccessible > SwViewShell::CreateAccessiblePreview()
{
    OSL_ENSURE( IsPreview(),
                "Can't create accessible preview for non-preview SwViewShell" );

    // We require a layout and an XModel to be accessible.
    OSL_ENSURE( mpLayout, "no layout, no access" );
    OSL_ENSURE( GetWin(), "no window, no access" );

    if ( IsPreview() && GetLayout()&& GetWin() )
    {
        return Imp()->GetAccessibleMap().GetDocumentPreview(
                    PagePreviewLayout()->maPreviewPages,
                    GetWin()->GetMapMode().GetScaleX(),
                    GetLayout()->GetPageByPageNum( PagePreviewLayout()->mnSelectedPageNum ),
                    PagePreviewLayout()->maWinSize );
    }
    return nullptr;
}

void SwViewShell::InvalidateAccessibleFocus()
{
    if( Imp() && Imp()->IsAccessible() )
        Imp()->GetAccessibleMap().InvalidateFocus();
}

/**
 * invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs #i27138#
 */
void SwViewShell::InvalidateAccessibleParaFlowRelation( const SwTextFrame* _pFromTextFrame,
                                                      const SwTextFrame* _pToTextFrame )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaFlowRelation( _pFromTextFrame, _pToTextFrame );
    }
}

/**
 * invalidate text selection for paragraphs #i27301#
 */
void SwViewShell::InvalidateAccessibleParaTextSelection()
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaTextSelection();
    }
}

/**
 * invalidate attributes for paragraphs #i88069#
 */
void SwViewShell::InvalidateAccessibleParaAttrs( const SwTextFrame& rTextFrame )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaAttrs( rTextFrame );
    }
}

SwAccessibleMap* SwViewShell::GetAccessibleMap()
{
    if ( Imp()->IsAccessible() )
    {
        return &(Imp()->GetAccessibleMap());
    }

    return nullptr;
}

void SwViewShell::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    if(mpOpt->IsPagePreview() && !rAccessibilityOptions.GetIsForPagePreviews())
    {
        mpAccOptions->SetAlwaysAutoColor(false);
        mpAccOptions->SetStopAnimatedGraphics(false);
        mpAccOptions->SetStopAnimatedText(false);
    }
    else
    {
        mpAccOptions->SetAlwaysAutoColor(rAccessibilityOptions.GetIsAutomaticFontColor());
        mpAccOptions->SetStopAnimatedGraphics(! rAccessibilityOptions.GetIsAllowAnimatedGraphics());
        mpAccOptions->SetStopAnimatedText(! rAccessibilityOptions.GetIsAllowAnimatedText());

        // Formular view
        // Always set this option, not only if document is read-only:
        mpOpt->SetSelectionInReadonly(rAccessibilityOptions.IsSelectionInReadonly());
    }
}

ShellResource* SwViewShell::GetShellRes()
{
    return mpShellRes;
}

void SwViewShell::SetCareWin( vcl::Window* pNew )
{
    mpCareWindow = pNew;
}

sal_uInt16 SwViewShell::GetPageCount() const
{
    return GetLayout() ? GetLayout()->GetPageNum() : 1;
}

const Size SwViewShell::GetPageSize( sal_uInt16 nPageNum, bool bSkipEmptyPages ) const
{
    Size aSize;
    const SwRootFrame* pTmpRoot = GetLayout();
    if( pTmpRoot && nPageNum )
    {
        const SwPageFrame* pPage = static_cast<const SwPageFrame*>
                                 (pTmpRoot->Lower());

        while( --nPageNum && pPage->GetNext() )
            pPage = static_cast<const SwPageFrame*>( pPage->GetNext() );

        if( !bSkipEmptyPages && pPage->IsEmptyPage() && pPage->GetNext() )
            pPage = static_cast<const SwPageFrame*>( pPage->GetNext() );

        aSize = pPage->Frame().SSize();
    }
    return aSize;
}

// #i12836# enhanced pdf export
sal_Int32 SwViewShell::GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const
{
    OSL_ENSURE( GetLayout(), "GetPageNumAndSetOffsetForPDF assumes presence of layout" );

    sal_Int32 nRet = -1;

    // #i40059# Position out of bounds:
    SwRect aRect( rRect );
    aRect.Pos().X() = std::max( aRect.Left(), GetLayout()->Frame().Left() );

    const SwPageFrame* pPage = GetLayout()->GetPageAtPos( aRect.Center() );
    if ( pPage )
    {
        OSL_ENSURE( pPage, "GetPageNumAndSetOffsetForPDF: No page found" );

        Point aOffset( pPage->Frame().Pos() );
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
const BitmapEx& SwViewShell::GetReplacementBitmap( bool bIsErrorState )
{
    BitmapEx** ppRet;
    sal_uInt16 nResId = 0;
    if( bIsErrorState )
    {
        ppRet = &m_pErrorBmp;
        nResId = RID_GRAPHIC_ERRORBMP;
    }
    else
    {
        ppRet = &m_pReplaceBmp;
        nResId = RID_GRAPHIC_REPLACEBMP;
    }

    if( !*ppRet )
    {
        *ppRet = new BitmapEx( SW_RES( nResId ) );
    }
    return **ppRet;
}

void SwViewShell::DeleteReplacementBitmaps()
{
    DELETEZ( m_pErrorBmp );
    DELETEZ( m_pReplaceBmp );
}

SwPostItMgr* SwViewShell::GetPostItMgr()
{
    SwView* pView =  GetDoc()->GetDocShell() ? GetDoc()->GetDocShell()->GetView() : nullptr;
    if ( pView )
        return pView->GetPostItMgr();

    return nullptr;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess& SwViewShell::getIDocumentSettingAccess() const { return mpDoc->GetDocumentSettingManager(); }
IDocumentSettingAccess& SwViewShell::getIDocumentSettingAccess() { return mpDoc->GetDocumentSettingManager(); }
const IDocumentDeviceAccess& SwViewShell::getIDocumentDeviceAccess() const { return mpDoc->getIDocumentDeviceAccess(); }
IDocumentDeviceAccess& SwViewShell::getIDocumentDeviceAccess() { return mpDoc->getIDocumentDeviceAccess(); }
const IDocumentMarkAccess* SwViewShell::getIDocumentMarkAccess() const { return mpDoc->getIDocumentMarkAccess(); }
IDocumentMarkAccess* SwViewShell::getIDocumentMarkAccess() { return mpDoc->getIDocumentMarkAccess(); }
const IDocumentDrawModelAccess& SwViewShell::getIDocumentDrawModelAccess() const { return mpDoc->getIDocumentDrawModelAccess(); }
IDocumentDrawModelAccess& SwViewShell::getIDocumentDrawModelAccess() { return mpDoc->getIDocumentDrawModelAccess(); }
const IDocumentRedlineAccess& SwViewShell::getIDocumentRedlineAccess() const { return mpDoc->getIDocumentRedlineAccess(); }
IDocumentRedlineAccess& SwViewShell::getIDocumentRedlineAccess() { return mpDoc->getIDocumentRedlineAccess(); }
const IDocumentLayoutAccess& SwViewShell::getIDocumentLayoutAccess() const { return mpDoc->getIDocumentLayoutAccess(); }
IDocumentLayoutAccess& SwViewShell::getIDocumentLayoutAccess() { return mpDoc->getIDocumentLayoutAccess(); }
IDocumentContentOperations& SwViewShell::getIDocumentContentOperations() { return mpDoc->getIDocumentContentOperations(); }
IDocumentStylePoolAccess& SwViewShell::getIDocumentStylePoolAccess() { return mpDoc->getIDocumentStylePoolAccess(); }
const IDocumentStatistics& SwViewShell::getIDocumentStatistics() const { return mpDoc->getIDocumentStatistics(); }

IDocumentUndoRedo      & SwViewShell::GetIDocumentUndoRedo()
{ return mpDoc->GetIDocumentUndoRedo(); }
IDocumentUndoRedo const& SwViewShell::GetIDocumentUndoRedo() const
{ return mpDoc->GetIDocumentUndoRedo(); }

// --> OD 2007-11-14 #i83479#
const IDocumentListItems* SwViewShell::getIDocumentListItemsAccess() const
{
    return &mpDoc->getIDocumentListItems();
}

const IDocumentOutlineNodes* SwViewShell::getIDocumentOutlineNodesAccess() const
{
    return &mpDoc->getIDocumentOutlineNodes();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
