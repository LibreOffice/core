/*************************************************************************
 *
 *  $RCSfile: SlsClipboard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:13:17 $
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

#include "controller/SlsClipboard.hxx"

#include "SlideSorterViewShell.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsViewOverlay.hxx"
#include "view/SlsPageObject.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsSelectionFunction.hxx"
#include "SlsTransferable.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsFocusManager.hxx"

#include "SlideChangeChildWindow.hxx"
#include "Window.hxx"
#include "fupoor.hxx"
#include "fuslprms.hxx"
#include "fuslhide.hxx"
#include "fuzoom.hxx"
#include "fucushow.hxx"
#include "fusldlg.hxx"
#include "fuexpand.hxx"
#include "fusumry.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdxfer.hxx"
#include "sdmod.hxx"
#include "sddll.hxx"
#include "ins_paste.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svxids.hrc>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>

namespace sd { namespace slidesorter { namespace controller {


Clipboard::Clipboard (SlideSorterController& rController)
    : ViewClipboard (rController.GetView()),
      mrController (rController)
{
}




Clipboard::~Clipboard (void)
{
}




/** With the current implementation the forwarded calls to the current
    function will come back eventually to call the local Do(Cut|Copy|Paste)
    methods.  A shortcut is possible but would be an unclean hack.
*/
void Clipboard::HandleSlotCall (SfxRequest& rRequest)
{
    FuPoor* pCurrentFunction = mrController.GetViewShell().GetActualFunction();
    switch (rRequest.GetSlot())
    {
        case SID_CUT:
            if (pCurrentFunction != NULL)
                pCurrentFunction->DoCut();
            else
                DoCut();
            rRequest.Done();
            break;

        case SID_COPY:
            if (pCurrentFunction != NULL)
                pCurrentFunction->DoCopy();
            else
                DoCopy();
            rRequest.Done();
            break;

        case SID_PASTE:
            // Prevent redraws while inserting pages from the clipboard
            // because the intermediate inconsistent state might lead to
            // a crash.
            mrController.GetView().LockRedraw (TRUE);
            if (pCurrentFunction != NULL)
                pCurrentFunction->DoPaste();
            else
                DoPaste();
            mrController.MakeSelectionVisible();
            mrController.GetView().LockRedraw (FALSE);
            rRequest.Done();
            break;
    }
}




void Clipboard::DoCut (::Window* pWindow)
{
    if (mrController.GetModel().GetPageCount() > 1)
    {
        QueryBox aDialog (
            pWindow,
            WB_YES_NO,
            SdResId(STR_WARN_DEL_SEL_PAGES));
        if (aDialog.Execute() == RET_YES)
        {
            DoCopy (pWindow);
            mrController.DeleteSelectedPages();
        }
    }
}




void Clipboard::DoCopy (::Window* pWindow )
{
    CreateSlideTransferable( pWindow, FALSE );
}




void Clipboard::DoPaste (::Window* pWindow)
{
    SdTransferable* pClipTransferable = SD_MOD()->pTransferClip;

    if (pClipTransferable!=NULL && pClipTransferable->IsPageTransferable())
    {
        sal_Int32 nInsertPosition = 0;
        bool bMergeMasterPages = !pClipTransferable->HasSourceDoc (
            mrController.GetModel().GetDocument());

        // Determine the insertion position.  That is
        // a) When the focus indicator is visible, then before or after the
        // focused page, depending on user input to a dialog.
        // b) When there is a selection but no focus, then before the
        // selection.
        // c) After the last page when there is no selection and no focus.
        if (mrController.GetFocusManager().IsFocusShowing())
        {
            SdInsertPasteDlg aDialog (pWindow);
            if (aDialog.Execute() != RET_OK)
                return;

            nInsertPosition
                = mrController.GetFocusManager().GetFocusedPageIndex();
            if ( ! aDialog.IsInsertBefore())
                nInsertPosition ++;
        }
        else
        {
            model::SlideSorterModel::Enumeration aSelectedPages
                (mrController.GetModel().GetSelectedPagesEnumeration());
            if (aSelectedPages.HasMoreElements())
            {
                nInsertPosition
                    = aSelectedPages.GetNextElement().GetPage()->GetPageNum();
            }
            else
                nInsertPosition = mrController.GetModel().GetPageCount();
        }


        USHORT nInsertIndex = (USHORT)(nInsertPosition * 2 + 1);
        USHORT nInsertPgCnt;
        if (pClipTransferable->HasPageBookmarks())
        {
            const List& rBookmarkList = pClipTransferable->GetPageBookmarks();
            const ::vos::OGuard aGuard (Application::GetSolarMutex());

            nInsertPgCnt = (USHORT) rBookmarkList.Count();
            mrController.GetModel().GetDocument()->InsertBookmarkAsPage(
                const_cast<List*>(&rBookmarkList),
                NULL,
                FALSE,
                FALSE,
                nInsertIndex,
                FALSE,
                pClipTransferable->GetPageDocShell(),
                TRUE,
                bMergeMasterPages,
                FALSE);
        }
        else
        {
            SvEmbeddedObject* pObj = pClipTransferable->GetDocShell();
            DrawDocShell* pDataDocSh = (DrawDocShell*) pObj;
            SdDrawDocument* pDataDoc = pDataDocSh->GetDoc();

            if (pDataDoc!=NULL
                && pDataDoc->GetSdPageCount(PK_STANDARD))
            {
                const ::vos::OGuard aGuard (Application::GetSolarMutex());

                nInsertPgCnt = pDataDoc->GetSdPageCount( PK_STANDARD );
                mrController.GetModel().GetDocument()->InsertBookmarkAsPage(
                    NULL,
                    NULL,
                    FALSE,
                    FALSE,
                    nInsertIndex,
                    FALSE,
                    pDataDocSh,
                    TRUE,
                    bMergeMasterPages,
                    FALSE);
            }
        }
        /*AF
        SfxUInt16Item aItem (SID_PAGES_PER_ROW, nPagesPerRow);

        ( pDocSh->GetViewShell()
            ? pDocSh->GetViewShell()->GetViewFrame()
            : SfxViewFrame::Current()
            )->GetDispatcher()->Execute(
                SID_PAGES_PER_ROW,
                SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD,
                &aItem,
                0L,
                0L);
            */
        mrController.HandleModelChange();
    }
}




void Clipboard::CreateSlideTransferable (
    ::Window* pWindow,
    bool bDrag)
{
    List aBookmarkList;
    SdPage* pPage = NULL;

    // Insert all selected pages into a bookmark list and remember them in
    // maPagesToRemove for possible later removal.
    model::SlideSorterModel::Enumeration aSelectedPages
        (mrController.GetModel().GetSelectedPagesEnumeration());
    while (aSelectedPages.HasMoreElements())
    {
        model::PageDescriptor& rDescriptor (aSelectedPages.GetNextElement());
        aBookmarkList.Insert (
            new String(rDescriptor.GetPage()->GetName()),
            LIST_APPEND);
        maPagesToRemove.push_back (rDescriptor.GetPage());
    }

    if (aBookmarkList.Count() > 0)
    {
        mrController.GetView().BrkAction();
        SdDrawDocument* pDocument = mrController.GetModel().GetDocument();
        SdTransferable* pTransferable = new Transferable (
            pDocument, NULL, FALSE, &mrController.GetViewShell());

        if (bDrag)
            SD_MOD()->pTransferDrag = pTransferable;
        else
            SD_MOD()->pTransferClip = pTransferable;

        pDocument->CreatingDataObj (pTransferable);
        pTransferable->SetWorkDocument (
            static_cast<SdDrawDocument*>(
                mrController.GetView().GetAllMarkedModel()));
        pDocument->CreatingDataObj (NULL);
        TransferableObjectDescriptor aObjDesc;
        pTransferable->GetWorkDocument()->GetDocSh()
            ->FillTransferableObjectDescriptor (aObjDesc);

        if (pDocument->GetDocSh() != NULL)
            aObjDesc.maDisplayName = pDocument->GetDocSh()
                ->GetMedium()->GetURLObject().GetURLNoPass();

        ::Window* pActionWindow = pWindow;
        if (pActionWindow == NULL)
            pActionWindow = mrController.GetViewShell().GetActiveWindow();

        pTransferable->SetStartPos (pActionWindow->PixelToLogic(
            pActionWindow->GetPointerPosPixel()));
        pTransferable->SetObjectDescriptor (aObjDesc);
        pTransferable->SetPageBookmarks (aBookmarkList, !bDrag);

        for (void* p=aBookmarkList.First(); p!=NULL; p=aBookmarkList.Next())
            delete static_cast<String*>(p);

        if (bDrag)
        {
            pTransferable->SetView (&mrController.GetView());
            pTransferable->StartDrag (
                pActionWindow,
                DND_ACTION_COPY | DND_ACTION_MOVE );
        }
        else
            pTransferable->CopyToClipboard (pActionWindow);
    }
}




void Clipboard::StartDrag (
    const Point& rDragPoint,
    ::Window* pWindow)
{
    maPagesToRemove.clear();
    maPagesToSelect.clear();
    CreateSlideTransferable (pWindow, TRUE);
}




void Clipboard::DragFinished (sal_Int8 nDropAction)
{
    // Hide the substitution display and insertion indicator.
    mrController.GetView().GetOverlay().GetSubstitutionOverlay().Hide();
    mrController.GetView().GetOverlay().GetInsertionIndicatorOverlay().Hide();

    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if (pDragTransferable != NULL)
        pDragTransferable->SetView (NULL);

    PageSelector& rSelector = mrController.GetPageSelector();
    if ((nDropAction & DND_ACTION_MOVE) != 0)
    {
        // Remove the pages that have been moved to another place (possibly
        // in the same document.)
        rSelector.DeselectAllPages();
        PageList::iterator aDraggedPage;
        for (aDraggedPage=maPagesToRemove.begin();
             aDraggedPage!=maPagesToRemove.end();
             aDraggedPage++)
        {
            rSelector.SelectPage (*aDraggedPage);
        }
        mrController.DeleteSelectedPages ();
    }

    // Select the dropped pages.
    PageList::iterator aDroppedPage;
    rSelector.DeselectAllPages();
    for (aDroppedPage=maPagesToSelect.begin();
         aDroppedPage!=maPagesToSelect.end();
         aDroppedPage++)
    {
        rSelector.SelectPage (*aDroppedPage);
    }
}




sal_Int8 Clipboard::AcceptDrop (
    const AcceptDropEvent& rEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    sal_Int8 nResult = DND_ACTION_NONE;
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;

    if (pDragTransferable!=NULL
        && pDragTransferable->IsPageTransferable())
    {
        // Accept a drop.
        nResult = rEvent.mnAction;

        // Show the insertion marker and the substitution for a drop.
        Point aPosition = pTargetWindow->PixelToLogic (rEvent.maPosPixel);
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
        rOverlay.GetInsertionIndicatorOverlay().SetPosition (aPosition);
        rOverlay.GetInsertionIndicatorOverlay().Show();
        rOverlay.GetSubstitutionOverlay().SetPosition (aPosition);

        // Scroll the window when the mouse reaches the window border.
        mrController.GetScrollBarManager().AutoScroll (rEvent.maPosPixel);
    }

    return nResult;
}




sal_Int8 Clipboard::ExecuteDrop (
    const ExecuteDropEvent& rEvent,
    DropTargetHelper& rTargetHelper,
    ::sd::Window* pTargetWindow,
    USHORT nPage,
    USHORT nLayer)
{
    SdTransferable* pDragTransferable = SD_MOD()->pTransferDrag;
    sal_Int8 nResult = DND_ACTION_NONE;

    if (pDragTransferable != NULL && pDragTransferable->IsPageTransferable())
    {
        const Point aEventModelPosition (
            pTargetWindow->PixelToLogic (rEvent.maPosPixel));
        long int nXOffset = labs (pDragTransferable->GetStartPos().X()
            - aEventModelPosition.X());
        long int nYOffset = labs (pDragTransferable->GetStartPos().Y()
            - aEventModelPosition.Y());
        const bool bContinue =
            ( pDragTransferable->GetView() != &mrController.GetView() )
            || ( nXOffset >= 2 && nYOffset >= 2 );

        // Get insertion position and then turn off the insertion indicator.
        view::ViewOverlay& rOverlay (mrController.GetView().GetOverlay());
        rOverlay.GetInsertionIndicatorOverlay().SetPosition(
            aEventModelPosition);
        USHORT nIndex = DetermineInsertPosition (*pDragTransferable);
        OSL_TRACE ("Clipboard::AcceptDrop() called for index %d",
            nIndex);
        rOverlay.GetInsertionIndicatorOverlay().Hide();

        if (bContinue)
        {
            if (pDragTransferable->GetView() == &mrController.GetView()
                && rEvent.mnAction == DND_ACTION_MOVE)
            {
                // We are asked to move pages inside one view.  For this we
                // call MoveSelectedPages() which is faster than going the
                // generic way.

                // Remember to select the moved pages afterwards.
                maPagesToRemove.swap (maPagesToSelect);
                maPagesToRemove.clear();

                USHORT nSdrModelIndex;
                if (nIndex != SDRPAGE_NOTFOUND)
                    nSdrModelIndex = nIndex / 2 - 1;
                else
                    nSdrModelIndex = SDRPAGE_NOTFOUND;
                mrController.MoveSelectedPages (nSdrModelIndex);
                nResult = DND_ACTION_NONE;
            }
            else
            {
                // Handle a general drop operation.
                HandlePageDrop (*pDragTransferable);
                nResult = rEvent.mnAction;
            }
        }
    }

    return nResult;
}




USHORT Clipboard::DetermineInsertPosition (const SdTransferable& rTransferable)
{
    USHORT nInsertPosition = SDRPAGE_NOTFOUND;

    // Tell the model to move the dragged pages behind the one with the
    // index nInsertionIndex which first has to be transformed into an index
    // understandable by the document.
    view::InsertionIndicatorOverlay& rOverlay (
        mrController.GetView().GetOverlay().GetInsertionIndicatorOverlay());
    sal_Int32 nInsertionIndex (rOverlay.GetInsertionPageIndex());

    // The index returned by the overlay starts with 1 for the first slide.
    // This is now converted that to an SdModel index that also starts with 1.
    if (nInsertionIndex >= 0)
        nInsertPosition = (USHORT)nInsertionIndex * 2 + 1;

    return nInsertPosition;
}




USHORT Clipboard::InsertSlides (
    const SdTransferable& rTransferable,
    USHORT nInsertPosition)
{
    USHORT nInsertedPageCount = ViewClipboard::InsertSlides (
        rTransferable,
        nInsertPosition);

    // Remember the inserted pages so that they can be selected when the
    // operation is finished.
    int nDocumentIndex = nInsertPosition / 2 - 1;
    for (USHORT i=1; i<=nInsertedPageCount; i++)
    {
        model::PageDescriptor* pDescriptor
            = mrController.GetModel().GetPageDescriptor(nDocumentIndex + i);
        if (pDescriptor != NULL)
            maPagesToSelect.push_back (pDescriptor->GetPage());
    }
    return nInsertedPageCount;
}

#if 0
    SdDrawDocument* pDocument = mrController.GetModel().GetDocument();
    do
    {
        if (pDragTransferable == NULL)
            break;
        if ( ! pDragTransferable->IsPageTransferable())
            break;

        const Point aEventModelPosition (
            pTargetWindow->PixelToLogic (rEvent.maPosPixel));
        long int nXOffset = labs (pDragTransferable->GetStartPos().X()
            - aEventModelPosition.X());
        long int nYOffset = labs (pDragTransferable->GetStartPos().Y()
            - aEventModelPosition.Y());
        const bool bContinue =
            ( pDragTransferable->GetView() != &mrController.GetView() )
            || ( nXOffset >= 2 && nYOffset >= 2 );

        if ( ! bContinue)
            break;

        // Tell the model to move the dragged pages behind the one with the
        // index nInsertionIndex which first has to be transformed into an
        // index understandable by the document.
        view::InsertionIndicatorOverlay& rInsertionIndicatorOverlay (
            mrController.GetView().GetOverlay()
            .GetInsertionIndicatorOverlay());
        rInsertionIndicatorOverlay.SetPosition(aEventModelPosition);
        sal_Int32 nInsertionIndex
            = rInsertionIndicatorOverlay.GetInsertionPageIndex();
        if (nInsertionIndex < 0)
            break;
        USHORT nDocumentIndex = (USHORT)nInsertionIndex-1;

        if ((pDragTransferable->GetView() == &mrController.GetView())
            && (rEvent.mnAction == DND_ACTION_MOVE))
        {
            mrController.MoveSelectedPages (nDocumentIndex);
            nResult = DND_ACTION_NONE;
        }
        else
        {
            USHORT nInsertPgCnt;
            USHORT nInsertPos = (nDocumentIndex + 1) * 2 + 1;
            bool bMergeMasterPages = !pDragTransferable->HasSourceDoc(
                pDocument);

            if( pDragTransferable->HasPageBookmarks() )
            {
                const List& rBookmarkList
                    = pDragTransferable->GetPageBookmarks();

                nInsertPgCnt = (USHORT) rBookmarkList.Count();
                pDocument->InsertBookmarkAsPage (
                    const_cast< List* >( &rBookmarkList ),
                    NULL,
                    FALSE,
                    FALSE,
                    nInsertPos,
                    TRUE,
                    pDragTransferable->GetPageDocShell(),
                    TRUE,
                    bMergeMasterPages,
                    FALSE);
            }
            else
            {
                SvEmbeddedObject* pObj = pDragTransferable->GetDocShell();
                DrawDocShell* pDataDocShell;
                if (pDragTransferable->HasPageBookmarks())
                    pDataDocShell = pDragTransferable->GetPageDocShell();
                else
                    pDataDocShell = static_cast<DrawDocShell*>(pObj);
                SdDrawDocument* pDataDoc = pDataDocShell->GetDoc();

                nInsertPgCnt = pDataDoc->GetSdPageCount( PK_STANDARD );
                pDocument->InsertBookmarkAsPage (
                    NULL,
                    NULL,
                    FALSE,
                    FALSE,
                    nInsertPos,
                    TRUE,
                    pDataDocShell,
                    TRUE,
                    bMergeMasterPages,
                    FALSE);
            }

            PageSelector& rSelector = mrController.GetPageSelector();
            rSelector.DeselectAllPages();
            // select inserted pages
            for (USHORT i=1; i<=nInsertPgCnt; i++)
            {
                model::PageDescriptor* pDescriptor
                    = mrController.GetModel().GetPageDescriptor(
                        nDocumentIndex + i);

                if (pDescriptor != NULL)
                    rSelector.SelectPage (*pDescriptor);
            }

            // Update
            //af                ( (DragAndDropManagerShell*) pViewSh )->SetPagesPerRow( nPagesPerRow );
            nResult = rEvent.mnAction;
        }
    }
    while (false);

    return nResult;
}

#endif


} } } // end of namespace ::sd::slidesorter::controller

