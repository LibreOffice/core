/*************************************************************************
 *
 *  $RCSfile: MasterPagesSelector.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:25:51 $
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

#ifndef SD_TASKPANE_CONTROLS_MASTER_PAGES_SELECTOR_HXX
#define SD_TASKPANE_CONTROLS_MASTER_PAGES_SELECTOR_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include "MasterPageContainer.hxx"

#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#include <sfx2/shell.hxx>
#include <vcl/image.hxx>
#include "glob.hxx"

#include <queue>

class MouseEvent;
class SdDrawDocument;
class SdPage;
class SfxModule;

namespace sd {
class DrawViewShell;
class TemplateEntry;
class TemplateDir;
class ViewShellBase;
}

namespace sd { namespace toolpanel { namespace controls {

class PreviewValueSet;

/** Base class of a menu that lets the user select from a list of
    templates or designs that are loaded from files.
*/
class MasterPagesSelector
    : public TreeNode,
      public SfxShell
{
public:
    static const int snSmallPreviewWidth;
    static const int snLargePreviewWidth;

    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDMASTERPAGESSELECTOR);

    MasterPagesSelector (
        TreeNode* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase);
    virtual ~MasterPagesSelector (void);

    virtual void LateInit (void);

    /** Return the height that this control needs to show all of its lines.
    */
    long GetRequiredHeight (int nWidth) const;

    /** The given master page, either the master page of a slide or a notes
        page, is cloned and inserted into mrDocument.  The necessary styles
        are copied as well.
    */
    static SdPage* AddMasterPage (
        SdDrawDocument* pTargetDocument,
        SdPage* pMasterPage,
        USHORT nInsertionIndex);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);
    virtual sal_Int32 GetMinimumWidth (void);

    void Execute (SfxRequest& rRequest);
    void GetState (SfxItemSet& rItemSet);

    void SetPreviewWidth (int nPreviewWidth);

    /** Update the selection of previews according to whatever
        influences them appart from mouse and keyboard.  If, for
        example, the current page of the main pane changes, then call
        this method at the CurrentMasterPagesSelector to select the
        previews of the master pages that are assigned to the new
        current page.

        The default implementation of this method ignores the call. This is
        used by e.g. the RecentMasterPagesSelector because it does not show
        the currently used master pages by default and thus is not
        influenced by its changes.
    */
    virtual void UpdateSelection (void);

    virtual void Fill (void) = 0;

    /** Make the selector empty.  This method clear the value set from any
        entries. Overload this method to add functionality, especially to
        destroy objects set as data items at the value set.
    */
    virtual void Clear (void);

    void    SetSmartHelpId( const SmartId& aId, SmartIdUpdateMode aMode = SMART_SET_SMART );

    /** Call this method to tell a selector object that the preview that
        belongs to the given page does not reflect the pages content
        anymore.  This method forwards the call to the variant that takes a
        value set index as argument so call that variant directly when the
        index is already present.
        @param pPage
            When NULL or a pointer to page is given that does not belong to
            the selector then the call is ignored.
    */
    virtual void InvalidatePreview (const SdPage* pPage) = 0;
    /** Invalidate the preview that is specified via an index into the value
        set.
    */
    void InvalidatePreview (USHORT nIndex);
    void InvalidateAllPreviews (void);

protected:
    SdDrawDocument& mrDocument;
    ::std::auto_ptr<PreviewValueSet> mpPageSet;
    int mnPreviewWidth;
    ViewShellBase& mrBase;
    /** Slot that is executed as default action when the left mouse button is
        clicked over a master page.
    */
    sal_uInt16 mnDefaultClickAction;
    /** Timer for scheduling updates of master page previews.
    */
    Timer maPreviewUpdateTimer;
    /** Pages with pointers in this queue have their previews updated
        eventually.  Filled by InvalidatePreview() and operated upon by
        UpdatePreviews().
    */
    ::std::queue<USHORT> maPreviewUpdateQueue;

    virtual SdPage* GetSelectedMasterPage (void);

    /** Assign the given master page to all slides of the document.
        @param pMasterPage
            The master page to assign to all slides.
    */
    void AssignMasterPageToAllSlides (SdPage* pMasterPage);

    /** Assign the given master page to all slides that are selected in a
        slide sorter that is displayed in the lef or center pane.  When both
        panes display a slide sorter then the one in the center pane is
        used.
    */
    void AssignMasterPageToSelectedSlides (SdPage* pMasterPage);

    void AddItemForPage (
        const String& sURL,
        const String& sPageName,
        SdPage* pMasterPage,
        Image aPreview,
        bool bCreatePreview = true);
    void AddItemForToken (
        MasterPageContainer::Token aToken,
        bool bCreatePreview = true);

    /** Ask the master page container for the previews of all items and set
        them to the value set.  Call this function when the size of the
        previews has changed.
    */
    virtual void UpdateAllPreviews (void) = 0;
    virtual void UpdatePreview (USHORT nIndex) = 0;

    DECL_LINK(ProcessPreviewUpdateRequest, Timer*);

protected:
    /** Assign the given master page to the list of pages.
        @param pMasterPage
            This master page will usually be a member of the list of all
            available master pages as provided by the MasterPageContainer.
        @param rPageList
            The pages to which to assign the master page.  These pages may
            be slides or master pages themselves.
    */
    virtual void AssignMasterPageToPageList (
        SdPage* pMasterPage,
        const ::std::vector<SdPage*>& rPageList);

private:
    DECL_LINK(ClickHandler, PreviewValueSet*);
    DECL_LINK(RightClickHandler, MouseEvent*);
    DECL_LINK(ContextMenuCallback, CommandEvent*);

    static void ProvideStyles (
        SdDrawDocument* pSourceDoc,
        SdDrawDocument* pTargetDoc,
        SdPage* pPage);

    /** Assign the given master page to the given page.
        @param pMasterPage
            In contrast to AssignMasterPageToPageList() this page is assumed
            to be in the target document, i.e. the same document that pPage
            is in.  The caller will usually call AddMasterPage() to create a
            clone of a master page in a another document to create it.
        @param rsBaseLayoutName
            The layout name of the given master page.  It is given so that
            it has not to be created on every call.  It could be generated
            from the given master page, though.
        @param pPage
            The page to which to assign the master page.  It can be a slide
            or a master page itself.
    */
    void AssignMasterPageToPage (
        SdPage* pMasterPage,
        const String& rsBaseLayoutName,
        SdPage* pPage);

    SdPage* ProvideMasterPage (
        SdPage* pMasterPage,
        const ::std::vector<SdPage*>& rPageList);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
