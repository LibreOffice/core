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

#include <com/sun/star/office/XAnnotation.hpp>

#include <vcl/ctrl.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/floatwin.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>

#include <svx/sdr/overlay/overlayobject.hxx>
#include <editeng/editstat.hxx>

class OutlinerView;
class Outliner;
class ScrollBar;
class MultiLineEdit;
class SvxLanguageItem;
class SdDrawDocument;

namespace sd {

class AnnotationManagerImpl;
class AnnotationWindow;
class DrawDocShell;
class TextApiObject;
class View;

class AnnotationTextWindow : public Control
{
private:
    OutlinerView*       mpOutlinerView;
    VclPtr<AnnotationWindow>   mpAnnotationWindow;

protected:
    virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKeyEvt ) SAL_OVERRIDE;
    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    LoseFocus() SAL_OVERRIDE;

public:
    AnnotationTextWindow( AnnotationWindow* pParent, WinBits nBits );
    virtual ~AnnotationTextWindow();
    virtual void dispose() SAL_OVERRIDE;

    void SetOutlinerView( OutlinerView* pOutlinerView ) { mpOutlinerView = pOutlinerView; }

    virtual OUString GetSurroundingText() const SAL_OVERRIDE;
    virtual Selection GetSurroundingTextSelection() const SAL_OVERRIDE;

    virtual void    GetFocus() SAL_OVERRIDE;
};

class AnnotationWindow : public FloatingWindow
{
    private:
        AnnotationManagerImpl&  mrManager;
        DrawDocShell*           mpDocShell;
        View*                   mpView;
        SdDrawDocument*         mpDoc;

        OutlinerView*           mpOutlinerView;
        Outliner*               mpOutliner;
        VclPtr<ScrollBar>              mpVScrollbar;
        ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation > mxAnnotation;
        bool                    mbReadonly;
        bool                    mbProtected;
        bool                    mbMouseOverButton;
        VclPtr<AnnotationTextWindow>   mpTextWindow;
        VclPtr<MultiLineEdit>          mpMeta;
        Rectangle               maRectMetaButton;
        basegfx::B2DPolygon     maPopupTriangle;

    protected:
        void            SetSizePixel( const Size& rNewSize ) SAL_OVERRIDE;

        DECL_LINK_TYPED(ScrollHdl, ScrollBar*, void);

    public:
        AnnotationWindow( AnnotationManagerImpl& rManager, DrawDocShell* pDocShell, vcl::Window* pParent );
        virtual ~AnnotationWindow();
        virtual void dispose() SAL_OVERRIDE;

        void StartEdit();

        SvxLanguageItem GetLanguage();

        void setAnnotation( const ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation, bool bGrabFocus = false );

        void ExecuteSlot( sal_uInt16 nSID );

        DrawDocShell*           DocShell()      { return mpDocShell; }
        OutlinerView*           getView()       { return mpOutlinerView; }
        sd::View*               DocView()       { return mpView; }
        Outliner*               Engine()        { return mpOutliner; }
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

        virtual void    Deactivate() SAL_OVERRIDE;
        virtual void    Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect) SAL_OVERRIDE;
        virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
        virtual void    GetFocus() SAL_OVERRIDE;

        void            SetColor();

        Color           maColor;
        Color           maColorDark;
        Color           maColorLight;
};

TextApiObject* getTextApiObject( const css::uno::Reference< css::office::XAnnotation >& xAnnotation );


} // namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
