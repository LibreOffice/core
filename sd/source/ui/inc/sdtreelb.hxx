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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SDTREELB_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SDTREELB_HXX

#include <pres.hxx>
#include <sddllapi.h>
#include <vcl/weld.hxx>
#include <svl/urlbmk.hxx>
#include <tools/ref.hxx>
#include "sdxfer.hxx"
#include <memory>
#include <vector>

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
typedef ::tools::SvRef<DrawDocShell> DrawDocShellRef;
#endif
}
namespace svt {
    class AcceleratorExecute;
}

class SdPageObjsTLVDropTarget : public DropTargetHelper
{
private:
    weld::TreeView& m_rTreeView;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    SdPageObjsTLVDropTarget(weld::TreeView& rTreeView);
};

class SD_DLLPUBLIC SdPageObjsTLV
{
private:
    static bool SAL_DLLPRIVATE bIsInDrag;      ///< static, in the case the navigator is deleted in ExecuteDrag

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

    SelectionMode m_eSelectionMode;

    ImplSVEvent* m_nSelectEventId;
    ImplSVEvent* m_nRowActivateEventId;

    OUString m_aDocName;
    ::sd::DrawDocShellRef m_xBookmarkDocShRef; ///< for the loading of bookmarks
    Link<weld::TreeView&, void> m_aChangeHdl;
    Link<weld::TreeView&, bool> m_aRowActivatedHdl;
    Link<const KeyEvent&, bool> m_aKeyPressHdl;

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

    SdPageObjsTLV(std::unique_ptr<weld::TreeView> xTreeview);
    ~SdPageObjsTLV();

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

    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, bool);

    void Select();

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

    bool HasSelectedChildren(std::u16string_view rName);
    bool SelectEntry(std::u16string_view rName);

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

    void SetViewFrame(const SfxViewFrame* pViewFrame);

    void Fill(const SdDrawDocument*, bool bAllPages, const OUString& rDocName);
    void Fill(const SdDrawDocument*, SfxMedium* pSfxMedium, const OUString& rDocName);

    void SetShowAllShapes (const bool bShowAllShapes, const bool bFill);
    bool GetShowAllShapes() const { return m_bShowAllShapes; }

    bool IsNavigationGrabsFocus() const { return m_bNavigationGrabsFocus; }
    bool IsEqualToDoc(const SdDrawDocument* pInDoc);
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
        SdrObject* pShape,
        const OUString& rsName,
        const bool bIsExcluded,
        const weld::TreeIter* pParentEntry);

    /** Add the given object to a transferable object so that the object can
        be dragged and dropped without having a name.
    */
    void AddShapeToTransferable (
        SdTransferable& rTransferable,
        SdrObject& rObject) const;

    /** return selected entries
          nDepth == 0 -> pages
          nDepth == 1 -> objects  */

    std::vector<OUString> GetSelectEntryList(const int nDepth) const;

    SdDrawDocument* GetBookmarkDoc(SfxMedium* pMedium = nullptr);

    bool IsLinkableSelected() const { return m_bLinkableSelected; }

    void InsertEntry(const OUString &rName, const OUString &rExpander)
    {
        m_xTreeView->insert(nullptr, -1, &rName, nullptr, nullptr, nullptr, false, m_xScratchIter.get());
        m_xTreeView->set_image(*m_xScratchIter, rExpander);
    }

    void InsertEntry(const weld::TreeIter* pParent, const OUString& rId, const OUString &rName, const OUString &rExpander, weld::TreeIter* pEntry = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rName, &rId, nullptr, nullptr, false, m_xScratchIter.get());
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
    class SAL_DLLPRIVATE SdPageObjsTransferable : public SdTransferable
    {
    public:
        SdPageObjsTransferable(
            const INetBookmark& rBookmark,
            ::sd::DrawDocShell& rDocShell,
            NavigatorDragType eDragType );
        ::sd::DrawDocShell&     GetDocShell() const { return mrDocShell;}
        NavigatorDragType   GetDragType() const { return meDragType;}

        static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();
        static SdPageObjsTransferable* getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData ) throw();
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

        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) override;
    };

    friend class SdPageObjsTLV::SdPageObjsTransferable;

private:
    rtl::Reference<SdPageObjsTransferable> m_xHelper;
};

#endif // INCLUDED_SD_SOURCE_UI_INC_SDTREELB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
