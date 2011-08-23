/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "event.hxx"

namespace DOM { namespace events 
{

    CEvent::~CEvent()
    {
    }

    OUString SAL_CALL CEvent::getType() throw (RuntimeException)
    {
        return m_eventType;
    }

    Reference< XEventTarget > SAL_CALL CEvent::getTarget() throw (RuntimeException)
    {
        return m_target;
    }

    Reference< XEventTarget > SAL_CALL CEvent::getCurrentTarget() throw (RuntimeException)
    {
        return m_currentTarget;
    }

    PhaseType SAL_CALL CEvent::getEventPhase() throw (RuntimeException)
    {
        return m_phase;
    }

    sal_Bool SAL_CALL CEvent::getBubbles() throw (RuntimeException)
    {
        return m_bubbles;
    }

    sal_Bool SAL_CALL CEvent::getCancelable() throw (RuntimeException)
    {
        return m_cancelable;
    }

    com::sun::star::util::Time SAL_CALL CEvent::getTimeStamp() throw (RuntimeException)
    {
        return m_time;
    }

    void SAL_CALL CEvent::stopPropagation() throw (RuntimeException)
    {
        if (m_cancelable) m_canceled = sal_True;
    }

    void SAL_CALL CEvent::preventDefault() throw (RuntimeException)
    {
    }

    void SAL_CALL CEvent::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg, 
        sal_Bool cancelableArg) throw (RuntimeException)
    {
        m_eventType = eventTypeArg;
        m_bubbles = canBubbleArg;
        m_cancelable = cancelableArg;
    }

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
