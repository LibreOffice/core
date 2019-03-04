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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_SHELL_SLIDESORTERSERVICE_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_SHELL_SLIDESORTERSERVICE_HXX

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/XSlideSorterBase.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <memory>

namespace com { namespace sun { namespace star { namespace awt { class XWindow; } } } }

namespace sd { namespace slidesorter {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::XSlideSorterBase,
    css::lang::XInitialization,
    css::awt::XWindowListener
> SlideSorterServiceInterfaceBase;

class SlideSorter;

/** Implementation of the com.sun.star.drawing.SlideSorter service.
*/
class SlideSorterService
    : protected ::cppu::BaseMutex,
      public SlideSorterServiceInterfaceBase
{
public:
    explicit SlideSorterService();
    virtual ~SlideSorterService() override;
    SlideSorterService(const SlideSorterService&) = delete;
    SlideSorterService& operator=(const SlideSorterService&) = delete;
    virtual void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XResourceId

    css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId() override;

    sal_Bool SAL_CALL isAnchorOnly() override;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent) override;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

    // XDrawView

    virtual void SAL_CALL setCurrentPage(
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide) override;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage() override;

    // Attributes

    virtual css::uno::Reference<css::container::XIndexAccess> SAL_CALL getDocumentSlides() override;

    virtual void SAL_CALL setDocumentSlides (
        const css::uno::Reference<css::container::XIndexAccess >& rxSlides) override;

    virtual sal_Bool SAL_CALL getIsHighlightCurrentSlide() override;

    virtual void SAL_CALL setIsHighlightCurrentSlide (sal_Bool bIsHighlightCurrentSlide) override;

    virtual sal_Bool SAL_CALL getIsShowSelection() override;

    virtual void SAL_CALL setIsShowSelection (sal_Bool bIsShowSelection) override;

    virtual sal_Bool SAL_CALL getIsCenterSelection() override;

    virtual void SAL_CALL setIsCenterSelection (sal_Bool bIsCenterSelection) override;

    virtual sal_Bool SAL_CALL getIsSuspendPreviewUpdatesDuringFullScreenPresentation() override;

    virtual void SAL_CALL setIsSuspendPreviewUpdatesDuringFullScreenPresentation (
        sal_Bool bIsSuspendPreviewUpdatesDuringFullScreenPresentation) override;

    virtual sal_Bool SAL_CALL getIsOrientationVertical() override;

    virtual void SAL_CALL setIsOrientationVertical (sal_Bool bIsOrientationVertical) override;

    virtual sal_Bool SAL_CALL getIsSmoothScrolling() override;

    virtual void SAL_CALL setIsSmoothScrolling (sal_Bool bIsOrientationVertical) override;

    virtual sal_Int32 SAL_CALL getBackgroundColor() override;

    virtual void SAL_CALL setBackgroundColor (sal_Int32 aBackgroundColor) override;

    virtual sal_Int32 SAL_CALL getTextColor() override;

    virtual void SAL_CALL setTextColor (sal_Int32 aTextColor) override;

    virtual sal_Int32 SAL_CALL getSelectionColor() override;

    virtual void SAL_CALL setSelectionColor (sal_Int32 aSelectionColor) override;

    virtual sal_Int32 SAL_CALL getHighlightColor() override;

    virtual void SAL_CALL setHighlightColor (sal_Int32 aHighlightColor) override;

    virtual sal_Bool SAL_CALL getIsUIReadOnly() override;

    virtual void SAL_CALL setIsUIReadOnly (sal_Bool bIsUIReadOnly) override;

    virtual sal_Bool SAL_CALL getIsShowFocus() override;

    virtual void SAL_CALL setIsShowFocus (sal_Bool bIsShowFocus) override;

private:
    std::shared_ptr<SlideSorter> mpSlideSorter;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;

    void Resize();

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();
};

} } // end of namespace ::sd::slidesorter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
