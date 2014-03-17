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


#include "DrawDocShell.hxx"
#include <vcl/svapp.hxx>

#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>
#include <editeng/flstitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <svx/drawitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/graphicfilter.hxx>
#include <comphelper/classids.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/visitem.hxx>

#include <svx/svdoutl.hxx>

#include <sfx2/fcontnr.hxx>

#include "app.hrc"
#include "sdmod.hxx"
#include "strmname.h"
#include "stlpool.hxx"
#include "strings.hrc"
#include "View.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "fupoor.hxx"
#include "fusearch.hxx"
#include "ViewShell.hxx"
#include "sdresid.hxx"
#include "slideshow.hxx"
#include "drawview.hxx"
#include "FrameView.hxx"
#include "unomodel.hxx"
#include "undo/undomanager.hxx"
#include "undo/undofactory.hxx"
#include "OutlineView.hxx"
#include "ViewShellBase.hxx"

using namespace sd;
#define DrawDocShell
#include "sdslots.hxx"

SFX_IMPL_INTERFACE(DrawDocShell, SfxObjectShell, SdResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
}


namespace sd {

GraphicFilter* GetGrfFilter();

/**
 * slotmaps and definitions of SFX
 */
TYPEINIT1( DrawDocShell, SfxObjectShell );

SFX_IMPL_OBJECTFACTORY(
    DrawDocShell,
    SvGlobalName(SO3_SIMPRESS_CLASSID),
    SFXOBJECTSHELL_STD_NORMAL,
    "simpress" )

void DrawDocShell::Construct( bool bClipboard )
{
    mbInDestruction = sal_False;
    SetSlotFilter();     // setzt Filter zurueck

    mbOwnDocument = mpDoc == 0;
    if( mbOwnDocument )
        mpDoc = new SdDrawDocument(meDocType, this);

    // The document has been created so we can call UpdateRefDevice() to set
    // the document's ref device.
    UpdateRefDevice();

    SetBaseModel( new SdXImpressDocument( this, bClipboard ) );
    SetPool( &mpDoc->GetItemPool() );
    mpUndoManager = new sd::UndoManager;
    mpDoc->SetSdrUndoManager( mpUndoManager );
    mpDoc->SetSdrUndoFactory( new sd::UndoFactory );
    UpdateTablePointers();
    SetStyleFamily(5);       //CL: actually SFX_STYLE_FAMILY_PSEUDO
}

DrawDocShell::DrawDocShell(SfxObjectCreateMode eMode,
                               sal_Bool bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell( eMode == SFX_CREATE_MODE_INTERNAL ?  SFX_CREATE_MODE_EMBEDDED : eMode),
    mpDoc(NULL),
    mpUndoManager(NULL),
    mpPrinter(NULL),
    mpViewShell(NULL),
    mpFontList(NULL),
    meDocType(eDocumentType),
    mpFilterSIDs(0),
    mbSdDataObj(bDataObject),
    mbOwnPrinter(sal_False),
    mbNewDocument( sal_True )
{
    Construct( eMode == SFX_CREATE_MODE_INTERNAL );
}

DrawDocShell::DrawDocShell( const sal_uInt64 nModelCreationFlags, sal_Bool bDataObject, DocumentType eDocumentType ) :
    SfxObjectShell( nModelCreationFlags ),
    mpDoc(NULL),
    mpUndoManager(NULL),
    mpPrinter(NULL),
    mpViewShell(NULL),
    mpFontList(NULL),
    meDocType(eDocumentType),
    mpFilterSIDs(0),
    mbSdDataObj(bDataObject),
    mbOwnPrinter(sal_False),
    mbNewDocument( sal_True )
{
    Construct( false );
}

DrawDocShell::DrawDocShell(SdDrawDocument* pDoc, SfxObjectCreateMode eMode,
                               sal_Bool bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell(eMode == SFX_CREATE_MODE_INTERNAL ?  SFX_CREATE_MODE_EMBEDDED : eMode),
    mpDoc(pDoc),
    mpUndoManager(NULL),
    mpPrinter(NULL),
    mpViewShell(NULL),
    mpFontList(NULL),
    meDocType(eDocumentType),
    mpFilterSIDs(0),
    mbSdDataObj(bDataObject),
    mbOwnPrinter(sal_False),
    mbNewDocument( sal_True )
{
    Construct( eMode == SFX_CREATE_MODE_INTERNAL );
}

DrawDocShell::~DrawDocShell()
{
    // Tell all listeners that the doc shell is about to be
    // destroyed.  This has been introduced for the PreviewRenderer to
    // free its view (that uses the item poll of the doc shell) but
    // may be useful in other places as well.
    Broadcast(SfxSimpleHint(SFX_HINT_DYING));

    mbInDestruction = sal_True;

    SetDocShellFunction(0);

    delete mpFontList;

    if( mpDoc )
        mpDoc->SetSdrUndoManager( 0 );
    delete mpUndoManager;

    if (mbOwnPrinter)
        delete mpPrinter;

    if( mbOwnDocument )
        delete mpDoc;

    // that the navigator get informed about the disappearance of the document
    SfxBoolItem     aItem(SID_NAVIGATOR_INIT, true);
    SfxViewFrame*   pFrame = mpViewShell ? mpViewShell->GetFrame() : GetFrame();

    if( !pFrame )
        pFrame = SfxViewFrame::GetFirst( this );

    if( pFrame )
        pFrame->GetDispatcher()->Execute(
            SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
}

void DrawDocShell::GetState(SfxItemSet &rSet)
{

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;

        switch ( nSlotId )
        {
            case SID_SEARCH_ITEM:
            {
                rSet.Put( *SD_MOD()->GetSearchItem() );
            }
            break;

            case SID_CLOSEDOC:
            {
                sal_Bool bDisabled = sal_False;
                if (bDisabled)
                {
                    rSet.DisableItem(SID_CLOSEDOC);
                }
                else
                {
                    GetSlotState(SID_CLOSEDOC, SfxObjectShell::GetInterface(), &rSet);
                }
            }
            break;

            case SID_SEARCH_OPTIONS:
            {
                sal_uInt16 nOpt = SEARCH_OPTIONS_SEARCH      |
                              SEARCH_OPTIONS_WHOLE_WORDS |
                              SEARCH_OPTIONS_BACKWARDS   |
                              SEARCH_OPTIONS_REG_EXP     |
                              SEARCH_OPTIONS_EXACT       |
                              SEARCH_OPTIONS_SIMILARITY  |
                              SEARCH_OPTIONS_SELECTION;

                if (!IsReadOnly())
                {
                    nOpt |= SEARCH_OPTIONS_REPLACE;
                    nOpt |= SEARCH_OPTIONS_REPLACE_ALL;
                }

                rSet.Put(SfxUInt16Item(nWhich, nOpt));
            }
            break;

            case SID_VERSION:
            {
                GetSlotState( SID_VERSION, SfxObjectShell::GetInterface(), &rSet );
            }
            break;

            case SID_CHINESE_CONVERSION:
            case SID_HANGUL_HANJA_CONVERSION:
            {
                rSet.Put(SfxVisibilityItem(nWhich, SvtCJKOptions().IsAnyEnabled()));
            }
            break;

            default:
            break;
        }
        nWhich = aIter.NextWhich();
    }

    SfxViewFrame* pFrame = SfxViewFrame::Current();

    if (pFrame)
    {
        if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
        {
            pFrame->GetSlotState(SID_RELOAD,
                                 pFrame->GetInterface(), &rSet);
        }
    }
}

void DrawDocShell::InPlaceActivate( bool bActive )
{
    ViewShell* pViewSh = NULL;
    SfxViewShell* pSfxViewSh = NULL;
    SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, false);
    std::vector<FrameView*> &rViews = mpDoc->GetFrameViewList();

    if( !bActive )
    {
        std::vector<FrameView*>::iterator pIter;
        for ( pIter = rViews.begin(); pIter != rViews.end(); ++pIter )
            delete *pIter;

        rViews.clear();

        while (pSfxViewFrame)
        {
            // determine the number of FrameViews
            pSfxViewSh = pSfxViewFrame->GetViewShell();
            pViewSh = PTR_CAST( ViewShell, pSfxViewSh );

            if ( pViewSh && pViewSh->GetFrameView() )
            {
                pViewSh->WriteFrameViewData();
                rViews.push_back( new FrameView( mpDoc, pViewSh->GetFrameView() ) );
            }

            pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, false);
        }
    }

    SfxObjectShell::InPlaceActivate( bActive );

    if( bActive )
    {
        for( sal_uInt32 i = 0; pSfxViewFrame && (i < rViews.size()); i++ )
        {
            // determine the number of FrameViews
            pSfxViewSh = pSfxViewFrame->GetViewShell();
            pViewSh = PTR_CAST( ViewShell, pSfxViewSh );

            if ( pViewSh )
            {
                pViewSh->ReadFrameViewData( rViews[ i ] );
            }

            pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, false);
        }
    }
}

void DrawDocShell::Activate( sal_Bool bMDI)
{
    if (bMDI)
    {
        ApplySlotFilter();
        mpDoc->StartOnlineSpelling();
    }
}

void DrawDocShell::Deactivate( sal_Bool )
{
}

::svl::IUndoManager* DrawDocShell::GetUndoManager()
{
    return mpUndoManager;
}

void DrawDocShell::UpdateTablePointers()
{
    PutItem( SvxColorListItem( mpDoc->GetColorList(), SID_COLOR_TABLE ) );
    PutItem( SvxGradientListItem( mpDoc->GetGradientList(), SID_GRADIENT_LIST ) );
    PutItem( SvxHatchListItem( mpDoc->GetHatchList(), SID_HATCH_LIST ) );
    PutItem( SvxBitmapListItem( mpDoc->GetBitmapList(), SID_BITMAP_LIST ) );
    PutItem( SvxDashListItem( mpDoc->GetDashList(), SID_DASH_LIST ) );
    PutItem( SvxLineEndListItem( mpDoc->GetLineEndList(), SID_LINEEND_LIST ) );

    UpdateFontList();
}

void DrawDocShell::CancelSearching()
{
    if( dynamic_cast<FuSearch*>( mxDocShellFunction.get() ) )
    {
        SetDocShellFunction(0);
    }
}

/**
 * apply configured slot filters
 */
void DrawDocShell::ApplySlotFilter() const
{
    SfxViewShell* pTestViewShell = SfxViewShell::GetFirst();

    while( pTestViewShell )
    {
        if( pTestViewShell->GetObjectShell()
            == const_cast<DrawDocShell*>( this )
            && pTestViewShell->GetViewFrame()
            && pTestViewShell->GetViewFrame()->GetDispatcher() )
        {
            SfxDispatcher* pDispatcher = pTestViewShell->GetViewFrame()->GetDispatcher();

            if( mpFilterSIDs )
                pDispatcher->SetSlotFilter( mbFilterEnable, mnFilterCount, mpFilterSIDs );
            else
                pDispatcher->SetSlotFilter();

            if( pDispatcher->GetBindings() )
                pDispatcher->GetBindings()->InvalidateAll( true );
        }

        pTestViewShell = SfxViewShell::GetNext( *pTestViewShell );
    }
}

void DrawDocShell::SetModified( bool bSet /* = true */ )
{
    SfxObjectShell::SetModified( bSet );

    // change model state, too
    // only set the changed state if modification is enabled
    if( IsEnableSetModified() )
    {
        if ( mpDoc )
            mpDoc->NbcSetChanged( bSet );

        Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
    }
}

/**
 * Callback for ExecuteSpellPopup()
 */
// ExecuteSpellPopup now handled by DrawDocShell. This is necessary
// to get hands on the outliner and the text object.
IMPL_LINK(DrawDocShell, OnlineSpellCallback, SpellCallbackInfo*, pInfo)
{
    SdrObject* pObj = NULL;
    SdrOutliner* pOutl = NULL;

    if(GetViewShell())
    {
        pOutl = GetViewShell()->GetView()->GetTextEditOutliner();
        pObj = GetViewShell()->GetView()->GetTextEditObject();
    }

    mpDoc->ImpOnlineSpellCallback(pInfo, pObj, pOutl);
    return(0);
}

void DrawDocShell::ClearUndoBuffer()
{
    // clear possible undo buffers of outliners
    SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, false);
    while(pSfxViewFrame)
    {
        ViewShellBase* pViewShellBase = dynamic_cast< ViewShellBase* >( pSfxViewFrame->GetViewShell() );
        if( pViewShellBase )
        {
            ::boost::shared_ptr<ViewShell> pViewSh( pViewShellBase->GetMainViewShell() );
            if( pViewSh.get() )
            {
                ::sd::View* pView = pViewSh->GetView();
                if( pView )
                {
                    pView->SdrEndTextEdit();
                    sd::OutlineView* pOutlView = dynamic_cast< sd::OutlineView* >( pView );
                    if( pOutlView )
                    {
                        SdrOutliner* pOutliner = pOutlView->GetOutliner();
                        if( pOutliner )
                            pOutliner->GetUndoManager().Clear();
                    }
                }
            }
        }
        pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, false);
    }

    ::svl::IUndoManager* pUndoManager = GetUndoManager();
    if(pUndoManager && pUndoManager->GetUndoActionCount())
        pUndoManager->Clear();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
