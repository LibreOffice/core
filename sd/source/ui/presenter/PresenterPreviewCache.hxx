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

#include <com/sun/star/drawing/XSlidePreviewCache.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <tools/gen.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <memory>

namespace sd::slidesorter::cache { class PageCache; }

namespace sd::presenter {

typedef ::cppu::WeakComponentImplHelper<
    css::lang::XInitialization,
    css::drawing::XSlidePreviewCache
> PresenterPreviewCacheInterfaceBase;

/** Uno API wrapper around the slide preview cache.
*/
class PresenterPreviewCache
    : private ::cppu::BaseMutex,
      public PresenterPreviewCacheInterfaceBase
{
public:
    PresenterPreviewCache ();
    virtual ~PresenterPreviewCache() override;
    PresenterPreviewCache(const PresenterPreviewCache&) = delete;
    PresenterPreviewCache& operator=(const PresenterPreviewCache&) = delete;

    // XInitialize

    /** Accepts no arguments.  All values that are necessary to set up a
        preview cache can be provided via methods.
    */
    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // XSlidePreviewCache

    virtual void SAL_CALL setDocumentSlides (
        const css::uno::Reference<css::container::XIndexAccess>& rxSlides,
        const css::uno::Reference<css::uno::XInterface>& rxDocument) override;

    virtual void SAL_CALL setVisibleRange (
        sal_Int32 nFirstVisibleSlideIndex,
        sal_Int32 nLastVisibleSlideIndex) override;

    virtual void SAL_CALL setPreviewSize (
        const css::geometry::IntegerSize2D& rSize) override;

    virtual css::uno::Reference<css::rendering::XBitmap> SAL_CALL
        getSlidePreview (
            sal_Int32 nSlideIndex,
            const css::uno::Reference<css::rendering::XCanvas>& rxCanvas) override;

    virtual void SAL_CALL addPreviewCreationNotifyListener (
        const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener) override;

    virtual void SAL_CALL removePreviewCreationNotifyListener (
        const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener) override;

    virtual void SAL_CALL pause() override;

    virtual void SAL_CALL resume() override;

private:
    class PresenterCacheContext;
    Size maPreviewSize;
    std::shared_ptr<PresenterCacheContext> mpCacheContext;
    std::shared_ptr<sd::slidesorter::cache::PageCache> mpCache;

    /** @throws css::lang::DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed();
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
