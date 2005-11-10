/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docshell.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-10 15:47:32 $
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

#include "DrawDocShell.hxx"

#define ITEMID_FONTLIST                 SID_ATTR_CHAR_FONTLIST
#define ITEMID_COLOR_TABLE              SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST            SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST               SID_HATCH_LIST
#define ITEMID_BITMAP_LIST              SID_BITMAP_LIST
#define ITEMID_DASH_LIST                SID_DASH_LIST
#define ITEMID_LINEEND_LIST             SID_LINEEND_LIST
#define ITEMID_SEARCH                   SID_SEARCH_ITEM

#ifndef _PSTM_HXX
#include <tools/pstm.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <sfx2/docfac.hxx>
#include <sfx2/objface.hxx>

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVX_SVXIFACT_HXX
#include <svx/svxifact.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_DOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _B3D_BASE3D_HXX
#include "goodies/base3d.hxx"
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX
#include <svx/srchdlg.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <sot/clsids.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX
#include <sfx2/topfrm.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _SFXVISIBILITYITEM_HXX
#include <svtools/visitem.hxx>
#endif

#include <sfx2/fcontnr.hxx>

#include "app.hrc"
#include "app.hxx"
#include "strmname.h"
#include "stlpool.hxx"
#include "strings.hrc"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "res_bmp.hrc"
#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif
#ifndef SD_FU_SEARCH_HXX
#include "fusearch.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#include "sdresid.hxx"
#ifndef SD_FU_SLIDE_SHOW_DLG_HXX
#include "slideshow.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SD_FRAMW_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "unomodel.hxx"
#include "formatclipboard.hxx"

using namespace sd;
#define DrawDocShell
#include "sdslots.hxx"

SFX_IMPL_INTERFACE(DrawDocShell, SfxObjectShell, SdResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
        SFX_CHILDWINDOW_REGISTRATION(SID_HYPERLINK_INSERT);
}


namespace sd {

#define POOL_BUFFER_SIZE                (USHORT)32768
#define BASIC_BUFFER_SIZE               (USHORT)8192
#define DOCUMENT_BUFFER_SIZE            (USHORT)32768


GraphicFilter* GetGrfFilter();

SfxProgress* DrawDocShell::mpSpecialProgress = NULL;
Link*        DrawDocShell::mpSpecialProgressHdl = NULL;

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

void DrawDocShell::Construct()
{
    bInDestruction = FALSE;
    SetSlotFilter();     // setzt Filter zurueck

    pDoc = new SdDrawDocument(eDocType, this);
    SetModel( new SdXImpressDocument( this ) );
    SetPool( &pDoc->GetItemPool() );
    pUndoManager = new SfxUndoManager;
    UpdateTablePointers();
    SetStyleFamily(5);       //CL: eigentlich SFX_STYLE_FAMILY_PSEUDO
}

/*************************************************************************
|*
|* Konstruktor 1
|*
\************************************************************************/

DrawDocShell::DrawDocShell(SfxObjectCreateMode eMode,
                               BOOL bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell(eMode),
    pPrinter(NULL),
    pViewShell(NULL),
    pDoc(NULL),
    pUndoManager(NULL),
    pFontList(NULL),
    pFuActual(NULL),
    //bUIActive(FALSE),
    pFormatClipboard(new SdFormatClipboard()),
    pProgress(NULL),
//  pStbMgr( NULL ),
    bSdDataObj(bDataObject),
    bOwnPrinter(FALSE),
    eDocType(eDocumentType),
    mbNewDocument( sal_True )
{
//    pDoc = new SdDrawDocument(eDocType, this);
    Construct();
}

/*************************************************************************
|*
|* Konstruktor 2
|*
\************************************************************************/

DrawDocShell::DrawDocShell(SdDrawDocument* pDoc, SfxObjectCreateMode eMode,
                               BOOL bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell(eMode),
    pPrinter(NULL),
    pViewShell(NULL),
    pDoc(pDoc),
    pUndoManager(NULL),
    pFontList(NULL),
    pFuActual(NULL),
    //bUIActive(FALSE),
    pFormatClipboard(new SdFormatClipboard()),
    pProgress(NULL),
//  pStbMgr( NULL ),
    bSdDataObj(bDataObject),
    bOwnPrinter(FALSE),
    eDocType(eDocumentType),
    mbNewDocument( sal_True )
{
    Construct();
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

    bInDestruction = TRUE;
    delete pFuActual;
    pFuActual = NULL;

    delete pFontList;
    delete pUndoManager;
    if(pFormatClipboard)
        delete pFormatClipboard;

    if (bOwnPrinter)
        delete pPrinter;

    delete pDoc;

    // damit der Navigator das Verschwinden des Dokuments mitbekommt
    SfxBoolItem     aItem(SID_NAVIGATOR_INIT, TRUE);
    SfxViewFrame*   pFrame = pViewShell ? pViewShell->GetFrame() : GetFrame();

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
    USHORT nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich(nWhich)
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
                BOOL bDisabled = FALSE;
/* with new slide show, closing document should always be possible
                if (pViewShell && pViewShell->GetSlideShow() &&
                    pViewShell->GetSlideShow()->IsInputLocked())
                {
                    // Es laeuft ein Effekt in der Slideshow
                    bDisabled = TRUE;
                }
                else if (pViewShell && pViewShell->GetView() && pViewShell->GetView()->ISA(DrawView) &&
                         static_cast<DrawView*>(pViewShell->GetView())->GetSlideShow() &&
                    static_cast<DrawView*>(pViewShell->GetView())->GetSlideShow()->IsInputLocked())
                {
                    // Es laeuft ein Effekt auf dem Zeichentisch
                    bDisabled = TRUE;
                }
                else
                {
                    SfxViewFrame* pFrame = pViewShell ? pViewShell->GetFrame() : GetFrame();

                    if( !pFrame )
                        pFrame = SfxViewFrame::GetFirst( this );
                    DBG_ASSERT( pFrame, "kein ViewFrame" );

                    SfxChildWindow* pPreviewChildWindow = pFrame->GetChildWindow(PreviewChildWindow::GetChildWindowId());
                    PreviewWindow* pPreviewWin = static_cast<PreviewWindow*>(
                        pPreviewChildWindow
                        ? pPreviewChildWindow->GetWindow()
                        : NULL);
                    FuSlideShow* pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

                    if (pShow && pShow->IsInputLocked())
                    {
                        // Es laeuft ein Effekt in der Preview
                        bDisabled = TRUE;
                    }
                }
*/
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
                UINT16 nOpt = SEARCH_OPTIONS_SEARCH      |
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

    SfxViewFrame* pFrame = SFX_APP()->GetViewFrame();

    if (pFrame)
    {
        if (rSet.GetItemState(SID_RELOAD) != SFX_ITEM_UNKNOWN)
        {
            pFrame->GetSlotState(SID_RELOAD,
                                 pFrame->GetInterface(), &rSet);
        }
    }
}

void DrawDocShell::InPlaceActivate( BOOL bActive )
{
    if( !bActive )
    {
        FrameView* pFrameView = NULL;
        List* pFrameViewList = pDoc->GetFrameViewList();

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
            SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, 0, false);

            while (pSfxViewFrame)
            {
                // Anzahl FrameViews ermitteln
                pSfxViewSh = pSfxViewFrame->GetViewShell();
                pViewSh = PTR_CAST( ViewShell, pSfxViewSh );

                if ( pViewSh && pViewSh->GetFrameView() )
                {
                    pViewSh->WriteFrameViewData();
                    pFrameViewList->Insert( new FrameView( pDoc, pViewSh->GetFrameView() ) );
                }

                pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, 0, false);
            }
        }
    }

    SfxObjectShell::InPlaceActivate( bActive );

    if( bActive )
    {
        FrameView* pFrameView = NULL;
        List* pFrameViewList = pDoc->GetFrameViewList();

        DBG_ASSERT( pFrameViewList, "No FrameViewList?" );
        if( pFrameViewList )
        {
            ViewShell* pViewSh = NULL;
            SfxViewShell* pSfxViewSh = NULL;
            SfxViewFrame* pSfxViewFrame = SfxViewFrame::GetFirst(this, 0,false);

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

                pSfxViewFrame = SfxViewFrame::GetNext(*pSfxViewFrame, this, 0,false);
            }
        }
    }
}

/*************************************************************************
|*
|* SFX-Aktivierung
|*
\************************************************************************/

void DrawDocShell::Activate( BOOL bMDI)
{
    if (bMDI)
    {
        ApplySlotFilter();
        pDoc->StartOnlineSpelling();
    }
}

/*************************************************************************
|*
|* SFX-Deaktivierung
|*
\************************************************************************/

void DrawDocShell::Deactivate( BOOL )
{
}

/*************************************************************************
|*
|* SFX-Undomanager zurueckgeben
|*
\************************************************************************/

SfxUndoManager* DrawDocShell::GetUndoManager()
{
    return pUndoManager;
}



/*************************************************************************
|*
|* Tabellenzeiger auffrischen
|*
\************************************************************************/

void DrawDocShell::UpdateTablePointers()
{
    PutItem( SvxColorTableItem( pDoc->GetColorTable() ) );
    PutItem( SvxGradientListItem( pDoc->GetGradientList() ) );
    PutItem( SvxHatchListItem( pDoc->GetHatchList() ) );
    PutItem( SvxBitmapListItem( pDoc->GetBitmapList() ) );
    PutItem( SvxDashListItem( pDoc->GetDashList() ) );
    PutItem( SvxLineEndListItem( pDoc->GetLineEndList() ) );

    delete pFontList;
    pFontList = new FontList( GetPrinter(TRUE), Application::GetDefaultDevice(), FALSE );
    SvxFontListItem aFontListItem( pFontList );
    PutItem( aFontListItem );
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void DrawDocShell::CancelSearching()
{
    if ( pFuActual && pFuActual->ISA(FuSearch) )
    {
        delete pFuActual;
        pFuActual = NULL;
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

            if( pFilterSIDs )
                pDispatcher->SetSlotFilter( bFilterEnable, nFilterCount, pFilterSIDs );
            else
                pDispatcher->SetSlotFilter();

            if( pDispatcher->GetBindings() )
                pDispatcher->GetBindings()->InvalidateAll( TRUE );
        }

        pTestViewShell = SfxViewShell::GetNext( *pTestViewShell );
    }
}


void DrawDocShell::SetModified( BOOL bSet /* = TRUE */ )
{
    SfxObjectShell::SetModified( bSet );

    // #100237# change model state, too
    // #103182# only set the changed state if modification is enabled
    if( IsEnableSetModified() && pDoc )
        pDoc->NbcSetChanged( bSet );

    Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
}

::Window* DrawDocShell::GetWindow() const
{
    SfxViewFrame* pFrame = GetFrame();
    if( pFrame == NULL )
        pFrame = SfxViewFrame::GetFirst( this );

    if( pFrame )
        return &(pFrame->GetWindow());
    else
    {
        DBG_ASSERT( 0, "No active window for DrawDocShell found! (next gpf is caused by this assertion)" );
        return NULL;
    }
}

/*************************************************************************
|*
|* Callback fuer ExecuteSpellPopup()
|*
\************************************************************************/

// #91457# ExecuteSpellPopup now handled by DrawDocShell. This is necessary
// to get hands on the outliner and the text object.
#ifndef SVX_LIGHT
IMPL_LINK(DrawDocShell, OnlineSpellCallback, SpellCallbackInfo*, pInfo)
{
    SdrObject* pObj = NULL;
    SdrOutliner* pOutl = NULL;

    if(GetViewShell())
    {
        pOutl = GetViewShell()->GetView()->GetTextEditOutliner();
        pObj = GetViewShell()->GetView()->GetTextEditObject();
    }

    pDoc->ImpOnlineSpellCallback(pInfo, pObj, pOutl);
    return(0);
}
#endif // !SVX_LIGHT

} // end of namespace sd
