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
#include <vcl/treelistbox.hxx>
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
class SvTreeListEntry;

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

/**
 * Effect-Tab-Dialog
 */
class SD_DLLPUBLIC SdPageObjsTLB final : public SvTreeListBox
{
    static bool  SAL_DLLPRIVATE bIsInDrag;      ///< static, in the case the navigator is deleted in ExecuteDrag

    ::std::unique_ptr< ::svt::AcceleratorExecute> m_pAccel;

public:

    // nested class to implement the TransferableHelper
    class SdPageObjsTransferable : public SdTransferable
    {
    public:
        SdPageObjsTransferable(
            SdPageObjsTLB& rParent,
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

        SdPageObjsTLB&      mrParent;
        INetBookmark const      maBookmark;
        ::sd::DrawDocShell&     mrDocShell;
        NavigatorDragType const   meDragType;
        SAL_DLLPRIVATE virtual               ~SdPageObjsTransferable() override;

        SAL_DLLPRIVATE virtual void      AddSupportedFormats() override;
        SAL_DLLPRIVATE virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
        SAL_DLLPRIVATE virtual void      DragFinished( sal_Int8 nDropAction ) override;

        SAL_DLLPRIVATE virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& rId ) override;
    };

    friend class SdPageObjsTLB::SdPageObjsTransferable;

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

private:

    VclPtr<SdNavigatorWin>  mpNavigator;
    const SdDrawDocument*   mpDoc;
    SdDrawDocument*         mpBookmarkDoc;
    SfxMedium*              mpMedium;
    SfxMedium*              mpOwnMedium;
    Image const             maImgOle;
    Image const             maImgGraphic;
    bool                    mbLinkableSelected;
    OUString                maDocName;
    ::sd::DrawDocShellRef   mxBookmarkDocShRef; ///< for the loading of bookmarks
    std::vector<OUString>   maTreeItem;
    bool                    mbSaveTreeItemState;
    OUString                maSelectionEntryText;

    // DragSourceHelper
    virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

    // DropTargetHelper
    virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    virtual void            RequestingChildren( SvTreeListEntry* pParent ) override;

    void                    DoDrag();
    void                    OnDragFinished();

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
    void                    CloseBookmarkDoc();
                            DECL_LINK(ExecDragHdl, void*, void);

    /** Handle the reordering of entries in the navigator.  This method
        reorders both the involved shapes in their page as well as the
        associated list box entries.
    */
    virtual TriState NotifyMoving(
        SvTreeListEntry*  pTarget,
        SvTreeListEntry*  pEntry,
        SvTreeListEntry*& rpNewParent,
        sal_uLong&        rNewChildPos) override;

    using Window::GetDropTarget;
    virtual SvTreeListEntry* GetDropTarget (const Point& rLocation) override;
    virtual void InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&,SvLBoxButtonKind) override;

public:

    SdPageObjsTLB(vcl::Window* pParent, WinBits nStyle);
    virtual ~SdPageObjsTLB() override;
    virtual void            dispose() override;

   // helper function for   GetEntryAltText and GetEntryLongDescription
    OUString                getAltLongDescText( SvTreeListEntry* pEntry , bool isAltText) const;
    OUString                GetEntryAltText( SvTreeListEntry* pEntry ) const override;
    OUString                GetEntryLongDescription( SvTreeListEntry* pEntry ) const override;
    virtual void            SelectHdl() override;
    virtual void            KeyInput( const KeyEvent& rKEvt ) override;
    void MouseButtonDown(const MouseEvent& rMEvt) override;

    void                    SetViewFrame( const SfxViewFrame* pViewFrame );

    void                    Fill( const SdDrawDocument*, bool bAllPages, const OUString& rDocName );
    void                    Fill( const SdDrawDocument*, SfxMedium* pSfxMedium, const OUString& rDocName );
    void                    SetShowAllShapes (const bool bShowAllShapes, const bool bFill);
    bool                    GetShowAllShapes() const { return mbShowAllShapes;}
    bool IsNavigationGrabsFocus() const { return mbNavigationGrabsFocus; }
    bool                    IsEqualToDoc( const SdDrawDocument* pInDoc );
    /// Visits rList recursively and tries to advance pEntry accordingly.
    bool IsEqualToShapeList(SvTreeListEntry*& pEntry, const SdrObjList& rList,
                            const OUString& rListName);
    bool                    HasSelectedChildren( const OUString& rName );
    bool                    SelectEntry( const OUString& rName );
    OUString                GetSelectedEntry();

    //Mark Current Entry
    void                    SetSdNavigator(SdNavigatorWin* pNavigator);

    void                    Clear();
    void                    SetSaveTreeItemStateFlag(bool bState){mbSaveTreeItemState = bState;}
    void                    SaveExpandedTreeItemState(SvTreeListEntry* pEntry, std::vector<OUString>& vectTreeItem);

    SdDrawDocument*         GetBookmarkDoc(SfxMedium* pMedium = nullptr);

    bool                    IsLinkableSelected() const { return mbLinkableSelected; }

    static bool             IsInDrag();
    using SvTreeListBox::ExecuteDrop;

    using SvTreeListBox::SelectEntry;

    /** Return the view shell that is linked to the given doc shell.
        Call this method when the there is a chance that the doc shell
        has been disconnected from the view shell (but not the other
        way round.)
        @return
            May return <NULL/> when the link between view shell and
            doc shell has been severed.
    */
    static ::sd::ViewShell* GetViewShellForDocShell (::sd::DrawDocShell &rDocShell);

private:
    /** This flag controls whether all shapes are shown as children of pages
        and group shapes or only the named shapes.
    */
    bool mbShowAllShapes;
    /** This flag controls whether to show all pages.
    */
    bool mbShowAllPages;
    /**
     * If changing the selection should also result in navigating to the
     * relevant shape.
     */
    bool mbSelectionHandlerNavigates;
    /**
     * If navigation should not only select the relevant shape but also change
     * focus to it.
     */
    bool mbNavigationGrabsFocus;

    /** Return <TRUE/> when the current transferable may be dropped at the
        given list box entry.
    */
    bool IsDropAllowed (SvTreeListEntry const * pEntry);

    /** This inner class is defined in sdtreelb.cxx and is basically a
        container for the icons used in the list box for the entries.
    */
    class IconProvider;

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
        @param rIconProvider
            Icons used to visualize the different shape and page types.
    */
    void AddShapeList (
        const SdrObjList& rList,
        SdrObject* pShape,
        const OUString& rsName,
        const bool bIsExcluded,
        SvTreeListEntry* pParentEntry,
        const IconProvider& rIconProvider);

    /** Add the given object to a transferable object so that the object can
        be dragged and dropped without having a name.
    */
    void AddShapeToTransferable (
        SdTransferable& rTransferable,
        SdrObject& rObject) const;
};

class SD_DLLPUBLIC SdPageObjsTLV
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;
    std::unique_ptr<::svt::AcceleratorExecute> m_xAccel;
    const SdDrawDocument* m_pDoc;
    SdDrawDocument* m_pBookmarkDoc;
    SfxMedium* m_pMedium;
    SfxMedium* m_pOwnMedium;
    bool m_bLinkableSelected;
    /** This flag controls whether to show all pages.
    */
    bool m_bShowAllPages;
    OUString m_aDocName;
    ::sd::DrawDocShellRef m_xBookmarkDocShRef; ///< for the loading of bookmarks
    Link<weld::TreeView&, void> m_aChangeHdl;

    /** Return the name of the object.  When the object has no user supplied
        name and the bCreate flag is <TRUE/> then a name is created
        automatically.  Additionally the mbShowAllShapes flag is taken into
        account when there is no user supplied name.  When this flag is
        <FALSE/> then no name is created.
        @param pObject
            When this is NULL then an empty string is returned, regardless
            of the value of bCreate.
    */
    static OUString GetObjectName (const SdrObject* pObject);

    void CloseBookmarkDoc();

    DECL_LINK(RequestingChildrenHdl, const weld::TreeIter&, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);

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

public:

    SdPageObjsTLV(std::unique_ptr<weld::TreeView> xTreeview);
    ~SdPageObjsTLV();

    void hide()
    {
        m_xTreeView->hide();
    }

    void show()
    {
        m_xTreeView->show();
    }

    void set_size_request(int nWidth, int nHeight)
    {
        m_xTreeView->set_size_request(nWidth, nHeight);
    }

    float get_approximate_digit_width() const
    {
        return m_xTreeView->get_approximate_digit_width();
    }

    int get_height_rows(int nRows) const
    {
        return m_xTreeView->get_height_rows(nRows);
    }

    void set_selection_mode(SelectionMode eMode)
    {
        m_xTreeView->set_selection_mode(eMode);
    }

    bool SelectEntry(const OUString& rName);

    OUString get_selected_text() const
    {
        return m_xTreeView->get_selected_text();
    }

    bool get_selected() const
    {
        return m_xTreeView->get_selected(nullptr);
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
        weld::TreeIter* pParentEntry);

    /** return selected entries
          nDepth == 0 -> pages
          nDepth == 1 -> objects  */

    std::vector<OUString> GetSelectEntryList(const int nDepth) const;

    SdDrawDocument* GetBookmarkDoc();

    bool IsLinkableSelected() const { return m_bLinkableSelected; }

    void InsertEntry(const OUString &rName, const OUString &rExpander)
    {
        m_xTreeView->insert(nullptr, -1, &rName, nullptr, nullptr, nullptr, &rExpander, false, nullptr);
    }

    void InsertEntry(weld::TreeIter* pParent, const OUString& rId, const OUString &rName, const OUString &rExpander, weld::TreeIter* pEntry = nullptr)
    {
        m_xTreeView->insert(pParent, -1, &rName, &rId, nullptr, nullptr, &rExpander, false, pEntry);
    }

    void clear()
    {
        m_xTreeView->clear();
    }
};

#endif // INCLUDED_SD_SOURCE_UI_INC_SDTREELB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
