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

#include "sddllapi.h"
#include <tools/gen.hxx>
#include <comphelper/compbase.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XSlidePreviewCacheListener.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <memory>

namespace sd::slidesorter::cache { class PageCache; }

namespace sd::presenter {

typedef comphelper::WeakComponentImplHelper<> PresenterPreviewCacheInterfaceBase;

/** wrapper around the slide preview cache.
*/
class SD_DLLPUBLIC PresenterPreviewCache final
    : public PresenterPreviewCacheInterfaceBase
{
public:
    PresenterPreviewCache ();
    virtual ~PresenterPreviewCache() override;
    PresenterPreviewCache(const PresenterPreviewCache&) = delete;
    PresenterPreviewCache& operator=(const PresenterPreviewCache&) = delete;

    void setDocumentSlides (
        const css::uno::Reference<css::container::XIndexAccess>& rxSlides,
        const css::uno::Reference<css::uno::XInterface>& rxDocument);

    void setVisibleRange (
        sal_Int32 nFirstVisibleSlideIndex,
        sal_Int32 nLastVisibleSlideIndex);

    void setPreviewSize (
        const css::geometry::IntegerSize2D& rSize);

    css::uno::Reference<css::rendering::XBitmap>
        getSlidePreview (
            sal_Int32 nSlideIndex,
            const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    void addPreviewCreationNotifyListener (
        const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener);

    void removePreviewCreationNotifyListener (
        const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener) ;

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
