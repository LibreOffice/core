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

#include "MasterPageDescriptor.hxx"
#include "MasterPageContainerFiller.hxx"
#include "MasterPageContainerQueue.hxx"
#include "TemplateScanner.hxx"
#include "tools/AsynchronousTask.hxx"
#include "strings.hrc"
#include <algorithm>
#include <list>
#include <set>

#include "unomodel.hxx"
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <svx/svdpage.hxx>
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include "sdresid.hxx"
#include "tools/TimerBasedTaskExecution.hxx"
#include "pres.hxx"
#include <osl/mutex.hxx>
#include <boost/weak_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sd::toolpanel::controls;

namespace {

typedef ::std::vector<SharedMasterPageDescriptor> MasterPageContainerType;

} // end of anonymous namespace


namespace sd { namespace toolpanel { namespace controls {


/** Inner implementation class of the MasterPageContainer.
*/
class MasterPageContainer::Implementation
    : public SdGlobalResource,
      public MasterPageContainerFiller::ContainerAdapter,
      public MasterPageContainerQueue::ContainerAdapter
{
public:
    mutable ::osl::Mutex maMutex;

    static ::boost::weak_ptr<Implementation> mpInstance;
    MasterPageContainerType maContainer;

    static ::boost::shared_ptr<Implementation> Instance (void);

    void LateInit (void);
    void AddChangeListener (const Link& rLink);
    void RemoveChangeListener (const Link& rLink);
    void UpdatePreviewSizePixel (void);
    Size GetPreviewSizePixel (PreviewSize eSize) const;

    bool HasToken (Token aToken) const;
    const SharedMasterPageDescriptor GetDescriptor (MasterPageContainer::Token aToken) const;
    SharedMasterPageDescriptor GetDescriptor (MasterPageContainer::Token aToken);
    virtual Token PutMasterPage (const SharedMasterPageDescriptor& rDescriptor);
    void InvalidatePreview (Token aToken);
    Image GetPreviewForToken (
        Token aToken,
        PreviewSize ePreviewSize);
    PreviewState GetPreviewState (Token aToken) const;
    bool RequestPreview (Token aToken);

    Reference<frame::XModel> GetModel (void);
    SdDrawDocument* GetDocument (void);

    void FireContainerChange (
        MasterPageContainerChangeEvent::EventType eType,
        Token aToken,
        bool bNotifyAsynchronously = false);

    virtual bool UpdateDescriptor (
        const SharedMasterPageDescriptor& rpDescriptor,
        bool bForcePageObject,
        bool bForcePreview,
        bool bSendEvents);

    void ReleaseDescriptor (Token aToken);

    /** Called by the MasterPageContainerFiller to notify that all master
        pages from template documents have been added.
    */
    virtual void FillingDone (void);

private:
    Implementation (void);
    virtual ~Implementation (void);

    class Deleter { public:
        void operator() (Implementation* pObject) { delete pObject; }
    };
    friend class Deleter;

    enum InitializationState { NOT_INITIALIZED, INITIALIZING, INITIALIZED } meInitializationState;

    ::boost::scoped_ptr<MasterPageContainerQueue> mpRequestQueue;
    ::com::sun::star::uno::Reference<com::sun::star::frame::XModel> mxModel;
    SdDrawDocument* mpDocument;
    PreviewRenderer maPreviewRenderer;
    /** Remember whether the first page object has already been used to
        determine the correct size ratio.
    */
    bool mbFirstPageObjectSeen;

    // The widths for the previews contain two pixels for the border that is
    // painted arround the preview.
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

    ::std::vector<Link> maChangeListeners;

    // We have to remember the tasks for initialization and filling in case
    // a MasterPageContainer object is destroyed before these tasks have
    // been completed.
    ::boost::weak_ptr<sd::tools::TimerBasedTaskExecution> mpFillerTask;

    Size maSmallPreviewSizePixel;
    Size maLargePreviewSizePixel;

    bool mbContainerCleaningPending;

    typedef ::std::pair<MasterPageContainerChangeEvent::EventType,Token> EventData;
    DECL_LINK(AsynchronousNotifyCallback, EventData*);

    Image GetPreviewSubstitution (sal_uInt16 nId, PreviewSize ePreviewSize);

    void CleanContainer (void);
};




//===== MasterPageContainer ===================================================

::boost::weak_ptr<MasterPageContainer::Implementation>
    MasterPageContainer::Implementation::mpInstance;
static const MasterPageContainer::Token NIL_TOKEN (-1);




::boost::shared_ptr<MasterPageContainer::Implementation>
    MasterPageContainer::Implementation::Instance (void)
{
    ::boost::shared_ptr<MasterPageContainer::Implementation> pInstance;

    if (Implementation::mpInstance.expired())
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (Implementation::mpInstance.expired())
        {
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            pInstance = ::boost::shared_ptr<MasterPageContainer::Implementation>(
                new MasterPageContainer::Implementation(),
                MasterPageContainer::Implementation::Deleter());
            SdGlobalResourceContainer::Instance().AddResource(pInstance);
            Implementation::mpInstance = pInstance;
        }
        else
            pInstance = ::boost::shared_ptr<MasterPageContainer::Implementation>(
                Implementation::mpInstance);
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        pInstance = ::boost::shared_ptr<MasterPageContainer::Implementation>(
            Implementation::mpInstance);
    }

    DBG_ASSERT (pInstance.get()!=NULL,
        "MasterPageContainer::Implementation::Instance(): instance is NULL");
    return pInstance;
}




MasterPageContainer::MasterPageContainer (void)
    : mpImpl(Implementation::Instance()),
      mePreviewSize(SMALL)
{
    mpImpl->LateInit();
}




MasterPageContainer::~MasterPageContainer (void)
{
}




void MasterPageContainer::AddChangeListener (const Link& rLink)
{
    mpImpl->AddChangeListener(rLink);
}




void MasterPageContainer::RemoveChangeListener (const Link& rLink)
{
    mpImpl->RemoveChangeListener(rLink);
}




void MasterPageContainer::SetPreviewSize (PreviewSize eSize)
{
    mePreviewSize = eSize;
    mpImpl->FireContainerChange(
        MasterPageContainerChangeEvent::SIZE_CHANGED,
        NIL_TOKEN);
}




MasterPageContainer::PreviewSize MasterPageContainer::GetPreviewSize (void) const
{
    return mePreviewSize;
}




Size MasterPageContainer::GetPreviewSizePixel (void) const
{
    return mpImpl->GetPreviewSizePixel(mePreviewSize);
}




MasterPageContainer::Token MasterPageContainer::PutMasterPage (
    const SharedMasterPageDescriptor& rDescriptor)
{
    return mpImpl->PutMasterPage(rDescriptor);
}




void MasterPageContainer::AcquireToken (Token aToken)
{
    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != NULL)
    {
        ++pDescriptor->mnUseCount;
    }
}




void MasterPageContainer::ReleaseToken (Token aToken)
{
    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != NULL)
    {
        OSL_ASSERT(pDescriptor->mnUseCount>0);
        --pDescriptor->mnUseCount;
        if (pDescriptor->mnUseCount <= 0)
        {
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
    }
}




int MasterPageContainer::GetTokenCount (void) const
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
    if (pPage != NULL)
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
    if (pDescriptor.get() != NULL)
        return pDescriptor->msURL;
    else
        return OUString();
}




OUString MasterPageContainer::GetPageNameForToken (
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != NULL)
        return pDescriptor->msPageName;
    else
        return OUString();
}




OUString MasterPageContainer::GetStyleNameForToken (
    MasterPageContainer::Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != NULL)
        return pDescriptor->msStyleName;
    else
        return OUString();
}




SdPage* MasterPageContainer::GetPageObjectForToken (
    MasterPageContainer::Token aToken,
    bool bLoad)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SdPage* pPageObject = NULL;
    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != NULL)
    {
        pPageObject = pDescriptor->mpMasterPage;
        if (pPageObject == NULL)
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
    if (pDescriptor.get() != NULL)
        return pDescriptor->meOrigin;
    else
        return UNKNOWN;
}




sal_Int32 MasterPageContainer::GetTemplateIndexForToken (Token aToken)
{
    const ::osl::MutexGuard aGuard (mpImpl->maMutex);

    SharedMasterPageDescriptor pDescriptor = mpImpl->GetDescriptor(aToken);
    if (pDescriptor.get() != NULL)
        return pDescriptor->mnTemplateIndex;
    else
        return -1;
}




SharedMasterPageDescriptor MasterPageContainer::GetDescriptorForToken (
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

MasterPageContainer::Implementation::Implementation (void)
    : maMutex(),
      maContainer(),
      meInitializationState(NOT_INITIALIZED),
      mpRequestQueue(NULL),
      mxModel(NULL),
      mpDocument(NULL),
      maPreviewRenderer(),
      mbFirstPageObjectSeen(false),
      maLargePreviewBeingCreated(),
      maSmallPreviewBeingCreated(),
      maLargePreviewNotAvailable(),
      maSmallPreviewNotAvailable(),
      maChangeListeners(),
      maSmallPreviewSizePixel(),
      maLargePreviewSizePixel(),
      mbContainerCleaningPending(true)

{
    UpdatePreviewSizePixel();
}




MasterPageContainer::Implementation::~Implementation (void)
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
        catch (const ::com::sun::star::util::CloseVetoException&)
        {
        }
    }
    mxModel = NULL;
}




void MasterPageContainer::Implementation::LateInit (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (meInitializationState == NOT_INITIALIZED)
    {
        meInitializationState = INITIALIZING;

        OSL_ASSERT(Instance().get()==this);
        mpRequestQueue.reset(MasterPageContainerQueue::Create(
            ::boost::shared_ptr<MasterPageContainerQueue::ContainerAdapter>(Instance())));

        mpFillerTask = ::sd::tools::TimerBasedTaskExecution::Create(
            ::boost::shared_ptr<tools::AsynchronousTask>(new MasterPageContainerFiller(*this)),
            5,
            50);

        meInitializationState = INITIALIZED;
    }
}




void MasterPageContainer::Implementation::AddChangeListener (const Link& rLink)
{
    const ::osl::MutexGuard aGuard (maMutex);

    ::std::vector<Link>::iterator iListener (
        ::std::find(maChangeListeners.begin(),maChangeListeners.end(),rLink));
    if (iListener == maChangeListeners.end())
        maChangeListeners.push_back(rLink);

}




void MasterPageContainer::Implementation::RemoveChangeListener (const Link& rLink)
{
    const ::osl::MutexGuard aGuard (maMutex);

    ::std::vector<Link>::iterator iListener (
        ::std::find(maChangeListeners.begin(),maChangeListeners.end(),rLink));
    if (iListener != maChangeListeners.end())
        maChangeListeners.erase(iListener);
}




void MasterPageContainer::Implementation::UpdatePreviewSizePixel (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    // The default aspect ratio is 4:3
    int nWidth (4);
    int nHeight (3);

    // Search for the first entry with an existing master page.
    MasterPageContainerType::const_iterator iDescriptor;
    MasterPageContainerType::const_iterator iContainerEnd(maContainer.end());
    for (iDescriptor=maContainer.begin(); iDescriptor!=iContainerEnd; ++iDescriptor)
        if (*iDescriptor!=0 && (*iDescriptor)->mpMasterPage != NULL)
        {
            Size aPageSize ((*iDescriptor)->mpMasterPage->GetSize());
            OSL_ASSERT(aPageSize.Width() > 0 && aPageSize.Height() > 0);
            if (aPageSize.Width() > 0)
                nWidth = aPageSize.Width();
            if (aPageSize.Height() > 0)
                nHeight = aPageSize.Height();
            mbFirstPageObjectSeen = true;
            break;
        }

    maSmallPreviewSizePixel.Width() = SMALL_PREVIEW_WIDTH;
    maLargePreviewSizePixel.Width() = LARGE_PREVIEW_WIDTH;

    int nNewSmallHeight ((maSmallPreviewSizePixel.Width()-2) * nHeight / nWidth + 2);
    int nNewLargeHeight ((maLargePreviewSizePixel.Width()-2) * nHeight / nWidth + 2);

    if (nNewSmallHeight!=maSmallPreviewSizePixel.Height()
        || nNewLargeHeight!=maLargePreviewSizePixel.Height())
    {
        maSmallPreviewSizePixel.Height() = nNewSmallHeight;
        maLargePreviewSizePixel.Height() = nNewLargeHeight;
        FireContainerChange(
            MasterPageContainerChangeEvent::SIZE_CHANGED,
            NIL_TOKEN);
    }
}




Size MasterPageContainer::Implementation::GetPreviewSizePixel (PreviewSize eSize) const
{
    if (eSize == SMALL)
        return maSmallPreviewSizePixel;
    else
        return maLargePreviewSizePixel;
}




IMPL_LINK(MasterPageContainer::Implementation,AsynchronousNotifyCallback, EventData*, pData)
{
    const ::osl::MutexGuard aGuard (maMutex);

    if (pData != NULL)
    {
        FireContainerChange(pData->first, pData->second, false);
        delete pData;
    }

    return 0;
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
        bool bIgnore (rpDescriptor->mpPageObjectProvider.get()==NULL
            && rpDescriptor->msURL.isEmpty());

        if ( ! bIgnore)
        {
            if (mbContainerCleaningPending)
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

            FireContainerChange(MasterPageContainerChangeEvent::CHILD_ADDED,aResult);
        }
    }
    else
    {
        // Update an existing MasterPageDescriptor.
        aResult = (*aEntry)->maToken;
        ::std::auto_ptr<std::vector<MasterPageContainerChangeEvent::EventType> > pEventTypes(
            (*aEntry)->Update(*rpDescriptor));
        if (pEventTypes.get()!=NULL && pEventTypes->size()>0)
        {
            // One or more aspects of the descriptor have changed.  Send
            // appropriate events to the listeners.
            UpdateDescriptor(*aEntry,false,false, true);

            std::vector<MasterPageContainerChangeEvent::EventType>::const_iterator iEventType;
            for (iEventType=pEventTypes->begin(); iEventType!=pEventTypes->end(); ++iEventType)
            {
                FireContainerChange(
                    *iEventType,
                    (*aEntry)->maToken,
                    false);
            }
        }
    }

    return aResult;
}




bool MasterPageContainer::Implementation::HasToken (Token aToken) const
{
    return aToken>=0
        && (unsigned)aToken<maContainer.size()
        && maContainer[aToken].get()!=NULL;
}




const SharedMasterPageDescriptor MasterPageContainer::Implementation::GetDescriptor (
    Token aToken) const
{
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
        return maContainer[aToken];
    else
        return SharedMasterPageDescriptor();
}




SharedMasterPageDescriptor MasterPageContainer::Implementation::GetDescriptor (Token aToken)
{
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
        return maContainer[aToken];
    else
        return SharedMasterPageDescriptor();
}




void MasterPageContainer::Implementation::InvalidatePreview (Token aToken)
{
    const ::osl::MutexGuard aGuard (maMutex);

    SharedMasterPageDescriptor pDescriptor (GetDescriptor(aToken));
    if (pDescriptor.get() != NULL)
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
    if (pDescriptor.get()!=NULL)
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
    if (pDescriptor.get() != NULL)
    {
        if (pDescriptor->maLargePreview.GetSizePixel().Width() != 0)
            eState = PS_AVAILABLE;
        else if (pDescriptor->mpPreviewProvider.get() != NULL)
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
    if (pDescriptor.get() != NULL)
        return mpRequestQueue->RequestPreview(pDescriptor);
    else
        return false;
}




Reference<frame::XModel> MasterPageContainer::Implementation::GetModel (void)
{
    const ::osl::MutexGuard aGuard (maMutex);

    if ( ! mxModel.is())
    {
        // Get the desktop a s service factory.
        uno::Reference<frame::XDesktop2> xDesktop  = frame::Desktop::create(
            ::comphelper::getProcessComponentContext() );

        // Create a new model.
        OUString sModelServiceName ( "com.sun.star.presentation.PresentationDocument");
        mxModel = uno::Reference<frame::XModel>(
            ::comphelper::getProcessServiceFactory()->createInstance(
                sModelServiceName),
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
                sal_Int32 nIndex (0);
                uno::Reference<drawing::XDrawPage> xNewPage (xSlides->insertNewByIndex(nIndex));
                uno::Reference<beans::XPropertySet> xProperties(xNewPage, uno::UNO_QUERY);
                if (xProperties.is())
                    xProperties->setPropertyValue(
                        "Layout",
                        makeAny((sal_Int16)AUTOLAYOUT_TITLE));
            }
        }
    }
    return mxModel;
}




SdDrawDocument* MasterPageContainer::Implementation::GetDocument (void)
{
    GetModel();
    return mpDocument;
}




Image MasterPageContainer::Implementation::GetPreviewSubstitution (
    sal_uInt16 nId,
    PreviewSize ePreviewSize)
{
    const ::osl::MutexGuard aGuard (maMutex);

    Image aPreview;

    switch (nId)
    {
        case STR_TASKPANEL_PREPARING_PREVIEW_SUBSTITUTION:
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
        break;

        case STR_TASKPANEL_NOT_AVAILABLE_SUBSTITUTION:
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
        break;
    }

    return aPreview;
}




void MasterPageContainer::Implementation::CleanContainer (void)
{
    // Remove the empty elements at the end of the container.  The empty
    // elements in the middle can not be removed because that would
    // invalidate the references still held by others.
    int nIndex (maContainer.size()-1);
    while (nIndex>=0 && maContainer[nIndex].get()==NULL)
        --nIndex;
    maContainer.resize(++nIndex);
}




void MasterPageContainer::Implementation::FireContainerChange (
    MasterPageContainerChangeEvent::EventType eType,
    Token aToken,
    bool bNotifyAsynchronously)
{
    if (bNotifyAsynchronously)
    {
        Application::PostUserEvent(
            LINK(this,Implementation,AsynchronousNotifyCallback),
            new EventData(eType,aToken));
    }
    else
    {
        ::std::vector<Link> aCopy(maChangeListeners.begin(),maChangeListeners.end());
        ::std::vector<Link>::iterator iListener;
        MasterPageContainerChangeEvent aEvent;
        aEvent.meEventType = eType;
        aEvent.maChildToken = aToken;
        for (iListener=aCopy.begin(); iListener!=aCopy.end(); ++iListener)
            iListener->Call(&aEvent);
    }
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
        && rpDescriptor->mpMasterPage==NULL);

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
            MasterPageContainerChangeEvent::DATA_CHANGED,
            rpDescriptor->maToken);
    if (nPageObjectModified == -1 && bSendEvents)
        FireContainerChange(
            MasterPageContainerChangeEvent::CHILD_REMOVED,
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
            MasterPageContainerChangeEvent::PREVIEW_CHANGED,
            rpDescriptor->maToken);

    return nPageObjectModified || bPreviewModified;
}




void MasterPageContainer::Implementation::ReleaseDescriptor (Token aToken)
{
    if (aToken>=0 && (unsigned)aToken<maContainer.size())
    {
        maContainer[aToken].reset();
        mbContainerCleaningPending = true;
    }
}




void MasterPageContainer::Implementation::FillingDone (void)
{
    mpRequestQueue->ProcessAllRequests();
}



} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
