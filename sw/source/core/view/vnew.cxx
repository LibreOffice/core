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
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <docsh.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <fntcache.hxx>
#include <docufld.hxx>
#include <ptqueue.hxx>
#include <dview.hxx>
#include <ndgrf.hxx>
#include <ndindex.hxx>
#include <accessibilityoptions.hxx>
#include <switerator.hxx>

void SwViewShell::Init( const SwViewOption *pNewOpt )
{
    mbDocSizeChgd = false;

    // We play it save: Remove old font information whenever the printer
    // resolution or the zoom factor changes. For that, Init() and Reformat()
    // are the most secure places.
     pFntCache->Flush( );

    // ViewOptions are created dynamically

    if( !mpOpt )
    {
        mpOpt = new SwViewOption;

        // ApplyViewOptions() does not need to be called
        if( pNewOpt )
        {
            *mpOpt = *pNewOpt;
            // Zoom factor needs to be set because there is no call to ApplyViewOptions() during
            // CTOR for performance reasons.
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
    mpDoc->GetDocumentSettingManager().set(IDocumentSettingAccess::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
    // JP 02.02.99: Bug 61335 - set readonly flag at ViewOptions before creating layout. Otherwise,
    //                          one would have to reformat again.

    if( pDShell && pDShell->IsReadOnly() )
        mpOpt->SetReadonly( true );

    SAL_INFO( "sw.core", "View::Init - before InitPrt" );
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

    SAL_INFO( "sw.core", "View::Init - after InitPrt" );
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
        SwViewShell *pCurrShell = GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
        if( pCurrShell )
            mpLayout = pCurrShell->mpLayout;
        // end of "disable multiple layouts"
        if( !mpLayout )
        {
            // switched to two step construction because creating the layout in SwRootFrm needs a valid pLayout set
            mpLayout = SwRootFrmPtr(new SwRootFrm( mpDoc->GetDfltFrmFmt(), this ));
            mpLayout->Init( mpDoc->GetDfltFrmFmt() );
        }
    }
    SizeChgNotify();

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

/// CTor for the first Shell.
SwViewShell::SwViewShell( SwDoc& rDocument, vcl::Window *pWindow,
                        const SwViewOption *pNewOpt, OutputDevice *pOutput,
                        long nFlags )
    :
    maBrowseBorder(),
    mpSfxViewShell( 0 ),
    mpImp( new SwViewImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rDocument.getIDocumentDeviceAccess().getPrinter( true )),
    mpTmpRef( 0 ),
    mpOpt( 0 ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mbTiledRendering(false),
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
    mpDoc( &rDocument ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPaintWorks = mbEnableSmooth = true;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    // --> OD 2005-02-11 #i38810# - Do not reset modified state of document,
    // if it's already been modified.
    const bool bIsDocModified( mpDoc->getIDocumentState().IsModified() );
    mpDoc->acquire();
    pOutput = mpOut;
    Init( pNewOpt );    // may change the Outdev (InitPrt())
    mpOut = pOutput;

    // OD 28.03.2003 #108470# - initialize print preview layout after layout
    // is created in <SwViewShell::Init(..)> - called above.
    if ( mbPreview )
    {
        // OD 12.12.2002 #103492# - init page preview layout
        mpImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

    ((SwHiddenTxtFieldType*)mpDoc->getIDocumentFieldsAccess().GetSysFldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrmFmt is created.
    // --> OD 2005-02-11 #i38810#
    if (   !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
        && !bIsDocModified )
    // <--
    {
        mpDoc->getIDocumentState().ResetModified();
    }

    // extend format cache.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    // OD 2004-06-01 #i26791#
    mbInConstructor = false;
}

/// CTor for further Shells on a document.
SwViewShell::SwViewShell( SwViewShell& rShell, vcl::Window *pWindow,
                        OutputDevice *pOutput, long nFlags ) :
    Ring( &rShell ),
    maBrowseBorder( rShell.maBrowseBorder ),
    mpSfxViewShell( 0 ),
    mpImp( new SwViewImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? (OutputDevice*)pWindow
                            : (OutputDevice*)rShell.GetDoc()->getIDocumentDeviceAccess().getPrinter( true )),
    mpTmpRef( 0 ),
    mpOpt( 0 ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mbTiledRendering(false),
    mpTargetPaintWindow(0), // #i74769#
    mpBufferedOut(0), // #i74769#
    mpDoc( rShell.GetDoc() ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mpPrePostOutDev(0), // #i72754#
    maPrePostMapMode()
{
    // OD 2004-06-01 #i26791# - in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintWorks = mbEnableSmooth = true;
    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    // OD 12.12.2002 #103492#
    if( nFlags & VSHELLFLAG_SHARELAYOUT )
        mpLayout = rShell.mpLayout;

    SET_CURR_SHELL( this );

    mpDoc->acquire();
    bool bModified = mpDoc->getIDocumentState().IsModified();

    pOutput = mpOut;
    Init( rShell.GetViewOptions() ); // might change Outdev (InitPrt())
    mpOut = pOutput;

    // OD 12.12.2002 #103492#
    if ( mbPreview )
        mpImp->InitPagePreviewLayout();

    ((SwHiddenTxtFieldType*)mpDoc->getIDocumentFieldsAccess().GetSysFldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrmFmt is created.
    if( !bModified && !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
    {
        mpDoc->getIDocumentState().ResetModified();
    }

    // extend format cache.
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    // OD 2004-06-01 #i26791#
    mbInConstructor = false;

}

SwViewShell::~SwViewShell()
{
    {
        SET_CURR_SHELL( this );
        mbPaintWorks = false;

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

        delete mpImp; // Delete first, so that the LayoutViews are destroyed.
        mpImp = 0;   // Set to zero, because ~SwFrm relies on it.

        if ( mpDoc )
        {
            if( !mpDoc->release() )
                delete mpDoc, mpDoc = 0;
            else
                GetLayout()->ResetNewLayout();
        }

        delete mpOpt;

        // resize format cache.
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

        // Remove from PaintQueue if necessary
        SwPaintQueue::Remove( this );

        OSL_ENSURE( !mnStartAction, "EndAction() pending." );
    }

    if ( mpDoc )
    {
        GetLayout()->DeRegisterShell( this );
        if(mpDoc->getIDocumentLayoutAccess().GetCurrentViewShell()==this)
            mpDoc->getIDocumentLayoutAccess().SetCurrentViewShell( this->GetNext()!=this ?
            (SwViewShell*)this->GetNext() : NULL );
    }

    delete mpTmpRef;
    delete mpAccOptions;
}

bool SwViewShell::HasDrawView() const
{
    return Imp() && Imp()->HasDrawView();
}

void SwViewShell::MakeDrawView()
{
    Imp()->MakeDrawView( );
}

SdrView* SwViewShell::GetDrawView()
{
    return Imp()->GetDrawView();
}

SdrView* SwViewShell::GetDrawViewWithValidMarkList()
{
    SwDrawView* pDView = Imp()->GetDrawView();
    pDView->ValidateMarkList();
    return pDView;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
