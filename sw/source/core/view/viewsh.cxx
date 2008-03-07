/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewsh.cxx,v $
 *
 *  $Revision: 1.82 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:00:41 $
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


#define _SVX_PARAITEM_HXX
#define _SVX_TEXTITEM_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>

#include <sfx2/viewfrm.hxx>

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
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
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
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
// OD 14.01.2003 #103492#
#ifndef _PAGEPREVIEWLAYOUT_HXX
#include <pagepreviewlayout.hxx>
#endif
// --> OD 2004-05-24 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif
// <--

#include "../../ui/inc/view.hxx"
#include <PostItMgr.hxx>

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include <vcl/svapp.hxx>

// #i74769#
#ifndef _SDRPAINTWINDOW_HXX
#include <svx/sdrpaintwindow.hxx>
#endif

BOOL ViewShell::bLstAct = FALSE;
ShellResource *ViewShell::pShellRes = 0;
Window *ViewShell::pCareWindow = 0;
BitmapEx* ViewShell::pErrorBmp = NULL;
BitmapEx* ViewShell::pReplaceBmp = NULL;

BOOL bInSizeNotify = FALSE;

DBG_NAME(LayoutIdle)

TYPEINIT0(ViewShell);

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// #i72754# 2nd set of Pre/PostPaints
// This time it uses the lock counter mnPrePostPaintCount to allow only one activation
// and deactivation and mpPrePostOutDev to remember the OutDev from the BeginDrawLayers
// call. That way, all places where paint take place can be handled the same way, even
// when calling other paint methods. This is the case at the places where SW paints
// buffered into VDevs to avoid flicker. Tis is in general problematic and should be
// solved once using the BufferedOutput functionality of the DrawView.

void ViewShell::DLPrePaint2(const Region& rRegion)
{
    if(0L == mnPrePostPaintCount)
    {
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
    }

    mnPrePostPaintCount++;
}

void ViewShell::DLPostPaint2()
{
    OSL_ENSURE(mnPrePostPaintCount > 0L, "ViewShell::DLPostPaint2: Pre/PostPaint encapsulation broken (!)");
    mnPrePostPaintCount--;

    if((0L == mnPrePostPaintCount) && (0 != mpTargetPaintWindow))
    {
        // #i74769# restore buffered OutDev
        if(mpTargetPaintWindow->GetPreRenderDevice())
        {
            pOut = mpBufferedOut;
        }

        // #i74769# use SdrPaintWindow now direct
        Imp()->GetDrawView()->EndDrawLayers(*mpTargetPaintWindow);
        mpTargetPaintWindow = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////

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

    const bool bIsShellForCheckViewLayout = ( this == GetLayout()->GetCurrShell() );

    SET_CURR_SHELL( this );
    if ( Imp()->HasDrawView() && !Imp()->GetDrawView()->areMarkHandlesHidden() )
        Imp()->StartAction();

    if ( Imp()->GetRegion() && Imp()->GetRegion()->GetOrigin() != VisArea() )
        Imp()->DelRegions();

    const BOOL bExtraData = ::IsExtraData( GetDoc() );

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

    if ( bIsShellForCheckViewLayout )
        GetLayout()->CheckViewLayout( GetViewOptions(), &aVisArea );

    //Wenn wir selbst keine Paints erzeugen, so warten wir auf das Paint
    //vom System. Dann ist das Clipping korrekt gesetzt; Beispiel: verschieben
    //eines DrawObjektes.
    if ( Imp()->GetRegion()     || Imp()->GetScrollRects() ||
         aInvalidRect.HasArea() || bExtraData )
    {
        if ( !nLockPaint )
        {
            BOOL bPaintsFromSystem = aInvalidRect.HasArea();
            GetWin()->Update();
            if ( aInvalidRect.HasArea() )
            {
                if ( bPaintsFromSystem )
                    Imp()->AddPaintRect( aInvalidRect );

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
                            pLayout->Paint( aRect );
                            pOld->DrawOutDev( aRect.Pos(), aRect.SSize(),
                                              aRect.Pos(), aRect.SSize(), *pVout );
                            pOut = pOld;

                            // #i72754# end Pre/PostPaint encapsulation when pOut is back and content is painted
                            DLPostPaint2();
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
                        pLayout->Paint( aRect );

                        // #i75172# end DrawingLayer paint
                        DLPostPaint2();
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


    //Damit sich die automatischen Scrollbars auch richtig anordnen k?nnen
    //muessen wir die Aktion hier kuenstlich beenden (EndAction loesst ein
    //Notify aus, und das muss Start-/EndAction rufen um die  Scrollbars
    //klarzubekommen.
    --nStartAction;
    UISizeNotify();
    ++nStartAction;

#ifndef PRODUCT
    // test option 'No Scroll' suppresses the automatic repair of the scrolled area
    if ( !GetViewOptions()->IsTest8() )
#endif
    if ( Imp()->IsScrolled() )
        Imp()->RestartScrollTimer();

    if( Imp()->IsAccessible() )
        Imp()->FireAccessibleEvents();
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
                DLPostPaint2();
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
        // #125243# there are lots of stacktraces indicating that Imp() returns NULL
        // this ViewShell seems to be invalid - but it's not clear why
        // this return is only a workaround!
        DBG_ASSERT(Imp(), "ViewShell already deleted?")
        if(!Imp())
            return;
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

/** local method to invalidate/re-calculate positions of floating screen
    objects (Writer fly frame and drawing objects), which are anchored
    to paragraph or to character.

    OD 2004-03-16 #i11860#

    @author OD
*/
void lcl_InvalidateAllObjPos( ViewShell &_rSh )
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
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::PARA_SPACE_MAX, bNew );
        const BYTE nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

void ViewShell::SetParaSpaceMaxAtPages( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES, bNew );
        const BYTE nInv = INV_PRTAREA | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this,  nInv );
    }
}

void ViewShell::SetTabCompat( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::TAB_COMPAT) != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::TAB_COMPAT, bNew );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

/*-- 29.11.2007 09:03:18---------------------------------------------------
    //#i24363# tab stops relative to indent
  -----------------------------------------------------------------------*/
void ViewShell::SetTabsRelativeToIndent(bool bNew)
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if( pIDSA->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT) != bNew  )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT, bNew );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void ViewShell::SetAddExtLeading( bool bNew )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::ADD_EXT_LEADING) != bNew )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::ADD_EXT_LEADING, bNew );
        SdrModel* pTmpDrawModel = getIDocumentDrawModelAccess()->GetDrawModel();
        if ( pTmpDrawModel )
            pTmpDrawModel->SetAddExtLeading( bNew );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

void ViewShell::SetUseVirDev( bool bNewVirtual )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) != bNewVirtual )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        // this sets the flag at the document and calls PrtDataChanged
        IDocumentDeviceAccess* pIDDA = getIDocumentDeviceAccess();
        pIDDA->setReferenceDeviceType( bNewVirtual, true );
    }
}

// OD 2004-02-16 #106629# - control, if paragraph and table spacing is added
// at bottom of table cells
void ViewShell::SetAddParaSpacingToTableCells( bool _bAddParaSpacingToTableCells )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS) != _bAddParaSpacingToTableCells )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS, _bAddParaSpacingToTableCells );
        const BYTE nInv = INV_PRTAREA;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

// OD 06.01.2004 #i11859# - control, if former formatting of text lines with
// proportional line spacing is used or not.
void ViewShell::SetUseFormerLineSpacing( bool _bUseFormerLineSpacing )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::OLD_LINE_SPACING) != _bUseFormerLineSpacing )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::OLD_LINE_SPACING, _bUseFormerLineSpacing );
        const BYTE nInv = INV_PRTAREA;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}

// OD 2004-03-12 #i11860# - control, if former object positioning is used or not.
void ViewShell::SetUseFormerObjectPositioning( bool _bUseFormerObjPos )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_OBJECT_POS) != _bUseFormerObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::USE_FORMER_OBJECT_POS, _bUseFormerObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}

// OD 2004-05-05 #i28701#
void ViewShell::SetConsiderWrapOnObjPos( bool _bConsiderWrapOnObjPos )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION) != _bConsiderWrapOnObjPos )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::CONSIDER_WRAP_ON_OBJECT_POSITION, _bConsiderWrapOnObjPos );
        lcl_InvalidateAllObjPos( *this );
    }
}

// --> FME #108724#
void ViewShell::SetUseFormerTextWrapping( bool _bUseFormerTextWrapping )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING) != _bUseFormerTextWrapping )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::USE_FORMER_TEXT_WRAPPING, _bUseFormerTextWrapping );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}
// <--

// -> PB 2007-06-11 #i45491#
void ViewShell::SetDoNotJustifyLinesWithManualBreak( bool _bDoNotJustifyLinesWithManualBreak )
{
    IDocumentSettingAccess* pIDSA = getIDocumentSettingAccess();
    if ( pIDSA->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK) != _bDoNotJustifyLinesWithManualBreak )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), TRUE );
        pIDSA->set(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK, _bDoNotJustifyLinesWithManualBreak );
        const BYTE nInv = INV_PRTAREA | INV_SIZE | INV_TABLE | INV_SECTION;
        lcl_InvalidateAllCntnt( *this, nInv );
    }
}
// <--

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
        GetLayout()->InvalidateAllCntnt( INV_SIZE | INV_POS | INV_PRTAREA );
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
        GetDoc()->UpdateExpFlds(NULL, true);

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

void ViewShell::SizeChgNotify()
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
        ::SizeNotify( this, GetDocSize() );
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

    //SwSaveHdl aSaveHdl( Imp() );

    bool bScrolled = false;

    SwPostItMgr* pPostItMgr = GetPostItMgr();

    if ( bFull )
        GetWin()->Invalidate();
    else
    {
        // Betrag ausrechnen, um den gescrolled werden muss.
        const long nXDiff = aPrevArea.Left() - VisArea().Left();
        const long nYDiff = aPrevArea.Top()  - VisArea().Top();

        if( !nXDiff && !getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
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
            SwTwips nMinLeft = LONG_MAX;
            SwTwips nMaxRight= 0;

            const SwTwips nSidebarWidth = pPostItMgr && pPostItMgr->ShowNotes() && pPostItMgr->HasNotes() ?
                                          pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth() :
                                          0;
            const bool bBookMode = GetViewOptions()->IsViewLayoutBookMode();

            while ( pPage && pPage->Frm().Top() <= nBottom )
            {
                SwRect aPageRect( pPage->Frm() );
                if ( bBookMode )
                {
                    const SwPageFrm& rFormatPage = static_cast<const SwPageFrm*>(pPage)->GetFormatPage();
                    aPageRect.SSize() = rFormatPage.Frm().SSize();
                }

                if ( aPageRect.IsOver( aBoth ) )
                {
                    // OD 12.02.2003 #i9719#, #105645# - consider new border
                    // and shadow width
                    const SwTwips nBorderWidth =
                            GetOut()->PixelToLogic( Size( pPage->BorderPxWidth(), 0 ) ).Width();
                    const SwTwips nShadowWidth =
                            GetOut()->PixelToLogic( Size( pPage->ShadowPxWidth(), 0 ) ).Width();

                    SwTwips nPageLeft = 0;
                    SwTwips nPageRight = 0;
                    if (pPage->MarginSide())
                    {
                        nPageLeft =  aPageRect.Left() - nBorderWidth - nSidebarWidth;
                        nPageRight = aPageRect.Right() + nBorderWidth + nShadowWidth;
                    }
                    else
                    {
                        // OD 03.03.2003 #107927# - use correct datatype
                        nPageLeft =  aPageRect.Left() - nBorderWidth;
                        nPageRight = aPageRect.Right() + nBorderWidth + nShadowWidth + nSidebarWidth;
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
                        for ( USHORT i = 0;
                              i < pPage->GetSortedObjs()->Count(); ++i )
                        {
                            SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
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
    Imp()->bPaintInScroll = TRUE;
    GetWin()->Update();
    Imp()->bPaintInScroll = FALSE;

    if ( !bScrolled && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        pPostItMgr->CorrectPositions();

    if( Imp()->IsAccessible() )
        Imp()->UpdateAccessible();

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

    // #i75172# isolated static conditions
    const bool bOnlyYScroll(!lXDiff && Abs(lYDiff) != 0 && Abs(lYDiff) < lMax);
    const bool bAllowedWithChildWindows(GetWin()->GetWindowClipRegionPixel(WINDOW_GETCLIPREGION_NOCHILDREN|WINDOW_GETCLIPREGION_NULL).IsNull());
    const bool bSmoothScrollAllowed(bOnlyYScroll && bEnableSmooth && GetViewOptions()->IsSmoothScroll() &&  bAllowedWithChildWindows);
    const bool bIAmCursorShell(ISA(SwCrsrShell));
    (void) bIAmCursorShell;

    // #i75172# with selection on overlay, smooth scroll should be allowed with it
    const bool bAllowedForSelection(true || (bIAmCursorShell && !((SwCrsrShell*)this)->HasSelection()));

    // #i75172# with cursors on overlay, smooth scroll should be allowed with it
    const bool bAllowedForMultipleCursors(true || (bIAmCursorShell && ((SwCrsrShell*)this)->GetCrsrCnt() < 2));

    if(bSmoothScrollAllowed  && bAllowedForSelection && bAllowedForMultipleCursors)
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
                ViewShell::bLstAct = TRUE;
                GetLayout()->Paint( aRect );
                ViewShell::bLstAct = FALSE;

                // end paint and destroy ObjectContact again
                DLPostPaint2();
                pDrawView->DeleteWindowFromPaintView(pVout);

                // temporary debug paint checking...
                static bool bDoSaveForVisualControl(false);
                if(bDoSaveForVisualControl)
                {
                    const bool bMapModeWasEnabledVDev(pVout->IsMapModeEnabled());
                    pVout->EnableMapMode(false);
                    const Bitmap aBitmap(pVout->GetBitmap(Point(), pVout->GetOutputSizePixel()));
                    const String aTmpString(ByteString( "c:\\test.bmp" ), RTL_TEXTENCODING_UTF8);
                    SvFileStream aNew(aTmpString, STREAM_WRITE|STREAM_TRUNC);
                    aNew << aBitmap;
                    pVout->EnableMapMode(bMapModeWasEnabledVDev);
                }
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

                    Imp()->bSmoothUpdate = TRUE;
                    GetWin()->Update();
                    Imp()->bSmoothUpdate = FALSE;

                    if(!Imp()->bStopSmooth)
                    {
                        static bool bDoItOnPixels(true);
                        if(bDoItOnPixels)
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

                            // copy content
                            static bool bTestDirectToWindowPaint(false);
                            if(bTestDirectToWindowPaint)
                            {
                                const bool bMapModeWasEnabledWin(GetWin()->IsMapModeEnabled());
                                GetWin()->EnableMapMode(false);

                                GetWin()->DrawOutDev(
                                    aTargetPixel.TopLeft(), aTargetPixel.GetSize(), // dest
                                    aSourceTopLeft, aTargetPixel.GetSize(), // source
                                    *pVout);

                                GetWin()->EnableMapMode(bMapModeWasEnabledWin);
                            }

                            rTargetDevice.DrawOutDev(
                                aTargetPixel.TopLeft(), aTargetPixel.GetSize(), // dest
                                aSourceTopLeft, aTargetPixel.GetSize(), // source
                                *pVout);

                            // restore MapModes
                            rTargetDevice.EnableMapMode(bMapModeWasEnabledDest);
                            pVout->EnableMapMode(bMapModeWasEnabledSource);

                            // end paint on logoc base
                            DLPostPaint2();
                        }
                        else
                        {
                            Rectangle aRectangle(Imp()->aSmoothRect.SVRect());
                            aRectangle.Left() -= aPixSz.Width();
                            aRectangle.Right() += aPixSz.Width();
                            aRectangle.Top() -= aPixSz.Height();
                            aRectangle.Bottom() += aPixSz.Height();
                            const Point aUpdateTopLeft(aRectangle.TopLeft());
                            const Size aUpdateSize(aRectangle.GetSize());

                            // #i75172# the part getting visible needs to be handled like a repaint.
                            // For that, start with DLPrePaint2 and the correct Rectangle
                            DLPrePaint2(Region(aRectangle));

                            static bool bTestDirectToWindowPaint(false);
                            if(bTestDirectToWindowPaint)
                            {
                                GetWin()->DrawOutDev(aUpdateTopLeft, aUpdateSize, aUpdateTopLeft, aUpdateSize, *pVout);
                            }

                            mpTargetPaintWindow->GetTargetOutputDevice().DrawOutDev(aUpdateTopLeft, aUpdateSize, aUpdateTopLeft, aUpdateSize, *pVout);

                            // #i75172# Corret repaint end
                            // Note: This also correcty creates the overlay, thus smooth scroll will
                            // also be allowed now wth selection (see big IF above)
                            DLPostPaint2();
                        }
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

    aVisArea.Pos().X() -= lXDiff;
    aVisArea.Pos().Y() -= lYDiff;
    if ( pRect )
        GetWin()->Scroll( lXDiff, lYDiff, *pRect, SCROLL_CHILDREN);
    else
        GetWin()->Scroll( lXDiff, lYDiff, SCROLL_CHILDREN);
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
    BOOL bBorderOnly = FALSE;
    const SwRootFrm *pRoot = GetDoc()->GetRootFrm();
    if ( rRect.Top() > pRoot->Frm().Bottom() )
    {
        const SwFrm *pPg = pRoot->Lower();
        while ( pPg && pPg->GetNext() )
            pPg = pPg->GetNext();
        if ( !pPg || !pPg->Frm().IsOver( VisArea() ) )
            bBorderOnly = TRUE;
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
        const SwFrm *pPage = pRoot->Lower();
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
        //const SwTwips nRight  = rRect.Right();
        while ( pPage && aRegion.Count() &&
                (pPage->Frm().Top() <= nBottom) ) // PAGES01 && (pPage->Frm().Left() <= nRight))
        {
            SwRect aPageRect( pPage->Frm() );
            if ( bBookMode )
            {
                const SwPageFrm& rFormatPage = static_cast<const SwPageFrm*>(pPage)->GetFormatPage();
                aPageRect.SSize() = rFormatPage.Frm().SSize();
            }

            const bool bSidebarRight = !static_cast<const SwPageFrm*>(pPage)->MarginSide();
            aPageRect.Pos().X() -= bSidebarRight ? 0 : nSidebarWidth;
            aPageRect.SSize().Width() += nSidebarWidth;

            if ( aPageRect.IsOver( rRect ) )
                aRegion -= aPageRect;

            pPage = pPage->GetNext();
        }
    }
    if ( aRegion.Count() )
        _PaintDesktop( aRegion );
}


// PaintDesktop gesplittet, dieser Teil wird auch von PreViewPage benutzt
void ViewShell::_PaintDesktop( const SwRegionRects &rRegion )
{
    // OD 2004-04-23 #116347#
    GetOut()->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
    GetOut()->SetLineColor();
    // OD 14.02.2003 #107424# - no longer needed, because color configuration
    // is loaded in constructor of <SwModule>.
    /*
    //make sure the color configuration has been loaded
    SW_MOD()->GetColorConfig();
    */

    for ( USHORT i = 0; i < rRegion.Count(); ++i )
    {
        const Rectangle aRectangle(rRegion[i].SVRect());

        // #i68597# inform Drawinglayer about display change
        DLPrePaint2(Region(aRectangle));

        // #i75172# needed to move line/Fill color setters into loop since DLPrePaint2
        // may exchange GetOut(), that's it's purpose. This happens e.g. at print preview.
        GetOut()->SetFillColor( SwViewOption::GetAppBackgroundColor());
        GetOut()->SetLineColor();
        GetOut()->DrawRect(aRectangle);

        DLPostPaint2();
    }

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
                //SwSaveHdl *pSaveHdl = 0;
                //if ( Imp()->HasDrawView() )
                //  pSaveHdl = new SwSaveHdl( Imp() );

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

                //delete pSaveHdl;
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
            // #i68597#
            const Region aDLRegion(rRect);
            DLPrePaint2(aDLRegion);

            // OD 2004-04-23 #116347#
            pOut->Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
            pOut->SetFillColor( Imp()->GetRetoucheColor() );
            pOut->SetLineColor();
            pOut->DrawRect( rRect );
            pOut->Pop();

            // #i68597#
            DLPostPaint2();
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
    if( rNew != aBrowseBorder )
    {
        aBrowseBorder = rNew;
        if ( aVisArea.HasArea() )
            CheckBrowseView( FALSE );
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

/******************************************************************************
|*
|*  ViewShell::CheckBrowseView()
|*
|*  Ersterstellung      MA 04. Mar. 96
|*  Letzte Aenderung    MA 04. Jul. 96
|*
******************************************************************************/

void ViewShell::CheckBrowseView( BOOL bBrowseChgd )
{
    if ( !bBrowseChgd &&
         !getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
        return;

    SET_CURR_SHELL( this );

    ASSERT( GetLayout(), "Layout not ready" );

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

OutputDevice& ViewShell::GetRefDev() const
{
    OutputDevice* pTmpOut = 0;
    if (  GetWin() &&
          getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
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

    //mod #i6193# added sidebar width
    // fme: Sidebar is already part of the root frame
    /*SwView* pView = pDoc->GetDocShell()->GetView() ;
    if (pView)
    {
        SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
        if (pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes())
        {
            aSz.Width() += pPostItMgr->GetSidebarWidth() + pPostItMgr->GetSidebarBorderWidth();
        }
    }*/

    return aSz;
}


SfxItemPool& ViewShell::GetAttrPool()
{
    return GetDoc()->GetAttrPool();
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
        aOpt.SetShowHiddenField( rOpt.IsShowHiddenField() );
        aOpt.SetShowHiddenPara( rOpt.IsShowHiddenPara() );
        aOpt.SetShowHiddenChar( rOpt.IsShowHiddenChar() );
        aOpt.SetViewLayoutBookMode( rOpt.IsViewLayoutBookMode() );
        aOpt.SetViewLayoutColumns( rOpt.GetViewLayoutColumns() );

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

    Window *pMyWin = GetWin();
    if( !pMyWin )
    {
        ASSERT( pMyWin, "ViewShell::ApplyViewOptions: no window" );
        return;
    }

    SET_CURR_SHELL( this );

    BOOL bReformat   = FALSE;

    if( pOpt->IsShowHiddenField() != rOpt.IsShowHiddenField() )
    {
        ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
                                            SetHiddenFlag( !rOpt.IsShowHiddenField() );
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
    if ( !bReformat && pOpt->IsShowHiddenChar() != rOpt.IsShowHiddenChar() )
    {
        bReformat = GetDoc()->ContainsHiddenChars();
    }

    // bReformat wird TRUE, wenn ...
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
        if( getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
            bReformat = TRUE;
    }

    if ( getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
         pOpt->IsPrtFormat() != rOpt.IsPrtFormat() )
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
        Fraction aSnGrWdtX(rSz.Width(), rOpt.GetDivisionX() + 1);
        Fraction aSnGrWdtY(rSz.Height(), rOpt.GetDivisionY() + 1);
        pDView->SetSnapGridWidth( aSnGrWdtX, aSnGrWdtY );

        if ( pOpt->IsSolidMarkHdl() != rOpt.IsSolidMarkHdl() )
            pDView->SetSolidMarkHdl( rOpt.IsSolidMarkHdl() );

            // it's a JOE interface !
        if ( pOpt->IsBigMarkHdl() != rOpt.IsBigMarkHdl() )
            pDView->SetMarkHdlSizePixel(rOpt.IsBigMarkHdl() ? 9 : 7);
    }

    BOOL bOnlineSpellChgd = pOpt->IsOnlineSpell() != rOpt.IsOnlineSpell();

    *pOpt = rOpt;   // Erst jetzt werden die Options uebernommen.
    pOpt->SetUIOptions(rOpt);

    pDoc->set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode(pDoc->GetDocShell()));

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
        if( bSet &&
            getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
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
        BOOL bOld = bInSizeNotify;
        bInSizeNotify = TRUE;
        ::SizeNotify( this, GetDocSize() );
        bInSizeNotify = bOld;
    }
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

uno::Reference< ::com::sun::star::accessibility::XAccessible > ViewShell::CreateAccessible()
{
    uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc;

    SwDoc *pMyDoc = GetDoc();

    // We require a layout and an XModel to be accessible.
    ASSERT( pMyDoc->GetRootFrm(), "no layout, no access" );
    ASSERT( GetWin(), "no window, no access" );

    if( pMyDoc->GetRootFrm() && GetWin() )
        xAcc = Imp()->GetAccessibleMap().GetDocumentView();

    return xAcc;
}

uno::Reference< ::com::sun::star::accessibility::XAccessible >
ViewShell::CreateAccessiblePreview()
{
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

/** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

    OD 2005-12-01 #i27138#

    @author OD
*/
void ViewShell::InvalidateAccessibleParaFlowRelation( const SwTxtFrm* _pFromTxtFrm,
                                                      const SwTxtFrm* _pToTxtFrm )
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaFlowRelation( _pFromTxtFrm, _pToTxtFrm );
    }
}

/** invalidate text selection for paragraphs

    OD 2005-12-12 #i27301#

    @author OD
*/
void ViewShell::InvalidateAccessibleParaTextSelection()
{
    if ( GetLayout() && GetLayout()->IsAnyShellAccessible() )
    {
        Imp()->_InvalidateAccessibleParaTextSelection();
    }
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

        // --> FME 2004-06-29 #114856# Formular view
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


// --> FME 2004-06-15 #i12836# enhanced pdf export
sal_Int32 ViewShell::GetPageNumAndSetOffsetForPDF( OutputDevice& rOut, const SwRect& rRect ) const
{
    ASSERT( GetLayout(), "GetPageNumAndSetOffsetForPDF assumes presence of layout" )

    sal_Int32 nRet = -1;

    // --> FME 2005-01-07 #i40059# Position out of bounds:
    SwRect aRect( rRect );
    aRect.Pos().X() = Max( aRect.Left(), GetLayout()->Frm().Left() );
    // <--

    const SwPageFrm* pPage = GetLayout()->GetPageAtPos( aRect.Center() );
    if ( pPage )
    {
        ASSERT( pPage, "GetPageNumAndSetOffsetForPDF: No page found" )

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
// <--

// --> PB 2007-05-30 #146850#
const BitmapEx& ViewShell::GetReplacementBitmap( bool bIsErrorState )
{
    BitmapEx** ppRet;
    USHORT nResId = 0, nHCResId = 0;
    if( bIsErrorState )
    {
        ppRet = &pErrorBmp;
        nResId = RID_GRAPHIC_ERRORBMP;
        nHCResId = RID_GRAPHIC_ERRORBMP_HC;
    }
    else
    {
        ppRet = &pReplaceBmp;
        nResId = RID_GRAPHIC_REPLACEBMP;
        nHCResId = RID_GRAPHIC_REPLACEBMP_HC;
    }

    if( !*ppRet )
    {
        USHORT nBmpResId =
            Application::GetSettings().GetStyleSettings().GetWindowColor().IsDark()
                ? nHCResId : nResId;
        *ppRet = new BitmapEx( SW_RES( nBmpResId ) );
    }
    return **ppRet;
}

void ViewShell::DeleteReplacementBitmaps()
{
    DELETEZ( pErrorBmp );
    DELETEZ( pReplaceBmp );
}
// <--

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
const IDocumentBookmarkAccess* ViewShell::getIDocumentBookmarkAccess() const { return pDoc; }
IDocumentBookmarkAccess* ViewShell::getIDocumentBookmarkAccess() { return pDoc; }
const IDocumentDrawModelAccess* ViewShell::getIDocumentDrawModelAccess() const { return pDoc; }
IDocumentDrawModelAccess* ViewShell::getIDocumentDrawModelAccess() { return pDoc; }
const IDocumentRedlineAccess* ViewShell::getIDocumentRedlineAccess() const { return pDoc; }
IDocumentRedlineAccess* ViewShell::getIDocumentRedlineAccess() { return pDoc; }
const IDocumentLayoutAccess* ViewShell::getIDocumentLayoutAccess() const { return pDoc; }
IDocumentLayoutAccess* ViewShell::getIDocumentLayoutAccess() { return pDoc; }
const IDocumentFieldsAccess* ViewShell::getIDocumentFieldsAccess() const { return pDoc; }
IDocumentContentOperations* ViewShell::getIDocumentContentOperations() { return pDoc; }
IDocumentStylePoolAccess* ViewShell::getIDocumentStylePoolAccess() { return pDoc; }
const IDocumentStatistics* ViewShell::getIDocumentStatistics() const { return pDoc; }
IDocumentUndoRedo* ViewShell::getIDocumentUndoRedoAccess() { return pDoc; }
// --> OD 2007-11-14 #i83479#
const IDocumentListItems* ViewShell::getIDocumentListItemsAccess() const
{
    return pDoc;
}
const IDocumentOutlineNodes* ViewShell::getIDocumentOutlineNodesAccess() const
{
    return pDoc;
}
// <--

