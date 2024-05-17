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

#include "PresenterPreviewCache.hxx"

#include <cache/SlsPageCache.hxx>
#include <cache/SlsCacheContext.hxx>
#include <vcl/bitmapex.hxx>
#include <sdpage.hxx>
#include <cppcanvas/vclfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <osl/diagnose.h>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::cache;

namespace sd::presenter {

class PresenterPreviewCache::PresenterCacheContext : public CacheContext
{
public:
    PresenterCacheContext();

    void SetDocumentSlides (
        const Reference<container::XIndexAccess>& rxSlides,
        const Reference<XInterface>& rxDocument);
    void SetVisibleSlideRange (
        const sal_Int32 nFirstVisibleSlideIndex,
        const sal_Int32 nLastVisibleSlideIndex);
    const SdrPage* GetPage (const sal_Int32 nSlideIndex) const;
    void AddPreviewCreationNotifyListener (const Reference<drawing::XSlidePreviewCacheListener>& rxListener);
    void RemovePreviewCreationNotifyListener (const Reference<drawing::XSlidePreviewCacheListener>& rxListener);

    // CacheContext
    virtual void NotifyPreviewCreation (CacheKey aKey) override;
    virtual bool IsIdle() override;
    virtual bool IsVisible (CacheKey aKey) override;
    virtual const SdrPage* GetPage (CacheKey aKey) override;
    virtual std::shared_ptr<std::vector<CacheKey> > GetEntryList (bool bVisible) override;
    virtual sal_Int32 GetPriority (CacheKey aKey) override;
    virtual css::uno::Reference<css::uno::XInterface> GetModel() override;

private:
    Reference<container::XIndexAccess> mxSlides;
    Reference<XInterface> mxDocument;
    sal_Int32 mnFirstVisibleSlideIndex;
    sal_Int32 mnLastVisibleSlideIndex;
    typedef ::std::vector<css::uno::Reference<css::drawing::XSlidePreviewCacheListener> > ListenerContainer;
    ListenerContainer maListeners;

    void CallListeners (const sal_Int32 nSlideIndex);
};

//===== PresenterPreviewCache =================================================

PresenterPreviewCache::PresenterPreviewCache ()
    : maPreviewSize(Size(200,200)),
      mpCacheContext(std::make_shared<PresenterCacheContext>()),
      mpCache(std::make_shared<PageCache>(maPreviewSize, Bitmap::HasFastScale(), mpCacheContext))
{
}

PresenterPreviewCache::~PresenterPreviewCache()
{
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterPreviewCache::initialize (const Sequence<Any>& rArguments)
{
    if (rArguments.hasElements())
        throw RuntimeException();
}

OUString PresenterPreviewCache::getImplementationName() {
    return u"com.sun.star.comp.Draw.PresenterPreviewCache"_ustr;
}

sal_Bool PresenterPreviewCache::supportsService(OUString const & ServiceName) {
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> PresenterPreviewCache::getSupportedServiceNames() {
    return {u"com.sun.star.drawing.PresenterPreviewCache"_ustr};
}

//----- XSlidePreviewCache ----------------------------------------------------

void SAL_CALL PresenterPreviewCache::setDocumentSlides (
    const Reference<container::XIndexAccess>& rxSlides,
    const Reference<XInterface>& rxDocument)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext != nullptr);

    mpCacheContext->SetDocumentSlides(rxSlides, rxDocument);
}

void SAL_CALL PresenterPreviewCache::setVisibleRange (
    sal_Int32 nFirstVisibleSlideIndex,
    sal_Int32 nLastVisibleSlideIndex)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext != nullptr);

    mpCacheContext->SetVisibleSlideRange (nFirstVisibleSlideIndex, nLastVisibleSlideIndex);
}

void SAL_CALL PresenterPreviewCache::setPreviewSize (
    const css::geometry::IntegerSize2D& rSize)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache != nullptr);

    maPreviewSize = Size(rSize.Width, rSize.Height);
    mpCache->ChangeSize(maPreviewSize, Bitmap::HasFastScale());
}

Reference<rendering::XBitmap> SAL_CALL PresenterPreviewCache::getSlidePreview (
    sal_Int32 nSlideIndex,
    const Reference<rendering::XCanvas>& rxCanvas)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext != nullptr);

    cppcanvas::CanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::createCanvas(rxCanvas));

    const SdrPage* pPage = mpCacheContext->GetPage(nSlideIndex);
    if (pPage == nullptr)
        throw RuntimeException();

    const BitmapEx aPreview (mpCache->GetPreviewBitmap(pPage,true));
    if (aPreview.IsEmpty())
        return nullptr;
    else
        return cppcanvas::VCLFactory::createBitmap(
            pCanvas,
            aPreview)->getUNOBitmap();
}

void SAL_CALL PresenterPreviewCache::addPreviewCreationNotifyListener (
    const Reference<drawing::XSlidePreviewCacheListener>& rxListener)
{
    if (m_bDisposed)
        return;
    if (rxListener.is())
        mpCacheContext->AddPreviewCreationNotifyListener(rxListener);
}

void SAL_CALL PresenterPreviewCache::removePreviewCreationNotifyListener (
    const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener)
{
    ThrowIfDisposed();
    mpCacheContext->RemovePreviewCreationNotifyListener(rxListener);
}

void SAL_CALL PresenterPreviewCache::pause()
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache != nullptr);
    mpCache->Pause();
}

void SAL_CALL PresenterPreviewCache::resume()
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache != nullptr);
    mpCache->Resume();
}

void PresenterPreviewCache::ThrowIfDisposed()
{
    if (m_bDisposed)
    {
        throw lang::DisposedException (u"PresenterPreviewCache object has already been disposed"_ustr,
            static_cast<uno::XWeak*>(this));
    }
}

//===== PresenterPreviewCache::PresenterCacheContext ==========================

PresenterPreviewCache::PresenterCacheContext::PresenterCacheContext()
    : mnFirstVisibleSlideIndex(-1),
      mnLastVisibleSlideIndex(-1)
{
}

void PresenterPreviewCache::PresenterCacheContext::SetDocumentSlides (
    const Reference<container::XIndexAccess>& rxSlides,
    const Reference<XInterface>& rxDocument)
{
    mxSlides = rxSlides;
    mxDocument = rxDocument;
    mnFirstVisibleSlideIndex = -1;
    mnLastVisibleSlideIndex = -1;
}

void PresenterPreviewCache::PresenterCacheContext::SetVisibleSlideRange (
    const sal_Int32 nFirstVisibleSlideIndex,
    const sal_Int32 nLastVisibleSlideIndex)
{
    if (nFirstVisibleSlideIndex > nLastVisibleSlideIndex || nFirstVisibleSlideIndex<0)
    {
        mnFirstVisibleSlideIndex = -1;
        mnLastVisibleSlideIndex = -1;
    }
    else
    {
        mnFirstVisibleSlideIndex = nFirstVisibleSlideIndex;
        mnLastVisibleSlideIndex = nLastVisibleSlideIndex;
    }
    if (mxSlides.is() && mnLastVisibleSlideIndex >= mxSlides->getCount())
        mnLastVisibleSlideIndex = mxSlides->getCount() - 1;
}

void PresenterPreviewCache::PresenterCacheContext::AddPreviewCreationNotifyListener (
    const Reference<drawing::XSlidePreviewCacheListener>& rxListener)
{
    maListeners.push_back(rxListener);
}

void PresenterPreviewCache::PresenterCacheContext::RemovePreviewCreationNotifyListener (
    const Reference<drawing::XSlidePreviewCacheListener>& rxListener)
{
    auto iListener = std::find(maListeners.begin(), maListeners.end(), rxListener);
    if (iListener != maListeners.end())
        maListeners.erase(iListener);
}

//----- CacheContext ----------------------------------------------------------

void PresenterPreviewCache::PresenterCacheContext::NotifyPreviewCreation (
    CacheKey aKey)
{
    if ( ! mxSlides.is())
        return;
    const sal_Int32 nCount(mxSlides->getCount());
    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        if (aKey == GetPage(nIndex))
            CallListeners(nIndex);
}

bool PresenterPreviewCache::PresenterCacheContext::IsIdle()
{
    return true;
}

bool PresenterPreviewCache::PresenterCacheContext::IsVisible (CacheKey aKey)
{
    if (mnFirstVisibleSlideIndex < 0)
        return false;
    for (sal_Int32 nIndex=mnFirstVisibleSlideIndex; nIndex<=mnLastVisibleSlideIndex; ++nIndex)
    {
        const SdrPage* pPage = GetPage(nIndex);
        if (pPage == aKey)
            return true;
    }
    return false;
}

const SdrPage* PresenterPreviewCache::PresenterCacheContext::GetPage (CacheKey aKey)
{
    return aKey;
}

std::shared_ptr<std::vector<CacheKey> >
    PresenterPreviewCache::PresenterCacheContext::GetEntryList (bool bVisible)
{
    auto pKeys = std::make_shared<std::vector<CacheKey>>();

    if ( ! mxSlides.is())
        return pKeys;

    const sal_Int32 nFirstIndex (bVisible ? mnFirstVisibleSlideIndex : 0);
    const sal_Int32 nLastIndex (bVisible ? mnLastVisibleSlideIndex : mxSlides->getCount()-1);

    if (nFirstIndex < 0)
        return pKeys;

    for (sal_Int32 nIndex=nFirstIndex; nIndex<=nLastIndex; ++nIndex)
    {
        pKeys->push_back(GetPage(nIndex));
    }

    return pKeys;
}

sal_Int32 PresenterPreviewCache::PresenterCacheContext::GetPriority (CacheKey aKey)
{
    if ( ! mxSlides.is())
        return 0;

    const sal_Int32 nCount (mxSlides->getCount());

    for (sal_Int32 nIndex=mnFirstVisibleSlideIndex; nIndex<=mnLastVisibleSlideIndex; ++nIndex)
        if (aKey == GetPage(nIndex))
            return -nCount-1+nIndex;

    for (sal_Int32 nIndex=0; nIndex<=nCount; ++nIndex)
        if (aKey == GetPage(nIndex))
            return nIndex;

    return 0;
}

Reference<XInterface> PresenterPreviewCache::PresenterCacheContext::GetModel()
{
    return mxDocument;
}

const SdrPage* PresenterPreviewCache::PresenterCacheContext::GetPage (
    const sal_Int32 nSlideIndex) const
{
    if ( ! mxSlides.is())
        return nullptr;
    if (nSlideIndex < 0 || nSlideIndex >= mxSlides->getCount())
        return nullptr;

    Reference<drawing::XDrawPage> xSlide (mxSlides->getByIndex(nSlideIndex), UNO_QUERY);
    const SdPage* pPage = SdPage::getImplementation(xSlide);
    return pPage;
}

void PresenterPreviewCache::PresenterCacheContext::CallListeners (
    const sal_Int32 nIndex)
{
    ListenerContainer aListeners (maListeners);
    for (const auto& rxListener : aListeners)
    {
        try
        {
            rxListener->notifyPreviewCreation(nIndex);
        }
        catch (lang::DisposedException&)
        {
            RemovePreviewCreationNotifyListener(rxListener);
        }
    }
}

} // end of namespace ::sd::presenter


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Draw_PresenterPreviewCache_get_implementation(css::uno::XComponentContext*,
                                                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::presenter::PresenterPreviewCache);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
