/*************************************************************************
 *
 *  $RCSfile: docshell.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-21 16:11:43 $
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
#include <so3/clsids.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX
#include <sfx2/topfrm.hxx>
#endif

#include "app.hrc"
#include "app.hxx"
#include "strmname.h"
#include "stlpool.hxx"
#include "strings.hrc"
#include "docshell.hxx"
#include "sdview.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "res_bmp.hrc"
#include "fupoor.hxx"
#include "fusearch.hxx"
#include "viewshel.hxx"
#include "sdresid.hxx"
#include "fuslshow.hxx"
#include "preview.hxx"
#include "drawview.hxx"

#define POOL_BUFFER_SIZE                (USHORT)32768
#define BASIC_BUFFER_SIZE               (USHORT)8192
#define DOCUMENT_BUFFER_SIZE            (USHORT)32768


GraphicFilter* GetGrfFilter();


/*************************************************************************
|*
|* SFX-Slotmaps und -Definitionen
|*
\************************************************************************/
TYPEINIT1( SdDrawDocShell, SfxObjectShell );

#define SdDrawDocShell
#include "sdslots.hxx"


SFX_IMPL_INTERFACE(SdDrawDocShell, SfxObjectShell, SdResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION(SID_SEARCH_DLG);
}

SFX_IMPL_OBJECTFACTORY_LOD(SdDrawDocShell, simpress,
                           SvGlobalName(SO3_SIMPRESS_CLASSID_50), Sd)

/*************************************************************************
|*
|* Construct
|*
\************************************************************************/

void SdDrawDocShell::Construct()
{
    bInDestruction = FALSE;
    SetSlotFilter();     // setzt Filter zurueck
    SetShell(this);
}

/*************************************************************************
|*
|* Konstruktor 1
|*
\************************************************************************/

SdDrawDocShell::SdDrawDocShell(SfxObjectCreateMode eMode,
                               BOOL bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell(eMode),
    pPrinter(NULL),
    pViewShell(NULL),
    pDoc(NULL),
    pUndoManager(NULL),
    pFontList(NULL),
    pFuActual(NULL),
    bUIActive(FALSE),
    pProgress(NULL),
//  pStbMgr( NULL ),
    bSdDataObj(bDataObject),
    bOwnPrinter(FALSE),
    pDocStor(NULL),
    eDocType(eDocumentType)
{
//    pDoc = new SdDrawDocument(eDocType, this);
    Construct();
}

/*************************************************************************
|*
|* Konstruktor 2
|*
\************************************************************************/

SdDrawDocShell::SdDrawDocShell(SdDrawDocument* pDoc, SfxObjectCreateMode eMode,
                               BOOL bDataObject,
                               DocumentType eDocumentType) :
    SfxObjectShell(eMode),
    pPrinter(NULL),
    pViewShell(NULL),
    pDoc(pDoc),
    pUndoManager(NULL),
    pFontList(NULL),
    pFuActual(NULL),
    bUIActive(FALSE),
    pProgress(NULL),
//  pStbMgr( NULL ),
    bSdDataObj(bDataObject),
    bOwnPrinter(FALSE),
    pDocStor(NULL),
    eDocType(eDocumentType)
{
    Construct();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

__EXPORT  SdDrawDocShell::~SdDrawDocShell()
{
    bInDestruction = TRUE;
    delete pFuActual;
    pFuActual = NULL;

    delete pFontList;
    delete pUndoManager;

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

void SdDrawDocShell::GetState(SfxItemSet &rSet)
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

                if (pViewShell && pViewShell->GetSlideShow() &&
                    pViewShell->GetSlideShow()->IsInputLocked())
                {
                    // Es laeuft ein Effekt in der SlideShow
                    bDisabled = TRUE;
                }
                else if (pViewShell && pViewShell->GetView()->ISA(SdDrawView) &&
                         ((SdDrawView*) pViewShell->GetView())->GetSlideShow() &&
                         ((SdDrawView*) pViewShell->GetView())->GetSlideShow()->IsInputLocked())
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

                    SfxChildWindow* pPreviewChildWindow = pFrame->GetChildWindow(SdPreviewChildWindow::GetChildWindowId());
                    SdPreviewWin*   pPreviewWin = (SdPreviewWin*) ( pPreviewChildWindow ? pPreviewChildWindow->GetWindow() : NULL );
                    FuSlideShow*    pShow = pPreviewWin ? pPreviewWin->GetSlideShow() : NULL;

                    if (pShow && pShow->IsInputLocked())
                    {
                        // Es laeuft ein Effekt in der Preview
                        bDisabled = TRUE;
                    }
                }

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

/*************************************************************************
|*
|* SFX-Aktivierung
|*
\************************************************************************/

void SdDrawDocShell::Activate( BOOL bMDI)
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

void SdDrawDocShell::Deactivate( BOOL )
{
}

/*************************************************************************
|*
|* SFX-Undomanager zurueckgeben
|*
\************************************************************************/

SfxUndoManager* SdDrawDocShell::GetUndoManager()
{
    return pUndoManager;
}



/*************************************************************************
|*
|* Tabellenzeiger auffrischen
|*
\************************************************************************/

void SdDrawDocShell::UpdateTablePointers()
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

void SdDrawDocShell::CancelSearching()
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

void SdDrawDocShell::ApplySlotFilter() const
{
    SfxViewFrame* pFrame = pViewShell ? pViewShell->GetFrame() : GetFrame();

    if( !pFrame )
        pFrame = SfxViewFrame::GetFirst( this );
    DBG_ASSERT( pFrame, "kein ViewFrame" );

    SfxDispatcher* pDispatcher = pFrame->GetDispatcher();

    if( pFilterSIDs )
        pDispatcher->SetSlotFilter(bFilterEnable, nFilterCount, pFilterSIDs);
    else
        pDispatcher->SetSlotFilter();

    if( pFrame )
        pFrame->GetBindings().InvalidateAll(TRUE);
}


/*************************************************************************
|*
|* Document-Stream herausgeben (fuer load-on-demand Graphiken)
|*
\************************************************************************/

SvStream* SdDrawDocShell::GetDocumentStream(FASTBOOL& rbDeleteAfterUse)
{
    rbDeleteAfterUse = FALSE;
    SvStorage* pStor = GetStorage();

    if (pStor && !pDocStor)
    {
        if (pStor->IsStream(pStarDrawDoc))
        {
            BOOL bOK = pStor->Rename(pStarDrawDoc, pStarDrawDoc3);
            DBG_ASSERT(bOK, "Umbenennung des Streams gescheitert");
        }

        xDocStream =  pStor->OpenStream(pStarDrawDoc3, STREAM_READ | STREAM_WRITE | STREAM_TRUNC);
        xDocStream->SetVersion(pStor->GetVersion());
        xDocStream->SetKey( pStor->GetKey() ); // Passwort setzen
        pDocStor = pStor;
    }

    return xDocStream;
}

void SdDrawDocShell::SetModified( BOOL bSet /* = TRUE */ )
{
    SfxInPlaceObject::SetModified( bSet );

    Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
}

Window* SdDrawDocShell::GetWindow() const
{
    SfxViewFrame* pFrame = GetFrame();
    if( pFrame == NULL )
        pFrame = SfxViewFrame::GetFirst( this );

    if( pFrame )
        return &(pFrame->GetWindow());
    else
    {
        DBG_ASSERT( 0, "No active window for SdDrawDocShell found! (next gpf is caused by this assertion)" );
        return NULL;
    }
}


