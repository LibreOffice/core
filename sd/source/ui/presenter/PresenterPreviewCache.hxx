/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterPreviewCache.hxx,v $
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

#ifndef SD_PRESENTER_PRESENTER_PREVIEW_CACHE_HXX
#define SD_PRESENTER_PRESENTER_PREVIEW_CACHE_HXX

#include <com/sun/star/drawing/XSlidePreviewCache.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "cache/SlsPageCache.hxx"
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper2<
        css::lang::XInitialization,
        css::drawing::XSlidePreviewCache
    > PresenterPreviewCacheInterfaceBase;
}

/** Uno API wrapper around the slide preview cache.
*/
class PresenterPreviewCache
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterPreviewCacheInterfaceBase
{
public:
    PresenterPreviewCache (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterPreviewCache (void);

    // XInitialize

    /** Accepts no arguments.  All values that are necessary to set up a
        preview cache can be provided via methods.
    */
    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw(css::uno::Exception,css::uno::RuntimeException);


    // XSlidePreviewCache

    virtual void SAL_CALL setDocumentSlides (
        const css::uno::Reference<css::container::XIndexAccess>& rxSlides,
        const css::uno::Reference<css::uno::XInterface>& rxDocument)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setVisibleRange (
        sal_Int32 nFirstVisibleSlideIndex,
        sal_Int32 nLastVisibleSlideIndex)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setPreviewSize (
        const css::geometry::IntegerSize2D& rSize)
        throw (css::uno::RuntimeException);

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL
        getSlidePreview (
            sal_Int32 nSlideIndex,
            const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addPreviewCreationNotifyListener (
        const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL removePreviewCreationNotifyListener (
        const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL pause (void)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL resume (void)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    class PresenterCacheContext;
    Size maPreviewSize;
    ::boost::shared_ptr<PresenterCacheContext> mpCacheContext;
    ::boost::shared_ptr<sd::slidesorter::cache::PageCache> mpCache;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif
