/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __EVENT_HXX
#define __EVENT_HXX

#include <sal/types.h>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/util/Time.hpp>

#include "../dom/node.hxx"

#include <libxml/tree.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;


namespace DOM {namespace events
{
class CEvent : public cppu::WeakImplHelper1< XEvent >
{
friend class CEventDispatcher;
friend class CNode;
friend class CDocument;
friend class CElement;
friend class CText;
friend class CCharacterData;
friend class CAttr;


private:
    sal_Bool m_canceled;

protected:
    OUString m_eventType;
    Reference< XEventTarget > m_target;
    Reference< XEventTarget > m_currentTarget;
    //xmlNodePtr m_target;
    //xmlNodePtr m_currentTarget;
    PhaseType m_phase;
    sal_Bool m_bubbles;
    sal_Bool m_cancelable;
    com::sun::star::util::Time m_time;

public:

    CEvent() : m_canceled(sal_False){}

    virtual ~CEvent();
    virtual OUString SAL_CALL getType() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getTarget() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getCurrentTarget() throw (RuntimeException);
    virtual PhaseType SAL_CALL getEventPhase() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException);
    virtual com::sun::star::util::Time SAL_CALL getTimeStamp() throw (RuntimeException);
    virtual void SAL_CALL stopPropagation() throw (RuntimeException);
    virtual void SAL_CALL preventDefault() throw (RuntimeException);
    virtual void SAL_CALL initEvent(
        const OUString& eventTypeArg, 
        sal_Bool canBubbleArg, 
        sal_Bool cancelableArg)  throw (RuntimeException);    
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
