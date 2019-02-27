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

#ifndef INCLUDED_SVX_GALCTRL_HXX
#define INCLUDED_SVX_GALCTRL_HXX

#include <vcl/graph.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>
#include <vcl/customweld.hxx>
#include <svl/slstitm.hxx>
#include <vcl/transfer.hxx>
#include <svtools/valueset.hxx>
#include <svtools/brwbox.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/galmisc.hxx>
#include <svx/svxdllapi.h>

class GalleryTheme;
class GalleryBrowser2;

class SVX_DLLPUBLIC GalleryPreview : public vcl::Window, public DropTargetHelper, public DragSourceHelper
{
private:

    GraphicObject       aGraphicObj;
    tools::Rectangle           aPreviewRect;
    GalleryTheme* const       mpTheme;

    SVX_DLLPRIVATE bool             ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const;
    SVX_DLLPRIVATE void             InitSettings();

    // Window
    SVX_DLLPRIVATE virtual void     Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    SVX_DLLPRIVATE virtual Size     GetOptimalSize() const override;
    SVX_DLLPRIVATE virtual void     MouseButtonDown(const MouseEvent& rMEvt) override;
    SVX_DLLPRIVATE virtual void     Command(const CommandEvent& rCEvt) override;
    SVX_DLLPRIVATE virtual void     KeyInput( const KeyEvent& rKEvt ) override;
    SVX_DLLPRIVATE virtual void     DataChanged( const DataChangedEvent& rDCEvt ) override;


    // DropTargetHelper
    SVX_DLLPRIVATE virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    SVX_DLLPRIVATE virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    // DragSourceHelper
    SVX_DLLPRIVATE virtual void     StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

public:

    GalleryPreview(vcl::Window* pParent,
        WinBits nStyle = WB_TABSTOP | WB_BORDER,
        GalleryTheme* pTheme = nullptr);

    void                SetGraphic( const Graphic& rGraphic ) { aGraphicObj.SetGraphic( rGraphic ); }
    static void         PreviewMedia( const INetURLObject& rURL );
};

class SVX_DLLPUBLIC SvxGalleryPreview : public weld::CustomWidgetController
{
private:
    GraphicObject aGraphicObj;
    tools::Rectangle aPreviewRect;

    SVX_DLLPRIVATE bool             ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const;

    SVX_DLLPRIVATE virtual void     Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

public:

    SvxGalleryPreview();

    virtual void        SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void                SetGraphic( const Graphic& rGraphic ) { aGraphicObj.SetGraphic( rGraphic ); }
    bool                SetGraphic( const INetURLObject& );
};

class GalleryIconView : public ValueSet, public DropTargetHelper, public DragSourceHelper
{
    using ValueSet::StartDrag;

private:

    GalleryTheme*       mpTheme;

    void                InitSettings();

    // ValueSet
    virtual void        UserDraw( const UserDrawEvent& rUDEvt ) override;

    // Window
    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

public:

                        GalleryIconView( GalleryBrowser2* pParent, GalleryTheme* pTheme );
};

class GalleryListView : public BrowseBox
{
    using BrowseBox::AcceptDrop;
    using BrowseBox::ExecuteDrop;

private:

    Link<GalleryListView*,void>  maSelectHdl;
    GalleryTheme*       mpTheme;
    sal_uInt32          mnCurRow;

    void                InitSettings();

    // BrowseBox
    virtual bool        SeekRow( long nRow ) override;
    virtual void        PaintField( vcl::RenderContext& rDev, const tools::Rectangle& rRect, sal_uInt16 nColumnId ) const override;
    virtual void        DoubleClick( const BrowserMouseEvent& rEvt ) override;
    virtual void        Select() override;
    virtual sal_Int8    AcceptDrop( const BrowserAcceptDropEvent& rEvt ) override;
    virtual sal_Int8    ExecuteDrop( const BrowserExecuteDropEvent& rEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;

    // Window
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;

public:

                        GalleryListView( GalleryBrowser2* pParent, GalleryTheme* pTheme );

    void                SetSelectHdl( const Link<GalleryListView*,void>& rSelectHdl ) { maSelectHdl = rSelectHdl; }

    /** GetCellText returns the text at the given position
        @param  _nRow
            the number of the row
        @param  _nColId
            the ID of the column
        @return
            the text out of the cell
    */
    virtual OUString  GetCellText(long _nRow, sal_uInt16 _nColId) const override;

    // from IAccessibleTableProvider
    virtual tools::Rectangle GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex) override;
    virtual sal_Int32 GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint) override;
};

#endif // INCLUDED_SVX_GALCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
