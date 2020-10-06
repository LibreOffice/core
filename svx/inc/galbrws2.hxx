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

#ifndef INCLUDED_SVX_INC_GALBRWS2_HXX
#define INCLUDED_SVX_INC_GALBRWS2_HXX

#include <vcl/transfer.hxx>
#include <svl/lstner.hxx>
#include <svx/galctrl.hxx>
#include <svtools/miscopt.hxx>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>


enum GalleryBrowserMode
{
    GALLERYBROWSERMODE_NONE = 0,
    GALLERYBROWSERMODE_ICON = 1,
    GALLERYBROWSERMODE_LIST = 2,
    GALLERYBROWSERMODE_PREVIEW = 3
};


enum class GalleryBrowserTravel
{
    First, Last, Previous, Next
};

enum class GalleryItemFlags {
    Title      = 0x0002,
    Path       = 0x0004
};
namespace o3tl
{
    template<> struct typed_flags<GalleryItemFlags> : is_typed_flags<GalleryItemFlags, 0x0006> {};
}


class Gallery;
class GalleryDragDrop;
class GalleryTheme;
class GalleryIconView;
class GalleryListView;
class GalleryPreview;
class GalleryTransferable;
class Menu;
class SgaObject;
struct DispatchInfo;

namespace svx::sidebar { class GalleryControl; }

class GalleryBrowser2 : public SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;

private:

    Gallery*            mpGallery;
    GalleryTheme*       mpCurTheme;
    std::unique_ptr<GalleryIconView> mxIconView;
    std::unique_ptr<weld::CustomWeld> mxIconViewWin;
    std::unique_ptr<weld::TreeView> mxListView;
    std::unique_ptr<GalleryDragDrop> mxDragDropTargetHelper;
    std::unique_ptr<GalleryPreview> mxPreview;
    std::unique_ptr<weld::CustomWeld> mxPreviewWin;
    std::unique_ptr<weld::ToggleButton> mxIconButton;
    std::unique_ptr<weld::ToggleButton> mxListButton;
    std::unique_ptr<weld::Label> mxInfoBar;
    VclPtr<VirtualDevice> mxDev;
    Size maPreviewSize;
    rtl::Reference<GalleryTransferable> m_xHelper;
    sal_uInt32 mnCurActionPos;
    GalleryBrowserMode  meMode;
    GalleryBrowserMode  meLastMode;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::util::XURLTransformer > m_xTransformer;

    void                ImplUpdateViews( sal_uInt16 nSelectionId );
    void                ImplUpdateInfoBar();
    sal_uInt32          ImplGetSelectedItemId( const Point* pSelPosPixel, Point& rSelPos );
    void                ImplSelectItemId(sal_uInt32 nItemId);
    void                ImplUpdateSelection();
    void                UpdateRows(bool bVisibleOnly);

    // SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                        DECL_LINK( SelectObjectHdl, weld::TreeView&, void );
                        DECL_LINK( SelectObjectValueSetHdl, ValueSet*, void );
                        DECL_LINK( SelectTbxHdl, weld::ToggleButton&, void );
                        DECL_LINK( PopupMenuHdl, const CommandEvent&, bool );
                        DECL_LINK( KeyInputHdl, const KeyEvent&, bool );
                        DECL_LINK( RowActivatedHdl, weld::TreeView&, bool );
                        DECL_LINK( DragBeginHdl, bool&, bool );
                        DECL_LINK( VisRowsScrolledHdl, weld::TreeView&, void );
                        DECL_LINK( SizeAllocHdl, const Size&, void );

private:

    static GalleryBrowserMode meInitMode;

public:

    static OUString     GetItemText( const SgaObject& rObj, GalleryItemFlags nItemTextFlags );

public:

    GalleryBrowser2(weld::Builder& rBuilder, Gallery* pGallery);
    ~GalleryBrowser2();

    void                SelectTheme( const OUString& rThemeName );

    GalleryBrowserMode  GetMode() const { return meMode; }
    void                SetMode( GalleryBrowserMode eMode );

    weld::Widget*       GetViewWindow() const;

    void                Travel( GalleryBrowserTravel eTravel );

    INetURLObject       GetURL() const;
    OUString            GetFilterName() const;

    sal_Int8            AcceptDrop( const DropTargetHelper& rTarget );
    sal_Int8            ExecuteDrop( const ExecuteDropEvent& rEvt );
    bool                StartDrag();
    void                TogglePreview();
    void                ShowContextMenu(const CommandEvent& rCEvt);
    bool                KeyInput(const KeyEvent& rEvt);
    bool                ViewBoxHasFocus() const;

    static css::uno::Reference< css::frame::XFrame > GetFrame();
    const css::uno::Reference< css::util::XURLTransformer >& GetURLTransformer() const { return m_xTransformer; }

    void Execute(const OString &rIdent);
    void DispatchAdd(const css::uno::Reference<css::frame::XDispatch> &rxDispatch,
                     const css::util::URL &rURL);

    DECL_STATIC_LINK( GalleryBrowser2, AsyncDispatch_Impl, void*, void );
};

class GalleryDragDrop : public DropTargetHelper
{
private:
    GalleryBrowser2* m_pParent;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& /*rEvt*/) override
    {
        return m_pParent->AcceptDrop(*this);
    }

    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        return m_pParent->ExecuteDrop(rEvt);
    }

public:
    GalleryDragDrop(GalleryBrowser2* pParent, const css::uno::Reference<css::datatransfer::dnd::XDropTarget>& rDropTarget)
        : DropTargetHelper(rDropTarget)
        , m_pParent(pParent)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
