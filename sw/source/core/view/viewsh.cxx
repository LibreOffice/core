/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <touch/touch.h>

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

#include "../../ui/inc/view.hxx"
#include <PostItMgr.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdrpaintwindow.hxx>

sal_Bool SwViewShell::mbLstAct = sal_False;
ShellResource *SwViewShell::mpShellRes = 0;
Window *SwViewShell::mpCareWindow = 0;
BitmapEx* SwViewShell::mpErrorBmp = NULL;
BitmapEx* SwViewShell::mpReplaceBmp = NULL;

bool bInSizeNotify = false;

DBG_NAME(LayoutIdle)

TYPEINIT0(SwViewShell);

using namespace ::com::sun::star;

void SwViewShell::ToggleHeaderFooterEdit()
{
    mbHeaderFooterEdit = !mbHeaderFooterEdit;
    if ( !mbHeaderFooterEdit )
    {
        SetShowHeaderFooterSeparator( Header, false );
        SetShowHeaderFooterSeparator( Footer, false );
    }

    
    if ( !IsShowHeaderFooterSeparator( Header ) &&
         !IsShowHeaderFooterSeparator( Footer ) )
    {
        mbHeaderFooterEdit = false;
    }

    
    GetWin()->Invalidate();
}

static void
lcl_PaintTransparentFormControls(SwViewShell & rShell, SwRect const& rRect)
{
    
    
    if (rShell.GetWin())
    {
        Window& rWindow = *(rShell.GetWin());
        const Rectangle aRectanglePixel(rWindow.LogicToPixel(rRect.SVRect()));
        PaintTransparentChildren(rWindow, aRectanglePixel);
    }
}









void SwViewShell::PrePaint()
{
    
    if(HasDrawView())
    {
        Imp()->GetDrawView()->PrePaint();
    }
}

void SwViewShell::DLPrePaint2(const Region& rRegion)
{
    if(mPrePostPaintRegions.empty())
    {
        mPrePostPaintRegions.push( rRegion );
        
        if ( !HasDrawView() )
            MakeDrawView();

        
        mpPrePostOutDev = (GetWin() && !mbTiledRendering)? GetWin(): GetOut();

        
        mpTargetPaintWindow = Imp()->GetDrawView()->BeginDrawLayers(mpPrePostOutDev, rRegion);
        OSL_ENSURE(mpTargetPaintWindow, "BeginDrawLayers: Got no SdrPaintWindow (!)");

        
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            mpBufferedOut = mpOut;
            mpOut = &(mpTargetPaintWindow->GetTargetOutputDevice());
        }

        
        maPrePostMapMode = mpOut->GetMapMode();
    }
    else
    {
        
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
        Region current = mPrePostPaintRegions.top();
        mPrePostPaintRegions.pop();
        if( current != mPrePostPaintRegions.top())
            Imp()->GetDrawView()->UpdateDrawLayersRegion(mpPrePostOutDev, mPrePostPaintRegions.top());
        return;
    }
    mPrePostPaintRegions.pop(); 
    if(0 != mpTargetPaintWindow)
    {
        
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            mpOut = mpBufferedOut;
        }

        
        Imp()->GetDrawView()->EndDrawLayers(*mpTargetPaintWindow, bPaintFormLayer);
        mpTargetPaintWindow = 0;
    }
}


void SwViewShell::ImplEndAction( const sal_Bool bIdleEnd )
{
    
    if ( !GetWin() || IsPreview() )
    {
        mbPaintWorks = sal_True;
        UISizeNotify();
        return;
    }

    mbInEndAction = sal_True;
    

    SwViewShell::mbLstAct = sal_True;
    SwViewShell *pSh = (SwViewShell*)this->GetNext();
    while ( pSh != this )
    {
        if ( pSh->ActionPend() )
        {
            SwViewShell::mbLstAct = sal_False;
            pSh = this;
        }
        else
            pSh = (SwViewShell*)pSh->GetNext();
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
        aAction.SetComplete( sal_False );
        if ( mnLockPaint )
            aAction.SetPaint( sal_False );
        aAction.SetInputType( VCL_INPUT_KEYBOARD );
        aAction.Action();
    }

    if ( bIsShellForCheckViewLayout )
        GetLayout()->CheckViewLayout( GetViewOptions(), &maVisArea );

    
    
    if ( Imp()->GetRegion()     ||
         maInvalidRect.HasArea() ||
         bExtraData )
    {
        if ( !mnLockPaint )
        {
            SolarMutexGuard aGuard;

            sal_Bool bPaintsFromSystem = maInvalidRect.HasArea();
            GetWin()->Update();
            if ( maInvalidRect.HasArea() )
            {
                if ( bPaintsFromSystem )
                    Imp()->AddPaintRect( maInvalidRect );

                ResetInvalidRect();
                bPaintsFromSystem = sal_True;
            }
            mbPaintWorks = sal_True;

            SwRegionRects *pRegion = Imp()->GetRegion();

            
            
            
            
            
            bool bShowCrsr = pRegion && IsA( TYPE(SwCrsrShell) );
            if( bShowCrsr )
                ((SwCrsrShell*)this)->HideCrsrs();

            if ( pRegion )
            {
                SwRootFrm* pCurrentLayout = GetLayout();

                Imp()->pRegion = NULL;

                
                pRegion->Invert();

                pRegion->Compress();

                VirtualDevice *pVout = 0;
                while ( !pRegion->empty() )
                {
                    SwRect aRect( pRegion->back() );
                    pRegion->pop_back();

                    bool bPaint = true;
                    if ( IsEndActionByVirDev() )
                    {
                        
                        if ( !pVout )
                            pVout = new VirtualDevice( *GetOut() );
                        MapMode aMapMode( GetOut()->GetMapMode() );
                        pVout->SetMapMode( aMapMode );

                        sal_Bool bSizeOK = sal_True;

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

                            
                            
                            const Region aRepaintRegion(aRect.SVRect());
                            DLPrePaint2(aRepaintRegion);
                            

                            OutputDevice  *pOld = GetOut();
                            pVout->SetLineColor( pOld->GetLineColor() );
                            pVout->SetFillColor( pOld->GetFillColor() );
                            Point aOrigin( aRect.Pos() );
                            aOrigin.X() = -aOrigin.X(); aOrigin.Y() = -aOrigin.Y();
                            aMapMode.SetOrigin( aOrigin );
                            pVout->SetMapMode( aMapMode );

                            mpOut = pVout;
                            if ( bPaintsFromSystem )
                                PaintDesktop( aRect );
                            pCurrentLayout->Paint( aRect );
                            pOld->DrawOutDev( aRect.Pos(), aRect.SSize(),
                                              aRect.Pos(), aRect.SSize(), *pVout );
                            mpOut = pOld;

                            
                            DLPostPaint2(true);
                        }
                    }
                    if ( bPaint )
                    {
                        
                        
                        
                        
                        
                        
                        DLPrePaint2(Region(aRect.SVRect()));

                        if ( bPaintsFromSystem )
                            PaintDesktop( aRect );
                        pCurrentLayout->Paint( aRect );

                        
                        DLPostPaint2(true);
                    }

                    lcl_PaintTransparentFormControls(*this, aRect); 
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
            mbPaintWorks =  sal_True;
        }
    }
    else
        mbPaintWorks = sal_True;

    mbInEndAction = sal_False;
    SwViewShell::mbLstAct = sal_False;
    Imp()->EndAction();

    
    
    
    
    --mnStartAction;
    UISizeNotify();
    ++mnStartAction;

    if( Imp()->IsAccessible() )
        Imp()->FireAccessibleEvents();
}

void SwViewShell::ImplStartAction()
{
    mbPaintWorks = sal_False;
    Imp()->StartAction();
}

void SwViewShell::ImplLockPaint()
{
    if ( GetWin() && GetWin()->IsVisible() )
        GetWin()->EnablePaint( false ); 
    Imp()->LockPaint();
}

void SwViewShell::ImplUnlockPaint( sal_Bool bVirDev )
{
    SET_CURR_SHELL( this );
    if ( GetWin() && GetWin()->IsVisible() )
    {
        if ( (bInSizeNotify || bVirDev ) && VisArea().HasArea() )
        {
            
            VirtualDevice *pVout = new VirtualDevice( *mpOut );
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

                
                const Region aRepaintRegion(VisArea().SVRect());
                DLPrePaint2(aRepaintRegion);

                OutputDevice *pOld = mpOut;
                mpOut = pVout;
                Paint( VisArea().SVRect() );
                mpOut = pOld;
                mpOut->DrawOutDev( VisArea().Pos(), aSize,
                                  VisArea().Pos(), aSize, *pVout );

                
                DLPostPaint2(true);

                lcl_PaintTransparentFormControls(*this, VisArea()); 
            }
            else
            {
                Imp()->UnlockPaint();
                GetWin()->EnablePaint( true );
                GetWin()->Invalidate( INVALIDATE_CHILDREN );
            }
            delete pVout;
        }
        else
        {
            Imp()->UnlockPaint();
            GetWin()->EnablePaint( true );
            GetWin()->Invalidate( INVALIDATE_CHILDREN );
        }
    }
    else
        Imp()->UnlockPaint();
}

sal_Bool SwViewShell::AddPaintRect( const SwRect & rRect )
{
    sal_Bool bRet = sal_False;
    SwViewShell *pSh = this;
    do
    {
        if( pSh->Imp() )
        {
        if ( pSh->IsPreview() && pSh->GetWin() )
            ::RepaintPagePreview( pSh, rRect );
        else
                bRet |= pSh->Imp()->AddPaintRect( rRect );
        }
        pSh = (SwViewShell*)pSh->GetNext();
    } while ( pSh != this );
    return bRet;
}

void SwViewShell::InvalidateWindows( const SwRect &rRect )
{
    if ( !Imp()->IsCalcLayoutProgress() )
    {
        SwViewShell *pSh = this;
        do
        {
            if ( pSh->GetWin() )
            {
                if ( pSh->IsPreview() )
                    ::RepaintPagePreview( pSh, rRect );
                else if ( pSh->VisArea().IsOver( rRect ) )
                    pSh->GetWin()->Invalidate( rRect.SVRect() );
            }
            pSh = (SwViewShell*)pSh->GetNext();

        } while ( pSh != this );
    }
}

void SwViewShell::MakeVisible( const SwRect &rRect )
{
    if ( !VisArea().IsInside( rRect ) || IsScrollMDI( this, rRect ) || GetCareWin(*this) )
    {
        if ( !IsViewLocked() )
        {
            if( mpWin )
            {
                const SwFrm* pRoot = GetLayout();
                int nLoopCnt = 3;
                long nOldH;
                do{
                    nOldH = pRoot->Frm().Height();
                    StartAction();
                    ScrollMDI( this, rRect, USHRT_MAX, USHRT_MAX );
                    EndAction();
                } while( nOldH != pRoot->Frm().Height() && nLoopCnt-- );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                
                OSL_ENSURE( !this, "Is MakeVisible still needed for printers?" );
            }

#endif
        }
    }
}

Window* SwViewShell::CareChildWin(SwViewShell& rVSh)
{
    if(rVSh.mpSfxViewShell)
    {
        const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = rVSh.mpSfxViewShell->GetViewFrame();
        const SfxChildWindow* pChWin = pVFrame->GetChildWindow( nId );
        Window *pWin = pChWin ? pChWin->GetWindow() : NULL;
        if ( pWin && pWin->IsVisible() )
            return pWin;
    }
    return NULL;
}

Point SwViewShell::GetPagePos( sal_uInt16 nPageNum ) const
{
    return GetLayout()->GetPagePos( nPageNum );
}

sal_uInt16 SwViewShell::GetNumPages()
{
    
    
    return GetLayout() ? GetLayout()->GetPageNum() : 0;
}

sal_Bool SwViewShell::IsDummyPage( sal_uInt16 nPageNum ) const
{
    return GetLayout() ? GetLayout()->IsDummyPage( nPageNum ) : 0;
}

/**
 * Forces update of each field.
 * It notifies all fields with pNewHt. If that is 0 (default), the field
 * type is sent (???).
 * @param[in] bCloseDB Passed in to GetDoc()->UpdateFlds. [TODO] Purpose???
 */
void SwViewShell::UpdateFlds(sal_Bool bCloseDB)
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
void SwViewShell::UpdateAllCharts()
{
    SET_CURR_SHELL( this );
    
    GetDoc()->UpdateAllCharts();
}

sal_Bool SwViewShell::HasCharts() const
{
    sal_Bool bRet = sal_False;
    SwNodeIndex aIdx( *GetDoc()->GetNodes().GetEndOfAutotext().
                        StartOfSectionNode(), 1 );
    while (aIdx.GetNode().GetStartNode())
    {
        ++aIdx;
        const SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && !pNd->GetChartTblName().isEmpty() )
        {
            bRet = sal_True;
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

    
    SwViewShell *pSh = this;
    do
    {   if ( !pSh->GetWin() )
            return;
        pSh = (SwViewShell*)pSh->GetNext();

    } while ( pSh != this );

    SET_CURR_SHELL( this );

#ifdef DBG_UTIL
    
    if( mpOpt->IsTest5() )
        return;
#endif

    {
        DBG_PROFSTART( LayoutIdle );

        
        SwSaveSetLRUOfst aSave( *SwTxtFrm::GetTxtCache(),
                             SwTxtFrm::GetTxtCache()->GetCurMax() - 50 );
        
        
        
        OSL_ENSURE(Imp(), "SwViewShell already deleted?");
        if(!Imp())
            return;
        SwLayIdle aIdle( GetLayout(), Imp() );
        DBG_PROFSTOP( LayoutIdle );
    }
}

static void lcl_InvalidateAllCntnt( SwViewShell& rSh, sal_uInt8 nInv )
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
static void lcl_InvalidateAllObjPos( SwViewShell &_rSh )
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

void SwViewShell::SetParaSpaceMax( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::PARA_SPACE_MAX, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

void SwViewShell::SetParaSpaceMaxAtPages( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

void SwViewShell::SetTabCompat( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::TAB_COMPAT) != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::TAB_COMPAT, bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void SwViewShell::SetAddExtLeading( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::ADD_EXT_LEADING) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::ADD_EXT_LEADING, bNew );
        SdrModel* pTmpDrawModel = getIDocumentDrawModelAccess()->GetDrawModel();
        if ( pTmpDrawModel )
            pTmpDrawModel->SetAddExtLeading( bNew );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void SwViewShell::SetUseVirDev( bool bNewVirtual )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) != bNewVirtual )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        
        IDocumentDeviceAccess* pIDDA = getIDocumentDeviceAccess();
        pIDDA->setReferenceDeviceType( bNewVirtual, true );
    }
}

/** Sets if paragraph and table spacing is added at bottom of table cells.
 * #106629#
 * @param[in] (bool) setting of the new value
 */
void SwViewShell::SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) != _bAddParaSpacingToTableCells )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
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
void SwViewShell::SetUseFormerLineSpacing( bool _bUseFormerLineSpacing )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING) != _bUseFormerLineSpacing )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
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
void SwViewShell::SetUseFormerObjectPositioning( bool _bUseFormerObjPos )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) != _bUseFormerObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, _bUseFormerObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}


void SwViewShell::SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) != _bConsiderWrapOnObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, _bConsiderWrapOnObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}

void SwViewShell::SetUseFormerTextWrapping( bool _bUseFormerTextWrapping )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) != _bUseFormerTextWrapping )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, _bUseFormerTextWrapping );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}


void SwViewShell::SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK) != _bDoNotJustifyLinesWithManualBreak )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        pIDSA->set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, _bDoNotJustifyLinesWithManualBreak );
        const sal_uInt8 nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void SwViewShell::Reformat()
{
    SwWait aWait( *GetDoc()->GetDocShell(), true );

    
    
    
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

void SwViewShell::ChgNumberDigits()
{
    SdrModel* pTmpDrawModel = getIDocumentDrawModelAccess()->GetDrawModel();
    if ( pTmpDrawModel )
           pTmpDrawModel->ReformatAllTextObjects();
    Reformat();
}

void SwViewShell::CalcLayout()
{
    SET_CURR_SHELL( this );
    SwWait aWait( *GetDoc()->GetDocShell(), true );

    
    SwSaveSetLRUOfst aSaveLRU( *SwTxtFrm::GetTxtCache(),
                                  SwTxtFrm::GetTxtCache()->GetCurMax() - 50 );

    
    const bool bEndProgress = SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) == 0;
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

void SwViewShell::SetFirstVisPageInvalid()
{
    SwViewShell *pSh = this;
    do
    {
        if ( pSh->Imp() )
            pSh->Imp()->SetFirstVisPageInvalid();
        pSh = (SwViewShell*)pSh->GetNext();

    } while ( pSh != this );
}

void SwViewShell::SizeChgNotify()
{
    if ( !mpWin )
        mbDocSizeChgd = sal_True;
    else if( ActionPend() || Imp()->IsCalcLayoutProgress() || mbPaintInProgress )
    {
        mbDocSizeChgd = sal_True;

        if ( !Imp()->IsCalcLayoutProgress() && ISA( SwCrsrShell ) )
        {
            const SwFrm *pCnt = ((SwCrsrShell*)this)->GetCurrFrm( sal_False );
            const SwPageFrm *pPage;
            if ( pCnt && 0 != (pPage = pCnt->FindPageFrm()) )
            {
                sal_uInt16 nVirtNum = pPage->GetVirtPageNum();
                 const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
                OUString sDisplay = rNum.GetNumStr( nVirtNum );
                PageNumNotify( this, pCnt->GetPhyPageNum(), nVirtNum, sDisplay );
            }
        }
    }
    else
    {
        mbDocSizeChgd = sal_False;
        ::SizeNotify( this, GetDocSize() );
    }
}

void SwViewShell::VisPortChgd( const SwRect &rRect)
{
    OSL_ENSURE( GetWin(), "VisPortChgd ohne Window." );

    if ( rRect == VisArea() )
        return;

#if OSL_DEBUG_LEVEL > 1
    if ( mbInEndAction )
    {
        
        OSL_ENSURE( !this, "Scrolling during EndAction." );
    }
#endif

    
    
    const SwFrm *pOldPage = Imp()->GetFirstVisPage();

    const SwRect aPrevArea( VisArea() );
    const sal_Bool bFull = aPrevArea.IsEmpty();
    maVisArea = rRect;
    SetFirstVisPageInvalid();

    
    
    
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
        
        const long nXDiff = aPrevArea.Left() - VisArea().Left();
        const long nYDiff = aPrevArea.Top()  - VisArea().Top();

        if( !nXDiff && !GetViewOptions()->getBrowseMode() &&
            (!Imp()->HasDrawView() || !Imp()->GetDrawView()->IsGridVisible() ) )
        {
            
            
            
            const SwPageFrm *pPage = (SwPageFrm*)GetLayout()->Lower();
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
                    
                    
                    
                    if ( pPage->GetSortedObjs() )
                    {
                        const long nOfst = GetOut()->PixelToLogic(
                            Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width();
                        for ( sal_uInt16 i = 0;
                              i < pPage->GetSortedObjs()->Count(); ++i )
                        {
                            SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                            
                            if (pObj->IsFormatPossible())
                            {
                                const Rectangle &rBound = pObj->GetObjRect().SVRect();
                                
                                const SwTwips nL = std::max( 0L, rBound.Left() - nOfst );
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
        else if ( !mnLockPaint ) 
        {
            if( VisArea().IsOver( aPrevArea ) )
            {
                bScrolled = true;
                maVisArea.Pos() = aPrevArea.Pos();
                if ( SmoothScroll( nXDiff, nYDiff, 0 ) )
                    return;
                maVisArea.Pos() = rRect.Pos();
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

    if ( pPostItMgr ) 
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

sal_Bool SwViewShell::SmoothScroll( long lXDiff, long lYDiff, const Rectangle *pRect )
{
#if !defined(MACOSX) && !defined(ANDROID) && !defined(IOS)
    

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

    
    const bool bOnlyYScroll(!lXDiff && std::abs(lYDiff) != 0 && std::abs(lYDiff) < lMax);
    const bool bAllowedWithChildWindows(GetWin()->GetWindowClipRegionPixel(WINDOW_GETCLIPREGION_NOCHILDREN|WINDOW_GETCLIPREGION_NULL).IsNull());
    const bool bSmoothScrollAllowed(bOnlyYScroll && mbEnableSmooth && GetViewOptions()->IsSmoothScroll() &&  bAllowedWithChildWindows);

    if(bSmoothScrollAllowed)
    {
        Imp()->bStopSmooth = sal_False;

        const SwRect aOldVis( VisArea() );

        
        const Size aPixSz = GetWin()->PixelToLogic(Size(1,1));
        VirtualDevice *pVout = new VirtualDevice( *GetWin() );
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
            mpOut = pVout;

            {
                
                
                
                
                
                
                
                if(!HasDrawView())
                    MakeDrawView();
                SdrView* pDrawView = GetDrawView();
                pDrawView->AddWindowToPaintView(pVout);

                
                
                
                Window* pOldWin = mpWin;
                mpWin = 0;
                DLPrePaint2(Region(aRect.SVRect()));
                mpWin = pOldWin;

                
                PaintDesktop( aRect );
                SwViewShell::mbLstAct = sal_True;
                GetLayout()->Paint( aRect );
                SwViewShell::mbLstAct = sal_False;

                
                DLPostPaint2(true);
                pDrawView->DeleteWindowFromPaintView(pVout);
            }

            mpOut = pOld;
            maVisArea = aOldVis;

            

            
            
            
            
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
                if ( Imp()->bStopSmooth || std::abs(lDiff) <= std::abs(lMaDelta) )
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
                            
                            const Rectangle aTargetLogic(Imp()->aSmoothRect.SVRect());
                            DLPrePaint2(Region(aTargetLogic));

                            
                            OutputDevice& rTargetDevice = mpTargetPaintWindow->GetTargetOutputDevice();
                            const Rectangle aTargetPixel(rTargetDevice.LogicToPixel(aTargetLogic));

                            
                            const Point aSourceTopLeft(pVout->LogicToPixel(aTargetLogic.TopLeft()));

                            
                            const bool bMapModeWasEnabledDest(rTargetDevice.IsMapModeEnabled());
                            const bool bMapModeWasEnabledSource(pVout->IsMapModeEnabled());
                            rTargetDevice.EnableMapMode(false);
                            pVout->EnableMapMode(false);

                            rTargetDevice.DrawOutDev(
                                aTargetPixel.TopLeft(), aTargetPixel.GetSize(), 
                                aSourceTopLeft, aTargetPixel.GetSize(), 
                                *pVout);

                            
                            rTargetDevice.EnableMapMode(bMapModeWasEnabledDest);
                            pVout->EnableMapMode(bMapModeWasEnabledSource);

                            
                            DLPostPaint2(true);
                    }
                    else
                        --mnLockPaint;
                }
            }
            delete pVout;
            GetWin()->Update();
            if ( !Imp()->bStopSmooth )
                --mnLockPaint;
            SetFirstVisPageInvalid();
            return sal_True;
        }
        delete pVout;
    }
#endif

    maVisArea.Pos().X() -= lXDiff;
    maVisArea.Pos().Y() -= lYDiff;
    if ( pRect )
        GetWin()->Scroll( lXDiff, lYDiff, *pRect, SCROLL_CHILDREN);
    else
        GetWin()->Scroll( lXDiff, lYDiff, SCROLL_CHILDREN);
    return sal_False;
}

void SwViewShell::PaintDesktop( const SwRect &rRect )
{
    if ( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return;                     

    
    
    
    
    bool bBorderOnly = false;
    const SwRootFrm *pRoot = GetLayout();
    if ( rRect.Top() > pRoot->Frm().Bottom() )
    {
        const SwFrm *pPg = pRoot->Lower();
        while ( pPg && pPg->GetNext() )
            pPg = pPg->GetNext();
        if ( !pPg || !pPg->Frm().IsOver( VisArea() ) )
            bBorderOnly = true;
    }

    const bool bBookMode = GetViewOptions()->IsViewLayoutBookMode();

    SwRegionRects aRegion( rRect );

    
    const SwPostItMgr* pPostItMgr = GetPostItMgr();
    const SwTwips nSidebarWidth = pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() ?
                                  pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth() :
                                  0;

    if ( bBorderOnly )
    {
        const SwFrm *pPage =pRoot->Lower();
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
        while ( pPage && !aRegion.empty() &&
                (pPage->Frm().Top() <= nBottom) )
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


void SwViewShell::_PaintDesktop( const SwRegionRects &rRegion )
{
    
    GetOut()->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
    GetOut()->SetLineColor();

    for ( sal_uInt16 i = 0; i < rRegion.size(); ++i )
    {
        const Rectangle aRectangle(rRegion[i].SVRect());

        
        
        
        
        
        
        
        //
        
        //
        
        
        //
        
        
        
        
        //
        
        
        //
        
        
        
        

        
        DLPrePaint2(Region(aRectangle));

        
        
        GetOut()->SetFillColor( SwViewOption::GetAppBackgroundColor());
        GetOut()->SetLineColor();
        GetOut()->DrawRect(aRectangle);

        DLPostPaint2(false);
    }

    GetOut()->Pop();
}

sal_Bool SwViewShell::CheckInvalidForPaint( const SwRect &rRect )
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
        
        
        
        if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea())
             Imp()->DelRegion();

        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( sal_False );
        
        
        
        
        
        ++mnStartAction;
        aAction.Action();
        --mnStartAction;

        SwRegionRects *pRegion = Imp()->GetRegion();
        if ( pRegion && aAction.IsBrowseActionStop() )
        {
            
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
                        
                        
                        
                        if ( maInvalidRect.IsEmpty() )
                            maInvalidRect = rRect;
                        else
                            maInvalidRect.Union( rRect );
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

void SwViewShell::Paint(const Rectangle &rRect)
{
    if ( mnLockPaint )
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
        
        SwPaintQueue::Add( this, SwRect( rRect ) );
        return;
    }

    
    
    if ( mbPaintWorks && !mnStartAction )
    {
        if( GetWin() && GetWin()->IsVisible() )
        {
            SwRect aRect( rRect );
            if ( mbPaintInProgress ) 
            {
                GetWin()->Invalidate( rRect );
                return;
            }

            mbPaintInProgress = sal_True;
            SET_CURR_SHELL( this );
            SwRootFrm::SetNoVirDev( sal_True );

            
            
            
            
            
            if( !GetOut()->GetConnectMetaFile() && GetOut()->IsClipRegion())
                GetOut()->SetClipRegion();

            if ( IsPreview() )
            {
                
                if ( aRect.IsInside( maInvalidRect ) )
                    ResetInvalidRect();
                SwViewShell::mbLstAct = sal_True;
                GetLayout()->Paint( aRect );
                SwViewShell::mbLstAct = sal_False;
            }
            else
            {
                
                
                if ( !CheckInvalidForPaint( aRect ) )
                {
                    
                    
                    const Region aRepaintRegion(aRect.SVRect());
                    DLPrePaint2(aRepaintRegion);

                    
                    PaintDesktop( aRect );

                    
                    if ( aRect.IsInside( maInvalidRect ) )
                        ResetInvalidRect();
                    SwViewShell::mbLstAct = sal_True;
                    GetLayout()->Paint( aRect );
                    SwViewShell::mbLstAct = sal_False;
                    
                    
                    DLPostPaint2(true);
                    
                }
            }
            SwRootFrm::SetNoVirDev( sal_False );
            mbPaintInProgress = sal_False;
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
            const Region aRegion(GetWin()->GetPaintRegion());
            RectangleVector aRectangles;
            aRegion.GetRegionRectangles(aRectangles);

            for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
            {
                Imp()->AddPaintRect(*aRectIter);
            }

            
            
            
            
            
        }
        else if ( SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) &&
                  GetOut() == GetWin() )
        {
            
            const Region aDLRegion(rRect);
            DLPrePaint2(aDLRegion);

            mpOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            mpOut->SetFillColor( Imp()->GetRetoucheColor() );
            mpOut->SetLineColor();
            mpOut->DrawRect( rRect );
            mpOut->Pop();
            
            DLPostPaint2(true);
        }
    }
}

void SwViewShell::PaintTile(VirtualDevice &rDevice, int contextWidth, int contextHeight, int tilePosX, int tilePosY, long tileWidth, long tileHeight)
{
    
    
    
    
    OutputDevice *pSaveOut = mpOut;
    mbTiledRendering = true;
    mpOut = &rDevice;

    
    rDevice.SetOutputSizePixel(Size(contextWidth, contextHeight));

    
    MapMode aMapMode(rDevice.GetMapMode());
    aMapMode.SetMapUnit(MAP_TWIP);
    aMapMode.SetOrigin(Point(-tilePosX, -tilePosY));

    
    
    Fraction scaleX = Fraction(contextWidth, 96) * Fraction(1440L) / Fraction(tileWidth);
    Fraction scaleY = Fraction(contextHeight, 96) * Fraction(1440L) / Fraction(tileHeight);
    aMapMode.SetScaleX(scaleX);
    aMapMode.SetScaleY(scaleY);
    rDevice.SetMapMode(aMapMode);

    
    MakeVisible(SwRect(Point(tilePosX, tilePosY), rDevice.PixelToLogic(Size(contextWidth, contextHeight))));

    
    Paint(Rectangle(Point(tilePosX, tilePosY), rDevice.PixelToLogic(Size(contextWidth, contextHeight))));

    
    mpOut = pSaveOut;
    mbTiledRendering = false;
}

#if !HAVE_FEATURE_DESKTOP
extern "C"
void touch_lo_draw_tile(void *context, int contextWidth, int contextHeight, MLODpxPoint tileDpxPosition, MLODpxSize tileDpxSize)
{
#ifdef IOS
    SAL_INFO("sw", "touch_lo_draw_tile(" << contextWidth << ", " << contextHeight << ", (" << tileDpxPosition.x << "," << tileDpxPosition.y << "), " << tileDpxSize.width << "x" << tileDpxSize.height << ")");
    MLORipPoint tileRipPosition = MLORipPointByDpxPoint(tileDpxPosition);
    MLORipSize rileRipSize = MLORipSizeByDpxSize(tileDpxSize);
    MLORip tileRipPosX = tileRipPosition.x;
    MLORip tileRipPosY = tileRipPosition.y;
    MLORip tileRipWidth = rileRipSize.width;
    MLORip tileRipHeight = rileRipSize.height;
    
    
    int tilePosX = tileRipPosX;
    int tilePosY = tileRipPosY;
    long tileWidth  = tileRipWidth;
    long tileHeight = tileRipHeight;
    
    
    
    
    

    SwWrtShell *pViewShell;

    
    while (!(pViewShell = GetActiveWrtShell()))
    {
        sleep(1);
    }

    
    
    Application::AcquireSolarMutex(1);
    if (pViewShell)
    {
        SystemGraphicsData aData;
        aData.rCGContext = (CGContextRef) context;
        VirtualDevice aDevice(&aData, (sal_uInt16)0);
        
        pViewShell->PaintTile(aDevice, contextWidth, contextHeight, tilePosX, tilePosY, tileWidth, tileHeight);
    }
    Application::ReleaseSolarMutex();
#else
    (void) context;
    (void) contextWidth;
    (void) contextHeight;
    (void) tileDpxPosition;
    (void) tileDpxSize;
#endif
}
extern "C"
MLODpxSize touch_lo_get_content_size()
{
#ifdef IOS
    SwWrtShell *pViewShell;
    
    while (!(pViewShell = GetActiveWrtShell()))
    {
        sleep(1);
    }

    if (pViewShell)
    {
        static const MLORip WIDTH_ADDITION  = 6L * DOCUMENTBORDER;
        static const MLORip HEIGHT_ADDITION = 2L * DOCUMENTBORDER;
        Size documentSize =pViewShell->GetView().GetDocSz();
        return MLODpxSizeByRips(((MLORip)documentSize.Width()) + WIDTH_ADDITION,
                               ((MLORip)documentSize.Height()) + HEIGHT_ADDITION);
    }
    return MLODpxSizeByDpxes(0,0);
#else
    return MLODpxSize();
#endif
}

extern "C"
MLORipPoint MLORipPointByDpxPoint(MLODpxPoint mloDpxPoint)
{
#ifdef IOS
    
    MLORip x = MLORipByDpx(mloDpxPoint.x /*- (contentSize.width/2.0f)*/);
    MLORip y = MLORipByDpx(mloDpxPoint.y);
    return MLORipPointByRips(x,y);
#else
    (void) mloDpxPoint;
    return MLORipPoint();
#endif
}

extern "C"
MLODpxPoint MLODpxPointByRipPoint(MLORipPoint mloRipPoint)
{
    
    MLODpx x = MLODpxByRip(mloRipPoint.x)/* + (contentSize.width/2.0f)*/;
    MLODpx y = MLODpxByRip(mloRipPoint.y);
    return MLODpxPointByDpxes(x,y);
}
#endif

void SwViewShell::SetBrowseBorder( const Size& rNew )
{
    if( rNew != maBrowseBorder )
    {
        maBrowseBorder = rNew;
        if ( maVisArea.HasArea() )
            CheckBrowseView( sal_False );
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

void SwViewShell::CheckBrowseView( sal_Bool bBrowseChgd )
{
    if ( !bBrowseChgd && !GetViewOptions()->getBrowseMode() )
        return;

    SET_CURR_SHELL( this );

    OSL_ENSURE( GetLayout(), "Layout not ready" );

    
    
    
    
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

    
    
    sal_uInt8 nInv = INV_PRTAREA | INV_TABLE | INV_POS;
    
    
    if( bBrowseChgd )
        nInv |= INV_SIZE | INV_DIRECTION;

    GetLayout()->InvalidateAllCntnt( nInv );

    SwFrm::CheckPageDescs( (SwPageFrm*)GetLayout()->Lower() );

    EndAction();
    UnlockPaint();
}

SwRootFrm *SwViewShell::GetLayout() const
{
    return mpLayout.get();
}

OutputDevice& SwViewShell::GetRefDev() const
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
    const SwRootFrm* pRoot = GetLayout();
    if( pRoot )
        aSz = pRoot->Frm().SSize();

    return aSz;
}

SfxItemPool& SwViewShell::GetAttrPool()
{
    return GetDoc()->GetAttrPool();
}

void SwViewShell::ApplyViewOptions( const SwViewOption &rOpt )
{

    SwViewShell *pSh = this;
    do
    {   pSh->StartAction();
        pSh = (SwViewShell*)pSh->GetNext();
    } while ( pSh != this );

    ImplApplyViewOptions( rOpt );

    
    
    
    pSh = (SwViewShell*)this->GetNext();
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
        pSh = (SwViewShell*)pSh->GetNext();
    }
    

    pSh = this;
    do
    {   pSh->EndAction();
        pSh = (SwViewShell*)pSh->GetNext();
    } while ( pSh != this );

}

void SwViewShell::ImplApplyViewOptions( const SwViewOption &rOpt )
{
    if (*mpOpt == rOpt)
        return;

    Window *pMyWin = GetWin();
    if( !pMyWin )
    {
        OSL_ENSURE( pMyWin, "SwViewShell::ApplyViewOptions: no window" );
        return;
    }

    SET_CURR_SHELL( this );

    sal_Bool bReformat   = sal_False;

    if( mpOpt->IsShowHiddenField() != rOpt.IsShowHiddenField() )
    {
        ((SwHiddenTxtFieldType*)mpDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
                                            SetHiddenFlag( !rOpt.IsShowHiddenField() );
        bReformat = sal_True;
    }
    if ( mpOpt->IsShowHiddenPara() != rOpt.IsShowHiddenPara() )
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
    if ( !bReformat && mpOpt->IsShowHiddenChar() != rOpt.IsShowHiddenChar() )
    {
        bReformat = GetDoc()->ContainsHiddenChars();
    }

    
    
    
    
    bReformat = bReformat || mpOpt->IsFldName() != rOpt.IsFldName();

    
    if( mpOpt->GetZoom() != rOpt.GetZoom() && !IsPreview() )
    {
        MapMode aMode( pMyWin->GetMapMode() );
        Fraction aNewFactor( rOpt.GetZoom(), 100 );
        aMode.SetScaleX( aNewFactor );
        aMode.SetScaleY( aNewFactor );
        pMyWin->SetMapMode( aMode );
        
        
        if( mpOpt->getBrowseMode() )
            bReformat = sal_True;
    }

    bool bBrowseModeChanged = false;
    if( mpOpt->getBrowseMode() != rOpt.getBrowseMode() )
    {
        bBrowseModeChanged = true;
        bReformat = sal_True;
    }
    else if( mpOpt->getBrowseMode() && mpOpt->IsPrtFormat() != rOpt.IsPrtFormat() )
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

            
            pDView->SetMarkHdlSizePixel(9);
    }

    bool bOnlineSpellChgd = mpOpt->IsOnlineSpell() != rOpt.IsOnlineSpell();

    *mpOpt = rOpt;   
    mpOpt->SetUIOptions(rOpt);

    mpDoc->set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode(mpDoc->GetDocShell()));

    if( bBrowseModeChanged )
    {
        
        
        mpDoc->CheckDefaultPageFmt();
        CheckBrowseView( sal_True );
    }

    pMyWin->Invalidate();
    if ( bReformat )
    {
        
        
        StartAction();
        Reformat();
        EndAction();
    }

    if( bOnlineSpellChgd )
    {
        SwViewShell *pSh = (SwViewShell*)this->GetNext();
        sal_Bool bOnlineSpl = rOpt.IsOnlineSpell();
        while( pSh != this )
        {   pSh->mpOpt->SetOnlineSpell( bOnlineSpl );
            Window *pTmpWin = pSh->GetWin();
            if( pTmpWin )
                pTmpWin->Invalidate();
            pSh = (SwViewShell*)pSh->GetNext();
        }
    }

}

void SwViewShell::SetUIOptions( const SwViewOption &rOpt )
{
    mpOpt->SetUIOptions(rOpt);
    
    
    if(rOpt.IsStarOneSetting())
        mpOpt->SetStarOneSetting(sal_True);

    mpOpt->SetSymbolFont(rOpt.GetSymbolFont());
}

void SwViewShell::SetReadonlyOption(sal_Bool bSet)
{
    
    

    
    if( bSet != mpOpt->IsReadonly() )
    {
        
        mpOpt->SetReadonly( sal_False );

        sal_Bool bReformat = mpOpt->IsFldName();

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
            Imp()->InvalidateAccessibleEditableState( sal_False );
    }
}

void  SwViewShell::SetPDFExportOption(sal_Bool bSet)
{
    if( bSet != mpOpt->IsPDFExport() )
    {
        if( bSet && mpOpt->getBrowseMode() )
            mpOpt->SetPrtFormat( sal_True );
        mpOpt->SetPDFExport(bSet);
    }
}

void  SwViewShell::SetReadonlySelectionOption(sal_Bool bSet)
{
    if( bSet != mpOpt->IsSelectionInReadonly() )
    {
        mpOpt->SetSelectionInReadonly(bSet);
    }
}

void SwViewShell::SetPrtFormatOption( sal_Bool bSet )
{
    mpOpt->SetPrtFormat( bSet );
}

void SwViewShell::UISizeNotify()
{
    if ( mbDocSizeChgd )
    {
        mbDocSizeChgd = sal_False;
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

sal_Bool SwViewShell::IsNewLayout() const
{
    return GetLayout()->IsNewLayout();
}

uno::Reference< ::com::sun::star::accessibility::XAccessible > SwViewShell::CreateAccessible()
{
    uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc;

    
    OSL_ENSURE( mpLayout, "no layout, no access" );
    OSL_ENSURE( GetWin(), "no window, no access" );

    if( mpDoc->GetCurrentViewShell() && GetWin() )
        xAcc = Imp()->GetAccessibleMap().GetDocumentView();

    return xAcc;
}

uno::Reference< ::com::sun::star::accessibility::XAccessible >
SwViewShell::CreateAccessiblePreview()
{
    OSL_ENSURE( IsPreview(),
                "Can't create accessible preview for non-preview SwViewShell" );

    
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
    return NULL;
}

void SwViewShell::InvalidateAccessibleFocus()
{
    if( Imp()->IsAccessible() )
        Imp()->GetAccessibleMap().InvalidateFocus();
}

/**
 * invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs #i27138#
 */
void SwViewShell::InvalidateAccessibleParaFlowRelation( const SwTxtFrm* _pFromTxtFrm,
                                                      const SwTxtFrm* _pToTxtFrm )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaFlowRelation( _pFromTxtFrm, _pToTxtFrm );
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
void SwViewShell::InvalidateAccessibleParaAttrs( const SwTxtFrm& rTxtFrm )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaAttrs( rTxtFrm );
    }
}

SwAccessibleMap* SwViewShell::GetAccessibleMap()
{
    if ( Imp()->IsAccessible() )
    {
        return &(Imp()->GetAccessibleMap());
    }

    return 0;
}

void SwViewShell::ApplyAccessiblityOptions(SvtAccessibilityOptions& rAccessibilityOptions)
{
    if(mpOpt->IsPagePreview() && !rAccessibilityOptions.GetIsForPagePreviews())
    {
        mpAccOptions->SetAlwaysAutoColor(sal_False);
        mpAccOptions->SetStopAnimatedGraphics(sal_False);
        mpAccOptions->SetStopAnimatedText(sal_False);
    }
    else
    {
        mpAccOptions->SetAlwaysAutoColor(rAccessibilityOptions.GetIsAutomaticFontColor());
        mpAccOptions->SetStopAnimatedGraphics(! rAccessibilityOptions.GetIsAllowAnimatedGraphics());
        mpAccOptions->SetStopAnimatedText(! rAccessibilityOptions.GetIsAllowAnimatedText());

        
        
        mpOpt->SetSelectionInReadonly(rAccessibilityOptions.IsSelectionInReadonly());
    }
}

ShellResource* SwViewShell::GetShellRes()
{
    return mpShellRes;
}

void SwViewShell::SetCareWin( Window* pNew )
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


sal_Int32 SwViewShell::GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const
{
    OSL_ENSURE( GetLayout(), "GetPageNumAndSetOffsetForPDF assumes presence of layout" );

    sal_Int32 nRet = -1;

    
    SwRect aRect( rRect );
    aRect.Pos().X() = std::max( aRect.Left(), GetLayout()->Frm().Left() );

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


const BitmapEx& SwViewShell::GetReplacementBitmap( bool bIsErrorState )
{
    BitmapEx** ppRet;
    sal_uInt16 nResId = 0;
    if( bIsErrorState )
    {
        ppRet = &mpErrorBmp;
        nResId = RID_GRAPHIC_ERRORBMP;
    }
    else
    {
        ppRet = &mpReplaceBmp;
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
    DELETEZ( mpErrorBmp );
    DELETEZ( mpReplaceBmp );
}

SwPostItMgr* SwViewShell::GetPostItMgr()
{
    SwView* pView =  GetDoc()->GetDocShell() ? GetDoc()->GetDocShell()->GetView() : 0;
    if ( pView )
        return pView->GetPostItMgr();

    return 0;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwViewShell::getIDocumentSettingAccess() const { return mpDoc; }
IDocumentSettingAccess* SwViewShell::getIDocumentSettingAccess() { return mpDoc; }
const IDocumentDeviceAccess* SwViewShell::getIDocumentDeviceAccess() const { return mpDoc; }
IDocumentDeviceAccess* SwViewShell::getIDocumentDeviceAccess() { return mpDoc; }
const IDocumentMarkAccess* SwViewShell::getIDocumentMarkAccess() const { return mpDoc->getIDocumentMarkAccess(); }
IDocumentMarkAccess* SwViewShell::getIDocumentMarkAccess() { return mpDoc->getIDocumentMarkAccess(); }
const IDocumentDrawModelAccess* SwViewShell::getIDocumentDrawModelAccess() const { return mpDoc; }
IDocumentDrawModelAccess* SwViewShell::getIDocumentDrawModelAccess() { return mpDoc; }
const IDocumentRedlineAccess* SwViewShell::getIDocumentRedlineAccess() const { return mpDoc; }
IDocumentRedlineAccess* SwViewShell::getIDocumentRedlineAccess() { return mpDoc; }
const IDocumentLayoutAccess* SwViewShell::getIDocumentLayoutAccess() const { return mpDoc; }
IDocumentLayoutAccess* SwViewShell::getIDocumentLayoutAccess() { return mpDoc; }
IDocumentContentOperations* SwViewShell::getIDocumentContentOperations() { return mpDoc; }
IDocumentStylePoolAccess* SwViewShell::getIDocumentStylePoolAccess() { return mpDoc; }
const IDocumentStatistics* SwViewShell::getIDocumentStatistics() const { return mpDoc; }

IDocumentUndoRedo      & SwViewShell::GetIDocumentUndoRedo()
{ return mpDoc->GetIDocumentUndoRedo(); }
IDocumentUndoRedo const& SwViewShell::GetIDocumentUndoRedo() const
{ return mpDoc->GetIDocumentUndoRedo(); }


const IDocumentListItems* SwViewShell::getIDocumentListItemsAccess() const
{
    return mpDoc;
}

const IDocumentOutlineNodes* SwViewShell::getIDocumentOutlineNodesAccess() const
{
    return mpDoc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
