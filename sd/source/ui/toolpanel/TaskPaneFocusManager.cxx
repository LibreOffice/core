/*************************************************************************
 *
 *  $RCSfile: TaskPaneFocusManager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:21:08 $
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

#include "TaskPaneFocusManager.hxx"

#include <vcl/window.hxx>
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif
#include <hash_map>

namespace {

class WindowHash
{
public:
    size_t operator()(::Window* argument) const
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



class FocusManager::LinkMap : public ::std::hash_multimap< ::Window*, EventDescriptor, WindowHash>
{
};



FocusManager* FocusManager::spInstance = NULL;


FocusManager& FocusManager::Instance (void)
{
    if (spInstance == NULL)
    {
        ::vos::OGuard aGuard (::Application::GetSolarMutex());
        if (spInstance == NULL)
            spInstance = new FocusManager ();
    }
    return *spInstance;
}




FocusManager::FocusManager (void)
    : mpLinks(new LinkMap())
{
}




FocusManager::~FocusManager (void)
{
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
    // Register this focus manager as event listener at the source window.
    if (mpLinks->equal_range(pSource).first == mpLinks->end())
        pSource->AddEventListener (LINK (this, FocusManager, WindowEventListener));
    mpLinks->insert(LinkMap::value_type(pSource, EventDescriptor(rKey,pTarget)));
}




void FocusManager::RemoveLinks (
    ::Window* pSourceWindow,
    ::Window* pTargetWindow)
{
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
    // Remove the links from the given window.
    ::std::pair<LinkMap::iterator,LinkMap::iterator> aCandidates(mpLinks->equal_range(pWindow));
    mpLinks->erase(aCandidates.first, aCandidates.second);
    pWindow->RemoveEventListener (LINK (this, FocusManager, WindowEventListener));

    // Remove links to the given window.
    bool bLinkRemoved (false);
    do
    {
        LinkMap::iterator iLink;
        for (iLink=mpLinks->begin(); iLink!=mpLinks->end(); ++iLink)
        {
            if (iLink->second.mpTargetWindow == pWindow)
            {
                mpLinks->erase(iLink);
                RemoveUnusedEventListener(iLink->first);
                bLinkRemoved;
                break;
            }
        }
    }
    while (bLinkRemoved);
}




void FocusManager::RemoveUnusedEventListener (::Window* pWindow)
{
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

    ::std::pair<LinkMap::iterator,LinkMap::iterator> aCandidates (
        mpLinks->equal_range(pSourceWindow));
    LinkMap::const_iterator iCandidate;
    for (iCandidate=aCandidates.first; iCandidate!=aCandidates.second; ++iCandidate)
        if (iCandidate->second.maKeyCode == rKeyCode)
        {
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
                Window* pSource = pWindowEvent->GetWindow();
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
