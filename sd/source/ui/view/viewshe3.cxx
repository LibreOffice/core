/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewshe3.cxx,v $
 *
 *  $Revision: 1.56 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:00:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "ViewShell.hxx"
#include "GraphicViewShell.hxx"
#include "GraphicViewShellBase.hxx"

#include <sfx2/viewfrm.hxx>

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#include <utility>
#include <vector>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "sdabstdlg.hxx"

#include "fupoor.hxx"

#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SVX_PRTQRY_HXX
#include <svx/prtqry.hxx>
#endif

#ifndef _SVDOPAGE_HXX //autogen
#include <svx/svdopage.hxx>
#endif

#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SVDETC_HXX //autogen
#include <svx/svdetc.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#ifndef _SFX_MISCCFG_HXX
#include <svtools/misccfg.hxx>
#endif

#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _SD_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef SD_WINDOW_UPDATER_HXX
#include "WindowUpdater.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_OUTLINE_VIEW_SHELL_HXX
#include "OutlineViewShell.hxx"
#endif
#ifndef SD_SLIDE_VIEW_SHELL_HXX
#include "SlideViewShell.hxx"
#endif
#include "TaskPaneViewShell.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#include "sdattr.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "unoaprms.hxx"                 // Undo-Action
#include "sdundogr.hxx"                 // Undo Gruppe
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "DrawDocShell.hxx"
#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif
#include "optsitem.hxx"
#include "sdresid.hxx"

// #96090#
#ifndef _SVXIDS_HXX
#include <svx/svxids.hrc>
#endif

#ifndef _B3D_BASE3D_HXX
#include "goodies/base3d.hxx"
#endif
#include <sfx2/request.hxx>
#include <svtools/aeitem.hxx>
#include <basic/sbstar.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

namespace sd {

/*************************************************************************
|*
|* Status (Enabled/Disabled) von Menue-SfxSlots setzen
|*
\************************************************************************/

void  ViewShell::GetMenuState( SfxItemSet &rSet )
{
    if( SFX_ITEM_AVAILABLE == rSet.GetItemState( SID_STYLE_FAMILY ) )
    {
        UINT16 nFamily = (UINT16)GetDocSh()->GetStyleFamily();

        SdrView* pDrView = GetDrawView();

        if( pDrView->AreObjectsMarked() )
        {
            SfxStyleSheet* pStyleSheet = pDrView->GetStyleSheet();
            if( pStyleSheet )
            {
                if (pStyleSheet->GetFamily() == SD_STYLE_FAMILY_MASTERPAGE)
                    pStyleSheet = ((SdStyleSheet*)pStyleSheet)->GetPseudoStyleSheet();

                if( pStyleSheet )
                {
                    SfxStyleFamily eFamily = pStyleSheet->GetFamily();
                    if(eFamily == SD_STYLE_FAMILY_GRAPHICS)
                        nFamily = 2;
                    else if(eFamily == SD_STYLE_FAMILY_CELL )
                        nFamily = 3;
                    else // SD_STYLE_FAMILY_PSEUDO
                        nFamily = 5;

                    GetDocSh()->SetStyleFamily(nFamily);
                }
            }
        }
        rSet.Put(SfxUInt16Item(SID_STYLE_FAMILY, nFamily ));
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_GETUNDOSTRINGS))
    {
        ImpGetUndoStrings(rSet);
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_GETREDOSTRINGS))
    {
        ImpGetRedoStrings(rSet);
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_UNDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(FALSE);

        if(pUndoManager)
        {
            if(pUndoManager->GetUndoActionCount() != 0)
            {
                bActivate = TRUE;
            }
        }

        if(bActivate)
        {
            // #87229# Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1072 ff.
            String aTmp(ResId(STR_UNDO, *SFX_APP()->GetSfxResManager()));
            aTmp += pUndoManager->GetUndoActionComment(0);
            rSet.Put(SfxStringItem(SID_UNDO, aTmp));
        }
        else
        {
            rSet.DisableItem(SID_UNDO);
        }
    }

    // #96090#
    if(SFX_ITEM_AVAILABLE == rSet.GetItemState(SID_REDO))
    {
        SfxUndoManager* pUndoManager = ImpGetUndoManager();
        sal_Bool bActivate(FALSE);

        if(pUndoManager)
        {
            if(pUndoManager->GetRedoActionCount() != 0)
            {
                bActivate = TRUE;
            }
        }

        if(bActivate)
        {
            // #87229# Set the necessary string like in
            // sfx2/source/view/viewfrm.cxx ver 1.23 ln 1081 ff.
            String aTmp(ResId(STR_REDO, *SFX_APP()->GetSfxResManager()));
            aTmp += pUndoManager->GetRedoActionComment(0);
            rSet.Put(SfxStringItem(SID_REDO, aTmp));
        }
        else
        {
            rSet.DisableItem(SID_REDO);
        }
    }
}




/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* ViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage)
{
    USHORT nSId = rRequest.GetSlot();
    SdDrawDocument* pDocument = GetDoc();
    SdrLayerAdmin& rLayerAdmin = pDocument->GetLayerAdmin();
    BYTE aBckgrnd = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRND)), FALSE);
    BYTE aBckgrndObj = rLayerAdmin.GetLayerID(String(SdResId(STR_LAYER_BCKGRNDOBJ)), FALSE);
    SetOfByte aVisibleLayers;
    // Determine the page from which to copy some values, such as layers,
    // size, master page, to the new page.  This is usually the given page.
    // When the given page is NULL then use the first page of the document.
    SdPage* pTemplatePage = pPage;
    if (pTemplatePage == NULL)
        if (pDocument->GetSdPage(0, ePageKind) > 0)
            pTemplatePage = pDocument->GetSdPage(0, ePageKind);
    if (pTemplatePage != NULL && pTemplatePage->TRG_HasMasterPage())
        aVisibleLayers = pTemplatePage->TRG_GetMasterPageVisibleLayers();
    else
        aVisibleLayers.SetAll();

    String aStandardPageName;
    String aNotesPageName;
    AutoLayout eStandardLayout (AUTOLAYOUT_NONE);
    AutoLayout eNotesLayout (AUTOLAYOUT_NOTES);
    BOOL bIsPageBack = aVisibleLayers.IsSet(aBckgrnd);
    BOOL bIsPageObj = aVisibleLayers.IsSet(aBckgrndObj);

    // 1. Process the arguments.
    const SfxItemSet* pArgs = rRequest.GetArgs();
    if (! pArgs)
    {
        // Make the layout menu visible in the tool pane.
        framework::FrameworkHelper::Instance(GetViewShellBase())->RequestTaskPanel(
            framework::FrameworkHelper::msLayoutTaskPanelURL);

        // AutoLayouts muessen fertig sein
        pDocument->StopWorkStartupDelay();

        // Use the layouts of the previous page and notes page as template.
        if (pTemplatePage != NULL)
        {
            eStandardLayout = pTemplatePage->GetAutoLayout();
            SdPage* pNotesTemplatePage = static_cast<SdPage*>(pDocument->GetPage(pTemplatePage->GetPageNum()+1));
            if (pNotesTemplatePage != NULL)
                eNotesLayout = pNotesTemplatePage->GetAutoLayout();
        }
    }
    else if (pArgs->Count () != 4)
    {
        Cancel();

        if(HasCurrentFunction(SID_BEZIER_EDIT) )
            GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

        StarBASIC::FatalError (SbERR_WRONG_ARGS);
        rRequest.Ignore ();
        return NULL;
    }
    else
    {
        // AutoLayouts muessen fertig sein
        pDocument->StopWorkStartupDelay();

        SFX_REQUEST_ARG (rRequest, pPageName, SfxStringItem, ID_VAL_PAGENAME, FALSE);
        SFX_REQUEST_ARG (rRequest, pLayout, SfxUInt32Item, ID_VAL_WHATLAYOUT, FALSE);
        SFX_REQUEST_ARG (rRequest, pIsPageBack, SfxBoolItem, ID_VAL_ISPAGEBACK, FALSE);
        SFX_REQUEST_ARG (rRequest, pIsPageObj, SfxBoolItem, ID_VAL_ISPAGEOBJ, FALSE);

        if (CHECK_RANGE (AUTOLAYOUT__START, (AutoLayout) pLayout->GetValue (), AUTOLAYOUT__END))
        {
            if (ePageKind == PK_NOTES)
            {
                aNotesPageName = pPageName->GetValue ();
                eNotesLayout   = (AutoLayout) pLayout->GetValue ();
            }
            else
            {
                aStandardPageName = pPageName->GetValue ();
                eStandardLayout   = (AutoLayout) pLayout->GetValue ();
            }

            bIsPageBack = pIsPageBack->GetValue ();
            bIsPageObj  = pIsPageObj->GetValue ();
        }
        else
        {
            Cancel();

            if(HasCurrentFunction( SID_BEZIER_EDIT ) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

            StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);
            rRequest.Ignore ();
            return NULL;
        }
    }

    // 2. Create a new page or duplicate an existing one.
    View* pDrView = GetView();
    pDrView->BegUndo( String( SdResId(STR_INSERTPAGE) ) );

    USHORT nNewPageIndex = 0xffff;
    switch (nSId)
    {
        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        case SID_INSERT_MASTER_PAGE:
            // There are three cases.  a) pPage is not NULL: we use it as a
            // template and create a new slide behind it. b) pPage is NULL
            // but the document is not empty: we use the first slide/notes
            // page as template, create a new slide after it and move it
            // then to the head of the document. c) pPage is NULL and the
            // document is empty: We use CreateFirstPages to create the
            // first page of the document.
            if (pPage == NULL)
                if (pTemplatePage == NULL)
                {
                    pDocument->CreateFirstPages();
                    nNewPageIndex = 0;
                }
                else
                {
                    // Create a new page with the first page as template and
                    // insert it after the first page.
                    nNewPageIndex = pDocument->CreatePage (
                        pTemplatePage,
                        ePageKind,
                        aStandardPageName,
                        aNotesPageName,
                        eStandardLayout,
                        eNotesLayout,
                        bIsPageBack,
                        bIsPageObj);
                    // Select exactly the new page.
                    USHORT nPageCount (pDocument->GetSdPageCount(ePageKind));
                    for (USHORT i=0; i<nPageCount; i++)
                    {
                        pDocument->GetSdPage(i, PK_STANDARD)->SetSelected(
                            i == nNewPageIndex);
                        pDocument->GetSdPage(i, PK_NOTES)->SetSelected(
                            i == nNewPageIndex);
                    }
                    // Move the selected page to the head of the document
                    pDocument->MovePages ((USHORT)-1);
                    nNewPageIndex = 0;
                }
            else
                nNewPageIndex = pDocument->CreatePage (
                    pPage,
                    ePageKind,
                    aStandardPageName,
                    aNotesPageName,
                    eStandardLayout,
                    eNotesLayout,
                    bIsPageBack,
                    bIsPageObj);
            break;

        case SID_DUPLICATE_PAGE:
            // Duplication makes no sense when pPage is NULL.
            if (pPage != NULL)
                nNewPageIndex = pDocument->DuplicatePage (
                    pPage,
                    ePageKind,
                    aStandardPageName,
                    aNotesPageName,
                    eStandardLayout,
                    eNotesLayout,
                    bIsPageBack,
                    bIsPageObj);
            break;

        default:
            DBG_WARNING("wrong slot id given to CreateOrDuplicatePage");
            // Try to handle another slot id gracefully.
    }
    SdPage* pNewPage = 0;
    if(nNewPageIndex != 0xffff)
    {
        pNewPage = pDocument->GetSdPage(nNewPageIndex, PK_STANDARD);
        pDrView->AddUndo(pDocument->GetSdrUndoFactory().CreateUndoNewPage(*pNewPage));
        pDrView->AddUndo(pDocument->GetSdrUndoFactory().CreateUndoNewPage(*pDocument->GetSdPage (nNewPageIndex, PK_NOTES)));
    }

    pDrView->EndUndo();

    return pNewPage;
}


} // end of namespace sd
