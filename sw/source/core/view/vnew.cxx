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

#include <sfx2/printer.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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
    mbDocSizeChgd = sal_False;

    
    
    
     pFntCache->Flush( );

    

    if( !mpOpt )
    {
        mpOpt = new SwViewOption;

        
        if( pNewOpt )
        {
            *mpOpt = *pNewOpt;
            
            
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
    
    

    if( pDShell && pDShell->IsReadOnly() )
        mpOpt->SetReadonly( sal_True );

    SAL_INFO( "sw.core", "View::Init - before InitPrt" );
    
    OutputDevice* pPDFOut = 0;

    if ( mpOut && mpOut->GetPDFWriter() )
        pPDFOut = mpOut;
    

    
    
    const bool bBrowseMode = mpOpt->getBrowseMode();
    if( pPDFOut )
        InitPrt( pPDFOut );
    

    
    
    if ( !bBrowseMode )
    {
        mpDoc->CheckDefaultPageFmt();
    }
    

    SAL_INFO( "sw.core", "View::Init - after InitPrt" );
    if( GetWin() )
    {
        mpOpt->Init( GetWin() );
        GetWin()->SetFillColor();
        GetWin()->SetBackground();
        GetWin()->SetLineColor();
    }

    
    if( !mpLayout )
    {
        
        
        
        SwViewShell *pCurrShell = GetDoc()->GetCurrentViewShell();
        if( pCurrShell )
            mpLayout = pCurrShell->mpLayout;
        
        if( !mpLayout )
        {
            
            mpLayout = SwRootFrmPtr(new SwRootFrm( mpDoc->GetDfltFrmFmt(), this ));
            mpLayout->Init( mpDoc->GetDfltFrmFmt() );
        }
    }
    SizeChgNotify();

    
    
    
    if( GetDoc()->isXForms() )
    {
        if( ! HasDrawView() )
            MakeDrawView();
        mpOpt->SetFormView( ! GetDrawView()->IsDesignMode() );
    }
    
}


SwViewShell::SwViewShell( SwDoc& rDocument, Window *pWindow,
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
    mbTiledRendering(false),
    mpTargetPaintWindow(0), 
    mpBufferedOut(0), 
    mpDoc( &rDocument ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mpPrePostOutDev(0), 
    maPrePostMapMode()
{
    
    
    mbInConstructor = true;

    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = sal_False;
    mbPaintWorks = mbEnableSmooth = sal_True;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );

    
    
    const bool bIsDocModified( mpDoc->IsModified() );
    mpDoc->acquire();
    pOutput = mpOut;
    Init( pNewOpt );    
    mpOut = pOutput;

    
    
    if ( mbPreview )
    {
        
        mpImp->InitPagePreviewLayout();
    }

    SET_CURR_SHELL( this );

    ((SwHiddenTxtFieldType*)mpDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
        SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    
    
    if (   !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified()
        && !bIsDocModified )
    
    {
        mpDoc->ResetModified();
    }

    
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    
    mbInConstructor = false;
}


SwViewShell::SwViewShell( SwViewShell& rShell, Window *pWindow,
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
    mbTiledRendering(false),
    mpTargetPaintWindow(0), 
    mpBufferedOut(0), 
    mpDoc( rShell.GetDoc() ),
    mnStartAction( 0 ),
    mnLockPaint( 0 ),
    mbSelectAll(false),
    mpPrePostOutDev(0), 
    maPrePostMapMode()
{
    
    
    mbInConstructor = true;

    mbPaintWorks = mbEnableSmooth = sal_True;
    mbPaintInProgress = mbViewLocked = mbInEndAction = mbFrameView =
    mbEndActionByVirDev = sal_False;
    mbPreview = 0 !=( VSHELLFLAG_ISPREVIEW & nFlags );
    
    if( nFlags & VSHELLFLAG_SHARELAYOUT )
        mpLayout = rShell.mpLayout;

    SET_CURR_SHELL( this );

    mpDoc->acquire();
    sal_Bool bModified = mpDoc->IsModified();

    pOutput = mpOut;
    Init( rShell.GetViewOptions() ); 
    mpOut = pOutput;

    
    if ( mbPreview )
        mpImp->InitPagePreviewLayout();

    ((SwHiddenTxtFieldType*)mpDoc->GetSysFldType( RES_HIDDENTXTFLD ))->
            SetHiddenFlag( !mpOpt->IsShowHiddenField() );

    
    if( !bModified && !mpDoc->GetIDocumentUndoRedo().IsUndoNoResetModified() )
    {
        mpDoc->ResetModified();
    }

    
    if ( SwTxtFrm::GetTxtCache()->GetCurMax() < 2550 )
        SwTxtFrm::GetTxtCache()->IncreaseMax( 100 );
    if( mpOpt->IsGridVisible() || getIDocumentDrawModelAccess()->GetDrawModel() )
        Imp()->MakeDrawView();

    
    mbInConstructor = false;

}

SwViewShell::~SwViewShell()
{
    {
        SET_CURR_SHELL( this );
        mbPaintWorks = sal_False;

        
        
        
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

        delete mpImp; 
        mpImp = 0;   

        if ( mpDoc )
        {
            if( !mpDoc->release() )
                delete mpDoc, mpDoc = 0;
            else
                GetLayout()->ResetNewLayout();
        }

        delete mpOpt;

        
        if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
            SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );

        
        SwPaintQueue::Remove( this );

        OSL_ENSURE( !mnStartAction, "EndAction() pending." );
    }

    if ( mpDoc )
    {
        GetLayout()->DeRegisterShell( this );
        if(mpDoc->GetCurrentViewShell()==this)
            mpDoc->SetCurrentViewShell( this->GetNext()!=this ?
            (SwViewShell*)this->GetNext() : NULL );
    }

    delete mpTmpRef;
    delete mpAccOptions;
}

sal_Bool SwViewShell::HasDrawView() const
{
    return Imp() ? Imp()->HasDrawView() : 0;
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
