/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "event.hxx"
#include "uievent.hxx"

namespace DOM { namespace events
{    

    Reference< XAbstractView > SAL_CALL CUIEvent::getView() throw(RuntimeException)
    {
        return m_view;
    }

    sal_Int32 SAL_CALL CUIEvent::getDetail() throw(RuntimeException)
    {
        return m_detail;
    }

    void SAL_CALL CUIEvent::initUIEvent(const OUString& typeArg, 
                     sal_Bool canBubbleArg, 
                     sal_Bool cancelableArg, 
                     const Reference< XAbstractView >& viewArg, 
                     sal_Int32 detailArg) throw(RuntimeException)
    {
        initEvent(typeArg, canBubbleArg, cancelableArg);
        m_view = viewArg;
        m_detail = detailArg;
    }


    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    OUString SAL_CALL CUIEvent::getType() throw (RuntimeException)
    {
        return CEvent::getType();
    }

    Reference< XEventTarget > SAL_CALL CUIEvent::getTarget() throw (RuntimeException)
    {
        return CEvent::getTarget();
    }

    Reference< XEventTarget > SAL_CALL CUIEvent::getCurrentTarget() throw (RuntimeException)
    {
        return CEvent::getCurrentTarget();
    }

    PhaseType SAL_CALL CUIEvent::getEventPhase() throw (RuntimeException)
    {
        return CEvent::getEventPhase();
    }

    sal_Bool SAL_CALL CUIEvent::getBubbles() throw (RuntimeException)
    {
        return CEvent::getBubbles();
    }

    sal_Bool SAL_CALL CUIEvent::getCancelable() throw (RuntimeException)
    {
        // mutation events cannot be canceled
        return sal_False;
    }

    com::sun::star::util::Time SAL_CALL CUIEvent::getTimeStamp() throw (RuntimeException)
    {
        return CEvent::getTimeStamp();
    }

    void SAL_CALL CUIEvent::stopPropagation() throw (RuntimeException)
    {
        CEvent::stopPropagation();
    }
    void SAL_CALL CUIEvent::preventDefault() throw (RuntimeException)
    {
        CEvent::preventDefault();
    }

    void SAL_CALL CUIEvent::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg, 
        sal_Bool cancelableArg) throw (RuntimeException)
    {
        // base initializer
        CEvent::initEvent(eventTypeArg, canBubbleArg, cancelableArg);
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
