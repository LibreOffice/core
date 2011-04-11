/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "precompiled_sd.hxx"

#include "PresenterPreviewCache.hxx"

#include "cache/SlsCacheContext.hxx"
#include "tools/IdleDetection.hxx"
#include "sdpage.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::slidesorter::cache;
using ::rtl::OUString;

namespace sd { namespace presenter {


class PresenterPreviewCache::PresenterCacheContext : public CacheContext
{
public:
    PresenterCacheContext (void);
    virtual ~PresenterCacheContext (void);

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
        const Bitmap& rPreview);
    virtual bool IsIdle (void);
    virtual bool IsVisible (CacheKey aKey);
    virtual const SdrPage* GetPage (CacheKey aKey);
    virtual ::boost::shared_ptr<std::vector<CacheKey> > GetEntryList (bool bVisible);
    virtual sal_Int32 GetPriority (CacheKey aKey);
    virtual ::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> GetModel (void);

private:
    Reference<container::XIndexAccess> mxSlides;
    Reference<XInterface> mxDocument;
    sal_Int32 mnFirstVisibleSlideIndex;
    sal_Int32 mnLastVisibleSlideIndex;
    typedef ::std::vector<css::uno::Reference<css::drawing::XSlidePreviewCacheListener> > ListenerContainer;
    ListenerContainer maListeners;

    void CallListeners (const sal_Int32 nSlideIndex);
};




//===== Service ===============================================================

Reference<XInterface> SAL_CALL PresenterPreviewCache_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterPreviewCache(rxContext)));
}




::rtl::OUString PresenterPreviewCache_getImplementationName (void) throw(RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.PresenterPreviewCache"));
}




Sequence<rtl::OUString> SAL_CALL PresenterPreviewCache_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PresenterPreviewCache")));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== PresenterPreviewCache =================================================

PresenterPreviewCache::PresenterPreviewCache (const Reference<XComponentContext>& rxContext)
    : PresenterPreviewCacheInterfaceBase(m_aMutex),
      maPreviewSize(Size(200,200)),
      mpCacheContext(new PresenterCacheContext()),
      mpCache(new PageCache(maPreviewSize, false, mpCacheContext))
{
    (void)rxContext;
}




PresenterPreviewCache::~PresenterPreviewCache (void)
{
}




//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterPreviewCache::initialize (const Sequence<Any>& rArguments)
    throw(Exception, RuntimeException)
{
    if (rArguments.getLength() != 0)
        throw RuntimeException();
}




//----- XSlidePreviewCache ----------------------------------------------------

void SAL_CALL PresenterPreviewCache::setDocumentSlides (
    const Reference<container::XIndexAccess>& rxSlides,
    const Reference<XInterface>& rxDocument)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    mpCacheContext->SetDocumentSlides(rxSlides, rxDocument);
}




void SAL_CALL PresenterPreviewCache::setVisibleRange (
    sal_Int32 nFirstVisibleSlideIndex,
    sal_Int32 nLastVisibleSlideIndex)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    mpCacheContext->SetVisibleSlideRange (nFirstVisibleSlideIndex, nLastVisibleSlideIndex);
}




void SAL_CALL PresenterPreviewCache::setPreviewSize (
    const css::geometry::IntegerSize2D& rSize)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache.get()!=NULL);

    maPreviewSize = Size(rSize.Width, rSize.Height);
    mpCache->ChangeSize(maPreviewSize, false);
}




Reference<rendering::XBitmap> SAL_CALL PresenterPreviewCache::getSlidePreview (
    sal_Int32 nSlideIndex,
    const Reference<rendering::XCanvas>& rxCanvas)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCacheContext.get()!=NULL);

    cppcanvas::BitmapCanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::getInstance().createCanvas(
            Reference<rendering::XBitmapCanvas>(rxCanvas, UNO_QUERY)));

    const SdrPage* pPage = mpCacheContext->GetPage(nSlideIndex);
    if (pPage == NULL)
        throw RuntimeException();

    const BitmapEx aPreview (mpCache->GetPreviewBitmap(pPage,true));
    if (aPreview.IsEmpty())
        return NULL;
    else
        return cppcanvas::VCLFactory::getInstance().createBitmap(
            pCanvas,
            aPreview)->getUNOBitmap();
}




void SAL_CALL PresenterPreviewCache::addPreviewCreationNotifyListener (
    const Reference<drawing::XSlidePreviewCacheListener>& rxListener)
    throw (css::uno::RuntimeException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        return;
    if (rxListener.is())
        mpCacheContext->AddPreviewCreationNotifyListener(rxListener);
}




void SAL_CALL PresenterPreviewCache::removePreviewCreationNotifyListener (
    const css::uno::Reference<css::drawing::XSlidePreviewCacheListener>& rxListener)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    mpCacheContext->RemovePreviewCreationNotifyListener(rxListener);
}




void SAL_CALL PresenterPreviewCache::pause (void)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache.get()!=NULL);
    mpCache->Pause();
}




void SAL_CALL PresenterPreviewCache::resume (void)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    OSL_ASSERT(mpCache.get()!=NULL);
    mpCache->Resume();
}




//-----------------------------------------------------------------------------

void PresenterPreviewCache::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterPreviewCache object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}




//===== PresenterPreviewCache::PresenterCacheContext ==========================


PresenterPreviewCache::PresenterCacheContext::PresenterCacheContext (void)
    : mxSlides(),
      mxDocument(),
      mnFirstVisibleSlideIndex(-1),
      mnLastVisibleSlideIndex(-1),
      maListeners()
{
}




PresenterPreviewCache::PresenterCacheContext::~PresenterCacheContext (void)
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




bool PresenterPreviewCache::PresenterCacheContext::IsIdle (void)
{
    return true;
    /*
    sal_Int32 nIdleState (tools::IdleDetection::GetIdleState(NULL));
    if (nIdleState == tools::IdleDetection::IDET_IDLE)
        return true;
    else
        return false;
    */
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




::boost::shared_ptr<std::vector<CacheKey> >
    PresenterPreviewCache::PresenterCacheContext::GetEntryList (bool bVisible)
{
    ::boost::shared_ptr<std::vector<CacheKey> > pKeys (new std::vector<CacheKey>());

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




Reference<XInterface> PresenterPreviewCache::PresenterCacheContext::GetModel (void)
{
    return mxDocument;
}




//-----------------------------------------------------------------------------

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
