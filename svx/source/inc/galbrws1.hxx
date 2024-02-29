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

#include <svl/lstner.hxx>
#include <vcl/customweld.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <svx/galmisc.hxx>
#include <svx/galctrl.hxx>
#include <unotools/charclass.hxx>
#include <vector>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class GalleryBrowser1;

class Gallery;
class GalleryThemeEntry;
class GalleryTheme;
class VclAbstractDialog;
struct ExchangeData;
class SfxItemSet;
class GalleryDragDrop;
class GalleryIconView;
class GalleryListView;
class GalleryPreview;
class SgaObject;

enum GalleryBrowserMode
{
    GALLERYBROWSERMODE_NONE = 0,
    GALLERYBROWSERMODE_ICON = 1,
    GALLERYBROWSERMODE_LIST = 2,
    GALLERYBROWSERMODE_PREVIEW = 3
};

enum class GalleryBrowserTravel
{
    First,
    Last,
    Previous,
    Next
};

enum class GalleryItemFlags
{
    Title = 0x0002,
    Path = 0x0004
};
namespace o3tl
{
template <> struct typed_flags<GalleryItemFlags> : is_typed_flags<GalleryItemFlags, 0x0006>
{
};
}

namespace svx::sidebar
{
class GalleryControl;
}

struct ThemeEntry
{
    OUString maThemeName;
    OUString maEntryTitle;
    size_t mnIdInTheme;

    ThemeEntry(const OUString& rThemeName, const OUString& rEntryTitle, size_t nId)
        : maThemeName(rThemeName)
        , maEntryTitle(rEntryTitle)
        , mnIdInTheme(nId)
    {
    }
};
typedef ::std::vector<ThemeEntry> GalleryThemeEntries;

class GalleryBrowser1 final : public SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;

private:
    std::unique_ptr<weld::Button> mxNewTheme;
    std::unique_ptr<weld::TreeView> mxThemes;
    std::unique_ptr<weld::Button> mxMoreGalleries;
    Gallery* mpGallery;
    std::unique_ptr<ExchangeData> mpExchangeData;
    std::unique_ptr<SfxItemSet> mpThemePropsDlgItemSet;

    OUString aImgNormal;
    OUString aImgDefault;
    OUString aImgReadOnly;

    GalleryTheme* mpCurTheme;
    std::unique_ptr<GalleryIconView> mxIconView;
    std::unique_ptr<weld::CustomWeld> mxIconViewWin;
    std::unique_ptr<weld::TreeView> mxListView;
    std::unique_ptr<GalleryDragDrop> mxDragDropTargetHelper;
    std::unique_ptr<GalleryPreview> mxPreview;
    std::unique_ptr<weld::CustomWeld> mxPreviewWin;
    std::unique_ptr<weld::ToggleButton> mxIconButton;
    std::unique_ptr<weld::ToggleButton> mxListButton;
    std::unique_ptr<weld::Entry> mxSearchField;
    std::unique_ptr<weld::Label> mxInfoBar;
    Size maPreviewSize;
    rtl::Reference<GalleryTransferable> m_xHelper;
    sal_uInt32 mnCurActionPos;
    GalleryBrowserMode meMode;
    GalleryBrowserMode meLastMode;

    GalleryThemeEntries maAllThemeEntries;
    GalleryThemeEntries maFoundThemeEntries;

    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::util::XURLTransformer> m_xTransformer;

    CharClass m_aCharacterClassficator;

    void ImplInsertThemeEntry(const GalleryThemeEntry* pEntry);
    static void ImplFillExchangeData(const GalleryTheme* pThm, ExchangeData& rData);
    void ImplGetExecuteVector(std::vector<OUString>& o_aExec);
    void ImplExecute(std::u16string_view rIdent);
    void ImplGalleryThemeProperties(std::u16string_view rThemeName, bool bCreateNew);
    void EndNewThemePropertiesDlgHdl(sal_Int32 nResult);
    void EndThemePropertiesDlgHdl(sal_Int32 nResult);
    void ImplEndGalleryThemeProperties(bool bCreateNew, sal_Int32 nResult);

    void ImplUpdateViews(sal_uInt16 nSelectionId);
    void ImplUpdateInfoBar();
    sal_uInt32 ImplGetSelectedItemId(const Point* pSelPosPixel, Point& rSelPos);
    void ImplSelectItemId(sal_uInt32 nItemId);
    void ImplUpdateSelection();
    void UpdateRows(bool bVisibleOnly);
    void FillThemeEntries();

    // SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    DECL_LINK(ClickNewThemeHdl, weld::Button&, void);
    DECL_LINK(SelectThemeHdl, weld::TreeView&, void);
    DECL_LINK(PopupMenuHdl1, const CommandEvent&, bool);
    DECL_LINK(KeyInputHdl1, const KeyEvent&, bool);
    DECL_STATIC_LINK(GalleryBrowser1, OnMoreGalleriesClick, weld::Button&, void);

    DECL_LINK(SelectObjectHdl, weld::TreeView&, void);
    DECL_LINK(SelectObjectValueSetHdl, ValueSet*, void);
    DECL_LINK(SelectTbxHdl, weld::Toggleable&, void);
    DECL_LINK(PopupMenuHdl2, const CommandEvent&, bool);
    DECL_LINK(KeyInputHdl2, const KeyEvent&, bool);
    DECL_LINK(RowActivatedHdl, weld::TreeView&, bool);
    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_LINK(VisRowsScrolledHdl, weld::TreeView&, void);
    DECL_LINK(SizeAllocHdl, const Size&, void);
    DECL_LINK(SearchHdl, weld::Entry&, void);

    static GalleryBrowserMode meInitMode;

public:
    GalleryBrowser1(weld::Builder& rBuilder, Gallery* pGallery);

    ~GalleryBrowser1();

    void SelectTheme(sal_uInt16 nThemePos)
    {
        mxThemes->select(nThemePos);
        SelectThemeHdl(*mxThemes);
    }
    OUString GetSelectedTheme() const { return mxThemes->get_selected_text(); }

    static OUString GetItemText(const SgaObject& rObj, GalleryItemFlags nItemTextFlags);

public:
    void SelectTheme(std::u16string_view rThemeName);

    GalleryBrowserMode GetMode() const { return meMode; }
    void SetMode(GalleryBrowserMode eMode);

    weld::Widget* GetViewWindow() const;

    void Travel(GalleryBrowserTravel eTravel);

    INetURLObject GetURL() const;
    OUString GetFilterName() const;

    sal_Int8 AcceptDrop(const DropTargetHelper& rTarget);
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);
    bool StartDrag();
    void TogglePreview();
    bool ShowContextMenu(const CommandEvent& rCEvt);
    bool KeyInput(const KeyEvent& rEvt);
    bool ViewBoxHasFocus() const;

    static css::uno::Reference<css::frame::XFrame> GetFrame();
    const css::uno::Reference<css::util::XURLTransformer>& GetURLTransformer() const
    {
        return m_xTransformer;
    }

    void Execute(std::u16string_view rIdent);
    void DispatchAdd(const css::uno::Reference<css::frame::XDispatch>& rxDispatch,
                     const css::util::URL& rURL);

    DECL_STATIC_LINK(GalleryBrowser1, AsyncDispatch_Impl, void*, void);
};

class GalleryDragDrop final : public DropTargetHelper
{
private:
    GalleryBrowser1* m_pParent;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& /*rEvt*/) override
    {
        return m_pParent->AcceptDrop(*this);
    }

    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        return m_pParent->ExecuteDrop(rEvt);
    }

public:
    GalleryDragDrop(GalleryBrowser1* pParent,
                    const css::uno::Reference<css::datatransfer::dnd::XDropTarget>& rDropTarget)
        : DropTargetHelper(rDropTarget)
        , m_pParent(pParent)
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
