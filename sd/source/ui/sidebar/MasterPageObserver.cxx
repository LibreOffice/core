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

#include <MasterPageObserver.hxx>

#include <algorithm>
#include <iterator>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <set>
#include <unordered_map>
#include <memory>
#include <vector>
#include <svl/lstner.hxx>
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>
#include <tools/debug.hxx>

namespace sd {

class MasterPageObserver::Implementation
    : public SfxListener
{
public:
    /** The single instance of this class.  It is created on demand when
        Instance() is called for the first time.
    */
    static MasterPageObserver* mpInstance;

    /** The master page observer will listen to events of this document and
        detect changes of the use of master pages.
    */
    void RegisterDocument (SdDrawDocument& rDocument);

    /** The master page observer will stop to listen to events of this
        document.
    */
    void UnregisterDocument (SdDrawDocument& rDocument);

    /** Add a listener that is informed of master pages that are newly
        assigned to slides or become unassigned.
        @param rEventListener
            The event listener to call for future events.  Call
            RemoveEventListener() before the listener is destroyed.
    */
    void AddEventListener (const Link<MasterPageObserverEvent&,void>& rEventListener);

    /** Remove the given listener from the list of listeners.
        @param rEventListener
            After this method returns the given listener is not called back
            from this object.  Passing a listener that has not
            been registered before is safe and is silently ignored.
    */
    void RemoveEventListener (const Link<MasterPageObserverEvent&,void>& rEventListener);

private:
    ::std::vector<Link<MasterPageObserverEvent&,void>> maListeners;

    struct DrawDocHash {
        size_t operator()(SdDrawDocument* argument) const
        { return reinterpret_cast<unsigned long>(argument); }
    };
    typedef std::unordered_map<SdDrawDocument*,
                            MasterPageObserver::MasterPageNameSet,
                            DrawDocHash>
        MasterPageContainer;
    MasterPageContainer maUsedMasterPages;

    virtual void Notify(
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint) override;

    void AnalyzeUsedMasterPages (SdDrawDocument& rDocument);

    void SendEvent (MasterPageObserverEvent& rEvent);
};

MasterPageObserver* MasterPageObserver::Implementation::mpInstance = nullptr;

//===== MasterPageObserver ====================================================

MasterPageObserver&  MasterPageObserver::Instance()
{
    if (Implementation::mpInstance == nullptr)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (Implementation::mpInstance == nullptr)
        {
            MasterPageObserver* pInstance = new MasterPageObserver ();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::unique_ptr<SdGlobalResource>(pInstance));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            Implementation::mpInstance = pInstance;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    DBG_ASSERT(Implementation::mpInstance!=nullptr,
        "MasterPageObserver::Instance(): instance is NULL");
    return *Implementation::mpInstance;
}

void MasterPageObserver::RegisterDocument (SdDrawDocument& rDocument)
{
    mpImpl->RegisterDocument (rDocument);
}

void MasterPageObserver::UnregisterDocument (SdDrawDocument& rDocument)
{
    mpImpl->UnregisterDocument (rDocument);
}

void MasterPageObserver::AddEventListener (const Link<MasterPageObserverEvent&,void>& rEventListener)
{

    mpImpl->AddEventListener (rEventListener);
}

void MasterPageObserver::RemoveEventListener (const Link<MasterPageObserverEvent&,void>& rEventListener)
{
    mpImpl->RemoveEventListener (rEventListener);
}

MasterPageObserver::MasterPageObserver()
    : mpImpl (new Implementation)
{}

MasterPageObserver::~MasterPageObserver()
{}

//===== MasterPageObserver::Implementation ====================================

void MasterPageObserver::Implementation::RegisterDocument (
    SdDrawDocument& rDocument)
{
    // Gather the names of all the master pages in the given document.
    MasterPageContainer::mapped_type aMasterPageSet;
    sal_uInt16 nMasterPageCount = rDocument.GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = rDocument.GetMasterSdPage (nIndex, PageKind::Standard);
        if (pMasterPage != nullptr)
            aMasterPageSet.insert (pMasterPage->GetName());
    }

    bool bAlreadyExists = maUsedMasterPages.find(&rDocument) != maUsedMasterPages.end();
    maUsedMasterPages[&rDocument] = aMasterPageSet;

    if (!bAlreadyExists)
        StartListening (rDocument);
}

void MasterPageObserver::Implementation::UnregisterDocument (
    SdDrawDocument& rDocument)
{
    EndListening (rDocument);

    MasterPageContainer::iterator aMasterPageDescriptor(maUsedMasterPages.find(&rDocument));
    if(aMasterPageDescriptor != maUsedMasterPages.end())
        maUsedMasterPages.erase(aMasterPageDescriptor);
}

void MasterPageObserver::Implementation::AddEventListener (
    const Link<MasterPageObserverEvent&,void>& rEventListener)
{
    if (::std::find (
        maListeners.begin(),
        maListeners.end(),
        rEventListener) != maListeners.end())
        return;

    maListeners.push_back (rEventListener);

    // Tell the new listener about all the master pages that are
    // currently in use.
    for (const auto& rDocument : maUsedMasterPages)
    {
        ::std::set<OUString>::reverse_iterator aNameIterator;
        for (aNameIterator=rDocument.second.rbegin();
             aNameIterator!=rDocument.second.rend();
             ++aNameIterator)
        {
          MasterPageObserverEvent aEvent (
              MasterPageObserverEvent::ET_MASTER_PAGE_EXISTS,
              *aNameIterator);
          SendEvent (aEvent);
        }
    }
}

void MasterPageObserver::Implementation::RemoveEventListener (
    const Link<MasterPageObserverEvent&,void>& rEventListener)
{
    maListeners.erase (
        ::std::find (
            maListeners.begin(),
            maListeners.end(),
            rEventListener));
}

void MasterPageObserver::Implementation::Notify(
    SfxBroadcaster& rBroadcaster,
    const SfxHint& rHint)
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (!pSdrHint)
        return;

    switch (pSdrHint->GetKind())
    {
        case SdrHintKind::PageOrderChange:
            // Process the modified set of pages only when the number of
            // standard and notes master pages are equal.  This test
            // filters out events that are sent in between the insertion
            // of a new standard master page and a new notes master
            // page.
            if (dynamic_cast< const SdDrawDocument *>( &rBroadcaster ) !=  nullptr)
            {
                SdDrawDocument& rDocument (
                    static_cast<SdDrawDocument&>(rBroadcaster));
                if (rDocument.GetMasterSdPageCount(PageKind::Standard)
                    == rDocument.GetMasterSdPageCount(PageKind::Notes))
                {
                    AnalyzeUsedMasterPages (rDocument);
                }
            }
            break;

        default:
            break;
    }
}

void MasterPageObserver::Implementation::AnalyzeUsedMasterPages (
    SdDrawDocument& rDocument)
{
    // Create a set of names of the master pages used by the given document.
    sal_uInt16 nMasterPageCount = rDocument.GetMasterSdPageCount(PageKind::Standard);
    ::std::set<OUString> aCurrentMasterPages;
    for (sal_uInt16 nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = rDocument.GetMasterSdPage (nIndex, PageKind::Standard);
        if (pMasterPage != nullptr)
            aCurrentMasterPages.insert (pMasterPage->GetName());
    }

    std::vector<OUString> aNewMasterPages;
    std::vector<OUString> aRemovedMasterPages;
    MasterPageContainer::iterator aOldMasterPagesDescriptor (
        maUsedMasterPages.find(&rDocument));
    if (aOldMasterPagesDescriptor == maUsedMasterPages.end())
        return;

    // Send events about the newly used master pages.
    ::std::set_difference (
        aCurrentMasterPages.begin(),
        aCurrentMasterPages.end(),
        aOldMasterPagesDescriptor->second.begin(),
        aOldMasterPagesDescriptor->second.end(),
        std::back_inserter(aNewMasterPages));
    for (auto& aNewMasterPage : aNewMasterPages)
    {
        MasterPageObserverEvent aEvent (
            MasterPageObserverEvent::ET_MASTER_PAGE_ADDED,
            aNewMasterPage);
        SendEvent (aEvent);
    }

    // Send events about master pages that are not used any longer.
    ::std::set_difference (
        aOldMasterPagesDescriptor->second.begin(),
        aOldMasterPagesDescriptor->second.end(),
        aCurrentMasterPages.begin(),
        aCurrentMasterPages.end(),
        std::back_inserter(aRemovedMasterPages));
    for (auto& aRemovedMasterPage : aRemovedMasterPages)
    {
        MasterPageObserverEvent aEvent (
            MasterPageObserverEvent::ET_MASTER_PAGE_REMOVED,
            aRemovedMasterPage);
        SendEvent (aEvent);
    }

    // Store the new list of master pages.
    aOldMasterPagesDescriptor->second = aCurrentMasterPages;
}

void MasterPageObserver::Implementation::SendEvent (
    MasterPageObserverEvent& rEvent)
{
    for (auto& aLink : maListeners)
    {
        aLink.Call(rEvent);
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
