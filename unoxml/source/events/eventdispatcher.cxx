#include "eventdispatcher.hxx"
#include "mutationevent.hxx"
#include "../dom/node.hxx"

namespace DOM { namespace events {

    TypeListenerMap CEventDispatcher::captureListeners;
    TypeListenerMap CEventDispatcher::targetListeners;

    void CEventDispatcher::addListener(xmlNodePtr pNode, EventType aType, const Reference<XEventListener>& aListener, sal_Bool bCapture)
    {
        TypeListenerMap* pTMap = &targetListeners;
        if (bCapture) pTMap = &captureListeners;        

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

    void CEventDispatcher::removeListener(xmlNodePtr pNode, EventType aType, const Reference<XEventListener>& aListener, sal_Bool bCapture)
    {
        TypeListenerMap *pTMap = &targetListeners;
        if (bCapture) pTMap = &captureListeners;
        
        // get the multimap for the specified type
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter != pTMap->end()) {
            ListenerMap *pMap = tIter->second;
            // find listeners of specied type for specified node
            ListenerMap::iterator iter = pMap->find(pNode);
            ListenerMap::const_iterator ibound = pMap->upper_bound(pNode);
            while (iter != ibound)
            {
                // erase all references to specified listener
                if((iter->second).is() && (iter->second) == aListener)
                {
                    ListenerMap::iterator i2 = iter;
                    iter++;
                    pMap->erase(i2);
                }
                else
                    iter++;
            }
        }
    }
    
    void CEventDispatcher::callListeners(xmlNodePtr pNode, EventType aType, const Reference< XEvent >& xEvent, sal_Bool bCapture)
    {        
        TypeListenerMap *pTMap = &targetListeners;
        if (bCapture) pTMap = &captureListeners;
        
        // get the multimap for the specified type
        TypeListenerMap::const_iterator tIter = pTMap->find(aType);
        if (tIter != pTMap->end()) {
            ListenerMap *pMap = tIter->second;
            ListenerMap::const_iterator iter = pMap->find(pNode);
            if( iter == pMap->end() ) return;
            ListenerMap::const_iterator ibound = pMap->upper_bound(pNode);
            while (iter != ibound)
            {
                if((iter->second).is())
                {                    
                    (iter->second)->handleEvent(xEvent);
                }
                iter++;
            }
        }
    }

    sal_Bool CEventDispatcher::dispatchEvent(xmlNodePtr aNodePtr, const Reference< XEvent >& aEvent)
    {
        EventType aType = aEvent->getType();
        switch (aType)
        {
            case EventType_DOMSubtreeModified:
            case EventType_DOMNodeInserted:
            case EventType_DOMNodeRemoved:
            case EventType_DOMNodeRemovedFromDocument:
            case EventType_DOMNodeInsertedIntoDocument:
            case EventType_DOMAttrModified:
            case EventType_DOMCharacterDataModified:
            {                
                Reference< XMutationEvent > aMEvent(aEvent, UNO_QUERY);
                // dispatch a mutation event
                // we need to clone the event in order to have complete control 
                // over the implementation
                CMutationEvent* pEvent = new CMutationEvent;
                pEvent->m_target = aEvent->getTarget();
                pEvent->m_currentTarget = aEvent->getCurrentTarget();
                pEvent->m_time = aEvent->getTimeStamp();
                pEvent->initMutationEvent(
                    aType, aMEvent->getBubbles(), aMEvent->getCancelable(),
                    aMEvent->getRelatedNode(), aMEvent->getPrevValue(),
                    aMEvent->getNewValue(), aMEvent->getAttrName(), 
                    aMEvent->getAttrChange());    
                Reference< XEvent > xEvent(static_cast< CEvent* >(pEvent));

                // build the path from target node to the root
                NodeVector captureVector;  
                Reference< XUnoTunnel > aTunnel(xEvent->getTarget(), UNO_QUERY_THROW);
                xmlNodePtr cur = (xmlNodePtr)aTunnel->getSomething(Sequence< sal_Int8 >());
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
                NodeVector::const_iterator inode;                
                
                // start at the root
                inode = captureVector.end();
                inode--;
                if (inode != captureVector.end())
                {
                    // capturing phase:
                    pEvent->m_phase = PhaseType_CAPTURING_PHASE;
                    while (inode != captureVector.begin())
                    {  
                        //pEvent->m_currentTarget = *inode;
                        pEvent->m_currentTarget = Reference< XEventTarget >(CNode::get(*inode));
                        callListeners(*inode, aType, xEvent, sal_True);
                        inode--;
                    }

                    // target phase
                    pEvent->m_phase = PhaseType_AT_TARGET;
                    callListeners(*inode, aType, xEvent, sal_False);
                    // bubbeling phase
                    inode++;
                    if (aEvent->getBubbles()) {
                        pEvent->m_phase = PhaseType_BUBBLING_PHASE;
                        while (inode != captureVector.end())
                        {
                            pEvent->m_currentTarget = Reference< XEventTarget >(CNode::get(*inode));
                            callListeners(*inode, aType, xEvent, sal_False);
                            inode++;
                        }
                    }
                }                
            }
            break;
        }
        return sal_True;
    }
}}