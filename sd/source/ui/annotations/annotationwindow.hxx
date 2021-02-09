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

#include <vcl/ctrl.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/InterimItemWindow.hxx>
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
class AnnotationWindow;
class DrawDocShell;
class TextApiObject;

class AnnotationTextWindow : public Control
{
private:
    OutlinerView*       mpOutlinerView;
    VclPtr<AnnotationWindow>   mpAnnotationWindow;

protected:
    virtual void    Command( const CommandEvent& rCEvt ) override;

public:
    AnnotationTextWindow( AnnotationWindow* pParent, WinBits nBits );
    virtual ~AnnotationTextWindow() override;
    virtual void dispose() override;

    void SetOutlinerView( OutlinerView* pOutlinerView ) { mpOutlinerView = pOutlinerView; }
};

class AnnotationContents;

class AnnotationTextControl : public WeldEditView
{
private:
    AnnotationContents& mrContents;

public:
    AnnotationTextControl(AnnotationContents& rContents);

    virtual EditView* GetEditView() const override;

    virtual EditEngine* GetEditEngine() const override;

    virtual void EditViewScrollStateChange() override;

    void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual bool KeyInput(const KeyEvent& rKeyEvt) override;
};

class AnnotationContents final : public InterimItemWindow
{
private:
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

    static sal_Int32 GetPrefScrollbarWidth() { return 16; }
public:
    AnnotationContents(vcl::Window* pParent, DrawDocShell* pDocShell);

    void InitControls();
    void DoResize();
    void ResizeIfNecessary(::tools::Long aOldHeight, ::tools::Long aNewHeight);
    void SetScrollbar();
    void StartEdit();

    void setAnnotation(const css::uno::Reference< css::office::XAnnotation >& xAnnotation);

    ::tools::Long GetPostItTextHeight();

    DrawDocShell* DocShell() { return mpDocShell; }

    void SetLanguage(const SvxLanguageItem &aNewItem);

    void Rescale();

    void ToggleInsMode();

    bool IsProtected() const { return mbProtected; }

    OutlinerView* GetOutlinerView() { return mpOutlinerView.get();}
    virtual ~AnnotationContents() override { disposeOnce(); }
    virtual void dispose() override;

    virtual void GetFocus() override;

    void SetColor();
};

class AnnotationWindow : public FloatingWindow
{
    private:
        AnnotationManagerImpl&  mrManager;
        DrawDocShell*           mpDocShell;
        SdDrawDocument*         mpDoc;

        VclPtr<AnnotationContents> mxContents;

    public:
        AnnotationWindow( AnnotationManagerImpl& rManager, DrawDocShell* pDocShell, vcl::Window* pParent );
        virtual ~AnnotationWindow() override;
        virtual void dispose() override;

        void ExecuteSlot( sal_uInt16 nSID );

        AnnotationContents&  GetContents() const { return *mxContents; }

        void            InitControls();
        void            DoResize();

#if 0
        virtual void    Deactivate() override;
        virtual void    Command( const CommandEvent& rCEvt ) override;
#endif
        virtual void    GetFocus() override;
};

TextApiObject* getTextApiObject( const css::uno::Reference< css::office::XAnnotation >& xAnnotation );


} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
