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

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdpagv.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>

#include <svx/svdoutl.hxx>

#include "fesh.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"
#include "dflyobj.hxx"
#include "viewopt.hxx"
#include "printdata.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <IDocumentDrawModelAccess.hxx>

void SwViewImp::StartAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( pSh->ISA(SwFEShell) )
            ((SwFEShell*)pSh)->HideChainMarker(); 
    }
}

void SwViewImp::EndAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( pSh->ISA(SwFEShell) )
            ((SwFEShell*)pSh)->SetChainMarker(); 
    }
}

void SwViewImp::LockPaint()
{
    if ( HasDrawView() )
    {
        bResetHdlHiddenPaint = !GetDrawView()->areMarkHandlesHidden();
        GetDrawView()->hideMarkHandles();
    }
    else
    {
        bResetHdlHiddenPaint = sal_False;
    }
}

void SwViewImp::UnlockPaint()
{
    if ( bResetHdlHiddenPaint )
        GetDrawView()->showMarkHandles();
}

void SwViewImp::PaintLayer( const SdrLayerID _nLayerID,
                            SwPrintData const*const pPrintData,
                            const SwRect& aPaintRect,
                            const Color* _pPageBackgrdColor,
                            const bool _bIsPageRightToLeft,
                            sdr::contact::ViewObjectContactRedirector* pRedirector ) const
{
    if ( HasDrawView() )
    {
        
        OutputDevice* pOutDev = GetShell()->GetOut();
        sal_uLong nOldDrawMode = pOutDev->GetDrawMode();
        if( GetShell()->GetWin() &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode() &&
            (!GetShell()->IsPreview()||SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews()))
        {
            pOutDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                                DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
        }

        
        
        
        
        Color aOldOutlinerBackgrdColor;
        
        
        EEHorizontalTextDirection aOldEEHoriTextDir = EE_HTEXTDIR_L2R;
        const IDocumentDrawModelAccess* pIDDMA = GetShell()->getIDocumentDrawModelAccess();
        if ( (_nLayerID == pIDDMA->GetHellId()) ||
             (_nLayerID == pIDDMA->GetHeavenId()) )
        {
            OSL_ENSURE( _pPageBackgrdColor,
                    "incorrect usage of SwViewImp::PaintLayer: pPageBackgrdColor have to be set for painting layer <hell> or <heaven>");
            if ( _pPageBackgrdColor )
            {
                aOldOutlinerBackgrdColor =
                        GetDrawView()->GetModel()->GetDrawOutliner().GetBackgroundColor();
                GetDrawView()->GetModel()->GetDrawOutliner().SetBackgroundColor( *_pPageBackgrdColor );
            }

            aOldEEHoriTextDir =
                GetDrawView()->GetModel()->GetDrawOutliner().GetDefaultHorizontalTextDirection();
            EEHorizontalTextDirection aEEHoriTextDirOfPage =
                _bIsPageRightToLeft ? EE_HTEXTDIR_R2L : EE_HTEXTDIR_L2R;
            GetDrawView()->GetModel()->GetDrawOutliner().SetDefaultHorizontalTextDirection( aEEHoriTextDirOfPage );
        }

        pOutDev->Push( PUSH_LINECOLOR ); 
        if (pPrintData)
        {
            
            SdrView &rSdrView = const_cast< SdrView & >(GetPageView()->GetView());
            rSdrView.setHideDraw( !pPrintData->IsPrintDraw() );
        }
        GetPageView()->DrawLayer( _nLayerID, pOutDev, pRedirector, aPaintRect.SVRect() );
        pOutDev->Pop();

        
        if ( (_nLayerID == pIDDMA->GetHellId()) ||
             (_nLayerID == pIDDMA->GetHeavenId()) )
        {
            GetDrawView()->GetModel()->GetDrawOutliner().SetBackgroundColor( aOldOutlinerBackgrdColor );
            GetDrawView()->GetModel()->GetDrawOutliner().SetDefaultHorizontalTextDirection( aOldEEHoriTextDir );
        }

        pOutDev->SetDrawMode( nOldDrawMode );
    }
}

#define FUZZY_EDGE 400

sal_Bool SwViewImp::IsDragPossible( const Point &rPoint )
{
    if ( !HasDrawView() )
        return sal_False;

    const SdrMarkList &rMrkList = GetDrawView()->GetMarkedObjectList();

    if( !rMrkList.GetMarkCount() )
        return sal_False;

    SdrObject *pO = rMrkList.GetMark(rMrkList.GetMarkCount()-1)->GetMarkedSdrObj();

    SwRect aRect;
    if( pO && ::CalcClipRect( pO, aRect, sal_False ) )
    {
        SwRect aTmp;
        ::CalcClipRect( pO, aTmp, sal_True );
        aRect.Union( aTmp );
    }
    else
        aRect = GetShell()->GetLayout()->Frm();

    aRect.Top(    aRect.Top()    - FUZZY_EDGE );
    aRect.Bottom( aRect.Bottom() + FUZZY_EDGE );
    aRect.Left(   aRect.Left()   - FUZZY_EDGE );
    aRect.Right(  aRect.Right()  + FUZZY_EDGE );
    return aRect.IsInside( rPoint );
}

void SwViewImp::NotifySizeChg( const Size &rNewSz )
{
    if ( !HasDrawView() )
        return;

    if ( GetPageView() )
        GetPageView()->GetPage()->SetSize( rNewSz );

    
    const Rectangle aRect( Point( DOCUMENTBORDER, DOCUMENTBORDER ), rNewSz );
    const Rectangle &rOldWork = GetDrawView()->GetWorkArea();
    bool bCheckDrawObjs = false;
    if ( aRect != rOldWork )
    {
        if ( rOldWork.Bottom() > aRect.Bottom() || rOldWork.Right() > aRect.Right())
            bCheckDrawObjs = true;
        GetDrawView()->SetWorkArea( aRect );
    }
    if ( !bCheckDrawObjs )
        return;

    OSL_ENSURE( pSh->getIDocumentDrawModelAccess()->GetDrawModel(), "NotifySizeChg without DrawModel" );
    SdrPage* pPage = pSh->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 );
    const sal_uLong nObjs = pPage->GetObjCount();
    for( sal_uLong nObj = 0; nObj < nObjs; ++nObj )
    {
        SdrObject *pObj = pPage->GetObj( nObj );
        if( !pObj->ISA(SwVirtFlyDrawObj) )
        {
            
            const SwContact *pCont = (SwContact*)GetUserCall(pObj);
            
            
            
            if( !pCont || !pCont->ISA(SwDrawContact) )
                continue;

            const SwFrm *pAnchor = ((SwDrawContact*)pCont)->GetAnchorFrm();
            if ( !pAnchor || pAnchor->IsInFly() || !pAnchor->IsValid() ||
                 !pAnchor->GetUpper() || !pAnchor->FindPageFrm() ||
                 (FLY_AS_CHAR == pCont->GetFmt()->GetAnchor().GetAnchorId()) )
            {
                continue;
            }

            
            if ( pAnchor->FindFooterOrHeader() )
            {
                continue;
            }

            const Rectangle aBound( pObj->GetCurrentBoundRect() );
            if ( !aRect.IsInside( aBound ) )
            {
                Size aSz;
                if ( aBound.Left() > aRect.Right() )
                    aSz.Width() = (aRect.Right() - aBound.Left()) - MINFLY;
                if ( aBound.Top() > aRect.Bottom() )
                    aSz.Height() = (aRect.Bottom() - aBound.Top()) - MINFLY;
                if ( aSz.Width() || aSz.Height() )
                    pObj->Move( aSz );

                
                aSz.Width() = aSz.Height() = 0;
                if ( aBound.Bottom() < aRect.Top() )
                    aSz.Width() = (aBound.Bottom() - aRect.Top()) - MINFLY;
                if ( aBound.Right() < aRect.Left() )
                    aSz.Height() = (aBound.Right() - aRect.Left()) - MINFLY;
                if ( aSz.Width() || aSz.Height() )
                    pObj->Move( aSz );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
