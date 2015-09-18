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

#include "SlideSorter.hxx"

#include "tools/PropertySet.hxx"
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/SlideSorter.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/propshlp.hxx>
#include <boost/noncopyable.hpp>
#include <memory>

namespace sd { namespace slidesorter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::drawing::XSlideSorterBase,
        css::lang::XInitialization,
        css::awt::XWindowListener
    > SlideSorterServiceInterfaceBase;
}

/** Implementation of the com.sun.star.drawing.SlideSorter service.
*/
class SlideSorterService
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public SlideSorterServiceInterfaceBase
{
public:
    explicit SlideSorterService (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~SlideSorterService();
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XResourceId

    css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    sal_Bool SAL_CALL isAnchorOnly()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDrawView

    virtual void SAL_CALL setCurrentPage(
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // Attributes

    virtual css::uno::Reference<css::container::XIndexAccess> SAL_CALL getDocumentSlides()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setDocumentSlides (
        const css::uno::Reference<css::container::XIndexAccess >& rxSlides)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsHighlightCurrentSlide()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsHighlightCurrentSlide (sal_Bool bIsHighlightCurrentSlide)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsShowSelection()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsShowSelection (sal_Bool bIsShowSelection)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsCenterSelection()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsCenterSelection (sal_Bool bIsCenterSelection)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsSuspendPreviewUpdatesDuringFullScreenPresentation()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsSuspendPreviewUpdatesDuringFullScreenPresentation (
        sal_Bool bIsSuspendPreviewUpdatesDuringFullScreenPresentation)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsOrientationVertical()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsOrientationVertical (sal_Bool bIsOrientationVertical)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsSmoothScrolling()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsSmoothScrolling (sal_Bool bIsOrientationVertical)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::util::Color SAL_CALL getBackgroundColor()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setBackgroundColor (css::util::Color aBackgroundColor)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::util::Color SAL_CALL getTextColor()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setTextColor (css::util::Color aTextColor)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::util::Color SAL_CALL getSelectionColor()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setSelectionColor (css::util::Color aSelectionColor)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::util::Color SAL_CALL getHighlightColor()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setHighlightColor (css::util::Color aHighlightColor)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsUIReadOnly()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsUIReadOnly (sal_Bool bIsUIReadOnly)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL getIsShowFocus()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setIsShowFocus (sal_Bool bIsShowFocus)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    std::shared_ptr<SlideSorter> mpSlideSorter;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    std::unique_ptr<cppu::IPropertyArrayHelper> mpPropertyArrayHelper;

    void Resize();

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed() throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::slidesorter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
