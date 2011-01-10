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

#ifndef _SVX_GALCTRL_HXX_
#define _SVX_GALCTRL_HXX_

#include <vcl/dialog.hxx>
#include <vcl/graph.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/sound.hxx>
#include <svl/slstitm.hxx>
#include <svtools/transfer.hxx>
#include <svtools/valueset.hxx>
#include <svtools/brwbox.hxx>
#include <svtools/grfmgr.hxx>
#include "svx/galmisc.hxx"
#include "svx/svxdllapi.h"

// ------------
// - Forwards -
// ------------

class GalleryTheme;
class GalleryBrowser2;

// ------------------
// - GalleryPreview -
// ------------------

class SVX_DLLPUBLIC GalleryPreview : public Window, public DropTargetHelper, public DragSourceHelper
{
private:

    GraphicObject       aGraphicObj;
    Rectangle           aPreviewRect;
    GalleryTheme*       mpTheme;

    SVX_DLLPRIVATE sal_Bool             ImplGetGraphicCenterRect( const Graphic& rGraphic, Rectangle& rResultRect ) const;
    SVX_DLLPRIVATE void                InitSettings();

    // Window
    SVX_DLLPRIVATE virtual void         Paint(const Rectangle& rRect);
    SVX_DLLPRIVATE virtual void         MouseButtonDown(const MouseEvent& rMEvt);
    SVX_DLLPRIVATE virtual void     Command(const CommandEvent& rCEvt);
    SVX_DLLPRIVATE virtual void     KeyInput( const KeyEvent& rKEvt );
    SVX_DLLPRIVATE virtual void        DataChanged( const DataChangedEvent& rDCEvt );


    // DropTargetHelper
    SVX_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    SVX_DLLPRIVATE virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper
    SVX_DLLPRIVATE virtual void     StartDrag( sal_Int8 nAction, const Point& rPosPixel );

                        DECL_LINK( MenuSelectHdl, Menu* );

public:

                        GalleryPreview( GalleryBrowser2* pParent, GalleryTheme* pTheme );
                        GalleryPreview( Window* pParent, const ResId& rResId  );
                        ~GalleryPreview();

    void                SetGraphic( const Graphic& rGraphic ) { aGraphicObj.SetGraphic( rGraphic ); }
    bool                SetGraphic( const INetURLObject& );
    void                PreviewMedia( const INetURLObject& rURL );
};

// -------------------
// - GalleryIconView -
// -------------------

class GalleryIconView : public ValueSet, public DropTargetHelper, public DragSourceHelper
{
    using ValueSet::StartDrag;

private:

    GalleryTheme*       mpTheme;

    void                InitSettings();

    // ValueSet
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    // Window
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

public:

                        GalleryIconView( GalleryBrowser2* pParent, GalleryTheme* pTheme );
                        ~GalleryIconView();
};

// -------------------
// - GalleryListView -
// -------------------

class GalleryListView : public BrowseBox
{
    using BrowseBox::AcceptDrop;
    using BrowseBox::ExecuteDrop;

private:

    Link                maSelectHdl;
    GalleryTheme*       mpTheme;
    sal_uIntPtr               mnCurRow;
    sal_Bool                mbInit;

    void                InitSettings();

    // BrowseBox
    virtual sal_Bool        SeekRow( long nRow );
    virtual void        PaintField( OutputDevice& rDev, const Rectangle& rRect, sal_uInt16 nColumnId ) const;
    virtual void        DoubleClick( const BrowserMouseEvent& rEvt );
    virtual void        Select();
    virtual sal_Int8    AcceptDrop( const BrowserAcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const BrowserExecuteDropEvent& rEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );

    // Window
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

public:

                        GalleryListView( GalleryBrowser2* pParent, GalleryTheme* pTheme );
                        ~GalleryListView();

    void                SetSelectHdl( const Link& rSelectHdl ) { maSelectHdl = rSelectHdl; }

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual String  GetCellText(long _nRow, sal_uInt16 _nColId) const;

    // from IAccessibleTableProvider
    virtual Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex);
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint);
};

#endif // _SVX_GALCTRL_HXX_
