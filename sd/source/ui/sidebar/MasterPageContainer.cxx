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

#include "MasterPageContainer.hxx"

#include "MasterPageContainerProviders.hxx"
#include "MasterPageDescriptor.hxx"
#include "MasterPageContainerFiller.hxx"
#include "MasterPageContainerQueue.hxx"
#include <TemplateScanner.hxx>
#include <PreviewRenderer.hxx>
#include <tools/AsynchronousTask.hxx>
#include <tools/SdGlobalResourceContainer.hxx>
#include <strings.hrc>
#include <algorithm>
#include <list>
#include <memory>
#include <set>

#include <unomodel.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <svx/svdpage.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <sdresid.hxx>
#include <tools/TimerBasedTaskExecution.hxx>
#include <pres.hxx>
#include <osl/mutex.hxx>
#include <osl/getglobalmutex.hxx>
#include <xmloff/autolayout.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

typedef ::std::vector<sd::sidebar::SharedMasterPageDescriptor> MasterPageContainerType;

} // end of anonymous namespace

namespace sd { namespace sidebar {

/** Inner implementation class of the MasterPageContainer.
*/
class MasterPageContainer::Implementation
    : public SdGlobalResource,
      public MasterPageContainerFiller::ContainerAdapter,
      public MasterPageContainerQueue::ContainerAdapter
{
public:
    mutable ::osl::Mutex maMutex;

    static std::weak_ptr<Implementation> mpInstance;
    MasterPageContainerType maContainer;

    static std::shared_ptr<Implementation> Instance();

    void LateInit();
    void AddChangeListener (const Link<MasterPageContainerChangeEvent&,void>& rLink);
    void RemoveChangeListener (const Link<MasterPageContainerChangeEvent&,void>& rLink);
    void UpdatePreviewSizePixel();
    const Size& GetPreviewSizePixel (PreviewSize eSize) const;

    bool HasToken (Token aToken) const;
    const SharedMasterPageDescriptor GetDescriptor (MasterPageContainer::Token aToken) const;
    SharedMasterPageDescriptor GetDescriptor (MasterPageContainer::Token aToken);
    virtual Token PutMasterPage (const SharedMasterPageDescriptor& rDescriptor) override;
    void InvalidatePreview (Token aToken);
    Image GetPreviewForToken (
        Token aToken,
        PreviewSize ePreviewSize);
    PreviewState GetPreviewState (Token aToken) const;
    bool RequestPreview (Token aToken);

    Reference<frame::XModel> GetModel();
    SdDrawDocument* GetDocument();

    void FireContainerChange (
        MasterPageContainerChangeEvent::EventType eType,
        Token aToken);

    virtual bool UpdateDescriptor (
        const SharedMasterPageDescriptor& rpDescriptor,
        bool bForcePageObject,
        bool bForcePreview,
        bool bSendEvents) override;

    void ReleaseDescriptor (Token aToken);

    /** Called by the MasterPageContainerFiller to notify that all master
        pages from template documents have been added.
    */
    virtual void FillingDone() override;

private:
    Implementation();
    virtual ~Implementation() override;

    class Deleter { public:
        void operator() (Implementation* pObject) { delete pObject; }
    };
    friend class Deleter;

    enum InitializationState { NOT_INITIALIZED, INITIALIZING, INITIALIZED } meInitializationState;

    std::unique_ptr<MasterPageContainerQueue> mpRequestQueue;
    css::uno::Reference<css::frame::XModel> mxModel;
    SdDrawDocument* mpDocument;
    PreviewRenderer maPreviewRenderer;
    /** Remember whether the first page object has already been used to
        determine the correct size ratio.
    */
    bool mbFirstPageObjectSeen;

    // The widths for the previews contain two pixels for the border that is
    // painted around the preview.
    static const int SMALL_PREVIEW_WIDTH = 72 + 2;
    static const int LARGE_PREVIEW_WIDTH = 2*72 + 2;

    /** This substition of page preview shows "Preparing preview" and is
        shown as long as the actual previews are not being present.
    */
    Image maLargePreviewBeingCreated;
    Image maSmallPreviewBeingCreated;

    /** This substition of page preview is shown when a preview can not be
        created and thus is not available.
    */
    Image maLargePreviewNotAvailable;
    Image maSmallPreviewNotAvailable;

    ::std::vector<Link<MasterPageContainerChangeEvent&,void>> maChangeListeners;

    // We have to remember the tasks for initialization and filling in case
    // a MasterPageContainer object is destroyed before these tasks have
    // been completed.
    std::weak_ptr<sd::tools::TimerBasedTaskExecution> mpFillerTask;

    Size maSmallPreviewSizePixel;
    Size maLargePreviewSizePixel;

    Image GetPreviewSubstitution(const char* pId, PreviewSize ePreviewSize);

    void CleanContainer();
};

//===== MasterPageContainer ===================================================

std::weak_ptr<MasterPageContainer::Implementation>
    MasterPageContainer::Implementation::mpInstance;

std::shared_ptr<MasterPageContainer::Implementation>
    MasterPageContainer::Implementation::Instance()
{
    std::shared_ptr<MasterPageContainer::Implementation> pInstance;

    if (Implementation::mpInstance.expired())
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (Implementation::mpInstance.expired())
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            pInstance = std::shared_ptr<MasterPageContainer::Implementation>(
                new MasterPageContainer::Implementation(),
                MasterPageContainer::Implementation::Deleter());
            SdGlobalResourceContainer::Instance().AddResource(pInstance);
            Implementation::mpInstance = pInstance;
        }
        else
            pInstance = std::shared_ptr<MasterPageContainer::Implementation>(
                Implementation::mpInstance);
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        pInstance = std::shared_ptr<MasterPageContainer::Implementation>(
            Implementation::mpInstance);
    }

    DBG_ASSERT(pInstance != nullptr,
               "MasterPageContainer::Implementation::Instance(): instance is nullptr");
    return pInstance;
}

MasterPageContainer::MasterPageContainer()
    : mpImpl(Implementation::Instance()),
      mePreviewSize(SMALL)
{
    mpImpl->LateInit();
}

MasterPageContainer::~MasterPageContainer()
{
}

void MasterPageContainer::AddChangeListener (const Link<MasterPageContainerChangeEvent&,void>& rLink)
{
    mpImpl->AddChangeListener(rLink);
}

void MasterPageContainer::RemoveChangeListener (const Link<MasterPageContainerChangeEvent&,void>& rLink)
{
    mpImpl->RemoveChangeListener(rLink);
}

void MasterPageContainer::SetPreviewSize (PreviewSize eSize)
{
    mePreviewSize = eSize;
    mpImpl->FireContainerChange(
        MasterPageContainerChangeEvent::EventType::SIZE_CHANGED,
        NIL_TOKEN);
}

Size const & MasterPageContainer::GetPreviewSizePixel() const
{
    return mpImpl->GetPreviewSizePixel(mePreviewSize);
}

MasterPageContainer::Token MasterPageContainer::PutMasterPage (
    const std::shared_ptr<MasterPageDescriptor>& rDescriptor)
{
    return mpImpl->PutMasterPage(rDescriptor);
}

void MasterPageContainer::AcquireToken (Token aToken)
{
    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
    {
        ++pDescriptor->mnUseCount;
    }
}

void MasterPageContainer::ReleaseToken (Token aToken)
{
    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() == nullptr)
        return;

    OSL_ASSERT(pDescriptor->mnUseCount>0);
    --pDescriptor->mnUseCount;
    if (pDescriptor->mnUseCount > 0)
        return;

    switch (pDescriptor->meOrigin)
    {
        case DEFAULT:
        case TEMPLATE:
        default:
            break;

        case MASTERPAGE:
            mpImpl->ReleaseDescriptor(aToken);
            break;
    }
}

int MasterPageContainer::GetTokenCount() const
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    return mpImpl->maContainer.size();
}

bool MasterPageContainer::HasToken (Token aToken) const
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    return mpImpl->HasToken(aToken);
}

MasterPageContainer::Token MasterPageContainer::GetTokenForIndex (int nIndex)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Token aResult (NIL_TOKEN);
    if (HasToken(nIndex))
        aResult = mpImpl->maContainer[nIndex]->maToken;
    return aResult;
}

MasterPageContainer::Token MasterPageContainer::GetTokenForURL (
    const OUString& sURL)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Token aResult (NIL_TOKEN);
    if (!sURL.isEmpty())
    {
        MasterPageContainerType::iterator iEntry (
            ::std::find_if (
                mpImpl->maContainer.begin(),
                mpImpl->maContainer.end(),
                MasterPageDescriptor::URLComparator(sURL)));
        if (iEntry != mpImpl->maContainer.end())
            aResult = (*iEntry)->maToken;
    }
    return aResult;
}

MasterPageContainer::Token MasterPageContainer::GetTokenForStyleName (const OUString& sStyleName)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Token aResult (NIL_TOKEN);
    if (!sStyleName.isEmpty())
    {
        MasterPageContainerType::iterator iEntry (
            ::std::find_if (
                mpImpl->maContainer.begin(),
                mpImpl->maContainer.end(),
                MasterPageDescriptor::StyleNameComparator(sStyleName)));
        if (iEntry != mpImpl->maContainer.end())
            aResult = (*iEntry)->maToken;
    }
    return aResult;
}

MasterPageContainer::Token MasterPageContainer::GetTokenForPageObject (
    const SdPage* pPage)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    Token aResult (NIL_TOKEN);
    if (pPage != nullptr)
    {
        MasterPageContainerType::iterator iEntry (
            ::std::find_if (
                mpImpl->maContainer.begin(),
                mpImpl->maContainer.end(),
                MasterPageDescriptor::PageObjectComparator(pPage)));
        if (iEntry != mpImpl->maContainer.end())
            aResult = (*iEntry)->maToken;
    }
    return aResult;
}

OUString MasterPageContainer::GetURLForToken (
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
        return pDescriptor->msURL;
    else
        return OUString();
}

OUString MasterPageContainer::GetPageNameForToken (
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
        return pDescriptor->msPageName;
    else
        return OUString();
}

OUString MasterPageContainer::GetStyleNameForToken (
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
        return pDescriptor->msStyleName;
    else
        return OUString();
}

SdPage* MasterPageContainer::GetPageObjectForToken (
    MasterPageContainer::Token aToken,
    bool bLoad)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SdPage* pPageObject = nullptr;
    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
    {
        pPageObject = pDescriptor->mpMasterPage;
        if (pPageObject == nullptr)
        {
            // The page object is not (yet) present.  Call
            // UpdateDescriptor() to trigger the PageObjectProvider() to
            // provide it.
            if (bLoad)
                mpImpl->GetModel();
            if (mpImpl->UpdateDescriptor(pDescriptor,bLoad,false, true))
                pPageObject = pDescriptor->mpMasterPage;
        }
    }
    return pPageObject;
}

MasterPageContainer::Origin MasterPageContainer::GetOriginForToken (Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
        return pDescriptor->meOrigin;
    else
        return UNKNOWN;
}

sal_Int32 MasterPageContainer::GetTemplateIndexForToken (Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
        return pDescriptor->mnTemplateIndex;
    else
        return -1;
}

std::shared_ptr<MasterPageDescriptor> MasterPageContainer::GetDescriptorForToken (
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    return mpImpl->GetDescriptor(aToken);
}

void MasterPageContainer::InvalidatePreview (MasterPageContainer::Token aToken)
{
    mpImpl->InvalidatePreview(aToken);
}

Image MasterPageContainer::GetPreviewForToken (MasterPageContainer::Token aToken)
{
    return mpImpl->GetPreviewForToken(aToken,mePreviewSize);
}

MasterPageContainer::PreviewState MasterPageContainer::GetPreviewState (Token aToken)
{
    return mpImpl->GetPreviewState(aToken);
}

bool MasterPageContainer::RequestPreview (Token aToken)
{
    return mpImpl->RequestPreview(aToken);
}

//==== Implementation ================================================

MasterPageContainer::Implementation::Implementation()
    : maMutex(),
      maContainer(),
      meInitializationState(NOT_INITIALIZED),
      mpDocument(nullptr),
      maPreviewRenderer(),
      mbFirstPageObjectSeen(false),
      maLargePreviewBeingCreated(),
      maSmallPreviewBeingCreated(),
      maLargePreviewNotAvailable(),
      maSmallPreviewNotAvailable(),
      maChangeListeners(),
      maSmallPreviewSizePixel(),
      maLargePreviewSizePixel()
{
    UpdatePreviewSizePixel();
}

MasterPageContainer::Implementation::~Implementation()
{
    // When the initializer or filler tasks are still running then we have
    // to stop them now in order to prevent them from calling us back.
    tools::TimerBasedTaskExecution::ReleaseTask(mpFillerTask);

    mpRequestQueue.reset();

    uno::Reference<util::XCloseable> xCloseable (mxModel, uno::UNO_QUERY);
    if (xCloseable.is())
    {
        try
        {
            xCloseable->close(true);
        }
        catch (const css::util::CloseVetoException&)
        {
        }
    }
    mxModel = nullptr;
}

void MasterPageContainer::Implementation::LateInit()
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (meInitializationState != NOT_INITIALIZED)
        return;

    meInitializationState = INITIALIZING;

    OSL_ASSERT(Instance().get()==this);
    mpRequestQueue.reset(MasterPageContainerQueue::Create(
        std::shared_ptr<MasterPageContainerQueue::ContainerAdapter>(Instance())));

    mpFillerTask = ::sd::tools::TimerBasedTaskExecution::Create(
        std::shared_ptr<tools::AsynchronousTask>(new MasterPageContainerFiller(*this)),
        5,
        50);

    meInitializationState = INITIALIZED;
}

void MasterPageContainer::Implementation::AddChangeListener (const Link<MasterPageContainerChangeEvent&,void>& rLink)
{
    const ::osl::MutexGuard aGuard (maMutex);

    ::std::vector<Link<MasterPageContainerChangeEvent&,void>>::iterator iListener (
        ::std::find(maChangeListeners.begin(),maChangeListeners.end(),rLink));
    if (iListener == maChangeListeners.end())
        maChangeListeners.push_back(rLink);

}

void MasterPageContainer::Implementation::RemoveChangeListener (const Link<MasterPageContainerChangeEvent&,void>& rLink)
{
    const ::osl::MutexGuard aGuard (maMutex);

    ::std::vector<Link<MasterPageContainerChangeEvent&,void>>::iterator iListener (
        ::std::find(maChangeListeners.begin(),maChangeListeners.end(),rLink));
    if (iListener != maChangeListeners.end())
        maChangeListeners.erase(iListener);
}

void MasterPageContainer::Implementation::UpdatePreviewSizePixel()
{
    const ::osl::MutexGuard aGuard (maMutex);

    // The default aspect ratio is 4:3
    int nWidth (4);
    int nHeight (3);

    // Search for the first entry with an existing master page.
    auto iDescriptor = std::find_if(maContainer.begin(), maContainer.end(),
        [](const SharedMasterPageDescriptor& rxDescriptor) {
            return rxDescriptor != nullptr && rxDescriptor->mpMasterPage != nullptr;
        });
    if (iDescriptor != maContainer.end())
    {
        Size aPageSize ((*iDescriptor)->mpMasterPage->GetSize());
        OSL_ASSERT(aPageSize.Width() > 0 && aPageSize.Height() > 0);
        if (aPageSize.Width() > 0)
            nWidth = aPageSize.Width();
        if (aPageSize.Height() > 0)
            nHeight = aPageSize.Height();
        mbFirstPageObjectSeen = true;
    }

    maSmallPreviewSizePixel.setWidth( SMALL_PREVIEW_WIDTH );
    maLargePreviewSizePixel.setWidth( LARGE_PREVIEW_WIDTH );

    int nNewSmallHeight ((maSmallPreviewSizePixel.Width()-2) * nHeight / nWidth + 2);
    int nNewLargeHeight ((maLargePreviewSizePixel.Width()-2) * nHeight / nWidth + 2);

    if (nNewSmallHeight!=maSmallPreviewSizePixel.Height()
        || nNewLargeHeight!=maLargePreviewSizePixel.Height())
    {
        maSmallPreviewSizePixel.setHeight( nNewSmallHeight );
        maLargePreviewSizePixel.setHeight( nNewLargeHeight );
        FireContainerChange(
            MasterPageContainerChangeEvent::EventType::SIZE_CHANGED,
            NIL_TOKEN);
    }
}

const Size& MasterPageContainer::Implementation::GetPreviewSizePixel (PreviewSize eSize) const
{
    if (eSize == SMALL)
        return maSmallPreviewSizePixel;
    else
        return maLargePreviewSizePixel;
}

MasterPageContainer::Token MasterPageContainer::Implementation::PutMasterPage (
    const SharedMasterPageDescriptor& rpDescriptor)
{
    const ::osl::MutexGuard aGuard (maMutex);

    Token aResult (NIL_TOKEN);

    // Get page object and preview when that is inexpensive.
    UpdateDescriptor(rpDescriptor,false,false, false);

    // Look up the new MasterPageDescriptor and either insert it or update
    // an already existing one.
    MasterPageContainerType::iterator aEntry (
        ::std::find_if (
            maContainer.begin(),
            maContainer.end(),
            MasterPageDescriptor::AllComparator(rpDescriptor)));
    if (aEntry == maContainer.end())
    {
        // Insert a new MasterPageDescriptor.
        bool bIgnore(rpDescriptor->mpPageObjectProvider == nullptr
                     && rpDescriptor->msURL.isEmpty());

        if ( ! bIgnore)
        {
            CleanContainer();

            aResult = maContainer.size();
            rpDescriptor->SetToken(aResult);

            // Templates are precious, i.e. we lock them so that they will
            // not be destroyed when (temporarily) no one references them.
            // They will only be deleted when the container is destroyed.
            switch (rpDescriptor->meOrigin)
            {
                case TEMPLATE:
                case DEFAULT:
                    ++rpDescriptor->mnUseCount;
                    break;

                default:
                    break;
            }

            maContainer.push_back(rpDescriptor);
            aEntry = maContainer.end()-1;

            FireContainerChange(MasterPageContainerChangeEvent::EventType::CHILD_ADDED,aResult);
        }
    }
    else
    {
        // Update an existing MasterPageDescriptor.
        aResult = (*aEntry)->maToken;
        std::unique_ptr<std::vector<MasterPageContainerChangeEvent::EventType> > pEventTypes(
            (*aEntry)->Update(*rpDescriptor));
        if (pEventTypes != nullptr && !pEventTypes->empty())
        {
            // One or more aspects of the descriptor have changed.  Send
            // appropriate events to the listeners.
            UpdateDescriptor(*aEntry,false,false, true);

            for (auto& rEventType : *pEventTypes)
            {
                FireContainerChange(rEventType, (*aEntry)->maToken);
            }
        }
    }

    return aResult;
}

bool MasterPageContainer::Implementation::HasToken (Token aToken) const
{
    return aToken>=0
        && static_cast<unsigned>(aToken)<maContainer.size()
        && maContainer[aToken].get()!=nullptr;
}

const SharedMasterPageDescriptor MasterPageContainer::Implementation::GetDescriptor (
    Token aToken) const
{
    if (aToken>=0 && static_cast<unsigned>(aToken)<maContainer.size())
        return maContainer[aToken];
    else
        return SharedMasterPageDescriptor();
}

SharedMasterPageDescriptor MasterPageContainer::Implementation::GetDescriptor (Token aToken)
{
    if (aToken>=0 && static_cast<unsigned>(aToken)<maContainer.size())
        return maContainer[aToken];
    else
        return SharedMasterPageDescriptor();
}

void MasterPageContainer::Implementation::InvalidatePreview (Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    SharedMasterPageDescriptor pDescriptor (GetDescriptor(aToken));
    if (pDescriptor.get() != nullptr)
    {
        pDescriptor->maSmallPreview = Image();
        pDescriptor->maLargePreview = Image();
        RequestPreview(aToken);
    }
}

Image MasterPageContainer::Implementation::GetPreviewForToken (
    MasterPageContainer::Token aToken,
    PreviewSize ePreviewSize)
{
    const ::osl::MutexGuard aGuard (maMutex);

    Image aPreview;
    PreviewState ePreviewState (GetPreviewState(aToken));

    SharedMasterPageDescriptor pDescriptor = GetDescriptor(aToken);

    // When the preview is missing but inexpensively creatable then do that
    // now.
    if (pDescriptor.get()!=nullptr)
    {
        if (ePreviewState == PS_CREATABLE)
            if (UpdateDescriptor(pDescriptor, false,false, true))
                if (pDescriptor->maLargePreview.GetSizePixel().Width() != 0)
                    ePreviewState = PS_AVAILABLE;

        switch (ePreviewState)
        {
            case PS_AVAILABLE:
                aPreview = pDescriptor->GetPreview(ePreviewSize);
                break;

            case PS_PREPARING:
                aPreview = GetPreviewSubstitution(
                    STR_TASKPANEL_PREPARING_PREVIEW_SUBSTITUTION,
                    ePreviewSize);
                break;

            case PS_CREATABLE:
                aPreview = GetPreviewSubstitution(
                    STR_TASKPANEL_PREPARING_PREVIEW_SUBSTITUTION,
                    ePreviewSize);
                break;

            case PS_NOT_AVAILABLE:
                aPreview = GetPreviewSubstitution(
                    STR_TASKPANEL_NOT_AVAILABLE_SUBSTITUTION,
                    ePreviewSize);
                if (ePreviewSize == SMALL)
                    pDescriptor->maSmallPreview = aPreview;
                else
                    pDescriptor->maLargePreview = aPreview;
                break;
        }
    }

    return aPreview;
}

MasterPageContainer::PreviewState MasterPageContainer::Implementation::GetPreviewState (
    Token aToken) const
{
    const ::osl::MutexGuard aGuard (maMutex);

    PreviewState eState (PS_NOT_AVAILABLE);

    SharedMasterPageDescriptor pDescriptor = GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
    {
        if (pDescriptor->maLargePreview.GetSizePixel().Width() != 0)
            eState = PS_AVAILABLE;
        else if (pDescriptor->mpPreviewProvider != nullptr)
        {
            // The preview does not exist but can be created.  When that is
            // not expensive then do it at once.
            if (mpRequestQueue->HasRequest(aToken))
                eState = PS_PREPARING;
            else
                eState = PS_CREATABLE;
        }
        else
            eState = PS_NOT_AVAILABLE;
    }

    return eState;
}

bool MasterPageContainer::Implementation::RequestPreview (Token aToken)
{
    SharedMasterPageDescriptor pDescriptor = GetDescriptor(aToken);
    if (pDescriptor.get() != nullptr)
        return mpRequestQueue->RequestPreview(pDescriptor);
    else
        return false;
}

Reference<frame::XModel> MasterPageContainer::Implementation::GetModel()
{
    const ::osl::MutexGuard aGuard (maMutex);

    if ( ! mxModel.is())
    {
        // Get the desktop a s service factory.
        uno::Reference<frame::XDesktop2> xDesktop  = frame::Desktop::create(
            ::comphelper::getProcessComponentContext() );

        // Create a new model.
        mxModel.set(
            ::comphelper::getProcessServiceFactory()->createInstance(
                "com.sun.star.presentation.PresentationDocument"),
            uno::UNO_QUERY);

        // Initialize the model.
        uno::Reference<frame::XLoadable> xLoadable (mxModel,uno::UNO_QUERY);
        if (xLoadable.is())
            xLoadable->initNew();

        // Use its tunnel to get a pointer to its core implementation.
        uno::Reference<lang::XUnoTunnel> xUnoTunnel (mxModel, uno::UNO_QUERY);
        if (xUnoTunnel.is())
        {
            mpDocument = reinterpret_cast<SdXImpressDocument*>(
                xUnoTunnel->getSomething(
                    SdXImpressDocument::getUnoTunnelId()))->GetDoc();
        }

        // Create a default page.
        uno::Reference<drawing::XDrawPagesSupplier> xSlideSupplier (mxModel, uno::UNO_QUERY);
        if (xSlideSupplier.is())
        {
            uno::Reference<drawing::XDrawPages> xSlides (
                xSlideSupplier->getDrawPages(), uno::UNO_QUERY);
            if (xSlides.is())
            {
                uno::Reference<drawing::XDrawPage> xNewPage (xSlides->insertNewByIndex(0));
                uno::Reference<beans::XPropertySet> xProperties(xNewPage, uno::UNO_QUERY);
                if (xProperties.is())
                    xProperties->setPropertyValue(
                        "Layout",
                        makeAny(sal_Int16(AUTOLAYOUT_TITLE)));
            }
        }
    }
    return mxModel;
}

SdDrawDocument* MasterPageContainer::Implementation::GetDocument()
{
    GetModel();
    return mpDocument;
}

Image MasterPageContainer::Implementation::GetPreviewSubstitution (
    const char* pId,
    PreviewSize ePreviewSize)
{
    const ::osl::MutexGuard aGuard (maMutex);

    Image aPreview;

    if (strcmp(pId, STR_TASKPANEL_PREPARING_PREVIEW_SUBSTITUTION) == 0)
    {
        Image& rPreview (ePreviewSize==SMALL
            ? maSmallPreviewBeingCreated
            : maLargePreviewBeingCreated);
        if (rPreview.GetSizePixel().Width() == 0)
        {
            rPreview = maPreviewRenderer.RenderSubstitution(
                ePreviewSize==SMALL ? maSmallPreviewSizePixel : maLargePreviewSizePixel,
                SdResId(STR_TASKPANEL_PREPARING_PREVIEW_SUBSTITUTION));
        }
        aPreview = rPreview;
    }
    else if (strcmp(pId, STR_TASKPANEL_NOT_AVAILABLE_SUBSTITUTION) == 0)
    {
        Image& rPreview (ePreviewSize==SMALL
            ? maSmallPreviewNotAvailable
            : maLargePreviewNotAvailable);
        if (rPreview.GetSizePixel().Width() == 0)
        {
            rPreview = maPreviewRenderer.RenderSubstitution(
                ePreviewSize==SMALL ? maSmallPreviewSizePixel : maLargePreviewSizePixel,
                SdResId(STR_TASKPANEL_NOT_AVAILABLE_SUBSTITUTION));
        }
        aPreview = rPreview;
    }

    return aPreview;
}

void MasterPageContainer::Implementation::CleanContainer()
{
    // Remove the empty elements at the end of the container.  The empty
    // elements in the middle can not be removed because that would
    // invalidate the references still held by others.
    int nIndex (maContainer.size()-1);
    while (nIndex>=0 && maContainer[nIndex].get()==nullptr)
        --nIndex;
    maContainer.resize(++nIndex);
}

void MasterPageContainer::Implementation::FireContainerChange (
    MasterPageContainerChangeEvent::EventType eType,
    Token aToken)
{
    ::std::vector<Link<MasterPageContainerChangeEvent&,void>> aCopy(maChangeListeners);
    MasterPageContainerChangeEvent aEvent;
    aEvent.meEventType = eType;
    aEvent.maChildToken = aToken;
    for (auto& rListener : aCopy)
        rListener.Call(aEvent);
}

bool MasterPageContainer::Implementation::UpdateDescriptor (
    const SharedMasterPageDescriptor& rpDescriptor,
    bool bForcePageObject,
    bool bForcePreview,
    bool bSendEvents)
{
    const ::osl::MutexGuard aGuard (maMutex);

    // We have to create the page object when the preview provider needs it
    // and the caller needs the preview.
    bForcePageObject |= (bForcePreview
        && rpDescriptor->mpPreviewProvider->NeedsPageObject()
        && rpDescriptor->mpMasterPage==nullptr);

    // Define a cost threshold so that an update or page object or preview
    // that is at least this cost are made at once. Updates with higher cost
    // are scheduled for later.
    sal_Int32 nCostThreshold (mpRequestQueue->IsEmpty() ? 5 : 0);

    // Update the page object (which may be used for the preview update).
    if (bForcePageObject)
        GetDocument();
    int nPageObjectModified (rpDescriptor->UpdatePageObject(
        (bForcePageObject ? -1 : nCostThreshold),
        mpDocument));
    if (nPageObjectModified == 1 && bSendEvents)
        FireContainerChange(
            MasterPageContainerChangeEvent::EventType::DATA_CHANGED,
            rpDescriptor->maToken);
    if (nPageObjectModified == -1 && bSendEvents)
        FireContainerChange(
            MasterPageContainerChangeEvent::EventType::CHILD_REMOVED,
            rpDescriptor->maToken);
    if (nPageObjectModified && ! mbFirstPageObjectSeen)
        UpdatePreviewSizePixel();

    // Update the preview.
    bool bPreviewModified (rpDescriptor->UpdatePreview(
        (bForcePreview ? -1 : nCostThreshold),
        maSmallPreviewSizePixel,
        maLargePreviewSizePixel,
        maPreviewRenderer));

    if (bPreviewModified && bSendEvents)
        FireContainerChange(
            MasterPageContainerChangeEvent::EventType::PREVIEW_CHANGED,
            rpDescriptor->maToken);

    return nPageObjectModified || bPreviewModified;
}

void MasterPageContainer::Implementation::ReleaseDescriptor (Token aToken)
{
    if (aToken>=0 && static_cast<unsigned>(aToken)<maContainer.size())
    {
        maContainer[aToken].reset();
    }
}

void MasterPageContainer::Implementation::FillingDone()
{
    mpRequestQueue->ProcessAllRequests();
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
