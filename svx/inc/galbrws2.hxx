/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_GALBRWS2_HXX_
#define _SVX_GALBRWS2_HXX_

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/transfer.hxx>
#include <svl/lstner.hxx>
#include <svtools/miscopt.hxx>
#include "svx/galbrws.hxx"

// ----------------------
// - GalleryBrowserMode -
// ----------------------

enum GalleryBrowserMode
{
    GALLERYBROWSERMODE_NONE = 0,
    GALLERYBROWSERMODE_ICON = 1,
    GALLERYBROWSERMODE_LIST = 2,
    GALLERYBROWSERMODE_PREVIEW = 3
};

// -----------------
// - GalleryTravel -
// -----------------

enum GalleryBrowserTravel
{
    GALLERYBROWSERTRAVEL_CURRENT = 0,
    GALLERYBROWSERTRAVEL_FIRST = 1,
    GALLERYBROWSERTRAVEL_LAST = 2,
    GALLERYBROWSERTRAVEL_PREVIOUS = 3,
    GALLERYBROWSERTRAVEL_NEXT = 4
};

// ------------------------
// - GalleryItemTextFlags -
// ------------------------

#define GALLERY_ITEM_THEMENAME  0x00000001
#define GALLERY_ITEM_TITLE      0x00000002
#define GALLERY_ITEM_PATH       0x00000004

// ------------------
// - GalleryToolBox -
// ------------------

class GalleryToolBox : public ToolBox
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt );

public:

                    GalleryToolBox( GalleryBrowser2* pParent );
                    ~GalleryToolBox();
};

// -------------------
// - GalleryBrowser2 -
// -------------------

class Gallery;
class GalleryTheme;
class GalleryIconView;
class GalleryListView;
class GalleryPreview;
class Menu;
class SgaObject;

class GalleryBrowser2 : public Control, public SfxListener
{
    friend class GalleryBrowser;
    using Control::Notify;
    using Window::KeyInput;

private:

    SvtMiscOptions      maMiscOptions;
    Gallery*            mpGallery;
    GalleryTheme*       mpCurTheme;
    GalleryIconView*    mpIconView;
    GalleryListView*    mpListView;
    GalleryPreview*     mpPreview;
    GalleryToolBox      maViewBox;
    FixedLine           maSeparator;
    FixedText           maInfoBar;
    Point               maDragStartPos;
    sal_uIntPtr             mnCurActionPos;
    GalleryBrowserMode  meMode;
    GalleryBrowserMode  meLastMode;
    sal_Bool                mbCurActionIsLinkage;

    void                InitSettings();

    void                ImplUpdateViews( sal_uInt16 nSelectionId );
    void                ImplUpdateInfoBar();
    sal_uIntPtr               ImplGetSelectedItemId( const Point* pSelPosPixel, Point& rSelPos );
    void                ImplSelectItemId( sal_uIntPtr nItemId );
    void                ImplExecute( sal_uInt16 nId );

    // Control
    virtual void        Resize();
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    // SfxListener
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                        DECL_LINK( MenuSelectHdl, Menu* pMenu );
                        DECL_LINK( SelectObjectHdl, void* );
                        DECL_LINK( SelectTbxHdl, ToolBox* );
                        DECL_LINK( MiscHdl, void* );

private:

    static GalleryBrowserMode meInitMode;

public:

    static String       GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, sal_uIntPtr nItemTextFlags );

public:

                        GalleryBrowser2( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery );
                        ~GalleryBrowser2();

    void                SelectTheme( const String& rThemeName );

    GalleryBrowserMode  GetMode() const { return meMode; }
    void                SetMode( GalleryBrowserMode eMode );

    Window*             GetViewWindow() const;

    void                Travel( GalleryBrowserTravel eTravel );

    INetURLObject       GetURL() const;
    String              GetFilterName() const;
    Graphic             GetGraphic() const;
    sal_Bool                GetVCDrawModel( FmFormModel& rModel ) const;
    sal_Bool                IsLinkage() const;

    sal_Int8            AcceptDrop( DropTargetHelper& rTarget, const AcceptDropEvent& rEvt );
    sal_Int8            ExecuteDrop( DropTargetHelper& rTarget, const ExecuteDropEvent& rEvt );
    void                StartDrag( Window* pWindow, const Point* pDragPoint = NULL );
    void                TogglePreview( Window* pWindow, const Point* pPreviewPoint = NULL );
    void                ShowContextMenu( Window* pWindow, const Point* pContextPoint = NULL );
    sal_Bool                KeyInput( const KeyEvent& rEvt, Window* pWindow );
};

#endif
