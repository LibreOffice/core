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

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/transfer.hxx>
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
    ThemeName  = 0x0001,
    Title      = 0x0002,
    Path       = 0x0004
};
namespace o3tl
{
    template<> struct typed_flags<GalleryItemFlags> : is_typed_flags<GalleryItemFlags, 0x0007> {};
}


class GalleryToolBox : public ToolBox
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

public:

                    GalleryToolBox( GalleryBrowser2* pParent );
};


class Gallery;
class GalleryTheme;
class GalleryIconView;
class GalleryListView;
class GalleryPreview;
class Menu;
class SgaObject;
struct DispatchInfo;

namespace svx { namespace sidebar { class GalleryControl; } }

class GalleryBrowser2 : public Control, public SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;
    using Window::KeyInput;

private:

    SvtMiscOptions      maMiscOptions;
    Gallery*            mpGallery;
    GalleryTheme*       mpCurTheme;
    VclPtr<GalleryIconView>    mpIconView;
    VclPtr<GalleryListView>    mpListView;
    VclPtr<GalleryPreview> mpPreview;
    VclPtr<GalleryToolBox> maViewBox;
    VclPtr<FixedLine>   maSeparator;
    VclPtr<FixedText>   maInfoBar;
    sal_uIntPtr         mnCurActionPos;
    GalleryBrowserMode  meMode;
    GalleryBrowserMode  meLastMode;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::util::XURLTransformer > m_xTransformer;

    void                InitSettings();

    void                ImplUpdateViews( sal_uInt16 nSelectionId );
    void                ImplUpdateInfoBar();
    sal_uIntPtr               ImplGetSelectedItemId( const Point* pSelPosPixel, Point& rSelPos );
    void                ImplSelectItemId( sal_uIntPtr nItemId );

    // Control
    virtual void        Resize() override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    // SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                        DECL_LINK( SelectObjectHdl, GalleryListView*, void );
                        DECL_LINK( SelectObjectValueSetHdl, ValueSet*, void );
                        DECL_LINK( SelectTbxHdl, ToolBox*, void );
                        DECL_LINK( MiscHdl, LinkParamNone*, void );

private:

    static GalleryBrowserMode meInitMode;

public:

    static OUString     GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, GalleryItemFlags nItemTextFlags );

public:

    GalleryBrowser2(vcl::Window* pParent, Gallery* pGallery);
    virtual ~GalleryBrowser2() override;
    virtual void dispose() override;

    void                SelectTheme( const OUString& rThemeName );

    GalleryBrowserMode  GetMode() const { return meMode; }
    void                SetMode( GalleryBrowserMode eMode );

    vcl::Window*        GetViewWindow() const;

    void                Travel( GalleryBrowserTravel eTravel );

    INetURLObject       GetURL() const;
    OUString            GetFilterName() const;

    sal_Int8            AcceptDrop( DropTargetHelper& rTarget, const AcceptDropEvent& rEvt );
    sal_Int8            ExecuteDrop( DropTargetHelper& rTarget, const ExecuteDropEvent& rEvt );
    void                StartDrag( vcl::Window* pWindow, const Point* pDragPoint = nullptr );
    void                TogglePreview( vcl::Window* pWindow, const Point* pPreviewPoint = nullptr );
    void                ShowContextMenu( vcl::Window* pWindow, const Point* pContextPoint );
    bool                KeyInput( const KeyEvent& rEvt, vcl::Window* pWindow );

    static css::uno::Reference< css::frame::XFrame > GetFrame();
    const css::uno::Reference< css::util::XURLTransformer >& GetURLTransformer() const { return m_xTransformer; }

    void Execute( sal_uInt16 nId );
    void Dispatch( sal_uInt16 nId,
                   const css::uno::Reference< css::frame::XDispatch > &rxDispatch,
                   const css::util::URL &rURL );

    DECL_STATIC_LINK( GalleryBrowser2, AsyncDispatch_Impl, void*, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
