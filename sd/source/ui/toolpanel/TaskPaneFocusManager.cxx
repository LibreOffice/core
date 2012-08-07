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


#include "TaskPaneFocusManager.hxx"

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <rtl/instance.hxx>
#include <boost/unordered_map.hpp>

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
    : public ::boost::unordered_multimap< ::Window*, EventDescriptor, WindowHash>
{
};

struct FocusManagerCreator
{
    FocusManager m_aFocusManager;
};

namespace
{
    class theFocusManagerInstance : public rtl::Static< FocusManagerCreator, theFocusManagerInstance> {};
}

FocusManager& FocusManager::Instance (void)
{
    return theFocusManagerInstance::get().m_aFocusManager;
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
    mpLinks->erase(pWindow);

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
                ::Window* const pSourceWindow(iLink->first);
                mpLinks->erase(iLink);
                RemoveUnusedEventListener(pSourceWindow);
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
    if (pEvent!=NULL && pEvent->ISA(VclWindowEvent))
    {
        VclWindowEvent* pWindowEvent = static_cast<VclWindowEvent*>(pEvent);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
