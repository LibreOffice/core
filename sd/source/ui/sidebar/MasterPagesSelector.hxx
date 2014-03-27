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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGESSELECTOR_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_MASTERPAGESSELECTOR_HXX

#include "MasterPageContainer.hxx"
#include "SlideSorterViewShell.hxx"
#include "PreviewValueSet.hxx"
#include "ISidebarReceiver.hxx"
#include <sfx2/sidebar/ILayoutableWindow.hxx>

#include "pres.hxx"
#include <sfx2/shell.hxx>
#include <vcl/image.hxx>
#include "glob.hxx"
#include <osl/mutex.hxx>
#include <com/sun/star/ui/XSidebar.hpp>

#include <queue>

namespace cssu = ::com::sun::star::uno;

class MouseEvent;
class SdDrawDocument;
class SdPage;

namespace sd {
class ViewShellBase;
}

namespace sd { namespace sidebar {

class PreviewValueSet;


/** Base class of a menu that lets the user select from a list of
    templates or designs that are loaded from files.
*/
class MasterPagesSelector
    : public PreviewValueSet,
      public sfx2::sidebar::ILayoutableWindow
{
public:
    MasterPagesSelector (
        ::Window* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const ::boost::shared_ptr<MasterPageContainer>& rpContainer,
        const cssu::Reference<css::ui::XSidebar>& rxSidebar);
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
        sal_uInt16 nInsertionIndex);

    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);
    virtual sal_Int32 GetMinimumWidth (void);

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

    void SetHelpId( const OString& aId );

    /** Mark the preview that belongs to the given index as not up-to-date
        anymore with respect to page content or preview size.
        The implementation of this method will either sunchronously or
        asynchronously call UpdatePreview().
        @param nIndex
            Index into the value set control that is used for displaying the
            previews.
    */
    void InvalidatePreview (const SdPage* pPage);

    void UpdateAllPreviews (void);

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) SAL_OVERRIDE;

protected:
    mutable ::osl::Mutex maMutex;
    ::boost::shared_ptr<MasterPageContainer> mpContainer;

    SdDrawDocument& mrDocument;
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
    ::std::queue<sal_uInt16> maPreviewUpdateQueue;

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

    /** Give derived classes the oportunity to provide their own context
        menu.  If they do then they probably have to provide their own
        Execute() and GetState() methods as well.
    */
    virtual ResId GetContextMenuResId (void) const;

    virtual void Command (const CommandEvent& rEvent) SAL_OVERRIDE;

    virtual void ProcessPopupMenu (Menu& rMenu);
    virtual void ExecuteCommand (const sal_Int32 nCommandId);

private:
    cssu::Reference<css::ui::XSidebar> mxSidebar;

    /** The offset between ValueSet index and MasterPageContainer::Token
        last seen.  This value is used heuristically to speed up the lookup
        of an index for a token.
    */
    DECL_LINK(ClickHandler, void *);
    DECL_LINK(RightClickHandler, MouseEvent*);
    DECL_LINK(ContextMenuCallback, CommandEvent*);
    DECL_LINK(ContainerChangeListener, MasterPageContainerChangeEvent*);
    DECL_LINK(OnMenuItemSelected, Menu*);

    void SetItem (
        sal_uInt16 nIndex,
        MasterPageContainer::Token aToken);
    void AddTokenToIndexEntry (
        sal_uInt16 nIndex,
        MasterPageContainer::Token aToken);
    void RemoveTokenToIndexEntry (
        sal_uInt16 nIndex,
        MasterPageContainer::Token aToken);
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
