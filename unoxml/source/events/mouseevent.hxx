#ifndef __MOUSEEVENT_HXX
#define __MOUSEEVENT_HXX

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
#include <com/sun/star/xml/dom/events/XMouseEvent.hpp>
#include "event.hxx"
#include "uievent.hxx"

using ::rtl::OUString;

namespace DOM { namespace events {

class CMouseEvent : public cppu::ImplInheritanceHelper1< CUIEvent, XMouseEvent >
{
    friend class CEventDispatcher;
protected:
    sal_Int32 m_screenX;
    sal_Int32 m_screenY;
    sal_Int32 m_clientX;
    sal_Int32 m_clientY;
    sal_Bool m_ctrlKey;
    sal_Bool m_shiftKey;
    sal_Bool m_altKey;
    sal_Bool m_metaKey;
    sal_Int16 m_button;
    Reference< XEventTarget > m_relatedTarget;

public:

    virtual sal_Int32 SAL_CALL getScreenX() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getScreenY() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getClientX() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getClientY() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getCtrlKey() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getShiftKey() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getAltKey() throw (RuntimeException);
    virtual sal_Bool SAL_CALL getMetaKey() throw (RuntimeException);
    virtual sal_Int16 SAL_CALL getButton() throw (RuntimeException);
    virtual Reference< XEventTarget > SAL_CALL getRelatedTarget()  throw(RuntimeException);

    virtual void SAL_CALL initMouseEvent(
                        const OUString& typeArg, 
                        sal_Bool canBubbleArg, 
                        sal_Bool cancelableArg, 
                        const Reference< XAbstractView >& viewArg, 
                        sal_Int32 detailArg, 
                        sal_Int32 screenXArg, 
                        sal_Int32 screenYArg, 
                        sal_Int32 clientXArg, 
                        sal_Int32 clientYArg, 
                        sal_Bool ctrlKeyArg, 
                        sal_Bool altKeyArg, 
                        sal_Bool shiftKeyArg, 
                        sal_Bool metaKeyArg, 
                        sal_Int16 buttonArg, 
                        const Reference< XEventTarget >& relatedTargetArg)
        throw(RuntimeException);

    // delegate to CUIevent    
    virtual Reference< XAbstractView > SAL_CALL getView() throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getDetail() throw (RuntimeException);
    virtual void SAL_CALL initUIEvent(const OUString& typeArg, 
                     sal_Bool canBubbleArg, 
                     sal_Bool cancelableArg, 
                     const Reference< XAbstractView >& viewArg, 
                     sal_Int32 detailArg) throw (RuntimeException);
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
