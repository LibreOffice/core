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

#include <memory>
#include "MasterPageContainer.hxx"
#include <SlideSorterViewShell.hxx>
#include "PreviewValueSet.hxx"
#include "ISidebarReceiver.hxx"
#include <sfx2/sidebar/ILayoutableWindow.hxx>

#include <pres.hxx>
#include <sfx2/shell.hxx>
#include <vcl/image.hxx>
#include <glob.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/ui/XSidebar.hpp>

#include <queue>


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
        vcl::Window* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        const std::shared_ptr<MasterPageContainer>& rpContainer,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);
    virtual ~MasterPagesSelector() override;
    virtual void dispose() override;

    virtual void LateInit();

    sal_Int32 GetPreferredHeight (sal_Int32 nWidth);

    /** Make the selector empty.  This method clear the value set from any
        entries. Override this method to add functionality, especially to
        destroy objects set as data items at the value set.
    */
    void ClearPageSet();

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

    void UpdateAllPreviews();

    // ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth) override;

protected:
    mutable ::osl::Mutex maMutex;
    std::shared_ptr<MasterPageContainer> mpContainer;

    SdDrawDocument& mrDocument;
    ViewShellBase& mrBase;
    /** menu entry that is executed as default action when the left mouse button is
        clicked over a master page.
    */
    OString msDefaultClickAction;

    SdPage* GetSelectedMasterPage();

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
    UserData* GetUserData (int nIndex) const;
    void SetUserData (int nIndex, UserData* pData);

    sal_Int32 GetIndexForToken (MasterPageContainer::Token aToken) const;
    typedef ::std::vector<MasterPageContainer::Token> ItemList;
    void UpdateItemList (::std::unique_ptr<ItemList> && pList);
    void Clear();
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
    /** Lock master pages in the given list and release locks that were
        previously acquired.
    */
    void UpdateLocks (const ItemList& rItemList);

    void Fill();
    virtual void Fill (ItemList& rItemList) = 0;

    /** Give derived classes the opportunity to provide their own context
        menu.  If they do then they probably have to provide their own
        Execute() and GetState() methods as well.
    */
    virtual OUString GetContextMenuUIFile() const;

    virtual void Command (const CommandEvent& rEvent) override;

    virtual void ProcessPopupMenu (Menu& rMenu);
    virtual void ExecuteCommand(const OString& rIdent);

private:
    css::uno::Reference<css::ui::XSidebar> mxSidebar;

    /** The offset between ValueSet index and MasterPageContainer::Token
        last seen.  This value is used heuristically to speed up the lookup
        of an index for a token.
    */
    DECL_LINK(ClickHandler, ValueSet*, void);
    DECL_LINK(RightClickHandler, const MouseEvent&, void);
    DECL_LINK(ContainerChangeListener, MasterPageContainerChangeEvent&, void);
    DECL_LINK(OnMenuItemSelected, Menu*, bool);

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
