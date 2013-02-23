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

    mbDocSizeChgd = sal_False;

    // Wir gehen auf Nummer sicher:
    // Wir muessen die alten Fontinformationen wegschmeissen,
    // wenn die Druckeraufloesung oder der Zoomfaktor sich aendert.
    // Init() und Reformat() sind die sichersten Stellen.
     pFntCache->Flush( );

    // ViewOptions werden dynamisch angelegt

    if( !mpOpt )
    {
        mpOpt = new SwViewOption;

        // Ein ApplyViewOptions braucht nicht gerufen zu werden
        if( pNewOpt )
        {
            *mpOpt = *pNewOpt;
            // Der Zoomfaktor muss eingestellt werden, weil in der CTOR-
            // phase aus Performancegruenden kein ApplyViewOptions gerufen wird.
            if( GetWin() && 100 != mpOpt->GetZoom() )
            {
                MapMode aMode( mpWin->GetMapMode() );
                const Fraction aNewFactor( mpOpt->GetZoom(), 100 );
                aMode.SetScaleX( aNewFactor );
                aMode.SetScaleY( aNewFactor );
                mpWin->SetMapMode( aMode );
            }
        }
    }

    SwDocShell* pDShell = mpDoc->GetDocShell();
    mpDoc->set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
    // JP 02.02.99: Bug 61335 - Readonly-Flag an den ViewOptions setzen,
    //              bevor das Layout angelegt wird. Ansonsten muesste man
    //              nochmals durchformatieren!!

    if( pDShell && pDShell->IsReadOnly() )
        mpOpt->SetReadonly( sal_True );

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - before InitPrt" );

    // --> FME 2007-11-06 #i82967#
    OutputDevice* pPDFOut = 0;

    if ( mpOut && mpOut->GetPDFWriter() )
        pPDFOut = mpOut;
    // <--

    // --> FME 2005-01-21 #i41075#
    // Only setup the printer if we need one:
    const bool bBrowseMode = mpOpt->getBrowseMode();
    if( pPDFOut )
        InitPrt( pPDFOut );
    // <--

    // --> FME 2005-03-16 #i44963# Good occasion to check if page sizes in
    // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
    if ( !bBrowseMode )
    {
        mpDoc->CheckDefaultPageFmt();
    }
    // <--

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - after InitPrt" );

    if( GetWin() )
    {
        mpOpt->Init( GetWin() );
        GetWin()->SetFillColor();
        GetWin()->SetBackground();
        GetWin()->SetLineColor();
    }

    // Create a new layout, if there is no one available
    if( !mpLayout )
    {
        // Here's the code which disables the usage of "multiple" layouts at the moment
        // If the problems with controls and groups objects are solved,
        // this code can be removed...
        ViewShell *pCurrShell = GetDoc()->GetCurrentViewShell();
        if( pCurrShell )
            mpLayout = pCurrShell->mpLayout;
        // end of "disable multiple layouts"
        if( !mpLayout )
        {
            // switched to two step construction because creating the layout in SwRootFrm needs a valid pLayout set
            mpLayout = SwRootFrmPtr(new SwRootFrm( mpDoc->GetDfltFrmFmt(), this ));
            mpLayout->Init( mpDoc->GetDfltFrmFmt() );

            // mba: the layout refactoring overlooked an important detail
            // prior to that change, the layout always was destroyed in the dtor of swdoc
            // it is necessary to suppress notifications in the layout when the layout is discarded in its dtor
            // unfortunately this was done by asking whether the doc is in dtor - though the correct question should
            // have been if the rootfrm is in dtor (or even better: discard the layout before the SwRootFrm is destroyed!)
            // SwDoc::IsInDtor() is used at several places all over the code that need to be checked whether
            // "pDoc->IsInDtor()" means what is says or in fact should check for "pRootFrm->IsInDtor()". As this will take some time, I decided
            // to postpone that investigations and the changes it will bring to the 3.5 release and for 3.4 make sure
            // that the layout still gets destroyed in the doc dtor. This is done by sharing "the" layout (that we still have) with the doc.
            GetDoc()->ShareLayout( mpLayout );
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
        mpOpt->SetFormView( ! GetDrawView()->IsDesignMode() );
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
    maBrowseBorder(),
    mpSfxViewShell( 0 ),
    mpImp( new SwViewImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rDocument.getPrinter( true )),
    mpTmpRef( 0 ),
    mpOpt( 0 ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
    mpDoc( &rDocument ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::SwViewShell" );

    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during contruction of <ViewShell> instance
    mbInConstructor = true;

    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = sal_False;
    mbPaintWorks = mbEnableSmooth = sal_True;
    mbPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    // --> OD 2005-02-11 #i38810# - Do not reset modified state of document,
    // if it's already been modified.
    const bool bIsDocModified( mpDoc->IsModified() );
    mpDoc->acquire();
    pOutput = mpOut;
    Init( pNewOpt );    //verstellt ggf. das Outdev (InitPrt())
    mpOut = pOutput;

    // OD 28.03.2003 #108470# - initialize print preview layout after layout
    // is created in <ViewShell::Init(..)> - called above.
    if ( mbPreView )
    {
        // OD 12.12.2002 #103492# - init page preview layout
        mpImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

    ((SwHiddenTxtFieldType*)mpDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    //In Init wird ein Standard-FrmFmt angelegt.
    // --> OD 2005-02-11 #i38810#
    if (   !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
        && !bIsDocModified )
    // <--
    {
        mpDoc->ResetModified();
    }

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
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
    maBrowseBorder( rShell.maBrowseBorder ),
    mpSfxViewShell( 0 ),
    mpImp( new SwViewImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rShell.GetDoc()->getPrinter( true )),
    mpTmpRef( 0 ),
    mpOpt( 0 ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
    mpDoc( rShell.GetDoc() ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::SwViewShell" );

    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during contruction of <ViewShell> instance
    mbInConstructor = true;

    mbPaintWorks = mbEnableSmooth = sal_True;
    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = sal_False;
    mbPreView = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    // OD 12.12.2002 #103492#
    if( nFlags & VSHELLFLAG_SHARELAYOUT ) //swmod 080125
        mpLayout = rShell.mpLayout;

    SET_CURR_SHELL( this );

    mpDoc->acquire();
    sal_Bool bModified = mpDoc->IsModified();

    pOutput = mpOut;
    Init( rShell.GetViewOptions() );    //verstellt ggf. das Outdev (InitPrt())
    mpOut = pOutput;

    // OD 12.12.2002 #103492#
    if ( mbPreView )
        mpImp->InitPagePreviewLayout();

    ((SwHiddenTxtFieldType*)mpDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // in Init wird ein Standard-FrmFmt angelegt
    if( !bModified && !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
    {
        mpDoc->ResetModified();
    }

    //Format-Cache erweitern.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
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
        mbPaintWorks = sal_False;

        // FME 2004-06-21 #i9684# Stopping the animated graphics is not
        // necessary during printing or pdf export, because the animation
        // has not been started in this case.
        if( mpDoc && GetWin() )
        {
            SwNodes& rNds = mpDoc->GetNodes();
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
                            ((SwNoTxtFrm*)pFrm)->StopAnimation( mpOut );
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
            }

            GetDoc()->StopNumRuleAnimations( mpOut );
        }

        delete mpImp; //Erst loeschen, damit die LayoutViews vernichtet werden.
        mpImp = 0;   // Set to zero, because ~SwFrm relies on it.

        if ( mpDoc )
        {
            if( !mpDoc->release() )
                delete mpDoc, mpDoc = 0;
            else
                GetLayout()->ResetNewLayout();
        }//swmod 080317

        delete mpOpt;

        //Format-Cache zurueckschrauben.
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

        //Ggf. aus der PaintQueue entfernen lassen
        SwPaintQueue::Remove( this );

        OSL_ENSURE( !mnStartAction, "EndAction() pending." );
    }

    if ( mpDoc )
    {
        GetLayout()->DeRegisterShell( this );
        if(mpDoc->GetCurrentViewShell()==this)
            mpDoc->SetCurrentViewShell( this->GetNext()!=this ?
            (ViewShell*)this->GetNext() : NULL );
    }

    delete mpTmpRef;
    delete mpAccOptions;
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
