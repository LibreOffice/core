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

#ifndef SDEXT_PRESENTER_PRESENTER_PANE_BORDER_PAINTER_HXX
#define SDEXT_PRESENTER_PRESENTER_PANE_BORDER_PAINTER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/framework/XPaneBorderPainter.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterPane;
class PresenterTheme;

namespace {
    typedef ::cppu::WeakComponentImplHelper1<
        css::drawing::framework::XPaneBorderPainter
    > PresenterPaneBorderPainterInterfaceBase;
}

/** This class is responsible for painting window borders of PresenterPane
    objects.
*/
class PresenterPaneBorderPainter
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public PresenterPaneBorderPainterInterfaceBase
{
public:
    PresenterPaneBorderPainter (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterPaneBorderPainter (void);

    /** Transform the bounding box of the window content to the outer
        bounding box of the border that is painted around it.
        @param rsPaneURL
            Specifies the pane style that is used to determine the border sizes.
        @param rInnerBox
            The rectangle of the inner window content.
    */
    css::awt::Rectangle AddBorder (
        const ::rtl::OUString& rsPaneURL,
        const css::awt::Rectangle& rInnerBox,
        const css::drawing::framework::BorderType eBorderType) const;

    /** Transorm the outer bounding box of a window to the bounding box of
        the inner content area.
        @param rsPaneURL
            Specifies the pane style that is used to determine the border sizes.
        @param rOuterBox
            The bounding box of the rectangle around the window.
        @param bIsTitleVisible
            This flag controls whether the upper part of the frame is
            supposed to contain the window title.
    */
    css::awt::Rectangle RemoveBorder (
        const ::rtl::OUString& rsPaneURL,
        const css::awt::Rectangle& rOuterBox,
        const css::drawing::framework::BorderType eBorderType) const;

    void SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme);

    class Renderer;

    // XPaneBorderPainter

    virtual css::awt::Rectangle SAL_CALL addBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        css::drawing::framework::BorderType eBorderType)
        throw(css::uno::RuntimeException);

    virtual css::awt::Rectangle SAL_CALL removeBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        css::drawing::framework::BorderType eBorderType)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL paintBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rOuterBorderRectangle,
        const css::awt::Rectangle& rRepaintArea,
        const rtl::OUString& rsTitle)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL paintBorderWithCallout (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rOuterBorderRectangle,
        const css::awt::Rectangle& rRepaintArea,
        const rtl::OUString& rsTitle,
        const css::awt::Point& rCalloutAnchor)
        throw(css::uno::RuntimeException);

    virtual css::awt::Point SAL_CALL getCalloutOffset (
        const rtl::OUString& rsPaneBorderStyleName)
        throw(css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    ::boost::scoped_ptr<Renderer> mpRenderer;

    /** When the theme for the border has not yet been loaded then try again
        when this method is called.
        @return
            Returns <TRUE/> only one time when the theme is loaded and/or the
            renderer is initialized.
    */
    bool ProvideTheme (
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    bool ProvideTheme (void);

    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
