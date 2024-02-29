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
#include <vcl/customweld.hxx>
#include <svtools/valueset.hxx>
#include <vcl/GraphicObject.hxx>
#include <svx/svxdllapi.h>

class GalleryDragDrop;
class GalleryTheme;
class GalleryBrowser1;
class INetURLObject;

class GalleryPreview final : public weld::CustomWidgetController
{
private:

    std::unique_ptr<GalleryDragDrop> mxDragDropTargetHelper;
    std::unique_ptr<weld::ScrolledWindow> mxScrolledWindow;
    GraphicObject aGraphicObj;
    tools::Rectangle aPreviewRect;
    GalleryBrowser1* mpParent;
    GalleryTheme* mpTheme;

    bool             ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const;

    // Window
    virtual void     Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void     SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual bool     MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool     Command(const CommandEvent& rCEvt) override;
    virtual bool     KeyInput( const KeyEvent& rKEvt ) override;

public:

    GalleryPreview(GalleryBrowser1* pParent, std::unique_ptr<weld::ScrolledWindow> xScrolledWindow);
    void SetTheme(GalleryTheme* pTheme) { mpTheme = pTheme; }
    virtual ~GalleryPreview() override;

    virtual bool StartDrag() override;

    virtual void Show() override;
    virtual void Hide() override;

    void                SetGraphic( const Graphic& rGraphic ) { aGraphicObj.SetGraphic( rGraphic ); }
    static void         PreviewMedia( const INetURLObject& rURL );
};

class SVXCORE_DLLPUBLIC DialogGalleryPreview final : public weld::CustomWidgetController
{
private:
    GraphicObject aGraphicObj;
    tools::Rectangle aPreviewRect;

    SVX_DLLPRIVATE bool             ImplGetGraphicCenterRect( const Graphic& rGraphic, tools::Rectangle& rResultRect ) const;

    SVX_DLLPRIVATE virtual void     Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

public:

    DialogGalleryPreview();

    virtual void        SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void                SetGraphic( const Graphic& rGraphic ) { aGraphicObj.SetGraphic( rGraphic ); }
    bool                SetGraphic( const INetURLObject& );
};

class GalleryIconView final : public ValueSet
{
private:
    std::unique_ptr<GalleryDragDrop> mxDragDropTargetHelper;

    GalleryBrowser1*    mpParent;
    GalleryTheme*       mpTheme;

    // ValueSet
    virtual void        UserDraw( const UserDrawEvent& rUDEvt ) override;

    // Window
    virtual bool        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool        Command( const CommandEvent& rCEvt ) override;
    virtual bool        KeyInput( const KeyEvent& rKEvt ) override;

    virtual void        SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

public:

    GalleryIconView(GalleryBrowser1* pParent, std::unique_ptr<weld::ScrolledWindow> xScrolledWindow);
    void SetTheme(GalleryTheme* pTheme) { mpTheme = pTheme; }
    virtual ~GalleryIconView() override;

    virtual bool StartDrag() override;

    static void drawTransparenceBackground(vcl::RenderContext& rOut, const Point& rPos, const Size& rSize);
};

#endif // INCLUDED_SVX_GALCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
