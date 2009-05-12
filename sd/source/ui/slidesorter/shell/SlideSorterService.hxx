/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterService.hxx,v $
 *
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_SLIDE_SORTER_SERVICE_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_SERVICE_HXX

#include "SlideSorter.hxx"

#include "tools/PropertySet.hxx"
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/SlideSorter.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/propshlp.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace slidesorter {

namespace {
    typedef ::cppu::WeakComponentImplHelper3 <
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
    virtual ~SlideSorterService (void);
    virtual void SAL_CALL disposing (void);


    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // XResourceId

    css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId (void)
        throw (css::uno::RuntimeException);

    sal_Bool SAL_CALL isAnchorOnly (void)
        throw (css::uno::RuntimeException);


    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XDrawView

    virtual void SAL_CALL setCurrentPage(
        const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::drawing::XDrawPage> SAL_CALL getCurrentPage (void)
        throw (css::uno::RuntimeException);


    // Attributes

    virtual css::uno::Reference<css::container::XIndexAccess> SAL_CALL getDocumentSlides (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setDocumentSlides (
        const css::uno::Reference<css::container::XIndexAccess >& rxSlides)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsHighlightCurrentSlide (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsHighlightCurrentSlide (::sal_Bool bIsHighlightCurrentSlide)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsShowSelection (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsShowSelection (sal_Bool bIsShowSelection)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsCenterSelection (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsCenterSelection (sal_Bool bIsCenterSelection)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsSuspendPreviewUpdatesDuringFullScreenPresentation (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsSuspendPreviewUpdatesDuringFullScreenPresentation (
        sal_Bool bIsSuspendPreviewUpdatesDuringFullScreenPresentation)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsOrientationVertical (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsOrientationVertical (sal_Bool bIsOrientationVertical)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsSmoothScrolling (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsSmoothScrolling (sal_Bool bIsOrientationVertical)
        throw (css::uno::RuntimeException);

    virtual css::util::Color SAL_CALL getBackgroundColor (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setBackgroundColor (css::util::Color aBackgroundColor)
        throw (css::uno::RuntimeException);

    virtual css::util::Color SAL_CALL getTextColor (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setTextColor (css::util::Color aTextColor)
        throw (css::uno::RuntimeException);

    virtual css::util::Color SAL_CALL getSelectionColor (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setSelectionColor (css::util::Color aSelectionColor)
        throw (css::uno::RuntimeException);

    virtual css::util::Color SAL_CALL getHighlightColor (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setHighlightColor (css::util::Color aHighlightColor)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsUIReadOnly (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsUIReadOnly (sal_Bool bIsUIReadOnly)
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getIsShowFocus (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setIsShowFocus (sal_Bool bIsShowFocus)
        throw (css::uno::RuntimeException);

private:
    ::boost::shared_ptr<SlideSorter> mpSlideSorter;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    ::boost::scoped_ptr<cppu::IPropertyArrayHelper> mpPropertyArrayHelper;

    void Resize (void);
    void Rearrange (void);

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::slidesorter

#endif
