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

#ifndef INCLUDED_SD_SOURCE_UI_ANNOTATIONS_ANNOTATIONWINDOW_HXX
#define INCLUDED_SD_SOURCE_UI_ANNOTATIONS_ANNOTATIONWINDOW_HXX

#include <vcl/ctrl.hxx>
#include <vcl/floatwin.hxx>
#include <svtools/svmedit.hxx>
#include <vcl/scrbar.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>

namespace com { namespace sun { namespace star { namespace office { class XAnnotation; } } } }

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
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& rRect) override;
    virtual void    KeyInput( const KeyEvent& rKeyEvt ) override;
    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;

public:
    AnnotationTextWindow( AnnotationWindow* pParent, WinBits nBits );
    virtual ~AnnotationTextWindow() override;
    virtual void dispose() override;

    void SetOutlinerView( OutlinerView* pOutlinerView ) { mpOutlinerView = pOutlinerView; }

    virtual OUString GetSurroundingText() const override;
    virtual Selection GetSurroundingTextSelection() const override;
};

class AnnotationWindow : public FloatingWindow
{
    private:
        AnnotationManagerImpl&  mrManager;
        DrawDocShell*           mpDocShell;
        SdDrawDocument*         mpDoc;

        std::unique_ptr<OutlinerView> mpOutlinerView;
        std::unique_ptr<::Outliner>   mpOutliner;
        VclPtr<ScrollBar>       mpVScrollbar;
        css::uno::Reference< css::office::XAnnotation > mxAnnotation;
        bool const              mbReadonly;
        bool                    mbProtected;
        bool                    mbMouseOverButton;
        VclPtr<AnnotationTextWindow>   mpTextWindow;
        VclPtr<MultiLineEdit>   mpMeta;
        ::tools::Rectangle               maRectMetaButton;
        basegfx::B2DPolygon     maPopupTriangle;

    protected:
        DECL_LINK(ScrollHdl, ScrollBar*, void);

    public:
        AnnotationWindow( AnnotationManagerImpl& rManager, DrawDocShell* pDocShell, vcl::Window* pParent );
        virtual ~AnnotationWindow() override;
        virtual void dispose() override;

        void StartEdit();

        void setAnnotation( const css::uno::Reference< css::office::XAnnotation >& xAnnotation );

        void ExecuteSlot( sal_uInt16 nSID );

        DrawDocShell*           DocShell()      { return mpDocShell; }
        OutlinerView*           getView()       { return mpOutlinerView.get(); }
        ::Outliner*             Engine()        { return mpOutliner.get(); }
        SdDrawDocument*         Doc()           { return mpDoc; }

        long            GetPostItTextHeight();

        void            InitControls();
        void            DoResize();
        void            ResizeIfNecessary(long aOldHeight, long aNewHeight);
        void            SetScrollbar();

        void            Rescale();

        bool            IsProtected() { return mbProtected; }

        void            SetLanguage(const SvxLanguageItem &aNewItem);

        static sal_Int32 GetScrollbarWidth() { return 16; }

        void            ToggleInsMode();

        virtual void    Deactivate() override;
        virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const ::tools::Rectangle& rRect) override;
        virtual void    MouseMove( const MouseEvent& rMEvt ) override;
        virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void    Command( const CommandEvent& rCEvt ) override;
        virtual void    GetFocus() override;

        void            SetColor();

        Color           maColor;
        Color           maColorDark;
        Color           maColorLight;
};

TextApiObject* getTextApiObject( const css::uno::Reference< css::office::XAnnotation >& xAnnotation );


} // namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
