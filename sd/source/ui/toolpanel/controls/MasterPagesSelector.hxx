/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterPagesSelector.hxx,v $
 * $Revision: 1.13 $
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

#ifndef SD_TASKPANE_CONTROLS_MASTER_PAGES_SELECTOR_HXX
#define SD_TASKPANE_CONTROLS_MASTER_PAGES_SELECTOR_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include "MasterPageContainer.hxx"
#include "SlideSorterViewShell.hxx"

#include "pres.hxx"
#include <sfx2/shell.hxx>
#include <vcl/image.hxx>
#include "glob.hxx"
#include <osl/mutex.hxx>

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
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDMASTERPAGESSELECTOR)

    MasterPagesSelector (
        TreeNode* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer);
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

    void FillPageSet (void);

    /** Make the selector empty.  This method clear the value set from any
        entries. Overload this method to add functionality, especially to
        destroy objects set as data items at the value set.
    */
    void ClearPageSet (void);

    void SetSmartHelpId( const SmartId& aId, SmartIdUpdateMode aMode = SMART_SET_SMART );

    /** Mark the preview that belongs to the given index as not up-to-date
        anymore with respect to page content or preview size.
        The implementation of this method will either sunchronously or
        asynchronously call UpdatePreview().
        @param nIndex
            Index into the value set control that is used for displaying the
            previews.
    */
    void InvalidatePreview (MasterPageContainer::Token aToken);
    void InvalidatePreview (const SdPage* pPage);

    /** Mark all previews as no being up-to-date anymore.  This method is
        typically called when the size of the previews has changed.
    */
    void InvalidateAllPreviews (void);

    void UpdateAllPreviews (void);

protected:
    mutable ::osl::Mutex maMutex;
    ::boost::shared_ptr<MasterPageContainer> mpContainer;

    SdDrawDocument& mrDocument;
    ::std::auto_ptr<PreviewValueSet> mpPageSet;
    bool mbSmallPreviewSize;
    ViewShellBase& mrBase;
    /** Slot that is executed as default action when the left mouse button is
        clicked over a master page.
    */
    sal_uInt16 mnDefaultClickAction;
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

    virtual void AssignMasterPageToPageList (
        SdPage* pMasterPage,
        const ::sd::slidesorter::SharedPageSelection& rPageList);

    virtual void NotifyContainerChangeEvent (const MasterPageContainerChangeEvent& rEvent);

    typedef ::std::pair<int, MasterPageContainer::Token> UserData;
    UserData* CreateUserData (int nIndex, MasterPageContainer::Token aToken) const;
    UserData* GetUserData (int nIndex) const;
    void SetUserData (int nIndex, UserData* pData);

    virtual sal_Int32 GetIndexForToken (MasterPageContainer::Token aToken) const;
    typedef ::std::vector<MasterPageContainer::Token> ItemList;
    void UpdateItemList (::std::auto_ptr<ItemList> pList);
    void Clear (void);
    /** Invalidate the specified item so that on the next Fill() this item
        is updated.
    */
    void InvalidateItem (MasterPageContainer::Token aToken);

    // For every item in the ValueSet we store its associated token.  This
    // allows a faster access and easier change tracking.
    ItemList maCurrentItemList;
    typedef ::std::map<MasterPageContainer::Token,sal_Int32> TokenToValueSetIndex;
    TokenToValueSetIndex maTokenToValueSetIndex;

    ItemList maLockedMasterPages;
    /** Lock master pages in the given list and release locks that where
        previously aquired.
    */
    void UpdateLocks (const ItemList& rItemList);

    void Fill (void);
    virtual void Fill (ItemList& rItemList) = 0;

private:
    /** The offset between ValueSet index and MasterPageContainer::Token
        last seen.  This value is used heuristically to speed up the lookup
        of an index for a token.
    */
    DECL_LINK(ClickHandler, PreviewValueSet*);
    DECL_LINK(RightClickHandler, MouseEvent*);
    DECL_LINK(ContextMenuCallback, CommandEvent*);
    DECL_LINK(ContainerChangeListener, MasterPageContainerChangeEvent*);

    void SetItem (
        USHORT nIndex,
        MasterPageContainer::Token aToken);
    void AddTokenToIndexEntry (
        USHORT nIndex,
        MasterPageContainer::Token aToken);
    void RemoveTokenToIndexEntry (
        USHORT nIndex,
        MasterPageContainer::Token aToken);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
