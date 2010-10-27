/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef __XFORMSEVENT_HXX
#define __XFORMSEVENT_HXX

#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xforms/XFormsEvent.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>

namespace com {
namespace sun {
namespace star {
namespace xforms {

class XFormsEventConcrete : public cppu::WeakImplHelper1< XFormsEvent > {

    public:

        typedef com::sun::star::uno::RuntimeException RuntimeException_t;
        typedef com::sun::star::uno::Reference< com::sun::star::xml::dom::events::XEventTarget > XEventTarget_t;
        typedef com::sun::star::xml::dom::events::PhaseType PhaseType_t;
        typedef com::sun::star::util::Time Time_t;

        inline XFormsEventConcrete( void ) : m_canceled(sal_False) {}
        virtual ~XFormsEventConcrete( void ) {}

        virtual rtl::OUString SAL_CALL getType() throw (RuntimeException_t);
        virtual XEventTarget_t SAL_CALL getTarget() throw (RuntimeException_t);
        virtual XEventTarget_t SAL_CALL getCurrentTarget() throw (RuntimeException_t);
        virtual PhaseType_t SAL_CALL getEventPhase() throw (RuntimeException_t);
        virtual sal_Bool SAL_CALL getBubbles() throw (RuntimeException_t);
        virtual sal_Bool SAL_CALL getCancelable() throw (RuntimeException_t);
        virtual Time_t SAL_CALL getTimeStamp() throw (RuntimeException_t);
        virtual void SAL_CALL stopPropagation() throw (RuntimeException_t);
        virtual void SAL_CALL preventDefault() throw (RuntimeException_t);

        virtual void SAL_CALL initXFormsEvent(
                            const rtl::OUString& typeArg,
                            sal_Bool canBubbleArg,
                            sal_Bool cancelableArg )
                            throw (RuntimeException_t);

        virtual void SAL_CALL initEvent(
            const rtl::OUString& eventTypeArg,
            sal_Bool canBubbleArg,
            sal_Bool cancelableArg)
            throw (RuntimeException_t);

    private:

        sal_Bool m_canceled;

    protected:

        rtl::OUString m_eventType;
        XEventTarget_t m_target;
        XEventTarget_t m_currentTarget;
        PhaseType_t m_phase;
        sal_Bool m_bubbles;
        sal_Bool m_cancelable;
        Time_t m_time;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
