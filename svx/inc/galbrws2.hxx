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


// - GalleryBrowserMode -


enum GalleryBrowserMode
{
    GALLERYBROWSERMODE_NONE = 0,
    GALLERYBROWSERMODE_ICON = 1,
    GALLERYBROWSERMODE_LIST = 2,
    GALLERYBROWSERMODE_PREVIEW = 3
};


// - GalleryTravel -


enum GalleryBrowserTravel
{
    GALLERYBROWSERTRAVEL_CURRENT = 0,
    GALLERYBROWSERTRAVEL_FIRST = 1,
    GALLERYBROWSERTRAVEL_LAST = 2,
    GALLERYBROWSERTRAVEL_PREVIOUS = 3,
    GALLERYBROWSERTRAVEL_NEXT = 4
};


// - GalleryItemTextFlags -


#define GALLERY_ITEM_THEMENAME  0x00000001
#define GALLERY_ITEM_TITLE      0x00000002
#define GALLERY_ITEM_PATH       0x00000004


// - GalleryToolBox -


class GalleryToolBox : public ToolBox
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

public:

                    GalleryToolBox( GalleryBrowser2* pParent );
};


// - GalleryBrowser2 -


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
    using Control::Notify;
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
    Point               maDragStartPos;
    sal_uIntPtr         mnCurActionPos;
    GalleryBrowserMode  meMode;
    GalleryBrowserMode  meLastMode;

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;
    com::sun::star::uno::Reference< com::sun::star::util::XURLTransformer > m_xTransformer;

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

                        DECL_LINK_TYPED( SelectObjectHdl, GalleryListView*, void );
                        DECL_LINK_TYPED( SelectObjectValueSetHdl, ValueSet*, void );
                        DECL_LINK_TYPED( SelectTbxHdl, ToolBox*, void );
                        DECL_LINK_TYPED( MiscHdl, LinkParamNone*, void );

private:

    static GalleryBrowserMode meInitMode;

public:

    static OUString     GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, sal_uIntPtr nItemTextFlags );

public:

    GalleryBrowser2(vcl::Window* pParent, Gallery* pGallery);
    virtual ~GalleryBrowser2();
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
    void                StartDrag( vcl::Window* pWindow, const Point* pDragPoint = NULL );
    void                TogglePreview( vcl::Window* pWindow, const Point* pPreviewPoint = NULL );
    void                ShowContextMenu( vcl::Window* pWindow, const Point* pContextPoint = NULL );
    bool                KeyInput( const KeyEvent& rEvt, vcl::Window* pWindow );

    static com::sun::star::uno::Reference< com::sun::star::frame::XFrame > GetFrame();
    com::sun::star::uno::Reference< com::sun::star::util::XURLTransformer > GetURLTransformer() const { return m_xTransformer; }

    void Execute( sal_uInt16 nId );
    void Dispatch( sal_uInt16 nId,
                   const com::sun::star::uno::Reference< com::sun::star::frame::XDispatch > &rxDispatch = com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >(),
                   const com::sun::star::util::URL &rURL = com::sun::star::util::URL() );

    DECL_STATIC_LINK_TYPED( GalleryBrowser2, AsyncDispatch_Impl, void*, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
