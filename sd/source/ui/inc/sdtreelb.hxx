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

#ifndef _SDTREELB_HXX
#define _SDTREELB_HXX

#include <svtools/transfer.hxx>

#include "sdresid.hxx"
#include "pres.hxx"
#include "sddllapi.h"
#include <svtools/treelistbox.hxx>
#include <svl/urlbmk.hxx>
#include <tools/ref.hxx>
#include "sdxfer.hxx"
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

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
SV_DECL_REF(DrawDocShell)
#endif
}

/**
 * Effect-Tab-Dialog
 */
class SD_DLLPUBLIC SdPageObjsTLB : public SvTreeListBox
{
private:

    static sal_Bool  SAL_DLLPRIVATE bIsInDrag;      ///< static, in the case the navigator is deleted in ExecuteDrag

public:

    // nested class to implement the TransferableHelper
    class SdPageObjsTransferable : public SdTransferable
    {
    public:
        SdPageObjsTransferable(
            SdPageObjsTLB& rParent,
            const INetBookmark& rBookmark,
            ::sd::DrawDocShell& rDocShell,
            NavigatorDragType eDragType,
            const ::com::sun::star::uno::Any& rTreeListBoxData );
        ::sd::DrawDocShell&     GetDocShell() const;
        NavigatorDragType   GetDragType() const;

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
        static SdPageObjsTransferable* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();
        /** Return a temporary transferable data flavor that is used
            internally in the navigator for reordering entries.  Its
            lifetime ends with the office application.
        */
        static sal_uInt32 GetListBoxDropFormatId (void);

    private:
        /** Temporary drop flavor id that is used internally in the
            navigator.
        */
        static sal_uInt32 mnListBoxDropFormatId;

        SdPageObjsTLB&      mrParent;
        INetBookmark        maBookmark;
        ::sd::DrawDocShell&     mrDocShell;
        NavigatorDragType   meDragType;
        const ::com::sun::star::uno::Any maTreeListBoxData;
        SAL_DLLPRIVATE virtual               ~SdPageObjsTransferable();

        SAL_DLLPRIVATE virtual void      AddSupportedFormats();
        SAL_DLLPRIVATE virtual sal_Bool  GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        SAL_DLLPRIVATE virtual void      DragFinished( sal_Int8 nDropAction );

        SAL_DLLPRIVATE virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );
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

protected:

    Window*                 mpParent;
    const SdDrawDocument*   mpDoc;
    SdDrawDocument*         mpBookmarkDoc;
    SfxMedium*              mpMedium;
    SfxMedium*              mpOwnMedium;
    Image                   maImgOle;
    Image                   maImgGraphic;
    sal_Bool                mbLinkableSelected;
    sal_Bool                mbDragEnabled;
    OUString                maDocName;
    ::sd::DrawDocShellRef   mxBookmarkDocShRef; ///< for the loading of bookmarks
    ::sd::DrawDocShell*     mpDropDocSh;
    SdNavigatorWin*         mpDropNavWin;
    SfxViewFrame*           mpFrame;

    // DragSourceHelper
    virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // DropTargetHelper
    virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual void            RequestingChildren( SvTreeListEntry* pParent );

    void                    DoDrag();
    void                    OnDragFinished( sal_uInt8 nDropAction );

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
                            DECL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*);

    /** Handle the reordering of entries in the navigator.  This method
        reorders both the involved shapes in their page as well as the
        associated list box entries.
    */
    virtual sal_Bool NotifyMoving(
        SvTreeListEntry*  pTarget,
        SvTreeListEntry*  pEntry,
        SvTreeListEntry*& rpNewParent,
        sal_uLong&        rNewChildPos);

    using Window::GetDropTarget;
    virtual SvTreeListEntry* GetDropTarget (const Point& rLocation);

public:

                            SdPageObjsTLB( Window* pParent, const SdResId& rSdResId );
                            ~SdPageObjsTLB();

    virtual void            SelectHdl();
    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    SetViewFrame( SfxViewFrame* pViewFrame ) { mpFrame = pViewFrame; }
    SfxViewFrame*           GetViewFrame() const { return mpFrame; }

    void                    Fill( const SdDrawDocument*, sal_Bool bAllPages, const OUString& rDocName );
    void                    Fill( const SdDrawDocument*, SfxMedium* pSfxMedium, const OUString& rDocName );
    void                    SetShowAllShapes (const bool bShowAllShapes, const bool bFill);
    bool                    GetShowAllShapes (void) const;
    sal_Bool                    IsEqualToDoc( const SdDrawDocument* pInDoc = NULL );
    sal_Bool                    HasSelectedChildren( const OUString& rName );
    sal_Bool                    SelectEntry( const OUString& rName );
    OUString                GetSelectEntry();

    /** return selected entries
          nDepth == 0 -> pages
          nDepth == 1 -> objects  */

    std::vector<OUString> GetSelectEntryList (const sal_uInt16 nDepth) const;

    SdDrawDocument*         GetBookmarkDoc(SfxMedium* pMedium = NULL);
    ::sd::DrawDocShell*         GetDropDocSh() { return(mpDropDocSh); }

    sal_Bool                    IsLinkableSelected() const { return mbLinkableSelected; }

    static sal_Bool             IsInDrag();
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

    /** Return <TRUE/> when the current transferable may be dropped at the
        given list box entry.
    */
    bool IsDropAllowed (SvTreeListEntry* pEntry);

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

#endif      // _SDTREELB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
