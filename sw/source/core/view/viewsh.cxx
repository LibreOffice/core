/*************************************************************************
 *
 *  $RCSfile: viewsh.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: kz $ $Date: 2003-10-15 09:58:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#define _SVX_PARAITEM_HXX
#define _SVX_TEXTITEM_HXX

#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SHL_HXX
//#include <tools/shl.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>
#endif
#ifndef _SWREGION_HXX
#include <swregion.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _LAYACT_HXX
#include <layact.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _SCRRECT_HXX
#include <scrrect.hxx>      // SwScrollRect, SwScrollRects
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifdef ACCESSIBLE_LAYOUT
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_ACCESSIBILITYOPTIONS_HXX
#include <svtools/accessibilityoptions.hxx>
#endif
#ifndef _ACCESSIBILITYOPTIONS_HXX
#include <accessibilityoptions.hxx>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif
// OD 14.01.2003 #103492#
#ifndef _PAGEPREVIEWLAYOUT_HXX
#include <pagepreviewlayout.hxx>
#endif

BOOL ViewShell::bLstAct = FALSE;
ShellResource *ViewShell::pShellRes = 0;
Window *ViewShell::pCareWindow = 0;

FASTBOOL bInSizeNotify = FALSE;

DBG_NAME(LayoutIdle);

TYPEINIT0(ViewShell);

/******************************************************************************
|*
|*  ViewShell::ImplEndAction()
|*
|*  Letzte Aenderung    MA 04. Sep. 96
|*
******************************************************************************/

void ViewShell::ImplEndAction( const BOOL bIdleEnd )
{
    //Fuer den Drucker gibt es hier nichts zu tun.
    if ( !GetWin() || IsPreView() )
    {
        bPaintWorks = TRUE;
        UISizeNotify();
        return;
    }

    // #94195# remember when the handles need refresh at end of method
    sal_Bool bRefreshMarker(sal_False);

    bInEndAction = TRUE;

    //Laeuft hiermit das EndAction der Letzten Shell im Ring?
    ViewShell::bLstAct = TRUE;
    ViewShell *pSh = (ViewShell*)this->GetNext();
    while ( pSh != this )
    {   if ( pSh->ActionPend() )
        {   ViewShell::bLstAct = FALSE;
            pSh = this;
        }
        else
            pSh = (ViewShell*)pSh->GetNext();
    }

    SET_CURR_SHELL( this );
    if ( Imp()->HasDrawView() && !Imp()->GetDrawView()->IsMarkHdlHidden() )
        Imp()->StartAction();

    if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegions();

    const FASTBOOL bExtraData = ::IsExtraData( GetDoc() );

    if ( !bIdleEnd )
    {
        if ( Imp()->IsNextScroll() && !bExtraData )
            Imp()->SetScroll();
        else
        {
            if ( bExtraData )
                Imp()->bScroll = FALSE;
            Imp()->SetNextScroll();
            Imp()->ResetScroll();
        }
        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( FALSE );
        if ( nLockPaint )
            aAction.SetPaint( FALSE );
        aAction.SetInputType( INPUT_KEYBOARD );
        aAction.Action();
        Imp()->SetScroll();
    }

    //Wenn wir selbst keine Paints erzeugen, so warten wir auf das Paint
    //vom System. Dann ist das Clipping korrekt gesetzt; Beispiel: verschieben
    //eines DrawObjektes.
    if ( Imp()->GetRegion()     || Imp()->GetScrollRects() ||
         aInvalidRect.HasArea() || bExtraData )
    {
        if ( !nLockPaint )
        {
            FASTBOOL bPaintsFromSystem = aInvalidRect.HasArea();
            GetWin()->Update();
            if ( aInvalidRect.HasArea() )
            {
                if ( bPaintsFromSystem )
                    Imp()->AddPaintRect( aInvalidRect );

                // AW 22.09.99: tell DrawView that drawing order will be rearranged
                // to give it a chance to react with proper IAO updates
                if (HasDrawView())
                {
                    GetDrawView()->ForceInvalidateMarkHandles();

                    // #94195# set remark
                    bRefreshMarker = sal_True;
                }

                ResetInvalidRect();
                bPaintsFromSystem = TRUE;
            }
            bPaintWorks = TRUE;

            SwRegionRects *pRegion = Imp()->GetRegion();

            //JP 27.11.97: wer die Selection hided, muss sie aber auch
            //              wieder Showen. Sonst gibt es Paintfehler!
            //  z.B.: addional Mode, Seite vertikal hab zu sehen, in der
            // Mitte eine Selektion und mit einem anderen Cursor an linken
            // rechten Rand springen. Ohne ShowCrsr verschwindet die
            // Selektion
            BOOL bShowCrsr = (pRegion || Imp()->GetScrollRects()) &&
                                IsA( TYPE(SwCrsrShell) );
            if( bShowCrsr )
                ((SwCrsrShell*)this)->HideCrsrs();

            Scroll();
            if ( bPaintsFromSystem && Imp()->pScrolledArea )
                Imp()->FlushScrolledArea();

            if ( pRegion )
            {
                SwRootFrm* pLayout = GetLayout();

                Imp()->pRegion = NULL;

                //Erst Invert dann Compress, niemals andersherum!
                pRegion->Invert();

                const USHORT nCnt = pRegion->Count();
                pRegion->Compress();

                VirtualDevice *pVout = 0;
                while ( pRegion->Count() )
                {
                    SwRect aRect( (*pRegion)[ pRegion->Count() - 1 ] );
                    pRegion->Remove( pRegion->Count() - 1 );

                    BOOL bPaint = TRUE;
                    if ( IsEndActionByVirDev() )
                    {
                        //virtuelles device erzeugen und einstellen.
                        if ( !pVout )
                            pVout = new VirtualDevice( *GetOut() );
                        MapMode aMapMode( GetOut()->GetMapMode() );
                        pVout->SetMapMode( aMapMode );

                        BOOL bSizeOK = TRUE;

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
                            bPaint = FALSE;
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
                            pLayout->Paint( aRect );
                            pOld->DrawOutDev( aRect.Pos(), aRect.SSize(),
                                              aRect.Pos(), aRect.SSize(), *pVout );
                            pOut = pOld;

                            if( !GetViewOptions()->IsReadonly() &&
                                GetViewOptions()->IsControl() )
                            {
                                Imp()->PaintLayer( pDoc->GetControlsId(), VisArea() );
                            }
                        }
                    }
                    if ( bPaint )
                    {
                        if ( bPaintsFromSystem )
                            PaintDesktop( aRect );
                        pLayout->Paint( aRect );
                    }
                }
                delete pVout;
                delete pRegion;
                Imp()->DelRegions();
            }
            if( bShowCrsr )
                ((SwCrsrShell*)this)->ShowCrsrs( TRUE );
        }
        else
        {
            Imp()->DelRegions();
            bPaintWorks =  TRUE;
        }
    }
    else
        bPaintWorks = TRUE;

    bInEndAction = FALSE;
    ViewShell::bLstAct = FALSE;
    Imp()->EndAction();


    //Damit sich die automatischen Scrollbars auch richtig anordnen k”nnen
    //muessen wir die Aktion hier kuenstlich beenden (EndAction loesst ein
    //Notify aus, und das muss Start-/EndAction rufen um die  Scrollbars
    //klarzubekommen.
    --nStartAction;
    UISizeNotify();
    ++nStartAction;

#ifndef PRODUCT
    // No Scroll starts the timer to repair the scrolled area automatically
    if( GetViewOptions()->IsTest8() )
#endif
    if ( Imp()->IsScrolled() )
        Imp()->RestartScrollTimer();

    // #94195# refresh handles when they were hard removed for display change
    if(bRefreshMarker && HasDrawView())
    {
        GetDrawView()->AdjustMarkHdl(FALSE);
    }

#ifdef ACCESSIBLE_LAYOUT
    if( Imp()->IsAccessible() )
        Imp()->FireAccessibleEvents();
#endif
}

/******************************************************************************
|*
|*  ViewShell::ImplStartAction()
|*
|*  Ersterstellung      MA 25. Jul. 94
|*  Letzte Aenderung    MA 25. Jul. 94
|*
******************************************************************************/

void ViewShell::ImplStartAction()
{
    bPaintWorks = FALSE;
    Imp()->StartAction();
}


/******************************************************************************
|*
|*  ViewShell::ImplLockPaint(), ImplUnlockPaint()
|*
|*  Ersterstellung      MA 11. Jun. 96
|*  Letzte Aenderung    MA 11. Jun. 96
|*
******************************************************************************/

void ViewShell::ImplLockPaint()
{
    if ( GetWin() && GetWin()->IsVisible() )
        GetWin()->EnablePaint( FALSE ); //Auch die Controls abklemmen.
    Imp()->LockPaint();
}


void ViewShell::ImplUnlockPaint( BOOL bVirDev )
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
                GetWin()->EnablePaint( TRUE );
                GetWin()->Validate();

                Imp()->UnlockPaint();
                pVout->SetLineColor( pOut->GetLineColor() );
                pVout->SetFillColor( pOut->GetFillColor() );
                OutputDevice *pOld = pOut;
                pOut = pVout;
                Paint( VisArea().SVRect() );
                pOut = pOld;
                pOut->DrawOutDev( VisArea().Pos(), aSize,
                                  VisArea().Pos(), aSize, *pVout );
                if( GetViewOptions()->IsControl() )
                {
                    Imp()->PaintLayer( pDoc->GetControlsId(), VisArea() );
                    GetWin()->Update();//Damit aktive, transparente Controls auch
                                       //gleich durchkommen
                }
            }
            else
            {
                Imp()->UnlockPaint();
                GetWin()->EnablePaint( TRUE );
                GetWin()->Invalidate( INVALIDATE_CHILDREN );
            }
            delete pVout;
        }
        else
        {
            Imp()->UnlockPaint();
            GetWin()->EnablePaint( TRUE );
            GetWin()->Invalidate( INVALIDATE_CHILDREN );
        }
    }
    else
        Imp()->UnlockPaint();
}

/******************************************************************************
|*
|*  ViewShell::AddPaintRect()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 09. Feb. 97
|*
******************************************************************************/

BOOL ViewShell::AddPaintRect( const SwRect & rRect )
{
    BOOL bRet = FALSE;
    ViewShell *pSh = this;
    do
    {
        if ( pSh->IsPreView() && pSh->GetWin() )
//          pSh->GetWin()->Invalidate();
            ::RepaintPagePreview( pSh, rRect );
        else
            bRet |= pSh->Imp()->AddPaintRect( rRect );
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != this );
    return bRet;
}

/******************************************************************************
|*
|*  ViewShell::InvalidateWindows()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 09. Feb. 97
|*
******************************************************************************/

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
//                  pSh->GetWin()->Invalidate();
                    ::RepaintPagePreview( pSh, rRect );
                else if ( pSh->VisArea().IsOver( rRect ) )
                    pSh->GetWin()->Invalidate( rRect.SVRect() );
            }
            pSh = (ViewShell*)pSh->GetNext();

        } while ( pSh != this );
    }
}

/******************************************************************************
|*
|*  ViewShell::MakeVisible()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    AMA 10. Okt. 95
|*
******************************************************************************/

void ViewShell::MakeVisible( const SwRect &rRect )
{
    if ( !VisArea().IsInside( rRect ) || IsScrollMDI( this, rRect ) || GetCareWin(*this) )
    {
        if ( !IsViewLocked() )
        {
            if( pWin )
            {
                const SwFrm* pRoot = GetDoc()->GetRootFrm();
                int nLoopCnt = 3;
                long nOldH;
                do{
                    nOldH = pRoot->Frm().Height();
                    StartAction();
                    ScrollMDI( this, rRect, USHRT_MAX, USHRT_MAX );
                    EndAction();
                } while( nOldH != pRoot->Frm().Height() && nLoopCnt-- );
            }
#ifndef PRODUCT
            else
            {
                //MA: 04. Nov. 94, braucht doch keiner oder??
                ASSERT( !this, "MakeVisible fuer Drucker wird doch gebraucht?" );
            }

#endif
        }
    }
}

/******************************************************************************
|*
|*  ViewShell::CareChildWindow()
|*
|*  Ersterstellung      AMA 10. Okt. 95
|*  Letzte Aenderung    AMA 10. Okt. 95
|*
******************************************************************************/

Window* ViewShell::CareChildWin(ViewShell& rVSh)
{
    if(rVSh.pSfxViewShell)
    {
        const USHORT nId = SvxSearchDialogWrapper::GetChildWindowId();
        SfxViewFrame* pVFrame = rVSh.pSfxViewShell->GetViewFrame();
        const SfxChildWindow* pChWin = pVFrame->GetChildWindow( nId );
        Window *pWin = pChWin ? pChWin->GetWindow() : NULL;
        if ( pWin && pWin->IsVisible() )
            return pWin;
    }
    return NULL;
}

/******************************************************************************
|*
|*  ViewShell::GetPagePos()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 04. Aug. 93
|*
******************************************************************************/

Point ViewShell::GetPagePos( USHORT nPageNum ) const
{
    return GetLayout()->GetPagePos( nPageNum );
}

/******************************************************************************
|*
|*  ViewShell::GetNumPages()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 20. Apr. 94
|*
******************************************************************************/

USHORT ViewShell::GetNumPages()
{
    //Es kann sein, das noch kein Layout existiert weil die Methode vom
    //Root-Ctor gerufen wird.
    return GetLayout() ? GetLayout()->GetPageNum() : 0;
}

sal_Bool ViewShell::IsDummyPage( USHORT nPageNum ) const
{
    return GetLayout() ? GetLayout()->IsDummyPage( nPageNum ) : 0;
}

/*************************************************************************
|*
|*                  ViewShell::UpdateFlds()
|*
|*    Ersterstellung    BP 04.05.92
|*    Beschreibung      erzwingt ein Update fuer jedes Feld
|*
|*  UpdateFlds benachrichtigt alle Felder mit pNewHt.
|*  Wenn pNewHt == 0 ist (default), wird der Feldtyp verschickt.
|*
*************************************************************************/

void ViewShell::UpdateFlds(BOOL bCloseDB)
{
    SET_CURR_SHELL( this );

    BOOL bCrsr = ISA(SwCrsrShell);
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

// update all charts, for that exists any table
void ViewShell::UpdateAllCharts()
{
    SET_CURR_SHELL( this );
    // Start-/EndAction handled in the SwDoc-Method!
    GetDoc()->UpdateAllCharts();
}

BOOL ViewShell::HasCharts() const
{
    BOOL bRet = FALSE;
    const SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetDoc()->GetNodes().GetEndOfAutotext().
                        StartOfSectionNode(), 1 );
    while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        aIdx++;
        const SwOLENode *pNd = aIdx.GetNode().GetOLENode();
        if( pNd && pNd->GetChartTblName().Len() )
        {
            bRet = TRUE;
            break;
        }
    }
    return bRet;
}

/*************************************************************************
|*
|*    ViewShell::LayoutIdle()
|*
|*    Ersterstellung    MA 26. May. 92
|*    Letzte Aenderung  OG 19. Mar. 96
|*
*************************************************************************/

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

#ifndef PRODUCT
    // Wenn Test5 gedrueckt ist, wird der IdleFormatierer abgeknipst.
    if( pOpt->IsTest5() )
        return;
#endif

    {
        DBG_PROFSTART( LayoutIdle );

        //Cache vorbereiten und restaurieren, damit er nicht versaut wird.
        SwSaveSetLRUOfst aSave( *SwTxtFrm::GetTxtCache(),
                             SwTxtFrm::GetTxtCache()->GetCurMax() - 50 );
        SwLayIdle aIdle( GetLayout(), Imp() );
        DBG_PROFSTOP( LayoutIdle );
    }
}

/*************************************************************************
|*
|*    DOCUMENT COMPATIBILITY FLAGS
|*
*************************************************************************/

void lcl_InvalidateAllCntnt( ViewShell& rSh, BYTE nInv )
{
    BOOL bCrsr = rSh.ISA(SwCrsrShell);
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

// Absatzabstaende koennen wahlweise addiert oder maximiert werden

BOOL ViewShell::IsParaSpaceMax() const
{
    return GetDoc()->IsParaSpaceMax();
}

BOOL ViewShell::IsParaSpaceMaxAtPages() const
{
    return GetDoc()->IsParaSpaceMaxAtPages();
}

void ViewShell::SetParaSpaceMax( BOOL bNew, BOOL bAtPages )
{
    if( GetDoc()->IsParaSpaceMax() != bNew  ||
    GetDoc()->IsParaSpaceMaxAtPages() != bAtPages )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        GetDoc()->SetParaSpaceMax( bNew, bAtPages );
        const BYTE nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

BOOL ViewShell::IsTabCompat() const
{
    return GetDoc()->IsTabCompat();
}

void ViewShell::SetTabCompat( BOOL bNew )
{
    if( GetDoc()->IsTabCompat() != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        GetDoc()->SetTabCompat( bNew );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

BOOL ViewShell::IsAddFlyOffsets() const
{
    return GetDoc()->IsAddFlyOffsets();
}

void ViewShell::SetAddFlyOffsets( BOOL bNew )
{
    if( GetDoc()->IsAddFlyOffsets() != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        GetDoc()->SetAddFlyOffsets( bNew );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

sal_Bool ViewShell::IsAddExtLeading() const
{
    return GetDoc()->IsAddExtLeading();
}

void ViewShell::SetAddExtLeading( sal_Bool bNew )
{
    if ( GetDoc()->IsAddExtLeading() != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        GetDoc()->SetAddExtLeading( bNew );
        if ( GetDoc()->GetDrawModel() )
            GetDoc()->GetDrawModel()->SetAddExtLeading( bNew );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

short ViewShell::IsUseVirtualDevice() const
{
    return GetDoc()->IsUseVirtualDevice();
}

void ViewShell::SetUseVirtualDevice( short nNew )
{
    // this sets the flag at the document and calls PrtDataChanged
    GetDoc()->SetUseVirtualDevice( nNew );
}


/******************************************************************************
|*
|*  ViewShell::Reformat
|*
|*  Ersterstellung      BP ???
|*  Letzte Aenderung    MA 13. Feb. 98
|*
******************************************************************************/

void ViewShell::Reformat()
{
    SwWait aWait( *GetDoc()->GetDocShell(), TRUE );

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
        GetLayout()->InvalidateAllCntnt();
        EndAction();
    }
}

/******************************************************************************
|*
|*  ViewShell::CalcLayout()
|*                  Vollstaendige Formatierung von Layout und Inhalt.
|*
|*  Ersterstellung      MA 31. Jan. 94
|*  Letzte Aenderung    MA 08. Oct. 96
|*
******************************************************************************/

void ViewShell::CalcLayout()
{
    SET_CURR_SHELL( this );
    SwWait aWait( *GetDoc()->GetDocShell(), TRUE );

    //Cache vorbereiten und restaurieren, damit er nicht versaut wird.
    SwSaveSetLRUOfst aSaveLRU( *SwTxtFrm::GetTxtCache(),
                                  SwTxtFrm::GetTxtCache()->GetCurMax() - 50 );

    //Progress einschalten wenn noch keiner Lauft.
    const BOOL bEndProgress = SfxProgress::GetActiveProgress( GetDoc()->GetDocShell() ) == 0;
    if ( bEndProgress )
    {
        USHORT nEndPage = GetLayout()->GetPageNum();
        nEndPage += nEndPage * 10 / 100;
        ::StartProgress( STR_STATSTR_REFORMAT, 0, nEndPage, GetDoc()->GetDocShell() );
    }

    SwLayAction aAction( GetLayout(), Imp() );
    aAction.SetPaint( FALSE );
    aAction.SetStatBar( TRUE );
    aAction.SetCalcLayout( TRUE );
    aAction.SetReschedule( TRUE );
    GetDoc()->LockExpFlds();
    aAction.Action();
    GetDoc()->UnlockExpFlds();

    //Das SetNewFldLst() am Doc wurde unterbunden und muss nachgeholt
    //werden (siehe flowfrm.cxx, txtfld.cxx)
    if ( aAction.IsExpFlds() )
    {
        aAction.Reset();
        aAction.SetPaint( FALSE );
        aAction.SetStatBar( TRUE );
        aAction.SetReschedule( TRUE );

        SwDocPosUpdate aMsgHnt( 0 );
        GetDoc()->UpdatePageFlds( &aMsgHnt );
        GetDoc()->UpdateExpFlds();

        aAction.Action();
    }

    if ( VisArea().HasArea() )
        InvalidateWindows( VisArea() );
    if ( bEndProgress )
        ::EndProgress( GetDoc()->GetDocShell() );
}

/******************************************************************************
|*
|*  ViewShell::SetFirstVisPageInvalid()
|*
|*  Ersterstellung      MA 19. May. 94
|*  Letzte Aenderung    MA 19. May. 94
|*
******************************************************************************/

void ViewShell::SetFirstVisPageInvalid()
{
    ViewShell *pSh = this;
    do
    {   pSh->Imp()->SetFirstVisPageInvalid();
        pSh = (ViewShell*)pSh->GetNext();

    } while ( pSh != this );
}

/******************************************************************************
|*
|*  ViewShell::SizeChgNotify()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 17. Sep. 96
|*
******************************************************************************/

void ViewShell::SizeChgNotify(const Size &rSize)
{
    if ( !pWin )
        bDocSizeChgd = TRUE;
    else if( ActionPend() || Imp()->IsCalcLayoutProgress() || bPaintInProgress )
    {
        bDocSizeChgd = TRUE;

        if ( !Imp()->IsCalcLayoutProgress() && ISA( SwCrsrShell ) )
        {
            const SwFrm *pCnt = ((SwCrsrShell*)this)->GetCurrFrm( FALSE );
            const SwPageFrm *pPage;
            if ( pCnt && 0 != (pPage = pCnt->FindPageFrm()) )
            {
                USHORT nVirtNum = pPage->GetVirtPageNum();
                 const SvxNumberType& rNum = pPage->GetPageDesc()->GetNumType();
                String sDisplay = rNum.GetNumStr( nVirtNum );
                PageNumNotify( this, pCnt->GetPhyPageNum(), nVirtNum, sDisplay );
            }
        }
    }
    else
    {
        bDocSizeChgd = FALSE;
        ::SizeNotify( this, GetLayout()->Frm().SSize() );
    }
}

/******************************************************************************
|*
|*  ViewShell::VisPortChgd()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 22. Jul. 96
|*
******************************************************************************/

void ViewShell::VisPortChgd( const SwRect &rRect)
{
    ASSERT( GetWin(), "VisPortChgd ohne Window." );

    if ( rRect == VisArea() )
        return;

#ifndef PRODUCT
    if ( bInEndAction )
    {
        //Da Rescheduled doch schon wieder irgendwo einer?
        ASSERT( !this, "Scroll waehrend einer EndAction." );
    }
#endif

    ASSERT( rRect.Top() >= 0 && rRect.Left() >= 0 &&
            rRect.Bottom() >= 0 && rRect.Right() >= 0,
            "VisArea in die Wiese?" );

    //Ersteinmal die alte sichtbare Seite holen, dann braucht nacher nicht
    //lange gesucht werden.
    const SwFrm *pOldPage = Imp()->GetFirstVisPage();

    const SwRect aPrevArea( VisArea() );
    const BOOL bFull = aPrevArea.IsEmpty();
    aVisArea = rRect;
    SetFirstVisPageInvalid();

    //Wenn noch eine PaintRegion herumsteht und sich die VisArea geaendert hat,
    //so ist die PaintRegion spaetestens jetzt obsolete. Die PaintRegion kann
    //vom RootFrm::Paint erzeugt worden sein.
    if ( !bInEndAction &&
         Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegions();

    SET_CURR_SHELL( this );

    SwSaveHdl aSaveHdl( Imp() );

    if ( bFull )
        GetWin()->Invalidate();
    else
    {
        // Betrag ausrechnen, um den gescrolled werden muss.
        const long nXDiff = aPrevArea.Left() - VisArea().Left();
        const long nYDiff = aPrevArea.Top()  - VisArea().Top();

        if( !nXDiff && !GetDoc()->IsBrowseMode() &&
            (!Imp()->HasDrawView() || !Imp()->GetDrawView()->IsGridVisible() ) )
        {
            //Falls moeglich die Wiese nicht mit Scrollen.
            //Also linke und rechte Kante des Scrollbereiches auf die
            //Seiten begrenzen.
            const SwPageFrm *pPage = (SwPageFrm*)GetDoc()->GetRootFrm()->Lower();
            if ( pPage->Frm().Top() > pOldPage->Frm().Top() )
                pPage = (SwPageFrm*)pOldPage;
            SwRect aBoth( VisArea() );
            aBoth.Union( aPrevArea );
            const SwTwips nBottom = aBoth.Bottom();
            const SwTwips nRight  = aBoth.Right();
            SwTwips nMinLeft = LONG_MAX;
            SwTwips nMaxRight= 0;
            while ( pPage &&
                    !((pPage->Frm().Top()  > nBottom) ||
                        (pPage->Frm().Left() > nRight)))
            {
                if ( pPage->Frm().IsOver( aBoth ) )
                {
                    // OD 12.02.2003 #i9719#, #105645# - consider new border
                    // and shadow width
                    const SwTwips nBorderWidth =
                            GetOut()->PixelToLogic( Size( pPage->BorderPxWidth(), 0 ) ).Width();
                    const SwTwips nShadowWidth =
                            GetOut()->PixelToLogic( Size( pPage->ShadowPxWidth(), 0 ) ).Width();
                    // OD 03.03.2003 #107927# - use correct datatype
                    const SwTwips nPageLeft = pPage->Frm().Left() - nBorderWidth;
                    if( nPageLeft < nMinLeft )
                        nMinLeft = nPageLeft;
                    // OD 03.03.2003 #107927# - use correct datatype
                    const SwTwips nPageRight = pPage->Frm().Right() + nBorderWidth + nShadowWidth;
                    if( nPageRight > nMaxRight )
                        nMaxRight = nPageRight;
                    //Zus. auf die Zeichenobjekte abgleichen.
                    //Einen Ofst beruecksichtigen, weil die Objekte u.U.
                    //selektiert sind und die Henkel dann hinausstehen.
                    if ( pPage->GetSortedObjs() )
                    {
                        const long nOfst = GetOut()->PixelToLogic(
                            Size(Imp()->GetDrawView()->GetMarkHdlSizePixel()/2,0)).Width();
                        for ( USHORT i = 0;
                              i < pPage->GetSortedObjs()->Count(); ++i )
                        {
                            SdrObject *pObj = (*pPage->GetSortedObjs())[i];
//JP 22.12.99: why ignore FlyFrames? The result is Bug 69762 for FlyFrames
//                          if ( pObj->IsWriterFlyFrame() )
//                              continue;
                            const Rectangle &rBound = pObj->GetBoundRect();
                            // OD 03.03.2003 #107927# - use correct datatype
                            const SwTwips nL = Max( 0L, rBound.Left() - nOfst );
                            if ( nL < nMinLeft )
                                nMinLeft = nL;
                            if( rBound.Right() + nOfst > nMaxRight )
                                nMaxRight = rBound.Right() + nOfst;
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
    Imp()->bPaintInScroll = TRUE;
    GetWin()->Update();
    Imp()->bPaintInScroll = FALSE;

#ifdef ACCESSIBLE_LAYOUT
    if( Imp()->IsAccessible() )
        Imp()->UpdateAccessible();
#endif

}

/******************************************************************************
|*
|*  ViewShell::SmoothScroll()
|*
|*  Ersterstellung      MA 04. Jul. 96
|*  Letzte Aenderung    MA 25. Mar. 97
|*
******************************************************************************/

BOOL ViewShell::SmoothScroll( long lXDiff, long lYDiff, const Rectangle *pRect )
{
    const ULONG nColCnt = pOut->GetColorCount();
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

    if ( !lXDiff && bEnableSmooth && Abs(lYDiff) < lMax &&
         GetViewOptions()->IsSmoothScroll() &&
         (!ISA( SwCrsrShell ) ||
          (!((SwCrsrShell*)this)->HasSelection() &&
           ((SwCrsrShell*)this)->GetCrsrCnt() < 2)) &&
         GetWin()->GetWindowClipRegionPixel(
             WINDOW_GETCLIPREGION_NOCHILDREN|WINDOW_GETCLIPREGION_NULL )
                                                            .IsNull() )
    {
        Imp()->bStopSmooth = FALSE;

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
            PaintDesktop( aRect );
            ViewShell::bLstAct = TRUE;
            GetLayout()->Paint( aRect );
            ViewShell::bLstAct = FALSE;
            pOut = pOld;
            aVisArea = aOldVis;
            BOOL bControls = GetViewOptions()->IsControl();


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

                SwRect aOldVis = VisArea();
                aVisArea.Pos().Y() -= lScroll;
                aVisArea.Pos() = GetWin()->PixelToLogic( GetWin()->LogicToPixel( VisArea().Pos()));
                lScroll = aOldVis.Top() - VisArea().Top();
                if ( pRect )
                {
                    Rectangle aTmp( aOldVis.SVRect() );
                    aTmp.Left() = pRect->Left();
                    aTmp.Right()= pRect->Right();
                    GetWin()->Scroll( 0, lScroll, aTmp, SCROLL_CHILDREN );
                }
                else
                    GetWin()->Scroll( 0, lScroll, SCROLL_CHILDREN );
                const Point aPt( -VisArea().Left(), -VisArea().Top() );
                MapMode aMapMode( GetWin()->GetMapMode() );
                aMapMode.SetOrigin( aPt );
                GetWin()->SetMapMode( aMapMode );

                if ( Imp()->HasDrawView() )
                    Imp()->GetDrawView()->VisAreaChanged( GetWin() );

                SetFirstVisPageInvalid();
                if ( !Imp()->bStopSmooth )
                {
                    Imp()->aSmoothRect = VisArea();
                    if ( lScroll > 0 )
                        Imp()->aSmoothRect.Bottom( VisArea().Top() +
                                    lScroll + aPixSz.Height() );
                    else
                        Imp()->aSmoothRect.Top( VisArea().Bottom() +
                                    lScroll + aPixSz.Height() );

                    Imp()->bSmoothUpdate = TRUE;
                    GetWin()->Update();
                    Imp()->bSmoothUpdate = FALSE;

                    if ( !Imp()->bStopSmooth )
                    {
                        SwRect &rTmp = Imp()->aSmoothRect;
                        rTmp.Pos().Y() -= aPixSz.Height();
                        rTmp.Pos().X() -= aPixSz.Width();
                        rTmp.SSize().Height() += 2*aPixSz.Height();
                        rTmp.SSize().Width() += 2*aPixSz.Width();
                        GetWin()->DrawOutDev( rTmp.Pos(), rTmp.SSize(),
                                                rTmp.Pos(), rTmp.SSize(),
                                                *pVout );
                        if( bControls )
                            Imp()->PaintLayer( pDoc->GetControlsId(), rTmp );
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
            return TRUE;
        }
        delete pVout;
    }
//#endif
    aVisArea.Pos().X() -= lXDiff;
    aVisArea.Pos().Y() -= lYDiff;
    if ( pRect )
        GetWin()->Scroll( lXDiff, lYDiff, *pRect, SCROLL_CHILDREN );
    else
        GetWin()->Scroll( lXDiff, lYDiff, SCROLL_CHILDREN );
    return FALSE;
}

/******************************************************************************
|*
|*  ViewShell::PaintDesktop()
|*
|*  Ersterstellung      MA 16. Dec. 93
|*  Letzte Aenderung    MA 30. Nov. 95
|*
******************************************************************************/

void ViewShell::PaintDesktop( const SwRect &rRect )
{
    if ( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return;                     //Fuer den Drucker tun wir hier nix

    //Sonderfaelle abfangen, damit es nicht gar so ueberraschend aussieht.
    //Kann z.B. waehrend des Idle'ns zwischenzeitlich auftreten.
    //Die Rechtecke neben den Seiten muessen wir leider auf jedenfall Painten,
    //den diese werden spaeter beim VisPortChgd ausgespart.
    FASTBOOL bBorderOnly = FALSE;
    const SwRootFrm *pRoot = GetDoc()->GetRootFrm();
    if ( rRect.Top() > pRoot->Frm().Bottom() )
    {
        const SwFrm *pPg = pRoot->Lower();
        while ( pPg && pPg->GetNext() )
            pPg = pPg->GetNext();
        if ( !pPg || !pPg->Frm().IsOver( VisArea() ) )
            bBorderOnly = TRUE;
    }

    SwRegionRects aRegion( rRect );

    if ( bBorderOnly )
    {
        const SwFrm *pPage = pRoot->Lower();
        SwRect aLeft( rRect ), aRight( rRect );
        while ( pPage )
        {
            long nTmp = pPage->Frm().Left();
            if ( nTmp < aLeft.Right() )
                aLeft.Right( nTmp );
            nTmp = pPage->Frm().Right();
            if ( nTmp > aRight.Left() )
                aRight.Left( nTmp );
            pPage = pPage->GetNext();
        }
        aRegion.Remove( 0, aRegion.Count() );
        if ( aLeft.HasArea() )
            aRegion.Insert( aLeft, 0 );
        if ( aRight.HasArea() )
            aRegion.Insert( aRight, 1 );
    }
    else
    {
        const SwFrm *pPage = Imp()->GetFirstVisPage();
        const SwTwips nBottom = rRect.Bottom();
        const SwTwips nRight  = rRect.Right();
        while ( pPage && aRegion.Count() &&
                !((pPage->Frm().Top() > nBottom) || (pPage->Frm().Left() > nRight)))
        {
            if ( pPage->Frm().IsOver( rRect ) )
                aRegion -= pPage->Frm();
            pPage = pPage->GetNext();
        }
    }
    if ( aRegion.Count() )
        _PaintDesktop( aRegion );
}


// PaintDesktop gesplittet, dieser Teil wird auch von PreViewPage benutzt
void ViewShell::_PaintDesktop( const SwRegionRects &rRegion )
{
    GetOut()->Push( PUSH_FILLCOLOR );
    // OD 14.02.2003 #107424# - no longer needed, because color configuration
    // is loaded in constructor of <SwModule>.
    /*
    //make sure the color configuration has been loaded
    SW_MOD()->GetColorConfig();
    */
    GetOut()->SetFillColor( SwViewOption::GetAppBackgroundColor());
    for ( USHORT i = 0; i < rRegion.Count(); ++i )
        GetOut()->DrawRect( rRegion[i].SVRect() );
    GetOut()->Pop();
}

/******************************************************************************
|*
|*  ViewShell::CheckInvalidForPaint()
|*
|*  Ersterstellung      MA 19. May. 94
|*  Letzte Aenderung    MA 09. Jun. 94
|*
******************************************************************************/

BOOL ViewShell::CheckInvalidForPaint( const SwRect &rRect )
{
    if ( !GetWin() )
        return FALSE;

    const SwPageFrm *pPage = Imp()->GetFirstVisPage();
    const SwTwips nBottom = VisArea().Bottom();
    const SwTwips nRight  = VisArea().Right();
    BOOL bRet = FALSE;
    while ( !bRet && pPage && !((pPage->Frm().Top()  > nBottom) ||
                                   (pPage->Frm().Left() > nRight)))
    {
        if ( pPage->IsInvalid() || pPage->IsInvalidFly() )
            bRet = TRUE;
        pPage = (SwPageFrm*)pPage->GetNext();
    }

    if ( bRet )
    {
        //Start/EndAction wuerden hier leider nix helfen, weil das Paint vom
        //GUI 'reinkam und somit ein Clipping gesetzt ist gegen das wir nicht
        //nicht ankommen.
        //Ergo: Alles selbst machen (siehe ImplEndAction())
        if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea())
             Imp()->DelRegions();

        Imp()->ResetScroll();
        SwLayAction aAction( GetLayout(), Imp() );
        aAction.SetComplete( FALSE );
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
            BOOL bStop = TRUE;
            for ( USHORT i = 0; i < pRegion->Count(); ++i )
            {
                const SwRect &rTmp = (*pRegion)[i];
                if ( FALSE == (bStop = rTmp.IsOver( VisArea() )) )
                    break;
            }
            if ( bStop )
            {
                Imp()->DelRegions();
                pRegion = 0;
            }
        }

        if ( pRegion )
        {
            //Erst Invert dann Compress, niemals andersherum!
            pRegion->Invert();
            pRegion->Compress();
            bRet = FALSE;
            if ( pRegion->Count() )
            {
                SwRegionRects aRegion( rRect );
                for ( USHORT i = 0; i < pRegion->Count(); ++i )
                {   const SwRect &rTmp = (*pRegion)[i];
                    if ( !rRect.IsInside( rTmp ) )
                    {
                        InvalidateWindows( rTmp );
                        if ( rTmp.IsOver( VisArea() ) )
                        {   aRegion -= rTmp;
                            bRet = TRUE;
                        }
                    }
                }
                if ( bRet )
                {
                    for ( USHORT i = 0; i < aRegion.Count(); ++i )
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
                bRet = FALSE;
            Imp()->DelRegions();
        }
        else
            bRet = FALSE;
    }
    return bRet;
}

/******************************************************************************
|*
|*  ViewShell::Paint()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 17. Sep. 96
|*
******************************************************************************/

void ViewShell::Paint(const Rectangle &rRect)
{
    if ( nLockPaint )
    {
        if ( Imp()->bSmoothUpdate )
        {
            SwRect aTmp( rRect );
            if ( !Imp()->aSmoothRect.IsInside( aTmp ) )
                Imp()->bStopSmooth = TRUE;
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

    //MA 30. Jul. 95: fix(16787): mit !nStartAction versuche ich mal mich gegen
    //fehlerhaften Code an anderen Stellen zu wehren. Hoffentlich fuehrt das
    //nicht zu Problemen!?
    if ( bPaintWorks && !nStartAction )
    {
        if( GetWin() && GetWin()->IsVisible() )
        {
            //Wenn mit dem Paint ein Bereich betroffen ist, der vorher gescrolled
            //wurde, so wiederholen wir das Paint mit dem Gesamtbereich. Nur so
            //koennen wir sicherstellen, das (nicht mal kurzfristig) durch das Paint
            //keine Alignmentfehler sichtbar werden.
            SwRect aRect( rRect );
            if ( Imp()->IsScrolled() && Imp()->FlushScrolledArea( aRect ) )
            {
                GetWin()->Invalidate( aRect.SVRect() );
                return;
            }

            if ( bPaintInProgress ) //Schutz gegen doppelte Paints!
            {
                GetWin()->Invalidate( rRect );
                return;
            }

            bPaintInProgress = TRUE;
            SET_CURR_SHELL( this );
            SwRootFrm::SetNoVirDev( TRUE );

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
                ViewShell::bLstAct = TRUE;
                GetLayout()->Paint( aRect );
                ViewShell::bLstAct = FALSE;
            }
            else
            {
                SwSaveHdl *pSaveHdl = 0;
                if ( Imp()->HasDrawView() )
                    pSaveHdl = new SwSaveHdl( Imp() );

                //Wenn eine der sichtbaren Seiten noch irgendetwas zum Repaint
                //angemeldet hat, so muessen Repaints ausgeloest werden.
                if ( !CheckInvalidForPaint( aRect ) )
                {
                    PaintDesktop( aRect );
                    //Falls sinnvoll gleich das alte InvalidRect verarbeiten bzw.
                    //vernichten.
                    if ( aRect.IsInside( aInvalidRect ) )
                        ResetInvalidRect();
                    ViewShell::bLstAct = TRUE;
                    GetLayout()->Paint( aRect );
                    ViewShell::bLstAct = FALSE;
                }
                delete pSaveHdl;
            }
            SwRootFrm::SetNoVirDev( FALSE );
            bPaintInProgress = FALSE;
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
            pOut->Push( PUSH_FILLCOLOR );
            pOut->SetFillColor( Imp()->GetRetoucheColor() );
            pOut->DrawRect( rRect );
            pOut->Pop();
        }
    }
}

/******************************************************************************
|*
|*  ViewShell::SetBrowseBorder()
|*
|*  Ersterstellung      AMA 20. Aug. 96
|*  Letzte Aenderung    AMA 20. Aug. 96
|*
******************************************************************************/

void ViewShell::SetBrowseBorder( const Size& rNew )
{
    if( rNew != GetBrowseBorder() )
    {
        aBrowseBorder = rNew;
        if ( aVisArea.HasArea() )
            CheckBrowseView( FALSE );
    }
}

/******************************************************************************
|*
|*  ViewShell::CheckBrowseView()
|*
|*  Ersterstellung      MA 04. Mar. 96
|*  Letzte Aenderung    MA 04. Jul. 96
|*
******************************************************************************/

void ViewShell::CheckBrowseView( FASTBOOL bBrowseChgd )
{
    if ( !bBrowseChgd && !GetDoc()->IsBrowseMode() )
        return;

    SET_CURR_SHELL( this );

    ASSERT( GetLayout(), "Layout not ready" );

    // Wenn das Layout noch nicht einmal eine Hoehe hat,
    // ist sowieso nichts formatiert.
    // Dann eruebrigt sich die Invalidierung
    // Falsch, z.B. beim Anlegen einer neuen View wird der Inhalt eingefügt
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

    FASTBOOL bBrowseOn = GetDoc()->IsBrowseMode();

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
    BYTE nInv = INV_PRTAREA | INV_TABLE | INV_POS;
    // Beim BrowseModus-Wechsel benoetigen die CntntFrms
    // wg. der Drucker/Bildschirmformatierung eine Size-Invalidierung
    if( bBrowseChgd )
        nInv |= INV_SIZE | INV_DIRECTION;

    GetLayout()->InvalidateAllCntnt( nInv );

    SwFrm::CheckPageDescs( (SwPageFrm*)GetLayout()->Lower() );

    EndAction();
    UnlockPaint();
}

/******************************************************************************
|*
|*  ViewShell::Is/Set[Head|Foot]InBrowse()
|*
|*  Ersterstellung      MA 10. Feb. 97
|*  Letzte Aenderung    MA 10. Feb. 97
|*
******************************************************************************/

BOOL ViewShell::IsHeadInBrowse() const
{
    return GetDoc()->IsHeadInBrowse();
}


void ViewShell::SetHeadInBrowse( BOOL bOn )
{
    if ( GetDoc()->IsHeadInBrowse() != bOn )
    {
        BOOL bCrsr = ISA(SwCrsrShell);
        if ( bCrsr )
            ((SwCrsrShell*)this)->StartAction();
        else
            StartAction();
        GetDoc()->SetHeadInBrowse( bOn );

        SwPageFrm *pPg = GetLayout() ? (SwPageFrm*)GetLayout()->Lower() : 0;
        while ( pPg )
        {
            if ( bCrsr )
                ::MA_ParkCrsr( pPg->GetPageDesc(), *(SwCrsrShell*)this );
            pPg->PrepareHeader();
            pPg = (SwPageFrm*)pPg->GetNext();
        }
        if ( bCrsr )
            ((SwCrsrShell*)this)->EndAction();
        else
            EndAction();
    }
}


BOOL ViewShell::IsFootInBrowse() const
{
    return GetDoc()->IsFootInBrowse();
}


void ViewShell::SetFootInBrowse( BOOL bOn )
{
    if ( GetDoc()->IsFootInBrowse() != bOn )
    {
        BOOL bCrsr = ISA(SwCrsrShell);
        if ( bCrsr )
            ((SwCrsrShell*)this)->StartAction();
        else
            StartAction();
        GetDoc()->SetFootInBrowse( bOn );

        SwPageFrm *pPg = GetLayout() ? (SwPageFrm*)GetLayout()->Lower() : 0;
        while ( pPg )
        {
            if ( bCrsr )
                ::MA_ParkCrsr( pPg->GetPageDesc(), *(SwCrsrShell*)this );
            pPg->PrepareFooter();
            pPg = (SwPageFrm*)pPg->GetNext();
        }
        if ( bCrsr )
            ((SwCrsrShell*)this)->EndAction();
        else
            EndAction();
    }
}


/*************************************************************************
|*
|*    ViewShell::GetLayout()
|*    ViewShell::GetNodes()
|*
|*    Ersterstellung    OK 26. May. 92
|*    Letzte Aenderung  MA 16. Sep. 93
|*
*************************************************************************/

SwRootFrm *ViewShell::GetLayout() const
{
    return GetDoc()->GetRootFrm();
}

SfxPrinter* ViewShell::GetPrt( BOOL bCreate ) const
{
    return GetDoc()->GetPrt( bCreate );
}

VirtualDevice* ViewShell::GetVirDev( BOOL bCreate ) const
{
    return GetDoc()->GetVirDev( bCreate );
}

OutputDevice& ViewShell::GetRefDev() const
{
    OutputDevice* pTmpOut = 0;
    if ( GetWin() && IsBrowseMode() &&
         ! GetViewOptions()->IsPrtFormat() )
        pTmpOut = GetWin();
    else if ( 0 != mpTmpRef )
        pTmpOut = mpTmpRef;
    else
        pTmpOut = &GetDoc()->GetRefDev();

    return *pTmpOut;
}

SwPrintData*    ViewShell::GetPrintData() const
{
    return GetDoc()->GetPrintData();
}
void            ViewShell::SetPrintData(SwPrintData& rPrtData)
{
    GetDoc()->SetPrintData(rPrtData);
}

const SwNodes& ViewShell::GetNodes() const
{
    return pDoc->GetNodes();
}


void ViewShell::DrawSelChanged(SdrView*)
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

/*************************************************************************
|*
|*    ViewShell::SetSubsLines()
|*
|*    Beschreibung      Hilfslinien An-/Abschalten
|*    Ersterstellung    MA 26. May. 92
|*    Letzte Aenderung  MA 03. May. 95
|*
*************************************************************************/

void ViewShell::SetSubsLines()
{
    if( GetWin() )
        GetWin()->Invalidate();
}

/******************************************************************************
|*
|*  ViewShell::ApplyViewOptions(), ImplApplyViewOptions()
|*
|*  Ersterstellung      ??
|*  Letzte Aenderung    MA 03. Mar. 98
|*
******************************************************************************/

void ViewShell::ApplyViewOptions( const SwViewOption &rOpt )
{

    ViewShell *pSh = this;
    do
    {   pSh->StartAction();
        pSh = (ViewShell*)pSh->GetNext();
    } while ( pSh != this );

    ImplApplyViewOptions( rOpt );

    //Einige Aenderungen muessen synchronisiert werden.
    pSh = (ViewShell*)this->GetNext();
    while ( pSh != this )
    {
        SwViewOption aOpt( *pSh->GetViewOptions() );
        aOpt.SetFldName( rOpt.IsFldName() );
        aOpt.SetHidden( rOpt.IsHidden() );
        aOpt.SetShowHiddenPara( rOpt.IsShowHiddenPara() );
        if ( !(aOpt == *pSh->GetViewOptions()) )
            pSh->ImplApplyViewOptions( aOpt );
        pSh = (ViewShell*)pSh->GetNext();
    }

    pSh = this;
    do
    {   pSh->EndAction();
        pSh = (ViewShell*)pSh->GetNext();
    } while ( pSh != this );

}

void ViewShell::ImplApplyViewOptions( const SwViewOption &rOpt )
{
    ASSERT( !(*pOpt == rOpt), "ViewShell::ApplyViewOptions: ");

    Window *pWin = GetWin();
    if( !pWin )
    {
        ASSERT( pWin, "ViewShell::ApplyViewOptions: no window" );
        return;
    }

    SET_CURR_SHELL( this );

    BOOL bReformat   = FALSE;

    if( pOpt->IsHidden() != rOpt.IsHidden() )
    {
        ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
                                            SetHiddenFlag( !rOpt.IsHidden() );
        bReformat = TRUE;
    }
    if ( pOpt->IsShowHiddenPara() != rOpt.IsShowHiddenPara() )
    {
        SwHiddenParaFieldType* pFldType = (SwHiddenParaFieldType*)GetDoc()->
                                          GetSysFldType(RES_HIDDENPARAFLD);
        if( pFldType && pFldType->GetDepends() )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
        }
        bReformat = TRUE;
    }

    // bReformat wird TRUE, wenn ...
    // - Feldnamen anzeigen oder nicht ...
    // ( - SwEndPortion muessen _nicht_ mehr generiert werden. )
    // - Das Window ist natuerlich was ganz anderes als der Drucker...
    bReformat = bReformat || pOpt->IsFldName()   != rOpt.IsFldName();

    // Der Mapmode wird veraendert, Minima/Maxima werden von der UI beachtet
    if( pOpt->GetZoom() != rOpt.GetZoom() && !IsPreView() )
    {
        MapMode aMode( pWin->GetMapMode() );
        Fraction aNewFactor( rOpt.GetZoom(), 100 );
        aMode.SetScaleX( aNewFactor );
        aMode.SetScaleY( aNewFactor );
        pWin->SetMapMode( aMode );
        // Wenn kein ReferenzDevice (Drucker) zum Formatieren benutzt wird,
        // sondern der Bildschirm, muss bei Zoomfaktoraenderung neu formatiert
        // werden.
        if( IsBrowseMode() )
            bReformat = TRUE;
    }

    if ( IsBrowseMode() && pOpt->IsPrtFormat() != rOpt.IsPrtFormat() )
        bReformat = TRUE;

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
        pDView->SetSnapGrid( aFSize );
        Fraction aSnGrWdtX(rSz.Width(), rOpt.GetDivisionX() + 1);
        Fraction aSnGrWdtY(rSz.Height(), rOpt.GetDivisionY() + 1);
        pDView->SetSnapGridWidth( aSnGrWdtX, aSnGrWdtY );

        if ( pOpt->IsDraw() != rOpt.IsDraw() )
        {
            FASTBOOL bDraw = !rOpt.IsDraw();
            pDView->SetLineDraft( bDraw );
            pDView->SetFillDraft( bDraw );
            pDView->SetGrafDraft( bDraw );
            pDView->SetTextDraft( bDraw );
        }
        if ( pOpt->IsSolidMarkHdl() != rOpt.IsSolidMarkHdl() )
            pDView->SetSolidMarkHdl( rOpt.IsSolidMarkHdl() );

            // it's a JOE interface !
        if ( pOpt->IsBigMarkHdl() != rOpt.IsBigMarkHdl() )
            pDView->SetMarkHdlSizePixel(rOpt.IsBigMarkHdl() ? 9 : 7);
    }

    FASTBOOL bOnlineSpellChgd = pOpt->IsOnlineSpell() != rOpt.IsOnlineSpell();

    *pOpt = rOpt;   // Erst jetzt werden die Options uebernommen.
    pOpt->SetUIOptions(rOpt);

    pDoc->SetHTMLMode( 0 != ::GetHtmlMode(pDoc->GetDocShell()) );

    pWin->Invalidate();
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
        BOOL bOnlineSpl = rOpt.IsOnlineSpell();
        while( pSh != this )
        {   pSh->pOpt->SetOnlineSpell( bOnlineSpl );
            Window *pTmpWin = pSh->GetWin();
            if( pTmpWin )
                pTmpWin->Invalidate();
            pSh = (ViewShell*)pSh->GetNext();
        }
    }

}

/******************************************************************************
|*
|*  ViewShell::SetUIOptions()
|*
|*  Ersterstellung      OS 29.07.96
|*  Letzte Aenderung    OS 29.07.96
|*
******************************************************************************/

void ViewShell::SetUIOptions( const SwViewOption &rOpt )
{
    pOpt->SetUIOptions(rOpt);
    //the API-Flag of the view options is set but never reset
    //it is required to set scroll bars in readonly documents
    if(rOpt.IsStarOneSetting())
        pOpt->SetStarOneSetting(TRUE);

    pOpt->SetSymbolFont(rOpt.GetSymbolFont());
}

/******************************************************************************
|*
|*  ViewShell::SetReadonly()
|*
|*  Ersterstellung      OS 05.09.96
|*  Letzte Aenderung    MA 12. Feb. 97
|*
******************************************************************************/

void ViewShell::SetReadonlyOption(BOOL bSet)
{
    //JP 01.02.99: bei ReadOnly Flag richtig abfragen und ggfs. neu
    //              formatieren; Bug 61335

    // Schalten wir gerade von Readonly auf Bearbeiten um?
    if( bSet != pOpt->IsReadonly() )
    {
        // damit die Flags richtig erfragt werden koennen.
        pOpt->SetReadonly( FALSE );

        BOOL bReformat = pOpt->IsFldName();

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
/* -----------------28.08.2003 15:45-----------------

 --------------------------------------------------*/
void  ViewShell::SetPDFExportOption(sal_Bool bSet)
{
    if( bSet != pOpt->IsPDFExport() )
    {
        if(bSet && IsBrowseMode() )
            pOpt->SetPrtFormat( TRUE );
        pOpt->SetPDFExport(bSet);
    }
}
/* -----------------------------2002/07/31 17:06------------------------------

 ---------------------------------------------------------------------------*/
void  ViewShell::SetReadonlySelectionOption(sal_Bool bSet)
{
    if( bSet != pOpt->IsSelectionInReadonly() )
    {
        pOpt->SetSelectionInReadonly(bSet);
    }
}
/******************************************************************************
|*
|*  ViewShell::SetPrtFormatOption()
|*
|*  Ersterstellung      AMA 10. Sep. 97
|*  Letzte Aenderung    AMA 10. Sep. 97
|*
******************************************************************************/

void ViewShell::SetPrtFormatOption( BOOL bSet )
{
    pOpt->SetPrtFormat( bSet );
}

/******************************************************************************
|*
|*  ViewShell::UISizeNotify()
|*
|*  Ersterstellung      MA 14. Jan. 97
|*  Letzte Aenderung    MA 14. Jan. 97
|*
******************************************************************************/


void ViewShell::UISizeNotify()
{
    if ( bDocSizeChgd )
    {
        bDocSizeChgd = FALSE;
        FASTBOOL bOld = bInSizeNotify;
        bInSizeNotify = TRUE;
        ::SizeNotify( this, GetLayout()->Frm().SSize() );
        bInSizeNotify = bOld;
    }
}


BOOL ViewShell::IsBrowseMode() const
{
    return GetDoc()->IsBrowseMode();
}

void    ViewShell::SetRestoreActions(USHORT nSet)
{
    DBG_ASSERT(!GetRestoreActions()||!nSet, "mehrfaches Restore der Actions ?")
    Imp()->SetRestoreActions(nSet);
}
USHORT  ViewShell::GetRestoreActions() const
{
    return Imp()->GetRestoreActions();
}

BOOL ViewShell::IsNewLayout() const
{
    return GetLayout()->IsNewLayout();
}

::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible > ViewShell::CreateAccessible()
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::accessibility;
    Reference< XAccessible > xAcc;

    SwDoc *pDoc = GetDoc();

    // We require a layout and an XModel to be accessible.
    ASSERT( pDoc->GetRootFrm(), "no layout, no access" );
    ASSERT( GetWin(), "no window, no access" );

    if( pDoc->GetRootFrm() && GetWin() )
        xAcc = Imp()->GetAccessibleMap().GetDocumentView();

    return xAcc;
}

::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible >
ViewShell::CreateAccessiblePreview()
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;

    DBG_ASSERT( IsPreView(),
                "Can't create accessible preview for non-preview ViewShell" );

    // We require a layout and an XModel to be accessible.
    ASSERT( pDoc->GetRootFrm(), "no layout, no access" );
    ASSERT( GetWin(), "no window, no access" );

    // OD 15.01.2003 #103492# - add condition <IsPreView()>
    if ( IsPreView() && pDoc->GetRootFrm() && GetWin() )
    {
        // OD 14.01.2003 #103492# - adjustment for new method signature
        return Imp()->GetAccessibleMap().GetDocumentPreview(
                    PagePreviewLayout()->maPrevwPages,
                    GetWin()->GetMapMode().GetScaleX(),
                    pDoc->GetRootFrm()->GetPageByPageNum( PagePreviewLayout()->mnSelectedPageNum ),
                    PagePreviewLayout()->maWinSize );
    }
    return NULL;
}

void ViewShell::InvalidateAccessibleFocus()
{
    if( Imp()->IsAccessible() )
        Imp()->GetAccessibleMap().InvalidateFocus();
}

/* -----------------------------06.05.2002 13:23------------------------------

 ---------------------------------------------------------------------------*/
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

        if(pOpt->IsReadonly())
            pOpt->SetSelectionInReadonly(rAccessibilityOptions.IsSelectionInReadonly());
    }
}
