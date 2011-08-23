/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __UIEVENT_HXX
#define __UIEVENT_HXX

#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/events/EventType.hpp>
#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/AttrChangeType.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XUIEvent.hpp>
#include <com/sun/star/xml/dom/views/XAbstractView.hpp>
#include "event.hxx"

using ::rtl::OUString;
using namespace com::sun::star::xml::dom::views;

namespace DOM { namespace events {

class CUIEvent : public cppu::ImplInheritanceHelper1< CEvent, XUIEvent >
{
    friend class CEventDispatcher;
protected:
    sal_Int32 m_detail;
    Reference< XAbstractView > m_view;

public:
    virtual Reference< XAbstractView > SAL_CALL getView() throw(RuntimeException);
    virtual sal_Int32 SAL_CALL getDetail() throw(RuntimeException);
    virtual void SAL_CALL initUIEvent(const OUString& typeArg, 
                     sal_Bool canBubbleArg, 
                     sal_Bool cancelableArg, 
                     const Reference< XAbstractView >& viewArg, 
                     sal_Int32 detailArg) throw(RuntimeException);

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
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
        sal_Bool cancelableArg) 
        throw (RuntimeException);    
};
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
