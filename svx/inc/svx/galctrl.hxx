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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
