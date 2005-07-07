/*************************************************************************
 *
 *  $RCSfile: SlsClipboard.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:36:53 $
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
#ifndef SD_SLIDESORTER_CLIPBOARD
#define SD_SLIDESORTER_CLIPBOARD

#include "ViewClipboard.hxx"

#include <sal/types.h>
#include <tools/solar.h>
#include <svx/svdpage.hxx>

#include <set>

class SfxRequest;
class Window;

struct AcceptDropEvent;
class DropTargetHelper;
struct ExecuteDropEvent;
class Point;
class SdPage;
class Window;

namespace sd {
class Window;
}

namespace sd { namespace slidesorter { namespace model {
class PageDescriptor;
} } }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;

class Clipboard
    : public ViewClipboard
{
public:
    Clipboard (SlideSorterController& rController);
    ~Clipboard (void);

    void HandleSlotCall (SfxRequest& rRequest);

    void DoCut (::Window* pWindow = 0);
    void DoCopy (::Window* pWindow = 0);
    void DoPaste (::Window* pWindow = 0);
    void DoDelete (::Window* pWindow = 0);

    void StartDrag (
        const Point& rDragPt,
        ::Window* pWindow );

    void DragFinished (
        sal_Int8 nDropAction);

    sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND );

    sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND);

protected:
    virtual USHORT DetermineInsertPosition (
        const SdTransferable& rTransferable);

    virtual USHORT InsertSlides (
        const SdTransferable& rTransferable,
        USHORT nInsertPosition);

private:
    SlideSorterController& mrController;

    typedef ::std::vector<SdPage*> PageList;
    /** Remember the pages that are dragged to another document or to
        another place in the same document so that they can be removed after
        a move operation.
    */
    PageList maPagesToRemove;

    /** Remember the pages inserted from another document or another place
        in the same document so that they can be selected after the
        drag-and-drop operation is completed.
    */
    PageList maPagesToSelect;

    /** When pages are moved or copied then the selection of the slide
        sorter has to be updated.  This flag is used to remember whether the
        selection has to be updated or can stay as it is (FALSE).
    */
    bool mbUpdateSelectionPending;

    void CreateSlideTransferable (
        ::Window* pWindow,
        bool bDrag);

    /** Select the pages stored in the maPagesToSelect member.  The list in
        the member is cleared afterwards.
    */
    void SelectPages (void);

    /** Determine the position of where to insert the pages in the current
        transferable of the sd module.
        @param pWindow
            This window is used as parent for dialogs that have to be shown
            to the user.
        @return
            The index in the range [0,n] (both inclusive) with n the number
            of pages is returned.
    */
    sal_Int32 GetInsertionPosition (::Window* pWindow);

    /** Paste the pages of the transferable of the sd module at the given
        position.
        @param nInsertPosition
            The position at which to insert the pages.  The valid range is
            [0,n] (both inclusive) with n the number of pages in the
            document.
        @return
            The number of inserted pages is returned.
    */
    sal_Int32 PasteTransferable (sal_Int32 nInsertPosition);

    /** Select a range of pages of the model.  Typicall usage is the
        selection of newly inserted pages.
        @param nFirstIndex
            The index of the first page to select.
        @param nPageCount
            The number of pages to select.
    */
    void SelectPageRange (sal_Int32 nFirstIndex, sal_Int32 nPageCount);

    /** Return <TRUE/> when the current transferable in the current state of
        the slidesorter is acceptable to be pasted.  For this the
        transferable has to
        a) exist,
        b) contain one or more regular draw pages, no master pages.
        When master pages are involved, either in the transferable or in the
        slide sorter (by it displaying master pages) the drop of the
        transferable is not accepted.  The reason is the missing
        implementation of proper handling master pages copy-and-paste.
    */
    bool IsDropAccepted (void) const;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

