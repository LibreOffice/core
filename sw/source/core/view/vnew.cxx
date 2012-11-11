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

#include <sfx2/printer.hxx>
#include <rtl/logfile.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <txtfrm.hxx>       // Zugriff auf TxtCache
#include <notxtfrm.hxx>
#include <fntcache.hxx>
#include <docufld.hxx>
#include <ptqueue.hxx>
#include <dview.hxx>        // SdrView
#include <ndgrf.hxx>
#include <ndindex.hxx>
#include <accessibilityoptions.hxx>
#include <switerator.hxx>


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

    bDocSizeChgd = sal_False;

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
    pDoc->set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
    // JP 02.02.99: Bug 61335 - Readonly-Flag an den ViewOptions setzen,
    //              bevor das Layout angelegt wird. Ansonsten muesste man
    //              nochmals durchformatieren!!

    if( pDShell && pDShell->IsReadOnly() )
        pOpt->SetReadonly( sal_True );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - before InitPrt" );

    // --> FME 2007-11-06 #i82967#
    OutputDevice* pPDFOut = 0;

    if ( pOut && pOut->GetPDFWriter() )
        pPDFOut = pOut;
    // <--

    // --> FME 2005-01-21 #i41075#
    // Only setup the printer if we need one:
    const bool bBrowseMode = pOpt->getBrowseMode();
    if( pPDFOut )
        InitPrt( pPDFOut );
    // <--

    // --> FME 2005-03-16 #i44963# Good occasion to check if page sizes in
    // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
    if ( !bBrowseMode )
    {
        pDoc->CheckDefaultPageFmt();
    }
    // <--

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - after InitPrt" );

    if( GetWin() )
    {
        pOpt->Init( GetWin() );
        GetWin()->SetFillColor();
        GetWin()->SetBackground();
        GetWin()->SetLineColor();
    }

    // Create a new layout, if there is no one available
    if( !pLayout )
    {
        // Here's the code which disables the usage of "multiple" layouts at the moment
        // If the problems with controls and groups objects are solved,
        // this code can be removed...
        ViewShell *pCurrShell = GetDoc()->GetCurrentViewShell();
        if( pCurrShell )
            pLayout = pCurrShell->pLayout;
        // end of "disable multiple layouts"
        if( !pLayout )
        {
            // switched to two step construction because creating the layout in SwRootFrm needs a valid pLayout set
            pLayout = SwRootFrmPtr(new SwRootFrm( pDoc->GetDfltFrmFmt(), this ));//swmod081016
            pLayout->Init( pDoc->GetDfltFrmFmt() );
        }
    }
    SizeChgNotify();    //swmod 071108

    // --> #i31958#
    // XForms mode: initialize XForms mode, based on design mode (draw view)
    //   MakeDrawView() requires layout
    if( GetDoc()->isXForms() )
    {
        if( ! HasDrawView() )
            MakeDrawView();
        pOpt->SetFormView( ! GetDrawView()->IsDesignMode() );
    }
    // <-- #i31958#
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
    :
    aBrowseBorder(),
    pSfxViewShell( 0 ),
    pImp( new SwViewImp( this ) ),
    pWin( pWindow ),
    pOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rDocument.getPrinter( true )),
    mpTmpRef( 0 ),
    pOpt( 0 ),
    pAccOptions( new SwAccessibilityOptions ),
    bShowHeaderSeparator( false ),
    bShowFooterSeparator( false ),
    bHeaderFooterEdit( false ),
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
    pDoc( &rDocument ),
    nStartAction( 0 ),
    nLockPaint( 0 ),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::SwViewShell" );

    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during contruction of <ViewShell> instance
    mbInConstructor = true;

    bPaintInProgress = bViewLocked = bInEndAction = bFrameView =
    bEndActionByVirDev = sal_False;
    bPaintWorks = bEnableSmooth = sal_True;
    bPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    // --> OD 2005-02-11 #i38810# - Do not reset modified state of document,
    // if it's already been modified.
    const bool bIsDocModified( pDoc->IsModified() );
    pDoc->acquire();
    pOutput = pOut;
    Init( pNewOpt );    //verstellt ggf. das Outdev (InitPrt())
    pOut = pOutput;

    // OD 28.03.2003 #108470# - initialize print preview layout after layout
    // is created in <ViewShell::Init(..)> - called above.
    if ( bPreView )
    {
        // OD 12.12.2002 #103492# - init page preview layout
        pImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

    ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !pOpt->IsShowHiddenField() );

    //In Init wird ein Standard-FrmFmt angelegt.
    // --> OD 2005-02-11 #i38810#
    if (   !pDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
        && !bIsDocModified )
    // <--
    {
        pDoc->ResetModified();
    }

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( pOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    // OD 2004-06-01 #i26791#
    mbInConstructor = false;
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
    aBrowseBorder( rShell.aBrowseBorder ),
    pSfxViewShell( 0 ),
    pImp( new SwViewImp( this ) ),
    pWin( pWindow ),
    pOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rShell.GetDoc()->getPrinter( true )),
    mpTmpRef( 0 ),
    pOpt( 0 ),
    pAccOptions( new SwAccessibilityOptions ),
    bShowHeaderSeparator( false ),
    bShowFooterSeparator( false ),
    bHeaderFooterEdit( false ),
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
    pDoc( rShell.GetDoc() ),
    nStartAction( 0 ),
    nLockPaint( 0 ),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::SwViewShell" );

    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during contruction of <ViewShell> instance
    mbInConstructor = true;

    bPaintWorks = bEnableSmooth = sal_True;
    bPaintInProgress = bViewLocked = bInEndAction = bFrameView =
    bEndActionByVirDev = sal_False;
    bPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    // OD 12.12.2002 #103492#
    if( nFlags & VSHELLFLAG_SHARELAYOUT ) //swmod 080125
        pLayout = rShell.pLayout;//swmod 080125

    SET_CURR_SHELL( this );

    pDoc->acquire();
    sal_Bool bModified = pDoc->IsModified();

    pOutput = pOut;
    Init( rShell.GetViewOptions() );    //verstellt ggf. das Outdev (InitPrt())
    pOut = pOutput;

    // OD 12.12.2002 #103492#
    if ( bPreView )
        pImp->InitPagePreviewLayout();

    ((SwHiddenTxtFieldType*)pDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !pOpt->IsShowHiddenField() );

    // in Init wird ein Standard-FrmFmt angelegt
    if( !bModified && !pDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
    {
        pDoc->ResetModified();
    }

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( pOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    // OD 2004-06-01 #i26791#
    mbInConstructor = false;

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
        bPaintWorks = sal_False;

        // FME 2004-06-21 #i9684# Stopping the animated graphics is not
        // necessary during printing or pdf export, because the animation
        // has not been started in this case.
        if( pDoc && GetWin() )
        {
            SwNodes& rNds = pDoc->GetNodes();
            SwGrfNode *pGNd;

            SwStartNode *pStNd;
            SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
            while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                ++aIdx;
                if ( 0 != ( pGNd = aIdx.GetNode().GetGrfNode() ) )
                {
                    if( pGNd->IsAnimated() )
                    {
                        SwIterator<SwFrm,SwGrfNode> aIter( *pGNd );
                        for( SwFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
                        {
                            OSL_ENSURE( pFrm->IsNoTxtFrm(), "GraphicNode with Text?" );
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
            if( !pDoc->release() )
                delete pDoc, pDoc = 0;
            else
                GetLayout()->ResetNewLayout();
        }//swmod 080317

        delete pOpt;

        //Format-Cache zurueckschrauben.
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

        //Ggf. aus der PaintQueue entfernen lassen
        SwPaintQueue::Remove( this );

        OSL_ENSURE( !nStartAction, "EndAction() pending." );
    }

    if ( pDoc )
    {
        GetLayout()->DeRegisterShell( this );
        if(pDoc->GetCurrentViewShell()==this)
            pDoc->SetCurrentViewShell( this->GetNext()!=this ?
            (ViewShell*)this->GetNext() : NULL );
    }

    delete mpTmpRef;
    delete pAccOptions;
}

sal_Bool ViewShell::HasDrawView() const
{
    return Imp() ? Imp()->HasDrawView() : 0;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
