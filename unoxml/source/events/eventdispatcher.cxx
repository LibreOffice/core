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

        // get the multimap for the specified type
        ListenerMap *pMap = 0;
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter == pTMap->end()) {
            // the map has to be created
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

        // get the multimap for the specified type
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter != pTMap->end()) {
            ListenerMap *pMap = tIter->second;
            // find listeners of specied type for specified node
            ListenerMap::iterator iter = pMap->find(pNode);
            while (iter != pMap->end() && iter->first == pNode)
            {
                // erase all references to specified listener
                if ((iter->second).is() && iter->second == aListener)
                {
                    ListenerMap::iterator tmp_iter = iter;
                    iter++;
                    pMap->erase(tmp_iter);
                }
                else
                    iter++;
            }
        }
    }

    void CEventDispatcher::callListeners(xmlNodePtr const pNode,
            OUString aType, Reference< XEvent > const& xEvent,
            sal_Bool const bCapture) const
    {
        TypeListenerMap const*const pTMap = (bCapture)
            ? (& m_CaptureListeners) : (& m_TargetListeners);

        // get the multimap for the specified type
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter != pTMap->end()) {
            ListenerMap *pMap = tIter->second;
            ListenerMap::const_iterator iter = pMap->lower_bound(pNode);
            ListenerMap::const_iterator ibound = pMap->upper_bound(pNode);
            for( ; iter != ibound; iter++ )
            {
                if((iter->second).is())
                    (iter->second)->handleEvent(xEvent);
            }
        }
    }

    bool CEventDispatcher::dispatchEvent(DOM::CDocument & rDocument,
            xmlNodePtr const pNode, Reference<XNode> const& xNode,
            Reference< XEvent > const& i_xEvent) const
    {
        CEvent *pEvent = 0; // pointer to internal event representation

        OUString const aType = i_xEvent->getType();
        if (aType.compareToAscii("DOMSubtreeModified")          == 0||
            aType.compareToAscii("DOMNodeInserted")             == 0||
            aType.compareToAscii("DOMNodeRemoved")              == 0||
            aType.compareToAscii("DOMNodeRemovedFromDocument")  == 0||
            aType.compareToAscii("DOMNodeInsertedIntoDocument") == 0||
            aType.compareToAscii("DOMAttrModified")             == 0||
            aType.compareToAscii("DOMCharacterDataModified")    == 0)
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
            aType.compareToAscii("DOMFocusIn")  == 0||
            aType.compareToAscii("DOMFocusOut") == 0||
            aType.compareToAscii("DOMActivate") == 0)
        {
            Reference< XUIEvent > const aUIEvent(i_xEvent, UNO_QUERY_THROW);
            CUIEvent* pUIEvent = new CUIEvent;
            pUIEvent->initUIEvent(aType,
                aUIEvent->getBubbles(), aUIEvent->getCancelable(),
                aUIEvent->getView(), aUIEvent->getDetail());
            pEvent = pUIEvent;
        } else if ( // MouseEvent
            aType.compareToAscii("click")     == 0||
            aType.compareToAscii("mousedown") == 0||
            aType.compareToAscii("mouseup")   == 0||
            aType.compareToAscii("mouseover") == 0||
            aType.compareToAscii("mousemove") == 0||
            aType.compareToAscii("mouseout")  == 0)
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

        // create the reference to the provate event implementation
        // that will be dispatched to the listeners
        Reference< XEvent > const xEvent(pEvent);

        // build the path from target node to the root
        NodeVector captureVector;
        xmlNodePtr cur = pNode;
        while (cur != NULL)
        {
            captureVector.push_back(cur);
            cur = cur->parent;
        }

        // the caputre vector now holds the node path from target to root
        // first we must search for capture listernes in order root to
        // to target. after that, any target listeners have to be called
        // then bubbeling phase listeners are called in target to root
        // order
        // start at the root
        NodeVector::const_reverse_iterator rinode =
            const_cast<const NodeVector&>(captureVector).rbegin();
        if (rinode != const_cast<const NodeVector&>(captureVector).rend())
        {
            // capturing phase:
            pEvent->m_phase = PhaseType_CAPTURING_PHASE;
            while (rinode !=
                    const_cast<const NodeVector&>(captureVector).rend())
            {
                pEvent->m_currentTarget = Reference< XEventTarget >(
                        rDocument.GetCNode(*rinode).get());
                callListeners(*rinode, aType, xEvent, sal_True);
                if  (pEvent->m_canceled) return sal_True;
                rinode++;
            }

            NodeVector::const_iterator inode = captureVector.begin();

            // target phase
            pEvent->m_phase = PhaseType_AT_TARGET;
            callListeners(*inode, aType, xEvent, sal_False);
            if  (pEvent->m_canceled) return sal_True;
            // bubbeling phase
            inode++;
            if (i_xEvent->getBubbles()) {
                pEvent->m_phase = PhaseType_BUBBLING_PHASE;
                while (inode != captureVector.end())
                {
                    pEvent->m_currentTarget = Reference< XEventTarget >(
                            rDocument.GetCNode(*inode).get());
                    callListeners(*inode, aType, xEvent, sal_False);
                    if  (pEvent->m_canceled) return sal_True;
                    inode++;
                }
            }
        }
        return sal_True;
    }
}}
