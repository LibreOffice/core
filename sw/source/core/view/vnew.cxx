/*************************************************************************
 *
 *  $RCSfile: vnew.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:41:39 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>       // Zugriff auf TxtCache
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>        // SdrView
#endif
#ifndef _CALBCK_HXX
#include <calbck.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _ACCESSIBILITYOPTIONS_HXX
#include <accessibilityoptions.hxx>
#endif

/*************************************************************************
|*
|*  ViewShell::Init()
|*
|*  Letzte Aenderung    MA 14. Jun. 96
|*
|*************************************************************************/

void ViewShell::Init( const SwViewOption *pNewOpt )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::Init" );

    bDocSizeChgd = FALSE;

    // Wir gehen auf Nummer sicher:
    // Wir muessen die alten Fontinformationen wegschmeissen,
    // wenn die Druckeraufloesung oder der Zoomfaktor sich aendert.
    // Init() und Reformat() sind die sichersten Stellen.
    pFntCache->Flush( );

    // ViewOptions werden dynamisch angelegt
    if( !pOpt )
    {
        pOpt = new SwViewOption;

        // Ein ApplyViewOptions braucht nicht gerufen zu werden
        if( pNewOpt )
        {
            *pOpt = *pNewOpt;
            // Der Zoomfaktor muss eingestellt werden, weil in der CTOR-
            // phase aus Performancegruenden kein ApplyViewOptions gerufen wird.
            if( GetWin() && 100 != pOpt->GetZoom() )
            {
                MapMode aMode( pWin->GetMapMode() );
                const Fraction aNewFactor( pOpt->GetZoom(), 100 );
                aMode.SetScaleX( aNewFactor );
                aMode.SetScaleY( aNewFactor );
                pWin->SetMapMode( aMode );
            }
        }
    }

    SwDocShell* pDShell = pDoc->GetDocShell();
    pDoc->SetHTMLMode( 0 != ::GetHtmlMode( pDShell ) );

    // JP 02.02.99: Bug 61335 - Readonly-Flag an den ViewOptions setzen,
    //              bevor das Layout angelegt wird. Ansonsten muesste man
    //              nochmals durchformatieren!!
    if( pDShell && pDShell->IsReadOnly() )
        pOpt->SetReadonly( TRUE );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - before InitPrt" );
    if( GetPrt( TRUE ^ pDoc->IsBrowseMode() ) )
        InitPrt( GetPrt() );
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - after InitPrt" );

    if( GetWin() )
    {
        pOpt->Init( GetWin() );
        GetWin()->SetFillColor();
        GetWin()->SetBackground();
        GetWin()->SetLineColor();
    }

    //Layout erzeugen wenn es noch nicht vorhanden ist.
    SwRootFrm* pRoot = GetDoc()->GetRootFrm();
    if( !pRoot )
        GetDoc()->SetRootFrm( pRoot = new SwRootFrm( pDoc->GetDfltFrmFmt(), this ) );

    SizeChgNotify( pRoot->Frm().SSize() );
}

/*************************************************************************
|*
|*  ViewShell::ViewShell()  CTor fuer die erste Shell.
|*
|*  Letzte Aenderung    MA 29. Aug. 95
|*
|*************************************************************************/

ViewShell::ViewShell( SwDoc& rDocument, Window *pWindow,
                        const SwViewOption *pNewOpt, OutputDevice *pOutput,
                        long nFlags )
    : pDoc( &rDocument ),
    pOpt( 0 ),
    pAccOptions( new SwAccessibilityOptions ),
    pWin( pWindow ),
    pOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rDocument.GetPrt(TRUE)),
    pRef( 0 ),
    nStartAction( 0 ),
    nLockPaint( 0 ),
    pSfxViewShell( 0 ),
    pImp( new SwViewImp( this ) ),
    aBrowseBorder()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::SwViewShell" );

    bPaintInProgress = bViewLocked = bInEndAction = bFrameView =
    bEndActionByVirDev = FALSE;
    bPaintWorks = bEnableSmooth = TRUE;
    bPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    if ( bPreView )
    {
        // OD 12.12.2002 #103492# - init page preview layout
        pImp->InitPagePreviewLayout();
    }

    pDoc->AddLink();
    pOutput = pOut;
    Init( pNewOpt );    //verstellt ggf. das Outdev (InitPrt())
    pOut = pOutput;

    SET_CURR_SHELL( this );

    ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !pOpt->IsHidden() );

    //In Init wird ein Standard-FrmFmt angelegt.
    if( !pDoc->IsUndoNoResetModified() )
        pDoc->ResetModified();

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( GetDoc()->GetDrawModel() || pOpt->IsGridVisible() )
        Imp()->MakeDrawView();
}

/*************************************************************************
|*
|*  ViewShell::ViewShell()  CTor fuer weitere Shells auf ein Dokument.
|*
|*  Letzte Aenderung    MA 29. Aug. 95
|*
|*************************************************************************/

ViewShell::ViewShell( ViewShell& rShell, Window *pWindow,
                        OutputDevice *pOutput, long nFlags ) :
    Ring( &rShell ),
    pDoc( rShell.GetDoc() ),
    pWin( pWindow ),
    pOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rShell.GetDoc()->GetPrt(TRUE)),
    pRef( 0 ),
    pOpt( 0 ),
    pAccOptions( new SwAccessibilityOptions ),
    nStartAction( 0 ),
    nLockPaint( 0 ),
    pSfxViewShell( 0 ),
    pImp( new SwViewImp( this ) ),
    aBrowseBorder( rShell.GetBrowseBorder() )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::SwViewShell" );
    bPaintWorks = bEnableSmooth = TRUE;
    bPaintInProgress = bViewLocked = bInEndAction = bFrameView =
    bEndActionByVirDev = FALSE;
    bPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    // OD 12.12.2002 #103492#
    if ( bPreView )
        pImp->InitPagePreviewLayout();

    SET_CURR_SHELL( this );

    pDoc->AddLink();
    BOOL bModified = pDoc->IsModified();

    pOutput = pOut;
    Init( rShell.GetViewOptions() );    //verstellt ggf. das Outdev (InitPrt())
    pOut = pOutput;

    ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !pOpt->IsHidden() );

    // in Init wird ein Standard-FrmFmt angelegt
    if( !bModified && !pDoc->IsUndoNoResetModified() )
        pDoc->ResetModified();

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( GetDoc()->GetDrawModel() || pOpt->IsGridVisible() )
        Imp()->MakeDrawView();
}

/******************************************************************************
|*
|*  ViewShell::~ViewShell()
|*
|*  Ersterstellung      MA ??
|*  Letzte Aenderung    MA 10. May. 95
|*
******************************************************************************/

ViewShell::~ViewShell()
{
    {
        SET_CURR_SHELL( this );
        bPaintWorks = FALSE;

        //Die Animierten Grafiken abschalten!
        if( pDoc )
        {
            SwNodes& rNds = pDoc->GetNodes();
            SwGrfNode *pGNd;

            SwStartNode *pStNd;
            SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
            while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                aIdx++;
                if ( 0 != ( pGNd = aIdx.GetNode().GetGrfNode() ) )
                {
                    if( pGNd->IsAnimated() )
                    {
                        SwClientIter aIter( *pGNd );
                        for( SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
                            pFrm; pFrm = (SwFrm*)aIter.Next() )
                        {
                            ASSERT( pFrm->IsNoTxtFrm(), "GraphicNode with Text?" );
                            ((SwNoTxtFrm*)pFrm)->StopAnimation( pOut );
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
            }

            GetDoc()->StopNumRuleAnimations( pOut );
        }

        delete pImp; //Erst loeschen, damit die LayoutViews vernichtet werden.
        pImp = 0;   // Set to zero, because ~SwFrm relies on it.

        if ( pDoc )
        {
            if( !pDoc->RemoveLink() )
                delete pDoc, pDoc = 0;
            else
                pDoc->GetRootFrm()->ResetNewLayout();
        }

        delete pOpt;

        //Format-Cache zurueckschrauben.
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

        //Ggf. aus der PaintQueue entfernen lassen
        SwPaintQueue::Remove( this );

        ASSERT( !nStartAction, "EndAction() pending." );
    }

    if ( pDoc )
        GetLayout()->DeRegisterShell( this );

    delete pRef;
    delete pAccOptions;
}

const BOOL ViewShell::HasDrawView() const
{
    return Imp()->HasDrawView();
}

void ViewShell::MakeDrawView()
{
    Imp()->MakeDrawView( );
}

SdrView* ViewShell::GetDrawView()
{
    return Imp()->GetDrawView();
}

SdrView* ViewShell::GetDrawViewWithValidMarkList()
{
    SwDrawView* pDView = Imp()->GetDrawView();
    pDView->ValidateMarkList();
    return pDView;
}
