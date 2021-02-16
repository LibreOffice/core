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

#include <eventdispatcher.hxx>

#include <event.hxx>
#include <mutationevent.hxx>
#include <uievent.hxx>
#include <mouseevent.hxx>

#include "../dom/document.hxx"

#include <osl/mutex.hxx>

using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::dom::events;

namespace DOM::events {

    void CEventDispatcher::addListener(xmlNodePtr pNode, const OUString& aType, const Reference<XEventListener>& aListener, bool bCapture)
    {
        TypeListenerMap *const pTMap = bCapture
            ? (& m_CaptureListeners) : (& m_TargetListeners);

        // get the multimap for the specified type
        ListenerMap *pMap = nullptr;
        auto tIter = pTMap->find(aType);
        if (tIter == pTMap->end()) {
            // the map has to be created
            auto const pair = pTMap->emplace(aType, ListenerMap());
            pMap = & pair.first->second;
        } else {
            pMap = & tIter->second;
        }
        assert(pMap != nullptr);
        pMap->emplace(pNode, aListener);
    }

    void CEventDispatcher::removeListener(xmlNodePtr pNode, const OUString& aType, const Reference<XEventListener>& aListener, bool bCapture)
    {
        TypeListenerMap *const pTMap = bCapture
            ? (& m_CaptureListeners) : (& m_TargetListeners);

        // get the multimap for the specified type
        auto tIter = pTMap->find(aType);
        if (tIter == pTMap->end())
            return;

        ListenerMap & rMap = tIter->second;
        // find listeners of specified type for specified node
        ListenerMap::iterator iter = rMap.find(pNode);
        while (iter != rMap.end() && iter->first == pNode)
        {
            // erase all references to specified listener
            if (iter->second.is() && iter->second == aListener)
            {
                iter = rMap.erase(iter);
            }
            else
                ++iter;
        }
    }

    CEventDispatcher::~CEventDispatcher()
    {
    }

    void CEventDispatcher::callListeners(
            TypeListenerMap const& rTMap,
            xmlNodePtr const pNode,
            const OUString& aType, Reference< XEvent > const& xEvent)
    {
        // get the multimap for the specified type
        TypeListenerMap::const_iterator tIter = rTMap.find(aType);
        if (tIter != rTMap.end()) {
            ListenerMap const& rMap = tIter->second;
            auto iterRange = rMap.equal_range(pNode);
            for( auto iter = iterRange.first; iter != iterRange.second; ++iter )
            {
                if(iter->second.is())
                    (iter->second)->handleEvent(xEvent);
            }
        }
    }

    void CEventDispatcher::dispatchEvent(
            DOM::CDocument & rDocument, ::osl::Mutex & rMutex,
            xmlNodePtr const pNode, Reference<XNode> const& xNode,
            Reference< XEvent > const& i_xEvent) const
    {
        TypeListenerMap captureListeners;
        TypeListenerMap targetListeners;
        {
            ::osl::MutexGuard g(rMutex);

            captureListeners = m_CaptureListeners;
            targetListeners = m_TargetListeners;
        }

        if (captureListeners.empty() && targetListeners.empty())
            return;

        CEvent *pEvent = nullptr; // pointer to internal event representation

        OUString const aType = i_xEvent->getType();
        if (aType == "DOMSubtreeModified"          ||
            aType == "DOMNodeInserted"             ||
            aType == "DOMNodeRemoved"              ||
            aType == "DOMNodeRemovedFromDocument"  ||
            aType == "DOMNodeInsertedIntoDocument" ||
            aType == "DOMAttrModified"             ||
            aType == "DOMCharacterDataModified"    )
        {
                Reference< XMutationEvent > const aMEvent(i_xEvent,
                        UNO_QUERY_THROW);
                // dispatch a mutation event
                // we need to clone the event in order to have complete control
                // over the implementation
                CMutationEvent* pMEvent = new CMutationEvent;
                pMEvent->initMutationEvent(
                    aType, aMEvent->getBubbles(), aMEvent->getCancelable(),
                    aMEvent->getRelatedNode(), aMEvent->getPrevValue(),
                    aMEvent->getNewValue(), aMEvent->getAttrName(),
                    aMEvent->getAttrChange());
                pEvent = pMEvent;
        } else if ( // UIEvent
            aType == "DOMFocusIn"  ||
            aType == "DOMFocusOut" ||
            aType == "DOMActivate" )
        {
            Reference< XUIEvent > const aUIEvent(i_xEvent, UNO_QUERY_THROW);
            CUIEvent* pUIEvent = new CUIEvent;
            pUIEvent->initUIEvent(aType,
                aUIEvent->getBubbles(), aUIEvent->getCancelable(),
                aUIEvent->getView(), aUIEvent->getDetail());
            pEvent = pUIEvent;
        } else if ( // MouseEvent
            aType == "click"     ||
            aType == "mousedown" ||
            aType == "mouseup"   ||
            aType == "mouseover" ||
            aType == "mousemove" ||
            aType == "mouseout"  )
        {
            Reference< XMouseEvent > const aMouseEvent(i_xEvent,
                    UNO_QUERY_THROW);
            CMouseEvent *pMouseEvent = new CMouseEvent;
            pMouseEvent->initMouseEvent(aType,
                aMouseEvent->getBubbles(), aMouseEvent->getCancelable(),
                aMouseEvent->getView(), aMouseEvent->getDetail(),
                aMouseEvent->getScreenX(), aMouseEvent->getScreenY(),
                aMouseEvent->getClientX(), aMouseEvent->getClientY(),
                aMouseEvent->getCtrlKey(), aMouseEvent->getAltKey(),
                aMouseEvent->getShiftKey(), aMouseEvent->getMetaKey(),
                aMouseEvent->getButton(), aMouseEvent->getRelatedTarget());
            pEvent = pMouseEvent;
        }
        else // generic event
        {
            pEvent = new CEvent;
            pEvent->initEvent(
                aType, i_xEvent->getBubbles(), i_xEvent->getCancelable());
        }
        pEvent->m_target.set(xNode, UNO_QUERY_THROW);
        pEvent->m_currentTarget = i_xEvent->getCurrentTarget();
        pEvent->m_time = i_xEvent->getTimeStamp();

        // create the reference to the private event implementation
        // that will be dispatched to the listeners
        Reference< XEvent > const xEvent(pEvent);

        // build the path from target node to the root
        typedef std::vector< ::std::pair<Reference<XEventTarget>, xmlNodePtr> >
            NodeVector_t;
        NodeVector_t captureVector;
        {
            ::osl::MutexGuard g(rMutex);

            xmlNodePtr cur = pNode;
            while (cur != nullptr)
            {
                Reference< XEventTarget > const xRef(
                        rDocument.GetCNode(cur));
                captureVector.emplace_back(xRef, cur);
                cur = cur->parent;
            }
        }

        // the capture vector now holds the node path from target to root
        // first we must search for capture listeners in order root to
        // to target. after that, any target listeners have to be called
        // then bubbeling phase listeners are called in target to root
        // order
        // start at the root
        NodeVector_t::const_reverse_iterator rinode =
            const_cast<NodeVector_t const&>(captureVector).rbegin();
        if (rinode == const_cast<NodeVector_t const&>(captureVector).rend())
            return;

        // capturing phase:
        pEvent->m_phase = PhaseType_CAPTURING_PHASE;
        while (rinode !=
                const_cast<NodeVector_t const&>(captureVector).rend())
        {
            pEvent->m_currentTarget = rinode->first;
            callListeners(captureListeners, rinode->second, aType, xEvent);
            if  (pEvent->m_canceled) return;
            ++rinode;
        }

        NodeVector_t::const_iterator inode = captureVector.begin();

        // target phase
        pEvent->m_phase = PhaseType_AT_TARGET;
        pEvent->m_currentTarget = inode->first;
        callListeners(targetListeners, inode->second, aType, xEvent);
        if  (pEvent->m_canceled) return;
        // bubbeling phase
        ++inode;
        if (i_xEvent->getBubbles()) {
            pEvent->m_phase = PhaseType_BUBBLING_PHASE;
            while (inode != captureVector.end())
            {
                pEvent->m_currentTarget = inode->first;
                callListeners(targetListeners,
                        inode->second, aType, xEvent);
                if  (pEvent->m_canceled) return;
                ++inode;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
