/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <eventdispatcher.hxx>

#include <event.hxx>
#include <mutationevent.hxx>
#include <uievent.hxx>
#include <mouseevent.hxx>

#include "../dom/document.hxx"


namespace DOM { namespace events {

    void CEventDispatcher::addListener(xmlNodePtr pNode, OUString aType, const Reference<XEventListener>& aListener, sal_Bool bCapture)
    {
        TypeListenerMap *const pTMap = (bCapture)
            ? (& m_CaptureListeners) : (& m_TargetListeners);

        
        ListenerMap *pMap = 0;
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter == pTMap->end()) {
            
            pMap = new ListenerMap();
            pTMap->insert(TypeListenerMap::value_type(aType, pMap));
        } else {
            pMap = tIter->second;
        }
        if (pMap !=0)
            pMap->insert(ListenerMap::value_type(pNode, aListener));
    }

    void CEventDispatcher::removeListener(xmlNodePtr pNode, OUString aType, const Reference<XEventListener>& aListener, sal_Bool bCapture)
    {
        TypeListenerMap *const pTMap = (bCapture)
            ? (& m_CaptureListeners) : (& m_TargetListeners);

        
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter != pTMap->end()) {
            ListenerMap *pMap = tIter->second;
            
            ListenerMap::iterator iter = pMap->find(pNode);
            while (iter != pMap->end() && iter->first == pNode)
            {
                
                if ((iter->second).is() && iter->second == aListener)
                {
                    ListenerMap::iterator tmp_iter = iter;
                    ++iter;
                    pMap->erase(tmp_iter);
                }
                else
                    ++iter;
            }
        }
    }

    CEventDispatcher::~CEventDispatcher()
    {
        
        for (TypeListenerMap::iterator aI = m_CaptureListeners.begin(); aI != m_CaptureListeners.end(); ++aI)
            delete aI->second;

        for (TypeListenerMap::iterator aI = m_TargetListeners.begin(); aI != m_TargetListeners.end(); ++aI)
            delete aI->second;
    }

    void CEventDispatcher::callListeners(
            TypeListenerMap const& rTMap,
            xmlNodePtr const pNode,
            OUString aType, Reference< XEvent > const& xEvent)
    {
        
        TypeListenerMap::const_iterator tIter = rTMap.find(aType);
        if (tIter != rTMap.end()) {
            ListenerMap *pMap = tIter->second;
            ListenerMap::const_iterator iter = pMap->lower_bound(pNode);
            ListenerMap::const_iterator ibound = pMap->upper_bound(pNode);
            for( ; iter != ibound; ++iter )
            {
                if((iter->second).is())
                    (iter->second)->handleEvent(xEvent);
            }
        }
    }

    bool CEventDispatcher::dispatchEvent(
            DOM::CDocument & rDocument, ::osl::Mutex & rMutex,
            xmlNodePtr const pNode, Reference<XNode> const& xNode,
            Reference< XEvent > const& i_xEvent) const
    {
        CEvent *pEvent = 0; 

        OUString const aType = i_xEvent->getType();
        if (aType.equalsAscii("DOMSubtreeModified")          ||
            aType.equalsAscii("DOMNodeInserted")             ||
            aType.equalsAscii("DOMNodeRemoved")              ||
            aType.equalsAscii("DOMNodeRemovedFromDocument")  ||
            aType.equalsAscii("DOMNodeInsertedIntoDocument") ||
            aType.equalsAscii("DOMAttrModified")             ||
            aType.equalsAscii("DOMCharacterDataModified")    )
        {
                Reference< XMutationEvent > const aMEvent(i_xEvent,
                        UNO_QUERY_THROW);
                
                
                
                CMutationEvent* pMEvent = new CMutationEvent;
                pMEvent->initMutationEvent(
                    aType, aMEvent->getBubbles(), aMEvent->getCancelable(),
                    aMEvent->getRelatedNode(), aMEvent->getPrevValue(),
                    aMEvent->getNewValue(), aMEvent->getAttrName(),
                    aMEvent->getAttrChange());
                pEvent = pMEvent;
        } else if ( 
            aType.equalsAscii("DOMFocusIn")  ||
            aType.equalsAscii("DOMFocusOut") ||
            aType.equalsAscii("DOMActivate") )
        {
            Reference< XUIEvent > const aUIEvent(i_xEvent, UNO_QUERY_THROW);
            CUIEvent* pUIEvent = new CUIEvent;
            pUIEvent->initUIEvent(aType,
                aUIEvent->getBubbles(), aUIEvent->getCancelable(),
                aUIEvent->getView(), aUIEvent->getDetail());
            pEvent = pUIEvent;
        } else if ( 
            aType.equalsAscii("click")     ||
            aType.equalsAscii("mousedown") ||
            aType.equalsAscii("mouseup")   ||
            aType.equalsAscii("mouseover") ||
            aType.equalsAscii("mousemove") ||
            aType.equalsAscii("mouseout")  )
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
        else 
        {
            pEvent = new CEvent;
            pEvent->initEvent(
                aType, i_xEvent->getBubbles(), i_xEvent->getCancelable());
        }
        pEvent->m_target.set(xNode, UNO_QUERY_THROW);
        pEvent->m_currentTarget = i_xEvent->getCurrentTarget();
        pEvent->m_time = i_xEvent->getTimeStamp();

        
        
        Reference< XEvent > const xEvent(pEvent);

        
        typedef std::vector< ::std::pair<Reference<XEventTarget>, xmlNodePtr> >
            NodeVector_t;
        NodeVector_t captureVector;
        TypeListenerMap captureListeners;
        TypeListenerMap targetListeners;
        {
            ::osl::MutexGuard g(rMutex);

            xmlNodePtr cur = pNode;
            while (cur != NULL)
            {
                Reference< XEventTarget > const xRef(
                        rDocument.GetCNode(cur).get());
                captureVector.push_back(::std::make_pair(xRef, cur));
                cur = cur->parent;
            }
            captureListeners = m_CaptureListeners;
            targetListeners = m_TargetListeners;
        }

        
        
        
        
        
        
        NodeVector_t::const_reverse_iterator rinode =
            const_cast<NodeVector_t const&>(captureVector).rbegin();
        if (rinode != const_cast<NodeVector_t const&>(captureVector).rend())
        {
            
            pEvent->m_phase = PhaseType_CAPTURING_PHASE;
            while (rinode !=
                    const_cast<NodeVector_t const&>(captureVector).rend())
            {
                pEvent->m_currentTarget = rinode->first;
                callListeners(captureListeners, rinode->second, aType, xEvent);
                if  (pEvent->m_canceled) return true;
                ++rinode;
            }

            NodeVector_t::const_iterator inode = captureVector.begin();

            
            pEvent->m_phase = PhaseType_AT_TARGET;
            pEvent->m_currentTarget = inode->first;
            callListeners(targetListeners, inode->second, aType, xEvent);
            if  (pEvent->m_canceled) return true;
            
            ++inode;
            if (i_xEvent->getBubbles()) {
                pEvent->m_phase = PhaseType_BUBBLING_PHASE;
                while (inode != captureVector.end())
                {
                    pEvent->m_currentTarget = inode->first;
                    callListeners(targetListeners,
                            inode->second, aType, xEvent);
                    if  (pEvent->m_canceled) return true;
                    ++inode;
                }
            }
        }
        return true;
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
