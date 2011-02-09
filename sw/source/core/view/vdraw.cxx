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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <tools/shl.hxx>
#include <swmodule.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/svdpagv.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>

/// OD 29.08.2002 #102450#
/// include <svx/svdoutl.hxx>
#include <svx/svdoutl.hxx>

#ifdef DBG_UTIL
#include <svx/fmglob.hxx>
#endif

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

#include <IDocumentDrawModelAccess.hxx>


/*************************************************************************
|*
|*  SwSaveHdl
|*
|*  Ersterstellung      MA 14. Feb. 95
|*  Letzte Aenderung    MA 02. Jun. 98
|*
|*************************************************************************/
//SwSaveHdl::SwSaveHdl( SwViewImp *pI ) :
//  pImp( pI ),
//  bXorVis( sal_False )
//{
    //if ( pImp->HasDrawView() )
    //{
    //  bXorVis = pImp->GetDrawView()->IsShownXorVisible( pImp->GetShell()->GetOut());
    //  if ( bXorVis )
    //      pImp->GetDrawView()->HideShownXor( pImp->GetShell()->GetOut() );
    //}
//}


//SwSaveHdl::~SwSaveHdl()
//{
    //if ( bXorVis )
    //  pImp->GetDrawView()->ShowShownXor( pImp->GetShell()->GetOut() );
//}


/*************************************************************************
|*
|*  SwViewImp::StartAction(), EndAction()
|*
|*  Ersterstellung      MA 14. Feb. 95
|*  Letzte Aenderung    MA 14. Sep. 98
|*
|*************************************************************************/


void SwViewImp::StartAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        if ( pSh->ISA(SwFEShell) )
            ((SwFEShell*)pSh)->HideChainMarker();   //Kann sich geaendert haben
        //bResetXorVisibility = GetDrawView()->IsShownXorVisible( GetShell()->GetOut());
        //GetDrawView()->HideShownXor( GetShell()->GetOut() );
    }
}



void SwViewImp::EndAction()
{
    if ( HasDrawView() )
    {
        SET_CURR_SHELL( GetShell() );
        //if ( bResetXorVisibility )
        //  GetDrawView()->ShowShownXor( GetShell()->GetOut() );
        if ( pSh->ISA(SwFEShell) )
            ((SwFEShell*)pSh)->SetChainMarker();    //Kann sich geaendert haben
    }
}

/*************************************************************************
|*
|*  SwViewImp::LockPaint(), UnlockPaint()
|*
|*  Ersterstellung      MA 11. Jun. 96
|*  Letzte Aenderung    MA 11. Jun. 96
|*
|*************************************************************************/


void SwViewImp::LockPaint()
{
    if ( HasDrawView() )
    {
        //HMHbShowHdlPaint = GetDrawView()->IsMarkHdlShown();
        //HMHif ( bShowHdlPaint )
        //HMH   GetDrawView()->HideMarkHdl();
        bResetHdlHiddenPaint = !GetDrawView()->areMarkHandlesHidden();
        GetDrawView()->hideMarkHandles();
    }
    else
    {
        //HMHbShowHdlPaint = sal_False;
        bResetHdlHiddenPaint = sal_False;
    }
}



void SwViewImp::UnlockPaint()
{
    if ( bResetHdlHiddenPaint )
        GetDrawView()->showMarkHandles();
    //HMHif ( bShowHdlPaint )
    //HMH   GetDrawView()->ShowMarkHdl();
}


/*************************************************************************
|*
|*  SwViewImp::PaintLayer(), PaintDispatcher()
|*
|*  Ersterstellung      MA 20. Dec. 94
|*  Letzte Aenderung    AMA 04. Jun. 98
|*
|*************************************************************************/
// OD 29.08.2002 #102450#
// add 3rd paramter <const Color* pPageBackgrdColor> for setting this
// color as the background color at the outliner of the draw view.
// OD 09.12.2002 #103045# - add 4th parameter for the horizontal text direction
// of the page in order to set the default horizontal text direction at the
// outliner of the draw view for painting layers <hell> and <heaven>.
// OD 25.06.2003 #108784# - correct type of 1st parameter
void SwViewImp::PaintLayer( const SdrLayerID _nLayerID,
                            SwPrintData const*const pPrintData,
                            const SwRect& ,
                            const Color* _pPageBackgrdColor,
                            const bool _bIsPageRightToLeft ) const
{
    if ( HasDrawView() )
    {
        //change the draw mode in high contrast mode
        OutputDevice* pOutDev = GetShell()->GetOut();
        sal_uLong nOldDrawMode = pOutDev->GetDrawMode();
        if( GetShell()->GetWin() &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode() &&
            (!GetShell()->IsPreView()||SW_MOD()->GetAccessibilityOptions().GetIsForPagePreviews()))
        {
            pOutDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                                DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
        }

        // OD 29.08.2002 #102450#
        // For correct handling of accessibility, high contrast, the page background
        // color is set as the background color at the outliner of the draw view.
        // Only necessary for the layers hell and heaven
        Color aOldOutlinerBackgrdColor;
        // OD 09.12.2002 #103045# - set default horizontal text direction on
        // painting <hell> or <heaven>.
        EEHorizontalTextDirection aOldEEHoriTextDir = EE_HTEXTDIR_L2R;
        const IDocumentDrawModelAccess* pIDDMA = GetShell()->getIDocumentDrawModelAccess();
        if ( (_nLayerID == pIDDMA->GetHellId()) ||
             (_nLayerID == pIDDMA->GetHeavenId()) )
        {
            ASSERT( _pPageBackgrdColor,
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

        pOutDev->Push( PUSH_LINECOLOR ); // #114231#
        if (pPrintData)
        {
            // hide drawings but not form controls (form controls are handled elsewhere)
            SdrView &rSdrView = const_cast< SdrView & >(GetPageView()->GetView());
            rSdrView.setHideDraw( !pPrintData->IsPrintDraw() );
        }
        GetPageView()->DrawLayer(_nLayerID, pOutDev);
        pOutDev->Pop();

        // OD 29.08.2002 #102450#
        // reset background color of the outliner
        // OD 09.12.2002 #103045# - reset default horizontal text direction
        if ( (_nLayerID == pIDDMA->GetHellId()) ||
             (_nLayerID == pIDDMA->GetHeavenId()) )
        {
            GetDrawView()->GetModel()->GetDrawOutliner().SetBackgroundColor( aOldOutlinerBackgrdColor );
            GetDrawView()->GetModel()->GetDrawOutliner().SetDefaultHorizontalTextDirection( aOldEEHoriTextDir );
        }

        pOutDev->SetDrawMode( nOldDrawMode );
    }
}

/*************************************************************************
|*
|*  SwViewImp::IsDragPossible()
|*
|*  Ersterstellung      MA 19. Jan. 93
|*  Letzte Aenderung    MA 16. Jan. 95
|*
|*************************************************************************/
#define WIEDUWILLST 400

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

    aRect.Top(    aRect.Top()    - WIEDUWILLST );
    aRect.Bottom( aRect.Bottom() + WIEDUWILLST );
    aRect.Left(   aRect.Left()   - WIEDUWILLST );
    aRect.Right(  aRect.Right()  + WIEDUWILLST );
    return aRect.IsInside( rPoint );
}

/*************************************************************************
|*
|*  SwViewImp::NotifySizeChg()
|*
|*  Ersterstellung      MA 23. Jun. 93
|*  Letzte Aenderung    MA 05. Oct. 98
|*
|*************************************************************************/

void SwViewImp::NotifySizeChg( const Size &rNewSz )
{
    if ( !HasDrawView() )
        return;

    if ( GetPageView() )
        GetPageView()->GetPage()->SetSize( rNewSz );

    //Begrenzung des Arbeitsbereiches.
    const Rectangle aRect( Point( DOCUMENTBORDER, DOCUMENTBORDER ), rNewSz );
    const Rectangle &rOldWork = GetDrawView()->GetWorkArea();
    sal_Bool bCheckDrawObjs = sal_False;
    if ( aRect != rOldWork )
    {
        if ( rOldWork.Bottom() > aRect.Bottom() || rOldWork.Right() > aRect.Right())
            bCheckDrawObjs = sal_True;
        GetDrawView()->SetWorkArea( aRect );
    }
    if ( !bCheckDrawObjs )
        return;

    ASSERT( pSh->getIDocumentDrawModelAccess()->GetDrawModel(), "NotifySizeChg without DrawModel" );
    SdrPage* pPage = pSh->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 );
    const sal_uLong nObjs = pPage->GetObjCount();
    for( sal_uLong nObj = 0; nObj < nObjs; ++nObj )
    {
        SdrObject *pObj = pPage->GetObj( nObj );
        if( !pObj->ISA(SwVirtFlyDrawObj) )
        {
            //Teilfix(26793): Objekte, die in Rahmen verankert sind, brauchen
            //nicht angepasst werden.
            const SwContact *pCont = (SwContact*)GetUserCall(pObj);
            //JP - 16.3.00 Bug 73920: this function might be called by the
            //              InsertDocument, when a PageDesc-Attribute is
            //              set on a node. Then the SdrObject must not have
            //              an UserCall.
            if( !pCont || !pCont->ISA(SwDrawContact) )
                continue;

            const SwFrm *pAnchor = ((SwDrawContact*)pCont)->GetAnchorFrm();
            if ( !pAnchor || pAnchor->IsInFly() || !pAnchor->IsValid() ||
                 !pAnchor->GetUpper() || !pAnchor->FindPageFrm() ||
                 (FLY_AS_CHAR == pCont->GetFmt()->GetAnchor().GetAnchorId()) )
            {
                continue;
            }

            // OD 19.06.2003 #108784# - no move for drawing objects in header/footer
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

                //Notanker: Grosse Objekte nicht nach oben verschwinden lassen.
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



