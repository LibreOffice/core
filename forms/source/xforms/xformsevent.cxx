#include "xformsevent.hxx"

namespace com {
namespace sun {
namespace star {
namespace xforms {

using rtl::OUString;
using com::sun::star::uno::RuntimeException;

void SAL_CALL XFormsEventConcrete::initXFormsEvent(const OUString& typeArg,
    sal_Bool canBubbleArg, sal_Bool cancelableArg)
    throw (RuntimeException)
{
    initEvent(typeArg, canBubbleArg, cancelableArg);
}

OUString SAL_CALL XFormsEventConcrete::getType() throw (RuntimeException)
{
    return m_eventType;
}

XFormsEventConcrete::XEventTarget_t SAL_CALL XFormsEventConcrete::getTarget() throw (RuntimeException)
{
    return m_target;
}

XFormsEventConcrete::XEventTarget_t SAL_CALL XFormsEventConcrete::getCurrentTarget() throw (RuntimeException)
{
    return m_currentTarget;
}

XFormsEventConcrete::PhaseType_t SAL_CALL XFormsEventConcrete::getEventPhase() throw (RuntimeException)
{
    return m_phase;
}

sal_Bool SAL_CALL XFormsEventConcrete::getBubbles() throw (RuntimeException)
{
    return m_bubbles;
}

sal_Bool SAL_CALL XFormsEventConcrete::getCancelable() throw (RuntimeException)
{
    return m_cancelable;
}

XFormsEventConcrete::Time_t SAL_CALL XFormsEventConcrete::getTimeStamp() throw (RuntimeException)
{
    return m_time;
}

void SAL_CALL XFormsEventConcrete::stopPropagation() throw (RuntimeException)
{
    if(m_cancelable)
        m_canceled = sal_True;
}
void SAL_CALL XFormsEventConcrete::preventDefault() throw (RuntimeException)
{
}

void SAL_CALL XFormsEventConcrete::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
    sal_Bool cancelableArg) throw (RuntimeException)
{
    m_eventType = eventTypeArg;
    m_bubbles = canBubbleArg;
    m_cancelable = cancelableArg;
}

} } } }
