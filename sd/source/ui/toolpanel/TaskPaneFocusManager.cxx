/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TaskPaneFocusManager.hxx"

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <rtl/instance.hxx>
#include <hash_map>

namespace {

class WindowHash
{
public:
    size_t operator()(const ::Window* argument) const
    { return reinterpret_cast<unsigned long>(argument); }
};

class EventDescriptor
{
public:
    EventDescriptor (const KeyCode& rKey, ::Window* pWindow)
        : maKeyCode(rKey), mpTargetWindow(pWindow) {}
    KeyCode maKeyCode;
    ::Window* mpTargetWindow;
};

} // end of anonymous namespace




namespace sd { namespace toolpanel {



class FocusManager::LinkMap
    : public ::std::hash_multimap< ::Window*, EventDescriptor, WindowHash>
{
};



FocusManager& FocusManager::Instance (void)
{
    static FocusManager* spInstance = NULL;

    if (spInstance == NULL)
    {
        ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());
        if (spInstance == NULL)
        {
            static FocusManager aInstance;
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            spInstance = &aInstance;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return *spInstance;
}




FocusManager::FocusManager (void)
    : mpLinks(new LinkMap())
{
}




FocusManager::~FocusManager (void)
{
    Clear();
}




void FocusManager::Clear (void)
{
    if (mpLinks.get() != NULL)
    {
        while ( ! mpLinks->empty())
        {
            ::Window* pWindow = mpLinks->begin()->first;
            if (pWindow == NULL)
            {
                mpLinks->erase(mpLinks->begin());
            }
            else
            {
                RemoveLinks(pWindow);
            }
        }
    }
}




void FocusManager::RegisterUpLink (::Window* pSource, ::Window* pTarget)
{
    RegisterLink(pSource, pTarget, KEY_ESCAPE);
}




void FocusManager::RegisterDownLink (::Window* pSource, ::Window* pTarget)
{
    RegisterLink(pSource, pTarget, KEY_RETURN);
}




void FocusManager::RegisterLink (
    ::Window* pSource,
    ::Window* pTarget,
    const KeyCode& rKey)
{
    OSL_ASSERT(pSource!=NULL);
    OSL_ASSERT(pTarget!=NULL);

    if (pSource==NULL || pTarget==NULL)
        return;

    // Register this focus manager as event listener at the source window.
    if (mpLinks->equal_range(pSource).first == mpLinks->end())
        pSource->AddEventListener (LINK (this, FocusManager, WindowEventListener));
    mpLinks->insert(LinkMap::value_type(pSource, EventDescriptor(rKey,pTarget)));
}




void FocusManager::RemoveLinks (
    ::Window* pSourceWindow,
    ::Window* pTargetWindow)
{
    OSL_ASSERT(pSourceWindow!=NULL);
    OSL_ASSERT(pTargetWindow!=NULL);

    if (pSourceWindow==NULL || pTargetWindow==NULL)
    {
        // This method was called with invalid arguments.  To avoid
        // referencing windows that will soon be deleted we clear *all*
        // links as an emergency fallback.
        Clear();
        return;
    }

    ::std::pair<LinkMap::iterator,LinkMap::iterator> aCandidates;
    LinkMap::iterator iCandidate;
    bool bLoop (mpLinks->size() > 0);
    while (bLoop)
    {
        aCandidates = mpLinks->equal_range(pSourceWindow);
        if (aCandidates.first == mpLinks->end())
        {
            // No links for the source window found -> nothing more to do.
            bLoop = false;
        }
        else
        {
            // Set the loop control to false so that when no candidate for
            // deletion is found the loop is left.
            bLoop = false;
            for (iCandidate=aCandidates.first; iCandidate!=aCandidates.second; ++iCandidate)
                if (iCandidate->second.mpTargetWindow == pTargetWindow)
                {
                    mpLinks->erase(iCandidate);
                    // One link erased.  The iterators have become invalid so
                    // start the search for links to delete anew.
                    bLoop = true;
                    break;
                }
        }
    }

    RemoveUnusedEventListener(pSourceWindow);
}




void FocusManager::RemoveLinks (::Window* pWindow)
{
    OSL_ASSERT(pWindow!=NULL);

    if (pWindow == NULL)
    {
        // This method was called with invalid arguments.  To avoid
        // referencing windows that will soon be deleted we clear *all*
        // links as an emergency fallback.
        Clear();
        return;
    }

    // Make sure that we are not called back for the window.
    pWindow->RemoveEventListener (LINK (this, FocusManager, WindowEventListener));

    // Remove the links from the given window.
    ::std::pair<LinkMap::iterator,LinkMap::iterator> aCandidates(mpLinks->equal_range(pWindow));
    mpLinks->erase(aCandidates.first, aCandidates.second);

    // Remove links to the given window.
    bool bLinkRemoved;
    do
    {
        bLinkRemoved = false;
        LinkMap::iterator iLink;
        for (iLink=mpLinks->begin(); iLink!=mpLinks->end(); ++iLink)
        {
            if (iLink->second.mpTargetWindow == pWindow)
            {
                RemoveUnusedEventListener(iLink->first);
                mpLinks->erase(iLink);
                bLinkRemoved = true;
                break;
            }
        }
    }
    while (bLinkRemoved);
}




void FocusManager::RemoveUnusedEventListener (::Window* pWindow)
{
    OSL_ASSERT(pWindow!=NULL);

    if (pWindow == NULL)
        return;

    // When there are no more links from the window to another window
    // then remove the event listener from the window.
    if (mpLinks->find(pWindow) == mpLinks->end())
        pWindow->RemoveEventListener (LINK (this, FocusManager, WindowEventListener));
}




bool FocusManager::TransferFocus (
    ::Window* pSourceWindow,
    const KeyCode& rKeyCode)
{
    bool bSuccess (false);

    OSL_ASSERT(pSourceWindow!=NULL);
    if (pSourceWindow == NULL)
        return bSuccess;

    ::std::pair<LinkMap::iterator,LinkMap::iterator> aCandidates (
        mpLinks->equal_range(pSourceWindow));
    LinkMap::const_iterator iCandidate;
    for (iCandidate=aCandidates.first; iCandidate!=aCandidates.second; ++iCandidate)
        if (iCandidate->second.maKeyCode == rKeyCode)
        {
            OSL_ASSERT(iCandidate->second.mpTargetWindow != NULL);
            iCandidate->second.mpTargetWindow->GrabFocus();
            bSuccess = true;
            break;
        }

    return bSuccess;
}




IMPL_LINK(FocusManager, WindowEventListener, VclSimpleEvent*, pEvent)
{
    VclWindowEvent* pWindowEvent = dynamic_cast< VclWindowEvent* >(pEvent);

    if (pWindowEvent)
    {
        switch (pWindowEvent->GetId())
        {
            case VCLEVENT_WINDOW_KEYINPUT:
            {
                ::Window* pSource = pWindowEvent->GetWindow();
                KeyEvent* pKeyEvent = static_cast<KeyEvent*>(pWindowEvent->GetData());
                TransferFocus(pSource, pKeyEvent->GetKeyCode());
            }
            break;

            case VCLEVENT_OBJECT_DYING:
                RemoveLinks(pWindowEvent->GetWindow());
                break;
        }
    }
    return 1;
}


} } // end of namespace ::sd::toolpanel
