/*************************************************************************
 *
 *  $RCSfile: galbrws2.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 15:32:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <svtools/lstner.hxx>
#include <svtools/miscopt.hxx>
#include "galbrws.hxx"

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
    ULONG               mnCurActionPos;
    GalleryBrowserMode  meMode;
    GalleryBrowserMode  meLastMode;
    BOOL                mbCurActionIsLinkage;

    void                InitSettings();

    void                ImplUpdateViews( USHORT nSelectionId );
    void                ImplUpdateInfoBar();
    ULONG               ImplGetSelectedItemId( const Point* pSelPosPixel, Point& rSelPos );
    void                ImplSelectItemId( ULONG nItemId );
    void                ImplExecute( USHORT nId );

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

    static String       GetItemText( const GalleryTheme& rTheme, const SgaObject& rObj, ULONG nItemTextFlags );

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
    BOOL                GetVCDrawModel( FmFormModel& rModel ) const;
    BOOL                IsLinkage() const;

    sal_Int8            AcceptDrop( DropTargetHelper& rTarget, const AcceptDropEvent& rEvt );
    sal_Int8            ExecuteDrop( DropTargetHelper& rTarget, const ExecuteDropEvent& rEvt );
    void                StartDrag( Window* pWindow, const Point* pDragPoint = NULL );
    void                TogglePreview( Window* pWindow, const Point* pPreviewPoint = NULL );
    void                ShowContextMenu( Window* pWindow, const Point* pContextPoint = NULL );
    BOOL                KeyInput( const KeyEvent& rEvt, Window* pWindow );
};

#endif
