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

#include <vcl/weld.hxx>
#include <tools/long.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svx/weldeditview.hxx>

namespace com::sun::star::office { class XAnnotation; }

class OutlinerView;
class Outliner;
class SvxLanguageItem;
class SdDrawDocument;

namespace sd {

class AnnotationManagerImpl;
class DrawDocShell;
class TextApiObject;

class AnnotationWindow;

class AnnotationTextWindow : public WeldEditView
{
private:
    AnnotationWindow& mrContents;

public:
    AnnotationTextWindow(AnnotationWindow& rContents);

    virtual EditView* GetEditView() const override;

    virtual EditEngine* GetEditEngine() const override;

    virtual void EditViewScrollStateChange() override;

    void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;
    virtual bool KeyInput(const KeyEvent& rKeyEvt) override;
    virtual bool Command(const CommandEvent& rCEvt) override;
};

class AnnotationWindow final
{
private:
    std::unique_ptr<weld::Builder> mxBuilder;
    std::unique_ptr<weld::Popover> mxPopover;
    std::unique_ptr<weld::Widget> mxContainer;

    DrawDocShell* mpDocShell;
    SdDrawDocument* mpDoc;

    bool mbReadonly;
    bool mbProtected;

    css::uno::Reference< css::office::XAnnotation > mxAnnotation;

public:
    Color maColor;
    Color maColorDark;
    Color maColorLight;

private:
    vcl::Font maLabelFont;

    std::unique_ptr<OutlinerView> mpOutlinerView;
    std::unique_ptr<::Outliner>  mpOutliner;

    std::unique_ptr<weld::ScrolledWindow> mxVScrollbar;
    std::unique_ptr<WeldEditView> mxTextControl;
    std::unique_ptr<weld::CustomWeld> mxTextControlWin;
    std::unique_ptr<weld::Label> mxMeta;
    std::unique_ptr<weld::MenuButton> mxMenuButton;

    DECL_LINK(ScrollHdl, weld::ScrolledWindow&, void);
    DECL_LINK(MenuItemSelectedHdl, const OString&, void);

    void FillMenuButton();
    void InitControls();

    void SetMapMode(const MapMode& rNewMapMode);
    void setAnnotation(const css::uno::Reference<css::office::XAnnotation>& xAnnotation);

    static sal_Int32 GetPrefScrollbarWidth() { return 16; }
public:
    AnnotationWindow(weld::Window* pParent, const ::tools::Rectangle& rRect, DrawDocShell* pDocShell,
                     const css::uno::Reference<css::office::XAnnotation>& xAnnotation);

    void connect_closed(const Link<weld::Popover&, void>& rLink) { mxPopover->connect_closed(rLink); }

    void DoResize();
    void ResizeIfNecessary(::tools::Long aOldHeight, ::tools::Long aNewHeight);
    void SetScrollbar();
    void StartEdit();

    const css::uno::Reference<css::office::XAnnotation>& getAnnotation() const { return mxAnnotation; }

    void SaveToDocument();

    ::tools::Long GetPostItTextHeight();

    DrawDocShell* DocShell() { return mpDocShell; }

    void SetLanguage(const SvxLanguageItem &aNewItem);

    void Rescale();

    void ToggleInsMode();

    bool IsProtected() const { return mbProtected; }

    OutlinerView* GetOutlinerView() { return mpOutlinerView.get();}
    ::Outliner* GetOutliner() { return mpOutliner.get();}
    ~AnnotationWindow();

    void SetColor();
};

TextApiObject* getTextApiObject( const css::uno::Reference< css::office::XAnnotation >& xAnnotation );


} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
