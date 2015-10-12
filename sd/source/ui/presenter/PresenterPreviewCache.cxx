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
#include "facreg.hxx"

#include "cache/SlsCacheContext.hxx"
#include "tools/IdleDetection.hxx"
#include "sdpage.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::cache;

namespace sd { namespace presenter {

class PresenterPreviewCache::PresenterCacheContext : public CacheContext
{
public:
    PresenterCacheContext();
    virtual ~PresenterCacheContext();

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
    virtual void NotifyPreviewCreation (
        CacheKey aKey,
        const Bitmap& rPreview) override;
    virtual bool IsIdle() override;
    virtual bool IsVisible (CacheKey aKey) override;
    virtual const SdrPage* GetPage (CacheKey aKey) override;
    virtual std::shared_ptr<std::vector<CacheKey> > GetEntryList (bool bVisible) override;
    virtual sal_Int32 GetPriority (CacheKey aKey) override;
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel() override;

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

PresenterPreviewCache::PresenterPreviewCache (const Reference<XComponentContext>& rxContext)
    : PresenterPreviewCacheInterfaceBase(m_aMutex),
      maPreviewSize(Size(200,200)),
      mpCacheContext(new PresenterCacheContext()),
      mpCache(new PageCache(maPreviewSize, Bitmap::HasFastScale(), mpCacheContext))
{
    (void)rxContext;
}

PresenterPreviewCache::~PresenterPreviewCache()
{
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterPreviewCache::initialize (const Sequence<Any>& rArguments)
    throw(Exception, RuntimeException, std::exception)
{
    if (rArguments.getLength() != 0)
        throw RuntimeException();
}

//----- XSlidePreviewCache ----------------------------------------------------

void SAL_CALL PresenterPreviewCache::setDocumentSlides (
    const Reference<container::XIndexAccess>& rxSlides,
    const Reference<XInterface>& rxDocument)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    mpCacheContext->SetDocumentSlides(rxSlides, rxDocument);
}

void SAL_CALL PresenterPreviewCache::setVisibleRange (
    sal_Int32 nFirstVisibleSlideIndex,
    sal_Int32 nLastVisibleSlideIndex)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    mpCacheContext->SetVisibleSlideRange (nFirstVisibleSlideIndex, nLastVisibleSlideIndex);
}

void SAL_CALL PresenterPreviewCache::setPreviewSize (
    const css::geometry::IntegerSize2D& rSize)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache.get()!=NULL);

    maPreviewSize = Size(rSize.Width, rSize.Height);
    mpCache->ChangeSize(maPreviewSize, Bitmap::HasFastScale());
}

Reference<rendering::XBitmap> SAL_CALL PresenterPreviewCache::getSlidePreview (
    sal_Int32 nSlideIndex,
    const Reference<rendering::XCanvas>& rxCanvas)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    cppcanvas::CanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::createCanvas(rxCanvas));

    const SdrPage* pPage = mpCacheContext->GetPage(nSlideIndex);
    if (pPage == NULL)
        throw RuntimeException();

    const BitmapEx aPreview (mpCache->GetPreviewBitmap(pPage,true));
    if (aPreview.IsEmpty())
        return NULL;
    else
        return cppcanvas::VCLFactory::createBitmap(
            pCanvas,
            aPreview)->getUNOBitmap();
}

void SAL_CALL PresenterPreviewCache::addPreviewCreationNotifyListener (
    const Reference<drawing::XSlidePreviewCacheListener>& rxListener)
    throw (css::uno::RuntimeException, std::exception)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;
    if (rxListener.is())
        mpCacheContext->AddPreviewCreationNotifyListener(rxListener);
}

void SAL_CALL PresenterPreviewCache::removePreviewCreationNotifyListener (
    const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener)
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    mpCacheContext->RemovePreviewCreationNotifyListener(rxListener);
}

void SAL_CALL PresenterPreviewCache::pause()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache.get()!=NULL);
    mpCache->Pause();
}

void SAL_CALL PresenterPreviewCache::resume()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache.get()!=NULL);
    mpCache->Resume();
}

void PresenterPreviewCache::ThrowIfDisposed()
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException ("PresenterPreviewCache object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

//===== PresenterPreviewCache::PresenterCacheContext ==========================

PresenterPreviewCache::PresenterCacheContext::PresenterCacheContext()
    : mxSlides(),
      mxDocument(),
      mnFirstVisibleSlideIndex(-1),
      mnLastVisibleSlideIndex(-1),
      maListeners()
{
}

PresenterPreviewCache::PresenterCacheContext::~PresenterCacheContext()
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
    ListenerContainer::iterator iListener;
    for (iListener=maListeners.begin(); iListener!=maListeners.end(); ++iListener)
        if (*iListener == rxListener)
        {
            maListeners.erase(iListener);
            return;
        }
}

//----- CacheContext ----------------------------------------------------------

void PresenterPreviewCache::PresenterCacheContext::NotifyPreviewCreation (
    CacheKey aKey,
    const Bitmap& rPreview)
{
    (void)rPreview;

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
        if (pPage == static_cast<const SdrPage*>(aKey))
            return true;
    }
    return false;
}

const SdrPage* PresenterPreviewCache::PresenterCacheContext::GetPage (CacheKey aKey)
{
    return static_cast<const SdrPage*>(aKey);
}

std::shared_ptr<std::vector<CacheKey> >
    PresenterPreviewCache::PresenterCacheContext::GetEntryList (bool bVisible)
{
    std::shared_ptr<std::vector<CacheKey> > pKeys (new std::vector<CacheKey>());

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
        return NULL;
    if (nSlideIndex < 0 || nSlideIndex >= mxSlides->getCount())
        return NULL;

    Reference<drawing::XDrawPage> xSlide (mxSlides->getByIndex(nSlideIndex), UNO_QUERY);
    const SdPage* pPage = SdPage::getImplementation(xSlide);
    return dynamic_cast<const SdrPage*>(pPage);
}

void PresenterPreviewCache::PresenterCacheContext::CallListeners (
    const sal_Int32 nIndex)
{
    ListenerContainer aListeners (maListeners);
    ListenerContainer::const_iterator iListener;
    for (iListener=aListeners.begin(); iListener!=aListeners.end(); ++iListener)
    {
        try
        {
            (*iListener)->notifyPreviewCreation(nIndex);
        }
        catch (lang::DisposedException&)
        {
            RemovePreviewCreationNotifyListener(*iListener);
        }
    }
}

} } // end of namespace ::sd::presenter


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_PresenterPreviewCache_get_implementation(::com::sun::star::uno::XComponentContext* context,
                                                                ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::presenter::PresenterPreviewCache(context));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
