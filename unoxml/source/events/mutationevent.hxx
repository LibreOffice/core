#ifndef __MUTATIONEVENT_HXX
#define __MUTATIONEVENT_HXX

#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/events/EventType.hpp>
#include <com/sun/star/xml/dom/events/PhaseType.hpp>
#include <com/sun/star/xml/dom/events/AttrChangeType.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XMutationEvent.hpp>
#include "event.hxx"

using ::rtl::OUString;

namespace DOM { namespace events {

class CMutationEvent : public cppu::ImplInheritanceHelper1< CEvent, XMutationEvent >
{
    friend class CEventDispatcher;
protected:
    Reference< XNode > m_relatedNode;
    OUString m_prevValue;
    OUString m_newValue;
    OUString m_attrName;
    AttrChangeType m_attrChangeType;

public:

    virtual ~CMutationEvent();

    virtual Reference< XNode > SAL_CALL getRelatedNode() throw (RuntimeException);
    virtual OUString SAL_CALL getPrevValue() throw (RuntimeException);
    virtual OUString SAL_CALL getNewValue() throw (RuntimeException);
    virtual OUString SAL_CALL getAttrName() throw (RuntimeException);
    virtual AttrChangeType SAL_CALL getAttrChange() throw (RuntimeException);
    virtual void SAL_CALL initMutationEvent(
                           const OUString& typeArg,
                           sal_Bool canBubbleArg,
                           sal_Bool cancelableArg,
                           const Reference< XNode >& relatedNodeArg,
                           const OUString& prevValueArg,
                           const OUString& newValueArg,
                           const OUString& attrNameArg,
                           AttrChangeType attrChangeArg) throw (RuntimeException);

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
