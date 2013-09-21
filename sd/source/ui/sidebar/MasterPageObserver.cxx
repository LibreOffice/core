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

#include "MasterPageObserver.hxx"

#include <algorithm>
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <boost/unordered_map.hpp>
#include <set>
#include <vector>
#include <svl/lstner.hxx>
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>


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
    void AddEventListener (const Link& rEventListener);

    /** Remove the given listener from the list of listeners.
        @param rEventListener
            After this method returns the given listener is not called back
            from this object.  Passing a listener that has not
            been registered before is safe and is silently ignored.
    */
    void RemoveEventListener (const Link& rEventListener);

    /** Return a set of the names of master pages for the given document.
        This convenience method exists because this set is part of the
        internal data structure and thus takes no time to create.
    */
    inline MasterPageObserver::MasterPageNameSet GetMasterPageNames (
        SdDrawDocument& rDocument);

private:
    ::std::vector<Link> maListeners;

    struct DrawDocHash {
        size_t operator()(SdDrawDocument* argument) const
        { return reinterpret_cast<unsigned long>(argument); }
    };
    typedef ::boost::unordered_map<SdDrawDocument*,
                            MasterPageObserver::MasterPageNameSet,
                            DrawDocHash>
        MasterPageContainer;
    MasterPageContainer maUsedMasterPages;

    virtual void Notify(
        SfxBroadcaster& rBroadcaster,
        const SfxHint& rHint);

    void AnalyzeUsedMasterPages (SdDrawDocument& rDocument);

    void SendEvent (MasterPageObserverEvent& rEvent);
};

MasterPageObserver* MasterPageObserver::Implementation::mpInstance = NULL;



//===== MasterPageObserver ====================================================

MasterPageObserver&  MasterPageObserver::Instance (void)
{
    if (Implementation::mpInstance == NULL)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (Implementation::mpInstance == NULL)
        {
            MasterPageObserver* pInstance = new MasterPageObserver ();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::auto_ptr<SdGlobalResource>(pInstance));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            Implementation::mpInstance = pInstance;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    DBG_ASSERT(Implementation::mpInstance!=NULL,
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




void MasterPageObserver::AddEventListener (const Link& rEventListener)
{

    mpImpl->AddEventListener (rEventListener);
}




void MasterPageObserver::RemoveEventListener (const Link& rEventListener)
{
    mpImpl->RemoveEventListener (rEventListener);
}




MasterPageObserver::MasterPageObserver (void)
    : mpImpl (new Implementation())
{}




MasterPageObserver::~MasterPageObserver (void)
{}




//===== MasterPageObserver::Implementation ====================================

void MasterPageObserver::Implementation::RegisterDocument (
    SdDrawDocument& rDocument)
{
    // Gather the names of all the master pages in the given document.
    MasterPageContainer::mapped_type aMasterPageSet;
    sal_uInt16 nMasterPageCount = rDocument.GetMasterSdPageCount(PK_STANDARD);
    for (sal_uInt16 nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = rDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage != NULL)
            aMasterPageSet.insert (pMasterPage->GetName());
    }

    maUsedMasterPages[&rDocument] = aMasterPageSet;

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
    const Link& rEventListener)
{
    if (::std::find (
        maListeners.begin(),
        maListeners.end(),
        rEventListener) == maListeners.end())
    {
        maListeners.push_back (rEventListener);

        // Tell the new listener about all the master pages that are
        // currently in use.
        typedef ::std::vector<String> StringList;
        StringList aNewMasterPages;
        StringList aRemovedMasterPages;
        MasterPageContainer::iterator aDocumentIterator;
        for (aDocumentIterator=maUsedMasterPages.begin();
             aDocumentIterator!=maUsedMasterPages.end();
             ++aDocumentIterator)
        {
            ::std::set<String>::reverse_iterator aNameIterator;
            for (aNameIterator=aDocumentIterator->second.rbegin();
                 aNameIterator!=aDocumentIterator->second.rend();
                 ++aNameIterator)
            {
              MasterPageObserverEvent aEvent (
                  MasterPageObserverEvent::ET_MASTER_PAGE_EXISTS,
                  *aDocumentIterator->first,
                  *aNameIterator);
              SendEvent (aEvent);
            }
        }
    }
}




void MasterPageObserver::Implementation::RemoveEventListener (
    const Link& rEventListener)
{
    maListeners.erase (
        ::std::find (
            maListeners.begin(),
            maListeners.end(),
            rEventListener));
}




MasterPageObserver::MasterPageNameSet
    MasterPageObserver::Implementation::GetMasterPageNames (
        SdDrawDocument& rDocument)
{
    MasterPageContainer::iterator aMasterPageDescriptor (
        maUsedMasterPages.find(&rDocument));
    if (aMasterPageDescriptor != maUsedMasterPages.end())
        return aMasterPageDescriptor->second;
    else
        // Not found so return an empty set.
        return MasterPageObserver::MasterPageNameSet();
}




void MasterPageObserver::Implementation::Notify(
    SfxBroadcaster& rBroadcaster,
    const SfxHint& rHint)
{
    if (rHint.ISA(SdrHint))
    {
        SdrHint& rSdrHint (*PTR_CAST(SdrHint,&rHint));
        switch (rSdrHint.GetKind())
        {
            case HINT_PAGEORDERCHG:
                // Process the modified set of pages only when the number of
                // standard and notes master pages are equal.  This test
                // filters out events that are sent in between the insertion
                // of a new standard master page and a new notes master
                // page.
                if (rBroadcaster.ISA(SdDrawDocument))
                {
                    SdDrawDocument& rDocument (
                        static_cast<SdDrawDocument&>(rBroadcaster));
                    if (rDocument.GetMasterSdPageCount(PK_STANDARD)
                        == rDocument.GetMasterSdPageCount(PK_NOTES))
                    {
                        AnalyzeUsedMasterPages (rDocument);
                    }
                }
                break;

            default:
                break;
        }
    }
}




void MasterPageObserver::Implementation::AnalyzeUsedMasterPages (
    SdDrawDocument& rDocument)
{
    // Create a set of names of the master pages used by the given document.
    sal_uInt16 nMasterPageCount = rDocument.GetMasterSdPageCount(PK_STANDARD);
    ::std::set<String> aCurrentMasterPages;
    for (sal_uInt16 nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = rDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage != NULL)
            aCurrentMasterPages.insert (pMasterPage->GetName());
        OSL_TRACE("currently used master page %d is %s",
            nIndex,
            ::rtl::OUStringToOString(pMasterPage->GetName(),
                RTL_TEXTENCODING_UTF8).getStr());
    }

    typedef ::std::vector<String> StringList;
    StringList aNewMasterPages;
    StringList aRemovedMasterPages;
    MasterPageContainer::iterator aOldMasterPagesDescriptor (
        maUsedMasterPages.find(&rDocument));
    if (aOldMasterPagesDescriptor != maUsedMasterPages.end())
    {
        StringList::iterator I;

        ::std::set<String>::iterator J;
        int i=0;
        for (J=aOldMasterPagesDescriptor->second.begin();
             J!=aOldMasterPagesDescriptor->second.end();
             ++J)
            OSL_TRACE("old used master page %d is %s",
            i++,
            ::rtl::OUStringToOString(*J,
                RTL_TEXTENCODING_UTF8).getStr());

        // Send events about the newly used master pages.
        ::std::set_difference (
            aCurrentMasterPages.begin(),
            aCurrentMasterPages.end(),
            aOldMasterPagesDescriptor->second.begin(),
            aOldMasterPagesDescriptor->second.end(),
            ::std::back_insert_iterator<StringList>(aNewMasterPages));
        for (I=aNewMasterPages.begin(); I!=aNewMasterPages.end(); ++I)
        {
            OSL_TRACE("    added master page %s",
                ::rtl::OUStringToOString(*I,
                    RTL_TEXTENCODING_UTF8).getStr());

            MasterPageObserverEvent aEvent (
                MasterPageObserverEvent::ET_MASTER_PAGE_ADDED,
                rDocument,
                *I);
            SendEvent (aEvent);
        }

        // Send events about master pages that are not used any longer.
        ::std::set_difference (
            aOldMasterPagesDescriptor->second.begin(),
            aOldMasterPagesDescriptor->second.end(),
            aCurrentMasterPages.begin(),
            aCurrentMasterPages.end(),
            ::std::back_insert_iterator<StringList>(aRemovedMasterPages));
        for (I=aRemovedMasterPages.begin(); I!=aRemovedMasterPages.end(); ++I)
        {
            OSL_TRACE("    removed master page %s",
                ::rtl::OUStringToOString(*I,
                    RTL_TEXTENCODING_UTF8).getStr());

            MasterPageObserverEvent aEvent (
                MasterPageObserverEvent::ET_MASTER_PAGE_REMOVED,
                rDocument,
                *I);
            SendEvent (aEvent);
        }

        // Store the new list of master pages.
        aOldMasterPagesDescriptor->second = aCurrentMasterPages;
    }
}




void MasterPageObserver::Implementation::SendEvent (
    MasterPageObserverEvent& rEvent)
{
    ::std::vector<Link>::iterator aLink (maListeners.begin());
    ::std::vector<Link>::iterator aEnd (maListeners.end());
    while (aLink!=aEnd)
    {
        aLink->Call (&rEvent);
        ++aLink;
    }
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
