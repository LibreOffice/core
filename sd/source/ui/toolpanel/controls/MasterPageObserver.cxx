/*************************************************************************
 *
 *  $RCSfile: MasterPageObserver.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:59:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "MasterPageObserver.hxx"

#include <algorithm>
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include <hash_map>
#include <set>
#include <vector>
#include <svtools/lstner.hxx>
#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#include <osl/doublecheckedlocking.h>
#endif
#ifndef INCLUDED_OSL_GETGLOBALMUTEX_HXX
#include <osl/getglobalmutex.hxx>
#endif


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
    typedef ::std::hash_map<SdDrawDocument*,
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
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();

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




MasterPageObserver::MasterPageNameSet MasterPageObserver::GetMasterPageNames (
    SdDrawDocument& rDocument)
{
    return mpImpl->GetMasterPageNames (rDocument);
}




//===== MasterPageObserver::Implementation ====================================

void MasterPageObserver::Implementation::RegisterDocument (
    SdDrawDocument& rDocument)
{
    // Gather the names of all the master pages in the given document.
    MasterPageContainer::data_type aMasterPageSet;
    USHORT nMasterPageCount = rDocument.GetMasterSdPageCount(PK_STANDARD);
    for (USHORT nIndex=0; nIndex<nMasterPageCount; nIndex++)
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

    maUsedMasterPages.erase(maUsedMasterPages.find(&rDocument));
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
        }
    }
}




void MasterPageObserver::Implementation::AnalyzeUsedMasterPages (
    SdDrawDocument& rDocument)
{
    // Create a set of names of the master pages used by the given document.
    USHORT nMasterPageCount = rDocument.GetMasterSdPageCount(PK_STANDARD);
    ::std::set<String> aCurrentMasterPages;
    for (USHORT nIndex=0; nIndex<nMasterPageCount; nIndex++)
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
