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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawDocShell.hxx"
#include <tools/pstm.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svl/srchitem.hxx>
#include <svx/srchdlg.hxx>
#include <editeng/flstitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/printer.hxx>
#ifndef _SFX_DOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#include <svx/drawitem.hxx>
#include <editeng/flstitem.hxx>
#include <svx/drawitem.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/whiter.hxx>
#include <svl/itempool.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/filter.hxx>
#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#include <svl/cjkoptions.hxx>
#include <svl/visitem.hxx>

#include <svx/svdoutl.hxx>

#include <sfx2/fcontnr.hxx>

#include "app.hrc"
#include "app.hxx"
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
#ifndef SD_FU_SLIDE_SHOW_DLG_HXX
#include "slideshow.hxx"
#endif
#include "drawview.hxx"
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
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
        SFX_CHILDWINDOW_REGISTRATION(SID_HYPERLINK_INSERT);
}


namespace sd {

#define POOL_BUFFER_SIZE                (sal_uInt16)32768
#define BASIC_BUFFER_SIZE               (sal_uInt16)8192
#define DOCUMENT_BUFFER_SIZE            (sal_uInt16)32768


GraphicFilter* GetGrfFilter();

/*************************************************************************
|*
|* SFX-Slotmaps und -Definitionen
|*
\************************************************************************/
TYPEINIT1( DrawDocShell, SfxObjectShell );

SFX_IMPL_OBJECTFACTORY(
    DrawDocShell,
    SvGlobalName(SO3_SIMPRESS_CLASSID),
    SFXOBJECTSHELL_STD_NORMAL,
    "simpress" )

/*************************************************************************
|*
|* Construct
|*
\************************************************************************/

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
    SetStyleFamily(5);       //CL: eigentlich SFX_STYLE_FAMILY_PSEUDO
}

/*************************************************************************
|*
|* Konstruktor 1
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Konstruktor 2
|*
\************************************************************************/

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
    Construct( sal_False );
}

/*************************************************************************
|*
|* Konstruktor 3
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

DrawDocShell::~DrawDocShell()
{
    // Tell all listeners that the doc shell is about to be
    // destroyed.  This has been introduced for the PreviewRenderer to
    // free its view (that uses the item poll of the doc shell) but
    // may be usefull in other places as well.
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

    // damit der Navigator das Verschwinden des Dokuments mitbekommt
    SfxBoolItem     aItem(SID_NAVIGATOR_INIT, sal_True);
    SfxViewFrame*   pFrame = mpViewShell ? mpViewShell->GetFrame() : GetFrame();

    if( !pFrame )
        pFrame = SfxViewFrame::GetFirst( this );

    if( pFrame )
        pFrame->GetDispatcher()->Execute(
            SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L);
}

/*************************************************************************
|*
|* Slot-Stati setzen
|*
\************************************************************************/

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

void DrawDocShell::InPlaceActivate( sal_Bool bActive )
{
    if( !bActive )
    {
        FrameView* pFrameView = NULL;
        List* pFrameViewList = mpDoc->GetFrameViewList();

        DBG_ASSERT( pFrameViewList, "No FrameViewList?" );
        if( pFrameViewList )
        {
            sal_uInt32 i;
            for ( i = 0; i < pFrameViewList->Count(); i++)
            {
                // Ggf. FrameViews loeschen
                pFrameView = (FrameView*) pFrameViewList->GetObject(i);

                if (pFrameView)
                    delete pFrameView;
            }

            pFrameViewList->Clear();

            ViewShell* pViewSh = NULL;
            SfxViewShell* pSfxViewSh = NULL;
            SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, false);

            while (pSfxViewFrame)
            {
                // Anzahl FrameViews ermitteln
                pSfxViewSh = pSfxViewFrame->GetViewShell();
                pViewSh = PTR_CAST( ViewShell, pSfxViewSh );

                if ( pViewSh && pViewSh->GetFrameView() )
                {
                    pViewSh->WriteFrameViewData();
                    pFrameViewList->Insert( new FrameView( mpDoc, pViewSh->GetFrameView() ) );
                }

                pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, false);
            }
        }
    }

    SfxObjectShell::InPlaceActivate( bActive );

    if( bActive )
    {
        List* pFrameViewList = mpDoc->GetFrameViewList();

        DBG_ASSERT( pFrameViewList, "No FrameViewList?" );
        if( pFrameViewList )
        {
            ViewShell* pViewSh = NULL;
            SfxViewShell* pSfxViewSh = NULL;
            SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, false);

            sal_uInt32 i;
            for( i = 0; pSfxViewFrame && (i < pFrameViewList->Count()); i++ )
            {
                // Anzahl FrameViews ermitteln
                pSfxViewSh = pSfxViewFrame->GetViewShell();
                pViewSh = PTR_CAST( ViewShell, pSfxViewSh );

                if ( pViewSh )
                {
                    pViewSh->ReadFrameViewData( (FrameView*)pFrameViewList->GetObject(i) );
                }

                pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, false);
            }
        }
    }
}

/*************************************************************************
|*
|* SFX-Aktivierung
|*
\************************************************************************/

void DrawDocShell::Activate( sal_Bool bMDI)
{
    if (bMDI)
    {
        ApplySlotFilter();
        mpDoc->StartOnlineSpelling();
    }
}

/*************************************************************************
|*
|* SFX-Deaktivierung
|*
\************************************************************************/

void DrawDocShell::Deactivate( sal_Bool )
{
}

/*************************************************************************
|*
|* SFX-Undomanager zurueckgeben
|*
\************************************************************************/

::svl::IUndoManager* DrawDocShell::GetUndoManager()
{
    return mpUndoManager;
}



/*************************************************************************
|*
|* Tabellenzeiger auffrischen
|*
\************************************************************************/

void DrawDocShell::UpdateTablePointers()
{
    PutItem( SvxColorTableItem( mpDoc->GetColorTable(), SID_COLOR_TABLE ) );
    PutItem( SvxGradientListItem( mpDoc->GetGradientList(), SID_GRADIENT_LIST ) );
    PutItem( SvxHatchListItem( mpDoc->GetHatchList(), SID_HATCH_LIST ) );
    PutItem( SvxBitmapListItem( mpDoc->GetBitmapList(), SID_BITMAP_LIST ) );
    PutItem( SvxDashListItem( mpDoc->GetDashList(), SID_DASH_LIST ) );
    PutItem( SvxLineEndListItem( mpDoc->GetLineEndList(), SID_LINEEND_LIST ) );

    UpdateFontList();
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawDocShell::CancelSearching()
{
    if( dynamic_cast<FuSearch*>( mxDocShellFunction.get() ) )
    {
        SetDocShellFunction(0);
    }
}

/*************************************************************************
|*
|*  den eingestellten SlotFilter anwenden
|*
\************************************************************************/

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
                pDispatcher->GetBindings()->InvalidateAll( sal_True );
        }

        pTestViewShell = SfxViewShell::GetNext( *pTestViewShell );
    }
}

void DrawDocShell::SetModified( sal_Bool bSet /* = sal_True */ )
{
    SfxObjectShell::SetModified( bSet );

    // #100237# change model state, too
    // #103182# only set the changed state if modification is enabled
    if( IsEnableSetModified() )
    {
        if ( mpDoc )
            mpDoc->NbcSetChanged( bSet );

        Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
    }
}

/*************************************************************************
|*
|* Callback fuer ExecuteSpellPopup()
|*
\************************************************************************/

// #91457# ExecuteSpellPopup now handled by DrawDocShell. This is necessary
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
