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

#pragma once

#include <pres.hxx>
#include <sddllapi.h>
#include <vcl/weld.hxx>
#include <svl/urlbmk.hxx>
#include <tools/ref.hxx>
#include "sdxfer.hxx"
#include <memory>
#include <vector>

class SdrView;
class SdDrawDocument;
class SfxMedium;
class SfxViewFrame;
class SdNavigatorWin;
class SdrObject;
class SdrObjList;
class SdPage;
struct ImplSVEvent;

namespace sd {
class ViewShell;

class DrawDocShell;
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
typedef rtl::Reference<DrawDocShell> DrawDocShellRef;
#endif
}
namespace svt {
    class AcceleratorExecute;
}

class SdPageObjsTLVDropTarget final : public DropTargetHelper
{
private:
    weld::TreeView& m_rTreeView;
    SdrView* m_pSdrView;
    bool m_bOrderFrontToBack = false;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    SdPageObjsTLVDropTarget(weld::TreeView& rTreeView);

    void SetDrawView(SdrView* pSdrView) { m_pSdrView = pSdrView; }
    void SetOrderFrontToBack(bool bSet) { m_bOrderFrontToBack = bSet; }
};

class SdPageObjsTLV
{
private:
    static bool bIsInDrag;      ///< static, in the case the navigator is deleted in ExecuteDrag

    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    std::unique_ptr<SdPageObjsTLVDropTarget> m_xDropTargetHelper;
    std::unique_ptr<::svt::AcceleratorExecute> m_xAccel;
    SdNavigatorWin* m_pNavigator;
    const SdDrawDocument* m_pDoc;
    SdDrawDocument* m_pBookmarkDoc;
    SfxMedium* m_pMedium;
    SfxMedium* m_pOwnMedium;
    bool m_bLinkableSelected;
    bool m_bShowAllShapes;
    bool m_bOrderFrontToBack;

    /** This flag controls whether to show all pages.
    */
    bool m_bShowAllPages;

    /**
     * If changing the selection should also result in navigating to the
     * relevant shape.
     */
    bool m_bSelectionHandlerNavigates;

    /**
     * If navigation should not only select the relevant shape but also change
     * focus to it.
     */
    bool m_bNavigationGrabsFocus;

    bool m_bMouseReleased = true; // hack for x11 inplace editing
    bool m_bEditing = false;

    SelectionMode m_eSelectionMode;

    ImplSVEvent* m_nSelectEventId;
    ImplSVEvent* m_nRowActivateEventId;

    OUString m_aDocName;
    ::sd::DrawDocShellRef m_xBookmarkDocShRef; ///< for the loading of bookmarks
    Link<weld::TreeView&, void> m_aChangeHdl;
    Link<weld::TreeView&, bool> m_aRowActivatedHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;
    Link<const MouseEvent&, bool> m_aMouseReleaseHdl;
    Link<const CommandEvent&, bool> m_aPopupMenuHdl;

    /** Return the name of the object.  When the object has no user supplied
        name and the bCreate flag is <TRUE/> then a name is created
        automatically.  Additionally the mbShowAllShapes flag is taken into
        account when there is no user supplied name.  When this flag is
        <FALSE/> then no name is created.
        @param pObject
            When this is NULL then an empty string is returned, regardless
            of the value of bCreate.
        @param bCreate
            This flag controls for objects without user supplied name
            whether a name is created.  When a name is created then this
            name is not stored in the object.
    */
    OUString GetObjectName (
        const SdrObject* pObject,
        const bool bCreate = true) const;

    void CloseBookmarkDoc();

    DECL_LINK(RequestingChildrenHdl, const weld::TreeIter&, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(AsyncSelectHdl, void*, void);
    DECL_LINK(RowActivatedHdl, weld::TreeView&, bool);
    DECL_LINK(AsyncRowActivatedHdl, void*, void);
    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

    DECL_LINK(EditingEntryHdl, const weld::TreeIter&, bool);
    typedef std::pair<const weld::TreeIter&, OUString> IterString;
    DECL_LINK(EditedEntryHdl, const IterString&, bool);
    DECL_LINK(EditEntryAgain, void*, void);

    DECL_LINK(CommandHdl, const CommandEvent&, bool);

    /** Determine whether the specified page belongs to the current show
        which is either the standard show or a custom show.
        @param pPage
            Pointer to the page for which to check whether it belongs to the
            show.
        @return
            Returns <FALSE/> if there is no custom show or if the current
            show does not contain the specified page at least once.
    */
    bool PageBelongsToCurrentShow (const SdPage* pPage) const;

    bool DoDrag();
    static void OnDragFinished();

    // DragSourceHelper
    bool StartDrag();

public:

    SD_DLLPUBLIC SdPageObjsTLV(std::unique_ptr<weld::TreeView> xTreeview);
    SD_DLLPUBLIC ~SdPageObjsTLV();

    bool IsEditingActive() const {return m_bEditing;}

    void set_sensitive(bool bSensitive)
    {
        m_xTreeView->set_sensitive(bSensitive);
    }

    void hide()
    {
        m_xTreeView->hide();
    }

    void show()
    {
        m_xTreeView->show();
    }

    void grab_focus()
    {
        m_xTreeView->grab_focus();
    }

    void set_size_request(int nWidth, int nHeight)
    {
        m_xTreeView->set_size_request(nWidth, nHeight);
    }

    float get_approximate_digit_width() const
    {
        return m_xTreeView->get_approximate_digit_width();
    }

    DECL_LINK(MousePressHdl, const MouseEvent&, SD_DLLPUBLIC bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, SD_DLLPUBLIC bool);

    SD_DLLPUBLIC void Select();

    int get_height_rows(int nRows) const
    {
        return m_xTreeView->get_height_rows(nRows);
    }

    void set_selection_mode(SelectionMode eMode)
    {
        m_eSelectionMode = eMode;
        m_xTreeView->set_selection_mode(eMode);
    }

    SelectionMode get_selection_mode() const
    {
        return m_eSelectionMode;
    }

    void connect_row_activated(const Link<weld::TreeView&, bool>& rLink)
    {
        m_aRowActivatedHdl = rLink;
    }

    void connect_key_press(const Link<const KeyEvent&, bool>& rLink)
    {
        m_aKeyPressHdl = rLink;
    }

    void connect_mouse_release(const Link<const MouseEvent&, bool>& rLink)
    {
        m_aMouseReleaseHdl = rLink;
    }

    void connect_popup_menu(const Link<const CommandEvent&, bool>& rLink)
    {
        m_aPopupMenuHdl = rLink;
    }

    bool HasSelectedChildren(std::u16string_view rName);
    SD_DLLPUBLIC bool SelectEntry(std::u16string_view rName);
    void SelectEntry(const SdrObject* pObj);

    OUString get_selected_text() const
    {
        return m_xTreeView->get_selected_text();
    }

    bool get_selected() const
    {
        return m_xTreeView->get_selected(nullptr);
    }

    int count_selected_rows() const
    {
        return m_xTreeView->count_selected_rows();
    }

    void connect_changed(const Link<weld::TreeView&, void>& rLink)
    {
        m_aChangeHdl = rLink;
    }

    bool is_selected(const weld::TreeIter& rIter) const
    {
        return m_xTreeView->is_selected(rIter);
    }

    bool get_iter_first(weld::TreeIter& rIter) const
    {
        return m_xTreeView->get_iter_first(rIter);
    }

    weld::TreeView& get_treeview()
    {
        return *m_xTreeView;
    }

    std::unique_ptr<weld::TreeIter> make_iterator()
    {
        return m_xTreeView->make_iterator();
    }

    bool get_visible() const
    {
        return m_xTreeView->get_visible();
    }

    void unselect_all()
    {
        m_xTreeView->unselect_all();
    }

    OUString get_cursor_text() const
    {
        std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xIter.get()))
            return m_xTreeView->get_text(*xIter);
        return OUString();
    }

    OUString get_cursor_id() const
    {
        std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xIter.get()))
            return m_xTreeView->get_id(*xIter);
        return OUString();
    }

    SD_DLLPUBLIC void SetViewFrame(const SfxViewFrame* pViewFrame);

    SD_DLLPUBLIC void Fill(const SdDrawDocument*, bool bAllPages, const OUString& rDocName);
    SD_DLLPUBLIC void Fill(const SdDrawDocument*, SfxMedium* pSfxMedium, const OUString& rDocName);

    void SetShowAllShapes (const bool bShowAllShapes, const bool bFill);
    bool GetShowAllShapes() const { return m_bShowAllShapes; }

    void SetOrderFrontToBack (const bool bOrderFrontToBack);
    bool GetOrderFrontToBack() const { return m_bOrderFrontToBack; }

    bool IsNavigationGrabsFocus() const { return m_bNavigationGrabsFocus; }
    SD_DLLPUBLIC bool IsEqualToDoc(const SdDrawDocument* pInDoc);
    /// Visits rList recursively and tries to advance rEntry accordingly.
    bool IsEqualToShapeList(std::unique_ptr<weld::TreeIter>& rEntry, const SdrObjList& rList,
                            std::u16string_view rListName);

    static bool             IsInDrag();

    /** Return the view shell that is linked to the given doc shell.
        Call this method when the there is a chance that the doc shell
        has been disconnected from the view shell (but not the other
        way round.)
        @return
            May return <NULL/> when the link between view shell and
            doc shell has been severed.
    */
    static ::sd::ViewShell* GetViewShellForDocShell (::sd::DrawDocShell &rDocShell);

    /** Add one list box entry for the parent of the given shapes and one child entry for
        each of the given shapes.
        @param rList
            The container of shapes that are to be inserted.
        @param pShape
            The parent shape or NULL when the parent is a page.
        @param rsName
            The name to be displayed for the new parent node.
        @param bIsExcluded
            Some pages can be excluded (from the show?).
        @param pParentEntry
            The parent entry of the new parent entry.
    */
    void AddShapeList (
        const SdrObjList& rList,
        const SdrObject* pShape,
        const OUString& rsName,
        const bool bIsExcluded,
        const weld::TreeIter* pParentEntry);

    /** Add the given object to a transferable object so that the object can
        be dragged and dropped without having a name.
    */
    void AddShapeToTransferable (
        SdTransferable& rTransferable,
        const SdrObject& rObject) const;

    /** return selected entries
          nDepth == 0 -> pages
          nDepth == 1 -> objects  */

    SD_DLLPUBLIC std::vector<OUString> GetSelectEntryList(const int nDepth) const;

    std::vector<OUString> GetSelectedEntryIds() const;

    SD_DLLPUBLIC SdDrawDocument* GetBookmarkDoc(SfxMedium* pMedium = nullptr);

    bool IsLinkableSelected() const { return m_bLinkableSelected; }

    void InsertEntry(const OUString &rName, const OUString &rExpander)
    {
        m_xTreeView->insert(nullptr, -1, &rName, nullptr, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rExpander);
    }

    void InsertEntry(const weld::TreeIter* pParent, const OUString& rId, const OUString &rName, const OUString &rExpander, weld::TreeIter* pEntry = nullptr)
    {
        if (pParent)
            m_xTreeView->insert(pParent, m_bOrderFrontToBack ? 0 : -1, &rName, &rId, nullptr,
                                nullptr, false, m_xScratchIter.get());
        else
            // always append page/slide entry
            m_xTreeView->insert(nullptr, -1, &rName, &rId, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rExpander);
        if (pEntry)
            m_xTreeView->copy_iterator(*m_xScratchIter, *pEntry);
    }

    //Mark Current Entry
    void SetSdNavigator(SdNavigatorWin* pNavigator);

    void clear()
    {
        m_xTreeView->clear();
    }

    // nested class to implement the TransferableHelper
    class SdPageObjsTransferable final : public SdTransferable
    {
    public:
        SdPageObjsTransferable(
            INetBookmark aBookmark,
            ::sd::DrawDocShell& rDocShell,
            NavigatorDragType eDragType );
        ::sd::DrawDocShell&     GetDocShell() const { return mrDocShell;}
        NavigatorDragType   GetDragType() const { return meDragType;}

        static SdPageObjsTransferable* getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData ) noexcept;
        /** Return a temporary transferable data flavor that is used
            internally in the navigator for reordering entries.  Its
            lifetime ends with the office application.
        */
        static SotClipboardFormatId GetListBoxDropFormatId();

    private:
        /** Temporary drop flavor id that is used internally in the
            navigator.
        */
        static SotClipboardFormatId mnListBoxDropFormatId;

        INetBookmark const      maBookmark;
        ::sd::DrawDocShell&     mrDocShell;
        NavigatorDragType const   meDragType;
        virtual               ~SdPageObjsTransferable() override;

        virtual void      AddSupportedFormats() override;
        virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        virtual void      DragFinished( sal_Int8 nDropAction ) override;
    };

    friend class SdPageObjsTLV::SdPageObjsTransferable;

private:
    rtl::Reference<SdPageObjsTransferable> m_xHelper;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
