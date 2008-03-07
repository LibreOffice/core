/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vdraw.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:00:00 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _XOUTX_HXX //autogen
#include <svx/xoutx.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif

/// OD 29.08.2002 #102450#
/// include <svx/svdoutl.hxx>
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#ifndef PRODUCT
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#endif

#include "fesh.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "viewimp.hxx"
#include "dflyobj.hxx"
#include "viewopt.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef IDOCUMENTDRAWMODELACCESS_HXX_INCLUDED
#include <IDocumentDrawModelAccess.hxx>
#endif

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
//  bXorVis( FALSE )
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
        //HMHbShowHdlPaint = FALSE;
        bResetHdlHiddenPaint = FALSE;
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
                            const SwRect& ,
                            const Color* _pPageBackgrdColor,
                            const bool _bIsPageRightToLeft ) const
{
    if ( HasDrawView() )
    {
        //change the draw mode in high contrast mode
        OutputDevice* pOutDev = GetShell()->GetOut();
        ULONG nOldDrawMode = pOutDev->GetDrawMode();
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
        // Region aDrawRegion(_rRect.SVRect());
        //GetPageView()->DrawLayer( _nLayerID, aDrawRegion, pOutDev,
        //              GetShell()->IsPreView() ? SDRPAINTMODE_ANILIKEPRN : 0);
        GetPageView()->DrawLayer( _nLayerID, pOutDev, GetShell()->IsPreView() ? SDRPAINTMODE_ANILIKEPRN : 0);
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

BOOL SwViewImp::IsDragPossible( const Point &rPoint )
{
    if ( !HasDrawView() )
        return FALSE;

    const SdrMarkList &rMrkList = GetDrawView()->GetMarkedObjectList();

    if( !rMrkList.GetMarkCount() )
        return FALSE;

    SdrObject *pO = rMrkList.GetMark(rMrkList.GetMarkCount()-1)->GetMarkedSdrObj();

    SwRect aRect;
    if( ::CalcClipRect( pO, aRect, FALSE ) )
    {
        SwRect aTmp;
        ::CalcClipRect( pO, aTmp, TRUE );
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
    BOOL bCheckDrawObjs = FALSE;
    if ( aRect != rOldWork )
    {
        if ( rOldWork.Bottom() > aRect.Bottom() || rOldWork.Right() > aRect.Right())
            bCheckDrawObjs = TRUE;
        GetDrawView()->SetWorkArea( aRect );
    }
    if ( !bCheckDrawObjs )
        return;

    ASSERT( pSh->getIDocumentDrawModelAccess()->GetDrawModel(), "NotifySizeChg without DrawModel" );
    SdrPage* pPage = pSh->getIDocumentDrawModelAccess()->GetDrawModel()->GetPage( 0 );
    const ULONG nObjs = pPage->GetObjCount();
    for( ULONG nObj = 0; nObj < nObjs; ++nObj )
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
                 FLY_IN_CNTNT == pCont->GetFmt()->GetAnchor().GetAnchorId() )
                continue;

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



