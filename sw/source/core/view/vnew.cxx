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
#include <calbck.hxx>

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
    mpDoc->GetDocumentSettingManager().set(DocumentSettingId::HTML_MODE, 0 != ::GetHtmlMode( pDShell ) );
    // set readonly flag at ViewOptions before creating layout. Otherwise,
    // one would have to reformat again.

    if( pDShell && pDShell->IsReadOnly() )
        mpOpt->SetReadonly( true );

    SAL_INFO( "sw.core", "View::Init - before InitPrt" );
    OutputDevice* pPDFOut = nullptr;

    if ( mpOut && mpOut->GetPDFWriter() )
        pPDFOut = mpOut;

    // Only setup the printer if we need one:
    const bool bBrowseMode = mpOpt->getBrowseMode();
    if( pPDFOut )
        InitPrt( pPDFOut );

    // i#44963 Good occasion to check if page sizes in
    // page descriptions are still set to (LONG_MAX, LONG_MAX) (html import)
    if ( !bBrowseMode )
    {
        mpDoc->CheckDefaultPageFormat();
    }

    SAL_INFO( "sw.core", "View::Init - after InitPrt" );
    if( GetWin() )
    {
        SwViewOption::Init( GetWin() );
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
            // switched to two step construction because creating the layout in SwRootFrame needs a valid pLayout set
            mpLayout = SwRootFramePtr(new SwRootFrame(mpDoc->GetDfltFrameFormat(), this),
                                    &SwFrame::DestroyFrame);
            mpLayout->Init( mpDoc->GetDfltFrameFormat() );
        }
    }
    SizeChgNotify();

    // XForms mode: initialize XForms mode, based on design mode (draw view)
    //   MakeDrawView() requires layout
    if( GetDoc()->isXForms() )
    {
        if( ! HasDrawView() )
            MakeDrawView();
        mpOpt->SetFormView( ! GetDrawView()->IsDesignMode() );
    }
}

/// CTor for the first Shell.
SwViewShell::SwViewShell( SwDoc& rDocument, vcl::Window *pWindow,
                        const SwViewOption *pNewOpt, OutputDevice *pOutput,
                        long nFlags )
    : m_pReplaceBmp(nullptr)
    , m_pErrorBmp(nullptr)
    ,
    maBrowseBorder(),
    mpSfxViewShell( nullptr ),
    mpImp( new SwViewShellImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? static_cast<OutputDevice*>(pWindow)
                            : static_cast<OutputDevice*>(rDocument.getIDocumentDeviceAccess().getPrinter( true ))),
    mpTmpRef( nullptr ),
    mpOpt( nullptr ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mpTargetPaintWindow(nullptr),
    mpBufferedOut(nullptr),
    mpDoc( &rDocument ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mbOutputToWindow(false),
    mpPrePostOutDev(nullptr),
    maPrePostMapMode()
{
    // in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPaintWorks = mbEnableSmooth = true;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    // i#38810 Do not reset modified state of document,
    // if it's already been modified.
    const bool bIsDocModified( mpDoc->getIDocumentState().IsModified() );
    mpDoc->acquire();
    pOutput = mpOut;
    Init( pNewOpt );    // may change the Outdev (InitPrt())
    mpOut = pOutput;

    // initialize print preview layout after layout
    // is created in <SwViewShell::Init(..)> - called above.
    if ( mbPreview )
    {
        // init page preview layout
        mpImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

    static_cast<SwHiddenTextFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrameFormat is created.
    if (   !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
        && !bIsDocModified )
    {
        mpDoc->getIDocumentState().ResetModified();
    }

    // extend format cache.
    if ( SwTextFrame::GetTextCache()->GetCurMax() < 2550 )
        SwTextFrame::GetTextCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess().GetDrawModel() )
        Imp()->MakeDrawView();

    mbInConstructor = false;
}

/// CTor for further Shells on a document.
SwViewShell::SwViewShell( SwViewShell& rShell, vcl::Window *pWindow,
                        OutputDevice * pOutput, long const nFlags)
    : Ring( &rShell )
    , m_pReplaceBmp(nullptr)
    , m_pErrorBmp(nullptr)
    ,
    maBrowseBorder( rShell.maBrowseBorder ),
    mpSfxViewShell( nullptr ),
    mpImp( new SwViewShellImp( this ) ),
    mpWin( pWindow ),
    mpOut( pOutput ? pOutput
                  : pWindow ? static_cast<OutputDevice*>(pWindow)
                            : static_cast<OutputDevice*>(rShell.GetDoc()->getIDocumentDeviceAccess().getPrinter( true ))),
    mpTmpRef( nullptr ),
    mpOpt( nullptr ),
    mpAccOptions( new SwAccessibilityOptions ),
    mbShowHeaderSeparator( false ),
    mbShowFooterSeparator( false ),
    mbHeaderFooterEdit( false ),
    mpTargetPaintWindow(nullptr),
    mpBufferedOut(nullptr),
    mpDoc( rShell.GetDoc() ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mbOutputToWindow(false),
    mpPrePostOutDev(nullptr),
    maPrePostMapMode()
{
    // in order to suppress event handling in
    // <SwDrawContact::Changed> during construction of <SwViewShell> instance
    mbInConstructor = true;

    mbPaintWorks = mbEnableSmooth = true;
    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = false;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    if( nFlags & VSHELLFLAG_SHARELAYOUT )
        mpLayout = rShell.mpLayout;

    SET_CURR_SHELL( this );

    mpDoc->acquire();
    bool bModified = mpDoc->getIDocumentState().IsModified();

    pOutput = mpOut;
    Init( rShell.GetViewOptions() ); // might change Outdev (InitPrt())
    mpOut = pOutput;

    if ( mbPreview )
        mpImp->InitPagePreviewLayout();

    static_cast<SwHiddenTextFieldType*>(mpDoc->getIDocumentFieldsAccess().GetSysFieldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    // In Init a standard FrameFormat is created.
    if( !bModified && !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
    {
        mpDoc->getIDocumentState().ResetModified();
    }

    // extend format cache.
    if ( SwTextFrame::GetTextCache()->GetCurMax() < 2550 )
        SwTextFrame::GetTextCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess().GetDrawModel() )
        Imp()->MakeDrawView();

    mbInConstructor = false;

}

SwViewShell::~SwViewShell()
{
    {
        SET_CURR_SHELL( this );
        mbPaintWorks = false;

        // i#9684 Stopping the animated graphics is not
        // necessary during printing or pdf export, because the animation
        // has not been started in this case.
        if( mpDoc && GetWin() )
        {
            SwNodes& rNds = mpDoc->GetNodes();

            SwStartNode *pStNd;
            SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
            while ( nullptr != (pStNd = aIdx.GetNode().GetStartNode()) )
            {
                ++aIdx;
                SwGrfNode *pGNd = aIdx.GetNode().GetGrfNode();
                if ( nullptr != pGNd )
                {
                    if( pGNd->IsAnimated() )
                    {
                        SwIterator<SwFrame,SwGrfNode> aIter( *pGNd );
                        for( SwFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next() )
                        {
                            OSL_ENSURE( pFrame->IsNoTextFrame(), "GraphicNode with Text?" );
                            static_cast<SwNoTextFrame*>(pFrame)->StopAnimation( mpOut );
                        }
                    }
                }
                aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
            }

            GetDoc()->StopNumRuleAnimations( mpOut );
        }

        delete mpImp; // Delete first, so that the LayoutViews are destroyed.
        mpImp = nullptr;   // Set to zero, because ~SwFrame relies on it.

        if ( mpDoc )
        {
            if( !mpDoc->release() )
            {
                delete mpDoc;
                mpDoc = nullptr;
            }
            else
                GetLayout()->ResetNewLayout();
        }

        delete mpOpt;

        // resize format cache
        if ( SwTextFrame::GetTextCache()->GetCurMax() > 250 )
            SwTextFrame::GetTextCache()->DecreaseMax( 100 );

        // Remove from PaintQueue if necessary
        SwPaintQueue::Remove( this );

        OSL_ENSURE( !mnStartAction, "EndAction() pending" );
    }

    if ( mpDoc )
    {
        GetLayout()->DeRegisterShell( this );
        auto& rLayoutAccess(mpDoc->getIDocumentLayoutAccess());
        if(rLayoutAccess.GetCurrentViewShell()==this)
        {
            rLayoutAccess.SetCurrentViewShell(nullptr);
            for(SwViewShell& rShell : GetRingContainer())
            {
                if(&rShell != this)
                {
                    rLayoutAccess.SetCurrentViewShell(&rShell);
                    break;
                }
            }
        }
    }

    mpTmpRef.disposeAndClear();
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
